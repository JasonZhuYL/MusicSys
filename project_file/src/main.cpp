#include <Arduino.h>
#include <U8g2lib.h>
#include <STM32FreeRTOS.h>
#include <ES_CAN.h>
#include <math.h>
#include <knob.h>

// Constants
const uint8_t lpf[10][12] = {
    {10, 10, 10, 10, 10, 10, 10, 10, 10, 9, 8, 7}, {10, 10, 10, 10, 10, 10, 10, 10, 9, 8, 7, 6}, {10, 10, 10, 10, 10, 10, 10, 9, 8, 7, 6, 5}, {10, 10, 10, 10, 10, 10, 9, 8, 7, 6, 5, 4}, {10, 10, 10, 10, 10, 9, 8, 7, 6, 5, 4, 3}, {10, 10, 10, 10, 9, 8, 7, 6, 5, 4, 3, 2}, {10, 10, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1}, {10, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 1}, {10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 1, 1}, {9, 8, 7, 6, 5, 4, 3, 2, 1, 1, 1, 1}};
const uint8_t hpf[10][12] = {
    {7, 8, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10}, {6, 7, 8, 9, 10, 10, 10, 10, 10, 10, 10, 10}, {5, 6, 7, 8, 9, 10, 10, 10, 10, 10, 10, 10}, {4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 10, 10}, {3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 10}, {2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10}, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10}, {1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10}, {1, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, {1, 1, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9}};
SemaphoreHandle_t keypressedMutex;
volatile uint8_t keypressed[12];
volatile uint32_t currentStepSize[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile uint8_t keypressed_pointer = 0;
volatile uint8_t volume = 12; // range from 0 to 16
volatile uint8_t octave = 4;  // range from 0 to 16
volatile uint8_t waveform_mode = 0;
volatile uint8_t reverb_switch = 0;
volatile uint8_t board_location = 1;
uint8_t var0 = 0;

volatile uint16_t keysBytes_Pre = 0xFFFF;
volatile uint16_t keysBytes_B1 = 0xFFFF;
volatile uint16_t keysBytes_B3 = 0xFFFF;

const int32_t stepSizes[] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 0};
const String note[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", ""};
const String Filter_display[] = {"NONE", "LPF", "HPF"};
const String wave_display[] = {"Sawtooth", "Triangular", "Square"};
const String reverb_display[] = {"Reverb off", "Reverb on"};
uint8_t filter_mode = 0;
uint8_t TX_Message[8] = {0};
QueueHandle_t msgInQ;
QueueHandle_t msgOutQ;
SemaphoreHandle_t CAN_TX_Semaphore;

// Initialize knob decoder
knob_decoder *Decoder3 = new knob_decoder(12, 16, 0);
knob_decoder *Decoder2 = new knob_decoder(4, 11, 0);
knob_decoder *Decoder1 = new knob_decoder(0, 2, 0);
knob_decoder *Decoder0 = new knob_decoder(0, 9, 0);

// Pin definitions
// Row select and enable
const int RA0_PIN = D3;
const int RA1_PIN = D6;
const int RA2_PIN = D12;
const int REN_PIN = A5;

// Matrix input and output
const int C0_PIN = A2;
const int C1_PIN = D9;
const int C2_PIN = A6;
const int C3_PIN = D1;
const int OUT_PIN = D11;

// Audio analogue out
const int OUTL_PIN = A4;
const int OUTR_PIN = A3;

// Joystick analogue in
const int JOYY_PIN = A0;
const int JOYX_PIN = A1;

// Output multiplexer bits
const int DEN_BIT = 3;
const int DRST_BIT = 4;
const int HKOW_BIT = 5;
const int HKOE_BIT = 6;

// Display driver object
U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);

static int32_t phaseAcc[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int32_t VoutReverb[51] = {0};

// CAN part init
uint32_t ID = 0x123;
uint8_t RX_Message[8] = {0};

// Function to set outputs using key matrix
void setOutMuxBit(const uint8_t bitIdx, const bool value)
{
    digitalWrite(REN_PIN, LOW);
    digitalWrite(RA0_PIN, bitIdx & 0x01);
    digitalWrite(RA1_PIN, bitIdx & 0x02);
    digitalWrite(RA2_PIN, bitIdx & 0x04);
    digitalWrite(OUT_PIN, value);
    digitalWrite(REN_PIN, HIGH);
    delayMicroseconds(2);
    digitalWrite(REN_PIN, LOW);
}

uint8_t readCols()
{
    uint8_t C0 = digitalRead(C0_PIN);
    uint8_t C1 = digitalRead(C1_PIN);
    uint8_t C2 = digitalRead(C2_PIN);
    uint8_t C3 = digitalRead(C3_PIN);
    uint8_t byteRead = (C3 << 3) | (C2 << 2) | (C1 << 1) | C0;
    return byteRead;
}

void setRow(uint8_t rowIdx)
{
    digitalWrite(REN_PIN, LOW);
    if (bitRead(rowIdx, 0) == 0)
    {
        digitalWrite(RA0_PIN, LOW);
    }
    else
    {
        digitalWrite(RA0_PIN, HIGH);
    }
    if (bitRead(rowIdx, 1) == 0)
    {
        digitalWrite(RA1_PIN, LOW);
    }
    else
    {
        digitalWrite(RA1_PIN, HIGH);
    }
    if (bitRead(rowIdx, 2) == 0)
    {
        digitalWrite(RA2_PIN, LOW);
    }
    else
    {
        digitalWrite(RA2_PIN, HIGH);
    }
    digitalWrite(REN_PIN, HIGH);
}

void sampleISR()
{
    const uint8_t keypressed_pointer_local = __atomic_load_n(&keypressed_pointer, __ATOMIC_RELAXED);
    const uint8_t waveform_mode_local = __atomic_load_n(&waveform_mode, __ATOMIC_RELAXED);
    const uint8_t volume_local = __atomic_load_n(&volume, __ATOMIC_RELAXED);
    const uint8_t reverb_switch_local = __atomic_load_n(&reverb_switch, __ATOMIC_RELAXED);
    uint8_t keypressed_local[keypressed_pointer_local] = {0};
    for(uint8_t i=0;i<keypressed_pointer_local;i++){
        // xSemaphoreTake(keypressedMutex, portMAX_DELAY);
        keypressed_local[i] = keypressed[i];
        // xSemaphoreGive(keypressedMutex);
    }
    
    int32_t Vout = 0;
    if (waveform_mode_local == 0)
    { // generate sawtooth wave
        if (filter_mode == 2)
        {
            for (uint8_t i = 0; i < keypressed_pointer_local; i++)
            {
                phaseAcc[i] += (currentStepSize[i] * 195225 / 10) * hpf[var0][keypressed_local[i]];
            }
        }
        else if (filter_mode == 1)
        {
            for (uint8_t i = 0; i < keypressed_pointer_local; i++)
            {
                phaseAcc[i] += (currentStepSize[i] * 195225 / 10) * lpf[var0][keypressed_local[i]];
            }
        }
        else
        {
            for (uint8_t i = 0; i < keypressed_pointer_local; i++)
            {
                phaseAcc[i] += (currentStepSize[i] * 195225);
            }
        }

        for (uint8_t i = 0; i < keypressed_pointer_local; i++)
        {
            Vout += (phaseAcc[i] >> 24);
        }
        Vout = Vout >> (8 - int(volume_local) / 2);

        //*******   Reverb  ***********
        if (reverb_switch_local == 1)
        {
            VoutReverb[0] = Vout;
            for (uint8_t i = 50; i > 0; i--)
            {
                VoutReverb[i] = VoutReverb[i - 1];
            }
            Vout = Vout + VoutReverb[45] * 0.7 + VoutReverb[35] * 0.5 + VoutReverb[20] * 0.2;
        }
        //*******   Reverb  ***********
        analogWrite(OUTR_PIN, Vout + 128);
    }
    else if (waveform_mode_local == 1)
    { // generate triangular wave

        if (filter_mode == 2)
        {
            for (uint8_t i = 0; i < keypressed_pointer_local; i++)
            {
                phaseAcc[i] += (currentStepSize[i] * 195225 / 10) * hpf[var0][keypressed_local[i]];
            }
        }
        else if (filter_mode == 1)
        {
            for (uint8_t i = 0; i < keypressed_pointer_local; i++)
            {
                phaseAcc[i] += (currentStepSize[i] * 195225 / 10) * lpf[var0][keypressed_local[i]];
            }
        }
        else
        {
            for (uint8_t i = 0; i < keypressed_pointer_local; i++)
            {
                phaseAcc[i] += (currentStepSize[i] * 195225);
            }
        }
        for (uint8_t i = 0; i < keypressed_pointer_local; i++)
        {
            Vout += (phaseAcc[i] >> 24);
        }

        if (Vout < 0)
        {
            Vout = 128 + (Vout >> (8 - int(volume_local) / 2)) * 4;
        }
        else
        {
            Vout = 127 - (Vout >> (8 - int(volume_local) / 2)) * 4;
        }

        //*******   Reverb  ***********
        if (reverb_switch_local == 1)
        {
            VoutReverb[0] = Vout;
            for (uint8_t i = 50; i > 0; i--)
            {
                VoutReverb[i] = VoutReverb[i - 1];
            }
            Vout = Vout + VoutReverb[45] * 0.7 + VoutReverb[35] * 0.5 + VoutReverb[20] * 0.2;
        }
        //*******   Reverb  ***********
        analogWrite(OUTR_PIN, Vout);
    }
    else if (waveform_mode_local == 2)
    { // generate square wave
        if (filter_mode == 2)
        {
            for (uint8_t i = 0; i < keypressed_pointer_local; i++)
            {
                phaseAcc[i] += (currentStepSize[i] * 195225 / 10) * hpf[var0][keypressed_local[i]];
            }
        }
        else if (filter_mode == 1)
        {
            for (uint8_t i = 0; i < keypressed_pointer_local; i++)
            {
                phaseAcc[i] += (currentStepSize[i] * 195225 / 10) * lpf[var0][keypressed_local[i]];
            }
        }
        else
        {
            for (uint8_t i = 0; i < keypressed_pointer_local; i++)
            {
                phaseAcc[i] += (currentStepSize[i] * 195225);
            }
        }
        int32_t Vout_intermediate = 0;
        for (uint8_t i = 0; i < keypressed_pointer_local; i++)
        {
            Vout_intermediate += (phaseAcc[i] >> 24);
        }
        Vout_intermediate = Vout_intermediate >> (8 - int(volume_local) / 2);
        if (Vout_intermediate > 0)
        {
            Vout = INT32_MAX >> 24;
            Vout = Vout >> (8 - int(volume_local) / 2);
        }
        else
        {
            Vout = INT32_MIN >> 24;
            Vout = Vout >> (8 - int(volume_local) / 2);
        }

        //*******   Reverb  ***********
        if (reverb_switch_local == 1)
        {
            VoutReverb[0] = Vout;
            for (uint8_t i = 50; i > 0; i--)
            {
                VoutReverb[i] = VoutReverb[i - 1];
            }
            Vout = Vout + VoutReverb[45] * 0.7 + VoutReverb[35] * 0.5 + VoutReverb[20] * 0.2;
        }
        //*******   Reverb  ***********
        analogWrite(OUTR_PIN, Vout + 128);
    }
}

int noteDemux(int note)
{
    if (note == 0)
    {
        return 12;
    }
    if (note > 3)
    {
        return (note * 2) - 3;
    }
    else
    {
        return (note * 2) - 1;
    }
}

void play_Twinkle_star()
{
    uint8_t twinkleStar[] = {1, 1, 8, 8, 10, 10, 8, 6, 6, 5, 5, 3, 3, 1, 8, 8, 6, 6, 5, 5, 3, 8, 8, 6, 6, 5, 5, 3, 1, 1, 8, 8, 10, 10, 8, 6, 6, 5, 5, 3, 3, 1};
    __atomic_store_n(&keypressed_pointer, 1, __ATOMIC_RELAXED);
    for (int i = 0; i < 42; i++)
    {
        if ((i + 1) % 7 == 0)
        {
            currentStepSize[0]= stepSizes[twinkleStar[i]];
            delayMicroseconds(450000);
            currentStepSize[0]= 0;
            delayMicroseconds(150000);
        }
        else
        {
            currentStepSize[0]= stepSizes[twinkleStar[i]];
            delayMicroseconds(200000);
            currentStepSize[0]= 0;
            delayMicroseconds(100000);
        }
    }
}

void handshake()
{
    TX_Message[0] = 'H';
    TX_Message[1] = __atomic_load_n(&board_location, __ATOMIC_RELAXED);
    TX_Message[2] = __atomic_load_n(&octave, __ATOMIC_RELAXED);
    TX_Message[3] = __atomic_load_n(&volume, __ATOMIC_RELAXED);
    xQueueSend(msgOutQ, TX_Message, portMAX_DELAY);
    Serial.println("handshake message sent");
}
void configVO()
{
    TX_Message[0] = 'C';
    TX_Message[1] = __atomic_load_n(&board_location, __ATOMIC_RELAXED);
    TX_Message[2] = __atomic_load_n(&octave, __ATOMIC_RELAXED);
    TX_Message[3] = __atomic_load_n(&volume, __ATOMIC_RELAXED);
    TX_Message[4] = __atomic_load_n(&waveform_mode, __ATOMIC_RELAXED);
    TX_Message[5] = __atomic_load_n(&reverb_switch, __ATOMIC_RELAXED);
    xQueueSend(msgOutQ, TX_Message, portMAX_DELAY);
    Serial.println("Configure message sent");
}
void sendNotes()
{
    TX_Message[0] = 'P';
    TX_Message[1] = __atomic_load_n(&board_location, __ATOMIC_RELAXED);
    TX_Message[2] = (__atomic_load_n(&keysBytes_Pre, __ATOMIC_RELAXED) >> 6);
    TX_Message[3] = (__atomic_load_n(&keysBytes_Pre, __ATOMIC_RELAXED) & 0x003F);
    xQueueSend(msgOutQ, TX_Message, portMAX_DELAY);
    Serial.println("Notes pressed message sent");
}

void scanKeysTask(void *pvParameters)
{
    const TickType_t xFrequency = 50 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint16_t keysBytes = 0;
    uint16_t keysBytes_Changed;
    uint8_t keyArray[7];
    // Previous values
    uint8_t waveform_mode_pre = 0;
    uint8_t volume_pre = 12; // range from 0 to 16
    uint8_t octave_pre = 4;  // range from 0 to 16
    uint8_t pos_raw_pre = 3;
    uint8_t keypressed_pointer_pre = 12;
    uint8_t num_keypressed = 0;

    uint8_t keypressed_local[12]={0};
    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        for (uint8_t i = 0; i < 7; i++)
        {
            setRow(i);
            delayMicroseconds(3);
            keyArray[i] = readCols();
        }
        keysBytes = (keyArray[2] << 8) | (keyArray[1] << 4) | keyArray[0];
        // By the XOR opeartion we can see the notes that are changed
        keysBytes_Changed = keysBytes ^ __atomic_load_n(&keysBytes_Pre, __ATOMIC_RELAXED);
        if ((keysBytes_Changed) == 0)
        {
            // keyboard nothing changed we do nothing
        }
        else
        {
            // keyboard changed
            num_keypressed = 0;
            for (uint8_t i = 0; i < 12; i++)
            {
                if (bitRead(keysBytes, i) == 0)
                {
                    keypressed_local[num_keypressed]=i;
                    num_keypressed += 1;
                }
            }
            __atomic_store_n(&keypressed_pointer, num_keypressed, __ATOMIC_RELAXED);
            __atomic_store_n(&keysBytes_Pre, keysBytes, __ATOMIC_RELAXED);
            for(uint8_t i=0;i<num_keypressed;i++){
                // xSemaphoreTake(keypressedMutex, portMAX_DELAY);
                keypressed[i] = keypressed_local[i];
                // xSemaphoreGive(keypressedMutex);
            }        

            if ((__atomic_load_n(&board_location, __ATOMIC_RELAXED) != 2) & (pos_raw_pre != 3))
            {
                sendNotes();
            }
        }
        if (num_keypressed > 0)
        {
            for (uint8_t i = 0; i < num_keypressed; i++)
            {
                uint8_t octave_local = __atomic_load_n(&octave, __ATOMIC_RELAXED);
                if (octave_local < 4)
                {
                    currentStepSize[i]= stepSizes[keypressed_local[i]] >> (4 - octave_local);
                }
                else
                {
                    currentStepSize[i]= stepSizes[keypressed_local[i]] << (octave_local - 4);
                }
            }
        }

        // Using knob class decode knob
        Decoder3->update(keyArray[3] & 0b00000011);
        Decoder2->update((keyArray[3] & 0b00001100) >> 2);
        Decoder1->update(keyArray[4] & 0b00000011);
        Decoder0->update((keyArray[4] & 0b00001100) >> 2);
        __atomic_store_n(&volume, Decoder3->get_val(), __ATOMIC_RELAXED);
        __atomic_store_n(&octave, Decoder2->get_val(), __ATOMIC_RELAXED);
        __atomic_store_n(&waveform_mode, Decoder1->get_val(), __ATOMIC_RELAXED);
        var0 = Decoder0->get_val();

        // Update the volume and ocatve to subs
        if (((Decoder1->get_val() != waveform_mode_pre) | (Decoder2->get_val() != octave_pre) | (Decoder3->get_val() != volume_pre)) & (pos_raw_pre != 3) & (__atomic_load_n(&board_location, __ATOMIC_RELAXED) == 1))
        {
            configVO();
            octave_pre = Decoder2->get_val();
            volume_pre = Decoder3->get_val();
            waveform_mode_pre = Decoder1->get_val();
        }

        // Handling the knob press
        if ((keyArray[5] & 0b00000001) == 0)
        {
            // knob2 pressed
            if (__atomic_load_n(&board_location, __ATOMIC_RELAXED) == 1)
            {
                play_Twinkle_star();
            }
        }
        else if ((keyArray[5] & 0b00000010) == 0)
        {
            // knob3 pressed
        }
        else if ((keyArray[6] & 0b00000010) == 0)
        {
            // knob1 pressed
        }
        else if ((keyArray[6] & 0b00000001) == 0)
        {
            if (filter_mode == 0)
            {
                Serial.println("lpf_enabled");
                __atomic_store_n(&filter_mode, 1, __ATOMIC_RELAXED);
            }
            else if (filter_mode == 1)
            {
                Serial.println("hpf_enabled");
                __atomic_store_n(&filter_mode, 2, __ATOMIC_RELAXED);
            }
            else if (filter_mode == 2)
            {
                Serial.println("no_filter");
                __atomic_store_n(&filter_mode, 0, __ATOMIC_RELAXED);
            }
            // knob0 pressed
        }
        else if ((keyArray[5] & 0b00000100) == 0)
        {
            // joystick pressed
            if (__atomic_load_n(&reverb_switch, __ATOMIC_RELAXED) == 0)
            {
                __atomic_store_n(&reverb_switch, 1, __ATOMIC_RELAXED);
            }
            else
            {
                __atomic_store_n(&reverb_switch, 0, __ATOMIC_RELAXED);
            }
            configVO();
        }
        uint8_t pos_raw = (bitRead(keyArray[5], 3) << 1) | (bitRead(keyArray[6], 3));
        // Updating position of the board
        if (pos_raw != pos_raw_pre)
        {
            if (pos_raw == 3)
            {
                __atomic_store_n(&board_location, 1, __ATOMIC_RELAXED);
            }
            else if (pos_raw == 2)
            {
                __atomic_store_n(&board_location, 1, __ATOMIC_RELAXED);
                // The left most board
                handshake();
            }
            else if (pos_raw == 1)
            {
                // board on the right can be 2 or 3
                __atomic_store_n(&board_location, 2, __ATOMIC_RELAXED);
            }
            else if (pos_raw == 0)
            {
                // The middle board should inform other board its existance
                __atomic_store_n(&board_location, 2, __ATOMIC_RELAXED);
                delayMicroseconds(20000);
                handshake();
            }
            pos_raw_pre = pos_raw;
        }
    }
}

void displayUpdateTask(void *pvParameters)
{
    const TickType_t xFrequency = 100 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        const uint8_t octave_local = __atomic_load_n(&octave, __ATOMIC_RELAXED);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        if (__atomic_load_n(&board_location, __ATOMIC_RELAXED) == 1)
        {
            u8g2.clearBuffer();              // clear the internal memory
            u8g2.setFont(u8g2_font_6x12_mr); // choose a suitable font

            // First line in display
            u8g2.setCursor(28, 10);
            u8g2.drawStr(2, 10, "VOL:");
            u8g2.print(__atomic_load_n(&volume, __ATOMIC_RELAXED), DEC);

            u8g2.drawStr(65, 10, "OCT:");
            u8g2.setCursor(95, 10);
            if (octave_local != 11)
            {
                u8g2.print(octave_local, DEC);
            }
            else
            {
                u8g2.print(10, DEC);
            }

            // Second line in display;
            u8g2.setCursor(2, 30);
            u8g2.drawStr(2, 20, reverb_display[__atomic_load_n(&reverb_switch, __ATOMIC_RELAXED)].c_str());

            // Third line in display
            u8g2.drawStr(2, 30, Filter_display[filter_mode].c_str());
            u8g2.setCursor(30, 30);
            u8g2.print(var0, DEC);
            u8g2.drawStr(65, 30, wave_display[__atomic_load_n(&waveform_mode, __ATOMIC_RELAXED)].c_str());

            u8g2.sendBuffer(); // transfer internal memory to the display
        }
        else if (__atomic_load_n(&board_location, __ATOMIC_RELAXED) == 2)
        {
            u8g2.clearBuffer();              // clear the internal memory
            u8g2.setFont(u8g2_font_6x12_mr); // choose a suitable font

            u8g2.setCursor(2, 10);
            u8g2.print("OCT");
            u8g2.setCursor(20, 10);
            u8g2.print(octave_local - 1, DEC);

            u8g2.setCursor(2, 20);
            u8g2.print("OCT");
            u8g2.setCursor(20, 20);
            u8g2.print(octave_local, DEC);

            u8g2.setCursor(2, 30);
            u8g2.print("OCT");
            u8g2.setCursor(20, 30);
            u8g2.print(octave_local + 1, DEC);

            uint8_t indent1 = 0;
            uint8_t indent2 = 0;
            uint8_t indent3 = 0;
            const uint16_t keysBytes_Pre_local = __atomic_load_n(&keysBytes_Pre, __ATOMIC_RELAXED);
            const uint16_t keysBytes_B1_local = __atomic_load_n(&keysBytes_B1, __ATOMIC_RELAXED);
            const uint16_t keysBytes_B3_local = __atomic_load_n(&keysBytes_B3, __ATOMIC_RELAXED);
            for (uint8_t i = 0; i < 12; i++)
            {
                if (bitRead(keysBytes_B1_local, i) == 0)
                {
                    u8g2.drawStr(42 + indent1, 10, (note[i]).c_str()); // notes of left-most board
                    indent1 += 12;
                }
                if (bitRead(keysBytes_Pre_local, i) == 0)
                {
                    u8g2.drawStr(42 + indent2, 20, (note[i]).c_str()); // notes of second board from left
                    indent2 += 12;
                }
                if (bitRead(keysBytes_B3_local, i) == 0)
                {
                    u8g2.drawStr(42 + indent3, 30, (note[i]).c_str()); // notes of third board from left
                    indent3 += 12;
                }
            }
            u8g2.sendBuffer(); // transfer internal memory to the display
        }
        else
        {
            u8g2.clearBuffer(); // clear the internal memory
            u8g2.sendBuffer();
        }
    }
}

void CAN_RX_ISR(void)
{
    uint8_t RX_Message_ISR[8];
    uint32_t ID;
    CAN_RX(ID, RX_Message_ISR);
    xQueueSendFromISR(msgInQ, RX_Message_ISR, NULL);
}

void canDecodeTask(void *pvParameters)
{
    const TickType_t xFrequency = 25 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    char inMsg[8];
    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        xQueueReceive(msgInQ, inMsg, portMAX_DELAY);
        if (inMsg[0] == 'P')
        {
            if (inMsg[1] == 1)
            {
                __atomic_store_n(&keysBytes_B1,(inMsg[2] << 6) | inMsg[3],__ATOMIC_RELAXED);
            }
            else if (inMsg[1] == 3)
            {
                __atomic_store_n(&keysBytes_B3,(inMsg[2] << 6) | inMsg[3],__ATOMIC_RELAXED);
            }
        }
        else if (inMsg[0] == 'H')
        {
            uint8_t msg_loc = inMsg[1];
            if (msg_loc == __atomic_load_n(&board_location, __ATOMIC_RELAXED))
            {
                __atomic_store_n(&board_location,3,__ATOMIC_RELAXED);
            }
            if (__atomic_load_n(&board_location, __ATOMIC_RELAXED) > msg_loc)
            {
                __atomic_store_n(&octave,inMsg[2]+1,__ATOMIC_RELAXED);
                __atomic_store_n(&octave,inMsg[3],__ATOMIC_RELAXED);
                Decoder2->change_val(inMsg[2] + 1);
                Decoder3->change_val(inMsg[3]);
            }
        }
        else if (inMsg[0] == 'C')
        {
            // C can only be sent from pos1
            __atomic_store_n(&octave,inMsg[2] + (__atomic_load_n(&board_location, __ATOMIC_RELAXED) - 1),__ATOMIC_RELAXED);
            __atomic_store_n(&volume,inMsg[3],__ATOMIC_RELAXED);
            __atomic_store_n(&waveform_mode,inMsg[4],__ATOMIC_RELAXED);
            __atomic_store_n(&reverb_switch,inMsg[5],__ATOMIC_RELAXED);
            Decoder2->change_val(inMsg[2] + (__atomic_load_n(&board_location, __ATOMIC_RELAXED) - 1));
            Decoder3->change_val(inMsg[3]);
            Decoder1->change_val(inMsg[4]);
        }
    }
}

void canTxTask(void *pvParameters)
{
    const TickType_t xFrequency = 30 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8_t msgOut[8];
    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        xQueueReceive(msgOutQ, msgOut, portMAX_DELAY);
        xSemaphoreTake(CAN_TX_Semaphore, portMAX_DELAY);
        CAN_TX(0x123, msgOut);
    }
}

void CAN_TX_ISR(void)
{
    xSemaphoreGiveFromISR(CAN_TX_Semaphore, NULL);
}

void setup()
{
    // put your setup code here, to run once:
    // Set pin directions
    pinMode(RA0_PIN, OUTPUT);
    pinMode(RA1_PIN, OUTPUT);
    pinMode(RA2_PIN, OUTPUT);
    pinMode(REN_PIN, OUTPUT);
    pinMode(OUT_PIN, OUTPUT);
    pinMode(OUTL_PIN, OUTPUT);
    pinMode(OUTR_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(C0_PIN, INPUT);
    pinMode(C1_PIN, INPUT);
    pinMode(C2_PIN, INPUT);
    pinMode(C3_PIN, INPUT);
    pinMode(JOYX_PIN, INPUT);
    pinMode(JOYY_PIN, INPUT);

    // Initialise display
    setOutMuxBit(DRST_BIT, LOW); // Assert display logic reset
    delayMicroseconds(2);
    setOutMuxBit(DRST_BIT, HIGH); // Release display logic reset
    u8g2.begin();
    setOutMuxBit(DEN_BIT, HIGH); // Enable display power supply

    // Initialise UART
    Serial.begin(9600);

    // Timer init
    TIM_TypeDef *Instance = TIM1;
    HardwareTimer *sampleTimer = new HardwareTimer(Instance);
    sampleTimer->setOverflow(22000, HERTZ_FORMAT);
    sampleTimer->attachInterrupt(sampleISR);
    sampleTimer->resume();

    // RTOS initialize thread
    TaskHandle_t scanKeysHandle = NULL;
    xTaskCreate(
        scanKeysTask,   /* Function that implements the task */
        "scanKeys",     /* Text name for the task */
        64,             /* Stack size in words, not bytes */
        NULL,           /* Parameter passed into the task */
        1,              /* Task priority */
        &scanKeysHandle /* Pointer to store the task handle */
    );
    TaskHandle_t displayUpdateHandle = NULL;
    xTaskCreate(
        displayUpdateTask,
        "displayUpdate",
        256,
        NULL,
        2,
        &displayUpdateHandle);

    TaskHandle_t canDecodeTaskHanle = NULL;
    xTaskCreate(
        canDecodeTask,
        "canDecode",
        64,
        NULL,
        3,
        &canDecodeTaskHanle);

    TaskHandle_t canTxTaskHanle = NULL;
    xTaskCreate(
        canTxTask,
        "canTx",
        64,
        NULL,
        4,
        &canTxTaskHanle);

    CAN_Init(false);
    setCANFilter(0x123, 0x7ff);
    CAN_RegisterRX_ISR(CAN_RX_ISR);
    CAN_RegisterTX_ISR(CAN_TX_ISR);
    CAN_Start();

    // Queue length = 36, queue size = 8 bytes
    msgInQ = xQueueCreate(36, 8);
    msgOutQ = xQueueCreate(36, 8);

    //  slots = 3 max count = 3
    CAN_TX_Semaphore = xSemaphoreCreateCounting(3, 3);
    keypressedMutex = xSemaphoreCreateMutex();
    // Start of RTOS scheduler
    // Better to put it at the end of setup
    vTaskStartScheduler();
}

void loop() {}
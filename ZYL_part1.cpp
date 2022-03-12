#include <Arduino.h>
#include <U8g2lib.h>
#include <STM32FreeRTOS.h>
#include <ES_CAN.h>

// Constants
const uint32_t interval = 100; // Display update interval
volatile int32_t currentStepSize;
volatile String currentNote;
volatile uint8_t noteIndex;
uint8_t volume = 12; // range from 0 to 16
uint8_t octive = 4;  // range from 0 to 16
uint8_t var0 = 0;
uint8_t var1 = 0;

const int32_t stepSizes[] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 0};
const String note[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", ""};
uint8_t TX_Message[8] = {0};
QueueHandle_t msgInQ;

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

static int32_t phaseAcc = 0;
volatile uint8_t keyArray[7];
SemaphoreHandle_t keyArrayMutex;

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
    phaseAcc += currentStepSize * 195225;
    int32_t Vout = phaseAcc >> 24;
    Vout = Vout >> (8 - int((volume + 4) * 0.8) / 2);
    analogWrite(OUTR_PIN, Vout + 128);
}

int knobDecode(volatile uint8_t previous, volatile uint8_t current)
{
    // only two bits are used in each param
    if (previous > 1)
    {
        previous = ~(previous | 0xFC);
        current = ~(current | 0xFC);
    }
    switch (previous)
    {
    case 0:
        switch (current)
        {
        case 1:
            return 1;
        case 2:
            return -1;
        case 3:
            return 0;
        default:
            return 0;
        }
    case 1:
        switch (current)
        {
        case 3:
            return 1;
        case 0:
            return -1;
        case 2:
            return 0;
        default:
            return 0;
        }
    }
    return 0;
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
    uint8_t twinkleStar[] = {1, 1, 1, 8, 8, 10, 10, 8, 6, 6, 5, 5, 3, 3, 1, 8, 8, 6, 6, 5, 5, 3, 8, 8, 6, 6, 5, 5, 3, 1, 1, 8, 8, 10, 10, 8, 6, 6, 5, 5, 3, 3, 1};

    for (int i = 0; i < 43; i++)
    {
        if ((i + 1) % 7 == 0)
        {
            __atomic_store_n(&currentStepSize, stepSizes[twinkleStar[i]], __ATOMIC_RELAXED);
            delayMicroseconds(200000);
            __atomic_store_n(&currentStepSize, 0, __ATOMIC_RELAXED);
            delayMicroseconds(150000);
        }
        else
        {
            __atomic_store_n(&currentStepSize, stepSizes[twinkleStar[i]], __ATOMIC_RELAXED);
            delayMicroseconds(200000);
            __atomic_store_n(&currentStepSize, 0, __ATOMIC_RELAXED);
            delayMicroseconds(100000);
        }
    }
}

void play_QinTian()
{
    uint8_t qts[] = {0, 5, 5, 1, 1, 2, 3, 0, 5, 5, 1, 1, 2, 3, 2, 1, 5, 0, 5, 5, 1, 1, 2, 3, 0, 3, 2, 3, 4, 3, 2, 4, 3, 2, 1};
    uint8_t oct[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    uint8_t del[] = {2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, 2, 4, 2, 2, 2, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    int current = 0;
    int len = sizeof(qts) / sizeof(*qts);
    for (int i = 0; i < len - 1; i++)
    {
        if (oct[i] == 2)
        {
            current = stepSizes[noteDemux(qts[i])] / 2;
        }
        else
        {
            current = stepSizes[noteDemux(qts[i])];
        }
        __atomic_store_n(&currentStepSize, current, __ATOMIC_RELAXED);
        delayMicroseconds(del[i] * 100000);
        __atomic_store_n(&currentStepSize, 0, __ATOMIC_RELAXED);
        delayMicroseconds(del[i] * 20000);
    }
    Serial.println("finished");
}

void scanKeysTask(void *pvParameters)
{
    // FIXME: Here the type of variables need to be optimized
    // FIXME: And the name confliction between octive and octave
    const TickType_t xFrequency = 50 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    volatile uint8_t knobState = 0;
    volatile uint8_t knob3_stat = 0;
    volatile uint8_t knob2_stat = 0;
    volatile uint8_t knob1_stat = 0;
    volatile uint8_t knob0_stat = 0;
    uint16_t keyNum = 12;
    uint16_t octave = 0;
    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        for (uint8_t i = 0; i < 7; i++)
        {
            setRow(i);
            delayMicroseconds(3);
            // xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
            keyArray[i] = readCols();
            // xSemaphoreGive(keyArrayMutex);
        }
        // xSemaphoreTake(keyArrayMutex, portMAX_DELAY);

        // Handling the notes
        octave = (keyArray[2] << 8) | (keyArray[1] << 4) | keyArray[0];
        keyNum = 12;
        for (uint8_t i = 0; i < 12; i++)
        {
            if (bitRead(octave, i) == 0)
            {
                keyNum = i;
            }
        }
        if (keyNum != 12)
        {
            TX_Message[0] = 'P';
            TX_Message[1] = octive;
            TX_Message[2] = keyNum;
            // Serial.print("start sending message");
            CAN_TX(0x123, TX_Message);
            // Serial.print("finished sending message");
        }
        else
        {
            TX_Message[0] = 'R';
            CAN_TX(0x123, TX_Message);
        }
        // __atomic_store_n(&noteIndex, keyNum, __ATOMIC_RELAXED);
        // __atomic_store_n(&currentStepSize, stepSizes[keyNum], __ATOMIC_RELAXED);

        // Handling the knob rotation
        knob3_stat = knobDecode((knobState & 0b11000000) >> 6, keyArray[3] & 0b00000011);
        knobState = ((keyArray[3] & 0b00000011) << 6) | (knobState & 0b00111111);
        volume += knob3_stat;
        if (volume > 16)
        {
            volume = 16;
        }
        knob2_stat = knobDecode((knobState & 0b00110000) >> 4, (keyArray[3] & 0b00001100) >> 2);
        knobState = ((keyArray[3] & 0b00001100) << 2) | (knobState & 0b11001111);
        octive += knob2_stat;

        knob1_stat = knobDecode((knobState & 0b00001100) >> 2, keyArray[4] & 0b00000011);
        knobState = ((keyArray[4] & 0b00000011) << 2 | (knobState & 0b11110011));
        var1 += knob1_stat;

        knob0_stat = knobDecode((knobState & 0b00000011), (keyArray[4] & 0b00001100) >> 2);
        knobState = ((keyArray[4] & 0b00001100)) >> 2 | (knobState & 0b11111100);
        var0 += knob0_stat;

        // Handling the knob press
        if ((keyArray[5] & 0b00000001) == 0)
        {
            // knob3 pressed
            play_QinTian();
        }
        else if ((keyArray[5] & 0b00000010) == 0)
        {
            // knob2 pressed
            play_Twinkle_star();
        }
        else if ((keyArray[6] & 0b00000010) == 0)
        {
            // knob1 pressed
        }
        else if ((keyArray[6] & 0b00000010) == 0)
        {
            // knob0 pressed
        }
    }
}

void displayUpdateTask(void *pvParameters)
{
    const TickType_t xFrequency = 100 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        u8g2.clearBuffer();                 // clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font

        // First line in display
        u8g2.setCursor(28, 10);
        u8g2.drawStr(2, 10, "VOL:");
        u8g2.print(volume, DEC);
        u8g2.setCursor(73, 10);
        u8g2.drawStr(45, 10, "OCT:");
        u8g2.print(octive, DEC);
        // u8g2.drawStr(85,10,"Vs:");
        u8g2.setCursor(95, 10);
        u8g2.print(var0, DEC);
        u8g2.setCursor(110, 10);
        u8g2.print(var1, DEC);

        // Second line in display;
        u8g2.setCursor(2, 20);
        for (uint8_t i = 0; i < 7; i++)
        {
            // xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
            u8g2.print(keyArray[i], HEX);
            // xSemaphoreGive(keyArrayMutex);
        }
        u8g2.setCursor(70, 20);
        u8g2.print(currentStepSize, DEC);

        // Third line in display;
        u8g2.drawStr(2, 30, (note[noteIndex]).c_str()); // write something to the internal memory
        u8g2.setCursor(70, 30);
        u8g2.drawStr(35, 30, "CAN:");
        u8g2.print((char)RX_Message[0]);
        u8g2.print(RX_Message[1]);
        u8g2.print(RX_Message[2]);

        u8g2.sendBuffer(); // transfer internal memory to the display
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
    char inMsg[8];
    while (1)
    {
        xQueueReceive(msgInQ, inMsg, portMAX_DELAY);
        //   Serial.println(inMsg);
        if (inMsg[0] == 'P')
        {
            uint8_t oct = inMsg[1];
            uint8_t note = inMsg[2];
            __atomic_store_n(&currentStepSize, stepSizes[note] * (2 ^ (oct - 4)), __ATOMIC_RELAXED);
            __atomic_store_n(&noteIndex, note, __ATOMIC_RELAXED);
        }
        else if (inMsg[0] == 'R')
        {
            __atomic_store_n(&currentStepSize, 0, __ATOMIC_RELAXED);
            __atomic_store_n(&noteIndex, 12, __ATOMIC_RELAXED);
        }
    }
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
        256,
        NULL,
        1,
        &canDecodeTaskHanle);

    keyArrayMutex = xSemaphoreCreateMutex();

    CAN_Init(true);
    setCANFilter(0x123, 0x7ff);
    CAN_RegisterRX_ISR(CAN_RX_ISR);
    CAN_Start();

    // Queue length = 36, queue size = 8 bytes
    msgInQ = xQueueCreate(36, 8);

    // Start of RTOS scheduler
    vTaskStartScheduler();
}

void loop() {}

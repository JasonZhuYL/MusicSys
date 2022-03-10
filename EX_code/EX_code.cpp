#include <U8g2lib.h>
#include <Wire.h>
#include <STM32FreeRTOS.h>
#include <knob.h>
#include <LUT.h>
#include <build_in_func.h>
#define PI 3.14159265
// Display driver object
U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);
// Globals
SemaphoreHandle_t keyArrayMutex;
SemaphoreHandle_t SerialkeyPressedMutex;
QueueHandle_t msgOutQ;
volatile uint8_t keyArray[7]; // It cannot be accessed atomically ; it can be protected by a mutex
volatile static int knob3Rotation = 12;
volatile static int knob2Rotation = 4;
volatile static int knob1Rotation = 0;
volatile static int knob0Rotation = 1;
volatile static int JoyRotation = 1;
volatile static bool knob3_pressed = false;
volatile static bool knob2_pressed = false;
volatile static bool knob1_pressed = false;
volatile static bool knob0_pressed = false;
knob_decoder *Joy = new knob_decoder(JoyRotation, 2, 0);         // waveform control
knob_decoder *Decoder3 = new knob_decoder(knob3Rotation, 12, 0); // volume control
knob_decoder *Decoder2 = new knob_decoder(knob2Rotation, 8, 0);  // octave control
knob_decoder *Decoder1 = new knob_decoder(knob1Rotation, 6, 0);  // reverb control
knob_decoder *Decoder0 = new knob_decoder(knob0Rotation, 7, 0);  // filter strengh control
volatile char noteMessage[] = "xxx";
volatile uint32_t globalstepcorr[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile uint32_t globalkeypressed[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
volatile int phaseAccindex[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t joystickread = 0;

void sampleISR()
{
    if (JoyRotation == 1)
    {
        generate_sawtooth();
    }
    else if (JoyRotation == 2)
    {
        generate_triangle();
    }
    else
    {
        generate_square();
    }
}
// Function to set outputs via matrix
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
    Serial.begin(115200);
    Serial.println("Hello World");
    // Setup timer
    TIM_TypeDef *Instance = TIM1;
    HardwareTimer *sampleTimer = new HardwareTimer(Instance);
    // Configure timer
    sampleTimer->setOverflow(22000, HERTZ_FORMAT);
    sampleTimer->attachInterrupt(sampleISR);
    sampleTimer->resume();
    // Set up queue
    msgOutQ = xQueueCreate(8, 4);

    TaskHandle_t scanKeysHandle = NULL;
    xTaskCreate(
        scanKeysTask,     /* Function that implements the task */
        "scanKeys",       /* Text name for the task */
        128,              /* Stack size in words, not bytes */
        NULL,             /* Parameter passed into the task */
        5,                /* Task priority */
        &scanKeysHandle); /* Pointer to store the task handle */
    TaskHandle_t msgInTaskHandle = NULL;
    xTaskCreate(
        msgInTask,         /* Function that implements the task */
        "msgInTask",       /* Text name for the task */
        128,               /* Stack size in words, not bytes */
        NULL,              /* Parameter passed into the task */
        4,                 /* Task priority */
        &msgInTaskHandle); /* Pointer to store the task handle */
    TaskHandle_t displayUpdateHandle = NULL;
    xTaskCreate(
        displayUpdateTask,     /* Function that implements the task */
        "displayUpdate",       /* Text name for the task */
        256,                   /* Stack size in words, not bytes */
        NULL,                  /* Parameter passed into the task */
        3,                     /* Task priority */
        &displayUpdateHandle); /* Pointer to store the task handle */

    TaskHandle_t msgOutTaskHandle = NULL;
    xTaskCreate(
        msgOutTask,         /* Function that implements the task */
        "msgOut",           /* Text name for the task */
        32,                 /* Stack size in words, not bytes */
        NULL,               /* Parameter passed into the task */
        2,                  /* Task priority */
        &msgOutTaskHandle); /* Pointer to store the task handle */

    TaskHandle_t playmusicHandle = NULL;
    xTaskCreate(
        playmusic,         /* Function that implements the task */
        "music",           /* Text name for the task */
        32,                /* Stack size in words, not bytes */
        NULL,              /* Parameter passed into the task */
        1,                 /* Task priority */
        &playmusicHandle); /* Pointer to store the task handle */

    keyArrayMutex = xSemaphoreCreateMutex();
    SerialkeyPressedMutex = xSemaphoreCreateMutex();
    vTaskStartScheduler();
}

void scanKeysTask(void *pvParameters)
{
    const TickType_t xFrequency = 20 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        static int previous_state_knob3_ab = 0b00;
        static int previous_state_knob2_ab = 0b00;
        static int previous_state_knob1_ab = 0b00;
        static int previous_state_knob0_ab = 0b00;
        int current_state_knob3_ab;
        int current_state_knob2_ab;
        int current_state_knob1_ab;
        int current_state_knob0_ab;

        for (uint8_t i = 0; i < 7; ++i)
        {
            setRow(i);
            delayMicroseconds(2);
            uint8_t keys = readCols();
            xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
            keyArray[i] = keys;
            xSemaphoreGive(keyArrayMutex);
            if (i == 3)
            {
                current_state_knob3_ab = keys >> 2;
                current_state_knob2_ab = keys & 0b11;
            } // detect knob rotation
            if (i == 4)
            {
                current_state_knob1_ab = keys >> 2;
                current_state_knob0_ab = keys & 0b11;
            } // detect knob rotation
            if (i == 5 && ((keys >> 2) % 2) == 0)
            {
                __atomic_store_n(&knob3_pressed, true, __ATOMIC_RELAXED);
            } // detect knob3 press
            if (i == 5 && (keys >> 3) == 0)
            {
                __atomic_store_n(&knob2_pressed, true, __ATOMIC_RELAXED);
            } // detect knob2 press
            if (i == 5 && ((keys >> 1) % 2) == 0)
            {
                __atomic_store_n(&joystickread, joytable[joystickread], __ATOMIC_RELAXED);
            } // detect joystick press
            if (i == 6 && ((keys >> 2) % 2) == 0)
            {
                __atomic_store_n(&knob1_pressed, true, __ATOMIC_RELAXED);
            } // detect knob1 press
            if (i == 6 && (keys >> 3) == 0)
            {
                __atomic_store_n(&knob0_pressed, true, __ATOMIC_RELAXED);
            } // detect knob0 press
        }
        if (digitalRead(A0) == 0)
        {
            Joy->increase_joy();
        }
        else
        {
        }
        if (digitalRead(A1) == 0)
        {
            Joy->decrease_joy();
        }
        else
        {
        }

        __atomic_store_n(&JoyRotation, Joy->get_rotation(), __ATOMIC_RELAXED); // write back to global

        // local copy of global
        int localknob3Rotation = knob3Rotation;
        int localknob2Rotation = knob2Rotation;
        int localknob1Rotation = knob1Rotation;
        int localknob0Rotation = knob0Rotation;

        // knob decoder starts
        Decoder3->update(previous_state_knob3_ab, current_state_knob3_ab);
        localknob3Rotation = Decoder3->get_rotation();
        Decoder2->update(previous_state_knob2_ab, current_state_knob2_ab);
        localknob2Rotation = Decoder2->get_rotation();
        Decoder1->update(previous_state_knob1_ab, current_state_knob1_ab);
        localknob1Rotation = Decoder1->get_rotation();
        Decoder0->update(previous_state_knob0_ab, current_state_knob0_ab);
        localknob0Rotation = Decoder0->get_rotation();

        // atomic store to global
        __atomic_store_n(&knob3Rotation, localknob3Rotation, __ATOMIC_RELAXED);
        previous_state_knob3_ab = current_state_knob3_ab;
        __atomic_store_n(&knob1Rotation, localknob1Rotation, __ATOMIC_RELAXED);
        previous_state_knob1_ab = current_state_knob1_ab;
        __atomic_store_n(&knob0Rotation, localknob0Rotation, __ATOMIC_RELAXED);
        previous_state_knob0_ab = current_state_knob0_ab;

        // read states of each key
        uint32_t keypressed[12];
        for (int i = 0; i < 3; i++)
        {
            xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
            uint8_t key = keyArray[i];
            xSemaphoreGive(keyArrayMutex);
            keypressupdate(keypressed, key, i); // this is written in atomic manner in build_in_func.h
            delayMicroseconds(2);
        }

        // copy Serial keypressed
        uint32_t localSerialkeypressed[12];
        for (int i = 0; i < 12; i++)
        {
            xSemaphoreTake(SerialkeyPressedMutex, portMAX_DELAY);
            localSerialkeypressed[i] = Serialkeypressed[i];
            xSemaphoreGive(SerialkeyPressedMutex);
        }

        // add Serial keypressed result to keypressed
        for (int i = 0; i < 12; i++)
        {
            xSemaphoreTake(SerialkeyPressedMutex, portMAX_DELAY);
            if (localSerialkeypressed[i] == 0)
            {
                __atomic_store_n(&keypressed[i], 0, __ATOMIC_RELAXED);
            }
            xSemaphoreGive(SerialkeyPressedMutex);
        }

        // deal with globalstepcorr for chords
        for (int i = 0; i < 12; i++)
        {
            bool tmp = __atomic_load_n(&keypressed[i], __ATOMIC_RELAXED); // atomic load
            if (tmp == 1)
            {
                __atomic_store_n(&globalstepcorr[i], 0, __ATOMIC_RELAXED);
            }
            else
            {
                __atomic_store_n(&globalstepcorr[i], stepSizes[localknob2Rotation][i], __ATOMIC_RELAXED);
                265
            }
        }

        // update phaseAccindex
        for (int i = 0; i < 12; i++)
        {
            int tmp = __atomic_load_n(&phaseAccindex[i], __ATOMIC_RELAXED); // atomic load
            if (globalstepcorr[i] != 0)
            {
                __atomic_store_n(&phaseAccindex[i], 100, __ATOMIC_RELAXED);
            }
            else if (tmp >= reverbtable[localknob1Rotation])
            { // changed to "local" knob2Rotation, not global
                __atomic_sub_fetch(&phaseAccindex[i], reverbtable[localknob1Rotation], __ATOMIC_RELAXED);
                276
            }
            else
            {
                __atomic_store_n(&phaseAccindex[i], 0, __ATOMIC_RELAXED);
            }
        }

        // copy globalkeypressed to prevkeypressed
        uint32_t prevkeypressed[12];
        for (int i = 0; i < 12; i++)
        {
            prevkeypressed[i] = __atomic_load_n(&globalkeypressed[i], __ATOMIC_RELAXED); // changed to atomic 286 }

            // Detect release and press and send messages
            for (int i = 0; i < 12; i++)
            {
                bool tmp = __atomic_load_n(&keypressed[i], __ATOMIC_RELAXED); // atomic load
                if (prevkeypressed[i] == 0 && tmp == 1)
                {
                    __atomic_store_n(&noteMessage[0], 'R', __ATOMIC_RELAXED);
                    __atomic_store_n(&noteMessage[1], int_to_char(knob2Rotation), __ATOMIC_RELAXED);
                    __atomic_store_n(&noteMessage[2], intToHex[i], __ATOMIC_RELAXED);
                    xQueueSend(msgOutQ, (char *)noteMessage, portMAX_DELAY);
                }
                else if ((prevkeypressed[i] == 1) && (tmp == 0))
                {
                    __atomic_store_n(&noteMessage[0], 'P', __ATOMIC_RELAXED);
                    __atomic_store_n(&noteMessage[1], int_to_char(knob2Rotation), __ATOMIC_RELAXED);
                    __atomic_store_n(&noteMessage[2], intToHex[i], __ATOMIC_RELAXED);
                    xQueueSend(msgOutQ, (char *)noteMessage, portMAX_DELAY);
                }
            }

            // update globalkeypressed
            for (int i = 0; i < 12; i++)
            {
                bool tmp = __atomic_load_n(&keypressed[i], __ATOMIC_RELAXED);
                __atomic_store_n(&globalkeypressed[i], tmp, __ATOMIC_RELAXED);
            }

            __atomic_store_n(&knob2Rotation, localknob2Rotation, __ATOMIC_RELAXED);
            previous_state_knob2_ab = current_state_knob2_ab; // update knob2 status
        }
    }

    void displayUpdateTask(void *pvParameters)
    {
        // Update display
        const TickType_t xFrequency = 50 / portTICK_PERIOD_MS;
        TickType_t xLastWakeTime = xTaskGetTickCount();
        while (1)
        {
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
            u8g2.clearBuffer(); // clear the internal memory
            // Display Volume
            u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
            u8g2.drawStr(2, 10, "Volume: ");
            u8g2.setCursor(50, 10);
            u8g2.print(__atomic_load_n(&knob3Rotation, __ATOMIC_RELAXED)); // atomic load
            // display current played nodes
            u8g2.setCursor(60, 32);
            uint32_t localkeypressed[12];
            for (int i = 0; i < 12; i++)
            {
                localkeypressed[i] = __atomic_load_n(&globalkeypressed[i], __ATOMIC_RELAXED); // changed to atomic 333 }
                for (int i = 0; i < 12; i++)
                {
                    if (localkeypressed[i] == 0)
                    {
                        u8g2.print(state_to_note(i + 1));
                    }
                }
                u8g2.drawStr(74, 10, "Octave: ");
                u8g2.setCursor(113, 10);
                u8g2.print(__atomic_load_n(&knob2Rotation, __ATOMIC_RELAXED));
                u8g2.drawStr(2, 20, "Reverb: ");
                u8g2.setCursor(45, 20);
                u8g2.print(__atomic_load_n(&knob1Rotation, __ATOMIC_RELAXED));
                uint8_t localjoy = __atomic_load_n(&JoyRotation, __ATOMIC_RELAXED);
                if (localjoy == 1)
                {
                    u8g2.drawStr(74, 20, "Sawtooth");
                }
                else if (localjoy == 2)
                {
                    u8g2.drawStr(74, 20, "Triangle");
                }
                else
                {
                    u8g2.drawStr(74, 20, "Square");
                }
                uint8_t localjoystickread = __atomic_load_n(&joystickread, __ATOMIC_RELAXED);
                if (localjoystickread == 0)
                {
                    u8g2.drawStr(2, 32, "No filter");
                }
                else if (localjoystickread == 1)
                {
                    u8g2.drawStr(2, 32, "LPF:");
                    u8g2.setCursor(30, 32);
                    u8g2.print(__atomic_load_n(&knob0Rotation, __ATOMIC_RELAXED));
                }
                else
                {
                    u8g2.drawStr(2, 32, "HPF:");
                    u8g2.setCursor(30, 32);
                    u8g2.print(__atomic_load_n(&knob0Rotation, __ATOMIC_RELAXED));
                }
                u8g2.sendBuffer(); // transfer internal memory to the display

                // Toggle LED
                digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            }
        }

        void msgOutTask(void *pvParameters)
        {
            char outMsg[4];
            while (1)
            {
                xQueueReceive(msgOutQ, outMsg, portMAX_DELAY);
                Serial.println(outMsg);
            }
        }

        void msgInTask(void *pvParameters)
        {
            const TickType_t xFrequency = 5 / portTICK_PERIOD_MS;
            TickType_t xLastWakeTime = xTaskGetTickCount();
            while (1)
            {
                vTaskDelayUntil(&xLastWakeTime, xFrequency);
                char inMsg[] = "xxx";
                int index = 0;
                char tmp;
                while (Serial.available() > 0)
                {
                    tmp = Serial.read();
                    if (tmp != '\n')
                    {
                        inMsg[index] = tmp;
                        index++;
                    }
                    else
                    {
                        int pos = char_to_int(inMsg[2]);
                        int octaveupdate = char_to_int(inMsg[1]);
                        if (inMsg[0] == 'P')
                        {
                            Decoder2->change_rotation(octaveupdate);
                            xSemaphoreTake(SerialkeyPressedMutex, portMAX_DELAY);
                            Serialkeypressed[pos] = 0;
                            xSemaphoreGive(SerialkeyPressedMutex); // added mutex
                        }
                        else if (inMsg[0] == 'R')
                        {
                            xSemaphoreTake(SerialkeyPressedMutex, portMAX_DELAY);
                            Serialkeypressed[pos] = 1;
                            xSemaphoreGive(SerialkeyPressedMutex);
                        }
                        break;
                    }
                }
            }
        }

        void playmusic(void *pvParameters)
        {
            const TickType_t xFrequency = 1 / portTICK_PERIOD_MS;
            TickType_t xLastWakeTime = xTaskGetTickCount();
            while (1)
            {
                vTaskDelayUntil(&xLastWakeTime, xFrequency);
if(__atomic_load_n(&knob3_pressed, __ATOMIC_RELAXED) == true && __atomic_load_n(&knob2_pressed, 422 play_Mary_has_a_little_lamb();
__atomic_store_n(&knob0_pressed, false, __ATOMIC_RELAXED);
__atomic_store_n(&knob1_pressed, false, __ATOMIC_RELAXED);
__atomic_store_n(&knob2_pressed, false, __ATOMIC_RELAXED);
__atomic_store_n(&knob3_pressed, false, __ATOMIC_RELAXED);
            }
else if (__atomic_load_n(&knob3_pressed, __ATOMIC_RELAXED) == false && __atomic_load_n(&knob2_429 play_Twinkle_star();
__atomic_store_n(&knob0_pressed, false, __ATOMIC_RELAXED);
__atomic_store_n(&knob1_pressed, false, __ATOMIC_RELAXED);
__atomic_store_n(&knob2_pressed, false, __ATOMIC_RELAXED);
__atomic_store_n(&knob3_pressed, false, __ATOMIC_RELAXED);
        }
else if (__atomic_load_n(&knob3_pressed, __ATOMIC_RELAXED) == false && __atomic_load_n(&knob2_436 play_Ode_an_die_Freude();
__atomic_store_n(&knob0_pressed, false, __ATOMIC_RELAXED);
__atomic_store_n(&knob1_pressed, false, __ATOMIC_RELAXED);
__atomic_store_n(&knob2_pressed, false, __ATOMIC_RELAXED);
__atomic_store_n(&knob3_pressed, false, __ATOMIC_RELAXED);
    }
else if (__atomic_load_n(&knob3_pressed, __ATOMIC_RELAXED) == false && __atomic_load_n(&knob2_443 play_London_Bridge_Is_Falling_Down();
__atomic_store_n(&knob0_pressed, false, __ATOMIC_RELAXED);
__atomic_store_n(&knob1_pressed, false, __ATOMIC_RELAXED);
__atomic_store_n(&knob2_pressed, false, __ATOMIC_RELAXED);
__atomic_store_n(&knob3_pressed, false, __ATOMIC_RELAXED);
}
else {}
}
}

void loop() {}

// since the variable is read in an ISR, an atomic operation is unneeded because the ISR cannot be 456 //below code can walk free without atomic operations

void generate_sawtooth()
{
    static uint32_t phaseAcc[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static uint32_t phaseAcc2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t outValue = 0;
    for (int i = 0; i < 12; i++)
    {
        phaseAcc[i] += stepSizes[knob2Rotation][i];
    }
    for (int i = 0; i < 12; i++)
    {
        if (joystickread == 1)
            phaseAcc2[i] = phaseAcc[i] / 10 * lpf[knob0Rotation][i];
        else if (joystickread == 2)
            phaseAcc2[i] = phaseAcc[i] / 10 * hpf[knob0Rotation][i];
        else
        {
            phaseAcc2[i] = phaseAcc[i];
        }
    }
    for (int i = 0; i < 12; i++)
    {
        outValue += ((phaseAcc2[i] / 100) * phaseAccindex[i]) / 10;
    }
    outValue = (outValue >> 24) >> (6 - knob3Rotation / 2);
    analogWrite(A3, outValue);
}

void generate_triangle()
{
    static uint32_t phaseAcc[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static bool up[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    static uint32_t phaseAcc2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t outValue = 0;
    for (int i = 0; i < 12; i++)
    {
        if (up[i] == 1)
        {
            uint32_t phaseAccupdate = phaseAcc[i] + 2 * stepSizes[knob2Rotation][i];
            if (phaseAccupdate < phaseAcc[i])
            {
                up[i] = 0;
                phaseAccupdate = maxuint32 - phaseAccupdate;
            }
            phaseAcc[i] = phaseAccupdate;
        }
        else
        {
            uint32_t phaseAccupdate = phaseAcc[i] - 2 * stepSizes[knob2Rotation][i];
            if (phaseAccupdate > phaseAcc[i])
            {
                up[i] = 1;
                phaseAccupdate = maxuint32 - phaseAccupdate;
            }
            phaseAcc[i] = phaseAccupdate;
        }
    }
    for (int i = 0; i < 12; i++)
    {
        if (joystickread == 1)
            phaseAcc2[i] = phaseAcc[i] / 10 * lpf[knob0Rotation][i];
        else if (joystickread == 2)
            phaseAcc2[i] = phaseAcc[i] / 10 * hpf[knob0Rotation][i];
        else
        {
            phaseAcc2[i] = phaseAcc[i];
        }
    }
    for (int i = 0; i < 12; i++)
    {
        outValue += phaseAcc2[i] / 100 * phaseAccindex[i] / 10;
    }
    outValue = (outValue >> 24) >> (6 - knob3Rotation / 2);
    analogWrite(A3, outValue);
}

void generate_square()
{
    static uint32_t phaseAcc[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static bool up[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    static uint32_t phaseAcc2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static uint32_t phaseAcc3[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t outValue = 0;
    for (int i = 0; i < 12; i++)
    {
        if (up[i] == 1)
        {
            uint32_t phaseAccupdate = phaseAcc[i] + 2 * stepSizes[knob2Rotation][i];
            if (phaseAccupdate < phaseAcc[i])
            {
                up[i] = 0;
                phaseAccupdate = maxuint32 - phaseAccupdate;
            }
            phaseAcc[i] = phaseAccupdate;
        }
        else
        {
            uint32_t phaseAccupdate = phaseAcc[i] - 2 * stepSizes[knob2Rotation][i];
            if (phaseAccupdate > phaseAcc[i])
            {
                up[i] = 1;
                phaseAccupdate = maxuint32 - phaseAccupdate;
            }
            phaseAcc[i] = phaseAccupdate;
        }
        if (up[i] == 1)
        {
            phaseAcc2[i] = maxuint32;
        }
        else
        {
            phaseAcc2[i] = 0;
        }
    }
    for (int i = 0; i < 12; i++)
    {
        if (joystickread == 1)
            phaseAcc3[i] = phaseAcc2[i] / 10 * lpf[knob0Rotation][i];
        else if (joystickread == 2)
            phaseAcc3[i] = phaseAcc2[i] / 10 * hpf[knob0Rotation][i];
        else
        {
            phaseAcc3[i] = phaseAcc2[i];
        }
    }
    for (int i = 0; i < 12; i++)
    {
        outValue += phaseAcc3[i] / 100 * phaseAccindex[i] / 10;
    }
    outValue = (outValue >> 24) >> (6 - knob3Rotation / 2);
    analogWrite(A3, outValue);
}
#include <Arduino.h>
#include <U8g2lib.h>

//Constants
  const uint32_t interval = 100; //Display update interval
  volatile int32_t currentStepSize;
  volatile String currentNote;
  const int32_t stepSizes [] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 0};
  const String note [] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B",""};

//Pin definitions
  //Row select and enable
  const int RA0_PIN = D3;
  const int RA1_PIN = D6;
  const int RA2_PIN = D12;
  const int REN_PIN = A5;

  //Matrix input and output
  const int C0_PIN = A2;
  const int C1_PIN = D9;
  const int C2_PIN = A6;
  const int C3_PIN = D1;
  const int OUT_PIN = D11;

  //Audio analogue out
  const int OUTL_PIN = A4;
  const int OUTR_PIN = A3;

  //Joystick analogue in
  const int JOYY_PIN = A0;
  const int JOYX_PIN = A1;

  //Output multiplexer bits
  const int DEN_BIT = 3;
  const int DRST_BIT = 4;
  const int HKOW_BIT = 5;
  const int HKOE_BIT = 6;

static int32_t phaseAcc = 0;


//Display driver object
U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);

//Function to set outputs using key matrix
void setOutMuxBit(const uint8_t bitIdx, const bool value) {
      digitalWrite(REN_PIN,LOW);
      digitalWrite(RA0_PIN, bitIdx & 0x01);
      digitalWrite(RA1_PIN, bitIdx & 0x02);
      digitalWrite(RA2_PIN, bitIdx & 0x04);
      digitalWrite(OUT_PIN,value);
      digitalWrite(REN_PIN,HIGH);
      delayMicroseconds(2);
      digitalWrite(REN_PIN,LOW);
}

uint8_t readCols(){

  uint8_t C0 = digitalRead(C0_PIN);
  uint8_t C1 = digitalRead(C1_PIN);
  uint8_t C2 = digitalRead(C2_PIN);
  uint8_t C3 = digitalRead(C3_PIN);
  uint8_t byteRead = (C3 << 3) | (C2 << 2) | (C1 << 1) | C0 ;
  return byteRead;
}

void setRow(uint8_t rowIdx){

  digitalWrite(REN_PIN,LOW);
 // Serial.println(rowIdx/2);
  if(rowIdx%2 == 0){
    digitalWrite(RA0_PIN,LOW);
  }
  else {
    digitalWrite(RA0_PIN,HIGH);
  }
  if((rowIdx/2)%2 == 0){
    digitalWrite(RA1_PIN,LOW);
  }
  else {
    digitalWrite(RA1_PIN,HIGH);
  }
  if((rowIdx/4) == 0){
    digitalWrite(RA2_PIN,LOW);
  }
  else {
    digitalWrite(RA2_PIN,HIGH);
  }

  digitalWrite(REN_PIN,HIGH);

}

void sampleISR() {
  phaseAcc += currentStepSize*195225;
  int32_t Vout = phaseAcc >> 24;
  analogWrite(OUTR_PIN, Vout + 128);
}

void readStep(uint8_t keyArray[7]){
  uint32_t keyNum =12;
  //uint32_t oldKey = keyNum;
  //find the corresponding stepsize
  if(bitRead(keyArray[0], 0) == 0){
    keyNum = 0;
  } else
  if(bitRead(keyArray[0], 1) == 0){
    keyNum = 1;
  } else
  if(bitRead(keyArray[0], 2) == 0){
    keyNum = 2;
  } else
  if(bitRead(keyArray[0], 3) == 0){
    keyNum = 3;
  } else
  if(bitRead(keyArray[1], 0) == 0){
    keyNum = 4;
  } else
  if(bitRead(keyArray[1], 1) == 0){
    keyNum = 5;
  } else
  if(bitRead(keyArray[1], 2) == 0){
    keyNum = 6;
  } else
  if(bitRead(keyArray[1], 3) == 0){
    keyNum = 7;
  } else
  if(bitRead(keyArray[2], 0) == 0){
    keyNum = 8;
  } else
  if(bitRead(keyArray[2], 1) == 0){
    keyNum = 9;
  } else
  if(bitRead(keyArray[2], 2) == 0){
    keyNum = 10;
  } else
  if(bitRead(keyArray[2], 3) == 0){
    keyNum = 11;
  } else {keyNum = 12;}
 // int keyDiff = keyNum - oldKey;
 // if (keyDiff > 0)

  currentStepSize = stepSizes[keyNum];

}

void setup() {
  // put your setup code here, to run once:

  //Set pin directions
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

  //Initialise display
  setOutMuxBit(DRST_BIT, LOW);  //Assert display logic reset
  delayMicroseconds(2);
  setOutMuxBit(DRST_BIT, HIGH);  //Release display logic reset
  u8g2.begin();
  setOutMuxBit(DEN_BIT, HIGH);  //Enable display power supply

  TIM_TypeDef *Instance = TIM1;
  HardwareTimer *sampleTimer = new HardwareTimer(Instance);
  sampleTimer->setOverflow(22000, HERTZ_FORMAT);
  sampleTimer->attachInterrupt(sampleISR);
  sampleTimer->resume();

  //Initialise UART
  Serial.begin(9600);
  Serial.println("Hello World");
}

void loop() {
  // put your main code here, to run repeatedly:
  static uint32_t next = millis();
  static uint32_t count = 0;
  uint8_t keyArray[7];


  if (millis() > next) {
    next += interval;

    //Update display
    u8g2.clearBuffer();         // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    u8g2.drawStr(2,10,"Hello World!");  // write something to the internal memory

    //u8g2.print(count++);
    for(uint8_t i = 0; i<3; i++){
      setRow(i);
      delayMicroseconds(3);
      keyArray[i] = readCols();
      
    }
    readStep(keyArray);
    for(uint8_t i = 0; i<3; i++){
      u8g2.print(keyArray[i],HEX);
    }

    u8g2.print(currentStepSize,DEC);
    u8g2.setCursor(2,20);
    u8g2.sendBuffer();          // transfer internal memory to the display

    //Toggle LED
    digitalToggle(LED_BUILTIN);
  }
}

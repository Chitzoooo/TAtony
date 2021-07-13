#include <SoftwareSerial.h>
#include "VoiceRecognitionV3.h"
#include <TMRpcm.h>
#include <SPI.h>
#include <HCSR04.h>
#include <SD.h>

#define SD_ChipSelectPin 10   //SD card

//training suara
#define panas        (0)  //data training suara
#define dingin       (1)
#define perintah1    (2) //50 ml
#define perintah2    (3) //100 ml
#define perintah3    (4) //150 ml
#define perintah4    (5) //200 ml

//ultrasonikpin
/*#define trigPina 11
  #define echoPina 12*/
#define trigPinb 7
#define echoPinb 3

//ultrasonik1
/*int trig_pina = 11;
  int echo_pina = 12;*/
long echotimea;
float distancea;

//ultrasonik2
int trig_pinb = 7;
int echo_pinb = 3;
long echotimeb;
float distanceb;

//inisialisasi voice
VR myVR(13, 12); //pin modul voice 13:TX 12:RX
TMRpcm tmrpcm;  //spiker

uint8_t records[7];   // save record
uint8_t buf[64];

//relay
const int relay1 = 8;   //pin relay1
const int relay2 = 9;   //pin relay2
int relayON = LOW;      //relay nyala
int relayOFF = HIGH;    //relay mati


byte sensorInt = 0;   //sensor waterflow
byte flowsensor = 2;  //pin sensor waterflow
float konstanta = 4.5; //konstanta flow meter
volatile byte pulseCount;
float debit;
unsigned int flowmlt;
unsigned long totalmlt;
unsigned long oldTime;

int led1 = 0;
int led2 = 1;

int l;

void printSignature(uint8_t *buf, int len)    //modul voice
{
  int i;
  for (i = 0; i < len; i++)
  {
    if (buf[i] > 0x19 && buf[i] < 0x7F)
    {
      Serial.write(buf[i]);
    }
    else
    {
      Serial.print("[");
      Serial.print(buf[i], HEX);
      Serial.print("]");
    }
  }
}

void printVR(uint8_t *buf)
{
  Serial.println("VR Index\tGroup\tRecordNum\tSignature");
  Serial.print(buf[2], DEC);
  Serial.print("\t\t");

  if (buf[0] == 0xFF)
  {
    Serial.print("NONE");
  }
  else if (buf[0] & 0x80)
  {
    Serial.print("UG ");
    Serial.print(buf[0] & (~0x80), DEC);
  }
  else
  {
    Serial.print("SG ");
    Serial.print(buf[0], DEC);
  }
  Serial.print("\t");
  Serial.print(buf[1], DEC);
  Serial.print("\t\t");
  if (buf[3] > 0)
  {
    printSignature(buf + 4, buf[3]);
  }
  else
  {
    Serial.print("NONE");
  }
  Serial.println("\r\n");
}

void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}

void setup()
{
  tmrpcm.speakerPin = 6;  //spiker
  /** initialize */
  myVR.begin(9600);     //modul voice
  Serial.begin(115200);
  Serial.println("Elechouse Voice Recognition V3 Module\r\nTugasAkhir");


  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led1, LOW);
  pinMode(led2, LOW);

  /*  pinMode(trig_pina, OUTPUT);
    pinMode(echo_pina, INPUT);
    digitalWrite(trig_pina, LOW);
  */
  pinMode(trig_pinb, OUTPUT);
  pinMode(echo_pinb, INPUT);
  digitalWrite(trig_pinb, LOW);

  if (myVR.clear() == 0)
  {
    Serial.println("Recognizer cleared.");
  }
  else
  {
    Serial.println("Not find VoiceRecognitionModule.");
    Serial.println("Please check connection and restart Arduino.");
    while (1);
  }

  if (myVR.load((uint8_t)panas) >= 0)
  {
    Serial.println("panas");
  }

  if (myVR.load((uint8_t)dingin) >= 0)
  {
    Serial.println("dingin");
  }

  if (myVR.load((uint8_t)perintah1) >= 0)
  {
    Serial.println("perintah1");
  }

  if (myVR.load((uint8_t)perintah2) >= 0)
  {
    Serial.println("perintah2");
  }

  if (myVR.load((uint8_t)perintah3) >= 0)
  {
    Serial.println("perintah3");
  }

  if (myVR.load((uint8_t)perintah4) >= 0)
  {
    Serial.println("perintah4");
  }
  pinMode(flowsensor, INPUT);   //waterflow
  digitalWrite(flowsensor, HIGH);
  pulseCount = 0;
  debit = 0.0;
  flowmlt = 0;
  totalmlt = 0;
  oldTime = 0;
  attachInterrupt(sensorInt, pulseCounter, FALLING);
}

void ultsnc()
{
  /*  digitalWrite(trig_pina, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_pina, LOW);
    echotimea = pulseIn(echo_pina, HIGH);
    distancea = 0.0001 * ((float)echotimea * 340.0) / 2.0;
  */
  digitalWrite(trig_pinb, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pinb, LOW);
  echotimeb = pulseIn(echo_pinb, HIGH);
  distanceb = 0.0001 * ((float)echotimeb * 340.0) / 2.0;

  /* if (distancea < 5)
    { pinMode(relay1, OUTPUT);    //relay panas
     digitalWrite(relay1, relayOFF);
     Serial.println("GELAS PANAS TERDETEKSI");
     Serial.println("\n");
     Serial.println("masukkan jumlah air");
     digitalWrite(led1, HIGH);
     delay(1000);
    }*/
  if (distanceb < 5)
  { pinMode(relay2, OUTPUT);//relay dingin
    digitalWrite(relay2, relayOFF);
    Serial.println("GELAS DINGIN TERDETEKSI");
    Serial.println("\n");
    Serial.println("masukkan jumlah air");
    digitalWrite(led2, HIGH);
    delay(1000);
   l++;
  }
}



void waterflow()
{
  if ((millis() - oldTime) > 1000)
  {
    detachInterrupt(sensorInt);
    debit = ((1000.0 / (millis() - oldTime)) * pulseCount) / konstanta;
    oldTime = millis();
    flowmlt = (debit / 60) * 1000;
    totalmlt += flowmlt;

    unsigned int frac;

    if (debit > 0)
    {
      Serial.print("Debit air: ");
      Serial.print(int(debit));
      Serial.print("L/min");
      Serial.print("\n");

      Serial.print("Air Keluar: ");
      Serial.print(int(flowmlt));
      Serial.print("mL");
      Serial.print("\n");

      Serial.print("Volume: ");
      Serial.print(totalmlt);
      Serial.println("mL");
      Serial.println("\n");
    }
    pulseCount = 0;
    attachInterrupt(sensorInt, pulseCounter, FALLING);
  }
}
void loop()
{ l=0;
  while (l<1){
    ultsnc();
    int ret;
    ret = myVR.recognize(buf, 50);
    if (ret > 0) {
      switch (buf[1]) {
        case perintah1:
          while (flowmlt <= 50)
          {
            digitalWrite(relay1, relayON);
            digitalWrite(relay2, relayON);
            waterflow();
          }
          digitalWrite(relay1, relayOFF);
          digitalWrite(relay2, relayOFF);
          break;
        case perintah2:
          while (flowmlt <= 100)
          {
            Serial.println("Jumlah air: 100ml");
            digitalWrite(relay1, relayON);
            digitalWrite(relay2, relayON);
            waterflow();
          }
          digitalWrite(relay1, relayOFF);
          digitalWrite(relay2, relayOFF);
          break;
        case perintah3:
          while (flowmlt <= 150)
          {
            digitalWrite(relay1, relayON);
            digitalWrite(relay2, relayON);
            waterflow();
          }
          digitalWrite(relay1, relayOFF);
          digitalWrite(relay2, relayOFF);
          break;
        case perintah4:
          while (flowmlt <= 200)
          {
            digitalWrite(relay1, relayON);
            digitalWrite(relay2, relayON);
            waterflow();
          }
          digitalWrite(relay1, relayOFF);
          digitalWrite(relay2, relayOFF);
          break;
        default:
          Serial.println("Record function undefined");
          digitalWrite(led1, HIGH);
          digitalWrite(led2, HIGH);
          break;
      }
      /** voice recognized */
      printVR(buf);
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
      delay (1000);
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      delay (1000);
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
      delay (1000);
    }
   l=0;
  }
}

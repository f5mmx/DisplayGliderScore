/*
   Chrono soft 
   sur base Arduino avec reception temps de travail par radio 
   version Beta 0.1 du decembre 2018 
   O.Segouin et JM.Bombar.
*/


#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <Arduino.h>
#include <U8g2lib.h>
/*
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
*/
// Variables and constants
#define BOUNCE_DURATION 500   // define an appropriate bounce time in ms for your switches
volatile unsigned long bounceTime=0; // variable to hold ms count to debounce a pressed switch
const int RADIO_SS_PIN  = 8;
const int RADIO_CSN_PIN = 9;
RF24 radio(RADIO_SS_PIN, RADIO_CSN_PIN); // CE, CSN
const byte address[6] = "00001";
String manche = "0";
String groupe = "0";
String chronoS = "0000";
char chrono[32] = "";
uint16_t x, y;
boolean flag = false;
const uint64_t pipe = 0xE8E8F0F0E1LL;

String temps;
int bp = 2, fade = 0;
int raz = 2;
int ledPin = 3;
int sensorValue = 0;
int sensorPin = A0;
volatile unsigned long debut;
volatile byte chronomarche = false;
volatile byte chronoraz = false;
volatile unsigned long le_temps = 0, le_temps1 = 0;
unsigned long tempo=0;
boolean flip_flop=false;
bool Dot = true; //Dot state
String chronoS1 = "1";
String chronoS2 = "2";
String chronoS3 = "3";
String chronoS4 = "4";
String statutS ="NO";


U8G2_PCD8544_84X48_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 7, /* data=*/ 6, /* cs=*/ 4, /* dc=*/ 3, /* reset=*/ 5);  // Nokia 5110 Display

String lead_zero(int num) {
  String t = "";
  if (num < 10) t = "0";
  return t + String(num);
}

ISR(TIMER2_COMPA_vect) { //timer2 interrupt 2kHz
  if (chronomarche) {
    le_temps1 = le_temps1 + 1;
    le_temps = le_temps1 / 2;
  }
}

void isr1(void) {
  detachInterrupt(digitalPinToInterrupt(bp));
  if(millis() > bounceTime)  
  {
       // Your code here to handle new button press ?
       if (!chronoraz && !chronomarche) {
        chronomarche = true;
        chronoraz = false;
      } else if (chronomarche && !chronoraz) {
        chronomarche = false;
        chronoraz = true;
      } else if (!chronomarche && chronoraz) {
        chronomarche = false;
        chronoraz = false;
      }
      bounceTime = millis() + BOUNCE_DURATION;  // set whatever bounce time in ms is appropriate
  }  
  
  delay(50);
  attachInterrupt(digitalPinToInterrupt(bp), isr1, FALLING);
}

void setup(void) {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setChannel(1);// 108 2.508 Ghz
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_1MBPS);
  //radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(1);                     // Ensure autoACK is enabled
  radio.setRetries(2, 15);                 // Optionally, increase the delay between retries & # of retries
  radio.setCRCLength(RF24_CRC_8);          // Use 8-bit CRC for performance
  radio.openReadingPipe(0, address);
  radio.startListening();
  cli();//stop interrupts
  TCCR2A = 0;// set entire TCCR0A register to 0
  TCCR2B = 0;// same for TCCR0B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR2A = 249;// = (16*10^6) / (400*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR2B |= (1 << CS01) | (1 << CS00);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE0A);
  sei();//allow interrupts
  u8g2.begin();
  pinMode(bp, INPUT);
  digitalWrite(bp, HIGH); //Pullup
  pinMode(raz, INPUT);
  digitalWrite(raz, HIGH); //Pullup
  attachInterrupt(digitalPinToInterrupt(bp), isr1, FALLING);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
}

String cnv_temps(unsigned long t) {
  unsigned long  ce, se, mi;
  double tt;
  String l_temps;
  tt = t;
  mi = t / 60000;
  t = t - (mi * 60000);
  se = t / 1000;
  t = t - (1000 * se);
  ce = t / 10;
  l_temps = lead_zero(mi) + ":" + lead_zero(se) + ":" + lead_zero(ce);
  return l_temps;
}

void loop(void) {
    
    if ((millis()-tempo)>=500){flip_flop=!flip_flop;tempo=millis();}
    if (radio.available()) {
      char text[32] = "";
      radio.read(&text, sizeof(text));
      if (text[0]=='R')  // c'est bien une trame de gliderscore R01G1 ...trame commence par R
     {
        manche = text[2];
        groupe = text[5];
        chronoS1 = text[7];chronoS2 = text[8];chronoS3 = text[9];chronoS4 = text[10];
        chronoS = chronoS1+chronoS2+":"+chronoS3+chronoS4;
        statutS = &text[11];
        //Serial.println ("Manche "+manche+ " Groupe " + groupe+" Chrono "+chronoS+" Status "+statutS);
       //updateDisplayLCD();
     }
    //delay(100);
    Dot = not Dot ; // Clignotement des 2 points
    //Serial.println(text);
  }

  if (!chronoraz && !chronomarche) {
    // if (!digitalRead(raz)) {
      le_temps1 = 0;
      le_temps = 0;
    // }
  }
  temps = cnv_temps(le_temps);
  
  sensorValue = analogRead(sensorPin);

  Serial.println ("Sensor "+sensorValue);
 
  
  u8g2.clearBuffer();

  u8g2.drawFrame(0, 0, 41, 14); //RND Frame
  u8g2.drawFrame(43, 0, 41, 14); //GRP Frame

  u8g2.drawFrame(0, 16, 83, 14); //Working time Frame
  u8g2.drawFrame(0, 32, 83, 14); //Flight time Frame

  u8g2.setFont(u8g2_font_7x14_tf ); //u8g2_font_8x13B_mn );
  u8g2.setCursor(2, 1 + 1 * 11);
  u8g2.print("RND");
  u8g2.setFont(u8g2_font_7x14B_tf); //u8g2_font_8x13B_mn );
  u8g2.setCursor(31, 1 + 1 * 11);
  u8g2.print(manche);

  u8g2.setFont(u8g2_font_7x14_tf ); //u8g2_font_8x13B_mn );
  u8g2.setCursor(45, 1 + 1 * 11);
  u8g2.print("GRP");
  u8g2.setFont(u8g2_font_7x14B_tf); //u8g2_font_8x13B_mn );
  u8g2.setCursor(74, 1 + 1 * 11);
  u8g2.print(groupe);

  u8g2.setCursor(2, 17 + 11);
  u8g2.setFont(u8g2_font_7x14_tf);
  u8g2.print(statutS);
  u8g2.setFont(u8g2_font_8x13B_mn );
  u8g2.setCursor(35, 17 + 11);
  u8g2.print(chronoS);

  u8g2.setCursor(11, 33 + 11);
  u8g2.print(temps);
  u8g2.sendBuffer();					// transfer internal memory to the display
  delay(100);
}




#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
//#include <Arduino.h>

#include <nRF24L01.h>
#include <RF24.h>
//#include <Wire.h>

const int RADIO_SS_PIN  = 8;
const int RADIO_CSN_PIN = 9;

const int LCD_SS_PIN = 4;
const int LCD_CSN_PIN = 3;
const int LCD_RST_PIN = 5;
const int LCD_MOSI_PIN = 6;
const int LCD_SCK_PIN = 7;


// Variables and constants
RF24 radio(RADIO_SS_PIN, RADIO_CSN_PIN); // CE, CSN
Adafruit_PCD8544 lcd = Adafruit_PCD8544(LCD_SCK_PIN, LCD_MOSI_PIN, LCD_CSN_PIN, LCD_SS_PIN, LCD_RST_PIN);


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
int raz = 4;
int ledPin = 3;
int sensorValue = 0;
int sensorPin = A0;
volatile unsigned long debut;
volatile byte marche = false;
volatile unsigned long le_temps = 0, le_temps1 = 0;

unsigned long tempo=0;
boolean flip_flop=false;
String chronoS1 = "1";
String chronoS2 = "2";
String chronoS3 = "3";
String chronoS4 = "4";
String statutS ="NO";
bool Dot = true; //Dot state


String lead_zero(int num) {
  String t = "";
  if (num < 10) t = "0";
  return t + String(num);
}

ISR(TIMER2_COMPA_vect) { //timer2 interrupt 2kHz
  if (marche) {
    le_temps1 = le_temps1 + 1;
    le_temps = le_temps1 / 2;
  }
}

void isr1(void) {
  detachInterrupt(digitalPinToInterrupt(bp));
  if (!marche) {
    marche = true;
  } else marche = false;
  delay(50);
  attachInterrupt(digitalPinToInterrupt(bp), isr1, FALLING);
}

void updateDisplay() {
  lcd.clearDisplay();
  lcd.setTextSize(1);

/*  lcd.setCursor(0, 10);
  lcd.setTextColor(WHITE, BLACK);
  lcd.println("current");
  lcd.setTextColor(BLACK);
  lcd.setCursor(0, 0);
  lcd.println("nRF24 Scanner");
*/
  lcd.setCursor(0, 20);
  lcd.println("T: "+chronoS+" S: "+statutS+" "+tempo);
  Serial.println ("LCD - Manche "+manche+ " Groupe " + groupe+" Chrono "+chronoS+" Status "+statutS);

  //lcd.setCursor(0, 6);
  //lcd.println("64");

  lcd.display();
}





void setup(void) {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setChannel(1);// 108 2.508 Ghz
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_1MBPS);
  radio.setAutoAck(1);                     // Ensure autoACK is enabled
  radio.setRetries(2, 15);                 // Optionally, increase the delay between retries & # of retries
  radio.setCRCLength(RF24_CRC_8);          // Use 8-bit CRC for performance
  radio.openReadingPipe(0, address);
  radio.startListening();

  // Setup screen
  lcd.begin();
  lcd.setContrast(50);
  updateDisplay();
 
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
 
  pinMode(bp, INPUT);
  digitalWrite(bp, HIGH); //Pullup
  pinMode(raz, INPUT);
  digitalWrite(raz, HIGH); //Pullup
  attachInterrupt(digitalPinToInterrupt(bp), isr1, FALLING);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  
  tempo=millis(); 
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
      chronoS = chronoS1+chronoS2+chronoS3+chronoS4;
      statutS = &text[11];
      Serial.println ("Manche "+manche+ " Groupe " + groupe+" Chrono "+chronoS+" Status "+statutS);
      updateDisplay();
    }
    //delay(100);
    Dot = not Dot ; // Clignotement des 2 points
    //Serial.println(text);
 }

// debut chrono
  if (!marche) {
    if (!digitalRead(raz)) {
      le_temps1 = 0; le_temps = 0;
    }
  }
  temps = cnv_temps(le_temps);
  sensorValue = analogRead(sensorPin);
 // updateDisplay();
  delay(100);
}



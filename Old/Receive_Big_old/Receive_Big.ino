/*
* Gestion Big afficheur Glider Score - Reception
* 
*    Panneau avec segment 7 digits chinois et carte driver Big Display de sparkfun
*           
* Version 1.2 avril 2018
* Version 1.3 Juin 2018
*  - Gestion des rounds et manches >9          
*  - Masque des dizaines de minutes si valeur =0
*  - Init on affiche des 888888 puis 123456 afin de tester tous les segments et l'ordre
*                
* Olivier Segouin - Arduino 1.6.12
* 
* Library: SPI, RF24,
* 
* Programme reception et gestion afficheur
*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>


// Variables and constants
RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00001";
String manche = "1";
String groupe = "1"; 
String chronoS = "0000";
char chrono[32]="";

//GPIO declarations
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Branchement sur connecteur pas a cote de l'alim
byte segmentClock = 5;
byte segmentLatch = 4;
byte segmentData = 6;
bool Dot = true; //Dot state

void setup() {
  Serial.begin(9600);

  pinMode(segmentClock, OUTPUT);
  pinMode(segmentData, OUTPUT);
  pinMode(segmentLatch, OUTPUT);
  digitalWrite(segmentClock, LOW);
  digitalWrite(segmentData, LOW);
  digitalWrite(segmentLatch, LOW);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setChannel(1);// 108 2.508 Ghz
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_1MBPS);
  //radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(1);                     // Ensure autoACK is enabled
  radio.setRetries(2,15);                  // Optionally, increase the delay between retries & # of retries
  radio.setCRCLength(RF24_CRC_8);          // Use 8-bit CRC for performance
  radio.openReadingPipe(0, address);
  radio.startListening();
   // Init Afficheur afin de verifier son fonctionnement au demarrage
  //showNumber(888888);   
  //delay (3000);
  showNumber (123456);
}


// Convertion valeur vers afficheurs 
void TimeToArray() {
  long manchel = manche.toInt(); // convertion chaine en long pour affichage
  long groupel = groupe.toInt(); // convertion chaine en long pour affichage
  long chronol = chronoS.toInt(); // convertion chaine en long pour affichage
  long Now =  ((manchel %10) * 100000 + (groupel %10) * 10000 + chronol) ;
  
  showNumber(Now);
};



void loop() {
  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));

    if (text[1]=='F') 
      {
       // AF1F1 ... A10F1
        
        manche = text[2];
        groupe = text[4];
        Serial.println ("Manche "+manche+ "Groupe "+groupe);
      }
      //display.println("Manche :"+manche+"   Groupe :"+groupe);
      if (text[0]=='A') 
      {
      //display.print(&text[1]); // on enleve le A
        chronoS = &text[1];
      }
      else
      {
      //display.print(text);
      }
      TimeToArray(); // Get leds array with required configuration
      delay(100);
      Dot = not Dot ; // Clignotement des 2 points
   
     Serial.println(text);
  }
}

//Takes a number and displays 2 numbers. Displays absolute value (no negatives)

void showNumber(long number)
{
  // Write digits out to 7 segment display - Last number in the chain is the first number displayed
  int affdecim =0;
  Dot = true;
  for (byte x = 0 ; x < 6 ; x++)
  {
     int remainder = number % 10;
//   postNumber(remainder, Dot);
     if ((x==3) && (remainder ==0)){
        postNumber(10, Dot); // on masque le 0 des dizaines de minutes
        //Serial.print (remainder);
        //Serial.print (number);
     } else {
        postNumber(remainder, Dot);
     }
     number /= 10;
  }
  //Latch the current segment data
  digitalWrite(segmentLatch, LOW);
  digitalWrite(segmentLatch, HIGH); //Register moves storage register on the rising edge of RCK
}

//Given a number, or '-', shifts it out to the display
void postNumber(byte number, boolean decimal)
{
  //    -  A
  //   / / F/B
  //    -  G
  //   / / E/C
  //    -. D/DP

#define a  1<<0
#define b  1<<6
#define c  1<<5
#define d  1<<4
#define e  1<<3
#define f  1<<1
#define g  1<<2
#define dp 1<<7

  byte segments;

  switch (number)
  {
    case 1: segments = b | c; break;
    case 2: segments = a | b | d | e | g; break;
    case 3: segments = a | b | c | d | g; break;
    case 4: segments = f | g | b | c; break;
    case 5: segments = a | f | g | c | d; break;
    case 6: segments = a | f | g | e | c | d; break;
    case 7: segments = a | b | c; break;
    case 8: segments = a | b | c | d | e | f | g; break;
    case 9: segments = a | b | c | d | f | g; break;
    case 0: segments = a | b | c | d | e | f; break;
    case 10: segments = 0; break;
    case 'c': segments = g | e | d; break;
    case '-': segments = g; break;
  }

  if (decimal) segments |= dp;

  //Clock these bits out to the drivers
  for (byte x = 0 ; x < 8 ; x++)
  {
    digitalWrite(segmentClock, LOW);
    digitalWrite(segmentData, segments & 1 << (7 - x));
    digitalWrite(segmentClock, HIGH); //Data transfers to the register on the rising edge of SRCK
  }
}

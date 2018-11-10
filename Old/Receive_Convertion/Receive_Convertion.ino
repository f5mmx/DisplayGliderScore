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
*  Version pour faire convertion et compatibilité afficheur de type Chateaudun en RS485 
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
char chrono[]="A9876";
char text[]="A1234";
String chainerecu ="A1012";
String oldchainerecu;

//GPIO declarations
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Branchement sur connecteur pas a cote de l'alim
byte segmentClock = 5;
byte segmentLatch = 4;
byte segmentData = 6;
bool Dot = true; //Dot state

void setup() {
  while (!Serial) 
  {
    ; // wait fo r serial port to connect. Needed for native USB port only
  }
  // Start each software serial port
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
//  AffSerial.println("Envoi sur port serie");
}


void loop() {
  if (radio.available()) 
  {
    while (radio.available())
    {
     radio.read (&text, sizeof(text)); /// get the data
    }
    // on calme le jeux et on envoi seulement sur le port afficheur si la valeur a bougé
    chainerecu = &text[0];
    if (chainerecu != oldchainerecu)
    {
      Serial.println (text);
      oldchainerecu = chainerecu;
   
    delay(100);
    
}
}
}

/*
* Gestion Small afficheur Glider Score - Reception
*    Panneau de Led en ruban type WS2812 rgb 
*       version 1.21 mai 2018
*                
* Olivier Segouin - Arduino 1.6.12
* 
* Library: SPI, RF24,
* 
* Programme reception et gestion afficheur
* Gestion de m'affichage en led WS2812 ruban
* 1.2 non affichage du 0 des dizaines de minutes
* 1.21 modification du chiffre 9, rajout de la barre du bas
* 1.21 modification de l'init avec affichage de tous les segments sur 8 pour verification
*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <FastLED.h>

#define NUM_LEDS 170  //6*7*4 +2  Number of LED controles (remember I have 4 leds / controler
#define LED_TYPE    WS2812
#define LED_PIN 5 // Data pin for led comunication


// Variables and constants
RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00001";
String manche = "0";
String groupe = "0"; 
String chronoS = "0000";
char chrono[32]="";


CRGB leds[NUM_LEDS]; // Define LEDs strip
// 10 digit :0...10, each digit is composed of 7 segments of 4 leds - Gestion digit 11 pour effacement du 0 
byte digits[11][28] = {
  {
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0
  }
  , // Digit 0
  {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  }
  , // Digit 1

  {
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
  }
  , // Digit 2

  {
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1
  }
  , // Digit 3

  {
    1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1
  }
  , // Digit 4

  {
   1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1 
  }
  , // Digit 5

  {
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 
  }
  , // Digit 6

  {
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 
  }
  , // Digit 7

  {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
  }
  , // Digit 8

  {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1
  }
  , // Digit 9 
   {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  }
}; // Digit 10 special non affichage 0  | 2D Array for numbers on 7 segment

bool Dot = true; //Dot state
long ledColor = CRGB::Red;
long ledColorR = CRGB::DarkRed;
long ledColorW = CRGB::GhostWhite;
long ledColorB = CRGB::DarkSlateBlue;
long ledColorG = CRGB::DarkSlateGray;
long ledColorL = CRGB::Lime;
long ledColorV = CRGB::DarkViolet;

void showProgramShiftSinglePixel(CRGB crgb, long delayTime) {
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = crgb;
    FastLED.show();
    delay(delayTime);
    leds[i] = CRGB::Black;
  }
}

void setup() {
  Serial.begin(9600);
  
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  
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
  showProgramShiftSinglePixel(CRGB::Maroon, 5); // show "shift single pixel program" with maroon pixel
  delay(100); 
  // init avec allumage des segments en Full 8
  manche = "1";
  groupe = "2";
  chronoS = "3456";
  TimeToArray(); // Get leds array with required configuration
  FastLED.show(); // Display leds array

}


void BrightnessCheck() {
      FastLED.setBrightness( 255);
}

// Convertion valeur vers afficheurs les 2812 
void TimeToArray() {
  long manchel = manche.toInt(); // convertion chaine en long pour affichage
  long groupel = groupe.toInt(); // convertion chaine en long pour affichage
  long chronol = chronoS.toInt(); // convertion chaine en long pour affichage
  long Now =  (manchel * 100000 + groupel * 10000 + chronol) ;
  
  int cursor = 86; 
  //Serial.print("Valeur est: ");
  //Serial.println(chronoS+" : " + Now);
  if (Dot) {
    leds[168] = ledColor;
    leds[169] = ledColor;
   }
   else {
   leds[168] = 0x000000;
   leds[169] = 0x000000;
   };

   for (int i = 1; i <= 6; i++) {

    long digit = Now % 10; // get last digit in time
    if (i == 1) {  
      cursor = 140;
       for (int k = 0; k <= 27; k++) {
         if (digits[digit][k] == 1) {
         leds[cursor] = ledColorR;
        }
        else if (digits[digit][k] == 0) {
          leds[cursor] = 0x000000;
        };
        cursor ++;
      }; // fin for
     }// fin if
  
    else if (i == 2) {
      cursor = 112;
      for (int k = 0; k <= 27; k++) {
        if (digits[digit][k] == 1) {
          leds[cursor] = ledColorR;
        }
        else if (digits[digit][k] == 0) {
          leds[cursor] = 0x000000;
        };
        cursor ++;
      };
   }
    else if (i == 3) {
      cursor = 84;
      for (int k = 0; k <= 27; k++) {
        if (digits[digit][k] == 1) {
          leds[cursor] = ledColorR;
        }
        else if (digits[digit][k] == 0) {
          leds[cursor] = 0x000000;
        };
        cursor ++;
      };
    }
    else if (i == 4) {  // traitement particulier, on n'affiche pas le zero 
      cursor = 56;
      if (digit == 0) {
        digit = 10; // on affiche pas le 0 
      }
      for (int k = 0; k <= 27; k++) {
        if (digits[digit][k] == 1) {
           leds[cursor] = ledColorR; 
        }
        else if (digits[digit][k] == 0) {
          leds[cursor] = 0x000000;
         };
        cursor ++;
      };
    }
    else if (i == 5) {
      cursor = 28;
      for (int k = 0; k <= 27; k++) {
        if (digits[digit][k] == 1) {
          leds[cursor] = ledColorL;  // LedColorL
        }
        else if (digits[digit][k] == 0) {
          leds[cursor] = 0x000000;
        };
        cursor ++;
      };
     }
      else if (i == 6) {
      cursor = 0;
     for (int k = 0; k <= 27; k++) {
        if (digits[digit][k] == 1) {
          leds[cursor] = ledColorW ;   //LedColorW
        }
        else if (digits[digit][k] == 0) {
          leds[cursor] = 0x000000;
        };
        cursor ++;
      };

  };
    Now = Now / 10;
};
};



void loop() {
  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));

    if (text[1]=='F') 
      {
        manche = text[2];
        groupe = text[4];
        //Serial.println ("Manche "+manche+ "Groupe "+groupe);
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
      BrightnessCheck(); // Check brightness
      TimeToArray(); // Get leds array with required configuration
      FastLED.show(); // Display leds array
      delay(100);
      Dot = not Dot ; // Clignotement des 2 points
   
    //Serial.println(text);
  }
}

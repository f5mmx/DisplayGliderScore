/*
* Gestion Afficheur Glider Score - Emmission
*       version 1.5 juillet 2018
*       Adaptation au programme f3K et glider score avec protocole etendue
*       G01R01T0230WT .....
*       version 1.6 septembre 2018
*       Adaptation au programme enhanced protocol de GliderScoer
* rajout d'un OLED pour visu en local               
* 
* Olivier Segouin - Arduino 1.6.12
*
* Library: SPI, RF24,
* 
* Version 1.3 Rajout d'envoi une trame toutes les 250Ms
* Version 1.4 debug de la transmission cyclique envoi toutes les 250mS de la trame radio
* Version 1.5 adaptation protocol etendue
* 
* Etude des frequences Radio 2.4Ghz
* The range is 2.400 to 2.525 Ghz which is 2400 to 2525 MHz (MegaHz). 
* The nRF24L01 channel spacing is 1 Mhz which gives 125 possible channels numbered 0 .. 124. 
* WiFi uses most of the lower channels and we suggest using the highest 25 channels for nRF24L01 projects. 
* On utilisera le Canal 1 - donc 2400+1 le Canal 1 a donc son centre de frequence a 2401MHz
* Le Canal 1 en Wifi est centre sur 2412M€z avec une bande de +-11Mhz, donc descend jusqu'a 2401 
* Qu'en est il en 2.4GHz RC ? 
* SPECTRUM.........2.400 to 2.425 - 80 unique Channels
* FUTABA...........2.426 TO 2.450 - 26..27 unique Channels
* XPS..............2.451 TO 2.475 - 12 unique channels 
* ASSAN............2.476 TO 2.499 
* JETI.............
* GRAUPNER.........
* Afin de garder la compatibilité de tous les afficheurs réalisés, GARDONS le Canal 1 
* 
* Programme emmission connection en USB sur le PC eception et gestion afficheur avec GliderScore DigitalTime
*/






#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


RF24 radio(7, 8); // CE, CSN




const byte address[6] = "00001";
int compt = 0;
String chainerecu = "";
String chainecourante ="";
char chaineradio[]="G01R01T0230WT";

String caractererecu = "";
int octetreception = 0;

char charreception;
String manche ="0";
String groupe ="0"; 
String chrono ="0";
String statut ="NO";

unsigned long temps_ancien = 0;
unsigned long tempcyc = 250L; 

void initaff() {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println("  Version : 1.6 EP");
      display.setCursor(5,30);
      display.setTextSize(5);
      display.setTextColor(WHITE);
      display.print("WAIT");  
      display.display();      
}


void affdisplay() {
//  if (chainecourante != "") {
      manche = chainecourante.substring(2,3);
      groupe = chainecourante.substring(5,6);
      chrono = chainecourante.substring(7,11);
      statut = chainecourante.substring(11,13);
      Serial.println ("Manche "+manche+ "Groupe "+groupe="Chrono "+chrono);
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println("Manche:"+manche+" Groupe:"+groupe+"  "+statut);    
      display.setCursor(5,30);
      display.setTextSize(5);
      display.setTextColor(WHITE);
      display.print(chrono);  
      display.display(); 
      delay(2);
//  }
      
}


void setup() {
  radio.begin();
  radio.openWritingPipe(address);
  radio.setChannel(1);// 1 2.401 Ghz
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_1MBPS);
  //radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(1);                     // Ensure autoACK is enabled
  radio.setRetries(2,15);                  // Optionally, increase the delay between retries & # of retries
  radio.setCRCLength(RF24_CRC_8);          // Use 8-bit CRC for performance
  radio.stopListening();
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.clearDisplay();
  initaff();

  while (!Serial) 
  {
    ; // wait fo r serial port to connect. Needed for native USB port only
  }
  temps_ancien = millis();
}

void loop() {

if (millis() - temps_ancien >= tempcyc)
 {
      temps_ancien =millis();
      chainecourante.toCharArray(chaineradio,chainecourante.length()+1); // récupère la chaine courante dans le tableau de char de chaine radio 
      radio.write(&chaineradio,sizeof(chaineradio)); // envoi de la chaine recu RS en radio
      //Serial.println ("EnvoiradioCyc:"+chainecourante); 
 }

 
 while (Serial.available() > 0) 
 {
    charreception = Serial.read();
    if ((charreception==13) or (charreception==10)) // fin de la trame de gliderscore 
   {
      chainerecu = chainerecu.substring(0,13);  //(0,14) f3K programme demander d'enlever le LF et de ne garder que le CR 
      //Serial.println("On a une trame complete"+ chainerecu);
      chainerecu.toCharArray(chaineradio,chainerecu.length()+1); // récupère la chaine recu dans le tableau de char de chaine radio 
      radio.write(&chaineradio,sizeof(chaineradio)); // envoi de la chaine recu RS en radio
      Serial.println ("Envoiradio:"+chainerecu);
      chainecourante = chainerecu ;
      affdisplay();
      // Reset string et on sort de la boucle
      chainerecu=""; //RAZ le String de réception
      delay(10); // pause
      break; // sort de la boucle while
   }
   else
   {
      chainerecu = chainerecu + charreception;
   }


 }
}






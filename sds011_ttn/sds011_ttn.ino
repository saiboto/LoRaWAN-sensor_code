/**************************************************
      
      by wb1 & saiboto 2020
      
************* Configuration **********************/

#include <SoftwareSerial.h>
#include <avr/pgmspace.h>
#include <TinyLoRa.h>
#include <SPI.h>
#include <string.h> 
#include <Wire.h>
//#include <Adafruit_Sensor.h>
#include <TroykaDHT.h>  //russische Library
DHT dht(4, DHT22);      //Port und DHT-Type 
float t = 0;
float h;
int16_t tempInt;
int16_t humidInt;
int PM2_5;
int PM10;
int messwert[10];
int arbeitsstring[19];
int i = 0;
int z = 8;
const PROGMEM uint16_t startsequenz = {0xAAB4};
const PROGMEM uint8_t endsequenz = {0xAB};
unsigned int displayInt; // Rückgabewert der Funktion zum Auslesen der Daten
int highbyte;
int lowbyte;
char myChar; // Definiere einen Char, um diesen zu bearbeiten
uint16_t crcbyte = 0;

// Network Session Key (MSB)
uint8_t NwkSkey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Application Session Key (MSB)
uint8_t AppSkey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Device Address (MSB)
uint8_t DevAddr[4] = { 0x00, 0x00, 0x00, 0x00 };
/************************** Example Begins Here ***********************************/
// Data Packet to Send to TTN
unsigned char loraData[9]; // = {"hello LoRa"};

// How many times data transfer should occur, in seconds
const unsigned int sendInterval = 15;

// Pinout for Arduino Lora shield v1.4
TinyLoRa lora = TinyLoRa(2, 10, 9); //Arduino lora shield v1.4


SoftwareSerial Seriell1(5, 6); //Softwareseriell

// Mehrdimensionales ARRAY für Komandos, Ablage im Programmspeicher 
const PROGMEM uint16_t charSet[9][8] = {                                         
                                          {  0x0200, 0x000, 0x0, 0x0, 0x0, 0x0, 0xFF, 0xFF00 }, // Report mode
                                          {  0x0400, 0x000, 0x0, 0x0, 0x0, 0x0, 0xFF, 0xFF00 }, //query date mode
                                          {  0x0500, 0x000, 0x0, 0x0, 0x0, 0x0, 0xFF, 0xFF00 }, // set device-id 
                                          {  0x0601, 0x000, 0x0, 0x0, 0x0, 0x0, 0xFF, 0xFF00 }, // set sleep 
                                          {  0x0601, 0x100, 0x0, 0x0, 0x0, 0x0, 0xFF, 0xFF00 }, //set work
                                          {  0x0600, 0x000, 0x0, 0x0, 0x0, 0x0, 0xFF, 0xFF00 }, //get current mode
                                          {  0x0700, 0x000, 0x0, 0x0, 0x0, 0x0, 0xFF, 0xFF00 }, //check firmware
                                          {  0x0801, 0x000, 0x0, 0x0, 0x0, 0x0, 0xFF, 0xFF00 }, //set working period continous                                         
                                          {  0x0801, 0x300, 0x0, 0x0, 0x0, 0x0, 0xFF, 0xFF00 }, //set working period 1 Minute
                  // Beispiel zwei Miuten {  0x0801, 0x200, 0x0, 0x0, 0x0, 0x0, 0xFF, 0xFF00 }, // 2 Minuten                                                                                   
// laut Datasheet www.inovafitness.com Modell SDS011, Firmware Version after 2015.7.20
//5V TTL, 9600baud, 8N1, Datenpaket 19byte, 
};

//*********************** Programbeginn
void setup() {
  Serial.begin(9600);
  Seriell1.begin(9600);
    while (!Serial);  // Warte, bis der serielle Port verbunden ist 
  crcbyte = 0x0;
  arbeitsstring[0] = 0xAA; 
  arbeitsstring[1]= 0xB4;
  i=2;
  for (byte k = 0; k < 8; k++) {
    displayInt = pgm_read_word_near(charSet[z] + k); // Lies den charSet-Wert wieder aus dem Flash-/Programm-Speicher
    arbeitsstring[i] = highByte(displayInt); i++; arbeitsstring[i] = lowByte(displayInt); i++;
// Test    Serial.println(displayInt,HEX); // Gib den gelesenen Wert aus
//    Serial.print(highByte(displayInt),HEX);
//    Serial.print(" ");
//    Serial.println(lowByte(displayInt),HEX);

  if (k < 8)
  {  
      crcbyte = crcbyte + highByte(displayInt) + lowByte(displayInt);
  //    Serial.print("CRC "), Serial.println(crcbyte,HEX);
  }
    
}
//        Serial.println(lowByte(crcbyte),HEX); 
        arbeitsstring[17] = lowByte(crcbyte);
        arbeitsstring[18] = 0xAB;
for (i=0; i<19; i++)
{
  Seriell1.write(arbeitsstring[i]);
  Serial.print(arbeitsstring[i],HEX);
}
  Serial.println(" ");
  i=0;
   dht.begin();   //Sensor aktivieren
  delay(2000);
// Initialize LoRa
  Serial.print("Starting LoRa...");

  lora.setChannel(CH1);    //single Channel
  //  lora.setChannel(MULTI);  //Multichannel
    
  // set datarate
  lora.setDatarate(SF7BW125);
    if(!lora.begin())
  {
    Serial.println("Failed");
    Serial.println("Check your radio");
    while(true);
  }
  pinMode( 4, INPUT_PULLUP); 

Serial.println("Lora gestartet");



  
}
//****************************
void loop() {
  delay(10);
 // Serial.println("Schleife");
hole_daten();
//Serial.println(i);
if (i>9)
{
 //Datensatz komplett aber: Achtung keine ID und CRC Betrachtung
PM2_5= messwert[2] + 256*messwert[3];
Serial.print("PM2.5 : "), Serial.println(PM2_5);
PM10 = messwert[4] + 256*messwert[5];  
Serial.print("PM10  : "), Serial.println(PM10);
  i=0;

//hole die Daten vom DHT
dht.read();
   switch(dht.getState()) {
    // alles OK
    case DHT_OK:
      // hole Werte vom DHT
         // Read temperature in Celsius
     t = dht.getTemperatureC();
   // Read humidity
     h = dht.getHumidity();
  // Umwanlung Messwerte in int (multipliziert mit 100 und gerundet)   
  tempInt = round(t * 100);
   humidInt = round(h * 100);
   }

//LORA Vorbereitung Sendung
  loraData[0] = highByte(PM2_5);
  loraData[1] = lowByte(PM2_5);
  loraData[2] = highByte(PM10);
  loraData[3] = lowByte(PM10);
  loraData[4] = highByte(tempInt);
  loraData[5] = lowByte(tempInt);
  loraData[6] = highByte(humidInt);
  loraData[7] = lowByte(humidInt);
  loraData[8] = lowByte(loraData[0] + loraData[1] + loraData[2] + loraData[3] + loraData[4] + loraData[5] + loraData[6] + loraData[7]);
       Serial.print("Temp  : ");
      Serial.print(t);
      Serial.println(" C \t");
      Serial.print("Humi  : ");
      Serial.print(h);
      Serial.println(" %");
      Serial.print("CRC   : ");
      Serial.println(loraData[8],HEX);
      Serial.println();
// LORA Sendung "on Air" 
   lora.sendData(loraData, sizeof(loraData), lora.frameCounter); 
   lora.frameCounter++;
}}

void hole_daten()  
// Wenn byte verfügbar, hole es aus dem Empfangsbuffer
{
 if (Seriell1.available()) {
     messwert[i] = Seriell1.read();
    i++;
 }
}

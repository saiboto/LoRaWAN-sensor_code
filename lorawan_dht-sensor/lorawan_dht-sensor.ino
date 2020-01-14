// DHT11/22 sensor code & Arduino lora shield v1.4 for TTN Integration
// By Winfried Blase & Tobias v.K-S 
// TTN-Eberswalde 2019

/************************** Configuration ***********************************/
/* ++++ Payload Decoder 
 *  
 *  
 *  Decode:
function Decoder(bytes, port) {
var meinwert1 = ((bytes[0] << 8) | bytes[1]) / 100;
var meinwert2 = ((bytes[2] << 8) | bytes[3]) / 100
return {
measurement: {
temp1: meinwert1,
temp2: meinwert2
}
}
}
Ende decoder ++++++
 */
 
#include <TinyLoRa.h>
#include <SPI.h>
#include <string.h> 
#include <TroykaDHT.h>  
DHT dht(4, DHT11);      //Port! and DHT-Type! 
float t;
float h;
int16_t tempInt;
int16_t humidInt;
// Network Session Key (MSB)
uint8_t NwkSkey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
// Application Session Key (MSB)
uint8_t AppSkey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  };
// Device Address (MSB)
uint8_t DevAddr[4] = { 0x00, 0x00, 0x00, 0x00 };
/************************** Example Begins Here ***********************************/
// Data Packet to Send to TTN
unsigned char loraData[5]; // = {"hello LoRa"};
// How many times data transfer should occur, in seconds
const unsigned int sendInterval = 60;
// Pinout for Arduino Lora shield v1.4
TinyLoRa lora = TinyLoRa(2, 10, 9); //Arduino lora shield v1.4
void setup()
{
  dht.begin();   //Sensor aktivation
  delay(2000);
  Serial.begin(115200);
  while (! Serial);
  // Initialize LoRa
  Serial.print("Starting LoRa...");
 // lora.setChannel(CH1);    //single Channel
    lora.setChannel(MULTI);  //Multichannel
    
  // set datarate
  lora.setDatarate(SF7BW125);
    if(!lora.begin())
  {
    Serial.println("Failed");
    Serial.println("Check your radio");
    while(true);
  }
  
//++++++++ Set transmission power to 100 milliWatt (20 dbm)
// ++++++++++ if commented out --> then only 17dbm
// +++++++++++ lower values are also possible

//  lora.setPower(20);

//++++++++++++++++++  
  
  Serial.println("OK");
}
void loop()
{
  dht.read();
   switch(dht.getState()) {
    // nothing amiss
    case DHT_OK:
      // get values from DHT
         // Read temperature in Celsius
     t = dht.getTemperatureC();
   // Read humidity
     h = dht.getHumidity();
// Serial of the measured values can be commented out
      Serial.print("Temperature = ");
      Serial.print(t);
      Serial.println(" C \t");
       Serial.print("Humidity = ");
      Serial.print(h);
      Serial.println(" %");
// comment out end ++++++++++++
// Conversion measured values to int (multiplied by 100 and rounded)
   tempInt = round(t * 100);
   humidInt = round(h * 100);
   }
   
  loraData[0] = highByte(tempInt);
  loraData[1] = lowByte(tempInt);
  loraData[2] = highByte(humidInt);
  loraData[3] = lowByte(humidInt);
   lora.sendData(loraData, sizeof(loraData), lora.frameCounter); 
// can be commented out
  Serial.print("Frame Counter: ");
  Serial.println(lora.frameCounter);
// up to here you can comment out
  lora.frameCounter++;
/*
  // blink LED to indicate packet sent
  digitalWrite(13, HIGH);
  delay(1000);
  digitalWrite(13, LOW);
*/  
  Serial.println("delaying...");
  delay(sendInterval * 1000);
}

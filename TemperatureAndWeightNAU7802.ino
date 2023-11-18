#include <Arduino.h>
#include <Wire.h>
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h" 
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

NAU7802 amplifier;

#define TEMPSLOPE 17247 //the adc reading decreases with increasing temperature. So slope is actually negative. 
#define INTERCEPT 7446102
#define WEIGHTZERO 43714 
#define WEIGHTSLOPE 3.4338 //cal value for load cell units to g 

#define SAMPLEFREQ 300 //seconds
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

StaticJsonDocument<500> doc; //use StaticJSonDocument, which was imported, to create a 500 character long document called doc.

void setup() {

  bool powerUp();
  
  esp_sleep_enable_timer_wakeup(SAMPLEFREQ * uS_TO_S_FACTOR);

  // Initialize serial and wait for port to open.
  Serial.begin(115200);
  for (int i=0;i<20;i++){
      delay(100);
  }  
  
  Serial.println("Setup ESP32S2 dev module to sleep for every " + String(SAMPLEFREQ) + " Seconds");
  Wire.begin();
  amplifier.begin();
  amplifier.setSampleRate(NAU7802_SPS_80);

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
    //for loop for non-blocking. This helps the temperature sensor stabilize.
    for (int i=0;i<10;i++){
      delay(1000);
    }    
    getDevice();
    
    float temperature = getTemp();
//    Serial.print("temperature:");
    Serial.print(temperature, 3);// second number is the # digits printed.
    Serial.print(",");

    delay(1500);

    float weight = getWeight();
//    Serial.print("weight:");
    Serial.println(weight, 3);
//    Serial.print(",");

    POSTData();

    bool powerDown();
    esp_deep_sleep_start();
}

float getTemp(){

  amplifier.setRegister(0x11, 2);
  
  delay(1500);
  
  //default sample rate is 80hz. Noise increases at higher rates. This code crashes at some lower rates. Changing the rate also changes the output!

  float tempSlope = TEMPSLOPE;
  float intercept = INTERCEPT;
  
  float adcReading = amplifier.getAverage(40);

  float temperature = -(adcReading - intercept)/(tempSlope);

  doc["sensors"]["temperature"] = temperature;
  return temperature;
}

float getWeight(){
    amplifier.setRegister(0x11, 0);
    
    delay(1500);

    return amplifier.getAverage(70);//not more than 80 - times out at 1000 ms
}

void POSTData()
{
      if(WiFi.status()== WL_CONNECTED){
        HTTPClient http;
  
        http.begin(serverName);
        http.addHeader("Content-Type", "application/json");
  
        String json;
        serializeJson(doc, json);
  
        Serial.println(json);
        int httpResponseCode = http.POST(json);
        
        Serial.println(httpResponseCode);
      }
}

void getDevice()
{
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();

    uint64_t chipid=ESP.getEfuseMac();//The chip ID is essentially its MAC address(length: 6 bytes).

    char buffer[200];
    sprintf(buffer, "%04X%08X",(uint16_t)(chipid>>32),(uint32_t)chipid);

    doc["device"]["IP"] = WiFi.localIP().toString();
    doc["device"]["RSSI"] = String(WiFi.RSSI());
    doc["device"]["type"] = TYPE;
    doc["device"]["name"] = name;
    doc["device"]["chipid"] = buffer;
    
}

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
#define WEIGHTZERO 2000 //It reads about 2000 at zero although I do see bimodal fluctuation from 2077 to 2251
#define WEIGHTSLOPE 3.4338 //cal value for load cell units to g 

#define SAMPLEFREQ 5 //seconds
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

StaticJsonDocument<500> doc; //use StaticJSonDocument, which was imported, to create a 500 character long document called doc.

//void setTimezone(String timezone){
//  Serial.printf("  Setting Timezone to %s\n",timezone.c_str());
//  setenv("TZ",timezone.c_str(),1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
//  tzset();
//}
//
//void initTime(String timezone){
//  struct tm timeinfo;
//
//  Serial.println("Setting up time");
//  configTime(0, 0, "pool.ntp.org");    // First connect to NTP server, with 0 TZ offset
//  if(!getLocalTime(&timeinfo)){
//    Serial.println("  Failed to obtain time");
//    return;
//  }
//  Serial.println("  Got the time from NTP");
//  // Now we can set the real timezone
//  setTimezone(timezone);
//}

void setup() {
  
  //  initTime("PST8PDT,M3.2.0,M11.1.0");//Vancouver
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

//  Serial.print("Connecting to ");
//  Serial.print(ssid);
//  Serial.print(" with password ");
//  Serial.println(password);

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
    
//    Serial.println("temperature");
    float temperature = getTemp();
    Serial.print("temperature:");
    Serial.print(temperature, 3);// second number is the # digits printed.
    Serial.print(",");

//    Serial.println(amplifier.getRegister(0x11));
//Needs time for amplfier to switch over and stabilize?
    delay(1500);

    //Serial.println("weight");
    float weight = getWeight();
    Serial.print("weight:");
    Serial.println(weight, 3);
    Serial.print(",");

//    Serial.print("Connected to WiFi network with IP Address: ");
//    Serial.println(WiFi.localIP());

//    delay(1000);
//    Serial.println("Posting...");

      POSTData();
    
//    serializeJsonPretty(doc, Serial);
//    Serial.println("\nDone.");
    
//    Serial.println(amplifier.getRegister(0x11));
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
    
    delay(10);

    long accumulator;
    int counter = 0;
    long average;
    int numSamples = 300;

    for(counter = 0; counter < numSamples;counter++){
      if(amplifier.available() == true)
      {
        long currentReading = amplifier.getReading();
        accumulator += currentReading;
      }
    }

    average = accumulator/counter;

    float mass = (average-WEIGHTZERO)/WEIGHTSLOPE;
    doc["sensors"]["mass"] = mass;
    return mass;
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
//    Serial.printf("***ESP32 Chip ID = %04X%08X\n",(uint16_t)(chipid>>32),(uint32_t)chipid);//print High 2 bytes
    char buffer[200];
    sprintf(buffer, "%04X%08X",(uint16_t)(chipid>>32),(uint32_t)chipid);
    
    //sprintf(buffer, "esp32%" PRIu64, ESP.getEfuseMac());

    // int vbatt_raw = 0;
    // for (int i=0;i<SAMPLES;i++)
    // {
    //    vbatt_raw += analogRead(PIN_POWER);
    //    delay(100);
    // }
    // vbatt_raw = vbatt_raw/SAMPLES;
    //float vbatt = map(vbatt_raw, 0, 4096, 0, 4200);

    doc["device"]["IP"] = WiFi.localIP().toString();
    doc["device"]["RSSI"] = String(WiFi.RSSI());
    doc["device"]["type"] = TYPE;
    doc["device"]["name"] = name;
    doc["device"]["chipid"] = buffer;
    
//    doc["device"]["bootCount"] = bootCount;
//    doc["device"]["wakeup_reason"] = String(wakeup_reason);
//    doc["device"]["vbatt_raw"] = vbatt_raw;
    //doc["device"]["vbatt"] = map(vbatt_raw, 0, 4096, 0, 4200);

}

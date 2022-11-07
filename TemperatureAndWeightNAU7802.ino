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

StaticJsonDocument<500> doc; //use StaticJSonDocument, which was imported, to create a 500 character long document called doc.

void setup() {
  
  // Initialize serial and wait for port to open.
  Serial.begin(9600);
  delay(1500); 

  Wire.begin();
  amplifier.begin();
  amplifier.setSampleRate(NAU7802_SPS_80);

  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.print(" with password ");
  Serial.println(password);

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
    delay(200);
    
    getDevice();
    
//    Serial.println("temperature");
    getTemp();
//    Serial.println(temperature, 3);// second number is the # digits printed.

//    Serial.println(amplifier.getRegister(0x11));

    delay(1500);

    //Serial.println("weight");
    getWeight();
    //Serial.println(weight, 3);

    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());

    delay(1000);
    Serial.println("Posting...");
    POSTData();
    serializeJsonPretty(doc, Serial);
    Serial.println("\nDone.");
    
//    Serial.println(amplifier.getRegister(0x11));
    
}

float getTemp(){

  amplifier.setRegister(0x11, 2);
  
  delay(1500);
  
  //default sample rate is 80hz. Noise increases at higher rates. This code crashes at some lower rates. Changin the rate also changes the output!

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
    int mass;

    for(counter = 0; counter < numSamples;counter++){
      if(amplifier.available() == true)
      {
        long currentReading = amplifier.getReading();
        accumulator += currentReading;
      }
    }

    average = accumulator/counter; // counter survives outside the for loop because it is actually global.

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
    Serial.printf("***ESP32 Chip ID = %04X%08X\n",(uint16_t)(chipid>>32),(uint32_t)chipid);//print High 2 bytes
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
    doc["device"]["bootCount"] = bootCount;
    doc["device"]["wakeup_reason"] = String(wakeup_reason);
    //doc["device"]["vbatt_raw"] = vbatt_raw;
    //doc["device"]["vbatt"] = map(vbatt_raw, 0, 4096, 0, 4200);

}

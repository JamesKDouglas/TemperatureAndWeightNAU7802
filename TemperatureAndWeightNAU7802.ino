
#include <Wire.h>
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h" 
//#include "thingProperties.h"

NAU7802 amplifier;

#define TEMPSLOPE 17247 //the adc reading decreases with increasing temperature. So slope is actually negative. 
#define INTERCEPT 7446102

void setup() {
  
  // Initialize serial and wait for port to open.
  Serial.begin(9600);
  delay(1500); 

  Wire.begin();
  amplifier.begin();
  amplifier.setSampleRate(NAU7802_SPS_80);
}

void loop() {
    delay(200);
    
    Serial.println("temperature");
    Serial.println(getTemp(), 3);// second number is the # digits printed.

    Serial.println(amplifier.getRegister(0x11));

    delay(1500);

    Serial.println("weight");
    Serial.println(getWeight(), 3);
    
    Serial.println(amplifier.getRegister(0x11));
    
}

float getTemp(){

  amplifier.setRegister(0x11, 2);
  
  delay(1500);
  
  //default sample rate is 80hz. Noise increases at higher rates. This code crashes at some lower rates. Changin the rate also changes the output!

  float tempSlope = TEMPSLOPE;
  float intercept = INTERCEPT;
  
  float adcReading = amplifier.getAverage(40);

  float temperature = -(adcReading - intercept)/(tempSlope);
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

    average = accumulator/counter; // counter survives outside the for loop because it is actually global.
    return average;
}

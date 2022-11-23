// const char* ssid = "MakerLabsNG-2.4GHz";
// const char* password = "spiral-flute-endmill";
const char* ssid = "Casa St George";
const char* password = "Bananasinpajamas";
const char* serverName = "https://us-west-2.aws.data.mongodb-api.com/app/welygray-ggqoi/endpoint/deliverdata?secret=SoSecret";


// -- Project -------------------------------------------
#define CLIENT                  "Wely Gray 1"        // Client ID for the ESP (or something descriptive "Front Garden")
#define TYPE                    "ESP32s2 50Kg load cell and NAU7802"               // Type of Sensor ("Hornbill ESP32" or "Higrow" or "ESP8266" etc.)  

// -- Other - Helpers ------------------------------------
#define uS_TO_S_FACTOR          1000000               // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP           10                   // Time ESP32 will go to sleep (in seconds) 
#define TIME_TO_SNOOZE          5                     // Time ESP32 will go to sleep (in seconds) 

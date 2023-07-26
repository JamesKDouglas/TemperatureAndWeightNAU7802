//rename this file to secrets.h and fill out the values:

 const char* ssid = "*";
 const char* password = "*";

//serverName:
//The serverName comes from a MongoDB Realm application managed from realm.mongodb.com
//You can get to Realm using the App Services tab from the Atlas page.

//First you have to make an app, connect it to your database then make a function 
//called addData.

//The function acceptData uses the code,

// exports = function(payload){
//     var atlas = context.services.get("mongodb-atlas");
//     var coll = atlas.db("Cluster0").collection("TimeSeries.Gray1");
//     try {
//       if (payload.body) 
//       {
//         //const ts = {ts:new Date()};
//         //const ts_ej = EJSON.stringify(ts);
//         body = EJSON.parse(payload.body.text());
//         body['time'] = new Date();
//       }
//       coll.insertOne(body);
//       console.log(body);
//     } catch (e) {
//       console.log("Error inserting doc: " + e);
//       return e.message();
//     }
// };

//This code has to be associated with the name addData by putting it into
// the Function Editor in Realm.

//Then you will be able to find the string to put in the quotes by going to HTTPS Endpoints
//which is also in the Build section of the toolbar on the left.

//The first part of the serverName string will be listed under Operation Type
//The second is of the format ?secret=XXX
//You have to make the secret XXX text using the Authorization section of the HTTPS Endpoints page.

//Once you have both parts, attach them together and put them as a string below:
const char* serverName = "";
//ex: 
//const char* serverName = "https://us-west-2.aws.data.mongodb-api.com/app/welygray-ggqoi/endpoint/deliverdata?secret=sosecret"

//Name is the name of the Realm app.
const char* name = "";

// -- Project -------------------------------------------
#define CLIENT                  "Wely Gray 1"        // Client ID for the ESP (or something descriptive "Front Garden")
#define TYPE                    "ESP32s2 50Kg load cell and NAU7802"               // Type of Sensor ("Hornbill ESP32" or "Higrow" or "ESP8266" etc.)  

// -- Other - Helpers ------------------------------------
#define uS_TO_S_FACTOR          1000000               // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP           10                   // Time ESP32 will go to sleep (in seconds) 
#define TIME_TO_SNOOZE          5                     // Time ESP32 will go to sleep (in seconds) 

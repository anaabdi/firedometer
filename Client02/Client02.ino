#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>

#define FIREBASE_HOST ""
#define FIREBASE_AUTH ""
//#define WIFI_SSID "adfasdgasdgsdg"
//#define WIFI_PASSWORD "anakayamgapakespasi"
#define WIFI_SSID "mi"
#define WIFI_PASSWORD "futsalkuy"
 
boolean DEBUG = true;
const String INFORMED_SPEED_NODE = "/speeds/informed";

void setup()  
{
  Serial.begin(9600);
  mp3_set_serial (Serial);  //set Serial for DFPlayer-mini mp3 module 
  delay(100);  //wait 1ms for mp3 module to set volume
  mp3_set_volume (80);

  setupWifi();

  //Begin firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); 
  
  int initialSpeed = Firebase.getInt(INFORMED_SPEED_NODE);
  Serial.print("initialSpeed: ");
  Serial.println(initialSpeed);
  
  //Start stream
  Firebase.stream(INFORMED_SPEED_NODE);
}
 
void loop() {
  // Check if stream is failed
  if (Firebase.failed()) {
    Serial.print("ERROR: streaming error: ");
    Serial.println(Firebase.error());
    delay(1000);
    Firebase.stream(INFORMED_SPEED_NODE);
    return;
  }
  
  Serial.print("heap: ");
  if (Firebase.available()) {
      FirebaseObject event = Firebase.readEvent();
      String eventType = event.getString("type");
      eventType.toLowerCase();
      
      if (eventType == "put") {
          int receivedSpeed = event.getInt("data");
          Serial.print("receivedSpeed: ");
          Serial.println(receivedSpeed);
    
          inform(receivedSpeed);

          // reset speed in firebase
          //Firebase.setInt(INFORMED_SPEED_NODE, 0);
      }

      // set streaming again, manually
      //Firebase.stream(INFORMED_SPEED_NODE);
  }
  Serial.println(ESP.getFreeHeap());
  delay(1000);
}
 
void inform(int receivedSpeed) {
  Serial.print("informed speed: ");
  Serial.println(receivedSpeed);
  mp3_play(receivedSpeed);
  /*if(receivedSpeed >= 60) {
    mp3_play (receivedSpeed);
  } else {
    mp3_play (30);
  }*/
}

void setupWifi() {
//  connect to wifi.
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("connecting");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.println();
    Serial.print("connected: ");
    Serial.println(WiFi.localIP());
}

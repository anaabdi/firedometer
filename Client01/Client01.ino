#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

// Set these to run example.
#define FIREBASE_HOST ""
#define FIREBASE_AUTH ""
//#define WIFI_SSID "adfasdgasdgsdg"
//#define WIFI_PASSWORD "anakayamgapakespasi"
#define WIFI_SSID "mi"
#define WIFI_PASSWORD "futsalkuy"

 
// Change DEBUG to true to output debug information to the serial monitor
boolean DEBUG = true;
const int sensorPin = 16; //GPI0 2
const int buttonPin = D3; //GPI0 5
const float diameterRoda = 0.8;//0.3556;
const String CHANGED_SPEED_NODE = "/speeds/changed";
const String INFORMED_SPEED_NODE = "/speeds/informed";

unsigned long rotationCount = 0;
unsigned long currentTime = 0;
unsigned long lastSavedSpeedAt = 0;
unsigned long startTime = 0;
unsigned long deltaTime = 0;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH;

boolean countFlag = false;
boolean isMoreThanMaxSpeed = false;
int SPEED = 0;

void setup()  
{
  if (DEBUG) {
    // open serial communication for debugging and show 
    // the sketch filename and the date compile2d
    Serial.begin(9600);
    Serial.println(__FILE__);
    Serial.println(__DATE__);
    Serial.println(" ");
  }
  
  pinMode(buttonPin, INPUT_PULLUP);
  digitalWrite(buttonPin, HIGH);

  setupWifi();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop()  
{
  int reading = digitalRead(buttonPin);
  
  if (overFiveSecs(lastSavedSpeedAt)) {
    Serial.print("SPEED Before:");Serial.println(SPEED);
    SPEED = 0;
    lastSavedSpeedAt = 0;
    resetSpeed(INFORMED_SPEED_NODE);
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        Serial.println("ENOL");
        
        if (SPEED >= 20) {
          Serial.print("SPEED Sent:");Serial.println(SPEED);
          sendSpeed(INFORMED_SPEED_NODE, SPEED);
          }

      }
    }
  }

  if (digitalRead(sensorPin) == HIGH) {
    //Serial.println("HIGH");
    countFlag = true;
  }
  
  if (digitalRead(sensorPin) == LOW && countFlag == true) {
    Serial.println("LOW");
    rotationCount++;
    countFlag = false;
    currentTime = micros();
     
    if (startTime > 0) {
      deltaTime = currentTime - startTime;
      Serial.print("delta:");Serial.println(deltaTime);
      if(deltaTime > 2000000) {
        SPEED = 0;
      } else {
        
        SPEED = getSpeed();
        if (SPEED > 0) {
            sendSpeed(CHANGED_SPEED_NODE, SPEED);
          }
        
      }

      lastSavedSpeedAt = currentTime;

      Serial.print("SPEED:");Serial.println(SPEED);
      if(SPEED > 60) {
        if(isMoreThanMaxSpeed == false) {
          sendSpeed(INFORMED_SPEED_NODE, 180);
          isMoreThanMaxSpeed == true;
          Serial.println("MASUK");
        }
      } 
      else {
        isMoreThanMaxSpeed == false;
      }
    }
    Serial.print("current:");Serial.println(currentTime);
    startTime = currentTime;
  }

  lastButtonState = reading;
}

bool overFiveSecs(unsigned long lastSavedTime) {
  if (lastSavedTime == 0) {
    return false;
    }
    
  deltaTime = micros() - lastSavedTime;
  return deltaTime > 10000000;
  }

int getSpeed() {
  float mySpeed = 11314285.714285714285714*diameterRoda/deltaTime;
  return mySpeed;
}

void setupWifi() {
//     connect to wifi.
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

void resetSpeed(String node) {
  Firebase.remove(node);
  }

void sendSpeed(String node, int speed) {
    Firebase.setInt(node, speed);
    // handle error
    if (Firebase.failed()) {
        Serial.print(node);
        Serial.print(" - setting failed:");
        Serial.println(Firebase.error());  
        return;
    }
  }

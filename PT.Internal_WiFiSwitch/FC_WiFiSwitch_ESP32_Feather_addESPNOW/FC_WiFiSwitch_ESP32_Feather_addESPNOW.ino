// 12.18.2024 update: change from webSerial to webSerialLite, TCP replies for all incoming messages,  "STATE" message reply

/*
YOU MUST IMPLEMENT THE FOLLOWING CHANGES TO ENSURE COMPILATION:
  https://github.com/me-no-dev/ESPAsyncWebServer/issues/1419 read ednieuw's comment:
  Make changes to Arduino\libraries\ESPAsyncWebServer\src\WebAuthentication.cpp
  Make changes to AsyncEventSource.cpp
  Make changes to AsyncWebSocket.cpp

  https://github.com/me-no-dev/ESPAsyncWebServer/issues/1442 read sepp89117's comment: 
  Make changes to AsyncWebServer.cpp

  Everything should compile after that!
*/

#include "global.h"
#include "config.h"

#define RelayPin 32
#define LEDpin 14
Adafruit_NeoPixel pixels(3, LEDpin, NEO_GRB + NEO_KHZ800);

bool state = true;

void setup() {

  //TEMPLATE FUNCTIONS, DO NOT EDIT UNLESS NECESSARY//
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RelayPin, OUTPUT);
  pixels.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setPixelColor(0, pixels.Color(150, 0, 0));
  pixels.setPixelColor(1, pixels.Color(0, 150, 0));
  pixels.setPixelColor(2, pixels.Color(0, 0, 150));
  pixels.show();
  delay(1000);
  commsSetup();  //Initializes all communications protocols
  delay(5000);

  printLogs();
  pixels.clear();
  updateLEDs();
}

void loop() {
  receiveComms();
  // digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  // Serial.println("High!");
  // delay(1000);                     // wait for a second
  // digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
  // Serial.println("Low!");
  // delay(1000);  // wait for a second
}

void messageReceived(String msg) {
  pixels.setPixelColor(2, pixels.Color(0, 0, 150));
  pixels.show();

  //RECEIVE MESSAGES FROM ALL COMMS, CREATE LISTENERS FOR DIFFERENT MESSAGES BELOW//////


  if (msg.indexOf("RESET") > -1) {
    powerCycle();
  }

  else if (msg.indexOf("START") > -1) {
    broadcastln("START received");
    //do stuff for START
  }

  else if (msg.indexOf("ON") > -1) {
    broadcastln("ON received");
    //do stuff for ON
    state = true;
    digitalWrite(RelayPin, !state);
  }

  else if (msg.indexOf("OFF") > -1) {
    broadcastln("OFF received");
    //do stuff for "OFF"
    state = false;
    digitalWrite(RelayPin, !state);
  }

  else if (msg.indexOf("TOGGLE") > -1) {
    broadcastln("TOGGLE received");
    //do stuff for "OFF"
    state = !state;
    digitalWrite(RelayPin, !state);
  }

  else if (msg.indexOf("STATE") > -1) {
    broadcastln("STATE received");
    broadcast("State = ");
    broadcastln(String(state));
  }


  else if (msg.indexOf("LOG") > -1) {
    printLogs();
  }

  if (msg.length() > 0) {
    lastMsg = msg;
    lastMsgTime = millis();
  }
  updateLEDs();
  delay(100);
  pixels.setPixelColor(2, pixels.Color(0, 0, 0));
  pixels.show();
}

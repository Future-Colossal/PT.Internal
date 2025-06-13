/*
    Make an OSC message and send it over UDP via WiFi
    
    Dom Chang - Future Colossal
 */

#include <WiFi.h>
#include <NetworkUdp.h>
#include <SPI.h>
#include <OSCMessage.h>
#include <Bounce2.h>
#include "Adafruit_VL53L0X.h"


// WE WILL attach() THE BUTTON TO THE FOLLOWING PIN IN setup()
#define BUTTON_PIN 21

// DEFINE THE PIN FOR THE LED :
#define LED_PIN LED_BUILTIN

#include <Bounce2.h>
Bounce bounce = Bounce();
int ledState = LOW;

// WiFi network name and password:
const char *networkName = "FC_Orbi";
const char *networkPswd = "FC123456789office";

//IP address to send UDP data to:
// either use the ip address of the server or
// a network broadcast address
const char *udpAddress = "192.168.1.91";
const int udpPort = 8000;

//Are we currently connected?
boolean connected = false;

//The udp library class
NetworkUDP udp;


void setup() {
  // Initialize hardware serial:
  Serial.begin(115200);
  Serial.println("bruh");
  while (!Serial)
    ;

  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);
  bounce.attach(BUTTON_PIN, INPUT_PULLUP);  // USE EXTERNAL PULL-UP

  // DEBOUNCE INTERVAL IN MILLISECONDS
  bounce.interval(5);

  // LED SETUP
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState);
}

void loop() {
  //the message wants an OSC address as first argument
  if (connected) {
    bounce.update();
    if (bounce.changed()) {
      int debouncedInput = bounce.read();
      if (debouncedInput == HIGH) {       //IF THE BUTTON ISN'T PRESSED
        ledState = !ledState;             // SET ledState TO THE OPPOSITE OF ledState
        digitalWrite(LED_PIN, ledState);  // WRITE THE NEW ledState

        OSCMessage msg("Button up!");
        msg.add((int32_t)1);
        udp.beginPacket(udpAddress, udpPort);
        msg.send(udp);    // send the bytes to the SLIP stream
        udp.endPacket();  // mark the end of the OSC Packet
        msg.empty();      // free space occupied by message
        Serial.println("Button up!");
      } else if (debouncedInput == LOW) {  //IF THE BUTTON IS PRESSED
        ledState = !ledState;              // SET ledState TO THE OPPOSITE OF ledState
        digitalWrite(LED_PIN, ledState);   // WRITE THE NEW ledState

        OSCMessage msg("/Mx/button/4271");
        msg.add((int32_t)1);
        udp.beginPacket(udpAddress, udpPort);
        msg.send(udp);    // send the bytes to the SLIP stream
        udp.endPacket();  // mark the end of the OSC Packet
        msg.empty();      // free space occupied by message
        Serial.println("Button message sent!");
      }
    }
  }
}

void connectToWiFi(const char *ssid, const char *pwd) {
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);  // Will call WiFiEvent() from another thread.

  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

// WARNING: WiFiEvent is called from a separate FreeRTOS task (thread)!
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      //When connected set
      Serial.print("WiFi connected! IP address: ");
      Serial.println(WiFi.localIP());
      //initializes the UDP state
      //This initializes the transfer buffer
      udp.begin(WiFi.localIP(), udpPort);
      connected = true;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      connected = false;
      break;
    default: break;
  }
}

void readTOF() {
}

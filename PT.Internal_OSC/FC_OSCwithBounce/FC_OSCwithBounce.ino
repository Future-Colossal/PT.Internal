#include <OSCMessage.h>

#include <WiFi.h>
#include <NetworkUdp.h>
#include <SPI.h>
#include <OSCMessage.h>

#define BOUNCE_PIN 21
#define LED_PIN LED_BUILTIN
#include <Bounce2.h>

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

// INSTANTIATE A Bounce OBJECT
Bounce bounce = Bounce();

// SET A VARIABLE TO STORE THE LED STATE
int ledState = LOW;

void setup() {
  // Initialize hardware serial:
  Serial.begin(115200);
  while (!Serial)
    ;

  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);
  
  bounce.attach(BOUNCE_PIN, INPUT_PULLUP);  // USE INTERNAL PULL-UP

  // DEBOUNCE INTERVAL IN MILLISECONDS
  bounce.interval(5);  // interval in ms

  // LED SETUP
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState);
}

void loop() {
  //the message wants an OSC address as first argument
  if (connected) {
    bounce.update();
    if (bounce.changed()) {
      // THE STATE OF THE INPUT CHANGED
      // GET THE STATE
      int debouncedInput = bounce.read();

      // IF THE CHANGED VALUE IS LOW
      if (debouncedInput == LOW) {
        ledState = !ledState;             // SET ledState TO THE OPPOSITE OF ledState
        digitalWrite(LED_PIN, ledState);  // WRITE THE NEW ledState
        OSCMessage msg("/Mx/fader/4273");
        //msg.add((int32_t)analogRead(255));
        udp.beginPacket(udpAddress, udpPort);
        msg.send(udp);    // send the bytes to the SLIP stream
        udp.endPacket();  // mark the end of the OSC Packet
        msg.empty();      // free space occupied by message
        Serial.println("OSC Sent!");


      } else if (debouncedInput == HIGH) {
        OSCMessage msg("Button up...");
        udp.beginPacket(udpAddress, udpPort);
        msg.send(udp);    // send the bytes to the SLIP stream
        udp.endPacket();  // mark the end of the OSC Packet
        msg.empty();      // free space occupied by message
        Serial.println("OSC Sent!");
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

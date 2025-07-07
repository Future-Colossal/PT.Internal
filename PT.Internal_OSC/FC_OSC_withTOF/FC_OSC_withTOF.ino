/*
    Reads distance from TOF sensor. 
    If the distance is close:
    Make an OSC message and send it over UDP via WiFi.
    Dom Chang - Future Colossal
 */

#include <WiFi.h>
#include <NetworkUdp.h>
#include <SPI.h>
#include <OSCMessage.h>
//#include <Bounce2.h>
#include "Adafruit_VL53L0X.h"

// DEFINE THE PIN FOR THE LED :
#define LED_PIN LED_BUILTIN
int ledState = LOW;

/*
// WE WILL attach() THE BUTTON TO THE FOLLOWING PIN IN setup()
#define BUTTON_PIN 21
Bounce bounce = Bounce();
bounce.attach(BUTTON_PIN, INPUT_PULLUP);  // USE EXTERNAL PULL-UP

// DEBOUNCE INTERVAL IN MILLISECONDS
bounce.interval(5);
*/

int busy = 0;
int bounceCount = 0;
int bounceThresh = 3;
int sensorReading;
int minThresh = 20;
int maxThresh = 125;
boolean lastState = 0;


// WiFi network name and password:
const char *networkName = "FC_Orbi";
const char *networkPswd = "FC123456789office";
#define myID 85
IPAddress local_IP(192, 168, 1, myID);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0); 

//IP address to send UDP data to:
// either use the ip address of the server or
// a network broadcast

//const char *udpAddress = "192.168.1.78"; //DOM LAPTOP
 const char *udpAddress = "192.168.1.35";  //MAIN LIGHT PC

const int udpPort = 8000;

//Are we currently connected?
boolean connected = false;

//The udp library class
NetworkUDP udp;

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  // Initialize hardware serial:
  Serial.begin(115200);
  delay(2000);
  // while (!Serial) {
  //   delay(1);
  // };
  Serial.println("bruh");

  //Check connection with TOF sensor
  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    Serial.println("Check device connections / restart!");
    ESP.restart();
  }

  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);

  // LED SETUP
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState);
}

void loop() {
  if (connected) {  //IF WE ARE CONNECT TO WIFI, TAKE MEASUREMENTS
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);  // pass in 'true' to get debug data printout!

    if (measure.RangeStatus != 4) {  // If the measurement is within range and phase failures do not have incorrect data
      Serial.print("Distance (mm): ");
      Serial.println(measure.RangeMilliMeter);
      sensorReading = measure.RangeMilliMeter;

      if (sensorReading > maxThresh || sensorReading < minThresh) {  //IF THE BALL IS NOT DETECTED
        if (lastState == 1) {
          OSC_Trig1();  //IF THE lastState was HIGH, trigger OSC message 1
        }
      }

      else if (sensorReading <= maxThresh && sensorReading != 0 && sensorReading >= minThresh) {  //IF THE BALL IS DETECTED
        if (lastState == 0) {
          OSC_Trig2();  //IF THE LAST STATE WAS LOW, trigger OSC message 2
        }
      }
    } 
    
    else if (!connected) {  //IF NOT CONNECTED TO WIFI, TRY AND CONNECT TO WIFI
      Serial.println("WiFi Disconnected.");
      connectToWiFi(networkName, networkPswd);
      sensorReading = 8888;
    }
  }
  delay(5);
}

void connectToWiFi(const char *ssid, const char *pwd) {
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);  // Will call WiFiEvent() from another thread.

  //Initiate connection
  WiFi.begin(ssid, pwd);
  WiFi.config(local_IP, gateway, subnet);

  Serial.println("Waiting for WIFI connection...");
}

// WARNING: WiFiEvent is called from a separate FreeRTOS task (thread)!
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      //When connected set
      //Serial.print("WiFi connected! IP address: ");
      //Serial.println(WiFi.localIP());
      //initializes the UDP state
      //This initializes the transfer buffer
      udp.begin(WiFi.localIP(), udpPort);
      connected = true;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      //Serial.println("WiFi lost connection");
      connected = false;
      break;
    default: break;
  }
}

void OSC_Trig1() {
  ledState = !ledState;             // SET ledState TO THE OPPOSITE OF ledState
  digitalWrite(LED_PIN, ledState);  // WRITE THE NEW ledState

  OSCMessage msg("Button up!");  //SEND THE OSC MESSAGE
  msg.add((int32_t)1);
  udp.beginPacket(udpAddress, udpPort);
  msg.send(udp);    // send the bytes to the SLIP stream
  udp.endPacket();  // mark the end of the OSC Packet
  msg.empty();      // free space occupied by message
  Serial.println("Button up!");
  lastState = 0;
}

void OSC_Trig2() {
  ledState = !ledState;             // SET ledState TO THE OPPOSITE OF ledState
  digitalWrite(LED_PIN, ledState);  // WRITE THE NEW ledState

  OSCMessage msg("/Mx/button/4271");  //WRITE DA OSC MESSAGE
  msg.add((int32_t)1);
  udp.beginPacket(udpAddress, udpPort);
  msg.send(udp);    // send the bytes to the SLIP stream
  udp.endPacket();  // mark the end of the OSC Packet
  msg.empty();      // free space occupied by message
  Serial.println("Button message sent!");
  lastState = 1;
}

/*
    Reads distance from TOF sensor. 
    If the distance is close:
    Make an OSC message and send it over UDP via Wi-Fi.
    Dom Chang - Future Colossal
 */

#include <WiFi.h>
#include <NetworkUdp.h>
#include <SPI.h>
#include <OSCMessage.h>
#include <Adafruit_NeoPixel.h>


// DEFINE THE PIN FOR THE LED :
#define LED_PIN LED_BUILTIN
#define NEO_PIN PIN_NEOPIXEL
#define BUTTON1_PIN 11
#define BUTTON2_PIN 9

Adafruit_NeoPixel pixels(1, NEO_PIN, NEO_GRB + NEO_KHZ800);

boolean ledState = LOW;
boolean BUTTON1_STATE = LOW;
boolean BUTTON2_STATE = LOW;
int lastPress = 0;

// WiFi network name and password:
const char *networkName = "FC_Orbi";
const char *networkPswd = "FC123456789office";

/*
IP address to send UDP data to:
either use the ip address of the server or a network broadcast 
*/

//const char *udpAddress = "192.168.1.36"; //DOM LAPTOP
const char *udpAddress = "192.168.1.35";  //MAIN LIGHT PC
const int udpPort = 8000;

//Are we currently connected?
boolean connected = false;
boolean SOX_POWERED_STATE = false;
boolean START_DEMO = false;

//The udp library class
NetworkUDP udp;

void setup() {
  // Initialize hardware serial:
  Serial.begin(115200);
  // while (!Serial) {
  //   delay(1);
  // };

  // LED SETUP
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  digitalWrite(LED_PIN, ledState);
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(50, 50, 50));
  pixels.show();
  delay(1000);
  pixels.clear();

  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);
}

void loop() {
  if (connected) {  //IF WE ARE CONNECT TO WIFI, CHECK FOR BUTTON PRESSES
    checkButtons();
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

  Serial.println("Waiting for WIFI connection...");
  pixels.setPixelColor(0, pixels.Color(255, 0, 255));
  pixels.show();
}

// WARNING: WiFiEvent is called from a separate FreeRTOS task (thread)!
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      //When connected set
      Serial.print("WiFi connected! IP address: ");
      Serial.println(WiFi.localIP());
      pixels.setPixelColor(0, pixels.Color(0, 255, 0));
      pixels.show();
      delay(1000);
      pixels.setPixelColor(0, pixels.Color(55, 55, 55));
      pixels.show();

      //initializes the UDP state
      //This initializes the transfer buffer
      udp.begin(WiFi.localIP(), udpPort);
      connected = true;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));
      pixels.show();

      connected = false;
      break;
    default: break;
  }
}

void checkButtons() {
  BUTTON1_STATE = digitalRead(BUTTON1_PIN);
  BUTTON2_STATE = digitalRead(BUTTON2_PIN);
  if (BUTTON1_STATE && lastPress != 1) {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    pixels.show();
    OSC_Trig1();
  } else if (BUTTON2_STATE && lastPress != 2) {
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show();
    OSC_Trig2();
  }
  // else {
  //   pixels.setPixelColor(0, pixels.Color(50, 50, 50));
  //   pixels.show();
  // }
}


void OSC_Trig1() {
  ledState = !ledState;             // SET ledState TO THE OPPOSITE OF ledState
  digitalWrite(LED_PIN, ledState);  // WRITE THE NEW ledState

  OSCMessage msg("/Mx/button/4261");  //SEND THE OSC MESSAGE
  msg.add((int32_t)1);
  udp.beginPacket(udpAddress, udpPort);
  msg.send(udp);    // send the bytes to the SLIP stream
  udp.endPacket();  // mark the end of the OSC Packet
  msg.empty();      // free space occupied by message
  Serial.println("Sodium On!");
  lastPress = 1;
}

void OSC_Trig2() {
  ledState = !ledState;             // SET ledState TO THE OPPOSITE OF ledState
  digitalWrite(LED_PIN, ledState);  // WRITE THE NEW ledState

  OSCMessage msg("/Mx/button/4261");  //SEND THE OSC MESSAGE
  msg.add((int32_t)0);
  udp.beginPacket(udpAddress, udpPort);
  msg.send(udp);    // send the bytes to the SLIP stream
  udp.endPacket();  // mark the end of the OSC Packet
  msg.empty();      // free space occupied by message
  Serial.println("Sodium Off!");
  lastPress = 2;
}

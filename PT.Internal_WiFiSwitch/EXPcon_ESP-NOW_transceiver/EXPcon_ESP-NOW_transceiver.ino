/*
Adapted from:
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp-now-auto-pairing-esp32-esp8266/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  Based on JC Servaye example: https://github.com/Servayejc/esp_now_web_server/
*/
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>

// Replace with your network credentials (STATION)
const char *ssid = "MyOptimum 32d5a1";                // Your WiFi SSID
const char *password = "9303-sepia-71";  // Your WiFi Password

esp_now_peer_info_t slave;
int chan;
boolean ledState = LOW;
boolean BUTTON1_STATE = LOW;
boolean BUTTON2_STATE = LOW;
int lastPress = 0;

enum MessageType { PAIRING,
                   DATA,
};
MessageType messageType;

int counter = 0;

uint8_t clientMacAddress[6];

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  uint8_t msgType;
  uint8_t id;
  unsigned int readingID;
  String message;
} struct_message;


typedef struct struct_pairing {  // new structure for pairing
  uint8_t msgType;
  uint8_t id;
  uint8_t macAddr[6];
  uint8_t channel;
} struct_pairing;

struct_message incomingReadings;
struct_message outgoingMessage;
struct_pairing pairingData;

// DEFINE THE PIN FOR THE LED :
#define LED_PIN LED_BUILTIN
#define BUTTON1_PIN 15
#define BUTTON2_PIN 14

void readMacAddress() {
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}

void readDataToSend(String s) {
  outgoingMessage.msgType = DATA;
  outgoingMessage.id = 0;
  outgoingMessage.message = s;
  outgoingMessage.readingID = counter++;
}

// ---------------------------- esp_ now -------------------------
void printMAC(const uint8_t *mac_addr) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
}

bool addPeer(const uint8_t *peer_addr) {  // add pairing
  memset(&slave, 0, sizeof(slave));
  const esp_now_peer_info_t *peer = &slave;
  memcpy(slave.peer_addr, peer_addr, 6);

  slave.channel = chan;  // pick a channel
  slave.encrypt = 0;     // no encryption
  // check if the peer exists
  bool exists = esp_now_is_peer_exist(slave.peer_addr);
  if (exists) {
    // Slave already paired.
    Serial.println("Already Paired");
    return true;
  } else {
    esp_err_t addStatus = esp_now_add_peer(peer);
    if (addStatus == ESP_OK) {
      // Pair success
      Serial.println("Pair success");
      return true;
    } else {
      Serial.println("Pair failed");
      return false;
    }
  }
}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success to " : "Delivery Fail to ");
  printMAC(mac_addr);
  Serial.println();
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  Serial.print(len);
  Serial.println(" bytes of new data received.");
  // StaticJsonDocument<1000> root;
  // String payload;
  uint8_t type = incomingData[0];  // first message byte is the type of message
  switch (type) {
    case DATA:  // the message is data type
      memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
      // create a JSON document with received data and send it by event to the web page
      // root["id"] = incomingReadings.id;
      // root["temperature"] = incomingReadings.temp;
      // root["humidity"] = incomingReadings.hum;
      // root["readingId"] = String(incomingReadings.readingId);
      Serial.println(incomingReadings.id);
      Serial.println(incomingReadings.readingID);
      Serial.println(incomingReadings.message);
      // serializeJson(root, payload);
      // Serial.print("event send :");
      // serializeJson(root, Serial);
      //events.send(payload.c_str(), "new_readings", millis());
      Serial.println();
      break;

    case PAIRING:  // the message is a pairing request
      memcpy(&pairingData, incomingData, sizeof(pairingData));
      Serial.println(pairingData.msgType);
      Serial.println(pairingData.id);
      Serial.print("Pairing request from MAC Address: ");
      printMAC(pairingData.macAddr);
      Serial.print(" on channel ");
      Serial.println(pairingData.channel);

      clientMacAddress[0] = pairingData.macAddr[0];
      clientMacAddress[1] = pairingData.macAddr[1];
      clientMacAddress[2] = pairingData.macAddr[2];
      clientMacAddress[3] = pairingData.macAddr[3];
      clientMacAddress[4] = pairingData.macAddr[4];
      clientMacAddress[5] = pairingData.macAddr[5];

      if (pairingData.id > 0) {  // do not replay to server itself
        if (pairingData.msgType == PAIRING) {
          pairingData.id = 0;  // 0 is server
          // Server is in AP_STA mode: peers need to send data to server soft AP MAC address
          WiFi.softAPmacAddress(pairingData.macAddr);
          Serial.print("Pairing MAC Address: ");
          printMAC(clientMacAddress);
          pairingData.channel = chan;
          Serial.println(" send response");
          esp_err_t result = esp_now_send(clientMacAddress, (uint8_t *)&pairingData, sizeof(pairingData));
          addPeer(clientMacAddress);
        }
      }
      break;
  }
}

void initESP_NOW() {
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.STA.begin();
  Serial.print("Server MAC Address: ");
  readMacAddress();

  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);
  // Set device as a Wi-Fi Station
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }

  Serial.print("Server SOFT AP MAC Address:  ");
  Serial.println(WiFi.softAPmacAddress());

  chan = WiFi.channel();
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  initESP_NOW();
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  digitalWrite(LED_PIN, ledState);

}

void checkButtons() {
  BUTTON1_STATE = digitalRead(BUTTON1_PIN);
  BUTTON2_STATE = digitalRead(BUTTON2_PIN);
  if (BUTTON1_STATE && lastPress != 1) {
    readDataToSend("ON");
    esp_now_send(NULL, (uint8_t *)&outgoingMessage, sizeof(outgoingMessage));
    Serial.println("ON");
    ledState = HIGH;
    digitalWrite(LED_PIN, ledState);  // WRITE THE NEW ledState
    lastPress=1;

  } else if (BUTTON2_STATE && lastPress != 2) {
    readDataToSend("OFF");
    esp_now_send(NULL, (uint8_t *)&outgoingMessage, sizeof(outgoingMessage));
    Serial.println("OFF");
    ledState = LOW;
    digitalWrite(LED_PIN, ledState);  // WRITE THE NEW ledState
    lastPress=2;
  }
  // else {
  //   pixels.setPixelColor(0, pixels.Color(50, 50, 50));
  //   pixels.show();
  // }
}

void loop() {
  checkButtons();
}
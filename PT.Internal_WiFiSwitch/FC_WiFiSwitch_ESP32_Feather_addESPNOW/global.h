
#include <WiFi.h>
//#include <ArduinoJson.h>
#include <SPI.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>
#include <Adafruit_NeoPixel.h>
#include <elapsedMillis.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerialLite.h>
//ESP-NOW Stuff
#include <Arduino.h> 
#include <esp_now.h>  
#include <esp_wifi.h> 
#include <EEPROM.h>   //to store Wi-Fi channel for future pairing


WiFiClient client;
AsyncWebServer httpServer(80);

//this enum needed to be global due to how the compiler works when using enum to declare a function return type
enum PairingStatus { NOT_PAIRED,
                     PAIR_REQUEST,
                     PAIR_REQUESTED,
                     PAIR_PAIRED,
};

enum MessageType { PAIRING,
                   DATA,
};

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
void TaskTCPConnection(void* pvParameters);

//These bools indicate whether that communication protocol has been successfully connected and is currently active. Initialize all to not connecte
bool TCP = false;
bool RF = false;
bool WIFI = false;
bool OTA = false;
bool ESP_NOW = false;
unsigned long lastMsgTime = 0;
unsigned long lastWifiConnectionTime = 0;
String lastMsg = "";


#include <WiFi.h>
//#include <ArduinoJson.h>
#include <SPI.h>
#include <Ra  dioHead.h>
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




WiFiClient client;
AsyncWebServer httpServer(80);

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
void TaskTCPConnection(void* pvParameters);

bool TCP = false;
bool RF = false;
bool WIFI = false;
bool OTA = false;
unsigned long lastMsgTime = 0;
unsigned long lastWifiConnectionTime = 0;
String lastMsg = "";






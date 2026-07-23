#pragma once

// First: config so feature flags (ENABLE_WEBSERIAL, ENABLE_OTA, etc.) exist
#include "config.h"

// ========================
//  Core Arduino / ESP32
// ========================
#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// ========================
//  Optional: Web / WebSerial
// ========================
#if ENABLE_WEBSERIAL
  #include <AsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include <WebSerialLite.h>
#endif

// ========================
//  Optional: ESP-NOW
// ========================
#if ENABLE_ESPNOW
  #include "FClib_ESP_NOW_client.h"
#endif

// ========================
//  SuperCircuit hardware
// ========================
#include "superCircuitGlobal.h"

// ========================
//  EXPERIENCE HOOKS
//  (implemented in ESP32_SuperCircuit_Template.ino)
// ========================
void messageReceived(const String& msg);
void button1Pressed();
void button2Pressed();
void button3Pressed();

// ========================
//  COMMS GLOBALS
// ========================
extern bool gSystemEnabled;
extern bool gWifiConnected;
extern bool gTcpConnected;

extern WiFiServer gTcpServer;
extern WiFiClient gTcpClient;

#if ENABLE_WEBSERIAL
  extern AsyncWebServer httpServer;
#endif

// ========================
//  COMMS API
// ========================
void commsSetup();    // call in setup()
void receiveComms();  // call in loop()

// Logging & TCP send helpers
void logLine(const String& line);
void tcpPrint(const String& s);
void tcpPrintln(const String& s);

// Lower-level helpers provided in other .ino files
void wifiSetup();
void webSerialSetup();
void receiveSerial();

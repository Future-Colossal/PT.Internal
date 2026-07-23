#include "global.h"

// Internal helpers
static void wifiConnect();
static void otaSetup();

void wifiSetup() {
  WiFi.mode(WIFI_STA);
  wifiConnect();

#if ENABLE_OTA
  otaSetup();
#endif
}

// ========================
//  WIFI CONNECT
// ========================
static void wifiConnect() {
#if WIFI_PROFILE == 1
  IPAddress ip(WIFI_IP_1);
  IPAddress gw(WIFI_GW_1);
  IPAddress sn(WIFI_SUBNET_1);
  WiFi.config(ip, gw, sn);
  WiFi.begin(WIFI_SSID_1, WIFI_PASS_1);
#elif WIFI_PROFILE == 2
  IPAddress ip(WIFI_IP_2);
  IPAddress gw(WIFI_GW_2);
  IPAddress sn(WIFI_SUBNET_2);
  WiFi.config(ip, gw, sn);
  WiFi.begin(WIFI_SSID_2, WIFI_PASS_2);
#endif

  logLine(F("Connecting to WiFi..."));

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(200);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    logLine("WiFi connected: " + WiFi.localIP().toString());
    setWifiStatus(true, WiFi.localIP());
  } else {
    logLine(F("WiFi failed to connect."));
    setWifiStatus(false, IPAddress());
  }
}

// ========================
//  OTA SETUP
// ========================
static void otaSetup() {
  ArduinoOTA.setHostname(DEVICE_NAME);

  ArduinoOTA.onStart([]() {
    logLine(F("OTA: Start"));
  });
  ArduinoOTA.onEnd([]() {
    logLine(F("OTA: End"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    // Avoid spamming serial; uncomment if needed
    // Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    String msg = "OTA Error: " + String(error);
    logLine(msg);
  });

  ArduinoOTA.begin();
  logLine(F("OTA ready"));
}

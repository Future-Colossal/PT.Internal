#include "global.h"

// Definitions for globals declared in global.h
WiFiServer gTcpServer(TCP_PORT);
WiFiClient gTcpClient;

#if ENABLE_WEBSERIAL
AsyncWebServer httpServer(HTTP_PORT);
#endif

// Internal
static String gTcpRxBuffer;

// Forward decls for local functions
static void wifiLoop();
static void tcpLoop();


// ========================
//  LOGGING & TCP HELPERS
// ========================
void logLine(const String& line) {
  Serial.println(line);
#if ENABLE_WEBSERIAL
  WebSerial.println(line);
#endif
}

void tcpPrint(const String& s) {
  if (gTcpConnected && gTcpClient.connected()) {
    gTcpClient.print(s);
  }
}

void tcpPrintln(const String& s) {
  tcpPrint(s + "\n");
}

// ========================
//  COMMS LIFECYCLE
// ========================
void commsSetup() {
  wifiSetup();

  gTcpServer.begin();
  gTcpServer.setNoDelay(true);
  logLine("TCP server listening on port " + String(TCP_PORT));

#if ENABLE_WEBSERIAL
  webSerialSetup();
#endif
}

// Called every loop()
void receiveComms() {
#if ENABLE_OTA
  ArduinoOTA.handle();
#endif

  receiveSerial();
  wifiLoop();
  tcpLoop();
}

// ========================
//  WIFI LOOP (RECONNECT)
// ========================
static void wifiLoop() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < 2000) return;  // check every ~2 seconds
  lastCheck = millis();

  wl_status_t status = WiFi.status();

  if (status != WL_CONNECTED) {
    if (gWifiConnected) {
      // We just lost WiFi
      gWifiConnected = false;
      setWifiStatus(false, IPAddress());
      logLine("WiFi lost; will attempt reconnect in background");
    }

    logLine("WiFi reconnect attempt...");
    WiFi.reconnect();   // non-blocking; just kicks off reconnect
  } else {
    if (!gWifiConnected) {
      // We just regained WiFi
      gWifiConnected = true;
      logLine("WiFi reconnected: " + WiFi.localIP().toString());
    }
    setWifiStatus(true, WiFi.localIP());
  }
}

// ========================
//  TCP LOOP
// ========================
static void tcpLoop() {
  // Accept new client if needed
  if (!gTcpClient || !gTcpClient.connected()) {
    WiFiClient newClient = gTcpServer.available();
    if (newClient) {
      gTcpClient = newClient;
      gTcpConnected = true;
      setTcpStatus(true);
      logLine(F("TCP client connected"));
      gTcpRxBuffer = "";
    } else {
      if (gTcpConnected) {
        logLine(F("TCP client disconnected"));
      }
      gTcpConnected = false;
      setTcpStatus(false);
      return;
    }
  }

  // Read data
  while (gTcpClient.available()) {
    char c = gTcpClient.read();
    if (c == '\n' || c == '\r') {
      if (gTcpRxBuffer.length()) {
        String msg = gTcpRxBuffer;
        gTcpRxBuffer = "";
        msg.trim();
        if (msg.length()) {
          messageReceived(msg);
        }
      }
    } else {
      gTcpRxBuffer += c;
    }
  }
}



#include "global.h"

// ========================
//  WEBSERIAL SETUP
// ========================
void webSerialSetup() {
#if ENABLE_WEBSERIAL
  WebSerial.begin(&httpServer);

  WebSerial.onMessage([](uint8_t* data, size_t len) {
    String d;
    d.reserve(len);
    for (size_t i = 0; i < len; i++) {
      d += char(data[i]);
    }
    d.trim();
    if (d.length()) {
      messageReceived(d);
    }
  });

  httpServer.begin();
  logLine(F("WebSerial ready"));
#endif
}

// ========================
//  USB SERIAL -> MESSAGE
// ========================
void receiveSerial() {
  while (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length()) {
      messageReceived(line);
    }
  }
}


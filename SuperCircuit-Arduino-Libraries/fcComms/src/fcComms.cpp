#include "fcComms.h"

#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerialLite.h>

namespace {
  fcComms* gActiveInstance = nullptr;
  constexpr const char* kFcCommsBuildTag = "fcComms build 2026-04-22a";
}

class FcCommsImpl {
public:
  WiFiServer* tcpServer = nullptr;
  WiFiClient tcpClient;
  WiFiUDP udp;
  String tcpRxBuffer;
  AsyncWebServer* httpServer = nullptr;
};

fcComms::fcComms() = default;

bool fcComms::begin(const FcCommsConfig& config) {
  _config = config;
  _begun = true;
  _wifiConnected = false;
  _tcpConnected = false;
  _webSerialReady = false;
  _localIp = IPAddress();
  gActiveInstance = this;

  if (!_impl) {
    _impl = new FcCommsImpl();
  }

  // Loud startup banner so we can confirm the installed library really updated.
  printlnSerial(String("[fcComms] ") + kFcCommsBuildTag);
  printlnSerial("[fcComms] begin()");
  printlnSerial(String("[fcComms] device=") + _config.deviceName + " id=" + String(_config.deviceId));
  printlnSerial(String("[fcComms] flags wifi=") + (_config.enableWifi ? "1" : "0") +
                " tcp=" + (_config.enableTcp ? "1" : "0") +
                " udp=" + (_config.enableUdp ? "1" : "0") +
                " web=" + (_config.enableWebSerial ? "1" : "0") +
                " serial=" + (_config.enableSerial ? "1" : "0") +
                " ota=" + (_config.enableOta ? "1" : "0"));
  printlnSerial("[fcComms] compile webserial=1");

  if (_config.enableWifi) {
    WiFi.mode(WIFI_STA);
    WiFi.config(_config.wifiProfile.ip, _config.wifiProfile.gateway, _config.wifiProfile.subnet);
    WiFi.begin(_config.wifiProfile.ssid, _config.wifiProfile.password);
    printlnSerial("Connecting to WiFi...");

    if (WiFi.status() == WL_CONNECTED) {
      _wifiConnected = true;
      _localIp = WiFi.localIP();
      printlnAll("WiFi connected: " + _localIp.toString());
    }
  }

  if (_config.enableTcp) {
    _impl->tcpServer = new WiFiServer(_config.tcpPort);
    _impl->tcpServer->begin();
    _impl->tcpServer->setNoDelay(true);
    printlnAll("TCP server listening on port " + String(_config.tcpPort));
  }

  if (_config.enableUdp && _config.enableWifi) {
    if (_impl->udp.begin(_config.udpPort)) {
      printlnAll("UDP listening on port " + String(_config.udpPort));
    } else {
      printlnAll("UDP begin failed");
    }
  }

  if (_config.enableWebSerial) {
    printlnSerial(String("[fcComms] WebSerial init requested on http port ") + String(_config.httpPort));
    _impl->httpServer = new AsyncWebServer(_config.httpPort);
    WebSerial.begin(_impl->httpServer);
    WebSerial.onMessage([](uint8_t* data, size_t len) {
      if (!gActiveInstance) return;
      String d;
      d.reserve(len);
      for (size_t i = 0; i < len; i++) d += char(data[i]);
      d.trim();
      if (d.length()) {
        gActiveInstance->dispatchMessage(d, FcCommSource::WebSerial);
      }
    });
    _impl->httpServer->begin();
    _webSerialReady = true;
    printlnSerial("[fcComms] WebSerial init OK");
    printlnAll("WebSerial ready");
  } else {
    printlnSerial("[fcComms] WebSerial disabled in config");
  }

  if (_config.enableOta && _config.enableWifi) {
    ArduinoOTA.setHostname(_config.deviceName);
    ArduinoOTA.onStart([this]() {
      printlnAll("OTA: Start");
    });
    ArduinoOTA.onEnd([this]() {
      printlnAll("OTA: End");
    });
    ArduinoOTA.onError([this](ota_error_t error) {
      printlnAll("OTA Error: " + String(error));
    });
    ArduinoOTA.begin();
    printlnAll("OTA ready");
  }

  printlnSerial(String("[fcComms] begin complete webReady=") + (_webSerialReady ? "1" : "0"));

  return true;
}

void fcComms::update() {
  if (!_begun) return;

  if (_config.enableOta && _config.enableWifi) {
    ArduinoOTA.handle();
  }

  if (_config.enableSerial) {
    while (Serial.available()) {
      String line = Serial.readStringUntil('\n');
      line.trim();
      if (line.length()) {
        dispatchMessage(line, FcCommSource::Serial);
      }
    }
  }

  if (_config.enableWifi) {
    static unsigned long lastWifiCheck = 0;
    if (millis() - lastWifiCheck >= 2000) {
      lastWifiCheck = millis();

      wl_status_t status = WiFi.status();
      if (status != WL_CONNECTED) {
        if (_wifiConnected) {
          _wifiConnected = false;
          _localIp = IPAddress();
          printlnAll("WiFi lost; reconnecting");
        }
        WiFi.reconnect();
      } else {
        if (!_wifiConnected) {
          _wifiConnected = true;
          _localIp = WiFi.localIP();
          printlnAll("WiFi reconnected: " + _localIp.toString());
        }
        _localIp = WiFi.localIP();
      }
    }
  }

  if (_config.enableTcp && _impl && _impl->tcpServer) {
    if (!_impl->tcpClient || !_impl->tcpClient.connected()) {
      WiFiClient newClient = _impl->tcpServer->available();
      if (newClient) {
        _impl->tcpClient = newClient;
        _tcpConnected = true;
        _impl->tcpRxBuffer = "";
        printlnAll("TCP client connected");
      } else {
        if (_tcpConnected) {
          printlnAll("TCP client disconnected");
        }
        _tcpConnected = false;
      }
    }

    while (_tcpConnected && _impl->tcpClient.available()) {
      char c = _impl->tcpClient.read();
      if (c == '\n' || c == '\r') {
        if (_impl->tcpRxBuffer.length()) {
          String msg = _impl->tcpRxBuffer;
          _impl->tcpRxBuffer = "";
          msg.trim();
          if (msg.length()) {
            dispatchMessage(msg, FcCommSource::Tcp);
          }
        }
      } else {
        _impl->tcpRxBuffer += c;
      }
    }
  }

  if (_config.enableUdp && _config.enableWifi && _impl) {
    int packetSize = _impl->udp.parsePacket();
    while (packetSize > 0) {
      String msg;
      while (_impl->udp.available()) {
        msg += char(_impl->udp.read());
      }
      msg.trim();
      if (msg.length()) {
        FcCommSource source = (_config.cluster.enabled && msg.startsWith("CLUSTER_"))
          ? FcCommSource::Cluster
          : FcCommSource::Udp;
        dispatchMessage(msg, source);
      }
      packetSize = _impl->udp.parsePacket();
    }
  }
}

void fcComms::onMessage(FcMessageHandler handler) {
  _messageHandler = handler;
}

void fcComms::printAll(const String& msg) {
  printSerial(msg);
  printTcp(msg);
  sendUdp(msg);
  printlnWebSerial(msg);
}

void fcComms::printlnAll(const String& msg) {
  printlnSerial(msg);
  printlnTcp(msg);
  sendUdp(msg);
  printlnWebSerial(msg);
}

void fcComms::printTcp(const String& msg) {
  if (!_tcpConnected || !_impl) return;
  if (_impl->tcpClient && _impl->tcpClient.connected()) {
    _impl->tcpClient.print(msg);
  }
}

void fcComms::printlnTcp(const String& msg) {
  printTcp(msg + "\n");
}

void fcComms::sendUdp(const String& msg) {
  if (!_config.enableUdp || !_config.enableWifi || !_wifiConnected || !_impl) return;

  IPAddress target = _config.cluster.enabled
    ? IPAddress(255, 255, 255, 255)
    : _config.udpDestination;

  _impl->udp.beginPacket(target, _config.udpPort);
  _impl->udp.print(msg);
  _impl->udp.endPacket();
}

void fcComms::printSerial(const String& msg) {
  if (!_config.enableSerial) return;
  Serial.print(msg);
}

void fcComms::printlnSerial(const String& msg) {
  printSerial(msg + "\n");
}

void fcComms::printlnWebSerial(const String& msg) {
  if (_config.enableWebSerial && _webSerialReady) {
    WebSerial.println(msg);
  }
}

void fcComms::restart() {
  ESP.restart();
}

uint16_t fcComms::deviceId() const {
  return _config.deviceId;
}

const char* fcComms::deviceName() const {
  return _config.deviceName;
}

const char* fcComms::deviceTitle() const {
  return _config.deviceTitle;
}

bool fcComms::isWifiConnected() const {
  return _wifiConnected;
}

bool fcComms::isTcpConnected() const {
  return _tcpConnected;
}

IPAddress fcComms::localIP() const {
  return _localIp;
}

void fcComms::dispatchMessage(const String& msg, FcCommSource source) {
  if (_messageHandler) {
    _messageHandler(msg, source);
  }
}

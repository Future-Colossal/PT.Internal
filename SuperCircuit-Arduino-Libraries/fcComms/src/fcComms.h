#pragma once

#include <Arduino.h>
#include <IPAddress.h>

enum class FcCommSource : uint8_t {
  Serial,
  Tcp,
  Udp,
  WebSerial,
  Cluster,
  EspNow
};

struct FcWifiProfile {
  const char* ssid = "";
  const char* password = "";
  IPAddress ip;
  IPAddress gateway;
  IPAddress subnet;
};

struct FcClusterConfig {
  bool enabled = false;
  uint16_t udpPort = 9001;
  uint16_t code = 0;
  uint8_t size = 0;
  uint16_t leaderDeviceId = 0;
};

struct FcCommsConfig {
  uint16_t deviceId = 0;
  const char* deviceName = "fc-device";
  const char* deviceTitle = "fc-device";
  bool enableWifi = true;
  bool enableTcp = true;
  bool enableUdp = true;
  bool enableWebSerial = false;
  bool enableSerial = true;
  bool enableOta = true;
  uint16_t tcpPort = 8888;
  uint16_t udpPort = 9001;
  uint16_t httpPort = 80;
  FcWifiProfile wifiProfile;
  FcClusterConfig cluster;
  IPAddress udpDestination = IPAddress(255, 255, 255, 255);
};

using FcMessageHandler = void (*)(const String& msg, FcCommSource source);

class fcComms {
public:
  fcComms();

  bool begin(const FcCommsConfig& config);
  void update();

  void onMessage(FcMessageHandler handler);

  void printAll(const String& msg);
  void printlnAll(const String& msg);
  void printTcp(const String& msg);
  void printlnTcp(const String& msg);
  void sendUdp(const String& msg);
  void printSerial(const String& msg);
  void printlnSerial(const String& msg);
  void printlnWebSerial(const String& msg);
  void restart();

  uint16_t deviceId() const;
  const char* deviceName() const;
  const char* deviceTitle() const;
  bool isWifiConnected() const;
  bool isTcpConnected() const;
  IPAddress localIP() const;

private:
  void dispatchMessage(const String& msg, FcCommSource source);

  class FcCommsImpl* _impl = nullptr;
  FcCommsConfig _config;
  FcMessageHandler _messageHandler = nullptr;
  bool _begun = false;
  bool _wifiConnected = false;
  bool _tcpConnected = false;
  bool _webSerialReady = false;
  IPAddress _localIp;
};

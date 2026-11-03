#pragma once

#include <Arduino.h>
#include <IPAddress.h>
#include <stdint.h>
#include "SuperCircuitBoard.h"

enum class SuperCircuitToggle : uint8_t {
  Disabled = 0,
  Enabled = 1
};

enum class SuperCircuitStartupSpeed : uint8_t {
  Fast = 0,
  Slow = 1,
  Custom = 2
};

enum class SuperCircuitAudioMode : uint8_t {
  Single = 0,
  Loop = 1
};

struct SuperCircuitFeatureConfig {
  SuperCircuitToggle dfPlayer = SuperCircuitToggle::Enabled;
  SuperCircuitToggle screen = SuperCircuitToggle::Enabled;
  SuperCircuitToggle pixels = SuperCircuitToggle::Enabled;
  SuperCircuitToggle volumePot = SuperCircuitToggle::Enabled;
};

struct SuperCircuitDisplayConfig {
  SuperCircuitToggle allowAlerts = SuperCircuitToggle::Enabled;
  SuperCircuitToggle showUptimeWhenIdle = SuperCircuitToggle::Enabled;
  uint16_t refreshIntervalMs = 50;
  uint16_t defaultAlertDurationMs = 1000;
};

struct SuperCircuitTextConfig {
  const char* interactiveOnLine = "Interactive ON";
  const char* interactiveOffLine = "Interactive OFF";
  const char* turnOnAlertTitle = "SYS";
  const char* turnOnAlertBody = "Enabled";
  const char* turnOffAlertTitle = "SYS";
  const char* turnOffAlertBody = "Disabled";
  const char* readyAlertTitle = "READY";
  const char* readyAlertBody = "SuperCircuit ready";
  const char* wifiPendingLine = "WiFi: pending";
  const char* wifiDisconnectedLine = "WiFi: not connected";
  const char* audioTrackPrefix = "Track ";
};

struct SuperCircuitPixelConfig {
  uint8_t externalBrightness = 20;
  uint8_t builtInBrightness = 10;
  uint32_t interactiveOnColor = 0x002800;
  uint32_t interactiveOffColor = 0x280000;
  uint32_t commsOfflineColor = 0x281400;
  uint32_t commsWifiOnlyColor = 0x000028;
  uint32_t commsTcpReadyColor = 0x002828;
  uint32_t userDefaultColor = 0x000000;
};

struct SuperCircuitAudioConfig {
  uint8_t defaultVolume = 15;
  uint8_t minVolume = 0;
  uint8_t maxVolume = 30;
  SuperCircuitToggle followVolumePot = SuperCircuitToggle::Enabled;
  SuperCircuitToggle amplifierEnabled = SuperCircuitToggle::Enabled;
  SuperCircuitAudioMode defaultMode = SuperCircuitAudioMode::Single;
};

struct SuperCircuitStartupConfig {
  SuperCircuitToggle runSequence = SuperCircuitToggle::Enabled;
  SuperCircuitToggle ledRainbow = SuperCircuitToggle::Disabled;
  SuperCircuitToggle screenSequence = SuperCircuitToggle::Enabled;
  SuperCircuitToggle showDisplayCheck = SuperCircuitToggle::Enabled;
  SuperCircuitToggle showPixelCheck = SuperCircuitToggle::Enabled;
  SuperCircuitToggle showButtonCheck = SuperCircuitToggle::Enabled;
  SuperCircuitToggle showAudioCheck = SuperCircuitToggle::Enabled;
  SuperCircuitToggle showWifiPending = SuperCircuitToggle::Enabled;
  SuperCircuitToggle showReadyAlert = SuperCircuitToggle::Enabled;
  SuperCircuitStartupSpeed speed = SuperCircuitStartupSpeed::Fast;
  uint16_t customStepDurationMs = 0;
};

struct SuperCircuitConfig {
  const char* defaultTitle = "SuperCircuit";
  const char* defaultBottomLine = "";
  bool defaultInteractiveEnabled = true;
  SuperCircuitFeatureConfig features;
  SuperCircuitDisplayConfig display;
  SuperCircuitTextConfig text;
  SuperCircuitPixelConfig pixel;
  SuperCircuitAudioConfig audio;
  SuperCircuitStartupConfig startup;
};

using SuperCircuitButtonHandler = void (*)(uint8_t buttonNumber);
using SuperCircuitStateHandler = void (*)();

class SuperCircuit {
private:
  class SuperCircuitImpl;

public:
  SuperCircuit();

  bool begin(const SuperCircuitConfig& config);
  void update();

  void onButtonPressed(SuperCircuitButtonHandler handler);
  void onTurnOn(SuperCircuitStateHandler handler);
  void onTurnOff(SuperCircuitStateHandler handler);

  void setEnabled(bool enabled);
  void turnOn();
  void turnOff();
  bool isOn() const;

  void playTrack(uint16_t track);
  void stopTrack();
  void setVolume(uint8_t volume);
  uint8_t volume() const;
  bool isAudioReady() const;
  bool isTrackPlaying() const;
  uint16_t lastTrack() const;
  String currentTrackName() const;
  void setAudioModeSingle();
  void setAudioModeLoop();
  void enableAmplifier();
  void disableAmplifier();

  void showAlert(const String& title, const String& body, uint32_t durationMs = 0);
  void clearAlert();
  void setTitle(const String& title);
  String title() const;
  void setBottomLine(const String& line);
  void clearBottomLine();
  String bottomLine() const;
  String wifiLine() const;
  void setInteractivePalette(uint32_t onColor, uint32_t offColor);
  void setCommsPalette(uint32_t offlineColor, uint32_t wifiOnlyColor, uint32_t tcpReadyColor);
  void setPixelBrightness(uint8_t externalBrightness, uint8_t builtInBrightness);
  void setWifiStatus(bool connected, const IPAddress& ip);
  void setWifiStatus(bool connected, const String& message);
  void setWifiStatusMessage(const String& message);
  void clearWifiStatusMessage();
  void setTcpConnected(bool connected);
  void setCommsStatus(bool wifiConnected, bool tcpConnected);
  bool isWifiConnected() const;
  bool isTcpConnected() const;

  void setPixelLaneColor(uint8_t lane, uint32_t color);
  void setUserPixelColor(uint32_t color);
  void setUserPixelRgb(uint8_t red, uint8_t green, uint8_t blue);
  uint32_t userPixelColor() const;

  bool isDisplayReady() const;
  bool arePixelsReady() const;
  bool areButtonsReady() const;

  void applyPixels();
  void drawScreen();
  void showStartupLine(const String& line, bool finalStatusKnown = false, bool ok = false);
  void runStartupSequence();
  void syncWifiLine();
  String buildOverlayLine() const;
  void drawScrollingLine(uint8_t y, const String& line, unsigned long elapsedMs);
  int16_t textPixelWidth(const String& text);
  uint16_t startupStepDurationMs() const;
  uint16_t alertDurationOrDefault(uint32_t durationMs) const;
  String buildUptimeLine() const;
  bool featureEnabled(SuperCircuitToggle toggle) const;
  uint32_t packRgb(uint8_t red, uint8_t green, uint8_t blue) const;

  SuperCircuitImpl* _impl = nullptr;
  SuperCircuitConfig _config;
  bool _begun = false;
  bool _isOn = false;
  bool _wifiConnected = false;
  bool _tcpConnected = false;
  uint32_t _userPixelColor = 0;
  String _title;
  String _bottomLine;
  String _wifiLine = "WiFi: offline";
  String _wifiStatusMessage;
  IPAddress _wifiIp;

  SuperCircuitButtonHandler _buttonHandler = nullptr;
  SuperCircuitStateHandler _turnOnHandler = nullptr;
  SuperCircuitStateHandler _turnOffHandler = nullptr;
};

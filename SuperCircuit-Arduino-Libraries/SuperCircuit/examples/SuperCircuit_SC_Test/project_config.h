#pragma once

#include <IPAddress.h>
#include <fcComms.h>
#include <SuperCircuit.h>

// ========================
//  DEVICE IDENTITY
// ========================
static const uint16_t kDeviceId = 251;
static const char* kDeviceName = "FC SuperCircuit Test";
static const char* kDeviceTitle = "SuperCircuit Test 251";

// ========================
//  COMMS BACKEND
// ========================
static const FcCommsConfig kCommsConfig = {
  kDeviceId,
  kDeviceName,
  kDeviceTitle,
  true,
  true,
  true,
  true,
  true,
  true,
  8888,
  9001,
  80,
  {
    "Verizon_3WRZKF",
    "cab-void9-saga",
    IPAddress(192, 168, 1, kDeviceId),
    IPAddress(192, 168, 1, 1),
    IPAddress(255, 255, 255, 0)
  },
  {
    false,
    9001,
    0,
    0,
    0
  }
};

// ========================
//  SUPERCIRCUIT CONFIG
// ========================
// These are the main board-shell settings a technologist is most likely to tune.
// Keep these values obvious here instead of burying them inside library internals.

// ------------------------
//  DISPLAY / SHELL TEXT
// ------------------------
static const char* kDefaultBottomLine = "SuperCircuit boot";
static const char* kInteractiveOnLine = "Interactive ON";
static const char* kInteractiveOffLine = "Interactive OFF";
static const char* kTurnOnAlertTitle = "SYS";
static const char* kTurnOnAlertBody = "Enabled";
static const char* kTurnOffAlertTitle = "SYS";
static const char* kTurnOffAlertBody = "Disabled";
static const char* kReadyAlertTitle = "READY";
static const char* kReadyAlertBody = "SuperCircuit ready";
static const char* kWifiPendingLine = "WiFi: pending";
static const char* kWifiDisconnectedLine = "WiFi: not connected";
static const char* kAudioTrackPrefix = "Track ";

// ------------------------
//  PIXEL PALETTE
// ------------------------
static const uint8_t kExternalPixelBrightness = 20;
static const uint8_t kBuiltInPixelBrightness = 10;
static const uint32_t kInteractiveOnColor = 0x002800;
static const uint32_t kInteractiveOffColor = 0x280000;
static const uint32_t kCommsOfflineColor = 0x281400;
static const uint32_t kCommsWifiOnlyColor = 0x000028;
static const uint32_t kCommsTcpReadyColor = 0x002828;
static const uint32_t kUserPixelDefaultColor = 0x001020;

// ------------------------
//  AUDIO DEFAULTS
// ------------------------
static const uint8_t kDefaultVolume = 18;
static const uint8_t kMinVolume = 0;
static const uint8_t kMaxVolume = 30;

// ------------------------
//  STARTUP POLICY
// ------------------------
static const uint16_t kDisplayRefreshMs = 50;
static const uint16_t kDefaultAlertMs = 1000;

static const SuperCircuitConfig kSuperCircuitConfig = [] {
  SuperCircuitConfig config;

  config.defaultTitle = kDeviceTitle;
  config.defaultBottomLine = kDefaultBottomLine;
  config.defaultInteractiveEnabled = true;

  config.features.dfPlayer = SuperCircuitToggle::Enabled;
  config.features.screen = SuperCircuitToggle::Enabled;
  config.features.pixels = SuperCircuitToggle::Enabled;
  config.features.volumePot = SuperCircuitToggle::Enabled;

  config.display.allowAlerts = SuperCircuitToggle::Enabled;
  config.display.showUptimeWhenIdle = SuperCircuitToggle::Enabled;
  config.display.refreshIntervalMs = kDisplayRefreshMs;
  config.display.defaultAlertDurationMs = kDefaultAlertMs;

  config.text.interactiveOnLine = kInteractiveOnLine;
  config.text.interactiveOffLine = kInteractiveOffLine;
  config.text.turnOnAlertTitle = kTurnOnAlertTitle;
  config.text.turnOnAlertBody = kTurnOnAlertBody;
  config.text.turnOffAlertTitle = kTurnOffAlertTitle;
  config.text.turnOffAlertBody = kTurnOffAlertBody;
  config.text.readyAlertTitle = kReadyAlertTitle;
  config.text.readyAlertBody = kReadyAlertBody;
  config.text.wifiPendingLine = kWifiPendingLine;
  config.text.wifiDisconnectedLine = kWifiDisconnectedLine;
  config.text.audioTrackPrefix = kAudioTrackPrefix;

  config.pixel.externalBrightness = kExternalPixelBrightness;
  config.pixel.builtInBrightness = kBuiltInPixelBrightness;
  config.pixel.interactiveOnColor = kInteractiveOnColor;
  config.pixel.interactiveOffColor = kInteractiveOffColor;
  config.pixel.commsOfflineColor = kCommsOfflineColor;
  config.pixel.commsWifiOnlyColor = kCommsWifiOnlyColor;
  config.pixel.commsTcpReadyColor = kCommsTcpReadyColor;
  config.pixel.userDefaultColor = kUserPixelDefaultColor;

  config.audio.defaultVolume = kDefaultVolume;
  config.audio.minVolume = kMinVolume;
  config.audio.maxVolume = kMaxVolume;
  config.audio.followVolumePot = SuperCircuitToggle::Enabled;
  config.audio.amplifierEnabled = SuperCircuitToggle::Enabled;
  config.audio.defaultMode = SuperCircuitAudioMode::Single;

  config.startup.runSequence = SuperCircuitToggle::Enabled;
  config.startup.ledRainbow = SuperCircuitToggle::Disabled;
  config.startup.screenSequence = SuperCircuitToggle::Enabled;
  config.startup.showDisplayCheck = SuperCircuitToggle::Enabled;
  config.startup.showPixelCheck = SuperCircuitToggle::Enabled;
  config.startup.showButtonCheck = SuperCircuitToggle::Enabled;
  config.startup.showAudioCheck = SuperCircuitToggle::Enabled;
  config.startup.showWifiPending = SuperCircuitToggle::Enabled;
  config.startup.showReadyAlert = SuperCircuitToggle::Enabled;
  config.startup.speed = SuperCircuitStartupSpeed::Fast;
  config.startup.customStepDurationMs = 0;

  return config;
}();

// ========================
//  TEST DEFAULTS
// ========================
static const uint16_t kDefaultTrack = 1;
static const uint32_t kUserPixelReady = 0x001020;
static const uint32_t kUserPixelOn = 0x001000;
static const uint32_t kUserPixelOff = 0x100000;

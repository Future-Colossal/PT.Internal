#pragma once

#include <IPAddress.h>
#include <fcComms.h>
#include <SuperCircuit.h>

constexpr bool stringsEqual(const char* left, const char* right) {
  while (*left && *right) {
    if (*left != *right) return false;
    ++left;
    ++right;
  }
  return *left == *right;
}

// ========================
//  WIFI / NETWORK - EDIT THESE FIRST
// ========================
static const uint16_t kDeviceId = 241;
static const char* kDeviceName = "SuperCircuit Flex Example";
static const char* kDeviceTitle = "Flex Example 241";

static constexpr char WIFI_SSID[] = "YOUR_WIFI_SSID";
static constexpr char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";
static const IPAddress WIFI_IP(192, 168, 1, kDeviceId);
static const IPAddress WIFI_GATEWAY(192, 168, 1, 1);
static const IPAddress WIFI_SUBNET(255, 255, 255, 0);

static_assert(!stringsEqual(WIFI_SSID, "YOUR_WIFI_SSID"),
              "Set WIFI_SSID in FlexSensorExample/project_config.h");
static_assert(!stringsEqual(WIFI_PASSWORD, "YOUR_WIFI_PASSWORD"),
              "Set WIFI_PASSWORD in FlexSensorExample/project_config.h");

static const uint16_t TCP_PORT = 8888;
static const uint16_t UDP_PORT = 9001;
static const uint16_t HTTP_PORT = 80;

// ========================
//  PIN MAP - EDIT THESE FIRST
// ========================
static const uint8_t FLEX_SENSOR_PIN = 32;  // Exposed analog-friendly GPIO32

// ========================
//  SENSOR TUNING
// ========================
static const uint16_t FLEX_TRIGGER_THRESHOLD = 2300;
static const uint16_t FLEX_RELEASE_THRESHOLD = 2100;
static const uint16_t FLEX_SAMPLE_INTERVAL_MS = 40;
static const uint16_t FLEX_RAINBOW_STEP_MS = 40;

// ========================
//  USER FEEDBACK
// ========================
static const uint32_t USER_LED_IDLE_COLOR = 0x001020;
static const uint32_t USER_LED_SYSTEM_OFF_COLOR = 0x100000;

// ========================
//  AUDIO REACTION
// ========================
static const bool FLEX_PLAY_SOUND_ON_TRIGGER = true;
static const bool FLEX_PLAY_SOUND_ON_RELEASE = false;
static const uint16_t FLEX_TRIGGER_TRACK = 2;
static const uint16_t FLEX_RELEASE_TRACK = 3;

// ========================
//  SHELL TEST DEFAULTS
// ========================
// These board-button actions are example-level convenience presets.
// They are not a limit of the SuperCircuit library itself.
// If a project needs custom button behavior, replace the handler logic
// directly in the sketch instead of trying to force it into this menu.
enum ExampleBoardButtonAction : uint8_t {
  ExampleBoardButtonNone = 0,
  ExampleBoardButtonToggleEnabled = 1,
  ExampleBoardButtonShowState = 2,
  ExampleBoardButtonPlayDefaultTrack = 3,
  ExampleBoardButtonStopAndRefresh = 4
};

static const uint16_t DEFAULT_TRACK = 1;
static const uint32_t SHELL_USER_PIXEL_READY = 0x001020;
static const uint32_t SHELL_USER_PIXEL_ON = 0x001000;
static const uint32_t SHELL_USER_PIXEL_OFF = 0x100000;
static const uint32_t SHELL_INTERACTIVE_ON_COLOR = 0x002800;
static const uint32_t SHELL_INTERACTIVE_OFF_COLOR = 0x280000;
static const uint32_t SHELL_COMMS_OFFLINE_COLOR = 0x281400;
static const uint32_t SHELL_COMMS_WIFI_ONLY_COLOR = 0x000028;
static const uint32_t SHELL_COMMS_TCP_READY_COLOR = 0x002828;
static const ExampleBoardButtonAction BOARD_BUTTON_1_ACTION = ExampleBoardButtonToggleEnabled;
static const ExampleBoardButtonAction BOARD_BUTTON_2_ACTION = ExampleBoardButtonPlayDefaultTrack;
static const ExampleBoardButtonAction BOARD_BUTTON_3_ACTION = ExampleBoardButtonStopAndRefresh;

static const FcCommsConfig kCommsConfig = {
  kDeviceId, kDeviceName, kDeviceTitle,
  true, true, true, true, true, true,
  TCP_PORT, UDP_PORT, HTTP_PORT,
  {WIFI_SSID, WIFI_PASSWORD, WIFI_IP, WIFI_GATEWAY, WIFI_SUBNET},
  {false, UDP_PORT, 0, 0, 0}
};

static const SuperCircuitConfig kSuperCircuitConfig = [] {
  SuperCircuitConfig config;
  config.defaultTitle = kDeviceTitle;
  config.defaultBottomLine = "Flex idle";
  config.defaultInteractiveEnabled = true;
  config.pixel.userDefaultColor = USER_LED_IDLE_COLOR;
  config.features.volumePot = SuperCircuitToggle::Disabled;
  config.audio.defaultVolume = 18;
  config.audio.followVolumePot = SuperCircuitToggle::Disabled;
  config.startup.speed = SuperCircuitStartupSpeed::Fast;
  return config;
}();

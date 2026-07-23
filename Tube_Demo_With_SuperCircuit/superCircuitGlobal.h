#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>
#include "config.h"

// ========================
//  SUPERCIRCUIT PIN MAP
// ========================
constexpr uint8_t NUM_PIXELS = 3;
constexpr uint8_t PIXEL_PIN  = 13;

// Buttons (external pulldown -> IDLE=LOW, PRESSED=HIGH)
constexpr uint8_t BUTTON_A_PIN       = 37;
constexpr uint8_t BUTTON_B_PIN       = 27;
constexpr uint8_t BUTTON_C_PIN       = 34;
constexpr uint8_t NUM_BUTTONS        = 3;
constexpr uint8_t BUTTON_ACTIVE_STATE = HIGH;


constexpr uint8_t POT_VOL_PIN = 39;


// OLED 128x32 over I2C
constexpr uint8_t SCREEN_WIDTH   = 128;
constexpr uint8_t SCREEN_HEIGHT  = 32;
constexpr int8_t  OLED_RESET_PIN = -1;
constexpr uint8_t OLED_I2C_ADDR  = 0x3C;

// DFPlayer Pro (DF1201S) UART pins
#if ENABLE_DFPLAYER
constexpr int DF_RX_PIN = 7;   // ESP32 RX
constexpr int DF_TX_PIN = 8;   // ESP32 TX
#endif

// ========================
//  UI / SCROLL CONSTANTS
// ========================
constexpr unsigned long OVERLAY_DURATION_MS = 500;  // default alert/init display
constexpr unsigned long SCROLL_STEP_MS      = 60;
constexpr unsigned long SCROLL_HOLD_MS      = 10000;
constexpr int           SCROLL_GAP          = 40;

// ========================
//  GLOBAL HARDWARE OBJECTS
// ========================
extern Adafruit_NeoPixel pixels;
extern Adafruit_SSD1306 display;
extern Bounce buttonDebounce[NUM_BUTTONS];

#if ENABLE_DFPLAYER
#include <DFRobot_DF1201S.h>
extern HardwareSerial& DFSerial;
extern DFRobot_DF1201S DF1201S;
#endif

// ========================
//  GLOBAL STATE FLAGS
// ========================
extern bool displayReady;
extern bool pixelsReady;
extern bool buttonsReady;
extern bool dfReady;
extern bool potsReady;

extern bool gSystemEnabled;
extern bool gWifiConnected;
extern bool gTcpConnected;

// ========================
//  TIME / TEXT STATE
// ========================
extern unsigned long bootMillis;

extern String titleLine;
extern String wifiLine;
extern String bottomLine;

// Overlay/alert state
extern bool overlayActive;
extern String overlayHeader;
extern String overlayText;
extern unsigned long overlayStartMs;
extern unsigned long overlayDurationMs;

// ========================
//  SCROLL STATE
// ========================
extern int16_t titleScrollX;
extern unsigned long titleLastScroll;

extern int16_t wifiScrollX;
extern unsigned long wifiLastScroll;

extern int16_t systemScrollX;
extern unsigned long systemLastScroll;

extern int16_t bottomScrollX;
extern unsigned long bottomLastScroll;

// ========================
//  POT STATE (stubbed but global)
// ========================
extern float pot1Filt;
extern float pot2Filt;
extern int pot1Reported;
extern int pot2Reported;

// ========================
//  DFPLAYER TRACK STATE
// ========================
#if ENABLE_DFPLAYER
extern bool dfIsPlaying;
extern uint16_t lastPlayedFile;
extern unsigned long currentTrackStartMs;
extern uint16_t currentTrackTotalSec;
extern unsigned long inferredTrackEndMs;
extern uint16_t lastCurTimeSec;
extern unsigned long lastDfTimePollMs;
extern String currentTrackName;
#endif

// ========================
//  PUBLIC API
// ========================

// Core lifecycle
void superCircuitSetup();
void manageSuperCircuit();

// State setters
void setSystemEnabled(bool enabled);
void setWifiStatus(bool connected, const IPAddress& ip);
void setTcpStatus(bool connected);

// Alerts & messaging
void showAlert(const String& header, const String& body,
               unsigned long durationMs = OVERLAY_DURATION_MS);

// Init display helper (used by superCircuit + other modules)
void showInitLine(const __FlashStringHelper* line);
void showInitLine(const __FlashStringHelper* line,
                  bool finalStatusKnown,
                  bool ok);

// Scrolling helpers (if other files ever want them)
int16_t textPixelWidth(const String& s, uint8_t size);
void drawScrollingLine(int16_t y, const String& s, uint8_t size,
                       int16_t& scrollX, unsigned long& lastStep);

#if ENABLE_DFPLAYER
// DFPlayer control + status
void setupDFplayer();
void audioPlayFile(uint16_t fileNum);
void audioStop();
void audioSetVolume(uint8_t vol);
void showCurrentTrack();
void noteTrackStarted(uint16_t fileNum);
void pollTrackEnded();
void trackEnded(uint16_t fileNum);
static void updateVolumePot();

#endif

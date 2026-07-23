/*
  EightTube_WS2815_Generative.ino

  Hardware:
    - Adafruit ESP32 Feather V2
    - WS2815 addressable LEDs
    - Tubes 1-4 daisy chained on GPIO 14
    - Tubes 5-8 daisy chained on GPIO 4

  Logical geometry:
    - 8 tube columns
    - Maximum height: 56 pixels per side
    - Heights: 56, 53, 50, 47, 44, 41, 38, 35
    - Logical Y=0 is the bottom; Y increases upward.
    - Each tube's physical data starts at bottom-front, travels upward,
      then wraps to top-back and travels downward, ending bottom-back.

  Serial monitor:
    - 115200 baud
    - Line ending: Newline

  Example commands:
    FRONT MODE WAVE
    FRONT SPEED 35
    FRONT DIR UP
    FRONT COLOR1 FF0055
    FRONT COLOR2 00AAFF
    FRONT PALETTE PARTY

    BACK MODE NOISE
    BACK SPEED 18
    BACK DIR LEFT
    BACK PALETTE OCEAN

    BACK LINK INVERSECOMP
    BACK LINK OFF

    BOTH MODE PLASMA
    BOTH SPEED 25
    BOTH PALETTE RAINBOW

    MASTER BRIGHTNESS 160
    FPS 60
    STATUS
    HELP
*/

#include <FastLED.h>

//SuperCircuit Stuff
#include "global.h"
#include "config.h"
#include "superCircuitGlobal.h"

// -----------------------------------------------------------------------------
// Hardware configuration
// -----------------------------------------------------------------------------

#define DATA_PIN_A 14
#define DATA_PIN_B 4
#define LED_TYPE WS2812B  // WS2815 uses the WS2812-style data protocol.
#define COLOR_ORDER RGB

constexpr uint8_t NUM_TUBES = 8;
constexpr uint8_t MAX_HEIGHT = 56;

constexpr uint8_t TUBE_HEIGHT[NUM_TUBES] = {
  56, 53, 50, 47, 44, 41, 38, 35
};

// Tubes 1-4: 2 * (56+53+50+47) = 412
// Tubes 5-8: 2 * (44+41+38+35) = 316
constexpr uint16_t LEDS_PIN_A = 412;
constexpr uint16_t LEDS_PIN_B = 316;

CRGB ledsPinA[LEDS_PIN_A];
CRGB ledsPinB[LEDS_PIN_B];

// Logical 2D frame buffers. Invalid pixels above shorter tubes remain black.
CRGB frontMap[NUM_TUBES][MAX_HEIGHT];
CRGB backMap[NUM_TUBES][MAX_HEIGHT];

// Start position of each tube inside its own physical daisy chain.
uint16_t tubeChainOffset[NUM_TUBES];

// -----------------------------------------------------------------------------
// Pattern configuration
// -----------------------------------------------------------------------------

enum SurfaceId : uint8_t {
  SURFACE_FRONT,
  SURFACE_BACK
};

enum PatternId : uint8_t {
  PATTERN_OFF,
  PATTERN_SOLID,
  PATTERN_GRADIENT,
  PATTERN_WAVE,
  PATTERN_CHASE,
  PATTERN_NOISE,
  PATTERN_TWINKLE,
  PATTERN_PLASMA
};

enum DirectionId : uint8_t {
  DIR_UP,
  DIR_DOWN,
  DIR_LEFT,
  DIR_RIGHT
};

enum PaletteId : uint8_t {
  PALETTE_RAINBOW,
  PALETTE_PARTY,
  PALETTE_OCEAN,
  PALETTE_LAVA,
  PALETTE_FOREST,
  PALETTE_CLOUD,
  PALETTE_HEAT,
  PALETTE_PURPLE,
  PALETTE_CUSTOM
};

enum BackLinkMode : uint8_t {
  BACK_LINK_OFF,
  BACK_LINK_INVERSE_COMPLEMENT
};

struct SurfaceSettings {
  PatternId pattern;
  DirectionId direction;
  PaletteId paletteId;
  CRGBPalette16 palette;
  CRGB color1;
  CRGB color2;

  // Nominal animation rate. Negative values reverse the phase direction.
  int16_t speed;

  // Spatial scale used by noise/plasma/gradient-style effects.
  uint8_t scale;

  // Surface-level brightness before master brightness.
  uint8_t brightness;

  // Pattern-local state.
  uint32_t lastTwinkleMs;
};

SurfaceSettings frontSettings;
SurfaceSettings backSettings;

uint8_t masterBrightness = 180;
uint8_t targetFPS = 60;

uint32_t lastFrameMs = 0;
String serialLine;

BackLinkMode backLinkMode = BACK_LINK_INVERSE_COMPLEMENT;

//SuperCircuit Stuff
long now = 0;

// -----------------------------------------------------------------------------
// Arduino lifecycle
// -----------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(1000);
  superCircuitSetup();  // SuperCircuit hardware + UI
  delay(1000);
  calculateTubeOffsets();
  initializeSettings();
  initLEDs();
  commsSetup();  // WiFi + TCP + WebSerial + OTA
  delay(1000);
  printHelp();
  printStatus();
}

void loop() {
  readSerialCommands();
  receiveComms();        // WiFi/TCP/Serial/OTA
  manageSuperCircuit();  // Buttons, screen, Neopixels, alerts

  if (gSystemEnabled) {
    // Put interactive logic here if you want a per-frame loop.
    // Most things can be driven from messageReceived() and button callbacks.
  }

  const uint32_t nowMs = millis();
  const uint16_t frameInterval = 1000U / ((targetFPS > 0) ? targetFPS : 1);

  if (nowMs - lastFrameMs < frameInterval) {
    return;
  }
  lastFrameMs = nowMs;

  renderSurface(SURFACE_FRONT, frontSettings, nowMs);

  if (backLinkMode == BACK_LINK_INVERSE_COMPLEMENT) {
    applyInverseComplementBackLink();
  } else {
    renderSurface(SURFACE_BACK, backSettings, nowMs);
  }

  copyLogicalMapsToPhysical();

  FastLED.show();
}

// ========================
//  EXPERIENCE CALLBACKS
// ========================


// Core message handler for TCP, WebSerial & USB Serial
void messageReceived(const String& raw) {
  String msg = raw;
  msg.trim();

  // Simple logging
  logLine("MSG: " + msg);

  // ---- Core/common commands ----
  if (msg.equalsIgnoreCase("PING")) {
    tcpPrintln("PONG");
    showAlert("TCP", "PING", 800);
    return;
  }

  if (msg.equalsIgnoreCase("ON")) {
    setSystemEnabled(true);
    return;
  }

  if (msg.equalsIgnoreCase("OFF")) {
    setSystemEnabled(false);
    return;
  }

#if ENABLE_DFPLAYER
  // Example: "PLAY 5"
  if (msg.startsWith("PLAY ")) {
    uint16_t fileNum = msg.substring(5).toInt();
    audioPlayFile(fileNum);
    return;
  }
  if (msg.equalsIgnoreCase("STOP")) {
    audioStop();
    return;
  }
#endif
  // ---- Experience-specific commands ----
  // Add your own protocol here. Examples:
  //
  // if (msg.equalsIgnoreCase("RED")) { /* set a state, etc */ return; }
  // if (msg.startsWith("STATE ")) { ... }

  // Fallback
  showAlert("MSG", msg, 1500);
}

// Button callbacks
void button1Pressed() {
  // Toggle current state
  bool newState = !gSystemEnabled;
  setSystemEnabled(newState);
  // Optional feedback to TCP controller or Serial/WebSerial
  tcpPrintln(String("SYSTEM ") + (newState ? "ON" : "OFF"));
}

void button2Pressed() {
  showAlert("BTN", "Randomize", 500);
  tcpPrintln("BTN 2");
  randomPatternFront();
  randomPatternBack();
}

void button3Pressed() {
  int currentMode = backLinkMode;
  Serial.println(currentMode);
  if (currentMode == 1) {
    backLinkMode = BACK_LINK_OFF;
    showAlert("BTN", "Backlink Off", 500);
  } else if (currentMode == 0) {
    backLinkMode = BACK_LINK_INVERSE_COMPLEMENT;
    showAlert("BTN", "Backlink On", 500);
  }
}


// ========================================================
// HOW TO SHOW AN ALERT ON THE BOTTOM OLED LINE
// --------------------------------------------------------
// Use showAlert("HEADER", "MESSAGE", durationMs)
//
// Example:
//     showAlert("BTN", "Pressed", 800);
//     showAlert("VOL", String(volume), 300);
//     showAlert("SYS", "Enabled", 1000);
//
// Alerts temporarily replace the bottom row of the display.
// After the duration expires, the bottom row automatically
// returns to showing the uptime timer.
//
// Notes:
//  * Calling showAlert again replaces the previous alert
//    (alerts are NOT queued).
//  * overlayActive = true while an alert is shown.
//  * drawScreen() handles everything—just call showAlert().
// ========================================================

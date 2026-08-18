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

BackLinkMode backLinkMode = BACK_LINK_INVERSE_COMPLEMENT;

uint8_t masterBrightness = 180;
uint8_t targetFPS = 60;

uint32_t lastFrameMs = 0;
String serialLine;

void calculateTubeOffsets() {
  uint16_t runningA = 0;
  uint16_t runningB = 0;

  for (uint8_t tube = 0; tube < NUM_TUBES; ++tube) {
    if (tube < 4) {
      tubeChainOffset[tube] = runningA;
      runningA += 2U * TUBE_HEIGHT[tube];
    } else {
      tubeChainOffset[tube] = runningB;
      runningB += 2U * TUBE_HEIGHT[tube];
    }
  }
}

bool pixelExists(uint8_t tube, uint8_t y) {
  return tube < NUM_TUBES && y < TUBE_HEIGHT[tube];
}

// Returns a pointer to the physical CRGB pixel corresponding to one logical
// front/back coordinate. Logical Y=0 is bottom on both maps.
CRGB *physicalPixel(uint8_t tube, bool isBack, uint8_t y) {
  if (!pixelExists(tube, y)) {
    return nullptr;
  }

  const uint16_t base = tubeChainOffset[tube];
  const uint16_t height = TUBE_HEIGHT[tube];
  uint16_t index;

  if (!isBack) {
    // Bottom-front to top-front.
    index = base + y;
  } else {
    // Physical back data travels top to bottom.
    // Logical y=0 (bottom-back) is therefore the final physical pixel.
    index = base + (2U * height - 1U - y);
  }

  return (tube < 4) ? &ledsPinA[index] : &ledsPinB[index];
}

CRGB &mapPixel(SurfaceId surface, uint8_t x, uint8_t y) {
  return (surface == SURFACE_FRONT) ? frontMap[x][y] : backMap[x][y];
}

void clearLogicalMap(CRGB mapBuffer[NUM_TUBES][MAX_HEIGHT]) {
  for (uint8_t x = 0; x < NUM_TUBES; ++x) {
    for (uint8_t y = 0; y < MAX_HEIGHT; ++y) {
      mapBuffer[x][y] = CRGB::Black;
    }
  }
}

void copyLogicalMapsToPhysical() {
  fill_solid(ledsPinA, LEDS_PIN_A, CRGB::Black);
  fill_solid(ledsPinB, LEDS_PIN_B, CRGB::Black);

  for (uint8_t x = 0; x < NUM_TUBES; ++x) {
    for (uint8_t y = 0; y < TUBE_HEIGHT[x]; ++y) {
      CRGB *frontPhysical = physicalPixel(x, false, y);
      CRGB *backPhysical = physicalPixel(x, true, y);

      if (frontPhysical != nullptr) {
        *frontPhysical = frontMap[x][y];
      }

      if (backPhysical != nullptr) {
        *backPhysical = backMap[x][y];
      }
    }
  }
}

uint8_t surfaceCoordinate(
  DirectionId direction,
  uint8_t x,
  uint8_t y,
  uint8_t xScale = 32,
  uint8_t yScale = 5) {
  switch (direction) {
    case DIR_DOWN: return uint8_t(255 - y * yScale);
    case DIR_LEFT: return uint8_t(255 - x * xScale);
    case DIR_RIGHT: return uint8_t(x * xScale);
    case DIR_UP:
    default: return uint8_t(y * yScale);
  }
}

int8_t directionSign(DirectionId direction) {
  return (direction == DIR_DOWN || direction == DIR_LEFT) ? -1 : 1;
}

uint8_t phase8(const SurfaceSettings &settings, uint32_t nowMs) {
  // speed=30 gives a useful moderate animation rate.
  int32_t phase = (int32_t(nowMs) * int32_t(settings.speed)) / 40;
  phase *= directionSign(settings.direction);
  return uint8_t(phase);
}

uint16_t phase16(const SurfaceSettings &settings, uint32_t nowMs) {
  int32_t phase = (int32_t(nowMs) * int32_t(settings.speed)) / 4;
  phase *= directionSign(settings.direction);
  return uint16_t(phase);
}

CRGB applySurfaceBrightness(CRGB color, uint8_t brightness) {
  color.nscale8_video(brightness);
  return color;
}

CRGB complementaryColor(const CRGB &source) {
  // Rotate HSV hue by 180 degrees while preserving saturation.
  CHSV hsv = rgb2hsv_approximate(source);
  hsv.hue += 128;
  hsv.value = 255;
  return CRGB(hsv);
}

uint8_t perceivedBrightness(const CRGB &color) {
  // Fast integer approximation of relative luminance.
  return uint8_t(
    (uint16_t(color.r) * 54U + uint16_t(color.g) * 183U + uint16_t(color.b) * 19U) >> 8);
}

void applyInverseComplementBackLink() {
  for (uint8_t x = 0; x < NUM_TUBES; ++x) {
    for (uint8_t y = 0; y < TUBE_HEIGHT[x]; ++y) {
      const CRGB front = frontMap[x][y];
      const uint8_t frontLevel = perceivedBrightness(front);
      const uint8_t inverseLevel = 255U - frontLevel;

      CRGB complement = complementaryColor(front);

      // A black front pixel has no meaningful hue. Use the front surface's
      // current primary color as the hue source in that case.
      if (frontLevel == 0) {
        complement = complementaryColor(frontSettings.color1);
      }

      complement.nscale8_video(inverseLevel);
      complement.nscale8_video(backSettings.brightness);
      backMap[x][y] = complement;
    }

    // Keep invalid cells black.
    for (uint8_t y = TUBE_HEIGHT[x]; y < MAX_HEIGHT; ++y) {
      backMap[x][y] = CRGB::Black;
    }
  }
}


bool applySurfaceCommand(
  SurfaceSettings &settings,
  const String &parameterToken,
  const String &valueToken) {
  const String parameter = upperCopy(parameterToken);

  if (parameter == "MODE") {
    PatternId parsed;
    if (!parsePattern(valueToken, parsed)) return false;
    settings.pattern = parsed;
    return true;
  }

  if (parameter == "SPEED") {
    settings.speed = constrain(valueToken.toInt(), -100, 100);
    return true;
  }

  if (parameter == "DIR" || parameter == "DIRECTION") {
    DirectionId parsed;
    if (!parseDirection(valueToken, parsed)) return false;
    settings.direction = parsed;
    return true;
  }

  if (parameter == "COLOR1") {
    CRGB parsed;
    if (!parseHexColor(valueToken, parsed)) return false;
    settings.color1 = parsed;
    if (settings.paletteId == PALETTE_CUSTOM) refreshPalette(settings);
    return true;
  }

  if (parameter == "COLOR2") {
    CRGB parsed;
    if (!parseHexColor(valueToken, parsed)) return false;
    settings.color2 = parsed;
    if (settings.paletteId == PALETTE_CUSTOM) refreshPalette(settings);
    return true;
  }

  if (parameter == "PALETTE") {
    PaletteId parsed;
    if (!parsePalette(valueToken, parsed)) return false;
    settings.paletteId = parsed;
    refreshPalette(settings);
    return true;
  }

  if (parameter == "SCALE") {
    settings.scale = constrain(valueToken.toInt(), 1, 255);
    return true;
  }

  if (parameter == "BRIGHTNESS") {
    settings.brightness = constrain(valueToken.toInt(), 0, 255);
    return true;
  }

  return false;
}


// -----------------------------------------------------------------------------
// Arduino lifecycle
// -----------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(1000);

  calculateTubeOffsets();
  initializeSettings();

  FastLED.addLeds<LED_TYPE, DATA_PIN_A, COLOR_ORDER>(
           ledsPinA,
           LEDS_PIN_A)
    .setCorrection(TypicalLEDStrip);

  FastLED.addLeds<LED_TYPE, DATA_PIN_B, COLOR_ORDER>(
           ledsPinB,
           LEDS_PIN_B)
    .setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(masterBrightness);
  FastLED.clear(true);

  random16_add_entropy(esp_random());

  printHelp();
  printStatus();
}

void loop() {
  readSerialCommands();

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

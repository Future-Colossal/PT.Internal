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

// -----------------------------------------------------------------------------
// Utility functions
// -----------------------------------------------------------------------------

CRGBPalette16 purplePalette() {
  return CRGBPalette16(
    CRGB(8, 0, 20),
    CRGB(45, 0, 90),
    CRGB(125, 0, 255),
    CRGB(255, 40, 180));
}

CRGBPalette16 customPalette(const CRGB &a, const CRGB &b) {
  return CRGBPalette16(
    a,
    blend(a, b, 85),
    blend(a, b, 170),
    b);
}

CRGBPalette16 paletteFor(PaletteId id, const CRGB &c1, const CRGB &c2) {
  switch (id) {
    case PALETTE_PARTY: return PartyColors_p;
    case PALETTE_OCEAN: return OceanColors_p;
    case PALETTE_LAVA: return LavaColors_p;
    case PALETTE_FOREST: return ForestColors_p;
    case PALETTE_CLOUD: return CloudColors_p;
    case PALETTE_HEAT: return HeatColors_p;
    case PALETTE_PURPLE: return purplePalette();
    case PALETTE_CUSTOM: return customPalette(c1, c2);
    case PALETTE_RAINBOW:
    default: return RainbowColors_p;
  }
}

void refreshPalette(SurfaceSettings &settings) {
  settings.palette = paletteFor(
    settings.paletteId,
    settings.color1,
    settings.color2);
}

void initializeSettings() {
  frontSettings.pattern = PATTERN_PLASMA;
  frontSettings.direction = DIR_UP;
  frontSettings.paletteId = PALETTE_OCEAN;
  frontSettings.color1 = CRGB(255, 0, 80);
  frontSettings.color2 = CRGB(0, 120, 255);
  frontSettings.speed = 3;
  frontSettings.scale = 64;
  frontSettings.brightness = 128;
  frontSettings.lastTwinkleMs = 0;
  refreshPalette(frontSettings);

  backSettings.pattern = PATTERN_PLASMA;
  backSettings.direction = DIR_DOWN;
  backSettings.paletteId = PALETTE_LAVA;
  backSettings.color1 = CRGB(120, 0, 255);
  backSettings.color2 = CRGB(0, 180, 255);
  backSettings.speed = 8;
  backSettings.scale = 32;
  backSettings.brightness = 255;
  backSettings.lastTwinkleMs = 0;
  refreshPalette(backSettings);
}

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

// -----------------------------------------------------------------------------
// Pattern renderer
// -----------------------------------------------------------------------------

void renderSurface(
  SurfaceId surface,
  SurfaceSettings &settings,
  uint32_t nowMs) {
  CRGB(*buffer)
  [MAX_HEIGHT] =
    (surface == SURFACE_FRONT) ? frontMap : backMap;

  const uint8_t phase = phase8(settings, nowMs);
  const uint16_t widePhase = phase16(settings, nowMs);

  if (settings.pattern != PATTERN_TWINKLE) {
    clearLogicalMap(buffer);
  }

  switch (settings.pattern) {
    case PATTERN_OFF:
      clearLogicalMap(buffer);
      break;

    case PATTERN_SOLID:
      for (uint8_t x = 0; x < NUM_TUBES; ++x) {
        for (uint8_t y = 0; y < TUBE_HEIGHT[x]; ++y) {
          buffer[x][y] =
            applySurfaceBrightness(settings.color1, settings.brightness);
        }
      }
      break;

    case PATTERN_GRADIENT:
      for (uint8_t x = 0; x < NUM_TUBES; ++x) {
        for (uint8_t y = 0; y < TUBE_HEIGHT[x]; ++y) {
          const uint8_t spatial =
            surfaceCoordinate(settings.direction, x, y, 36, 5);
          CRGB color = ColorFromPalette(
            settings.palette,
            spatial + phase,
            255,
            LINEARBLEND);
          buffer[x][y] =
            applySurfaceBrightness(color, settings.brightness);
        }
      }
      break;

    case PATTERN_WAVE:
      for (uint8_t x = 0; x < NUM_TUBES; ++x) {
        for (uint8_t y = 0; y < TUBE_HEIGHT[x]; ++y) {
          const uint8_t spatial =
            surfaceCoordinate(settings.direction, x, y, 40, 7);
          const uint8_t wave = sin8(spatial + phase);
          CRGB color = ColorFromPalette(
            settings.palette,
            spatial + phase,
            wave,
            LINEARBLEND);
          buffer[x][y] =
            applySurfaceBrightness(color, settings.brightness);
        }
      }
      break;

    case PATTERN_CHASE:
      for (uint8_t x = 0; x < NUM_TUBES; ++x) {
        for (uint8_t y = 0; y < TUBE_HEIGHT[x]; ++y) {
          const uint8_t spatial =
            surfaceCoordinate(settings.direction, x, y, 48, 12);
          const uint8_t position = spatial + phase;
          const uint8_t brightness =
            (position % 64U < 18U) ? 255 : 12;

          CRGB color = ColorFromPalette(
            settings.palette,
            position * 2U,
            brightness,
            LINEARBLEND);
          buffer[x][y] =
            applySurfaceBrightness(color, settings.brightness);
        }
      }
      break;

    case PATTERN_NOISE:
      for (uint8_t x = 0; x < NUM_TUBES; ++x) {
        for (uint8_t y = 0; y < TUBE_HEIGHT[x]; ++y) {
          uint16_t nx = uint16_t(x) * settings.scale * 4U;
          uint16_t ny = uint16_t(y) * settings.scale;

          if (settings.direction == DIR_LEFT || settings.direction == DIR_RIGHT) {
            nx += widePhase;
          } else {
            ny += widePhase;
          }

          const uint8_t noiseValue = inoise8(nx, ny, widePhase / 2U);
          CRGB color = ColorFromPalette(
            settings.palette,
            noiseValue + phase,
            qadd8(noiseValue, 40),
            LINEARBLEND);
          buffer[x][y] =
            applySurfaceBrightness(color, settings.brightness);
        }
      }
      break;

    case PATTERN_TWINKLE:
      {
        // Persistent frame: fade old stars and periodically add new ones.
        const uint8_t fadeAmount =
          constrain(map(abs(settings.speed), 1, 100, 8, 45), 8, 45);

        for (uint8_t x = 0; x < NUM_TUBES; ++x) {
          for (uint8_t y = 0; y < TUBE_HEIGHT[x]; ++y) {
            buffer[x][y].fadeToBlackBy(fadeAmount);
          }
        }

        const uint16_t interval =
          constrain(map(abs(settings.speed), 1, 100, 220, 18), 18, 220);

        if (nowMs - settings.lastTwinkleMs >= interval) {
          settings.lastTwinkleMs = nowMs;

          // Add 1-3 stars per update depending on speed.
          const uint8_t starCount =
            constrain(map(abs(settings.speed), 1, 100, 1, 3), 1, 3);

          for (uint8_t i = 0; i < starCount; ++i) {
            const uint8_t x = random8(NUM_TUBES);
            const uint8_t y = random8(TUBE_HEIGHT[x]);
            CRGB color = ColorFromPalette(
              settings.palette,
              random8() + phase,
              255,
              LINEARBLEND);
            buffer[x][y] +=
              applySurfaceBrightness(color, settings.brightness);
          }
        }
        break;
      }

    case PATTERN_PLASMA:
      for (uint8_t x = 0; x < NUM_TUBES; x++) {
        uint8_t xPhase = x * 31;

        for (uint8_t y = 0; y < TUBE_HEIGHT[x]; y++) {
          uint8_t yPhase = y * 7;

          uint8_t a = sin8(xPhase + phase);
          uint8_t b = cos8(yPhase + phase);
          uint8_t c = sin8(x * 19 + y * 5 + phase);

          uint8_t index = (uint16_t(a) + b + c) / 3;

          CRGB color = ColorFromPalette(
            settings.palette,
            index,
            255,
            LINEARBLEND);

          buffer[x][y] = applySurfaceBrightness(color, settings.brightness);
        }
      }
      break;
  }
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

// -----------------------------------------------------------------------------
// Serial command parser
// -----------------------------------------------------------------------------

String upperCopy(String value) {
  value.trim();
  value.toUpperCase();
  return value;
}

bool parseHexColor(const String &text, CRGB &result) {
  String value = text;
  value.trim();

  if (value.startsWith("#")) {
    value.remove(0, 1);
  }
  if (value.startsWith("0X")) {
    value.remove(0, 2);
  }

  if (value.length() != 6) {
    return false;
  }

  char *endPointer = nullptr;
  const uint32_t parsed = strtoul(value.c_str(), &endPointer, 16);

  if (endPointer == value.c_str() || *endPointer != '\0') {
    return false;
  }

  result = CRGB(
    uint8_t((parsed >> 16) & 0xFF),
    uint8_t((parsed >> 8) & 0xFF),
    uint8_t(parsed & 0xFF));
  return true;
}

bool parsePattern(const String &token, PatternId &pattern) {
  const String value = upperCopy(token);

  if (value == "OFF") pattern = PATTERN_OFF;
  else if (value == "SOLID") pattern = PATTERN_SOLID;
  else if (value == "GRADIENT") pattern = PATTERN_GRADIENT;
  else if (value == "WAVE") pattern = PATTERN_WAVE;
  else if (value == "CHASE") pattern = PATTERN_CHASE;
  else if (value == "NOISE") pattern = PATTERN_NOISE;
  else if (value == "TWINKLE") pattern = PATTERN_TWINKLE;
  else if (value == "PLASMA") pattern = PATTERN_PLASMA;
  else return false;

  return true;
}

bool parseDirection(const String &token, DirectionId &direction) {
  const String value = upperCopy(token);

  if (value == "UP") direction = DIR_UP;
  else if (value == "DOWN") direction = DIR_DOWN;
  else if (value == "LEFT") direction = DIR_LEFT;
  else if (value == "RIGHT") direction = DIR_RIGHT;
  else return false;

  return true;
}

bool parsePalette(const String &token, PaletteId &palette) {
  const String value = upperCopy(token);

  if (value == "RAINBOW") palette = PALETTE_RAINBOW;
  else if (value == "PARTY") palette = PALETTE_PARTY;
  else if (value == "OCEAN") palette = PALETTE_OCEAN;
  else if (value == "LAVA") palette = PALETTE_LAVA;
  else if (value == "FOREST") palette = PALETTE_FOREST;
  else if (value == "CLOUD") palette = PALETTE_CLOUD;
  else if (value == "HEAT") palette = PALETTE_HEAT;
  else if (value == "PURPLE") palette = PALETTE_PURPLE;
  else if (value == "CUSTOM") palette = PALETTE_CUSTOM;
  else return false;

  return true;
}

const char *patternName(PatternId pattern) {
  switch (pattern) {
    case PATTERN_OFF: return "OFF";
    case PATTERN_SOLID: return "SOLID";
    case PATTERN_GRADIENT: return "GRADIENT";
    case PATTERN_WAVE: return "WAVE";
    case PATTERN_CHASE: return "CHASE";
    case PATTERN_NOISE: return "NOISE";
    case PATTERN_TWINKLE: return "TWINKLE";
    case PATTERN_PLASMA: return "PLASMA";
    default: return "?";
  }
}

const char *directionName(DirectionId direction) {
  switch (direction) {
    case DIR_UP: return "UP";
    case DIR_DOWN: return "DOWN";
    case DIR_LEFT: return "LEFT";
    case DIR_RIGHT: return "RIGHT";
    default: return "?";
  }
}

const char *paletteName(PaletteId palette) {
  switch (palette) {
    case PALETTE_RAINBOW: return "RAINBOW";
    case PALETTE_PARTY: return "PARTY";
    case PALETTE_OCEAN: return "OCEAN";
    case PALETTE_LAVA: return "LAVA";
    case PALETTE_FOREST: return "FOREST";
    case PALETTE_CLOUD: return "CLOUD";
    case PALETTE_HEAT: return "HEAT";
    case PALETTE_PURPLE: return "PURPLE";
    case PALETTE_CUSTOM: return "CUSTOM";
    default: return "?";
  }
}

String colorToHex(const CRGB &color) {
  char output[7];
  snprintf(
    output,
    sizeof(output),
    "%02X%02X%02X",
    color.r,
    color.g,
    color.b);
  return String(output);
}

void printSurfaceStatus(const char *name, const SurfaceSettings &settings) {
  Serial.print(name);
  Serial.print(F(": MODE="));
  Serial.print(patternName(settings.pattern));
  Serial.print(F(" SPEED="));
  Serial.print(settings.speed);
  Serial.print(F(" DIR="));
  Serial.print(directionName(settings.direction));
  Serial.print(F(" PALETTE="));
  Serial.print(paletteName(settings.paletteId));
  Serial.print(F(" COLOR1="));
  Serial.print(colorToHex(settings.color1));
  Serial.print(F(" COLOR2="));
  Serial.print(colorToHex(settings.color2));
  Serial.print(F(" SCALE="));
  Serial.print(settings.scale);
  Serial.print(F(" BRIGHTNESS="));
  Serial.println(settings.brightness);
}

void printStatus() {
  Serial.println(F("--- LED STATUS ---"));
  printSurfaceStatus("FRONT", frontSettings);
  printSurfaceStatus("BACK", backSettings);
  Serial.print(F("BACK LINK="));
  Serial.println(
    backLinkMode == BACK_LINK_INVERSE_COMPLEMENT
      ? "INVERSECOMP"
      : "OFF");
  Serial.print(F("MASTER BRIGHTNESS="));
  Serial.println(masterBrightness);
  Serial.print(F("FPS="));
  Serial.println(targetFPS);
}

void printHelp() {
  Serial.println();
  Serial.println(F("=== Eight Tube LED Controller ==="));
  Serial.println(F("Commands are not case-sensitive."));
  Serial.println();
  Serial.println(F("Surface selection: FRONT, BACK, or BOTH"));
  Serial.println(F("  <surface> MODE OFF|SOLID|GRADIENT|WAVE|CHASE|NOISE|TWINKLE|PLASMA"));
  Serial.println(F("  <surface> SPEED -100..100"));
  Serial.println(F("  <surface> DIR UP|DOWN|LEFT|RIGHT"));
  Serial.println(F("  <surface> COLOR1 RRGGBB"));
  Serial.println(F("  <surface> COLOR2 RRGGBB"));
  Serial.println(F("  <surface> PALETTE RAINBOW|PARTY|OCEAN|LAVA|FOREST|CLOUD|HEAT|PURPLE|CUSTOM"));
  Serial.println(F("  <surface> SCALE 1..255"));
  Serial.println(F("  <surface> BRIGHTNESS 0..255"));
  Serial.println();
  Serial.println(F("Back-link mode:"));
  Serial.println(F("  BACK LINK INVERSECOMP"));
  Serial.println(F("  BACK LINK OFF"));
  Serial.println();
  Serial.println(F("Global:"));
  Serial.println(F("  MASTER BRIGHTNESS 0..255"));
  Serial.println(F("  FPS 1..120"));
  Serial.println(F("  STATUS"));
  Serial.println(F("  HELP"));
  Serial.println();
}

int splitTokens(const String &line, String tokens[], int maxTokens) {
  int count = 0;
  int start = 0;
  const int length = line.length();

  while (start < length && count < maxTokens) {
    while (start < length && isspace(line[start])) {
      ++start;
    }
    if (start >= length) {
      break;
    }

    int end = start;
    while (end < length && !isspace(line[end])) {
      ++end;
    }

    tokens[count++] = line.substring(start, end);
    start = end;
  }

  return count;
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

void processSerialCommand(String line) {
  line.trim();
  if (line.length() == 0) return;

  String tokens[4];
  const int count = splitTokens(line, tokens, 4);

  for (int i = 0; i < count; ++i) {
    tokens[i].toUpperCase();
  }

  if (tokens[0] == "HELP") {
    printHelp();
    return;
  }

  if (tokens[0] == "STATUS") {
    printStatus();
    return;
  }

  if (tokens[0] == "FPS" && count >= 2) {
    targetFPS = constrain(tokens[1].toInt(), 1, 120);
    Serial.print(F("OK FPS "));
    Serial.println(targetFPS);
    return;
  }

  if (tokens[0] == "MASTER" && count >= 3 && tokens[1] == "BRIGHTNESS") {
    masterBrightness = constrain(tokens[2].toInt(), 0, 255);
    FastLED.setBrightness(masterBrightness);
    Serial.print(F("OK MASTER BRIGHTNESS "));
    Serial.println(masterBrightness);
    return;
  }

  if (tokens[0] == "BACK" && count >= 3 && tokens[1] == "LINK") {
    if (tokens[2] == "INVERSECOMP" || tokens[2] == "INVERSE_COMPLEMENT") {
      backLinkMode = BACK_LINK_INVERSE_COMPLEMENT;
      Serial.println(F("OK BACK LINK INVERSECOMP"));
      return;
    }

    if (tokens[2] == "OFF") {
      backLinkMode = BACK_LINK_OFF;
      Serial.println(F("OK BACK LINK OFF"));
      return;
    }

    Serial.println(F("ERR Expected INVERSECOMP or OFF"));
    return;
  }

  if (count >= 3 && (tokens[0] == "FRONT" || tokens[0] == "BACK" || tokens[0] == "BOTH")) {
    bool success = true;

    if (tokens[0] == "FRONT" || tokens[0] == "BOTH") {
      success &= applySurfaceCommand(
        frontSettings,
        tokens[1],
        tokens[2]);
    }

    if (tokens[0] == "BACK" || tokens[0] == "BOTH") {
      success &= applySurfaceCommand(
        backSettings,
        tokens[1],
        tokens[2]);
    }

    if (success) {
      Serial.print(F("OK "));
      Serial.println(line);
    } else {
      Serial.print(F("ERR Invalid command: "));
      Serial.println(line);
    }
    return;
  }

  Serial.print(F("ERR Unknown command: "));
  Serial.println(line);
  Serial.println(F("Type HELP for command syntax."));
}

void readSerialCommands() {
  while (Serial.available() > 0) {
    char incoming = char(Serial.read());

    if (incoming == '\n' || incoming == '\r') {
      if (serialLine.length() > 0) {
        processSerialCommand(serialLine);
        serialLine = "";
      }
    } else if (serialLine.length() < 120) {
      serialLine += incoming;
    }
  }
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

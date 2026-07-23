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
  Serial.println(F("Passing command to SuperCircuit Parser!"));
  messageReceived(line);
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

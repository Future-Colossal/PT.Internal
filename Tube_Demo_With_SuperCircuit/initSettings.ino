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

void initLEDs() {
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
}

void randomPatternFront() {
  backLinkMode = BACK_LINK_OFF;
  int randomPat = random(8);
  int randomDir = random(4);
  int randomPal = random(8);
  int randomSpeed = random(10);
  int randomScale = random(64);

  frontSettings.pattern = (PatternId)randomPat;
  frontSettings.direction = (DirectionId)randomDir;
  frontSettings.paletteId = (PaletteId)randomPal;
  frontSettings.color1 = CRGB(random(255), random(255), random(255));
  frontSettings.color2 = CRGB(random(255), random(255), random(255));
  frontSettings.speed = randomSpeed;
  frontSettings.scale = randomScale;
  frontSettings.brightness = 128;
  frontSettings.lastTwinkleMs = 0;
  refreshPalette(frontSettings);
}

void randomPatternBack() {
  backLinkMode = BACK_LINK_OFF;
  int randomPat = random(8);
  int randomDir = random(4);
  int randomPal = random(8);
  int randomSpeed = random(10);
  int randomScale = random(64);

  backSettings.pattern = (PatternId)randomPat;
  backSettings.direction = (DirectionId)randomDir;
  backSettings.paletteId = (PaletteId)randomPal;
  backSettings.color1 = CRGB(random(255), random(255), random(255));
  backSettings.color2 = CRGB(random(255), random(255), random(255));
  backSettings.speed = randomSpeed;
  backSettings.scale = randomScale;
  backSettings.brightness = 128;
  backSettings.lastTwinkleMs = 0;
  refreshPalette(backSettings);
}
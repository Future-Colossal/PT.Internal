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
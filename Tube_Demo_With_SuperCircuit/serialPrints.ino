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
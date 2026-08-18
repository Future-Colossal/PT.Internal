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
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
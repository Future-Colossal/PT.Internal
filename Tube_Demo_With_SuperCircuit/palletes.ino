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

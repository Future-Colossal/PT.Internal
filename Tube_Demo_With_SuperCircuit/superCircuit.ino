#include "global.h"
#include "superCircuitGlobal.h"

// ========================
//  CONSTANTS
// ========================
constexpr unsigned long DF_TIME_POLL_INTERVAL_MS = 500;

// ========================
//  HARDWARE OBJECTS
// ========================
Adafruit_NeoPixel pixels(NUM_PIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
// ESP32 Feather V2 built-in NeoPixel
Adafruit_NeoPixel builtInPixel(1, 0, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET_PIN);
Bounce buttonDebounce[NUM_BUTTONS];

#if ENABLE_DFPLAYER
HardwareSerial& DFSerial = Serial1;
DFRobot_DF1201S DF1201S;
#endif

// ========================
//  GLOBAL STATE DEFINITIONS
// ========================
bool displayReady = false;
bool pixelsReady = false;
bool buttonsReady = false;
bool dfReady = false;
bool potsReady = false;

bool gSystemEnabled = DEFAULT_SYSTEM_ENABLED;
bool gWifiConnected = false;
bool gTcpConnected = false;

unsigned long bootMillis = 0;

String titleLine = DEFAULT_TITLE;
String wifiLine = "WiFi: offline";
String bottomLine = "";

// Overlay/alert
bool overlayActive = false;
String overlayHeader;
String overlayText;
unsigned long overlayStartMs = 0;
unsigned long overlayDurationMs = 0;

// Scroll state
int16_t titleScrollX = 0;
unsigned long titleLastScroll = 0;
int16_t wifiScrollX = 0;
unsigned long wifiLastScroll = 0;
int16_t systemScrollX = 0;
unsigned long systemLastScroll = 0;
int16_t bottomScrollX = 0;
unsigned long bottomLastScroll = 0;

// Pot state (stubbed)
float pot1Filt = 0.0f;
float pot2Filt = 0.0f;
int pot1Reported = 0;
int pot2Reported = 0;

// DFPlayer state
#if ENABLE_DFPLAYER
bool dfIsPlaying = false;
uint16_t lastPlayedFile = 0;
unsigned long currentTrackStartMs = 0;
uint16_t currentTrackTotalSec = 0;
unsigned long inferredTrackEndMs = 0;
uint16_t lastCurTimeSec = 0;
unsigned long lastDfTimePollMs = 0;
String currentTrackName;
#endif

// Forward declarations (internal)
static void updateWifiLine();
static void drawScreen();
static void setupButtons();
static void updateButtons();
static void updateVolumePot();
static void updateStatusPixels();
static String buildUptimeLine();
static void setupDisplay();
static void setupNeoPixels();
static void setupPots();
static void rainbowOnce(uint16_t stepDelayMs);


// ========================
//  INIT SCREEN HELPERS
// ========================

// 1-arg convenience wrapper
void showInitLine(const __FlashStringHelper* line) {
  showInitLine(line, false, false);
}

// 3-arg implementation
void showInitLine(const __FlashStringHelper* line,
                  bool finalStatusKnown,
                  bool ok) {
  // If we don't have a display, at least log to Serial
  if (!displayReady) {
    Serial.print(F("[Init] "));
    Serial.println(line);
    if (finalStatusKnown) {
      Serial.print(F(" -> "));
      Serial.println(ok ? F("OK") : F("FAIL"));
    }

    // Flash NeoPixel 0 even if display isn't ready (if pixels are)
    if (finalStatusKnown && pixelsReady) {
      uint32_t c = ok ? pixels.Color(0, 40, 0)   // green = OK
                      : pixels.Color(40, 0, 0);  // red   = FAIL
      pixels.setPixelColor(0, c);
      pixels.show();
    }

    delay(OVERLAY_DURATION_MS);
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  // Header
  display.setCursor(0, 0);
  display.println(F("Initializing..."));

  // Main line (what we’re initializing)
  display.setCursor(0, 10);
  display.print(line);

  // Optional status line (OK / FAIL)
  if (finalStatusKnown) {
    display.setCursor(0, 20);
    display.print(F("Status: "));
    display.println(ok ? F("OK") : F("FAIL"));
  }

  display.display();

  // Flash NeoPixel 0 to reflect this init step's result,
  // if pixels have been brought up already.
  if (finalStatusKnown && pixelsReady) {
    uint32_t c = ok ? pixels.Color(0, 40, 0)   // green = OK
                    : pixels.Color(40, 0, 0);  // red   = FAIL
    pixels.setPixelColor(0, c);
    pixels.show();
    delay(500);
    pixels.clear();
    pixels.show();
  }

  delay(OVERLAY_DURATION_MS);
}


// ========================
//  LOW-LEVEL SETUP HELPERS
// ========================
static void setupDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    displayReady = false;
    Serial.println(F("SSD1306 init failed (continuing headless)."));
    delay(2000);
    return;
  }

  displayReady = true;
  display.setTextWrap(false);

  // Simple boot splash
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Future"));
  display.println(F("Colossal"));
  display.display();
  delay(1000);
}

static void setupNeoPixels() {
  builtInPixel.begin();
  builtInPixel.setBrightness(10);
  builtInPixel.setPixelColor(0, builtInPixel.Color(40, 0, 0));
  builtInPixel.show();
  pixels.begin();
  pixels.setBrightness(20);
  pixels.clear();
  pixels.show();
  pixelsReady = true;
}

static void setupPots() {
  // Stub: mark as ready; you can wire real analog reads later
  potsReady = true;
  pot1Filt = 0.0f;
  pot2Filt = 0.0f;
  pot1Reported = 0;
  pot2Reported = 0;
}

// Uses wheel(pos) from your util.ino
static void rainbowOnce(uint16_t stepDelayMs) {
  if (!pixelsReady) return;

  for (uint16_t pos = 0; pos < 256; pos++) {
    for (uint8_t i = 0; i < NUM_PIXELS; i++) {
      uint8_t p = (pos + (i * 10)) & 0xFF;
      pixels.setPixelColor(i, wheel(p));
      builtInPixel.setPixelColor(0, wheel(p));
    }
    builtInPixel.show();
    pixels.show();
    delay(stepDelayMs);
  }
  delay(100);
  builtInPixel.setPixelColor(0, builtInPixel.Color(40, 0, 0));
  delay(100);
  builtInPixel.show();
  pixels.clear();
  pixels.show();
}

// ========================
//  SETUP
// ========================
void superCircuitSetup() {
  bootMillis = millis();

  // --- Display first ---
  showInitLine(F("Init: Display..."));
  setupDisplay();
  showInitLine(F("Init: Display"), true, displayReady);

  // --- NeoPixels + Rainbow banner ---
  showInitLine(F("Init: NeoPixels..."));
  setupNeoPixels();
  if (pixelsReady) {
    rainbowOnce(5);
  }
  showInitLine(F("Init: NeoPixels"), true, pixelsReady);

  // --- Buttons ---
  showInitLine(F("Init: Buttons..."));
  setupButtons();
  showInitLine(F("Init: Buttons"), true, buttonsReady);

  // --- Pots (placeholder) ---
  showInitLine(F("Init: Pots..."));
  setupPots();
  showInitLine(F("Init: Pots"), true, potsReady);

  // --- DFPlayer (audio) ---
  showInitLine(F("Init: DFPlayer..."));
  setupDFplayer();
  showInitLine(F("Init: DFPlayer"), true, dfReady);

  // Clear init screen before normal UI takes over
  overlayActive = false;
  if (displayReady) {
    display.clearDisplay();
    display.display();
  }
}

// ========================
//  MAIN HARDWARE LOOP
// ========================
void manageSuperCircuit() {
  updateButtons();

#if ENABLE_DFPLAYER
  updateVolumePot();
  pollTrackEnded();
#endif

  if (overlayActive && (millis() - overlayStartMs >= overlayDurationMs)) {
    overlayActive = false;
  }

  updateStatusPixels();
  drawScreen();
}

// ========================
//  STATE SETTERS
// ========================
void setSystemEnabled(bool enabled) {
  gSystemEnabled = enabled;
  showAlert("SYS", enabled ? "Enabled" : "Disabled", 1000);
  if (gSystemEnabled) {
    initializeSettings();
    backSettings.brightness = 255;
    frontSettings.brightness = 128;
  } else {
    backSettings.brightness = 0;
    frontSettings.brightness = 0;
  }
}

void setWifiStatus(bool connected, const IPAddress& ip) {
  gWifiConnected = connected;
  if (!connected) {
    wifiLine = "WiFi: offline";
  } else {
    wifiLine = "WiFi: ";
    wifiLine += ip.toString();
    //wifiLine += gTcpConnected ? " TCP:ON" : " TCP:OFF";
  }
}

void setTcpStatus(bool connected) {
  gTcpConnected = connected;
  if (gWifiConnected) {
    setWifiStatus(true, WiFi.localIP());
  }
}

// ========================
//  ALERTS
// ========================
void showAlert(const String& header, const String& body,
               unsigned long durationMs) {
  overlayHeader = header;
  overlayText = body;
  overlayDurationMs = durationMs;
  overlayStartMs = millis();
  overlayActive = true;
}

// ========================
//  DFPLAYER HELPERS
// ========================
void setupDFplayer() {
#if ENABLE_DFPLAYER
  DFSerial.begin(115200, SERIAL_8N1, DF_RX_PIN, DF_TX_PIN);
  if (DF1201S.begin(DFSerial)) {
    DF1201S.setLED(true);
    DF1201S.setPrompt(false);
    DF1201S.switchFunction(DF1201S.MUSIC);
    DF1201S.setPlayMode(DF1201S.SINGLE);
    DF1201S.setVol(15);
    DF1201S.disableAMP();
    DF1201S.playFileNum(1);
    //delay(500);
    DF1201S.pause();
    DF1201S.playFileNum(3);
    DF1201S.pause();
    updateVolumePot();
    DF1201S.enableAMP();
    DF1201S.setLED(false);
    dfReady = true;
  } else {
    dfReady = false;
    logLine(F("DF1201S not found; audio disabled."));
  }
#endif
}

#if ENABLE_DFPLAYER
void audioPlayFile(uint16_t fileNum) {
  if (!dfReady) return;
  DF1201S.playFileNum(fileNum);
  noteTrackStarted(fileNum);
}

void audioStop() {
  if (!dfReady) return;
  DF1201S.pause();
  dfIsPlaying = false;
  showAlert("Audio", "Stop", 800);
}

void audioSetVolume(uint8_t vol) {
  if (!dfReady) return;
  if (vol > 30) vol = 30;
  DF1201S.setVol(vol);
}

void showCurrentTrack() {
  if (!dfReady) return;

  String name = DF1201S.getFileName();
  if (name.length() > 0) {
    currentTrackName = name;
  } else {
    currentTrackName = "Track " + String(lastPlayedFile);
  }

  showAlert("Play", currentTrackName, 2000);

  Serial.print(F("Now playing: "));
  Serial.println(currentTrackName);
}

void noteTrackStarted(uint16_t fileNum) {
  lastPlayedFile = fileNum;
  dfIsPlaying = true;
  currentTrackStartMs = millis();
  lastCurTimeSec = 0;
  currentTrackTotalSec = 0;
  inferredTrackEndMs = 0;
  lastDfTimePollMs = 0;

  showCurrentTrack();
}

void pollTrackEnded() {
  if (!dfReady || !dfIsPlaying) return;

  unsigned long now = millis();
  if (now - lastDfTimePollMs < DF_TIME_POLL_INTERVAL_MS) return;
  lastDfTimePollMs = now;

  uint16_t cur = DF1201S.getCurTime();
  uint16_t total = DF1201S.getTotalTime();

  if (total > 0 && currentTrackTotalSec == 0) {
    currentTrackTotalSec = total;
    inferredTrackEndMs = currentTrackStartMs + (unsigned long)total * 1000UL + 1000UL;
  }

  if (currentTrackTotalSec == 0 && total == 0) {
    lastCurTimeSec = cur;
    return;
  }

  bool ended = false;

  if (inferredTrackEndMs > 0 && now >= inferredTrackEndMs) {
    ended = true;
  }

  if (!ended && lastCurTimeSec > 0 && cur == 0 && (now - currentTrackStartMs) > 1000) {
    ended = true;
  }

  lastCurTimeSec = cur;

  if (ended) {
    dfIsPlaying = false;
    currentTrackTotalSec = 0;
    inferredTrackEndMs = 0;
    trackEnded(lastPlayedFile);
  }
}

void trackEnded(uint16_t trackNum) {
  String body;
  if (currentTrackName.length() > 0) {
    body = currentTrackName;
  } else {
    body = "Track " + String(trackNum);
  }

  showAlert("Done", body, 1500);

  Serial.print(F("Track ended: "));
  Serial.println(trackNum);
}
#endif  // ENABLE_DFPLAYER

// ========================
//  INTERNAL: WIFI LINE (placeholder)
// ========================
static void updateWifiLine() {
  // wifiLine is built in setWifiStatus()
}

// ========================
//  INTERNAL: UPTIME LINE
// ========================
static String buildUptimeLine() {
  unsigned long ms = millis() - bootMillis;
  unsigned long secs = (ms / 1000) % 60;
  unsigned long mins = (ms / 60000) % 60;
  unsigned long hrs = (ms / 3600000);

  char buf[24];
  snprintf(buf, sizeof(buf), "Uptime:%lu:%02lu:%02lu", hrs, mins, secs);
  return String(buf);
}

// ========================
//  INTERNAL: SCREEN
// ========================
static void drawScreen() {
  if (!displayReady) return;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextWrap(false);

  // Row 0: Title
  display.setCursor(0, 0);
  display.print(titleLine);

  // Row 1: WiFi / TCP
  display.setCursor(0, 8);
  display.print(wifiLine);

  // Row 2: System ON/OFF
  display.setCursor(0, 16);
  String systemLine = gSystemEnabled ? "System: ON," : "System: OFF,";
  systemLine += gTcpConnected ? "  TCP:ON" : "  TCP:OFF";
  display.print(systemLine);

  // Row 3: Alert OR uptime
  display.setCursor(0, 24);
  if (overlayActive) {
    if (overlayHeader.length()) {
      display.print(overlayHeader);
      display.print(": ");
    }
    display.print(overlayText);
  } else {
    display.print(buildUptimeLine());
  }

  display.display();
}




// ========================
//  TEXT WIDTH + SCROLLING
// ========================
int16_t textPixelWidth(const String& s, uint8_t size) {
  return (int16_t)s.length() * 6 * size;  // 5px glyph + 1px space
}

void drawScrollingLine(int16_t y, const String& s, uint8_t size,
                       int16_t& scrollX, unsigned long& lastStep) {
  const int16_t w = textPixelWidth(s, size);
  display.setTextSize(size);

  if (w <= SCREEN_WIDTH) {
    display.setCursor(0, y);
    display.print(s);
    return;
  }

  unsigned long now = millis();
  unsigned long interval = (scrollX == 0) ? SCROLL_HOLD_MS : SCROLL_STEP_MS;

  if (now - lastStep >= interval) {
    lastStep = now;
    scrollX -= 1;

    const int16_t resetPoint = -w - SCROLL_GAP;
    if (scrollX < resetPoint) {
      scrollX = 0;
      lastStep = now;
    }
  }

  display.setCursor(scrollX, y);
  display.print(s);
  display.setCursor(scrollX + w + SCROLL_GAP, y);
  display.print(s);
}

// ========================
//  INTERNAL: BUTTONS
// ========================
static void setupButtons() {
  uint8_t pins[NUM_BUTTONS] = { BUTTON_A_PIN, BUTTON_B_PIN, BUTTON_C_PIN };
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    pinMode(pins[i], INPUT);  // external pulldown
    buttonDebounce[i].attach(pins[i], INPUT);
    buttonDebounce[i].interval(10);
  }
  buttonsReady = true;
}

static void updateButtons() {
  if (!buttonsReady) return;

  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    buttonDebounce[i].update();

    // Active-high press: LOW -> HIGH
    if (buttonDebounce[i].rose() && BUTTON_ACTIVE_STATE == HIGH) {
      switch (i) {
        case 0:
          showAlert("BTN", "Button 1", 800);
          button1Pressed();
          break;
        case 1:
          showAlert("BTN", "Button 2", 800);
          button2Pressed();
          break;
        case 2:
          showAlert("BTN", "Button 3", 800);
          button3Pressed();
          break;
      }
    }
  }
}

#if ENABLE_DFPLAYER
static void updateVolumePot() {
  if (!potsReady || !dfReady) return;

  static unsigned long lastReadMs = 0;
  static unsigned long lastAlertMs = 0;
  static int lastRaw = -1;  // last raw ADC reading

  unsigned long now = millis();

  // Read pot at ~20 Hz
  if (now - lastReadMs < 50) return;
  lastReadMs = now;

  int raw = analogRead(POT_VOL_PIN);

  // Initialize lastRaw on first run
  if (lastRaw < 0) {
    lastRaw = raw;
  }

  // --- MINIMUM TURN THRESHOLD ---
  // Only consider it a "real" movement if the raw ADC value
  // has changed by at least this amount.
  //
  // 4095 / 80 ≈ 51 "steps" around the knob; tweak as needed.
  const int RAW_THRESHOLD = 80;
  int diff = abs(raw - lastRaw);
  if (diff < RAW_THRESHOLD) {
    // Ignore tiny changes / noise
    return;
  }

  // We've moved enough: accept this as the new position
  lastRaw = raw;

  // Map to DFPlayer volume (0–30)
  int vol = map(raw, 0, 4095, 0, 30);
  vol = constrain(vol, 0, 30);

  // Only react when the mapped volume actually changes
  if (vol != pot1Reported) {
    pot1Reported = vol;

    // 1) Update DFPlayer volume immediately (snappy audio)
    audioSetVolume(vol);

    // 2) Show VOL alert, but rate-limit so it doesn't flicker with uptime
    //    duration (600 ms) > throttle interval (150 ms) to avoid gaps
    if (now - lastAlertMs > 150) {
      lastAlertMs = now;
      showAlert("VOL", String(vol), 600);
    }
  }
}
#else
static void updateVolumePot() {
  // No DFPlayer compiled in; nothing to do
}
#endif



// ========================
//  INTERNAL: STATUS PIXELS
// ========================
static void updateStatusPixels() {
  if (!pixelsReady) return;

  // Pixel 0: system enabled
  uint32_t c0 = gSystemEnabled ? pixels.Color(0, 40, 0)
                               : pixels.Color(40, 0, 0);
  pixels.setPixelColor(0, c0);

  // Pixel 1: WiFi/TCP state
  uint32_t c1;
  if (!gWifiConnected) {
    c1 = pixels.Color(40, 20, 0);  // amber: no WiFi
  } else if (gWifiConnected && !gTcpConnected) {
    c1 = pixels.Color(0, 0, 40);  // blue: WiFi only
  } else {
    c1 = pixels.Color(0, 40, 40);  // cyan: WiFi + TCP
  }
  pixels.setPixelColor(1, c1);

  // Pixel 2: reserved for app use (default off)
  pixels.setPixelColor(2, 0);

  pixels.show();
}

uint32_t wheel(byte pos) {
  pos = 255 - pos;
  if (pos < 85) return pixels.Color(255 - pos * 3, 0, pos * 3);
  if (pos < 170) {
    pos -= 85;
    return pixels.Color(0, pos * 3, 255 - pos * 3);
  }
  pos -= 170;
  return pixels.Color(pos * 3, 255 - pos * 3, 0);
}

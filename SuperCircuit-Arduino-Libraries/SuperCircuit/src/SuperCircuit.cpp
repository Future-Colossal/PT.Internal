#include "SuperCircuit.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>
#include <DFRobot_DF1201S.h>

namespace {
constexpr uint8_t kNumPixels = 3;
constexpr uint8_t kPixelPin = 13;
constexpr uint8_t kBuiltInPixelPin = 0;

constexpr uint8_t kButtonPins[3] = {37, 27, 34};
constexpr uint8_t kNumButtons = 3;

constexpr uint8_t kPotVolPin = 39;

constexpr uint8_t kScreenWidth = 128;
constexpr uint8_t kScreenHeight = 32;
constexpr int8_t kOledResetPin = -1;
constexpr uint8_t kOledI2cAddr = 0x3C;

constexpr int kDfRxPin = 7;
constexpr int kDfTxPin = 8;

constexpr unsigned long kVolumePotSampleMs = 50;
constexpr int kVolumePotNoiseThreshold = 80;
constexpr unsigned long kVolumePotAlertCooldownMs = 150;
constexpr unsigned long kOverlayScrollStepMs = 40;
constexpr int16_t kOverlayScrollGapPx = 16;

uint32_t wheel(Adafruit_NeoPixel& pixels, uint8_t pos) {
  pos = 255 - pos;
  if (pos < 85) return pixels.Color(255 - pos * 3, 0, pos * 3);
  if (pos < 170) {
    pos -= 85;
    return pixels.Color(0, pos * 3, 255 - pos * 3);
  }
  pos -= 170;
  return pixels.Color(pos * 3, 255 - pos * 3, 0);
}
}

class SuperCircuit::SuperCircuitImpl {
public:
  Adafruit_NeoPixel pixels{kNumPixels, kPixelPin, NEO_GRB + NEO_KHZ800};
  Adafruit_NeoPixel builtInPixel{1, kBuiltInPixelPin, NEO_GRB + NEO_KHZ800};
  Adafruit_SSD1306 display{kScreenWidth, kScreenHeight, &Wire, kOledResetPin};
  Bounce buttons[kNumButtons];
  HardwareSerial* dfSerial = &Serial1;
  DFRobot_DF1201S dfPlayer;

  bool displayReady = false;
  bool pixelsReady = false;
  bool buttonsReady = false;
  bool dfReady = false;

  bool overlayActive = false;
  String overlayHeader;
  String overlayBody;
  unsigned long overlayStartMs = 0;
  unsigned long overlayDurationMs = 0;
  unsigned long lastScreenRefreshMs = 0;

  uint32_t pixelLanes[kNumPixels] = {0, 0, 0};
  uint8_t currentVolume = 15;
  bool dfIsPlaying = false;
  uint16_t lastTrack = 0;
  String currentTrackName;

  unsigned long lastVolumePotReadMs = 0;
  unsigned long lastVolumePotAlertMs = 0;
  int lastVolumePotRaw = -1;
};

SuperCircuit::SuperCircuit() = default;

bool SuperCircuit::featureEnabled(SuperCircuitToggle toggle) const {
  return toggle == SuperCircuitToggle::Enabled;
}

uint32_t SuperCircuit::packRgb(uint8_t red, uint8_t green, uint8_t blue) const {
  return (static_cast<uint32_t>(red) << 16) |
         (static_cast<uint32_t>(green) << 8) |
         static_cast<uint32_t>(blue);
}

uint16_t SuperCircuit::startupStepDurationMs() const {
  switch (_config.startup.speed) {
    case SuperCircuitStartupSpeed::Slow:
      return 2000;
    case SuperCircuitStartupSpeed::Custom:
      return _config.startup.customStepDurationMs > 0 ? _config.startup.customStepDurationMs : 200;
    case SuperCircuitStartupSpeed::Fast:
    default:
      return 200;
  }
}

uint16_t SuperCircuit::alertDurationOrDefault(uint32_t durationMs) const {
  if (durationMs > 0) {
    return static_cast<uint16_t>(durationMs);
  }
  return _config.display.defaultAlertDurationMs;
}

String SuperCircuit::buildUptimeLine() const {
  unsigned long ms = millis();
  unsigned long secs = (ms / 1000UL) % 60UL;
  unsigned long mins = (ms / 60000UL) % 60UL;
  unsigned long hrs = ms / 3600000UL;

  char buf[24];
  snprintf(buf, sizeof(buf), "Uptime:%lu:%02lu:%02lu", hrs, mins, secs);
  return String(buf);
}

void SuperCircuit::showStartupLine(const String& line, bool finalStatusKnown, bool ok) {
  const uint16_t stepDurationMs = startupStepDurationMs();

  if (!_impl || !_impl->displayReady || !featureEnabled(_config.features.screen)) {
    Serial.println("[SuperCircuit] " + line);
    if (finalStatusKnown) {
      Serial.println(String("[SuperCircuit] status=") + (ok ? "OK" : "FAIL"));
    }
    if (stepDurationMs > 0) delay(stepDurationMs);
    return;
  }

  _impl->display.clearDisplay();
  _impl->display.setTextSize(1);
  _impl->display.setTextColor(SSD1306_WHITE);
  _impl->display.setCursor(0, 0);
  _impl->display.println(F("Initializing..."));
  _impl->display.setCursor(0, 10);
  _impl->display.print(line);

  if (finalStatusKnown) {
    _impl->display.setCursor(0, 20);
    _impl->display.print(F("Status: "));
    _impl->display.print(ok ? F("OK") : F("FAIL"));
  }

  _impl->display.display();

  if (_impl->pixelsReady && featureEnabled(_config.features.pixels) && finalStatusKnown) {
    const uint32_t color = ok ? packRgb(0, 40, 0) : packRgb(40, 0, 0);
    _impl->pixels.setPixelColor(0, _impl->pixels.Color((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF));
    _impl->pixels.show();
    _impl->pixels.clear();
    _impl->pixels.show();
  }

  if (stepDurationMs > 0) delay(stepDurationMs);
}

void SuperCircuit::runStartupSequence() {
  if (!featureEnabled(_config.startup.runSequence)) return;

  if (featureEnabled(_config.startup.screenSequence) && featureEnabled(_config.startup.showDisplayCheck)) {
    showStartupLine("Display", true, _impl->displayReady);
  }
  if (featureEnabled(_config.startup.screenSequence) && featureEnabled(_config.startup.showPixelCheck)) {
    showStartupLine("Pixels", true, _impl->pixelsReady);
  }
  if (featureEnabled(_config.startup.screenSequence) && featureEnabled(_config.startup.showButtonCheck)) {
    showStartupLine("Buttons", true, _impl->buttonsReady);
  }
  if (featureEnabled(_config.startup.screenSequence) &&
      featureEnabled(_config.startup.showAudioCheck) &&
      featureEnabled(_config.features.dfPlayer)) {
    showStartupLine("DFPlayer", true, _impl->dfReady);
  }
  if (featureEnabled(_config.startup.screenSequence) && featureEnabled(_config.startup.showWifiPending)) {
    showStartupLine(_config.text.wifiPendingLine ? String(_config.text.wifiPendingLine) : String("WiFi: pending"), false, false);
  }

  if (_impl->pixelsReady &&
      featureEnabled(_config.features.pixels) &&
      featureEnabled(_config.startup.ledRainbow)) {
    const uint16_t rainbowDelayMs = (_config.startup.speed == SuperCircuitStartupSpeed::Slow) ? 15 : 5;
    for (uint16_t pos = 0; pos < 256; pos++) {
      for (uint8_t i = 0; i < kNumPixels; i++) {
        const uint8_t p = (pos + (i * 10)) & 0xFF;
        _impl->pixels.setPixelColor(i, wheel(_impl->pixels, p));
      }
      _impl->builtInPixel.setPixelColor(0, wheel(_impl->builtInPixel, pos));
      _impl->pixels.show();
      _impl->builtInPixel.show();
      delay(rainbowDelayMs);
    }
  }
}

void SuperCircuit::syncWifiLine() {
  if (_wifiStatusMessage.length() > 0) {
    _wifiLine = _wifiStatusMessage;
    return;
  }

  if (!_wifiConnected) {
    _wifiLine = _config.text.wifiDisconnectedLine ? String(_config.text.wifiDisconnectedLine) : String("WiFi: not connected");
    return;
  }

  if (_wifiIp != IPAddress()) {
    _wifiLine = "WiFi: " + _wifiIp.toString();
    return;
  }

  _wifiLine = "WiFi: connected";
}

String SuperCircuit::buildOverlayLine() const {
  if (!_impl) return String("");
  if (_impl->overlayHeader.length() == 0) return _impl->overlayBody;
  if (_impl->overlayBody.length() == 0) return _impl->overlayHeader;
  return _impl->overlayHeader + ": " + _impl->overlayBody;
}

int16_t SuperCircuit::textPixelWidth(const String& text) {
  if (!_impl) return 0;
  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t w = 0;
  uint16_t h = 0;
  _impl->display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  return static_cast<int16_t>(w);
}

void SuperCircuit::drawScrollingLine(uint8_t y, const String& line, unsigned long elapsedMs) {
  if (!_impl) return;

  const int16_t width = textPixelWidth(line);
  if (width <= kScreenWidth) {
    _impl->display.setCursor(0, y);
    _impl->display.print(line);
    return;
  }

  const int16_t cycleWidth = width + kOverlayScrollGapPx + kScreenWidth;
  const int16_t progress = static_cast<int16_t>((elapsedMs / kOverlayScrollStepMs) % cycleWidth);
  const int16_t x = kScreenWidth - progress;

  _impl->display.setCursor(x, y);
  _impl->display.print(line);

  const int16_t repeatX = x + width + kOverlayScrollGapPx;
  if (repeatX < kScreenWidth) {
    _impl->display.setCursor(repeatX, y);
    _impl->display.print(line);
  }
}

void SuperCircuit::applyPixels() {
  if (!_impl || !_impl->pixelsReady || !featureEnabled(_config.features.pixels)) return;

  _impl->pixelLanes[0] = _isOn ? _config.pixel.interactiveOnColor : _config.pixel.interactiveOffColor;

  if (!_wifiConnected) {
    _impl->pixelLanes[1] = _config.pixel.commsOfflineColor;
  } else if (!_tcpConnected) {
    _impl->pixelLanes[1] = _config.pixel.commsWifiOnlyColor;
  } else {
    _impl->pixelLanes[1] = _config.pixel.commsTcpReadyColor;
  }

  for (uint8_t i = 0; i < kNumPixels; i++) {
    const uint32_t color = _impl->pixelLanes[i];
    _impl->pixels.setPixelColor(i, _impl->pixels.Color((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF));
  }

  const uint32_t commsColor = _impl->pixelLanes[1];
  _impl->builtInPixel.setPixelColor(
    0,
    _impl->builtInPixel.Color((commsColor >> 16) & 0xFF, (commsColor >> 8) & 0xFF, commsColor & 0xFF)
  );

  _impl->pixels.show();
  _impl->builtInPixel.show();
}

void SuperCircuit::drawScreen() {
  if (!_impl || !_impl->displayReady || !featureEnabled(_config.features.screen)) return;

  _impl->display.clearDisplay();
  _impl->display.setTextColor(SSD1306_WHITE);
  _impl->display.setTextSize(1);
  _impl->display.setTextWrap(false);

  _impl->display.setCursor(0, 0);
  _impl->display.print(_title);

  _impl->display.setCursor(0, 8);
  _impl->display.print(_wifiLine);

  _impl->display.setCursor(0, 16);
  String systemLine = _isOn ? "System: ON," : "System: OFF,";
  systemLine += _tcpConnected ? "  TCP:ON" : "  TCP:OFF";
  _impl->display.print(systemLine);

  _impl->display.setCursor(0, 24);
  if (_impl->overlayActive && featureEnabled(_config.display.allowAlerts)) {
    drawScrollingLine(24, buildOverlayLine(), millis() - _impl->overlayStartMs);
  } else if (_bottomLine.length() > 0) {
    _impl->display.print(_bottomLine);
  } else if (featureEnabled(_config.display.showUptimeWhenIdle)) {
    _impl->display.print(buildUptimeLine());
  }

  _impl->display.display();
}

bool SuperCircuit::begin(const SuperCircuitConfig& config) {
  _config = config;
  _begun = true;
  _isOn = config.defaultInteractiveEnabled;
  _wifiConnected = false;
  _tcpConnected = false;
  _wifiIp = IPAddress();
  _wifiStatusMessage = "";
  _title = config.defaultTitle ? String(config.defaultTitle) : String("SuperCircuit");
  _bottomLine = config.defaultBottomLine ? String(config.defaultBottomLine) : String("");
  _wifiLine = config.text.wifiDisconnectedLine ? String(config.text.wifiDisconnectedLine) : String("WiFi: not connected");
  _userPixelColor = config.pixel.userDefaultColor;

  if (!_impl) {
    _impl = new SuperCircuitImpl();
  }

  _impl->currentVolume = config.audio.defaultVolume;
  _impl->dfIsPlaying = false;
  _impl->lastTrack = 0;
  _impl->currentTrackName = "";
  _impl->overlayActive = false;
  _impl->overlayHeader = "";
  _impl->overlayBody = "";
  _impl->lastScreenRefreshMs = 0;
  _impl->lastVolumePotReadMs = 0;
  _impl->lastVolumePotAlertMs = 0;
  _impl->lastVolumePotRaw = -1;

  if (featureEnabled(_config.features.screen)) {
    Wire.begin();
    if (_impl->display.begin(SSD1306_SWITCHCAPVCC, kOledI2cAddr)) {
      _impl->displayReady = true;
      _impl->display.setTextWrap(false);
    } else {
      _impl->displayReady = false;
      Serial.println(F("[SuperCircuit] SSD1306 init failed"));
    }
  } else {
    _impl->displayReady = false;
  }

  if (featureEnabled(_config.features.pixels)) {
    _impl->builtInPixel.begin();
    _impl->builtInPixel.setBrightness(_config.pixel.builtInBrightness);
    _impl->builtInPixel.clear();
    _impl->builtInPixel.show();

    _impl->pixels.begin();
    _impl->pixels.setBrightness(_config.pixel.externalBrightness);
    _impl->pixels.clear();
    _impl->pixels.show();

    _impl->pixelsReady = true;
  } else {
    _impl->pixelsReady = false;
  }

  for (uint8_t i = 0; i < kNumButtons; i++) {
    pinMode(kButtonPins[i], INPUT);
    _impl->buttons[i].attach(kButtonPins[i], INPUT);
    _impl->buttons[i].interval(10);
  }
  _impl->buttonsReady = true;

  if (featureEnabled(_config.features.dfPlayer)) {
    _impl->dfSerial->begin(115200, SERIAL_8N1, kDfRxPin, kDfTxPin);
    if (_impl->dfPlayer.begin(*_impl->dfSerial)) {
      _impl->dfPlayer.setLED(true);
      _impl->dfPlayer.setPrompt(false);
      _impl->dfPlayer.switchFunction(_impl->dfPlayer.MUSIC);
      if (_config.audio.defaultMode == SuperCircuitAudioMode::Loop) {
        _impl->dfPlayer.setPlayMode(_impl->dfPlayer.ALLCYCLE);
      } else {
        _impl->dfPlayer.setPlayMode(_impl->dfPlayer.SINGLE);
      }
      _impl->dfPlayer.setVol(_impl->currentVolume);
      if (featureEnabled(_config.audio.amplifierEnabled)) {
        _impl->dfPlayer.enableAMP();
      } else {
        _impl->dfPlayer.disableAMP();
      }
      _impl->dfPlayer.setLED(false);
      _impl->dfReady = true;
      pinMode(kPotVolPin, INPUT);
    } else {
      _impl->dfReady = false;
      Serial.println(F("[SuperCircuit] DFPlayer init failed"));
    }
  } else {
    _impl->dfReady = false;
  }

  _impl->pixelLanes[2] = _userPixelColor;

  runStartupSequence();
  syncWifiLine();
  applyPixels();
  drawScreen();

  if (featureEnabled(_config.startup.showReadyAlert)) {
    showAlert(
      _config.text.readyAlertTitle ? String(_config.text.readyAlertTitle) : String("READY"),
      _config.text.readyAlertBody ? String(_config.text.readyAlertBody) : String("SuperCircuit ready")
    );
  }

  return true;
}

void SuperCircuit::update() {
  if (!_begun || !_impl) return;

  if (_impl->buttonsReady) {
    for (uint8_t i = 0; i < kNumButtons; i++) {
      _impl->buttons[i].update();
      if (_impl->buttons[i].rose() && _buttonHandler) {
        _buttonHandler(i + 1);
      }
    }
  }

  const unsigned long now = millis();
  if (_impl->overlayActive && (now - _impl->overlayStartMs >= _impl->overlayDurationMs)) {
    _impl->overlayActive = false;
  }

  if (_impl->dfReady &&
      featureEnabled(_config.features.volumePot) &&
      featureEnabled(_config.audio.followVolumePot)) {
    if (now - _impl->lastVolumePotReadMs >= kVolumePotSampleMs) {
      _impl->lastVolumePotReadMs = now;
      const int raw = analogRead(kPotVolPin);

      if (_impl->lastVolumePotRaw < 0) {
        _impl->lastVolumePotRaw = raw;
      }

      if (abs(raw - _impl->lastVolumePotRaw) >= kVolumePotNoiseThreshold) {
        _impl->lastVolumePotRaw = raw;
        const uint8_t mappedVolume = static_cast<uint8_t>(
          constrain(map(raw, 0, 4095, _config.audio.minVolume, _config.audio.maxVolume),
                    _config.audio.minVolume,
                    _config.audio.maxVolume)
        );
        if (mappedVolume != _impl->currentVolume) {
          setVolume(mappedVolume);
          if (now - _impl->lastVolumePotAlertMs >= kVolumePotAlertCooldownMs) {
            _impl->lastVolumePotAlertMs = now;
            showAlert("VOL", String(mappedVolume), 600);
          }
        }
      }
    }
  }

  syncWifiLine();
  applyPixels();

  if (_impl->displayReady && (now - _impl->lastScreenRefreshMs >= _config.display.refreshIntervalMs)) {
    _impl->lastScreenRefreshMs = now;
    drawScreen();
  }
}

void SuperCircuit::onButtonPressed(SuperCircuitButtonHandler handler) {
  _buttonHandler = handler;
}

void SuperCircuit::onTurnOn(SuperCircuitStateHandler handler) {
  _turnOnHandler = handler;
}

void SuperCircuit::onTurnOff(SuperCircuitStateHandler handler) {
  _turnOffHandler = handler;
}

void SuperCircuit::setEnabled(bool enabled) {
  if (enabled) {
    turnOn();
  } else {
    turnOff();
  }
}

void SuperCircuit::turnOn() {
  if (_isOn) return;
  _isOn = true;
  if (_bottomLine.length() == 0) {
    _bottomLine = _config.text.interactiveOnLine ? String(_config.text.interactiveOnLine) : String("Interactive ON");
  }
  showAlert(
    _config.text.turnOnAlertTitle ? String(_config.text.turnOnAlertTitle) : String("SYS"),
    _config.text.turnOnAlertBody ? String(_config.text.turnOnAlertBody) : String("Enabled")
  );
  if (_turnOnHandler) _turnOnHandler();
}

void SuperCircuit::turnOff() {
  if (!_isOn) return;
  _isOn = false;
  if (_bottomLine.length() == 0) {
    _bottomLine = _config.text.interactiveOffLine ? String(_config.text.interactiveOffLine) : String("Interactive OFF");
  }
  showAlert(
    _config.text.turnOffAlertTitle ? String(_config.text.turnOffAlertTitle) : String("SYS"),
    _config.text.turnOffAlertBody ? String(_config.text.turnOffAlertBody) : String("Disabled")
  );
  if (_turnOffHandler) _turnOffHandler();
}

bool SuperCircuit::isOn() const {
  return _isOn;
}

void SuperCircuit::playTrack(uint16_t track) {
  if (!_impl || !_impl->dfReady) return;
  _impl->dfPlayer.playFileNum(track);
  _impl->dfIsPlaying = true;
  _impl->lastTrack = track;
  _impl->currentTrackName = String(_config.text.audioTrackPrefix ? _config.text.audioTrackPrefix : "Track ") + String(track);
  showAlert("AUDIO", "PLAY " + String(track), 1000);
}

void SuperCircuit::stopTrack() {
  if (!_impl || !_impl->dfReady) return;
  _impl->dfPlayer.pause();
  _impl->dfIsPlaying = false;
  showAlert("AUDIO", "STOP", 800);
}

void SuperCircuit::setVolume(uint8_t volume) {
  if (!_impl) return;
  volume = constrain(volume, _config.audio.minVolume, _config.audio.maxVolume);
  _impl->currentVolume = volume;
  if (_impl->dfReady) {
    _impl->dfPlayer.setVol(volume);
  }
}

uint8_t SuperCircuit::volume() const {
  return _impl ? _impl->currentVolume : _config.audio.defaultVolume;
}

bool SuperCircuit::isAudioReady() const {
  return _impl ? _impl->dfReady : false;
}

bool SuperCircuit::isTrackPlaying() const {
  return _impl ? _impl->dfIsPlaying : false;
}

uint16_t SuperCircuit::lastTrack() const {
  return _impl ? _impl->lastTrack : 0;
}

String SuperCircuit::currentTrackName() const {
  return _impl ? _impl->currentTrackName : String("");
}

void SuperCircuit::setAudioModeSingle() {
  if (_impl && _impl->dfReady) {
    _impl->dfPlayer.setPlayMode(_impl->dfPlayer.SINGLE);
  }
}

void SuperCircuit::setAudioModeLoop() {
  if (_impl && _impl->dfReady) {
    _impl->dfPlayer.setPlayMode(_impl->dfPlayer.ALLCYCLE);
  }
}

void SuperCircuit::enableAmplifier() {
  if (_impl && _impl->dfReady) {
    _impl->dfPlayer.enableAMP();
  }
}

void SuperCircuit::disableAmplifier() {
  if (_impl && _impl->dfReady) {
    _impl->dfPlayer.disableAMP();
  }
}

void SuperCircuit::showAlert(const String& title, const String& body, uint32_t durationMs) {
  if (!_impl || !featureEnabled(_config.display.allowAlerts)) return;
  _impl->overlayHeader = title;
  _impl->overlayBody = body;
  uint32_t resolvedDurationMs = alertDurationOrDefault(durationMs);
  const String overlayLine = buildOverlayLine();
  if (_impl->displayReady) {
    const int16_t width = textPixelWidth(overlayLine);
    if (width > kScreenWidth) {
      const uint32_t minimumScrollDurationMs =
        static_cast<uint32_t>(width + kOverlayScrollGapPx + kScreenWidth) * kOverlayScrollStepMs;
      if (resolvedDurationMs < minimumScrollDurationMs) {
        resolvedDurationMs = minimumScrollDurationMs;
      }
    }
  }
  _impl->overlayDurationMs = resolvedDurationMs;
  _impl->overlayStartMs = millis();
  _impl->overlayActive = true;
}

void SuperCircuit::clearAlert() {
  if (!_impl) return;
  _impl->overlayActive = false;
  _impl->overlayHeader = "";
  _impl->overlayBody = "";
}

void SuperCircuit::setTitle(const String& title) {
  _title = title;
}

String SuperCircuit::title() const {
  return _title;
}

void SuperCircuit::setBottomLine(const String& line) {
  _bottomLine = line;
}

void SuperCircuit::clearBottomLine() {
  _bottomLine = "";
}

String SuperCircuit::bottomLine() const {
  return _bottomLine;
}

String SuperCircuit::wifiLine() const {
  return _wifiLine;
}

void SuperCircuit::setInteractivePalette(uint32_t onColor, uint32_t offColor) {
  _config.pixel.interactiveOnColor = onColor;
  _config.pixel.interactiveOffColor = offColor;
}

void SuperCircuit::setCommsPalette(uint32_t offlineColor, uint32_t wifiOnlyColor, uint32_t tcpReadyColor) {
  _config.pixel.commsOfflineColor = offlineColor;
  _config.pixel.commsWifiOnlyColor = wifiOnlyColor;
  _config.pixel.commsTcpReadyColor = tcpReadyColor;
}

void SuperCircuit::setPixelBrightness(uint8_t externalBrightness, uint8_t builtInBrightness) {
  _config.pixel.externalBrightness = externalBrightness;
  _config.pixel.builtInBrightness = builtInBrightness;

  if (_impl && _impl->pixelsReady) {
    _impl->pixels.setBrightness(externalBrightness);
    _impl->builtInPixel.setBrightness(builtInBrightness);
    _impl->pixels.show();
    _impl->builtInPixel.show();
  }
}

void SuperCircuit::setWifiStatus(bool connected, const IPAddress& ip) {
  _wifiConnected = connected;
  if (!connected) {
    _wifiIp = IPAddress();
    _wifiStatusMessage = "";
    _wifiLine = _config.text.wifiDisconnectedLine ? String(_config.text.wifiDisconnectedLine) : String("WiFi: not connected");
  } else {
    _wifiIp = ip;
    _wifiStatusMessage = "";
    syncWifiLine();
  }
}

void SuperCircuit::setWifiStatus(bool connected, const String& message) {
  _wifiConnected = connected;
  if (message.length() > 0) {
    _wifiStatusMessage = message;
    _wifiLine = message;
  } else {
    syncWifiLine();
  }
}

void SuperCircuit::setWifiStatusMessage(const String& message) {
  _wifiStatusMessage = message;
  syncWifiLine();
}

void SuperCircuit::clearWifiStatusMessage() {
  _wifiStatusMessage = "";
  syncWifiLine();
}

void SuperCircuit::setTcpConnected(bool connected) {
  _tcpConnected = connected;
}

void SuperCircuit::setCommsStatus(bool wifiConnected, bool tcpConnected) {
  _wifiConnected = wifiConnected;
  _tcpConnected = tcpConnected;
  if (!wifiConnected) {
    _wifiIp = IPAddress();
  }
  syncWifiLine();
}

bool SuperCircuit::isWifiConnected() const {
  return _wifiConnected;
}

bool SuperCircuit::isTcpConnected() const {
  return _tcpConnected;
}

void SuperCircuit::setPixelLaneColor(uint8_t lane, uint32_t color) {
  if (!_impl || lane >= kNumPixels) return;
  _impl->pixelLanes[lane] = color;
  if (lane == 2) {
    _userPixelColor = color;
  }
}

void SuperCircuit::setUserPixelColor(uint32_t color) {
  _userPixelColor = color;
  if (_impl) {
    _impl->pixelLanes[2] = color;
  }
}

void SuperCircuit::setUserPixelRgb(uint8_t red, uint8_t green, uint8_t blue) {
  setUserPixelColor(packRgb(red, green, blue));
}

uint32_t SuperCircuit::userPixelColor() const {
  return _userPixelColor;
}

bool SuperCircuit::isDisplayReady() const {
  return _impl ? _impl->displayReady : false;
}

bool SuperCircuit::arePixelsReady() const {
  return _impl ? _impl->pixelsReady : false;
}

bool SuperCircuit::areButtonsReady() const {
  return _impl ? _impl->buttonsReady : false;
}

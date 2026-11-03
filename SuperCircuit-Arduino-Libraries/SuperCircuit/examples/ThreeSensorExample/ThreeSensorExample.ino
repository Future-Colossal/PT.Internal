#include <fcComms.h>
#include <SuperCircuit.h>
#include "project_config.h"

static fcComms Comms;
static SuperCircuit superCircuit;

struct FlexState { uint16_t raw = 0; bool active = false; unsigned long lastSampleMs = 0; unsigned long lastRainbowMs = 0; uint8_t phase = 0; } gFlex;
struct ButtonState { bool rawActive = false; bool stableActive = false; unsigned long lastEdgeMs = 0; } gButton;
struct ReedState { bool rawActive = false; bool stableActive = false; bool sent = false; unsigned long lastEdgeMs = 0; } gReed;

void showState();
void runBoardButtonAction(uint8_t index, ExampleBoardButtonAction action);
void onButtonPressed(uint8_t index);
void onTurnOn();
void onTurnOff();

void showHelp() {
  Comms.printlnAll("CMDS HELP PING STATE");
}

uint32_t wheelColor(uint8_t phase) {
  if (phase < 85) return superCircuit.packRgb(255 - phase * 3, 0, phase * 3);
  if (phase < 170) { phase -= 85; return superCircuit.packRgb(0, phase * 3, 255 - phase * 3); }
  phase -= 170;
  return superCircuit.packRgb(phase * 3, 255 - phase * 3, 0);
}

void sendButtonMessage() {
  Comms.printlnAll(BUTTON_MESSAGE);
  superCircuit.showAlert("BTN", BUTTON_MESSAGE, 900);
}

void sendReedMessage() {
  Comms.printlnAll(REED_MESSAGE);
  superCircuit.showAlert("REED", REED_MESSAGE, 1000);
}

void messageReceived(const String& msg, FcCommSource) {
  String upper = msg;
  upper.trim();
  upper.toUpperCase();
  if (upper == "HELP") {
    showHelp();
    return;
  }
  if (upper == "PING") Comms.printlnAll("PONG");
  if (upper == "STATE") {
    Comms.printlnAll(String("FLEX_RAW ") + gFlex.raw);
    Comms.printlnAll(String("FLEX_ACTIVE ") + (gFlex.active ? "1" : "0"));
    Comms.printlnAll(String("BUTTON ") + (gButton.stableActive ? "1" : "0"));
    Comms.printlnAll(String("REED ") + (gReed.stableActive ? "1" : "0"));
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(FLEX_SENSOR_PIN, INPUT);
  pinMode(BUTTON_SENSOR_PIN, BUTTON_INPUT_MODE);
  pinMode(REED_SWITCH_PIN, REED_INPUT_MODE);
  gFlex.raw = analogRead(FLEX_SENSOR_PIN);
  gButton.rawActive = (digitalRead(BUTTON_SENSOR_PIN) == BUTTON_ACTIVE_STATE);
  gButton.stableActive = gButton.rawActive;
  gReed.rawActive = (digitalRead(REED_SWITCH_PIN) == REED_ACTIVE_STATE);
  gReed.stableActive = gReed.rawActive;

  Comms.onMessage(messageReceived);
  Comms.begin(kCommsConfig);

  superCircuit.onButtonPressed(onButtonPressed);
  superCircuit.onTurnOn(onTurnOn);
  superCircuit.onTurnOff(onTurnOff);
  superCircuit.begin(kSuperCircuitConfig);
  superCircuit.setTitle(kDeviceTitle);
  superCircuit.setBottomLine("F:--- B:- R:-");
  superCircuit.setUserPixelColor(SHELL_USER_PIXEL_READY);
  superCircuit.showAlert("READY", "Three sensor example", 1200);

  Comms.printlnAll("THREE SENSOR EXAMPLE READY");
  showHelp();
}

void loop() {
  const unsigned long now = millis();
  Comms.update();
  if (Comms.isWifiConnected()) superCircuit.setWifiStatus(true, Comms.localIP());
  else superCircuit.setWifiStatus(false, IPAddress());
  superCircuit.setTcpConnected(Comms.isTcpConnected());

  if (superCircuit.isOn()) {
    if (now - gFlex.lastSampleMs >= FLEX_SAMPLE_INTERVAL_MS) {
      gFlex.lastSampleMs = now;
      gFlex.raw = analogRead(FLEX_SENSOR_PIN);
      if (!gFlex.active && gFlex.raw >= FLEX_TRIGGER_THRESHOLD) gFlex.active = true;
      else if (gFlex.active && gFlex.raw <= FLEX_RELEASE_THRESHOLD) gFlex.active = false;
      if (gFlex.active && now - gFlex.lastRainbowMs >= FLEX_RAINBOW_STEP_MS) { gFlex.lastRainbowMs = now; gFlex.phase += 7; }
    }

    const bool buttonRaw = (digitalRead(BUTTON_SENSOR_PIN) == BUTTON_ACTIVE_STATE);
    if (buttonRaw != gButton.rawActive) { gButton.rawActive = buttonRaw; gButton.lastEdgeMs = now; }
    if (now - gButton.lastEdgeMs >= BUTTON_DEBOUNCE_MS) {
      if (gButton.stableActive != gButton.rawActive) {
        gButton.stableActive = gButton.rawActive;
        if (gButton.stableActive) sendButtonMessage();
      }
    }

    const bool reedRaw = (digitalRead(REED_SWITCH_PIN) == REED_ACTIVE_STATE);
    if (reedRaw != gReed.rawActive) { gReed.rawActive = reedRaw; gReed.lastEdgeMs = now; }
    if (now - gReed.lastEdgeMs >= REED_DEBOUNCE_MS) {
      if (gReed.stableActive != gReed.rawActive) {
        gReed.stableActive = gReed.rawActive;
        if (gReed.stableActive) {
          if (!gReed.sent) { gReed.sent = true; sendReedMessage(); }
        } else {
          gReed.sent = false;
          superCircuit.showAlert("REED", "OPEN", 700);
        }
      }
    }
  }

  if (!superCircuit.isOn()) superCircuit.setUserPixelColor(USER_LED_SYSTEM_OFF_COLOR);
  else if (gFlex.active) superCircuit.setUserPixelColor(wheelColor(gFlex.phase));
  else if (gReed.stableActive) superCircuit.setUserPixelColor(USER_LED_REED_COLOR);
  else if (gButton.stableActive) superCircuit.setUserPixelColor(USER_LED_BUTTON_COLOR);
  else superCircuit.setUserPixelColor(USER_LED_IDLE_COLOR);

  char line[28];
  snprintf(line, sizeof(line), "F:%s B:%s R:%s", gFlex.active ? "ON" : "OFF", gButton.stableActive ? "1" : "0", gReed.stableActive ? "1" : "0");
  superCircuit.setBottomLine(String(line));
  superCircuit.update();
}

void onButtonPressed(uint8_t index) {
  if (index == 1) runBoardButtonAction(index, BOARD_BUTTON_1_ACTION);
  else if (index == 2) runBoardButtonAction(index, BOARD_BUTTON_2_ACTION);
  else if (index == 3) runBoardButtonAction(index, BOARD_BUTTON_3_ACTION);
}

void runBoardButtonAction(uint8_t index, ExampleBoardButtonAction action) {
  switch (action) {
    case ExampleBoardButtonToggleEnabled:
      if (superCircuit.isOn()) {
        superCircuit.turnOff();
        Comms.printlnAll("BTN " + String(index) + " OFF");
      } else {
        superCircuit.turnOn();
        Comms.printlnAll("BTN " + String(index) + " ON");
      }
      break;
    case ExampleBoardButtonShowState:
      Comms.printlnAll("BTN " + String(index) + " STATE");
      showState();
      break;
    case ExampleBoardButtonPlayDefaultTrack:
      Comms.printlnAll("BTN " + String(index));
      superCircuit.showAlert("BTN", "Button " + String(index), 800);
      superCircuit.playTrack(DEFAULT_TRACK);
      break;
    case ExampleBoardButtonStopAndRefresh:
      Comms.printlnAll("BTN " + String(index));
      superCircuit.showAlert("BTN", "Button " + String(index), 800);
      superCircuit.stopTrack();
      superCircuit.setCommsStatus(Comms.isWifiConnected(), Comms.isTcpConnected());
      break;
    case ExampleBoardButtonNone:
    default:
      Comms.printlnAll("BTN " + String(index));
      superCircuit.showAlert("BTN", "Button " + String(index), 800);
      break;
  }
}

void onTurnOn() {
  superCircuit.setUserPixelColor(SHELL_USER_PIXEL_ON);
  Comms.printlnAll("TURN ON");
}

void onTurnOff() {
  superCircuit.setUserPixelColor(SHELL_USER_PIXEL_OFF);
  Comms.printlnAll("TURN OFF");
}

void showState() {
  Comms.printlnAll(String("SUPERCIRCUIT ") + (superCircuit.isOn() ? "ON" : "OFF"));
  Comms.printlnAll("DISPLAY " + String(superCircuit.isDisplayReady() ? "1" : "0"));
  Comms.printlnAll("PIXELS " + String(superCircuit.arePixelsReady() ? "1" : "0"));
  Comms.printlnAll("BUTTONS " + String(superCircuit.areButtonsReady() ? "1" : "0"));
  Comms.printlnAll("AUDIO " + String(superCircuit.isAudioReady() ? "1" : "0"));
  Comms.printlnAll("WIFI_LINE " + superCircuit.wifiLine());
  Comms.printlnAll("VOL " + String(superCircuit.volume()));
  Comms.printlnAll(String("FLEX_RAW ") + gFlex.raw);
  Comms.printlnAll(String("FLEX_ACTIVE ") + (gFlex.active ? "1" : "0"));
  Comms.printlnAll(String("BUTTON ") + (gButton.stableActive ? "1" : "0"));
  Comms.printlnAll(String("REED ") + (gReed.stableActive ? "1" : "0"));
}

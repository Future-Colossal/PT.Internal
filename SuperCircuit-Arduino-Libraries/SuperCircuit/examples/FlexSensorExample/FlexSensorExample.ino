#include <fcComms.h>
#include <SuperCircuit.h>
#include "project_config.h"

static fcComms Comms;
static SuperCircuit superCircuit;

struct FlexState {
  uint16_t raw = 0;
  bool active = false;
  unsigned long lastSampleMs = 0;
  unsigned long lastRainbowMs = 0;
  uint8_t phase = 0;
} gFlex;

String commSourceName(FcCommSource source);
void logLine(const String& line);
void showState();
void showHelp();
void runBoardButtonAction(uint8_t index, ExampleBoardButtonAction action);
void onButtonPressed(uint8_t index);
void onTurnOn();
void onTurnOff();

uint32_t wheelColor(uint8_t phase) {
  if (phase < 85) return superCircuit.packRgb(255 - phase * 3, 0, phase * 3);
  if (phase < 170) {
    phase -= 85;
    return superCircuit.packRgb(0, phase * 3, 255 - phase * 3);
  }
  phase -= 170;
  return superCircuit.packRgb(phase * 3, 255 - phase * 3, 0);
}

void setup() {
  Serial.begin(115200);
  pinMode(FLEX_SENSOR_PIN, INPUT);

  Comms.onMessage(messageReceived);
  Comms.begin(kCommsConfig);

  superCircuit.onButtonPressed(onButtonPressed);
  superCircuit.onTurnOn(onTurnOn);
  superCircuit.onTurnOff(onTurnOff);
  superCircuit.begin(kSuperCircuitConfig);

  superCircuit.setTitle(kDeviceTitle);
  superCircuit.setBottomLine("Flex sensor boot");
  superCircuit.setUserPixelColor(SHELL_USER_PIXEL_READY);
  superCircuit.showAlert("READY", "Flex sensor example", 1200);
  gFlex.raw = analogRead(FLEX_SENSOR_PIN);

  Comms.printlnAll("FLEX SENSOR EXAMPLE READY");
  showHelp();
}

void loop() {
  const unsigned long now = millis();

  Comms.update();
  if (Comms.isWifiConnected()) {
    superCircuit.setWifiStatus(true, Comms.localIP());
  } else {
    superCircuit.setWifiStatus(false, IPAddress());
  }
  superCircuit.setTcpConnected(Comms.isTcpConnected());

  if (superCircuit.isOn() && now - gFlex.lastSampleMs >= FLEX_SAMPLE_INTERVAL_MS) {
    gFlex.lastSampleMs = now;
    gFlex.raw = analogRead(FLEX_SENSOR_PIN);

    if (!gFlex.active && gFlex.raw >= FLEX_TRIGGER_THRESHOLD) {
      gFlex.active = true;
      if (FLEX_PLAY_SOUND_ON_TRIGGER) {
        superCircuit.playTrack(FLEX_TRIGGER_TRACK);
      }
      superCircuit.showAlert("FLEX", "Rainbow active", 900);
    } else if (gFlex.active && gFlex.raw <= FLEX_RELEASE_THRESHOLD) {
      gFlex.active = false;
      if (FLEX_PLAY_SOUND_ON_RELEASE) {
        superCircuit.playTrack(FLEX_RELEASE_TRACK);
      }
      superCircuit.showAlert("FLEX", "Rainbow stop", 800);
    }

    if (gFlex.active && now - gFlex.lastRainbowMs >= FLEX_RAINBOW_STEP_MS) {
      gFlex.lastRainbowMs = now;
      gFlex.phase += 7;
    }
  }

  if (!superCircuit.isOn()) {
    superCircuit.setUserPixelColor(USER_LED_SYSTEM_OFF_COLOR);
    superCircuit.setBottomLine("System OFF");
  } else if (gFlex.active) {
    superCircuit.setUserPixelColor(wheelColor(gFlex.phase));
    superCircuit.setBottomLine(String("Flex: ") + gFlex.raw);
  } else {
    superCircuit.setUserPixelColor(USER_LED_IDLE_COLOR);
    superCircuit.setBottomLine(String("Flex idle: ") + gFlex.raw);
  }

  superCircuit.update();
}

void messageReceived(const String& raw, FcCommSource source) {
  String msg = raw;
  msg.trim();
  String upper = msg;
  upper.toUpperCase();

  logLine("MSG[" + commSourceName(source) + "] " + upper);

  if (upper == "PING") {
    Comms.printlnAll("PONG");
    superCircuit.showAlert(commSourceName(source), "PING", 800);
    return;
  }

  if (upper == "HELP") {
    showHelp();
    return;
  }

  if (upper == "ON") {
    superCircuit.turnOn();
    Comms.printlnAll("STATE 1");
    return;
  }

  if (upper == "OFF") {
    superCircuit.turnOff();
    Comms.printlnAll("STATE 0");
    return;
  }

  if (upper == "STATE") {
    showState();
    return;
  }

  if (upper == "NET") {
    Comms.printlnAll("DEVICE " + String(Comms.deviceId()) + " " + String(Comms.deviceName()));
    Comms.printlnAll("WIFI " + String(Comms.isWifiConnected() ? "1" : "0"));
    Comms.printlnAll("TCP " + String(Comms.isTcpConnected() ? "1" : "0"));
    if (Comms.isWifiConnected()) {
      Comms.printlnAll("IP " + Comms.localIP().toString());
      Comms.printlnAll("WEB http://" + Comms.localIP().toString() + "/webserial");
    }
    return;
  }

  if (upper == "ALERT") {
    superCircuit.showAlert("FLEX", "Manual alert", 1200);
    return;
  }

  if (upper == "TITLE") {
    superCircuit.setTitle("Flex Title Test");
    superCircuit.showAlert("FLEX", "Title set", 800);
    return;
  }

  if (upper == "LINE") {
    superCircuit.setBottomLine("Flex line test");
    superCircuit.showAlert("FLEX", "Line set", 800);
    return;
  }

  if (upper == "LINE CLEAR") {
    superCircuit.clearBottomLine();
    superCircuit.showAlert("FLEX", "Line cleared", 800);
    return;
  }

  if (upper == "PIXEL") {
    superCircuit.setUserPixelColor(0x101000);
    superCircuit.showAlert("PIXEL", "User color set", 800);
    return;
  }

  if (upper == "PIXEL OFF") {
    superCircuit.setUserPixelColor(0x000000);
    superCircuit.showAlert("PIXEL", "User color cleared", 800);
    return;
  }

  if (upper == "COMMS") {
    if (Comms.isWifiConnected()) {
      superCircuit.setWifiStatus(true, Comms.localIP());
    } else {
      superCircuit.setWifiStatus(false, IPAddress());
    }
    superCircuit.setTcpConnected(Comms.isTcpConnected());
    superCircuit.showAlert("COMMS", "Status refresh", 800);
    return;
  }

  if (upper == "PALETTE DEFAULT") {
    superCircuit.setInteractivePalette(SHELL_INTERACTIVE_ON_COLOR, SHELL_INTERACTIVE_OFF_COLOR);
    superCircuit.setCommsPalette(SHELL_COMMS_OFFLINE_COLOR, SHELL_COMMS_WIFI_ONLY_COLOR, SHELL_COMMS_TCP_READY_COLOR);
    superCircuit.setUserPixelColor(USER_LED_IDLE_COLOR);
    superCircuit.showAlert("PIXEL", "Default palette", 900);
    return;
  }

  if (upper == "PALETTE DEMO") {
    superCircuit.setInteractivePalette(0x001c30, 0x300600);
    superCircuit.setCommsPalette(0x180000, 0x181200, 0x001818);
    superCircuit.setUserPixelColor(0x120012);
    superCircuit.showAlert("PIXEL", "Demo palette", 900);
    return;
  }

  if (upper == "WIFI MSG") {
    superCircuit.showAlert("WIFI", "Alert only; line 2 unchanged", 1200);
    return;
  }

  if (upper == "WIFI AUTO") {
    if (Comms.isWifiConnected()) {
      superCircuit.setWifiStatus(true, Comms.localIP());
    } else {
      superCircuit.setWifiStatus(false, IPAddress());
    }
    superCircuit.showAlert("WIFI", "Status refresh", 800);
    return;
  }

  if (upper.startsWith("VOL ")) {
    uint8_t volume = (uint8_t)upper.substring(4).toInt();
    superCircuit.setVolume(volume);
    Comms.printlnAll("VOL " + String(superCircuit.volume()));
    superCircuit.showAlert("AUDIO", "VOL " + String(superCircuit.volume()), 1000);
    return;
  }

  if (upper.startsWith("PLAY ")) {
    uint16_t track = (uint16_t)upper.substring(5).toInt();
    superCircuit.playTrack(track);
    Comms.printlnAll("PLAY " + String(track));
    superCircuit.showAlert("AUDIO", "PLAY " + String(track), 1000);
    return;
  }

  if (upper == "STOP") {
    superCircuit.stopTrack();
    Comms.printlnAll("STOP");
    superCircuit.showAlert("AUDIO", "STOP", 1000);
    return;
  }

  if (upper == "LOOP") {
    superCircuit.setAudioModeLoop();
    Comms.printlnAll("AUDIO LOOP");
    superCircuit.showAlert("AUDIO", "Loop mode", 1000);
    return;
  }

  if (upper == "SINGLE") {
    superCircuit.setAudioModeSingle();
    Comms.printlnAll("AUDIO SINGLE");
    superCircuit.showAlert("AUDIO", "Single mode", 1000);
    return;
  }

  if (upper == "RESET") {
    Comms.restart();
    return;
  }

  superCircuit.showAlert(commSourceName(source), msg, 1200);
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
  superCircuit.setBottomLine("Interactive ON");
  Comms.printlnAll("TURN ON");
}

void onTurnOff() {
  superCircuit.setUserPixelColor(SHELL_USER_PIXEL_OFF);
  superCircuit.setBottomLine("Interactive OFF");
  Comms.printlnAll("TURN OFF");
}

void showState() {
  String state = String("SUPERCIRCUIT ") + (superCircuit.isOn() ? "ON" : "OFF");
  Comms.printlnAll(state);
  Comms.printlnAll("DISPLAY " + String(superCircuit.isDisplayReady() ? "1" : "0"));
  Comms.printlnAll("PIXELS " + String(superCircuit.arePixelsReady() ? "1" : "0"));
  Comms.printlnAll("BUTTONS " + String(superCircuit.areButtonsReady() ? "1" : "0"));
  Comms.printlnAll("AUDIO " + String(superCircuit.isAudioReady() ? "1" : "0"));
  Comms.printlnAll("WIFI_LINE " + superCircuit.wifiLine());
  Comms.printlnAll("VOL " + String(superCircuit.volume()));
  Comms.printlnAll(String("FLEX_RAW ") + gFlex.raw);
  Comms.printlnAll(String("FLEX_ACTIVE ") + (gFlex.active ? "1" : "0"));
  superCircuit.showAlert("STATE", state, 1000);
}

void showHelp() {
  Comms.printlnAll("CMDS HELP PING ON OFF STATE NET ALERT TITLE LINE LINE CLEAR PIXEL PIXEL OFF COMMS PALETTE DEFAULT PALETTE DEMO WIFI MSG WIFI AUTO VOL <n> PLAY <n> STOP LOOP SINGLE RESET");
}

String commSourceName(FcCommSource source) {
  switch (source) {
    case FcCommSource::Serial:    return "SER";
    case FcCommSource::Tcp:       return "TCP";
    case FcCommSource::Udp:       return "UDP";
    case FcCommSource::WebSerial: return "WEB";
    case FcCommSource::Cluster:   return "CLU";
    case FcCommSource::EspNow:    return "NOW";
    default:                      return "?";
  }
}

void logLine(const String& line) {
  Comms.printlnSerial(line);
  Comms.printlnWebSerial(line);
}

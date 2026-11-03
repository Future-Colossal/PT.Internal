#include <fcComms.h>
#include <SuperCircuit.h>
#include "project_config.h"

static fcComms Comms;
static SuperCircuit superCircuit;

struct ButtonState {
  bool rawActive = false;
  bool stableActive = false;
  unsigned long lastEdgeMs = 0;
} gButton;

String commSourceName(FcCommSource source);
void logLine(const String& line);
void showState();
void showHelp();
void runBoardButtonAction(uint8_t index, ExampleBoardButtonAction action);
void onButtonPressed(uint8_t index);
void onTurnOn();
void onTurnOff();

void sendPressed() {
  Comms.printlnAll(BUTTON_MESSAGE);
  superCircuit.setUserPixelColor(USER_LED_PRESSED_COLOR);
  if (BUTTON_PLAY_SOUND_ON_PRESS) {
    superCircuit.playTrack(BUTTON_PRESS_TRACK);
  }
  superCircuit.showAlert("BTN", BUTTON_MESSAGE, 900);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_SENSOR_PIN, BUTTON_INPUT_MODE);
  gButton.rawActive = (digitalRead(BUTTON_SENSOR_PIN) == BUTTON_ACTIVE_STATE);
  gButton.stableActive = gButton.rawActive;

  Comms.onMessage(messageReceived);
  Comms.begin(kCommsConfig);

  superCircuit.onButtonPressed(onButtonPressed);
  superCircuit.onTurnOn(onTurnOn);
  superCircuit.onTurnOff(onTurnOff);
  superCircuit.begin(kSuperCircuitConfig);

  superCircuit.setTitle(kDeviceTitle);
  superCircuit.setBottomLine("Button sensor boot");
  superCircuit.setUserPixelColor(SHELL_USER_PIXEL_READY);
  superCircuit.showAlert("READY", "Button sensor example", 1200);

  Comms.printlnAll("BUTTON SENSOR EXAMPLE READY");
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

  const bool rawActive = (digitalRead(BUTTON_SENSOR_PIN) == BUTTON_ACTIVE_STATE);
  if (rawActive != gButton.rawActive) {
    gButton.rawActive = rawActive;
    gButton.lastEdgeMs = now;
  }

  if (now - gButton.lastEdgeMs >= BUTTON_DEBOUNCE_MS) {
    if (gButton.stableActive != gButton.rawActive) {
      gButton.stableActive = gButton.rawActive;
      if (gButton.stableActive && superCircuit.isOn()) {
        sendPressed();
      }
    }
  }

  if (!superCircuit.isOn()) {
    superCircuit.setUserPixelColor(USER_LED_SYSTEM_OFF_COLOR);
    superCircuit.setBottomLine("System OFF");
  } else if (gButton.stableActive) {
    superCircuit.setBottomLine("Button: pressed");
  } else {
    superCircuit.setUserPixelColor(USER_LED_IDLE_COLOR);
    superCircuit.setBottomLine("Button: idle");
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
    superCircuit.showAlert("BTN", "Manual alert", 1200);
    return;
  }

  if (upper == "TITLE") {
    superCircuit.setTitle("Button Title Test");
    superCircuit.showAlert("BTN", "Title set", 800);
    return;
  }

  if (upper == "LINE") {
    superCircuit.setBottomLine("Button line test");
    superCircuit.showAlert("BTN", "Line set", 800);
    return;
  }

  if (upper == "LINE CLEAR") {
    superCircuit.clearBottomLine();
    superCircuit.showAlert("BTN", "Line cleared", 800);
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

  if (upper == "SEND") {
    sendPressed();
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
  Comms.printlnAll("BUTTON_SENSOR " + String(gButton.stableActive ? "1" : "0"));
  Comms.printlnAll("BUTTON_MESSAGE " + String(BUTTON_MESSAGE));
  superCircuit.showAlert("STATE", state, 1000);
}

void showHelp() {
  Comms.printlnAll("CMDS HELP PING ON OFF STATE NET ALERT TITLE LINE LINE CLEAR PIXEL PIXEL OFF COMMS PALETTE DEFAULT PALETTE DEMO WIFI MSG WIFI AUTO VOL <n> PLAY <n> STOP LOOP SINGLE SEND RESET");
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

#include <fcComms.h>
#include <SuperCircuit.h>
#include "project_config.h"

// Project-specific hardware libraries should stay visible here in real
// interactives. SuperCircuit owns the board shell, but sensors and custom
// outputs should remain sketch-level dependencies so they are easy to swap.

fcComms Comms;
SuperCircuit superCircuit;

String commSourceName(FcCommSource source);
void logLine(const String& line);
void showState();
void showHelp();

void setup() {
  Serial.begin(115200);

  Comms.onMessage(messageReceived);
  Comms.begin(kCommsConfig);

  superCircuit.onButtonPressed(onButtonPressed);
  superCircuit.onTurnOn(onTurnOn);
  superCircuit.onTurnOff(onTurnOff);
  superCircuit.begin(kSuperCircuitConfig);

  superCircuit.setTitle(kDeviceTitle);
  superCircuit.setBottomLine("SuperCircuit test boot");
  superCircuit.setUserPixelColor(kUserPixelReady);
  superCircuit.showAlert("READY", "SuperCircuit test booted", 1200);

  Comms.printlnAll("SUPERCIRCUIT TEST READY");
  showHelp();
}

void loop() {
  Comms.update();
  if (Comms.isWifiConnected()) {
    superCircuit.setWifiStatus(true, Comms.localIP());
  } else {
    superCircuit.setWifiStatus(false, IPAddress());
  }
  superCircuit.setTcpConnected(Comms.isTcpConnected());
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
    superCircuit.showAlert("SUPER", "Manual alert", 1200);
    return;
  }

  if (upper == "TITLE") {
    superCircuit.setTitle("SuperCircuit Title Test");
    superCircuit.showAlert("SUPER", "Title set", 800);
    return;
  }

  if (upper == "LINE") {
    superCircuit.setBottomLine("Bottom line test");
    superCircuit.showAlert("SUPER", "Line set", 800);
    return;
  }

  if (upper == "LINE CLEAR") {
    superCircuit.clearBottomLine();
    superCircuit.showAlert("SUPER", "Line cleared", 800);
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
    superCircuit.setInteractivePalette(kInteractiveOnColor, kInteractiveOffColor);
    superCircuit.setCommsPalette(kCommsOfflineColor, kCommsWifiOnlyColor, kCommsTcpReadyColor);
    superCircuit.setUserPixelColor(kUserPixelDefaultColor);
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
  String label = "Button " + String(index);
  Comms.printlnAll("BTN " + String(index));
  superCircuit.showAlert("BTN", label, 800);

  // These defaults make the test sketch immediately useful on hardware:
  // button 1 = show state
  // button 2 = play a known track
  // button 3 = stop audio and refresh comms status
  if (index == 1) {
    showState();
  } else if (index == 2) {
    superCircuit.playTrack(kDefaultTrack);
  } else if (index == 3) {
    superCircuit.stopTrack();
    superCircuit.setCommsStatus(Comms.isWifiConnected(), Comms.isTcpConnected());
  }
}

void onTurnOn() {
  superCircuit.setUserPixelColor(kUserPixelOn);
  superCircuit.setBottomLine("Interactive ON");
  Comms.printlnAll("TURN ON");
}

void onTurnOff() {
  superCircuit.setUserPixelColor(kUserPixelOff);
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

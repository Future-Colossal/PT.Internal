#include <fcComms.h>
#include <SuperCircuit.h>

static fcComms Comms;
static SuperCircuit superCircuit;

enum ExampleBoardButtonAction : uint8_t {
  ExampleBoardButtonNone = 0,
  ExampleBoardButtonToggleEnabled = 1,
  ExampleBoardButtonShowState = 2,
  ExampleBoardButtonPlayDefaultTrack = 3,
  ExampleBoardButtonStopAndRefresh = 4
};

static const uint16_t DEFAULT_TRACK = 1;
static const ExampleBoardButtonAction BOARD_BUTTON_1_ACTION = ExampleBoardButtonToggleEnabled;
static const ExampleBoardButtonAction BOARD_BUTTON_2_ACTION = ExampleBoardButtonPlayDefaultTrack;
static const ExampleBoardButtonAction BOARD_BUTTON_3_ACTION = ExampleBoardButtonStopAndRefresh;

// ========================
//  WIFI / NETWORK - EDIT THESE FIRST
// ========================
static const char* WIFI_SSID = "YOUR_WIFI_SSID";
static const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
static const IPAddress WIFI_IP(192, 168, 1, 240);
static const IPAddress WIFI_GATEWAY(192, 168, 1, 1);
static const IPAddress WIFI_SUBNET(255, 255, 255, 0);

static const FcCommsConfig kCommsConfig = {
  240,
  "SuperCircuit Minimal Example",
  "SuperCircuit 240",
  true,
  true,
  true,
  true,
  true,
  true,
  8888,
  9001,
  80,
  {
    WIFI_SSID,
    WIFI_PASSWORD,
    WIFI_IP,
    WIFI_GATEWAY,
    WIFI_SUBNET
  },
  {
    false,
    9001,
    0,
    0,
    0
  }
};

static const SuperCircuitConfig kSuperCircuitConfig = [] {
  SuperCircuitConfig config;
  config.defaultTitle = "SuperCircuit 240";
  config.defaultBottomLine = "Minimal example";
  config.defaultInteractiveEnabled = true;

  config.features.dfPlayer = SuperCircuitToggle::Enabled;
  config.features.screen = SuperCircuitToggle::Enabled;
  config.features.pixels = SuperCircuitToggle::Enabled;
  config.features.volumePot = SuperCircuitToggle::Enabled;

  config.startup.speed = SuperCircuitStartupSpeed::Fast;
  config.startup.showReadyAlert = SuperCircuitToggle::Enabled;
  config.pixel.userDefaultColor = 0x001020;
  config.audio.defaultVolume = 18;
  return config;
}();

String sourceName(FcCommSource source) {
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

void showState() {
  Comms.printlnAll(String("SUPERCIRCUIT ") + (superCircuit.isOn() ? "ON" : "OFF"));
  Comms.printlnAll("DISPLAY " + String(superCircuit.isDisplayReady() ? "1" : "0"));
  Comms.printlnAll("PIXELS " + String(superCircuit.arePixelsReady() ? "1" : "0"));
  Comms.printlnAll("BUTTONS " + String(superCircuit.areButtonsReady() ? "1" : "0"));
  Comms.printlnAll("AUDIO " + String(superCircuit.isAudioReady() ? "1" : "0"));
  Comms.printlnAll("WIFI_LINE " + superCircuit.wifiLine());
  Comms.printlnAll("VOL " + String(superCircuit.volume()));
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

void messageReceived(const String& msg, FcCommSource source) {
  String upper = msg;
  upper.trim();
  upper.toUpperCase();

  if (upper == "PING") {
    Comms.printlnAll("PONG");
    superCircuit.showAlert(sourceName(source), "PING", 800);
    return;
  }

  if (upper == "ON") {
    superCircuit.turnOn();
    return;
  }

  if (upper == "OFF") {
    superCircuit.turnOff();
    return;
  }

  superCircuit.showAlert(sourceName(source), msg, 1200);
}

void onButtonPressed(uint8_t buttonNumber) {
  if (buttonNumber == 1) runBoardButtonAction(buttonNumber, BOARD_BUTTON_1_ACTION);
  else if (buttonNumber == 2) runBoardButtonAction(buttonNumber, BOARD_BUTTON_2_ACTION);
  else if (buttonNumber == 3) runBoardButtonAction(buttonNumber, BOARD_BUTTON_3_ACTION);
}

void onTurnOn() {
  superCircuit.setUserPixelColor(0x001000);
}

void onTurnOff() {
  superCircuit.setUserPixelColor(0x100000);
}

void setup() {
  Serial.begin(115200);

  Comms.onMessage(messageReceived);
  Comms.begin(kCommsConfig);

  superCircuit.onButtonPressed(onButtonPressed);
  superCircuit.onTurnOn(onTurnOn);
  superCircuit.onTurnOff(onTurnOff);
  superCircuit.begin(kSuperCircuitConfig);
  superCircuit.setTitle(kCommsConfig.deviceTitle);
  superCircuit.setBottomLine("Minimal example");
  superCircuit.showAlert("READY", "SuperCircuit", 1200);
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

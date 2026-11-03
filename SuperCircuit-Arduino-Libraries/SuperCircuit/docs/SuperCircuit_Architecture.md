# SuperCircuit Architecture

## What "SuperCircuit" Means In This Repo

In this workspace, "SuperCircuit" is not a single program. It is a reusable sketch pattern for ESP32-based interactives. Each interactive is a separate Arduino sketch folder, but most of them share the same file roles and the same runtime flow.

At a high level, each sketch is made of three layers:

1. SuperCircuit board/runtime layer
2. Communications layer
3. Experience-specific logic layer

## Shared File Anatomy

Most sketch folders contain some or all of the following files:

- `config.h`
  - Device name and ID
  - Wi-Fi profile and static IP config
  - Ports
  - Feature flags such as `ENABLE_OTA`, `ENABLE_WEBSERIAL`, `ENABLE_DFPLAYER`, `ENABLE_CLUSTER`
  - Sketch-specific constants
- `global.h`
  - Global includes
  - Shared declarations for comms and callbacks
  - Optional library includes needed by that sketch
- `superCircuitGlobal.h`
  - SuperCircuit hardware pin definitions
  - Shared UI state
  - Public API for the board/runtime layer
- `superCircuit.ino`
  - OLED setup and drawing
  - SuperCircuit buttons
  - Built-in status UI
  - NeoPixel status/feedback
  - Optional DFPlayer, pots, and other board services in some sketches
- `wifi.ino`
  - Wi-Fi connection setup
  - Static IP selection from `config.h`
  - OTA setup
- `serial.ino`
  - WebSerial setup
  - USB serial parsing
- `util.ino`
  - TCP server
  - reconnect logic
  - shared logging helpers
  - top-level comms lifecycle
- `<SketchName>.ino`
  - The actual interactive
  - custom setup and loop logic
  - command handling
  - button behavior

Some sketches also add their own files such as `cluster.ino` for multi-device coordination.

## Runtime Flow

The common runtime flow looks like this:

1. `setup()`
2. `superCircuitSetup()`
3. sketch-specific setup
4. `commsSetup()`
5. `loop()`
6. `receiveComms()`
7. `manageSuperCircuit()`
8. sketch-specific loop work

That means the SuperCircuit layer handles the operator-facing board services all the time, while the experience code mainly fills in the application behavior.

## The SuperCircuit Layer

The SuperCircuit layer is the reusable "device shell" around the interactive.

Its responsibilities usually include:

- OLED boot and status display
- local button handling
- on-board or attached NeoPixel status feedback
- shared state such as:
  - `gSystemEnabled`
  - `gWifiConnected`
  - `gTcpConnected`
  - title and status lines
  - temporary overlay/alert messages

Typical public API:

- `superCircuitSetup()`
- `manageSuperCircuit()`
- `setSystemEnabled(bool enabled)`
- `setWifiStatus(bool connected, const IPAddress& ip)`
- `setTcpStatus(bool connected)`
- `showAlert(const String& header, const String& body, unsigned long durationMs)`

In simple sketches like `WifiPowerSwitch_3.10`, `superCircuit.ino` is easiest to read because it clearly shows the responsibilities of setup, display drawing, button polling, and status updates.

## The Comms Layer

The comms layer gives sketches a common external control surface.

It typically provides:

- Wi-Fi station mode
- static IP configuration based on a selected profile
- TCP server for incoming command strings
- WebSerial for browser-based serial messaging
- USB Serial command input
- OTA support
- helper methods for outbound replies

Typical public API:

- `commsSetup()`
- `receiveComms()`
- `logLine(const String& line)`
- `tcpPrint(const String& s)`
- `tcpPrintln(const String& s)`

Typical inbound message path:

1. TCP, WebSerial, or USB Serial receives text
2. text is trimmed and normalized
3. sketch-specific `messageReceived()` is called

This is a clean design choice in the repo: the transport is shared, but the meaning of commands belongs to the sketch.

## The Experience Layer

The experience layer is where each project becomes unique.

It typically owns:

- sensors
- relays or actuators
- custom LED strips
- audio behaviors
- game state
- custom network messages

Every sketch implements the callback contract declared in `global.h`:

- `messageReceived(const String& raw)`
- `button1Pressed()`
- `button2Pressed()`
- `button3Pressed()`

This is the key extension seam in the codebase. If you understand those callbacks plus `setup()` and `loop()`, you can usually understand the whole sketch.

## Common Code Reading Strategy

When reading a SuperCircuit sketch, use this order:

1. `config.h`
   - Learn identity, feature flags, and hardware/network assumptions.
2. Main sketch `.ino`
   - Learn the application lifecycle and command vocabulary.
3. `superCircuit.ino`
   - Learn what the shared board layer is already doing for you.
4. `util.ino`, `wifi.ino`, `serial.ino`
   - Learn how commands enter and leave the system.
5. Extra modules like `cluster.ino`
   - Learn any project-specific subsystems.

## Architectural Patterns That Repeat

- Shared operator UI plus custom experience logic
- Shared text-based command handling
- `gSystemEnabled` used as a top-level enable gate
- Button 1 often toggles system on/off
- `STATE`, `ON`, `OFF`, `PING`, and `RESET` are common commands
- `showAlert()` is used as lightweight operator feedback
- sketch-specific hardware logic runs only when enabled

## Current Caveats

- `WifiPowerSwitch_3.10/config.h` has unresolved merge markers, so it is not currently a clean reference for build readiness.
- Similar files across sketch folders are not perfectly standardized. The pattern is strong, but not fully centralized into one library.
- Some sketches include optional hardware support paths that are only active when the corresponding feature flags are enabled.

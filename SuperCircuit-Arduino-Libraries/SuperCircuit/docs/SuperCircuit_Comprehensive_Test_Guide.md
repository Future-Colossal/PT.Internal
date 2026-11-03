# SuperCircuit Comprehensive Test Guide

This guide is the production validation plan for the `SuperCircuit` library and the 3-button SuperCircuit board.

It is based on:

- the current library implementation in [libraries/SuperCircuit/src](../libraries/SuperCircuit/src)
- the known-good `fcComms` backend
- the legacy SuperCircuit shell used in:
  - `SuperCircuit_Template`
  - `SuperCircuit_Distance_Test`
  - `SuperCircuit_Sensor_Showcase`
  - `TCM02_Hoop_Supercircuit_cluster`
  - `TCM_Fires_SuperCircuit_cluster`
  - `TCM02_Button_Reaction_SuperCircuit`
  - `TCM_Maze_SuperCircuit`
  - `TCM_LEDReact_SuperCircuit`
- the newer board image with clearer silkscreen and connector labels

## Purpose

This guide is meant to answer four questions:

1. Does the library cover the real board hardware and operator UX?
2. Does the public API cover the actual use cases we already ship?
3. Can an average technologist build simple interactives without editing library internals?
4. Are we following good Arduino-library practice instead of rebuilding the old `.ino` sprawl?

## Best-Practice Direction

The target architecture remains the right one:

- one sketch
- one config file
- `fcComms` owns networking and transports
- `SuperCircuit` owns onboard hardware, UI, local state, and operator feedback

That direction is consistent with the Arduino library spec:

- libraries should expose a narrow public header
- implementations should live under `src/`
- dependencies should be declared in `library.properties`
- examples should live under `examples/`

Source:

- Arduino library specification: https://docs.arduino.cc/arduino-cli/library-specification

The API style reference should remain Adafruit NeoPixel:

- one obvious include
- one obvious setup call
- named methods instead of public globals
- examples that teach usage
- internal complexity hidden behind the class

Sources:

- Adafruit NeoPixel repository: https://github.com/adafruit/Adafruit_NeoPixel
- Adafruit NeoPixel class reference: https://adafruit.github.io/Adafruit_NeoPixel/html/class_adafruit___neo_pixel.html

## Hide Plumbing, Expose Controls

This is the production rule the library should be tested against.

The library should hide:

- raw OLED object access
- raw pixel object access
- raw button debounce objects
- DFPlayer wiring/setup plumbing
- board-internal render/update sequencing

But it must keep these things easy to adjust:

- default colors
- brightness
- title and runtime text
- alert content and duration
- startup behavior
- audio defaults
- user-lane pixel behavior
- project-triggered shell responses

If a technologist has to edit `SuperCircuit.cpp` just to change a color, text line, or track choice, the library is not doing its job.

## Dependency Boundary

For this platform, the best split is:

- `SuperCircuit` owns board-shell dependencies internally:
  - OLED
  - NeoPixel
  - button debounce
  - DFPlayer
- project-specific hardware libraries stay explicit in the main sketch:
  - sensor drivers
  - external ADC drivers
  - ToF libraries
  - seesaw libraries
  - project-specific LED strip libraries

That keeps the board framework stable while still letting a technologist see and change the real project dependencies quickly.

## Generic Sensor-Friendly Rule

The library should support arbitrary project sensor combinations by doing two things well:

1. owning the board shell cleanly
2. documenting the board resources clearly

The sketch should remain free to choose:

- any exposed usable pin
- any sensor library
- any threshold/timing rules
- any mapping from sensor events to:
  - OLED state
  - pixel state
  - audio
  - outbound messages

That is the correct "sensor agnostic" model for this platform.

## Real SuperCircuit Use Cases Found In Existing Sketches

The legacy sketches show that `SuperCircuit` is not just a screen-and-buttons helper. It is the operator shell for several kinds of installations.

### Common use cases already present in this repo

- local operator shell:
  - boot feedback
  - status screen
  - buttons for enable, test, and reset-style actions
- sensor-driven interactives:
  - analog threshold triggers
  - digital trigger sensors
  - distance sensors
  - multi-sensor readouts
- score and state feedback:
  - alerts like `PING`, `GOOD`, `MISS`, `GOAL TRIGGERED`
  - bottom-line runtime messages
  - user-controlled status pixel lane
- audio playback:
  - track trigger
  - stop
  - volume control
  - track name / playback-state messaging
- comms/status mirroring:
  - Wi-Fi connected/offline
  - TCP connected/disconnected
- installation bring-up:
  - startup checks
  - ready screen
  - basic self-test behavior

### What this means for the library

The library should make these easy:

- turn the interactive on and off
- show a short operator alert
- set a title and a persistent runtime line
- reflect comms state
- control DFPlayer without raw object access
- keep internal pixel lanes stable while still giving the app a user lane
- let projects opt into or out of startup behavior through config

## Current Board Truth

### Onboard hardware

- 3 local buttons: `SW1`, `SW2`, `SW3`
- 3 external status pixel positions on `GPIO13`
- built-in Feather-style pixel on `GPIO0`
- OLED header `LCD1`
- DFPlayer Pro serial/power header and stereo output header
- power selector switch
- pot locations `P1` and `P2`

### Core board pin usage

| Function | Pin | Notes |
| --- | --- | --- |
| Built-in pixel | `0` | Feather onboard NeoPixel |
| External status pixel string | `13` | 3-pixel onboard status string |
| Button 1 / `SW1` | `37` | Current shell uses `37` |
| Button 2 / `SW2` | `27` | Current shell uses `27` |
| Button 3 / `SW3` | `34` | Current shell uses `34` |
| Pot `P1` | `36` | Available analog resource |
| Pot `P2` | `39` | Current shell volume pot path |
| DFPlayer RX | `7` | `Serial1` RX |
| DFPlayer TX | `8` | `Serial1` TX |

### Confirmed exposed connectors

| Connector | Labels |
| --- | --- |
| Screw terminal | `G`, `5V`, `3V`, `RS`, `VB`, `EN`, `21`, `22` |
| Wago terminal strip | `5V`, `3V`, `15`, `33`, `32`, `4`, `14` |
| JST1 | `5`, `G`, `5V` |
| JST2 | `G`, `19`, `(18)`, `5V` |
| JST3 | `25`, `G`, `5V` |
| Additional exposed pins | `D0`, `D1`, `15` |
| Pot connectors | `P1 = 36`, `P2 = 39` |

See also:

- [SuperCircuit_3Button_Pinout.md](./SuperCircuit_3Button_Pinout.md)
- [SuperCircuit_3Button_Wiring_Quick_Ref.md](./SuperCircuit_3Button_Wiring_Quick_Ref.md)

## Current Public Library Surface

The public `SuperCircuit` API should be tested as a workhorse operator shell, not just as a demo wrapper.

### Lifecycle

- `superCircuit.begin(config)`
- `superCircuit.update()`

### Interaction state

- `superCircuit.setEnabled(...)`
- `superCircuit.turnOn()`
- `superCircuit.turnOff()`
- `superCircuit.isOn()`

### Audio

- `superCircuit.playTrack(...)`
- `superCircuit.stopTrack()`
- `superCircuit.setVolume(...)`
- `superCircuit.volume()`
- `superCircuit.setAudioModeSingle()`
- `superCircuit.setAudioModeLoop()`
- `superCircuit.enableAmplifier()`
- `superCircuit.disableAmplifier()`
- `superCircuit.isAudioReady()`
- `superCircuit.isTrackPlaying()`
- `superCircuit.lastTrack()`
- `superCircuit.currentTrackName()`

### Display and operator messaging

- `superCircuit.showAlert(...)`
- `superCircuit.clearAlert()`
- `superCircuit.setTitle(...)`
- `superCircuit.title()`
- `superCircuit.setBottomLine(...)`
- `superCircuit.clearBottomLine()`
- `superCircuit.bottomLine()`
- `superCircuit.wifiLine()`

### Comms/status handoff

- `superCircuit.setWifiStatus(bool, IPAddress)`
- `superCircuit.setWifiStatus(bool, String)`
- `superCircuit.setWifiStatusMessage(...)`
- `superCircuit.clearWifiStatusMessage()`
- `superCircuit.setTcpConnected(...)`
- `superCircuit.setCommsStatus(...)`
- `superCircuit.isWifiConnected()`
- `superCircuit.isTcpConnected()`

### Pixels

- `superCircuit.setInteractivePalette(...)`
- `superCircuit.setCommsPalette(...)`
- `superCircuit.setPixelBrightness(...)`
- `superCircuit.setPixelLaneColor(...)`
- `superCircuit.setUserPixelColor(...)`
- `superCircuit.setUserPixelRgb(...)`
- `superCircuit.userPixelColor()`

### Hardware readiness

- `superCircuit.isDisplayReady()`
- `superCircuit.arePixelsReady()`
- `superCircuit.areButtonsReady()`

### Callbacks

- `superCircuit.onButtonPressed(...)`
- `superCircuit.onTurnOn(...)`
- `superCircuit.onTurnOff(...)`

## Config Surface That Should Be Easy To Use

The config now needs to be read and edited by humans safely. That means named blocks, not positional boolean soup.

### Current config categories

| Config block | What it should own |
| --- | --- |
| `defaultTitle`, `defaultBottomLine`, `defaultInteractiveEnabled` | project defaults |
| `features` | major hardware enable/disable flags |
| `display` | refresh rate, idle behavior, alert policy |
| `text` | shell text defaults for on/off, ready, Wi-Fi pending, track labels |
| `pixel` | brightness and lane colors |
| `audio` | volume, play mode, amp, volume-pot behavior |
| `startup` | startup sequence items, speed, and rainbow behavior |

### Best-practice rule

If a future project author is likely to change it per project, it belongs in config.

Examples:

- yes in config:
  - startup speed
  - whether startup rainbow runs
  - default title
  - default volume
  - whether the volume pot is active
  - comms/status colors
- no, keep internal:
  - OLED address
  - button pin numbers
  - built-in pixel pin
  - DFPlayer UART pins

## Pixel Ownership Model To Preserve

The intended lane model should stay explicit:

| Lane | Owner | Meaning |
| --- | --- | --- |
| `0` | library | interactive state |
| `1` | library | comms state |
| `2` | app | user-controlled feedback |

Best-practice expectation:

- app code should normally only use lane `2`
- lanes `0` and `1` should remain reserved for reliable operator status
- the built-in pixel should mirror comms state by default

If the app can casually stomp lane `0` or `1`, that is a design problem.

## Screen Model To Preserve

The current operator UX from the legacy shell should remain recognizable:

| Row | Purpose |
| --- | --- |
| Row 0 | title |
| Row 1 | Wi-Fi/IP line |
| Row 2 | interactive state + TCP state |
| Row 3 | alert overlay, bottom line, or uptime |

Best-practice expectation:

- parity first
- redesign later if needed
- keep the operator’s mental model stable across projects

## Current Validation Baseline

The current board-shell validation baseline is:

- [SuperCircuit_SC_Test](../SuperCircuit_SC_Test/README.md)

The current shell-plus-sensor validation baselines are:

- [FlexSensorExample](../libraries/SuperCircuit/examples/FlexSensorExample/FlexSensorExample.ino)
- [ButtonSensorExample](../libraries/SuperCircuit/examples/ButtonSensorExample/ButtonSensorExample.ino)
- [ReedSwitchExample](../libraries/SuperCircuit/examples/ReedSwitchExample/ReedSwitchExample.ino)
- [ThreeSensorExample](../libraries/SuperCircuit/examples/ThreeSensorExample/ThreeSensorExample.ino)

Interpret them this way:

- `SuperCircuit_SC_Test` is the board-shell truth target
- the examples prove that the shell still behaves correctly when sensor logic is added on top

## Comprehensive Manual Test Matrix

Run these in order.

### 1. Boot and startup sequence

Verify:

- the board boots reliably
- the board reaches steady runtime
- startup checks appear only when enabled
- startup speed changes when config changes
- rainbow startup can be enabled and disabled

Pass criteria:

- startup behavior is config-driven, not hardcoded per sketch

### 2. Buttons

Verify:

- button 1 callback arrives as button number `1`
- button 2 callback arrives as button number `2`
- button 3 callback arrives as button number `3`
- repeated tapping does not double-fire badly

Pass criteria:

- callback numbering is clear and human-friendly

### 3. OLED

Verify:

- title changes
- Wi-Fi line changes
- custom Wi-Fi status message overrides the auto line
- bottom line changes
- idle uptime appears when no bottom line is set
- alert overlay appears and clears

Pass criteria:

- the screen can act as the primary operator surface

### 4. External pixels

Verify:

- lane `0` changes with on/off state
- lane `1` changes with comms state
- lane `2` can be set by the app
- lane `2` does not break lanes `0` and `1`

Pass criteria:

- the lane model is stable and predictable

### 5. Built-in pixel

Verify:

- built-in pixel mirrors comms state
- it updates with Wi-Fi/TCP state changes

Pass criteria:

- the built-in pixel is useful operator feedback, not dead status

### 6. DFPlayer

Verify:

- DFPlayer initializes
- `setVolume(...)` works
- `playTrack(...)` works
- `stopTrack()` works
- single/loop mode methods work
- amp enable/disable methods do not crash runtime
- current volume and last track getters report sane values

Pass criteria:

- app code does not need raw DFPlayer object access

### 7. Volume pot behavior

Verify:

- `P2 / GPIO39` changes volume when volume-pot following is enabled
- volume alert appears when the pot moves
- disabling volume-pot following in config silences this behavior

Pass criteria:

- the pot path is optional and predictable

### 8. On/off state

Verify:

- `turnOn()`
- `turnOff()`
- `setEnabled(...)`
- on/off callbacks fire once
- screen and pixels reflect changes

Pass criteria:

- interactive enable state is centralized and reliable

### 9. Comms-state mirroring

Using `fcComms`, verify:

- Wi-Fi offline
- Wi-Fi connected / TCP offline
- Wi-Fi connected / TCP connected

Pass criteria:

- lane `1` and the screen both reflect comms status correctly

### 10. Exposed-pin board sanity checks

Test and document:

- `32` and `33` as known-good analog bench pins
- `21` and `22` on the screw terminal
- Wago strip: `15`, `33`, `32`, `4`, `14`
- `JST1` pin `5`
- `JST2` and `JST3` GPIO/power breakouts
- `P1 = 36` as analog-capable input

Pass criteria:

- docs match the real silkscreen and real accessible wiring points

## Suggested Command Checklist For `SuperCircuit_SC_Test`

- `HELP`
- `PING`
- `ON`
- `OFF`
- `STATE`
- `NET`
- `ALERT`
- `TITLE`
- `LINE`
- `PIXEL`
- `PIXEL OFF`
- `COMMS`
- `WIFI MSG`
- `WIFI AUTO`
- `VOL 18`
- `PLAY 1`
- `STOP`
- `LOOP`
- `SINGLE`

## Engineering Checks Against Existing Sketch History

These are the big parity questions from the legacy sketches.

### Must remain easy

- short operator alerts
- fast local button actions
- audio test / success / fail feedback
- setting a persistent runtime line
- reflecting transport state
- using the board as a quick sensor-shell base

### Must not leak back out

- raw `Adafruit_NeoPixel` objects in app code
- raw `Adafruit_SSD1306` calls in app code
- raw DFPlayer object calls in app code
- `titleLine`, `wifiLine`, and `bottomLine` as public globals
- direct button debounce object access from sketches

## Exit Criteria

Do not call `SuperCircuit` production-ready until:

- the board boots reliably through `superCircuit.begin(...)`
- the config is readable and safe to edit
- screen parity is acceptable
- pixel lane ownership is stable
- audio wrappers cover normal project use
- comms state mirroring works
- the docs match the real board

At that point, the right next step is:

1. bench-test [SuperCircuit_SC_Test](../SuperCircuit_SC_Test/README.md)
2. fix any parity gaps found there
3. then make the library template depend on the real `SuperCircuit` implementation by default

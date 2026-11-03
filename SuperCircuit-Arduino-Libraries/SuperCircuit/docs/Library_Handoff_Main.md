# Library Handoff Main

This is the primary handoff document for the current `fcComms` and `SuperCircuit` library work.

Use this first if you are:

- installing the libraries into Arduino
- validating the current implementation on hardware
- checking whether the public API matches the intended production workflow
- extending the examples into a real interactive

## What Is Being Handed Off

Two Arduino libraries:

- `fcComms`
- `SuperCircuit`

Supporting materials:

- install-package copies under [Arduino_Library_Install_Package](../Arduino_Library_Install_Package/INSTALL.md)
- validation sketches
- production-style example sketches
- board pinout and hardware docs

## Intended Architecture

The intended split is:

- `fcComms`
  - Wi-Fi
  - TCP
  - UDP
  - WebSerial
  - USB Serial
  - OTA
  - device identity
  - inbound message routing
- `SuperCircuit`
  - OLED shell
  - onboard buttons
  - onboard status pixels
  - built-in pixel
  - DFPlayer wrapper
  - startup/operator shell
  - system enabled/disabled state
- project sketch
  - sensor libraries
  - sensor pins
  - thresholds and debounce
  - project messages
  - project-specific behavior

That is the production rule to preserve.

## Current Status

Current understanding from the migration work:

- `fcComms` has been manually validated as the communications backend
- `SuperCircuit_SC_Test` has been manually validated as the board-shell validation target
- the single-sensor examples have been expanded to test both:
  - sensor behavior
  - `SuperCircuit` shell behavior

Validated core target:

- [SuperCircuit_SC_Test](../SuperCircuit_SC_Test/README.md)

## Current Source Of Truth

If an older sketch, an older doc, and the current examples disagree, treat these as the current behavioral source of truth:

1. `fcComms` library source
2. `SuperCircuit` library source
3. [SuperCircuit_SC_Test](../SuperCircuit_SC_Test/README.md)
4. the current example sketches under [libraries/SuperCircuit/examples](../libraries/SuperCircuit/examples)

Older repo sketches are still useful history, but they are not automatically the current handoff target.

## Library Locations

Repo copies:

- [libraries/fcComms](../libraries/fcComms/library.properties)
- [libraries/SuperCircuit](../libraries/SuperCircuit/library.properties)

Install-package copies to manually copy into Arduino `libraries`:

- [Arduino_Library_Install_Package/fcComms](../Arduino_Library_Install_Package/fcComms/library.properties)
- [Arduino_Library_Install_Package/SuperCircuit](../Arduino_Library_Install_Package/SuperCircuit/library.properties)

## Installation

Copy these folders into the Arduino sketchbook `libraries` directory:

- `Arduino_Library_Install_Package/fcComms`
- `Arduino_Library_Install_Package/SuperCircuit`

Then restart Arduino IDE before compiling.

If the IDE appears to keep stale metadata or old code:

1. close Arduino IDE
2. replace the installed library folder again
3. reopen Arduino IDE

## Board Truth

Confirmed board connector map:

- screw terminal: `G 5V 3V RS VB EN 21 22`
- Wago strip: `5V 3V 15 33 32 4 14`
- `JST1`: `5 G 5V`
- `JST2`: `19 (18) G 5V`
- `JST3`: `25 G 5V`
- additional exposed: `D0`, `D1`, `15`

Primary board reference docs:

- [SuperCircuit 3-button pinout](./SuperCircuit_3Button_Pinout.md)
- [SuperCircuit 3-button wiring quick ref](./SuperCircuit_3Button_Wiring_Quick_Ref.md)
- [SuperCircuit 3-button visual diagram](./SuperCircuit_3Button_Pinout_Diagram.md)

## `fcComms` Feature Summary

Current `fcComms` responsibilities:

- device identity
- static IP configuration
- Wi-Fi connect/reconnect support
- TCP server
- UDP listen/send
- USB Serial routing
- WebSerial support
- OTA initialization
- source-aware inbound dispatch through `messageReceived(msg, source)`

Key transport sources:

- Serial
- TCP
- UDP
- WebSerial
- Cluster
- ESP-NOW

Primary validation reference:

- [fcComms manual test plan](./fcComms_Manual_Test_Plan.md)

## `SuperCircuit` Feature Summary

Current `SuperCircuit` responsibilities:

- OLED shell
  - title line
  - Wi-Fi/IP line
  - system/TCP line
  - bottom line
  - scrolling alerts
- onboard buttons
- external 3-pixel status string
- built-in pixel mirror
- DFPlayer wrapper
- startup sequence
- interactive enabled/disabled state
- comms-state reflection from `fcComms`

Primary public API areas:

- lifecycle
  - `begin(config)`
  - `update()`
- system state
  - `turnOn()`
  - `turnOff()`
  - `setEnabled(...)`
  - `isOn()`
- display
  - `setTitle(...)`
  - `setBottomLine(...)`
  - `showAlert(...)`
  - `setWifiStatus(...)`
- pixels
  - `setUserPixelColor(...)`
  - `setInteractivePalette(...)`
  - `setCommsPalette(...)`
  - `setPixelBrightness(...)`
- audio
  - `playTrack(...)`
  - `stopTrack()`
  - `setVolume(...)`
  - `setAudioModeSingle()`
  - `setAudioModeLoop()`
- callbacks
  - `onButtonPressed(...)`
  - `onTurnOn(...)`
  - `onTurnOff(...)`

Primary validation references:

- [SuperCircuit manual test plan](./SuperCircuit_Manual_Test_Plan.md)
- [SuperCircuit comprehensive test guide](./SuperCircuit_Comprehensive_Test_Guide.md)

## Config Philosophy

The intended project shape is:

- one sketch
- one `project_config.h`

Project config should own:

- Wi-Fi credentials
- sensor pins
- thresholds
- debounce
- project messages
- project shell defaults
- project startup defaults

The library should keep internal:

- OLED address
- board-owned button pins
- board-owned pixel pins
- DFPlayer UART pins

## Important Design Boundary

`SuperCircuit` should hide board plumbing, but expose controls.

Hide:

- raw driver objects
- board update/render internals
- hardware setup sequencing

Expose:

- colors
- brightness
- shell text
- startup behavior
- audio defaults
- status/alert behavior

If a technologist has to edit `SuperCircuit.cpp` just to change a color, label, track choice, or default behavior, that is a design failure.

## Example Sketches

Primary example overview:

- [SuperCircuit example sketches](./SuperCircuit_Example_Sketches.md)

Current example set:

- `MinimalSuperCircuit`
- `FlexSensorExample`
- `ButtonSensorExample`
- `ReedSwitchExample`
- `ThreeSensorExample`

These examples are intended to show:

- production-style sketch/config separation
- explicit GPIO-based pin editing
- sensor-specific logic living in the sketch
- board shell behavior staying in `SuperCircuit`

## Important Example Note

Some example sketches expose:

- `BOARD_BUTTON_1_ACTION`
- `BOARD_BUTTON_2_ACTION`
- `BOARD_BUTTON_3_ACTION`

These are example-level convenience presets only.

They are **not** a library-level limit on what the onboard buttons can do.

Important reference:

- [SuperCircuit example button actions](./SuperCircuit_Example_Button_Actions.md)

The correct interpretation is:

- the library exposes the board-button event
- examples provide quick default remaps
- projects may replace button logic directly in the sketch

## Known Good Validation Targets

### Communications baseline

- [SuperCircuit_fcComms_Test](../SuperCircuit_fcComms_Test/README.md)

Use this to validate:

- serial
- Wi-Fi
- TCP
- UDP
- WebSerial
- OTA
- source-aware dispatch

### Board-shell baseline

- [SuperCircuit_SC_Test](../SuperCircuit_SC_Test/README.md)

Use this to validate:

- OLED shell
- IP/not-connected line behavior
- scrolling alerts
- pixels
- onboard buttons
- audio wrapper
- comms-state reflection

Current `SuperCircuit_SC_Test` button defaults:

- button 1: show state
- button 2: play the default test track
- button 3: stop audio and refresh comms status

Current `SuperCircuit_SC_Test` shell command set:

- `HELP`
- `PING`
- `ON`
- `OFF`
- `STATE`
- `NET`
- `ALERT`
- `TITLE`
- `LINE`
- `LINE CLEAR`
- `PIXEL`
- `PIXEL OFF`
- `COMMS`
- `PALETTE DEFAULT`
- `PALETTE DEMO`
- `WIFI MSG`
- `WIFI AUTO`
- `VOL <n>`
- `PLAY <n>`
- `STOP`
- `LOOP`
- `SINGLE`
- `RESET`

### Sensor + shell combined testing

- [libraries/SuperCircuit/examples/FlexSensorExample](../libraries/SuperCircuit/examples/FlexSensorExample/FlexSensorExample.ino)
- [libraries/SuperCircuit/examples/ButtonSensorExample](../libraries/SuperCircuit/examples/ButtonSensorExample/ButtonSensorExample.ino)
- [libraries/SuperCircuit/examples/ReedSwitchExample](../libraries/SuperCircuit/examples/ReedSwitchExample/ReedSwitchExample.ino)
- [libraries/SuperCircuit/examples/ThreeSensorExample](../libraries/SuperCircuit/examples/ThreeSensorExample/ThreeSensorExample.ino)

These are meant to test:

- the board shell from all angles
- plus one sensor at a time, or all three together

Important distinction:

- `SuperCircuit_SC_Test` is the board-shell truth target
- the example sketches are shell-plus-sensor targets built on top of that shell

## Default Example Board Buttons

Current example default behavior:

- button 1: toggle `ON` / `OFF`
- button 2: play default track
- button 3: stop audio + refresh comms

That is intentionally different from `SuperCircuit_SC_Test`, where button 1 shows state.

That can be changed:

- in `project_config.h` through the example action presets
- or directly in the example sketch handler for full custom behavior

## Recommended Validation Order For A Coworker

1. install fresh copies of `fcComms` and `SuperCircuit`
2. bench test [SuperCircuit_fcComms_Test](../SuperCircuit_fcComms_Test/README.md)
3. bench test [SuperCircuit_SC_Test](../SuperCircuit_SC_Test/README.md)
4. run `FlexSensorExample`
5. run `ButtonSensorExample`
6. run `ReedSwitchExample`
7. run `ThreeSensorExample`

That order isolates:

- comms layer
- board-shell layer
- single-sensor examples
- mixed-sensor example

## Known Expectations

- examples should use explicit GPIO numbers in their main edit blocks
- example configs should expose Wi-Fi creds, pins, thresholds, messages, and board-button defaults
- volume-pot following is disabled in the sensor examples so sensor tests stay isolated
- single-sensor examples now include the full `SuperCircuit_SC_Test` shell command surface plus their sensor-specific commands/state

## What To Check During Handoff Testing

### `fcComms`

- boot logs
- Wi-Fi connect
- IP reporting
- TCP behavior
- UDP behavior
- WebSerial behavior
- OTA readiness
- source-aware message dispatch

### `SuperCircuit`

- OLED rows behave correctly
- Wi-Fi line shows IP or `not connected`
- bottom-line alerts scroll fully
- onboard buttons behave predictably
- lane ownership is stable
- audio wrapper functions work
- `ON` / `OFF` state is reflected consistently

### Examples

- right ready banner appears
- `HELP` lists real supported commands
- shell-test commands work
- sensor-specific behavior works
- board-button defaults match the documented behavior

## If Something Fails

Debug in this order:

1. confirm the correct sketch was actually flashed
2. confirm the installed library copy is the fresh one
3. confirm `project_config.h` values are real and quoted correctly
4. re-test the same issue in `SuperCircuit_SC_Test`
5. only then decide whether the problem is:
   - example-level
   - `SuperCircuit`-level
   - `fcComms`-level

Do not assume an example bug means the core library is broken.

## Most Important Handoff Rule

Treat this as a layered system:

- `fcComms` first
- `SuperCircuit` second
- example sketch third
- project logic fourth

That is the cleanest way to test, debug, and extend the current work.

## Supporting Docs

- [SuperCircuit handoff guide](./SuperCircuit_Handoff_Guide.md)
- [fcComms manual test plan](./fcComms_Manual_Test_Plan.md)
- [SuperCircuit manual test plan](./SuperCircuit_Manual_Test_Plan.md)
- [SuperCircuit comprehensive test guide](./SuperCircuit_Comprehensive_Test_Guide.md)
- [SuperCircuit example sketches](./SuperCircuit_Example_Sketches.md)
- [SuperCircuit example button actions](./SuperCircuit_Example_Button_Actions.md)
- [SuperCircuit 3-button pinout](./SuperCircuit_3Button_Pinout.md)
- [SuperCircuit 3-button wiring quick ref](./SuperCircuit_3Button_Wiring_Quick_Ref.md)

# SuperCircuit Manual Test Plan

This document is the practical setup and validation plan for the `SuperCircuit` library migration pass.

It assumes:

- `fcComms` has already been validated
- the 3-button board is known-good hardware
- the goal is still one sketch plus one config file for future projects

## Boss Requirements Check

This phase is aligned with the original handoff target:

- future projects should be lightweight
- important tunable behavior should live in config
- the library should wrap hardware instead of exposing raw globals
- the existing SuperCircuit OLED/status UX should be preserved
- the board should still feel like a fast operator shell, not a complex framework

The public shape should now be thought of as:

- `superCircuit.begin(config)`
- `superCircuit.update()`
- `superCircuit.turnOn()`
- `superCircuit.turnOff()`
- `superCircuit.playTrack(...)`
- `superCircuit.stopTrack()`
- `superCircuit.setVolume(...)`
- `superCircuit.showAlert(...)`

That is closer to the Adafruit NeoPixel model:

- one clear include
- one clear setup call
- named methods
- config and examples doing the teaching

References:

- Arduino library spec: https://docs.arduino.cc/arduino-cli/library-specification
- Adafruit NeoPixel: https://github.com/adafruit/Adafruit_NeoPixel

## What This Phase Should Cover

This pass is for:

- OLED bring-up and operator layout
- buttons
- pixels
- interactive on/off state
- DFPlayer wrapper behavior
- startup sequence behavior
- alert behavior
- comms-state reflection from `fcComms`

This pass is not primarily for:

- cluster migration
- new sensor systems
- redesigning the board UX
- replacing every existing sketch immediately

## Current Validation Target

Use:

- [SuperCircuit_SC_Test](../SuperCircuit_SC_Test/README.md)

This plan is specifically about the current `SuperCircuit_SC_Test` behavior, not the sensor-example defaults.

Current library under test:

- [libraries/SuperCircuit/src/SuperCircuit.h](../libraries/SuperCircuit/src/SuperCircuit.h)
- [libraries/SuperCircuit/src/SuperCircuit.cpp](../libraries/SuperCircuit/src/SuperCircuit.cpp)

## Config Boundary To Protect

The config should stay easy for a technologist to edit.

### Keep in config

- default title
- default bottom line
- default interactive enabled state
- feature toggles:
  - DFPlayer
  - screen
  - pixels
  - volume pot
- display behavior:
  - refresh rate
  - default alert duration
  - idle uptime behavior
- shell text defaults:
  - on/off lines
  - on/off alert text
  - ready alert text
  - Wi-Fi pending line
  - track label prefix
- pixel colors and brightness
- audio defaults:
  - default volume
  - min/max volume
  - single vs loop mode
  - amp enabled
  - follow volume pot
- startup behavior:
  - run sequence
  - rainbow
  - which startup checks show
  - fast/slow/custom speed

### Keep internal

- OLED address
- button pins
- DFPlayer UART pins
- built-in pixel pin
- onboard status pixel pin

## Manual Test Order

### 1. Boot

Verify:

- board boots cleanly
- OLED initializes
- pixels initialize
- buttons do not hang boot

### 2. Startup sequence

Verify:

- startup checks show when enabled
- fast/slow/custom behavior works
- rainbow can be turned on and off

### 3. Buttons

Verify:

- button callbacks arrive as `1`, `2`, `3`
- no obvious double-fire problem
- current `SuperCircuit_SC_Test` board-button defaults are:
  - button 1 = show state
  - button 2 = play the default test track
  - button 3 = stop audio and refresh comms status

### 4. Screen

Verify:

- title line
- Wi-Fi line shows the live IP when connected
- Wi-Fi line falls back to `WiFi: not connected` when Wi-Fi is down
- bottom line
- alert overlays scroll the full `TITLE: BODY` message across the bottom instead of clipping
- idle uptime fallback

### 5. Pixels

Verify:

- lane `0` = interactive state
- lane `1` = comms state
- lane `2` = app/user state

### 6. Audio

Verify:

- `setVolume(...)`
- `playTrack(...)`
- `stopTrack()`
- loop/single mode
- amp methods
- volume pot following

### 7. Comms-state handoff

With `fcComms`, verify:

- Wi-Fi offline
- Wi-Fi connected
- TCP connected

### 8. Exposed board resources

Spot-check:

- `32`, `33`
- `21`, `22`
- Wago strip: `15`, `33`, `32`, `4`, `14`
- `JST1` pin `5`
- `19`, `18`, `25`
- `P1 = 36`

## What A Good Result Looks Like

- the sketch only includes `fcComms.h`, `SuperCircuit.h`, and project config
- the project config reads clearly
- no app code needs raw OLED, NeoPixel, button, or DFPlayer objects
- the board still feels like the old SuperCircuit operator shell
- average project authors can change titles, startup behavior, colors, audio defaults, and enable flags without opening the library

## Bench Checklist

- Boot:
  - OLED comes up
  - line 1 shows the configured title
  - line 2 starts in a pending or not-connected state during bring-up
  - line 3 shows system state and TCP state
  - line 4 shows the ready alert, then returns to the default bottom line or uptime
- Wi-Fi:
  - when Wi-Fi connects, line 2 changes to `WiFi: <ip>`
  - when Wi-Fi is intentionally unavailable, line 2 shows `WiFi: not connected`
  - `WIFI MSG` overrides the line with a custom message
  - `WIFI AUTO` restores automatic IP/not-connected behavior
- Alerts:
  - `ALERT` scrolls fully across the bottom line
  - `PING`, `STATE`, button presses, and audio commands all produce readable scrolling alerts
  - when the alert expires, the bottom line returns cleanly
- Interactive state:
  - `ON` sets the interactive state to enabled
  - `OFF` sets it to disabled
  - line 3 matches the enabled/disabled state
  - pixel lane `0` changes with the state
- Comms state:
  - built-in pixel mirrors comms color
  - pixel lane `1` changes between offline, Wi-Fi-only, and TCP-ready colors
  - `COMMS` refreshes the shell from live `fcComms` state
- User pixel lane:
  - `PIXEL` changes the user lane color
  - `PIXEL OFF` clears the user lane
  - `PALETTE DEFAULT` restores the normal shell palette
  - `PALETTE DEMO` applies an obvious alternate palette
- Audio:
  - `PLAY <n>` starts a track
  - `STOP` stops it
  - `VOL <n>` changes volume
  - `LOOP` and `SINGLE` switch playback mode
  - volume-pot movement changes volume if enabled
- Buttons:
  - button 1 shows state
  - button 2 plays the default track
  - button 3 stops audio and refreshes comms state
  - no obvious button double-fire occurs

## Current `SuperCircuit_SC_Test` Command Set

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

## Failure Signs

- config has turned back into positional booleans
- app code needs raw hardware includes
- screen rows are no longer predictable
- pixel lane ownership is fuzzy
- button numbering is confusing
- common old-shell use cases require library edits instead of config or public methods

## Exit Criteria

This pass is ready to move forward when:

- [SuperCircuit_SC_Test](../SuperCircuit_SC_Test/README.md) is stable on hardware
- the config reads cleanly
- the library covers the normal operator shell tasks from the legacy sketches
- the docs match the real board and real public API

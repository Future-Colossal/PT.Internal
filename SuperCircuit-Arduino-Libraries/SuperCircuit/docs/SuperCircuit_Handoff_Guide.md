# SuperCircuit Handoff Guide

This document is the practical pass-off guide for the SuperCircuit codebase and the current 3-button SuperCircuit board workflow.

It is written so a technician or developer can begin immediately with:

- the right mental model of the repo
- the board-level pin and wiring constraints
- the current starter sketches
- the current sensor demo workflow
- a safe process for adding more hardware
- the main upgrade opportunities in the codebase

## 1. Executive Summary

In this repo, "SuperCircuit" is a reusable ESP32 sketch pattern for interactives.

Each sketch usually combines three layers:

1. a board/runtime shell
2. a communications shell
3. a project-specific interactive

The board/runtime shell gives you:

- OLED status display
- three local buttons
- status pixels
- optional DFPlayer Pro support
- a shared "system enabled" concept

The communications shell gives you:

- Wi-Fi
- static IP configuration
- TCP command input
- WebSerial
- USB Serial
- OTA update support

The interactive layer gives you the actual project behavior:

- sensors
- relays
- LED strips
- audio triggers
- game logic
- cluster logic when needed

The most important onboarding idea is this:

Do not treat each sketch as completely separate. Most of them are variations on the same shell.

## 2. Current Repo Structure

Key top-level sketches in this workspace:

| Sketch | Role |
| --- | --- |
| `SuperCircuit_Template` | clean starting point with OLED, DFPlayer, and simple sensor hooks |
| `SuperCircuit_Distance_Test` | focused HC-SR04 bench test |
| `SuperCircuit_Sensor_Showcase` | multi-sensor production demo for the 3-button board |
| `WifiPowerSwitch_3.10` | simple architecture reference for command-driven control |
| `TCM02_Hoop_Supercircuit_cluster` | ToF hoop interactive with cluster logic |
| `TCM_Fires_SuperCircuit_cluster` | multi-output fire interactive with sensors, LEDs, and audio |
| `TCM02_Button_Reaction_SuperCircuit` | I2C button game |
| `TCM_Maze_SuperCircuit` | sensor-processing maze interactive |
| `TCM_LEDReact_SuperCircuit` | LED reaction/timing interactive |

The best current starting points for new work are:

- `SuperCircuit_Template` for a clean new build
- `SuperCircuit_Sensor_Showcase` for sensor-heavy bench and demo work

## 3. Shared File Anatomy

Most sketch folders follow this file pattern:

| File | Purpose |
| --- | --- |
| `config.h` | device identity, Wi-Fi profile, ports, feature flags, sensor constants |
| `global.h` | includes, declarations, callback contract |
| `superCircuitGlobal.h` | shared board pin definitions and UI globals |
| `superCircuit.ino` | OLED, local buttons, status pixels, shell behavior, DFPlayer helpers |
| `wifi.ino` | Wi-Fi and OTA setup |
| `serial.ino` | USB Serial and WebSerial handling |
| `util.ino` | TCP server, logging, comms lifecycle |
| main sketch `.ino` | the project-specific interactive |

This is the normal runtime flow:

1. `setup()`
2. `superCircuitSetup()`
3. sketch-specific setup
4. `commsSetup()`
5. `loop()`
6. `receiveComms()`
7. `manageSuperCircuit()`
8. sketch-specific loop work

The extension contract is also consistent:

- `messageReceived(const String& raw)`
- `button1Pressed()`
- `button2Pressed()`
- `button3Pressed()`

## 4. How The 3-Button SuperCircuit Board Works

This guide is focused on the newer 3-button board family.

The board provides:

- an ESP32 Feather-style footprint
- three local control buttons
- three external status pixels driven by the shell
- a built-in pixel
- OLED header on I2C
- DFPlayer Pro support over UART
- onboard volume pot path used by the shell when DFPlayer is enabled
- several exposed field-wiring pins and power points

From the board photos and corrected board breakdown, the large screw terminal is left-to-right:

- `G 5V 3V RS VB EN 21 22`

Confirmed exposed connector groups include:

- Wago terminal strip: `5V 3V 15 33 32 4 14`
- JST1: `5 G 5V`
- JST2: `19 (18) G 5V`
- JST3: `25 G 5V`
- additional exposed pins: `D0`, `D1`, `15`

## 5. Reserved Shell Pins

These pins are already owned by the shared SuperCircuit runtime and should be considered reserved unless you intentionally modify the shell:

| Function | Pin |
| --- | --- |
| built-in pixel | `0` |
| DFPlayer UART | `7`, `8` |
| status pixels | `13` |
| local button A | `37` |
| local button B | `27` |
| local button C | `34` |
| shell volume pot | `39` |

The OLED runs on the board-default I2C pins through `Wire.begin()` and uses address `0x3C`.

## 6. Practical Pin Guidance For New Hardware

For this board, the most important pin rule is:

- `GPIO32` and `GPIO33` are the best exposed analog pins when Wi-Fi is enabled

### Best Exposed Analog Pins With Wi-Fi Enabled

- `32`
- `33`

These are the first choice for:

- photoresistors
- flex sensors
- potentiometers
- analog distance sensors
- analog pressure or force sensors

### Analog-Capable But Weak With Wi-Fi Enabled

- `4`
- `14`
- `15`
- `25`

These are `ADC2` pins, so analog reads can become unreliable when Wi-Fi is active on ESP32.

These pins are still useful for:

- local-only analog tests
- analog sketches with Wi-Fi disabled
- digital sensors

### Good Exposed Digital Pins

- `4`
- `5`
- `14`
- `15`
- `19`
- `21`
- `22`
- `25`
- `32`
- `33`

### Use With Caution

- `12`
  - ESP32 boot strapping pin
  - external sensors can interfere with boot
- `15`
  - can be boot-sensitive depending on hardware and startup conditions
- `32` and `33`
  - safe as digital, but they are too valuable as analog pins to spend casually

## 7. Current Starter Sketches

## SuperCircuit_Template

Purpose:

- clean reusable base for new work
- OLED + DFPlayer + comms shell
- simple sensor hooks

Good for:

- new interactives
- quick board bring-up
- first-pass sensor experiments

Typical use:

1. copy the folder
2. rename the sketch
3. update `config.h`
4. replace the demo sensor logic with project logic

## SuperCircuit_Distance_Test

Purpose:

- isolate HC-SR04 setup and troubleshooting

Good for:

- testing power
- verifying TRIG/ECHO wiring
- confirming distance behavior without other sensor complexity

## SuperCircuit_Sensor_Showcase

Purpose:

- run several analog and digital sensors on one SuperCircuit
- demonstrate production-style structure for sensor handling

Current showcased sensors:

- external push button
- HC-SR04
- reed switch
- PIR sensor
- ZD10-100 flex sensor
- 6-pin potentiometer using one gang

This sketch is the current best tech demo for the 3-button board.

## 8. Current Showcase Sensor Map

The current `SuperCircuit_Sensor_Showcase` map is:

| Sensor | Pin |
| --- | --- |
| HC-SR04 `TRIG` | `33` |
| HC-SR04 `ECHO` | `32` |
| external push button | `4` |
| reed switch | `21` |
| PIR output | `15` |
| flex sensor | `14` |
| potentiometer | `25` |

Remaining easy exposed pins after this showcase:

- `5`
- `19`
- `22`
- `11`

Important tradeoff:

- The distance sensor currently uses the best analog pair, `32/33`
- That forces the flex sensor and potentiometer onto weaker analog pins
- Result: HC-SR04 works better, but analog reliability may get worse when Wi-Fi is enabled

This tradeoff should be considered normal and expected, not mysterious.

## 9. How To Wire The Current Showcase Sensors

### External Push Button

- one side to `GPIO4`
- one side to `GND`

The sketch uses pull-up logic, so the button is active when it shorts the pin to ground.

### Reed Switch

- one side to `GPIO21`
- one side to `GND`

This is also treated as a pull-up style digital input.

### PIR Sensor

- output to `GPIO15`
- power and ground per module requirements

The sketch expects PIR motion output to go `HIGH`.

### HC-SR04

- `TRIG` to `GPIO33`
- `ECHO` to `GPIO32`
- `GND` to `GND`
- `VCC` to `3.3V` or `5V` depending on module behavior

Important:

- many HC-SR04 modules behave better on `5V`
- if powered from `5V`, the `ECHO` line should be treated carefully because ESP32 GPIO is not 5V tolerant

### Flex Sensor

Wire as a voltage divider:

- one side of flex sensor to `3.3V`
- other side to `GPIO14`
- fixed resistor from `GPIO14` to `GND`

### Potentiometer

Use one gang only:

- outer pin to `3.3V`
- opposite outer pin to `GND`
- center wiper to `GPIO25`

If the direction feels reversed, swap the two outer pins.

## 10. How The SuperCircuit Shell Behaves At Runtime

The shell is the board-level operator layer around the sketch.

It normally provides:

- boot setup
- OLED title and status lines
- temporary alerts
- local button polling
- status pixels
- DFPlayer helpers when enabled
- top-level enabled/disabled state

Common shared state used across sketches:

- `gSystemEnabled`
- `gWifiConnected`
- `gTcpConnected`
- `titleLine`
- `bottomLine`

In most sketches:

- Button 1 toggles enable/disable
- Button 2 triggers a local test or status action
- Button 3 triggers a second local action, often audio or reset-like behavior

## 11. Communications Model

The comms layer allows the same sketch to be controlled through multiple text paths.

Typical supported paths:

- Wi-Fi TCP socket
- WebSerial
- USB Serial
- OTA for updates

Typical standard commands across sketches:

- `PING`
- `ON`
- `OFF`
- `STATE`
- `RESET`

The main design rule is:

- transports are shared
- command meaning belongs to the sketch

That means a new sketch can keep the same transport shell and only redefine `messageReceived()`.

## 12. How To Read A New Sketch Quickly

Recommended reading order:

1. `config.h`
2. main sketch `.ino`
3. `superCircuit.ino`
4. `util.ino`
5. `wifi.ino`
6. `serial.ino`
7. any extra modules such as `cluster.ino`

This separates:

- deployment assumptions
- project behavior
- shared shell behavior
- network details

## 13. How To Add A New Sensor

This is the recommended process for adding a new sensor to a 3-button SuperCircuit.

### Step 1: Decide Whether The Sensor Is Analog Or Digital

Examples:

- analog: photoresistor, flex sensor, potentiometer, analog IR sensor
- digital: push button, reed switch, PIR output, tilt switch, limit switch

### Step 2: Choose The Right Pin Type

If Wi-Fi stays enabled:

- prefer `32` or `33` for analog
- use `4`, `5`, `14`, `15`, `19`, `21`, `22`, or `25` for digital

If Wi-Fi is disabled:

- `14` and `25` become more acceptable as analog inputs

### Step 3: Start From The Right Base Sketch

- one sensor or generic bring-up: `SuperCircuit_Template`
- HC-SR04 only: `SuperCircuit_Distance_Test`
- several sensors together: `SuperCircuit_Sensor_Showcase`

### Step 4: Update `config.h`

Add:

- pin defines
- threshold values
- timing values
- feature flags if needed

### Step 5: Add Clear Wiring Comments

Every sensor section should state:

- exact pin
- supply voltage
- whether it needs a pull-up or divider
- active state
- any power or boot caveats

### Step 6: Add Dedicated Setup And Update Helpers

Recommended pattern:

- `setup<Thing>()`
- `update<Thing>()`
- `report<Thing>()`

This keeps the sketch maintainable and easy to hand off.

### Step 7: Add Operator Feedback

For new sensors, include at least one of:

- OLED status
- serial/TCP command report
- status pixel color
- audio feedback

The current showcase uses per-sensor pixel colors, which is a good pattern to keep.

## 14. Recommended Sensor Bring-Up Sequence

For any new mixed-sensor build, bring hardware online in this order:

1. OLED and basic boot
2. local buttons
3. DFPlayer
4. one known-good digital sensor
5. one known-good analog sensor
6. complex sensors such as HC-SR04
7. multi-sensor interactions

This avoids chasing several hardware problems at once.

## 15. Known Issues And Lessons Learned

These are the most important practical lessons from the current work:

### Lesson 1: Double-check board exposure before planning around a pin

A pin being valid on the ESP32 does not mean it is part of the practical field-wiring surface on this board.

Recommendation:

- trust the confirmed SuperCircuit connector map first
- plan around the actually exposed groups:
  - screw terminal `21`, `22`
  - Wago strip `15`, `33`, `32`, `4`, `14`
  - JST1 `5`
  - JST2 `19`, `(18)`
  - JST3 `25`
  - extra exposed `D0`, `D1`, `15`

### Lesson 2: HC-SR04 And ESP32 Need Care

The HC-SR04 may:

- behave poorly at `3.3V`
- behave better at `5V`
- return `no echo` if power or wiring is wrong

Recommendation:

- verify `TRIG`, `ECHO`, power, and common ground first
- if using `5V`, protect the `ECHO` line appropriately

### Lesson 3: `ADC2` Pins Are A Bad Fit For Wi-Fi Analog Work

When Wi-Fi is enabled, analog on `ADC2` pins can become unavailable or unreliable.

Recommendation:

- reserve `32/33` for analog if analog quality matters
- move digital sensors to other pins first

### Lesson 4: Pin Access Matters As Much As Pin Capability

A technically available GPIO is not useful if it is not practically accessible on the board.

Recommendation:

- trust the physical board connector access, not just generic ESP32 capability lists

## 16. Upgrade Opportunities

The current codebase works, but there are several clear improvements worth planning.

### High-Value Upgrade 1: Centralize The Shared Shell

Right now, similar shell files are duplicated across sketch folders.

Better future direction:

- one common shared SuperCircuit library or common folder
- sketch folders contain only app-specific logic plus config

Benefits:

- fixes propagate once
- onboarding gets easier
- less drift between sketches

### High-Value Upgrade 2: Standardize Folder Docs

Every sketch should have:

- purpose
- hardware list
- pin map
- commands
- local button behavior
- calibration notes
- known issues

Much of this now exists, but standardizing it fully would help.

### High-Value Upgrade 3: Create Sensor Modules

The sensor code can be made more reusable by extracting patterns such as:

- debounced digital sensor
- threshold analog sensor
- distance sensor wrapper
- shared reporting helpers

Benefits:

- faster prototyping
- easier testing
- cleaner main sketches

### High-Value Upgrade 4: Add A Dedicated Local-Only Sensor Lab Mode

Because analog and Wi-Fi compete on some pins, a local-only mode would be useful.

This mode would:

- disable Wi-Fi/WebSerial/OTA
- free up analog reliability
- provide rich local OLED and serial diagnostics

This is especially useful for flex, photoresistors, pots, and other analog bench work.

### High-Value Upgrade 5: Add Bench Test Checklists Per Sensor

Each common sensor type should have:

- known-good wiring
- test commands
- expected raw ranges
- common failure modes

This would reduce repeated troubleshooting.

## 17. Recommended Workflow For A Technician

If handing this system to a tech today, this is the recommended operating sequence:

1. Read this document once
2. Open `docs/SuperCircuit_3Button_Pinout.md`
3. Open `SuperCircuit_Sensor_Showcase/WIRING_CHECKLIST.md`
4. Bench test the board with `SuperCircuit_Sensor_Showcase`
5. Confirm:
   - OLED boot
   - local buttons
   - DFPlayer
   - push button
   - reed
   - PIR
   - HC-SR04
   - flex
   - potentiometer
6. If building a new experience:
   - duplicate `SuperCircuit_Template`
   - update `config.h`
   - add one sensor at a time
   - keep comments and wiring notes current

## 18. Recommended First Read Files

If someone is new and wants the shortest path into the repo:

1. `docs/SuperCircuit_Handoff_Guide.md`
2. `docs/SuperCircuit_3Button_Pinout.md`
3. `SuperCircuit_Template/README.md`
4. `SuperCircuit_Sensor_Showcase/README.md`
5. `SuperCircuit_Sensor_Showcase/WIRING_CHECKLIST.md`
6. `SuperCircuit_Template/config.h`
7. `SuperCircuit_Template/SuperCircuit_Template.ino`

## 19. Related Documents

- [SuperCircuit architecture](./SuperCircuit_Architecture.md)
- [How to use SuperCircuit](./How_To_Use_SuperCircuit.md)
- [Example interactives](./Example_Interactives.md)
- [3-button pinout](./SuperCircuit_3Button_Pinout.md)
- [3-button wiring quick ref](./SuperCircuit_3Button_Wiring_Quick_Ref.md)
- [3-button visual diagram](./SuperCircuit_3Button_Pinout_Diagram.md)
- [Sensor showcase README](../SuperCircuit_Sensor_Showcase/README.md)
- [Sensor showcase wiring checklist](../SuperCircuit_Sensor_Showcase/WIRING_CHECKLIST.md)
- [Template README](../SuperCircuit_Template/README.md)

## 20. Final Recommendation

If a technician is starting immediately, the current best operating model is:

- use `SuperCircuit_Template` as the base for new builds
- use `SuperCircuit_Sensor_Showcase` as the hardware validation sketch
- preserve `32` and `33` for analog whenever possible
- use the confirmed connector map when choosing expansion pins
- document every added sensor directly in the sketch and its local README

That approach matches the current board realities, the current repo structure, and the lessons already learned during bring-up.

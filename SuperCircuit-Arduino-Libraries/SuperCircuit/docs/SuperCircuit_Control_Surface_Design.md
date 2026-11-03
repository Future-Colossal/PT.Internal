# SuperCircuit Control-Surface Design

This document defines the intended production design for the `SuperCircuit` library.

It is not a one-off sketch plan.

It is the contract for how the board-runtime library should behave so technologists can build many different interactives quickly without editing library internals.

## Design Goal

`SuperCircuit` should be:

- a board-runtime hardware library
- sensor agnostic
- device agnostic
- reusable across many different exposed-pin combinations on the 3-button SuperCircuit board

`SuperCircuit` should not be:

- a flex-sensor library
- a distance-sensor library
- a game-specific framework
- a dumping ground for project-specific drivers

## Best-Practice Basis

This design follows the official Arduino library guidance:

- public API through a narrow top-level header
- implementation under `src/`
- explicit dependencies through `library.properties`
- examples used to teach usage

Source:

- Arduino library specification: https://docs.arduino.cc/arduino-cli/library-specification

The API style reference is Adafruit NeoPixel:

- one obvious include
- one obvious setup path
- clean named methods
- examples doing a lot of the teaching

Sources:

- Adafruit NeoPixel repository: https://github.com/adafruit/Adafruit_NeoPixel
- Adafruit NeoPixel class reference: https://adafruit.github.io/Adafruit_NeoPixel/html/class_adafruit___neo_pixel.html

## Core Boundary

The production split should be:

- `fcComms`
  - transports
  - device identity
  - network state
  - TCP / UDP / Serial / WebSerial / OTA
- `SuperCircuit`
  - onboard buttons
  - OLED shell
  - onboard status pixels
  - built-in pixel
  - DFPlayer shell
  - startup / operator UX
- project sketch
  - sensor libraries
  - sensor pins
  - thresholds and timing
  - project logic
  - event mapping

## Hide Plumbing, Expose Controls

The correct design rule is:

- hide the plumbing
- expose the controls

### Plumbing that should stay internal

- driver object instances
- low-level OLED rendering flow
- button poll/update internals
- DFPlayer transport setup
- NeoPixel lane application plumbing
- board-specific boot/update sequencing internals

### Controls that must stay easy to adjust

- title text
- persistent runtime text
- alert text and duration
- startup sequence options
- external and built-in pixel brightness
- interactive/comms/user pixel colors
- audio defaults
- whether volume pot following is enabled
- whether pixels/screen/audio are enabled at all
- what project events trigger shell feedback

## What Must Live In Config

If a project author is likely to want to change it per project, it should live in config.

That includes:

- device-facing default title
- default bottom/status line
- default on/off state
- hardware feature toggles
- display policy defaults
- pixel palette defaults
- audio defaults
- startup policy

Config should stay readable with:

- named blocks
- named assignments
- no positional bool soup

## What Must Be Adjustable At Runtime

These are not "internals". These are the active control surface.

Runtime methods should make it easy to:

- turn the interactive on or off
- show a temporary alert
- change title text
- change the bottom line
- hand in Wi-Fi / TCP state from `fcComms`
- change user pixel color
- play/stop audio
- change volume
- switch between loop and single modes

In other words, a technologist should be able to wire a new sensor and then choose:

- what shows on the OLED
- what pixel effect happens
- what audio happens
- what outbound message happens

without touching library internals.

## What Should Stay In The Main Sketch

Project-specific libraries should remain visible in the sketch.

Examples:

- HC-SR04 helper
- ToF sensor library
- I2C expander library
- ADS1115 / ADC expander
- seesaw button board
- external LED strip library
- relay or motor driver library

This is best practice for this platform because it keeps the real project dependency list obvious and lets technologists swap sensor approaches quickly.

## Generic Sensor Integration Pattern

The library should support this workflow cleanly:

1. Pick exposed pins from `SuperCircuitBoard.h`
2. Include the project-specific sensor library in the sketch if needed
3. Put sensor pin numbers and thresholds in the sketch config
4. Read the sensor in sketch code
5. Use `SuperCircuit` methods to express project feedback:
   - screen
   - user pixel
   - audio
   - local on/off state
6. Use `fcComms` to express project outbound messages

This keeps the board shell stable while letting the project logic stay completely flexible.

## Production-Friendly Control Areas

The library should continue evolving around these control areas:

### Display policy

- title default
- Wi-Fi line behavior
- idle line behavior
- alert timing
- optional future layout templates

### Pixel policy

- lane colors
- brightness
- default user-lane color
- optional future effect helpers

### Audio policy

- default volume
- volume bounds
- play mode
- amp enable
- optional track-label helpers

### Startup policy

- which steps run
- fast/slow/custom speed
- whether rainbow runs
- whether ready alerts show

### Button policy

- button callback routing
- predictable button numbering
- no raw debounce setup required in app code

## Board Truth

For the current 3-button board, the confirmed screw terminal is:

- `G 5V 3V RS VB EN 21 22`

That board truth should stay consistent across docs, examples, and future configs.

## Recommendation

Before more feature work, treat this as the library standard:

- `SuperCircuit` = board shell
- project sketch = project logic
- config = project tunables
- runtime methods = active control surface
- raw driver objects = internal plumbing

That is the production model most likely to stay flexible, understandable, and maintainable.

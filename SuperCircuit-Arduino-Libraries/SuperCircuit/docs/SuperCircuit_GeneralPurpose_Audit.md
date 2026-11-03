# SuperCircuit General-Purpose Audit

This document audits the current `SuperCircuit` library against the real target:

- a reusable board-level hardware/runtime library
- not a wrapper around any one example sketch
- fast for a technologist to configure
- safe for production reuse
- compatible with many sensor combinations on the exposed SuperCircuit board pins

## Short Verdict

The direction is now mostly right, but the library is not finished.

What is now in good shape:

- the public API is no longer tied to raw globals
- config is readable and named instead of positional-bool-heavy
- the library is clearly board-level, not project-level
- `fcComms` and `SuperCircuit` are more cleanly separated
- a board resource header now exists:
  - [libraries/SuperCircuit/src/SuperCircuitBoard.h](../libraries/SuperCircuit/src/SuperCircuitBoard.h)

What still needs improvement before this is a true production workhorse:

- more explicit support patterns for sensor-heavy app code
- stronger documentation around reserved vs preferred expansion pins
- hardware validation against the real board on the newest library pass
- possible future separation of optional subsystems into internal modules

## What "General Purpose" Means Here

For this codebase, general purpose does not mean the library should own every sensor.

It means:

- the library should own the board shell:
  - screen
  - buttons
  - status pixels
  - built-in pixel
  - DFPlayer
  - startup/operator UX
- the library should expose the real board resources cleanly
- app sketches should be able to add any sensor they want on top of that shell
- project sketches should not need to edit library internals just to wire a new sensor

That is consistent with the legacy repo:

- the shell is shared
- each interactive adds its own sensor logic, trigger logic, and outputs

## Audit Against Existing Sketch History

The old SuperCircuit and TCM sketches show recurring shell behavior:

- enable / disable interactive state
- OLED title + Wi-Fi line + TCP/system line + runtime/alert line
- fast local button actions
- alert overlays
- audio helpers
- comms state reflection
- startup readiness behavior

The old project sketches also show that sensor logic is highly project-specific:

- HC-SR04
- analog threshold sensors
- digital switches
- ToF sensors
- I2C button boards
- external ADC-based sensor arrays
- external LED strips

Conclusion:

- `SuperCircuit` should not try to absorb project-specific sensor logic
- it should make sensor integration easier by stabilizing the board shell and documenting the board resources

## Best-Practice Findings From Official Sources

### 1. Narrow public surface is correct

Arduino's library spec expects a library to expose a clean public header and keep implementation under `src/`.

Source:

- Arduino library specification: https://docs.arduino.cc/arduino-cli/library-specification

Why this matters here:

- app authors should use `SuperCircuit.h`
- they should not need to include private hardware headers directly just to use the board shell

### 2. Declared dependencies are correct and important

Arduino's spec explicitly supports `library.properties` metadata and `depends`.

Source:

- Arduino library specification, `library.properties` and `depends`: https://docs.arduino.cc/arduino-cli/library-specification

Why this matters here:

- we already saw this lesson on `fcComms`
- `SuperCircuit` should keep declaring its hardware dependencies cleanly

### 3. The NeoPixel-style API model is the right reference

The Adafruit NeoPixel library is a good reference because it gives users:

- one clear include
- one clear `begin()`
- named methods like `setPixelColor()`, `setBrightness()`, `show()`, `clear()`
- examples that teach usage

Sources:

- Adafruit NeoPixel repository: https://github.com/adafruit/Adafruit_NeoPixel
- Adafruit NeoPixel class reference: https://adafruit.github.io/Adafruit_NeoPixel/html/class_adafruit___neo_pixel.html

Why this matters here:

- `SuperCircuit` should feel like a board-runtime library
- it should not feel like a bag of ported globals from old `.ino` files

## Architectural Findings

### Good current decisions

- `fcComms` owns transport state
- `SuperCircuit` owns board/operator state
- config uses named blocks:
  - `features`
  - `display`
  - `pixel`
  - `audio`
  - `startup`
- the public API is now mostly shaped around real board behavior

### Sketch-level libraries should stay explicit

For production use, project-specific libraries should remain visible in the main sketch.

That means:

- `SuperCircuit` can keep its board-shell dependencies internal
- but sensor and project-output libraries should be included by the sketch author in the sketch

Examples that should usually remain sketch-visible:

- ToF sensor libraries
- HC-SR04 helpers
- ADC / I2C expander libraries
- seesaw button-board libraries
- project LED strip libraries
- relay or motor-controller libraries

Why this is the right split:

- technologists can see the real project dependency list quickly
- swapping sensor approaches stays easy
- the board shell does not become a dumping ground for every project dependency

### Important correction already made

`SuperCircuit` should not independently fetch network state from Wi-Fi internals when `fcComms` is already the source of truth.

That boundary is now improved:

- `fcComms` can pass IP/state in
- `SuperCircuit` renders it

That is a better separation of concerns.

## Hide Plumbing, Expose Controls

This is the production rule the library should be judged against.

What should stay hidden:

- raw OLED object access
- raw NeoPixel object access
- raw debounce objects
- DFPlayer setup and polling plumbing
- board-internal render/update flow

What must stay easy to adjust:

- pixel colors and brightness
- which pixel lane the project uses
- title and runtime text
- alert content and alert timing
- startup sequence items and speed
- audio defaults
- button reactions
- which project event triggers which shell response

That means the correct target is not "hide everything".

It is:

- hide the plumbing
- expose the controls

If a technologist needs to open `SuperCircuit.cpp` just to change a color, a title line, a startup behavior, or a track choice, the library design is not good enough yet.

## Board-Resource Audit

The biggest missing piece for a general-purpose board library was a stable board resource map.

That is now addressed by:

- [libraries/SuperCircuit/src/SuperCircuitBoard.h](../libraries/SuperCircuit/src/SuperCircuitBoard.h)

Why this matters:

- a technologist can now look up the board-owned pins and exposed pins in one public place
- app code can reference known board resources without scraping old example sketches

This is the right model for sensor-heavy future work:

- the shell owns the reserved board hardware
- the board header documents the expansion resources
- the app chooses sensors and wiring

## Current Gaps

These are the real remaining gaps from a general-purpose-production perspective.

### 1. No formal sensor helper layer yet

The board library is now a cleaner shell, but it does not yet provide optional helpers for common sensor patterns such as:

- debounced digital trigger
- analog threshold sensor
- analog mapped value sensor
- timed hold sensor

This is not a blocker, but it is a logical future addition if you want faster production bring-up.

### 2. No public "board profile" abstraction beyond the 3-button board

Right now this library is correctly rooted in the 3-button board hardware.

That is okay for now, but if future SuperCircuit revisions differ materially, a board-profile abstraction may eventually be useful.

### 3. No formal examples for common sensor integration patterns yet

Right now the main shell exists, but the next useful examples would be:

- analog input example
- digital trigger example
- mixed sensor example
- audio + alert + state example

### 4. Hardware validation still matters

This audit improves structure, but the latest `SuperCircuit` pass still needs real bench validation.

## Recommendation

Treat `SuperCircuit` as:

- the board-runtime shell
- the operator UX layer
- the board resource map

Do not treat it as:

- a monolithic sensor framework
- a direct port of any one TCM sketch

The right next steps are:

1. bench-test the latest `SuperCircuit` pass
2. tighten any parity issues
3. add one or two sensor-pattern examples
4. only then consider optional sensor helper types if they actually improve speed without bloating the library

## Final Assessment

The current direction is now aligned with the real goal:

- general board shell
- documented exposed resources
- project-specific sensor logic on top

That is the correct production model for this platform.

# SuperCircuit Example Button Actions

This note is important because it is easy to confuse **example-level defaults** with **library-level limits**.

## The Short Version

`SuperCircuit` does **not** limit the onboard board buttons to a fixed menu of actions.

What is currently standardized is only the **example sketch default behavior**.

That means:

- the `SuperCircuit` library exposes the button press event
- the example sketches provide a small config-driven action menu for common behavior
- project authors can still replace that logic entirely in the sketch

## Library Boundary

Inside the `SuperCircuit` library, the important part is this pattern:

- `superCircuit.onButtonPressed(...)`

The library handles:

- board button wiring
- button polling
- debounce/update plumbing
- calling your handler when a board button is pressed

The library does **not** say:

- "button 1 is always on/off"
- "button 2 is always play"
- "button 3 is always stop"
- "you may only choose from five built-in actions"

Those are example defaults, not hard library rules.

## Example Boundary

In the example sketches, there is now a small convenience enum:

```cpp
enum ExampleBoardButtonAction : uint8_t {
  ExampleBoardButtonNone = 0,
  ExampleBoardButtonToggleEnabled = 1,
  ExampleBoardButtonShowState = 2,
  ExampleBoardButtonPlayDefaultTrack = 3,
  ExampleBoardButtonStopAndRefresh = 4
};
```

And each example exposes:

- `BOARD_BUTTON_1_ACTION`
- `BOARD_BUTTON_2_ACTION`
- `BOARD_BUTTON_3_ACTION`

These are only there to make **common test/example remaps** fast and readable from `project_config.h`.

## Why This Exists

Without this layer, a technologist would need to edit sketch code for very common test changes like:

- "make button 1 toggle on/off"
- "make button 1 show state"
- "make button 2 play the default track"
- "make button 3 stop and refresh comms"

The example enum gives you a quick config-level remap for those cases.

## What You Can Change In Config

For the current examples, you can choose one of these defaults per onboard button:

- `ExampleBoardButtonNone`
- `ExampleBoardButtonToggleEnabled`
- `ExampleBoardButtonShowState`
- `ExampleBoardButtonPlayDefaultTrack`
- `ExampleBoardButtonStopAndRefresh`

Example:

```cpp
static const ExampleBoardButtonAction BOARD_BUTTON_1_ACTION = ExampleBoardButtonShowState;
static const ExampleBoardButtonAction BOARD_BUTTON_2_ACTION = ExampleBoardButtonPlayDefaultTrack;
static const ExampleBoardButtonAction BOARD_BUTTON_3_ACTION = ExampleBoardButtonStopAndRefresh;
```

That is a convenience feature for example sketches, not the final limit of the board.

## What You Can Change In Sketch Code

If the config menu is not enough, you can replace the example action logic directly in the sketch.

That means the sketch can do anything you want when a board button is pressed, for example:

- send custom messages
- react to sensors
- change OLED text
- trigger custom alerts
- play a specific audio track
- update user pixel colors
- toggle a relay
- run a mini state machine
- combine several actions at once

Example:

```cpp
void onButtonPressed(uint8_t index) {
  if (index == 1) {
    Comms.printlnAll("MY CUSTOM EVENT");
    superCircuit.setUserPixelColor(0x220044);
    superCircuit.showAlert("CUSTOM", "Button 1 fired", 1000);
  }
}
```

That is still the intended pattern for project-specific behavior.

## Recommended Interpretation

Use the config action enum when:

- you want a quick remap
- you are still testing
- the behavior is one of the common shell actions

Edit the sketch directly when:

- the project needs custom behavior
- a button should trigger something sensor- or interactive-specific
- you need several actions at once
- the project logic does not fit the default menu

## Production Rule

The rule to preserve is:

- `SuperCircuit` owns the board-button hardware plumbing
- examples may expose common defaults in config
- project sketches remain free to replace button behavior completely

That keeps the board shell reusable without turning the library into a rigid preset system.

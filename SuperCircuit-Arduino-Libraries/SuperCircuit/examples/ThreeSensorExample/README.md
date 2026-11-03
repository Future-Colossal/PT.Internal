# ThreeSensorExample

This example is the production-style mixed-sensor starting point for the SuperCircuit board.

## Files

- [project_config.h](./project_config.h)
- [ThreeSensorExample.ino](./ThreeSensorExample.ino)

## What it does

- flex sensor on `GPIO32`
- button on `GPIO21`
- reed switch on `GPIO19`
- routes all three through the same `SuperCircuit + fcComms` shell pattern
- disables volume-pot following so sensor behavior stays isolated while testing

## Edit These First

Open [project_config.h](./project_config.h) and set:

- `WIFI_SSID`
- `WIFI_PASSWORD`
- `WIFI_IP`
- `FLEX_SENSOR_PIN`
- `BUTTON_SENSOR_PIN`
- `REED_SWITCH_PIN`
- `FLEX_TRIGGER_THRESHOLD`
- `FLEX_RELEASE_THRESHOLD`
- `BUTTON_INPUT_MODE`
- `BUTTON_ACTIVE_STATE`
- `REED_INPUT_MODE`
- `REED_ACTIVE_STATE`
- `BUTTON_MESSAGE`
- `REED_MESSAGE`
- `BOARD_BUTTON_1_ACTION`
- `BOARD_BUTTON_2_ACTION`
- `BOARD_BUTTON_3_ACTION`

This example now fails at compile time if `WIFI_SSID` or `WIFI_PASSWORD` are still left as the placeholder values.

Default board-button behavior:

- button 1: toggle `ON` / `OFF`
- button 2: play default track
- button 3: stop audio + refresh comms

You can change that in `project_config.h`, or replace the handler logic directly in the sketch if a project needs something custom.

## Behavior

Priority order on the user pixel lane:

1. flex active: rainbow
2. reed closed: red
3. button pressed: neutral pulse color
4. idle: dim idle color

OLED bottom line shows:

- `F:ON/OFF`
- `B:1/0`
- `R:1/0`

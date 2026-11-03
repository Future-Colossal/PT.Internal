# FlexSensorExample

This example is the clean single-sensor starting point for an analog flex sensor on the SuperCircuit board.

## Files

- [project_config.h](./project_config.h)
- [FlexSensorExample.ino](./FlexSensorExample.ino)

## What it does

- reads the flex sensor on `GPIO32`
- shows raw flex value on the OLED bottom line
- triggers rainbow on the user pixel lane above threshold
- stops the rainbow below release threshold
- can play a track on trigger and optionally another on release
- disables volume-pot following so flex input cannot accidentally change audio volume
- includes the full `SuperCircuit_SC_Test` shell command surface so you can test board behavior and flex behavior in one sketch
- keeps line 2 reserved for true Wi-Fi status; `WIFI MSG` now uses only the alert line

## Edit These First

Open [project_config.h](./project_config.h) and set:

- `WIFI_SSID`
- `WIFI_PASSWORD`
- `WIFI_IP`
- `FLEX_SENSOR_PIN`
- `FLEX_TRIGGER_THRESHOLD`
- `FLEX_RELEASE_THRESHOLD`
- `FLEX_PLAY_SOUND_ON_TRIGGER`
- `FLEX_PLAY_SOUND_ON_RELEASE`
- `FLEX_TRIGGER_TRACK`
- `FLEX_RELEASE_TRACK`
- `BOARD_BUTTON_1_ACTION`
- `BOARD_BUTTON_2_ACTION`
- `BOARD_BUTTON_3_ACTION`

This example now fails at compile time if `WIFI_SSID` or `WIFI_PASSWORD` are still left as the placeholder values.

Default board-button behavior:

- button 1: toggle `ON` / `OFF`
- button 2: play default track
- button 3: stop audio + refresh comms

You can change that in `project_config.h`, or replace the handler logic directly in the sketch if a project needs something custom.

## Wiring

Use the flex sensor as a voltage divider into:

- signal to `GPIO32`
- power to `3V`
- ground to `G`

## Useful Commands

- full shell test set:
  `HELP PING ON OFF STATE NET ALERT TITLE LINE LINE CLEAR PIXEL PIXEL OFF COMMS PALETTE DEFAULT PALETTE DEMO WIFI MSG WIFI AUTO VOL <n> PLAY <n> STOP LOOP SINGLE RESET`
- sensor state:
  `STATE` also reports `FLEX_RAW` and `FLEX_ACTIVE`

Wi-Fi behavior:

- `WIFI MSG` should only use the alert line
- `WIFI AUTO` should refresh the real Wi-Fi status line

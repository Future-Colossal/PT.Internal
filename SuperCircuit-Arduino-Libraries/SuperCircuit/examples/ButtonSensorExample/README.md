# ButtonSensorExample

This example is the clean single-sensor starting point for a digital trigger button on the SuperCircuit board.

## Files

- [project_config.h](./project_config.h)
- [ButtonSensorExample.ino](./ButtonSensorExample.ino)

## What it does

- reads a button on `GPIO21`
- sends `SENT` when the button is pressed
- updates the user pixel lane while pressed
- shows `Button: idle` / `Button: pressed` on the OLED bottom line
- can play a track when the button is pressed
- disables volume-pot following so the example stays sensor-focused
- includes the full `SuperCircuit_SC_Test` shell command surface so you can test board behavior and button behavior in one sketch
- keeps line 2 reserved for true Wi-Fi status; `WIFI MSG` now uses only the alert line

## Edit These First

Open [project_config.h](./project_config.h) and set:

- `WIFI_SSID`
- `WIFI_PASSWORD`
- `WIFI_IP`
- `BUTTON_SENSOR_PIN`
- `BUTTON_INPUT_MODE`
- `BUTTON_ACTIVE_STATE`
- `BUTTON_MESSAGE`
- `BUTTON_PLAY_SOUND_ON_PRESS`
- `BUTTON_PRESS_TRACK`
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

Default expectation:

- one side of the button to `GPIO21`
- other side to `G`
- sketch uses `INPUT_PULLUP`

## Useful Commands

- full shell test set:
  `HELP PING ON OFF STATE NET ALERT TITLE LINE LINE CLEAR PIXEL PIXEL OFF COMMS PALETTE DEFAULT PALETTE DEMO WIFI MSG WIFI AUTO VOL <n> PLAY <n> STOP LOOP SINGLE RESET`
- button-specific:
  `SEND`

`STATE` includes button sensor state, and `SEND` lets you verify the comms/output path without pressing the physical button.

Wi-Fi behavior:

- `WIFI MSG` should only use the alert line
- `WIFI AUTO` should refresh the real Wi-Fi status line

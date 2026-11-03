# ReedSwitchExample

This example is the clean single-sensor starting point for a reed switch or other digital contact sensor on the SuperCircuit board.

## Files

- [project_config.h](./project_config.h)
- [ReedSwitchExample.ino](./ReedSwitchExample.ino)

## What it does

- reads the reed switch on `GPIO19`
- sends `APPROACHED` when the reed closes
- updates the user pixel lane while closed
- shows `Reed: open` / `Reed: closed` on the OLED bottom line
- can play a track on close and optionally another on open
- disables volume-pot following so the example stays sensor-focused
- includes the full `SuperCircuit_SC_Test` shell command surface so you can test board behavior and reed behavior in one sketch
- keeps line 2 reserved for true Wi-Fi status; `WIFI MSG` now uses only the alert line

## Edit These First

Open [project_config.h](./project_config.h) and set:

- `WIFI_SSID`
- `WIFI_PASSWORD`
- `WIFI_IP`
- `REED_SWITCH_PIN`
- `REED_INPUT_MODE`
- `REED_ACTIVE_STATE`
- `REED_MESSAGE`
- `REED_PLAY_SOUND_ON_CLOSE`
- `REED_PLAY_SOUND_ON_OPEN`
- `REED_CLOSE_TRACK`
- `REED_OPEN_TRACK`
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

- one side of the reed switch to `GPIO19`
- other side to `G`
- sketch uses `INPUT_PULLUP`

## Useful Commands

- full shell test set:
  `HELP PING ON OFF STATE NET ALERT TITLE LINE LINE CLEAR PIXEL PIXEL OFF COMMS PALETTE DEFAULT PALETTE DEMO WIFI MSG WIFI AUTO VOL <n> PLAY <n> STOP LOOP SINGLE RESET`
- reed-specific:
  `APPROACH`

`STATE` includes reed sensor state, and `APPROACH` lets you verify the comms/output path without physically closing the reed switch.

Wi-Fi behavior:

- `WIFI MSG` should only use the alert line
- `WIFI AUTO` should refresh the real Wi-Fi status line

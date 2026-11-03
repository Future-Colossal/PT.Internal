# SuperCircuit_SC_Test

This packaged doc mirrors the current `SuperCircuit_SC_Test` validation target so the board-shell baseline travels with the `SuperCircuit` library zip.

Purpose:

- use the working `fcComms` library as the communications backend
- use the `SuperCircuit` library as the hardware/UI surface under test
- keep the app surface small and close to the intended production shape:
  - one sketch
  - one config file

What this sketch is for:

- OLED wrapper validation
- legacy-style Wi-Fi line validation:
  - IP when connected
  - explicit not-connected line when Wi-Fi is down
- button callback validation
- on/off state validation
- user pixel and comms-status pixel validation
- DFPlayer wrapper validation
- volume-pot behavior validation
- config-driven startup behavior validation
- Wi-Fi line override validation
- scrolling alert validation on the OLED bottom line

What this sketch is not for:

- sensor-heavy interaction logic
- cluster behavior
- replacing every existing sketch immediately

Key files in the repo:

- `SuperCircuit_SC_Test/project_config.h`
- `SuperCircuit_SC_Test/SuperCircuit_SC_Test.ino`

Packaged copy:

- `examples/SuperCircuit_SC_Test/project_config.h`
- `examples/SuperCircuit_SC_Test/SuperCircuit_SC_Test.ino`

Useful commands:

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

Button defaults:

- button 1 shows state
- button 2 plays the default test track
- button 3 stops audio and refreshes comms status

Why it matters:

- this is the board-shell truth target for the current `SuperCircuit` handoff
- sensor examples should be judged against this baseline, not the other way around
- if a sensor example behaves oddly, compare it to `SuperCircuit_SC_Test` first

# SuperCircuit_SC_Test

This sketch is the dedicated validation target for the `SuperCircuit` library migration.

Purpose:

- use the working `fcComms` library as the communications backend
- use the `SuperCircuit` library as the hardware/UI surface under test
- keep the app surface small and close to the boss's desired end state:
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
- Wi-Fi line ownership validation
- scrolling alert validation on the OLED bottom line

What this sketch is not for yet:

- sensor-heavy interaction logic
- cluster behavior
- replacing every existing sketch immediately

Files:

- [project_config.h](./project_config.h)
- [SuperCircuit_SC_Test.ino](./SuperCircuit_SC_Test.ino)
- [SuperCircuit library header](../libraries/SuperCircuit/src/SuperCircuit.h)
- [SuperCircuit library implementation](../libraries/SuperCircuit/src/SuperCircuit.cpp)
- [fcComms manual test sketch](../SuperCircuit_fcComms_Test/README.md)
- [SuperCircuit manual test plan](../docs/SuperCircuit_Manual_Test_Plan.md)
- [SuperCircuit comprehensive test guide](../docs/SuperCircuit_Comprehensive_Test_Guide.md)

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
- `PIXEL`
- `PIXEL OFF`
- `COMMS`
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

Notes:

- WebSerial is still served by `fcComms` at `/webserial`
- this sketch is the main hardware-validation target for the current `SuperCircuit` library pass
- the library now covers a parity-first operator shell: OLED, buttons, pixels, alerts, comms-status reflection, on/off state, startup sequence, volume pot, and basic DFPlayer control
- line 2 of the OLED should show `WiFi: <ip>` when connected and `WiFi: not connected` when Wi-Fi is down
- `WIFI MSG` should only create a line-4 alert and should not overwrite line 2
- `WIFI AUTO` should refresh the real Wi-Fi status on line 2 and show only a transient alert on line 4
- `showAlert(...)` should scroll long `TITLE: BODY` messages across the bottom line instead of clipping them
- callback button numbers are now human-facing `1`, `2`, `3`
- the config is intentionally built with named assignments in `project_config.h` so future projects do not need to remember positional bool order

# SuperCircuit Example Sketches

These are the first validated example patterns built on top of the `SuperCircuit` hardware shell and `fcComms` backend.

## Examples

- `SuperCircuit_SC_Test`
  - packaged board-shell validation baseline
  - validates the full `SuperCircuit` shell without added project sensors
- `MinimalSuperCircuit`
  - smallest board-shell sanity check
- `FlexSensorExample`
  - analog flex sensor on explicit `GPIO32`
  - user pixel rainbow feedback
- `ButtonSensorExample`
  - digital button on explicit `GPIO21`
  - sends `SENT`
- `ReedSwitchExample`
  - reed switch on explicit `GPIO19`
  - sends `APPROACHED`
- `ThreeSensorExample`
  - flex on explicit `GPIO32`
  - button on explicit `GPIO21`
  - reed on explicit `GPIO19`
  - best starting point for mixed-sensor interactives

## Design Rule

Each example keeps:

- Wi-Fi SSID and password in `project_config.h`
- sensor pins in `project_config.h`
- thresholds and debounce in `project_config.h`
- project messages in `project_config.h`
- project reactions in the sketch

And leaves this in `SuperCircuit`:

- OLED shell
- alerts
- status pixels
- built-in pixel
- startup shell
- onboard buttons
- DFPlayer wrapper

That is the production pattern to preserve.

One important convention:

- example sketches should prefer explicit GPIO numbers in their main edit blocks
- the public `SuperCircuitBoard.h` map should also use explicit GPIO-based names rather than abstract nicknames or role-only aliases

Each sensor example now follows the preferred repo shape:

- one `.ino`
- one `project_config.h`
- one local `README.md`

`MinimalSuperCircuit` is the intentional exception because it remains a compact single-file sanity-check example.

## Important Note About Button Actions

The example `BOARD_BUTTON_1_ACTION`, `BOARD_BUTTON_2_ACTION`, and `BOARD_BUTTON_3_ACTION` values are **example-level convenience defaults**, not hard limits imposed by the `SuperCircuit` library.

Use them when a quick config remap is enough. Replace the sketch handler directly when a project needs custom button behavior.

See:

- [SuperCircuit example button actions](SuperCircuit_Example_Button_Actions.md)

## Relationship To `SuperCircuit_SC_Test`

Use `SuperCircuit_SC_Test` as the board-shell validation baseline.

It now exists in both places:

- repo root: [SuperCircuit_SC_Test](../SuperCircuit_SC_Test/README.md)
- packaged library example: [libraries/SuperCircuit/examples/SuperCircuit_SC_Test](../libraries/SuperCircuit/examples/SuperCircuit_SC_Test/README.md)

Use the example sketches when you want:

- the same broad shell behavior
- plus one sensor at a time, or all three together

Current interpretation:

- `SuperCircuit_SC_Test` is the board-shell truth target
- `SuperCircuit_SC_Test` should be treated as validation example zero before any sensor example
- `FlexSensorExample`, `ButtonSensorExample`, and `ReedSwitchExample` are shell-plus-single-sensor validation targets
- `ThreeSensorExample` is the shell-plus-mixed-sensor validation target

# SuperCircuit Docs

This folder packages the main `SuperCircuit` handoff, validation, board-reference, and example-boundary docs with the library itself.

Recommended reading order:

1. [Library_Handoff_Main.md](./Library_Handoff_Main.md)
2. [SuperCircuit_SC_Test.md](./SuperCircuit_SC_Test.md)
3. [SuperCircuit_Handoff_Guide.md](./SuperCircuit_Handoff_Guide.md)
4. [SuperCircuit_Manual_Test_Plan.md](./SuperCircuit_Manual_Test_Plan.md)
5. [SuperCircuit_Comprehensive_Test_Guide.md](./SuperCircuit_Comprehensive_Test_Guide.md)
6. [SuperCircuit_Example_Sketches.md](./SuperCircuit_Example_Sketches.md)
7. [SuperCircuit_Example_Button_Actions.md](./SuperCircuit_Example_Button_Actions.md)
8. [SuperCircuit_3Button_Pinout.md](./SuperCircuit_3Button_Pinout.md)
9. [SuperCircuit_3Button_Wiring_Quick_Ref.md](./SuperCircuit_3Button_Wiring_Quick_Ref.md)

Purpose:

- install the library with the docs included
- hand off the board shell without depending on the repo-level `docs` folder
- keep board truth, testing guidance, and example-boundary notes inside the library zip
- include the current `SuperCircuit_SC_Test` validation baseline directly in the packaged docs
- include the actual `SuperCircuit_SC_Test` sketch under `examples/SuperCircuit_SC_Test`

## Tested Dependency Versions

These are the dependency versions found in the current validated Arduino environment. They are documentation targets for handoff/testing, not hard-pinned version constraints enforced by the library code.

- ESP32 Arduino core: `3.0.7`
  - provides built-in platform libraries like `Wire`
- `Adafruit GFX Library`: `1.12.1`
- `Adafruit NeoPixel`: `1.12.5`
- `Adafruit SSD1306`: `2.5.13`
- `Bounce2`: `2.71`
- `DFRobot_DF1201S`: `1.0.2`

## Version Status

Latest upstream versions checked on May 11, 2026:

- ESP32 Arduino core: validated `3.0.7`, newer upstream `3.3.8`
- `Adafruit GFX Library`: validated `1.12.1`, newer upstream `1.12.4`
- `Adafruit NeoPixel`: validated `1.12.5`, newer upstream `1.15.2`
- `Adafruit SSD1306`: validated `2.5.13`, newer upstream at least `2.5.15` and repo page indicates an even newer `2.5.16`
- `Bounce2`: validated `2.71`, no newer tagged release was confirmed from the checked indexed sources
- `DFRobot_DF1201S`: validated `1.0.2`, no newer upstream release was confirmed from the checked indexed sources

Practical recommendation:

- treat the validated versions above as the handoff baseline
- do not upgrade just because newer releases exist unless you plan to re-run validation on hardware

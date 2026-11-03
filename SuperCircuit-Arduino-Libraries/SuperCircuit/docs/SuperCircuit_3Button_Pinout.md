# SuperCircuit 3-Button Pinout

This document covers the newer 3-button SuperCircuit board variant used by the current template-based sketches such as `SuperCircuit_Template`, `SuperCircuit_Distance_Test`, and the new library validation work.

It is based on:

- the pin usage declared in the repo's current 3-button SuperCircuit shell
- the newer board image with silkscreen labels visible in the IDE

## Scope

This pinout is for the 3-button board family only.

It does not describe the older 2-button switch variant used in `WifiPowerSwitch_3.10`.

## Core Reserved Pins

These pins are already claimed by the shared SuperCircuit runtime and should be treated as reserved unless you are intentionally modifying the board shell.

| Function | Pin | Notes |
| --- | --- | --- |
| External status NeoPixels | `13` | Shared SuperCircuit pixel string, `NUM_PIXELS = 3` |
| Local button A / `SW1` | `37` | Board silkscreen also shows `(21)` nearby, but current shell uses `37` |
| Local button B / `SW2` | `27` | SuperCircuit button input |
| Local button C / `SW3` | `34` | SuperCircuit button input |
| Volume pot `P2` | `39` | Used by DFPlayer volume logic when enabled |
| Pot `P1` | `36` | Additional pot location shown on board image |
| DFPlayer Pro RX | `7` | `Serial1` RX for DF1201S |
| DFPlayer Pro TX | `8` | `Serial1` TX for DF1201S |
| Built-in ESP32 pixel | `0` | Feather-style built-in NeoPixel |
| OLED / `LCD1` | I2C bus | Header shows `G`, `V+`, `SCL`, `SDA`; OLED address is `0x3C` |
| Large screw terminal | `G`, `5V`, `3V`, `RS`, `VB`, `EN`, `21`, `22` | Confirmed from corrected board breakdown, left to right |

## Pin Definitions In Code

The current 3-button shell declares:

- `PIXEL_PIN = 13`
- `BUTTON_A_PIN = 37`
- `BUTTON_B_PIN = 27`
- `BUTTON_C_PIN = 34`
- `POT_VOL_PIN = 39`
- `DF_RX_PIN = 7`
- `DF_TX_PIN = 8`
- OLED address `0x3C`

The shared runtime also creates a built-in pixel on pin `0`.

## What The Shell Actively Uses

In the current 3-button runtime:

- the three local buttons are debounced and handled continuously
- the external SuperCircuit status pixels are updated continuously
- the built-in pixel is used during boot and comms feedback
- the OLED is brought up over I2C
- `Serial1` is used for DFPlayer Pro when DFPlayer is enabled
- the volume pot input on `39` is read when DFPlayer support is enabled

## Confirmed Board Labels From The Newer Image

The newer board image adds several useful confirmations:

| Area | Confirmed labels |
| --- | --- |
| Pot connectors | `P1 = 36`, `P2 = 39` |
| DFPlayer serial/power header | `TX`, `RX`, `G`, `V+` |
| DFPlayer audio header | `L`, `GL`, `R`, `GR` |
| OLED header `LCD1` | `G`, `V+`, `SCL`, `SDA` |
| JST1 | `5`, `G`, `5V` |
| JST2 | `G`, `19`, `(18)`, `5V` |
| JST3 | `25`, `G`, `5V` |
| Wago terminal strip | `5V`, `3V`, `15`, `33`, `32`, `4`, `14` |
| Additional exposed pins | `D0`, `D1`, `15` |

## Expansion Pins Already Used Successfully In This Repo

These are not part of the shared SuperCircuit shell itself, but they are already used by template-based sketches as application pins:

| Sketch | Function | Pin(s) |
| --- | --- | --- |
| `SuperCircuit_Template` | Example analog sensor | `32` |
| `SuperCircuit_Template` | Example digital sensor | `33` |
| `SuperCircuit_Distance_Test` | HC-SR04 TRIG / ECHO | `32`, `33` |

Based on the current repo, `32` and `33` are still the cleanest starting expansion pins for quick sensor tests on the 3-button board family.

## Confirmed Exposed Connectors

From the board silkscreen and connector labels, these exposed groups are visible on the 3-button board:

| Connector area | Visible labels |
| --- | --- |
| Large screw terminal | `G`, `5V`, `3V`, `RS`, `VB`, `EN`, `21`, `22` |
| Wago terminal strip | `5V`, `3V`, `15`, `33`, `32`, `4`, `14` |
| JST1 | `5`, `G`, `5V` |
| JST2 | `G`, `19`, `(18)`, `5V` |
| JST3 | `25`, `G`, `5V` |
| Additional exposed pins | `D0`, `D1`, `15` |
| Pot connectors | `P1 = 36`, `P2 = 39` |

This means the board exposes more field-friendly I/O than the earliest code-only reading suggested.

## Practical Expansion Guidance

If you are building on the 3-button board shell and want to avoid conflicts, treat these as reserved first:

- `0`
- `7`
- `8`
- `13`
- `27`
- `34`
- `37`
- `39`
- the board-default I2C pins used by `Wire`

For quick sensor bring-up in this repo, the most proven general-purpose test pins are:

- `32`
- `33`

Other clearly exposed GPIOs visible on the board include:

- `4`
- `5`
- `14`
- `15`
- `18`
- `19`
- `21`
- `22`
- `25`

## Exposed Pin Recommendations

This is the practical "what should I use this for?" view for the exposed 3-button board pins.

### Best For Analog With Wi-Fi Enabled

These are the preferred exposed analog inputs because they are `ADC1` pins, which keep working reliably while Wi-Fi is active:

| Pin | Why it is good |
| --- | --- |
| `32` | Exposed, `ADC1`, proven in repo |
| `33` | Exposed, `ADC1`, proven in repo |
| `36` | Board image confirms `P1`; `ADC1`, good analog candidate if you wire to that pot/input location |
| `39` | Board image confirms `P2`; reserved by shell for volume pot in current runtime |

These are the first pins to reach for when you want photoresistors, flex sensors, pots, or other analog sensors and still want Wi-Fi/WebSerial/OTA enabled.

### Analog-Capable But Less Reliable With Wi-Fi

These exposed pins can read analog, but they are `ADC2` pins. On ESP32, `ADC2` and Wi-Fi do not play well together, so these are best used for analog only when Wi-Fi is disabled or when you can tolerate unreliable readings.

| Pin | Notes |
| --- | --- |
| `4` | Better used as digital in most demos |
| `14` | Usable for local analog tests, weak choice with Wi-Fi on |
| `15` | Better treated as digital first |
| `25` | Exposed JST pin, works for analog in local-only setups |

### Good For Digital Sensors And Triggers

These exposed pins are good candidates for switches, triggers, PIR outputs, reed switches, HC-SR04 control, and other digital signals:

| Pin | Notes |
| --- | --- |
| `4` | Good digital input/output |
| `5` | Good general digital pin |
| `22` | Exposed on screw terminal, good digital pin |
| `14` | Good digital pin if not needed for analog |
| `15` | Good digital pin if boot behavior is verified in your setup |
| `18` | Marked on JST2 in parentheses; treat as board-access dependent |
| `19` | Good general digital pin if physically accessible |
| `21` | Exposed on screw terminal, good digital pin |
| `25` | Good digital pin if physically accessible |
| `32` | Also works well as digital |
| `33` | Also works well as digital |

### Use With Caution

These pins are not automatically off-limits, but they deserve extra care:

| Pin | Reason |
| --- | --- |
| `15` | Can be board-dependent at boot, so verify it in your target build |
| `32`, `33`, `36` | Best analog candidates, so using them for digital may force analog sensors onto weaker pins |

### Reserved By The Shell

Do not plan new sensor wiring around these unless you are intentionally modifying the shared board runtime:

| Pin | Reason |
| --- | --- |
| `0` | built-in pixel / board behavior |
| `7`, `8` | DFPlayer UART |
| `13` | external status pixels |
| `27`, `34`, `37` | local board buttons |
| `39` | shell volume pot |

## Notes About I2C

The repo does not redefine SDA and SCL in the 3-button shell. The code calls `Wire.begin()` without custom I2C pin arguments, which means the board uses its default I2C mapping.

So this document can confirm:

- the OLED is on the I2C bus
- the OLED address is `0x3C`
- the shared OLED still lives on the board I2C bus through `LCD1`

The corrected connector breakdown means `JST1` should be treated as a `GPIO5` breakout with power, not as the OLED/I2C header.

## Recommended Workflow

When adding hardware to the 3-button SuperCircuit board:

1. Start from `SuperCircuit_Template` or `SuperCircuit_SC_Test`
2. Leave the reserved shell pins alone
3. Use `32` and `33` first for bench testing where possible
4. Treat `36` and `39` as high-value analog board resources
5. Only reuse shell pins if you are intentionally editing the shared runtime

## Related Files

- [SuperCircuit_Template/superCircuitGlobal.h](../SuperCircuit_Template/superCircuitGlobal.h)
- [SuperCircuit_Template/superCircuit.ino](../SuperCircuit_Template/superCircuit.ino)
- [SuperCircuit_SC_Test/README.md](../SuperCircuit_SC_Test/README.md)
- [SuperCircuit_Distance_Test/config.h](../SuperCircuit_Distance_Test/config.h)

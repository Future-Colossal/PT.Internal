# SuperCircuit 3-Button Wiring Quick Ref

This is a bench-side quick reference for the 3-button SuperCircuit board family used by `SuperCircuit_Template`, `SuperCircuit_fcComms_Test`, and `SuperCircuit_SC_Test`.

For the more detailed written version, see [SuperCircuit_3Button_Pinout.md](SuperCircuit_3Button_Pinout.md).

## Board-Level Quick View

```text
SUPERCIRCUIT 3-BUTTON BOARD

Reserved by shared shell:

  GPIO 0   -> Built-in status pixel
  GPIO 7   -> DFPlayer Pro RX
  GPIO 8   -> DFPlayer Pro TX
  GPIO 13  -> External SuperCircuit NeoPixels
  GPIO 27  -> Button B / SW2
  GPIO 34  -> Button C / SW3
  GPIO 37  -> Button A / SW1
  GPIO 39  -> Volume pot P2
  GPIO 36  -> Pot P1 location
  I2C      -> OLED at 0x3C
  Screw blk-> G, 5V, 3V, RS, VB, EN, 21, 22

Useful exposed groups:

  Wago     -> 5V, 3V, 15, 33, 32, 4, 14
  JST1     -> 5, G, 5V
  JST2     -> G, 19, (18), 5V
  JST3     -> 25, G, 5V
  Extra    -> D0, D1, 15

Best proven expansion/test pins in this repo:

  GPIO 32  -> General sensor/test pin
  GPIO 33  -> General sensor/test pin
```

## Bench Wiring Map

```text
                        +---------------------------+
                        |  SuperCircuit 3-Button    |
                        |                           |
                        |  OLED     -> LCD1 / I2C   |
                        |  Pixel    -> GPIO 13      |
                        |  Btn A    -> GPIO 37      |
                        |  Btn B    -> GPIO 27      |
                        |  Btn C    -> GPIO 34      |
                        |  Pot P2   -> GPIO 39      |
                        |  Pot P1   -> GPIO 36      |
                        |  DF RX    -> GPIO 7       |
                        |  DF TX    -> GPIO 8       |
                        |  Built-in -> GPIO 0       |
                        |                           |
                        |  Free/Test -> GPIO 32     |
                        |  Free/Test -> GPIO 33     |
                        +---------------------------+
```

## Reserved Vs Expansion

| Group | Pins |
| --- | --- |
| Reserved by shell | `0`, `7`, `8`, `13`, `27`, `34`, `37`, `39`, board-default I2C pins |
| Best current expansion pins | `32`, `33` |
| Other clearly exposed expansion pins | `4`, `5`, `14`, `15`, `18`, `19`, `21`, `22`, `25`, `36` |

## Common Test Wiring

## Analog Sensor

```text
Sensor analog out -> GPIO 32
Sensor GND        -> GND
Sensor VCC        -> 3.3V
```

## HC-SR04 Test

```text
TRIG -> GPIO 32
ECHO -> GPIO 33
VCC  -> 3.3V
GND  -> GND
```

Note:

- Some HC-SR04 boards work poorly at `3.3V`.
- If readings are unstable, power is the first thing to re-check.

## I2C Peripheral Test

```text
SIG -> JST1 pin 5
GND -> JST1 G
VCC -> JST1 5V
```

## Keep Clear

Avoid using these unless you are intentionally editing the shared SuperCircuit runtime:

- `0`
- `7`
- `8`
- `13`
- `27`
- `34`
- `37`
- `39`
- the default I2C pins used by `Wire`

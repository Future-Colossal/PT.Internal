#pragma once

#include <Arduino.h>
#include <stdint.h>

// SuperCircuit 3-button board resource map.
// This header is intentionally public so project authors can wire sensors and
// peripherals against the actual exposed board resources without digging
// through library internals or old example sketches.
namespace SuperCircuitBoard {

namespace Reserved {
constexpr uint8_t BuiltInPixel = 0;
constexpr uint8_t DfPlayerRx = 7;
constexpr uint8_t DfPlayerTx = 8;
constexpr uint8_t StatusPixels = 13;
constexpr uint8_t Button1 = 37;
constexpr uint8_t Button2 = 27;
constexpr uint8_t Button3 = 34;
constexpr uint8_t Gpio36 = 36;  // Board pot/header location P1
constexpr uint8_t Gpio39 = 39;  // Board pot/header location P2, shell volume path
}

namespace Exposed {
namespace ScrewTerminal {
constexpr uint8_t Gpio21 = 21;
constexpr uint8_t Gpio22 = 22;
}

namespace Wago {
constexpr uint8_t Gpio15 = 15;
constexpr uint8_t Gpio33 = 33;
constexpr uint8_t Gpio32 = 32;
constexpr uint8_t Gpio4 = 4;
constexpr uint8_t Gpio14 = 14;
}

namespace JST {
constexpr uint8_t Gpio5 = 5;
constexpr uint8_t Gpio19 = 19;
constexpr uint8_t Gpio18 = 18;
constexpr uint8_t Gpio25 = 25;
}

namespace Additional {
constexpr uint8_t D0 = 0;
constexpr uint8_t D1 = 1;
constexpr uint8_t Gpio15 = 15;
}

namespace Common {
constexpr uint8_t Gpio4 = 4;
constexpr uint8_t Gpio5 = 5;
constexpr uint8_t Gpio14 = 14;
constexpr uint8_t Gpio15 = 15;
constexpr uint8_t Gpio19 = 19;
constexpr uint8_t Gpio21 = 21;
constexpr uint8_t Gpio22 = 22;
constexpr uint8_t Gpio25 = 25;
constexpr uint8_t Gpio32 = 32;
constexpr uint8_t Gpio33 = 33;
}
}

namespace Counts {
constexpr uint8_t StatusPixelCount = 3;
constexpr uint8_t ButtonCount = 3;
}

}

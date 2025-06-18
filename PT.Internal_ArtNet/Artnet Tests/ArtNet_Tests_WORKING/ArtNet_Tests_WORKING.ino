// ArtNetWS2812
// Requires Teensy 4.1 + Teensy Ethernet Kit
// Make sure to alter the "Artnet" library that ships with Teensyduino to point to "<NativeEthernet.h>" & "<NativeEthernetUdp.h>"
// Code adapted from the original example: https://github.com/natcl/Artnet/blob/master/examples/NeoPixel/ArtnetNeoPixel/ArtnetNeoPixel.ino

/*
This example will receive multiple universes via Artnet and control a strip of ws2811 leds via 
Adafruit's NeoPixel library: https://github.com/adafruit/Adafruit_NeoPixel
This example may be copied under the terms of the MIT license, see the LICENSE file for details
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define USE_ADAFRUIT_GFX_LAYERS  //HUB75

#include <Artnet.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
//#include <SPI.h>
//#include <Adafruit_NeoPixel.h>
//HUB75-Related
#include <MatrixHardware_Teensy4_ShieldV5.h>  // SmartLED Shield for Teensy 4 (V5)
#include <SmartMatrix.h>                      //HUB75 driver

//HUB75 Panel Settings
#define COLOR_DEPTH 24              // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)
const uint16_t kMatrixWidth = 64;   // Set to the width of your display, must be a multiple of 8
const uint16_t kMatrixHeight = 32;  // Set to the height of your display
const uint8_t kRefreshDepth = 36;   // Tradeoff of color quality vs refresh rate, max brightness, and RAM usage.  36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up to 48: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;   // known working: 2-4, use 2 to save RAM, more to keep from dropping frames and automatically lowering refresh rate.  (This isn't used on ESP32, leave as default)
//const uint8_t kPanelType = SM_PANELTYPE_HUB75_16ROW_MOD8SCAN;                                                   // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint8_t kPanelType = SM_PANELTYPE_HUB75_32ROW_MOD16SCAN;                                                  // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_BOTTOM_TO_TOP_STACKING | SM_HUB75_OPTIONS_C_SHAPE_STACKING);  // see docs for options:
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);
const uint8_t kIndexedLayerOptions = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);

SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

#ifdef USE_ADAFRUIT_GFX_LAYERS
// there's not enough allocated memory to hold the long strings used by this sketch by default, this increases the memory, but it may not be large enough
SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, 6 * 1024, 1, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer01, kMatrixWidth, kMatrixHeight, 6 * 1024, 1, COLOR_DEPTH, kScrollingLayerOptions);

#else
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer01, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);

#endif

SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);

const int defaultBrightness = (100 * 255) / 100;  // full (100%) brightness
//const int defaultBrightness = (15*255)/100;       // dim: 15% brightness
const int defaultScrollOffset = 6;
const rgb24 defaultBackgroundColor = { 0, 0, 0 };

// Neopixel settings
const int numLeds = 2048;                 // change for your setup
const int numberOfChannels = numLeds * 3;  // Total number of channels you want to receive (1 led = 3 channels)
//const byte dataPin = 3;
//Adafruit_NeoPixel leds = Adafruit_NeoPixel(numLeds, dataPin, NEO_GRB + NEO_KHZ800);

// Artnet settings
Artnet artnet;
const int startUniverse = 0;  // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;
int row;

// Change ip and mac address for your setup
byte ip[] = { 2, 0, 0, 201 }; //2.0.0.201
byte mac[] = { 0x04, 0xE9, 0xE5, 0x00, 0x69, 0xEC };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  artnet.begin(mac, ip);
  // leds.begin();
  //initTest();
  Serial.println("Hello, ArtNet Node Starting...");

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);

  matrix.addLayer(&backgroundLayer);
  matrix.begin();
  matrix.setBrightness(255);
  backgroundLayer.fillScreen({ 0, 0, 0 });
  backgroundLayer.enableColorCorrection(false);
  matrix.setRotation(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  // we call the read function inside the loop
  artnet.read();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
  sendFrame = 1;
  // set brightness of the whole strip
  // if (universe == 15) {
  //   leds.setBrightness(data[0]);
  //   leds.show();
  // }

  //Store which universe has got in
  if ((universe - startUniverse) < maxUniverses)
    universesReceived[universe - startUniverse] = 1;

  for (int i = 0; i < maxUniverses; i++) {
    if (universesReceived[i] == 0) {
      Serial.println("Something is broken...");
      Serial.println(i);
      sendFrame = 0;
      break;
    }
  }

  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++) {
    int led = i + (universe - startUniverse) * (previousDataLength / 3);
    rgb24 pixel_color = { data[i * 3], data[i * 3 + 1], data[i * 3 + 2] };
    row = led/64;
    if (led <= numLeds) {
      backgroundLayer.drawPixel(led%64, row, pixel_color);
    }
    // if (led>63 && led <= 128) {
    //   Serial.println("Row 2");
    //   backgroundLayer.drawPixel(led%63, 1, pixel_color);
    // }
    //leds.setPixelColor(led, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
  }
  previousDataLength = length;

  if (sendFrame) {
    //leds.show();
    // Reset universeReceived to 0
    backgroundLayer.swapBuffers();
    memset(universesReceived, 0, maxUniverses);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// void initTest() {
//   for (int i = 0; i < numLeds; i++)
//     leds.setPixelColor(i, 127, 0, 0);
//   leds.show();
//   delay(500);
//   for (int i = 0; i < numLeds; i++)
//     leds.setPixelColor(i, 0, 127, 0);
//   leds.show();
//   delay(500);
//   for (int i = 0; i < numLeds; i++)
//     leds.setPixelColor(i, 0, 0, 127);
//   leds.show();
//   delay(500);
//   for (int i = 0; i < numLeds; i++)
//     leds.setPixelColor(i, 0, 0, 0);
//   leds.show();
// }
#include <HCSR04.h>
#include <FastLED.h>
#include "fx/1d/pride2015.h"

using namespace fl;

bool low_flag = 0;
bool high_flag = 0;
float low_thresh = 75.0;
float high_thresh = 75.0;
bool last_trig = 0;

#define NUM_LEDS 190
#define DATA_PIN 21
CRGB leds[NUM_LEDS];
#define BRIGHTNESS 255

Pride2015 pride(NUM_LEDS);
UltraSonicDistanceSensor lowSensor(12, 27);   // Initialize sensor that uses digital pins 13 and 12.
UltraSonicDistanceSensor highSensor(33, 15);  // Initialize sensor that uses digital pins 13 and 12.

unsigned long previousLowMillis = 0;   // will store last time LED was updated
unsigned long previousHighMillis = 0;  // will store last time LED was updated
const long interval = 50;              // interval at which to blink (milliseconds)

void setup() {
  Serial.begin(115200);  // We initialize serial connection so that we could print values from sensor.
  delay(1000);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  // Every 500 miliseconds, do a measurement using the sensor and print the distance in centimeters.
  checkSensors();
}

void checkSensors() {
  float low_reading = lowSensor.measureDistanceCm();
  delay(10);
  float high_reading = highSensor.measureDistanceCm();
  delay(10);
  Serial.printf("The low reading is %f\r\n", low_reading);
  Serial.printf("The high reading if %f\r\n", high_reading);

  if (low_reading < low_thresh && !low_flag) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousLowMillis >= interval) {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(0, 255, i);
        delay(25);
        FastLED.show();
      }
      low_flag = 1;
      last_trig = 0;
      previousLowMillis = currentMillis;
    }
  } else if (low_reading >= low_thresh && low_flag && !last_trig) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousLowMillis >= interval) {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
        delay(25);
        FastLED.show();
      }
      low_flag = 0;
      previousLowMillis = currentMillis;
    }
  } else if (high_reading < high_thresh && !high_flag) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousHighMillis >= interval) {
      for (int i = NUM_LEDS; i >= 0; i--) {
        //leds[i] = CRGB::Blue;
        leds[i] = CRGB(i, 255 - i, 255);
        //leds[i] = CRGB(((i + 85) % 255), ((i + 170) % 255), 255);
        delay(33);
        FastLED.show();
      }
      high_flag = 1;
      last_trig = 1;
      previousHighMillis = currentMillis;
    }
  } else if (high_reading >= high_thresh && high_flag && last_trig) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousHighMillis >= interval) {
      for (int i = NUM_LEDS; i >= 0; i--) {
        leds[i] = CRGB::Black;
        delay(33);
        FastLED.show();
      }
      high_flag = 0;
      previousHighMillis = currentMillis;
    }
  }
}
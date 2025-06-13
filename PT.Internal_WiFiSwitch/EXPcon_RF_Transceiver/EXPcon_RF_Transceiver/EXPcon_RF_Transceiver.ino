// rf69 demo tx rx.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable
// messaging client with the RH_RF69 class.
// It is designed to work with the other example RadioHead69_AddrDemo_RX.
// Demonstrates the use of AES encryption, setting the frequency and
// modem configuration.

#include <SPI.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>

/************ Radio Setup ***************/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF69_FREQ 915.0

// Who am i? (server address)
#define MY_ADDRESS 1

// Where to send packets to! MY_ADDRESS in client (RX) should match this.
#define DEST_ADDRESS 201

// First 3 here are boards w/radio BUILT-IN. Boards using FeatherWing follow.
#if defined(__AVR_ATmega32U4__)  // Feather 32u4 w/Radio
#define RFM69_CS 8
#define RFM69_INT 7
#define RFM69_RST 4
#define LED 13

#elif defined(ADAFRUIT_FEATHER_M0) || defined(ADAFRUIT_FEATHER_M0_EXPRESS) || defined(ARDUINO_SAMD_FEATHER_M0)  // Feather M0 w/Radio
#define RFM69_CS 8
#define RFM69_INT 3
#define RFM69_RST 4
#define LED 13

#elif defined(ARDUINO_ADAFRUIT_FEATHER_RP2040_RFM)  // Feather RP2040 w/Radio
#define RFM69_CS 16
#define RFM69_INT 21
#define RFM69_RST 17
#define LED LED_BUILTIN

#elif defined(__AVR_ATmega328P__)  // Feather 328P w/wing
#define RFM69_CS 4                 //
#define RFM69_INT 3                //
#define RFM69_RST 2                // "A"
#define LED 13

#elif defined(ESP8266)  // ESP8266 feather w/wing
#define RFM69_CS 2      // "E"
#define RFM69_INT 15    // "B"
#define RFM69_RST 16    // "D"
#define LED 0

#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2) || defined(ARDUINO_NRF52840_FEATHER) || defined(ARDUINO_NRF52840_FEATHER_SENSE)
#define RFM69_CS 10   // "B"
#define RFM69_INT 9   // "A"
#define RFM69_RST 11  // "C"
#define LED 13

#elif defined(ESP32)  // ESP32 feather w/wing
#define RFM69_CS 33   // "B"
#define RFM69_INT 27  // "A"
#define RFM69_RST 13  // same as LED
#define LED 13

#elif defined(ARDUINO_NRF52832_FEATHER)  // nRF52832 feather w/wing
#define RFM69_CS 11                      // "B"
#define RFM69_INT 31                     // "C"
#define RFM69_RST 7                      // "A"
#define LED 17

#endif

/* Teensy 3.x w/wing
#define RFM69_CS     10  // "B"
#define RFM69_INT     4  // "C"
#define RFM69_RST     9  // "A"
#define RFM69_IRQN   digitalPinToInterrupt(RFM69_INT)
*/

/* WICED Feather w/wing
#define RFM69_CS     PB4  // "B"
#define RFM69_INT    PA15 // "C"
#define RFM69_RST    PA4  // "A"
#define RFM69_IRQN   RFM69_INT
*/

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);

int16_t packetnum = 0;  // packet counter, we increment per xmission

void setup() {
  Serial.begin(115200);
  //while (!Serial) delay(1); // Wait for Serial Console (comment out line if no computer)

  pinMode(LED, OUTPUT);
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  Serial.println("Feather Addressed RFM69 TX Test!");
  Serial.println();

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  if (!rf69_manager.init()) {
    Serial.println("RFM69 radio init failed");
    while (1)
      ;
  }
  Serial.println("RFM69 radio init OK!");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the client
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
  rf69.setEncryptionKey(key);

  Serial.print("RFM69 radio @");
  Serial.print((int)RF69_FREQ);
  Serial.println(" MHz");
}

// Dont put this on the stack:
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
uint8_t data[] = "  OK";

void loop() {
  //delay(1000);  // Wait 1 second between transmits, could also 'sleep' here!

  receiveSerial();
  // Send a message to the DESTINATION!
}

void Blink(byte pin, byte delay_ms, byte loops) {
  while (loops--) {
    digitalWrite(pin, HIGH);
    delay(delay_ms);
    digitalWrite(pin, LOW);
    delay(delay_ms);
  }
}
void receiveSerial() {
  while (Serial.available()) {
    String str = Serial.readStringUntil('\n');
    processInput(str);
  }
}


void processInput(String input) {
  int commaIndex = input.indexOf(',');  // Find the position of the comma

  if (commaIndex != -1) {
    // Extract the channel number
    String channelString = input.substring(0, commaIndex);
    int channel = channelString.toInt();

    // Extract the message
    String message = input.substring(commaIndex + 1);

    // Print the extracted channel and message
    Serial.print("Channel: ");
    Serial.println(channel);
    Serial.print("Message: ");
    Serial.println(message);
    sendRF(channel, message);
  } else {
    Serial.println("Invalid format. Please use 'channel,message'.");
  }
}


void sendRF(int channel, String msg) {


  
  char data[msg.length() + 1];
  msg.toCharArray(data, sizeof(data));


  if (rf69_manager.sendtoWait((uint8_t *)data, strlen(data), channel)) {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (rf69_manager.recvfromAckTimeout(buf, &len, 2000, &from)) {
      buf[len] = 0;  // zero out remaining string

      Serial.print("Got reply from #");
      Serial.print(from);
      Serial.print(" [RSSI :");
      Serial.print(rf69.lastRssi());
      Serial.print("] : ");
      Serial.println((char *)buf);
      Blink(LED, 40, 3);  // blink LED 3 times, 40ms between blinks
    } else {
      Serial.println("No reply, is anyone listening?");
    }
  } else {
    Serial.println("Sending failed (no ack)");
  }
  delay(1000);
}
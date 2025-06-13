


/*
// ESP32 feather w/wing
#define RFM69_CS 10   // "B"
#define RFM69_INT 9   // "A"
#define RFM69_RST 11  // same as LED
#define LED 11
*/

// ESP32 feather w/wing
#define RFM69_CS 33   // "B"
#define RFM69_INT 15  // "A"
#define RFM69_RST 27  // C
#define LED 13

#define RF69_FREQ 915.0
RH_RF69 rf69(RFM69_CS, RFM69_INT);
RHReliableDatagram rf69_manager(rf69, myID);
uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];

void receiveRF() {  // This is a task.


  if (rf69_manager.available()) {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (rf69_manager.recvfromAck(buf, &len, &from)) {
      // buf[len] = 0;  // zero out remaining string

      // Serial.print("Got packet from #");
      // Serial.print(from);
      // Serial.print(" [RSSI :");
      // Serial.print(rf69.lastRssi());
      // Serial.print("] : ");
      // Serial.println((char *)buf);

      buf[len] = 0;
      Serial.print("RF received: ");
      Serial.println((char *)buf);
      String s(buf, len);
      messageReceived(s);

      Blink(LED, 40, 3);  // blink LED 3 times, 40ms between blinks

      String reply = String(s) + " received";
      char data[reply.length()+1];
      reply.toCharArray(data, sizeof(data));

      // Send a reply back to the originator client
      if (!rf69_manager.sendtoWait((uint8_t *)data, sizeof(data), from))
        Serial.println("Sending failed (no ack)");
    }
  }




  if (rf69.available()) {
    // Should be a message for us now
    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf69.recv(buf, &len)) {
      //digitalWrite(LED_BUILTIN, HIGH);
      //RH_RF95::printBuffer("Received: ", buf, len);
      buf[len] = 0;
      Serial.print("RF received: ");
      Serial.println((char *)buf);
      String s(buf, len);

      //unpackJson(s);
      //Serial.print("RSSI: ");
      //Serial.println(rf95.lastRssi(), DEC);
      //Blink(LED, 20, 2);  // blink LED 3 times, 50ms between blinks
    } else {
      Serial.println("Receive failed");
    }
    delay(10);
  }
}

void rfSetup() {
  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  if (!rf69_manager.init()) {
    Serial.println("RFM69 radio init failed");

  } else {
    Serial.println("RFM69 radio init OK!");
    RF = true;
    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
    // No encryption
    if (!rf69.setFrequency(RF69_FREQ)) {
      Serial.println("setFrequency failed");
    }

    // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
    // ishighpowermodule flag set like this:
    rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

    // The encryption key has to be the same as the one in the server
    uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                      0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
    rf69.setEncryptionKey(key);

    Serial.print("RFM69 radio @");
    Serial.print((int)RF69_FREQ);
    Serial.println(" MHz");
  }
}

void sendRFpacket(String message) {
  char radiopacket[message.length() + 1];  // +1 for the null terminator
  strcpy(radiopacket, message.c_str());
  Serial.print("Sending ");
  Serial.println(radiopacket);
  rf69.send((uint8_t *)radiopacket, message.length() + 1);
  //  Serial.println("Waiting for packet to complete...");
  rf69.waitPacketSent();
}

void Blink(byte pin, byte delay_ms, byte loops) {
  while (loops--) {
    digitalWrite(pin, HIGH);
    delay(delay_ms);
    digitalWrite(pin, LOW);
    delay(delay_ms);
  }
}

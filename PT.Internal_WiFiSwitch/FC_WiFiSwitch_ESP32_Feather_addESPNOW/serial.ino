
/*void recvWebSerialMsg(uint8_t *data, size_t len) {
  String d = "";
  for (int i = 0; i < len; i++) {
    d += char(data[i]);
  }
  messageReceived(d);
}*/

void webSerialSetup() {

  WebSerial.begin(&httpServer);
  /* Attach Message Callback */
  // WebSerial.msgCallback(recvWebSerialMsg);
  // httpServer.begin();


  WebSerial.onMessage([&](uint8_t *data, size_t len) {
    //Serial.printf("Received %u bytes from WebSerial: ", len);
    //Serial.write(data, len);
    //Serial.println();
    //WebSerial.println("Received Data...");
    String d = "";
    for (size_t i = 0; i < len; i++) {
      d += char(data[i]);
    }
    messageReceived(d);
    //WebSerial.println(d);
  });

  httpServer.begin();

  Serial.println("WebSerial Begin");
}

void receiveSerial() {
  while (Serial.available()) {
    String str = Serial.readStringUntil('\n');
    messageReceived(str);
  }
}

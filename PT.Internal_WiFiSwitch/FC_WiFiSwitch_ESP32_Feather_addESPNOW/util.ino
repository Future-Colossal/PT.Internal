

void printLogs() {
  broadcast("Time since boot: ");
  broadcastln(timeFromMillis(millis()));

  broadcast("Time on WiFi: ");
  broadcastln(timeFromMillis(millis() - lastWifiConnectionTime));

  broadcast("Time since last message received: ");
  broadcastln(timeFromMillis(millis() - lastMsgTime));

  broadcast("State: ");
  broadcastln(String(state));

  broadcast("Last message receceived: ");
  broadcastln(lastMsg);
}

void commsSetup() { //initializes various communications protocols
  rfSetup();
  wifiSetup();
  webSerialSetup();
  esp_nowSetup();  
  if (WIFI) {
    OTAsetup();
  }
  xTaskCreate(TaskTCPConnection, "Task TCP", 2048, NULL, 1, NULL);
}

void receiveComms() {
  manageOTA();
  if (TCP) receiveTCP();
  if (RF) receiveRF();
  receiveESP(); //only check ESP-NOW messages if TCP connection drops. 
  receiveSerial();
}



String timeFromMillis(unsigned long millis) {
  // Constants for time conversion
  const unsigned long msPerSecond = 1000;
  const unsigned long msPerMinute = msPerSecond * 60;
  const unsigned long msPerHour = msPerMinute * 60;
  const unsigned long msPerDay = msPerHour * 24;

  // Calculate days, hours, minutes, and seconds
  unsigned long days = millis / msPerDay;
  millis %= msPerDay;  // remaining milliseconds after calculating days

  unsigned long hours = millis / msPerHour;
  millis %= msPerHour;  // remaining milliseconds after calculating hours

  unsigned long minutes = millis / msPerMinute;
  millis %= msPerMinute;  // remaining milliseconds after calculating minutes

  unsigned long seconds = millis / msPerSecond;

  String time = "";

  // Print the results to the Serial Monitor
  time.concat("Days: ");
  time.concat(days);
  time.concat(", Hours: ");
  time.concat(hours);
  time.concat(", Minutes: ");
  time.concat(minutes);
  time.concat(", Seconds: ");
  time.concat(seconds);

  return time;
}

void broadcast(String s) {
  Serial.print(s);
  WebSerial.print(s);
  if (TCP) {
    client.print(s);
  }
}

void broadcastln(String s) {
  Serial.println(s);
  WebSerial.println(s);
  if (TCP) {
    client.println(s);
  }
}


void monitorConnections() {

  Serial.print("WIFI: ");
  Serial.println(WIFI);
  Serial.print("RF: ");
  Serial.println(RF);
  Serial.print("TCP: ");
  Serial.println(TCP);
}

void powerCycle() {
  ESP.restart();
}

void updateLEDs() {
  if (WIFI) {
    pixels.setPixelColor(1, pixels.Color(0, 150, 0));
  } else {
    pixels.setPixelColor(1, pixels.Color(0, 0, 150));
  }

  if (state) {
    pixels.setPixelColor(0, pixels.Color(0, 150, 0));
  } else {
    pixels.setPixelColor(0, pixels.Color(150, 0, 0));
  }
  pixels.show();
}

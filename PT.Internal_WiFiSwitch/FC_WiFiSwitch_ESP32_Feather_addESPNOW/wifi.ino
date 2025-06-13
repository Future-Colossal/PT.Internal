
void wifiSetup() {
  // Connect to the WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(local_IP, gateway, subnet);

  // Wait for WiFi connection
  int i;
  Serial.print("Connecting to wifi");
  while (WiFi.status() != WL_CONNECTED && i++ < 20) {

    delay(100);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi connection failed");
  } else {
    Serial.println("\nConnected to " + String(ssid));
    Serial.println("IP address: " + String(WiFi.localIP().toString()));
    lastWifiConnectionTime = millis();
    tcpServer.begin();
    //Serial.println("Open Telnet and connect to IP:" + String(WiFi.localIP().toString()) + " on port " + String(tcpPort));
  }
}

void TaskTCPConnection(void *pvParameters) {  // This is a task. // uses freeRTOS library
  (void)pvParameters;
  while (1) {
    if (WiFi.status() == WL_CONNECTED) {
      WIFI = true;
      client = tcpServer.available();
      if (client) {
        while (client.connected()) {
          TCP = true;
          vTaskDelay(100);
        }
        TCP = false;
      }
    } else {
      WIFI = false;
      wifiSetup();
    }
  }
}

//Having issues with ASyncTCP library in the 3.0 ESP32 core for Arduino IDE. 

void receiveTCP() {
  if (client.available()) {
    int bytesAvailable = client.available();
    String message;
    if (bytesAvailable < 2) {
      client.flush();
    } else {
      message = client.readStringUntil('\n');
      //Serial.print("TCP received: ");
      //Serial.println(message);
      messageReceived(message);
    }
  }
}

void rejoinWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.config(local_IP, gateway, subnet);

  // Wait for WiFi connection
  int i;
  Serial.print("Connecting to wifi");
  while (WiFi.status() != WL_CONNECTED && i++ < 20) {
    delay(100);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wifi connection failed");
  } else {
    Serial.println("\nConnected to " + String(ssid));
    Serial.println("IP address: " + String(WiFi.localIP().toString()));
    lastWifiConnectionTime = millis();

    tcpServer.begin();
    //Serial.println("Open Telnet and connect to IP:" + String(WiFi.localIP().toString()) + " on port " + String(tcpPort));
  }
}

void OTAsetup() {
  OTA = true;
  ArduinoOTA.setPassword("admin");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else  // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      Serial.println();
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
}

void manageOTA() {
  if (WIFI && !OTA) {
    OTAsetup();
  }
  if (OTA) {
    ArduinoOTA.handle();
  }
}
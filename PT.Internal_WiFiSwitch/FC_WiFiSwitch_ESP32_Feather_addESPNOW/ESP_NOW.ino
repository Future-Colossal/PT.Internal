/* 
Adapted from:
Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp-now-auto-pairing-esp32-esp8266/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. 
  Based on JC Servaye example: https://github.com/Servayejc/esp_now_sender/
*/

#define BOARD_ID 1                                                 //ID of server board MUST BE 0
#define MAX_CHANNEL 11                                             //11 for North America, 13 for Europe
uint8_t serverAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };  //broadcast address for auto-pairing
uint8_t clientMacAddress[6];

// Structure to send data
// Must match the receiver structure
// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  uint8_t msgType;
  uint8_t id;
  unsigned int readingID;
  String message;
} struct_message;

// Structure for pairing message
typedef struct struct_pairing {  // new structure for pairing
  uint8_t msgType;
  uint8_t id;
  uint8_t macAddr[6];
  uint8_t channel;
} struct_pairing;

esp_now_peer_info_t peer;  //creates a variable to store peer info

// Create 2 struct_message
struct_message myData;  // data to send
struct_message inData;  // data received
struct_pairing pairingData;

PairingStatus pairingStatus = NOT_PAIRED;
MessageType messageType;

#ifdef SAVE_CHANNEL
int lastChannel;
#endif
int channel = 1;

unsigned long currentMillis = millis();
unsigned long previousMillis = 0;  // Stores last time temperature was published
const long interval = 10000;       // Interval at which to publish sensor readings
unsigned long start;               // used to measure Pairing time
unsigned int readingId = 0;

//Get MAC address of this device
void readGetMacAddress() {
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
  clientMacAddress[0] = baseMac[0];
  clientMacAddress[1] = baseMac[1];
  clientMacAddress[2] = baseMac[2];
  clientMacAddress[3] = baseMac[3];
  clientMacAddress[4] = baseMac[4];
  clientMacAddress[5] = baseMac[5];
}

//Adds a peer device.
void addPeer(const uint8_t *mac_addr, uint8_t chan) {
  //ESP_ERROR_CHECK(esp_wifi_set_channel(chan, WIFI_SECOND_CHAN_NONE));
  esp_now_del_peer(mac_addr);
  memset(&peer, 0, sizeof(esp_now_peer_info_t));
  peer.channel = chan;
  peer.encrypt = false;
  memcpy(peer.peer_addr, mac_addr, sizeof(uint8_t[6]));
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  memcpy(serverAddress, mac_addr, sizeof(uint8_t[6]));
}

//Print MAC address to serial monitor
void printMAC(const uint8_t *mac_addr) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
}

//What to do when data is sent.
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

//What to do when data is received.
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  Serial.print("Packet received with ");
  Serial.print("data size = ");
  Serial.println(sizeof(incomingData));
  uint8_t type = incomingData[0];
  switch (type) {
    case DATA:  // we received data from server
      memcpy(&inData, incomingData, sizeof(inData));
      Serial.print("ID  = ");
      Serial.println(inData.id);
      Serial.print("Message = ");
      Serial.println(inData.message);

      if (inData.readingID % 2 == 1) {
        digitalWrite(LED_BUILTIN, LOW);
      } else {
        digitalWrite(LED_BUILTIN, HIGH);
      }
      break;

    case PAIRING:  // we received pairing data from server
      memcpy(&pairingData, incomingData, sizeof(pairingData));
      if (pairingData.id == 0) {  // the message comes from server
        Serial.print("Pairing done for MAC Address: ");
        printMAC(pairingData.macAddr);
        Serial.print(" on channel ");
        Serial.print(pairingData.channel);                  // channel used by the server
        addPeer(pairingData.macAddr, pairingData.channel);  // add the server  to the peer list
#ifdef SAVE_CHANNEL
        lastChannel = pairingData.channel;
        EEPROM.write(0, pairingData.channel);
        EEPROM.commit();
#endif
        pairingStatus = PAIR_PAIRED;  // set the pairing status
      }
      break;
  }
}

//Auto-pairing stuff, must declare enum in a separate header due to compile issues if using enum to define function return type
PairingStatus autoPairing() {
  switch (pairingStatus) {
    case PAIR_REQUEST:
      Serial.print("Pairing request on channel ");
      Serial.println(channel);

      // set WiFi channel
      //ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE));
      if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
      }

      // set callback routines
      esp_now_register_send_cb(OnDataSent);
      esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

      // set pairing data to send to the server
      pairingData.msgType = PAIRING;
      pairingData.id = BOARD_ID;
      pairingData.channel = channel;
      pairingData.macAddr[0] = clientMacAddress[0];
      pairingData.macAddr[1] = clientMacAddress[1];
      pairingData.macAddr[2] = clientMacAddress[2];
      pairingData.macAddr[3] = clientMacAddress[3];
      pairingData.macAddr[4] = clientMacAddress[4];
      pairingData.macAddr[5] = clientMacAddress[5];

      // add peer and send request
      addPeer(serverAddress, channel);
      esp_now_send(serverAddress, (uint8_t *)&pairingData, sizeof(pairingData));
      previousMillis = millis();
      pairingStatus = PAIR_REQUESTED;
      break;

    case PAIR_REQUESTED:
      // time out to allow receiving response from server
      currentMillis = millis();
      if (currentMillis - previousMillis > 1000) {
        previousMillis = currentMillis;
        // time out expired,  try next channel
        channel++;
        if (channel > MAX_CHANNEL) {
          channel = 1;
        }
        pairingStatus = PAIR_REQUEST;
      }
      break;

    case PAIR_PAIRED:
      // nothing to do here
      break;
  }
  return pairingStatus;
}

//Setup function called in util.ino
void esp_nowSetup() {
  readGetMacAddress();

#ifdef SAVE_CHANNEL
  EEPROM.begin(10);
  lastChannel = EEPROM.read(0);
  Serial.println(lastChannel);
  if (lastChannel >= 1 && lastChannel <= MAX_CHANNEL) {
    channel = lastChannel;
  }
  Serial.println(channel);
#endif

  pairingStatus = PAIR_REQUEST;
}

//This is being called in main loop() of main program file, treat as looping section. Only active when TCP drops.
void receiveESP() {
  if (autoPairing() == PAIR_PAIRED) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      // Save the last time a new reading was published
      previousMillis = currentMillis;
      //Set values to send
      myData.msgType = DATA;
      myData.id = BOARD_ID;
      myData.readingID = readingId++;
      esp_err_t result = esp_now_send(serverAddress, (uint8_t *)&myData, sizeof(myData));
    }
  }
}
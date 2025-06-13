int myID = 201; //device ID is last octet of IP address

const char* ssid = "MyOptimum 32d5a1";                // Your WiFi SSID
const char* password = "9303-sepia-71";  // Your WiFi Password

IPAddress local_IP(192, 168, 1, myID);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

int tcpPort = 8888;
WiFiServer tcpServer(tcpPort);



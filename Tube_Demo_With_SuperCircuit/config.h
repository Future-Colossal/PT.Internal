#pragma once
#include <Arduino.h>

// ========================
//  DEVICE IDENTITY
// ========================
#define DEVICE_ID          203
#define DEVICE_NAME        "LED Tubes Demo"
#define DEFAULT_TITLE      "LED Tubes"
#define DEFAULT_SYSTEM_ENABLED 1   // 1 = on at boot, 0 = off

// ========================
//  WIFI PROFILES
//  (choose WIFI_PROFILE 1 or 2)
// ========================
#define WIFI_PROFILE 2

// Profile 1: 177.50.69.X network
#define WIFI_SSID_1     "Squiggle_5G"
#define WIFI_PASS_1     "Squiggle-fi!"
#define WIFI_IP_1       177, 50, 69, DEVICE_ID
#define WIFI_GW_1       177, 50, 69, 1
#define WIFI_SUBNET_1   255, 255, 0, 0

// Profile 2: 192.168.1.X network
#define WIFI_SSID_2     "FC_Orbi"
#define WIFI_PASS_2     "FC123456789office"
#define WIFI_IP_2       192, 168, 1, DEVICE_ID
#define WIFI_GW_2       192, 168, 1, 1
#define WIFI_SUBNET_2   255, 255, 0, 0

// ========================
//  PORTS
// ========================
#define TCP_PORT   9000
#define HTTP_PORT  80

// ========================
//  FEATURE FLAGS
// ========================
#define ENABLE_OTA       1
#define ENABLE_WEBSERIAL 1
#define ENABLE_DFPLAYER  0
#define ENABLE_ESPNOW    0   

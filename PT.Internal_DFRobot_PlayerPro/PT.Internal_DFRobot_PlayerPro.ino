/*!
 *@file play.ino
 *@brief Music Playing Example Program
 *@details  Experimental phenomenon: control MP3 play music, obtain song information
 *@copyright  Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@license     The MIT license (MIT)
 *@author [fengli](li.feng@dfrobot.com)
 *@maintainer [qsjhyy](yihuan.huang@dfrobot.com)
 *@version  V1.0
 *@date  2023-10-09
 *@url https://github.com/DFRobot/DFRobot_DF1201S
*/
#include <DFRobot_DF1201S.h>
#include <Bonezegei_HCSR04.h>

const int TRIGGER_PIN = 15;
const int ECHO_PIN = 33;

Bonezegei_HCSR04 ultrasonic(TRIGGER_PIN, ECHO_PIN);
/* ---------------------------------------------------------------------------------------------------------------------
 *    board   |             MCU                | Leonardo/Mega2560/M0 |    UNO    | ESP8266 | ESP32 |  microbit  |   m0  |
 *     VCC    |            3.3V/5V             |        VCC           |    VCC    |   VCC   |  VCC  |     X      |  vcc  |
 *     GND    |              GND               |        GND           |    GND    |   GND   |  GND  |     X      |  gnd  |
 *     RX     |              TX                |     Serial1 TX1      |     3     |   5/D6  |  D2   |     X      |  tx1  |
 *     TX     |              RX                |     Serial1 RX1      |     2     |   4/D7  |  D3   |     X      |  rx1  |
 * ----------------------------------------------------------------------------------------------------------------------*/
#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
#include "SoftwareSerial.h"
SoftwareSerial DF1201SSerial(2, 3);  //RX  TX
#else
#define DF1201SSerial Serial1
#endif

unsigned long currentMillis = 0;            //some global variables available anywhere in the program
unsigned long lastDebounceTime = 0;         //some global variables available anywhere in the program
unsigned long lastDebounceTimeDown = 0;         //some global variables available anywhere in the program
const unsigned long debouncePeriod = 2000;  //the value is a number of milliseconds

bool useFileNum = 0;   //flag for using the numbering system to call songs
bool useFileName = 1;  //flag for using song names to call songs

String song1 = "/slow-cat.wav";
String song2 = "/fast-cat.wav";  //"\OIIAI\Fast_Cat.wav"
String song3 = "/rave-cat.wav";

String thisFile;

bool button1_state = 0;
bool button2_state = 0;
bool sensorState = 0;
bool hasPlayed = false;

boolean debouncing = false;


int button1_pin = 14;
int button2_pin = 32;
int last_press = 0;
int d;

DFRobot_DF1201S DF1201S;
void setup() {
  Serial.begin(115200);
  pinMode(button1_pin, INPUT);
  pinMode(button2_pin, INPUT);
#if (defined ESP32)
  DF1201SSerial.begin(115200, SERIAL_8N1, /*rx =*/16, /*tx =*/17);
#else
  DF1201SSerial.begin(115200);
#endif
  while (!DF1201S.begin(DF1201SSerial)) {
    Serial.println("Init failed, please check the wire connection!");
    delay(1000);
  }
  /*Set volume to 20*/
  DF1201S.setVol(/*VOL = */ 15);
  Serial.print("VOL:");
  /*Get volume*/
  Serial.println(DF1201S.getVol());
  /*Enter music mode*/
  DF1201S.switchFunction(DF1201S.MUSIC);
  /*Wait for the end of the prompt tone */
  delay(2000);
  /*Set playback mode to "repeat all"*/
  DF1201S.setPlayMode(DF1201S.SINGLECYCLE);
  Serial.print("PlayMode:");
  /*Get playback mode*/
  Serial.println(DF1201S.getPlayMode());

  //Set baud rate to 115200(Need to power off and restart, power-down save)
  //DF1201S.setBaudRate(115200);
  //Turn on indicator LED (Power-down save)
  //DF1201S.setLED(true);
  //Turn on the prompt tone (Power-down save)
  //DF1201S.setPrompt(true);
  //Enable amplifier chip
  //DF1201S.enableAMP();
  //Disable amplifier chip
  //DF1201S.disableAMP();'
}

void loop() {
  /*
  Serial.println("Start playing");
  DF1201S.start();
  delay(3000);
  DF1201S.pause();
  delay(3000);
  Serial.println("Next");
  //PLAY THE NEXT SONG
  DF1201S.next();
  delay(3000);
  Serial.println("Previous");
  //PLAY THE PREVIOUS SONG
  DF1201S.last();
  delay(3000);
  Serial.println("Start playing");
  //Fast forward 10S
  DF1201S.fastForward(10); //FF=
  //Fast Rewind 10S
  DF1201S.fastReverse(10); //FR =
  //Start the song from the 10th second
  DF1201S.setPlayTime(10); //PLAYTIME
  */
  //currentMillis = millis();
  //checkDistance();
  checkButtons();
}

void checkButtons() {
  button1_state = digitalRead(button1_pin);
  button2_state = digitalRead(button2_pin);
  //play track 1 if the first button is pressed
  if (button1_state && !button2_state && last_press != 1) {
    //if (useFileNum) DF1201S.playFileNum(1);
    DF1201S.playSpecFile(song1);
    Serial.println("o i i a i");
    Serial.println(DF1201S.getFileName());
    last_press = 1;
  }  //play track 2 if the second button is pressed
  else if (button2_state && !button1_state && last_press != 2) {
    //if (useFileNum) DF1201S.playFileNum(2);
    DF1201S.playSpecFile(song2);
    Serial.println("o...i...i...a...i..");
    Serial.println(DF1201S.getFileName());
    last_press = 2;
  }  //
  else if (button2_state && button1_state && last_press != 3) {
    //if (useFileNum) DF1201S.playFileNum(3);
    if (useFileName) Serial.println(DF1201S.playSpecFile(song3));
    Serial.println("O I I A I O I I A I");
    Serial.println(DF1201S.getFileName());
    last_press = 3;

  } else if (!button1_state && !button2_state && last_press != 0) {
    DF1201S.pause();
    Serial.println("NO CAT SOUNDS");
    last_press = 0;
  }
  delay(10);
}

void checkDistance() {
  d = ultrasonic.getDistance();
  //Serial.print("Distance:");
  //Serial.print(d);
  //Serial.println(" cm");
  if (d > 5 && d <175) {
    if (hasPlayed == false) {
      //Serial.println("We are here!");
      lastDebounceTime = millis();
      //Serial.println(currentMillis);
      //Serial.println(lastDebounceTime);
      if (currentMillis - lastDebounceTime >= debouncePeriod)  //test whether the period has elapsed
      {
        if (d != 1 && !DF1201S.isPlaying()) {
          DF1201S.playSpecFile(song3);
          //Serial.println("Presence detected!");
          hasPlayed = true;
          //Serial.println(hasPlayed);
        }
      }
    }
  }
  else if (hasPlayed && !DF1201S.isPlaying()) {
    DF1201S.pause();
    if (d = -1) {
      lastDebounceTimeDown = millis();
      if (currentMillis - lastDebounceTimeDown >= debouncePeriod) {  //test whether the period has elapsed
        hasPlayed = false;
        //Serial.println("Stepped away!");
        Serial.println(hasPlayed);
      }
    }
  }
  delay(250);
}
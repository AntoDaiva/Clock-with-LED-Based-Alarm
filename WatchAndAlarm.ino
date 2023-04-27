//Antonius Daiva Prabawisesa
//479269/TK/52842
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include "time.h"

#include <map>
#include <string>

//oled
#define OLED_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//wifi
const char* ssid = "Chikafluffy";
const char* password = "05082001";

//ntp
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600*6;
const int   daylightOffset_sec = 3600;

//keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};
char keyPressed;

//LED
const int LED_PIN = 18;
int ledState = LOW;
#define DEBOUNCE_DELAY 200
long lastDebounce = 0;

//Alarm
struct tm timeinfo;
tm Alarm;
bool triggered = false;
int hour = 0;
int minute = 0;
bool isAlarmOn = false;
bool isAlarmTriggered = false;

void setup(){
    Serial.begin(115200);
    delay(500);

    //led
    pinMode(LED_PIN, OUTPUT);

    SetupOLED();

    //wifi connect and display
    WiFi.mode(WIFI_STA); //Optional
    WiFi.begin(ssid, password);
    
    display.println("Connecting");
    display.display();

    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(100);
    }

    display.clearDisplay();
    display.println("\nConnected to the WiFi network");
    display.println("Local ESP32 IP: ");
    display.println(WiFi.localIP());
    Serial.println("Connected");
    display.display();

    //get time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();

    //keypad
    for (int i = 0; i < 4; i++) {
    pinMode(colPins[i], INPUT_PULLUP);
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH);
    }  

    Alarm = timeinfo;
    Alarm.tm_hour = 0;
    Alarm.tm_min = 0;

}

void loop(){
  printLocalTime();
  if(!isAlarmTriggered){
    printAlarm();
  }
  KeyScan();
  triggerAlarm();
  display.display();

}

void SetupOLED(){
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1.5);
  display.setTextColor(WHITE);
}

void alarmLedOn() {
  ledState = (ledState == LOW) ? HIGH : LOW;
  digitalWrite(LED_PIN, ledState);
  delay(200);
}

void alarmLedOff() {
  ledState = LOW;
  digitalWrite(LED_PIN, ledState); 
}

void triggerAlarm() {
  if (timeinfo.tm_hour == Alarm.tm_hour && timeinfo.tm_min == Alarm.tm_min) {
    isAlarmTriggered = true;
  }

  if (isAlarmOn && isAlarmTriggered) {
    alarmLedOn();
    display.println("\nALARMMMMMM");
  }
  else {
    alarmLedOff();
  }
  display.display();
}

void KeyScan() {
      for (int i = 0; i<4; i++) {
      digitalWrite(rowPins[i], LOW);

      for (int j = 0; j<4; j++) {
        if (digitalRead(colPins[j]) == LOW) {
          keyPressed = keys[i][j];
          if ((millis() - lastDebounce) > DEBOUNCE_DELAY) {
            lastDebounce = millis();
            keyAction(keyPressed);
          }
        }
      }
      digitalWrite(rowPins[i], HIGH);
    }
}

void keyAction(char keyPressed) {
  if (keyPressed == 'A') {
    // turn on alarm
    isAlarmOn = true;
  }
  else if (keyPressed == 'B') {
    // turn off alarm
    isAlarmOn = false;
    isAlarmTriggered = false;
  }
  else if (keyPressed == '2') {
    // add hour
    if(Alarm.tm_hour > 22){
      Alarm.tm_hour -= 24;
    }
    Alarm.tm_hour += 1;
  }
  else if (keyPressed == '1') {
    // subtract hour
    if(Alarm.tm_hour < 1){
      Alarm.tm_hour += 24;
    }
    Alarm.tm_hour -= 1;
  }
  else if (keyPressed == '5') {
    // add hour
    if(Alarm.tm_min > 59){
      Alarm.tm_min -= 60;
    }
    Alarm.tm_min += 1;
  }
  else if (keyPressed == '4') {
    // subtract hour
    if(Alarm.tm_min < 1){
      Alarm.tm_min += 60;
    }
    Alarm.tm_min -= 1;
  }
  else if (keyPressed == 'C') {
    // reset alarm
      Alarm.tm_hour = 0;
      Alarm.tm_min = 0;
  }
  else if (keyPressed == 'D') {
    // turn off alarm
    isAlarmOn = false;
    isAlarmTriggered = false;
    Alarm.tm_hour = 0;
    Alarm.tm_min = 0;
  }
}

void printAlarm(){
  if (isAlarmOn) {
    display.println("\n\nAlarm is set for ");
    display.print(&Alarm, "\n%H:%M");
  }
  else {
    display.println("\nAlarm is off");
    display.println("12 for hour, 45 for min");
  }
}

void printLocalTime(){
  display.clearDisplay();
  display.setCursor(0, 0);
  if(!getLocalTime(&timeinfo)){
    display.println("Failed to obtain time");
    return;
  }

  display.println(&timeinfo, "%A,");
  display.print(&timeinfo, "\n%d ");
  display.print(timeinfo.tm_mon + 1);
  display.print(&timeinfo, " %Y");
  display.print(&timeinfo, "\n%H:%M:%S");
}


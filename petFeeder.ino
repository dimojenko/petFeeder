/* petFeeder
 *  
 */

#include <NTPClient.h>
#include <WiFiNINA.h> // WiFi library specific to board
#include <WiFiUdp.h>
#include "SingleSweepServo.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
const char *ssid     = "WiFiName";
const char *password = "WiFiPassword";
int status = WL_IDLE_STATUS; // set WiFi connection status

WiFiUDP ntpUDP;
int servoPin = 14;
SingleSweepServo servo1(14, 42, 107); // servo1 instantiation (15, 47, 105)
enum {WAITING, FEEDTIME, FED} currentState = WAITING;  // main loop states

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "pool.ntp.org", -14400, 60000); // -14400 = -4 hrs (EST DST), -18000 = -5 hrs (EST)

// simple time structure
struct timeStrct {
  int hr;
  int mn;
} curTime, feedTime[2];

// backup-time vars
int seconds = 0;
int minutes = 0;
int hours = 0;

// millis update setup
int updateIntervalNTP = 10000; // 10s
unsigned long lastUpdateNTP = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup(){
  // set times to feed
  feedTime[0].hr = 7;
  feedTime[0].mn = 0;
  feedTime[1].hr = 17;
  feedTime[1].mn = 0;
  
  Serial.begin(115200);
  
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, password);

    // wait for connection (ms):
    delay(1000);
  }

  // print WiFi status:
  Serial.println("Connected to wifi");
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.println("\nStarting connection to server...");
  timeClient.begin();
} //setup

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop() {
  if ((millis() - lastUpdateNTP) > (updateIntervalNTP)) {
    lastUpdateNTP = millis();
    // get time from server
    if (currentState != FEEDTIME) { // WiFi and time client reconnection interferes with servo
      if (timeClient.update()) { 
        //Serial.println(timeClient.getDay()); // Sunday == 0
        Serial.println(timeClient.getFormattedTime());
        curTime.hr = timeClient.getHours();
        curTime.mn = timeClient.getMinutes();
        seconds = timeClient.getSeconds();
      }
      // if disconnected:
      else {
        Serial.println("No WiFi connection.");
        // try to re-establish WiFi connection
        WiFi.begin(ssid, password);
        timeClient.begin(); // restart time client
        // keep track of time locally
        seconds += (updateIntervalNTP / 1000);
        if (seconds >= 60) {
          seconds -= 60;
          curTime.mn += 1;
        }
        if (curTime.mn == 60) {
          curTime.mn = 0;
          curTime.hr += 1;
        }
        if (curTime.hr == 24) {curTime.hr = 0;}
        Serial.print(curTime.hr);
        Serial.print(":");
        Serial.print(curTime.mn);
        Serial.print(":");
        Serial.println(seconds);
      }
    }
  }

  //check if time to feed
  bool trigger = false;
  for (int i=0; i<2; i++) {
    if ((curTime.hr == feedTime[i].hr)&&(curTime.mn == feedTime[i].mn)) {
      trigger = true;
    }
  }
//  // debugging: feed every other minute
//  if (curTime.mn % 2 == 0) {
//    trigger = true;
//  }

  switch (currentState) {
    case WAITING:
      if (trigger) {
        Serial.print("Time to feed!\n");
        servo1.Attach(servoPin); // attach servo to pin
        servo1.Initialize();     // move servo to initial position
        currentState = FEEDTIME;
      }
      break;
    case FEEDTIME:
      servo1.Update(); // activate servo
      if (servo1.isSweepComplete()) {
        currentState = FED;
        Serial.println("feeding complete\n");
      }
      break;
    case FED:
      servo1.Detach();  // detach servo to conserve power
      if (!trigger) {
        currentState = WAITING;
      }
      break;
  } //switch
} //loop

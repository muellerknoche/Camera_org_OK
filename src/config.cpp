#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>


// Default values if SD read fails
String defaultSsid = "ELECROW";
String defaultPassword = "dummyPASS";               // Must be at least 8 chars for WPA2
IPAddress defaultIP(192, 168, 4, 1);
IPAddress defaultGateway(192, 168, 4, 1);
IPAddress defaultSubnet(255, 255, 255, 0);
String defaultPin = "1111";
String defaultLaenge = "4";

String ssid;
String password;
IPAddress localIP;
IPAddress gateway;				// gateway has the same value than localIP
IPAddress subnet;
String pin;
String laenge;

bool streaming = false;
// SD card CS pin (adjust based on your wiring)
//const int csPin = 10;

#endif // CONFIG_H
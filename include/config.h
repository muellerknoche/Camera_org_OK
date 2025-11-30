#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>


// Default values if SD read fails
extern String defaultSsid;
extern String defaultPassword;               // Must be at least 8 chars for WPA2
extern IPAddress defaultIP;
extern IPAddress defaultGateway;
extern IPAddress defaultSubnet;
extern String defaultPin;
extern String defaultLaenge;

extern String ssid;
extern String password;
extern IPAddress localIP;
extern IPAddress gateway;				    // gateway has the same value than localIP
extern IPAddress subnet;
extern String pin;
extern String laenge;

extern bool streaming;
// SD card CS pin (adjust based on your wiring)
//const int csPin = 10;

#endif // CONFIG_H
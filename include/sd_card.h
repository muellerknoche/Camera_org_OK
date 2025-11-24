/**
 * Project Spion for ME with ESP32 CAM and 5" CROWPANEL
 * 
 * @author Rainer Müller-Knoche mk@muekno.de
 * @date 21.11.2025 last change 
 */


#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>
#include <WiFi.h>
#include "FS.h"
#include "SD_MMC.h"
#include <config.h>

// Function to trim whitespace from strings
String trim(String str)
{
  	str.trim();
  	return str;
}

// Start WiFi with values from SD ard or fallback from default
void startwifi()
{
    if (!SD_MMC.begin("/SDCARD", true))
	{
    	Serial.println("SD Card Mount Failed");
    	return;
  	}

	uint8_t cardType = SD_MMC.cardType();
  	if (cardType == CARD_NONE)
	{

		#ifdef DEBUG
			Serial.println("No SD Card attached");
		#endif

 	   	return;
  	} else {

		#ifdef DEBUG
    		Serial.print("Ccard Tye: ");
			Serial.println( cardType);					// card type  in klartext
		#endif

	}
	// Open the file for reading
  	File file = SD_MMC.open("/config.txt", FILE_READ);
  	if (!file)
  	{

		#ifdef DEBUG
			Serial.println("Failed to open file for reading");
		#endif

		return;
	}

	#ifdef DEBUG
		Serial.println("SD card mounted");
	#endif
	
	// get information from SD card
	while (file.available())
  	{
      	String line = file.readStringUntil('\n');
    	line = trim(line);
    	if (line.length() == 0 || line.startsWith("#")) continue;  // Skip empty or comments

    	int eqIndex = line.indexOf('=');
    	if (eqIndex == -1) continue;  // Invalid line

    	String key = trim(line.substring(0, eqIndex));
    	String value = trim(line.substring(eqIndex + 1));

		if (key == "ssid") ssid = value;
		else if (key == "password") password = value;
		else if (key == "ip")	localIP.fromString(value);
		else if (key == "gateway") gateway.fromString(value);
		else if (key == "subnet") subnet.fromString(value);
		else if (key == "pin") pin = value;
		else if (key == "laenge") laenge = value;
  	}
	file.close();

	#ifdef DEBUG
		Serial.println("SD Card readings");
		Serial.print("SSID: ");Serial.println(ssid);
		Serial.print("Paswort: ");Serial.println(password);
		Serial.print("IP: ");Serial.println(localIP);
		Serial.print("gateway: ");Serial.println(gateway);		// wsHost
		Serial.print("subnet: ");Serial.println(subnet);
		Serial.print("PIN: ");Serial.println(pin);
		Serial.print("Pin len: ");Serial.println(laenge);
	#endif

  	// Start AP mode
  	WiFi.mode(WIFI_AP);
  	WiFi.softAP(ssid.c_str(), password.c_str());
  	Serial.println("AP started with SSID: " + ssid);
  	delay(100);  // Brief delay for AP init

  	// Configure IP settings
  	if (!WiFi.softAPConfig(localIP, gateway, subnet))
  	{
    	Serial.println("AP config failed");
  	} else {
    	Serial.println("AP config successful");
  	}

  	// Print AP IP
  	Serial.print("AP IP address: ");
  	Serial.println(WiFi.softAPIP());

} // END startwifi

#endif // SDCARD_H
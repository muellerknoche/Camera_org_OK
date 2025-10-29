#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>
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
		else if (key == "gateway") gateway = value;			// used for wsHost
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

	// Validate if all were loaded
  	if (ssid.isEmpty() || password.isEmpty() || localIP == IPAddress(0,0,0,0) ||
      gateway.isEmpty() || subnet == IPAddress(0,0,0,0) || password.isEmpty())
  	{
		#ifdef DEBUG
			Serial.println("Incomplete config, using defaults");
			Serial.println("set defaults");
			Serial.println(defaultSsid);
			Serial.println(defaultPassword);
//			Serial.println(defaultWebsockets_server_host);
		#endif

		WiFi.begin(defaultSsid,defaultPassword);

  	} else {								// try with values from SD
	
		#ifdef DEBUG
			Serial.println("Set Wifi to STA mode");
		#endif

		WiFi.mode(WIFI_STA);
		WiFi.disconnect(true);
		
		#ifdef DEBUG
			Serial.println(ssid);
			Serial.println(password);
		#endif

  		WiFi.begin(ssid,password);				
		// Wait mx 15 secondsome time to connect to wifi
  		for(int i = 0; i < 15 && WiFi.status() != WL_CONNECTED; i++)
		{
      		Serial.print(".");
      		delay(1000);
  		}

		#ifdef DEBUG
			if (WiFi.status() == WL_CONNECTED)
			{
				Serial.println("WIFI CONNECTED");
			}
		#endif

	}







#endif // SDCARD_H
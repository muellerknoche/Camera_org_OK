/**
 * @author Rainer Müller-Knoche mk@muekno.de
 * @date 09.11.2025 mk Version 1.0.0
 * @date 21.11.2025 mk Version 1.1.0
 * CAM als Websockets socket_server
 * @date 26.11.2025 merged to main
 * @date 26.11.2025 mk new branch fertig_machen
 * @date 27.11.2025 just test
 * @date 28.11.2025 mk weiter
  */

#include <Arduino.h>
#include "FS.h"
#include "SD_MMC.h"
#include <sd_card.h>
#include <config.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>

#define DEBUG

//Defines the camera GPIO (“AI Thinker” camera model).
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
//----------------------------------------
// ESP32 TFT LCD (socket_server) Access Point.
//const uint16_t websockets_socket_server_port = 8888;
using namespace websockets;
WebsocketsServer socket_server;
camera_fb_t * fb = NULL;
/**
 * @fn flash()
 * @author Rainer Müller-Knoche mk@muekno.de
 * @brief make the EP CAM flash light blink
 * @param uint16_t flashTime Time flashlight on in ms
 * @param uint16_t foutTime Time flashlight off in ms
 * @param uint8_t nTimes 
 * @date 28.11.2025 
 */
void flash(uint16_t flashTime, uint16_t outTime, uint8_t nTimes)
{
	for (uint8_t i = 0; i < nTimes;i++)
	{
		digitalWrite(4,HIGH);
		delay(flashTime);
		digitalWrite(4,LOW);
	}
}
/**
 * @fd setup()
 * @author Rainer Müller-knoche mk@muekno.de
 * @brief Setup functions
 * @date 26.11.2025
 * @brief init all set up  websocket server listening a port 8888
 * @date 28.11.2025 mk
 */
void setup()
{
  	pinMode(4, OUTPUT);								// init CAM  Flashlight GPIO 
	digitalWrite(4, LOW);							// flashlight off
	Serial.begin(115200);							// start Serial
//	while (!Serial) { delay(10); }					// let Serial come up loop blocks
	delay(200);										// wait a little bit
	startwifi();                      				// in incorrektem sd_card.h
	socket_server.listen(8888);						// set Websockets port
	Serial.print("Is server live? ");	Serial.println(socket_server.available());	// just notice
	//-Set up the ESP32-CAM camera configuration.
	Serial.println("Set the camera ESP32-CAM...");	// just notice
	camera_config_t config;
  	config.ledc_channel = LEDC_CHANNEL_0;
  	config.ledc_timer = LEDC_TIMER_0;
  	config.pin_d0 = Y2_GPIO_NUM;
  	config.pin_d1 = Y3_GPIO_NUM;
  	config.pin_d2 = Y4_GPIO_NUM;
  	config.pin_d3 = Y5_GPIO_NUM;
  	config.pin_d4 = Y6_GPIO_NUM;
  	config.pin_d5 = Y7_GPIO_NUM;
  	config.pin_d6 = Y8_GPIO_NUM;
  	config.pin_d7 = Y9_GPIO_NUM;
  	config.pin_xclk = XCLK_GPIO_NUM;
  	config.pin_pclk = PCLK_GPIO_NUM;
  	config.pin_vsync = VSYNC_GPIO_NUM;
  	config.pin_href = HREF_GPIO_NUM;
  	config.pin_sccb_sda = SIOD_GPIO_NUM;
  	config.pin_sccb_scl = SIOC_GPIO_NUM;
  	config.pin_pwdn = PWDN_GPIO_NUM;
  	config.pin_reset = RESET_GPIO_NUM;
  	config.xclk_freq_hz = 20000000;
  	config.pixel_format = PIXFORMAT_JPEG;
  	// init with high specs to pre-allocate larger buffers.
  	if(psramFound())
	{
    	config.frame_size = FRAMESIZE_VGA; //--> 320x240.
    	config.jpeg_quality = 10;
    	config.fb_count = 2;
	}
	else
	{
    	config.frame_size = FRAMESIZE_SVGA;
    	config.jpeg_quality = 12;
    	config.fb_count = 1;
	}
  	Serial.println("ESP32-CAM camera initialization...");
  	esp_err_t err = esp_camera_init(&config);
  	if (err != ESP_OK)										// shout never occur
	{
   	 	Serial.printf("Camera init failed with error 0x%x", err);
    	Serial.println("Restarting the ESP32 CAM.");
		flash(500, 500,2);									// switch Flashlight on for halph a second twice
															// to make CAN init failure visible
    	ESP.restart();										// restart ESP now
  	}
  	Serial.println("ESP32-CAM camera initialization successful.");
	flash(50,5,1);											// switch on flashlight a short time
															// to indicate OK
	Serial.println("Setup done.");							// just notice
} // END SETUP
/**
 * @fb callback
 * @author GROK
 * @brief handles message received from client
 * @date 28.11.2025 mk
 */
void handle_message(WebsocketsClient &client, WebsocketsMessage msg)
{
	if (msg.data() == "START")
	{
		streaming = true;
		client.send(("STREAMING"));
	}
	else if (msg.data() == "STOP")
	{
			streaming = false;
			client.send("STOPPED");
	}
}
/**
 * @fb loop
 * @author Rainer Müller-Knoche mk@muekno.de, GROK
 * @brief loop function does the work
 * @date 26.11.2025
 * @date 28.11.2025 mk
 * @date 29.11.2025 mk weiter
 */
 void loop()
//Accept Websock client
 {
	auto client = socket_server.accept();				// from lib example
	// Start GROK --------------------------------------------------------------
	if (client.available())
	{
		client.onMessage(handle_message);
		client.send("CONNECTED");

		while (client.available())
		{
			client.poll();
			if (streaming)
			// Capture JPEG from camera
			fb = esp_camera_fb_get();
			if (!fb)
			{
				Serial.println("CAM capture failed");
				continue;
			}
			// send JPEC as binary data over Websocket
			client.sendBinary((const char *)fb->buf, fb->len);
			esp_camera_fb_return(fb);
			fb = NULL;
			// delay to control frame rate
			delay(50);	// 100 = 10 FPS
		}
	}	
}

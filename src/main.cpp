// nochmal mit ArduinoWebsockets
// SD Card einbauen
/**
 * @author Rainer Müller-Knoche mk@muekno.de
 * @date 09.11.2025 mk Version 1.0.0
 * @date 21.11.2025 mk Version 1.1.0
 * CAM als Websockets Server
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
//----------------------------------------

//----------------------------------------Defines the camera GPIO (“AI Thinker” camera model).
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

// ESP32 TFT LCD (Server) Access Point.
//const uint16_t websockets_server_port = 8888;



using namespace websockets;

WebsocketsServer server;
void setup()
{
  	pinMode(4, OUTPUT);								// CAM flashlight off
	digitalWrite(4, LOW);

	Serial.begin(115200);

//	while (!Serial) { delay(10); }					// let Serial come up loop blocks
	
	delay(100);
  	// holt Daten von SD Card und startet WiFi
	startwifi();                      				// in incorrektem sd_card.h

	server.listen(8888);
	Serial.print("Is server live? ");
  	Serial.println(server.available());



  //----------------------------------------Set up the ESP32-CAM camera configuration.
Serial.println("Set the camera ESP32-CAM...");
  
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
  	if (err != ESP_OK)
	{
   	 	Serial.printf("Camera init failed with error 0x%x", err);
    	Serial.println();
    	Serial.println("Restarting the ESP32 CAM.");
    	delay(1000);
    	ESP.restart();			
  	}
  	Serial.println("ESP32-CAM camera initialization successful.");

  
//  	Serial.print("Successfully connected to : ");  Serial.println(ssid);
//  	Serial.print("IP Address : ");  Serial.println(WiFi.localIP());


	digitalWrite(4,HIGH);
	delay(50);
	digitalWrite(4,LOW);
	Serial.println("Setup done.");
	

}

void loop()
{

	auto client = server.accept();
	if (client.available())
	{
		auto msg = client.readBlocking();

		   // log
    Serial.print("Got Message: ");
    Serial.println(msg.data());
	digitalWrite(4, HIGH);
	delay(50);	
	digitalWrite(4, LOW);

    // return echo
    client.send("Echo: " + msg.data());

}

//     	// === amera captures image.
//     	camera_fb_t *fb = NULL;
//     	esp_err_t res = ESP_OK;
//     	fb = esp_camera_fb_get();
//     	if(!fb)
// 		{
//       		while (1==1)				// verstehe  ich nicht ist do im original
//       		{
//         		Serial.println("Camera capture failed");
//         		esp_camera_fb_return(fb);
//       		}
//       		return;
//     	}
//     	size_t fb_len = 0;
//     	if(fb->format != PIXFORMAT_JPEG)
// 		{
//       		Serial.println("Non-JPEG data not implemented");
//       		return;
//     	}
//     	// Send image data to ESP32 server (ESP32 TFT LCD).
//     	client.sendBinary((const char*) fb->buf, fb->len);
    
//     	esp_camera_fb_return(fb);
  
// 	}
}
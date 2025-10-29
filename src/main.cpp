// nochmal mit ArduinoWebsockets
// SD Card einbauen

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
const uint16_t websockets_server_port = 8888;

using namespace websockets;
WebsocketsClient client;

//________________________________________________________________________________ VOID SETUP()
void setup()
{
  	pinMode(4, OUTPUT);								// CAM flashlight off
	digitalWrite(4, LOW);
	Serial.begin(115200);
	while (!Serial) { delay(10); }					// let Serial come up

	Serial.println("START SETUP");
	startwifi();

  //----------------------------------------Set up the ESP32-CAM camera configuration.
  Serial.println();
  Serial.println("-------------");
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // init with high specs to pre-allocate larger buffers.
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA; //--> 320x240.
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  Serial.println();
  Serial.println("Set camera ESP32-CAM successfully.");
  Serial.println("-------------");
  //----------------------------------------

  //----------------------------------------Camera init.
  Serial.println();
  Serial.println("-------------");
  Serial.println("ESP32-CAM camera initialization...");
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    Serial.println();
    Serial.println("Restarting the ESP32 CAM.");
    delay(1000);
    ESP.restart();
  }
  Serial.println();
  Serial.println("ESP32-CAM camera initialization successful.");
  Serial.println("-------------");
  //----------------------------------------

  //----------------------------------------Set Wifi to STA mode.
  Serial.println();
  Serial.println("-------------Set Wifi to STA mode");
  Serial.println("WIFI mode : STA");
  WiFi.mode(WIFI_STA);
  Serial.println("-------------");
  delay(500);
  //---------------------------------------- 

  //----------------------------------------Connect to Wi-Fi (STA).
  Serial.println();
  Serial.println("-------------Connect to WiFi");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.disconnect(true);
  
  WiFi.begin(ssid, password);
  
  //:::::::::::::::::: The process of connecting ESP32-CAM with WiFi Hotspot / WiFi Router / ESP32 TFT LCD WiFI Access Point (Server).
  // The process timeout of connecting ESP32-CAM with WiFi Hotspot / WiFi Router is 20 seconds.
  // If within 20 seconds the ESP32-CAM has not been successfully connected to WiFi, the ESP32-CAM will restart.
  // I made this condition because on my ESP32-CAM, there are times when it seems like it can't connect to WiFi, so it needs to be restarted to be able to connect to WiFi.
  
  int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    
    if(connecting_process_timed_out > 0) connecting_process_timed_out--;
    if(connecting_process_timed_out == 0) {
      Serial.println();
      Serial.println("Failed to connect to WiFi. The ESP32-CAM will be restarted.");
      Serial.println("-------------");
      delay(1000);
      ESP.restart();
    }
  }
  
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);

  Serial.println();
  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());
  Serial.println("-------------");
  //:::::::::::::::::: 
  //---------------------------------------- 

  //---------------------------------------- 
  Serial.println();
  Serial.println("-------------");

  Serial.println("Connecting sockets");
  while(!client.connect(gateway, websockets_server_port, "/")){
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Socket Connected !"); 
  Serial.println("-------------");
  //---------------------------------------- 
}
//________________________________________________________________________________



//________________________________________________________________________________ VOID LOOP()
void loop() {
  // put your main code here, to run repeatedly:

  if (client.available()) {
    //----------------------------------------Camera captures image.
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    fb = esp_camera_fb_get();
    if(!fb){
      while (1==1)
      {
        Serial.println("Camera capture failed");
        esp_camera_fb_return(fb);
      }
      return;
    }
    //---------------------------------------- 

    //----------------------------------------Check image format.
    size_t fb_len = 0;
    if(fb->format != PIXFORMAT_JPEG){
      Serial.println("Non-JPEG data not implemented");
      return;
    }
    //---------------------------------------- 

    // Send image data to ESP32 server (ESP32 TFT LCD).
    client.sendBinary((const char*) fb->buf, fb->len);
    
    esp_camera_fb_return(fb);
  }
}
//________________________________________________________________________________
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
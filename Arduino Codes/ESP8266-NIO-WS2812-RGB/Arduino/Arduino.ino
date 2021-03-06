/*****************************************************************************************
   Name          : Set WS2812 RGB LED Strip color using Niomatic App
   Author        : niomatic open source team
   Notice        : © Copyright 2018 Nio group –
                 : Mobile App Design and Development Company. All Rights Reserved
   Email         : support@niomatic.com
   Web           : https://niomatic.com
   Github        :
   Date          : 15/1/18 10:00 AM
   Version       : 1.6
   Description   : Set WS2812 RGB LED Strip color with Json over TCP/UDP


  /*****************************************************************************************
                         List of libraries or plugins required                           *
******************************************************************************************
  ESP8266 Arduino platform stable release   https://github.com/esp8266/Arduino
   WiFi Connection manager with web config  https://github.com/tzapu/WiFiManager
   JSON library for Arduino and IoT         https://github.com/bblanchon/ArduinoJson
  Adafruit NeoPixel Library                 https://github.com/adafruit/Adafruit_NeoPixel


  /*****************************************************************************************
                               List of hardware required                                 *
******************************************************************************************
   Any ESP8266 development kit
   WS2812 RGB LED

*/

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include "ArduinoOTA.h"
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN  D1
#define NUMPIXELS      18 // How many NeoPixels are attached to the ESP8266

char UDP_packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet
const int TCP_port = 48000;
const int UDP_port = 48000;
#define MAX_TCP_CLIENTS 1

WiFiUDP Udp;
WiFiServer server(TCP_port);
WiFiClient serverClients[MAX_TCP_CLIENTS];
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
int delayval = 500; // delay for half a second

//Global variables
String j_ask_s;
const char* j_ask;
int j_red, j_green, j_blue, j_pixel_num;




/**
   Configuring OTA (Over-the-air programming)
*/
void OTA_Config() {
  WiFiManager wifiManager;
  wifiManager.autoConnect("Niomatic-Config");
  ArduinoOTA.setHostname("Niomatic Sensor Kit"); //Hostname
  //Customize OTA & Wifi manger

  /*
    wifiManager.resetSettings(); //Uncomment this & whenever want to reset saved WIFI settings
    wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));  //set custom ip for portal
    wifiManager.autoConnect("Niomatic-Config", "password");
    wifiManager.autoConnect();
    ArduinoOTA.setPort(8266);// Port defaults to 8266

  */
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem"; // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}


/**
   Json parser
   data_in: String input from UDP or TCP
*/
void Json_parse(String data_in) {
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(data_in);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  j_ask = root["ask"];
  j_ask_s = j_ask;
  j_pixel_num = root["pixel_number"];
  j_red = root["red"];
  j_green = root["green"];
  j_blue = root["blue"];
  if (j_ask_s == "Set_rgb")
  {
    pixels.setPixelColor(j_pixel_num, pixels.Color(j_red, j_green, j_blue)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(1); // Delay for a period of time (in milliseconds).
  }
}


/**
   Setup
*/

void setup() {
  Serial.begin(115200);
  OTA_Config();

  Udp.begin(UDP_port); // start UDP server
  server.begin();
  server.setNoDelay(true);

  pixels.begin(); // This initializes the NeoPixel library.
}


/**
   Main loop
*/
void loop() {
  ArduinoOTA.handle(); //Prepare ESP8266 for next programming by OTA - Note : delays will stop working this

  /************************************************************************************************
                                   UDP Server handler
  ************************************************************************************************/
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    IPAddress remote = Udp.remoteIP();
    Udp.read(UDP_packetBuffer, packetSize);
    Json_parse(UDP_packetBuffer);
    memset(UDP_packetBuffer, 0, sizeof(UDP_packetBuffer));

  }
  /************************************************************************************************
                                   TCP Server handler
  ************************************************************************************************/
  uint8_t i;
  if (server.hasClient()) {
    for (i = 0; i < MAX_TCP_CLIENTS; i++) {
      if (!serverClients[i] || !serverClients[i].connected()) {
        if (serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();//Serial1.print("New client: "); Serial1.print(i);
        continue;
      }
    }
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  //check clients for data
  for (i = 0; i < MAX_TCP_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      if (serverClients[i].available()) { //get data from the telnet client and push it to the UART
        int read_size = 0;
        char tcp_read_buffer[250];
        while (serverClients[i].available()) {
          tcp_read_buffer[read_size] = serverClients[i].read();
          read_size++;
        }
        Json_parse(tcp_read_buffer);


        read_size = 0;
      }
    }
  }
}

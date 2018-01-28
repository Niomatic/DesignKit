/*****************************************************************************************
   Name          : Running OLED display using Niomatic App
   Author        : niomatic open source team
   Notice        : © Copyright 2018 Nio group –
                 : Mobile App Design and Development Company. All Rights Reserved
   Email         : support@niomatic.com
   Web           : https://niomatic.com
   Github        :
   Date          : 15/1/18 10:00 AM
   Version       : 1.6
   Description   : Running OLED display with Json over TCP/UDP


  /*****************************************************************************************
                         List of libraries or plugins required                           *
******************************************************************************************
  ESP8266 Arduino platform stable release  https://github.com/esp8266/Arduino
  WiFi Connection manager with web config  https://github.com/tzapu/WiFiManager
  JSON library for Arduino and IoT         https://github.com/bblanchon/ArduinoJson
  SSD1306 OLED display library             https://github.com/squix78/esp8266-oled-ssd1306


  /*****************************************************************************************
                               List of hardware required                                 *
******************************************************************************************
   Any ESP8266 development kit
   I2C SSD1306 OLED display
*/

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h"
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include "ArduinoOTA.h"
#include <ArduinoJson.h>

/*
Initialize the OLED display
*/
#define SDA D3
#define SCL D5

SSD1306  display(0x3c, SDA, SCL);
char UDP_packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet
const int TCP_port = 48000;
const int UDP_port = 48000;
#define MAX_TCP_CLIENTS 1

WiFiUDP Udp;
WiFiServer server(TCP_port);
WiFiClient serverClients[MAX_TCP_CLIENTS];

//Global variables
String j_ask_s;
const char* j_ask;
String j_mode_s;
const char* j_mode;
int j_x;
int j_y;
int j_w;
int j_h;
int j_r;
int j_l;
bool j_visible;
bool j_fill;
String j_orient_s;
const char* j_orient;
String j_text_s;
const char* j_text;
String j_size_s;
const char* j_size;


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
  j_mode = root["mode"];
  j_mode_s = j_mode;
  j_x = root["x"];
  j_y = root["y"];
  j_w = root["width"];
  j_h = root["height"];
  j_l = root["lenght"];
  j_r = root["radius"];
  j_orient = root["orientation"];
  j_orient_s = j_orient;
  j_fill = root["fill"];

  j_text = root["text"];
  j_text_s = j_text;

  j_size = root["font_size"];
  j_size_s = j_size;

  if (j_ask_s == "Set_OLED")
  {
    if (j_mode_s == "draw_circle")
    {
      Serial.println("Mode : draw Circle");
      drawcircle(j_x, j_y, j_r, j_fill);
    }
    if (j_mode_s == "draw_rect")
    {
      Serial.printf("Mode : draw rect - x:%d  ,y:%d \n", j_x, j_y);
      draw_rect(j_x, j_y, j_w, j_h, j_fill);
    }
    if (j_mode_s == "draw_line")
    {
      Serial.println("Mode : draw line");
      draw_line(j_x, j_y, j_l, j_orient_s);
    }
    if (j_mode_s == "draw_text")
    {
      Serial.println("Mode : draw text");
      drawtext(j_text_s, j_x, j_y, j_size_s);
    }
    if (j_mode_s == "draw_pixel")
    {
      Serial.printf("Mode : draw pixel   x:%d  ,y:%d \n", j_x, j_y);
      display.setPixel(j_x, j_y);
      display.display();
    }
    if (j_mode_s == "clear")
    {
      Serial.println("Mode : Clear display");
      display.clear();
      display.display();
    }
  }
}

void drawcircle(int x, int y, int r, bool fill) {
  //if (visible == true)display.setColor(WHITE);
  // else display.setColor(BLACK);
  display.setColor(WHITE);
  if (fill == true)
  {
    display.fillCircle(x, y, r);
    display.display();
  }
  if (fill == false)
  {
    display.drawCircle(x, y, r);
    display.display();
  }
}

void draw_rect(int x, int y, int w, int h, bool fill) {
  if (fill == false)display.drawRect(x, y, w, h);
  else display.fillRect(x, y, w, h);
  display.display();
}

void draw_line(int x, int y, int l, String s) {
  if (s == "horizontal")display.drawHorizontalLine(x, y, l); // Draw a line horizontally
  if (s == "vertical")display.drawVerticalLine(x, y, l); // Draw a line horizontally
  display.display();
}

void drawtext(String text, int x, int y, String font_size) {
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  if (j_size_s == "small")display.setFont(ArialMT_Plain_10);
  if (j_size_s == "medium")display.setFont(ArialMT_Plain_16);
  if (j_size_s == "large")display.setFont(ArialMT_Plain_24);
  display.drawString(x, y, text);
  display.display();
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
  display.init();
  display.flipScreenVertically();
  display.clear();
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

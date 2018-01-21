/*****************************************************************************************
   Name          : Control Dc motor with L298 module using Niomatic App
   Author        : niomatic open source team
   Notice        : © Copyright 2018 Nio group –
                 : Mobile App Design and Development Company. All Rights Reserved
   Email         : support@niomatic.com
   Web           : https://niomatic.com
   Github        :
   Date          : 15/1/18 10:00 AM
   Version       : 1.6
   Description   : SControl Dc motor with L298 module with Json over TCP/UDP


  /*****************************************************************************************
                         List of libraries or plugins required                           *
******************************************************************************************
  ESP8266 Arduino platform stable release   https://github.com/esp8266/Arduino
   WiFi Connection manager with web config  https://github.com/tzapu/WiFiManager
   JSON library for Arduino and IoT         https://github.com/bblanchon/ArduinoJson


  /*****************************************************************************************
                               List of hardware required                                 *
******************************************************************************************
   Any ESP8266 development kit
   L298 motor driver module

*/


#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include "ArduinoOTA.h"
#include <ArduinoJson.h>


/**** Motor 1 control pins****/
int IN1 = D5;
int IN2 = D6;
/**** Motor 2 control pins****/
int IN3 = D7;
int IN4 = D8;


char UDP_packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet
const int TCP_port = 48000;
const int UDP_port = 48000;
#define MAX_TCP_CLIENTS 1

WiFiUDP Udp;
WiFiServer server(TCP_port);
WiFiClient serverClients[MAX_TCP_CLIENTS];


//Global variables
String j_ask_s;
String j_dir_s;
const char* j_ask;
int j_index;
const char* j_dir;
int j_speed;


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
  j_index = root["Index"];
  j_dir = root["Direction"];
  j_dir_s = j_dir;
  j_speed = root["Speed"];

  if (j_ask_s == "Set_Motor")
  {
    Serial.printf("Set motor num %d to direction %s with speed %d \n", j_index, j_dir, j_speed);

    if (j_index == 1)
    {
      analogWrite(D1, j_speed);
      if (j_dir_s == "Right")
      {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
      }
      if (j_dir_s == "Left")
      {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
      }
      if (j_dir_s == "Stop")
      {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
      }
    }
    if (j_index == 2)
    {
      analogWrite(D2, j_speed);
      if (j_dir_s == "Right")
      {
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
      }
      if (j_dir_s == "Left")
      {
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
      }
      if (j_dir_s == "Stop")
      {
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
      }
    }
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
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
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

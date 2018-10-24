/*****************************************************************************************
   Name          : Sending joysick status using Niomatic App
   Author        : Niomatic open source team
   Notice        : © Copyright 2018 Nio group –
                 : Mobile App Design and Development Company. All Rights Reserved
   Email         : support@niomatic.com
   Web           : https://niomatic.com
   Github        :
   Date          : 15/1/18 10:00 AM
   Version       : 1.6
   Description   : sending joysick status with Json over TCP/UDP


  /*****************************************************************************************
                         List of libraries or plugins required                           *
******************************************************************************************
  ESP8266 Arduino platform stable release  https://github.com/esp8266/Arduino
   WiFi Connection manager with web config  https://github.com/tzapu/WiFiManager
   JSON library for Arduino and IoT         https://github.com/bblanchon/ArduinoJson



  /*****************************************************************************************
                               List of hardware required                                 *
******************************************************************************************
   Any ESP8266 development kit
   Joystick modules
*/



#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include "ArduinoOTA.h"
#include <ArduinoJson.h>


char UDP_packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet
const int TCP_port = 48000;
const int UDP_port = 48000;
#define MAX_TCP_CLIENTS 1

WiFiUDP Udp;
WiFiServer server(TCP_port);
WiFiClient serverClients[MAX_TCP_CLIENTS];


//Global variables
bool sent = false;
IPAddress remote;
int remote_portt;
String JsonPrint;

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
   Setup
*/
void setup() {
  Serial.begin(115200);
  OTA_Config();

  Udp.begin(UDP_port); // start UDP server
  server.begin();
  server.setNoDelay(true);
}


/**
   Main loop
*/
void loop() {
  
  ArduinoOTA.handle(); //Prepare ESP8266 for next programming by OTA - Note : delays will stop working this  
  
  /********************************* TCP Server handler ********************************/
  if (server.hasClient()) {
    if (!serverClients[0] || !serverClients[0].connected()) {
      if (serverClients[0]) serverClients[0].stop();
      serverClients[0] = server.available();
      Serial.print("New TCP client");
    }
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  /******************************** UDP Server handler ********************************/
  remote = Udp.remoteIP();
  remote_portt = Udp.remotePort();


  /************************************************************************************************
                                    Joystick read handler
   ************************************************************************************************/
  pinMode(D1,INPUT);
  pinMode(D0,OUTPUT);
  digitalWrite(D0,LOW);
  int x = analogRead(A0);
  Serial.print("x: ");
  Serial.print(x );
  Serial.print("      y: ");
  pinMode(D0,INPUT);
  pinMode(D1,OUTPUT);
  digitalWrite(D1,LOW);
  int y = analogRead(A0);
  Serial.println(y);
  delay(100);        // delay in between reads for stability
  
  
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["identify"] = "Joystick_Read";
    root["Value-X"] = x;
    root["Value-Y"] = y;
    root.prettyPrintTo(JsonPrint);
    /******************** Sending UDP *****************/
    Udp.beginPacket(remote, remote_portt);
    Udp.print(JsonPrint);
    Udp.endPacket();
    /******************** Sending TCP *****************/
    serverClients[0].print(JsonPrint);
    
    JsonPrint = "";  
}

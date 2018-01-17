#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
//#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include "ArduinoOTA.h"  //https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA

//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     TCP      /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
const int TCP_port = 48000;

WiFiServer server(TCP_port);
#define MAX_SRV_CLIENTS 1
WiFiClient serverClients[MAX_SRV_CLIENTS];
//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     UDP      /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
const int UDP_port = 48000;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged";       // a string to send back

#include <WiFiUdp.h>
WiFiUDP Udp;
IPAddress remoteIp(192, 168, 1, 117);


//////////////////////////////////////////////////////////////////////////////////
//////////////////////////   OTA & WIFI manger  //////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void OTA_Config() {
  //Serial.println("Booting");
  WiFiManager wifiManager;
  //wifiManager.resetSettings(); //reset saved settings
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));  //set custom ip for portal
  //wifiManager.autoConnect("APNAME", "password");
  wifiManager.autoConnect("Hodak-Config");
  //wifiManager.autoConnect();
  Serial.println("connected...");//if you get here you have connected to the WiFi
  // ArduinoOTA.setPort(8266);// Port defaults to 8266
  // ArduinoOTA.setHostname("myesp8266");  // Hostname defaults to esp8266-[ChipID]
  //ArduinoOTA.setPassword("admin"); // No authentication by default
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");// Password can be set with it's md5 value as well // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
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
  Serial.println("Ready OTA");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  Udp.begin(UDP_port); // start UDP server
  server.begin();
  server.setNoDelay(true);
}



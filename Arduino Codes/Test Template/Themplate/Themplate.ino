/*
 * Copyright (c) 2018.
 * **********************************************************************************************
 *                         
 *                         NIOMATIC
 *                 NIO GROUP       2017
 *          Email: support@niomatic.com     Web: https://niomatic.com    GitHub Open source
 *
 *   Description: <توضیح در مورد این فایل که چیکار می کنه و.چه مدل سنسوری بهش متصل میشه>
 *   Developer Author:  niomatic open source team                  Date:  12/1/18 9:25 AM
 *   
 *   © Copyright 2018 Nio group – Mobile App Design and Development Company. All Rights Reserved
 *   
 *   Github repository: <بعدا اصافه میشه>
 */


/********************************************************************************
 *                   List of libraries or plugins required
 *                   
 *    jacksonlib..<For examle!>
 */

/********************************************************************************
 *                       List of hardware required
 *                   
 *    HT104 (Light sensor) <For examle!>
 */


/********************************************************************************
 *                        Hardware description
 *                   
 *    توضیحات فارسی (اگر بهم نمیریزد)
 */
 

//Comment: همه اینکلود ها در ابتدای فایل

#include <ArduinoJson.h>
#include <Ethernet.h>
#include "Network.h"
#include "DHT.h"

//Comment: همه ثابت ها در این بخش


#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11


//Comment: همه متغیر های گلوبال در این بخش

const char* j_ask;
String j_ask_s;
int j_port;
int j_continue;
int j_interval ;
int j_repeat ;
String JsonPrint;

DHT DHT_sensor(DHTPIN, DHTTYPE);


void sensor_to_Json() {
  if (j_ask_s = "Temperature-Humidity-Sensor")
  {
    float h = DHT_sensor.readHumidity();
    float t = DHT_sensor.readTemperature();

    //Comment: هر بخش کد که کامنت شده حذف بشه یا اگر به صورت آپشن برای کاربره توضیح داده بهش و با کامنت گذاری جدا بشه برای مثال قسمت زیر

    //If you want to do something..
    /*
    float hif = dht.computeHeatIndex(f, h);
    float hic = dht.computeHeatIndex(t, h, false);
    float f = dht.readTemperature(true);// Read temperature as Fahrenheit (isFahrenheit = true)
    */

    
    if (isnan(h) || isnan(t)) {// Check if any reads failed and exit early (to try again).
      //  Serial.println("Failed to read from DHT sensor!");
      return;
    }
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    root["identify"] = j_ask_s;
    root["Value-temp"] = t;
    root["Value-hum"] = h;
    root.prettyPrintTo(JsonPrint);
  }
}





/**
 * Setup
 */
void setup() {
  Serial.begin(115200);
  OTA_Config();

  DHT_sensor.begin();

}

/**
 * Main loop
 */
void loop() {
  ArduinoOTA.handle();
  /************************************************************************************************
   *                                UDP Server handler
   ************************************************************************************************/
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    IPAddress remote = Udp.remoteIP();
    Udp.read(packetBuffer, packetSize);
    Json_parse(packetBuffer);
    memset(packetBuffer, 0, sizeof(packetBuffer));
    sensor_to_Json();
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.print(JsonPrint);
    Udp.endPacket();
    JsonPrint = "";
  }
  
  /************************************************************************************************
   *                                TCP Server handler
   ************************************************************************************************/
  uint8_t i;
  if (server.hasClient()) {
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
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
  for (i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      if (serverClients[i].available()) {
        //get data from the telnet client and push it to the UART
        int read_size = 0;
        char tcp_read_buffer[250];
        while (serverClients[i].available()) {
          tcp_read_buffer[read_size] = serverClients[i].read();
          read_size++;
        }
        Json_parse(tcp_read_buffer);
        sensor_to_Json();
        if (serverClients[i] && serverClients[i].connected()) {
          serverClients[i].print(JsonPrint);
          delay(1);
        }
        JsonPrint = "";
        read_size = 0;
      }
    }
  }
}



/**
 * Json parser
 * data_in: String input ...
 */

void Json_parse(String data_in) {
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(packetBuffer);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  j_ask = root["ask"];
  j_port = root["port"];
  j_continue = root["continue"];
  j_interval = root["interval"];
  j_repeat = root["repeat"];
  j_ask_s = j_ask;
}

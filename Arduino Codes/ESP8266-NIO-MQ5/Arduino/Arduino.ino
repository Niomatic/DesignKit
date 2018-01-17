/*****************************************************************************************
   Name          : Run MQ-5 Gas sensor using Niomatic App
   Author        : niomatic open source team
   Notice        : © Copyright 2018 Nio group –
                 : Mobile App Design and Development Company. All Rights Reserved
   Email         : support@niomatic.com
   Web           : https://niomatic.com
   Github        :
   Date          : 15/1/18 10:00 AM
   Version       : 1.6
   Description   : Runnig MQ-5 Gas sensor and sending datas with Json over TCP/UDP


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
   MQ-5 Gas Sensor (HYDROGEN,LPG,METHANE,CARBON_MONOXIDE,ALCOHOL) Modules


  /*****************************************************************************************
                                   Hardware description                                  *
******************************************************************************************
  Wiring :
  (MQ-5 sensor module)AOUT pin --> ESP8266 A0 ADC
*/



#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include "ArduinoOTA.h"
#include <ArduinoJson.h>

/************************Hardware Related Macros************************************/
#define         MQ5PIN                       (0)      //define which analog input channel you are going to use
#define         RL_VALUE_MQ5                 (1)      //define the load resistance on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR_MQ5      (6.455)  //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
                                                      //which is derived from the chart in datasheet
/***********************Software Related Macros************************************/
#define         CALIBARAION_SAMPLE_TIMES     (50)    //define how many samples you are going to take in the calibration phase
#define         CALIBRATION_SAMPLE_INTERVAL  (500)   //define the time interal(in milisecond) between each samples in the
                                                     //cablibration phase
#define         READ_SAMPLE_INTERVAL         (50)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (5)     //define the time interal(in milisecond) between each samples in 
                                                     //normal operation
 
/**********************Application Related Macros**********************************/
#define         GAS_HYDROGEN                 (0)
#define         GAS_LPG                      (1)
#define         GAS_METHANE                  (2)
#define         GAS_CARBON_MONOXIDE          (3)
#define         GAS_ALCOHOL                  (4)
#define         accuracy                     (0)    //for linearcurves
//#define         accuracy                   (1)    //for nonlinearcurves, un comment this line and comment the above line if calculations 
                                                    //are to be done using non linear curve equations
float           Ro = 10;                            //Ro is initialized to 10 kilo ohms                                                    
                                                                                                        
/**********************Ethernet**********************************/
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
int j_port;
bool j_continue;
int j_interval ;
int j_repeat ;
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

/****************** MQResistanceCalculation ****************************************/
float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE_MQ5*(1023-raw_adc)/raw_adc));
}

/***************************** MQCalibration ****************************************/
float MQCalibration(int mq_pin)
{
  int i;
  float RS_AIR_val=0,r0;
  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {                     //take multiple samples
    RS_AIR_val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  RS_AIR_val = RS_AIR_val/CALIBARAION_SAMPLE_TIMES;              //calculate the average value
  r0 = RS_AIR_val/RO_CLEAN_AIR_FACTOR_MQ5;                      //RS_AIR_val divided by RO_CLEAN_AIR_FACTOR yields the Ro 
                                                                 //according to the chart in the datasheet 
  return r0; 
}

/*****************************  MQRead *********************************************/
float MQRead(int mq_pin)
{
  int i;
  float rs=0;
  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }
  rs = rs/READ_SAMPLE_TIMES;
  return rs;  
}
/*****************************  MQGetGasPercentage **********************************/
int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{ 
  if ( accuracy == 0 ) {
  if ( gas_id == GAS_HYDROGEN ) {
    return (pow(10,((-3.986*(log10(rs_ro_ratio))) + 3.075)));
  } else if ( gas_id == GAS_LPG ) {
    return (pow(10,((-2.513*(log10(rs_ro_ratio))) + 1.878)));
  } else if ( gas_id == GAS_METHANE ) {
    return (pow(10,((-2.554*(log10(rs_ro_ratio))) + 2.265 )));
  } else if ( gas_id == GAS_CARBON_MONOXIDE ) {
    return (pow(10,((-6.900*(log10(rs_ro_ratio))) + 6.241)));
  } else if ( gas_id == GAS_ALCOHOL ) {
    return (pow(10,((-4.590*(log10(rs_ro_ratio))) + 4.851)));
  }   
} 

  else if ( accuracy == 1 ) {
    if ( gas_id == GAS_HYDROGEN ) {
    return (pow(10,(-22.89*pow((log10(rs_ro_ratio)), 3) + 8.873*pow((log10(rs_ro_ratio)), 2) - 3.587*(log10(rs_ro_ratio)) + 2.948)));
  } else if ( gas_id == GAS_LPG ) {
    return (pow(10,((-2.513*(log10(rs_ro_ratio))) + 1.878)));
  } else if ( gas_id == GAS_METHANE ) {
    return (pow(10,(-0.428*pow((log10(rs_ro_ratio)), 2) - 2.867*(log10(rs_ro_ratio)) + 2.224)));
  } else if ( gas_id == GAS_CARBON_MONOXIDE ) {
    return (pow(10,(1401*pow((log10(rs_ro_ratio)), 4) - 2777*pow((log10(rs_ro_ratio)), 3) + 2059*pow((log10(rs_ro_ratio)), 2) - 682.5*(log10(rs_ro_ratio)) + 88.81)));
  } else if ( gas_id == GAS_ALCOHOL ) {
    return (pow(10,(14.90*pow((log10(rs_ro_ratio)), 3) - 19.26*pow((log10(rs_ro_ratio)), 2) + 3.108*(log10(rs_ro_ratio)) + 3.922)));
  }
}    
  return 0;
}



/**
   Json Generator
   Generate Json form sensor data
*/
void sensor_to_Json() {
  if (j_ask_s == "Gas-Sensor")
  {
    int hydrogen=MQGetGasPercentage(MQRead(MQ5PIN)/Ro,GAS_HYDROGEN);
    int lpg = MQGetGasPercentage(MQRead(MQ5PIN)/Ro,GAS_LPG);
    int methane =MQGetGasPercentage(MQRead(MQ5PIN)/Ro,GAS_METHANE) ;
    int carbon_monoxide =MQGetGasPercentage(MQRead(MQ5PIN)/Ro,GAS_CARBON_MONOXIDE);
    int alcohol = MQGetGasPercentage(MQRead(MQ5PIN)/Ro,GAS_ALCOHOL);
    
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["identify"] = j_ask_s;
    root["Value-hydrogen"] = hydrogen;
    root["Value-lpg"] = lpg;
    root["Value-methane"] = methane;
    root["Value-carbon_monoxide"] = carbon_monoxide;
    root["Value-alcohol"] = alcohol;
    root.prettyPrintTo(JsonPrint);
  }
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
  j_port = root["port"];
  j_continue = root["continue"];
  j_interval = root["interval"];
  j_repeat = root["repeat"];
  j_ask_s = j_ask;
}


/**
   Setup
*/

void setup() {
  Serial.begin(115200);
  OTA_Config();

  Ro = MQCalibration(MQ5PIN);            //Calibrating the sensor. Please make sure the sensor is in clean air when you perform the calibration
  
  Udp.begin(UDP_port); // start UDP server
  server.begin();
  server.setNoDelay(true);
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

    if (j_continue == false)
    {
      int dest_port;
      if (j_port != 0)  dest_port = j_port;
      else dest_port = Udp.remotePort();

      sensor_to_Json();
      Udp.beginPacket(Udp.remoteIP(), dest_port);
      Udp.print(JsonPrint);
      Udp.endPacket();
      JsonPrint = "";
    }
    /************************************************************************************************
                                    Continue, Interval & Repeat
    ************************************************************************************************/
    if (j_continue == true)
    {
      int dest_port;
      if (j_port != 0)  dest_port = j_port;
      else dest_port = Udp.remotePort();

      for (int i = 0; i < j_repeat; i++)
      {
        sensor_to_Json();
        Udp.beginPacket(Udp.remoteIP(), dest_port);
        Udp.print(JsonPrint);
        Udp.endPacket();
        JsonPrint = "";
        delay(j_interval);
      }
    }

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
        if (j_continue == false)
        {
          sensor_to_Json();
           if (serverClients[i] && serverClients[i].connected()) {
          serverClients[i].print(JsonPrint);
           delay(1);
           }
          JsonPrint = "";
        }
        /************************************************************************************************
                                          Continue, Interval & Repeat
          ************************************************************************************************/
        if (j_continue == true)
        {           
          for (int j = 0; j < j_repeat; j++)
          {
            sensor_to_Json();
            Serial.println(JsonPrint);
            
            if (serverClients[i] && serverClients[i].connected()) {
             serverClients[i].print(JsonPrint);
             delay(1);
            }
            JsonPrint = "";
            delay(j_interval);
          }
        }
        read_size = 0;
      }
    }
  }
}

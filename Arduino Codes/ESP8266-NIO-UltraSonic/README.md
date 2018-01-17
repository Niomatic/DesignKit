# ESP8266 NIO UltraSonic


### Schematic:
Tested with UltraSonic HC-SR04 and NODEMCU ESP8266 based Board.
![alt text](https://github.com/elahroodi/ESP8266-NIO-Ultrasonic/raw/master/Schematic/Schematic.png?raw=true)

Command send from NIO to ESP in the JSON structure via TCP or UDP, get back json response and show to graph.

Sending Json request from Niomatic app:
```javascript
{
  "ask": "Ultrasound-Sensor",
  "port": 50000,
  "continue":t ,
  "interval": 10, 
  "repeat": 10
}
```

ESP8266 Response with UltraSonic distance data:
```javascript
{
  "identify": "Ultrasound-Sensor",
  "Value-distance": "11.51",
  "Value-time": 670
}
```


Download nio SDK from google play:
https://play.google.com/store/apps/details?id=nio.sdk
[![GooglePlay](https://play.google.com/intl/en_us/badges/images/generic/en_badge_web_generic.png)](https://play.google.com/store/apps/details?id=nio.sdk&utm_source=Github&utm_campaign=example&pcampaignid=Github)
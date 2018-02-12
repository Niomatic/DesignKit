# ESP8266 Niomatic Set WS2812 RGB LED Strip colors
 Set WS2812 RGB LED Strip color with ESP8266 board and Niomatic android application trough Json over TCP/UDP

### Arduino needed libraries
Instructions on how to install them use this link https://www.arduino.cc/en/Guide/Libraries.

| Library | Download Link |
| ------ | ------ |
| ESP8266 | https://github.com/esp8266/Arduino |
| WiFi Connection manager | https://github.com/tzapu/WiFiManager |
| JSON library | https://github.com/bblanchon/ArduinoJson |
| Adafruit NeoPixel Library | https://github.com/adafruit/Adafruit_NeoPixel |


### Config WIFI Manager
* when your ESP starts up for the first time like below pictures, it will ask you to connect to hotspot connection., it sets it up in Station mode and tries to connect to a previously saved Access Point
* if this is unsuccessful (or no previous network saved) it moves the ESP into Access Point mode and spins up a DNS and WebServer (default ip 192.168.4.1)
using any wifi enabled device with a browser (computer, phone, tablet) connect to the newly created Access Point
because of the Captive Portal and the DNS server you will either get a 'Join to network' type of popup or get any domain you try to access redirected to the configuration portal
choose one of the access points scanned, enter password, click save
ESP will try to connect. If successful, it relinquishes control back to your app. If not, reconnect to AP and reconfigure.

![alt text](https://github.com/Niomatic/DesignKit/blob/design-stage/Arduino%20Codes/ESP8266-NIO-UltraSonic/Schematic/Guide.png?raw=true)


### OTA onfig (over the air programming)
After uploading the first code to ESp8266 you can select network port to upload your codes wireless.
![alt text](https://github.com/Niomatic/DesignKit/blob/design-stage/Arduino%20Codes/ESP8266-NIO-UltraSonic/Schematic/OTA_port.png?raw=true)

#### ESP8266 IP
You can monitor serial port at startup to get the ESP8266 IP.
![alt text](https://github.com/Niomatic/DesignKit/blob/design-stage/Arduino%20Codes/ESP8266-NIO-UltraSonic/Schematic/IP.png?raw=true)

### WS2812 pin Config
```C
#define PIN  D1
#define NUMPIXELS      18 // How many NeoPixels are attached to the ESP8266
```


### Schematic
![alt text](https://github.com/Niomatic/DesignKit/blob/design-stage/Arduino%20Codes/ESP8266-NIO-WS2812-RGB/Schemtaic/Schematic.png?raw=true)

### Code 

Command send from NIO to ESP in the JSON structure via TCP or UDP, get back json response and show to Niomatic app.

Sending Json request from Niomatic app:
```javascript
{
  "ask": "Set_rgb",
  "pixel_number": "13",
  "red": "0",
  "green": "0",
  "blue": "255"
}
```



Download nio SDK from google play:
https://play.google.com/store/apps/details?id=nio.sdk
[![GooglePlay](https://play.google.com/intl/en_us/badges/images/generic/en_badge_web_generic.png)](https://play.google.com/store/apps/details?id=nio.sdk&utm_source=Github&utm_campaign=example&pcampaignid=Github)

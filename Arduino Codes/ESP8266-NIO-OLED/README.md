# ESP8266 Niomatic Running OLED display
Running SSD1306 OLED display with ESP8266 board and Niomatic android application trough Json over TCP/UDP

### Arduino needed libraries
Instructions on how to install them use this link https://www.arduino.cc/en/Guide/Libraries.

| Library | Download Link |
| ------ | ------ |
| ESP8266 | https://github.com/esp8266/Arduino |
| WiFi Connection manager | https://github.com/tzapu/WiFiManager |
| JSON library | https://github.com/bblanchon/ArduinoJson |
| SSD1306 OLED library | https://github.com/squix78/esp8266-oled-ssd1306 |


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

### Define OLED pin config
```C
#define SDA D3
#define SCL D5
```


### Schematic
![alt text](https://github.com/Niomatic/DesignKit/blob/design-stage/Arduino%20Codes/ESP8266-NIO-OLED/Schemtaic/Schematic.png?raw=true)

# Code 
Command send from NIO to ESP in the JSON structure via TCP or UDP, get back json response and show to Niomatic app.
Sending Json request from Niomatic app:

# Draw Circle
```javascript
{
  "ask": "Set_OLED",
  "mode": "draw_circle",
  "x": "10",
  "y": "10",
  "radius": "5",
  "fill": true
}
```

# Draw Text
```javascript
{
  "ask": "Set_OLED",
  "mode": "draw_text",
  "x": "10",
  "y": "10",
  "text": "Hello",
  "font_size": "small"  //small,medium,large
}
```

# Draw Rectangular
```javascript
{
  "ask": "Set_OLED",
  "mode": "draw_rect",
  "x": "10",
  "y": "10",
  "width": "10",
  "height": "10",
  "fill": true
}
```

# Draw Line
```javascript
{
  "ask": "Set_OLED",
  "mode": "draw_line",
  "x": "10",
  "y": "10",
  "lenght": "100",
  "orientation": "horizontal"
}
```

# Draw Pixel
```javascript
{
  "ask": "Set_OLED",
  "mode": "draw_pixel",
  "x": "10",
  "y": "10"
}
```

# Clear OLED display
```javascript
{
  "ask": "Set_OLED",
  "mode": "clear"
}
```

Download nio SDK from google play:
https://play.google.com/store/apps/details?id=nio.sdk
[![GooglePlay](https://play.google.com/intl/en_us/badges/images/generic/en_badge_web_generic.png)](https://play.google.com/store/apps/details?id=nio.sdk&utm_source=Github&utm_campaign=example&pcampaignid=Github)

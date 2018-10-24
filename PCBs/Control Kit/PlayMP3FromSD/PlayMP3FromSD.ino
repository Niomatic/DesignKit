#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"
#include <SPI.h>
#include <SD.h>
Sd2Card card;
const int chipSelect = D0;
AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2SNoDAC *out;

void setup()
{
  WiFi.mode(WIFI_OFF);
  Serial.begin(115200);
  Serial.print("Initializing SD card...");
  if (!SD.begin(D0)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("SD OK.");
  delay(1000);
  Serial.printf("Sample MP3 playback begins...\n");
  file = new AudioFileSourceSD("/v.mp3");
  out = new AudioOutputI2SNoDAC();
  mp3 = new AudioGeneratorMP3();
  mp3->begin(file, out);
}
int i = 13;
bool finished;
char *ss[] = {"/0.mp3", "/1.mp3", "/2.mp3", "/3.mp3", "/4.mp3", "/5.mp3", "/6.mp3", "/7.mp3", "/8.mp3", "/9.mp3", "/10.mp3", "/v.mp3", "/20_.mp3", "/900_.mp3", "/40_.mp3"};
void loop()
{
  if (mp3->isRunning()) {
    finished = false;
    if (!mp3->loop())
    {
      finished = true;
      mp3->stop();

    }
  } else {
    // i++;
    //     Serial.println("MP3 done");
    //  delay(1000);
  }

  if (finished == true)
  {
    file = new AudioFileSourceSD(ss[i]);
    mp3->begin(file, out);
finished = false;
    if (i == 13)i = 14;
    else if (i == 14)i = 6;
    else if (i == 6)i = 13;
  }


}


// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// MAX44009
// This code is designed to work with the MAX44009_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/products

#include<Wire.h>

// MAX44009 I2C address is 0x4A(74)
#define Addr 0x4A

void setup()
{
  // Initialise I2C communication as MASTER
  Wire.begin();
  // Initialise serial communication, set baud rate = 9600
  Serial.begin(9600);

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select configuration register
  Wire.write(0x02);
  // Continuous mode, Integration time = 800 ms
  Wire.write(0x40);
  // Stop I2C transmission
  Wire.endTransmission();
  delay(300);
}

void loop()
{
  unsigned int data[2];

  // Start I2C Transmission
  Wire.beginTransmission(Addr);
  // Select data register
  Wire.write(0x03);
  // Stop I2C transmission
  Wire.endTransmission();

  // Request 2 bytes of data
  Wire.requestFrom(Addr, 2);

  // Read 2 bytes of data
  // luminance msb, luminance lsb
  if (Wire.available() == 2)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }

  // Convert the data to lux
  int exponent = (data[0] & 0xF0) >> 4;
  int mantissa = ((data[0] & 0x0F) << 4) | (data[1] & 0x0F);
  float luminance = pow(2, exponent) * mantissa * 0.045;

  // Output data to serial monitor
  Serial.print("Ambient Light luminance :");
  Serial.print(luminance);
  Serial.println(" lux");
  delay(300);
}
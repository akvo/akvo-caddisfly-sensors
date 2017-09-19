/*
 * MCP3426/7/8 Libary
 * Kerry D. Wong
 * http://www.kerrywong.com
 * 5/2012
 */

#include "MCP342X.h"
#include <Wire.h>

void MCP342X::init(byte A0, byte A1)
{
	if ((A0 == L && A1 == L) || (A0 == F && A1 == F)) I2C_ADDRESS = B1101000;
	else if (A0 == L && A1 == F) I2C_ADDRESS = B1101001;
	else if (A0 == L && A1 == H) I2C_ADDRESS = B1101010;
	else if (A0 == H && A1 == L) I2C_ADDRESS = B1101100;
	else if (A0 == H && A1 == F) I2C_ADDRESS = B1101101;
	else if (A0 == H && A1 == H) I2C_ADDRESS = B1101110;
	else if (A0 == F && A1 == L) I2C_ADDRESS = B1101011;
	else if (A0 == F && A1 == H) I2C_ADDRESS = B1101111;
}

void MCP342X::selectChannel(byte channel, byte gain)
{  

    //configuration register, assuming 16 bit resolution
	byte reg = 1 << BIT_RDY |
			channel << BIT_C0 |
			1 << BIT_OC |
			1 << BIT_S1 |
			gain;

	Wire.beginTransmission(I2C_ADDRESS);
	Wire.write(reg);
	Wire.endTransmission();			   
}

double MCP342X::readADC()
{
  	Wire.requestFrom(I2C_ADDRESS, (byte) 3);
  	
	int h = Wire.read();
  	int l = Wire.read();
  	int r = Wire.read();
  
  	long t = h << 8 |  l;
  	
	if (t >= 32768) t = t - 65536l;
  	
	double v = (double) t * 2.048/32768.0;

	return v;
}

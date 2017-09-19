#include "ecmeter.h"
#include "MCP342X.h"
#include <Wire.h>
#include <Arduino.h>

#define DEBUG false

MCP342X mcp3428;

//Change these variables to the TEMPERATURE COMPENSATED RAW EC values from testing
// At the moment, these values are ALWAYS overwritten with those in the flash memory
float point1 = 139;   //141
float point2 = 249;   //235
float point3 = 500;   //472
float point4 = 1065;  //1413
float point5 = 2601;  //3000
float point6 = 6114;  //12880

// Change this value to correspond to the open voltage value
// it is part of the calibration values
float vopen = 1970.0;

// DO NOT change these 5 variables
// These should correspond to the actual calibration fluid values @ 25 degrees
//#define point1ActualEC  141
//#define point2ActualEC  235
//#define point3ActualEC  472
//#define point4ActualEC  1413
//#define point5ActualEC  3000
//#define point6ActualEC  12880

#define point1ActualEC  141
#define point2ActualEC  235
#define point3ActualEC  472
#define point4ActualEC  1413
#define point5ActualEC  3000
#define point6ActualEC  12880

// resistor in series with measurement cell. This should not be changed,
// inaccuracies will be compensated in the calibration.
#define R2 1000.0

// factor to convert to microsiemens
#define TOMICROSIEMENS 1000000.0

// thermal correction coefficient
// We should use ISO 7888:1995 "“Water Quality – Determination of Electrical Conductivity”"
#define alpha 0.02

//These 2 variables are used for calibrating the low range (EC <= point2)
float line1Factor = 0.0;
float line1Offset = 0.0;

float line2Factor = 0.0;
float line2Offset = 0.0;

float line3Factor = 0.0;
float line3Offset = 0.0;

float line4Factor = 0.0;
float line4Offset = 0.0;

float line5Factor = 0.0;
float line5Offset = 0.0;

//This method initializes all IO's and ADC
void ECMeter_init()
{
  pinMode(PFET, OUTPUT);
  digitalWrite(PFET, HIGH);
  pinMode(NFET, OUTPUT);
  digitalWrite(NFET, LOW);

  pinMode(SWITCH, OUTPUT); //TO DO: WHY IS THIS NOT SET LOW OR HIGH?

  Wire.begin();
  mcp3428.init(MCP342X::L, MCP342X::L);
  calculateCalibrationValues();
}
///////////////////////////////////////////////////////////////////////

// Corrects an EC value at temperature T, using the linear temperature correction model.
float correctEC25(float ECraw, float T)
{
  return ECraw / (1 + alpha * (T - 25.0));
}

float returnLine1Calibration(float rawConductivity)
{
  return rawConductivity * line1Factor + line1Offset;
}

float returnLine2Calibration(float rawConductivity)
{
  return rawConductivity * line2Factor + line2Offset;
}

float returnLine3Calibration(float rawConductivity)
{
  return rawConductivity * line3Factor + line3Offset;
}

float returnLine4Calibration(float rawConductivity)
{
  return rawConductivity * line4Factor + line4Offset;
}

float returnLine5Calibration(float rawConductivity)
{
  return rawConductivity * line5Factor + line5Offset;
}

///////////////////////////////////////////////////////////////////////

// calculates slope and offset of all the calibration lines
void calculateCalibrationValues()
{

  line1Factor = (point2ActualEC-point1ActualEC) / (point2-point1);
  line1Offset = point1ActualEC - point1*line1Factor;

  line2Factor = (point3ActualEC-point2ActualEC) / (point3-point2);
  line2Offset = point2ActualEC - point2*line2Factor;

  line3Factor = (point4ActualEC-point3ActualEC) / (point4-point3);
  line3Offset = point3ActualEC - point3*line3Factor;

  line4Factor = (point5ActualEC-point4ActualEC) / (point5-point4);
  line4Offset = point4ActualEC - point4*line4Factor;

  line5Factor = (point6ActualEC-point5ActualEC) / (point6-point5);
  line5Offset = point5ActualEC - point5*line5Factor;
}

// returns the calibrated conductivity value
// this function is valid if there are 5 lines
float returnCalibratedValue(float rawECValue)
{
  //line 1 --> smaller than point2
  if(rawECValue < point2){
    return returnLine1Calibration(rawECValue);
  }
  else if(point2 <= rawECValue && rawECValue < point3){
    return returnLine2Calibration(rawECValue);
  }
  else if(point3 <= rawECValue && rawECValue < point4){
    return returnLine3Calibration(rawECValue);
  }
  else if(point4 <= rawECValue && rawECValue < point5){
    return returnLine4Calibration(rawECValue);
  }
  else{
    return returnLine5Calibration(rawECValue);
  }
}

// DEPRECATED - this uses the B-parameter method which is less accurate than the
// Steinhart-Hart method used below.
//Returns temperature of a thermistor as a float in degrees celcius
//float steinhartTemp1(float resistance)
//{
//  float steinhart;
//  steinhart = resistance / THERMISTORNOMINAL;
//  steinhart = log(steinhart);
//  steinhart /= BCOEFFICIENT;
//  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15);
//  steinhart = 1.0 / steinhart;
//  steinhart -= 273.15;
//
//  return steinhart;
//}

float steinhartTemp(float resistance){
  float temp;
  temp = log(resistance);
  temp = STEINHART_A + STEINHART_B * temp + STEINHART_C * temp * temp * temp;
  temp = 1/temp;
  temp = temp - 273.15; // Convert Kelvin to Celcius
  return temp;
  }

//Returns temperature as a float in degrees celcius
float getTemp()
{
  mcp3428.selectChannel(MCP342X::CHANNEL_1, MCP342X::GAIN_1);
  delay(100);
  
  // average 5 measurements
  float voltage = 0;
  for (int i=0; i < 5; i++){
    voltage += mcp3428.readADC();
    delayMicroseconds(10);
  }
  voltage /= 5.0;
  
  float resistance = (float)(((3000.0*3.3)/voltage) - 3000.0);
  float temperature = steinhartTemp(resistance);
  return temperature;
}


// measures voltage response of sensor, in millivolts
float getVoltage()
{
    mcp3428.selectChannel(MCP342X::CHANNEL_0, MCP342X::GAIN_1);
    // let ADC channel settle
    delay(10);
    
   float voltage = 0.0;
   for (int j=0; j<10; j++)
   {
      for(int i = 0; i < 20000; i++)
      {
          digitalWrite(PFET, LOW);
          digitalWrite(PFET, HIGH);
          digitalWrite(NFET, HIGH);
          digitalWrite(NFET, LOW);
      }
      voltage += mcp3428.readADC();
    }
    voltage /= 10.0;
    // return millivolts
    return voltage * 1000.0;
}


//Measures conductance in microSiemens. Note: this is NOT the conductivity, but simply the inverse of the resistance!
// it is also not temperature corrected yet.
float getRawConductance()
{
    // restrict conductance measurement to once every 2 seconds, in order to avoid polarisation
    delay(2 * 1000);

    float voltage = getVoltage();
    
    if (DEBUG) {
      SerialUSB.print("Vopen: "); SerialUSB.println(vopen);
      }
    
    // limit to zero. This can only happen when the open circuit voltage drifts, and we 
    // are measuring very low conductivity (i.e. demineralized water)
    if (voltage > vopen)
      return 0;
    float result = (TOMICROSIEMENS / R2) * ((vopen / voltage) - 1.0);
    if (DEBUG) {
      SerialUSB.print("Raw conductance: "); SerialUSB.println(result);
      }
    return result;
}

// returns temperature compensated and calibrated conductivity.
float getConductivity()
{
  float conductance = getRawConductance();
  float temp = getTemp();
  
  //Add temperature compensation
  conductance = correctEC25(conductance,temp);

  if (DEBUG) {
      SerialUSB.print("temperature compensated conductance: "); SerialUSB.println(conductance);
      }
  
  //Apply calibration
  return returnCalibratedValue(conductance);
}


void setVopen (float _vopen){
  vopen = _vopen;
}
  
void setPoints(float _point1, float _point2, float _point3, float _point4, float _point5, float _point6, float _vopen){
  point1 = _point1;
  point2 = _point2;
  point3 = _point3;
  point4 = _point4;
  point5 = _point5;
  point6 = _point6;
  vopen = _vopen;
}

//////////////////////////// functions used during calibration /////////////////////////////

// returns value for conductance without calibration. Temperature correction is applied
float onePointRawConductance(){
  int numAvg = 5;
  // skip first two measurements
  getRawConductance();
  getRawConductance();
  
  float totalTemp = 0;
  float totalConductance = 0;
  float avgTemp = 0;
  float avgConductance = 0;

  for (int i=0;i < numAvg;i++){
    totalTemp += getTemp();
    totalConductance += getRawConductance();
  }
  avgTemp = totalTemp/numAvg;
  avgConductance = totalConductance/numAvg;
  
  // apply temperature correction
  float conductance = correctEC25(avgConductance,avgTemp);

  return conductance;
}

// measures open circuit voltage
float openCircuitVoltage()
{
  int numAvg = 7;
 // skip first two measurements
  getVoltage();
  getVoltage();

  float totalVoltage = 0.0;

  for (int i=0;i < numAvg;i++){
    totalVoltage += getVoltage();
    delay(250);
  }
  return totalVoltage / numAvg;
}
 

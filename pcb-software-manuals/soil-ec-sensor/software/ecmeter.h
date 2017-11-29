#ifndef ECMETER_H
#define ECMETER_H

#include <Arduino.h>
#include <Wire.h>

// Deprecated - not used any more
//#define THERMISTORNOMINAL 5000   
//#define TEMPERATURENOMINAL 25   
//#define BCOEFFICIENT 3977

// Steinhart-Hart equation parameters
// calculated using http://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm
// from datasheet http://www.farnell.com/datasheets/2046880.pdf?_ga=1.98230492.238186371.1473424295
// at temperatures -10, 25, and 60.
#define STEINHART_A 1.2914E-3
#define STEINHART_B 2.3484E-4
#define STEINHART_C 1.0108E-7

#define PFET 8
#define NFET 9
#define SWITCH A4

void ECMeter_init();
void setVopen(float vopen);
void setPoints(float point1, float point2, float point3, float point4, float point5, float point6, float vopen);
float getTemp();
float getVoltage();
float getRawConductance();
float getConductivity();
void calculateCalibrationValues();
float returnCalibratedValue(float rawECValue);
float returnLine1Calibration(float rawConductivity);
float returnLine2Calibration(float rawConductivity);
float returnLine3Calibration(float rawConductivity);
float returnLine4Calibration(float rawConductivity);
float returnLine5Calibration(float rawConductivity);
float onePointRawConductance();
float openCircuitVoltage();

#endif


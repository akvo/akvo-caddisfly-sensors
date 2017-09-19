/*
 * Utils.cpp
 *
 * Created: 17/12/2015
 *  Author: Gabriel Notman
 */

#include <Arduino.h>
#include "RTCZero.h"
#include "Utils.h"
#include "ProjectDefinitions.h"

//void add2D(uint8_t val, String& str)
//{
//  if (val < 10) { 
//    str += "0";
//  }
//  str += val;
//}

/*
 * Format an integer as %0*x
 *
 * Arduino formatting sucks.
 */
//void add0Nx(String &str, uint16_t val, size_t width)
//{
//  char buf[20];
//  if (width >= 4 && val < 0x1000) {
//    str += '0';
//  }
//  if (width >= 3 && val < 0x100) {
//    str += '0';
//  }
//  if (width >= 2 && val < 0x10) {
//    str += '0';
//  }
//  utoa(val, buf, 16);
//  str += buf;
//}

uint16_t crc16ccitt(const uint8_t *buf, size_t len)
{
  uint16_t crc = 0;
  while(len--) {
    crc ^= (*buf++ << 8);
    for(uint8_t i = 0; i < 8; ++i ) {
      if (crc & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      }
      else {
        crc = crc << 1;
      }
    }
  }
  return crc;
}

uint16_t getBatteryVoltage()
{
  return (uint16_t)((ADC_AREF / 1.023) * (BATVOLT_R1 + BATVOLT_R2) / BATVOLT_R2 * (float)analogRead(BATVOLT_PIN));
}

//String getDateTime(RTCZero& rtc)
//{
//  String dateTimeStr;
//
//  add2D(rtc.getDay(), dateTimeStr);
//  dateTimeStr += "/";
//  add2D(rtc.getMonth(), dateTimeStr);
//  dateTimeStr += "/20";
//  add2D(rtc.getYear(), dateTimeStr);
//  dateTimeStr += " ";
//  add2D(rtc.getHours(), dateTimeStr);
//  dateTimeStr += ":";
//  add2D(rtc.getMinutes(), dateTimeStr);
//  dateTimeStr += ":";
//  add2D(rtc.getSeconds(), dateTimeStr);
//  
//  return dateTimeStr;  
//}

//bool timeOut32(uint32_t startTS, uint32_t ms)
//{
//  uint32_t nowTS = millis();
//  uint32_t diffTS = (nowTS >= startTS) ? nowTS - startTS : nowTS + (UINT32_MAX - startTS);
//
//  return (diffTS > ms);
//}

bool isTimedOut(uint32_t ts)
{
  return (long)(millis() - ts) >= 0;
}

bool isWithin(uint32_t value, uint32_t valueToCompareAgainst, uint32_t variation)
{
    return (value >= valueToCompareAgainst - variation) && (value <= valueToCompareAgainst + variation);
}

uint8_t getCompactBatteryLevel(uint16_t batteryLevel)
{
    uint16_t result = (batteryLevel - 2000) / 10;
    if (result > 255) {
        result = 255;
    }
    return static_cast<uint8_t>(result);
}

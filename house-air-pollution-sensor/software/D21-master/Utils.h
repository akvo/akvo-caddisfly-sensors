/*
 * Utils.h
 *
 * Created: 17/12/2015
 *  Author: Gabriel Notman
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>
#include "RTCZero.h"

//void add2D(uint8_t val, String& str);

//void add0Nx(String &str, uint16_t val, size_t width);
//static inline void add04x(String &str, uint16_t val) { add0Nx(str, val, 4); }
//static inline void add02x(String &str, uint16_t val) { add0Nx(str, val, 2); }

uint16_t crc16ccitt(const uint8_t *buf, size_t len);

uint16_t getBatteryVoltage();

//String getDateTime(RTCZero& rtc);

extern uint32_t getNow();

//bool timeOut32(uint32_t startTS, uint32_t ms);

bool isTimedOut(uint32_t ts);

bool isWithin(uint32_t value, uint32_t valueToCompareAgainst, uint32_t variation);

uint8_t getCompactBatteryLevel(uint16_t batteryLevel);

#endif /* UTILS_H_ */

/*
 * Copyright 2016, M2M4ALL
 *
 * This is part of the SSU WAP.
 * Various utility functions.
 */

#include <stdlib.h>
#include <Arduino.h>

#include "Settings.h"
#include "Utils.h"

/*
 * Format an integer as %0*x
 *
 * Arduino formatting sucks.
 */
void add0Nx(String &str, uint16_t val, size_t width)
{
  char buf[20];
  if (width >= 4 && val < 0x1000) {
    str += '0';
  }
  if (width >= 3 && val < 0x100) {
    str += '0';
  }
  if (width >= 2 && val < 0x10) {
    str += '0';
  }
  utoa(val, buf, 16);
  str += buf;
}

/*
 * Extract a number from an ASCII string
 */
bool getUValue(const char *buffer, uint32_t * value)
{
    char *eptr;
    if (*buffer == '=') {
        ++buffer;
    }
    *value = strtoul(buffer, &eptr, 0);
    if (eptr != buffer) {
        return true;
    }
    return false;
}

uint16_t crc16ccitt(const uint8_t *buf, size_t len)
{
    uint16_t crc = 0;
    while (len--) {
        crc ^= (*buf++ << 8);
        for (uint8_t i = 0; i < 8; ++i) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc = crc << 1;
            }
        }
    }
    return crc;
}

bool isTimedOut(uint32_t ts)
{
    return (long)(millis() - ts) >= 0;
}

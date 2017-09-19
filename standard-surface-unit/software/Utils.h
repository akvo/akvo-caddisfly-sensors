/*
 * Copyright 2016, M2M4ALL
 *
 * This is part of the SSU WAP.
 * Various utility functions.
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stddef.h>
#include <stdint.h>

void add0Nx(String &str, uint16_t val, size_t width);
static inline void add04x(String &str, uint16_t val) { add0Nx(str, val, 4); }
static inline void add02x(String &str, uint16_t val) { add0Nx(str, val, 2); }

bool getUValue(const char *buffer, uint32_t * value);

uint16_t crc16ccitt(const uint8_t *buf, size_t len);

bool isTimedOut(uint32_t ts);

#endif /* UTILS_H_ */

/*
 * DeviceID.cpp
 */

#include <Arduino.h>
#include <stdint.h>

#include "Utils.h"
#include "DiagStream.h"

#include "DeviceID.h"

#define ENABLE_DEVICE_ID_DIAG          1

#define DEBUG_PREFIX String("[DEVICE ID]")

#if ENABLE_DEVICE_ID_DIAG
  #define myDiagPrint(...) do { if (diag_stream.isEnabled()) diag_stream.print(__VA_ARGS__); } while(0)
  #define myDiagPrintLn(...) do { if (diag_stream.isEnabled()) diag_stream.println(__VA_ARGS__); } while (0)
#else
  #define myDiagPrint(...)
  #define myDiagPrintLn(...)
#endif

static uint8_t deviceID[8];
static bool IDready = false;

static void readID();

void getDeviceID(uint8_t (&idBuffer)[8])
{
  if (!IDready) {
    readID();
  }

  memcpy(idBuffer, deviceID, sizeof(idBuffer));
}

void addDeviceIDToString(String & str)
{
  if (!IDready) {
    readID();
  }
  uint8_t * ptr = deviceID;
  for (size_t i = 0; i < sizeof(deviceID); ++i) {
    add02x(str, *ptr);
    ++ptr;
  }
}

/*!
 * Create a unique device ID
 *
 * This is done by using the 4 words (32 bits) of the SAMD Serial number.
 * Word 1 and 3 are XOR-ed, and word 2 and 4 are XOR-ed.
 * Then the two words are placed in a 8 byte array. LS byte first (little
 * endian).
 */
static void readID()
{
  myDiagPrintLn(DEBUG_PREFIX + "SAMD Serial Number");
  const uint32_t *sernum1 = (uint32_t *)0x0080A00C;
  const uint32_t *sernum2 = (uint32_t *)0x0080A040;
  const uint32_t *sernum3 = (uint32_t *)0x0080A044;
  const uint32_t *sernum4 = (uint32_t *)0x0080A048;
  uint32_t word1 = *sernum1 ^ *sernum3;
  uint32_t word2 = *sernum2 ^ *sernum4;
  memcpy(&deviceID[0], &word1, sizeof(word1));
  memcpy(&deviceID[4], &word2, sizeof(word2));

  IDready = true;

  String str = "Device ID: ";
  addDeviceIDToString(str);
  myDiagPrintLn(DEBUG_PREFIX + str);
}

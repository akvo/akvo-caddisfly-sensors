#include "DeviceID.h"

bool DeviceId::isInitialized = false;
uint8_t DeviceId::deviceID[8] = {0,};

/*
 * Returns the cached device id. 
 * If the device id is not cached, it is initialized first.
 */
void DeviceId::get(uint8_t (&idBuffer)[8])
{
    if (!isInitialized) {
        initialize();
    }

    memcpy(idBuffer, deviceID, sizeof(idBuffer));
}

/*
 * Prints the cached device id. 
 * If the device id is not cached, it is initialized first.
 */
void DeviceId::printTo(Stream& stream)
{
    if (!isInitialized) {
        initialize();
    }

    for (uint8_t i = 0; i < 8; i++) {
        stream.print(deviceID[i], HEX);
        if (i < 7) {
            stream.print(":");
        }
    }
}

/*
 * Creates a unique device ID.
 *
 * This is done by using the 4 words (32 bits) of the SAMD Serial number.
 * Word 1 and 3 are XOR-ed, and word 2 and 4 are XOR-ed.
 * Then the two words are placed in a 8 byte array. LS byte first (little
 * endian).
 */
void DeviceId::initialize()
{
    const uint32_t* sernum1 = (uint32_t*)0x0080A00C;
    const uint32_t* sernum2 = (uint32_t*)0x0080A040;
    const uint32_t* sernum3 = (uint32_t*)0x0080A044;
    const uint32_t* sernum4 = (uint32_t*)0x0080A048;

    uint32_t word1 = *sernum1 ^ *sernum3;
    uint32_t word2 = *sernum2 ^ *sernum4;

    memcpy(&deviceID[0], &word1, sizeof(word1));
    memcpy(&deviceID[4], &word2, sizeof(word2));

    isInitialized = true;
}

/*
 * (Private) Constructor
 */
DeviceId::DeviceId()
{
}

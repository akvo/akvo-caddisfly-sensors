#ifndef DEVICE_ID_H_
#define DEVICE_ID_H_

#include <Arduino.h>

class DeviceId
{
public:
    static void get(uint8_t (&idBuffer)[8]);
    static void printTo(Stream& stream);

private:
    static bool isInitialized;
    static uint8_t deviceID[8];

    DeviceId();
    static void initialize();
};

#endif /* DEVICE_ID_H_ */


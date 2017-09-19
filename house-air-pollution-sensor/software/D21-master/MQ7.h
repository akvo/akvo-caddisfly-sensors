#ifndef _MQ7_h
#define _MQ7_h

#include "Arduino.h"

enum SensorPowerVoltage
{
    Off,
    FiveVolts,
    OnePointFourVolts
};

class MQ7
{
public:
    MQ7();
    void init(uint8_t analogPin, uint8_t powerPin);
    void disable();
    void powerCycleHandler();
    uint16_t read();
    bool canRead();
private:
    uint8_t _analogPin;
    uint8_t _powerPin;

    uint32_t _prevPowerCycleTimestamp;
    SensorPowerVoltage _currentVoltage;

    void setVoltage(SensorPowerVoltage voltage);
    void startTimerPWM() const;
    void stopTimerPWM() const;
};

extern MQ7 mq7;

#endif

#include "MQ7.h"
#include "PrintSystem.h"

#define MQ7_HIGH_CYCLE_LENGTH 60 * 1000 // 60 seconds
#define MQ7_LOW_CYCLE_LENGTH 90 * 1000 // 90 seconds

MQ7::MQ7(): _analogPin(0), _powerPin(0), _prevPowerCycleTimestamp(0), _currentVoltage()
{
}

void MQ7::init(uint8_t analogPin, uint8_t powerPin)
{
    _analogPin = analogPin;
    _powerPin = powerPin;

    pinMode(_powerPin, OUTPUT);
    digitalWrite(_powerPin, false);

    _prevPowerCycleTimestamp = 0;
}

void MQ7::disable()
{
    setVoltage(Off);
    _prevPowerCycleTimestamp = 0;
}

void MQ7::powerCycleHandler()
{
    uint32_t timestamp = millis();
    bool isHeaterHigh = _currentVoltage == FiveVolts;

    if (timestamp - _prevPowerCycleTimestamp > (isHeaterHigh ? MQ7_HIGH_CYCLE_LENGTH : MQ7_LOW_CYCLE_LENGTH)) {
        if (isHeaterHigh) {
            debugPrintLn("MQ7: Setting voltage to 1.4V");
            setVoltage(OnePointFourVolts);
        }
        else {
            debugPrintLn("MQ7: Setting voltage to 5V");
            setVoltage(FiveVolts);
        }

        _prevPowerCycleTimestamp = timestamp;
    }
}

uint16_t MQ7::read()
{
    return analogRead(_analogPin);
}

bool MQ7::canRead()
{
    return (_currentVoltage == OnePointFourVolts);
}

void MQ7::setVoltage(SensorPowerVoltage voltage)
{
    if (voltage == FiveVolts) {
        stopTimerPWM();
        digitalWrite(_powerPin, HIGH);
    }
    else if (voltage == OnePointFourVolts) {
        digitalWrite(_powerPin, LOW);
        startTimerPWM();
    }
    else {
        stopTimerPWM();
        digitalWrite(_powerPin, LOW);
    }

    _currentVoltage = voltage;
}

void MQ7::startTimerPWM() const
{
    analogWrite(_powerPin, 80);
}

void MQ7::stopTimerPWM() const
{
    analogWrite(_powerPin, 0);
}

MQ7 mq7;
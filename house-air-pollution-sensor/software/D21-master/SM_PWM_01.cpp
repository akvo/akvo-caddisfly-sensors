#include "SM_PWM_01.h"
#include "PrintSystem.h"

#define PREHEAT_SECONDS 100

uint8_t SM_PWM_01::_powerPin;
uint8_t SM_PWM_01::_smallParticlesPin;
uint8_t SM_PWM_01::_largeParticlesPin;

volatile uint32_t SM_PWM_01::_smallParticlesTimestamp;
volatile uint32_t SM_PWM_01::_largeParticlesTimestamp;

volatile uint32_t SM_PWM_01::_smallParticleOnTime;
volatile uint32_t SM_PWM_01::_largeParticleOnTime;

uint32_t SM_PWM_01::_lastCalculationTimestamp;
uint32_t SM_PWM_01::_lastPowerOnTimestamp;

void SM_PWM_01::init(const uint8_t powerPin, const uint8_t smallParticlesPin, const uint8_t largeParticlesPin)
{
    _powerPin = powerPin;
    _smallParticlesPin = smallParticlesPin;
    _largeParticlesPin = largeParticlesPin;

    pinMode(_powerPin, OUTPUT);
    pinMode(_smallParticlesPin, INPUT); // no pull-up, the sensor provides pull-up
    pinMode(_largeParticlesPin, INPUT); // no pull-up, the sensor provides pull-up

    _smallParticlesTimestamp = 0;
    _largeParticlesTimestamp = 0;

    _smallParticleOnTime = 0;
    _largeParticleOnTime = 0;

    _lastCalculationTimestamp = 0; // in ms
    _lastPowerOnTimestamp = 0; // in ms

    digitalWrite(_powerPin, LOW);

    // attach the intterrupts
#ifdef __AVR__
    attachInterrupt(digitalPinToInterrupt(_smallParticlesPin), smallParticlePinChangeCallback, CHANGE);
    attachInterrupt(digitalPinToInterrupt(_largeParticlesPin), largeParticlePinChangeCallback, CHANGE);
#else
    attachInterrupt(_smallParticlesPin, smallParticlePinChangeCallback, CHANGE); // SAMD only
    attachInterrupt(_largeParticlesPin, largeParticlePinChangeCallback, CHANGE); // SAMD only
#endif
}

void SM_PWM_01::disable()
{
    // detach the interrupts
#ifdef __AVR__
    detachInterrupt(digitalPinToInterrupt(_smallParticlesPin), smallParticlePinChangeCallback, CHANGE);
    detachInterrupt(digitalPinToInterrupt(_largeParticlesPin), largeParticlePinChangeCallback, CHANGE);
#else
    detachInterrupt(_smallParticlesPin); // SAMD only
    detachInterrupt(_largeParticlesPin); // SAMD only
#endif

    setPower(false);
}

void SM_PWM_01::setPower(bool on)
{
    digitalWrite(_powerPin, on);
    if (on) {
        _lastPowerOnTimestamp = millis();
    }
    else {
        _lastPowerOnTimestamp = 0; // it is always 0 when power is off
    }
}

void SM_PWM_01::smallParticlePinChangeCallback()
{
    if (digitalRead(_smallParticlesPin)) {
        // check if the timestamp has been initialized first, otherwise disregard the change
        if (_smallParticlesTimestamp > 0) {
            _smallParticleOnTime += millis() - _smallParticlesTimestamp;
        }
    }
    else {
        _smallParticlesTimestamp = millis();
    }
}

void SM_PWM_01::largeParticlePinChangeCallback()
{
    if (digitalRead(_largeParticlesPin)) {
        // check if the timestamp has been initialized first, otherwise disregard the change
        if (_largeParticlesTimestamp > 0) {
            _largeParticleOnTime += millis() - _largeParticlesTimestamp;
        }
    }
    else {
        _largeParticlesTimestamp = millis();
    }
}

void SM_PWM_01::getConcentrations(float* smallParticlesConcentration, float* largeParticlesConcentration)
{
    uint32_t currentMillis = millis();

    // _lastCalculationTimestamp in milliseconds to have smaller probability 
    // of overflowing in the middle of a measurement
    float period = currentMillis - _lastCalculationTimestamp;
    _lastCalculationTimestamp = currentMillis;

    noInterrupts();

    // finish up any pending measurements
    if (!digitalRead(_smallParticlesPin)) {
        _smallParticleOnTime += currentMillis - _smallParticlesTimestamp;
    }

    if (!digitalRead(_largeParticlesPin)) {
        _largeParticleOnTime += currentMillis - _largeParticlesTimestamp;
    }

    float smallRatioPercent = (float)_smallParticleOnTime / (float)period * 100.0f;
    float largeRatioPercent = (float)_largeParticleOnTime / (float)period * 100.0f;

    _smallParticleOnTime = 0;
    _largeParticleOnTime = 0;

    _smallParticlesTimestamp = 0;
    _largeParticlesTimestamp = 0;

    interrupts();

    // small particles
    *smallParticlesConcentration = calculateConcentration(smallRatioPercent);
    debugPrintLn("Small Particles: ", smallRatioPercent, "%\t ", *smallParticlesConcentration, "pcs/ft^3");

    // large particles
    *largeParticlesConcentration = calculateConcentration(largeRatioPercent);
    debugPrintLn("Large Particles: ", largeRatioPercent, "%\t ", *largeParticlesConcentration, "pcs/ft^3");
}

bool SM_PWM_01::canRead()
{
    return (_lastPowerOnTimestamp > 0) &&
        (millis() - _lastPowerOnTimestamp >= PREHEAT_SECONDS * 1000);
}

/*
 * Calculates the concentration in pcs/ft^3 according to the datasheet curve.
 */
float SM_PWM_01::calculateConcentration(float ratioPercent)
{
    return (1.1 * pow(ratioPercent, 3) - 3.8 * pow(ratioPercent, 2) + 520.0 * ratioPercent) * 100.0;
}


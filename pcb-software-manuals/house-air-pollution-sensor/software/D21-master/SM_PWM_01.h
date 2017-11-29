#ifndef _SM_PWM_01_h
#define _SM_PWM_01_h

#include "Arduino.h"

// static class
// Note: Timestamps throughout this class are assumed to be ticks (millis or micros) 
// since system power-up (not seconds from RTC)
class SM_PWM_01
{
public:
    static void init(const uint8_t powerPin, const uint8_t smallParticlesPin, const uint8_t largeParticlesPin);
    static void disable();
    static void setPower(bool on);
    static void getConcentrations(float* concentrationSmall, float* concentrationLarge);
    static bool canRead();
private:
    static uint8_t _powerPin;
    static uint8_t _smallParticlesPin;
    static uint8_t _largeParticlesPin;

    static volatile uint32_t _smallParticlesTimestamp;
    static volatile uint32_t _largeParticlesTimestamp;

    static volatile uint32_t _smallParticleOnTime;
    static volatile uint32_t _largeParticleOnTime;

    static uint32_t _lastCalculationTimestamp;
    static uint32_t _lastPowerOnTimestamp;

    static float calculateConcentration(float ratio);
    static void smallParticlePinChangeCallback();
    static void largeParticlePinChangeCallback();
};

#endif

#ifndef HAPAGGREGATOR_H_
#define HAPAGGREGATOR_H_

#include <Arduino.h>
#include "SensorDataRecord.h"

class HapAggregator
{
public:
    void init();
    void reset();
    void addCoReading(uint16_t value);
    void addCo2Reading(uint16_t value);
    void addPmReading(float smallParticlesConcentration, float largeParticlesConcentration);
    void addBatteryLevel(uint16_t value);

    void fillRecord(SensorDataRecord& record, uint32_t timestamp) const;
private:
    uint16_t coMax;

    uint16_t co2Max;

    float smallParticlesConcentrationSum;
    uint16_t smallParticlesConcentrationCount;
    float largeParticlesConcentrationSum;
    uint16_t largeParticlesConcentrationCount;
    
    uint32_t  batteryLevelSum;
    uint16_t  batteryLevelCount;

    template <typename T>
    uint16_t getAverage(T sum, uint16_t count) const;
};

extern HapAggregator hapAggregator;

#endif /* HAPAGGREGATOR_H_ */

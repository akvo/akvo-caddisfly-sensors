#include "HapAggregator.h"
#include "Utils.h"

void HapAggregator::init()
{
    coMax = 0;
    co2Max = 0;
    smallParticlesConcentrationSum = 0;
    smallParticlesConcentrationCount = 0;
    largeParticlesConcentrationSum = 0;
    largeParticlesConcentrationCount = 0;
    batteryLevelSum = 0;
    batteryLevelCount = 0;
}

void HapAggregator::reset()
{
    coMax = 0;
    co2Max = 0;
    
    smallParticlesConcentrationSum = getAverage(smallParticlesConcentrationSum, smallParticlesConcentrationCount);
    smallParticlesConcentrationCount = 1;
    largeParticlesConcentrationSum = getAverage(largeParticlesConcentrationSum, largeParticlesConcentrationCount);
    largeParticlesConcentrationCount = 1;
    
    batteryLevelSum = getAverage(batteryLevelSum, batteryLevelCount);
    batteryLevelCount = 1;
}

void HapAggregator::addCoReading(uint16_t value)
{
    if (value > coMax) {
        coMax = value;
    }
}

void HapAggregator::addCo2Reading(uint16_t value)
{
    if (value > co2Max) {
        co2Max = value;
    }
}

void HapAggregator::addPmReading(float smallParticlesConcentration, float largeParticlesConcentration)
{
    smallParticlesConcentrationSum += smallParticlesConcentration;
    smallParticlesConcentrationCount++;

    largeParticlesConcentrationSum += largeParticlesConcentration;
    largeParticlesConcentrationCount++;
}

void HapAggregator::addBatteryLevel(uint16_t value)
{
    batteryLevelSum += value;
    batteryLevelCount++;
}

template <typename T>
uint16_t HapAggregator::getAverage(T sum, uint16_t count) const
{
    return count > 0 ? sum / count : 0;
}

void HapAggregator::fillRecord(SensorDataRecord& record, uint32_t timestamp) const
{
    record.setTimestamp(timestamp);

    record.setCO(coMax);
    record.setCO2(co2Max);
    record.setP1(getAverage(smallParticlesConcentrationSum, smallParticlesConcentrationCount));
    record.setP2(getAverage(largeParticlesConcentrationSum, largeParticlesConcentrationCount));
    record.setBatteryLevel(getCompactBatteryLevel(getAverage(batteryLevelSum, batteryLevelCount)));
}

HapAggregator hapAggregator;
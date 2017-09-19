#ifndef SUMNOIDDATARECORD_H_
#define SUMNOIDDATARECORD_H_

#include <Arduino.h>
#include <stdint.h>
#include "DataRecord.h"
#include "MacroSum.h"
#include "MacroCount.h"

//    uint32_t StartTimestamp;
//    uint8_t Duration;
//    uint16_t MaxTemperature;
//    uint8_t BatteryLevel;

#define SUM_NO_ID_DATA_RECORD_HEADER "StartTime, Duration, MaxTemperature, BatteryLevel"

#define SUM_NO_ID_DATA_FIELD_SIZES sizeof(uint32_t), \
                                   sizeof(uint8_t), \
                                   sizeof(uint16_t), \
                                   sizeof(uint8_t)

#define SUM_NO_ID_DATA_BUFFER_SIZE (SUM(SUM_NO_ID_DATA_FIELD_SIZES))
#define SUM_NO_ID_DATA_FIELD_COUNT (COUNT(SUM_NO_ID_DATA_FIELD_SIZES))

class SumNoIdDataRecord : public DataRecord
{
public:
    SumNoIdDataRecord() { }
    ~SumNoIdDataRecord() { }

    void init();
    bool isValid() const;
    uint16_t getSize() const { return SUM_NO_ID_DATA_BUFFER_SIZE; }
    uint8_t getFieldCount() const { return SUM_NO_ID_DATA_FIELD_COUNT; }
    uint8_t* getBuffer() const { return (uint8_t*)buffer; }

    void printHeaderLn(Stream* stream) const { if (stream) { stream->println(SUM_NO_ID_DATA_RECORD_HEADER); } }
    void printRecordLn(Stream* stream, const char* separator = DATA_RECORD_SEPARATOR) const;

    // field getter/setters
    uint32_t getStartTimestamp() const { return getFieldValue<uint32_t>(StartTimestamp); }
    void setStartTimestamp(uint32_t value) const { setFieldValue(StartTimestamp, value); }

    uint8_t getDuration() const { return getFieldValue<uint8_t>(Duration); }
    void setDuration(uint8_t value) const { setFieldValue(Duration, value); }

    uint16_t getMaxTemperature() const { return getFieldValue<uint16_t>(MaxTemperature); }
    void setMaxTemperature(uint16_t value) const { setFieldValue(MaxTemperature, value); }

    uint8_t getBatteryLevel() const { return getFieldValue<uint8_t>(BatteryLevel); }
    void setBatteryLevel(uint8_t value) const { setFieldValue(BatteryLevel, value); }
protected:
    uint8_t getFieldSize(uint8_t fieldIndex) const;
private:
    uint8_t buffer[SUM_NO_ID_DATA_BUFFER_SIZE];
    static const uint8_t* fieldSizes;

    enum Fields { StartTimestamp, Duration, MaxTemperature, BatteryLevel };
};

#endif /* SUMNOIDDATARECORD_H_ */


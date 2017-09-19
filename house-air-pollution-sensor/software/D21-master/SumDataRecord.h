#ifndef SUMDATARECORD_H_
#define SUMDATARECORD_H_

#include <Arduino.h>
#include <stdint.h>
#include "DataRecord.h"
#include "MacroSum.h"
#include "MacroCount.h"

//    uint32_t StartTimestamp;
//    uint8_t DeviceId[8];
//    uint8_t Duration;
//    uint16_t MaxTemperature;
//    uint8_t BatteryLevel;

#define SUM_DATA_HEADER_MAGIC "SUM RECORDS" // Max 19 characters (20 with \0)
#define SUM_DATA_VERSION 1

#define SUM_DATA_RECORD_HEADER "StartTime, DeviceId, Duration, MaxTemperature, BatteryLevel"

#define SUM_DATA_FIELD_SIZES sizeof(uint32_t), \
                             8, \
                             sizeof(uint8_t), \
                             sizeof(uint16_t), \
                             sizeof(uint8_t)

#define SUM_DATA_BUFFER_SIZE (SUM(SUM_DATA_FIELD_SIZES))
#define SUM_DATA_FIELD_COUNT (COUNT(SUM_DATA_FIELD_SIZES))

class SumDataRecord : public DataRecord
{
public:
    SumDataRecord() { }
    ~SumDataRecord() { }

    void init();
    bool isValid() const;
    uint16_t getSize() const { return SUM_DATA_BUFFER_SIZE; }
    uint8_t getFieldCount() const { return SUM_DATA_FIELD_COUNT; }
    uint8_t* getBuffer() const { return (uint8_t*)buffer; }

    void printHeaderLn(Stream* stream) const { if (stream) { stream->println(SUM_DATA_RECORD_HEADER); } }
    void printRecordLn(Stream* stream, const char* separator = DATA_RECORD_SEPARATOR) const;

    // field getter/setters
    uint32_t getStartTimestamp() const { return getFieldValue<uint32_t>(StartTimestamp); }
    void setStartTimestamp(uint32_t value) const { setFieldValue(StartTimestamp, value); }

    void getDeviceId(uint8_t deviceId[8]) const { getFieldValue(DeviceId, deviceId, 8); }
    void setDeviceId(uint8_t deviceId[8]) const { setFieldValue(DeviceId, deviceId, 8); }

    uint8_t getDuration() const { return getFieldValue<uint8_t>(Duration); }
    void setDuration(uint8_t value) const { setFieldValue(Duration, value); }

    uint16_t getMaxTemperature() const { return getFieldValue<uint16_t>(MaxTemperature); }
    void setMaxTemperature(uint16_t value) const { setFieldValue(MaxTemperature, value); }

    uint8_t getBatteryLevel() const { return getFieldValue<uint8_t>(BatteryLevel); }
    void setBatteryLevel(uint8_t value) const { setFieldValue(BatteryLevel, value); }
protected:
    uint8_t getFieldSize(uint8_t fieldIndex) const;
private:
    uint8_t buffer[SUM_DATA_BUFFER_SIZE];
    static const uint8_t* fieldSizes;

    enum Fields { StartTimestamp, DeviceId, Duration, MaxTemperature, BatteryLevel };
};

#endif /* SUMDATARECORD_H_ */


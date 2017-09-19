#ifndef SENSORDATARECORD_H_
#define SENSORDATARECORD_H_

#include <Arduino.h>
#include <stdint.h>
#include "DataRecord.h"
#include "MacroSum.h"
#include "MacroCount.h"

//    uint32_t Timestamp;
//    uint16_t CO;
//    uint16_t CO2;
//    uint16_t P1;
//    uint16_t P2;
//    uint8_t BatteryLevel;

#define SENSOR_DATA_HEADER_MAGIC "SENSOR RECORDS" // Max 19 characters (20 with \0)
#define SENSOR_DATA_VERSION 1

#define SENSOR_DATA_RECORD_HEADER "Timestamp, CO, CO2, P1, P2, BatteryLevel"

#define SENSOR_DATA_FIELD_SIZES sizeof(uint32_t), \
                                sizeof(uint16_t), \
                                sizeof(uint16_t), \
                                sizeof(uint16_t), \
                                sizeof(uint16_t), \
                                sizeof(uint8_t)

#define SENSOR_DATA_BUFFER_SIZE (SUM(SENSOR_DATA_FIELD_SIZES))
#define SENSOR_DATA_FIELD_COUNT (COUNT(SENSOR_DATA_FIELD_SIZES))

class SensorDataRecord: public DataRecord
{
public:
    SensorDataRecord() { }
    ~SensorDataRecord() { }

    void init();
    bool isValid() const;
    uint16_t getSize() const { return SENSOR_DATA_BUFFER_SIZE; }
    uint8_t getFieldCount() const { return SENSOR_DATA_FIELD_COUNT; }
    uint8_t* getBuffer() const { return (uint8_t*)buffer; }

    void printHeaderLn(Stream* stream) const { if (stream) { stream->println(SENSOR_DATA_RECORD_HEADER); } }
    void printRecordLn(Stream* stream, const char* separator = DATA_RECORD_SEPARATOR) const;

    // field getter/setters
    uint32_t getTimestamp() const { return getFieldValue<uint32_t>(Timestamp); }
    void setTimestamp(uint32_t value) const { setFieldValue(Timestamp, value); }

    uint16_t getCO() const { return getFieldValue<uint16_t>(CO); }
    void setCO(uint16_t value) const { setFieldValue(CO, value); }

    uint16_t getCO2() const { return getFieldValue<uint16_t>(CO2); }
    void setCO2(uint16_t value) const { setFieldValue(CO2, value); }

    uint16_t getP1() const { return getFieldValue<uint16_t>(P1); }
    void setP1(uint16_t value) const { setFieldValue(P1, value); }

    uint16_t getP2() const { return getFieldValue<uint16_t>(P2); }
    void setP2(uint16_t value) const { setFieldValue(P2, value); }

    uint8_t getBatteryLevel() const { return getFieldValue<uint8_t>(BatteryLevel); }
    void setBatteryLevel(uint8_t value) const { setFieldValue(BatteryLevel, value); }
protected:
    uint8_t getFieldSize(uint8_t fieldIndex) const;
private:
    uint8_t buffer[SENSOR_DATA_BUFFER_SIZE];
    static const uint8_t* fieldSizes;

    enum Fields { Timestamp, CO, CO2, P1, P2, BatteryLevel };
};

#endif /* SENSORDATARECORD_H_ */


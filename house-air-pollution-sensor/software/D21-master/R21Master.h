#ifndef _R21MASTER_h
#define _R21MASTER_h

#include "Arduino.h"
#include <SPI.h>

struct R21Record
{
    uint32_t Timestamp;
    uint8_t DeviceId[8];
    uint16_t MaxTemperature;
    uint16_t Duration;
    uint16_t BatteryLevel;
    uint32_t Crc;
};

#define R21RECORD_SIZE sizeof(uint32_t) + \
                       8 + \
                       sizeof(uint16_t) + \
                       sizeof(uint16_t) + \
                       sizeof(uint16_t) + \
                       sizeof(uint32_t)

class R21MasterClass
{
private:
    uint8_t _ssPin;
    SPIClass* _spi;
    SPISettings _defaultSpiSettings;

    void selectSlave() const;
    void deselectSlave() const;

    void readBuffer(uint8_t* buffer, size_t size) const;
    void writeBuffer(const uint8_t* buffer, size_t size) const;

    static void bufferToR21Record(uint8_t* buffer, R21Record* record);
    static void r21RecordToBuffer(R21Record* record, uint8_t* buffer);
    static uint32_t calculateCRC32(R21Record* record);
public:
    R21MasterClass();
	void init(SPIClass* spi, uint8_t ssPin, uint32_t now);
    void setRTC(uint32_t epoch) const;
    uint32_t getRTC() const;
    uint16_t getAvailableRecordCount() const;
    bool getNextRecord(R21Record* record) const;
    void deleteRecord() const;
};

extern R21MasterClass R21Master;

#endif

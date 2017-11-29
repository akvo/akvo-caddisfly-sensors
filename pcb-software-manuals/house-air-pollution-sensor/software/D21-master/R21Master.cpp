#include "R21Master.h"
#include "crc32.h"
#include "PrintSystem.h"

#define CMD_SET_RTC 0xD1
#define CMD_GET_RTC 0xD2
#define CMD_GET_AVAILABLE_RECORD_COUNT 0xE0
#define CMD_GET_RECORD 0xEE
#define CMD_DELETE_RECORD 0XF0

void R21MasterClass::selectSlave() const
{
    _spi->beginTransaction(_defaultSpiSettings);
    digitalWrite(_ssPin, LOW);
}

void R21MasterClass::deselectSlave() const
{
    digitalWrite(_ssPin, HIGH);
    _spi->endTransaction();
}

void R21MasterClass::readBuffer(uint8_t* buffer, size_t size) const
{
    _spi->transfer(0x00); // start transfer

    for (size_t i = 0; i < size; i++) {
        uint8_t cc = _spi->transfer(0x00);
        buffer[i] = cc;
    }
}

void R21MasterClass::writeBuffer(const uint8_t* buffer, size_t size) const
{
    for (size_t i = 0; i < size; i++) {
        _spi->transfer(buffer[i]);
    }
}

R21MasterClass::R21MasterClass(): _ssPin(0), _spi(0), _defaultSpiSettings(10000, MSBFIRST, SPI_MODE0)
{
}

void R21MasterClass::init(SPIClass* spi, uint8_t ssPin, uint32_t now)
{
    _spi = spi;
    _ssPin = ssPin;

    pinMode(_ssPin, OUTPUT);
    deselectSlave();
    delay(25);
    setRTC(now);
}

void R21MasterClass::setRTC(uint32_t epoch) const
{
    debugPrintLn("R21MasterClass::setRTC(", epoch, ")");

    selectSlave();
    delay(20); // give time to the slave to detect being selected

    _spi->transfer(CMD_SET_RTC);
    delay(20);

    uint8_t parts[4];
    memcpy(parts, &epoch, sizeof(parts));
    writeBuffer(parts, sizeof(parts));

    delay(250);
    deselectSlave();
}

uint32_t R21MasterClass::getRTC() const
{
    debugPrintLn("R21MasterClass::getRTC()");

    uint32_t result = 0;

    selectSlave();
    delay(20); // give time to the slave to detect being selected

    _spi->transfer(CMD_GET_RTC);
    delay(20);

    uint8_t parts[4];
    readBuffer(parts, sizeof(parts));
    memcpy(&result, parts, sizeof(result));

    deselectSlave();

    debugPrintLn("R21MasterClass::getRTC() returns ", result);
    return result;
}

uint16_t R21MasterClass::getAvailableRecordCount() const
{
    debugPrintLn("R21MasterClass::getAvailableRecordCount()");

    uint16_t result = 0;

    selectSlave();
    delay(20); // give time to the slave to detect being selected

    _spi->transfer(CMD_GET_AVAILABLE_RECORD_COUNT);
    delay(20);

    uint8_t parts[2];
    readBuffer(parts, sizeof(parts));
    memcpy(&result, parts, sizeof(result));

    deselectSlave();

    return result;
}

bool R21MasterClass::getNextRecord(R21Record* record) const
{
    debugPrintLn("R21MasterClass::getNextRecord()");

    selectSlave();
    delay(20); // give time to the slave to detect being selected

    _spi->transfer(CMD_GET_RECORD);
    delay(100);

    uint8_t rxBuffer[R21RECORD_SIZE];
    readBuffer(rxBuffer, sizeof(rxBuffer));
    bufferToR21Record(rxBuffer, record);
    delay(20);

    deselectSlave();

    return (record->Crc == calculateCRC32(record));
}

void R21MasterClass::deleteRecord() const
{
    debugPrintLn("R21MasterClass::deleteRecord()");

    selectSlave();
    delay(20); // give time to the slave to detect being selected

    _spi->transfer(CMD_DELETE_RECORD);
    delay(20);

    deselectSlave();
}

void R21MasterClass::bufferToR21Record(uint8_t* buffer, R21Record* record)
{
    uint8_t index = 0;
    uint8_t size = sizeof(record->Timestamp);
    memcpy(&record->Timestamp, &buffer[index], size);

    index += size;
    size = sizeof(record->DeviceId);
    memcpy(record->DeviceId, &buffer[index], size);

    index += size;
    size = sizeof(record->MaxTemperature);
    memcpy(&record->MaxTemperature, &buffer[index], size);

    index += size;
    size = sizeof(record->Duration);
    memcpy(&record->Duration, &buffer[index], size);

    index += size;
    size = sizeof(record->BatteryLevel);
    memcpy(&record->BatteryLevel, &buffer[index], size);

    index += size;
    size = sizeof(record->Crc);
    memcpy(&record->Crc, &buffer[index], size);
}

void R21MasterClass::r21RecordToBuffer(R21Record* record, uint8_t* buffer)
{
    uint8_t index = 0;
    uint8_t size = sizeof(record->Timestamp);
    memcpy(&buffer[index], &record->Timestamp, size);

    index += size;
    size = sizeof(record->DeviceId);
    memcpy(&buffer[index], record->DeviceId, size);

    index += size;
    size = sizeof(record->MaxTemperature);
    memcpy(&buffer[index], &record->MaxTemperature, size);

    index += size;
    size = sizeof(record->Duration);
    memcpy(&buffer[index], &record->Duration, size);

    index += size;
    size = sizeof(record->BatteryLevel);
    memcpy(&buffer[index], &record->BatteryLevel, size);

    index += size;
    size = sizeof(record->Crc);
    memcpy(&buffer[index], &record->Crc, size);
}

uint32_t R21MasterClass::calculateCRC32(R21Record* record)
{
    crc32_t crc32;
    uint8_t recordBuffer[R21RECORD_SIZE];

    R21Record recordCopy = *record;
    recordCopy.Crc = 0;

    r21RecordToBuffer(&recordCopy, recordBuffer);
    crc32_calculate(recordBuffer, sizeof(recordBuffer), &crc32);

    return crc32;
}

R21MasterClass R21Master;


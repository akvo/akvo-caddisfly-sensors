#include "HapStorage.h"

#include "ProjectDefinitions.h"
#include "PrintSystem.h"

// Macro to be used as a pseudo-method at the beginning of each (non-function) method. Among others, 
// this also guarantees the rtc is initialized before use.
#define checkInitialized() { if (!isInitialized) { debugPrintLn("HapStorage is not initialized!"); return; } }

/*
 * Initializes the storage system (dataflash, segments, helpers)
 */
void HapStorage::init(RTCZero& rtc)
{
    this->rtc = &rtc;

    dflash.init(DATAFLASH_SS_PIN);
#ifdef DEBUG_STREAM
    dflash.setDiagStream(DEBUG_STREAM);
#endif

    debugPrintLn("Dataflash page count = ", dflash.nrPages());
    debugPrintLn("Dataflash page size = ", dflash.pageSize());

    currentSumPageIndex = -1;
    memset(currentSumPageDeviceId, 0, 8);

    // init segments
    configSegment.init(dflash, DFlashBuffer::DFBuf1, DF_SEG_CONFIG_START, DF_SEG_CONFIG_COUNT);
    sumDataSegment.init(dflash, DFlashBuffer::DFBuf1, DF_SEG_SUM_DATA_START, DF_SEG_SUM_DATA_COUNT);
    sensorDataSegment.init(dflash, DFlashBuffer::DFBuf2, DF_SEG_SENSOR_DATA_START, DF_SEG_SENSOR_DATA_COUNT);

    // init curPage and uploadPage for each segment
    sumDataHelper.init(this->rtc->getEpoch(), sumDataSegment, SUM_DATA_HEADER_MAGIC, SUM_DATA_VERSION);
    sensorDataHelper.init(this->rtc->getEpoch(), sensorDataSegment, SENSOR_DATA_HEADER_MAGIC, SENSOR_DATA_VERSION);

    isInitialized = true;
    debugPrintLn("HapStorage initialization finished!");
}

void HapStorage::clearSumRecords()
{
    checkInitialized();

    debugPrintLn("Clearing SUM records...");
    sumDataHelper.reset(rtc->getEpoch());
    debugPrintLn("Finished clearing SUM records!");
}

void HapStorage::clearSensorRecords()
{
    checkInitialized();

    debugPrintLn("Clearing Sensor records...");
    sensorDataHelper.reset(rtc->getEpoch());
    debugPrintLn("Finished clearing Sensor records!");
}

void HapStorage::addSumRecord(SumDataRecord& record)
{
    checkInitialized();

    if (!record.isValid()) {
        debugPrintLn("[HapStorage::addSumRecord]: SUM record is not valid!");
        return;
    }

    uint8_t recordDeviceId[8];
    record.getDeviceId(recordDeviceId);

    // if the record doesn't have the same device id as the previous one,
    // and the page is checked to be the same as before,
    // change the page to the next available
    if ((currentSumPageIndex != -1) 
            && (currentSumPageIndex == sumDataHelper.getCurPage())
            && (memcmp(currentSumPageDeviceId, recordDeviceId, 8) != 0)) {
        debugPrintLn("Same SUM page, different Device Id, changing page...");
        sumDataHelper.newCurPage(rtc->getEpoch());
    }

    sumDataHelper.addCurPageRecord(record.getBuffer(), record.getSize(), this->rtc->getEpoch());

    // update the members
    currentSumPageIndex = sumDataHelper.getCurPage();
    record.getDeviceId(currentSumPageDeviceId);
}

void HapStorage::addSensorRecord(SensorDataRecord& record)
{
    checkInitialized();

    if (!record.isValid()) {
        debugPrintLn("[HapStorage::addSensorRecord]: Sensor record is not valid!");
        return;
    }

    sensorDataHelper.addCurPageRecord(record.getBuffer(), record.getSize(), this->rtc->getEpoch());
}

DFlashSegment& HapStorage::getConfigSegment()
{
    return configSegment;
}

DataflashUtils& HapStorage::getSumDataHelper()
{
    return sumDataHelper;
}

DataflashUtils& HapStorage::getSensorDataHelper()
{
    return sensorDataHelper;
}

HapStorage::HapStorage(): rtc(0), isInitialized(false)
{
}

HapStorage hapStorage;

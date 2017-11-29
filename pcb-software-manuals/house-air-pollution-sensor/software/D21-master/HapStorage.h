#ifndef HAPSTORAGE_H_
#define HAPSTORAGE_H_

#include "RTCZero.h"
#include "DataflashUtils.h"
#include "Sodaq_segmented_df.h"
#include "SumDataRecord.h"
#include "SensorDataRecord.h"

class HapStorage
{
public:
    HapStorage();

    void init(RTCZero& rtc);
    
    void clearSumRecords();
    void clearSensorRecords();

    void addSumRecord(SumDataRecord& record);
    void addSensorRecord(SensorDataRecord& record);

    DFlashSegment& getConfigSegment();

    DataflashUtils& getSumDataHelper();
    DataflashUtils& getSensorDataHelper();

private:
    RTCZero* rtc;
    bool isInitialized;

    int currentSumPageIndex; // used for checking if the page has changed
    uint8_t currentSumPageDeviceId[8]; // used for checking if the device id is different and a page change is needed

    DF_AT45DB161D dflash;

    // segments
    DFlashSegment configSegment;
    DFlashSegment sumDataSegment;
    DFlashSegment sensorDataSegment;

    // dataflash helpers (current page, upload page etc)
    DataflashUtils sumDataHelper;
    DataflashUtils sensorDataHelper;
};

extern HapStorage hapStorage;

#endif /* HAPSTORAGE_H_ */

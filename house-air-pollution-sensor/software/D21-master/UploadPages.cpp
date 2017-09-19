/*
* Based on UploadPages.h by Gabriel Notman
*/

#include <Arduino.h>
#include "ProjectDefinitions.h"
#include "Config.h"
#include "DeviceID.h"
#include "DataflashUtils.h"
#include "DataRecord.h"
#include "SPUL.h"
#include "UploadPages.h"
#include "Utils.h"
#include "Sodaq_wdt.h"
#include "SumNoIdDataRecord.h"

/*
 * Returns how many pages were uploaded or -1 if there was an error while trying.
 */
int16_t uploadPages(DataflashUtils& dflashUtils, DataRecord& record)
{
    int16_t pageCount = 0;

    uint16_t recordSize = record.getSize();
    if (dflashUtils.isEmpty(recordSize)) {
        debugPrintLn("No records to upload!");

        return 0;
    }

    debugPrintLn("Uploading data...");

    int16_t page = dflashUtils.getUploadPage();
    bool uploadOK = true;

    int8_t socket = openSPULSocket(
        params.getAPN(),
        params.getAPNuser(),
        params.getAPNpassword(),
        params.getSPULserver(),
        params.getSPULport());

    if (socket != -1) {
        while ((page != -1) && (uploadOK) && (pageCount < MAX_PAGE_UPLOADS)) {
            sodaq_wdt_reset();

            SpulFrame_t frame;
            DeviceId::get(frame.deviceID);

            frame.recordCount = 0;
            frame.recordLength = recordSize;
            setSPULNetworkQuality(frame);

            while (dflashUtils.readPageNthRecord(page, frame.recordCount,
                                                 &frame.payload[frame.recordCount * recordSize],
                                                 recordSize)) {
                frame.recordCount++;
            }

            uint16_t len = frame.recordCount * recordSize;

            debugPrintLn("Uploading page: ", page);
            debugPrintLn("Payload size: ", len, ", ", frame.recordCount, " records");

            // If the page is blank erase it
            if (len == 0) {
                debugPrintLn("Page is blank...");
                dflashUtils.eraseUploadPage(getNow());
            }
            else { // else try and upload it
                pageCount++;
                uploadOK = sendSPULFrame(socket, frame, len);

                if (uploadOK) {
                    debugPrintLn("Page upload succeeded!");
                    dflashUtils.eraseUploadPage(getNow());
                }
                else {
                    debugPrintLn("Page upload failed!");
                }
            }

            // Get the next page to upload
            page = dflashUtils.getUploadPage();
        }
    }
    else {
        // Failed to open socket
        pageCount = -1;
    }

    closeSPULSocket(socket);

    return pageCount;
}

/*
 * Takes SUM records from the flash, transforms them into SPUL-SUM records 
 * (no device id in the field, but only in the header) and sends them.
*/
int16_t uploadSumRecords(DataflashUtils& dflashUtils, SumDataRecord& flashRecord)
{
    int16_t pageCount = 0;

    if (dflashUtils.isEmpty(flashRecord.getSize())) {
        debugPrintLn("No records to upload!");

        return 0;
    }

    debugPrintLn("Uploading data...");

    int16_t page = dflashUtils.getUploadPage();
    bool uploadOK = true;

    int8_t socket = openSPULSocket(
        params.getAPN(),
        params.getAPNuser(),
        params.getAPNpassword(),
        params.getSPULserver(),
        params.getSPULport());

    SumNoIdDataRecord spulRecord;
    uint8_t currentRecordSumDeviceId[8];
    
    if (socket != -1) {
        while ((page != -1) && (uploadOK) && (pageCount < MAX_PAGE_UPLOADS)) {
            sodaq_wdt_reset();

            debugPrintLn("Starting filling in new SPUL packet buffer...");
            SpulFrame_t frame;
            bool isDeviceIdInitialized = false;

            frame.recordCount = 0;
            frame.recordLength = spulRecord.getSize();
            setSPULNetworkQuality(frame);

            while (dflashUtils.readPageNthRecord(page, frame.recordCount, flashRecord.getBuffer(), flashRecord.getSize())) {
                if (!isDeviceIdInitialized) {
                    flashRecord.getDeviceId(frame.deviceID);

                    isDeviceIdInitialized = true;
                }
                
                // test the assumption that a page contains only records from the same device id
                flashRecord.getDeviceId(currentRecordSumDeviceId);
                if (memcmp(currentRecordSumDeviceId, frame.deviceID, sizeof(currentRecordSumDeviceId)) !=0 ) {
                    debugPrintLn("A different device id was found in the same page! Skipping the remaining records...", page);
                    break;
                }

                spulRecord.init();
                spulRecord.setBatteryLevel(flashRecord.getBatteryLevel());
                spulRecord.setDuration(flashRecord.getDuration());
                spulRecord.setMaxTemperature(flashRecord.getMaxTemperature());
                spulRecord.setStartTimestamp(flashRecord.getStartTimestamp());

                spulRecord.copyTo(&frame.payload[frame.recordCount * spulRecord.getSize()], spulRecord.getSize());
                
                frame.recordCount++;
            }

            uint16_t len = frame.recordCount * spulRecord.getSize();

            debugPrintLn("Uploading page: ", page);
            debugPrintLn("Payload size: ", len, ", ", frame.recordCount, " records");

            // If the page is blank erase it
            if (len == 0) {
                debugPrintLn("Page is blank...");
                dflashUtils.eraseUploadPage(getNow());
            }
            else { // else try and upload it
                pageCount++;
                uploadOK = sendSPULFrame(socket, frame, len);

                if (uploadOK) {
                    debugPrintLn("Page upload succeeded!");
                    dflashUtils.eraseUploadPage(getNow());
                }
                else {
                    debugPrintLn("Page upload failed!");
                }
            }

            // Get the next page to upload
            page = dflashUtils.getUploadPage();
        }
    }
    else {
        // Failed to open socket
        pageCount = -1;
    }

    closeSPULSocket(socket);

    return pageCount;
}


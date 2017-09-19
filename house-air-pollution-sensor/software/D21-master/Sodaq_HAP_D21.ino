#include <Arduino.h>
#include <SPI.h>

#include "RTCTimer.h"
#include "RTCZero.h"
#include "Sodaq_3Gbee.h"
#include "Sodaq_wdt.h"

#include "ProjectDefinitions.h"
#include "DiagPrint.h"
#include "R21Master.h"
#include "SM_PWM_01.h"
#include "MQ7.h"
#include "DeviceId.h"
#include "version.h"
#include "Config.h"
#include "BootUpCommands.h"
#include "HapStorage.h"
#include "SumDataRecord.h"
#include "SensorDataRecord.h"
#include "PrintSystem.h"
#include "Utils.h"
#include "HapAggregator.h"
#include "SPUL.h"
#include "UploadPages.h"


#define RESET_BY_WDT(...) { sodaq_wdt_enable(); while (true) { } }
#define RESET_BY_SOFTWARE(...) { NVIC_SystemReset(); while (true) { } }


enum RecordTypes
{
    SensorRecordType,
    SumRecordType
};

RTCTimer timer;
RTCZero rtc;

static uint8_t lastResetCause;
static uint32_t sumLastSeenTimestamp;
static bool isInitialized;
static bool isPMSensorReady; // used to make sure first measurement is thrown away
static bool shouldReuploadData;
static bool areSensorsInitialized;


void setup();
void loop();
static void printCpuResetCause(Stream& stream);
static void printDeviceId(Stream& stream);
static void printBootUpMessage(Stream& stream);
uint32_t getNow();
void initSerialConnections();
void initSensors();
void disableSensors();
void initNetworkComponents();
void initRtc();
void initR21Slave();
void initRtcTimer();
void handleBootUpCommands();
void handleR21Records(uint32_t now);
void readSensors(uint32_t now);
void debugPrintR21Record(const R21Record* record);
void debugPrintSensorRecord(SensorDataRecord& record);
bool isR21RtcInSync();
bool syncRtc();
bool syncR21Rtc();
void syncAllRtc(uint32_t now);
bool uploadData(RecordTypes recordType);
void uploadAllData(uint32_t now);
void handleSensorPower(uint32_t now);
void aggregateSensorData(uint32_t now);
bool isBatteryLevelEnoughForModem();

void setup()
{
    lastResetCause = PM->RCAUSE.reg;

    sodaq_wdt_disable();

    // init LEDs
    pinMode(LED1_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);
    pinMode(LED3_PIN, OUTPUT);
    SET_LED(LED1_PIN, false);
    SET_LED(LED2_PIN, false);
    SET_LED(LED3_PIN, false);

    initSerialConnections();

    delay(STARTUP_DELAY); // wdt not enabled yet
    printBootUpMessage(CONSOLE_STREAM);

    rtc.begin();

    SPI.begin();

    hapStorage.init(rtc);

    handleBootUpCommands();

    initNetworkComponents();

    initRtc(); // does not return unless successful

    initR21Slave(); // does not return unless successful

    initSensors();

    hapAggregator.init();

    initRtcTimer();

    isInitialized = true;

    debugPrintLn("** Boot-up completed successfully!");

    if (static_cast<Stream*>(printSystem.getDebug()) != static_cast<Stream*>(&SerialUSB)) {
        SerialUSB.println("USB serial will now be disconnected.");
        delay(3000);
        SerialUSB.end();
        USBDevice.detach();
    }

    sodaq_wdt_enable();
}

void loop()
{
    sodaq_wdt_reset();
    sodaq_wdt_flag = false;

    if (isInitialized) {
        timer.update();
    }
}

/*
 * Initializes the serial connections.
 * (console and debug if applicable)
 */
void initSerialConnections()
{
    CONSOLE_STREAM.begin(CONSOLE_BAUDRATE);

#ifdef DEBUG_STREAM
    if (DEBUG_STREAM != CONSOLE_STREAM) {
        DEBUG_STREAM.begin(DEBUG_BAUDRATE);
    }
#endif

    // now initialize the utility systems
    printSystem.setConsole(CONSOLE_STREAM);
#ifdef DEBUG_STREAM
    printSystem.setDebug(DEBUG_STREAM);
    setDiagStream(DEBUG_STREAM);
#endif
}

/*
 * Prints the cause of the last reset to the given stream.
 *
 * It uses the PM->RCAUSE register to detect the cause of the last reset.
 */
static void printCpuResetCause(Stream& stream)
{
    stream.print("CPU reset by");

    if (PM->RCAUSE.bit.SYST) {
        stream.print(" Software");
    }

    // Syntax error due to #define WDT in CMSIS/4.0.0-atmel/Device/ATMEL/samd21/include/samd21j18a.h
    // if (PM->RCAUSE.bit.WDT) {
    if ((PM->RCAUSE.reg & PM_RCAUSE_WDT) != 0) {
        stream.print(" Watchdog");
    }

    if (PM->RCAUSE.bit.EXT) {
        stream.print(" External");
    }

    if (PM->RCAUSE.bit.BOD33) {
        stream.print(" BOD33");
    }

    if (PM->RCAUSE.bit.BOD12) {
        stream.print(" BOD12");
    }

    if (PM->RCAUSE.bit.POR) {
        stream.print(" Power On Reset");
    }

    stream.print(" [");
    stream.print(PM->RCAUSE.reg);
    stream.println("]");
}

/*
 * Prints the device id.
 */
static void printDeviceId(Stream& stream)
{
    stream.print("Device ID: ");
    DeviceId::printTo(stream);
    stream.println();
}

/*
 * Prints a boot-up message that includes project name, version,
 * Cpu reset cause, Cpu device ID.
 */
static void printBootUpMessage(Stream& stream)
{
    stream.println("** " PROJECT_NAME " - " VERSION " **");

    stream.print(" -> ");
    printCpuResetCause(stream);

    stream.print(" -> ");
    printDeviceId(stream);

    stream.println();
}

/*
 * Returns the current datetime (seconds since unix epoch).
 */
uint32_t getNow()
{
    return rtc.getEpoch();
}

/*
 * Initializes the sensors.
 * Sets areSensorsInitialized = true.
 */
void initSensors()
{
    sodaq_wdt_reset();

    if (areSensorsInitialized) {
        debugPrintLn("Sensors are already on, skipping initSensors()...");
        return;
    }

    debugPrintLn("Initializing the sensors...");

    pinMode(BOOST_REGULATOR_ENABLE_PIN, OUTPUT);

    // turn on boost regulator
    digitalWrite(BOOST_REGULATOR_ENABLE_PIN, true);

    // init CO sensor
    analogReference(AR_DEFAULT);
    mq7.init(CO_ANALOG_PIN, CO_POWER_PIN);

    // init PM sensor
    SM_PWM_01::init(PM_POWER_PIN, PM_P1_PIN, PM_P2_PIN);
    SM_PWM_01::setPower(true);

    areSensorsInitialized = true;
}

/*
 * Turns off the sensors.
 * Sets areSensorsInitialized = false.
*/
void disableSensors()
{
    sodaq_wdt_reset();

    // turn off CO sensor
    mq7.disable();

    // turn off PM sensor
    SM_PWM_01::disable();
    isPMSensorReady = false;

    // turn off boost regulator
    digitalWrite(BOOST_REGULATOR_ENABLE_PIN, false);

    areSensorsInitialized = false;
}

/*
 * Initializes the Real Time Clock with the time from the server.
 * Does not return unless the clock has been synced.
*/
void initRtc()
{
    uint8_t failureCounter = 0;

    while (!syncRtc()) {
        failureCounter++;
        if (failureCounter > 6) {
            debugPrintLn("Failed to sync the RTC with the server. Will reset now.");
            RESET_BY_SOFTWARE();
        }

        uint16_t waitMillis = pow(2, failureCounter) * 1000; // ms
        debugPrintLn("Failed to sync the RTC with the server. Will retry in ", waitMillis, "ms");
        delay(waitMillis); // wdt not enabled yet
    }
}

/*
* Initializes the R21 Slave and makes sure its RTC is synced correctly.
* Does not return unless the clock has been synced successfully.
*/
void initR21Slave()
{
    // init R21Master with timestamp (after syncRTC) to make sure that once the slave 
    // is up, it has the correct clock to provide to the SUM
    uint8_t failureCounter = 0;
    R21Master.init(&SPI, R21_SS_PIN, getNow());
    while (!isR21RtcInSync()) {
        failureCounter++;
        if (failureCounter > 6) {
            debugPrintLn("R21Master: Failed to sync RTC! Will reset now.");
            RESET_BY_SOFTWARE();
        }

        uint16_t waitMillis = pow(2, failureCounter) * 1000; // ms
        debugPrintLn("R21Master: Failed to sync RTC! Will retry in ", waitMillis, "ms");
        delay(waitMillis); // wdt not enabled yet
        syncR21Rtc();
    }
}

/*
 * Initializes the modem and network protocols.
 */
void initNetworkComponents()
{
    debugPrintLn("Initializing the network...");

    MODEM_STREAM.begin(MODEM_BAUDRATE);
    sodaq_wdt_safe_delay(50);
#if defined(DEBUG_STREAM)
    sodaq_3gbee.setDiag(DEBUG_STREAM);
#endif

    sodaq_3gbee.init_wdt(MODEM_STREAM, MODEM_POWER_PIN);
    sodaq_3gbee.setPSDAuth(PAT_PAP);
    sodaq_3gbee.setFlushEverySend();
    
    sodaq_3gbee.off();

    setupSPULModem(sodaq_3gbee);
}

/*
 * Reads all the sensors (the ones that are ready for use each time),
 * and passes the values to the aggregators.
*/
void readSensors(uint32_t now)
{
    sodaq_wdt_reset();

    if (!areSensorsInitialized) {
        debugPrintLn("Sensors are off, skipping readSensors()...");
        return;
    }

    debugPrintLn("Reading the sensors...");

    // CO sensor
    if (mq7.canRead()) {
        uint16_t co = mq7.read();
        debugPrintLn("CO: ", co);

        hapAggregator.addCoReading(co);
    }

    // PM sensor
    if (SM_PWM_01::canRead()) {
        float small, large;
        SM_PWM_01::getConcentrations(&small, &large);

        if (isPMSensorReady) {
            hapAggregator.addPmReading(small, large);
        }
        else {
            // the first measurement was just taken, the sensor is ready
            isPMSensorReady = true;
        }
    }
    else {
        // make sure the first measurement is thrown away when the sensor 
        // becomes able to read again
        isPMSensorReady = false;
    }

    // Battery level
    hapAggregator.addBatteryLevel(getBatteryVoltage());
}

void handleSensorPower(uint32_t now)
{
    if (!areSensorsInitialized) {
        debugPrintLn("Sensors are off, skipping handleSensorPower()...");
        return;
    }

    // allow the sensor to adjust its voltage
    mq7.powerCycleHandler();
}

/*
 * Initializes the RTC Timer.
 */
void initRtcTimer()
{
    debugPrintLn("Initializing the RTC timer...");

    timer.setNowCallback(getNow); // set how to get the current time
    timer.allowMultipleEvents();

    // handles power cycling etc for the sensors
    timer.every(10, handleSensorPower);

    // read sensors (PM, CO etc)
    timer.every(20, readSensors);

    // read available SUM data from R21 slave
    timer.every(60, handleR21Records);

    // Schedule the data aggregation
    timer.every(params.getAggregationInterval() * 60, aggregateSensorData);

    // Schedule the upload
    timer.every(params.getUploadInterval() * 60, uploadAllData);

    // Schedule the RTC Sync
    timer.every(params.getRtcSyncInterval() * 60, syncAllRtc);

    // One time events
    timer.every(2 * 60, uploadAllData, 1); // Schedule an initial upload session after first 2 minutes
}

/*
 * Shows and handles the boot up commands.
 */
void handleBootUpCommands()
{
    DFlashSegment& configSegment = hapStorage.getConfigSegment();

    params.read(configSegment);

    uint8_t count = 0;
    do {
        startupCommands(CONSOLE_STREAM);
        count++;
    } while ((!params.checkConfig()) || (count < MIN_STARTUP_PROMPTS));

    params.dump();
    params.commit(configSegment);
}

/*
 * Handles reading any pending records from R21.
 */
void handleR21Records(uint32_t now)
{
    debugPrintLn("Requesting records from R21...");

    uint16_t availableRecords = R21Master.getAvailableRecordCount();

    uint8_t recordsRead = 0;
    while ((availableRecords > 0) && (recordsRead < MAX_R21RECORDS_TO_READ_AT_A_TIME)) {
        sodaq_wdt_reset();

        recordsRead++;
        debugPrintLn("Available R21 Records: ", availableRecords);

        R21Record r21Record;
        if (R21Master.getNextRecord(&r21Record)) {
            debugPrintR21Record(&r21Record);

            // check whether it is a status report (only add record if not status report)
            if ((r21Record.Duration == 0) && (r21Record.MaxTemperature == 0)) {
                // keep when SUM was seen
                if (r21Record.Timestamp == 0) {
                    // use local timestamp then
                    sumLastSeenTimestamp = getNow();
                }
                else {
                    sumLastSeenTimestamp = max(sumLastSeenTimestamp, r21Record.Timestamp);
                }
                debugPrintLn("SUM just sent out a status report.");
            }
            else {
                SumDataRecord sumRecord;
                sumRecord.init();

                sumRecord.setDeviceId(r21Record.DeviceId);
                sumRecord.setBatteryLevel(getCompactBatteryLevel(r21Record.BatteryLevel));
                sumRecord.setStartTimestamp(r21Record.Timestamp);
                sumRecord.setDuration(r21Record.Duration / 60); // R21 record in sec, SUM record in min
                sumRecord.setMaxTemperature(r21Record.MaxTemperature);

                hapStorage.addSumRecord(sumRecord);
            }

            R21Master.deleteRecord();
        }
        else {
            debugPrintLn("Received an R21 record with CRC mismatch!");
        }

        availableRecords = R21Master.getAvailableRecordCount();
    }

    if (sumLastSeenTimestamp > 0) {
        debugPrintLn("SUM was seen for the last time at ", sumLastSeenTimestamp);
    }
    else {
        debugPrintLn("SUM not seen!");
    }
}

/*
 * Prints an R21 record to the debug stream.
 */
void debugPrintR21Record(const R21Record* record)
{
    debugPrint("Device ID: ");
    for (uint8_t i = 0; i < 8; i++) {
        debugPrint(record->DeviceId[i], HEX);
        if (i < 7) {
            debugPrint(":");
        }
    }
    debugPrintLn("");

    debugPrintLn("Battery Level: ", record->BatteryLevel);
    debugPrintLn("Cooking Start Timestamp: ", record->Timestamp);
    debugPrintLn("Duration: ", record->Duration);
    debugPrintLn("Max Temperature: ", record->MaxTemperature);
    debugPrintLn("CRC: ", record->Crc);
    debugPrintLn("");
}

void debugPrintSensorRecord(SensorDataRecord& record)
{
    if (printSystem.getDebug()) {
        record.printHeaderLn(printSystem.getDebug());
        record.printRecordLn(printSystem.getDebug());
    }
}

/*
 * Returns true if the R21 RTC is within +- 1s the D21 RTC
 */
bool isR21RtcInSync()
{
    uint32_t r21Timestamp = R21Master.getRTC();
    uint32_t timestamp = getNow();
    debugPrintLn("isR21RtcInSync: ", r21Timestamp, " vs ", timestamp);

    return (isWithin(r21Timestamp, timestamp, 2));
}

/*
 * Syncs the RTC with the server. Low battery level leads to failure.
 * Returns true if successful.
 */
bool syncRtc()
{
    if (!isBatteryLevelEnoughForModem()) {
        return false;
    }

    bool enableSensors = areSensorsInitialized;
    disableSensors();

    bool result = false;

    debugPrintLn("Syncing timestamp...");

    uint32_t timeStamp = getSPULTimeStamp(
        params.getAPN(),
        params.getAPNuser(),
        params.getAPNpassword(),
        params.getSPULserver(),
        params.getTimePort());

    if (timeStamp != 0) {
        debugPrintLn("Setting new timestamp: ", timeStamp);
        rtc.setEpoch(timeStamp);
        result = true;
    }
    else {
        debugPrintLn("Timestamp sync failed!");
    }

    sodaq_3gbee.off(); // make sure it's off

    if (enableSensors) {
        initSensors();
    }

    return result;
}

/*
* Syncs the R21's RTC with the local RTC.
* Returns true if successful.
*/
bool syncR21Rtc()
{
    R21Master.setRTC(getNow());

    return isR21RtcInSync();
}

/*
 * Returns true if the battery level is greater than MIN_BATTERY_LEVEL. To be used 
 * when the modem is needed to work.
*/
bool isBatteryLevelEnoughForModem()
{
    uint16_t batteryLevel = getBatteryVoltage();

    if (batteryLevel < MIN_BATTERY_LEVEL) {
        debugPrintLn("Battery level is too low for using the modem!");
        debugPrintLn("Required level: ", MIN_BATTERY_LEVEL, ", Current level: ", batteryLevel);

        return false;
    }

    return true;
}

/*
* Uploads the given type of records. Low battery level leads to failure.
* Returns true if successful.
*/
bool uploadData(RecordTypes recordType)
{
    if (!isBatteryLevelEnoughForModem()) {
        return false;
    }

    bool enableSensors = areSensorsInitialized;
    disableSensors();

    bool result = false;

    int16_t uploadedPagesCount = 0;
    if (recordType == SumRecordType) {
        SumDataRecord sumRecord;
        uploadedPagesCount = uploadSumRecords(hapStorage.getSumDataHelper(), sumRecord);
    }
    else {
        SensorDataRecord sensorRecord;
        uploadedPagesCount = uploadPages(hapStorage.getSensorDataHelper(), sensorRecord);
    }

    if (uploadedPagesCount >= 0) {
        result = true;
    }

    sodaq_3gbee.off(); // make sure it's off

    if (enableSensors) {
        initSensors();
    }

    return result;
}

/*
 * Uploads both SumDataRecords and SensorDataRecords and handles re-upload if needed.
*/
void uploadAllData(uint32_t now)
{
    bool isReuploadRequest = shouldReuploadData;
    shouldReuploadData = false; // reset the flash

    bool sensorDataUploadResult = uploadData(SensorRecordType);
    bool sumDataUploadResult = uploadData(SumRecordType);

    if (!sensorDataUploadResult || !sumDataUploadResult) {
        if (!isReuploadRequest) {
            // schedule a single reupload attempt
            uint16_t uploadPeriod = params.getUploadInterval();
            uploadPeriod *= 60 / 2; // Half way to the next regular upload attempt

            // Subtract the time taken in the upload attempt
            uint16_t offset = static_cast<uint16_t>(getNow() - now);

            // make sure the reupload point is not in the past
            if (offset < uploadPeriod) {
                uploadPeriod -= offset;
            }

            timer.every(uploadPeriod, uploadAllData, 1);
            shouldReuploadData = true;
            debugPrintLn("Failed uploading and scheduled a re-upload in ", uploadPeriod, "s...");
        }
        else {
            // give up
            debugPrintLn("Failed re-uploading data and giving up!");
        }
    }
    else {
        debugPrintLn("All types of records were uploaded successfully!");
    }
}

/*
* Syncs the RTC with the server and if successful syncs the R21 RTC.
*/
void syncAllRtc(uint32_t now)
{
    if (syncRtc()) {
        syncR21Rtc();
    }
}

/*
 * Uses the data from the HapAggregator and saves a Sensor record to the flash.
*/
void aggregateSensorData(uint32_t now)
{
    SensorDataRecord record;
    record.init();

    hapAggregator.fillRecord(record, getNow());
    hapStorage.addSensorRecord(record);
    debugPrintSensorRecord(record);
    hapAggregator.reset();
}


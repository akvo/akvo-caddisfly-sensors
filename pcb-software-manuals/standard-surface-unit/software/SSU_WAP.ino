/*
 * Copyright 2016, M2M4ALL
 *
 * This program controls the VOS Station.
 * ...
 */

#include <SPI.h>
#include <Arduino.h>
#include <Wire.h>


// Standard libraries that we have copied into our project
#include "RTCTimer.h"
#include "RTCZero.h"
#include "Sodaq_3Gbee.h"
#include "Sodaq_MQTT.h"
#include "Sodaq_wdt.h"
#include "SparkFunBME280.h"

#include "Config.h"
#include "DataRecord.h"
#include "DeviceID.h"
#include "DiagStream.h"
#include "MyMQTT.h"
#include "Settings.h"
#include "StartupCommands.h"
#include "TickTimer.h"
#include "Utils.h"
#include "version.h"

// Serial ports
#define CONSOLE_STREAM SERIAL_PORT_MONITOR
#if defined(ARDUINO_SODAQ_AUTONOMO)
#define DIAG_STREAM Serial
#elif defined(ARDUINO_SODAQ_SSU)
#define DIAG_STREAM Serial
#else
#error "Please select Autonomo"
#endif


//RTC Timer
static RTCTimer timer;

// RTC Zero
static RTCZero rtc;

// Flag for one-minute interrupts
static volatile bool minute_flag = false;

// BME280 sensor
BME280 bme280;

// Last cause of reset
static uint8_t last_reset_cause;

static bool diag_enabled = false;

String deviceID;

//######### show pressure value #############

static const char txtPressCommands[] = "Pressure values:";
static void showPressureValues(Stream * stream);

// Forward declarations
static void uploadSensors(uint32_t ts);

static void setupSleep();
static void systemSleep();

void fatal();
void flashLED(size_t count = 1);
float getBatteryVoltage(int ADC_res);

static void setupHardware();
static void setupSensors();

static void setupCommunication();
static void setupWAPCommunication();
static void showModemDetails(bool switchOff = true);
//static void showConnectionDetails();
static bool doHTTPGET(const char * server, uint16_t port, const String & uri, char * result, size_t result_len);

bool getWapData(int16_t &PressureWap, int16_t &TempWap, bool show_diag = true);
bool readPressureWap(const String & str, int16_t & pres);
bool readTempWap(const String & str, int16_t & temp);

static void setupRTC();
static void handleRTCAlarm();
static void setupScheduling();
static uint32_t getNow();
static void updateRTC();
static String getDateTimeIso();
void syncRTC(uint32_t now);

static void myStartupCommands();

static void showResetCause(Stream & stream);
static void showStartupBanner(Stream & stream);
static void startDiagStream(Uart * ser, unsigned long baudrate);
static void stopDiagStream(Uart * ser);

double getPressure(int ADC_res);

#define diagPrint(...) do { if (diag_stream.isEnabled()) diag_stream.print(__VA_ARGS__); } while(0)
#define diagPrintLn(...) do { if (diag_stream.isEnabled()) diag_stream.println(__VA_ARGS__); } while (0)
#define diagDumpBuffer(buf, len) do { if (diag_stream.isEnabled()) diag_stream.dumpBuffer(buf, len); } while (0)

void setup()
{
    last_reset_cause = PM->RCAUSE.reg;

    // In case of reset (this is probably unnecessary)
    sodaq_wdt_disable();

    // Basic setup hardware, pins, etc
    setupHardware();

    delay(STARTUP_DELAY);

    startDiagStream(&DIAG_STREAM, 57600);
    CONSOLE_STREAM.begin(57600);
    diag_stream.setConsoleStream(CONSOLE_STREAM);

    showStartupBanner(diag_stream);
    showResetCause(diag_stream);

    addDeviceIDToString(deviceID);

    // Initialise sensors
    setupSensors();

    setupWAPCommunication();

    // Read the configuration
    params.read();

    if (!params.getEnableDiag()) {
        diag_stream.println("Diag will be disabled during config");
        stopDiagStream(&DIAG_STREAM);
    }

    // Handle startup commands
    myStartupCommands();

    // Re-enable "normal" diag mode
    startDiagStream(&DIAG_STREAM, 57600);
    
    // Setup communications module
    setupCommunication();
    // This switches on the 3Gbee
    // If you want to leave it on for the next syncRTC you must pass the switchOff=false flag
    showModemDetails(true);

    // Setup the RTC
    setupRTC();
    
    // Sync RTC
    while (true) {
        syncRTC(0);
        if (getNow() >= TIMESTAMP_TOO_EARLY) {
            //diagPrintLn(String("RTC ") + getDateTime(rtc));
            break;
        }
        sodaq_wdt_safe_delay(60000);
    }
    
    // Enable Watch Dog Timer
    sodaq_wdt_enable(WDT_PERIOD_8X);
    
    // Update RTC
    updateRTC();

    // Setup scheduling
    setupScheduling();

    setupTickTimer();

    diag_stream.println("Program start");

    if (!params.getKeepUSB()) {
        // Switch off USB if it is not the diag port
        SerialUSB.println("USB serial will be disconnected");
        sodaq_wdt_safe_delay(3000);
        SerialUSB.end();
        USBDevice.detach();
        diag_stream.setConsoleStream(0);
    }

    if (!params.getEnableDiag()) {
        diag_stream.println("Diag will now be disabled");
        stopDiagStream(&DIAG_STREAM);
    }

    // Setup sleep mode
    setupSleep();

    // Enable interrupts (if it hadn't already been enabled)
    interrupts();
}

void loop()
{
    if (tick_flag) {
        tick_flag = false;

        timer.update();

        flashLED();
    }

    if (sodaq_wdt_flag) {
        // The WDT is triggered every second
        sodaq_wdt_reset();
        sodaq_wdt_flag = false;
    }

    // Force diag off before going to sleep
    stopDiagStream(&DIAG_STREAM);

    systemSleep();

    if (params.getEnableDiag()) {
        startDiagStream(&DIAG_STREAM, 57600);
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////   UPLOAD THE SENSOR DATA  /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static void uploadSensors(uint32_t ts)
{
    diagPrintLn(String("uploadSensors: ") + getDateTimeIso());

    DataRecord_t rec;
    rec.init();

    String tmp;
    addDeviceIDToString(tmp);
    strncpy(rec.devid, tmp.c_str(), sizeof(rec.devid) - 1);

    // Always read BME280 temp first

    // Temperature in 0.1 C
    rec.Temp = (int16_t)(bme280.readTempC() * 10);

    // Pressure in milliBar
    rec.OutSidePressure = (uint16_t)(bme280.readFloatPressure() / 100);

    // Get the data from the WAP
    int16_t PressureWap = -999;
    int16_t TempWap = -999;
    // try a few times
    for (int8_t i = 0; i < 3; i++) {
        if (i > 0) {
            diagPrintLn(String("WAP reading retry number: ") + i);
        }
        if (getWapData(PressureWap, TempWap, true)) {
            break;
        }
    }
    rec.WAPPressure = PressureWap + params.getWapPressOffset();
    rec.WAPTemperature = TempWap;

    /*
     * Battery reading is only possible when the UBlox is switched on.
     * The UBlox will be switched off after the MQTT PUBLISH.
     */
    sodaq_3gbee.on();

    rec.lipoBattery = (uint16_t)(getBatteryVoltage(MY_ADC_RESOLUTION) * 1000);

    String msg;

    rec.getRecordString(msg);

    String topic = params.getMQTTtopic();
    // topic += "/" + deviceID;
    diagPrintLn(String("uploadSensors: topic ") + topic);
    diagPrintLn(String("uploadSensors: msg ") + msg);

    publishMQTT(topic, msg);
}

////////////////////////////////////////////////////////////////////////////////
////////////////   SETUP HARDWARE AND SENSORS  /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static void setupHardware()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    //init PressurePreInjector sensor
    digitalWrite(PRESS_SW_PIN, LOW);
    pinMode(PRESS_SW_PIN, OUTPUT);

#if 0
    // FIXME What is this?
    digitalWrite(BEE_VCC, LOW);
    pinMode(BEE_VCC, OUTPUT);
#endif

}

static void setupSensors()
{
    analogReadResolution(MY_ADC_RESOLUTION);

    // Initialise the wire protocol for the TPH / BME sensors
    Wire.begin();

    // Initialise the BME sensor
    // For I2C, enable the following and disable the SPI section
    bme280.settings.commInterface = I2C_MODE;

    //***Operation settings*****************************//

    //renMode can be:
    //  0, Sleep mode
    //  1 or 2, Forced mode
    //  3, Normal mode
    bme280.settings.runMode = 3; //Normal mode

    //tStandby can be:
    //  0, 0.5ms
    //  1, 62.5ms
    //  2, 125ms
    //  3, 250ms
    //  4, 500ms
    //  5, 1000ms
    //  6, 10ms
    //  7, 20ms
    bme280.settings.tStandby = 0;

    //filter can be off or number of FIR coefficients to use:
    //  0, filter off
    //  1, coefficients = 2
    //  2, coefficients = 4
    //  3, coefficients = 8
    //  4, coefficients = 16
    bme280.settings.filter = 0;

    //tempOverSample can be:
    //  0, skipped
    //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
    bme280.settings.tempOverSample = 1;

    //pressOverSample can be:
    //  0, skipped
    //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
    bme280.settings.pressOverSample = 1;

    //humidOverSample can be:
    //  0, skipped
    //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
    bme280.settings.humidOverSample = 1;

    bme280.settings.I2CAddress = 0x76;               // !! Not the default 0x77
    bme280.reset();
    sodaq_wdt_safe_delay(1000);
    uint8_t bmp_chip_id = bme280.begin();
    // 0x60 is the chip identification number of the BME280
    if (bmp_chip_id == 0x60) {
        diagPrintLn("INFO: Detected BME280 at 0x76");
    } else {
        bme280.settings.I2CAddress = 0x77;        // Try the old BMP185
        bme280.reset();
        sodaq_wdt_safe_delay(1000);
        bmp_chip_id = bme280.begin();
        if (bmp_chip_id == 0x60) {
            // Yes, it's the old one.
            diagPrintLn("INFO: Detected BME280 at 0x77");
        } else {
            // BME280 not found
            diagPrintLn("INFO: No BME280 detected");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////   MODEM  //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static void setupCommunication()
{
    // Start Serial1 the Bee port
    BEE_SERIAL.begin(BEE_BAUD);
#if defined(ARDUINO_SODAQ_AUTONOMO)
    sodaq_3gbee.init(BEE_SERIAL, BEE_VCC, BEEDTR, BEECTS);
#elif defined(ARDUINO_SODAQ_SSU)
    sodaq_3gbee.init_wdt(BEE_SERIAL, BEEDTR);
#endif
    sodaq_3gbee.setPSDAuth(PAT_PAP);

    sodaq_3gbee.setFlushEverySend();

    sodaq_3gbee.setApn(params.getAPN());
    sodaq_3gbee.setApnUser(params.getAPNuser());
    sodaq_3gbee.setApnPass(params.getAPNpassword());

    // Make sure the modem is off
    sodaq_3gbee.off();

    mqtt.setDiag(diag_stream);

    String clientId = params.getStationName();
    clientId += "/" + deviceID;
    setupMQTT(sodaq_3gbee, clientId);
}

/*!
 * Show the details of the modem (EMEI, IMSI, etc)
 *
 * Switch off diag for the modem because it will clobber the
 * output and you may miss the requested information.
 */
static void showModemDetails(bool switchOff)
{
    if (!sodaq_3gbee.on()) {
        // What can we do? It did not switch on.
        return;
    }

    bool diag_was_on = false;
    if (diag_enabled) {
        diag_was_on = true;
        sodaq_3gbee.setDiag(0);
    }
    char buf[64];
    if (sodaq_3gbee.getIMEI(buf, sizeof(buf))) {
        diagPrintLn(String("IMEI: ") + buf);
    }
    if (sodaq_3gbee.getIMSI(buf, sizeof(buf))) {
        diagPrintLn(String("IMSI: ") + buf);
    }
    if (sodaq_3gbee.getCCID(buf, sizeof(buf))) {
        diagPrintLn(String("CCID: ") + buf);
    }

    if (switchOff) {
        sodaq_3gbee.off();
    }

    if (diag_was_on) {
        sodaq_3gbee.setDiag(diag_stream);
    }
}



////////////////////////////////////////////////////////////////////////////////
////////////////   EXTRA 3GBEE FUNCTIONS  //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static bool doHTTPGET(const char * server, uint16_t port, const String & uri, char * result, size_t result_size)
{
    bool retval = false;
    memset(result, 0, result_size);
    if (sodaq_3gbee.connect()) {
        char buffer[200];
        if (sodaq_3gbee.httpRequest(server, port, uri.c_str(), GET, buffer, sizeof(buffer))) {
            // Strip trailing white space
            size_t len = strlen(buffer);
            bool done = false;
            while (!done && len > 0) {
                char c = buffer[len - 1];
                switch (c) {
                case ' ':
                case '\t':
                case '\r':
                case '\n':
                    --len;
                    buffer[len] = '\0';
                    break;
                default:
                    done = true;
                    break;
                }
            }
            // diagPrintLn(String("HTTP GET: ")); diagPrintLn(buffer);

            // Find start of line
            char *ptr = &buffer[len - 1];
            while (ptr > (buffer + 1)) {
                if (!isdigit(ptr[-1])) {
                    break;
                }
                --ptr;
            }
            // diagPrintLn(String("HTTP GET 2: ")); diagPrintLn(ptr);
            strncpy(result, ptr, result_size - 1);
            retval = true;
        }
    }
    sodaq_3gbee.off();
    return retval;
}

////////////////////////////////////////////////////////////////////////////////
////////////////   SETUP RTC AND TIMER  ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 * Initialize the RTC
 */
static void setupRTC()
{
    rtc.begin();

    // Schedule the wakeup interrupt for every minute
    // Alarm is triggered 1 cycle after match
    rtc.setAlarmSeconds(59);
    rtc.enableAlarm(RTCZero::MATCH_SS);   //Every Minute

    // Attach handler
    rtc.attachInterrupt(handleRTCAlarm);

    // This sets it to 2000-01-01
    rtc.setEpoch(0);
}

/*
 * Update the RTC from whatever source we have.
 */
static void updateRTC()
{
    //uint32_t ts;
    // TODO Print the time in iso format
    diagPrintLn(String("Date-time: ") + getDateTimeIso());
}

static void handleRTCAlarm()
{
    minute_flag = true;
}

/*
 * Add the ISO date and time to the string.
 */
static String getDateTimeIso()
{
    char buffer[50];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer) - 1,
            "%04d-%02d-%02d %02d:%02d:%02d",
            rtc.getYear() + 2000, rtc.getMonth(), rtc.getDay(),
            rtc.getHours(), rtc.getMinutes(), rtc.getSeconds());
    return buffer;
}

/*
 * A helper function to get the UTC from the RTC
 */
static uint32_t getNow()
{
  return rtc.getEpoch();
}

////////////////////////////////////////////////////////////////////////////////
////////////////   SYNC RTC  ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void syncRTC(uint32_t now)
{
    //DIAGPRINT(F("syncRTCwithServer ")); DIAGPRINTLN(now);

    String uri;
    uri = String("/?") + deviceID;
    if (last_reset_cause) {
        // Only send MCUSR the first time.
        uri += '&';
        uri += last_reset_cause;
        last_reset_cause = 0;
    }
    char buffer[20];
    if (doHTTPGET(TIMESRV, 80, uri, buffer, sizeof(buffer))) {
        diagPrintLn(String("HTTP GET: "));
        diagPrintLn(buffer);
        uint32_t newTs;
        if (getUValue(buffer, &newTs)) {
            // Tweak the timestamp a little because doHTTPGET took a few seconds
            // to close the connection after getting the time from the server
            newTs += 3;
            uint32_t oldTs = getNow();
            int32_t diffTs = labs(newTs - oldTs);
            if (diffTs > 4) {
                diagPrintLn(String("Updating RTC, old=") + oldTs + " new=" + newTs);
                if (now != 0) {
                    timer.adjust(oldTs, newTs);
                }
                rtc.setEpoch(newTs);
            }
        }
    }
    //doSystemCheck();
    //DIAGPRINTLN(F("syncRTCwithServer - end"));
}

////////////////////////////////////////////////////////////////////////////////
////////////////   SCHEDULING  /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 * Initialize the scheduling using RTCTimer
 *
 * RTCTimer can schedule tasks (if timer.update() is called from the main loop)
 */
static void setupScheduling()
{
    int8_t event_ix;

    // Instruct the RTCTimer how to get the current time reading
    timer.setNowCallback(getNow);

    // This is needed so that all events are handled within the same update() call
    timer.allowMultipleEvents();

    event_ix = timer.every((uint32_t)params.getSampleIntvl() * 60, uploadSensors);
    if (event_ix < 0) {
        // Panic
        fatal();
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////   STARTUP COMMANDS  ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static void myStartupCommands()
{
    params.addStartupCommands();
    supComRegisterCmnds(txtPressCommands,
        NULL, 0,
        showPressureValues,
        NULL
    );

    uint8_t count = 0;
    do {
      startupCommands(CONSOLE_STREAM);          // Use CONSOLE_STREAM, not diag_stream!
      count++;
    } while ((!params.checkConfig()) && (count < MAX_STARTUP_PROMPTS));

    params.dump(&diag_stream);
    params.commit();
}

////////////////////////////////////////////////////////////////////////////////
////////////////   STARTUP COMMANDS  ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static void showPressureValues(Stream * stream)
{
    int16_t wap_press;
    int16_t wap_temp;
    (void)getWapData(wap_press, wap_temp, false);       // false: Do not show diag

    // Always read BME280 temp first
    (void)bme280.readTempC();
    int16_t bme_press = (int16_t)(bme280.readFloatPressure() / 100);
    stream->println(String("WAP pressure: ") + wap_press);
    stream->println(String("BME280 pressure: ") + bme_press);
    if (wap_press > 0) {
        stream->println(String("WAP press offset: ") + (bme_press - wap_press));
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////   SLEEP MODE  /////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static void setupSleep()
{
    // Set the sleep mode
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
}

static void systemSleep()
{
    bool did_usb_switch_off = false;
    if (USB->DEVICE.FSMSTATUS.bit.FSMSTATE == USB_FSMSTATUS_FSMSTATE_SUSPEND_Val) {
        // Disable USB
        USBDevice.detach();
        did_usb_switch_off = true;
    }

    // In which cases do we NOT want to sleep?
    // * USB is ON
    if (!(USB->DEVICE.FSMSTATUS.bit.FSMSTATE == USB_FSMSTATUS_FSMSTATE_ON_Val)) {
        // Don't sleep if the timer_flag is set
        noInterrupts();
        if (!sodaq_wdt_flag) {
            interrupts();
            // SAMD sleep
            __WFI();
        }
        interrupts();
    }

    if (did_usb_switch_off) {
        // Enable USB and wait for resume if attached
        USBDevice.attach();
        USB->DEVICE.CTRLB.bit.UPRSM = 0x01u;
        while (USB->DEVICE.CTRLB.bit.UPRSM) {
            // Wait for ...
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////   UTILITY STUFF  //////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 * Fatal problem happened, cannot continue
 */
void fatal()
{
    // Wait here forever.
    sodaq_wdt_disable();
    while (true) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000);
        digitalWrite(LED_BUILTIN, LOW);
        delay(1000);
    }
}

/*
 * Flash the builtin led
 */
void flashLED(size_t count)
{
    const uint8_t led = LED_BUILTIN;
    do {
        digitalWrite(led, HIGH);
        delay(10);
        digitalWrite(led, LOW);
        --count;
        if (count > 0) {
            delay(50);
        }
    } while (count > 0);
}

/*
 * Convert a resolution to a valid value used by analogReadResolution
 */
static int getADCResolution(int res)
{
    if (res > 10) {
        res = 12;
    } else if (res > 8) {
        res = 10;
    } else {
        res = 8;
    }
    return res;
}

/*
 * Read the voltage of the Lipo battery
 *
 * The SODAQ boards have a voltage divider with two resistors (R1 and R2) between VCC and ground.
 * The middle point is connected to an analog port.
 */
float getBatteryVoltage(int ADC_res)
{
    ADC_res = getADCResolution(ADC_res);
    uint32_t value = analogRead(BATVOLTPIN);
    value *= (BATVOLT_R1 + BATVOLT_R2) / BATVOLT_R2;
    return (float)ADC_AREF * value / ((1 << ADC_res) - 1);
}

////////////////////////////////////////////////////////////////////////////////
////////////////   DIAG     ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 * Show the cause of the last reset
 *
 * It uses the PM->RCAUSE register to
 * detect the cause of the last reset
 */
static void showResetCause(Stream & stream)
{
    stream.println(String("Board reset by (") + PM->RCAUSE.reg + "):");
    if (PM->RCAUSE.bit.SYST) {
        stream.println(" Software");
    }
    // Syntax error due to #define WDT in CMSIS/4.0.0-atmel/Device/ATMEL/samd21/include/samd21j18a.h
    // if (PM->RCAUSE.bit.WDT) {
    if ((PM->RCAUSE.reg & PM_RCAUSE_WDT) != 0) {
        stream.println(" WDT");
    }
    if (PM->RCAUSE.bit.EXT) {
        stream.println(" External");
    }
    if (PM->RCAUSE.bit.BOD33) {
        stream.println(" BOD33");
    }
    if (PM->RCAUSE.bit.BOD12) {
        stream.println(" BOD12");
    }
    if (PM->RCAUSE.bit.POR) {
        stream.println(" Power On Reset");
    }
}

static void showStartupBanner(Stream & stream)
{
    stream.println();
    stream.println(PROGRAM_NAME " " VERSION);
}

static void startDiagStream(Uart * ser, unsigned long baudrate)
{
    // Don't initialize the diag stream if it was already enabled (as far as we know)
    if (!diag_enabled) {
        ser->begin(baudrate);
        diag_enabled = true;
        diag_stream.setStream(ser);
        sodaq_3gbee.setDiag(diag_stream);
        supComRegisterDiagPort(diag_stream);
    }
}

static void stopDiagStream(Uart * ser)
{
    if (diag_enabled) {
        // ArduinoCore is unsafe. Flush is disabled.
        // diag_stream.flush();
        delay(3); // ?? Is this enough
        diag_enabled = false;
        ser->end();
        diag_stream.setStream(0);
        sodaq_3gbee.setDiag(0);
        supComRegisterDiagPort(0);
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////   pressure sensor//////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


double getPressure(int ADC_res)
{
    ADC_res = getADCResolution(ADC_res);

    digitalWrite(PRESS_SW_PIN, PRESS_SW_ON);
    delay(20); // Warm up time of the sensor - from the datasheet

    const size_t count = 1;             // Make this higher to average several readings
    uint32_t sum = 0;
    for (size_t i = 0; i < count; i++) {
        sum += analogRead(PRESS_SENSOR_PIN);
        delay(2);
    }
    sum /= count;
    sum *= PRESS_SENSOR_DIVIDER_VALUE;


    digitalWrite(PRESS_SW_PIN, PRESS_SW_OFF);

    // Convert voltage to pressure in kPa
    double voltage = (double)ADC_AREF * sum / ((1 << ADC_res) - 1);
    return (voltage / ADC_AREF_Pressure_Sensor - 0.04) / 0.0012858;
}


////////////////////////////////////////////////////////////////////////////////
////////////////   DEBUG    ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



void PrintHex8(uint8_t *data, uint8_t length) // prints 8-bit data in hex with leading zeroes
{
    char tmp[16];
    for (int i = 0; i < length; i++) {
        sprintf(tmp, "%.2X", data[i]);
        //diagPrint(tmp); //diagPrint(" ");
    }
}

String PrintHex16(uint16_t *data, uint8_t length) // prints 16-bit data in hex with leading zeroes
{
    char tmp[16];
    for (int i = 0; i < length; i++) {
        sprintf(tmp, "%.4X", data[i]);
        //diagPrint(tmp); //diagPrint(" ");
    }
    return (String)tmp;
}

String PrintHex16_2(int16_t *data, uint8_t length) // prints 16-bit data in hex with leading zeroes
{
    char tmp[16];
    for (int i = 0; i < length; i++) {
        sprintf(tmp, "%.4X", data[i]);
        //diagPrintLn(tmp); //diagPrint(" ");
        //String temp = (String)tmp;
        //diagPrintLn((String)tmp);
        
    }
    return (String)tmp;
}

bool sendATcommand(const char* ATcommand, const char* expected_answer, unsigned int timeout)
{

    bool answer = false;
    uint8_t x = 0;
    char response[100];
    unsigned long previous;

    memset(response, '\0', 100);    // Initialize the string

    //delay(100);

    while (BEE_SERIAL.available() > 0) BEE_SERIAL.read();    // Clean the input buffer

    diagPrintLn(ATcommand);    // Send the AT command
    BEE_SERIAL.println(ATcommand);    // Send the AT command 
    //SerialUSB.println("here");

    x = 0;
    previous = millis();

    // this loop waits for the answer
    do {
        // if there are data in the UART input buffer, reads it and checks for the asnwer
        if (BEE_SERIAL.available() != 0) {
            response[x] = BEE_SERIAL.read();
            x++;
            // check if the desired answer is in the response of the module
            if (strstr(response, expected_answer) != NULL)
            {
                answer = 1;
            }
        }
        // Waits for the asnwer with time out
    } while ((answer == 0) && ((millis() - previous) < timeout));

    return answer;
}


bool sendATcommand1(const char* ATcommand, const char* expected_answer, unsigned int timeout, byte msg[], uint8_t msgSize)
{

    bool answer = false;
    uint8_t x = 0, i;
    char response[100];
    unsigned long previous;


    memset(response, '\0', 100);    // Initialize the string

    //delay(100);

    while (BEE_SERIAL.available() > 0) BEE_SERIAL.read();    // Clean the input buffer

    diagPrint(ATcommand);    // Send the AT command
    BEE_SERIAL.print(ATcommand);    // Send the AT command 

    for (i = 0; i<msgSize; i++) {
        diagPrint(msg[i], HEX);    // Send the AT command
    }
    diagPrintLn();

    for (i = 0; i<msgSize + 1; i++) {
        BEE_SERIAL.print(msg[i], HEX);    // Send the AT command   
    }
    BEE_SERIAL.println();
    //BEE.println(msg,HEX);    // Send the AT command 

    x = 0;
    previous = millis();

    // this loop waits for the answer
    do {
        // if there are data in the UART input buffer, reads it and checks for the asnwer
        if (BEE_SERIAL.available() != 0) {
            response[x] = BEE_SERIAL.read();
            x++;
            // check if the desired answer is in the response of the module
            if (strstr(response, expected_answer) != NULL)
            {
                answer = 1;
            }
        }
        // Waits for the asnwer with time out
    } while ((answer == 0) && ((millis() - previous) < timeout));

    return answer;
}


bool sendATcommand2(String ATcommand, const char* expected_answer, unsigned int timeout)
{

    bool answer = false;
    char response[100];


    memset(response, '\0', 100);    // Initialize the string

                                    //delay(100);

    while (BEE_SERIAL.available() > 0) BEE_SERIAL.read();    // Clean the input buffer

    diagPrintLn(ATcommand);    // Send the AT command
    BEE_SERIAL.println(ATcommand);    // Send the AT command 
    //BEE_SERIAL.println();

    //BEE.println(msg,HEX);    // Send the AT command 

    
    return answer;
}


static void setupWAPCommunication()
{
    diagPrintLn("WAP Communication setup...");
    WAP.begin(WAP_BAUD);
}

/*
 * The output of the WAP are lines with the following syntax:
 *  '$' '+' <digit>+ ',' '+' <digit>+ '*' <checksum> <CR> <LF>
 * It is very well possible that the '+' is in fact a sign. In other
 * words it could be a minus too.
 *
 * The first value is the pressure in units of 0.1 milliBar, or 10 Pa
 * The second value is the temperature in 0.01 Celsius
 *
 * Example:
 *   $+0000009992,+0000002164*6C
 */
bool getWapData(int16_t &PressureWap, int16_t &TempWap, bool show_diag)
{
    bool result = true;
    PressureWap = -999;
    TempWap = -999;

    //set timeout for request
    WAP.setTimeout(100);

    // Clear all previous input in the buffer
    while (WAP.available() > 0) {
        (void)WAP.read();
    }

    //set WAP power to HIGH.
    //It needs around 1 sec to send the first data.
    digitalWrite(A12, HIGH);
    for (int i = 0; i < 11; i++) {
        sodaq_wdt_reset();
        delay(100);
    }

    const int bSize = 50;
    char Buffer[bSize + 1];
    size_t ByteCount;

    ByteCount = -1;
    ByteCount = WAP.readBytesUntil('\n', Buffer, bSize);
    Buffer[ByteCount] = '\0';
    String str(Buffer);

    // First character must be '$'
    // TODO Validate checksum

    if (ByteCount  > 0) {
        if (show_diag) {
            diagPrintLn("---->New WAP Packet<----");
            diagPrintLn(Buffer);
        }
    }
    else {
        result = false;
    }

    if (!result) {
        if (show_diag) {
            diagPrintLn("problem reading the WAP");
        }
    }

    //set WAP power to LOW
    digitalWrite(A12, LOW);

    if (result) {
        int16_t t_pres;
        result = readPressureWap(str, t_pres);
        if (result) {
            PressureWap = t_pres / 10;
        }
    }

    if (result) {
        int16_t t_temp;
        result = readTempWap(str, t_temp);
        if (result) {
            TempWap = t_temp / 10;
        }
    }

    return result;
}

bool readPressureWap(const String & str, int16_t &pres)
{
    int8_t plus_ix = str.indexOf('+');
    int8_t comma_ix = str.indexOf(',');
    if (plus_ix < 0 || comma_ix < 0) {
        return false;
    }
    pres = str.substring(plus_ix, comma_ix).toInt();
    return true;
}

bool readTempWap(const String & str, int16_t &temp)
{
    int8_t comma_ix = str.indexOf(',');
    if (comma_ix < 0) {
        return -999;
    }
    int8_t plus_ix = str.indexOf('+', comma_ix);
    int8_t star_ix = str.indexOf('*');
    if (plus_ix < 0 || star_ix < 0) {
        return false;
    }
    temp = str.substring(plus_ix, star_ix).toInt();
    return true;
}


#if 0
/* Option 2 for reading the WAP: */
void foo()
{
    x = 0;
    previous = millis();

    // this loop waits for the answer
    do {
        // if there are data in the UART input buffer, reads it and checks for the asnwer
        if (BEE_SERIAL.available() != 0) {
            response[x] = BEE_SERIAL.read();
            x++;
            // check if the desired answer is in the response of the module
            if (strstr(response, expected_answer) != NULL)
            {
                answer = 1;
            }
            else {

            }

        }
        // Waits for the asnwer with time out
    }while ((answer == 0) && ((millis() - previous) < timeout));
}
#endif

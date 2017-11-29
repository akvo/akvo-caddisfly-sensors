#ifndef PROJECTDEFINITIONS_H_
#define PROJECTDEFINITIONS_H_

// ==== General
#define PROJECT_NAME "Household Air Polution Module (HAP)"

// Startup up delay so that the command prompts are not missed
#define STARTUP_DELAY 6 * 1000

// Minimum number of times to show startup commands
#define MIN_STARTUP_PROMPTS 1

// Maximum number of pages uploaded in one connection
#define MAX_PAGE_UPLOADS 16

// Minimum battery level needed to upload data (in mV)
#define MIN_BATTERY_LEVEL 3500

#define ADC_AREF 3.3f

#define BATVOLT_R1 4.7f
#define BATVOLT_R2 10.0f

// this is to make sure the firmware doesn't get stuck reading records
#define MAX_R21RECORDS_TO_READ_AT_A_TIME 16

// ==== Serial Connections
// Mandatory Streams
#define CONSOLE_STREAM SerialUSB
#define MODEM_STREAM Serial1
#define CO2_SENSOR_STREAM Serial2

// Optional Streams
#define DEBUG_STREAM SerialUSB

// Baudrates
// CONSOLE_BAUDRATE supersedes DEBUG_BAUDRATE if streams are the same
#define CONSOLE_BAUDRATE 57600
#define MODEM_BAUDRATE 57600
#define DEBUG_BAUDRATE 57600


// ==== DataFlash Segments
#define DF_SEG_CONFIG_START 0
#define DF_SEG_CONFIG_COUNT 1 // in pages

#define DF_SEG_SUM_DATA_START DF_SEG_CONFIG_COUNT
#define DF_SEG_SUM_DATA_COUNT 1024 // in pages

#define DF_SEG_SENSOR_DATA_START (DF_SEG_SUM_DATA_START + DF_SEG_SUM_DATA_COUNT)
#define DF_SEG_SENSOR_DATA_COUNT 2048 // in pages


// ==== Pins
#define BOOST_REGULATOR_ENABLE_PIN 38
#define R21_SS_PIN SS
#define DATAFLASH_SS_PIN SS_2
#define VBUS_DETECT_PIN 9
#define BATVOLT_PIN A13
#define BUZZER_PIN 10

// PM Sensor
#define PM_P1_PIN 22
#define PM_P2_PIN 21
#define PM_POWER_PIN 30

// CO Sensor
#define CO_ANALOG_PIN BAT_VOLT // 33
#define CO_POWER_PIN 39

// CO2 Sensor
#define CO2_ANALOG_PIN A10

// Modem
#define MODEM_POWER_PIN 5

// LEDs
#define LED1_PIN 6
#define LED2_PIN 7
#define LED3_PIN 8
#define SET_LED(led, on) digitalWrite(led, on)

// ==== Parameter Default Values
#define PARAM_AGGREGATION_INTERVAL (5L) // 5 mins
#define PARAM_UPLOAD_INTERVAL (60L) // 60 mins
#define PARAM_RTCSYNC_INTERVAL (24L * 60) // 24 hours


#endif /* PROJECTDEFINITIONS_H_ */

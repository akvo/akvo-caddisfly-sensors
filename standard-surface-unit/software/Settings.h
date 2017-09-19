/*
 * Copyright 2016, M2M4ALL
 *
 * This is part of the SSU WAP.
 * ...
 */

#ifndef _SETTINGS_H
#define _SETTINGS_H

// Delay at program startup.
#define STARTUP_DELAY   2000

// Timeout from the startup commands after this many second
#define TIME_FOR_STARTUP_COMMANDS  (60 * 1000)

// Maximum number of times to show startup commands
// After that it continuous anyway
#define MAX_STARTUP_PROMPTS 20


// These constants are used for reading the battery voltage
#define MY_ADC_RESOLUTION 10
#define ADC_AREF 3.3f
#define BATVOLTPIN UBLOX_SENSE
#define BATVOLT_R1 3.0f
#define BATVOLT_R2 10.0f


//pressure sensor
//Analog Pin
#define PRESS_SENSOR_PIN A2
//Switch Pin
#define PRESS_SW_PIN A7
//Misc
#define PRESS_SW_ON HIGH
#define PRESS_SW_OFF LOW
#define PRESS_SENSOR_DIVIDER_VALUE 3
#define ADC_AREF_Pressure_Sensor 5.0f


// Modem settings
#if defined(ARDUINO_SODAQ_AUTONOMO) || defined(ARDUINO_SODAQ_SSU)
#define BEE_SERIAL Serial3
#else
#error "Unsupported board"
#endif
#define BEE_BAUD 9600


// WAP RS485
#define WAP             Serial1
#define WAP_BAUD        4800


// These are default values for configuration items
#define PARAM_Sm        (60L)           //   X mins

// APN
#if 0
// Only during development / testing
#define DEFAULT_APN     "aerea.m2m.com"
#elif 1
#define DEFAULT_APN     "public4.m2minternet.com"
#endif

//############ time service ################
#define TIMESRV "time.sodaq.net"

// All timestamp before this are invalid. This is 2011-03-13 07:06:40
#define TIMESTAMP_TOO_EARLY     (1300000000L)


#endif

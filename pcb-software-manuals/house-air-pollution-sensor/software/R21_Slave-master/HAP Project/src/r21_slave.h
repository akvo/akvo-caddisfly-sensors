/*
 * r21_slave.h
 *
 * Created: 12/23/2015 12:20:13 AM
 *  Author: alex
 */ 


#ifndef R21_SLAVE_H_
#define R21_SLAVE_H_

#include <asf.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// SPI commands
#define CMD_SET_RTC 0xD1
#define CMD_GET_RTC 0xD2
#define CMD_GET_AVAILABLE_RECORD_COUNT 0xE0
#define CMD_GET_RECORD 0xEE
#define CMD_DELETE_RECORD 0XF0

// SS pin INT
#define SS_PIN PIN_PB03A_EIC_EXTINT3
#define SS_PINMUX PINMUX_PB03A_EIC_EXTINT3
#define SS_INT_LINE 3

typedef struct R21Record
{
	uint32_t Timestamp;
	uint8_t DeviceId[8];
	uint16_t MaxTemperature;
	uint16_t Duration;
	uint16_t BatteryLevel;
	uint32_t Crc;
} r21_record_t;

#define R21RECORD_SIZE sizeof(uint32_t) + \
                       8 + \
                       sizeof(uint16_t) + \
                       sizeof(uint16_t) + \
                       sizeof(uint16_t) + \
                       sizeof(uint32_t)

typedef uint32_t (*get_rtc_epoch_callback_ptr)(void);
typedef void (*set_rtc_epoch_callback_ptr)(uint32_t newEpoch);

uint16_t r21_slave_get_free_record_count(void);
bool r21_slave_add_record(r21_record_t* record);
uint16_t r21_slave_get_available_record_count(void);
void r21_slave_update_out_record_buffer_if_needed(void);
void r21_slave_delete_record(void);
bool r21_slave_get_record(r21_record_t* record);
void r21_slave_init(void);
void r21_slave_task(void);
void r21_slave_set_rtc_callbacks(get_rtc_epoch_callback_ptr getRTC, set_rtc_epoch_callback_ptr setRTC);

#endif /* R21_SLAVE_H_ */
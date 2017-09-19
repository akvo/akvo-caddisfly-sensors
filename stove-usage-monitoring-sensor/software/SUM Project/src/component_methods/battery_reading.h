/*
 * batter_reading.h
 *
 * Created: 27/10/2015 15:44:57
 *  Author: Gabriel Notman
 */ 


#ifndef BATTER_READING_H_
#define BATTER_READING_H_

#include <asf.h>
#include <adc.h>


#define BATTERY_ADC_CLOCK			GCLK_GENERATOR_3
#define BATTERY_ADC_CLOCK_DIV		ADC_CLOCK_PRESCALER_DIV32
#define BATTERY_ADC_RESOLUTION		ADC_RESOLUTION_16BIT
#define BATTERY_ADC_RES_SCALAR		65.535 //(2^bit - 1) / 1000
#define BATTERY_AIN_PIN				ADC_POSITIVE_INPUT_PIN16

#define R1 4.7
#define R2 10.0


uint16_t read_battery_mVolts(void);


#endif /* BATTER_READING_H_ */
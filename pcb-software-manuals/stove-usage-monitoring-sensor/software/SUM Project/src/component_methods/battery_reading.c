/*
 * battery_reading.c
 *
 * Created: 27/10/2015 15:45:10
 *  Author: Gabriel Notman
 */ 

#include <asf.h>
#include <adc.h>
#include "battery_reading.h"

struct adc_module adc_instance;

uint16_t read_battery_mVolts(void)
{
	struct adc_config config_adc;
	adc_get_config_defaults(&config_adc);
	
	config_adc.clock_source = BATTERY_ADC_CLOCK;
	config_adc.clock_prescaler = BATTERY_ADC_CLOCK_DIV;
	config_adc.resolution = BATTERY_ADC_RESOLUTION;
	
	config_adc.reference = ADC_REFERENCE_INTVCC1;
	config_adc.gain_factor = ADC_INPUTCTRL_GAIN_DIV2;
	
	config_adc.positive_input = BATTERY_AIN_PIN;
	config_adc.negative_input = ADC_NEGATIVE_INPUT_GND;

	config_adc.run_in_standby = true;
	
	adc_init(&adc_instance, ADC, &config_adc);
	adc_enable(&adc_instance);
	
	adc_start_conversion(&adc_instance);
	
	uint16_t result;
	do {
	} while (adc_read(&adc_instance, &result) == STATUS_BUSY);
	
	adc_disable(&adc_instance);
	system_apb_clock_clear_mask(SYSTEM_CLOCK_APB_APBC, PM_APBCMASK_ADC);

	result = (float)result * ((3.3 / BATTERY_ADC_RES_SCALAR) * (R1 + R2) / R2);
	
	return result;
}
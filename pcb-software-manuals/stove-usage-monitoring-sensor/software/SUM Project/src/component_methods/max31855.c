/*
 * Max31855.c
 *
 * Created: 23/10/2015 12:18:34
 *  Author: Gabriel Notman
 */ 

#include <asf.h>
#include "max31855.h"

struct spi_module spi_master_instance;
struct spi_slave_inst spi_slave;

uint32_t max31855_read_four_spi_bytes(void);

void max31855_init(void)
{
	struct spi_config config_spi_master;
	struct spi_slave_inst_config slave_dev_config;
	
	spi_slave_inst_get_config_defaults(&slave_dev_config);
	slave_dev_config.ss_pin = SPI_SLAVE_SELECT_PIN;
	spi_attach_slave(&spi_slave, &slave_dev_config);
	
	spi_get_config_defaults(&config_spi_master);
	
	config_spi_master.mux_setting = SPI_SERCOM_MUX_SETTING;
	config_spi_master.pinmux_pad0 = SPI_SERCOM_PINMUX_PAD0;
	config_spi_master.pinmux_pad1 = SPI_SERCOM_PINMUX_PAD1;
	config_spi_master.pinmux_pad2 = SPI_SERCOM_PINMUX_PAD2;
	config_spi_master.pinmux_pad3 = SPI_SERCOM_PINMUX_PAD3;
	
	spi_init(&spi_master_instance, SPI_MODULE, &config_spi_master);
	
	struct port_config pin_conf;
	port_get_config_defaults(&pin_conf);
	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(MAX_POWER_PIN, &pin_conf);
}

bool max31855_read_values(int16_t* internal, int16_t* thermo_couple)
{
	uint32_t data = max31855_read_four_spi_bytes();
	
	if (!(data & 0x07)) { // 0b00000111, the lower 3 bits indicate an error
		*internal = ((data >> 4) & 0x07FF) / 16; // 0b0000011111111111, the first 10 bits after shifting 4. Apply scale.
		*thermo_couple = ((data >> 18) & 0x3FFF) / 4; // 0b0011111111111111, the first 14 bits after shifting 18. Apply scale.
		
		//Convert to values to negative if sign bits are set
		if (data & 0x8000) { // 0b00000000000000001000000000000000, bit 16 is the sign bit for the internal reading
			*internal *= -1;
		}
		
		if (data & 0x80000000) { // 0b10000000000000000000000000000000, bit 32 is the sign bit for the couple reading
			*thermo_couple *= -1;
		}
		
		return true;
	}
	
	return false;
}

uint32_t max31855_read_four_spi_bytes(void)
{
	union RX_DATA {
		uint8_t rx_buffer[4];
		uint32_t result;
	} rx_data;
	
	port_pin_set_output_level(MAX_POWER_PIN, true);
    spi_enable(&spi_master_instance);
	
	delay_ms(WAKE_MS);
	
	spi_select_slave(&spi_master_instance, &spi_slave, true);
	
	for (int8_t i = 3; i >= 0; i--) {
		while (!spi_is_ready_to_write(&spi_master_instance)) {
		}

		spi_write(&spi_master_instance, 0x00); // Send dummy SPI character to read in master mode

		while (!spi_is_ready_to_read(&spi_master_instance)) {
		}

		uint16_t receivedData = 0;
		enum status_code retval = spi_read(&spi_master_instance, &receivedData);
		if (retval == STATUS_OK) {
			rx_data.rx_buffer[i] = (uint8_t)(receivedData & 0xFF);
		}
		else {
			return 0xFFFF; // Overflow, abort, return all ones (all error bits are on then)
		}
	}

	spi_select_slave(&spi_master_instance, &spi_slave, false);
	
	spi_disable(&spi_master_instance);	
	port_pin_set_output_level(MAX_POWER_PIN, false);
	
	return rx_data.result;
}

/*
 * Max31855.h
 *
 * Created: 23/10/2015 12:18:34
 *  Author: Gabriel Notman
 */ 


#ifndef MAX31855_H_
#define MAX31855_H_

#include <asf.h>

#define WAKE_MS					300
#define MAX_POWER_PIN			PIN_PA27

#define SPI_SLAVE_SELECT_PIN    PIN_PB03
#define SPI_MODULE              SERCOM5
#define SPI_SERCOM_MUX_SETTING  SPI_SIGNAL_MUX_SETTING_E
#define SPI_SERCOM_PINMUX_PAD0  PINMUX_PB02D_SERCOM5_PAD0
#define SPI_SERCOM_PINMUX_PAD1  PINMUX_UNUSED
#define SPI_SERCOM_PINMUX_PAD2  PINMUX_PB22D_SERCOM5_PAD2
#define SPI_SERCOM_PINMUX_PAD3  PINMUX_PB23D_SERCOM5_PAD3

void max31855_init(void);

bool max31855_read_values(int16_t* internal, int16_t* thermo_couple);

#endif /* MAX31855_H_ */
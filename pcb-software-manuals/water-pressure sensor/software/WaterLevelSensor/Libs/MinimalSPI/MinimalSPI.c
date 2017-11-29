#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/sfr_defs.h>
#include <InputOutputMacros.h>

#include "MinimalSPI.h"

// Initialize pins for SPI
void spi_init()
{
    in(MISO);
    out(MOSI);
    out(SCK);
    out(SS);
    
    SPCR = ((1<<SPE)|       // SPI Enable
    (0 << SPIE)|              // SPI Interupt Enable
    (0 << DORD)|              // Data Order (0:MSB first / 1:LSB first)
    (1 << MSTR)|              // Master/Slave select
    (0 << SPR1) | (1 << SPR0)|    // SPI Clock Rate
    (0 << CPOL)|              // Clock Polarity (0:SCK low / 1:SCK hi when idle)
    (0 << CPHA));             // Clock Phase (0:leading / 1:trailing edge sampling)

    SPSR = (1 << SPI2X);              // Double Clock Rate
}

// Shift full array through target device
void spi_transfer_sync(uint8_t* dataOut, uint8_t* dataIn, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        SPDR = dataOut[i];
        while((SPSR & (1 << SPIF)) == 0);
        dataIn[i] = SPDR;
    }
}

// Shift full array to target device without receiving any byte
void spi_write_array(uint8_t* dataOut, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        SPDR = dataOut[i];
        while((SPSR & (1 << SPIF)) == 0);
    }
}

// Clocks only one byte to target device and returns the received one
uint8_t spi_transfer(uint8_t data)
{
    SPDR = data;
    while((SPSR & (1 << SPIF)) == 0);
    
    return SPDR;
}

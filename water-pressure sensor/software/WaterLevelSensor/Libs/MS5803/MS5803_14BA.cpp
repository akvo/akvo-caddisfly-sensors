#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>
#include <avr/sfr_defs.h>
#include <InputOutputMacros.h>

#include "MS5803_14BA.h"

extern "C" {
    #include <MinimalSPI.h>
};

// Sensor constants
#define SENSOR_CMD_RESET 0x1E
#define SENSOR_CMD_ADC_READ 0x00
#define SENSOR_CMD_ADC_CONV 0x40
#define SENSOR_CMD_ADC_D1 0x00
#define SENSOR_CMD_ADC_D2 0x10
#define SENSOR_CMD_ADC_256 0x00
#define SENSOR_CMD_ADC_512 0x02
#define SENSOR_CMD_ADC_1024 0x04
#define SENSOR_CMD_ADC_2048 0x06
#define SENSOR_CMD_ADC_4096 0x08

#define CS 0,B // 0

#define setCStoHigh(dummy) on(CS) // TODO change to more generic version
#define setCStoLow(dummy) off(CS) // TODO change to more generic version

MS5803_14BA::MS5803_14BA(uint8_t cs): _cs(cs), _pressure(0), _temperature(0) {
    out(CS); // TODO change to more generic version
}

// Initializes sensor and reads coefficient values from the device.
bool MS5803_14BA::init() {
    setCStoHigh();
    
    reset();
    
    // Read all sensor coefficients and CRC
    for (uint8_t i = 0; i < 8; i++) {
        _sensorCoefficients[i] = readCoefficient(i);
        _delay_ms(10);
    }
    
    uint8_t readCRC = _sensorCoefficients[7];
    uint8_t calculatedCRC = calculateCRC4(_sensorCoefficients);

    return (readCRC == calculatedCRC);
}

// Sends a power-on reset command to the sensor.
// Should be done at power-up and maybe on a periodic basis.
void MS5803_14BA::reset() {
    setCStoLow();
    
    spi_transfer(SENSOR_CMD_RESET);
    _delay_ms(10);
    
    setCStoHigh();
    _delay_ms(5);
}

// Handles reading and converting or temperature and pressure.
void MS5803_14BA::read() {
    uint32_t uncompensatedPressure = readADC(SENSOR_CMD_ADC_D1 + SENSOR_CMD_ADC_4096);
    uint32_t uncompensatedTemperature = readADC(SENSOR_CMD_ADC_D2 + SENSOR_CMD_ADC_4096);
    
    int64_t dT = uncompensatedTemperature - _sensorCoefficients[5] * pow(2, 8);
    int64_t pressureOffset = _sensorCoefficients[2] * pow(2, 16) + (_sensorCoefficients[4] * dT) / pow(2, 7);
    int64_t pressureSensitivity = _sensorCoefficients[1] * pow(2, 15) + (_sensorCoefficients[3] * dT) / pow(2, 8);
    int64_t temperature = 2000 + dT * _sensorCoefficients[6] / pow(2, 23);
    
    // second order compensation
    int64_t temperatureOffset = 0;
    int64_t pressureOffset2 = 0;
    int64_t pressureSensitivity2 = 0;

    if (temperature < 2000) { // (less than 20.0 C)
        temperatureOffset = 3 * pow(dT, 2) / pow(2, 33);
        pressureOffset2 = 3 * pow(temperature - 2000, 2) / 2;
        pressureSensitivity2 = 5 * pow(temperature - 2000, 2) / pow(2, 3);
        
        if (temperature < -1500) { // (less than -15.0 C)
            pressureOffset2 += 7 * pow(temperature + 1500, 2);
            pressureSensitivity2 += 4 * pow(temperature + 1500, 2);
        }
    } else { // Over 20
        temperatureOffset = 7 * pow(dT, 2) / pow(2, 37);
        pressureOffset2 = pow(temperature - 2000, 2) / pow(2, 4);
        pressureSensitivity2 = 0;
    }
    
    temperature -= temperatureOffset;
    pressureOffset -= pressureOffset2;
    pressureSensitivity -= pressureSensitivity2;    
    
    _temperature = temperature;
    _pressure = ((static_cast<int64_t>(uncompensatedPressure) * static_cast<int64_t>(pressureSensitivity) / pow(2, 21) - pressureOffset) / pow(2, 15));
}

// Reads the coefficient from the sensor's ROM, from the given index (0-7).
uint16_t MS5803_14BA::readCoefficient(uint8_t index) {
    setCStoLow();
    
    // request coefficient by index
    spi_transfer(0xA0 + (index * 2));
    
    // send a value of 0 to read each byte
    uint16_t result = ((uint16_t)spi_transfer(0x00)) << 8;
    result |= spi_transfer(0x00);
    
    setCStoHigh();
    
    return(result);
}

// Returns the CRC4 calculated from the coefficients. It should match coefficient[7].
// Based on the AN520.
uint8_t MS5803_14BA::calculateCRC4(unsigned int n_prom[]) {
    uint16_t deviceCRC = _sensorCoefficients[7];
    _sensorCoefficients[7] = (0xFF00 & (_sensorCoefficients[7]));
    
    uint16_t result = 0x00;
    for (uint8_t i = 0; i < 16; i++) { 
        // choose LSB or MSB
        if (i%2 == 1) {
            result ^= (unsigned short)((_sensorCoefficients[i >> 1]) & 0x00FF);
        } else {
            result ^= (unsigned short)(_sensorCoefficients[i >> 1] >> 8);
        }
        
        for (uint8_t bit = 8; bit > 0; bit--) {
            if (result & (0x8000)) {
                result = (result << 1) ^ 0x3000;
            } else {
                result = (result << 1);
            }
        }
    }
    
    result = (0x000F & (result >> 12)); // final 4-bit reminder is CRC code
    _sensorCoefficients[7] = deviceCRC; // restore the CRC
    
    return (uint8_t)result;
}

// Returns sensor measurements according to the given command (type | precision).
uint32_t MS5803_14BA::readADC(uint8_t cmd) {
    setCStoLow();
    spi_transfer(SENSOR_CMD_ADC_CONV + cmd);
    
    switch (cmd & 0x0f) {
        case SENSOR_CMD_ADC_256:
        {
            _delay_ms(1);
            break;
        }
        case SENSOR_CMD_ADC_512:
        {
            _delay_ms(3);
            break;
        }
        case SENSOR_CMD_ADC_1024:
        {
            _delay_ms(4);
            break;
        }
        case SENSOR_CMD_ADC_2048:
        {
            _delay_ms(6);
            break;
        }
        case SENSOR_CMD_ADC_4096:
        {
            _delay_ms(10);
            break;
        }
    }
    
    setCStoHigh();
    _delay_ms(3);
    setCStoLow();
    
    spi_transfer(SENSOR_CMD_ADC_READ);
    
    // send a value of 0 to read each byte
    uint32_t result = ((uint32_t)spi_transfer(0x00)) << 16;
    result |= ((uint16_t)spi_transfer(0x00)) << 8;
    result |= spi_transfer(0x00);
    
    setCStoHigh();
    
    return result;
}

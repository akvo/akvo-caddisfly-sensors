#ifndef MS5803_14BA_H_
#define MS5803_14BA_H_

#include <stdbool.h>
#include <stdint.h>

class MS5803_14BA
{
public:
    MS5803_14BA(uint8_t cs);
    
    // Initializes sensor and reads coefficient values from the device.
    bool init();
    
    // Sends a power-on reset command to the sensor.
    // Should be done at power-up and maybe on a periodic basis.
    void reset();

    // Handles reading and converting for temperature and pressure.
    void read();
    
    // Returns temp in degrees Celsius after read() has been called
    int32_t getTemperature() { return _temperature; }
    // Returns pressure in mBars after read() has been called
    int32_t getPressure() { return _pressure; }
private:
    uint8_t _cs; // Chip Select
    int32_t _pressure; // Stores actual pressure in mBars
    int32_t _temperature; // Stores actual temp in degrees C
    uint16_t _sensorCoefficients[8]; // Stores the Calibration Coefficients

    // Reads the coefficient from the sensor's ROM, from the given index (0-7).
    inline uint16_t readCoefficient(uint8_t index);
    
    // Returns the CRC4 calculated from the coefficients. It should match coefficient[7].
    // Based on the AN520.
    inline uint8_t calculateCRC4(uint16_t n_prom[]);
    
    // Returns sensor measurements according to the given command (type | precision).
    uint32_t readADC(uint8_t cmd);
};

#endif /* MS5803_14BA_H_) */

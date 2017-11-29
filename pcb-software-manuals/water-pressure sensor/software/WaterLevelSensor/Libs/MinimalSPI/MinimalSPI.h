#ifndef MINIMALSPI_H_
#define MINIMALSPI_H_

#define SS 2,A // 8
#define MISO 0,A // 10
#define MOSI 1,A // 9
#define SCK 3,A // 7

void spi_init();
void spi_transfer_sync(uint8_t* dataOut, uint8_t* dataIn, uint8_t len);
void spi_write_array(uint8_t* dataOut, uint8_t len);
uint8_t spi_transfer(uint8_t data);

#endif /* MINIMALSPI_H_ */

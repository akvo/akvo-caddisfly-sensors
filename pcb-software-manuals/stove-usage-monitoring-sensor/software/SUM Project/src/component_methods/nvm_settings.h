/*
 * nvm_settings.h
 *
 * Created: 09/11/2015 12:34:54
 *  Author: Gabriel Notman
 */ 


#ifndef NVM_SETTINGS_H_
#define NVM_SETTINGS_H_

#include <asf.h>

#define NVM_HEADER_SIZE		1
#define NVM_USE_ROW			0

void nvm_configure(void);

bool nvm_read_settings(uint8_t* buffer, uint16_t size);

void nvm_write_settings(uint8_t* buffer, uint16_t size);

void nvm_erase(void);

uint16_t nvm_max_settings_size(void);


#endif /* NVM_SETTINGS_H_ */
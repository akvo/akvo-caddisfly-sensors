/*
 * nvm_settings.c
 *
 * Created: 09/11/2015 12:35:10
 *  Author: Gabriel Notman
 */ 

#include <asf.h>
#include "nvm_settings.h"

uint8_t nvm_buffer[NVMCTRL_ROW_SIZE];

void nvm_clear_buffer(void);

void nvm_configure(void)
{
	struct nvm_config config_nvm;
	nvm_get_config_defaults(&config_nvm);
	
	config_nvm.sleep_power_mode = NVM_SLEEP_POWER_MODE_WAKEONACCESS;
	config_nvm.manual_page_write = true;
	
	nvm_set_config(&config_nvm);
}bool nvm_read_settings(uint8_t* buffer, uint16_t size)
{
	nvm_clear_buffer();
	
	bool result = false;
	enum status_code error_code;

	do {
		error_code = nvm_read_buffer(NVM_USE_ROW * NVMCTRL_ROW_SIZE, nvm_buffer, sizeof(nvm_buffer));
	} while (error_code == STATUS_BUSY);		//If the first byte == 1 the data is initialised	if (nvm_buffer[0] == 1) {		memcpy(buffer, &nvm_buffer[1], size);		result = true;	}
	return result;
}

void nvm_write_settings(uint8_t* buffer, uint16_t size)
{
	nvm_erase();
	
	enum status_code error_code;
	uint16_t read_length = min(size, nvm_max_settings_size());

	nvm_buffer[0] = 1;
	memcpy(&nvm_buffer[1], buffer, read_length);
	
	uint8_t pages = (read_length + 1) / NVMCTRL_PAGE_SIZE + 1;
	
	for (uint8_t i = 0; i < pages; i++) {
		do {
			error_code = nvm_write_buffer(NVM_USE_ROW * NVMCTRL_ROW_SIZE + (i * NVMCTRL_PAGE_SIZE), 
					&nvm_buffer[i * NVMCTRL_PAGE_SIZE], NVMCTRL_PAGE_SIZE);
		} while (error_code == STATUS_BUSY);
		
		do {
			error_code = nvm_execute_command(NVM_COMMAND_WRITE_PAGE,
				NVM_USE_ROW * NVMCTRL_ROW_SIZE + (i * NVMCTRL_PAGE_SIZE), 0);
		} while (error_code == STATUS_BUSY);
	}
}

void nvm_erase(void)
{
	nvm_clear_buffer();
	
	enum status_code error_code;
	do {
		error_code = nvm_erase_row(NVM_USE_ROW * NVMCTRL_ROW_SIZE);
	} while (error_code == STATUS_BUSY);
}

uint16_t nvm_max_settings_size(void)
{
	return NVMCTRL_ROW_SIZE - NVM_HEADER_SIZE;
}

void nvm_clear_buffer(void)
{
	memset(nvm_buffer, sizeof(nvm_buffer), 0);
}
/*
 * usb_control.c
 *
 * Created: 05/11/2015 16:14:19
 *  Author: Gabriel Notman
 */ 

#include <asf.h>
#include <math.h>
#include "usb_control.h"

#define RX_BUFFER_SIZE		64
#define DOUBLE_BUFFER_SIZE	32
#define INT_BUFFER_SIZE		16

char usb_rx_buffer[RX_BUFFER_SIZE];
char command_word[RX_BUFFER_SIZE];
char key_word[RX_BUFFER_SIZE];
char value_word[RX_BUFFER_SIZE];

uint8_t usb_rx_buffer_size = 0;

bool usb_read_line(void);
void usb_clear_buffer(void);
void clear_word_buffers(void);

void usb_handle_command_status(void);
void usb_handle_command_device_id(void);
void usb_handle_command_get_reading(void);
void usb_handle_command_get_settings(void);
void usb_handle_command_supported_commands(void);
void usb_handle_command_supported_custom(void);
void usb_handle_command_get_record_count(uint8_t record_type);
void usb_handle_command_get_records(uint8_t record_type);
void usb_handle_command_delete_records(uint8_t record_type);
void usb_handle_command_get(char* key);
void usb_handle_command_set(char* key, char* value);
void usb_handle_command_erase(void);
void usb_handle_command_custom(char* command);

bool usb_read_line(void)
{
	bool result = false;
	
	if ((usb_rx_buffer_size + RX_SIZE()) > (RX_BUFFER_SIZE-1)) {
		usb_clear_buffer();
	}
	
	uint8_t max_size = min((RX_BUFFER_SIZE-1), (usb_rx_buffer_size + RX_SIZE()));
	
	char letter = '\0';
	while ((letter != LINE_ENDING_CHAR) && (usb_rx_buffer_size < max_size)) {
		letter = READ_CHAR();
		usb_rx_buffer[usb_rx_buffer_size] = letter;
		usb_rx_buffer_size++;
	}
	
	if (letter == LINE_ENDING_CHAR) {
		usb_rx_buffer[usb_rx_buffer_size] = '\0';
		result = true;		
	}
	
	return result;
}

void usb_clear_buffer(void)
{
	memset(usb_rx_buffer, 0, sizeof(usb_rx_buffer));
	usb_rx_buffer_size = 0;
}

void clear_word_buffers(void)
{
	memset(command_word, 0, sizeof(command_word));
	memset(key_word, 0, sizeof(key_word));
	memset(value_word, 0, sizeof(value_word));
}

void usb_handle_commands(uint8_t port)
{
	if (usb_read_line()) {
		clear_word_buffers();
		sscanf(usb_rx_buffer,"%s %s \"%[^\"]\"", command_word, key_word, value_word);
	
		if(strcasecmp(command_word, COMMAND_STATUS)==0) {
			usb_handle_command_status();
		}
		else
		if(strcasecmp(command_word, COMMAND_DEVICE_ID)==0) {
			usb_handle_command_device_id();
		}
		else
		if(strcasecmp(command_word, COMMAND_GET_READING)==0) {
			usb_handle_command_get_reading();
		}
		else
		if(strcasecmp(command_word, COMMAND_GET_SETTINGS)==0) {
			usb_handle_command_get_settings();
		}
		else
		if(strcasecmp(command_word, COMMAND_SUPPORTED_COMMANDS)==0) {
			usb_handle_command_supported_commands();
		}
		else
		if(strcasecmp(command_word, COMMAND_SUPPORTED_CUSTOM)==0) {
			usb_handle_command_supported_custom();
		}
		else
		if(strcasecmp(command_word, COMMAND_GET_RECORD_COUNT)==0) {
			uint8_t record_type = atoi(key_word);
			usb_handle_command_get_record_count(record_type);
		}
		else
		if(strcasecmp(command_word, COMMAND_GET_RECORDS)==0) {
			uint8_t record_type = atoi(key_word);
			usb_handle_command_get_records(record_type);
		}
		else
		if(strcasecmp(command_word, COMMAND_DELETE)==0) {
			uint8_t record_type = atoi(key_word);
			usb_handle_command_delete_records(record_type);
		}
		else
		if(strcasecmp(command_word, COMMAND_ERASE)==0) {
			usb_handle_command_erase();
		}
		else
		if(strcasecmp(command_word, COMMAND_PREFIX_GET)==0) {
			usb_handle_command_get(key_word);
		}
		else
		if(strcasecmp(command_word, COMMAND_PREFIX_SET)==0) {
			usb_handle_command_set(key_word, value_word);
		}
		else {
			usb_handle_command_custom(usb_rx_buffer);
		}
	
		usb_clear_buffer();
	}
}

void usb_handle_command_status(void)
{
	usb_println((char*)RESPONSE_OK);
}

void usb_handle_command_device_id(void)
{
	usb_println((char*)DEVICE_ID);
}

void usb_handle_command_get_reading(void)
{
	//Add your get reading, command handling code
	
	//Remove this line
	usb_println((char*)RESPONSE_NOT_SUPPORTED);
}

void usb_handle_command_get_settings(void)
{
	//Add your get settings, command handling code
	
	//Remove this line
	usb_println((char*)RESPONSE_NOT_SUPPORTED);
}

void usb_handle_command_supported_commands(void)
{
	// Labels line
	usb_print(COMMAND_STATUS); usb_print(SEPARATOR);
	usb_print(COMMAND_DEVICE_ID); usb_print(SEPARATOR);
	usb_print(COMMAND_GET_READING); usb_print(SEPARATOR);
	usb_print(COMMAND_GET_SETTINGS); usb_print(SEPARATOR);
	usb_print(COMMAND_SUPPORTED_COMMANDS); usb_print(SEPARATOR);
	usb_print(COMMAND_SUPPORTED_CUSTOM); usb_print(SEPARATOR);
	usb_print(COMMAND_GET_RECORD_COUNT); usb_print(SEPARATOR);
	usb_print(COMMAND_GET_RECORDS); usb_print(SEPARATOR);
	usb_print(COMMAND_DELETE); usb_print(SEPARATOR);
	usb_print(COMMAND_ERASE); usb_print(SEPARATOR);
	usb_print(COMMAND_PREFIX_GET); usb_print(SEPARATOR);
	usb_print(COMMAND_PREFIX_SET); usb_println("");
	
	// Value line
	usb_print_bool(SUPPORTS_COMMAND_STATUS); usb_print(SEPARATOR);
	usb_print_bool(SUPPORTS_COMMAND_DEVICE_ID); usb_print(SEPARATOR);
	usb_print_bool(SUPPORTS_COMMAND_GET_READING); usb_print(SEPARATOR);
	usb_print_bool(SUPPORTS_COMMAND_GET_SETTINGS); usb_print(SEPARATOR);
	usb_print_bool(SUPPORTS_COMMAND_SUPPORTED_COMMANDS); usb_print(SEPARATOR);
	usb_print_bool(SUPPORTS_COMMAND_SUPPORTED_CUSTOM); usb_print(SEPARATOR);
	usb_print_bool(SUPPORTS_COMMAND_GET_RECORD_COUNT); usb_print(SEPARATOR);
	usb_print_bool(SUPPORTS_COMMAND_GET_RECORDS); usb_print(SEPARATOR);
	usb_print_bool(SUPPORTS_COMMAND_DELETE); usb_print(SEPARATOR);
	usb_print_bool(SUPPORTS_COMMAND_ERASE); usb_print(SEPARATOR);
	usb_print_bool(SUPPORTS_COMMAND_GET); usb_print(SEPARATOR);
	usb_print_bool(SUPPORTS_COMMAND_SET); usb_println("");
}

void usb_handle_command_supported_custom(void)
{
	//Add your supported custom, command handling code
	
	//Remove this line
	usb_println((char*)RESPONSE_NOT_SUPPORTED);
}

void usb_handle_command_get_record_count(uint8_t record_type)
{
	//Add your get record count, command handling code
	
	//Remove this line
	usb_println((char*)RESPONSE_NOT_SUPPORTED);
}

void usb_handle_command_get_records(uint8_t record_type)
{
	//Add your get records, command handling code
	
	//Remove this line
	usb_println((char*)RESPONSE_NOT_SUPPORTED);
}

void usb_handle_command_delete_records(uint8_t record_type)
{
	//Add your delete records, command handling code
	
	//Remove this line
	usb_println((char*)RESPONSE_NOT_SUPPORTED);
}

void usb_handle_command_get(char* key)
{
	if (strlen(key) == 0) {
		usb_println((char*)RESPONSE_ERROR);
	}
	else {
		//Add your get, command handling code
	}
}

void usb_handle_command_set(char* key, char* value)
{
	if ((strlen(key) == 0) || (strlen(value) == 0)) {
		usb_println((char*)RESPONSE_ERROR);
	}
	else {
		//Add your set, command handling code
	}
}

void usb_handle_command_custom(char* command) 
{
	//Add your custom, command handling code
	
	//Use this for any unknown commands
	usb_println((char*)RESPONSE_NOT_SUPPORTED);
}

void usb_handle_command_erase(void)
{
	//Add your erase, command handling code
	
	//Use this for any unknown commands
	usb_println((char*)RESPONSE_NOT_SUPPORTED);
}

inline void usb_print(const char* data)
{
	WRITE_BUFFER(data, strlen(data));
}

inline void usb_println(const char* data)
{
	usb_print(data);
	WRITE_BUFFER(LINE_ENDING, strlen(LINE_ENDING));
}

void usb_print_bool(bool data)
{
	if (data) {
		WRITE_BUFFER(BOOL_TRUE, strlen(BOOL_TRUE));
	}
	else {
		WRITE_BUFFER(BOOL_FALSE, strlen(BOOL_FALSE));
	}
}

inline void usb_println_bool(bool data)
{
	usb_print_bool(data);
	WRITE_BUFFER(LINE_ENDING, strlen(LINE_ENDING));
}

void usb_print_double(double data,  uint8_t prec)
{
	char double_buffer[DOUBLE_BUFFER_SIZE];
	
	if (prec == 0) {
		int32_t whole = (int32_t)data;
		sprintf(double_buffer, "%ld", whole);
	}
	else {
		int32_t whole = (int32_t)data;
		int32_t fraction = (int32_t)((data - whole) * pow(10, prec) + 0.5);
		
		char format_buffer[12];
		
		sprintf(format_buffer, "%%ld.%%0%dld", prec);	
		sprintf(double_buffer, format_buffer, whole, fraction);
	}
	
	WRITE_BUFFER(double_buffer, strlen(double_buffer));
}

inline void usb_println_double(double data, uint8_t prec)
{
	usb_print_double(data, prec);
	WRITE_BUFFER(LINE_ENDING, strlen(LINE_ENDING));
}

void usb_print_int(int32_t data)
{
	char int_buffer[INT_BUFFER_SIZE];
	sprintf(int_buffer, "%ld", data);
	WRITE_BUFFER(int_buffer, strlen(int_buffer));
}

inline void usb_println_int(int32_t data)
{
	usb_print_int(data);
	WRITE_BUFFER(LINE_ENDING, strlen(LINE_ENDING));
}
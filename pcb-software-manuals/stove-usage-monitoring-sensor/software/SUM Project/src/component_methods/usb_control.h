/*
 * usb_control.h
 *
 * Created: 05/11/2015 16:13:28
 *  Author: Gabriel Notman
 */ 


#ifndef USB_CONTROL_H_
#define USB_CONTROL_H_

#include <asf.h>

// Change these after each standard command is implemented
#define SUPPORTS_COMMAND_STATUS				true
#define SUPPORTS_COMMAND_DEVICE_ID			true
#define SUPPORTS_COMMAND_GET_READING		false

#define SUPPORTS_COMMAND_GET_SETTINGS		false
#define SUPPORTS_COMMAND_SUPPORTED_COMMANDS	true
#define SUPPORTS_COMMAND_SUPPORTED_CUSTOM	false

#define SUPPORTS_COMMAND_GET_RECORD_COUNT	false
#define SUPPORTS_COMMAND_GET_RECORDS		false
#define SUPPORTS_COMMAND_DELETE				false
#define SUPPORTS_COMMAND_ERASE				false

#define SUPPORTS_COMMAND_GET				false
#define SUPPORTS_COMMAND_SET				false

// Definitions
#define LINE_ENDING					"\r\n"
#define LINE_ENDING_CHAR			'\n'
#define SEPARATOR					","

#define BOOL_TRUE					"true"
#define BOOL_FALSE					"false"

// Command definitions
#define COMMAND_STATUS				"STATUS"
#define COMMAND_DEVICE_ID			"DEVICE"
#define COMMAND_GET_READING			"READING"
#define COMMAND_GET_SETTINGS		"SETTINGS"

#define COMMAND_SUPPORTED_COMMANDS	"COMMANDS"
#define COMMAND_SUPPORTED_CUSTOM	"CUSTOM"

#define COMMAND_GET_RECORD_COUNT	"COUNT"
#define COMMAND_GET_RECORDS			"RECORDS"
#define COMMAND_DELETE				"DELETE"
#define COMMAND_ERASE				"ERASE"

#define COMMAND_PREFIX_GET			"GET"
#define COMMAND_PREFIX_SET			"SET"

#define RESPONSE_OK					"OK"
#define RESPONSE_ERROR				"ERROR"
#define RESPONSE_NOT_SUPPORTED		"NOT_SUPPORTED"

#define DEVICE_ID					"SUM_v0.1"

// Call back for when data is received on the UART
void usb_handle_commands(uint8_t port);

// Interface methods
#define RX_SIZE()					udi_cdc_get_nb_received_data()
#define READ_CHAR()					udi_cdc_getc()
#define WRITE_CHAR(c)				udi_cdc_putc(c)
#define WRITE_BUFFER(ptr, len)		udi_cdc_multi_write_buf(0, ptr, len)

// Utility methods
extern void usb_print(const char* data);
extern void usb_println(const char* data);

extern void usb_print_bool(bool data);
extern void usb_println_bool(bool data);

extern void usb_print_double(double data, uint8_t prec);
extern void usb_println_double(double data, uint8_t prec);

extern void usb_print_int(int32_t data);
extern void usb_println_int(int32_t data);

#endif /* USB_CONTROL_H_ */
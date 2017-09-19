/*
 * usb_io.h
 *
 * Created: 26/10/2015 17:41:12
 *  Author: Gabriel Notman
 */ 


#ifndef USB_SERIAL_IO_H_
#define USB_SERIAL_IO_H_

#include <asf.h>


bool usb_serial_status(void);

void usb_vbus_callback(void);

void usb_vbus_config(void);


#endif /* USB_SERIAL_IO_H_ */
/*
 * usb_io.c
 *
 * Created: 26/10/2015 17:41:22
 *  Author: Gabriel Notman
 */ 

#include <asf.h>
#include <extint.h>
#include "usb_serial_io.h"

static bool usb_interface_enable = false;

bool usb_serial_status(void)
{
	return usb_interface_enable;
}

void usb_vbus_callback(void)
{
	extint_chan_disable_callback(USB_VBUS_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
	if (port_pin_get_input_level(USB_VBUS_PIN)) {
		udc_start();
		usb_interface_enable = true;
	}
	else {
		udc_stop();
		usb_interface_enable = false;
	}
	extint_chan_enable_callback(USB_VBUS_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}

void usb_vbus_config(void)
{
	struct extint_chan_conf eint_chan_conf;
	extint_chan_get_config_defaults(&eint_chan_conf);

	eint_chan_conf.gpio_pin            = USB_VBUS_PIN;
	eint_chan_conf.gpio_pin_mux        = USB_VBUS_EIC_MUX;
	eint_chan_conf.gpio_pin_pull       = EXTINT_PULL_NONE;
	eint_chan_conf.detection_criteria  = EXTINT_DETECT_BOTH;
	eint_chan_conf.wake_if_sleeping    = true;
	eint_chan_conf.filter_input_signal = true;

	extint_chan_disable_callback(USB_VBUS_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_set_config(USB_VBUS_EIC_LINE, &eint_chan_conf);
	extint_register_callback(usb_vbus_callback, USB_VBUS_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(USB_VBUS_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}

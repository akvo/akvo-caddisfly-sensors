/*
 * DeviceID.h
 *
 * Created: 16/12/2015
 *  Author: Gabriel Notman
 */

#ifndef DEVICE_ID_H_
#define DEVICE_ID_H_

#include <stdint.h>
#include <WString.h>

void getDeviceID(uint8_t (&idBuffer)[8]);
void addDeviceIDToString(String & str);

#endif /* DEVICE_ID_H_ */

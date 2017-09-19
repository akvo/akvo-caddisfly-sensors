/*
 * rtc_methods.h
 *
 * Created: 23/10/2015 12:46:26
 *  Author: Gabriel Notman
 */ 

#ifndef RTC_METHODS_H_
#define RTC_METHODS_H_

#include <asf.h>


uint32_t rtc_getEpoch(struct rtc_calendar_time time, bool time24h);

uint32_t rtc_getY2kEpoch(struct rtc_calendar_time time, bool time24h);

void rtc_setEpoch(struct rtc_calendar_time* time, uint32_t ts);

void rtc_setY2kEpoch(struct rtc_calendar_time* time, uint32_t ts);


#endif /* RTC_METHODS_H_ */
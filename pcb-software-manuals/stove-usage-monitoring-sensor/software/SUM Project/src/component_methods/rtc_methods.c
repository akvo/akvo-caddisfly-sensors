/*
 * rtc_methods.c
 *
 * Created: 23/10/2015 12:45:52
 *  Author: Gabriel Notman
 */ 

#include <asf.h>
#include "rtc_methods.h"

#define EPOCH_TIME_OFF 946684800  

static const uint8_t daysInMonth[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
	
uint32_t rtc_getEpoch(struct rtc_calendar_time time, bool time24h)
{
	return rtc_getY2kEpoch(time, time24h) + EPOCH_TIME_OFF;
}

uint32_t rtc_getY2kEpoch(struct rtc_calendar_time time, bool time24h)
{
	uint16_t days = time.day;
	days = days > 0 ? days : 1;
	uint8_t months = time.month;
	uint16_t years = time.year - 2000;

	for (uint8_t i = 1; i < months; ++i) {
		days += daysInMonth[i - 1];
	}

	if ((months > 2) && (years % 4 == 0)) {
		++days;
	}
	days += 365 * years + (years + 3) / 4 - 1;
	
	uint8_t hours = time.hour;
	
	if (!time24h) {
		uint8_t pm = time.pm;

		if ((!pm) && (hours == 12)) {
			hours = 0;
		}
		else if ((pm) && (hours != 12)) {
			hours += 12;
		}
	}

	return ((days * 24 + hours) * 60 + time.minute) * 60 + time.second;
}

void rtc_setEpoch(struct rtc_calendar_time* time, uint32_t ts)
{
	if (ts < EPOCH_TIME_OFF) {
		rtc_setY2kEpoch(time, 0);
	}
	else {
		rtc_setY2kEpoch(time, ts - EPOCH_TIME_OFF);
	}
}

void rtc_setY2kEpoch(struct rtc_calendar_time* time, uint32_t ts)
{
	time->second = ts % 60;
	ts /= 60;
	time->minute = ts % 60;
	ts /= 60;
	time->hour = ts % 24;

	uint16_t days = ts / 24;
	uint8_t months;
	uint8_t years;

	uint8_t leap;

	// Calculate years
	for (years = 0; ; ++years) {
		leap = years % 4 == 0;
		if (days < 365 + leap)
		break;
		days -= 365 + leap;
	}

	// Calculate months
	for (months = 1; ; ++months) {
		uint8_t daysPerMonth = daysInMonth[months - 1];
		if (leap && months == 2)
		++daysPerMonth;
		if (days < daysPerMonth)
		break;
		days -= daysPerMonth;
	}

	time->year = years + 2000;
	time->month = months;
	time->day = days + 1;
}

/*
 * Copyright 2016, M2M4ALL
 *
 * This is part of the SSU WAP.
 * This module handles the (1 Hz) tick timer
 */

#ifndef _TICKTIMER_H
#define _TICKTIMER_H

void setupTickTimer();

extern volatile bool tick_flag;
//extern volatile uint32_t tick_millis;

#endif

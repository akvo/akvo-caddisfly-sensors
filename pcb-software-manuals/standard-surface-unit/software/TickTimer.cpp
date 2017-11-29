/*
 * Copyright 2016, M2M4ALL
 *
 * This is part of the SSU WAP.
 * This module handles the (1 Hz) tick timer
 */

#include <Arduino.h>

#include "TickTimer.h"

volatile bool tick_flag = false;
//volatile uint32_t tick_millis = 0;

void setupTickTimer()
{
    // The GCLK clock provider to use
    // GCLK0, GCLK1 & GCLK3 are used already, see startup.c
    const uint8_t GCLK_SRC = 4;

    // Configure the XOSC32K to run in standby
    //SYSCTRL->XOSC32K.bit.RUNSTDBY = 1;

    // Setup clock provider GCLK_SRC with a /2 source divider
    // GCLK_GENDIV_ID(X) specifies which GCLK we are configuring
    // GCLK_GENDIV_DIV(Y) specifies the clock prescalar / divider
    // If GENCTRL.DIVSEL is set (see further below) the divider
    // is 2^(Y+1). If GENCTRL.DIVSEL is 0, the divider is simply Y
    // This register has to be written in a single operation
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(GCLK_SRC) | GCLK_GENDIV_DIV(2);
    while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) {
        /* Wait for synchronization */
    }

    // Configure the GCLK module
    // GCLK_GENCTRL_GENEN, enable the specific GCLK module
    // GCLK_GENCTRL_SRC_XOSC32K, set the source to the XOSC32K
    // GCLK_GENCTRL_ID(X), specifies which GCLK we are configuring
    // GCLK_GENCTRL_DIVSEL, specify which prescalar mode we are using
    // GCLK_RUNSTDBY, keep the GCLK running when in standby mode
    // Output from this module is 16384hz (32768hz / 2)
    // This register has to be written in a single operation.
    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN |
            GCLK_GENCTRL_SRC_XOSC32K |
            GCLK_GENCTRL_ID(GCLK_SRC) |
            GCLK_GENCTRL_RUNSTDBY;
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) {
        /* Wait for synchronization */
    }

    // Turn the power to the TC3 module on
    PM->APBCMASK.reg |= PM_APBCMASK_TC3;

    // Set TC3 (shared with TCC2) GCLK source to GCLK_SRC
    // GCLK_CLKCTRL_CLKEN, enable the generic clock
    // GCLK_CLKCTRL_GEN(X), specifies the GCLK generator source
    // GCLK_CLKCTRL_ID(X), specifies which generic clock we are configuring
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |
            GCLK_CLKCTRL_GEN(GCLK_SRC) |
            GCLK_CLKCTRL_ID(GCM_TCC2_TC3);
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) {
        /* Wait for synchronization */
    }

    // Disable TC3. This is required (if enabled already)
    // before setting certain registers
    TC3->COUNT8.CTRLA.reg &= ~TC_CTRLA_ENABLE;
    while (TC3->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY) {
        /* Wait for synchronization */
    }

    // Set the mode to 8 bit and set it to run in standby
    // TC_CTRLA_MODE_COUNT8, specify 8bit mode
    // TC_CTRLA_RUNSTDBY, keep the module running when in standby
    // TC_CTRLA_PRESCALER_DIVxx, set the prescalar to 64
    // Prescalar options include: DIV1, DIV2, DIV4, DIV8,
    // DIV16, DIV64, DIV256, DIV1024
    // The 16384hz clock (above) /64 => 256hz
    // 8 bit counter gives 1Hz interrupts
    TC3->COUNT8.CTRLA.reg = TC_CTRLA_MODE_COUNT8 |
            TC_CTRLA_RUNSTDBY |
            TC_CTRLA_PRESCALER_DIV64;
    while (TC3->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY) {
        /* Wait for synchronization */
    }

    // Enable interrupt on overflow
    // TC_INTENSET_OVF, enable an interrupt on overflow
    TC3->COUNT8.INTENSET.reg = TC_INTENSET_OVF;
    while (TC3->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY) {
        /* Wait for synchronization */
    }

    // Enable TC3
    TC3->COUNT8.CTRLA.reg |= TC_CTRLA_ENABLE;
    while (TC3->COUNT8.STATUS.reg & TC_STATUS_SYNCBUSY) {
        /* Wait for synchronization */
    }

    // Enable the TC3 interrupt vector
    // Set the priority to max
    NVIC_EnableIRQ(TC3_IRQn);
    NVIC_SetPriority(TC3_IRQn, 0x00);
}

// TC3 ISR
void TC3_Handler()
{
    if (TC3->COUNT8.INTFLAG.bit.OVF) {
        // Reset interrupt flag
        TC3->COUNT8.INTFLAG.bit.OVF = 1;

        //tick_millis = millis();

        // Set compare match flag for CC0
        tick_flag = true;
    }
}

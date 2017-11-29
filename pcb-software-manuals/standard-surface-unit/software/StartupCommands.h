/*
 * Based on: SQ_StartupCommands.h
 *
 *  Created on: Mar 27, 2014
 *      Author: Kees Bakker
 */

#ifndef STARTUPCOMMANDS_H_
#define STARTUPCOMMANDS_H_

#include <stdint.h>
#include <Arduino.h>
#include "Command.h"

void startupCommands(Stream & stream);
void supComRegisterDiagPort(Stream * stream);
void supComRegisterDiagPort(Stream & stream);
void supComRegisterCmnds(const char * banner,
    const Command * cmds, uint8_t nrCmds,
    void (*preShow)(Stream * stream),
    void (*postExec)(void));

#endif /* STARTUPCOMMANDS_H_ */

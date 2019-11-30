/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352_debug.h

DESCRIPTION
    Provide debug functionality for the power smb1352 library.
*/

#ifndef POWER_SMB1352_DEBUG_H_
#define POWER_SMB1352_DEBUG_H_

#include <print.h>

#ifdef DEBUG_POWER_SMB1352_ENABLED
#define POWER_SMB1352_PRINT(x)    PRINT(x)
#else
#define POWER_SMB1352_PRINT(x)
#endif

/****************************************************************************
NAME
    powerSmb1352DebugDisplayAllRegisters

DESCRIPTION
    Read and print the contents of the SMB1352 registers

*/
#ifdef DEBUG_POWER_SMB1352_ENABLED
void powerSmb1352DebugDisplayAllRegisters(void);
#else
#define powerSmb1352DebugDisplayAllRegisters() ((void) (0))
#endif

#endif /* POWER_SMB1352_DEBUG_H_ */

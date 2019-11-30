/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352_debug.c

DESCRIPTION
    Provide debug functionality for the power smb1352 library.
*/

#ifdef DEBUG_POWER_SMB1352_ENABLED

#include <panic.h>

#include "power_smb1352_debug.h"
#include "power_smb1352_registers.h"
#include "power_smb1352_config.h"
#include "power_smb1352_interrupt.h"
#include "power_smb1352_i2c.h"


/****************************************************************************
NAME
    readConfigRegisters

DESCRIPTION
    Read and print the contents of the SMB1352 configuration registers

*/
static void readConfigRegisters(void)
{
    uint8 index;
    uint8 data;

    for (index = SMB1352_BASE_CONFIG_REGISTER; index <= SMB1352_TOP_CONFIG_REGISTER; index++)
    {
        powerSmb1352I2cRead(index, &data);
        POWER_SMB1352_PRINT(("POWER SMB1352: Configuration Register 0x%x, contains 0x%x\n", index, data));
    }
}

/****************************************************************************
NAME
    readSummitRegisters

DESCRIPTION
    Read and print the contents of the SMB1352 summit registers

*/
static void readSummitRegisters(void)
{
    uint8 index;
    uint8 data;

    for (index = SMB1352_BASE_SUMMIT_REGISTER; index <= SMB1352_TOP_SUMMIT_REGISTER; index++)
    {
        powerSmb1352I2cRead(index, &data);
        POWER_SMB1352_PRINT(("POWER SMB1352: Summit Register 0x%x, contains 0x%x\n", index, data));
    }
}

/****************************************************************************
NAME
    powerSmb1352DebugDisplayAllRegisters

DESCRIPTION
    Read and print the contents of the SMB1352 registers

*/
void powerSmb1352DebugDisplayAllRegisters(void)
{
    readConfigRegisters();
    readSummitRegisters();
    powerSmb1352InterruptProcessAll();
}

#endif

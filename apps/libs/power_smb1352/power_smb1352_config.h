/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352_config.h

DESCRIPTION
    Handle SMB1352 specific config data.
*/

#ifndef POWER_SMB1352_CONFIG_H_
#define POWER_SMB1352_CONFIG_H_

#include "power_smb1352_registers.h"

typedef struct
{
    uint8 configuration_registers[SMB1352_NUMBER_OF_CONFIGURATION_REGISTERS];
} power_smb_1352_config_t;

/******************************************************************************
NAME
    powerSmb1352ConfigEnableCharging

DESCRIPTION
    Enable/ Disable charging on the SMB1352
*/
void powerSmb1352ConfigEnableCharging(bool enable);

/******************************************************************************
NAME
    powerSmb1352ConfigControlChargingUsingRegister

DESCRIPTION
    Configure charging control by register value
*/
void powerSmb1352ConfigControlChargingUsingRegister(void);

/******************************************************************************
NAME
    powerSmb1352ConfigEnableAICL

DESCRIPTION
    Enable/ disable automatic input current limit
*/
void powerSmb1352ConfigEnableAICL(bool enable);

/******************************************************************************
NAME
    powerSmb1352ConfigSetAICLTo3A

DESCRIPTION
    Sets the automatic input current limit to 3A
*/
void powerSmb1352ConfigSetAICLTo3A(void);

/******************************************************************************
NAME
    powerSmb1352ConfigInitialiseRegisters

DESCRIPTION
    Initialise the SMB1352 registers

*/
void powerSmb1352ConfigInitialiseRegisters(void);

#endif /* POWER_SMB1352_CONFIG_H_*/

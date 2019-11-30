/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352_config_read.h

DESCRIPTION
    Read SMB1352 specific config data.
*/

#ifndef POWER_SMB1352_CONFIG_READ_H_
#define POWER_SMB1352_CONFIG_READ_H_

#include "power_smb1352_config.h"

/******************************************************************************
NAME
    powerSmb1352ConfigReadPopulateData

DESCRIPTION
    Getter for the SMB1352 configuration data
*/

void powerSmb1352ConfigReadPopulateData(power_smb_1352_config_t * config_data);

#endif /*POWER_SMB1352_CONFIG_READ_H_*/

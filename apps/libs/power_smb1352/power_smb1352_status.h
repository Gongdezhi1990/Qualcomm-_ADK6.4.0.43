/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352_status.h

DESCRIPTION
    Get the status of the SMB1352.
*/

#ifndef POWER_SMB1352_STATUS_H_
#define POWER_SMB1352_STATUS_H_

#include "power_smb1352.h"

typedef enum
{
    alternative_port = 0,
    standard_downstream_port,
    dedicated_charging_port,
    charging_downstream_port
} smb1352_usb_source_t;



/****************************************************************************
NAME
    powerSmb1352StatusCheckQC3Connected

DESCRIPTION
    Check the version of charger connected

RETURNS
    TRUE if the QC3 charger is connected
*/
bool powerSmb1352StatusCheckQC3Connected(void);

/****************************************************************************
NAME
    powerSmb1352StatusCheckSourceDetected

DESCRIPTION
    Check the power source to the SMB1352

RETURNS
    The usb source connected to the SMB1352
*/
smb1352_usb_source_t powerSmb1352StatusCheckSourceDetected(void);

/****************************************************************************
NAME
    powerSmb1352StatusIsCharging

DESCRIPTION
    Check that the SMB1352 is in precharging, fast or taper charging.

RETURNS
    TRUE if charging
*/
bool powerSmb1352StatusIsCharging(void);

#endif /* POWER_SMB1352_STATUS_H_ */

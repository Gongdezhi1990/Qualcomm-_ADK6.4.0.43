/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352_status.c

DESCRIPTION
    Get the status of the SMB1352.
*/

#include <panic.h>

#include "power_smb1352_i2c.h"
#include "power_smb1352_registers.h"
#include "power_smb1352_status.h"


bool powerSmb1352StatusCheckQC3Connected(void)
{
    uint8 data;

    powerSmb1352I2cRead(SMB1352_QC3_AUTH_ALOGORITHM_DONE_REG_ADDR, &data);

    return (data & SMB1352_QC3_AUTH_ALOGORITHM_DONE_STATUS_MASK);
}

smb1352_usb_source_t powerSmb1352StatusCheckSourceDetected(void)
{
    uint8 data;

    powerSmb1352I2cRead(SMB1352_APSD_DETECTION_RESULT_STATUS_REG_ADDR, &data);

    /* The ordering of the following checks is key to the device operation and should not be
     * changed, they originate from flow charts provided by the SMB1352 development team.
     */

    if (data & SMB1352_DCP_STATUS_MASK)
        return dedicated_charging_port;

    if (data & SMB1352_SDP_STATUS_MASK)
        return standard_downstream_port;

    if (data & SMB1352_CDP_STATUS_MASK)
        return charging_downstream_port;

    return alternative_port;
}

bool powerSmb1352StatusIsCharging(void)
{
    uint8 data;

    powerSmb1352I2cRead(SMB1352_CHARGING_STATUS_REG_ADDR, &data);

    return (data & SMB1352_CHARGING_STATUS_STATUS_MASK);
}

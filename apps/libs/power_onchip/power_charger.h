/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    power_charger.h

DESCRIPTION
    This file contains the monitoring of battery charging functionality.
    
    Once the battery charger is connected, charger is monitored and the
    charging state is repeatly reported to App.
NOTES
    
**************************************************************************/


#ifndef POWER_CHARGER_H_
#define POWER_CHARGER_H_

#include <charger.h>
#include "power_utils.h"

/* Charger Progress Monitor is 13-bits */
#define MAX_CHG_PRG_MON            0x1FFF

/****************************************************************************
NAME
    powerChargerConfigHelper

DESCRIPTION
    Performs safe configuration for single charger configuration items.

RETURNS
    void
*/
void powerChargerConfigHelper(charger_chg_cfg_key key, uint16 value);

/****************************************************************************
NAME
    powerChargerDisconnected
    
DESCRIPTION
    Check if charger is disconnected or connected
    
RETURNS
    TRUE if disconnected, FALSE if connected
*/
bool powerChargerDisconnected(void);


/****************************************************************************
NAME
    powerChargerHandleMonitorReq
    
DESCRIPTION
    Monitor charger status (and handle boost enable/disable).
    
RETURNS
    void
*/
void powerChargerHandleMonitorReq(void);


/****************************************************************************
NAME
    powerChargerHandleVoltageReading
    
DESCRIPTION
    Calculate the current charger voltage and manage boost enable/disable.
    
RETURNS
    void
*/
void powerChargerHandleVoltageReading(uint16 reading);

/****************************************************************************
NAME
    powerChargerGetTrim
    
DESCRIPTION
    Read PSKEY_CHARGER_TRIM or set to invalid if not present.
    
RETURNS
    void
*/
void powerChargerGetTrim(void);

/****************************************************************************
NAME
    powerOnChipChargerGetState

DESCRIPTION
    The value pointed to by state will be set to the updated charger state
    unless state is NULL.

RETURNS
    void
*/
void powerOnChipChargerGetState(power_charger_state* state);

#endif /* POWER_CHARGER_H_ */



/****************************************************************************
Copyright (c) 2005 - 2017    Qualcomm Technologies International, Ltd.

FILE NAME
    power_current_limiting.c

DESCRIPTION
    This file contains the current determination operation of the battery
    charging functionality.

NOTES

    Current determination starts charging at 500mA and Vchg is measured periodically.
    If Vchg is above the limit threshold then the charging current is increased.
    If charging current reaches its target then current determination is finished.
    If Vchg goes below the limit threshold then the charging current is periodically
    decreased until Vchg goes above the recovery threshold.

**************************************************************************/


#ifndef POWER_CURRENT_LIMITING_H_
#define POWER_CURRENT_LIMITING_H_


/****************************************************************************
NAME
    powerCurrentLimitingInit
    
DESCRIPTION
    Initialise the current limiting operation.
    
RETURNS
    void
*/
void powerCurrentLimitingInit(void);

/****************************************************************************
NAME
    PowerCurrentLimitingSendIndication

DESCRIPTION
    Send current limiting indication if appropriate.

RETURNS
    void
*/
void PowerCurrentLimitingSendIndication(uint16 reading);

#endif /* POWER_CURRENT_LIMITING_H_ */

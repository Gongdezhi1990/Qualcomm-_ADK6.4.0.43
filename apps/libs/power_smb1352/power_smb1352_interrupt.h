/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352_interrupt.h

DESCRIPTION
    Handle SMB1352 interrupts.
*/

#ifndef POWER_SMB1352_INTERRUPT_H_
#define POWER_SMB1352_INTERRUPT_H_

/****************************************************************************
NAME
    powerSmb1352InterruptProcessAll

DESCRIPTION
    Read all of the status registers into cache.
    Trigger any events.
    Clear all interrupts.
*/
void powerSmb1352InterruptProcessAll(void);

/****************************************************************************
NAME
    powerSmb1352InterruptIsPowerOkay

DESCRIPTION
    Returns the Power OK status.

RETURNS
    TRUE if power okay, FALSE otherwise.
*/
bool powerSmb1352InterruptIsPowerOkay(void);

/****************************************************************************
NAME
    powerSmb1352InterruptIsBatteryMissing

DESCRIPTION
    Returns the battery terminal removed status.

RETURNS
    TRUE if battery terminal removed, FALSE otherwise.
*/
bool powerSmb1352InterruptIsBatteryMissing(void);

/****************************************************************************
NAME
    powerSmb1352InterruptIsChargingComplete

DESCRIPTION
    Returns charging complete status

RETURNS
    TRUE if charging is complete, FALSE otherwise.
*/
bool powerSmb1352InterruptIsChargingComplete(void);

/****************************************************************************
NAME
    powerSmb1352InterruptIsAPSDComplete

DESCRIPTION
    Returns apsd complete status

RETURNS
    TRUE if apsd is complete, FALSE otherwise.
*/
bool powerSmb1352InterruptIsAPSDComplete(void);

#endif /* POWER_SMB1352_INTERRUPT_H_ */

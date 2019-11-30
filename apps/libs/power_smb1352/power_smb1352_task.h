/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352_task.h

DESCRIPTION
    Handle SMB1352 tasks.
*/

#ifndef POWER_SMB1352_TASK_H_
#define POWER_SMB1352_TASK_H_

/****************************************************************************
NAME
    powerSmb1352TaskRegisterChargerTask

DESCRIPTION
    Register the application charger task.

*/
void powerSmb1352TaskRegisterChargerTask(Task task);

/****************************************************************************
NAME
    powerSmb1352TaskBoot

DESCRIPTION
    Handler for the boot sequence

*/
void powerSmb1352TaskBoot(Task client);

/****************************************************************************
NAME
    powerSmb1352TaskPowerOk

DESCRIPTION
    Handler for the power OK interrupt

*/
void powerSmb1352TaskPowerOk(bool pok);

/****************************************************************************
NAME
    powerSmb1352TaskIsPowerOk

DESCRIPTION
    Checks the Power OK status.

RETURNS
    TRUE if power okay, FALSE otherwise.
*/
bool powerSmb1352TaskIsPowerOk(void);


/****************************************************************************
NAME
    powerSmb1352TaskChargingComplete

DESCRIPTION
    Handler for the charging complete interrupt

*/
void powerSmb1352TaskChargingComplete(bool complete);

/****************************************************************************
NAME
    powerSmb1352TaskPreToFastChargeThreshold

DESCRIPTION
    Handler for the pre to fast charge threshold interrupt

*/
void powerSmb1352TaskPreToFastChargeThreshold(bool exceeded);

/****************************************************************************
NAME
    powerSmb1352TaskTaperCharging

DESCRIPTION
    Handler for the taper charging interrupt

*/
void powerSmb1352TaskTaperCharging(bool taper);

/****************************************************************************
NAME
    powerSmb1352TaskBatteryOV

DESCRIPTION
    Handler for the battery OV interrupt

*/
void powerSmb1352TaskBatteryOV(bool ov);

/****************************************************************************
NAME
    powerSmb1352TaskBatteryMissing

DESCRIPTION
    Handler for the battery missing interrupt

*/
void powerSmb1352TaskBatteryMissing(bool missing);

/****************************************************************************
NAME
    powerSmb1352TaskAPSDComplete

DESCRIPTION
    Handler for the APSD complete interrupt

*/
void powerSmb1352TaskAPSDComplete(bool complete);

/****************************************************************************
NAME
    powerSmb1352TaskAICLComplete

DESCRIPTION
    Handler for the AICL complete interrupt

*/
void powerSmb1352TaskAICLComplete(bool complete);

/****************************************************************************
NAME
    powerSmb1352TaskPreToFastChargeTimeout

DESCRIPTION
    Handler for the pre to fast charge timeout interrupt

*/
void powerSmb1352TaskPreToFastChargeTimeout(bool timeout);

/****************************************************************************
NAME
    powerSmb1352TaskCompleteChargeTimeout

DESCRIPTION
    Handler for the complete charge timeout interrupt

*/
void powerSmb1352TaskCompleteChargeTimeout(bool timeout);

/****************************************************************************
NAME
    powerSmb1352TaskChargerError

DESCRIPTION
    Handler for the charger error interrupt

*/
void powerSmb1352TaskChargerError(bool charger_error);

/****************************************************************************
NAME
    powerSmb1352TaskBatteryThermPinMissing

DESCRIPTION
    Handler for the battery therm pin missing interrupt

*/
void powerSmb1352TaskBatteryThermPinMissing(bool therm_pin_missing);

/****************************************************************************
NAME
    powerSmb1352TaskBatteryTerminalRemoved

DESCRIPTION
    Handler for the battery terminal removed interrupt

*/
void powerSmb1352TaskBatteryTerminalRemoved(bool terminal_removed);

/****************************************************************************
NAME
    powerSmb1352TaskBatteryTemperatureHardLimitExceeded

DESCRIPTION
    Handler for the battery temperature hard limits being exceeded

*/
void powerSmb1352TaskBatteryTemperatureHardLimitExceeded(bool limit_exceeded);

/****************************************************************************
NAME
    powerSmb1352TaskBatteryTemperatureSoftLimitExceeded

DESCRIPTION
    Handler for the battery temperature soft limits being exceeded

*/
void powerSmb1352TaskBatteryTemperatureSoftLimitExceeded(bool limit_exceeded);

#endif /* POWER_SMB1352_TASK_H_ */

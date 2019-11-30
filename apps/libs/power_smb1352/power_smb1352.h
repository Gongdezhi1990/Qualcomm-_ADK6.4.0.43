/****************************************************************************
Copyright (c) 2005-2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file    power_smb1352.h

@brief   Declares the API for the SMB1352 power library.

         These functions are used to configure and monitor the SMB1352 charger.

         This file provides documentation for the Power SMB1352 API.
*/

#ifndef POWER_SMB1352_H_
#define POWER_SMB1352_H_

/*!
    @brief This function will initialise the SMB1352 as per the settings
           specified in chargerConfig.hex within the customer_ro_filesystem.

*/
void PowerSmb1352PowerInit(Task clientTask);

/*!
    @brief This function processes interrupts from the SMB1352, takes the
           appropriate action before clearing the interrupts.

*/
void PowerSmb1352ProcessInterrupts(void);

/*!
    @brief Register a task to handle messages from the Quick Charge hardware.

    @param task This task will receive messages when parts of the Quick Charger
           hardware changes.
*/
void PowerSmb1352QuickChargeRegisterTask(Task task);

/*!
    @brief Check if power is okay.

    @return TRUE if power is okay, FALSE otherwise.
*/
bool PowerSmb1352IsPowerOkay(void);

/*!
    @brief Check if charger is in pre, fast or taper charge states.

    @return TRUE if charging, FALSE otherwise.
*/
bool PowerSmb1352IsCharging(void);

#endif /* POWER_SMB1352_H_ */

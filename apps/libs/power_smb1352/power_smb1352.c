/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352.c

DESCRIPTION
    Configures and monitors the SMB1352 charger.
*/

/******************************************************************************/

#include "power_smb1352.h"
#include "power_smb1352_interrupt.h"
#include "power_smb1352_task.h"
#include "power_smb1352_status.h"

void PowerSmb1352PowerInit(Task clientTask)
{
    powerSmb1352TaskBoot(clientTask);
}

void PowerSmb1352ProcessInterrupts(void)
{
    powerSmb1352InterruptProcessAll();
}

void PowerSmb1352QuickChargeRegisterTask(Task task)
{
    powerSmb1352TaskRegisterChargerTask(task);
}

bool PowerSmb1352IsPowerOkay(void)
{
    return powerSmb1352TaskIsPowerOk();
}

bool PowerSmb1352IsCharging(void)
{
    return powerSmb1352StatusIsCharging();
}

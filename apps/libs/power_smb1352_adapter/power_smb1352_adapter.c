/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352_plugin.c

DESCRIPTION
    Calls the functions in the SMB1352 power library for fuel gauge, and the
	default power library for all other charger functionality. 
	
	These functions are used to monitor the reference, battery and charger 
	voltages; monitor the battery temperature; and configure the charger.
*/

#include <panic.h>

#include <power_smb1352_adapter.h>
#include <power_smb1352.h>
#include <power_onchip.h>

/******************************************************************************/
void PowerAdapterInit(Task clientTask, const power_config *power_data, const power_pmu_temp_mon_config *pmu_mon_config)
{
    PowerOnChipInit(clientTask, power_data, pmu_mon_config);
    PowerSmb1352PowerInit(clientTask);
}


/******************************************************************************/
void PowerAdapterClose(void)
{
    PowerOnChipClose();
}


/******************************************************************************/
bool PowerAdapterBatteryGetVoltage(voltage_reading* vbat)
{
    return PowerOnChipBatteryGetVoltage(vbat);
}


/******************************************************************************/
bool PowerAdapterBatteryGetTemperature(voltage_reading *vthm)
{
    /* Battery temperature is managed by SMB1352. */
    UNUSED(vthm);
    return FALSE;
}


/******************************************************************************/
bool PowerAdapterChargerGetVoltage(voltage_reading* vchg)
{
    /* Charger voltage is managed by SMB1352. */
    UNUSED(vchg);
    return FALSE;
}


/******************************************************************************/
void PowerAdapterChargerMonitor(void)
{
    /* Never monitor the default charger when using SMB1352. */
}


/******************************************************************************/
void PowerAdapterChargerEnable(bool enable)
{
    /* Never enable the default charger when using SMB1352. */
    if (enable)
        Panic();
}


/******************************************************************************/
void PowerAdapterChargerSetCurrent(uint16 current)
{
    /* Never set charge current when using SMB1352. */
    if (current > 0)
        Panic();
}


/******************************************************************************/
void PowerAdapterChargerSetBoost(power_boost_enable boost)
{
    /* Never configure boost settings when using SMB1352. */
    if (boost != power_boost_disabled)
        Panic();
}


/******************************************************************************/
bool PowerAdapterChargerSetVterm(uint16 vterm)
{
    /* Never configure termination voltage when using SMB1352. */
    UNUSED(vterm);
    return FALSE;
}

/******************************************************************************/
uint16 PowerAdapterChargerGetVoltageScale(void)
{
    return PowerOnChipChargerGetVoltageScale();
}


/******************************************************************************/
void PowerAdapterChargerSuppressLed0(bool suppress)
{
    /* Never configure Suppress LED0 when using SMB1352. */
    UNUSED(suppress);
}


/******************************************************************************/
void PowerAdapterChargerConfigure(void)
{
    /* Never configure the default charger when using SMB1352. */
}


/******************************************************************************/
bool PowerAdapterChargerEnabled(void)
{
    return PowerOnChipChargerEnabled();
}

/******************************************************************************/
bool PowerAdapterChargerConnected(void)
{
    return PowerSmb1352IsPowerOkay();
}

/******************************************************************************/
bool PowerAdapterChargerFullCurrent(void)
{
    return PowerSmb1352IsCharging();
}

/******************************************************************************/
bool PowerAdapterChargerFastCharge(void)
{
    return PowerSmb1352IsCharging();
}

/******************************************************************************/
bool PowerAdapterChargerDisabled(void)
{
    return !PowerSmb1352IsCharging();
}

/******************************************************************************/
void PowerAdapterRegisterChargerTask(Task task)
{
    PowerSmb1352QuickChargeRegisterTask(task);
}

/******************************************************************************/
void PowerAdapterMonitorStartChargerProgressMonitor(void)
{
    PowerOnChipMonitorStartChargerProgressMonitor();
}


/******************************************************************************/
void PowerAdapterMonitorStopChargerProgressMonitor(void)
{
    PowerOnChipMonitorStopChargerProgressMonitor();
}


/******************************************************************************/
void PowerAdapterCurrentLimitingHandleMonitorReq(uint16 vchg_reading)
{
    /* Never perform current limiting with default charger when using SMB1352. */
    UNUSED(vchg_reading);
}


/******************************************************************************/
void PowerAdapterCurrentLimitingStart(void)
{
    /* Never perform current limiting with default charger when using SMB1352. */
}


/******************************************************************************/
void PowerAdapterCurrentLimitingStop(void)
{
    /* Never perform current limiting with default charger when using SMB1352. */
}


/******************************************************************************/
void PowerAdapterCurrentLimitingHandleHeadroomError(void)
{
    /* Never perform current limiting with default charger when using SMB1352. */
}

/******************************************************************************/
bool PowerAdapterBatteryIsOverVoltage(void)
{
    return PowerOnChipBatteryIsOverVoltage();
}


/******************************************************************************/
bool PowerAdapterBatteryIsRecoveredFromOverVoltage(voltage_reading vbat)
{
    return PowerOnChipBatteryIsRecoveredFromOverVoltage(vbat);
}


/******************************************************************************/
charger_battery_status PowerAdapterBatteryGetStatusAtBoot(void)
{
    return PowerOnChipBatteryGetStatusAtBoot();
}

/******************************************************************************/
void PowerAdapterQuickChargeServiceInterrupt(void)
{
    PowerSmb1352ProcessInterrupts();
}

/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_default_plugin.c

DESCRIPTION
    Calls the functions in the power library for the default charger.
*/

#include <power_onchip_adapter.h>
#include <power_onchip.h>

/******************************************************************************/
void PowerAdapterInit(Task clientTask, const power_config *power_data, const power_pmu_temp_mon_config *pmu_mon_config)
{
    PowerOnChipInit(clientTask, power_data, pmu_mon_config);
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
    return PowerOnChipBatteryGetTemperature(vthm);
}


/******************************************************************************/
bool PowerAdapterChargerGetVoltage(voltage_reading* vchg)
{
    return PowerOnChipChargerGetVoltage(vchg);
}


/******************************************************************************/
void PowerAdapterChargerMonitor(void)
{
    PowerOnChipChargerMonitor();
}


/******************************************************************************/
void PowerAdapterChargerEnable(bool enable)
{
    PowerOnChipChargerEnable(enable);
}


/******************************************************************************/
void PowerAdapterChargerSetCurrent(uint16 current)
{
    PowerOnChipChargerSetCurrent(current);
}


/******************************************************************************/
void PowerAdapterChargerSetBoost(power_boost_enable boost)
{
    PowerOnChipChargerSetBoost(boost);
}


/******************************************************************************/
bool PowerAdapterChargerSetVterm(uint16 vterm)
{
    return PowerOnChipChargerSetVterm(vterm);
}

/******************************************************************************/
uint16 PowerAdapterChargerGetVoltageScale(void)
{
    return PowerOnChipChargerGetVoltageScale();
}


/******************************************************************************/
void PowerAdapterChargerSuppressLed0(bool suppress)
{
    PowerOnChipChargerSuppressLed0(suppress);
}


/******************************************************************************/
void PowerAdapterChargerConfigure(void)
{
    PowerOnChipChargerConfigure();
}


/******************************************************************************/
bool PowerAdapterChargerEnabled(void)
{
    return PowerOnChipChargerEnabled();
}

/******************************************************************************/
bool PowerAdapterChargerConnected(void)
{
    return PowerOnChipChargerConnected();
}

/******************************************************************************/
bool PowerAdapterChargerFullCurrent(void)
{
    return PowerOnChipChargerFullCurrent();
}

/******************************************************************************/
bool PowerAdapterChargerFastCharge(void)
{
    return PowerOnChipChargerFastCharge();
}

/******************************************************************************/
bool PowerAdapterChargerDisabled(void)
{
    return PowerOnChipChargerDisabled();
}

/******************************************************************************/
void PowerAdapterRegisterChargerTask(Task task)
{
    PowerOnChipChargerRegisterTask(task);
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
    PowerOnChipCurrentLimitingHandleMonitorReq(vchg_reading);
}


/******************************************************************************/
void PowerAdapterCurrentLimitingStart(void)
{
    PowerOnChipCurrentLimitingStart();
}


/******************************************************************************/
void PowerAdapterCurrentLimitingStop(void)
{
    PowerOnChipCurrentLimitingStop();
}


/******************************************************************************/
void PowerAdapterCurrentLimitingHandleHeadroomError(void)
{
    PowerOnChipCurrentLimitingHandleHeadroomError();
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
    /* Not applicable for onchip charger. */
}

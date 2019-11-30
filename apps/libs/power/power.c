/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power.c

DESCRIPTION
    These functions are used to monitor the reference, battery and charger
    voltages; monitor the battery temperature; and configure the charger.

    These functions use a Power Adapter library that is selected at link time
    to target the specific battery charger hardware.
*/

#include <power.h>
#include <power_adapter.h>

/******************************************************************************/
void PowerInit(Task clientTask, const power_config *power_data, const power_pmu_temp_mon_config *pmu_mon_config)
{
    PowerAdapterInit(clientTask, power_data, pmu_mon_config);
}

/******************************************************************************/
void PowerClose(void)
{
    PowerAdapterClose();
}

/******************************************************************************/
bool PowerBatteryGetVoltage(voltage_reading* vbat)
{
    return PowerAdapterBatteryGetVoltage(vbat);
}

/******************************************************************************/
bool PowerBatteryGetTemperature(voltage_reading *vthm)
{
    return PowerAdapterBatteryGetTemperature(vthm);
}

/******************************************************************************/
bool PowerChargerGetVoltage(voltage_reading* vchg)
{
    return PowerAdapterChargerGetVoltage(vchg);
}

/******************************************************************************/
void PowerChargerMonitor(void)
{
    PowerAdapterChargerMonitor();
}

/******************************************************************************/
void PowerChargerEnable(bool enable)
{
    PowerAdapterChargerEnable(enable);
}

/******************************************************************************/
void PowerChargerSetCurrent(uint16 current)
{
    PowerAdapterChargerSetCurrent(current);
}

/******************************************************************************/
void PowerChargerSetBoost(power_boost_enable boost)
{
    PowerAdapterChargerSetBoost(boost);
}

/******************************************************************************/
bool PowerChargerSetVterm(uint16 vterm)
{
    return PowerAdapterChargerSetVterm(vterm);
}

/******************************************************************************/
uint16 PowerChargerGetVoltageScale(void)
{
    return PowerAdapterChargerGetVoltageScale();
}

/******************************************************************************/
void PowerChargerSuppressLed0(bool suppress)
{
    PowerAdapterChargerSuppressLed0(suppress);
}

/******************************************************************************/
void PowerChargerConfigure(void)
{
    PowerAdapterChargerConfigure();
}

/******************************************************************************/
bool PowerChargerEnabled(void)
{
    return PowerAdapterChargerEnabled();
}

/******************************************************************************/
bool PowerChargerConnected(void)
{
    return PowerAdapterChargerConnected();
}

/******************************************************************************/
bool PowerChargerFullCurrent(void)
{
    return PowerAdapterChargerFullCurrent();
}

/******************************************************************************/
bool PowerChargerFastCharge(void)
{
    return PowerAdapterChargerFastCharge();
}

/******************************************************************************/
bool PowerChargerDisabled(void)
{
    return PowerAdapterChargerDisabled();
}

/******************************************************************************/
void PowerChargerRegisterTask(Task task)
{
    PowerAdapterRegisterChargerTask(task);
}

/******************************************************************************/
void PowerMonitorStartChargerProgressMonitor(void)
{
    PowerAdapterMonitorStartChargerProgressMonitor();
}

/******************************************************************************/
void PowerMonitorStopChargerProgressMonitor(void)
{
    PowerAdapterMonitorStopChargerProgressMonitor();
}

/******************************************************************************/
void PowerCurrentLimitingHandleMonitorReq(uint16 vchg_reading)
{
    PowerAdapterCurrentLimitingHandleMonitorReq(vchg_reading);
}

/******************************************************************************/
void PowerCurrentLimitingStart(void)
{
    PowerAdapterCurrentLimitingStart();
}

/******************************************************************************/
void PowerCurrentLimitingStop(void)
{
    PowerAdapterCurrentLimitingStop();
}

/******************************************************************************/
void PowerCurrentLimitingHandleHeadroomError(void)
{
    PowerAdapterCurrentLimitingHandleHeadroomError();
}

/******************************************************************************/
bool PowerBatteryIsOverVoltage(void)
{
    return PowerAdapterBatteryIsOverVoltage();
}

/******************************************************************************/
bool PowerBatteryIsRecoveredFromOverVoltage(voltage_reading vbat)
{
    return PowerAdapterBatteryIsRecoveredFromOverVoltage(vbat);
}

/******************************************************************************/
charger_battery_status PowerBatteryGetStatusAtBoot(void)
{
    return PowerAdapterBatteryGetStatusAtBoot();
}

/******************************************************************************/
void PowerQuickChargeServiceInterrupt(void)
{
    PowerAdapterQuickChargeServiceInterrupt();
}

/*************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.
 

FILE : 
    power.h

CONTAINS:
    API for the Battery power monitoring and charger management

**************************************************************************/


#ifndef POWER_ONCHIP_H_
#define POWER_ONCHIP_H_

#include <adc.h>
#include <charger.h>
#include <library.h>
#include <power.h>

/****************************************************************************
NAME    
    PowerOnChipInit
    
DESCRIPTION
    This function will initialise the battery and battery charging sub-system
    The sub-system manages the reading and calulation of the battery voltage 
    and temperature
    
    The application will receive a POWER_INIT_CFM message from the 
    library indicating the initialisation status.
    
RETURNS
    void
*/
void PowerOnChipInit(Task clientTask, const power_config *power_data, const power_pmu_temp_mon_config *pmu_mon_config);


/****************************************************************************
NAME
    PowerOnChipClose

DESCRIPTION
    This function will close the battery and battery charging sub-system

RETURNS
    void
*/
void PowerOnChipClose(void);


/****************************************************************************
NAME
    PowerOnChipBatteryGetVoltage
    
DESCRIPTION
    Call this function to get the current battery voltage (in mV) and the 
    level
 
RETURNS
    bool
*/
bool PowerOnChipBatteryGetVoltage(voltage_reading* vbat);


/****************************************************************************
NAME
    PowerOnChipBatteryGetTemperature
    
DESCRIPTION
    Call this function to get the current thermistor voltage (in mV) and the
    level
    
RETURNS
    bool
*/
bool PowerOnChipBatteryGetTemperature(voltage_reading *vthm);


/****************************************************************************
NAME
    PowerOnChipChargerGetVoltage
    
DESCRIPTION
    Call this function to get the current charger voltage (in mV)
    
RETURNS
    bool
*/
bool PowerOnChipChargerGetVoltage(voltage_reading* vchg);


/****************************************************************************
NAME
    PowerOnChipChargerMonitor
    
DESCRIPTION
    Enable/disable the charger monitoring. Monitoring is enabled by default 
    but is disabled when charger detach is detected. It is up to the 
    application to re-enable charger monitoring when the charger is attached
    
RETURNS
    void
*/
void PowerOnChipChargerMonitor(void);


/****************************************************************************
NAME
    PowerOnChipChargerEnable
    
DESCRIPTION
    Enable/disable the charger.
    
RETURNS
    void
*/
void PowerOnChipChargerEnable(bool enable);


/****************************************************************************
NAME
    PowerOnChipChargerSetCurrent
    
DESCRIPTION
    Set the charger fast current level. The value is expressed in mA.
    
    In Bluecore this value will be rounded to the nearest possible current setting.
    
    In Hydracore this value is the target current, which may not be achieved in external
    transister configurations due to current limiting.
    
RETURNS
    void
*/
void PowerOnChipChargerSetCurrent(uint16 current);


/****************************************************************************
NAME    
    PowerOnChipChargerSetBoost
    
DESCRIPTION
    Configure boost settings. By default this is disabled, but can be 
    configured to draw boost current from internal/external transistor. 
    Boost charge will be enabled on entering fast charge unless the boost 
    cycle has already completed. The boost cycle is reset on charger attach.
    
    Not available on Hydracore devices.
    
RETURNS
    void
*/
void PowerOnChipChargerSetBoost(power_boost_enable boost);


/****************************************************************************
NAME
    PowerOnChipChargerSetVterm
    
DESCRIPTION
    Reduce the termination voltage to (vterm * PowerOnChipChargerGetVoltageScale()). This will
    fail if requested termination voltage is higher than the configured 
    termination voltage.
    
RETURNS
    bool
*/
bool PowerOnChipChargerSetVterm(uint16 vterm);

/****************************************************************************
NAME
    PowerOnChipChargerGetVoltageScale

DESCRIPTION
    Gets the voltage scalar for the charger version.

RETURNS
    uint16
*/
uint16 PowerOnChipChargerGetVoltageScale(void);

/****************************************************************************
NAME
    PowerOnChipChargerSuppressLed0

DESCRIPTION
    Suppress LED0 whilst charging.

RETURNS
    void
*/
void PowerOnChipChargerSuppressLed0(bool suppress);

/****************************************************************************
NAME
    PowerOnChipChargerConfigure

DESCRIPTION
    Initialise power charger
*/
void PowerOnChipChargerConfigure(void);

/****************************************************************************
NAME
    PowerOnChipChargerEnabled

DESCRIPTION
    Check if charger is enabled

RETURNS
    TRUE if enabled, FALSE otherwise
*/
bool PowerOnChipChargerEnabled(void);

/****************************************************************************
NAME
    PowerOnChipChargerConnected

DESCRIPTION
    Determines charger connected status from underlying hardware. Can be called
    before Power library is initialised.

RETURNS
    TRUE if enabled, FALSE otherwise
*/
bool PowerOnChipChargerConnected(void);

/****************************************************************************
NAME
    PowerOnChipChargerFullCurrent

DESCRIPTION
    Determines if the charger is (potentially) drawing full current from
    underlying hardware. Can be called before Power library is initialised.

RETURNS
    TRUE if enabled, FALSE otherwise
*/
bool PowerOnChipChargerFullCurrent(void);

/****************************************************************************
NAME
    PowerOnChipChargerFastCharge

DESCRIPTION
    Determines if the charger is fast charging from underlying hardware.
    Can be called before Power library is initialised.

RETURNS
    TRUE if enabled, FALSE otherwise
*/
bool PowerOnChipChargerFastCharge(void);

/****************************************************************************
NAME
    PowerOnChipChargerDisabled

DESCRIPTION
    Determines if the charger is connected but disabled from underlying hardware.
    Can be called before Power library is initialised.

RETURNS
    TRUE if enabled, FALSE otherwise
*/
bool PowerOnChipChargerDisabled(void);

/****************************************************************************
NAME
    PowerOnChipChargerRegisterTask

DESCRIPTION
    This task will receive messages when the on-chip charger changes state.

RETURNS
    void
*/
void PowerOnChipChargerRegisterTask(Task task);

/****************************************************************************
NAME
    PowerOnChipMonitorStartChargerProgressMonitor

DESCRIPTION
    Start monitoring Charger Progress Monitor.

RETURNS
    void
*/
void PowerOnChipMonitorStartChargerProgressMonitor(void);

/****************************************************************************
NAME
    PowerOnChipMonitorStopChargerProgressMonitor

DESCRIPTION
    Stop monitoring Charger Progress Monitor.

RETURNS
    void
*/
void PowerOnChipMonitorStopChargerProgressMonitor(void);


/****************************************************************************
NAME
    PowerOnChipCurrentLimitingHandleMonitorReq

DESCRIPTION
    Handle current limiting power monitoring requests.

RETURNS
    void
*/
void PowerOnChipCurrentLimitingHandleMonitorReq(uint16 vchg_reading);

/****************************************************************************
NAME
    PowerOnChipCurrentLimitingStart

DESCRIPTION
    Start limiting the current.

RETURNS
    void
*/
void PowerOnChipCurrentLimitingStart(void);

/****************************************************************************
NAME
    PowerOnChipCurrentLimitingStop

DESCRIPTION
    Stop limiting the current.

RETURNS
    void
*/
void PowerOnChipCurrentLimitingStop(void);

/****************************************************************************
NAME
    PowerOnChipCurrentLimitingHandleHeadroomError

DESCRIPTION
    Handle current limiting 'Headroom Error' handler.

RETURNS
    void
*/
void PowerOnChipCurrentLimitingHandleHeadroomError(void);

/****************************************************************************
NAME
    PowerOnChipBatteryIsOverVoltage

DESCRIPTION
    This function is called by applications to check whether the battery
    is in Over Voltage.

RETURNS
    bool
*/
bool PowerOnChipBatteryIsOverVoltage(void);

/****************************************************************************
NAME
    PowerOnChipBatteryIsRecoveredFromOverVoltage

DESCRIPTION
    This function is called by applications to check whether the battery
    has recovered from Over Voltage.

RETURNS
    bool
*/
bool PowerOnChipBatteryIsRecoveredFromOverVoltage(voltage_reading vbat);

/****************************************************************************
NAME
    PowerOnChipBatteryGetStatusAtBoot

DESCRIPTION
    This function is called by applications to check the status of
    battery at boot time.

RETURNS
    The status of battery at boot time.
*/
charger_battery_status PowerOnChipBatteryGetStatusAtBoot(void);

#endif     /* POWER_DEFAULT_H_ */

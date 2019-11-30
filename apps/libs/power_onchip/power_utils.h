/****************************************************************************
Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_utils.h

DESCRIPTION
    This file contains helper functions for battery charging.
    
NOTES
    
**************************************************************************/


#ifndef POWER_UTILS_H_
#define POWER_UTILS_H_

#include "power_onchip.h"

#define powerUtilAdcSourceIsVref(adc_source)    (adc_source == power->config.vref.adc.source)
#define powerUtilAdcSourceIsVbat(adc_source)    (adc_source == power->config.vbat.adc.source)
#define powerUtilAdcSourceIsVchg(adc_source)    (adc_source == power->config.vchg.adc.source)
#define powerUtilAdcSourceIsVthm(adc_source)    (adc_source == power->config.vthm.adc.source)
#define powerUtilAdcSourceIsChargerProgressMonitor(adc_source) (powerUtilIsChargerProgressMonitorAdc(adc_source))

#define CHARGER_KEY_INVALID -1

/* This enumerated type MUST be translated into charger_config_key type
 * (see charger_if.h) before being used to configure the charger.
 */

typedef enum
{
    CHARGER_CONFIG_ENABLE,
    CHARGER_CONFIG_TRIM,
    CHARGER_CONFIG_CURRENT,
    CHARGER_CONFIG_SUPPRESS_LED0,
    CHARGER_CONFIG_ENABLE_BOOST,
    CHARGER_CONFIG_USE_EXT_TRAN,
    CHARGER_CONFIG_ATTACH_RESET_ENABLE,
    CHARGER_CONFIG_TERM_VOLTAGE,
    CHARGER_CONFIG_ENABLE_HIGH_CURRENT_EXTERNAL_MODE,
    CHARGER_CONFIG_SET_EXTERNAL_TRICKLE_CURRENT,
    CHARGER_CONFIG_STANDBY_FAST_HYSTERESIS,
    CHARGER_CONFIG_TRICKLE_CURRENT,
    CHARGER_CONFIG_PRE_CURRENT,
    CHARGER_CONFIG_FAST_CURRENT,
    CHARGER_CONFIG_EXTERNAL_RESISTOR,
    CHARGER_CONFIG_USE_EXTERNAL_RESISTOR_FOR_PRE_CHARGE,
    CHARGER_CONFIG_USE_EXTERNAL_RESISTOR_FOR_FAST_CHARGE,
    CHARGER_CONFIG_PRE_FAST_THRESHOLD,
    CHARGER_CONFIG_ITERM_CTRL,
    CHARGER_CONFIG_STATE_CHANGE_DEBOUNCE
}charger_chg_cfg_key;

/****************************************************************************
NAME
    PowerUtilInit

DESCRIPTION
    Initialise the power utilities.

RETURNS
    void
*/

void powerUtilInit(void);

/****************************************************************************
NAME
    powerUtilFatalError

DESCRIPTION
    Disable charger before panic.

RETURNS
    void
*/
void powerUtilFatalError(void);

/****************************************************************************
NAME
    powerUtilChargerConfigure

DESCRIPTION
    Configure the charger.

RETURNS
    void
*/
void powerUtilChargerConfigure(charger_config_key key, uint16 value);

/****************************************************************************
NAME
    powerUtilChargerEnable

DESCRIPTION
    Enables/disables the charger.

RETURNS
    bool
*/
bool powerUtilChargerEnable(bool enable);

/****************************************************************************
NAME
    powerUtilIsChargerEnabled

DESCRIPTION
    Check if charger is enabled

RETURNS
    TRUE if enabled, FALSE otherwise
*/
bool powerUtilIsChargerEnabled(void);

/****************************************************************************
NAME
    powerUtilSetChargerCurrents

DESCRIPTION
    Set the charger current

RETURNS
    void
*/
void powerUtilSetChargerCurrents(uint16 current);

/****************************************************************************
NAME
    powerUtilMapChargerKeys

DESCRIPTION
    Maps generic charger keys to specific charger keys.

RETURNS
    charger_config_key
*/
charger_config_key powerUtilMapChargerKeys(charger_chg_cfg_key key);

/****************************************************************************
NAME
    powerUtilGetChargerVoltageLevel

DESCRIPTION
    Returns TRUE if charging voltage is too high, FALSE otherwise.

RETURNS
    bool
*/
bool powerUtilGetChargerVoltageLevel(void);

/****************************************************************************
NAME
    powerUtilGetVoltageScale

DESCRIPTION
    Returns the voltage scalar for V2 chargers.

RETURNS
    uint16
*/
uint16 powerUtilGetVoltageScale(void);

/****************************************************************************
NAME
    powerUtilGetVoltage

DESCRIPTION
    Returns the voltage derived from ADC reading and Vref.

RETURNS
    uint16
*/
uint16 powerUtilGetVoltage(const MessageAdcResult *result, uint16 vref_reading);

/****************************************************************************
NAME
    powerUtilGetChargerState

DESCRIPTION
    Get current power library state for the charger (derived from f/w state)

RETURNS
    power_charger_state
*/
power_charger_state powerUtilGetChargerState(void);

/****************************************************************************
NAME
    powerUtilGetThermalConfig

DESCRIPTION
    Get configuration for die temperature management

RETURNS
    void
*/
void powerUtilGetThermalConfig(void);

/****************************************************************************
NAME
    powerUtilGetThermalDieTemperature

DESCRIPTION
    Get die temperature

RETURNS
    int16
*/
int16 powerUtilGetThermalDieTemperature(void);

/****************************************************************************
NAME
    powerUtilInitCompleteMask

DESCRIPTION
    Returns the initialisation mask.

RETURNS
    uint16
*/
uint16 powerUtilInitCompleteMask(void);

/****************************************************************************
NAME
    powerUtilGetAdcReadPeriod

DESCRIPTION
    Returns the period between ADC read requests.

RETURNS
    Delay
*/
Delay powerUtilGetAdcReadPeriod(power_adc* adc);

/****************************************************************************
NAME
    powerUtilHandleChargerProgressMonitorReq

DESCRIPTION
    Handles requests for Charger Progress Monitor.

RETURNS
    void
*/
void powerUtilHandleChargerProgressMonitorReq(Task task);

/****************************************************************************
NAME
    powerUtilIsChargerProgressMonitorAdc

DESCRIPTION
    Returns TRUE if ADC is Charger Progress Monitor, FALSE otherwise.

RETURNS
    bool
*/
bool powerUtilIsChargerProgressMonitorAdc(vm_adc_source_type adc_source);

/****************************************************************************
NAME
    powerUtilAdcRequest

DESCRIPTION
    Returns TRUE if ADC is successful, FALSE otherwise.

RETURNS
    bool
*/
bool powerUtilAdcRequest(Task task, vm_adc_source_type adc_source, bool is_vthm, bool drive_ics, uint16 delay);


/****************************************************************************
NAME
    powerUtilConditionVchgReading
    
DESCRIPTION
    Condition Vchg reading. Provides an opportunity to correct the Vchg reading
    if required by the H/W.
*/
uint16 powerUtilConditionVchgReading(uint16 vchg_reading);

/****************************************************************************
NAME
    powerUtilIsOverVoltage

DESCRIPTION
    This function is called by applications to check whether the battery
    is in Over Voltage.

RETURNS
    bool
*/
bool powerUtilIsOverVoltage(void);

/****************************************************************************
NAME
    powerUtilIsBatteryAboveTerminationVoltage

DESCRIPTION
    This function is called by applications to check whether the battery
    voltage is below the termination voltage.

RETURNS
    bool
*/
bool powerUtilIsBatteryBelowTerminationVoltage(uint16 vbat_millivolts);

/****************************************************************************
NAME
    powerUtilChargerFullCurrent

DESCRIPTION
    Determines if the charger is (potentially) drawing full current from
    underlying hardware. Can be called before Power library is initialised.

RETURNS
    TRUE if enabled, FALSE otherwise
*/
bool powerUtilChargerFullCurrent(void);

#endif /* POWER_UTILS_H_ */

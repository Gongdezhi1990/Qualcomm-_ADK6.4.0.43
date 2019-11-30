/****************************************************************************
Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_utils.c

DESCRIPTION
    This file contains helper functions for battery charging.

NOTES

**************************************************************************/


/****************************************************************************
    Header files
*/

#include <charger.h>
#include <panic.h>
#include <vm.h>
#include <ps.h>
#include <adc.h>

#include "power_private.h"
#include "power_utils.h"
#include "power_charger.h"
#include "power_monitor.h"
#include "power_thermal.h"
#include "power_onchip.h"

#define ADC_REF_VOLTAGE_IN_MV (1800)

/* See PMU_CHG_VFLOAT_CTRL for following values. */
#define MIN_VTERM 3650
#define VTERM_STEP 50

#ifndef MIN
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#endif

typedef struct
{
    uint16 vterm;
    uint16 trickle_current;
    uint16 pre_current;
    uint16 fast_current;
}   charger_configuration_settings;

typedef struct
{
    charger_configuration_settings requested;
    charger_configuration_settings configured;
}   charger_configuration;

static charger_configuration state_manager;

/****************************************************************************
NAME
    PowerUtilChargerConfigureHelper

DESCRIPTION
    Safely handle charger configuration failure.

RETURNS
    void
*/
static void PowerUtilChargerConfigureHelper(charger_config_key key, uint16 value)
{
    if (key == CHARGER_ENABLE)
    {
        if (value != FALSE)
        {
            PanicZero(state_manager.configured.trickle_current);
            PanicZero(state_manager.configured.pre_current);
            PanicZero(state_manager.configured.fast_current);
        }
        else
        {
            PanicFalse(power->charger_enabled != FALSE);
        }
    }

    PRINT(("POWER: Trp ChargerConfigure(key = %u, value = %u)\n", key, value));
    if (ChargerConfigure(key, value) == FALSE)
    {
        powerUtilFatalError();
    }
}

/****************************************************************************
NAME
    PowerUtilUpdateChargerConfiguration

DESCRIPTION
    If the requested value is different to the enabled value then configure
    the charger with the new value and remember the configured value.

RETURNS
    bool
*/
static bool PowerUtilUpdateChargerConfiguration(charger_config_key key, uint16 requested_value, uint16 *configured_value)
{
    if (requested_value != *configured_value)
    {
        PowerUtilChargerConfigureHelper(key, requested_value);
        *configured_value = requested_value;
        return TRUE;
    }

    return FALSE;
}

/****************************************************************************
NAME
    PowerUtilManageChargerConfiguration

DESCRIPTION
    Configures the charger with any values that have changed.
    If the termination voltage has changed then the charger must be reset.

RETURNS
    bool
*/
static bool PowerUtilManageChargerConfiguration(void)
{
    bool charger_reset_required;
    PowerUtilUpdateChargerConfiguration(CHARGER_TRICKLE_CURRENT, state_manager.requested.trickle_current, &state_manager.configured.trickle_current);
    PowerUtilUpdateChargerConfiguration(CHARGER_PRE_CURRENT, state_manager.requested.pre_current, &state_manager.configured.pre_current);
    PowerUtilUpdateChargerConfiguration(CHARGER_FAST_CURRENT, state_manager.requested.fast_current, &state_manager.configured.fast_current);
    charger_reset_required = PowerUtilUpdateChargerConfiguration(CHARGER_TERM_VOLTAGE, state_manager.requested.vterm, &state_manager.configured.vterm);
    return charger_reset_required;
}

/****************************************************************************
NAME
    PowerUtilInit

DESCRIPTION
    Initialise the power utilities.

RETURNS
    void
*/

void powerUtilInit(void)
{
    state_manager.requested.vterm = 0;
    state_manager.requested.trickle_current = 0;
    state_manager.requested.pre_current = 0;
    state_manager.requested.fast_current = 0;
    state_manager.configured.vterm = 0;
    state_manager.configured.trickle_current = 0;
    state_manager.configured.pre_current = 0;
    state_manager.configured.fast_current = 0;
}

/****************************************************************************
NAME
    powerUtilFatalError

DESCRIPTION
    Disable charger before panic.

RETURNS
    void
*/
void powerUtilFatalError(void)
{
    if (PowerOnChipChargerEnabled())
    {
        powerUtilChargerEnable(FALSE);
    }
    Panic();
}

/****************************************************************************
NAME
    powerUtilChargerConfigure

DESCRIPTION
    Configure the charger.

RETURNS
    void
*/
void powerUtilChargerConfigure(charger_config_key key, uint16 value)
{
    switch (key)
    {
    case CHARGER_TERM_VOLTAGE:
        state_manager.requested.vterm = value;
        break;
    case CHARGER_TRICKLE_CURRENT:
        PanicFalse(value <= MAX_TRICKLE_CURRENT);
        state_manager.requested.trickle_current = value;
        break;
    case CHARGER_PRE_CURRENT:
        PanicFalse(value <= MAX_INTERNAL_CURRENT);
        state_manager.requested.pre_current = value;
        break;
    case CHARGER_FAST_CURRENT:
        PanicFalse(value <= MAX_EXTERNAL_CURRENT);
        state_manager.requested.fast_current = value;
        break;
    default:
        PowerUtilChargerConfigureHelper(key, value);
        break;
    }
}

/****************************************************************************
NAME
    powerUtilChargerEnable

DESCRIPTION

    Current       | Requested     | Charger       | Action
    charger state | charger state | Configuration |
    --------------+---------------+---------------+----------------------------------
    Enabled       | Enabled       | Changed       | Configure and reset the charger
    Enabled       | Enabled       | Not changed   | Do nothing
    Enabled       | Disabled      | Don't care    | Disable the charger
    Disabled      | Enabled       | Changed       | Configure and enable the charger
    Disabled      | Enabled       | Not changed   | Enable the charger
    Disabled      | Disabled      | Don't care    | Do nothing

RETURNS
    bool
*/
bool powerUtilChargerEnable(bool enable)
{
    switch (ChargerStatus())
    {
        case CONFIG_FAIL_EFUSE_CRC_INVALID:
        case CONFIG_FAIL_EFUSE_TRIMS_ZERO:
        case ENABLE_FAIL_EFUSE_CRC_INVALID:
        {
            PowerUtilChargerConfigureHelper(CHARGER_ENABLE, FALSE);
            return FALSE;
        }
        default:
        {
            if (power->charger_enabled)
            {
                if (enable)
                {
                    if (PowerUtilManageChargerConfiguration())
                    {
                        PowerUtilChargerConfigureHelper(CHARGER_ENABLE, FALSE);
                        PowerUtilChargerConfigureHelper(CHARGER_ENABLE, TRUE);
                    }
                 }
                 else
                 {
                    PowerUtilChargerConfigureHelper(CHARGER_ENABLE, FALSE);
                 }
            }
            else
            {
                if (enable)
                {
                    PowerUtilManageChargerConfiguration();
                    PowerUtilChargerConfigureHelper(CHARGER_ENABLE, TRUE);
                }
            }

            return TRUE;
        }
    }
}

/****************************************************************************
NAME
    powerUtilIsChargerEnabled

DESCRIPTION
    Check if charger is enabled

RETURNS
    TRUE if enabled, FALSE otherwise
*/
bool powerUtilIsChargerEnabled(void)
{
    /* Have to use trap, called before Power lib initialised */
    charger_status status = ChargerStatus();
    PRINT(("POWER: Trp ChargerStatus(status = %u)\n", status));
    return ((status == TRICKLE_CHARGE) ||
            (status == PRE_CHARGE) ||
            (status == FAST_CHARGE));
}

/****************************************************************************
NAME
    powerUtilGetSafeCurrentLimit

DESCRIPTION
    Gets the maximum safe charger current based on use of the external resistor.

RETURNS
    uint16
*/

static uint16 powerUtilGetSafeCurrentLimit(bool use_ext, uint16 current)
{
    if (use_ext)
    {
        if (current > MAX_EXTERNAL_CURRENT)
            current = MAX_EXTERNAL_CURRENT;
    }
    else
    {
        if (current > MAX_INTERNAL_CURRENT)
            current = MAX_INTERNAL_CURRENT;
    }

    return current;
}

/****************************************************************************
NAME
    powerUtilSetChargerCurrents

DESCRIPTION
    Set the charger current

RETURNS
    void
*/
void powerUtilSetChargerCurrents(uint16 current)
{
    uint16 new_safe_current;

    current = powerThermalUpdateCurrent(current);

    powerChargerConfigHelper(CHARGER_CONFIG_TRICKLE_CURRENT, power->config.vchg.trickle_charge_current);

    powerChargerConfigHelper(CHARGER_CONFIG_PRE_CURRENT, power->config.vchg.precharge_current);

    new_safe_current = powerUtilGetSafeCurrentLimit(power->config.vchg.ext_fast, current);

    if (power->config.vchg.ext_fast)
    {
        if (new_safe_current < power->charger_i_safe_fast)
        {
            if (power->charger_i_actual_fast > new_safe_current)
            {
                power->charger_i_actual_fast = new_safe_current;
            }
        }
        else if (new_safe_current > power->charger_i_safe_fast)
        {
            power->charger_i_actual_fast = MIN(new_safe_current, INITIAL_FAST_DYNAMIC_CURRENT_IN_MA);
        }
        else
        {
            /* The safe target has not changed, so do not adjust the actual current. */
        }

        power->charger_i_safe_fast = new_safe_current;
    }
    else
    {
        power->charger_i_actual_fast = new_safe_current;
    }

    powerChargerConfigHelper(CHARGER_CONFIG_FAST_CURRENT,
                             power->charger_i_actual_fast);
}

/****************************************************************************
NAME
    powerUtilMapChargerKeys

DESCRIPTION
    Map charger keys to correct values for ChargerConfigure() Trap.

RETURNS
    charger_config_key
*/
charger_config_key powerUtilMapChargerKeys(charger_chg_cfg_key key)
{
    switch (key)
    {
        /* These are not used on V2 charger. */
        case CHARGER_CONFIG_TRIM:
        case CHARGER_CONFIG_CURRENT:
        case CHARGER_CONFIG_SUPPRESS_LED0:
        case CHARGER_CONFIG_ENABLE_BOOST:
        case CHARGER_CONFIG_USE_EXT_TRAN:
        case CHARGER_CONFIG_ATTACH_RESET_ENABLE:
        case CHARGER_CONFIG_ENABLE_HIGH_CURRENT_EXTERNAL_MODE:
        case CHARGER_CONFIG_SET_EXTERNAL_TRICKLE_CURRENT:
        default:
            return CHARGER_KEY_INVALID;

        case CHARGER_CONFIG_TERM_VOLTAGE:
            return CHARGER_TERM_VOLTAGE;

        case CHARGER_CONFIG_STANDBY_FAST_HYSTERESIS:
            return CHARGER_STANDBY_FAST_HYSTERESIS;

        case CHARGER_CONFIG_TRICKLE_CURRENT:
            return CHARGER_TRICKLE_CURRENT;

        case CHARGER_CONFIG_PRE_CURRENT:
            return CHARGER_PRE_CURRENT;

        case CHARGER_CONFIG_FAST_CURRENT:
            return CHARGER_FAST_CURRENT;

        case CHARGER_CONFIG_EXTERNAL_RESISTOR:
            return CHARGER_EXTERNAL_RESISTOR;

        case CHARGER_CONFIG_USE_EXTERNAL_RESISTOR_FOR_PRE_CHARGE:
            return CHARGER_USE_EXTERNAL_RESISTOR_FOR_PRE_CHARGE;

        case CHARGER_CONFIG_USE_EXTERNAL_RESISTOR_FOR_FAST_CHARGE:
            return CHARGER_USE_EXTERNAL_RESISTOR_FOR_FAST_CHARGE;

        case CHARGER_CONFIG_PRE_FAST_THRESHOLD:
            return CHARGER_PRE_FAST_THRESHOLD;

        case CHARGER_CONFIG_ITERM_CTRL:
            return CHARGER_ITERM_CTRL;

        case CHARGER_CONFIG_STATE_CHANGE_DEBOUNCE:
            return CHARGER_STATE_CHANGE_DEBOUNCE;

        case CHARGER_CONFIG_ENABLE:
            return CHARGER_ENABLE;
    }
}

/****************************************************************************
NAME
    powerUtilGetChargerVoltageLevel

DESCRIPTION
    Returns TRUE if charging voltage is too high, FALSE otherwise.

RETURNS
    bool
*/

bool powerUtilGetChargerVoltageLevel(void)
{
    return (power->vchg >= power->config.vchg.default_vfloat);
}

/****************************************************************************
NAME
    powerUtilGetVoltageScale

DESCRIPTION
    Returns the voltage scalar for V2 chargers.

RETURNS
    uint16
*/

uint16 powerUtilGetVoltageScale(void)
{
    return 0x01;
}

/****************************************************************************
NAME
    powerUtilGetVoltage

DESCRIPTION
    Returns the voltage derived from ADC reading and Vref.

RETURNS
    uint16
*/
uint16 powerUtilGetVoltage(const MessageAdcResult *result, uint16 vref_reading)
{
    return (((uint32)(VmReadVrefConstant()) * (uint32)result->reading)) / vref_reading;
}

/****************************************************************************
NAME
    powerUtilGetChargerState

DESCRIPTION
    Get current power library state for the charger (derived from f/w state)

RETURNS
    power_charger_state
*/
power_charger_state powerUtilGetChargerState(void)
{
    switch (ChargerStatus())
    {
        case TRICKLE_CHARGE:
        case PRE_CHARGE:
            return power_charger_trickle;

        case FAST_CHARGE:
             return power_charger_fast;

        case DISABLED_ERROR:
        case HEADROOM_ERROR:
        case VBAT_OVERVOLT_ERROR:
            return power_charger_disabled;

        case STANDBY:
            return power_charger_complete;

        case NO_POWER:
            return power_charger_disconnected;

        case CONFIG_FAIL_UNKNOWN:
        case CONFIG_FAIL_CHARGER_ENABLED:
        case CONFIG_FAIL_EFUSE_CRC_INVALID:
        case CONFIG_FAIL_EFUSE_TRIMS_ZERO:
        case CONFIG_FAIL_CURRENTS_ZERO:
        case CONFIG_FAIL_VALUES_OUT_OF_RANGE:
        case ENABLE_FAIL_UNKNOWN:
        case ENABLE_FAIL_EFUSE_CRC_INVALID:
        case INTERNAL_CURRENT_SOURCE_CONFIG_FAIL:
            powerUtilFatalError();
            break;

        default:
            break;
    }

    return power->chg_state;
}

/****************************************************************************
NAME
    powerUtilGetThermalConfig

DESCRIPTION
    Get configuration for die temperature management

RETURNS
    void
*/
void powerUtilGetThermalConfig(void)
{
    if (!((PsFullRetrieve(PSKEY_THERMAL_SHUTDOWN_PERIOD, &power->thermal.config.shutdown_period, sizeof(uint16))) &&
          (PsFullRetrieve(PSKEY_THERMAL_SHUTDOWN_LONG_PERIOD, &power->thermal.config.shutdown_long_period, sizeof(uint16))) &&
          (PsFullRetrieve(PSKEY_SHUTDOWN_TEMPERATURE, &power->thermal.config.shutdown_temperature, sizeof(int16))) &&
          (PsFullRetrieve(PSKEY_RAMPDOWN_TEMPERATURE, &power->thermal.config.rampdown_temperature, sizeof(int16))) &&
          (PsFullRetrieve(PSKEY_REENABLE_TEMPERATURE, &power->thermal.config.reenable_temperature, sizeof(int16)))))
    {
        powerUtilFatalError();
    }
}

/****************************************************************************
NAME
    powerUtilGetThermalDieTemperature

DESCRIPTION
    Get die temperature

RETURNS
    int16
*/
int16 powerUtilGetThermalDieTemperature(void)
{
    return VmGetTemperatureBySensor(TSENSOR_PMU);
}

/****************************************************************************
NAME
    powerUtilInitCompleteMask

DESCRIPTION
    Returns the initialisation mask.

RETURNS
    uint16
*/
uint16 powerUtilInitCompleteMask(void)
{
    return (power_init_vref | power_init_vbat | power_init_vthm | power_init_vchg | power_init_chg_prg_mon);
}

/****************************************************************************
NAME
    powerUtilGetAdcReadPeriod

DESCRIPTION
    Returns the period between ADC read requests.

RETURNS
    Delay
*/
Delay powerUtilGetAdcReadPeriod(power_adc* adc)
{
    Delay period;

    if (adc->source == adcsel_chg_mon)
    {
        period = (powerUtilGetChargerState() != power_charger_fast) ? adc->period_no_chg : adc->period_chg;
    }
    else
    {
        period = (powerUtilGetChargerState() == power_charger_disconnected) ? adc->period_no_chg : adc->period_chg;
    }

    return(period);
}

/****************************************************************************
NAME
    powerUtilHandleChargerProgressMonitorReq

DESCRIPTION
    Handles requests for Charger Progress Monitor.

RETURNS
    void
*/
void powerUtilHandleChargerProgressMonitorReq(Task task)
{
    if(!powerMonitorReadAdcNow(task, POWER_INTERNAL_CHG_PRG_MON_REQ, &power->config.chg_prg_mon.adc, power_init_chg_prg_mon))
        powerMonitorReadAdc(task, POWER_INTERNAL_CHG_PRG_MON_REQ, 0);
}

/****************************************************************************
NAME
    powerUtilIsChargerProgressMonitorAdc

DESCRIPTION
    Returns TRUE if ADC is Charger Progress Monitor, FALSE otherwise.

RETURNS
    bool
*/
bool powerUtilIsChargerProgressMonitorAdc(vm_adc_source_type adc_source)
{
    return (adc_source == power->config.chg_prg_mon.adc.source);
}

/****************************************************************************
NAME
    powerUtilAdcRequest

DESCRIPTION
    Returns TRUE if ADC is successful, FALSE otherwise.

RETURNS
    bool
*/
bool powerUtilAdcRequest(Task task, vm_adc_source_type adc_source, bool is_vthm, bool drive_ics, uint16 delay)
{
    vm_adc_extra_flag flags;

    if ((is_vthm) && (drive_ics))
    {
        flags = power->config.vthm.ics;
    }
    else
    {
        flags = ADC_STANDARD;
        delay = 0;
    }

    ADC_PRINT(("POWER: Trp AdcReadRequest(source = %u, flags = %u, delay = %u)\n", adc_source, flags, delay));
    return (AdcReadRequest(task, adc_source, flags, delay));
}


/****************************************************************************
NAME
    powerUtilConditionVchgReading
    
DESCRIPTION
    Condition Vchg reading. Provides an opportunity to correct the Vchg reading
    if required by the H/W.
*/
uint16 powerUtilConditionVchgReading(uint16 vchg_sns_reading)
{
    uint16 conditioned_vchg_reading = vchg_sns_reading;
    
    /*   Only condition Vchg if external current limiting is enabled. Note that if
         CHG_EXT is switched off the current through the external resiator will be
         zero and there won't be a voltage drop (Ohms Law).  */
    
    if(power->config.vchg.ext_fast)
    {
        /*   Calculate the value of CHG_PRG_MON for 1.2V (remember that CHG_PROG_MON is 
             a voltage derived from the charging current and that a value of Vref is
             defined as 100% of the requested current.   */
        
        uint16 chg_prg_mon_max = (MAX_CHG_PRG_MON*VmReadVrefConstant())/ADC_REF_VOLTAGE_IN_MV;
        
        /*   Cap the raw CHG_PRG_MON reading.   */
        
        uint16 chg_prg_mon = (power->chg_prg_mon > chg_prg_mon_max) ? chg_prg_mon_max : power->chg_prg_mon;
            
        /*    Calculate the voltage drop across the external resistor and add to the measured
              value of VCHG_SNS. This will be the actual value of VCHG.   */
            
        conditioned_vchg_reading += 
            (uint16)(((uint32)power->charger_i_actual_fast*chg_prg_mon*power->config.vchg.ext_resistor_value)/
                                                                                                    ((uint32)chg_prg_mon_max*1000));
    }
    
    return(conditioned_vchg_reading);
}

/****************************************************************************
NAME
    powerUtilIsOverVoltage

DESCRIPTION
    This function is called by applications to check whether the battery
    is in Over Voltage.

RETURNS
    bool
*/
bool powerUtilIsOverVoltage(void)
{
    return (ChargerStatus() == VBAT_OVERVOLT_ERROR);
}

/****************************************************************************
NAME
    powerUtilIsBatteryBelowTerminationVoltage

DESCRIPTION
    This function is called by applications to check whether the battery
    voltage is below the termination voltage.

RETURNS
    bool
*/
bool powerUtilIsBatteryBelowTerminationVoltage(uint16 vbat_millivolts)
{
    PRINT(("PM: powerUtilIsBatteryBelowTerminationVoltage() power->vterm is %u, vbat_millivolts is %u\n", power->vterm, vbat_millivolts));
    return (vbat_millivolts < power->vterm);
}

/****************************************************************************
NAME
    powerUtilChargerFullCurrent

DESCRIPTION
    Determines if the charger is (potentially) drawing full current from
    underlying hardware. Can be called before Power library is initialised.

    Full current is drawn when not in trickle charge state.

RETURNS
    TRUE if enabled, FALSE otherwise
*/
bool powerUtilChargerFullCurrent(void)
{
    charger_status status = ChargerStatus();
    return ((status == PRE_CHARGE) || (status == FAST_CHARGE));
}

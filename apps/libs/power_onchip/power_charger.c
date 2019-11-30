/****************************************************************************
Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_charger.c

DESCRIPTION
    This file contains the monitoring of battery charging functionality.

    Once the battery charger is connected, charger is monitored and the
    charging state is repeatly reported to App.

NOTES

**************************************************************************/


/****************************************************************************
    Header files
*/

#include <stdlib.h>
#include <charger.h>
#include <ps.h>
#include <vm.h>

#include "power_onchip.h"
#include "power_private.h"
#include "power_charger.h"
#include "power_init.h"
#include "power_monitor.h"
#include "power_thermal.h"
#include "power_current_limiting.h"
#include "power_utils.h"

/* Make sure these are defined so lib builds on old SDK */
#ifndef CHARGER_USE_EXT_TRAN
#define CHARGER_USE_EXT_TRAN    0x05
#endif
#ifndef CHARGER_TERM_VOLTAGE
#define CHARGER_TERM_VOLTAGE    0x07
#endif

/* See PMU_CHG_VFAST_CTRL for following values. */
#define PMU_CHG_VFAST_CTRL_2V9 0
#define PMU_CHG_VFAST_CTRL_3V0 1
#define PMU_CHG_VFAST_CTRL_3V1 2
#define PMU_CHG_VFAST_CTRL_2V5 3
/* Following values are assumed. */
#define VTERM_TOLERANCE  200
#define VBAT_DISCHARGED 2600
/* Charger Progress Monitor is 13-bits */
#define MAX_CHG_PRG_MON            0x1FFF

#define ONE_PERCENT                           1
#define TRICKLE_PERCENT             ONE_PERCENT

/****************************************************************************
NAME
    powerChargerConfigHelper

DESCRIPTION
    Performs safe configuration for single charger configuration items.

RETURNS
    void
*/
void powerChargerConfigHelper(charger_chg_cfg_key key, uint16 value)
{
    charger_config_key config_key = powerUtilMapChargerKeys(key);

    switch (config_key)
    {
        case CHARGER_KEY_INVALID:
            /* Do not configure this key/value. */
            break;

        case CHARGER_ENABLE:
            /* This would be a coding error. */
            powerUtilFatalError();
            break;

        default:
            powerUtilChargerConfigure(config_key, value);
            break;
    }
}

/****************************************************************************
NAME
    powerChargerSetExternalModeResistor

DESCRIPTION
    Safely configures the external charger mode.

RETURNS
    void
*/
static void powerChargerSetExternalModeResistor(uint16 resistance_milliohms, bool ext_fast)
{
    powerChargerConfigHelper(CHARGER_CONFIG_EXTERNAL_RESISTOR, resistance_milliohms);
    powerChargerConfigHelper(CHARGER_CONFIG_USE_EXTERNAL_RESISTOR_FOR_PRE_CHARGE, FALSE);
    powerChargerConfigHelper(CHARGER_CONFIG_USE_EXTERNAL_RESISTOR_FOR_FAST_CHARGE, ext_fast);
}

/****************************************************************************
NAME
    powerChargerSetStandbyFastHysteresis

DESCRIPTION
    Safely configures the charger satndby-fast voltage hysteresis.

RETURNS
    void
*/
static void powerChargerSetStandbyFastHysteresis(uint16 vhyst)
{
    powerChargerConfigHelper(CHARGER_CONFIG_STANDBY_FAST_HYSTERESIS, vhyst);
}

/****************************************************************************
NAME
    powerChargerSetPreFastThreshold

DESCRIPTION
    Safely configures the charger pre-fast voltage threshold.

RETURNS
    void
*/
static void powerChargerSetPreFastThreshold(uint16 threshold)
{
    powerChargerConfigHelper(CHARGER_CONFIG_PRE_FAST_THRESHOLD, threshold);
}


/****************************************************************************
NAME
    powerChargerSetItermControl

DESCRIPTION
    Safely configures the charger terminal current control.

RETURNS
    void
*/
static void powerChargerSetItermControl(uint16 control)
{
    powerChargerConfigHelper(CHARGER_CONFIG_ITERM_CTRL, control);
}


/****************************************************************************
NAME
    powerChargerSetStateChangeDebounceTimer

DESCRIPTION
    Safely configures the charger state change debounce timer.

RETURNS
    void
*/
static void powerChargerSetStateChangeDebounceTimer(uint16 timer)
{
    powerChargerConfigHelper(CHARGER_CONFIG_STATE_CHANGE_DEBOUNCE, timer);
}


/****************************************************************************
NAME
    PowerOnChipChargerSuppressLed0

DESCRIPTION
    Suppress LED0 whilst charging.

RETURNS
    void
*/
void PowerOnChipChargerSuppressLed0(bool suppress)
{
    powerChargerConfigHelper(CHARGER_CONFIG_SUPPRESS_LED0, suppress);
}

/****************************************************************************
NAME
    powerChargerEnableBoost

DESCRIPTION
    Enables boost charging.

RETURNS
    void
*/
static void powerChargerEnableBoost(bool boost)
{
    powerChargerConfigHelper(CHARGER_CONFIG_ENABLE_BOOST, boost);
}

/****************************************************************************
NAME
    powerChargerUseExtTrans

DESCRIPTION
    Use external transistor for charging.

RETURNS
    void
*/
static void powerChargerUseExtTran(bool use)
{
    powerChargerConfigHelper(CHARGER_CONFIG_USE_EXT_TRAN, use);
}


/****************************************************************************
NAME
    PowerOnChipChargerConfigure

DESCRIPTION
    Initialise power charger
*/
void PowerOnChipChargerConfigure(void)
{

    /* The charger must be properly configured before it can be enabled.
     * The following items are expected not to change after initialisation.
     * Items that can change after initialisation, i.e. charger currents and
     * termination voltages, are set only when the charger is about to be
     * enabled and not here.
     */
    PowerOnChipChargerEnable(FALSE);
    powerChargerSetExternalModeResistor(power->config.vchg.ext_resistor_value, power->config.vchg.ext_fast);
    powerChargerSetPreFastThreshold(power->config.vchg.pre_fast_threshold);
    powerChargerSetItermControl(power->config.vchg.iterm_ctrl);
    powerChargerSetStateChangeDebounceTimer(power->config.vchg.debounce_time);
    powerChargerSetStandbyFastHysteresis(power->config.vchg.standby_fast_hysteresis);
}

/****************************************************************************
NAME
    powerChargerReset

DESCRIPTION
    Turn the charger off and on again

RETURNS
    void
*/
static void powerChargerReset(void)
{
    PowerOnChipChargerEnable(FALSE);
    /* If charger has not been disabled turn it back on */
    if(power->chg_state != power_charger_disabled)
        PowerOnChipChargerEnable(TRUE);
}

/****************************************************************************
NAME
    powerChargerDisableBoost

DESCRIPTION
    Permanently disable internal/external boost until charger is detached

RETURNS
    void
*/
static void powerChargerDisableBoost(void)
{
    if(power->chg_state == power_charger_boost_internal)
    {
        powerChargerEnableBoost(FALSE);
        powerChargerReset();
    }
    else if(power->chg_state == power_charger_boost_external)
    {
        powerChargerUseExtTran(FALSE);
    }
}

/****************************************************************************
NAME
    powerChargerReadingMgr
DESCRIPTION
    Configure charging current and calculate monitoring interval

RETURNS
    uint16
*/
static uint16 powerChargerReadingMgr(void)
{
    uint16  new_charger_current;
    uint16  monitor_period = D_SEC(1);
        /* Load the present value of the charger current setting */
        new_charger_current = power->charger_i_setting;

        /* If the requested current is lower than the setting, reduce immediatley */
        if (new_charger_current > power->charger_i_target)
            new_charger_current = power->charger_i_target;

        /* Check the status of the charger */
        switch (powerUtilGetChargerState())
        {
            case power_charger_disabled:
            case power_charger_complete:
            {
                monitor_period = power->pmu_mon_config.monitor_period_idle;

                /* If the device is not charging, reduce the current to minimum */
                new_charger_current = power->pmu_mon_config.min_charge_i;
                break;
            }
            case power_charger_disconnected:
            {
                monitor_period = power->pmu_mon_config.monitor_period_nopwr;

                /* If the device is not charging, reduce the current to minimum */
                new_charger_current = power->pmu_mon_config.min_charge_i;
                break;
            }
            default:
            {
                int16   temperature;

                monitor_period = power->pmu_mon_config.monitor_period_active;

                /* Measure the PMU temperature */
                temperature = VmGetTemperatureBySensor(TSENSOR_PMU);

                if (temperature == INVALID_SENSOR)
                {
                    /* No PMU temperature sensor - set current now */
                    new_charger_current = power->charger_i_target;
                }
                else if (temperature == INVALID_TEMPERATURE)
                {
                    /* If the temperature could not be read, set the charger current to minimum */
                    new_charger_current = power->pmu_mon_config.min_charge_i;
                }
                else
                {
                    /* Check for the charger current setting being too high
                       or the PMU temperature being too high */
                    if (new_charger_current > power->charger_i_target
                        || temperature >= power->pmu_mon_config.decr_i_temp)
                    {
                        /* Reduce the current:
                           If the setting is larger than the step size, reduce by the step size
                           Else, set the current to minimum
                        */
                        if (new_charger_current >= power->pmu_mon_config.charger_i_step)
                            new_charger_current -= power->pmu_mon_config.charger_i_step;
                        else
                            new_charger_current = power->pmu_mon_config.min_charge_i;

                        /* Range-check the new setting */
                        if (new_charger_current < power->pmu_mon_config.min_charge_i)
                            new_charger_current = power->pmu_mon_config.min_charge_i;
                    }
                    /* Check for the charger current setting being below the target
                       AND that the temperature is below the no-increment temperature */
                    else if (new_charger_current < power->charger_i_target
                        && temperature < power->pmu_mon_config.no_incr_i_temp)
                    {
                        /* Increase current by the step-size */
                        new_charger_current += power->pmu_mon_config.charger_i_step;

                        /* Check that the current did not increase above the target */
                        if (new_charger_current > power->charger_i_target)
                            new_charger_current = power->charger_i_target;

                        /* Range-check the new setting */
                        if (new_charger_current > power->pmu_mon_config.max_charge_i)
                            new_charger_current = power->pmu_mon_config.max_charge_i;
                    }
                }
                break;
            }
        }

        /* Set the new charger current */
        if (new_charger_current != power->charger_i_setting){
            power->charger_i_setting = new_charger_current;
            powerUtilSetChargerCurrents(power->charger_i_setting);
        }
    return(monitor_period);
}

/****************************************************************************
NAME
    powerChargerHandleMonitorReq

DESCRIPTION
    Monitor charger status (and handle boost enable/disable).

RETURNS
    void
*/
void powerChargerHandleMonitorReq(void)
{
    uint16  monitor_period = D_SEC(1);
    power_charger_state new_state,old_state;
    old_state = power->chg_state;
    new_state = powerUtilGetChargerState();

/* Note that state is further updated if >= power_charger_fast */
    if((old_state != new_state)&&((old_state<power_charger_fast)||(new_state<power_charger_fast)||(old_state==power_charger_complete)||(new_state==power_charger_complete)))
    {
        /* if the charger was previously disconnected */
        if (power->chg_state==power_charger_disconnected)
        {
            /* then update the state to say it has just been connected */
            new_state = power_charger_just_connected;
        }
        /* Charger connected/disconnected, restart monitoring and re-enable boost */
        if(power->chg_state == power_charger_disconnected || new_state == power_charger_disconnected)
        {
            /* Wait until init complete to restart monitoring */
            if(POWER_INIT_GET(power_init_cfm_sent))
                powerMonitorRestart();
        }
        if(new_state==power_charger_fast)
        {
            switch(power->chg_boost)
            {
            case power_boost_internal_enabled:
                powerChargerEnableBoost(TRUE);
                new_state=power_charger_boost_internal;
                break;
            case power_boost_external_enabled:
                powerChargerUseExtTran(TRUE);
                PRINT(("power_charger_boost_external\n"));
                new_state=power_charger_boost_external;
                break;
            case power_boost_disabled:
            default:
                powerChargerDisableBoost();
                new_state=power_charger_fast;
            }
        }
        else
        {
            powerChargerDisableBoost();
        }
        power->chg_cycle_complete=chg_cycle_complete;
        power->chg_state = new_state;
    }
    /* Don't send any messages unless init complete */
    if(POWER_INIT_GET(power_init_cfm_sent)&&(power->chg_cycle_complete==chg_cycle_complete))
    {
        MAKE_POWER_MESSAGE(POWER_CHARGER_STATE_IND);
        message->state = new_state;
        PRINT(("POWER: Snd POWER_CHARGER_STATE_IND to app\n"));
        MessageSend(power->clientTask, POWER_CHARGER_STATE_IND, message);
        power->chg_cycle_complete=chg_cycle_notified;
    }

    if (power->pmu_mon_enabled)
    {
        monitor_period=powerChargerReadingMgr();
    }
    else
    {
        /* Original code before charger current control was added */
        if (power->chg_state == power_charger_disconnected)
            monitor_period = 0;
    }

    if (monitor_period)
    {
        CHG_MONITOR_PRINT(("POWER: Que POWER_INTERNAL_CHG_MONITOR_REQ in %u seconds\n", monitor_period / 1000));
        MessageSendLater(&power->task, POWER_INTERNAL_CHG_MONITOR_REQ, NULL, monitor_period);
    }
    else
    {
        PRINT(("POWER: Charger Monitor: STOP MONITORING\n"));
    }
}


/****************************************************************************
NAME
    powerChargerHandleVoltageReading

DESCRIPTION
    Calculate the current charger voltage and manage boost enable/disable.

RETURNS
    void
*/
void powerChargerHandleVoltageReading(uint16 reading)
{
    reading = powerUtilConditionVchgReading(reading);
    
    /*    Determine if Vchg monitoring is required - based on time period.   */

    power->cumulative_non_current_limit_period_chg  += power->config.vchg.adc.period_chg;
    
    if(power->cumulative_non_current_limit_period_chg >= power->config.vchg.non_current_limit_period_chg)
    {
        /*    Get old level and then update the Vchg global data value.   */
        
        bool old_level = powerUtilGetChargerVoltageLevel();
        power->vchg = reading;
    
        if(POWER_INIT_GET(power_init_vchg))
        {
            bool new_level = powerUtilGetChargerVoltageLevel();
            if(old_level != new_level)
            {
                MAKE_POWER_MESSAGE(POWER_CHARGER_VOLTAGE_IND);
                message->vchg.voltage = power->vchg;
                message->vchg.level   = new_level;
                PRINT(("POWER: Snd POWER_CHARGER_VOLTAGE_IND to app\n"));
                MessageSend(power->clientTask, POWER_CHARGER_VOLTAGE_IND, message);
            }
        }
        else
        {
            POWER_INIT_SET(power_init_vchg);
        } 
        
         /*   Zero the cumulative interval.   */
            
        power->cumulative_non_current_limit_period_chg = 0;
    }
    
    PowerCurrentLimitingSendIndication(reading);
}

/****************************************************************************
NAME
    powerChargerDisconnected

DESCRIPTION
    Check if charger is disconnected or connected

RETURNS
    TRUE if disconnected, FALSE if connected
*/
bool powerChargerDisconnected(void)
{
    return powerUtilGetChargerState() == power_charger_disconnected;
}

/****************************************************************************
NAME
    PowerOnChipChargerGetVoltage

DESCRIPTION
    Call this function to get the current charger voltage (in mV)

RETURNS
    bool
*/
bool PowerOnChipChargerGetVoltage(voltage_reading* vchg)
{
    if(POWER_INIT_GET(power_init_vchg))
    {
        vchg->voltage = power->vchg;
        vchg->level   = powerUtilGetChargerVoltageLevel();
        return TRUE;
    }
    return FALSE;
}


/****************************************************************************
NAME
    powerOnChipChargerGetState

DESCRIPTION
    The value pointed to by state will be set to the updated charger state
    unless state is NULL.

RETURNS
    void
*/
void powerOnChipChargerGetState(power_charger_state* state)
{
    /* Set latest state */
    if(state) *state = power->chg_state;
}


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
void PowerOnChipChargerMonitor(void)
{
    /* Restart charger monitoring */
    CHG_MONITOR_PRINT(("POWER: Ccl POWER_INTERNAL_CHG_MONITOR_REQ\n"));
    MessageCancelFirst(&power->task, POWER_INTERNAL_CHG_MONITOR_REQ);
    powerChargerHandleMonitorReq();

    /* Restart thermal monitoring */
    THERMAL_PRINT(("POWER: Ccl POWER_INTERNAL_THERMAL_REQ\n"));
    MessageCancelFirst(&power->task, POWER_INTERNAL_THERMAL_REQ);
    powerThermalHandleMonitorReq();
}


/****************************************************************************
NAME
    PowerOnChipChargerEnable

DESCRIPTION
    Enable/disable the charger.

RETURNS
    void
*/
void PowerOnChipChargerEnable(bool enable)
{
    /* Charger may not be enabled due to thermal considerations. */
    powerUtilChargerEnable(powerThermalUpdateEnable(enable));
    power->charger_enabled = enable;
}

/****************************************************************************
NAME
    PowerOnChipChargerEnabled

DESCRIPTION
    Check if charger is enabled

RETURNS
    TRUE if enabled, FALSE otherwise
*/
bool PowerOnChipChargerEnabled(void)
{
    return powerUtilIsChargerEnabled();
}


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
void PowerOnChipChargerSetCurrent(uint16 current)
{
    power->charger_i_target = current;

    if (power->pmu_mon_enabled){
        if (current < power->charger_i_setting){
            power->charger_i_setting = current;
            powerUtilSetChargerCurrents(current);
        }
    } else {
        powerUtilSetChargerCurrents(current);
    }
}


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
void PowerOnChipChargerSetBoost(power_boost_enable boost)
{
    power->chg_boost = boost;
}


/****************************************************************************
NAME
    PowerOnChipChargerSetVterm

DESCRIPTION
    Reduce the termination voltage. This will fail if requested termination
    voltage is higher than the configured termination voltage.

RETURNS
    bool
*/
bool PowerOnChipChargerSetVterm(uint16 vterm)
{
    powerChargerConfigHelper(CHARGER_CONFIG_TERM_VOLTAGE, (vterm * powerUtilGetVoltageScale()));
    power->vterm = vterm;
    return TRUE;
}

/****************************************************************************
NAME
    PowerOnChipChargerGetVoltageScale

DESCRIPTION
    Gets the voltage scalar for the charger version.

RETURNS
    uint16
*/
uint16 PowerOnChipChargerGetVoltageScale(void)
{
    return powerUtilGetVoltageScale();
}

/****************************************************************************
NAME
    PowerOnChipChargerConnected

DESCRIPTION
    Determines charger connected status from underlying hardware. Can be called
    before Power library is initialised.

RETURNS
    TRUE if enabled, FALSE otherwise
*/
bool PowerOnChipChargerConnected(void)
{
    return (ChargerStatus() != NO_POWER);
}

/****************************************************************************
NAME
    PowerOnChipChargerFullCurrent

DESCRIPTION
    Determines if the charger is (potentially) drawing full current from
    underlying hardware. Can be called before Power library is initialised.

RETURNS
    TRUE if enabled, FALSE otherwise
*/
bool PowerOnChipChargerFullCurrent(void)
{
    return powerUtilChargerFullCurrent();
}

/****************************************************************************
NAME
    PowerOnChipChargerFastCharge

DESCRIPTION
    Determines if the charger is fast charging from underlying hardware.
    Can be called before Power library is initialised.

RETURNS
    TRUE if enabled, FALSE otherwise
*/
bool PowerOnChipChargerFastCharge(void)
{
    return (ChargerStatus() == FAST_CHARGE);
}

/****************************************************************************
NAME
    PowerOnChipChargerDisabled

DESCRIPTION
    Determines if the charger is connected but disabled from underlying hardware.
    Can be called before Power library is initialised.

RETURNS
    TRUE if enabled, FALSE otherwise
*/
bool PowerOnChipChargerDisabled(void)
{
    return (ChargerStatus() == DISABLED_ERROR);
}

/****************************************************************************
NAME
    PowerOnChipChargerRegisterTask

DESCRIPTION
    This task will receive messages when the on-chip charger changes state.

RETURNS
    void
*/
void PowerOnChipChargerRegisterTask(Task task)
{
    MessageChargerTask(task);
}

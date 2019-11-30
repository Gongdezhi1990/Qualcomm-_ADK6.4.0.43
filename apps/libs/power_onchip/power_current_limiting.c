/****************************************************************************
Copyright (c) 2005 - 2018    Qualcomm Technologies International, Ltd.

FILE NAME
    power_current_limiting.c

DESCRIPTION
    This file contains the current determination operation of the battery
    charging functionality.

NOTES

    Current determination starts charging at 500mA and Vchg is measured periodically.
    If Vchg is above the limit threshold then the charging current is increased.
    If charging current reaches its target then current determination is finished.
    If Vchg goes below the limit threshold then the charging current is periodically
    decreased until Vchg goes above the recovery threshold.

**************************************************************************/


/****************************************************************************
    Header files
*/

#include "power_onchip.h"
#include "power_private.h"
#include "power_utils.h"
#include "power_current_limiting.h"
#include "power_charger.h"
#include "power_monitor.h"

typedef enum {
    event_init,
    event_start,
    event_stop,
    event_monitor,
    event_headroom_error,
    event_send_indication
} current_determination_event;

static void powerCurrentLimitingAdjust(int16 current_step)
{
    uint16 new_current = (uint16)((int16)power->charger_i_actual_fast + current_step);

    if(new_current > power->charger_i_safe_fast)
        new_current = power->charger_i_safe_fast;

    if(new_current < INITIAL_FAST_DYNAMIC_CURRENT_IN_MA)
        new_current = INITIAL_FAST_DYNAMIC_CURRENT_IN_MA;
        
    /* Don't change the current unless the charger is already enabled. */

    if((power->charger_enabled == TRUE) && (new_current != power->charger_i_actual_fast))
    {
        power->charger_i_actual_fast = new_current;
        powerChargerConfigHelper(CHARGER_CONFIG_FAST_CURRENT, power->charger_i_actual_fast);
        powerUtilChargerEnable(TRUE); /* Required to latch in the new current setting. */
    }
}

static bool isCurrentDeterminationRequired(void)
{
    return (power->config.vchg.ext_fast) && (power->charger_i_target > INITIAL_FAST_DYNAMIC_CURRENT_IN_MA);
}

static bool isFastCurrentAtInitialFastDynamicCurrentLevel(void)
{
    return (power->charger_i_actual_fast == INITIAL_FAST_DYNAMIC_CURRENT_IN_MA);
}

static void startCurrentLimiting(void)
{
    powerUtilSetChargerCurrents(power->charger_i_target);

    power->config.vchg.adc.period_chg = power->config.vchg.current_limit_period_chg;
    if(!powerMonitorReadAdcNow(&power->task, POWER_INTERNAL_VCHG_REQ, &power->config.vchg.adc, power_init_vchg))
        powerMonitorReadAdc(&power->task, POWER_INTERNAL_VCHG_REQ, 0);
}

static void stopCurrentLimiting(void)
{
    power->config.vchg.adc.period_chg = power->config.vchg.non_current_limit_period_chg;
}

static void currentDetermined(void)
{
    power->config.vchg.adc.period_chg = power->config.vchg.non_current_limit_period_chg;
}

static bool isVchgBelowLimitThreshold(uint16 vchg_reading)
{
    return (vchg_reading < power->config.vchg.current_limit_threshold);
}

static bool isVchgAboveRecoveryThreshold(uint16 vchg_reading)
{
    return (vchg_reading > power->config.vchg.current_limit_recovery_threshold);
}

static bool isTargetCurrentReached(void)
{
    return (power->charger_i_actual_fast == power->charger_i_safe_fast);
}

static void takeRecoveryStep(void)
{
    powerCurrentLimitingAdjust(-(power->config.vchg.negative_current_step));
}

static void takeLimitingStep(void)
{
    powerCurrentLimitingAdjust(power->config.vchg.positive_current_step);
}

static void sendIndication(uint16 vchg_reading)
{
    MAKE_POWER_MESSAGE(POWER_CHARGER_CURRENT_LIMITING_IND);
    message->voltage = vchg_reading;
    PRINT(("POWER: Snd POWER_CHARGER_CURRENT_LIMITING_IND to app with Vchg=%umV\n", vchg_reading));
    MessageSend(power->clientTask, POWER_CHARGER_CURRENT_LIMITING_IND, message);
}

static void handleEvent(current_determination_event event, uint16 vchg_reading)
{
    typedef enum {
        state_determined,
        state_finding_limit_threshold,
        state_finding_recovery_threshold
    } current_determination_state;

    static current_determination_state state;

    switch (state)
    {
    case state_determined:
        switch (event)
        {
        case event_start:
            if (isCurrentDeterminationRequired())
            {
                startCurrentLimiting();
                state = state_finding_limit_threshold;
            }
            break;

        case event_headroom_error:
            if (isCurrentDeterminationRequired())
            {
                takeRecoveryStep();
                stopCurrentLimiting();
            }
            break;

        case event_init:
        case event_stop:
        case event_monitor:
        case event_send_indication:
            break;

        default:
            Panic();
            break;
        }
        break;

    case state_finding_limit_threshold:
        switch (event)
        {
        case event_start:
            if (isCurrentDeterminationRequired())
            {
                startCurrentLimiting();
            }
            break;

        case event_init:
        case event_stop:
            stopCurrentLimiting();
            state = state_determined;
            break;

        case event_monitor:
            if (isTargetCurrentReached())
            {
                currentDetermined();
                state = state_determined;
            }
            else if (isVchgBelowLimitThreshold(vchg_reading))
            {
                takeRecoveryStep();
                state = state_finding_recovery_threshold;
            }
            else
            {
                takeLimitingStep();
            }
            break;

        case event_headroom_error:
            takeRecoveryStep();
            stopCurrentLimiting();
            state = state_determined;
            break;

        case event_send_indication:
            sendIndication(vchg_reading);
            break;

        default:
            Panic();
            break;
        }
        break;

    case state_finding_recovery_threshold:
        switch (event)
        {
        case event_start:
            if (isCurrentDeterminationRequired())
            {
                startCurrentLimiting();
                state = state_finding_limit_threshold;
            }
            break;

        case event_init:
        case event_stop:
            stopCurrentLimiting();
            state = state_determined;
            break;

        case event_monitor:
            if (isVchgAboveRecoveryThreshold(vchg_reading) || isFastCurrentAtInitialFastDynamicCurrentLevel())
            {
                currentDetermined();
                state = state_determined;
            }
            else
            {
                takeRecoveryStep();
            }
            break;

        case event_headroom_error:
            break;

        case event_send_indication:
            sendIndication(vchg_reading);
            break;

        default:
            Panic();
            break;
        }
        break;

    default:
        Panic();
        break;
    }
}

void PowerOnChipCurrentLimitingStart(void)
{
    handleEvent(event_start, 0);
}

void PowerOnChipCurrentLimitingStop(void)
{
    handleEvent(event_stop, 0);
}

void powerCurrentLimitingInit(void)
{
    handleEvent(event_init, 0);
}

void PowerOnChipCurrentLimitingHandleMonitorReq(uint16 vchg_reading)
{
    handleEvent(event_monitor, vchg_reading);
}

void PowerOnChipCurrentLimitingHandleHeadroomError(void)
{
    handleEvent(event_headroom_error, 0);
}

void PowerCurrentLimitingSendIndication(uint16 vchg_reading)
{
    handleEvent(event_send_indication, vchg_reading);
}


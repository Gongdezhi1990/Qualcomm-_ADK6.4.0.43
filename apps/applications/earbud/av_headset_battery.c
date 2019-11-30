/*!
\copyright  Copyright (c) 2008 - 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_battery.c
\brief      Battery monitoring
*/

#include <adc.h>

#include "av_headset.h"
#include "av_headset_battery.h"
#include "av_headset_log.h"
#include "av_headset_test.h"
#include "hydra_macros.h"
#include "panic.h"

/*! Whilst the filter is being filled read at this rate */
#define BATTERY_READ_PERIOD_INITIAL (0)

/*! Enumerated type for messages sent within the headset battery 
    handler only. */
enum headset_battery_internal_messages
{
        /*! Message sent to trigger an intermittent battery measurement */
    MESSAGE_BATTERY_INTERNAL_MEASUREMENT_TRIGGER = 1,

    MESSAGE_BATTERY_TEST_PROCESS_READING = MESSAGE_BATTERY_PROCESS_READING,
};

/*! TRUE when the filter is filled with results */
#define FILTER_IS_FULL(battery) ((battery)->filter.index >= BATTERY_FILTER_LEN)

/*! Add a client to the list of clients */
static bool appBatteryClientAdd(batteryTaskData *battery, batteryRegistrationForm *form)
{
    batteryRegisteredClient *new = calloc(1, sizeof(*new));
    if (new)
    {
        new->form = *form;
        new->next = battery->client_list;
        battery->client_list = new;
        return TRUE;
    }
    return FALSE;
}

/*! Remove a client from the list of clients */
static void appBatteryClientRemove(batteryTaskData *battery, Task task)
{
    batteryRegisteredClient **head;
    for (head = &battery->client_list; *head != NULL; head = &(*head)->next)
    {
        if ((*head)->form.task == task)
        {
            batteryRegisteredClient *toremove = *head;
            *head = (*head)->next;
            free(toremove);
            break;
        }
    }
}

static uint8 toPercentage(uint16 voltage)
{
    uint16 critical = appConfigBatteryVoltageCritical();
    uint16 charged = appConfigBatteryFullyCharged();

    if (voltage < critical)
        voltage = critical;
    else if (voltage > charged)
        voltage = charged;

    return (100UL * (uint32)(voltage - critical)) / (uint32)(charged - critical);
}

static battery_level_state toState(uint16 voltage)
{
    if (FILTER_IS_FULL(appGetBattery()))
    {
        if (voltage < appConfigBatteryVoltageCritical())
            return battery_level_too_low;
        if (voltage < appConfigBatteryVoltageLow())
            return battery_level_critical;
        if (voltage < appConfigBatteryVoltageOk())
            return battery_level_low;

        return battery_level_ok;
    }

    return battery_level_unknown;
}

/* TRUE if the current value is less than the threshold taking into account hysteresis */
static bool ltThreshold(uint16 current, uint16 threshold, uint16 hysteresis)
{
    return current < (threshold - hysteresis);
}

/* TRUE if the current value is greater than the threshold taking into account hysteresis. */
static bool gtThreshold(uint16 current, uint16 threshold, uint16 hysteresis)
{
    return current > (threshold + hysteresis);
}

/* TRUE if the current value is outside the threshold taking into account hysteresis */
static bool thresholdExceeded(uint16 current, uint16 threshold, uint16 hysteresis)
{
    return ltThreshold(current, threshold, hysteresis) ||
           gtThreshold(current, threshold, hysteresis);
}

/* Determine if the transition should be made between last_state and
   new_state. For the transition to happen, the current battery voltage must
   exceed the state defined voltage level by the amount of hysteresis. 

   If the state has jumped two levels, then we assume the level has jumped and
   this is acceptable.
 */
static bool stateUpdateIsRequired(battery_level_state last_state,
                                  battery_level_state new_state,
                                  uint16 voltage,
                                  uint16 hysteresis)
{
    if (   new_state == battery_level_unknown
        || new_state == last_state)
    {
        return FALSE;
    }
    
    if (last_state == battery_level_unknown)
    {
        return TRUE;
    }

    switch (last_state)
    {
        case battery_level_too_low:
            switch (new_state)
            {
                case battery_level_critical:
                    return gtThreshold(voltage, appConfigBatteryVoltageCritical(), hysteresis);
                case battery_level_low:
                case battery_level_ok:
                    return TRUE;
                default:
                    break;
            }
            break;

        case battery_level_critical:
            switch (new_state)
            {
                case battery_level_too_low:
                    return ltThreshold(voltage, appConfigBatteryVoltageCritical(), hysteresis);
                case battery_level_low:
                    return gtThreshold(voltage, appConfigBatteryVoltageLow(), hysteresis);
                case battery_level_ok:
                    return TRUE;
                default:
                    break;
            }
            break;

        case battery_level_low:
            switch (new_state)
            {
                case battery_level_too_low:
                    return TRUE;
                case battery_level_critical:
                    return ltThreshold(voltage, appConfigBatteryVoltageLow(), hysteresis);
                case battery_level_ok:
                    return gtThreshold(voltage, appConfigBatteryVoltageOk(), hysteresis);
                default:
                    break;
            }
            break;

        case battery_level_ok:
            switch (new_state)
            {
                case battery_level_too_low:
                case battery_level_critical:
                    return TRUE;
                case battery_level_low:
                    return ltThreshold(voltage, appConfigBatteryVoltageOk(), hysteresis);
                default:
                    break;
            }
            break;

        default:
            break;
    }
    return FALSE;
}

/*! Iterate through the list of clients, sending battery level messages when
    the representation criteria is met */
static void appBatteryServiceClients(batteryTaskData *battery)
{
    batteryRegisteredClient *client = NULL;
    uint16 voltage = appBatteryGetVoltage();
    for (client = battery->client_list; client != NULL; client = client->next)
    {
        uint16 hysteresis = client->form.hysteresis;
        switch (client->form.representation)
        {
            case battery_level_repres_voltage:
                if (thresholdExceeded(voltage, client->last.voltage, hysteresis))
                {
                    MESSAGE_MAKE(msg, MESSAGE_BATTERY_LEVEL_UPDATE_VOLTAGE_T);
                    msg->voltage_mv = voltage;
                    client->last.voltage = voltage;
                    MessageSend(client->form.task, MESSAGE_BATTERY_LEVEL_UPDATE_VOLTAGE, msg);
                }
            break;
            case battery_level_repres_percent:
            {
                uint8 percent = toPercentage(voltage);
                if (thresholdExceeded(percent, client->last.percent, hysteresis))
                {
                    MESSAGE_MAKE(msg, MESSAGE_BATTERY_LEVEL_UPDATE_PERCENT_T);
                    msg->percent = percent;
                    client->last.percent = percent;
                    MessageSend(client->form.task, MESSAGE_BATTERY_LEVEL_UPDATE_PERCENT, msg);
                }
            }
            break;
            case battery_level_repres_state:
            {
                battery_level_state new_state = toState(voltage);
                if (stateUpdateIsRequired(client->last.state, new_state, voltage, hysteresis))
                {
                    MESSAGE_MAKE(msg, MESSAGE_BATTERY_LEVEL_UPDATE_STATE_T);
                    msg->state = new_state;
                    client->last.state = new_state;
                    MessageSend(client->form.task, MESSAGE_BATTERY_LEVEL_UPDATE_STATE, msg);
                }
            }
            break;
        }
    }
}

static void appBatteryScheduleNextMeasurement(batteryTaskData *battery)
{
    uint32 delay = FILTER_IS_FULL(battery) ?
                        battery->period :
                        BATTERY_READ_PERIOD_INITIAL;
    MessageSendLater(&battery->task, MESSAGE_BATTERY_INTERNAL_MEASUREMENT_TRIGGER,
                        NULL, delay);
}

/*! Return TRUE if a new voltage is available,with enough samples that the result
    should be stable. This waits for the filter to be full. */
static bool appBatteryAdcResultHandler(batteryTaskData *battery, MessageAdcResult* result)
{
    uint16 reading = result->reading;

    switch (result->adc_source)
    {
        case adcsel_pmu_vbat_sns:
        {
            uint32 index = battery->filter.index & BATTERY_FILTER_MASK;
            uint16 vbatt_mv = (uint16)((uint32)VmReadVrefConstant() * reading / battery->vref_raw);

            battery->filter.accumulator -= battery->filter.buf[index];
            battery->filter.buf[index] = vbatt_mv;
            battery->filter.accumulator += vbatt_mv;
            battery->filter.index++;
            /* See the logic in appBatteryGetVoltage():
               0<=index<BATTERY_FILTER_LEN is only used when filling the filter.
               When incrementing to BATTERY_FILTER_LEN, initialisation is complete.
               When wrapping to 0, jump over the initialisation index range. */
            if (battery->filter.index == BATTERY_FILTER_LEN)
            {
                MessageSend(appGetAppTask(), MESSAGE_BATTERY_INIT_CFM, NULL);
            }
            if (battery->filter.index == 0)
            {
                battery->filter.index = BATTERY_FILTER_LEN;
            }
            appBatteryScheduleNextMeasurement(battery);
            return FILTER_IS_FULL(battery);
        }

        case adcsel_vref_hq_buff:
            battery->vref_raw = reading;
            break;

        default:
            DEBUG_LOGF("appBatteryAdcResultHandler unexpected source - %d",result->adc_source);
            break;
    }
    return FALSE;
}

static void appBatteryHandleMessage(Task task, MessageId id, Message message)
{
    batteryTaskData *battery = (batteryTaskData *)task;
    if (battery->period != 0)
    {
        switch (id)
        {
            case MESSAGE_ADC_RESULT:
                if (appBatteryAdcResultHandler(battery, (MessageAdcResult*)message))
                {
                    appBatteryServiceClients(battery);
                }
                break;

            case MESSAGE_BATTERY_TEST_PROCESS_READING:
                appBatteryServiceClients(battery);
                break;

            case MESSAGE_BATTERY_INTERNAL_MEASUREMENT_TRIGGER:
                /* Start immediate battery reading, note vref is read first */
                AdcReadRequest(&battery->task, adcsel_vref_hq_buff, 0, 0);
                AdcReadRequest(&battery->task, adcsel_pmu_vbat_sns, 0, 0);
                break;

            default:
                /* An unexpected message has arrived - must handle it */
                appHandleUnexpected(id);
                break;
        }
    }
}

void appBatteryInit(void)
{
    DEBUG_LOG("appBatteryMonInit");
    batteryTaskData *battery = appGetBattery();
    memset(battery, 0, sizeof(*battery));

    /* Set up task handler */
    battery->task.handler = appBatteryHandleMessage;
    battery->period = appConfigBatteryReadPeriodMs();

    appBatteryScheduleNextMeasurement(battery);
}

void appBatterySetPeriod(uint16 period)
{
    batteryTaskData *battery = appGetBattery();
    DEBUG_LOGF("appBatteryMonPeriod %d", period);
    if (period == 0)
    {
        /* Reset the filter data */
        memset(&battery->filter, 0, sizeof(battery->filter));
    }
    else if (battery->period == 0)
    {
        /* Restart */
        appBatteryScheduleNextMeasurement(battery);
    }
    battery->period = period;
}

uint16 appBatteryGetVoltage(void)
{
    if (appTestBatteryVoltage)
    {
        return appTestBatteryVoltage;
    }
    else
    {
        batteryTaskData *battery = appGetBattery();
        uint16 index = battery->filter.index;
        uint32 accumulator = battery->filter.accumulator;

        if (FILTER_IS_FULL(battery))
        {
            return accumulator / BATTERY_FILTER_LEN;
        }
        return index == 0 ? 0 : accumulator / index;
    }
}

battery_level_state appBatteryGetState(void)
{
    uint16 voltage = appBatteryGetVoltage();

    return toState(voltage);
}


uint8 appBatteryConvertLevelToPercentage(uint16 level_mv)
{
    return toPercentage(level_mv);
}


uint8 appBatteryGetPercent(void)
{
    uint16 voltage_mv = appBatteryGetVoltage();

    return toPercentage(voltage_mv);
}

bool appBatteryRegister(batteryRegistrationForm *client)
{
    batteryTaskData *battery = appGetBattery();
    if (appBatteryClientAdd(battery, client))
    {
        appBatteryServiceClients(battery);
        return TRUE;
    }
    return FALSE;
}

extern void appBatteryUnregister(Task task)
{
    batteryTaskData *battery = appGetBattery();
    appBatteryClientRemove(battery, task);
}

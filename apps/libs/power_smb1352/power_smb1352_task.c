/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352_task.c

DESCRIPTION
    Handle SMB1352 tasks.
*/

#include <panic.h>
#include <pio.h>
#include <stdlib.h>
#include <library.h>
#include <message.h>
#include <power.h>

#include "power_smb1352_interrupt.h"
#include "power_smb1352_task.h"
#include "power_smb1352_config.h"
#include "power_smb1352_debug.h"
#include "power_smb1352_registers.h"
#include "power_smb1352_i2c.h"
#include "power_smb1352_status.h"

#define APSD_TIMEOUT_MS    1500

static void internalMessageHandler(Task task, MessageId id, Message message);

typedef enum {
    APSD_COMPLETE_TIMER
} task_internal_message_type_t;

typedef enum
{
    state_uninitialised,
    state_booting,
    state_normal
} smb1352_state_t;

static smb1352_state_t smb1352_state = state_uninitialised;

const TaskData powerSmb1352TaskInternalTask = {internalMessageHandler};

static Task clientTask;
static Task chargerTask;

static void sendMessagePowerChargerStateInd(power_charger_state state)
{
    POWER_CHARGER_STATE_IND_T *msg = PanicUnlessNew(POWER_CHARGER_STATE_IND_T);
    msg->state = state;
    MessageCancelAll(clientTask, POWER_CHARGER_STATE_IND);
    MessageSend(clientTask, POWER_CHARGER_STATE_IND, msg);
}

static void sendMessageChargerStatus(charger_status status)
{
    MessageChargerStatus *msg = PanicUnlessNew(MessageChargerStatus);
    msg->chg_status = status;
    MessageCancelAll(chargerTask, MESSAGE_CHARGER_STATUS);
    MessageSend(chargerTask, MESSAGE_CHARGER_STATUS, msg);
}

static void apsdStage(void)
{
    smb1352_usb_source_t usb_source;

    if (!powerSmb1352InterruptIsBatteryMissing())
    {
        usb_source = powerSmb1352StatusCheckSourceDetected();

        if (usb_source == dedicated_charging_port)
        {
            powerSmb1352ConfigEnableCharging(FALSE);
            powerSmb1352ConfigEnableAICL(FALSE);
            powerSmb1352ConfigSetAICLTo3A();
        }
    }
}

static void disableCharger(void)
{
    powerSmb1352ConfigEnableCharging(FALSE);
    sendMessageChargerStatus(ENABLE_FAIL_UNKNOWN);
    sendMessagePowerChargerStateInd(power_charger_disabled);
}

static void updateChargingStateWhenWithinLimits(void)
{
    if (powerSmb1352InterruptIsChargingComplete())
    {
        sendMessagePowerChargerStateInd(power_charger_complete);
    }
    else
    {
        sendMessagePowerChargerStateInd(power_charger_fast);
    }
}

static void powerSmb1352TaskFinalBootStage(void)
{
    if (smb1352_state != state_normal)
    {
        PRINT(("SMB1352: Finalising Boot\n"));
        powerSmb1352ConfigControlChargingUsingRegister();
        smb1352_state = state_normal;
        if (powerSmb1352StatusIsCharging())
        {
            sendMessagePowerChargerStateInd(power_charger_fast);
        }

        if (powerSmb1352InterruptIsChargingComplete())
        {
            sendMessagePowerChargerStateInd(power_charger_complete);
        }
    }
}

static void internalMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    switch(id)
    {
        case APSD_COMPLETE_TIMER:
        {
            powerSmb1352InterruptProcessAll();

            if (powerSmb1352InterruptIsAPSDComplete())
            {
                apsdStage();
                powerSmb1352TaskFinalBootStage();
                powerSmb1352ConfigEnableCharging(!powerSmb1352InterruptIsBatteryMissing());
            }
            break;
        }

        default:
            Panic();
            break;
    }
}

void powerSmb1352TaskBoot(Task client)
{
    PRINT(("SMB1352: Booting\n"));

    clientTask = client;

    smb1352_state = state_booting;
    powerSmb1352ConfigInitialiseRegisters();
    powerSmb1352ConfigEnableAICL(TRUE);
    MessageSendLater((Task)&powerSmb1352TaskInternalTask, APSD_COMPLETE_TIMER, NULL, APSD_TIMEOUT_MS);
}


void powerSmb1352TaskPowerOk(bool pok)
{
    if (pok)
    {
        powerSmb1352ConfigEnableAICL(TRUE);
        MessageSendLater((Task)&powerSmb1352TaskInternalTask, APSD_COMPLETE_TIMER, NULL, APSD_TIMEOUT_MS);
    }
    else
    {
        sendMessageChargerStatus(NO_POWER);
        sendMessagePowerChargerStateInd(power_charger_disconnected);
    }
}

bool powerSmb1352TaskIsPowerOk(void)
{
    return powerSmb1352InterruptIsPowerOkay();
}

void powerSmb1352TaskChargingComplete(bool complete)
{
    if (complete)
    {
        sendMessageChargerStatus(STANDBY);
        sendMessagePowerChargerStateInd(power_charger_complete);
    }
}

void powerSmb1352TaskPreToFastChargeThreshold(bool exceeded)
{
    if (exceeded)
    {
        sendMessageChargerStatus(FAST_CHARGE);
        sendMessagePowerChargerStateInd(power_charger_fast);
    }
}

void powerSmb1352TaskTaperCharging(bool taper)
{
    if (taper)
    {
        sendMessageChargerStatus(FAST_CHARGE);
        sendMessagePowerChargerStateInd(power_charger_fast);
    }
}

void powerSmb1352TaskBatteryOV(bool ov)
{
    if (ov)
    {
        sendMessageChargerStatus(VBAT_OVERVOLT_ERROR);
        sendMessagePowerChargerStateInd(power_charger_disabled);
    }
}

void powerSmb1352TaskAICLComplete(bool complete)
{
    if (complete)
    {
        disableCharger();
    }
}

void powerSmb1352TaskChargerError(bool charger_error)
{
    if (charger_error)
    {
        disableCharger();
    }
}

void powerSmb1352TaskPreToFastChargeTimeout(bool timeout)
{
    if (timeout)
    {
        disableCharger();
    }
}

void powerSmb1352TaskCompleteChargeTimeout(bool timeout)
{
    if (timeout)
    {
        disableCharger();
    }
}

void powerSmb1352TaskBatteryThermPinMissing(bool therm_pin_missing)
{
    if (therm_pin_missing)
    {
        disableCharger();
    }
}

void powerSmb1352TaskBatteryTerminalRemoved(bool terminal_removed)
{
    if (terminal_removed)
    {
        disableCharger();
    }
}

void powerSmb1352TaskBatteryTemperatureHardLimitExceeded(bool limit_exceeded)
{
    if (limit_exceeded)
    {
        sendMessagePowerChargerStateInd(power_charger_disabled);
    }
    else
    {
        updateChargingStateWhenWithinLimits();
    }
}

void powerSmb1352TaskBatteryTemperatureSoftLimitExceeded(bool limit_exceeded)
{
    if (!limit_exceeded)
    {
        updateChargingStateWhenWithinLimits();
    }
}

void powerSmb1352TaskRegisterChargerTask(Task task)
{
    chargerTask = task;
}

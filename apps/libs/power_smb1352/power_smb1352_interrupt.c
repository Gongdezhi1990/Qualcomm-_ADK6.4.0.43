/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352_interrupt.c

DESCRIPTION
    Handle SMB1352 interrupts.
*/
#include <vmtypes.h>

#include "power_smb1352_interrupt.h"
#include "power_smb1352_task.h"
#include "power_smb1352_debug.h"
#include "power_smb1352_registers.h"
#include "power_smb1352_i2c.h"


#ifdef DEBUG_POWER_SMB1352_ENABLED
#define DEBUG_INTERRUPT_PRINT(x) POWER_SMB1352_PRINT(("SMB1352: %32s: IRQ %u: Status %u\n", x, register_status.irq_register, register_status.status_register))
#else
#define DEBUG_INTERRUPT_PRINT(x) UNUSED(x)
#endif

typedef struct
{
    bool irq_register;
    bool status_register;
} register_status_t;

typedef void (*task_event_handler) (bool);

static uint8 status_register_cache[SMB1352_NUMBER_OF_STATUS_REGISTERS - 1];
static bool cache_valid = FALSE;

static void populateStatusRegisterCache(void)
{
    uint8 status_register_address = SMB1352_BASE_STATUS_REGISTER;

    powerSmb1352I2cContinuousRead(status_register_address, SMB1352_NUMBER_OF_STATUS_REGISTERS, status_register_cache);

    cache_valid = TRUE;
}

static register_status_t processCachedInterrupt(uint8 addr, uint8 irq_mask, uint8 status_mask)
{
    register_status_t register_status;

    if (!cache_valid)
        populateStatusRegisterCache();

    register_status.irq_register = (status_register_cache[addr - SMB1352_BASE_STATUS_REGISTER] & irq_mask) ? TRUE : FALSE;
    register_status.status_register = (status_register_cache[addr - SMB1352_BASE_STATUS_REGISTER] & status_mask) ? TRUE : FALSE;

    return register_status;
}

static bool getStatusRegister(uint8 addr, uint8 irq_mask, uint8 status_mask)
{
    register_status_t register_status;
    register_status = processCachedInterrupt(addr, irq_mask, status_mask);
    return register_status.status_register;
}

static void processInterrupt(uint8 addr, uint8 irq_mask, uint8 status_mask, task_event_handler handler, char *name)
{
    register_status_t register_status = processCachedInterrupt(addr, irq_mask, status_mask);

    if (handler)
    {
        if (register_status.irq_register)
        {
            handler(register_status.status_register);
        }
    }

    DEBUG_INTERRUPT_PRINT(name);
}

static void processBatteryColdSoftLimitInterrupt(void)
{
    processInterrupt(SMB1352_BATTERY_COLD_SOFT_LIMIT_REG_ADDR,
                     SMB1352_BATTERY_COLD_SOFT_LIMIT_INTERRUPT_MASK,
                     SMB1352_BATTERY_COLD_SOFT_LIMIT_STATUS_MASK,
                     &powerSmb1352TaskBatteryTemperatureSoftLimitExceeded,
                     "Battery Cold Soft");
}

static void processBatteryHotSoftLimitInterrupt(void)
{
    processInterrupt(SMB1352_BATTERY_HOT_SOFT_LIMIT_REG_ADDR,
                     SMB1352_BATTERY_HOT_SOFT_LIMIT_INTERRUPT_MASK,
                     SMB1352_BATTERY_HOT_SOFT_LIMIT_STATUS_MASK,
                     &powerSmb1352TaskBatteryTemperatureSoftLimitExceeded,
                     "Battery Hot Soft");
}

static void processBatteryColdHardLimitInterrupt(void)
{
    processInterrupt(SMB1352_BATTERY_COLD_HARD_LIMIT_REG_ADDR,
                     SMB1352_BATTERY_COLD_HARD_LIMIT_INTERRUPT_MASK,
                     SMB1352_BATTERY_COLD_HARD_LIMIT_STATUS_MASK,
                     &powerSmb1352TaskBatteryTemperatureHardLimitExceeded,
                     "Battery Cold Hard");
}

static void processBatteryHotHardLimitInterrupt(void)
{
    processInterrupt(SMB1352_BATTERY_HOT_HARD_LIMIT_REG_ADDR,
                     SMB1352_BATTERY_HOT_HARD_LIMIT_INTERRUPT_MASK,
                     SMB1352_BATTERY_HOT_HARD_LIMIT_STATUS_MASK,
                     &powerSmb1352TaskBatteryTemperatureHardLimitExceeded,
                     "Battery Hot Hard");
}

static void processInternalTemperatureLimitReachedInterrupt(void)
{
    processInterrupt(SMB1352_INTERNAL_TEMPERATURE_LIMIT_REACHED_REG_ADDR,
                     SMB1352_INTERNAL_TEMPERATURE_LIMIT_REACHED_INTERRUPT_MASK,
                     SMB1352_INTERNAL_TEMPERATURE_LIMIT_REACHED_STATUS_MASK,
                     NULL,
                     "Int. Temp. Limit Reached");
}

static void processBatteryLowInterrupt(void)
{
    processInterrupt(SMB1352_BATTERY_LOW_REG_ADDR,
                     SMB1352_BATTERY_LOW_INTERRUPT_MASK,
                     SMB1352_BATTERY_LOW_STATUS_MASK,
                     NULL,
                     "Battery Low");
}

static void processBatteryThermPinMissingInterrupt(void)
{
    processInterrupt(SMB1352_BATTERY_THERM_PIN_MISSING_REG_ADDR,
                     SMB1352_BATTERY_THERM_PIN_MISSING_INTERRUPT_MASK,
                     SMB1352_BATTERY_THERM_PIN_MISSING_STATUS_MASK,
                     &powerSmb1352TaskBatteryThermPinMissing,
                     "Battery THERM Pin Missing");
}

static void processBatteryTerminalRemovedInterrupt(void)
{
    processInterrupt(SMB1352_BATTERY_TERMINAL_REMOVED_REG_ADDR,
                     SMB1352_BATTERY_TERMINAL_REMOVED_INTERRUPT_MASK,
                     SMB1352_BATTERY_TERMINAL_REMOVED_STATUS_MASK,
                     &powerSmb1352TaskBatteryTerminalRemoved,
                     "Battery Terminal Removed Missing");
}

static void processChargingCompleteInterrupt(void)
{
    processInterrupt(SMB1352_CHARGING_COMPLETE_REG_ADDR,
                     SMB1352_CHARGING_COMPLETE_INTERRUPT_MASK,
                     SMB1352_CHARGING_COMPLETE_STATUS_MASK,
                     &powerSmb1352TaskChargingComplete,
                     "Charging Complete");
}

static void processAutomaticRechargeInterrupt(void)
{
    processInterrupt(SMB1352_AUTOMATIC_RECHARGE_REG_ADDR,
                     SMB1352_AUTOMATIC_RECHARGE_INTERRUPT_MASK,
                     SMB1352_AUTOMATIC_RECHARGE_STATUS_MASK,
                     NULL,
                     "Automatic Recharge");
}

static void processPreToFastChargeThresholdInterrupt(void)
{
    processInterrupt(SMB1352_PRE_TO_FAST_CHARGE_THRESHOLD_REG_ADDR,
                     SMB1352_PRE_TO_FAST_CHARGE_THRESHOLD_INTERRUPT_MASK,
                     SMB1352_PRE_TO_FAST_CHARGE_THRESHOLD_STATUS_MASK,
                     &powerSmb1352TaskPreToFastChargeThreshold,
                     "Pre to Fast Charge Thres.");
}

static void processPreToFastChargeTimeoutInterrupt(void)
{
    processInterrupt(SMB1352_PRE_CHARGE_TIMEOUT_REG_ADDR,
                     SMB1352_PRE_CHARGE_TIMEOUT_INTERRUPT_MASK,
                     SMB1352_PRE_CHARGE_TIMEOUT_STATUS_MASK,
                     &powerSmb1352TaskPreToFastChargeTimeout,
                     "Pre to Fast Charge Timeout");
}

static void processTaperChargingInterrupt(void)
{
    processInterrupt(SMB1352_TAPER_CHARGING_REG_ADDR,
                     SMB1352_TAPER_CHARGING_INTERRUPT_MASK,
                     SMB1352_TAPER_CHARGING_STATUS_MASK,
                     &powerSmb1352TaskTaperCharging,
                     "Taper Charging");
}

static void processCompleteChargeTimeoutInterrupt(void)
{
    processInterrupt(SMB1352_COMPLETE_CHARGE_TIMEOUT_REG_ADDR,
                     SMB1352_COMPLETE_CHARGE_TIMEOUT_INTERRUPT_MASK,
                     SMB1352_COMPLETE_CHARGE_TIMEOUT_STATUS_MASK,
                     &powerSmb1352TaskCompleteChargeTimeout,
                     "Complete Charge Timeout");
}

static void processChargeErrorInterrupt(void)
{
    processInterrupt(SMB1352_CHARGE_ERROR_REG_ADDR,
                     SMB1352_CHARGE_ERROR_INTERRUPT_MASK,
                     SMB1352_CHARGE_ERROR_STATUS_MASK,
                     &powerSmb1352TaskChargerError,
                     "Charge Error");
}

static void processBatteryOVInterrupt(void)
{
    processInterrupt(SMB1352_BATTERY_OV_REG_ADDR,
                     SMB1352_BATTERY_OV_INTERRUPT_MASK,
                     SMB1352_BATTERY_OV_STATUS_MASK,
                     &powerSmb1352TaskBatteryOV,
                     "Battery OV");
}

static void processPowerOkInterrupt(void)
{
    processInterrupt(SMB1352_POWER_OK_REG_ADDR,
                     SMB1352_POWER_OK_INTERRUPT_MASK,
                     SMB1352_POWER_OK_STATUS_MASK,
                     &powerSmb1352TaskPowerOk,
                     "Power OK");
}

static void processAFVCActiveInterrupt(void)
{
    processInterrupt(SMB1352_AFVC_ACTIVE_REG_ADDR,
                     SMB1352_AFVC_ACTIVE_INTERRUPT_MASK,
                     SMB1352_AFVC_ACTIVE_STATUS_MASK,
                     NULL,
                     "AFVC Active");
}

static void processInputUVLOInterrupt(void)
{
    processInterrupt(SMB1352_INPUT_UVLO_REG_ADDR,
                     SMB1352_INPUT_UVLO_INTERRUPT_MASK,
                     SMB1352_INPUT_UVLO_STATUS_MASK,
                     NULL,
                     "Input UVLO");
}

static void processInputOVLOInterrupt(void)
{
    processInterrupt(SMB1352_INPUT_OVLO_REG_ADDR,
                     SMB1352_INPUT_OVLO_INTERRUPT_MASK,
                     SMB1352_INPUT_OVLO_STATUS_MASK,
                     NULL,
                     "Input OVLO");
}

static void processChargeInhibitInterrupt(void)
{
    processInterrupt(SMB1352_CHARGER_INHIBIT_REG_ADDR,
                     SMB1352_CHARGER_INHIBIT_INTERRUPT_MASK,
                     SMB1352_CHARGER_INHIBIT_STATUS_MASK,
                     NULL,
                     "Charger Inhibit");
}

static void processAICLFailedInterrupt(void)
{
    processInterrupt(SMB1352_AICL_FAILED_REG_ADDR,
                     SMB1352_AICL_FAILED_INTERRUPT_MASK,
                     SMB1352_AICL_FAILED_STATUS_MASK,
                     NULL,
                     "AICL Failed");
}

static void processAICLDoneInterrupt(void)
{
    processInterrupt(SMB1352_AICL_DONE_REG_ADDR,
                     SMB1352_AICL_DONE_INTERRUPT_MASK,
                     SMB1352_AICL_DONE_STATUS_MASK,
                     &powerSmb1352TaskAICLComplete,
                     "AICL Done");
}

static void processAPSDCompleteInterrupt(void)
{
    processInterrupt(SMB1352_APSD_COMPLETE_REG_ADDR,
                     SMB1352_APSD_COMPLETE_INTERRUPT_MASK,
                     SMB1352_APSD_COMPLETE_STATUS_MASK,
                     NULL,
                     "APSD Complete");
}

static void processQC3AuthAlgorithmDoneInterrupt(void)
{
    processInterrupt(SMB1352_QC3_AUTH_ALOGORITHM_DONE_REG_ADDR,
                     SMB1352_QC3_AUTH_ALOGORITHM_DONE_INTERRUPT_MASK,
                     SMB1352_QC3_AUTH_ALOGORITHM_DONE_STATUS_MASK,
                     NULL,
                     "QC3 Auth Alg Done");
}


void powerSmb1352InterruptProcessAll(void)
{
    populateStatusRegisterCache();

    processBatteryColdSoftLimitInterrupt();
    processBatteryHotSoftLimitInterrupt();
    processBatteryColdHardLimitInterrupt();
    processBatteryHotHardLimitInterrupt();
    processInternalTemperatureLimitReachedInterrupt();
    processBatteryLowInterrupt();
    processBatteryThermPinMissingInterrupt();
    processBatteryTerminalRemovedInterrupt();
    processChargingCompleteInterrupt();
    processAutomaticRechargeInterrupt();
    processPreToFastChargeThresholdInterrupt();
    processPreToFastChargeTimeoutInterrupt();
    processTaperChargingInterrupt();
    processCompleteChargeTimeoutInterrupt();
    processChargeErrorInterrupt();
    processBatteryOVInterrupt();
    processPowerOkInterrupt();
    processAFVCActiveInterrupt();
    processInputUVLOInterrupt();
    processInputOVLOInterrupt();
    processChargeInhibitInterrupt();
    processAICLFailedInterrupt();
    processAICLDoneInterrupt();
    processAPSDCompleteInterrupt();
    processQC3AuthAlgorithmDoneInterrupt();
}

bool powerSmb1352InterruptIsBatteryMissing(void)
{
    return getStatusRegister(SMB1352_BATTERY_MISSING_STATUS_REG_ADDR,
                             SMB1352_BATTERY_MISSING_INTERRUPT_MASK,
                             SMB1352_BATTERY_MISSING_STATUS_MASK);
}

bool powerSmb1352InterruptIsPowerOkay(void)
{
    return getStatusRegister(SMB1352_POWER_OK_REG_ADDR,
                             SMB1352_POWER_OK_INTERRUPT_MASK,
                             SMB1352_POWER_OK_STATUS_MASK);
}

bool powerSmb1352InterruptIsChargingComplete(void)
{
    return getStatusRegister(SMB1352_CHARGING_COMPLETE_REG_ADDR,
                             SMB1352_CHARGING_COMPLETE_INTERRUPT_MASK,
                             SMB1352_CHARGING_COMPLETE_STATUS_MASK);
}

bool powerSmb1352InterruptIsAPSDComplete(void)
{
    return getStatusRegister(SMB1352_APSD_COMPLETE_REG_ADDR,
                             SMB1352_APSD_COMPLETE_INTERRUPT_MASK,
                             SMB1352_APSD_COMPLETE_STATUS_MASK);
}

/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352_config.c

DESCRIPTION
    Configure the registers on the SMB1352
*/

#include <stdlib.h>
#include <panic.h>

#include "power_smb1352_registers.h"
#include "power_smb1352_config.h"
#include "power_smb1352_config_read.h"
#include "power_smb1352_i2c.h"

/******************************************************************************
NAME
    enableQ3InConfigData

DESCRIPTION
    Enable QC3.0 detection algorithm and auto increment
*/
static void enableQ3InConfigData(power_smb_1352_config_t *config_data)
{
    *(config_data->configuration_registers + SMB1352_QC3_CONFIG_REG_ADDR) |= SMB1352_QC3_AUTO_INC_CONFIG_MASK;
    *(config_data->configuration_registers + SMB1352_QC3_CONFIG_REG_ADDR) |= SMB1352_QC3_ALG_AUTH_CONFIG_MASK;
}

/******************************************************************************
NAME
    setVsysMinInConfigData

DESCRIPTION
    Set minimum system voltage to 3.6V
*/
static void setVsysMinInConfigData(power_smb_1352_config_t *config_data)
{
    *(config_data->configuration_registers + SMB1352_MIN_SYS_VOLTAGE_REG_ADDR) |= SMB1352_MIN_SYS_VOLTAGE_CONFIG_MASK;
}

/******************************************************************************
NAME
    enableBmdUsingThermInConfigData

DESCRIPTION
    Use therm pin to determine if the battery is missing
*/
static void enableBmdUsingThermInConfigData(power_smb_1352_config_t *config_data)
{
    *(config_data->configuration_registers + SMB1352_BATTERY_MISSING_CONFIG_REG_ADDR) |= SMB1352_BATT_MISSING_THERM_CONFIG_MASK;
}

/******************************************************************************
NAME
    enableIrqSources

DESCRIPTION
    Enables interrupts to be generated from power okay, battery missing and
    QC3.0 being detected
*/
static void enableIrqSources(power_smb_1352_config_t *config_data)
{
    *(config_data->configuration_registers + SMB1352_STATUS_INTERRUPT_REGISTER_REG_ADDR) |= SMB1352_BMD_INT_CONFIG_MASK;
    *(config_data->configuration_registers + SMB1352_STATUS_INTERRUPT_REGISTER_REG_ADDR) |= SMB1352_POK_INT_CONFIG_MASK;
    *(config_data->configuration_registers + SMB1352_QC3_AUTH_REG_ADDR) |= SMB1352_QC3_AUTH_INT_CONFIG_MASK;
}

/******************************************************************************
NAME
    enableQuickChargeAdaptors

DESCRIPTION
    Enables the quick charge Adaptors
*/
static void enableQuickChargeAdaptors(power_smb_1352_config_t *config_data)
{
    *(config_data->configuration_registers + SMB1352_HVDCP_CONFIG_REG_ADDR) |= SMB1352_HVDCP_CONFIG_MASK;
}

/******************************************************************************
NAME
    overwriteConfigValuesWithDefaultValues

DESCRIPTION
    Overwrites the config data with values essential to correct operation
*/
static void overwriteConfigValuesWithDefaultValues(power_smb_1352_config_t *config_data)
{
    enableQ3InConfigData(config_data);
    setVsysMinInConfigData(config_data);
    enableBmdUsingThermInConfigData(config_data);
    enableIrqSources(config_data);
    enableQuickChargeAdaptors(config_data);
}

/******************************************************************************
NAME
    writeConfigDataToSmb1352

DESCRIPTION
    Writes the config data to the SMBB1352 over I2C
*/
static void writeConfigDataToSmb1352(power_smb_1352_config_t *config_data)
{
    uint8 reg;

    for(reg = 0; reg < SMB1352_NUMBER_OF_CONFIGURATION_REGISTERS; reg++)
        powerSmb1352I2cWrite(reg, *(config_data->configuration_registers + reg));
}

/******************************************************************************
NAME
    powerSmb1352ConfigEnableCharging

DESCRIPTION
    Enable/ Disable charging on the SMB1352
*/
void powerSmb1352ConfigEnableCharging(bool enable)
{
    uint8 data;

    powerSmb1352I2cRead(SMB1352_CHG_COMMAND_REG_ADDR, &data);

    powerSmb1352I2cVolatileWriteEnable(TRUE);

    if (enable)
        data |= SMB1352_ENABLE_CHARGING_COMMAND_MASK;
    else
    {
        data &= (uint8)~(SMB1352_ENABLE_CHARGING_COMMAND_MASK);
    }

    powerSmb1352I2cWrite(SMB1352_CHG_COMMAND_REG_ADDR, data);

    powerSmb1352I2cVolatileWriteEnable(FALSE);
}

/******************************************************************************
NAME
    powerSmb1352ConfigControlChargingUsingRegister

DESCRIPTION
    Configure charging control by register value
*/
void powerSmb1352ConfigControlChargingUsingRegister(void)
{
    uint8 data;

    powerSmb1352I2cRead(SMB1352_PIN_AND_ENABLE_CONTROL_REG_ADDR, &data);

    powerSmb1352I2cVolatileWriteEnable(TRUE);

    data |= SMB1352_CHARGER_EN_REG_ACTIVE_HIGH_CONFIG_MASK;

    powerSmb1352I2cWrite(SMB1352_PIN_AND_ENABLE_CONTROL_REG_ADDR, data);

    powerSmb1352I2cVolatileWriteEnable(FALSE);
}

/******************************************************************************
NAME
    powerSmb1352ConfigEnableAICL

DESCRIPTION
    Enable/ disable automatic input current limit
*/
void powerSmb1352ConfigEnableAICL(bool enable)
{
    uint8 data;

    powerSmb1352I2cRead(SMB1352_AICL_CONFIG_REG_ADDR, &data);

    powerSmb1352I2cVolatileWriteEnable(TRUE);
    if (enable)
        data |= SMB1352_AICL_CONFIG_MASK;
    else
    {
        data &= (uint8)~(SMB1352_AICL_CONFIG_MASK);
    }

    powerSmb1352I2cWrite(SMB1352_AICL_CONFIG_REG_ADDR, data);
    powerSmb1352I2cVolatileWriteEnable(FALSE);
}

/******************************************************************************
NAME
    powerSmb1352ConfigSetAICLTo3A

DESCRIPTION
    Sets the automatic input current limit to 3A
*/
void powerSmb1352ConfigSetAICLTo3A(void)
{

    uint8 data;

    powerSmb1352I2cRead(SMB1352_CURRENT_CONFIG_REG_ADDR, &data);

    data |= SMB1352_ICL_3A_CONFIG_MASK;

    powerSmb1352I2cVolatileWriteEnable(TRUE);

    powerSmb1352I2cWrite(SMB1352_CURRENT_CONFIG_REG_ADDR, data);

    powerSmb1352I2cVolatileWriteEnable(FALSE);
}


/******************************************************************************
NAME
    powerSmb1352ConfigInitialiseRegisters

DESCRIPTION
    Initialise the SMB1352 registers
*/
void powerSmb1352ConfigInitialiseRegisters(void)
{
    power_smb_1352_config_t * config_data = PanicUnlessMalloc(sizeof(power_smb_1352_config_t));

    powerSmb1352ConfigReadPopulateData(config_data);
    overwriteConfigValuesWithDefaultValues(config_data);
    powerSmb1352I2cVolatileWriteEnable(TRUE);
    writeConfigDataToSmb1352(config_data);
    powerSmb1352I2cVolatileWriteEnable(FALSE);
    free(config_data);
}

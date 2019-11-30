/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352_i2c.c

DESCRIPTION
    Handle SMB1352 i2c access.
*/
#include <stdlib.h>
#include <i2c.h>
#include <panic.h>

#include "power_smb1352_i2c.h"
#include "power_smb1352_registers.h"

#define I2C_READ_BYTES_TO_ACK_OVERHEAD   3
#define I2C_READ_TX_LENGTH      1
#define I2C_READ_RX_LENGTH      1

#define I2C_WRITE_BYTES_TO_ACK_OVERHEAD  1
#define I2C_WRITE_TX_LENGTH     2
#define I2C_WRITE_RX_LENGTH     0

#define SMB1352_DEFAULT_ADDRESS    0xAE

void powerSmb1352I2cRead(uint8 reg, uint8 *data)
{
    uint16 acknowledged = I2cTransfer(SMB1352_DEFAULT_ADDRESS, &reg, I2C_READ_TX_LENGTH, data, I2C_READ_RX_LENGTH);

    PanicFalse(acknowledged == I2C_READ_RX_LENGTH + I2C_READ_BYTES_TO_ACK_OVERHEAD);
}

void powerSmb1352I2cContinuousRead(uint8 reg, uint8 number_of_registers, uint8 *data)
{
    uint16 acknowledged = I2cTransfer(SMB1352_DEFAULT_ADDRESS, &reg, I2C_READ_TX_LENGTH, data, number_of_registers);

    PanicFalse(acknowledged == number_of_registers + I2C_READ_BYTES_TO_ACK_OVERHEAD);
}

void powerSmb1352I2cWrite(uint8 reg, uint8 data)
{
    uint8 packet[2];
    uint16 acknowledged;

    packet[0] = reg;
    packet[1] = data;
    acknowledged = I2cTransfer(SMB1352_DEFAULT_ADDRESS, packet, I2C_WRITE_TX_LENGTH, NULL, I2C_WRITE_RX_LENGTH);

    PanicFalse(acknowledged == I2C_WRITE_TX_LENGTH + I2C_WRITE_BYTES_TO_ACK_OVERHEAD);
}

void powerSmb1352I2cVolatileWriteEnable(bool enable)
{
    powerSmb1352I2cWrite(SMB1352_I2C_COMMAND_REG_ADDR, enable ? SMB1352_ENABLE_VOLATILE_WRITES_COMMAND_MASK : 0);
}

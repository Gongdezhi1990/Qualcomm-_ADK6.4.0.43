/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352_i2c.h

DESCRIPTION
    Handle SMB1352 i2c access.
*/

#ifndef POWER_SMB1352_I2C_H_
#define POWER_SMB1352_I2C_H_

/****************************************************************************
NAME
    powerSmb1352I2cRead

DESCRIPTION
    Read register from the SMB1352

RETURNS
    void
*/
void powerSmb1352I2cRead(uint8 reg, uint8 *data);

/****************************************************************************
NAME
    powerSmb1352I2cContinuousRead

DESCRIPTION
    Do a continous read of the registers from the SMB1352

RETURNS
    void
*/
void powerSmb1352I2cContinuousRead(uint8 reg, uint8 number_of_registers, uint8 *data);

/****************************************************************************
NAME
    powerSmb1352I2cWrite

DESCRIPTION
    Write to a register on the SMB1352

RETURNS
    void
*/

void powerSmb1352I2cWrite(uint8 reg, uint8 data);

/****************************************************************************
NAME
    powerSmb1352I2cVolatileWriteEnable

DESCRIPTION
    Enable / disable volatile writes to the SMB1352

RETURNS
    void
*/

void powerSmb1352I2cVolatileWriteEnable(bool enable);

#endif /* POWER_SMB1352_I2C_H_ */

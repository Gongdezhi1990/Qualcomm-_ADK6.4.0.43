#ifndef __I2C_H__
#define __I2C_H__

/*! file  @brief Access to the i2c bus 
** 
** 
BlueCore can support one master i2c interface.
\note
I2C PIOs in BlueCore are configured with PSKEY_I2C_SDA_PIO & PSKEY_I2C_SCL_PIO if PS storage is not I2C EEPROM. The default value of I2C PIOs are defined in device datasheet. The BlueCore acts as the sole bus master. The speed of the bus (standard mode, i.e. up to 100kHz or fast mode, i.e. up to 400kHz) is configured with PSKEY_I2C_CONFIG.
\par 
Only 7bit slave addresses are explicitly supported. However, 10bit addresses can be used by splitting the 10bit address between the slave address (11110xx) and first data byte as defined by the I2C specification.
\par 
Use of the I2C bus requires pull-up resistors connected to the respective PIOs. Details of how to choose such resistors can be found in the I2C-Bus Specification, and recommended values for a typical design are listed in the BlueCore databooks.
*/

#if TRAPSET_IIC

/**
 *  \brief Allows transfer of data across the i2c interface. 
 *      Perform a composite transfer consisting of a sequence of writes followed
 *  by a sequence of reads. These will be separated by a repeated start condition
 *  (Sr) and slave address if both reads and writes are performed.
 *      The general sequence of operations performed is:
 *      - Start condition (S).
 *      - Write slave address and direction byte (address | 0).
 *      - Write \e tx_len data bytes from the buffer at \e tx.
 *      - Repeated start condition (Sr) if \e tx_len and \e rx_len are non-zero.
 *      - Write slave address and direction byte (address | 1) if \e tx_len and \e
 *  rx_len are non-zero.
 *      - Read \e rx_len data bytes into the buffer at \e rx, acknowledging all
 *  but the final byte.
 *      - Stop condition (P).
 *      If \e tx_len is non-zero and \e rx_len is zero then the sequence reduces
 *  to:
 *      - Start condition (S).
 *      - Write slave address and direction byte (address | 0).
 *      - Write \e tx_len data bytes from the buffer at \e tx.
 *      - Stop condition (P).
 *      Alternatively, if \e tx_len is zero and \e rx_len is non-zero then the
 *  sequence reduces to:
 *      - Start condition (S).
 *      - Write slave address and direction byte (address | 1).
 *      - Read \e rx_len data bytes, acknowledging all but the final byte.
 *      - Stop condition (P).
 *      Finally, if both \e tx_len and \e rx_len are zero then the following
 *  minimal sequence is used:
 *      - Start condition (S).
 *      - Write slave address and direction byte (address | 1).
 *      - Stop condition (P).
 *      The transfer will be aborted if either the slave address or a byte being
 *  written is not acknowledged. The stop condition (P) will still be driven to
 *  reset the bus.
 *      Note also that the address is not shifted before being combined with the
 *  direction bit, i.e. the slave address should occupy bits 7 to 1 of address.
 *  This allows the R/W bit to be forced to 1 for composite write/read transfers
 *  without a repeated start condition (Sr).
 *      If either \e tx_len or \e rx_len exceeds 64, failure may be returned or
 *  performance may be impaired; for instance, audio streaming may be disrupted.
 *  \param address The device address of the slave for this transfer. 
 *  \param tx  A pointer to the data we wish to transmit. One octet per word. 
 *  \param tx_len  The length of the data we wish to transmit. 
 *  \param rx  A pointer to the memory we wish to read received data into. One octet per
 *  word. 
 *  \param rx_len  The length of the data we wish to receive. 
 *  \return The number of bytes acknowledged including the address bytes and the final data
 *  byte (which isn't strictly acknowledged).
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_iic
 */
uint16 I2cTransfer(uint16 address, const uint8 * tx, uint16 tx_len, uint8 * rx, uint16 rx_len);
#endif /* TRAPSET_IIC */
#endif

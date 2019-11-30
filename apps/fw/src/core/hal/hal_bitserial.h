/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Bitserial hardware abstraction layer header file.
 * This file contains the interface abstraction to the bitserial hardware.
 */

#ifndef HAL_BITSERIAL_H
#define HAL_BITSERIAL_H

#if (defined(PROC_INSTALL_BITSERIAL) && defined(CHIP_INSTALL_BITSERIAL)) || defined(DOXYGEN_ONLY)
#define INSTALL_BITSERIAL
#endif

#ifdef INSTALL_BITSERIAL

#include "hydra/hydra_types.h"
#include "hydra/hydra_macros.h"
#define IO_DEFS_MODULE_BITSERIAL
#define IO_DEFS_MODULE_BITSERIAL0
#define IO_DEFS_MODULE_BITSERIAL1
#define IO_DEFS_MODULE_HOST_SYS_CONFIG
#define IO_DEFS_MODULE_APPS_SYS_PIO
#include "io/io_map.h"
#include "hal/hal_macros.h"
#include "mmu/mmu.h"

#define HAL_BITSERIAL_HOST_SS_FREQ_KHZ 32000

/**
 * Define for the underflow byte in slave mode. This is sent when there is no 
 * data queued out for transmission but the master is clocking data out. 
 */
#define SLAVE_UNDERFLOW_BYTE 0xff

/**
 * Bitserial instance type definition. This is used to indicate which bitserial
 * is used. 
 */
typedef enum bitserial_instance
{
#if HAVE_NUMBER_OF_BITSERIALS > 0
    BITSERIAL_INSTANCE_0 = 0x00,
#endif
#if HAVE_NUMBER_OF_BITSERIALS > 1
    BITSERIAL_INSTANCE_1 = 0x01,
#endif
    BITSERIAL_INSTANCE_NULL = 0xff
}bitserial_instance;

/** Helper macro for converting bitserial indices to instances */
#define bitserial_get_instance_from_index(index) \
            ((index==0)? BITSERIAL_INSTANCE_0 \
                       :((index==1)? BITSERIAL_INSTANCE_1 \
                                   :BITSERIAL_INSTANCE_NULL))

/**
 * Type definition for bitserial events(interrupts). 
 */
typedef enum bitserial_events
{
    BITSERIAL_EVENT_TX_SUCCESS_MASK = 1<<BITSERIAL0_EVENT_TX_SUCCESS,
    BITSERIAL_EVENT_RX_SUCCESS_MASK = 1<<BITSERIAL0_EVENT_RX_SUCCESS,
    BITSERIAL_EVENT_ERROR_IN_BUFFER_MASK = 1<<BITSERIAL0_EVENT_ERROR_IN_BUFFER,
    BITSERIAL_EVENT_ERROR_IN_READ_MASK = 1<<BITSERIAL0_EVENT_ERROR_IN_READ,
    BITSERIAL_EVENT_SLAVE_START_STOP_BITS_ERROR_MASK = 1<<BITSERIAL0_EVENT_SLAVE_START_STOP_BITS_ERROR,
    BITSERIAL_EVENT_FIFO_UNDERFLOW_MASK = 1<<BITSERIAL0_EVENT_FIFO_UNDERFLOW,
    BITSERIAL_EVENT_FIFO_OVERFLOW_MASK = 1<<BITSERIAL0_EVENT_FIFO_OVERFLOW,
    BITSERIAL_EVENT_I2C_ACK_ERROR_MASK = 1<<BITSERIAL0_EVENT_I2C_ACK_ERROR,
    BITSERIAL_EVENT_I2C_LOST_ARB_MASK = 1<<BITSERIAL0_EVENT_I2C_LOST_ARB,
    BITSERIAL_EVENT_I2C_NAK_STOP_MASK = 1<<BITSERIAL0_EVENT_I2C_NAK_STOP,
    BITSERIAL_EVENT_I2C_NAK_RESTART_MASK = 1<<BITSERIAL0_EVENT_I2C_NAK_RESTART,
    BITSERIAL_EVENT_TX_NOT_CONFIGURED_MASK = 1<<BITSERIAL0_EVENT_TX_NOT_CONFIGURED,
    BITSERIAL_EVENT_RX_CMD_DETECTED_MASK = 1<<BITSERIAL0_EVENT_RX_CMD_DETECTED,
    BITSERIAL_EVENT_TX_STREAMING_SUCCESS = 1<<BITSERIAL0_EVENT_TX_STREAMING_SUCCESS
}bitserial_events;

/**
 * Type definition for bitserial status bits.
 */
typedef enum bitserial_status_bits
{
    BITSERIAL_BUSY_MASK = BITSERIAL0_BUSY_MASK,
    BITSERIAL_RWB_MASK = BITSERIAL0_RWB_MASK,
    BITSERIAL_EVENT_MASK = BITSERIAL0_EVENT_MASK,
    BITSERIAL_FAILED_MASK = BITSERIAL0_FAILED_MASK,
    BITSERIAL_I2C_LOST_ARBITRATION_MASK = BITSERIAL0_I2C_LOST_ARBITRATION_MASK,
    BITSERIAL_I2C_BUS_BUSY_MASK = BITSERIAL0_I2C_BUS_BUSY_MASK,
    BITSERIAL_PART_2_MASK = BITSERIAL0_PART_2_MASK,
    BITSERIAL_FIFO_OVERFLOW_MASK = BITSERIAL0_FIFO_OVERFLOW_MASK,
    BITSERIAL_FIFO_UNDERFLOW_MASK = BITSERIAL0_FIFO_UNDERFLOW_MASK,
    BITSERIAL_SLAVE_START_STOP_BITS_ERROR_MASK = BITSERIAL0_SLAVE_START_STOP_BITS_ERROR_MASK,
    BITSERIAL_I2C_NACK_MASK = BITSERIAL0_I2C_NACK_MASK,
    BITSERIAL_DEEP_SLEEP_CDC_BUSY_MASK = BITSERIAL0_DEEP_SLEEP_CDC_BUSY_MASK,
    BITSERIAL_TX_NOT_CONFIGURED_MASK = BITSERIAL0_TX_NOT_CONFIGURED_MASK
}bitserial_status_bits;

/**
 * Handy macro for referring to all bitserial interrupts.
 */
#define BITSERIAL_EVENT_ALL_MASK (BITSERIAL_EVENT_TX_SUCCESS_MASK | \
                                  BITSERIAL_EVENT_RX_SUCCESS_MASK | \
                                  BITSERIAL_EVENT_ERROR_IN_BUFFER_MASK | \
                                  BITSERIAL_EVENT_ERROR_IN_READ_MASK | \
                                  BITSERIAL_EVENT_SLAVE_START_STOP_BITS_ERROR_MASK | \
                                  BITSERIAL_EVENT_FIFO_UNDERFLOW_MASK | \
                                  BITSERIAL_EVENT_FIFO_OVERFLOW_MASK | \
                                  BITSERIAL_EVENT_I2C_ACK_ERROR_MASK | \
                                  BITSERIAL_EVENT_I2C_LOST_ARB_MASK | \
                                  BITSERIAL_EVENT_I2C_NAK_STOP_MASK | \
                                  BITSERIAL_EVENT_I2C_NAK_RESTART_MASK | \
                                  BITSERIAL_EVENT_TX_NOT_CONFIGURED_MASK | \
                                  BITSERIAL_EVENT_RX_CMD_DETECTED_MASK | \
                                  BITSERIAL_EVENT_TX_STREAMING_SUCCESS)

/**
 * Type definition for basic read/write operations. Operations like
 * BITSERIAL_READ_WRITE_MODE refer to the first and second part of the COMBO
 * mode.
 */
typedef enum
{
    BITSERIAL_WRITE_MODE = 0,
    BITSERIAL_WRITE_WRITE_MODE = 0,
    BITSERIAL_READ_MODE = 1,
    BITSERIAL_READ_WRITE_MODE = 1,
    BITSERIAL_WRITE_READ_MODE = 2,
    BITSERIAL_READ_READ_MODE = 3
}bitserial_rw_modes;

#ifdef HYDRA_BITSERIAL
/**
 * Helper macro for enabling interrupts.
 */
#define bitserial_block_int() block_interrupts()
/**
 * Helper macro for disabling interrupts.
 */
#define bitserial_unblock_int() unblock_interrupts()
#else /* HYDRA_BITSERIAL */
/**
 * Helper macro for enabling interrupts.
 */
#define bitserial_block_int(); ATOMIC_BLOCK_START {
/**
 * Helper macro for disabling interrupts.
 */
#define bitserial_unblock_int(); } ATOMIC_BLOCK_END;
#endif /* HYDRA_BITSERIAL */

#ifdef HYDRA_BITSERIAL
/**
 * Helper macro for signalling when bitserial is to be enabled or disabled. 
 * Please note that this refers to enabling and disabling the HW which can be
 * done when going idle (no pending transactions or ops).
 */
#define bitserial_clock_enable(e) /*if e == false we can go to sleep*/
/**
 * This was defined for legacy chips but it's not used here.
 */
#define bitserial_update_indices()
#else /* HYDRA_BITSERIAL */
/**
 * Helper macro for signalling when bitserial is to be enabled or disabled. 
 * Please note that this refers to enabling and disabling the HW which can be
 * done when going idle (no pending transactions or ops).
 */
#define bitserial_clock_enable(e)   ATOMIC_BLOCK_START \
     { dorm_sleep_info(DORMID_BITSERIAL, e?DORM_STATE_NO_DEEP:DORM_STATE_ANY);\
       hal_set_clkgen_bitserial_en(e); } ATOMIC_BLOCK_END
 /**
 * This is defined for Hawkeye.
 */
#define bitserial_update_indices()
#endif /* HYDRA_BITSERIAL */

/**
 * Enables the interrupt for a bitserial instance and registers a handler.
 * \param i Bitserial instance whose interrupt will be enabled.
 * \param h Function pointer to the interrupt handler.
 */
void hal_bitserial_int_enable(bitserial_instance i, 
                                               void(*h)(bitserial_instance i));

/**
 * Disables the interrupt for a bitserial instance.
 * \param i Bitserial instance whose interrupt will be disabled.
 */
void hal_bitserial_int_disable(bitserial_instance i);

/**
 * Enables a software interrupt and registers a handler.
 * \param h Function pointer to the interrupt handler.
 */
void hal_bitserial_swint_enable(void (*h)(void));

/**
 * Disables the software interrupt.
 */
void hal_bitserial_swint_disable(void);

/**
 * Raises the software interrupt.
 */
void hal_bitserial_raise_swint(void);

/**
 * Abstraction for setting the interrupt enable register.
 * \param i Bitserial instance.
 * \param v Mask of interrupts to be enabled.
 */
void hal_bitserial_interrupt_en_set(bitserial_instance i, uint16 v);

/**
 * Abstraction for getting the interrupt enable register.
 * \param i Bitserial instance.
 * \return Mask of enabled interrupts.
 */
uint16 hal_bitserial_interrupt_en_get(bitserial_instance i);

/**
 * Sets the TX and RX buffers in HW. The first two are for RX, TX and the first
 * part of the COMBO operation and the later are only for the second part of 
 * the COMBO operation.
 * \param i Bitserial instance.
 * \param rdh1 Handle of the MMU buffer from where data is to be read by HW;
 *               this is used in TX and COMBO operations.
 * \param wrh1 Handle of the MMU buffer to where data is to be written by HW;
 *               this is used in RX and COMBO operations.
 * \param rdh2 Handle of the MMU buffer from where data is to be read by HW;
 *               this is used in TX and COMBO operations.
 * \param wrh2 Handle of the MMU buffer to where data is to be written by HW;
 *               this is used in RX and COMBO operations.
 */
void hal_bitserial_buffers_set(bitserial_instance i, mmu_handle rdh1, 
                            mmu_handle wrh1, mmu_handle rdh2, mmu_handle wrh2);

/**
 * Sets miscellaneous registers. This is present to facilitate handling various
 * functionalities that are platform dependant.
 * \param i Bitserial instance.
 */
void hal_bitserial_set_misc(bitserial_instance i);

/**
 * Abstraction for setting data output PIO.
 * \param i Bitserial instance.
 * \param v Pin number
 */
void hal_bitserial_data_output_pio_set(bitserial_instance i, uint8 v);

/**
 * Abstraction for setting clk output PIO.
 * \param i Bitserial instance.
 * \param v Pin number
 */
void hal_bitserial_clk_output_pio_set(bitserial_instance i, uint8 v);

/**
 * Abstraction for setting sel output PIO.
 * \param i Bitserial instance.
 * \param v Pin number
 */
void hal_bitserial_sel_output_pio_set(bitserial_instance i, uint8 v);

/**
 * Abstraction for setting data input PIO.
 * \param i Bitserial instance.
 * \param v Pin number
 */
void hal_bitserial_data_input_pio_set(bitserial_instance i, uint8 v);

/**
 * Abstraction for setting clk input PIO.
 * \param i Bitserial instance.
 * \param v Pin number
 */
void hal_bitserial_clk_input_pio_set(bitserial_instance i, uint8 v);

/**
 * Abstraction for setting sel input PIO.
 * \param i Bitserial instance.
 * \param v Pin number
 */
void hal_bitserial_sel_input_pio_set(bitserial_instance i, uint8 v);

/**
 * Abstraction for enabling/disabling the bitserial.
 * \param i Bitserial instance.
 * \param v On/Off flag.
 */
void hal_bitserial_enable_set(bitserial_instance i, bool v);

/**
 * Abstraction for setting the bitserial config register.
 * \param i Bitserial instance.
 * \param v Value to set.
 */
void hal_bitserial_config_set(bitserial_instance i, uint16 v);

/**
 * Abstraction for getting the bitserial config register.
 * \param i Bitserial instance.
 * \return Register value.
 */
uint16 hal_bitserial_config_get(bitserial_instance i);

/**
 * Abstraction for setting the bitserial config2 register.
 * \param i Bitserial instance.
 * \param v Value to set.
 */
void hal_bitserial_config2_set(bitserial_instance i, uint16 v);

/**
 * Abstraction for setting the bitserial config3 register.
 * \param i Bitserial instance.
 * \param v Value to set.
 */
void hal_bitserial_config3_set(bitserial_instance i, uint16 v);

/**
 * Abstraction for getting the bitserial config2 register.
 * \param i Bitserial instance.
 * \return Register value.
 */
uint16 hal_bitserial_config2_get(bitserial_instance i);

/**
 * Abstraction for setting the bitserial salve underflow byte.
 * \param i Bitserial instance.
 * \param v Value to set.
 */
void hal_bitserial_slave_underflow_byte_set(bitserial_instance i, uint8 v);

/**
 * Abstraction for setting the bitserial word config register.
 * \param i Bitserial instance.
 * \param v Value to set.
 */
void hal_bitserial_word_config_set(bitserial_instance i, uint16 v);

/**
 * Returns number of bytes per a word by reading WORD_CONFIG register
 * and decoding value of WORD_CONFIG_NUM_BYTES.
 * \param i Bitserial instance.
 * \return Number of bytes per word.
 */
uint8 hal_bitserial_bytes_per_word_get(bitserial_instance i);

/**
 * Abstraction for getting the bitserial word config register.
 * \param i Bitserial instance.
 * \return Register value.
 */
uint16 hal_bitserial_word_config_get(bitserial_instance i);

/**
 * Abstraction for setting the bitserial interbyte spacing register.
 * \param i Bitserial instance.
 * \param v Value to set.
 */
void hal_bitserial_interbyte_spacing_set(bitserial_instance i, uint16 v);

/**
 * Abstraction for setting the bitserial select offset registers.
 *
 * Sets offset between:
 * a) Select being asserted and the 1st bit
 * b) last bit and Select being deasserted
 *
 * 16-bit LSB is written into BITSERIAL_CONFIG_SEL_TIME
 * (16-bit MSB)&0xf is written into BITSERIAL_CONFIG_SEL_TIME2.
 * \param i Bitserial instance.
 * \param v Value to set.
 */
void hal_bitserial_sel_time_set(bitserial_instance i, uint32 v);

/**
 * Abstraction for setting the bitserial clock rate registers.
 * \param i Bitserial instance.
 * \param hi Value to set to the HI register.
 * \param lo Value to set to the LO register.
 */
void hal_bitserial_clk_rate_set(bitserial_instance i, uint16 hi, uint16 lo);

/**
 * Abstraction for setting the bitserial sample offset register.
 * \param i Bitserial instance.
 * \param v Value to set.
 */
void hal_bitserial_clk_samp_offset_set(bitserial_instance i, uint16 v);

/**
 * Abstraction for setting the bitserial i2c address.
 * \param i Bitserial instance.
 * \param v Address to set.
 */
void hal_bitserial_i2c_address_set(bitserial_instance i, uint16 v);

/**
 * Abstraction for clearing the bitserial sticky ACK.
 * \param i Bitserial instance.
 */
void hal_bitserial_clear_sticky_ack(bitserial_instance i);

/**
 * Abstraction for getting the bitserial interrupts.
 * \param i Bitserial instance.
 * \return Events bit mask.
 */
bitserial_events hal_bitserial_events_get(bitserial_instance i);

/**
 * Abstraction for getting the bitserial event status bit mask. This informs 
 * us if we have interrupts.
 * \param i Bitserial instance.
 * \return Event bit mask if set.
 */
uint16 hal_bitserial_event_get(bitserial_instance i);

/**
 * Abstraction for getting the bitserial status bits.
 * \param i Bitserial instance.
 * \return Bitserial status register value.
 */
uint16 hal_bitserial_status_get(bitserial_instance i);

/**
 * Abstraction for getting the bitserial error status. This decodes
 * reason for VML-related errors.
 * \param i Bitserial instance.
 * \return Error status register value.
 */
uint16 hal_bitserial_error_status_get(bitserial_instance i);

/**
 * Abstraction for getting the bitserial part2 status bit mask. This informs 
 * us if the event applies to the second part of a COMBO sequence.
 * \param i Bitserial instance.
 * \return Part2 bit mask if set.
 */
uint16 hal_bitserial_part_2_get(bitserial_instance i);

/**
 * Abstraction for getting the bitserial failed status bit mask. This informs 
 * us if we have encountered problems.
 * \param i Bitserial instance.
 * \return Failed bit mask if set.
 */
uint16 hal_bitserial_failed_get(bitserial_instance i);

/**
 * Abstraction for clearing the bitserial events.
 * \param i Bitserial instance.
 * \param event_mask Bit mask of events to clear.
 */
void hal_bitserial_event_clear(bitserial_instance i, uint32 event_mask);

/**
 * Abstraction for control bitserial clock forcing enable.
 * \param i Bitserial instance.
 * \param enable If TRUE clock forcing is enabled.
 */
void hal_bitserial_clk_force_enable(bitserial_instance i, bool enable);

/**
 * Abstraction for getting the bitserial busy status.
 * \param i Bitserial instance.
 */
bool hal_bitserial_busy_get(bitserial_instance i);

/**
 * Abstraction for getting the last 15 ACKs.
 * \param i Bitserial instance.
 * \return Bit mask of last 15 ACKs.
 */
uint16 hal_bitserial_i2c_acks_get(bitserial_instance i);

/**
 * Abstraction for enabling the bitserial clock.
 * \param i Bitserial instance.
 * \param v On/Off flag.
 */
void hal_bitserial_clock_enable_set(bitserial_instance i, bool v);

/**
 * Abstraction for setting the bitserial rwb register.
 * \param i Bitserial instance.
 * \param v Read/Write/Combo sequence identifier.
 */
void hal_bitserial_rwb_set(bitserial_instance i, bitserial_rw_modes v);

/**
 * Abstraction for setting the bitserial TX/RX length.
 * \param i Bitserial instance.
 * \param v Size of data.
 */
void hal_bitserial_txrx_length_set(bitserial_instance i, uint16 v);

/**
 * Abstraction for setting the bitserial TX/RX length for the second part of 
 * the COMBO sequence.
 * \param i Bitserial instance.
 * \param v Size of data.
 */
void hal_bitserial_txrx_length2_set(bitserial_instance i, uint16 v);

/**
 * Abstraction for setting the bitserial num_protocol_words register.
 * \param i Bitserial instance.
 * \param v Size of data.
 */
void hal_bitserial_num_protocol_words_set(bitserial_instance i, uint16 v);

/**
 * Abstraction for setting the bitserial slave_read_command_byte register.
 * \param i Bitserial instance.
 * \param v Size of data.
 */
void hal_bitserial_slave_read_command_byte_set(bitserial_instance i, uint16 v);
/**
 * Abstraction for getting the number of words read from the MMU by the slave.
 * \param i Bitserial instance.
 * \return Number of words read.
 */ 
uint16 hal_bitserial_slave_read_get(bitserial_instance i);

/**
 * Abstraction for getting the number of words written to the MMU by the slave.
 * \param i Bitserial instance.
 * \return Number of words written.
 */ 
uint16 hal_bitserial_slave_write_get(bitserial_instance i);

/**
 * Abstraction for getting the clock control settings.
 * \param i Bitserial instance.
 * \return Clock control settings.
 */ 
uint16 hal_bitserial_clk_control_get(bitserial_instance i);

/**
 * Abstraction for setting the clock control settings.
 * \param i Bitserial instance.
 * \param v Clock control settings.
 */
void hal_bitserial_clk_control_set(bitserial_instance i, uint16 v);

/**
 * Abstraction for getting the debug signal mux.
 * \param i Bitserial instance.
 * \return Mux value.
 */
uint16 hal_bitserial_debug_sel_get(bitserial_instance i);

/**
 * Abstraction for setting the debug signal mux.
 * \param i Bitserial instance.
 * \param v Mux value.
 */
void hal_bitserial_debug_sel_set(bitserial_instance i, uint16 v);

/**
 * Abstraction for setting the slave debug signal mux.
 * \param i Bitserial instance.
 * \param v Mux value.
 */
void hal_bitserial_slave_debug_sel_set(bitserial_instance i, uint16 v);


/* Abstracts hardware dependant constants */
#define HAL_BITSERIAL_COMBO_MODE_MASK BITSERIAL0_CONFIG2_COMBO_MODE_MASK
#define HAL_BITSERIAL_CONFIG_INT_EVENT_SUBSYSTEM_EN_MASK \
                                  BITSERIAL0_CONFIG_INT_EVENT_SUBSYSTEM_EN_MASK
#define HAL_BITSERIAL_CONFIG2_STOP_TOKEN_DISABLE2_MASK \
                                    BITSERIAL0_CONFIG2_STOP_TOKEN_DISABLE2_MASK
#define HAL_BITSERIAL_CONFIG_STOP_TOKEN_DISABLE_MASK \
                                      BITSERIAL0_CONFIG_STOP_TOKEN_DISABLE_MASK
#define HAL_BITSERIAL_CONFIG_NEG_EDGE_SAMP_EN_MASK \
                                        BITSERIAL0_CONFIG_NEG_EDGE_SAMP_EN_MASK
#define HAL_BITSERIAL_CONFIG_POSEDGE_LAUNCH_MODE_EN_MASK \
                                  BITSERIAL0_CONFIG_POSEDGE_LAUNCH_MODE_EN_MASK
#define HAL_BITSERIAL_CONFIG_CLK_INVERT_MASK BITSERIAL0_CONFIG_CLK_INVERT_MASK
#define HAL_BITSERIAL_CONFIG_DIN_INVERT_MASK BITSERIAL0_CONFIG_DIN_INVERT_MASK
#define HAL_BITSERIAL_CONFIG_DOUT_INVERT_MASK BITSERIAL0_CONFIG_DOUT_INVERT_MASK
#define HAL_BITSERIAL_CONFIG_BYTESWAP_EN_MASK BITSERIAL0_CONFIG_BYTESWAP_EN_MASK
#define HAL_BITSERIAL_CONFIG_BITREVERSE_EN_MASK \
                                            BITSERIAL0_CONFIG_BITREVERSE_EN_MASK
#define HAL_BITSERIAL_CONFIG_SEL_TIME_EN_MASK BITSERIAL0_CONFIG_SEL_TIME_EN_MASK
#define HAL_BITSERIAL_CONFIG_SEL_EN_MASK BITSERIAL0_CONFIG_SEL_EN_MASK
#define HAL_BITSERIAL_CONFIG_SEL_INVERT_MASK BITSERIAL0_CONFIG_SEL_INVERT_MASK
#define HAL_BITSERIAL_CONFIG2_SEL_EN2_MASK BITSERIAL0_CONFIG2_SEL_EN2_MASK
#define HAL_BITSERIAL_CONFIG2_SLAVE_MODE_MASK \
                                             BITSERIAL0_CONFIG2_SLAVE_MODE_MASK
#define HAL_BITSERIAL_INTERBYTE_SPACING_EN_MASK \
                                            BITSERIAL0_INTERBYTE_SPACING_EN_MASK
#define HAL_BITSERIAL_WORD_CONFIG_NUM_BYTES_POSN \
                                       BITSERIAL0_WORD_CONFIG_NUM_BYTES_LSB_POSN
#define HAL_BITSERIAL_WORD_CONFIG_NUM_BYTES_ONE \
                                     (BITSERIAL0_WORD_CONFIG_NUM_BYTES_ONE << \
                                      HAL_BITSERIAL_WORD_CONFIG_NUM_BYTES_POSN)
#define HAL_BITSERIAL_WORD_CONFIG_NUM_BYTES_TWO \
                                     (BITSERIAL0_WORD_CONFIG_NUM_BYTES_TWO << \
                                      HAL_BITSERIAL_WORD_CONFIG_NUM_BYTES_POSN)
#define HAL_BITSERIAL_WORD_CONFIG_NUM_BYTES_THREE \
                                   (BITSERIAL0_WORD_CONFIG_NUM_BYTES_THREE << \
                                    HAL_BITSERIAL_WORD_CONFIG_NUM_BYTES_POSN)
#define HAL_BITSERIAL_WORD_CONFIG_NUM_BYTES_FOUR \
                                    (BITSERIAL0_WORD_CONFIG_NUM_BYTES_FOUR << \
                                     HAL_BITSERIAL_WORD_CONFIG_NUM_BYTES_POSN)
#define HAL_BITSERIAL_WORD_CONFIG_NUM_BYTES_MASK \
                                (BITSERIAL0_WORD_CONFIG_NUM_BYTES_LSB_MASK | \
                                 BITSERIAL0_WORD_CONFIG_NUM_BYTES_MSB_MASK)
#define HAL_BITSERIAL_WORD_CONFIG_NUM_START_POS \
               BITSERIAL0_WORD_CONFIG_BITSERIAL0_WORD_CONFIG_NUM_START_LSB_POSN
#define HAL_BITSERIAL_WORD_CONFIG_START_BITS_POS \
              BITSERIAL0_WORD_CONFIG_BITSERIAL0_WORD_CONFIG_START_BITS_LSB_POSN
#define HAL_BITSERIAL_WORD_CONFIG_NUM_STOP_POS \
                BITSERIAL0_WORD_CONFIG_BITSERIAL0_WORD_CONFIG_NUM_STOP_LSB_POSN
#define HAL_BITSERIAL_WORD_CONFIG_STOP_BITS_POS \
               BITSERIAL0_WORD_CONFIG_BITSERIAL0_WORD_CONFIG_STOP_BITS_LSB_POSN
#define HAL_BITSERIAL_CONFIG2_DATA_READY_WORD_DISABLE_MASK \
                                BITSERIAL0_CONFIG2_DATA_READY_WORD_DISABLE_MASK
#define HAL_BITSERIAL_CONFIG2_SLAVE_READ_MODE_SWITCH_MANUAL \
                               BITSERIAL0_CONFIG2_SLAVE_READ_MODE_SWITCH_MANUAL
#define HAL_BITSERIAL_CONFIG2_SLAVE_READ_MODE_SWITCH_LSB_POSN \
                                 BITSERIAL0_CONFIG2_SLAVE_READ_MODE_SWITCH_POSN
#define HAL_BITSERIAL_CONFIG_I2C_MODE_EN_MASK \
                                             BITSERIAL0_CONFIG_I2C_MODE_EN_MASK
#define HAL_BITSERIAL_CONFIG_CLK_OFFSET_EN_MASK \
                                           BITSERIAL0_CONFIG_CLK_OFFSET_EN_MASK
#define HAL_BITSERIAL_CONFIG2_COMBO_MODE_MASK \
                                             BITSERIAL0_CONFIG2_COMBO_MODE_MASK
#define HAL_BITSERIAL_CONFIG2_DOUT_IDLE_SEL_LOW \
                                          BITSERIAL0_CONFIG2_DOUT_IDLE_SEL_LOW
#define HAL_BITSERIAL_CONFIG2_DOUT_IDLE_SEL_HIGH \
                                          BITSERIAL0_CONFIG2_DOUT_IDLE_SEL_HIGH
#define HAL_BITSERIAL_CONFIG2_DOUT_IDLE_SEL_LAST \
                                          BITSERIAL0_CONFIG2_DOUT_IDLE_SEL_LAST
#define HAL_BITSERIAL_CONFIG2_DOUT_IDLE_SEL_LSB_POSN \
                                      BITSERIAL0_CONFIG2_DOUT_IDLE_SEL_LSB_POSN
#define HAL_BITSERIAL_CONFIG2_SLAVE_10BIT_ADDR_MODE_MASK \
                                  BITSERIAL0_CONFIG2_SLAVE_10BIT_ADDR_MODE_MASK
#define HAL_BITSERIAL_CONFIG_CLEAR_STICKY_ACK_MASK \
                                        BITSERIAL0_CONFIG_CLEAR_STICKY_ACK_MASK
#define HAL_BITSERIAL_CONFIG2_DEBUG_SEL_MASK \
                                     (BITSERIAL0_CONFIG2_DEBUG_SEL_LSB_MASK | \
                                      BITSERIAL0_CONFIG2_DEBUG_SEL_MSB_MASK)
#define HAL_BITSERIAL_CONFIG2_SLAVE_ANY_CMD_BYTE_EN_MASK \
                                  BITSERIAL0_CONFIG2_SLAVE_ANY_CMD_BYTE_EN_MASK

#define HAL_BITSERIAL_CONFIG3_ACT_ON_NAK_LSB_POSN \
                                          BITSERIAL0_CONFIG3_ACT_ON_NAK_LSB_POSN
#define HAL_BITSERIAL_CONFIG3_ACT_ON_NAK_LEGACY \
                                            BITSERIAL0_CONFIG3_ACT_ON_NAK_LEGACY
#define HAL_BITSERIAL_CONFIG3_ACT_ON_NAK_STOP BITSERIAL0_CONFIG3_ACT_ON_NAK_STOP
#define HAL_BITSERIAL_CONFIG3_ACT_ON_NAK_RESTART \
                                           BITSERIAL0_CONFIG3_ACT_ON_NAK_RESTART

#define HAL_BITSERIAL_MASTER_SLAVE_CLK_REQ_EN_MASK \
                                         BITSERIAL0_MASTER_SLAVE_CLK_REQ_EN_MASK
#define HAL_BITSERIAL_MAIN_CLK_REQ_EN_MASK BITSERIAL0_MAIN_CLK_REQ_EN_MASK

/* Macros for making the bitserial code compile cleanly even with older
   registers */
#ifndef hal_set_reg_bitserial0_debug_sel
#define hal_set_reg_bitserial0_debug_sel(v) UNUSED(v)
#endif /* hal_set_reg_bitserial0_debug_sel */
#ifndef hal_set_reg_bitserial1_debug_sel
#define hal_set_reg_bitserial1_debug_sel(v) UNUSED(v)
#endif /* hal_set_reg_bitserial1_debug_sel */

/* Macros for adapting Hawkeye HAL layer API to hydra bitserial HAL */
#if defined(INSTALL_HAWKEYE) || defined(INSTALL_VULTAN) || \
                                                      defined (INSTALL_TRINITY)
#define hal_get_bitserial0_event() hal_get_bitserial_event_bitserial0_status()
#define hal_get_bitserial1_event() hal_get_bitserial_event_bitserial1_status()
#define BITSERIAL0_CONFIG2_DOUT_IDLE_SEL_LSB_POSN \
            BITSERIAL_CONFIG2_TEMPLATE_BITSERIAL_CONFIG2_DOUT_IDLE_SEL_LSB_POSN
#define generate_sw_interrupt(swintid) \
                                     generate_fg_int(FGINTID_BITSERIAL_FG_INT);
#endif


#endif /* INSTALL_BITSERIAL */

#endif  /* HAL_BITSERIAL_H */

/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    pio_if.h

CONTAINS
    Definitions for the pio subsystem.

DESCRIPTION
    This file is seen by the stack, and VM applications, and
    contains things that are common between them.
*/

#ifndef __PIO_IF_H__
#define __PIO_IF_H__

/*! @brief Functions which can be mapped onto a pin. This is used in PioSetFunction().
 *   The user must consult the relevant device datasheet for possible mappings
 */
typedef enum 
{
    /** UART data input */
    UART_RX,
    /** UART data output */
    UART_TX,
    /** UART "ready to send" output */
    UART_RTS,
    /** UART "clear to send" input */
    UART_CTS,
    PCM_IN,
    PCM_OUT,
    PCM_SYNC,
    PCM_CLK,
    SQIF,
/**
 * Associates a PIO to a LED ID.
 *
 * Each chip maps a set of PIOs to a specific LED controller instance. This
 * means a given PIO can only be associated to a particular led controller
 * instance.
 * The allowed configuration is specific to the chip. 
 *
 * CSRA68100:
 * There are 12 disjoint sets of PIOs. The PIOs from each set  E.g.:
 *
 * led_ctrl_instance_0  = { pio | pio in {0, 12, 24, 36, 48, 60, 72, 84 }}
 * led_ctrl_instance_1  = { pio | pio in {1, 13, 25, 37, 49, 61, 73, 85 }}
 *  ...
 * led_ctrl_instance_11 = { pio | pio in {11, 23, 35, 47, 59, 71, 83, 95 }}
 *
 * The led controller instances are statically mapped to led_ids so that the
 * led_id maps to the led pad with the same index. E.g.:
 *
 * +========+===========+===========+============================+
 * | led_id | ctrl inst | LED PADS  |         all PIOs           |
 * +========+===========+===========+============================+
 * |    0   |      5    |  65 (0)   |     5 17 29 41 53 65 77 89 |
 * |    1   |      6    |  66 (1)   |     6 18 30 42 54 66 78 90 |
 * |    2   |      7    |  67 (2)   |     7 19 31 43 55 67 79 91 |
 * |    3   |      8    |  68 (3)   |     8 20 32 44 56 68 80 92 |
 * |    4   |      9    |  69 (4)   |     9 21 33 45 57 69 81 93 |
 * |    5   |     10    |  70 (5)   |    10 22 34 46 58 70 82 94 |
 * |    6   |     11    |  71 (6)   |    11 23 35 47 59 71 83 95 |
 * |    7   |      0    |  72 (7)   |  0 12 24 36 48 60 72 84    |
 * |    8   |      1    |  73 (N/A) |  1 13 25 37 49 61 73 85    |
 * |    9   |      2    |  74 (N/A) |  2 14 26 38 50 62 74 86    |
 * |   10   |      3    |  75 (N/A) |  3 15 27 39 51 63 75 87    |
 * |   11   |      4    |  76 (N/A) |  4 16 28 40 52 64 76 88    |
 * +--------+-----------+-----------+----------------------------+
 *
 * QCC512x/QCC302x:
 * There are 6 disjoint sets of PIOs. The PIOs from each set  E.g.:
 *
 * led_ctrl_instance_0  = 
     { pio | pio in { 0,  6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66 }}
 * led_ctrl_instance_1  = 
     { pio | pio in { 1,  7, 13, 19, 25, 31, 37, 43, 49, 55, 61, 67 }}
 * led_ctrl_instance_2  = 
     { pio | pio in { 2,  8, 14, 20, 26, 32, 38, 44, 50, 56, 62, 68 }}
     ...
 * led_ctrl_instance_5  = 
     { pio | pio in { 5, 11, 17, 23, 29, 35, 41, 47, 53, 59, 65, 71 }}
 *
 * The led controller instances are statically mapped to led_ids so that the
 * led_id maps to the led pad with the same index. E.g.:
 *
 * +========+===========+==========+=====================================+
 * | led_id | ctrl inst | LED PADS |                all PIOs             |
 * +========+===========+==========+=====================================+
 * |    0   |      0    |  66 (0)  |  0  6 12 18 24 30 36 42 48 54 60 66 |
 * |    1   |      1    |  67 (1)  |  1  7 13 19 25 31 37 43 49 55 61 67 |
 * |    2   |      2    |  68 (2)  |  2  8 14 20 26 32 38 44 50 56 62 68 |
 * |    3   |      3    |  69 (3)  |  3  9 15 21 27 33 39 45 51 57 63 69 |
 * |    4   |      4    |  70 (4)  |  4 10 16 22 28 34 40 46 52 58 64 70 |
 * |    5   |      5    |  71 (5)  |  5 11 17 23 29 35 41 47 53 59 65 71 |
 * +--------+-----------+----------+-------------------------------------+
 */
    LED,
    LCD_SEGMENT,
    LCD_COMMON,
    PIO,
    SPDIF_RX,
    SPDIF_TX,
    /** Bitserial 0, clock input used in SPI slave and I2C */
    BITSERIAL_0_CLOCK_IN,
    /** Bitserial 0, clock output used in SPI master and I2C */
    BITSERIAL_0_CLOCK_OUT,
    /** Bitserial 0, data input used for SPI master MISO, SPI slave MOSI and I2C */
    BITSERIAL_0_DATA_IN,
    /** Bitserial 0, data output used for SPI master MOSI, SPI slave MISO and I2C */
    BITSERIAL_0_DATA_OUT,
    /** Bitserial 0, chip select input used for SPI slave */
    BITSERIAL_0_SEL_IN,
    /** Bitserial 0, chip select output used for SPI master */
    BITSERIAL_0_SEL_OUT,
    /** Bitserial 1, clock input used in SPI slave and I2C */
    BITSERIAL_1_CLOCK_IN,
    /** Bitserial 1, clock output used in SPI master and I2C */
    BITSERIAL_1_CLOCK_OUT,
    /** Bitserial 1, data input used for SPI master MISO, SPI slave MOSI and I2C */
    BITSERIAL_1_DATA_IN,
    /** Bitserial 1, data output used for SPI master MOSI, SPI slave MISO and I2C */
    BITSERIAL_1_DATA_OUT,
    /** Bitserial 1, chip select input used for SPI slave */
    BITSERIAL_1_SEL_IN,
    /** Bitserial 1, chip select output used for SPI master */
    BITSERIAL_1_SEL_OUT,
    /** Sets an XIO or an LED pin to analogue mode for ADC use (LED pins are
        set by the Curator during the measurement) */
    ANALOGUE,
    /** Associates a PIO to something outside of Apps control. */
    OTHER
} pin_function_id;

/*! @brief Drive strength IDs which can be set for a pin. This is used in
 *  PioSetDriveStrength() and PioGetDriveStrength().
 *  The user must consult the relevant device datasheet for supported drive
 *  strength IDs and their meaning on each pin.
 */
typedef enum
{
    DRIVE_STRENGTH_ID0 = 0,
    DRIVE_STRENGTH_ID1,
    DRIVE_STRENGTH_ID2,
    DRIVE_STRENGTH_ID3
} pin_drive_strength_id;

#endif /* __PIO_IF_H__  */

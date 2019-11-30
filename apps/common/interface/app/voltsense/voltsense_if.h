/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    voltsense_if.h

CONTAINS
    Definitions for the voltsense subsystem.

DESCRIPTION
    This file is seen by the stack, and VM applications, and
    contains things that are common between them.
*/

/*!
 @file voltsense_if.h
 @brief Parameters for XioMeasureVoltage().

*/

#ifndef __VOLTSENSE_IF_H__
#define __VOLTSENSE_IF_H__

/*Indicates voltage read failure*/
#define VOLTAGE_UNDEFINED 0x1fff

/*! @brief XIO on which a voltage may be measured or
 *   one of a pair on which a voltage difference may be measured.
 *   This is used in XioMeasureVoltage().
 *   The user must consult the relevant device datasheet for possible mappings
 */
typedef enum
{
    XIO0 = 0,
    XIO1 = 1,
    XIO2 = 2,
    XIO3 = 3,
    XIO4 = 4,
    XIO5 = 5,
    XIO6 = 6,
    XIO7 = 7,
    XIO8 = 8,
    XIO9 = 9,
    XIO10 = 10,
    XIO11 = 11,
    XIONone = 99
}xio_source;

/** Helper macro to convert a voltsense XIO source into a voltsense XIO source index */
#define voltsense_xio_src_to_idx(x) \
    ((uint16)(x) - (uint16)XIO0)
/** Helper macro to convert a voltsense XIO source index into a voltsense XIO source */
#define voltsense_xio_idx_to_src(x) \
    (xio_source)((x) + (uint16)XIO0)

/*! @brief Voltage gain to be used with the DAC
 *   This is used in DacSetGain().
 *   The user must consult the relevant device datasheet for possible mappings
 *   Actual values reflect specification of the DATA_CONV_DAC_BUFFER_0P9_GAIN
 *   bit in the register, DATA_CONV_CFG2
 */
typedef enum
{
    DBG1       = 0,
    DBGPOINT9  = 1
}dac_buffer_gain;

/*! @brief Voltage gain to be used when a voltage or voltage difference
 *   is being measured on an XIO or pair of XIOs.
 *   This is used in XioMeasureVoltage().
 *   The user must consult the relevant device datasheet for possible mappings
 *   Actual values reflect specification of the register, DATA_CONV_VSCL_GAIN
 */
typedef enum
{
    VGA1       = 1,
    VGAPOINT85 = 2,
    VGA2       = 3,
    VGA4       = 4,
    VGA8       = 5
}vga_gain;

/** Helper macro to convert a voltsense gain into a voltsense gain index */
#define voltsense_gain_src_to_idx(x) \
    ((uint16)(x) - (uint16)VGA1)
/** Helper macro to convert a voltsense gain index into a voltsense gain */
#define voltsense_gain_idx_to_src(x) \
    (vga_gain)((x) + (uint16)VGA1)

/*! @brief Values that can be returned by XioMeasureVoltage().
 *   The user must consult the relevant device datasheet for possible mappings
 */
typedef enum
{
    TRAP_VOLTSENSE_SUCCESS            = 0,
    TRAP_VOLTSENSE_INVALID_PARAMETERS = 1,
    TRAP_VOLTSENSE_INVALID_TASK       = 2,
    TRAP_VOLTSENSE_NOT_READY          = 3
}trap_voltsense_result;

#endif /* __VOLTSENSE_IF_H__  */

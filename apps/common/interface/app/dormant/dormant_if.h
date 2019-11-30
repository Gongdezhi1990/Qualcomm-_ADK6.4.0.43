/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    dormant_if.h

CONTAINS
    Definitions for dormant.

DESCRIPTION
    This file is seen by the stack/CDA devices, and customer applications, and
    contains things that are common between them.
*/

/*!
 @file dormant_if.h
 @brief Types used by the DormantConfigure() VM trap.
*/

#ifndef __APP_DORMANT_IF_H__
#define __APP_DORMANT_IF_H__


/*! @brief Dormant config keys - used to configure the dormant mode.
 */
typedef enum
{

    /*!
When using this key will cause the chip to attempt to enter dormant mode. The
value used is is not taken into account.
     */
    DORMANT_ENABLE,

    /*!
The mask of LEDs that should wake the chip. Bit 0 corresponds to LED0.
     */
    LED_WAKE_MASK,

    /*!
The mask of LEDs for which the wake input is inverted. Bit 0 corresponds to
LED0. Only meaningful if the bit is also set in LED_WAKE_MASK.
     */
    LED_WAKE_INVERT_MASK,

    /*!
The mask of LEDs for which the output needs to be set during dormant. Bit 0
corresponds to LED0.
     */
    LED_OUTPUT_DRIVE_MASK,

    /*!
The driven output on the LED pads. Bit 0 corresponds to LED0. Only meaningful
if the bit is also set in LED_OUTPUT_DRIVE_MASK.
Note: An LED pin cannot be a wakeup source and a output driver at the same time.
Also an LED pin can only be driven low. A request to drive it high will result
in the LED being configured in HighZ state.
     */
    LED_OUTPUT_LEVEL_MASK,

    /*!
The mask of LEDs to which a pull down needs to be applied. Bit 0 corresponds to
LED0. Only meaningful if the corresponding bit is also set in
LED_OUTPUT_DRIVE_MASK or LED_WAKE_MASK.
     */
    LED_PULL_DOWN_MASK,

    /*!
The mask of XIOs that should wake the chip. Bit 0 corresponds to XIO0.
     */
    XIO_WAKE_MASK,

    /*!
Enable dormant wake on NFC field detect and/or NFC overload. See
dormant_nfc_wake_mask for more information.
     */
    NFC_WAKE_MASK,

    /*!
If set then the deadline time will be used to wake-up the chip from dormant.
Otherwise the deadline will be ignored.
     */
    DEADLINE_VALID,

    /*!
The deadline by which the chip must be woken up from dormant. Only valid when
DEADLINE_VALID is set. Otherwise, it will be ignored.
Note: The wakeup timer in dormant runs from a 8KHz slow clock. Therefore, the
time duration to stay in dormant is obtained by converting the deadline time
(which is based on the system clock) to the slow clock ticks. If the slow clock
is un-calibrated then the dormant duration can be off by +/- 50%.
     */
    DEADLINE,

    /*!
The mask of PIOs that should wake the chip. Bit 0 corresponds to PIO0.
The PIOs that can be used to wake the chip up is product specific.
     */
    PIO_WAKE_MASK,

    /*!
The mask of PIOs for which the wake input is inverted. Bit 0 corresponds to
PIO0. Only meaningful if the bit is also set in PIO_WAKE_MASK.
     */
    PIO_WAKE_INVERT_MASK
    
}dormant_config_key;

/*! @brief NFC wake source masks - used to configure NFC wake from dormant.
 */
typedef enum
{
    /*!
Disable wake on NFC sources.
     */
    NFC_WAKE_NONE = 0,

    /*!
Enable wake on NFC field detect.
     */
    NFC_WAKE_FIELD_DETECT = 1 << 0,

    /*!
Enable wake on NFC overload.
     */
    NFC_WAKE_OVERLOAD = 1 << 1
}dormant_nfc_wake_mask;

/** Handy define for referring to all NFC wake sources. */
#define NFC_WAKE_ALL (NFC_WAKE_FIELD_DETECT | \
                      NFC_WAKE_OVERLOAD)

#endif /* __APP_DORMANT_IF_H__  */

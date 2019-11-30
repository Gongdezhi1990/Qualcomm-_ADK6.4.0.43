/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    infrared_if.h

CONTAINS
    Definitions for the infrared subsystem.

DESCRIPTION
    This file is seen by the stack, and VM applications, and
    contains things that are common between them.
*/

#ifndef __INFRARED_IF_H__
#define __INFRARED_IF_H__


/*! @brief Infrared config keys - used to configure the infrared receiver */
typedef enum
{
    /*!
When using this key a non zero value will enable the infrared receiver.  
Disabling the infrared receiver after it's been enabled is not currently
supported.
     */
    INFRARED_ENABLE,

    /*!
Used to select which protocol the receiver will attempt to decoder.
Valid values are defined in the #infrared_protocol enum.  If this key is not
set the F/W will default to using the RC5 decoder.
     */
    INFRARED_PROTOCOL,

    /*!
Used to select which PIO the infrared receiver IC is connected to.
If this key is not set the F/W will default to using PIO 6.
     */
    INFRARED_PIO,

    /*!
Used to configured if the received signal on PIO is inverted or not.  Normally
the signal is active low, eg. in presence of a modulated infrared signal the PIO
is driven low.  Currently this key is a place holder only, the F/W does not
decode an inverted signal.
     */
    INFRARED_PIO_INVERT,

    /*!
Used to configured how much jitter in microseconds is allowed on the expected
infrared pulse lengths.  By default this key is set to 300uS.  Valid values
are between 50uS and 500uS.
     */
    INFRARED_JITTER_ALLOWANCE,

    /*!
Used to configured how long in microseconds the F/W checks the initial start
pulse is stable before commencing sampling of the PIO.  By default this key is
set to 200uS.  Valid values are between 100uS and 1000uS.
    */
    INFRARED_START_PULSE_STABLE_PERIOD,

    /*!
Used to specify how long to wait in milliseconds, in the abscent of valid
infrared data before sending a key release event.  By default this key is set
to 250mS.  Valid values are between 100ms and 1000mS.
    */
    INFRARED_KEY_RELEASE_PERIOD,

    /*!
Used to specify how long in milliseconds to prevent deep-sleep after receiving
any infrared data.  By default this key is set to 100mS.  Valid values are
between 1ms and 1000mS.
    */
    INFRARED_KEEP_AWAKE_PERIOD,

    /*!
Used to specify if the glitch filter is enabled or not.  A zero value will
disable the glitch filter, a non-zero value (up to 500uS) configures the
maximum length glitch in microseconds that can be filtered out.
    */
    INFRARED_GLITCH_FILTER_ENABLE

} infrared_config_key;

/*! @brief Infrared events */
typedef enum
{
    EVENT_PRESSED,
    EVENT_RELEASED
} infrared_event;

/*! @brief Infrared protocols */
typedef enum
{
    PROTOCOL_AUTO,  /*!< Auto detect protocol */
    PROTOCOL_RC5,   /*!< Force RC5 protocol decoding */
    PROTOCOL_NEC    /*!< Force NEC protocol decoding */
} infrared_protocol;

#endif /* __INFRARED_IF_H__  */


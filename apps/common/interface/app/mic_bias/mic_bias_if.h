/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    mic_bias_if.h

CONTAINS
    Definitions for the mic_bias subsystem.

DESCRIPTION
    This file is seen by the stack, and VM applications, and
    contains things that are common between them.
*/

#ifndef __MIC_BIAS_IF_H__
#define __MIC_BIAS_IF_H__

/*! @brief MIC_BIAS pin identifiers */
typedef enum 
{
    MIC_BIAS_0,
    MIC_BIAS_1,
    NUM_OF_MIC_BIAS
}mic_bias_id;


/*! @brief MIC_BIAS config keys. */
typedef enum 
{
    /*!
Enable the microphone bias according to the passed in value (which is
a member of #mic_bias_enable_value).
     */
    MIC_BIAS_ENABLE,

    /*!
Set the microphone bias voltage.
When using this key, the exact mapping from value
to voltage is device dependent, so consult the datasheet. As an
example, for QCC302x/3x and QCC512x devices

      Value   Voltage

        0     2.1 V

        1     2.1 V

        2     2.0 V

        3     1.9 V

        4     1.8 V

        5     1.7 V

        6     1.6 V

        7     1.5 V
     */
    MIC_BIAS_VOLTAGE,

    /*!
Set the microphone bias current.
When using this key, the exact mapping from value
to current is device dependent, so consult the datasheet.
Only supported on BlueCore chips.
     */
    MIC_BIAS_CURRENT

}mic_bias_config_key;

/*! @brief #MIC_BIAS_ENABLE key values. */
typedef enum 
{
    MIC_BIAS_OFF,           /*!< Disable the microphone bias immediately. */
    MIC_BIAS_AUTOMATIC_ON,  /*!< Only supported on Bluecore chips. If
                                 PSKEY_CODEC_PIO is mapped to
                                 MIC_BIAS_HARDWARE_MIC_BIAS_PIN, automatically
                                 turn the microphone bias on or off when the
                                 #AUDIO_HARDWARE_CODEC is enabled or disabled.*/
    MIC_BIAS_FORCE_ON       /*!< Enable the microphone bias immediately. */

}mic_bias_enable_value;


#endif /* __MIC_BIAS_IF_H__  */

/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
        audio_if.h  -  Audio Interface

CONTAINS
        Interface elements between the Audio firmware and VM Applications, that are
        not in sys.[ch]

DESCRIPTION
        This file is seen by the VM applications and the Firmware.
*/

#ifndef __APP_AUDIO_IF_H__
#define __APP_AUDIO_IF_H__

/*!
    @brief Defines used to indicate Codec Gain Ranges (see #STREAM_CODEC_INPUT_GAIN and #STREAM_CODEC_OUTPUT_GAIN)
*/

#define CODEC_INPUT_GAIN_RANGE      22
#define CODEC_OUTPUT_GAIN_RANGE     22

typedef enum {
    AUDIO_HARDWARE_PCM, /*!< The audio PCM hardware. "instance" specifies the physical interface (AUDIO_INSTANCE_0 is the first PCM interface, AUDIO_INSTANCE_1 is the second, on chips which have two).
                         Audio hardware PCM, I2S and SPDIF are mutually exclusive for the same audio_instance.
                         "channel" specifies the PCM slot; only AUDIO_CHANNEL_SLOT_x are valid for this hardware type.*/
    AUDIO_HARDWARE_I2S, /*!< The audio I2S hardware. "instance" specifies the physical interface (AUDIO_INSTANCE_0 is the first I2S interface, AUDIO_INSTANCE_1 is the second, on chips which have two).
                         Audio hardware PCM, I2S and SPDIF are mutually exclusive for the same audio_instance.
                         "channel" specifies the I2S slot; only AUDIO_CHANNEL_SLOT_x are valid for this hardware type.*/
    AUDIO_HARDWARE_SPDIF, /*!< The audio SPDIF hardware. "instance" specifies the physical interface (AUDIO_INSTANCE_0 is the first SPDIF interface, AUDIO_INSTANCE_1 is the second, on chips which have two).
                           Audio hardware PCM, I2S and SPDIF are mutually exclusive for the same audio_instance.
                           "channel" specifies the SPDIF slot; only AUDIO_CHANNEL_SLOT_x are valid for this hardware type.*/
    AUDIO_HARDWARE_CODEC, /*!< The audio CODEC hardware. "instance" specifies the physical interface (AUDIO_INSTANCE_0 is the first CODEC interface, AUDIO_INSTANCE_1 is the second, on chips which have two).
                           "channel" specifies the CODEC channel; only AUDIO_CHANNEL_A or AUDIO_CHANNEL_B or AUDIO_CHANNEL_A_AND_B are valid for this hardware type.
                           On chips with stereo CODECs, an "instance" consists of a pair of channels (stereo).*/
    AUDIO_HARDWARE_DIGITAL_MIC, /*!< The audio digital MIC hardware. "instance" specifies the physical interface (AUDIO_INSTANCE_0 is the first digital MIC interface, AUDIO_INSTANCE_1 is the second,
                                     AUDIO_INSTANCE_2 is the third, on chips which have three).
                                     "channel" specifies the digital MIC channel; only AUDIO_CHANNEL_A or AUDIO_CHANNEL_B are valid for this hardware type.
                                     An "instance" consists of a pair of channels (stereo).*/
    AUDIO_HARDWARE_FM /*!< The audio FM hardware. "instance" specifies the physical interface (AUDIO_INSTANCE_0 is the FM interface).
                       The FM instance can be obtained either by StreamAudioSource (FM receiver) or by StreamAudioSink(FM transmitter).
                       "channel" specifies the FM channel; only AUDIO_CHANNEL_A or AUDIO_CHANNEL_B are valid for this hardware type.*/
} audio_hardware;

/*!
  @brief audio_instance used in StreamAudioSource(), StreamAudioSink() and CodecSetIirFilter() traps.

  The meaning of an "instance" depends on the hardware; see the descriptions in #audio_hardware.
*/

typedef enum {
    AUDIO_INSTANCE_0, /*!< The audio hardware instance 0*/
    AUDIO_INSTANCE_1, /*!< The audio hardware instance 1*/
    AUDIO_INSTANCE_2, /*!< The audio hardware instance 2*/
    AUDIO_INSTANCE_3  /*!< The audio hardware instance 3*/
} audio_instance;

/*!
  @brief audio_channel used in StreamAudioSource(), StreamAudioSink() and CodecSetIirFilter() traps.
*/

typedef enum {
    AUDIO_CHANNEL_A, /*!< The audio channel A*/
    AUDIO_CHANNEL_B, /*!< The audio channel B*/
    AUDIO_CHANNEL_A_AND_B, /*!< The audio channel A and B (obtaining the stereo CODEC in mono-mode)*/
    AUDIO_CHANNEL_SLOT_0 = 0, /*!< The audio digital slot 0*/
    AUDIO_CHANNEL_SLOT_1, /*!< The audio digital slot 1*/
    AUDIO_CHANNEL_SLOT_2, /*!< The audio digital slot 2*/
    AUDIO_CHANNEL_SLOT_3, /*!< The audio digital slot 3*/
    SPDIF_CHANNEL_A = 0, /*!< The 1st SPDIF subframe*/
    SPDIF_CHANNEL_B, /*!< The 2nd SPDIF subframe*/
    SPDIF_CHANNEL_USER, /*!< The User data in SPDIF subframes*/
    SPDIF_CHANNEL_A_B_INTERLEAVED /*!< SPDIF channels to be interleaved*/
} audio_channel;


typedef struct
{
    uint16 coefficients[11];
} IIR_COEFFICIENTS;

/*!
  @brief audio_anc_instance used in AudioAncFilterIirSet() and AudioAncFilterLpfSet traps.
*/
typedef enum {
    AUDIO_ANC_INSTANCE_NONE = 0x0000, /*!< Not Used */
    AUDIO_ANC_INSTANCE_0    = 0x0001, /*!< The audio ANC hardware instance 0 */
    AUDIO_ANC_INSTANCE_1    = 0x0002  /*!< The audio ANC hardware instance 1 */
} audio_anc_instance;

/*!
  @brief audio_anc_path_id used in AudioAncFilterIirSet() and AudioAncFilterLpfSet traps.
*/
typedef enum {
    AUDIO_ANC_PATH_ID_NONE   = 0x0000,  /*!< Not Used */
    AUDIO_ANC_PATH_ID_FFA    = 0x0001,  /*!< The audio ANC path for Feed Forward a */
    AUDIO_ANC_PATH_ID_FFB    = 0x0002,  /*!< The audio ANC path for Feed Forward b */
    AUDIO_ANC_PATH_ID_FB     = 0x0003,  /*!< The audio ANC path for FeedBack */
    AUDIO_ANC_PATH_ID_SM_LPF = 0x0004   /*!< The audio ANC path for Small LPF */
} audio_anc_path_id;

/*!
  @brief audio_power_save_mode used in AudioPowerSaveModeSet() and AudioPowerSaveModeGet traps.
*/
typedef enum {
    AUDIO_POWER_SAVE_MODE_1   = 0x0001, /*!< Power save mode which uses the default audio resources */
    AUDIO_POWER_SAVE_MODE_2   = 0x0002, /*!< Power save mode that may offer more power savings than \c AUDIO_POWER_SAVE_MODE_1.
                                             On QCC51xx product series, this is same as \c AUDIO_POWER_SAVE_MODE_1 */
    AUDIO_POWER_SAVE_MODE_3   = 0x0003, /*!< Power save mode that may offer more power savings than \c AUDIO_POWER_SAVE_MODE_2.
                                             On QCC51xx product series, this is same as \c AUDIO_POWER_SAVE_MODE_1 */
    AUDIO_POWER_SAVE_MODE_4   = 0x0004, /*!< Power save mode that may offer more power savings than \c AUDIO_POWER_SAVE_MODE_3.
                                             On QCC51xx product series, this limits AUDIO_HARDWARE_DIGITAL_MIC maximum sampling rate to 48KHz.*/
    AUDIO_POWER_SAVE_MODE_5   = 0x0005 /*!< Power save mode that may offer more power savings than \c AUDIO_POWER_SAVE_MODE_4.
                                             On QCC51xx product series, this limits AUDIO_HARDWARE_DIGITAL_MIC maximum sampling rate to 24KHz.*/
} audio_power_save_mode;

/*!
  @brief audio_dsp_clock_type used in \c audio_dsp_clock_configuration.
*/
typedef enum {
    AUDIO_DSP_CLOCK_NO_CHANGE  = 0x0000, /*!< Clock type to be used if no change required to the existing configuration */
    AUDIO_DSP_EXT_LP_CLOCK     = 0x0001, /*!< Default clock type for low-power mode unless specified in product data sheet */
    AUDIO_DSP_VERY_LP_CLOCK    = 0x0002, /*!< Next higher clock type for low-power mode and is typically \c AUDIO_DSP_EXT_LP_CLOCK * 2 */
    AUDIO_DSP_LP_CLOCK         = 0x0003, /*!< Highest clock type for low-power mode and is typically \c AUDIO_DSP_EXT_LP_CLOCK * 4 */
    AUDIO_DSP_VERY_SLOW_CLOCK  = 0x0004, /*!< Lowest clock type for trigger mode (transition from low-power to active mode) */
    AUDIO_DSP_SLOW_CLOCK       = 0x0005, /*!< Default clock type for trigger mode (transition from low-power to active mode)
                                              unless specified in product data sheet. The default clock value is same as the
                                              XTAL frequency without using a PLL. Can also be used for active mode while running
                                              low power audio use case*/
    AUDIO_DSP_BASE_CLOCK       = 0x0006, /*!< PLL generated active mode clock type required to run audio use case at higher clock
                                              than SLOW_CLOCK */
    AUDIO_DSP_TURBO_CLOCK      = 0x0007, /*!< Maximum active mode clock type supported in most product unless specified*/
    AUDIO_DSP_TURBO_PLUS_CLOCK = 0x0008  /*!< Active mode clock type higher than \c AUDIO_DSP_TURBO_CLOCK. Only available in selected products
                                              and required to run complex audio use case */
} audio_dsp_clock_type;

/*
 * Audio-DSP clock configuration used in \c AudioDspClockConfigure().
 */
typedef struct audio_dsp_clock_config
{
    audio_dsp_clock_type active_mode;    /*!< Clock configuration for active mode */
    audio_dsp_clock_type low_power_mode; /*!< Clock configuration for low-power mode */
    audio_dsp_clock_type trigger_mode;   /*!< Deprecated. Only AUDIO_DSP_CLOCK_NO_CHANGE allowed. */
}audio_dsp_clock_configuration;

/*
 * Audio-DSP clock raw values returned in \c AudioDspGetClock().
 */
typedef struct audio_dsp_clock
{
    uint16 active_mode;    /*!< Active mode clock value in MHz */
    uint16 low_power_mode; /*!< Low-power mode clock value in MHz */
    uint16 trigger_mode;   /*!< Deprecated. Always 0. */
}audio_dsp_clock;

#endif  /* __APP_AUDIO_IF_H__ */

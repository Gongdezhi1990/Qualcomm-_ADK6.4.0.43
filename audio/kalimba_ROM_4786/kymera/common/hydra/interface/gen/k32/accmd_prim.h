/*****************************************************************************

            Copyright (c) 2017 Qualcomm Technologies International, Ltd.

            WARNING: This is an auto-generated file!
                     DO NOT EDIT!

*****************************************************************************/
#ifndef ACCMD_PRIM_H__
#define ACCMD_PRIM_H__

#include "hydra_types.h"
#include "../../interface/gen/xap/system_keys_prim.h"


/*******************************************************************************

  NAME
    Accmd_Audio_Channel

  DESCRIPTION
    Audio Hardware Channels/Slots Keep the values here consistent with the
    BCCMD equivalent, audio_channel in
    //depot/bc/main/interface/app/audio/audio_if.h We need to add a "spoiler"
    value to avoid casting this type to 8 bit in case of Kalarch4 FIXME
    values have been allocated for SPDIF too, see B-120170

 VALUES
    Slot_0  -
    A       -
    B       -
    Slot_1  -
    A_and_B -
    Slot_2  -
    Slot_3  -
    MAX     -

*******************************************************************************/
typedef enum
{
    ACCMD_AUDIO_CHANNEL_SLOT_0 = 0x0000,
    ACCMD_AUDIO_CHANNEL_A = 0x0000,
    ACCMD_AUDIO_CHANNEL_B = 0x0001,
    ACCMD_AUDIO_CHANNEL_SLOT_1 = 0x0001,
    ACCMD_AUDIO_CHANNEL_A_AND_B = 0x0002,
    ACCMD_AUDIO_CHANNEL_SLOT_2 = 0x0002,
    ACCMD_AUDIO_CHANNEL_SLOT_3 = 0x0003,
    ACCMD_AUDIO_CHANNEL_MAX = 0x7FFF
} ACCMD_AUDIO_CHANNEL;
/*******************************************************************************

  NAME
    Accmd_Audio_Instance

  DESCRIPTION
    Audio Hardware Instances Keep the values here consistent with the BCCMD
    equivalent, audio_instance in
    //depot/bc/main/interface/app/audio/audio_if.h Audio Hardware Instance is
    also used for sending the HCI channel in case of a SCO endpoint. The HCI
    channel could be bigger than 8bit which means that we need to add a
    "spoiler" value to avoid casting this type to 8 bit in case of Kalarch4

 VALUES
    0   -
    1   -
    2   -
    3   -
    MAX -

*******************************************************************************/
typedef enum
{
    ACCMD_AUDIO_INSTANCE_0 = 0x0000,
    ACCMD_AUDIO_INSTANCE_1 = 0x0001,
    ACCMD_AUDIO_INSTANCE_2 = 0x0002,
    ACCMD_AUDIO_INSTANCE_3 = 0x0003,
    ACCMD_AUDIO_INSTANCE_MAX = 0x7FFF
} ACCMD_AUDIO_INSTANCE;
/*******************************************************************************

  NAME
    Accmd_Config_Key

  DESCRIPTION
    Stream Configuration Keys Keep the values here consistent with the BCCMD
    equivalent, stream_config_key in
    //depot/bc/main/interface/app/stream/stream_if.h 0x0000 - 0x00FF
    typically used for VM keys. The keys
    "STREAM_PCM_EXTEND_FEATURES_ENABLE"(0x0110),
    "STREAM_PCM_DDS_UPDATE_ENABLE"(0x0111),
    "STREAM_PCM_SLOW_CLOCK_RATE"(0x0112), "STREAM_PCM_RX_RATE_DELAY"(0x0113)
    are not supported on Hydra. 0x0300 - 0x03FF typically used for codec keys
    0x0309 would be STREAM_CODEC_MIC_INPUT_GAIN_ENABLE, but no HydraCore
    platform supports this (it's always enabled and not configurable), so no
    point defining it in the ACCMD protocol. Do not re-use this number, for
    compatibility with BlueCore. 0x030b was STREAM_CODEC_QUALITY_MODE but
    there is no equivalent in Crescendo hardware. Do not re-use this number,
    for compatibility with BlueCore. STREAM_CODEC_OUTPUT_POWER_MODE is split
    into STREAM_CODEC_OUTPUT_DRIVE for drive current (i.e. 16 Ohm vs 32 Ohm)
    and a TBD key, to be named when there is a need, for a setting to enable
    an output speaker driver vs. line output. For backwards compatibility, do
    not reuse 0x030d. 0x0400 - 0x04FF typically used for FM keys 0x0500 -
    0x05FF typically used for SPDIF keys 0x0600 - 0x06FF typically used for
    digital mic keys 0x0900 - 0x09FF typically used for FastPipe keys 0x1000
    - 0x10FF typically used for PWM keys 0x1100 - 0x11FF typically used for
    ANC 0x1200 - 0x12FF for rate measurement/rate matching.

 VALUES
    STREAM_SOURCE_NOTIFY_WHEN_DRAINED
                   -
    STREAM_PCM_SYNC_RATE
                   -
    STREAM_PCM_MASTER_CLOCK_RATE
                   -
    STREAM_PCM_MASTER_MODE
                   -
    STREAM_PCM_SLOT_COUNT
                   -
    STREAM_PCM_MANCH_MODE_ENABLE
                   -
    STREAM_PCM_SHORT_SYNC_ENABLE
                   -
    STREAM_PCM_MANCH_SLAVE_MODE_ENABLE
                   -
    STREAM_PCM_SIGN_EXTEND_ENABLE
                   -
    STREAM_PCM_LSB_FIRST_ENABLE
                   -
    STREAM_PCM_TX_TRISTATE_ENABLE
                   -
    STREAM_PCM_TX_TRISTATE_RISING_EDGE_ENABLE
                   -
    STREAM_PCM_SYNC_SUPPRESS_ENABLE
                   -
    STREAM_PCM_GCI_MODE_ENABLE
                   -
    STREAM_PCM_MUTE_ENABLE
                   -
    STREAM_PCM_LONG_LENGTH_SYNC_ENABLE
                   -
    STREAM_PCM_SAMPLE_RISING_EDGE_ENABLE
                   -
    STREAM_PCM_SAMPLE_FORMAT
                   -
    STREAM_PCM_MANCH_RX_OFFSET
                   -
    STREAM_PCM_AUDIO_GAIN
                   -
    STREAM_PCM_MASTER_CLK_SOURCE
                   - Set the source of the master clock for a PCM interface. By
                     default the subsystem root clock is used. This config key
                     can be used to select MCLK, or to borrow the clock from
                     PCM0/I2S0/EPCM (for PCM1/PCM2 only) instead. Note that MCLK
                     can be from internal MPLL or an external source MCLK_IN.
    STREAM_PCM_MASTER_MCLK_MULT
                   - Ratio of MCLK to sample rate. Valid choices are 128, 256 or
                     384.
    STREAM_I2S_SYNC_RATE
                   -
    STREAM_I2S_MASTER_CLOCK_RATE
                   -
    STREAM_I2S_MASTER_MODE
                   -
    STREAM_I2S_JSTFY_FORMAT
                   -
    STREAM_I2S_LFT_JSTFY_DLY
                   -
    STREAM_I2S_CHNL_PLRTY
                   -
    STREAM_I2S_AUDIO_ATTEN_ENABLE
                   -
    STREAM_I2S_AUDIO_ATTEN
                   -
    STREAM_I2S_JSTFY_RES
                   -
    STREAM_I2S_CROP_ENABLE
                   -
    STREAM_I2S_BITS_PER_SAMPLE
                   -
    STREAM_I2S_TX_START_SAMPLE
                   -
    STREAM_I2S_RX_START_SAMPLE
                   -
    STREAM_I2S_MASTER_CLK_SOURCE
                   - Set the source of the master clock for an I2S interface. By
                     default the subsystem root clock is used. This config key
                     can be used to select MCLK, or to borrow the clock from
                     PCM0/I2S0/EPCM (for I2S1/I2S2 only) instead. Note that MCLK
                     can be from internal MPLL or an external source MCLK_IN.
    STREAM_I2S_MASTER_MCLK_MULT
                   - Ratio of MCLK to sample rate. Valid choices are 128, 256 or
                     384.
    STREAM_CODEC_INPUT_RATE
                   - CODEC input sample rate in hertz. Consult the datasheet for
                     supported sample rates.
    STREAM_CODEC_OUTPUT_RATE
                   - CODEC output sample rate in hertz. Consult the datasheet
                     for supported sample rates.
    STREAM_CODEC_INPUT_GAIN
                   - CODEC input gain selection value. Valid range: 0 to 22.
                     Consult the datasheet of for further information.
    STREAM_CODEC_OUTPUT_GAIN
                   - CODEC output gain selection value. Valid range: 0 to 22.
                     Note that on CSRA681xx devices, unlike older devices, not
                     all of these values yield distinct gains; values 0-7 all
                     map to the same gain. Consult the datasheet for further
                     information.
    STREAM_CODEC_RAW_INPUT_GAIN
                   - CODEC raw input gain. Consult the datasheet for further
                     information.
    STREAM_CODEC_RAW_OUTPUT_GAIN
                   - CODEC raw output gain. Consult the datasheet for further
                     information.
    STREAM_CODEC_OUTPUT_GAIN_BOOST_ENABLE
                   - Enable CODEC output gain boost. This "boost" provides an
                     extra 3dB of gain when in differential output mode.
    STREAM_CODEC_SIDETONE_GAIN
                   - CODEC sidetone gain. Consult the datasheet for further
                     information.
    STREAM_CODEC_SIDETONE_ENABLE
                   - Enable CODEC sidetone.
    STREAM_CODEC_LOW_POWER_OUTPUT_STAGE_ENABLE
                   - Enable CODEC low power output stage.
    STREAM_CODEC_OUTPUT_INTERP_FILTER_MODE
                   - Set the FIR mode for the DAC interpolation filter. Valid
                     values can be selected from
                     #codec_output_interpolation_filter_mode enumeration. The
                     FIR mode can be changed when the DAC is running, however
                     this may cause audio disturbances and is not recommended
    STREAM_CODEC_SIDETONE_SOURCE
                   - The sidetone source. It should be one of the following
                     Codec A/B, Digital Mic A/B, Digital Mic C/D, Digital Mic
                     E/F
    STREAM_CODEC_SIDETONE_SOURCE_POINT
                   - The sidetone source point. Selects source point for ADC
                     data
    STREAM_CODEC_SIDETONE_INJECTION_POINT
                   - The sidetone injection point. Selects sidetone injection
                     point into DAC chain.
    STREAM_CODEC_SIDETONE_SOURCE_MASK
                   - The sidetone source mask. This key allows to select any
                     combination of 2 ADC channels to be used as sidetone
                     source.
    STREAM_CODEC_INDIVIDUAL_SIDETONE_GAIN
                   - ADC sidetone gain of single sidetone source.
    STREAM_CODEC_INDIVIDUAL_SIDETONE_ENABLE
                   - Enable sidetone for a particular DAC channel. Consult the
                     datasheet for further information.
    STREAM_CODEC_ADC_DATA_SOURCE_POINT
                   - ADC data source selection. ADC Data source can be selected
                     from either IIR Filter out or Digital Gain filter out
    STREAM_CODEC_ADC_ROUTE
                   - ADC chain routing. 0: IIR filter followed by Digital gain
                     1: Digital gain followed by IIR filter.
    STREAM_CODEC_SIDETONE_INVERT
                   - Invert sidetone phase for a particular DAC channel.
    STREAM_CODEC_G722_FILTER_ENABLE
                   - Enable optional G722 filter in ADC/DMIC chain. Improves
                     noise performance.
    STREAM_CODEC_G722_FIR_ENABLE
                   - Enable optional FIR filter in the G722 filter to droop the
                     response slightly.
    STREAM_CODEC_INPUT_TERMINATION
                   - Configure input termination such as single ended,
                     differential etc. in chips with CODEC input harware that
                     support it. Valid values for termination mode include :
                     0x0000 (differential), 0x0001 (single ended), 0x0002
                     (single ended inverted) 0x8000 - 0xFFFF (reserved)
    STREAM_CODEC_OUTPUT_QUALITY
                   - Configure an output quality vs power consumption tradeoff.
                     A lower number means less power consumption but also less
                     quality.
    STREAM_SPDIF_OUTPUT_RATE
                   - SPDIF output sample rate in hertz. Supported rates: 32000,
                     44100 and 48000.
    STREAM_SPDIF_CHNL_STS_REPORT_MODE
                   - Set the reporting mode for the SPDIF Rx channel Status.
    STREAM_SPDIF_OUTPUT_CHNL_STS_DUP_EN
                   - Set the SPDIF Tx channel B status same as that of channel
                     A.
    STREAM_SPDIF_OUTPUT_CHNL_STS_WORD
                   - Set the SPDIF Tx channel status word value.
    STREAM_SPDIF_AUTO_RATE_DETECT
                   - Set SPDIF RX in auto rate detect mode.
    STREAM_SPDIF_SET_EP_FORMAT
                   - Set SPDIF RX endpoint output format. SPDIF RX endpoint can
                     only be connected to a spdif_decode  operator which expects
                     the endpoint to have SPDIF_INPUT_DATA_FORMAT, this
                     behaviour can be changed using this config key. This key
                     shall not be used in any real product application, the main
                     purpose of the key is to test the HW independent from the
                     spdif_decode operator.
    STREAM_SPDIF_OUTPUT_CLK_SOURCE
                   - Set the source of the master clock for S/PDIF output. By
                     default the subsystem root clock is used for S/PDIF TX.
                     This config key can be used to select MCLK instead. Note
                     that MCLK can be from internal MPLL or an external source
                     MCLK_IN.
    STREAM_DIGITAL_MIC_INPUT_RATE
                   - Digital microphone sample rate in hertz. Consult the
                     datasheet for further information.
    STREAM_DIGITAL_MIC_INPUT_GAIN
                   - Digital microphone input gain. Consult the datasheet for
                     further information.
    STREAM_DIGITAL_MIC_SIDETONE_GAIN
                   - Sidetone gain. Consult the datasheet for further
                     information.
    STREAM_DIGITAL_MIC_SIDETONE_ENABLE
                   - Enable MIC sidetone.
    STREAM_DIGITAL_MIC_CLOCK_RATE
                   - Digital MIC clock rate in kHz. Valid values for clock rate
                     :  500 (500 kHz), 1000 (1 MHz), 2000 (2 MHz), 4000 (4 MHz).
    STREAM_DIGITAL_MIC_SIDETONE_SOURCE_POINT
                   - The sidetone source point. Selects source point for Digital
                     Mic data.
    STREAM_DIGITAL_MIC_INDIVIDUAL_SIDETONE_GAIN
                   - DMic sidetone gain. Consult the datasheet for further
                     information.
    STREAM_DIGITAL_MIC_DATA_SOURCE_POINT
                   - Digital Mic data source point selection. Digital Mic codec
                     data source can be selected from either IIR Filter out or
                     Digital Gain filter out
    STREAM_DIGITAL_MIC_ROUTE
                   - DMic ADC chain routing. 0: IIR filter followed by Digital
                     gain 1: Digital gain followed by IIR filter.
    STREAM_DIGITAL_MIC_G722_FILTER_ENABLE
                   - Enable optional G722 filter in ADC/DMIC chain. Improves
                     noise performance.
    STREAM_DIGITAL_MIC_G722_FIR_ENABLE
                   - Enable optional FIR filter in the G722 filter to droop the
                     response slightly.
    STREAM_DIGITAL_MIC_AMP_SEL
                   - DMic input amplitude select values. External Digital mic,
                     which usually has a sigma-delta modulator, provides one
                     bit(1 or 0) per sampling interval. However, Crescendo DMIC
                     hardware is capable of handling 3-bit(b000 to b111) input
                     per sampling interval. The key provides how a single bit
                     coming from a digital mic should be translated to a 3-bit
                     value. Usually, logic 1 is translated to a 7, and logic 0
                     is translated to 0. Values are chosen symmetrical around
                     3.5 to avoid any DC offset in the incoming signal. The
                     translation values for a logic 0 and logic 1 are selected
                     by the lower and higher 16-bits of of the value passed with
                     the key. The key can be used to invert an ADC channels
                     signal w.r.t. other ADC channel.
    STREAM_DIGITAL_MIC_CHAN_SWAP
                   - When set, swaps the digital mic channels.
    STREAM_AUDIO_MUTE_ENABLE
                   - Mute the audio. Valid values: 0 (un-mute), 1 (mute).
    STREAM_AUDIO_SAMPLE_SIZE
                   - Set Audio Sample Size. Valid values for PCM/I2S interface :
                       0 (13 bits in 16 cycle slot duration), 1 (16 bits in 16
                     cycle slot duration), 2 (8 bits in 16 cycle slot duration),
                     3 (8 bits in 8 cycle slot duration), 16 (16 bits)(same as
                     value 1) 24 (24 bits). Valid values for other interfaces:
                     16 (16 bit), 24 (24 bit).
    STREAM_AUDIO_SOURCE_METADATA_ENABLE
                   - Enable ToA metadata generation by audio source endpoint
    STREAM_AUDIO_SINK_DELAY
                   - Configure delay (microseconds) introduced by a particular
                     audio sink
    STREAM_AUDIO_SAMPLE_PERIOD_DEVIATION
                   - Allow restoring a previously obtained sp_deviation estimate
                     before starting (connecting) an endpoint
    STREAM_SINK_SHUNT_L2CAP_ATU
                   -
    STREAM_SINK_SHUNT_AUTOFLUSH_ENABLE
                   -
    STREAM_SINK_SHUNT_TOKENS_ALLOCATED
                   -
    STREAM_SOURCE_SHUNT_MAX_PDU_LENGTH
                   -
    STREAM_SCO_SRC_MAJORITY_VOTE_BYPASS
                   - Switch to bypass the majority voting in SCO SOURCE EP. 0:
                     MAJORITY VOTING NOT BYPASSED, 1: MAJORITY VOTING BYPASSED.
    STREAM_SCO_SRC_MAJORITY_VOTE_QUESTIONABLE_BITS_MAX
                   - Max. number of questionable bits per sco packet in the
                     majority voting to set status=valid
    STREAM_PWM_OUTPUT_RATE
                   - PWM output sample rate in hertz. Consult the datasheet for
                     supported sample rates.
    STREAM_PWM_OUTPUT_GAIN
                   - PWM output gain selection value. Consult the datasheet for
                     further information.
    STREAM_PWM_SIDETONE_GAIN
                   - PWM sidetone gain. Consult the datasheet for further
                     information.
    STREAM_PWM_SIDETONE_ENABLE
                   - Enable PWM sidetone.
    STREAM_PWM_OUTPUT_INTERP_FILTER_MODE
                   - Set the FIR mode for the PWM interpolation filter. Valid
                     values can be selected from
                     #codec_output_interpolation_filter_mode enumeration. The
                     FIR mode can be changed when the PWM is running, however
                     this may cause audio disturbances and is not recommended.
    STREAM_PWM_SIDETONE_SOURCE
                   - The sidetone source. It should be one of the following
                     Codec A/B, Digital Mic A/B, Digital Mic C/D, Digital Mic
                     E/F
    STREAM_PWM_SIDETONE_INJECTION_POINT
                   - The sidetone injection point. Selects sidetone injection
                     point into PWM-DAC chain.
    STREAM_PWM_SIDETONE_SOURCE_MASK
                   - The sidetone source mask. This key allows to select any
                     combination of 2 ADC/DMIC channels to be used as sidetone
                     source.
    STREAM_PWM_INDIVIDUAL_SIDETONE_ENABLE
                   - Enable sidetone for a particular PWM channel. Consult the
                     datasheet for further information.
    STREAM_PWM_SIDETONE_INVERT
                   - Invert sidetone phase for a particular PWM channel.
    STREAM_ANC_INSTANCE
                   - Set the ANC instance to be associated with an endpoint, if
                     any.
    STREAM_ANC_INPUT
                   - Set the ANC input to be associated with an input endpoint,
                     if any.
    STREAM_ANC_FFA_DC_FILTER_ENABLE
                   - Set the ANC FFA DC filter enable.
    STREAM_ANC_FFA_DC_FILTER_SHIFT
                   - Set the ANC FFA DC filter shift.
    STREAM_ANC_FFB_DC_FILTER_ENABLE
                   - Set the ANC FFB DC filter enable.
    STREAM_ANC_FFB_DC_FILTER_SHIFT
                   - Set the ANC FFB DC filter shift.
    STREAM_ANC_SM_LPF_FILTER_ENABLE
                   - Set the ANC SM LPF filter enable.
    STREAM_ANC_SM_LPF_FILTER_SHIFT
                   - Set the ANC SM LPF filter shift.
    STREAM_ANC_FFA_GAIN
                   - Set the ANC FFA path gain.
    STREAM_ANC_FFA_GAIN_SHIFT
                   - Set the ANC FFA path gain shift.
    STREAM_ANC_FFB_GAIN
                   - Set the ANC FFB path gain.
    STREAM_ANC_FFB_GAIN_SHIFT
                   - Set the ANC FFB path gain shift.
    STREAM_ANC_FB_GAIN
                   - Set the ANC FB path gain.
    STREAM_ANC_FB_GAIN_SHIFT
                   - Set the ANC FB path gain shift.
    STREAM_ANC_FFA_ADAPT_ENABLE
                   - Enable adaptive ANC on FFA path.
    STREAM_ANC_FFB_ADAPT_ENABLE
                   - Enable adaptive ANC on FFB path.
    STREAM_ANC_FB_ADAPT_ENABLE
                   - Enable adaptive ANC on FB path.
    STREAM_ANC_CONTROL
                   - Set ANC controls.
    STREAM_RM_ENABLE_SW_ADJUST
                   - Enable software rate adjustment on an endpoint. Default is
                     true (non-zero). Set to false (zero) to force adjustment to
                     occur on another endpoint.
    STREAM_RM_ENABLE_HW_ADJUST
                   - Enable hardware rate adjustment on an endpoint. Default is
                     true (non-zero). Set to false (zero) to force software rate
                     adjustment to be used, or adjustment to occur on another
                     endpoint.
    STREAM_RM_ENABLE_FAST_MEASUREMENT
                   - Enable fast, rounding-error-free rate measurements. Default
                     is true for endpoints which support this. Set to zero to
                     only provide slower approximate rate measurements.
    STREAM_RM_ENABLE_FAST_MATCHING
                   - Enable fast, rounding-error-free rate matching. Default is
                     true for endpoints which support this. Set to zero to use
                     the slower approximate rate matching method.
    STREAM_RM_ENABLE_VARIABLE_RESPONSIVENESS
                   - Enable slower responses while rate matching appears
                     settled. Default is true for endpoints which support this.
                     Set to zero to always use fastest response, at the expense
                     of more small rate variations in settled state.
    STREAM_RM_ENABLE_DEFERRED_KICK
                   - Value is boolean. If true, endpoint types which implement
                     this perform most of their kick processing in a high
                     priority background interrupt instead of in the
                     (foreground) interrupt context.
    STREAM_RM_RATE_MATCH_TRACE
                   - Value is boolean. If true, causes the endpoint to print
                     rate match state variables to the audio log.

*******************************************************************************/
typedef enum
{
    ACCMD_CONFIG_KEY_STREAM_SOURCE_NOTIFY_WHEN_DRAINED = 0x000A,
    ACCMD_CONFIG_KEY_STREAM_PCM_SYNC_RATE = 0x0100,
    ACCMD_CONFIG_KEY_STREAM_PCM_MASTER_CLOCK_RATE = 0x0101,
    ACCMD_CONFIG_KEY_STREAM_PCM_MASTER_MODE = 0x0102,
    ACCMD_CONFIG_KEY_STREAM_PCM_SLOT_COUNT = 0x0103,
    ACCMD_CONFIG_KEY_STREAM_PCM_MANCH_MODE_ENABLE = 0x0104,
    ACCMD_CONFIG_KEY_STREAM_PCM_SHORT_SYNC_ENABLE = 0x0105,
    ACCMD_CONFIG_KEY_STREAM_PCM_MANCH_SLAVE_MODE_ENABLE = 0x0106,
    ACCMD_CONFIG_KEY_STREAM_PCM_SIGN_EXTEND_ENABLE = 0x0107,
    ACCMD_CONFIG_KEY_STREAM_PCM_LSB_FIRST_ENABLE = 0x0108,
    ACCMD_CONFIG_KEY_STREAM_PCM_TX_TRISTATE_ENABLE = 0x0109,
    ACCMD_CONFIG_KEY_STREAM_PCM_TX_TRISTATE_RISING_EDGE_ENABLE = 0x010a,
    ACCMD_CONFIG_KEY_STREAM_PCM_SYNC_SUPPRESS_ENABLE = 0x010b,
    ACCMD_CONFIG_KEY_STREAM_PCM_GCI_MODE_ENABLE = 0x010c,
    ACCMD_CONFIG_KEY_STREAM_PCM_MUTE_ENABLE = 0x010d,
    ACCMD_CONFIG_KEY_STREAM_PCM_LONG_LENGTH_SYNC_ENABLE = 0x010e,
    ACCMD_CONFIG_KEY_STREAM_PCM_SAMPLE_RISING_EDGE_ENABLE = 0x010f,
    ACCMD_CONFIG_KEY_STREAM_PCM_SAMPLE_FORMAT = 0x0114,
    ACCMD_CONFIG_KEY_STREAM_PCM_MANCH_RX_OFFSET = 0x0115,
    ACCMD_CONFIG_KEY_STREAM_PCM_AUDIO_GAIN = 0x0116,
    ACCMD_CONFIG_KEY_STREAM_PCM_MASTER_CLK_SOURCE = 0x0118,
    ACCMD_CONFIG_KEY_STREAM_PCM_MASTER_MCLK_MULT = 0x0119,
    ACCMD_CONFIG_KEY_STREAM_I2S_SYNC_RATE = 0x0200,
    ACCMD_CONFIG_KEY_STREAM_I2S_MASTER_CLOCK_RATE = 0x0201,
    ACCMD_CONFIG_KEY_STREAM_I2S_MASTER_MODE = 0x0202,
    ACCMD_CONFIG_KEY_STREAM_I2S_JSTFY_FORMAT = 0x0203,
    ACCMD_CONFIG_KEY_STREAM_I2S_LFT_JSTFY_DLY = 0x0204,
    ACCMD_CONFIG_KEY_STREAM_I2S_CHNL_PLRTY = 0x0205,
    ACCMD_CONFIG_KEY_STREAM_I2S_AUDIO_ATTEN_ENABLE = 0x0206,
    ACCMD_CONFIG_KEY_STREAM_I2S_AUDIO_ATTEN = 0x0207,
    ACCMD_CONFIG_KEY_STREAM_I2S_JSTFY_RES = 0x0208,
    ACCMD_CONFIG_KEY_STREAM_I2S_CROP_ENABLE = 0x0209,
    ACCMD_CONFIG_KEY_STREAM_I2S_BITS_PER_SAMPLE = 0x020a,
    ACCMD_CONFIG_KEY_STREAM_I2S_TX_START_SAMPLE = 0x020b,
    ACCMD_CONFIG_KEY_STREAM_I2S_RX_START_SAMPLE = 0x020c,
    ACCMD_CONFIG_KEY_STREAM_I2S_MASTER_CLK_SOURCE = 0x020d,
    ACCMD_CONFIG_KEY_STREAM_I2S_MASTER_MCLK_MULT = 0x020e,
    ACCMD_CONFIG_KEY_STREAM_CODEC_INPUT_RATE = 0x0300,
    ACCMD_CONFIG_KEY_STREAM_CODEC_OUTPUT_RATE = 0x0301,
    ACCMD_CONFIG_KEY_STREAM_CODEC_INPUT_GAIN = 0x0302,
    ACCMD_CONFIG_KEY_STREAM_CODEC_OUTPUT_GAIN = 0x0303,
    ACCMD_CONFIG_KEY_STREAM_CODEC_RAW_INPUT_GAIN = 0x0304,
    ACCMD_CONFIG_KEY_STREAM_CODEC_RAW_OUTPUT_GAIN = 0x0305,
    ACCMD_CONFIG_KEY_STREAM_CODEC_OUTPUT_GAIN_BOOST_ENABLE = 0x0306,
    ACCMD_CONFIG_KEY_STREAM_CODEC_SIDETONE_GAIN = 0x0307,
    ACCMD_CONFIG_KEY_STREAM_CODEC_SIDETONE_ENABLE = 0x0308,
    ACCMD_CONFIG_KEY_STREAM_CODEC_LOW_POWER_OUTPUT_STAGE_ENABLE = 0x030a,
    ACCMD_CONFIG_KEY_STREAM_CODEC_OUTPUT_INTERP_FILTER_MODE = 0x030c,
    ACCMD_CONFIG_KEY_STREAM_CODEC_SIDETONE_SOURCE = 0x030e,
    ACCMD_CONFIG_KEY_STREAM_CODEC_SIDETONE_SOURCE_POINT = 0x030f,
    ACCMD_CONFIG_KEY_STREAM_CODEC_SIDETONE_INJECTION_POINT = 0x0310,
    ACCMD_CONFIG_KEY_STREAM_CODEC_SIDETONE_SOURCE_MASK = 0x0311,
    ACCMD_CONFIG_KEY_STREAM_CODEC_INDIVIDUAL_SIDETONE_GAIN = 0x0312,
    ACCMD_CONFIG_KEY_STREAM_CODEC_INDIVIDUAL_SIDETONE_ENABLE = 0x0313,
    ACCMD_CONFIG_KEY_STREAM_CODEC_ADC_DATA_SOURCE_POINT = 0x0314,
    ACCMD_CONFIG_KEY_STREAM_CODEC_ADC_ROUTE = 0x0315,
    ACCMD_CONFIG_KEY_STREAM_CODEC_SIDETONE_INVERT = 0x0316,
    ACCMD_CONFIG_KEY_STREAM_CODEC_G722_FILTER_ENABLE = 0x0317,
    ACCMD_CONFIG_KEY_STREAM_CODEC_G722_FIR_ENABLE = 0x0318,
    ACCMD_CONFIG_KEY_STREAM_CODEC_INPUT_TERMINATION = 0x0319,
    ACCMD_CONFIG_KEY_STREAM_CODEC_OUTPUT_QUALITY = 0x031a,
    ACCMD_CONFIG_KEY_STREAM_SPDIF_OUTPUT_RATE = 0x0500,
    ACCMD_CONFIG_KEY_STREAM_SPDIF_CHNL_STS_REPORT_MODE = 0x0501,
    ACCMD_CONFIG_KEY_STREAM_SPDIF_OUTPUT_CHNL_STS_DUP_EN = 0x0502,
    ACCMD_CONFIG_KEY_STREAM_SPDIF_OUTPUT_CHNL_STS_WORD = 0x0503,
    ACCMD_CONFIG_KEY_STREAM_SPDIF_AUTO_RATE_DETECT = 0x0504,
    ACCMD_CONFIG_KEY_STREAM_SPDIF_SET_EP_FORMAT = 0x0505,
    ACCMD_CONFIG_KEY_STREAM_SPDIF_OUTPUT_CLK_SOURCE = 0x0506,
    ACCMD_CONFIG_KEY_STREAM_DIGITAL_MIC_INPUT_RATE = 0x0600,
    ACCMD_CONFIG_KEY_STREAM_DIGITAL_MIC_INPUT_GAIN = 0x0601,
    ACCMD_CONFIG_KEY_STREAM_DIGITAL_MIC_SIDETONE_GAIN = 0x0602,
    ACCMD_CONFIG_KEY_STREAM_DIGITAL_MIC_SIDETONE_ENABLE = 0x0603,
    ACCMD_CONFIG_KEY_STREAM_DIGITAL_MIC_CLOCK_RATE = 0x0604,
    ACCMD_CONFIG_KEY_STREAM_DIGITAL_MIC_SIDETONE_SOURCE_POINT = 0x0605,
    ACCMD_CONFIG_KEY_STREAM_DIGITAL_MIC_INDIVIDUAL_SIDETONE_GAIN = 0x0606,
    ACCMD_CONFIG_KEY_STREAM_DIGITAL_MIC_DATA_SOURCE_POINT = 0x0607,
    ACCMD_CONFIG_KEY_STREAM_DIGITAL_MIC_ROUTE = 0x0608,
    ACCMD_CONFIG_KEY_STREAM_DIGITAL_MIC_G722_FILTER_ENABLE = 0x0609,
    ACCMD_CONFIG_KEY_STREAM_DIGITAL_MIC_G722_FIR_ENABLE = 0x060a,
    ACCMD_CONFIG_KEY_STREAM_DIGITAL_MIC_AMP_SEL = 0x060b,
    ACCMD_CONFIG_KEY_STREAM_DIGITAL_MIC_CHAN_SWAP = 0x060c,
    ACCMD_CONFIG_KEY_STREAM_AUDIO_MUTE_ENABLE = 0x0700,
    ACCMD_CONFIG_KEY_STREAM_AUDIO_SAMPLE_SIZE = 0x0701,
    ACCMD_CONFIG_KEY_STREAM_AUDIO_SOURCE_METADATA_ENABLE = 0x0703,
    ACCMD_CONFIG_KEY_STREAM_AUDIO_SINK_DELAY = 0x0704,
    ACCMD_CONFIG_KEY_STREAM_AUDIO_SAMPLE_PERIOD_DEVIATION = 0x0705,
    ACCMD_CONFIG_KEY_STREAM_SINK_SHUNT_L2CAP_ATU = 0x0800,
    ACCMD_CONFIG_KEY_STREAM_SINK_SHUNT_AUTOFLUSH_ENABLE = 0x0801,
    ACCMD_CONFIG_KEY_STREAM_SINK_SHUNT_TOKENS_ALLOCATED = 0x0802,
    ACCMD_CONFIG_KEY_STREAM_SOURCE_SHUNT_MAX_PDU_LENGTH = 0x0803,
    ACCMD_CONFIG_KEY_STREAM_SCO_SRC_MAJORITY_VOTE_BYPASS = 0x0902,
    ACCMD_CONFIG_KEY_STREAM_SCO_SRC_MAJORITY_VOTE_QUESTIONABLE_BITS_MAX = 0x0903,
    ACCMD_CONFIG_KEY_STREAM_PWM_OUTPUT_RATE = 0x1000,
    ACCMD_CONFIG_KEY_STREAM_PWM_OUTPUT_GAIN = 0x1001,
    ACCMD_CONFIG_KEY_STREAM_PWM_SIDETONE_GAIN = 0x1002,
    ACCMD_CONFIG_KEY_STREAM_PWM_SIDETONE_ENABLE = 0x1003,
    ACCMD_CONFIG_KEY_STREAM_PWM_OUTPUT_INTERP_FILTER_MODE = 0x1004,
    ACCMD_CONFIG_KEY_STREAM_PWM_SIDETONE_SOURCE = 0x1005,
    ACCMD_CONFIG_KEY_STREAM_PWM_SIDETONE_INJECTION_POINT = 0x1006,
    ACCMD_CONFIG_KEY_STREAM_PWM_SIDETONE_SOURCE_MASK = 0x1007,
    ACCMD_CONFIG_KEY_STREAM_PWM_INDIVIDUAL_SIDETONE_ENABLE = 0x1008,
    ACCMD_CONFIG_KEY_STREAM_PWM_SIDETONE_INVERT = 0x1009,
    ACCMD_CONFIG_KEY_STREAM_ANC_INSTANCE = 0x1100,
    ACCMD_CONFIG_KEY_STREAM_ANC_INPUT = 0x1101,
    ACCMD_CONFIG_KEY_STREAM_ANC_FFA_DC_FILTER_ENABLE = 0x1102,
    ACCMD_CONFIG_KEY_STREAM_ANC_FFA_DC_FILTER_SHIFT = 0x1103,
    ACCMD_CONFIG_KEY_STREAM_ANC_FFB_DC_FILTER_ENABLE = 0x1104,
    ACCMD_CONFIG_KEY_STREAM_ANC_FFB_DC_FILTER_SHIFT = 0x1105,
    ACCMD_CONFIG_KEY_STREAM_ANC_SM_LPF_FILTER_ENABLE = 0x1106,
    ACCMD_CONFIG_KEY_STREAM_ANC_SM_LPF_FILTER_SHIFT = 0x1107,
    ACCMD_CONFIG_KEY_STREAM_ANC_FFA_GAIN = 0x1108,
    ACCMD_CONFIG_KEY_STREAM_ANC_FFA_GAIN_SHIFT = 0x1109,
    ACCMD_CONFIG_KEY_STREAM_ANC_FFB_GAIN = 0x110a,
    ACCMD_CONFIG_KEY_STREAM_ANC_FFB_GAIN_SHIFT = 0x110b,
    ACCMD_CONFIG_KEY_STREAM_ANC_FB_GAIN = 0x110c,
    ACCMD_CONFIG_KEY_STREAM_ANC_FB_GAIN_SHIFT = 0x110d,
    ACCMD_CONFIG_KEY_STREAM_ANC_FFA_ADAPT_ENABLE = 0x110e,
    ACCMD_CONFIG_KEY_STREAM_ANC_FFB_ADAPT_ENABLE = 0x110f,
    ACCMD_CONFIG_KEY_STREAM_ANC_FB_ADAPT_ENABLE = 0x1110,
    ACCMD_CONFIG_KEY_STREAM_ANC_CONTROL = 0x1111,
    ACCMD_CONFIG_KEY_STREAM_RM_ENABLE_SW_ADJUST = 0x1200,
    ACCMD_CONFIG_KEY_STREAM_RM_ENABLE_HW_ADJUST = 0x1201,
    ACCMD_CONFIG_KEY_STREAM_RM_ENABLE_FAST_MEASUREMENT = 0x1202,
    ACCMD_CONFIG_KEY_STREAM_RM_ENABLE_FAST_MATCHING = 0x1203,
    ACCMD_CONFIG_KEY_STREAM_RM_ENABLE_VARIABLE_RESPONSIVENESS = 0x1204,
    ACCMD_CONFIG_KEY_STREAM_RM_ENABLE_DEFERRED_KICK = 0x1205,
    ACCMD_CONFIG_KEY_STREAM_RM_RATE_MATCH_TRACE = 0x1207
} ACCMD_CONFIG_KEY;
/*******************************************************************************

  NAME
    Accmd_Cpu_Clk

  DESCRIPTION
    NO_CHANGE is only used by the request. Accmd_Status will indicate an
    error at the request reserved for future use

 VALUES
    NO_CHANGE       -
    EXT_LP_CLOCK    -
    VERY_LP_CLOCK   -
    LP_CLOCK        -
    VERY_SLOW_CLOCK -
    SLOW_CLOCK      -
    BASE_CLOCK      -
    TURBO           -
    TURBO_PLUS      -

*******************************************************************************/
typedef enum
{
    ACCMD_CPU_CLK_NO_CHANGE = 0,
    ACCMD_CPU_CLK_EXT_LP_CLOCK = 1,
    ACCMD_CPU_CLK_VERY_LP_CLOCK = 2,
    ACCMD_CPU_CLK_LP_CLOCK = 3,
    ACCMD_CPU_CLK_VERY_SLOW_CLOCK = 4,
    ACCMD_CPU_CLK_SLOW_CLOCK = 5,
    ACCMD_CPU_CLK_BASE_CLOCK = 6,
    ACCMD_CPU_CLK_TURBO = 7,
    ACCMD_CPU_CLK_TURBO_PLUS = 8
} ACCMD_CPU_CLK;
/*******************************************************************************

  NAME
    Accmd_Info_Key

  DESCRIPTION
    Audio Hardware Instances

 VALUES
    AUDIO_SAMPLE_RATE             - Request the Audio Sample rate of an audio
                                    sink/source, measured in Hz
    AUDIO_LOCALLY_CLOCKED         - Request whether the audio hardware is
                                    locally or remotely clocked
    AUDIO_INSTANCE                - Request the hardware instance of an audio
                                    sink/source
    AUDIO_CHANNEL                 - Request the hardware channel of an audio
                                    sink/source
    AUDIO_SAMPLE_FORMAT           - Request the Audio Sample format, i.e.
                                    8,13,16 or 24 bit
    AUDIO_CLOCK_SOURCE_TYPE       - Request the clock source type
    MASTER_CLOCK_RATE             - Request the MCLK clock rate that is required
                                    by the endpoint, in Hz. Only S/PDIF TX and
                                    I2S/PCM in master mode can use MCLK.
    AUDIO_HW_RM_AVAILABLE         - Request whether the audio hardware supports
                                    warp
    ENDPOINT_EXISTS               - Requests whether the given sid corresponds
                                    to an endpoint that currently exists.
                                    Returns success if it does, failure if it
                                    doesn't. Returned value is meaningless and
                                    should be ignored.
    AUDIO_SAMPLE_PERIOD_DEVIATION - Requests the last estimate for the
                                    endpoint's sample period deviation from
                                    nominal, as a fractional number.

*******************************************************************************/
typedef enum
{
    ACCMD_INFO_KEY_AUDIO_SAMPLE_RATE = 0x0000,
    ACCMD_INFO_KEY_AUDIO_LOCALLY_CLOCKED = 0x0001,
    ACCMD_INFO_KEY_AUDIO_INSTANCE = 0x0002,
    ACCMD_INFO_KEY_AUDIO_CHANNEL = 0x0003,
    ACCMD_INFO_KEY_AUDIO_SAMPLE_FORMAT = 0x0004,
    ACCMD_INFO_KEY_AUDIO_CLOCK_SOURCE_TYPE = 0x0005,
    ACCMD_INFO_KEY_MASTER_CLOCK_RATE = 0x0006,
    ACCMD_INFO_KEY_AUDIO_HW_RM_AVAILABLE = 0x0007,
    ACCMD_INFO_KEY_ENDPOINT_EXISTS = 0x0008,
    ACCMD_INFO_KEY_AUDIO_SAMPLE_PERIOD_DEVIATION = 0x0009
} ACCMD_INFO_KEY;
/*******************************************************************************

  NAME
    Accmd_Mic_Bias_Config_Key

  DESCRIPTION
    Other keys ought to be allocated to match the numbers in
    interface/app/mic_bias/mic_bias_if.h; but no HydraCore chip supports any
    of those other mic bias hardware features.

 VALUES
    MIC_BIAS_ENABLE  - Enable or disable the microphone bias. The value is a
                       member of 'Accmd_Mic_Bias_Enable'.
    MIC_BIAS_VOLTAGE - Set the microphone bias voltage. The mapping from value
                       to voltage is chip dependent, so consult the datasheet.

*******************************************************************************/
typedef enum
{
    ACCMD_MIC_BIAS_CONFIG_KEY_MIC_BIAS_ENABLE = 0x0000,
    ACCMD_MIC_BIAS_CONFIG_KEY_MIC_BIAS_VOLTAGE = 0x0001
} ACCMD_MIC_BIAS_CONFIG_KEY;
/*******************************************************************************

  NAME
    Accmd_Mic_Bias_Enable

  DESCRIPTION
    NOTE: HydraCore does not support the value MIC_BIAS_AUTOMATIC_ON that
    BlueCore had; and the numeric value of MIC_BIAS_FORCE_ON is different
    from BlueCore (where it was 2).

 VALUES
    MIC_BIAS_OFF      - Disable the microphone bias immediately.
    MIC_BIAS_FORCE_ON - Enable the microphone bias immediately.

*******************************************************************************/
typedef enum
{
    ACCMD_MIC_BIAS_ENABLE_MIC_BIAS_OFF = 0x0000,
    ACCMD_MIC_BIAS_ENABLE_MIC_BIAS_FORCE_ON = 0x0001
} ACCMD_MIC_BIAS_ENABLE;
/*******************************************************************************

  NAME
    Accmd_Mic_Bias_Id

  DESCRIPTION
    Only one mic_bias instance on HydraCore chips to date

 VALUES
    MIC_BIAS_0 -

*******************************************************************************/
typedef enum
{
    ACCMD_MIC_BIAS_ID_MIC_BIAS_0 = 0x0000
} ACCMD_MIC_BIAS_ID;
/*******************************************************************************

  NAME
    Accmd_Power_Save_Modes

  DESCRIPTION
    reserved for future use

 VALUES
    PM_QUERY -
    PM1      -
    PM2      -
    PM3      -
    PM4      -
    PM5      -

*******************************************************************************/
typedef enum
{
    ACCMD_POWER_SAVE_MODES_PM_QUERY = 0,
    ACCMD_POWER_SAVE_MODES_PM1 = 1,
    ACCMD_POWER_SAVE_MODES_PM2 = 2,
    ACCMD_POWER_SAVE_MODES_PM3 = 3,
    ACCMD_POWER_SAVE_MODES_PM4 = 4,
    ACCMD_POWER_SAVE_MODES_PM5 = 5
} ACCMD_POWER_SAVE_MODES;
/*******************************************************************************

  NAME
    Accmd_Signal_Id

  DESCRIPTION
    Request messages Response messages Test Request messages 0x2003 formerly
    "SET_DEBUG_ENABLES_REQ", now unused. Test response messages Debug
    messages, sent as indications 0x8000 formerly "DEBUG_MESSAGE_IND", now
    unused. 0x8002 formerly "DEBUG_SEND_CONFIG", now unused.

 VALUES
    NO_CMD_REQ                       -
    STREAM_GET_SOURCE_REQ            -
    STREAM_GET_SINK_REQ              -
    STREAM_CLOSE_SOURCE_REQ          -
    STREAM_CLOSE_SINK_REQ            -
    STREAM_CONFIGURE_REQ             -
    STREAM_SYNC_SID_REQ              -
    STREAM_CONNECT_REQ               -
    STREAM_TRANSFORM_DISCONNECT_REQ  -
    GET_FIRMWARE_VERSION_REQ         -
    CREATE_OPERATOR_REQ              -
    DESTROY_OPERATORS_REQ            -
    OPERATOR_MESSAGE_REQ             -
    START_OPERATORS_REQ              -
    STOP_OPERATORS_REQ               -
    RESET_OPERATORS_REQ              -
    MESSAGE_FROM_OPERATOR_REQ        -
    GET_FIRMWARE_ID_STRING_REQ       -
    PRODTEST_REQ                     -
    STREAM_GET_INFO_REQ              -
    GET_CAPABILITIES_REQ             -
    STREAM_EXTERNAL_DETACH_REQ       -
    STREAM_SOURCE_DRAINED_REQ        -
    CREATE_OPERATOR_EX_REQ           -
    GET_MEM_USAGE_REQ                -
    CLEAR_MEM_WATERMARKS_REQ         -
    GET_MIPS_USAGE_REQ               -
    SET_SYSTEM_STREAM_RATE_REQ       -
    GET_SYSTEM_STREAM_RATE_REQ       -
    STREAM_DISCONNECT_REQ            -
    STREAM_GET_SINK_FROM_SOURCE_REQ  -
    STREAM_GET_SOURCE_FROM_SINK_REQ  -
    CAP_DOWNLOAD_REMOVE_KCS_REQ      -
    SET_SYSTEM_FRAMEWORK_REQ         -
    GET_SYSTEM_FRAMEWORK_REQ         -
    UNSOLICITED_SYSTEM_FRAMEWORK_REQ -
    MIC_BIAS_CONFIGURE_REQ           -
    CODEC_SET_IIR_FILTER_REQ         -
    CODEC_SET_IIR_FILTER_16BIT_REQ   -
    AUX_PROCESSOR_LOAD_REQ           -
    AUX_PROCESSOR_UNLOAD_REQ         -
    AUX_PROCESSOR_START_REQ          -
    AUX_PROCESSOR_STOP_REQ           -
    STREAM_MCLK_ACTIVATE_REQ         -
    SET_MCLK_SOURCE_TYPE_REQ         -
    GET_OPERATORS_REQ                -
    PS_SHUTDOWN_REQ                  -
    PS_READ_REQ                      -
    PS_WRITE_REQ                     -
    PS_DELETE_REQ                    -
    PS_SHUTDOWN_COMPLETE_REQ         -
    PS_REGISTER_REQ                  -
    SET_PROFILER_REQ                 -
    CLOCK_CONFIGURE_REQ              -
    SET_ANC_IIR_FILTER_REQ           -
    SET_ANC_LPF_FILTER_REQ           -
    STREAM_ANC_ENABLE_REQ            -
    STREAM_SIDETONE_EN_REQ           -
    DATA_FILE_ALLOCATE_REQ           -
    DATA_FILE_DEALLOCATE_REQ         -
    CLOCK_POWER_SAVE_MODE_REQ        -
    CAP_DOWNLOAD_ADD_KCS_REQ         -
    CAP_DOWNLOAD_COMPLETE_KCS_REQ    -
    AUX_PROCESSOR_DISABLE_REQ        -
    TRANSFORM_FROM_STREAM_REQ        -
    SET_PANIC_CODE_REQ               -
    SET_FAULT_CODE_REQ               -
    SET_PANIC_TRIGGER_REQ            -
    SET_IPC_INTERFACE_TEST_REQ       -
    CALL_FUNCTION_REQ                -
    NO_CMD_RESP                      -
    STREAM_GET_SOURCE_RESP           -
    STREAM_GET_SINK_RESP             -
    STANDARD_RESP                    -
    STREAM_CONNECT_RESP              -
    GET_FIRMWARE_VERSION_RESP        -
    CREATE_OPERATOR_RESP             -
    DESTROY_OPERATORS_RESP           -
    OPERATOR_MESSAGE_RESP            -
    START_OPERATORS_RESP             -
    STOP_OPERATORS_RESP              -
    RESET_OPERATORS_RESP             -
    GET_FIRMWARE_ID_STRING_RESP      -
    PRODTEST_RESP                    -
    STREAM_GET_INFO_RESP             -
    GET_CAPABILITIES_RESP            -
    CREATE_OPERATOR_EX_RESP          -
    GET_MEM_USAGE_RESP               -
    CLEAR_MEM_WATERMARKS_RESP        -
    GET_MIPS_USAGE_RESP              -
    SET_SYSTEM_STREAM_RATE_RESP      -
    GET_SYSTEM_STREAM_RATE_RESP      -
    STREAM_DISCONNECT_RESP           -
    STREAM_GET_SINK_FROM_SOURCE_RESP -
    STREAM_GET_SOURCE_FROM_SINK_RESP -
    SET_SYSTEM_FRAMEWORK_RESP        -
    GET_SYSTEM_FRAMEWORK_RESP        -
    GET_IPC_INTERFACE_TEST_RESP      -
    GET_OPERATORS_RESP               -
    PS_SHUTDOWN_RESP                 -
    PS_READ_RESP                     -
    PS_WRITE_RESP                    -
    PS_DELETE_RESP                   -
    PS_SHUTDOWN_COMPLETE_RESP        -
    TRANSFORM_FROM_STREAM_RESP       -
    SET_PROFILER_RESP                -
    CLOCK_CONFIGURE_RESP             -
    DATA_FILE_ALLOCATE_RESP          -
    CLOCK_POWER_SAVE_MODE_RESP       -
    CALL_FUNCTION_RESP               -

*******************************************************************************/
typedef enum
{
    ACCMD_SIGNAL_ID_NO_CMD_REQ = 0x0000,
    ACCMD_SIGNAL_ID_STREAM_GET_SOURCE_REQ = 0x0001,
    ACCMD_SIGNAL_ID_STREAM_GET_SINK_REQ = 0x0002,
    ACCMD_SIGNAL_ID_STREAM_CLOSE_SOURCE_REQ = 0x0003,
    ACCMD_SIGNAL_ID_STREAM_CLOSE_SINK_REQ = 0x0004,
    ACCMD_SIGNAL_ID_STREAM_CONFIGURE_REQ = 0x0005,
    ACCMD_SIGNAL_ID_STREAM_SYNC_SID_REQ = 0x0006,
    ACCMD_SIGNAL_ID_STREAM_CONNECT_REQ = 0x0007,
    ACCMD_SIGNAL_ID_STREAM_TRANSFORM_DISCONNECT_REQ = 0x0008,
    ACCMD_SIGNAL_ID_GET_FIRMWARE_VERSION_REQ = 0x0009,
    ACCMD_SIGNAL_ID_CREATE_OPERATOR_REQ = 0x000A,
    ACCMD_SIGNAL_ID_DESTROY_OPERATORS_REQ = 0x000B,
    ACCMD_SIGNAL_ID_OPERATOR_MESSAGE_REQ = 0x000C,
    ACCMD_SIGNAL_ID_START_OPERATORS_REQ = 0x000D,
    ACCMD_SIGNAL_ID_STOP_OPERATORS_REQ = 0x000E,
    ACCMD_SIGNAL_ID_RESET_OPERATORS_REQ = 0x000F,
    ACCMD_SIGNAL_ID_MESSAGE_FROM_OPERATOR_REQ = 0x0010,
    ACCMD_SIGNAL_ID_GET_FIRMWARE_ID_STRING_REQ = 0x0011,
    ACCMD_SIGNAL_ID_PRODTEST_REQ = 0x0012,
    ACCMD_SIGNAL_ID_STREAM_GET_INFO_REQ = 0x0013,
    ACCMD_SIGNAL_ID_GET_CAPABILITIES_REQ = 0x0014,
    ACCMD_SIGNAL_ID_STREAM_EXTERNAL_DETACH_REQ = 0x0015,
    ACCMD_SIGNAL_ID_STREAM_SOURCE_DRAINED_REQ = 0x0016,
    ACCMD_SIGNAL_ID_CREATE_OPERATOR_EX_REQ = 0x0017,
    ACCMD_SIGNAL_ID_GET_MEM_USAGE_REQ = 0x0018,
    ACCMD_SIGNAL_ID_CLEAR_MEM_WATERMARKS_REQ = 0x0019,
    ACCMD_SIGNAL_ID_GET_MIPS_USAGE_REQ = 0x001A,
    ACCMD_SIGNAL_ID_SET_SYSTEM_STREAM_RATE_REQ = 0x001B,
    ACCMD_SIGNAL_ID_GET_SYSTEM_STREAM_RATE_REQ = 0x001C,
    ACCMD_SIGNAL_ID_STREAM_DISCONNECT_REQ = 0x001D,
    ACCMD_SIGNAL_ID_STREAM_GET_SINK_FROM_SOURCE_REQ = 0x001E,
    ACCMD_SIGNAL_ID_STREAM_GET_SOURCE_FROM_SINK_REQ = 0x001F,
    ACCMD_SIGNAL_ID_CAP_DOWNLOAD_REMOVE_KCS_REQ = 0x0020,
    ACCMD_SIGNAL_ID_SET_SYSTEM_FRAMEWORK_REQ = 0x0021,
    ACCMD_SIGNAL_ID_GET_SYSTEM_FRAMEWORK_REQ = 0x0022,
    ACCMD_SIGNAL_ID_UNSOLICITED_SYSTEM_FRAMEWORK_REQ = 0x0023,
    ACCMD_SIGNAL_ID_MIC_BIAS_CONFIGURE_REQ = 0x0024,
    ACCMD_SIGNAL_ID_CODEC_SET_IIR_FILTER_REQ = 0x0025,
    ACCMD_SIGNAL_ID_CODEC_SET_IIR_FILTER_16BIT_REQ = 0x0026,
    ACCMD_SIGNAL_ID_AUX_PROCESSOR_LOAD_REQ = 0x0027,
    ACCMD_SIGNAL_ID_AUX_PROCESSOR_UNLOAD_REQ = 0x0028,
    ACCMD_SIGNAL_ID_AUX_PROCESSOR_START_REQ = 0x0029,
    ACCMD_SIGNAL_ID_AUX_PROCESSOR_STOP_REQ = 0x002A,
    ACCMD_SIGNAL_ID_STREAM_MCLK_ACTIVATE_REQ = 0x002B,
    ACCMD_SIGNAL_ID_SET_MCLK_SOURCE_TYPE_REQ = 0x002C,
    ACCMD_SIGNAL_ID_GET_OPERATORS_REQ = 0x002D,
    ACCMD_SIGNAL_ID_PS_SHUTDOWN_REQ = 0x002E,
    ACCMD_SIGNAL_ID_PS_READ_REQ = 0x002F,
    ACCMD_SIGNAL_ID_PS_WRITE_REQ = 0x0030,
    ACCMD_SIGNAL_ID_PS_DELETE_REQ = 0x0031,
    ACCMD_SIGNAL_ID_PS_SHUTDOWN_COMPLETE_REQ = 0x0032,
    ACCMD_SIGNAL_ID_PS_REGISTER_REQ = 0x0033,
    ACCMD_SIGNAL_ID_SET_PROFILER_REQ = 0x0034,
    ACCMD_SIGNAL_ID_CLOCK_CONFIGURE_REQ = 0x0035,
    ACCMD_SIGNAL_ID_SET_ANC_IIR_FILTER_REQ = 0x0036,
    ACCMD_SIGNAL_ID_SET_ANC_LPF_FILTER_REQ = 0x0037,
    ACCMD_SIGNAL_ID_STREAM_ANC_ENABLE_REQ = 0x0038,
    ACCMD_SIGNAL_ID_STREAM_SIDETONE_EN_REQ = 0x0039,
    ACCMD_SIGNAL_ID_DATA_FILE_ALLOCATE_REQ = 0x003A,
    ACCMD_SIGNAL_ID_DATA_FILE_DEALLOCATE_REQ = 0x003B,
    ACCMD_SIGNAL_ID_CLOCK_POWER_SAVE_MODE_REQ = 0x003C,
    ACCMD_SIGNAL_ID_CAP_DOWNLOAD_ADD_KCS_REQ = 0x003D,
    ACCMD_SIGNAL_ID_CAP_DOWNLOAD_COMPLETE_KCS_REQ = 0x003E,
    ACCMD_SIGNAL_ID_AUX_PROCESSOR_DISABLE_REQ = 0x003F,
    ACCMD_SIGNAL_ID_TRANSFORM_FROM_STREAM_REQ = 0x0040,
    ACCMD_SIGNAL_ID_SET_PANIC_CODE_REQ = 0x2001,
    ACCMD_SIGNAL_ID_SET_FAULT_CODE_REQ = 0x2002,
    ACCMD_SIGNAL_ID_SET_PANIC_TRIGGER_REQ = 0x2004,
    ACCMD_SIGNAL_ID_SET_IPC_INTERFACE_TEST_REQ = 0x2005,
    ACCMD_SIGNAL_ID_CALL_FUNCTION_REQ = 0x2006,
    ACCMD_SIGNAL_ID_NO_CMD_RESP = 0x4000,
    ACCMD_SIGNAL_ID_STREAM_GET_SOURCE_RESP = 0x4001,
    ACCMD_SIGNAL_ID_STREAM_GET_SINK_RESP = 0x4002,
    ACCMD_SIGNAL_ID_STANDARD_RESP = 0x4003,
    ACCMD_SIGNAL_ID_STREAM_CONNECT_RESP = 0x4004,
    ACCMD_SIGNAL_ID_GET_FIRMWARE_VERSION_RESP = 0x4005,
    ACCMD_SIGNAL_ID_CREATE_OPERATOR_RESP = 0x4006,
    ACCMD_SIGNAL_ID_DESTROY_OPERATORS_RESP = 0x4007,
    ACCMD_SIGNAL_ID_OPERATOR_MESSAGE_RESP = 0x4008,
    ACCMD_SIGNAL_ID_START_OPERATORS_RESP = 0x4009,
    ACCMD_SIGNAL_ID_STOP_OPERATORS_RESP = 0x400A,
    ACCMD_SIGNAL_ID_RESET_OPERATORS_RESP = 0x400B,
    ACCMD_SIGNAL_ID_GET_FIRMWARE_ID_STRING_RESP = 0x400C,
    ACCMD_SIGNAL_ID_PRODTEST_RESP = 0x400D,
    ACCMD_SIGNAL_ID_STREAM_GET_INFO_RESP = 0x400E,
    ACCMD_SIGNAL_ID_GET_CAPABILITIES_RESP = 0x400F,
    ACCMD_SIGNAL_ID_CREATE_OPERATOR_EX_RESP = 0x4010,
    ACCMD_SIGNAL_ID_GET_MEM_USAGE_RESP = 0x4011,
    ACCMD_SIGNAL_ID_CLEAR_MEM_WATERMARKS_RESP = 0x4012,
    ACCMD_SIGNAL_ID_GET_MIPS_USAGE_RESP = 0x4013,
    ACCMD_SIGNAL_ID_SET_SYSTEM_STREAM_RATE_RESP = 0x4014,
    ACCMD_SIGNAL_ID_GET_SYSTEM_STREAM_RATE_RESP = 0x4015,
    ACCMD_SIGNAL_ID_STREAM_DISCONNECT_RESP = 0x4016,
    ACCMD_SIGNAL_ID_STREAM_GET_SINK_FROM_SOURCE_RESP = 0x4017,
    ACCMD_SIGNAL_ID_STREAM_GET_SOURCE_FROM_SINK_RESP = 0x4018,
    ACCMD_SIGNAL_ID_SET_SYSTEM_FRAMEWORK_RESP = 0x4019,
    ACCMD_SIGNAL_ID_GET_SYSTEM_FRAMEWORK_RESP = 0x401A,
    ACCMD_SIGNAL_ID_GET_IPC_INTERFACE_TEST_RESP = 0x401B,
    ACCMD_SIGNAL_ID_GET_OPERATORS_RESP = 0x401C,
    ACCMD_SIGNAL_ID_PS_SHUTDOWN_RESP = 0x401D,
    ACCMD_SIGNAL_ID_PS_READ_RESP = 0x401E,
    ACCMD_SIGNAL_ID_PS_WRITE_RESP = 0x401F,
    ACCMD_SIGNAL_ID_PS_DELETE_RESP = 0x4020,
    ACCMD_SIGNAL_ID_PS_SHUTDOWN_COMPLETE_RESP = 0x4021,
    ACCMD_SIGNAL_ID_TRANSFORM_FROM_STREAM_RESP = 0x4022,
    ACCMD_SIGNAL_ID_SET_PROFILER_RESP = 0x4034,
    ACCMD_SIGNAL_ID_CLOCK_CONFIGURE_RESP = 0x4035,
    ACCMD_SIGNAL_ID_DATA_FILE_ALLOCATE_RESP = 0x403A,
    ACCMD_SIGNAL_ID_CLOCK_POWER_SAVE_MODE_RESP = 0x403C,
    ACCMD_SIGNAL_ID_CALL_FUNCTION_RESP = 0x6001
} ACCMD_SIGNAL_ID;
/*******************************************************************************

  NAME
    Accmd_Status

  DESCRIPTION
    Permissable Status responses from Accmd commands

 VALUES
    OK              -
    NO_SUCH_CMDID   -
    BAD_REQ         -
    QUEUE_FULL      -
    CMD_PENDING     -
    CMD_FAILED      -
    NO_SUCH_SUB_CMD -

*******************************************************************************/
typedef enum
{
    ACCMD_STATUS_OK = 0x0000,
    ACCMD_STATUS_NO_SUCH_CMDID = 0x0001,
    ACCMD_STATUS_BAD_REQ = 0x0002,
    ACCMD_STATUS_QUEUE_FULL = 0x0003,
    ACCMD_STATUS_CMD_PENDING = 0x0004,
    ACCMD_STATUS_CMD_FAILED = 0x0005,
    ACCMD_STATUS_NO_SUCH_SUB_CMD = 0x0006
} ACCMD_STATUS;
/*******************************************************************************

  NAME
    Accmd_Stream_Device

  DESCRIPTION
    Stream type (was once "Audio Hardware") This is a subset of the values
    defined in Kymera stream.xml's STREAM_DEVICE type. It's only duplicated
    here because we can't rely on every ACCMD user having a copy of
    stream.xml to hand yet. If you need to allocate a new value, don't rely
    on what's here to avoid clashes, check stream.xml and allocate it there,
    following the rules there to avoid clashes with usage outside Kymera
    (such as BCCMD); then come back here and put it here too. FASTPIPE =
    0x0008 FILE = 0x000A APPDATA = 0x000B L2CAP = 0x000C I2C = 0x000D
    USB_ENDPOINT = 0x000E TIMESTAMPED = 0x0011

 VALUES
    PCM          -
    I2S          -
    CODEC        -
    FM           -
    SPDIF        -
    DIGITAL_MIC  -
    SHUNT        -
    SCO          -
    CAP_DOWNLOAD -
    PWM          -
    FILE_MANAGER - Provides mechanism to download files from apps
    OPERATOR     -
    TESTER       - Endpoint that provides mechanisms to test the audio data
                   service.
    RAW_BUFFER   - Endpoint that provides a raw buffer without flow
                   control/kicks. If this endpoint is connected to a PCM
                   interface, the data in in the buffer will be looped around
                   and played out.

*******************************************************************************/
typedef enum
{
    ACCMD_STREAM_DEVICE_PCM = 0x0001,
    ACCMD_STREAM_DEVICE_I2S = 0x0002,
    ACCMD_STREAM_DEVICE_CODEC = 0x0003,
    ACCMD_STREAM_DEVICE_FM = 0x0004,
    ACCMD_STREAM_DEVICE_SPDIF = 0x0005,
    ACCMD_STREAM_DEVICE_DIGITAL_MIC = 0x0006,
    ACCMD_STREAM_DEVICE_SHUNT = 0x0007,
    ACCMD_STREAM_DEVICE_SCO = 0x0009,
    ACCMD_STREAM_DEVICE_CAP_DOWNLOAD = 0x000F,
    ACCMD_STREAM_DEVICE_PWM = 0x0010,
    ACCMD_STREAM_DEVICE_FILE_MANAGER = 0x0012,
    ACCMD_STREAM_DEVICE_OPERATOR = 0x001E,
    ACCMD_STREAM_DEVICE_TESTER = 0x007E,
    ACCMD_STREAM_DEVICE_RAW_BUFFER = 0x007F
} ACCMD_STREAM_DEVICE;
/*******************************************************************************

  NAME
    Accmd_Type_Of_File

  DESCRIPTION
    reserved for future use

 VALUES
    UNUSED -
    BIN    -
    PUK    -

*******************************************************************************/
typedef enum
{
    ACCMD_TYPE_OF_FILE_UNUSED = 0,
    ACCMD_TYPE_OF_FILE_BIN = 1,
    ACCMD_TYPE_OF_FILE_PUK = 2
} ACCMD_TYPE_OF_FILE;
/*******************************************************************************

  NAME
    Accmd_anc_control

  DESCRIPTION
    ANC control bitfield

 VALUES
    DMIC_X0P5_A_SEL_MASK     -
    DMIC_X0P5_B_SEL_MASK     -
    DMIC_X2_A_SEL_MASK       -
    DMIC_X2_B_SEL_MASK       -
    FB_ON_FBMON_IS_TRUE_MASK -
    FFLE_EN_MASK             -
    OUTMIX_EN_MASK           -
    FB_TUNE_DSM_EN_MASK      -
    FB_TUNE_DSM_CLR_MASK     -
    FFGAIN_ZCD_EN_MASK       -
    ZCD_SHIFT_MASK           -
    COEF_SMP_EN_MASK         -
    FFGAIN_SMP_EN_MASK       -
    VALID_MASK               -

*******************************************************************************/
typedef enum
{
    ACCMD_ANC_CONTROL_DMIC_X0P5_A_SEL_MASK = 0x0001,
    ACCMD_ANC_CONTROL_DMIC_X0P5_B_SEL_MASK = 0x0002,
    ACCMD_ANC_CONTROL_DMIC_X2_A_SEL_MASK = 0x0004,
    ACCMD_ANC_CONTROL_DMIC_X2_B_SEL_MASK = 0x0008,
    ACCMD_ANC_CONTROL_FB_ON_FBMON_IS_TRUE_MASK = 0x0010,
    ACCMD_ANC_CONTROL_FFLE_EN_MASK = 0x0020,
    ACCMD_ANC_CONTROL_OUTMIX_EN_MASK = 0x0040,
    ACCMD_ANC_CONTROL_FB_TUNE_DSM_EN_MASK = 0x0080,
    ACCMD_ANC_CONTROL_FB_TUNE_DSM_CLR_MASK = 0x0100,
    ACCMD_ANC_CONTROL_FFGAIN_ZCD_EN_MASK = 0x0200,
    ACCMD_ANC_CONTROL_ZCD_SHIFT_MASK = 0x0400,
    ACCMD_ANC_CONTROL_COEF_SMP_EN_MASK = 0x0800,
    ACCMD_ANC_CONTROL_FFGAIN_SMP_EN_MASK = 0x1000,
    ACCMD_ANC_CONTROL_VALID_MASK = 0x1fff
} ACCMD_ANC_CONTROL;
/*******************************************************************************

  NAME
    Accmd_anc_control_access

  DESCRIPTION
    ANC control access

 VALUES
    SELECT_ENABLES_SHIFT -

*******************************************************************************/
typedef enum
{
    ACCMD_ANC_CONTROL_ACCESS_SELECT_ENABLES_SHIFT = 16
} ACCMD_ANC_CONTROL_ACCESS;
/*******************************************************************************

  NAME
    Accmd_anc_dc_filter

  DESCRIPTION
    ANC DC filter details

 VALUES
    MIN_SHIFT -
    MAX_SHIFT -

*******************************************************************************/
typedef enum
{
    ACCMD_ANC_DC_FILTER_MIN_SHIFT = 0,
    ACCMD_ANC_DC_FILTER_MAX_SHIFT = 11
} ACCMD_ANC_DC_FILTER;
/*******************************************************************************

  NAME
    Accmd_anc_enable

  DESCRIPTION
    ANC filter/path bitfield enables

 VALUES
    FFA_MASK -
    FFB_MASK -
    FB_MASK  -
    OUT_MASK -

*******************************************************************************/
typedef enum
{
    ACCMD_ANC_ENABLE_FFA_MASK = 0x0001,
    ACCMD_ANC_ENABLE_FFB_MASK = 0x0002,
    ACCMD_ANC_ENABLE_FB_MASK = 0x0004,
    ACCMD_ANC_ENABLE_OUT_MASK = 0x0008
} ACCMD_ANC_ENABLE;
/*******************************************************************************

  NAME
    Accmd_anc_filter

  DESCRIPTION
    ANC filter details

 VALUES
    MIN_GAIN_SHIFT -
    MIN_GAIN       -
    MAX_GAIN       -
    MAX_GAIN_SHIFT -

*******************************************************************************/
typedef enum
{
    ACCMD_ANC_FILTER_MIN_GAIN_SHIFT = -4,
    ACCMD_ANC_FILTER_MIN_GAIN = 0,
    ACCMD_ANC_FILTER_MAX_GAIN_SHIFT = 6,
    ACCMD_ANC_FILTER_MAX_GAIN = 255
} ACCMD_ANC_FILTER;
/*******************************************************************************

  NAME
    Accmd_anc_iir_filter

  DESCRIPTION
    ANC IIR filter details

 VALUES
    FFB_NUM_COEFFS   -
    FB_NUM_COEFFS    -
    FFA_NUM_COEFFS   -
    OCTETS_PER_COEFF -

*******************************************************************************/
typedef enum
{
    ACCMD_ANC_IIR_FILTER_OCTETS_PER_COEFF = 2,
    ACCMD_ANC_IIR_FILTER_FFB_NUM_COEFFS = 15,
    ACCMD_ANC_IIR_FILTER_FB_NUM_COEFFS = 15,
    ACCMD_ANC_IIR_FILTER_FFA_NUM_COEFFS = 15
} ACCMD_ANC_IIR_FILTER;
/*******************************************************************************

  NAME
    Accmd_anc_instance

  DESCRIPTION
    ANC instance id

 VALUES
    NONE_ID -
    ANC0_ID -
    ANC1_ID -

*******************************************************************************/
typedef enum
{
    ACCMD_ANC_INSTANCE_NONE_ID = 0x0000,
    ACCMD_ANC_INSTANCE_ANC0_ID = 0x0001,
    ACCMD_ANC_INSTANCE_ANC1_ID = 0x0002
} ACCMD_ANC_INSTANCE;
/*******************************************************************************

  NAME
    Accmd_anc_lpf_filter

  DESCRIPTION
    ANC LPF filter details (coeffs are implemented as shifts)

 VALUES
    COEFF_SHIFT_MIN -
    COEFF_SHIFT_MAX -

*******************************************************************************/
typedef enum
{
    ACCMD_ANC_LPF_FILTER_COEFF_SHIFT_MIN = 0,
    ACCMD_ANC_LPF_FILTER_COEFF_SHIFT_MAX = 15
} ACCMD_ANC_LPF_FILTER;
/*******************************************************************************

  NAME
    Accmd_anc_path

  DESCRIPTION
    ANC filter path id

 VALUES
    NONE_ID   -
    FFA_ID    -
    FFB_ID    -
    FB_ID     -
    SM_LPF_ID -

*******************************************************************************/
typedef enum
{
    ACCMD_ANC_PATH_NONE_ID = 0x0000,
    ACCMD_ANC_PATH_FFA_ID = 0x0001,
    ACCMD_ANC_PATH_FFB_ID = 0x0002,
    ACCMD_ANC_PATH_FB_ID = 0x0003,
    ACCMD_ANC_PATH_SM_LPF_ID = 0x0004
} ACCMD_ANC_PATH;
/*******************************************************************************

  NAME
    Accmd_clk_type

  DESCRIPTION
    type of clock used to clock the internal digits of an audio interface

 VALUES
    SYSTEM_ROOT_CLK - System root clock
    MCLK            - MCLK, can be either from internal MPLL or externally
                      provided
    BORROW_PCM0     - Borrow for PCM0
    MAX             - Maximum value for clock source type
    BORROW_EPCM     - Borrow for EPCM

*******************************************************************************/
typedef enum
{
    ACCMD_CLK_TYPE_SYSTEM_ROOT_CLK = 0x0000,
    ACCMD_CLK_TYPE_MCLK = 0x0001,
    ACCMD_CLK_TYPE_BORROW_PCM0 = 0x0002,
    ACCMD_CLK_TYPE_MAX = 0x0003,
    ACCMD_CLK_TYPE_BORROW_EPCM = 0x0003
} ACCMD_CLK_TYPE;


/*******************************************************************************

  NAME
    ACCMD_PERSIST_ID_TYPE

  DESCRIPTION
    Persistent key id. Most significant octet must be 0.

*******************************************************************************/
typedef uint32 ACCMD_PERSIST_ID_TYPE;


#define ACCMD_PRIM_ANY_SIZE 1

/*******************************************************************************

  NAME
    ACCMD_CODEC_IIR_FILTER_COEFF

  DESCRIPTION
    Coefficient of IIR filter.

  MEMBERS
    coeff  - Coefficient of IIR filter

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} ACCMD_CODEC_IIR_FILTER_COEFF;

/* The following macros take ACCMD_CODEC_IIR_FILTER_COEFF *accmd_codec_iir_filter_coeff_ptr */
#define ACCMD_CODEC_IIR_FILTER_COEFF_COEFF_WORD_OFFSET (0)
#define ACCMD_CODEC_IIR_FILTER_COEFF_COEFF_GET(accmd_codec_iir_filter_coeff_ptr) ((accmd_codec_iir_filter_coeff_ptr)->_data[0])
#define ACCMD_CODEC_IIR_FILTER_COEFF_COEFF_SET(accmd_codec_iir_filter_coeff_ptr, coeff) ((accmd_codec_iir_filter_coeff_ptr)->_data[0] = (uint16)(coeff))
#define ACCMD_CODEC_IIR_FILTER_COEFF_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define ACCMD_CODEC_IIR_FILTER_COEFF_CREATE(coeff) \
    (uint16)(coeff)
#define ACCMD_CODEC_IIR_FILTER_COEFF_PACK(accmd_codec_iir_filter_coeff_ptr, coeff) \
    do { \
        (accmd_codec_iir_filter_coeff_ptr)->_data[0] = (uint16)((uint16)(coeff)); \
    } while (0)


/*******************************************************************************

  NAME
    ACCMD_OPERATOR_CREATE_EX_INFO

  DESCRIPTION
    Opmgr struct to define key/value style configuration for create operator
    (extended) command.

  MEMBERS
    key    - Key to specify the type of the configure
    value  - The configure value for the key

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} ACCMD_OPERATOR_CREATE_EX_INFO;

/* The following macros take ACCMD_OPERATOR_CREATE_EX_INFO *accmd_operator_create_ex_info_ptr */
#define ACCMD_OPERATOR_CREATE_EX_INFO_KEY_WORD_OFFSET (0)
#define ACCMD_OPERATOR_CREATE_EX_INFO_KEY_GET(accmd_operator_create_ex_info_ptr) ((accmd_operator_create_ex_info_ptr)->_data[0])
#define ACCMD_OPERATOR_CREATE_EX_INFO_KEY_SET(accmd_operator_create_ex_info_ptr, key) ((accmd_operator_create_ex_info_ptr)->_data[0] = (uint16)(key))
#define ACCMD_OPERATOR_CREATE_EX_INFO_VALUE_WORD_OFFSET (1)
#define ACCMD_OPERATOR_CREATE_EX_INFO_VALUE_GET(accmd_operator_create_ex_info_ptr)  \
    (((uint32)((accmd_operator_create_ex_info_ptr)->_data[1]) | \
      ((uint32)((accmd_operator_create_ex_info_ptr)->_data[2]) << 16)))
#define ACCMD_OPERATOR_CREATE_EX_INFO_VALUE_SET(accmd_operator_create_ex_info_ptr, value) do { \
        (accmd_operator_create_ex_info_ptr)->_data[1] = (uint16)((value) & 0xffff); \
        (accmd_operator_create_ex_info_ptr)->_data[2] = (uint16)((value) >> 16); } while (0)
#define ACCMD_OPERATOR_CREATE_EX_INFO_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define ACCMD_OPERATOR_CREATE_EX_INFO_CREATE(key, value) \
    (uint16)(key), \
    (uint16)((value) & 0xffff), \
    (uint16)((value) >> 16)
#define ACCMD_OPERATOR_CREATE_EX_INFO_PACK(accmd_operator_create_ex_info_ptr, key, value) \
    do { \
        (accmd_operator_create_ex_info_ptr)->_data[0] = (uint16)((uint16)(key)); \
        (accmd_operator_create_ex_info_ptr)->_data[1] = (uint16)((uint16)((value) & 0xffff)); \
        (accmd_operator_create_ex_info_ptr)->_data[2] = (uint16)(((value) >> 16)); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_Operators_Entry

  DESCRIPTION

  MEMBERS
    operator_id   - ID of operator matching the search criteria, in uint16s
    capability_id - Capability ID of the operator in the entry, in uint16s

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} ACCMD_GET_OPERATORS_ENTRY;

/* The following macros take ACCMD_GET_OPERATORS_ENTRY *accmd_get_operators_entry_ptr */
#define ACCMD_GET_OPERATORS_ENTRY_OPERATOR_ID_WORD_OFFSET (0)
#define ACCMD_GET_OPERATORS_ENTRY_OPERATOR_ID_GET(accmd_get_operators_entry_ptr) ((accmd_get_operators_entry_ptr)->_data[0])
#define ACCMD_GET_OPERATORS_ENTRY_OPERATOR_ID_SET(accmd_get_operators_entry_ptr, operator_id) ((accmd_get_operators_entry_ptr)->_data[0] = (uint16)(operator_id))
#define ACCMD_GET_OPERATORS_ENTRY_CAPABILITY_ID_WORD_OFFSET (1)
#define ACCMD_GET_OPERATORS_ENTRY_CAPABILITY_ID_GET(accmd_get_operators_entry_ptr) ((accmd_get_operators_entry_ptr)->_data[1])
#define ACCMD_GET_OPERATORS_ENTRY_CAPABILITY_ID_SET(accmd_get_operators_entry_ptr, capability_id) ((accmd_get_operators_entry_ptr)->_data[1] = (uint16)(capability_id))
#define ACCMD_GET_OPERATORS_ENTRY_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define ACCMD_GET_OPERATORS_ENTRY_CREATE(operator_id, capability_id) \
    (uint16)(operator_id), \
    (uint16)(capability_id)
#define ACCMD_GET_OPERATORS_ENTRY_PACK(accmd_get_operators_entry_ptr, operator_id, capability_id) \
    do { \
        (accmd_get_operators_entry_ptr)->_data[0] = (uint16)((uint16)(operator_id)); \
        (accmd_get_operators_entry_ptr)->_data[1] = (uint16)((uint16)(capability_id)); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Header

  DESCRIPTION

  MEMBERS
    Signal_Id - CmdID in protocol documentation, naming convention required for
                autogen
    Length    -
    Seq_No    - The upper 8 bits of Seq_No are reserved and must be set to zero
                for now. (They were reserved for a possible future credit-based
                flow control scheme.) FIXME: actually add a hidden field and
                reduce Seq_No to uint8

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} ACCMD_HEADER;

/* The following macros take ACCMD_HEADER *accmd_header_ptr */
#define ACCMD_HEADER_SIGNAL_ID_WORD_OFFSET (0)
#define ACCMD_HEADER_SIGNAL_ID_GET(accmd_header_ptr) ((ACCMD_SIGNAL_ID)(accmd_header_ptr)->_data[0])
#define ACCMD_HEADER_SIGNAL_ID_SET(accmd_header_ptr, signal_id) ((accmd_header_ptr)->_data[0] = (uint16)(signal_id))
#define ACCMD_HEADER_LENGTH_WORD_OFFSET (1)
#define ACCMD_HEADER_LENGTH_GET(accmd_header_ptr) ((accmd_header_ptr)->_data[1])
#define ACCMD_HEADER_LENGTH_SET(accmd_header_ptr, length) ((accmd_header_ptr)->_data[1] = (uint16)(length))
#define ACCMD_HEADER_SEQ_NO_WORD_OFFSET (2)
#define ACCMD_HEADER_SEQ_NO_GET(accmd_header_ptr) ((accmd_header_ptr)->_data[2])
#define ACCMD_HEADER_SEQ_NO_SET(accmd_header_ptr, seq_no) ((accmd_header_ptr)->_data[2] = (uint16)(seq_no))
#define ACCMD_HEADER_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define ACCMD_HEADER_CREATE(Signal_Id, Length, Seq_No) \
    (uint16)(Signal_Id), \
    (uint16)(Length), \
    (uint16)(Seq_No)
#define ACCMD_HEADER_PACK(accmd_header_ptr, Signal_Id, Length, Seq_No) \
    do { \
        (accmd_header_ptr)->_data[0] = (uint16)((uint16)(Signal_Id)); \
        (accmd_header_ptr)->_data[1] = (uint16)((uint16)(Length)); \
        (accmd_header_ptr)->_data[2] = (uint16)((uint16)(Seq_No)); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_OP_Mips_Usage

  DESCRIPTION
    Type for holding the mips usage of an operator. An operator mips type
    contains the operator id echoed followed by the operator mips usage in
    thousandths.

  MEMBERS
    OP_ID   - Operator id echoed.
    OP_MIPS - Operator mips usage in thousandths.

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} ACCMD_OP_MIPS_USAGE;

/* The following macros take ACCMD_OP_MIPS_USAGE *accmd_op_mips_usage_ptr */
#define ACCMD_OP_MIPS_USAGE_OP_ID_WORD_OFFSET (0)
#define ACCMD_OP_MIPS_USAGE_OP_ID_GET(accmd_op_mips_usage_ptr) ((accmd_op_mips_usage_ptr)->_data[0])
#define ACCMD_OP_MIPS_USAGE_OP_ID_SET(accmd_op_mips_usage_ptr, op_id) ((accmd_op_mips_usage_ptr)->_data[0] = (uint16)(op_id))
#define ACCMD_OP_MIPS_USAGE_OP_MIPS_WORD_OFFSET (1)
#define ACCMD_OP_MIPS_USAGE_OP_MIPS_GET(accmd_op_mips_usage_ptr) ((accmd_op_mips_usage_ptr)->_data[1])
#define ACCMD_OP_MIPS_USAGE_OP_MIPS_SET(accmd_op_mips_usage_ptr, op_mips) ((accmd_op_mips_usage_ptr)->_data[1] = (uint16)(op_mips))
#define ACCMD_OP_MIPS_USAGE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define ACCMD_OP_MIPS_USAGE_CREATE(OP_ID, OP_MIPS) \
    (uint16)(OP_ID), \
    (uint16)(OP_MIPS)
#define ACCMD_OP_MIPS_USAGE_PACK(accmd_op_mips_usage_ptr, OP_ID, OP_MIPS) \
    do { \
        (accmd_op_mips_usage_ptr)->_data[0] = (uint16)((uint16)(OP_ID)); \
        (accmd_op_mips_usage_ptr)->_data[1] = (uint16)((uint16)(OP_MIPS)); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_No_Cmd_Req

  DESCRIPTION
    From Client an empty req (header only)

  MEMBERS

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
} ACCMD_NO_CMD_REQ;

/* The following macros take ACCMD_NO_CMD_REQ *accmd_no_cmd_req_ptr or uint16 *addr */
#define ACCMD_NO_CMD_REQ_WORD_SIZE (3)


/*******************************************************************************

  NAME
    Accmd_Stream_Get_Source_Req

  DESCRIPTION
    From Client This command requests that the Audio subsystem reserves the
    specified source resource

  MEMBERS
    Device         - The type of hardware being requested
    Audio_Instance - The instance of the hardware being requested
    Channel        - The channel or slot being requested

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER         header;
    ACCMD_STREAM_DEVICE  Device;
    ACCMD_AUDIO_INSTANCE Audio_Instance;
    ACCMD_AUDIO_CHANNEL  Channel;
} ACCMD_STREAM_GET_SOURCE_REQ;

/* The following macros take ACCMD_STREAM_GET_SOURCE_REQ *accmd_stream_get_source_req_ptr or uint16 *addr */
#define ACCMD_STREAM_GET_SOURCE_REQ_DEVICE_WORD_OFFSET (3)
#define ACCMD_STREAM_GET_SOURCE_REQ_DEVICE_GET(addr) ((ACCMD_STREAM_DEVICE)*((addr) + 3))
#define ACCMD_STREAM_GET_SOURCE_REQ_DEVICE_SET(addr, device) (*((addr) + 3) = (uint16)(device))
#define ACCMD_STREAM_GET_SOURCE_REQ_AUDIO_INSTANCE_WORD_OFFSET (4)
#define ACCMD_STREAM_GET_SOURCE_REQ_AUDIO_INSTANCE_GET(addr) ((ACCMD_AUDIO_INSTANCE)*((addr) + 4))
#define ACCMD_STREAM_GET_SOURCE_REQ_AUDIO_INSTANCE_SET(addr, audio_instance) (*((addr) + 4) = (uint16)(audio_instance))
#define ACCMD_STREAM_GET_SOURCE_REQ_CHANNEL_WORD_OFFSET (5)
#define ACCMD_STREAM_GET_SOURCE_REQ_CHANNEL_GET(addr) ((ACCMD_AUDIO_CHANNEL)*((addr) + 5))
#define ACCMD_STREAM_GET_SOURCE_REQ_CHANNEL_SET(addr, channel) (*((addr) + 5) = (uint16)(channel))
#define ACCMD_STREAM_GET_SOURCE_REQ_WORD_SIZE (6)
#define ACCMD_STREAM_GET_SOURCE_REQ_PACK(addr, Device, Audio_Instance, Channel) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Device)); \
        *((addr) + 4) = (uint16)((uint16)(Audio_Instance)); \
        *((addr) + 5) = (uint16)((uint16)(Channel)); \
    } while (0)

#define ACCMD_STREAM_GET_SOURCE_REQ_MARSHALL(addr, accmd_stream_get_source_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_get_source_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_get_source_req_ptr)->Device); \
        *((addr) + 4) = (uint16)((accmd_stream_get_source_req_ptr)->Audio_Instance); \
        *((addr) + 5) = (uint16)((accmd_stream_get_source_req_ptr)->Channel); \
    } while (0)

#define ACCMD_STREAM_GET_SOURCE_REQ_UNMARSHALL(addr, accmd_stream_get_source_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_get_source_req_ptr)->header)); \
        (accmd_stream_get_source_req_ptr)->Device = ACCMD_STREAM_GET_SOURCE_REQ_DEVICE_GET(addr); \
        (accmd_stream_get_source_req_ptr)->Audio_Instance = ACCMD_STREAM_GET_SOURCE_REQ_AUDIO_INSTANCE_GET(addr); \
        (accmd_stream_get_source_req_ptr)->Channel = ACCMD_STREAM_GET_SOURCE_REQ_CHANNEL_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Get_Sink_Req

  DESCRIPTION
    From Client This command requests that the Audio subsystem reserves the
    specified sink resource

  MEMBERS
    Device         - The type of hardware being requested
    Audio_Instance - The instance of hardware being requested
    Channel        - The channel/slot being requested

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER         header;
    ACCMD_STREAM_DEVICE  Device;
    ACCMD_AUDIO_INSTANCE Audio_Instance;
    ACCMD_AUDIO_CHANNEL  Channel;
} ACCMD_STREAM_GET_SINK_REQ;

/* The following macros take ACCMD_STREAM_GET_SINK_REQ *accmd_stream_get_sink_req_ptr or uint16 *addr */
#define ACCMD_STREAM_GET_SINK_REQ_DEVICE_WORD_OFFSET (3)
#define ACCMD_STREAM_GET_SINK_REQ_DEVICE_GET(addr) ((ACCMD_STREAM_DEVICE)*((addr) + 3))
#define ACCMD_STREAM_GET_SINK_REQ_DEVICE_SET(addr, device) (*((addr) + 3) = (uint16)(device))
#define ACCMD_STREAM_GET_SINK_REQ_AUDIO_INSTANCE_WORD_OFFSET (4)
#define ACCMD_STREAM_GET_SINK_REQ_AUDIO_INSTANCE_GET(addr) ((ACCMD_AUDIO_INSTANCE)*((addr) + 4))
#define ACCMD_STREAM_GET_SINK_REQ_AUDIO_INSTANCE_SET(addr, audio_instance) (*((addr) + 4) = (uint16)(audio_instance))
#define ACCMD_STREAM_GET_SINK_REQ_CHANNEL_WORD_OFFSET (5)
#define ACCMD_STREAM_GET_SINK_REQ_CHANNEL_GET(addr) ((ACCMD_AUDIO_CHANNEL)*((addr) + 5))
#define ACCMD_STREAM_GET_SINK_REQ_CHANNEL_SET(addr, channel) (*((addr) + 5) = (uint16)(channel))
#define ACCMD_STREAM_GET_SINK_REQ_WORD_SIZE (6)
#define ACCMD_STREAM_GET_SINK_REQ_PACK(addr, Device, Audio_Instance, Channel) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Device)); \
        *((addr) + 4) = (uint16)((uint16)(Audio_Instance)); \
        *((addr) + 5) = (uint16)((uint16)(Channel)); \
    } while (0)

#define ACCMD_STREAM_GET_SINK_REQ_MARSHALL(addr, accmd_stream_get_sink_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_get_sink_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_get_sink_req_ptr)->Device); \
        *((addr) + 4) = (uint16)((accmd_stream_get_sink_req_ptr)->Audio_Instance); \
        *((addr) + 5) = (uint16)((accmd_stream_get_sink_req_ptr)->Channel); \
    } while (0)

#define ACCMD_STREAM_GET_SINK_REQ_UNMARSHALL(addr, accmd_stream_get_sink_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_get_sink_req_ptr)->header)); \
        (accmd_stream_get_sink_req_ptr)->Device = ACCMD_STREAM_GET_SINK_REQ_DEVICE_GET(addr); \
        (accmd_stream_get_sink_req_ptr)->Audio_Instance = ACCMD_STREAM_GET_SINK_REQ_AUDIO_INSTANCE_GET(addr); \
        (accmd_stream_get_sink_req_ptr)->Channel = ACCMD_STREAM_GET_SINK_REQ_CHANNEL_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Close_Source_Req

  DESCRIPTION
    From Client This command releases the resource currently held by the
    specified Source ID. If the command is issued for a Source ID that is
    currently active in a transform, the transform will automatically be
    disconnected as part of the command. Once released, the Source ID
    associated with the resource will be invalid and should be discarded.

  MEMBERS
    Source_ID - A sid representing the source to close

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Source_ID;
} ACCMD_STREAM_CLOSE_SOURCE_REQ;

/* The following macros take ACCMD_STREAM_CLOSE_SOURCE_REQ *accmd_stream_close_source_req_ptr or uint16 *addr */
#define ACCMD_STREAM_CLOSE_SOURCE_REQ_SOURCE_ID_WORD_OFFSET (3)
#define ACCMD_STREAM_CLOSE_SOURCE_REQ_SOURCE_ID_GET(addr) (*((addr) + 3))
#define ACCMD_STREAM_CLOSE_SOURCE_REQ_SOURCE_ID_SET(addr, source_id) (*((addr) + 3) = (uint16)(source_id))
#define ACCMD_STREAM_CLOSE_SOURCE_REQ_WORD_SIZE (4)
#define ACCMD_STREAM_CLOSE_SOURCE_REQ_PACK(addr, Source_ID) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Source_ID)); \
    } while (0)

#define ACCMD_STREAM_CLOSE_SOURCE_REQ_MARSHALL(addr, accmd_stream_close_source_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_close_source_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_close_source_req_ptr)->Source_ID); \
    } while (0)

#define ACCMD_STREAM_CLOSE_SOURCE_REQ_UNMARSHALL(addr, accmd_stream_close_source_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_close_source_req_ptr)->header)); \
        (accmd_stream_close_source_req_ptr)->Source_ID = ACCMD_STREAM_CLOSE_SOURCE_REQ_SOURCE_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Close_Sink_Req

  DESCRIPTION
    From Client As Stream_Close_Source_Req but closes a sink

  MEMBERS
    Sink_ID - A sid representing the sink to close

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Sink_ID;
} ACCMD_STREAM_CLOSE_SINK_REQ;

/* The following macros take ACCMD_STREAM_CLOSE_SINK_REQ *accmd_stream_close_sink_req_ptr or uint16 *addr */
#define ACCMD_STREAM_CLOSE_SINK_REQ_SINK_ID_WORD_OFFSET (3)
#define ACCMD_STREAM_CLOSE_SINK_REQ_SINK_ID_GET(addr) (*((addr) + 3))
#define ACCMD_STREAM_CLOSE_SINK_REQ_SINK_ID_SET(addr, sink_id) (*((addr) + 3) = (uint16)(sink_id))
#define ACCMD_STREAM_CLOSE_SINK_REQ_WORD_SIZE (4)
#define ACCMD_STREAM_CLOSE_SINK_REQ_PACK(addr, Sink_ID) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Sink_ID)); \
    } while (0)

#define ACCMD_STREAM_CLOSE_SINK_REQ_MARSHALL(addr, accmd_stream_close_sink_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_close_sink_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_close_sink_req_ptr)->Sink_ID); \
    } while (0)

#define ACCMD_STREAM_CLOSE_SINK_REQ_UNMARSHALL(addr, accmd_stream_close_sink_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_close_sink_req_ptr)->header)); \
        (accmd_stream_close_sink_req_ptr)->Sink_ID = ACCMD_STREAM_CLOSE_SINK_REQ_SINK_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Configure_Req

  DESCRIPTION
    From Client This command configures a single property of the specified
    Source or Sink ID.

  MEMBERS
    Sid    - The Sid parameter specifies the Source or Sink ID to be configured
    Key    - The Key parameter specifies the property of the Source or Sink ID
             to be configured
    Value  - The Value parameter specifies the data value to be assigned to the
             key

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER     header;
    uint16           Sid;
    ACCMD_CONFIG_KEY Key;
    uint32           Value;
} ACCMD_STREAM_CONFIGURE_REQ;

/* The following macros take ACCMD_STREAM_CONFIGURE_REQ *accmd_stream_configure_req_ptr or uint16 *addr */
#define ACCMD_STREAM_CONFIGURE_REQ_SID_WORD_OFFSET (3)
#define ACCMD_STREAM_CONFIGURE_REQ_SID_GET(addr) (*((addr) + 3))
#define ACCMD_STREAM_CONFIGURE_REQ_SID_SET(addr, sid) (*((addr) + 3) = (uint16)(sid))
#define ACCMD_STREAM_CONFIGURE_REQ_KEY_WORD_OFFSET (4)
#define ACCMD_STREAM_CONFIGURE_REQ_KEY_GET(addr) ((ACCMD_CONFIG_KEY)*((addr) + 4))
#define ACCMD_STREAM_CONFIGURE_REQ_KEY_SET(addr, key) (*((addr) + 4) = (uint16)(key))
#define ACCMD_STREAM_CONFIGURE_REQ_VALUE_WORD_OFFSET (5)
#define ACCMD_STREAM_CONFIGURE_REQ_VALUE_GET(addr)  \
    (((uint32)(*((addr) + 5)) | \
      ((uint32)(*((addr) + 6)) << 16)))
#define ACCMD_STREAM_CONFIGURE_REQ_VALUE_SET(addr, value) do { \
        *((addr) + 5) = (uint16)((value) & 0xffff); \
        *((addr) + 6) = (uint16)((value) >> 16); } while (0)
#define ACCMD_STREAM_CONFIGURE_REQ_WORD_SIZE (7)
#define ACCMD_STREAM_CONFIGURE_REQ_PACK(addr, Sid, Key, Value) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Sid)); \
        *((addr) + 4) = (uint16)((uint16)(Key)); \
        *((addr) + 5) = (uint16)((uint16)((Value) & 0xffff)); \
        *((addr) + 6) = (uint16)(((Value) >> 16)); \
    } while (0)

#define ACCMD_STREAM_CONFIGURE_REQ_MARSHALL(addr, accmd_stream_configure_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_configure_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_configure_req_ptr)->Sid); \
        *((addr) + 4) = (uint16)((accmd_stream_configure_req_ptr)->Key); \
        *((addr) + 5) = (uint16)(((accmd_stream_configure_req_ptr)->Value) & 0xffff); \
        *((addr) + 6) = (uint16)(((accmd_stream_configure_req_ptr)->Value) >> 16); \
    } while (0)

#define ACCMD_STREAM_CONFIGURE_REQ_UNMARSHALL(addr, accmd_stream_configure_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_configure_req_ptr)->header)); \
        (accmd_stream_configure_req_ptr)->Sid = ACCMD_STREAM_CONFIGURE_REQ_SID_GET(addr); \
        (accmd_stream_configure_req_ptr)->Key = ACCMD_STREAM_CONFIGURE_REQ_KEY_GET(addr); \
        (accmd_stream_configure_req_ptr)->Value = ACCMD_STREAM_CONFIGURE_REQ_VALUE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Sync_Sid_Req

  DESCRIPTION
    From Client This command marks two specified Source or Sink IDs for
    synchronisation with each other by putting them into the same sync group.
    All Source or Sink IDs within a particular sync group will always be
    enabled simultaneously. This is achieved by automatically deferring
    stream_connect commands involving synchronised Source or Sink IDs until
    all associated Source or Sink IDs have had a corresponding stream_connect
    command issued.

  MEMBERS
    Sid1   - first sid to sync
    Sid2   - second sid to sync

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Sid1;
    uint16       Sid2;
} ACCMD_STREAM_SYNC_SID_REQ;

/* The following macros take ACCMD_STREAM_SYNC_SID_REQ *accmd_stream_sync_sid_req_ptr or uint16 *addr */
#define ACCMD_STREAM_SYNC_SID_REQ_SID1_WORD_OFFSET (3)
#define ACCMD_STREAM_SYNC_SID_REQ_SID1_GET(addr) (*((addr) + 3))
#define ACCMD_STREAM_SYNC_SID_REQ_SID1_SET(addr, sid1) (*((addr) + 3) = (uint16)(sid1))
#define ACCMD_STREAM_SYNC_SID_REQ_SID2_WORD_OFFSET (4)
#define ACCMD_STREAM_SYNC_SID_REQ_SID2_GET(addr) (*((addr) + 4))
#define ACCMD_STREAM_SYNC_SID_REQ_SID2_SET(addr, sid2) (*((addr) + 4) = (uint16)(sid2))
#define ACCMD_STREAM_SYNC_SID_REQ_WORD_SIZE (5)
#define ACCMD_STREAM_SYNC_SID_REQ_PACK(addr, Sid1, Sid2) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Sid1)); \
        *((addr) + 4) = (uint16)((uint16)(Sid2)); \
    } while (0)

#define ACCMD_STREAM_SYNC_SID_REQ_MARSHALL(addr, accmd_stream_sync_sid_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_sync_sid_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_sync_sid_req_ptr)->Sid1); \
        *((addr) + 4) = (uint16)((accmd_stream_sync_sid_req_ptr)->Sid2); \
    } while (0)

#define ACCMD_STREAM_SYNC_SID_REQ_UNMARSHALL(addr, accmd_stream_sync_sid_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_sync_sid_req_ptr)->header)); \
        (accmd_stream_sync_sid_req_ptr)->Sid1 = ACCMD_STREAM_SYNC_SID_REQ_SID1_GET(addr); \
        (accmd_stream_sync_sid_req_ptr)->Sid2 = ACCMD_STREAM_SYNC_SID_REQ_SID2_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Connect_Req

  DESCRIPTION
    From Client Creates and starts a transform between the specified Source
    ID and Sink ID. A transform is a route along which data flows. Data
    enters the transform through the input, identified by the Source ID and
    leaves through the output, identified by the Sink ID. The format and rate
    of the input and output data are determined by the configuration of the
    Source and Sink IDs respectively. The direction of data flow through the
    transform will always be from source to sink.

  MEMBERS
    Source_ID - Source to connect
    Sink_ID   - Sink to connect

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Source_ID;
    uint16       Sink_ID;
} ACCMD_STREAM_CONNECT_REQ;

/* The following macros take ACCMD_STREAM_CONNECT_REQ *accmd_stream_connect_req_ptr or uint16 *addr */
#define ACCMD_STREAM_CONNECT_REQ_SOURCE_ID_WORD_OFFSET (3)
#define ACCMD_STREAM_CONNECT_REQ_SOURCE_ID_GET(addr) (*((addr) + 3))
#define ACCMD_STREAM_CONNECT_REQ_SOURCE_ID_SET(addr, source_id) (*((addr) + 3) = (uint16)(source_id))
#define ACCMD_STREAM_CONNECT_REQ_SINK_ID_WORD_OFFSET (4)
#define ACCMD_STREAM_CONNECT_REQ_SINK_ID_GET(addr) (*((addr) + 4))
#define ACCMD_STREAM_CONNECT_REQ_SINK_ID_SET(addr, sink_id) (*((addr) + 4) = (uint16)(sink_id))
#define ACCMD_STREAM_CONNECT_REQ_WORD_SIZE (5)
#define ACCMD_STREAM_CONNECT_REQ_PACK(addr, Source_ID, Sink_ID) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Source_ID)); \
        *((addr) + 4) = (uint16)((uint16)(Sink_ID)); \
    } while (0)

#define ACCMD_STREAM_CONNECT_REQ_MARSHALL(addr, accmd_stream_connect_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_connect_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_connect_req_ptr)->Source_ID); \
        *((addr) + 4) = (uint16)((accmd_stream_connect_req_ptr)->Sink_ID); \
    } while (0)

#define ACCMD_STREAM_CONNECT_REQ_UNMARSHALL(addr, accmd_stream_connect_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_connect_req_ptr)->header)); \
        (accmd_stream_connect_req_ptr)->Source_ID = ACCMD_STREAM_CONNECT_REQ_SOURCE_ID_GET(addr); \
        (accmd_stream_connect_req_ptr)->Sink_ID = ACCMD_STREAM_CONNECT_REQ_SINK_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Transform_Disconnect_Req

  DESCRIPTION
    From Client disconnects an existing transform that was formed using the
    stream_connect command

  MEMBERS
    Transform_ID - Streams transform id

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Transform_ID;
} ACCMD_STREAM_TRANSFORM_DISCONNECT_REQ;

/* The following macros take ACCMD_STREAM_TRANSFORM_DISCONNECT_REQ *accmd_stream_transform_disconnect_req_ptr or uint16 *addr */
#define ACCMD_STREAM_TRANSFORM_DISCONNECT_REQ_TRANSFORM_ID_WORD_OFFSET (3)
#define ACCMD_STREAM_TRANSFORM_DISCONNECT_REQ_TRANSFORM_ID_GET(addr) (*((addr) + 3))
#define ACCMD_STREAM_TRANSFORM_DISCONNECT_REQ_TRANSFORM_ID_SET(addr, transform_id) (*((addr) + 3) = (uint16)(transform_id))
#define ACCMD_STREAM_TRANSFORM_DISCONNECT_REQ_WORD_SIZE (4)
#define ACCMD_STREAM_TRANSFORM_DISCONNECT_REQ_PACK(addr, Transform_ID) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Transform_ID)); \
    } while (0)

#define ACCMD_STREAM_TRANSFORM_DISCONNECT_REQ_MARSHALL(addr, accmd_stream_transform_disconnect_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_transform_disconnect_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_transform_disconnect_req_ptr)->Transform_ID); \
    } while (0)

#define ACCMD_STREAM_TRANSFORM_DISCONNECT_REQ_UNMARSHALL(addr, accmd_stream_transform_disconnect_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_transform_disconnect_req_ptr)->header)); \
        (accmd_stream_transform_disconnect_req_ptr)->Transform_ID = ACCMD_STREAM_TRANSFORM_DISCONNECT_REQ_TRANSFORM_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_Firmware_Version_Req

  DESCRIPTION
    From Client asks for audio subsystem version

  MEMBERS

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
} ACCMD_GET_FIRMWARE_VERSION_REQ;

/* The following macros take ACCMD_GET_FIRMWARE_VERSION_REQ *accmd_get_firmware_version_req_ptr or uint16 *addr */
#define ACCMD_GET_FIRMWARE_VERSION_REQ_WORD_SIZE (3)


/*******************************************************************************

  NAME
    Accmd_Create_Operator_Req

  DESCRIPTION
    From Client This command requests that the Audio subsystem instantiates
    the provided capability as an operator.

  MEMBERS
    Capability_ID - The type of capability being requested

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Capability_ID;
} ACCMD_CREATE_OPERATOR_REQ;

/* The following macros take ACCMD_CREATE_OPERATOR_REQ *accmd_create_operator_req_ptr or uint16 *addr */
#define ACCMD_CREATE_OPERATOR_REQ_CAPABILITY_ID_WORD_OFFSET (3)
#define ACCMD_CREATE_OPERATOR_REQ_CAPABILITY_ID_GET(addr) (*((addr) + 3))
#define ACCMD_CREATE_OPERATOR_REQ_CAPABILITY_ID_SET(addr, capability_id) (*((addr) + 3) = (uint16)(capability_id))
#define ACCMD_CREATE_OPERATOR_REQ_WORD_SIZE (4)
#define ACCMD_CREATE_OPERATOR_REQ_PACK(addr, Capability_ID) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Capability_ID)); \
    } while (0)

#define ACCMD_CREATE_OPERATOR_REQ_MARSHALL(addr, accmd_create_operator_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_create_operator_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_create_operator_req_ptr)->Capability_ID); \
    } while (0)

#define ACCMD_CREATE_OPERATOR_REQ_UNMARSHALL(addr, accmd_create_operator_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_create_operator_req_ptr)->header)); \
        (accmd_create_operator_req_ptr)->Capability_ID = ACCMD_CREATE_OPERATOR_REQ_CAPABILITY_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Destroy_Operators_Req

  DESCRIPTION
    From Client This command requests that the Audio subsystem destroys the
    provided operators

  MEMBERS
    Count        - The number of operators in the list to destroy
    Destroy_list - The list of operators to destroy

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Count;
    uint16       Destroy_list[ACCMD_PRIM_ANY_SIZE];
} ACCMD_DESTROY_OPERATORS_REQ;

/* The following macros take ACCMD_DESTROY_OPERATORS_REQ *accmd_destroy_operators_req_ptr or uint16 *addr */
#define ACCMD_DESTROY_OPERATORS_REQ_COUNT_WORD_OFFSET (3)
#define ACCMD_DESTROY_OPERATORS_REQ_COUNT_GET(addr) (*((addr) + 3))
#define ACCMD_DESTROY_OPERATORS_REQ_COUNT_SET(addr, count) (*((addr) + 3) = (uint16)(count))
#define ACCMD_DESTROY_OPERATORS_REQ_DESTROY_LIST_WORD_OFFSET (4)
#define ACCMD_DESTROY_OPERATORS_REQ_DESTROY_LIST_GET(addr) (*((addr) + 4))
#define ACCMD_DESTROY_OPERATORS_REQ_DESTROY_LIST_SET(addr, destroy_list) (*((addr) + 4) = (uint16)(destroy_list))
#define ACCMD_DESTROY_OPERATORS_REQ_WORD_SIZE (5)
#define ACCMD_DESTROY_OPERATORS_REQ_PACK(addr, Count, Destroy_list) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Count)); \
        *((addr) + 4) = (uint16)((uint16)(Destroy_list)); \
    } while (0)

#define ACCMD_DESTROY_OPERATORS_REQ_MARSHALL(addr, accmd_destroy_operators_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_destroy_operators_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_destroy_operators_req_ptr)->Count); \
        *((addr) + 4) = (uint16)((accmd_destroy_operators_req_ptr)->Destroy_list); \
    } while (0)

#define ACCMD_DESTROY_OPERATORS_REQ_UNMARSHALL(addr, accmd_destroy_operators_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_destroy_operators_req_ptr)->header)); \
        (accmd_destroy_operators_req_ptr)->Count = ACCMD_DESTROY_OPERATORS_REQ_COUNT_GET(addr); \
        (accmd_destroy_operators_req_ptr)->Destroy_list = ACCMD_DESTROY_OPERATORS_REQ_DESTROY_LIST_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Operator_Message_Req

  DESCRIPTION
    From Client This command requests that the Audio subsystem sends the
    supplied message to the operator specified.

  MEMBERS
    opid       - The operator to send the message to
    op_message - Message to send to the specified operator

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       opid;
    uint16       op_message[ACCMD_PRIM_ANY_SIZE];
} ACCMD_OPERATOR_MESSAGE_REQ;

/* The following macros take ACCMD_OPERATOR_MESSAGE_REQ *accmd_operator_message_req_ptr or uint16 *addr */
#define ACCMD_OPERATOR_MESSAGE_REQ_OPID_WORD_OFFSET (3)
#define ACCMD_OPERATOR_MESSAGE_REQ_OPID_GET(addr) (*((addr) + 3))
#define ACCMD_OPERATOR_MESSAGE_REQ_OPID_SET(addr, opid) (*((addr) + 3) = (uint16)(opid))
#define ACCMD_OPERATOR_MESSAGE_REQ_OP_MESSAGE_WORD_OFFSET (4)
#define ACCMD_OPERATOR_MESSAGE_REQ_OP_MESSAGE_GET(addr) (*((addr) + 4))
#define ACCMD_OPERATOR_MESSAGE_REQ_OP_MESSAGE_SET(addr, op_message) (*((addr) + 4) = (uint16)(op_message))
#define ACCMD_OPERATOR_MESSAGE_REQ_WORD_SIZE (5)
#define ACCMD_OPERATOR_MESSAGE_REQ_PACK(addr, opid, op_message) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(opid)); \
        *((addr) + 4) = (uint16)((uint16)(op_message)); \
    } while (0)

#define ACCMD_OPERATOR_MESSAGE_REQ_MARSHALL(addr, accmd_operator_message_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_operator_message_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_operator_message_req_ptr)->opid); \
        *((addr) + 4) = (uint16)((accmd_operator_message_req_ptr)->op_message); \
    } while (0)

#define ACCMD_OPERATOR_MESSAGE_REQ_UNMARSHALL(addr, accmd_operator_message_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_operator_message_req_ptr)->header)); \
        (accmd_operator_message_req_ptr)->opid = ACCMD_OPERATOR_MESSAGE_REQ_OPID_GET(addr); \
        (accmd_operator_message_req_ptr)->op_message = ACCMD_OPERATOR_MESSAGE_REQ_OP_MESSAGE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Start_Operators_Req

  DESCRIPTION
    From Client This command requests that the Audio subsystem starts the
    provided operators

  MEMBERS
    Count      - The number of operators in the list to start
    Start_list - The list of operators to start

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Count;
    uint16       Start_list[ACCMD_PRIM_ANY_SIZE];
} ACCMD_START_OPERATORS_REQ;

/* The following macros take ACCMD_START_OPERATORS_REQ *accmd_start_operators_req_ptr or uint16 *addr */
#define ACCMD_START_OPERATORS_REQ_COUNT_WORD_OFFSET (3)
#define ACCMD_START_OPERATORS_REQ_COUNT_GET(addr) (*((addr) + 3))
#define ACCMD_START_OPERATORS_REQ_COUNT_SET(addr, count) (*((addr) + 3) = (uint16)(count))
#define ACCMD_START_OPERATORS_REQ_START_LIST_WORD_OFFSET (4)
#define ACCMD_START_OPERATORS_REQ_START_LIST_GET(addr) (*((addr) + 4))
#define ACCMD_START_OPERATORS_REQ_START_LIST_SET(addr, start_list) (*((addr) + 4) = (uint16)(start_list))
#define ACCMD_START_OPERATORS_REQ_WORD_SIZE (5)
#define ACCMD_START_OPERATORS_REQ_PACK(addr, Count, Start_list) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Count)); \
        *((addr) + 4) = (uint16)((uint16)(Start_list)); \
    } while (0)

#define ACCMD_START_OPERATORS_REQ_MARSHALL(addr, accmd_start_operators_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_start_operators_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_start_operators_req_ptr)->Count); \
        *((addr) + 4) = (uint16)((accmd_start_operators_req_ptr)->Start_list); \
    } while (0)

#define ACCMD_START_OPERATORS_REQ_UNMARSHALL(addr, accmd_start_operators_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_start_operators_req_ptr)->header)); \
        (accmd_start_operators_req_ptr)->Count = ACCMD_START_OPERATORS_REQ_COUNT_GET(addr); \
        (accmd_start_operators_req_ptr)->Start_list = ACCMD_START_OPERATORS_REQ_START_LIST_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stop_Operators_Req

  DESCRIPTION
    From Client This command requests that the Audio subsystem stops the
    provided operators.

  MEMBERS
    Count     - The number of operators in the list to stop
    stop_list - The list of operators to stop

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Count;
    uint16       stop_list[ACCMD_PRIM_ANY_SIZE];
} ACCMD_STOP_OPERATORS_REQ;

/* The following macros take ACCMD_STOP_OPERATORS_REQ *accmd_stop_operators_req_ptr or uint16 *addr */
#define ACCMD_STOP_OPERATORS_REQ_COUNT_WORD_OFFSET (3)
#define ACCMD_STOP_OPERATORS_REQ_COUNT_GET(addr) (*((addr) + 3))
#define ACCMD_STOP_OPERATORS_REQ_COUNT_SET(addr, count) (*((addr) + 3) = (uint16)(count))
#define ACCMD_STOP_OPERATORS_REQ_STOP_LIST_WORD_OFFSET (4)
#define ACCMD_STOP_OPERATORS_REQ_STOP_LIST_GET(addr) (*((addr) + 4))
#define ACCMD_STOP_OPERATORS_REQ_STOP_LIST_SET(addr, stop_list) (*((addr) + 4) = (uint16)(stop_list))
#define ACCMD_STOP_OPERATORS_REQ_WORD_SIZE (5)
#define ACCMD_STOP_OPERATORS_REQ_PACK(addr, Count, stop_list) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Count)); \
        *((addr) + 4) = (uint16)((uint16)(stop_list)); \
    } while (0)

#define ACCMD_STOP_OPERATORS_REQ_MARSHALL(addr, accmd_stop_operators_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stop_operators_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stop_operators_req_ptr)->Count); \
        *((addr) + 4) = (uint16)((accmd_stop_operators_req_ptr)->stop_list); \
    } while (0)

#define ACCMD_STOP_OPERATORS_REQ_UNMARSHALL(addr, accmd_stop_operators_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stop_operators_req_ptr)->header)); \
        (accmd_stop_operators_req_ptr)->Count = ACCMD_STOP_OPERATORS_REQ_COUNT_GET(addr); \
        (accmd_stop_operators_req_ptr)->stop_list = ACCMD_STOP_OPERATORS_REQ_STOP_LIST_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Reset_Operators_Req

  DESCRIPTION
    From Client This command requests that the Audio Subsystem resets the
    provided operators

  MEMBERS
    Count      - The number of operators in the list to reset
    reset_list - The list of operators to reset

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Count;
    uint16       reset_list[ACCMD_PRIM_ANY_SIZE];
} ACCMD_RESET_OPERATORS_REQ;

/* The following macros take ACCMD_RESET_OPERATORS_REQ *accmd_reset_operators_req_ptr or uint16 *addr */
#define ACCMD_RESET_OPERATORS_REQ_COUNT_WORD_OFFSET (3)
#define ACCMD_RESET_OPERATORS_REQ_COUNT_GET(addr) (*((addr) + 3))
#define ACCMD_RESET_OPERATORS_REQ_COUNT_SET(addr, count) (*((addr) + 3) = (uint16)(count))
#define ACCMD_RESET_OPERATORS_REQ_RESET_LIST_WORD_OFFSET (4)
#define ACCMD_RESET_OPERATORS_REQ_RESET_LIST_GET(addr) (*((addr) + 4))
#define ACCMD_RESET_OPERATORS_REQ_RESET_LIST_SET(addr, reset_list) (*((addr) + 4) = (uint16)(reset_list))
#define ACCMD_RESET_OPERATORS_REQ_WORD_SIZE (5)
#define ACCMD_RESET_OPERATORS_REQ_PACK(addr, Count, reset_list) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Count)); \
        *((addr) + 4) = (uint16)((uint16)(reset_list)); \
    } while (0)

#define ACCMD_RESET_OPERATORS_REQ_MARSHALL(addr, accmd_reset_operators_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_reset_operators_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_reset_operators_req_ptr)->Count); \
        *((addr) + 4) = (uint16)((accmd_reset_operators_req_ptr)->reset_list); \
    } while (0)

#define ACCMD_RESET_OPERATORS_REQ_UNMARSHALL(addr, accmd_reset_operators_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_reset_operators_req_ptr)->header)); \
        (accmd_reset_operators_req_ptr)->Count = ACCMD_RESET_OPERATORS_REQ_COUNT_GET(addr); \
        (accmd_reset_operators_req_ptr)->reset_list = ACCMD_RESET_OPERATORS_REQ_RESET_LIST_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Message_From_Operator_Req

  DESCRIPTION
    From AudioSS This command carries an unsolicited message from an operator

  MEMBERS
    opid    - The ID of the operator that sent the message
    message - The message that the operator sent

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       opid;
    uint16       message[ACCMD_PRIM_ANY_SIZE];
} ACCMD_MESSAGE_FROM_OPERATOR_REQ;

/* The following macros take ACCMD_MESSAGE_FROM_OPERATOR_REQ *accmd_message_from_operator_req_ptr or uint16 *addr */
#define ACCMD_MESSAGE_FROM_OPERATOR_REQ_OPID_WORD_OFFSET (3)
#define ACCMD_MESSAGE_FROM_OPERATOR_REQ_OPID_GET(addr) (*((addr) + 3))
#define ACCMD_MESSAGE_FROM_OPERATOR_REQ_OPID_SET(addr, opid) (*((addr) + 3) = (uint16)(opid))
#define ACCMD_MESSAGE_FROM_OPERATOR_REQ_MESSAGE_WORD_OFFSET (4)
#define ACCMD_MESSAGE_FROM_OPERATOR_REQ_MESSAGE_GET(addr) (*((addr) + 4))
#define ACCMD_MESSAGE_FROM_OPERATOR_REQ_MESSAGE_SET(addr, message) (*((addr) + 4) = (uint16)(message))
#define ACCMD_MESSAGE_FROM_OPERATOR_REQ_WORD_SIZE (5)
#define ACCMD_MESSAGE_FROM_OPERATOR_REQ_PACK(addr, opid, message) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(opid)); \
        *((addr) + 4) = (uint16)((uint16)(message)); \
    } while (0)

#define ACCMD_MESSAGE_FROM_OPERATOR_REQ_MARSHALL(addr, accmd_message_from_operator_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_message_from_operator_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_message_from_operator_req_ptr)->opid); \
        *((addr) + 4) = (uint16)((accmd_message_from_operator_req_ptr)->message); \
    } while (0)

#define ACCMD_MESSAGE_FROM_OPERATOR_REQ_UNMARSHALL(addr, accmd_message_from_operator_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_message_from_operator_req_ptr)->header)); \
        (accmd_message_from_operator_req_ptr)->opid = ACCMD_MESSAGE_FROM_OPERATOR_REQ_OPID_GET(addr); \
        (accmd_message_from_operator_req_ptr)->message = ACCMD_MESSAGE_FROM_OPERATOR_REQ_MESSAGE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_Firmware_Id_String_Req

  DESCRIPTION
    From Client Asks for audio subsystem ID string; this conveys more
    information than the firmware ID alone.

  MEMBERS

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
} ACCMD_GET_FIRMWARE_ID_STRING_REQ;

/* The following macros take ACCMD_GET_FIRMWARE_ID_STRING_REQ *accmd_get_firmware_id_string_req_ptr or uint16 *addr */
#define ACCMD_GET_FIRMWARE_ID_STRING_REQ_WORD_SIZE (3)


/*******************************************************************************

  NAME
    Accmd_Stream_Get_Info_Req

  DESCRIPTION
    From Client This command requests the value of a single property of the
    specified Source or Sink ID.

  MEMBERS
    Sid    - The Sid parameter specifies the Source or Sink ID to be queried
    Key    - The Key parameter specifies the property of the Source or Sink ID
             to be queried

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER   header;
    uint16         Sid;
    ACCMD_INFO_KEY Key;
} ACCMD_STREAM_GET_INFO_REQ;

/* The following macros take ACCMD_STREAM_GET_INFO_REQ *accmd_stream_get_info_req_ptr or uint16 *addr */
#define ACCMD_STREAM_GET_INFO_REQ_SID_WORD_OFFSET (3)
#define ACCMD_STREAM_GET_INFO_REQ_SID_GET(addr) (*((addr) + 3))
#define ACCMD_STREAM_GET_INFO_REQ_SID_SET(addr, sid) (*((addr) + 3) = (uint16)(sid))
#define ACCMD_STREAM_GET_INFO_REQ_KEY_WORD_OFFSET (4)
#define ACCMD_STREAM_GET_INFO_REQ_KEY_GET(addr) ((ACCMD_INFO_KEY)*((addr) + 4))
#define ACCMD_STREAM_GET_INFO_REQ_KEY_SET(addr, key) (*((addr) + 4) = (uint16)(key))
#define ACCMD_STREAM_GET_INFO_REQ_WORD_SIZE (5)
#define ACCMD_STREAM_GET_INFO_REQ_PACK(addr, Sid, Key) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Sid)); \
        *((addr) + 4) = (uint16)((uint16)(Key)); \
    } while (0)

#define ACCMD_STREAM_GET_INFO_REQ_MARSHALL(addr, accmd_stream_get_info_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_get_info_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_get_info_req_ptr)->Sid); \
        *((addr) + 4) = (uint16)((accmd_stream_get_info_req_ptr)->Key); \
    } while (0)

#define ACCMD_STREAM_GET_INFO_REQ_UNMARSHALL(addr, accmd_stream_get_info_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_get_info_req_ptr)->header)); \
        (accmd_stream_get_info_req_ptr)->Sid = ACCMD_STREAM_GET_INFO_REQ_SID_GET(addr); \
        (accmd_stream_get_info_req_ptr)->Key = ACCMD_STREAM_GET_INFO_REQ_KEY_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_Capabilities_Req

  DESCRIPTION
    From Client This command requests a list of available capability IDs.
    This includes any built-in capabilities and all those currently available
    for download, whether or not any operator instances currently exist.

  MEMBERS
    start_index - Index of the capability to start reporting from in uint16s.
                  This allows working around the limitations of the Maximum
                  Transfer Unit of the underlying transport protocol.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       start_index;
} ACCMD_GET_CAPABILITIES_REQ;

/* The following macros take ACCMD_GET_CAPABILITIES_REQ *accmd_get_capabilities_req_ptr or uint16 *addr */
#define ACCMD_GET_CAPABILITIES_REQ_START_INDEX_WORD_OFFSET (3)
#define ACCMD_GET_CAPABILITIES_REQ_START_INDEX_GET(addr) (*((addr) + 3))
#define ACCMD_GET_CAPABILITIES_REQ_START_INDEX_SET(addr, start_index) (*((addr) + 3) = (uint16)(start_index))
#define ACCMD_GET_CAPABILITIES_REQ_WORD_SIZE (4)
#define ACCMD_GET_CAPABILITIES_REQ_PACK(addr, start_index) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(start_index)); \
    } while (0)

#define ACCMD_GET_CAPABILITIES_REQ_MARSHALL(addr, accmd_get_capabilities_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_get_capabilities_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_get_capabilities_req_ptr)->start_index); \
    } while (0)

#define ACCMD_GET_CAPABILITIES_REQ_UNMARSHALL(addr, accmd_get_capabilities_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_get_capabilities_req_ptr)->header)); \
        (accmd_get_capabilities_req_ptr)->start_index = ACCMD_GET_CAPABILITIES_REQ_START_INDEX_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_External_Detach_Req

  DESCRIPTION

  MEMBERS
    Sid    - The Sid parameter specifies the Source or Sink ID to be detached

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Sid;
} ACCMD_STREAM_EXTERNAL_DETACH_REQ;

/* The following macros take ACCMD_STREAM_EXTERNAL_DETACH_REQ *accmd_stream_external_detach_req_ptr or uint16 *addr */
#define ACCMD_STREAM_EXTERNAL_DETACH_REQ_SID_WORD_OFFSET (3)
#define ACCMD_STREAM_EXTERNAL_DETACH_REQ_SID_GET(addr) (*((addr) + 3))
#define ACCMD_STREAM_EXTERNAL_DETACH_REQ_SID_SET(addr, sid) (*((addr) + 3) = (uint16)(sid))
#define ACCMD_STREAM_EXTERNAL_DETACH_REQ_WORD_SIZE (4)
#define ACCMD_STREAM_EXTERNAL_DETACH_REQ_PACK(addr, Sid) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Sid)); \
    } while (0)

#define ACCMD_STREAM_EXTERNAL_DETACH_REQ_MARSHALL(addr, accmd_stream_external_detach_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_external_detach_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_external_detach_req_ptr)->Sid); \
    } while (0)

#define ACCMD_STREAM_EXTERNAL_DETACH_REQ_UNMARSHALL(addr, accmd_stream_external_detach_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_external_detach_req_ptr)->header)); \
        (accmd_stream_external_detach_req_ptr)->Sid = ACCMD_STREAM_EXTERNAL_DETACH_REQ_SID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Source_Drained_Req

  DESCRIPTION

  MEMBERS
    Source_ID - A sid referencing the requested source

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Source_ID;
} ACCMD_STREAM_SOURCE_DRAINED_REQ;

/* The following macros take ACCMD_STREAM_SOURCE_DRAINED_REQ *accmd_stream_source_drained_req_ptr or uint16 *addr */
#define ACCMD_STREAM_SOURCE_DRAINED_REQ_SOURCE_ID_WORD_OFFSET (3)
#define ACCMD_STREAM_SOURCE_DRAINED_REQ_SOURCE_ID_GET(addr) (*((addr) + 3))
#define ACCMD_STREAM_SOURCE_DRAINED_REQ_SOURCE_ID_SET(addr, source_id) (*((addr) + 3) = (uint16)(source_id))
#define ACCMD_STREAM_SOURCE_DRAINED_REQ_WORD_SIZE (4)
#define ACCMD_STREAM_SOURCE_DRAINED_REQ_PACK(addr, Source_ID) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Source_ID)); \
    } while (0)

#define ACCMD_STREAM_SOURCE_DRAINED_REQ_MARSHALL(addr, accmd_stream_source_drained_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_source_drained_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_source_drained_req_ptr)->Source_ID); \
    } while (0)

#define ACCMD_STREAM_SOURCE_DRAINED_REQ_UNMARSHALL(addr, accmd_stream_source_drained_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_source_drained_req_ptr)->header)); \
        (accmd_stream_source_drained_req_ptr)->Source_ID = ACCMD_STREAM_SOURCE_DRAINED_REQ_SOURCE_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Create_Operator_Ex_Req

  DESCRIPTION
    From Client This command requests that the Audio subsystem instantiates
    the provided capability as an operator.

  MEMBERS
    Capability_ID - The type of capability being requested
    num_keys      - Number of configuration key-value pairs
    info          - An array of key-value objects

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER                  header;
    uint16                        Capability_ID;
    uint16                        num_keys;
    ACCMD_OPERATOR_CREATE_EX_INFO info[ACCMD_PRIM_ANY_SIZE];
} ACCMD_CREATE_OPERATOR_EX_REQ;

/* The following macros take ACCMD_CREATE_OPERATOR_EX_REQ *accmd_create_operator_ex_req_ptr or uint16 *addr */
#define ACCMD_CREATE_OPERATOR_EX_REQ_CAPABILITY_ID_WORD_OFFSET (3)
#define ACCMD_CREATE_OPERATOR_EX_REQ_CAPABILITY_ID_GET(addr) (*((addr) + 3))
#define ACCMD_CREATE_OPERATOR_EX_REQ_CAPABILITY_ID_SET(addr, capability_id) (*((addr) + 3) = (uint16)(capability_id))
#define ACCMD_CREATE_OPERATOR_EX_REQ_NUM_KEYS_WORD_OFFSET (4)
#define ACCMD_CREATE_OPERATOR_EX_REQ_NUM_KEYS_GET(addr) (*((addr) + 4))
#define ACCMD_CREATE_OPERATOR_EX_REQ_NUM_KEYS_SET(addr, num_keys) (*((addr) + 4) = (uint16)(num_keys))
#define ACCMD_CREATE_OPERATOR_EX_REQ_INFO_WORD_OFFSET (5)
#define ACCMD_CREATE_OPERATOR_EX_REQ_WORD_SIZE (8)
#define ACCMD_CREATE_OPERATOR_EX_REQ_PACK(addr, Capability_ID, num_keys) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Capability_ID)); \
        *((addr) + 4) = (uint16)((uint16)(num_keys)); \
    } while (0)

#define ACCMD_CREATE_OPERATOR_EX_REQ_MARSHALL(addr, accmd_create_operator_ex_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_create_operator_ex_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_create_operator_ex_req_ptr)->Capability_ID); \
        *((addr) + 4) = (uint16)((accmd_create_operator_ex_req_ptr)->num_keys); \
    } while (0)

#define ACCMD_CREATE_OPERATOR_EX_REQ_UNMARSHALL(addr, accmd_create_operator_ex_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_create_operator_ex_req_ptr)->header)); \
        (accmd_create_operator_ex_req_ptr)->Capability_ID = ACCMD_CREATE_OPERATOR_EX_REQ_CAPABILITY_ID_GET(addr); \
        (accmd_create_operator_ex_req_ptr)->num_keys = ACCMD_CREATE_OPERATOR_EX_REQ_NUM_KEYS_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_Mem_Usage_Req

  DESCRIPTION
    From Client: Get memory usage.

  MEMBERS
    Proc_ID - Identifier of processor to query (0 or 1).

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Proc_ID[ACCMD_PRIM_ANY_SIZE];
} ACCMD_GET_MEM_USAGE_REQ;

/* The following macros take ACCMD_GET_MEM_USAGE_REQ *accmd_get_mem_usage_req_ptr or uint16 *addr */
#define ACCMD_GET_MEM_USAGE_REQ_PROC_ID_WORD_OFFSET (3)
#define ACCMD_GET_MEM_USAGE_REQ_PROC_ID_GET(addr) (*((addr) + 3))
#define ACCMD_GET_MEM_USAGE_REQ_PROC_ID_SET(addr, proc_id) (*((addr) + 3) = (uint16)(proc_id))
#define ACCMD_GET_MEM_USAGE_REQ_WORD_SIZE (4)
#define ACCMD_GET_MEM_USAGE_REQ_PACK(addr, Proc_ID) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Proc_ID)); \
    } while (0)

#define ACCMD_GET_MEM_USAGE_REQ_MARSHALL(addr, accmd_get_mem_usage_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_get_mem_usage_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_get_mem_usage_req_ptr)->Proc_ID); \
    } while (0)

#define ACCMD_GET_MEM_USAGE_REQ_UNMARSHALL(addr, accmd_get_mem_usage_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_get_mem_usage_req_ptr)->header)); \
        (accmd_get_mem_usage_req_ptr)->Proc_ID = ACCMD_GET_MEM_USAGE_REQ_PROC_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Clear_Mem_Watermarks_Req

  DESCRIPTION
    From Client Clear memory watermarks.

  MEMBERS

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
} ACCMD_CLEAR_MEM_WATERMARKS_REQ;

/* The following macros take ACCMD_CLEAR_MEM_WATERMARKS_REQ *accmd_clear_mem_watermarks_req_ptr or uint16 *addr */
#define ACCMD_CLEAR_MEM_WATERMARKS_REQ_WORD_SIZE (3)


/*******************************************************************************

  NAME
    Accmd_Get_Mips_Usage_Req

  DESCRIPTION
    Request message for getting MIPS usage of an operator.

  MEMBERS
    Count  - The number of operators in the list OP_IDS.
    OP_IDS - The list of operators to get mips on.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Count;
    uint16       OP_IDS[ACCMD_PRIM_ANY_SIZE];
} ACCMD_GET_MIPS_USAGE_REQ;

/* The following macros take ACCMD_GET_MIPS_USAGE_REQ *accmd_get_mips_usage_req_ptr or uint16 *addr */
#define ACCMD_GET_MIPS_USAGE_REQ_COUNT_WORD_OFFSET (3)
#define ACCMD_GET_MIPS_USAGE_REQ_COUNT_GET(addr) (*((addr) + 3))
#define ACCMD_GET_MIPS_USAGE_REQ_COUNT_SET(addr, count) (*((addr) + 3) = (uint16)(count))
#define ACCMD_GET_MIPS_USAGE_REQ_OP_IDS_WORD_OFFSET (4)
#define ACCMD_GET_MIPS_USAGE_REQ_OP_IDS_GET(addr) (*((addr) + 4))
#define ACCMD_GET_MIPS_USAGE_REQ_OP_IDS_SET(addr, op_ids) (*((addr) + 4) = (uint16)(op_ids))
#define ACCMD_GET_MIPS_USAGE_REQ_WORD_SIZE (5)
#define ACCMD_GET_MIPS_USAGE_REQ_PACK(addr, Count, OP_IDS) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Count)); \
        *((addr) + 4) = (uint16)((uint16)(OP_IDS)); \
    } while (0)

#define ACCMD_GET_MIPS_USAGE_REQ_MARSHALL(addr, accmd_get_mips_usage_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_get_mips_usage_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_get_mips_usage_req_ptr)->Count); \
        *((addr) + 4) = (uint16)((accmd_get_mips_usage_req_ptr)->OP_IDS); \
    } while (0)

#define ACCMD_GET_MIPS_USAGE_REQ_UNMARSHALL(addr, accmd_get_mips_usage_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_get_mips_usage_req_ptr)->header)); \
        (accmd_get_mips_usage_req_ptr)->Count = ACCMD_GET_MIPS_USAGE_REQ_COUNT_GET(addr); \
        (accmd_get_mips_usage_req_ptr)->OP_IDS = ACCMD_GET_MIPS_USAGE_REQ_OP_IDS_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Set_System_Stream_Rate_Req

  DESCRIPTION
    Set the default system streaming rate

  MEMBERS
    Sample_rate - System sampling rate

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint32       Sample_rate;
} ACCMD_SET_SYSTEM_STREAM_RATE_REQ;

/* The following macros take ACCMD_SET_SYSTEM_STREAM_RATE_REQ *accmd_set_system_stream_rate_req_ptr or uint16 *addr */
#define ACCMD_SET_SYSTEM_STREAM_RATE_REQ_SAMPLE_RATE_WORD_OFFSET (3)
#define ACCMD_SET_SYSTEM_STREAM_RATE_REQ_SAMPLE_RATE_GET(addr)  \
    (((uint32)(*((addr) + 3)) | \
      ((uint32)(*((addr) + 4)) << 16)))
#define ACCMD_SET_SYSTEM_STREAM_RATE_REQ_SAMPLE_RATE_SET(addr, sample_rate) do { \
        *((addr) + 3) = (uint16)((sample_rate) & 0xffff); \
        *((addr) + 4) = (uint16)((sample_rate) >> 16); } while (0)
#define ACCMD_SET_SYSTEM_STREAM_RATE_REQ_WORD_SIZE (5)
#define ACCMD_SET_SYSTEM_STREAM_RATE_REQ_PACK(addr, Sample_rate) \
    do { \
        *((addr) + 3) = (uint16)((uint16)((Sample_rate) & 0xffff)); \
        *((addr) + 4) = (uint16)(((Sample_rate) >> 16)); \
    } while (0)

#define ACCMD_SET_SYSTEM_STREAM_RATE_REQ_MARSHALL(addr, accmd_set_system_stream_rate_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_set_system_stream_rate_req_ptr)->header)); \
        *((addr) + 3) = (uint16)(((accmd_set_system_stream_rate_req_ptr)->Sample_rate) & 0xffff); \
        *((addr) + 4) = (uint16)(((accmd_set_system_stream_rate_req_ptr)->Sample_rate) >> 16); \
    } while (0)

#define ACCMD_SET_SYSTEM_STREAM_RATE_REQ_UNMARSHALL(addr, accmd_set_system_stream_rate_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_set_system_stream_rate_req_ptr)->header)); \
        (accmd_set_system_stream_rate_req_ptr)->Sample_rate = ACCMD_SET_SYSTEM_STREAM_RATE_REQ_SAMPLE_RATE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_System_Stream_Rate_Req

  DESCRIPTION
    Get the default streaming rate

  MEMBERS

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
} ACCMD_GET_SYSTEM_STREAM_RATE_REQ;

/* The following macros take ACCMD_GET_SYSTEM_STREAM_RATE_REQ *accmd_get_system_stream_rate_req_ptr or uint16 *addr */
#define ACCMD_GET_SYSTEM_STREAM_RATE_REQ_WORD_SIZE (3)


/*******************************************************************************

  NAME
    Accmd_Stream_Disconnect_Req

  DESCRIPTION
    Disconnect endpoints request.

  MEMBERS
    Source - Source endpoint.
    Sink   - Sink endpoint

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Source;
    uint16       Sink;
} ACCMD_STREAM_DISCONNECT_REQ;

/* The following macros take ACCMD_STREAM_DISCONNECT_REQ *accmd_stream_disconnect_req_ptr or uint16 *addr */
#define ACCMD_STREAM_DISCONNECT_REQ_SOURCE_WORD_OFFSET (3)
#define ACCMD_STREAM_DISCONNECT_REQ_SOURCE_GET(addr) (*((addr) + 3))
#define ACCMD_STREAM_DISCONNECT_REQ_SOURCE_SET(addr, source) (*((addr) + 3) = (uint16)(source))
#define ACCMD_STREAM_DISCONNECT_REQ_SINK_WORD_OFFSET (4)
#define ACCMD_STREAM_DISCONNECT_REQ_SINK_GET(addr) (*((addr) + 4))
#define ACCMD_STREAM_DISCONNECT_REQ_SINK_SET(addr, sink) (*((addr) + 4) = (uint16)(sink))
#define ACCMD_STREAM_DISCONNECT_REQ_WORD_SIZE (5)
#define ACCMD_STREAM_DISCONNECT_REQ_PACK(addr, Source, Sink) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Source)); \
        *((addr) + 4) = (uint16)((uint16)(Sink)); \
    } while (0)

#define ACCMD_STREAM_DISCONNECT_REQ_MARSHALL(addr, accmd_stream_disconnect_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_disconnect_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_disconnect_req_ptr)->Source); \
        *((addr) + 4) = (uint16)((accmd_stream_disconnect_req_ptr)->Sink); \
    } while (0)

#define ACCMD_STREAM_DISCONNECT_REQ_UNMARSHALL(addr, accmd_stream_disconnect_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_disconnect_req_ptr)->header)); \
        (accmd_stream_disconnect_req_ptr)->Source = ACCMD_STREAM_DISCONNECT_REQ_SOURCE_GET(addr); \
        (accmd_stream_disconnect_req_ptr)->Sink = ACCMD_STREAM_DISCONNECT_REQ_SINK_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Get_Sink_From_Source_Req

  DESCRIPTION
    Find sink endpoint from stream source request.

  MEMBERS
    Source - Source endpoint.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Source;
} ACCMD_STREAM_GET_SINK_FROM_SOURCE_REQ;

/* The following macros take ACCMD_STREAM_GET_SINK_FROM_SOURCE_REQ *accmd_stream_get_sink_from_source_req_ptr or uint16 *addr */
#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_REQ_SOURCE_WORD_OFFSET (3)
#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_REQ_SOURCE_GET(addr) (*((addr) + 3))
#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_REQ_SOURCE_SET(addr, source) (*((addr) + 3) = (uint16)(source))
#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_REQ_WORD_SIZE (4)
#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_REQ_PACK(addr, Source) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Source)); \
    } while (0)

#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_REQ_MARSHALL(addr, accmd_stream_get_sink_from_source_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_get_sink_from_source_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_get_sink_from_source_req_ptr)->Source); \
    } while (0)

#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_REQ_UNMARSHALL(addr, accmd_stream_get_sink_from_source_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_get_sink_from_source_req_ptr)->header)); \
        (accmd_stream_get_sink_from_source_req_ptr)->Source = ACCMD_STREAM_GET_SINK_FROM_SOURCE_REQ_SOURCE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Get_Source_From_Sink_Req

  DESCRIPTION
    Find source endpoint from stream sink request.

  MEMBERS
    Sink   - Sink endpoint

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Sink;
} ACCMD_STREAM_GET_SOURCE_FROM_SINK_REQ;

/* The following macros take ACCMD_STREAM_GET_SOURCE_FROM_SINK_REQ *accmd_stream_get_source_from_sink_req_ptr or uint16 *addr */
#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_REQ_SINK_WORD_OFFSET (3)
#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_REQ_SINK_GET(addr) (*((addr) + 3))
#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_REQ_SINK_SET(addr, sink) (*((addr) + 3) = (uint16)(sink))
#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_REQ_WORD_SIZE (4)
#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_REQ_PACK(addr, Sink) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Sink)); \
    } while (0)

#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_REQ_MARSHALL(addr, accmd_stream_get_source_from_sink_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_get_source_from_sink_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_get_source_from_sink_req_ptr)->Sink); \
    } while (0)

#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_REQ_UNMARSHALL(addr, accmd_stream_get_source_from_sink_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_get_source_from_sink_req_ptr)->header)); \
        (accmd_stream_get_source_from_sink_req_ptr)->Sink = ACCMD_STREAM_GET_SOURCE_FROM_SINK_REQ_SINK_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Cap_Download_Remove_Kcs_Req

  DESCRIPTION
    Remove downloaded KCS.

  MEMBERS
    KCS_id - Id of KCS to remove

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       KCS_id;
} ACCMD_CAP_DOWNLOAD_REMOVE_KCS_REQ;

/* The following macros take ACCMD_CAP_DOWNLOAD_REMOVE_KCS_REQ *accmd_cap_download_remove_kcs_req_ptr or uint16 *addr */
#define ACCMD_CAP_DOWNLOAD_REMOVE_KCS_REQ_KCS_ID_WORD_OFFSET (3)
#define ACCMD_CAP_DOWNLOAD_REMOVE_KCS_REQ_KCS_ID_GET(addr) (*((addr) + 3))
#define ACCMD_CAP_DOWNLOAD_REMOVE_KCS_REQ_KCS_ID_SET(addr, kcs_id) (*((addr) + 3) = (uint16)(kcs_id))
#define ACCMD_CAP_DOWNLOAD_REMOVE_KCS_REQ_WORD_SIZE (4)
#define ACCMD_CAP_DOWNLOAD_REMOVE_KCS_REQ_PACK(addr, KCS_id) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(KCS_id)); \
    } while (0)

#define ACCMD_CAP_DOWNLOAD_REMOVE_KCS_REQ_MARSHALL(addr, accmd_cap_download_remove_kcs_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_cap_download_remove_kcs_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_cap_download_remove_kcs_req_ptr)->KCS_id); \
    } while (0)

#define ACCMD_CAP_DOWNLOAD_REMOVE_KCS_REQ_UNMARSHALL(addr, accmd_cap_download_remove_kcs_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_cap_download_remove_kcs_req_ptr)->header)); \
        (accmd_cap_download_remove_kcs_req_ptr)->KCS_id = ACCMD_CAP_DOWNLOAD_REMOVE_KCS_REQ_KCS_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Set_System_Framework_Req

  DESCRIPTION
    Set the system parameters specified by the key.

  MEMBERS
    key     - The key and the values for the parameters that need to be set.
    message - It contains the parameters that need to be set.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER    header;
    SYSTEM_KEYS_MSG key;
    uint16          message[ACCMD_PRIM_ANY_SIZE];
} ACCMD_SET_SYSTEM_FRAMEWORK_REQ;

/* The following macros take ACCMD_SET_SYSTEM_FRAMEWORK_REQ *accmd_set_system_framework_req_ptr or uint16 *addr */
#define ACCMD_SET_SYSTEM_FRAMEWORK_REQ_KEY_WORD_OFFSET (3)
#define ACCMD_SET_SYSTEM_FRAMEWORK_REQ_KEY_GET(addr) ((SYSTEM_KEYS_MSG)*((addr) + 3))
#define ACCMD_SET_SYSTEM_FRAMEWORK_REQ_KEY_SET(addr, key) (*((addr) + 3) = (uint16)(key))
#define ACCMD_SET_SYSTEM_FRAMEWORK_REQ_MESSAGE_WORD_OFFSET (4)
#define ACCMD_SET_SYSTEM_FRAMEWORK_REQ_MESSAGE_GET(addr) (*((addr) + 4))
#define ACCMD_SET_SYSTEM_FRAMEWORK_REQ_MESSAGE_SET(addr, message) (*((addr) + 4) = (uint16)(message))
#define ACCMD_SET_SYSTEM_FRAMEWORK_REQ_WORD_SIZE (5)
#define ACCMD_SET_SYSTEM_FRAMEWORK_REQ_PACK(addr, key, message) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(key)); \
        *((addr) + 4) = (uint16)((uint16)(message)); \
    } while (0)

#define ACCMD_SET_SYSTEM_FRAMEWORK_REQ_MARSHALL(addr, accmd_set_system_framework_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_set_system_framework_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_set_system_framework_req_ptr)->key); \
        *((addr) + 4) = (uint16)((accmd_set_system_framework_req_ptr)->message); \
    } while (0)

#define ACCMD_SET_SYSTEM_FRAMEWORK_REQ_UNMARSHALL(addr, accmd_set_system_framework_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_set_system_framework_req_ptr)->header)); \
        (accmd_set_system_framework_req_ptr)->key = ACCMD_SET_SYSTEM_FRAMEWORK_REQ_KEY_GET(addr); \
        (accmd_set_system_framework_req_ptr)->message = ACCMD_SET_SYSTEM_FRAMEWORK_REQ_MESSAGE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_System_Framework_Req

  DESCRIPTION
    From Client This command requests that the Audio subsystem gets the
    parameters specified by the key received.

  MEMBERS
    key     - The parameters to be get.
    message - The message that contains specific information for each case and
              may be empty.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER    header;
    SYSTEM_KEYS_MSG key;
    uint16          message[ACCMD_PRIM_ANY_SIZE];
} ACCMD_GET_SYSTEM_FRAMEWORK_REQ;

/* The following macros take ACCMD_GET_SYSTEM_FRAMEWORK_REQ *accmd_get_system_framework_req_ptr or uint16 *addr */
#define ACCMD_GET_SYSTEM_FRAMEWORK_REQ_KEY_WORD_OFFSET (3)
#define ACCMD_GET_SYSTEM_FRAMEWORK_REQ_KEY_GET(addr) ((SYSTEM_KEYS_MSG)*((addr) + 3))
#define ACCMD_GET_SYSTEM_FRAMEWORK_REQ_KEY_SET(addr, key) (*((addr) + 3) = (uint16)(key))
#define ACCMD_GET_SYSTEM_FRAMEWORK_REQ_MESSAGE_WORD_OFFSET (4)
#define ACCMD_GET_SYSTEM_FRAMEWORK_REQ_MESSAGE_GET(addr) (*((addr) + 4))
#define ACCMD_GET_SYSTEM_FRAMEWORK_REQ_MESSAGE_SET(addr, message) (*((addr) + 4) = (uint16)(message))
#define ACCMD_GET_SYSTEM_FRAMEWORK_REQ_WORD_SIZE (5)
#define ACCMD_GET_SYSTEM_FRAMEWORK_REQ_PACK(addr, key, message) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(key)); \
        *((addr) + 4) = (uint16)((uint16)(message)); \
    } while (0)

#define ACCMD_GET_SYSTEM_FRAMEWORK_REQ_MARSHALL(addr, accmd_get_system_framework_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_get_system_framework_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_get_system_framework_req_ptr)->key); \
        *((addr) + 4) = (uint16)((accmd_get_system_framework_req_ptr)->message); \
    } while (0)

#define ACCMD_GET_SYSTEM_FRAMEWORK_REQ_UNMARSHALL(addr, accmd_get_system_framework_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_get_system_framework_req_ptr)->header)); \
        (accmd_get_system_framework_req_ptr)->key = ACCMD_GET_SYSTEM_FRAMEWORK_REQ_KEY_GET(addr); \
        (accmd_get_system_framework_req_ptr)->message = ACCMD_GET_SYSTEM_FRAMEWORK_REQ_MESSAGE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Unsolicited_System_Framework_Req

  DESCRIPTION
    From Client This command is for sending unsolicited messages.

  MEMBERS
    key     - The key corresponding to the type of unsolicited message.
    payload - payload

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER    header;
    SYSTEM_KEYS_MSG key;
    uint16          payload[ACCMD_PRIM_ANY_SIZE];
} ACCMD_UNSOLICITED_SYSTEM_FRAMEWORK_REQ;

/* The following macros take ACCMD_UNSOLICITED_SYSTEM_FRAMEWORK_REQ *accmd_unsolicited_system_framework_req_ptr or uint16 *addr */
#define ACCMD_UNSOLICITED_SYSTEM_FRAMEWORK_REQ_KEY_WORD_OFFSET (3)
#define ACCMD_UNSOLICITED_SYSTEM_FRAMEWORK_REQ_KEY_GET(addr) ((SYSTEM_KEYS_MSG)*((addr) + 3))
#define ACCMD_UNSOLICITED_SYSTEM_FRAMEWORK_REQ_KEY_SET(addr, key) (*((addr) + 3) = (uint16)(key))
#define ACCMD_UNSOLICITED_SYSTEM_FRAMEWORK_REQ_PAYLOAD_WORD_OFFSET (4)
#define ACCMD_UNSOLICITED_SYSTEM_FRAMEWORK_REQ_PAYLOAD_GET(addr) (*((addr) + 4))
#define ACCMD_UNSOLICITED_SYSTEM_FRAMEWORK_REQ_PAYLOAD_SET(addr, payload) (*((addr) + 4) = (uint16)(payload))
#define ACCMD_UNSOLICITED_SYSTEM_FRAMEWORK_REQ_WORD_SIZE (5)
#define ACCMD_UNSOLICITED_SYSTEM_FRAMEWORK_REQ_PACK(addr, key, payload) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(key)); \
        *((addr) + 4) = (uint16)((uint16)(payload)); \
    } while (0)

#define ACCMD_UNSOLICITED_SYSTEM_FRAMEWORK_REQ_MARSHALL(addr, accmd_unsolicited_system_framework_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_unsolicited_system_framework_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_unsolicited_system_framework_req_ptr)->key); \
        *((addr) + 4) = (uint16)((accmd_unsolicited_system_framework_req_ptr)->payload); \
    } while (0)

#define ACCMD_UNSOLICITED_SYSTEM_FRAMEWORK_REQ_UNMARSHALL(addr, accmd_unsolicited_system_framework_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_unsolicited_system_framework_req_ptr)->header)); \
        (accmd_unsolicited_system_framework_req_ptr)->key = ACCMD_UNSOLICITED_SYSTEM_FRAMEWORK_REQ_KEY_GET(addr); \
        (accmd_unsolicited_system_framework_req_ptr)->payload = ACCMD_UNSOLICITED_SYSTEM_FRAMEWORK_REQ_PAYLOAD_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Mic_Bias_Configure_Req

  DESCRIPTION
    From Client This command allows on-chip microphone bias generator(s) to
    be enabled, disabled, and possibly configured.

  MEMBERS
    Mic_Bias_Id - The Mic_Bias_Id parameter identifies the mic bias instance to
                  configure (on chips with more than one), starting at 0.
    Key         - The Key parameter specifies the property of mic bias generator
                  to be configured
    Value       - The Value parameter specifies the data value to be assigned to
                  the key

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER              header;
    uint16                    Mic_Bias_Id;
    ACCMD_MIC_BIAS_CONFIG_KEY Key;
    uint32                    Value;
} ACCMD_MIC_BIAS_CONFIGURE_REQ;

/* The following macros take ACCMD_MIC_BIAS_CONFIGURE_REQ *accmd_mic_bias_configure_req_ptr or uint16 *addr */
#define ACCMD_MIC_BIAS_CONFIGURE_REQ_MIC_BIAS_ID_WORD_OFFSET (3)
#define ACCMD_MIC_BIAS_CONFIGURE_REQ_MIC_BIAS_ID_GET(addr) (*((addr) + 3))
#define ACCMD_MIC_BIAS_CONFIGURE_REQ_MIC_BIAS_ID_SET(addr, mic_bias_id) (*((addr) + 3) = (uint16)(mic_bias_id))
#define ACCMD_MIC_BIAS_CONFIGURE_REQ_KEY_WORD_OFFSET (4)
#define ACCMD_MIC_BIAS_CONFIGURE_REQ_KEY_GET(addr) ((ACCMD_MIC_BIAS_CONFIG_KEY)*((addr) + 4))
#define ACCMD_MIC_BIAS_CONFIGURE_REQ_KEY_SET(addr, key) (*((addr) + 4) = (uint16)(key))
#define ACCMD_MIC_BIAS_CONFIGURE_REQ_VALUE_WORD_OFFSET (5)
#define ACCMD_MIC_BIAS_CONFIGURE_REQ_VALUE_GET(addr)  \
    (((uint32)(*((addr) + 5)) | \
      ((uint32)(*((addr) + 6)) << 16)))
#define ACCMD_MIC_BIAS_CONFIGURE_REQ_VALUE_SET(addr, value) do { \
        *((addr) + 5) = (uint16)((value) & 0xffff); \
        *((addr) + 6) = (uint16)((value) >> 16); } while (0)
#define ACCMD_MIC_BIAS_CONFIGURE_REQ_WORD_SIZE (7)
#define ACCMD_MIC_BIAS_CONFIGURE_REQ_PACK(addr, Mic_Bias_Id, Key, Value) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Mic_Bias_Id)); \
        *((addr) + 4) = (uint16)((uint16)(Key)); \
        *((addr) + 5) = (uint16)((uint16)((Value) & 0xffff)); \
        *((addr) + 6) = (uint16)(((Value) >> 16)); \
    } while (0)

#define ACCMD_MIC_BIAS_CONFIGURE_REQ_MARSHALL(addr, accmd_mic_bias_configure_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_mic_bias_configure_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_mic_bias_configure_req_ptr)->Mic_Bias_Id); \
        *((addr) + 4) = (uint16)((accmd_mic_bias_configure_req_ptr)->Key); \
        *((addr) + 5) = (uint16)(((accmd_mic_bias_configure_req_ptr)->Value) & 0xffff); \
        *((addr) + 6) = (uint16)(((accmd_mic_bias_configure_req_ptr)->Value) >> 16); \
    } while (0)

#define ACCMD_MIC_BIAS_CONFIGURE_REQ_UNMARSHALL(addr, accmd_mic_bias_configure_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_mic_bias_configure_req_ptr)->header)); \
        (accmd_mic_bias_configure_req_ptr)->Mic_Bias_Id = ACCMD_MIC_BIAS_CONFIGURE_REQ_MIC_BIAS_ID_GET(addr); \
        (accmd_mic_bias_configure_req_ptr)->Key = ACCMD_MIC_BIAS_CONFIGURE_REQ_KEY_GET(addr); \
        (accmd_mic_bias_configure_req_ptr)->Value = ACCMD_MIC_BIAS_CONFIGURE_REQ_VALUE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Codec_Set_Iir_Filter_Req

  DESCRIPTION
    From Client This command allows configuration of IIR filter in each of
    the ADC channels.

  MEMBERS
    Codec_Channel_Mask - The Codec_Channel_Mask parameter is a bitfield that
                         identifies ADC channels on which new IIR coefficients
                         need to be applied. Bit 0 corresponds to ADC channel A.
                         Bit 1 corresponds to ADC channel B and so on.
    Enable             - The Enable parameter specifies whether the IIR filter
                         needs to be enabled as well.
    no_of_coeffs       - The number of coeffs.
    coeffs             - An array of filter coefficients

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER                 header;
    uint16                       Codec_Channel_Mask;
    uint16                       Enable;
    uint16                       no_of_coeffs;
    ACCMD_CODEC_IIR_FILTER_COEFF coeffs[ACCMD_PRIM_ANY_SIZE];
} ACCMD_CODEC_SET_IIR_FILTER_REQ;

/* The following macros take ACCMD_CODEC_SET_IIR_FILTER_REQ *accmd_codec_set_iir_filter_req_ptr or uint16 *addr */
#define ACCMD_CODEC_SET_IIR_FILTER_REQ_CODEC_CHANNEL_MASK_WORD_OFFSET (3)
#define ACCMD_CODEC_SET_IIR_FILTER_REQ_CODEC_CHANNEL_MASK_GET(addr) (*((addr) + 3))
#define ACCMD_CODEC_SET_IIR_FILTER_REQ_CODEC_CHANNEL_MASK_SET(addr, codec_channel_mask) (*((addr) + 3) = (uint16)(codec_channel_mask))
#define ACCMD_CODEC_SET_IIR_FILTER_REQ_ENABLE_WORD_OFFSET (4)
#define ACCMD_CODEC_SET_IIR_FILTER_REQ_ENABLE_GET(addr) (*((addr) + 4))
#define ACCMD_CODEC_SET_IIR_FILTER_REQ_ENABLE_SET(addr, enable) (*((addr) + 4) = (uint16)(enable))
#define ACCMD_CODEC_SET_IIR_FILTER_REQ_NO_OF_COEFFS_WORD_OFFSET (5)
#define ACCMD_CODEC_SET_IIR_FILTER_REQ_NO_OF_COEFFS_GET(addr) (*((addr) + 5))
#define ACCMD_CODEC_SET_IIR_FILTER_REQ_NO_OF_COEFFS_SET(addr, no_of_coeffs) (*((addr) + 5) = (uint16)(no_of_coeffs))
#define ACCMD_CODEC_SET_IIR_FILTER_REQ_COEFFS_WORD_OFFSET (6)
#define ACCMD_CODEC_SET_IIR_FILTER_REQ_COEFFS_GET(addr, coeffs_ptr) do {  \
        (coeffs_ptr)->_data[0] = *((addr) + 6); } while (0)
#define ACCMD_CODEC_SET_IIR_FILTER_REQ_COEFFS_SET(addr, coeffs_ptr) do {  \
        *((addr) + 6) = (coeffs_ptr)->_data[0]; } while (0)
#define ACCMD_CODEC_SET_IIR_FILTER_REQ_WORD_SIZE (7)
#define ACCMD_CODEC_SET_IIR_FILTER_REQ_PACK(addr, Codec_Channel_Mask, Enable, no_of_coeffs, coeffs_ptr) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Codec_Channel_Mask)); \
        *((addr) + 4) = (uint16)((uint16)(Enable)); \
        *((addr) + 5) = (uint16)((uint16)(no_of_coeffs)); \
        *((addr) + 6) = (uint16)((coeffs_ptr)->_data[0]); \
    } while (0)

#define ACCMD_CODEC_SET_IIR_FILTER_REQ_MARSHALL(addr, accmd_codec_set_iir_filter_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_codec_set_iir_filter_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_codec_set_iir_filter_req_ptr)->Codec_Channel_Mask); \
        *((addr) + 4) = (uint16)((accmd_codec_set_iir_filter_req_ptr)->Enable); \
        *((addr) + 5) = (uint16)((accmd_codec_set_iir_filter_req_ptr)->no_of_coeffs); \
        ACCMD_CODEC_IIR_FILTER_COEFF_MARSHALL((addr) + ACCMD_CODEC_SET_IIR_FILTER_REQ_COEFFS_WORD_OFFSET, &((accmd_codec_set_iir_filter_req_ptr)->coeffs)); \
    } while (0)

#define ACCMD_CODEC_SET_IIR_FILTER_REQ_UNMARSHALL(addr, accmd_codec_set_iir_filter_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_codec_set_iir_filter_req_ptr)->header)); \
        (accmd_codec_set_iir_filter_req_ptr)->Codec_Channel_Mask = ACCMD_CODEC_SET_IIR_FILTER_REQ_CODEC_CHANNEL_MASK_GET(addr); \
        (accmd_codec_set_iir_filter_req_ptr)->Enable = ACCMD_CODEC_SET_IIR_FILTER_REQ_ENABLE_GET(addr); \
        (accmd_codec_set_iir_filter_req_ptr)->no_of_coeffs = ACCMD_CODEC_SET_IIR_FILTER_REQ_NO_OF_COEFFS_GET(addr); \
        ACCMD_CODEC_IIR_FILTER_COEFF_UNMARSHALL((addr) + ACCMD_CODEC_SET_IIR_FILTER_REQ_COEFFS_WORD_OFFSET, &((accmd_codec_set_iir_filter_req_ptr)->coeffs)); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Codec_Set_Iir_Filter_16bit_Req

  DESCRIPTION
    From Client This command allows configuration of IIR filter in each of
    the ADC channels.

  MEMBERS
    Codec_Channel_Mask - The Codec_Channel_Mask parameter is a bitfield that
                         identifies ADC channels on which new IIR coefficients
                         need to be applied. Bit 0 corresponds to ADC channel A.
                         Bit 1 corresponds to ADC channel B and so on.
    Enable             - The Enable parameter specifies whether the IIR filter
                         needs to be enabled as well.
    no_of_coeffs       - The number of coeffs.
    coeffs             - An array of filter coefficients

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER                 header;
    uint16                       Codec_Channel_Mask;
    uint16                       Enable;
    uint16                       no_of_coeffs;
    ACCMD_CODEC_IIR_FILTER_COEFF coeffs[ACCMD_PRIM_ANY_SIZE];
} ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ;

/* The following macros take ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ *accmd_codec_set_iir_filter_16bit_req_ptr or uint16 *addr */
#define ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_CODEC_CHANNEL_MASK_WORD_OFFSET (3)
#define ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_CODEC_CHANNEL_MASK_GET(addr) (*((addr) + 3))
#define ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_CODEC_CHANNEL_MASK_SET(addr, codec_channel_mask) (*((addr) + 3) = (uint16)(codec_channel_mask))
#define ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_ENABLE_WORD_OFFSET (4)
#define ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_ENABLE_GET(addr) (*((addr) + 4))
#define ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_ENABLE_SET(addr, enable) (*((addr) + 4) = (uint16)(enable))
#define ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_NO_OF_COEFFS_WORD_OFFSET (5)
#define ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_NO_OF_COEFFS_GET(addr) (*((addr) + 5))
#define ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_NO_OF_COEFFS_SET(addr, no_of_coeffs) (*((addr) + 5) = (uint16)(no_of_coeffs))
#define ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_COEFFS_WORD_OFFSET (6)
#define ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_COEFFS_GET(addr, coeffs_ptr) do {  \
        (coeffs_ptr)->_data[0] = *((addr) + 6); } while (0)
#define ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_COEFFS_SET(addr, coeffs_ptr) do {  \
        *((addr) + 6) = (coeffs_ptr)->_data[0]; } while (0)
#define ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_WORD_SIZE (7)
#define ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_PACK(addr, Codec_Channel_Mask, Enable, no_of_coeffs, coeffs_ptr) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Codec_Channel_Mask)); \
        *((addr) + 4) = (uint16)((uint16)(Enable)); \
        *((addr) + 5) = (uint16)((uint16)(no_of_coeffs)); \
        *((addr) + 6) = (uint16)((coeffs_ptr)->_data[0]); \
    } while (0)

#define ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_MARSHALL(addr, accmd_codec_set_iir_filter_16bit_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_codec_set_iir_filter_16bit_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_codec_set_iir_filter_16bit_req_ptr)->Codec_Channel_Mask); \
        *((addr) + 4) = (uint16)((accmd_codec_set_iir_filter_16bit_req_ptr)->Enable); \
        *((addr) + 5) = (uint16)((accmd_codec_set_iir_filter_16bit_req_ptr)->no_of_coeffs); \
        ACCMD_CODEC_IIR_FILTER_COEFF_MARSHALL((addr) + ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_COEFFS_WORD_OFFSET, &((accmd_codec_set_iir_filter_16bit_req_ptr)->coeffs)); \
    } while (0)

#define ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_UNMARSHALL(addr, accmd_codec_set_iir_filter_16bit_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_codec_set_iir_filter_16bit_req_ptr)->header)); \
        (accmd_codec_set_iir_filter_16bit_req_ptr)->Codec_Channel_Mask = ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_CODEC_CHANNEL_MASK_GET(addr); \
        (accmd_codec_set_iir_filter_16bit_req_ptr)->Enable = ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_ENABLE_GET(addr); \
        (accmd_codec_set_iir_filter_16bit_req_ptr)->no_of_coeffs = ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_NO_OF_COEFFS_GET(addr); \
        ACCMD_CODEC_IIR_FILTER_COEFF_UNMARSHALL((addr) + ACCMD_CODEC_SET_IIR_FILTER_16BIT_REQ_COEFFS_WORD_OFFSET, &((accmd_codec_set_iir_filter_16bit_req_ptr)->coeffs)); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Aux_Processor_Load_Req

  DESCRIPTION
    Load secondary processor.

  MEMBERS
    Processor_Id   - Secondary processor ID.
    memory_type    - PM RAM/ROM/SQIF
    framework_type - Kymera or client framework.
    file_index     - File index in the SQIF file system to be loaded.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Processor_Id;
    uint16       memory_type;
    uint16       framework_type;
    uint16       file_index;
} ACCMD_AUX_PROCESSOR_LOAD_REQ;

/* The following macros take ACCMD_AUX_PROCESSOR_LOAD_REQ *accmd_aux_processor_load_req_ptr or uint16 *addr */
#define ACCMD_AUX_PROCESSOR_LOAD_REQ_PROCESSOR_ID_WORD_OFFSET (3)
#define ACCMD_AUX_PROCESSOR_LOAD_REQ_PROCESSOR_ID_GET(addr) (*((addr) + 3))
#define ACCMD_AUX_PROCESSOR_LOAD_REQ_PROCESSOR_ID_SET(addr, processor_id) (*((addr) + 3) = (uint16)(processor_id))
#define ACCMD_AUX_PROCESSOR_LOAD_REQ_MEMORY_TYPE_WORD_OFFSET (4)
#define ACCMD_AUX_PROCESSOR_LOAD_REQ_MEMORY_TYPE_GET(addr) (*((addr) + 4))
#define ACCMD_AUX_PROCESSOR_LOAD_REQ_MEMORY_TYPE_SET(addr, memory_type) (*((addr) + 4) = (uint16)(memory_type))
#define ACCMD_AUX_PROCESSOR_LOAD_REQ_FRAMEWORK_TYPE_WORD_OFFSET (5)
#define ACCMD_AUX_PROCESSOR_LOAD_REQ_FRAMEWORK_TYPE_GET(addr) (*((addr) + 5))
#define ACCMD_AUX_PROCESSOR_LOAD_REQ_FRAMEWORK_TYPE_SET(addr, framework_type) (*((addr) + 5) = (uint16)(framework_type))
#define ACCMD_AUX_PROCESSOR_LOAD_REQ_FILE_INDEX_WORD_OFFSET (6)
#define ACCMD_AUX_PROCESSOR_LOAD_REQ_FILE_INDEX_GET(addr) (*((addr) + 6))
#define ACCMD_AUX_PROCESSOR_LOAD_REQ_FILE_INDEX_SET(addr, file_index) (*((addr) + 6) = (uint16)(file_index))
#define ACCMD_AUX_PROCESSOR_LOAD_REQ_WORD_SIZE (7)
#define ACCMD_AUX_PROCESSOR_LOAD_REQ_PACK(addr, Processor_Id, memory_type, framework_type, file_index) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Processor_Id)); \
        *((addr) + 4) = (uint16)((uint16)(memory_type)); \
        *((addr) + 5) = (uint16)((uint16)(framework_type)); \
        *((addr) + 6) = (uint16)((uint16)(file_index)); \
    } while (0)

#define ACCMD_AUX_PROCESSOR_LOAD_REQ_MARSHALL(addr, accmd_aux_processor_load_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_aux_processor_load_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_aux_processor_load_req_ptr)->Processor_Id); \
        *((addr) + 4) = (uint16)((accmd_aux_processor_load_req_ptr)->memory_type); \
        *((addr) + 5) = (uint16)((accmd_aux_processor_load_req_ptr)->framework_type); \
        *((addr) + 6) = (uint16)((accmd_aux_processor_load_req_ptr)->file_index); \
    } while (0)

#define ACCMD_AUX_PROCESSOR_LOAD_REQ_UNMARSHALL(addr, accmd_aux_processor_load_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_aux_processor_load_req_ptr)->header)); \
        (accmd_aux_processor_load_req_ptr)->Processor_Id = ACCMD_AUX_PROCESSOR_LOAD_REQ_PROCESSOR_ID_GET(addr); \
        (accmd_aux_processor_load_req_ptr)->memory_type = ACCMD_AUX_PROCESSOR_LOAD_REQ_MEMORY_TYPE_GET(addr); \
        (accmd_aux_processor_load_req_ptr)->framework_type = ACCMD_AUX_PROCESSOR_LOAD_REQ_FRAMEWORK_TYPE_GET(addr); \
        (accmd_aux_processor_load_req_ptr)->file_index = ACCMD_AUX_PROCESSOR_LOAD_REQ_FILE_INDEX_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Aux_Processor_Start_Req

  DESCRIPTION
    Start secondary processor.

  MEMBERS
    Processor_Id - Secondary processor ID.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Processor_Id;
} ACCMD_AUX_PROCESSOR_START_REQ;

/* The following macros take ACCMD_AUX_PROCESSOR_START_REQ *accmd_aux_processor_start_req_ptr or uint16 *addr */
#define ACCMD_AUX_PROCESSOR_START_REQ_PROCESSOR_ID_WORD_OFFSET (3)
#define ACCMD_AUX_PROCESSOR_START_REQ_PROCESSOR_ID_GET(addr) (*((addr) + 3))
#define ACCMD_AUX_PROCESSOR_START_REQ_PROCESSOR_ID_SET(addr, processor_id) (*((addr) + 3) = (uint16)(processor_id))
#define ACCMD_AUX_PROCESSOR_START_REQ_WORD_SIZE (4)
#define ACCMD_AUX_PROCESSOR_START_REQ_PACK(addr, Processor_Id) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Processor_Id)); \
    } while (0)

#define ACCMD_AUX_PROCESSOR_START_REQ_MARSHALL(addr, accmd_aux_processor_start_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_aux_processor_start_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_aux_processor_start_req_ptr)->Processor_Id); \
    } while (0)

#define ACCMD_AUX_PROCESSOR_START_REQ_UNMARSHALL(addr, accmd_aux_processor_start_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_aux_processor_start_req_ptr)->header)); \
        (accmd_aux_processor_start_req_ptr)->Processor_Id = ACCMD_AUX_PROCESSOR_START_REQ_PROCESSOR_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Aux_Processor_Stop_Req

  DESCRIPTION
    Stop secondary processor.

  MEMBERS
    Processor_Id - Secondary processor ID.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Processor_Id;
} ACCMD_AUX_PROCESSOR_STOP_REQ;

/* The following macros take ACCMD_AUX_PROCESSOR_STOP_REQ *accmd_aux_processor_stop_req_ptr or uint16 *addr */
#define ACCMD_AUX_PROCESSOR_STOP_REQ_PROCESSOR_ID_WORD_OFFSET (3)
#define ACCMD_AUX_PROCESSOR_STOP_REQ_PROCESSOR_ID_GET(addr) (*((addr) + 3))
#define ACCMD_AUX_PROCESSOR_STOP_REQ_PROCESSOR_ID_SET(addr, processor_id) (*((addr) + 3) = (uint16)(processor_id))
#define ACCMD_AUX_PROCESSOR_STOP_REQ_WORD_SIZE (4)
#define ACCMD_AUX_PROCESSOR_STOP_REQ_PACK(addr, Processor_Id) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Processor_Id)); \
    } while (0)

#define ACCMD_AUX_PROCESSOR_STOP_REQ_MARSHALL(addr, accmd_aux_processor_stop_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_aux_processor_stop_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_aux_processor_stop_req_ptr)->Processor_Id); \
    } while (0)

#define ACCMD_AUX_PROCESSOR_STOP_REQ_UNMARSHALL(addr, accmd_aux_processor_stop_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_aux_processor_stop_req_ptr)->header)); \
        (accmd_aux_processor_stop_req_ptr)->Processor_Id = ACCMD_AUX_PROCESSOR_STOP_REQ_PROCESSOR_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Mclk_Activate_Req

  DESCRIPTION
    From Client This command allows to enable an MCLK output for an audio
    interface.

  MEMBERS
    Sid      - The Sid parameter specifies the endpoint(Source or Sink) ID for
               which we want to enable the MCLK
    Activate - whether to activate or de-activate the MCLK output
    Enable   - Only Makes MCLK available, so interface can use it, if Activate
               is TRUE, then this flag will be ignored. Eventually this flag
               will be deprecated, since we need to always make sure clock is
               available before any use.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Sid;
    uint16       Activate;
    uint16       Enable;
} ACCMD_STREAM_MCLK_ACTIVATE_REQ;

/* The following macros take ACCMD_STREAM_MCLK_ACTIVATE_REQ *accmd_stream_mclk_activate_req_ptr or uint16 *addr */
#define ACCMD_STREAM_MCLK_ACTIVATE_REQ_SID_WORD_OFFSET (3)
#define ACCMD_STREAM_MCLK_ACTIVATE_REQ_SID_GET(addr) (*((addr) + 3))
#define ACCMD_STREAM_MCLK_ACTIVATE_REQ_SID_SET(addr, sid) (*((addr) + 3) = (uint16)(sid))
#define ACCMD_STREAM_MCLK_ACTIVATE_REQ_ACTIVATE_WORD_OFFSET (4)
#define ACCMD_STREAM_MCLK_ACTIVATE_REQ_ACTIVATE_GET(addr) (*((addr) + 4))
#define ACCMD_STREAM_MCLK_ACTIVATE_REQ_ACTIVATE_SET(addr, activate) (*((addr) + 4) = (uint16)(activate))
#define ACCMD_STREAM_MCLK_ACTIVATE_REQ_ENABLE_WORD_OFFSET (5)
#define ACCMD_STREAM_MCLK_ACTIVATE_REQ_ENABLE_GET(addr) (*((addr) + 5))
#define ACCMD_STREAM_MCLK_ACTIVATE_REQ_ENABLE_SET(addr, enable) (*((addr) + 5) = (uint16)(enable))
#define ACCMD_STREAM_MCLK_ACTIVATE_REQ_WORD_SIZE (6)
#define ACCMD_STREAM_MCLK_ACTIVATE_REQ_PACK(addr, Sid, Activate, Enable) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Sid)); \
        *((addr) + 4) = (uint16)((uint16)(Activate)); \
        *((addr) + 5) = (uint16)((uint16)(Enable)); \
    } while (0)

#define ACCMD_STREAM_MCLK_ACTIVATE_REQ_MARSHALL(addr, accmd_stream_mclk_activate_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_mclk_activate_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_mclk_activate_req_ptr)->Sid); \
        *((addr) + 4) = (uint16)((accmd_stream_mclk_activate_req_ptr)->Activate); \
        *((addr) + 5) = (uint16)((accmd_stream_mclk_activate_req_ptr)->Enable); \
    } while (0)

#define ACCMD_STREAM_MCLK_ACTIVATE_REQ_UNMARSHALL(addr, accmd_stream_mclk_activate_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_mclk_activate_req_ptr)->header)); \
        (accmd_stream_mclk_activate_req_ptr)->Sid = ACCMD_STREAM_MCLK_ACTIVATE_REQ_SID_GET(addr); \
        (accmd_stream_mclk_activate_req_ptr)->Activate = ACCMD_STREAM_MCLK_ACTIVATE_REQ_ACTIVATE_GET(addr); \
        (accmd_stream_mclk_activate_req_ptr)->Enable = ACCMD_STREAM_MCLK_ACTIVATE_REQ_ENABLE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Set_Mclk_Source_Type_Req

  DESCRIPTION
    From Client This command allows the user to choose between internal MCLK
    from MPLL or external MCLK from a PIO. By default internal MCLK will be
    used, so this command should be needed only when external MCLK should be
    used. This command can only be sent when no interface is using MCLK,
    trying to change the MCLK source type while it is being used will fail.

  MEMBERS
    Use_External            - if enabled it means that interfaces will use
                              external MCLK when they need an MCLK clock,
                              otherwise they will use MCLK from internal MPLL.
    External_Mclk_Frequency - when setting for external MCLK source type, user
                              needs to provide the actual frequency of the clock
                              source. This field will be ignored when setting
                              for internal mode

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Use_External;
    uint32       External_Mclk_Frequency;
} ACCMD_SET_MCLK_SOURCE_TYPE_REQ;

/* The following macros take ACCMD_SET_MCLK_SOURCE_TYPE_REQ *accmd_set_mclk_source_type_req_ptr or uint16 *addr */
#define ACCMD_SET_MCLK_SOURCE_TYPE_REQ_USE_EXTERNAL_WORD_OFFSET (3)
#define ACCMD_SET_MCLK_SOURCE_TYPE_REQ_USE_EXTERNAL_GET(addr) (*((addr) + 3))
#define ACCMD_SET_MCLK_SOURCE_TYPE_REQ_USE_EXTERNAL_SET(addr, use_external) (*((addr) + 3) = (uint16)(use_external))
#define ACCMD_SET_MCLK_SOURCE_TYPE_REQ_EXTERNAL_MCLK_FREQUENCY_WORD_OFFSET (4)
#define ACCMD_SET_MCLK_SOURCE_TYPE_REQ_EXTERNAL_MCLK_FREQUENCY_GET(addr)  \
    (((uint32)(*((addr) + 4)) | \
      ((uint32)(*((addr) + 5)) << 16)))
#define ACCMD_SET_MCLK_SOURCE_TYPE_REQ_EXTERNAL_MCLK_FREQUENCY_SET(addr, external_mclk_frequency) do { \
        *((addr) + 4) = (uint16)((external_mclk_frequency) & 0xffff); \
        *((addr) + 5) = (uint16)((external_mclk_frequency) >> 16); } while (0)
#define ACCMD_SET_MCLK_SOURCE_TYPE_REQ_WORD_SIZE (6)
#define ACCMD_SET_MCLK_SOURCE_TYPE_REQ_PACK(addr, Use_External, External_Mclk_Frequency) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Use_External)); \
        *((addr) + 4) = (uint16)((uint16)((External_Mclk_Frequency) & 0xffff)); \
        *((addr) + 5) = (uint16)(((External_Mclk_Frequency) >> 16)); \
    } while (0)

#define ACCMD_SET_MCLK_SOURCE_TYPE_REQ_MARSHALL(addr, accmd_set_mclk_source_type_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_set_mclk_source_type_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_set_mclk_source_type_req_ptr)->Use_External); \
        *((addr) + 4) = (uint16)(((accmd_set_mclk_source_type_req_ptr)->External_Mclk_Frequency) & 0xffff); \
        *((addr) + 5) = (uint16)(((accmd_set_mclk_source_type_req_ptr)->External_Mclk_Frequency) >> 16); \
    } while (0)

#define ACCMD_SET_MCLK_SOURCE_TYPE_REQ_UNMARSHALL(addr, accmd_set_mclk_source_type_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_set_mclk_source_type_req_ptr)->header)); \
        (accmd_set_mclk_source_type_req_ptr)->Use_External = ACCMD_SET_MCLK_SOURCE_TYPE_REQ_USE_EXTERNAL_GET(addr); \
        (accmd_set_mclk_source_type_req_ptr)->External_Mclk_Frequency = ACCMD_SET_MCLK_SOURCE_TYPE_REQ_EXTERNAL_MCLK_FREQUENCY_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_Operators_Req

  DESCRIPTION
    From Client This command requests a list of operator IDs instanciated in
    the system.

  MEMBERS
    filter      - If set to 0, all the operators instanciated in the system will
                  be returned. Otherwise, only the id for the operators with a
                  capability id matching this field will be returned.
    start_index - Number of operators to skip from the result of the search.
                  This is used to work around limitations of packet sizes in the
                  underlying transport.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       filter;
    uint16       start_index;
} ACCMD_GET_OPERATORS_REQ;

/* The following macros take ACCMD_GET_OPERATORS_REQ *accmd_get_operators_req_ptr or uint16 *addr */
#define ACCMD_GET_OPERATORS_REQ_FILTER_WORD_OFFSET (3)
#define ACCMD_GET_OPERATORS_REQ_FILTER_GET(addr) (*((addr) + 3))
#define ACCMD_GET_OPERATORS_REQ_FILTER_SET(addr, filter) (*((addr) + 3) = (uint16)(filter))
#define ACCMD_GET_OPERATORS_REQ_START_INDEX_WORD_OFFSET (4)
#define ACCMD_GET_OPERATORS_REQ_START_INDEX_GET(addr) (*((addr) + 4))
#define ACCMD_GET_OPERATORS_REQ_START_INDEX_SET(addr, start_index) (*((addr) + 4) = (uint16)(start_index))
#define ACCMD_GET_OPERATORS_REQ_WORD_SIZE (5)
#define ACCMD_GET_OPERATORS_REQ_PACK(addr, filter, start_index) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(filter)); \
        *((addr) + 4) = (uint16)((uint16)(start_index)); \
    } while (0)

#define ACCMD_GET_OPERATORS_REQ_MARSHALL(addr, accmd_get_operators_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_get_operators_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_get_operators_req_ptr)->filter); \
        *((addr) + 4) = (uint16)((accmd_get_operators_req_ptr)->start_index); \
    } while (0)

#define ACCMD_GET_OPERATORS_REQ_UNMARSHALL(addr, accmd_get_operators_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_get_operators_req_ptr)->header)); \
        (accmd_get_operators_req_ptr)->filter = ACCMD_GET_OPERATORS_REQ_FILTER_GET(addr); \
        (accmd_get_operators_req_ptr)->start_index = ACCMD_GET_OPERATORS_REQ_START_INDEX_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_PS_Shutdown_Req

  DESCRIPTION
    From Client Indicates that the client wants to shut down the ACCMD
    connection. The AudioSS will respond by doing any PS_Writes it needs to
    and then sending PS_Shutdown_Done_Req. (The Standard_Resp response to
    _this_ ACCMD is just an acknowledgement and doesn't indicate that
    shutdown can proceed.) no fields

  MEMBERS

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
} ACCMD_PS_SHUTDOWN_REQ;

/* The following macros take ACCMD_PS_SHUTDOWN_REQ *accmd_ps_shutdown_req_ptr or uint16 *addr */
#define ACCMD_PS_SHUTDOWN_REQ_WORD_SIZE (3)


/*******************************************************************************

  NAME
    Accmd_PS_Read_Req

  DESCRIPTION
    From AudioSS Request the content of the key identified uniquely by Key_Id
    to be sent to the audio subsystem. Offset indicate the number of words of
    16 bits to skip at the beginning of the response.

  MEMBERS
    Key_Id - The 24-bit key ID padded to 32 bits.
    Offset -

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER          header;
    ACCMD_PERSIST_ID_TYPE Key_Id;
    uint16                Offset;
} ACCMD_PS_READ_REQ;

/* The following macros take ACCMD_PS_READ_REQ *accmd_ps_read_req_ptr or uint16 *addr */
#define ACCMD_PS_READ_REQ_KEY_ID_WORD_OFFSET (3)
#define ACCMD_PS_READ_REQ_KEY_ID_GET(addr)  \
    ((ACCMD_PERSIST_ID_TYPE)((uint32)(*((addr) + 3)) | \
                             ((uint32)(*((addr) + 4)) << 16)))
#define ACCMD_PS_READ_REQ_KEY_ID_SET(addr, key_id) do { \
        *((addr) + 3) = (uint16)((key_id) & 0xffff); \
        *((addr) + 4) = (uint16)((key_id) >> 16); } while (0)
#define ACCMD_PS_READ_REQ_OFFSET_WORD_OFFSET (5)
#define ACCMD_PS_READ_REQ_OFFSET_GET(addr) (*((addr) + 5))
#define ACCMD_PS_READ_REQ_OFFSET_SET(addr, offset) (*((addr) + 5) = (uint16)(offset))
#define ACCMD_PS_READ_REQ_WORD_SIZE (6)
#define ACCMD_PS_READ_REQ_PACK(addr, Key_Id, Offset) \
    do { \
        *((addr) + 3) = (uint16)((uint16)((Key_Id) & 0xffff)); \
        *((addr) + 4) = (uint16)(((Key_Id) >> 16)); \
        *((addr) + 5) = (uint16)((uint16)(Offset)); \
    } while (0)

#define ACCMD_PS_READ_REQ_MARSHALL(addr, accmd_ps_read_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_ps_read_req_ptr)->header)); \
        *((addr) + 3) = (uint16)(((accmd_ps_read_req_ptr)->Key_Id) & 0xffff); \
        *((addr) + 4) = (uint16)(((accmd_ps_read_req_ptr)->Key_Id) >> 16); \
        *((addr) + 5) = (uint16)((accmd_ps_read_req_ptr)->Offset); \
    } while (0)

#define ACCMD_PS_READ_REQ_UNMARSHALL(addr, accmd_ps_read_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_ps_read_req_ptr)->header)); \
        (accmd_ps_read_req_ptr)->Key_Id = ACCMD_PS_READ_REQ_KEY_ID_GET(addr); \
        (accmd_ps_read_req_ptr)->Offset = ACCMD_PS_READ_REQ_OFFSET_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_PS_Write_Req

  DESCRIPTION
    From AudioSS Initiate or continue writing a value to persistent store.
    Because a write may be fragmented across multiple ACCMDs, this protocol
    is stateful for both parties. The requester should write all chunks of
    the key in ascending order of offset with no gaps or other interleaved
    ACCMDs.

  MEMBERS
    Key_Id    - The 24-bit key ID, padded to 32 bits.
    Total_Len - The length of the entire key, in uint16s (regardless of any
                offset).
    Offset    -
    data      - the actual data; length implicit in ACCMD payload length

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER          header;
    ACCMD_PERSIST_ID_TYPE Key_Id;
    uint16                Total_Len;
    uint16                Offset;
    uint16                data[ACCMD_PRIM_ANY_SIZE];
} ACCMD_PS_WRITE_REQ;

/* The following macros take ACCMD_PS_WRITE_REQ *accmd_ps_write_req_ptr or uint16 *addr */
#define ACCMD_PS_WRITE_REQ_KEY_ID_WORD_OFFSET (3)
#define ACCMD_PS_WRITE_REQ_KEY_ID_GET(addr)  \
    ((ACCMD_PERSIST_ID_TYPE)((uint32)(*((addr) + 3)) | \
                             ((uint32)(*((addr) + 4)) << 16)))
#define ACCMD_PS_WRITE_REQ_KEY_ID_SET(addr, key_id) do { \
        *((addr) + 3) = (uint16)((key_id) & 0xffff); \
        *((addr) + 4) = (uint16)((key_id) >> 16); } while (0)
#define ACCMD_PS_WRITE_REQ_TOTAL_LEN_WORD_OFFSET (5)
#define ACCMD_PS_WRITE_REQ_TOTAL_LEN_GET(addr) (*((addr) + 5))
#define ACCMD_PS_WRITE_REQ_TOTAL_LEN_SET(addr, total_len) (*((addr) + 5) = (uint16)(total_len))
#define ACCMD_PS_WRITE_REQ_OFFSET_WORD_OFFSET (6)
#define ACCMD_PS_WRITE_REQ_OFFSET_GET(addr) (*((addr) + 6))
#define ACCMD_PS_WRITE_REQ_OFFSET_SET(addr, offset) (*((addr) + 6) = (uint16)(offset))
#define ACCMD_PS_WRITE_REQ_DATA_WORD_OFFSET (7)
#define ACCMD_PS_WRITE_REQ_DATA_GET(addr) (*((addr) + 7))
#define ACCMD_PS_WRITE_REQ_DATA_SET(addr, data) (*((addr) + 7) = (uint16)(data))
#define ACCMD_PS_WRITE_REQ_WORD_SIZE (8)
#define ACCMD_PS_WRITE_REQ_PACK(addr, Key_Id, Total_Len, Offset, data) \
    do { \
        *((addr) + 3) = (uint16)((uint16)((Key_Id) & 0xffff)); \
        *((addr) + 4) = (uint16)(((Key_Id) >> 16)); \
        *((addr) + 5) = (uint16)((uint16)(Total_Len)); \
        *((addr) + 6) = (uint16)((uint16)(Offset)); \
        *((addr) + 7) = (uint16)((uint16)(data)); \
    } while (0)

#define ACCMD_PS_WRITE_REQ_MARSHALL(addr, accmd_ps_write_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_ps_write_req_ptr)->header)); \
        *((addr) + 3) = (uint16)(((accmd_ps_write_req_ptr)->Key_Id) & 0xffff); \
        *((addr) + 4) = (uint16)(((accmd_ps_write_req_ptr)->Key_Id) >> 16); \
        *((addr) + 5) = (uint16)((accmd_ps_write_req_ptr)->Total_Len); \
        *((addr) + 6) = (uint16)((accmd_ps_write_req_ptr)->Offset); \
        *((addr) + 7) = (uint16)((accmd_ps_write_req_ptr)->data); \
    } while (0)

#define ACCMD_PS_WRITE_REQ_UNMARSHALL(addr, accmd_ps_write_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_ps_write_req_ptr)->header)); \
        (accmd_ps_write_req_ptr)->Key_Id = ACCMD_PS_WRITE_REQ_KEY_ID_GET(addr); \
        (accmd_ps_write_req_ptr)->Total_Len = ACCMD_PS_WRITE_REQ_TOTAL_LEN_GET(addr); \
        (accmd_ps_write_req_ptr)->Offset = ACCMD_PS_WRITE_REQ_OFFSET_GET(addr); \
        (accmd_ps_write_req_ptr)->data = ACCMD_PS_WRITE_REQ_DATA_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_PS_Delete_Req

  DESCRIPTION
    From AudioSS Request the content of the key identified uniquely by Key_Id
    to delete from storage.

  MEMBERS
    Key_Id - The 24-bit key ID, padded to 32 bits.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER          header;
    ACCMD_PERSIST_ID_TYPE Key_Id;
} ACCMD_PS_DELETE_REQ;

/* The following macros take ACCMD_PS_DELETE_REQ *accmd_ps_delete_req_ptr or uint16 *addr */
#define ACCMD_PS_DELETE_REQ_KEY_ID_WORD_OFFSET (3)
#define ACCMD_PS_DELETE_REQ_KEY_ID_GET(addr)  \
    ((ACCMD_PERSIST_ID_TYPE)((uint32)(*((addr) + 3)) | \
                             ((uint32)(*((addr) + 4)) << 16)))
#define ACCMD_PS_DELETE_REQ_KEY_ID_SET(addr, key_id) do { \
        *((addr) + 3) = (uint16)((key_id) & 0xffff); \
        *((addr) + 4) = (uint16)((key_id) >> 16); } while (0)
#define ACCMD_PS_DELETE_REQ_WORD_SIZE (5)
#define ACCMD_PS_DELETE_REQ_PACK(addr, Key_Id) \
    do { \
        *((addr) + 3) = (uint16)((uint16)((Key_Id) & 0xffff)); \
        *((addr) + 4) = (uint16)(((Key_Id) >> 16)); \
    } while (0)

#define ACCMD_PS_DELETE_REQ_MARSHALL(addr, accmd_ps_delete_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_ps_delete_req_ptr)->header)); \
        *((addr) + 3) = (uint16)(((accmd_ps_delete_req_ptr)->Key_Id) & 0xffff); \
        *((addr) + 4) = (uint16)(((accmd_ps_delete_req_ptr)->Key_Id) >> 16); \
    } while (0)

#define ACCMD_PS_DELETE_REQ_UNMARSHALL(addr, accmd_ps_delete_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_ps_delete_req_ptr)->header)); \
        (accmd_ps_delete_req_ptr)->Key_Id = ACCMD_PS_DELETE_REQ_KEY_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_PS_Shutdown_Complete_Req

  DESCRIPTION
    From AudioSS Indicates that the audio subsystem has flush the content of
    its cache. Client answers with a standard response. no fields

  MEMBERS

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
} ACCMD_PS_SHUTDOWN_COMPLETE_REQ;

/* The following macros take ACCMD_PS_SHUTDOWN_COMPLETE_REQ *accmd_ps_shutdown_complete_req_ptr or uint16 *addr */
#define ACCMD_PS_SHUTDOWN_COMPLETE_REQ_WORD_SIZE (3)


/*******************************************************************************

  NAME
    Accmd_PS_Register_Req

  DESCRIPTION
    From Client Notify the Persistent Store (PS) task to send PS related
    messages on the connection used by this message. This message should be
    sent before sending any OPMSG_COMMON_ID_SET_UCID operator message. no
    fields

  MEMBERS

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
} ACCMD_PS_REGISTER_REQ;

/* The following macros take ACCMD_PS_REGISTER_REQ *accmd_ps_register_req_ptr or uint16 *addr */
#define ACCMD_PS_REGISTER_REQ_WORD_SIZE (3)


/*******************************************************************************

  NAME
    Accmd_clock_configure_Req

  DESCRIPTION
    This command allow the APP to register its clock preference for the
    various operation modes.

  MEMBERS
    low_power_mode_clk - Operation Mode
    wake_on_mode_clk   - CPU clock variant
    active_mode_clk    - Kcodec clock variant

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER  header;
    ACCMD_CPU_CLK low_power_mode_clk;
    ACCMD_CPU_CLK wake_on_mode_clk;
    ACCMD_CPU_CLK active_mode_clk;
} ACCMD_CLOCK_CONFIGURE_REQ;

/* The following macros take ACCMD_CLOCK_CONFIGURE_REQ *accmd_clock_configure_req_ptr or uint16 *addr */
#define ACCMD_CLOCK_CONFIGURE_REQ_LOW_POWER_MODE_CLK_WORD_OFFSET (3)
#define ACCMD_CLOCK_CONFIGURE_REQ_LOW_POWER_MODE_CLK_GET(addr) ((ACCMD_CPU_CLK)*((addr) + 3))
#define ACCMD_CLOCK_CONFIGURE_REQ_LOW_POWER_MODE_CLK_SET(addr, low_power_mode_clk) (*((addr) + 3) = (uint16)(low_power_mode_clk))
#define ACCMD_CLOCK_CONFIGURE_REQ_WAKE_ON_MODE_CLK_WORD_OFFSET (4)
#define ACCMD_CLOCK_CONFIGURE_REQ_WAKE_ON_MODE_CLK_GET(addr) ((ACCMD_CPU_CLK)*((addr) + 4))
#define ACCMD_CLOCK_CONFIGURE_REQ_WAKE_ON_MODE_CLK_SET(addr, wake_on_mode_clk) (*((addr) + 4) = (uint16)(wake_on_mode_clk))
#define ACCMD_CLOCK_CONFIGURE_REQ_ACTIVE_MODE_CLK_WORD_OFFSET (5)
#define ACCMD_CLOCK_CONFIGURE_REQ_ACTIVE_MODE_CLK_GET(addr) ((ACCMD_CPU_CLK)*((addr) + 5))
#define ACCMD_CLOCK_CONFIGURE_REQ_ACTIVE_MODE_CLK_SET(addr, active_mode_clk) (*((addr) + 5) = (uint16)(active_mode_clk))
#define ACCMD_CLOCK_CONFIGURE_REQ_WORD_SIZE (6)
#define ACCMD_CLOCK_CONFIGURE_REQ_PACK(addr, low_power_mode_clk, wake_on_mode_clk, active_mode_clk) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(low_power_mode_clk)); \
        *((addr) + 4) = (uint16)((uint16)(wake_on_mode_clk)); \
        *((addr) + 5) = (uint16)((uint16)(active_mode_clk)); \
    } while (0)

#define ACCMD_CLOCK_CONFIGURE_REQ_MARSHALL(addr, accmd_clock_configure_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_clock_configure_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_clock_configure_req_ptr)->low_power_mode_clk); \
        *((addr) + 4) = (uint16)((accmd_clock_configure_req_ptr)->wake_on_mode_clk); \
        *((addr) + 5) = (uint16)((accmd_clock_configure_req_ptr)->active_mode_clk); \
    } while (0)

#define ACCMD_CLOCK_CONFIGURE_REQ_UNMARSHALL(addr, accmd_clock_configure_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_clock_configure_req_ptr)->header)); \
        (accmd_clock_configure_req_ptr)->low_power_mode_clk = ACCMD_CLOCK_CONFIGURE_REQ_LOW_POWER_MODE_CLK_GET(addr); \
        (accmd_clock_configure_req_ptr)->wake_on_mode_clk = ACCMD_CLOCK_CONFIGURE_REQ_WAKE_ON_MODE_CLK_GET(addr); \
        (accmd_clock_configure_req_ptr)->active_mode_clk = ACCMD_CLOCK_CONFIGURE_REQ_ACTIVE_MODE_CLK_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Set_Anc_Iir_Filter_Req

  DESCRIPTION
    From Client This command allows configuration of the ANC IIR filter for
    each ANC channel instance.

  MEMBERS
    anc_instance - The anc_instance parameter is an ID that identifies the ANC
                   channel instance to which the configuration is to be applied
                   (ANC0/ANC1).
    path_id      - The path_id parameter is an ID that specifies the ANC path
                   and hence the IIR filter instance to be configured ().
    num_coeffs   - The num_coeffs parameter specifies the number of IIR
                   coefficients to be configured ().
    coeffs       - An array of filter coefficients; length implicit in ACCMD
                   payload length.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER       header;
    ACCMD_ANC_INSTANCE anc_instance;
    ACCMD_ANC_PATH     path_id;
    uint16             num_coeffs;
    uint16             coeffs[ACCMD_PRIM_ANY_SIZE];
} ACCMD_SET_ANC_IIR_FILTER_REQ;

/* The following macros take ACCMD_SET_ANC_IIR_FILTER_REQ *accmd_set_anc_iir_filter_req_ptr or uint16 *addr */
#define ACCMD_SET_ANC_IIR_FILTER_REQ_ANC_INSTANCE_WORD_OFFSET (3)
#define ACCMD_SET_ANC_IIR_FILTER_REQ_ANC_INSTANCE_GET(addr) ((ACCMD_ANC_INSTANCE)*((addr) + 3))
#define ACCMD_SET_ANC_IIR_FILTER_REQ_ANC_INSTANCE_SET(addr, anc_instance) (*((addr) + 3) = (uint16)(anc_instance))
#define ACCMD_SET_ANC_IIR_FILTER_REQ_PATH_ID_WORD_OFFSET (4)
#define ACCMD_SET_ANC_IIR_FILTER_REQ_PATH_ID_GET(addr) ((ACCMD_ANC_PATH)*((addr) + 4))
#define ACCMD_SET_ANC_IIR_FILTER_REQ_PATH_ID_SET(addr, path_id) (*((addr) + 4) = (uint16)(path_id))
#define ACCMD_SET_ANC_IIR_FILTER_REQ_NUM_COEFFS_WORD_OFFSET (5)
#define ACCMD_SET_ANC_IIR_FILTER_REQ_NUM_COEFFS_GET(addr) (*((addr) + 5))
#define ACCMD_SET_ANC_IIR_FILTER_REQ_NUM_COEFFS_SET(addr, num_coeffs) (*((addr) + 5) = (uint16)(num_coeffs))
#define ACCMD_SET_ANC_IIR_FILTER_REQ_COEFFS_WORD_OFFSET (6)
#define ACCMD_SET_ANC_IIR_FILTER_REQ_COEFFS_GET(addr) (*((addr) + 6))
#define ACCMD_SET_ANC_IIR_FILTER_REQ_COEFFS_SET(addr, coeffs) (*((addr) + 6) = (uint16)(coeffs))
#define ACCMD_SET_ANC_IIR_FILTER_REQ_WORD_SIZE (7)
#define ACCMD_SET_ANC_IIR_FILTER_REQ_PACK(addr, anc_instance, path_id, num_coeffs, coeffs) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(anc_instance)); \
        *((addr) + 4) = (uint16)((uint16)(path_id)); \
        *((addr) + 5) = (uint16)((uint16)(num_coeffs)); \
        *((addr) + 6) = (uint16)((uint16)(coeffs)); \
    } while (0)

#define ACCMD_SET_ANC_IIR_FILTER_REQ_MARSHALL(addr, accmd_set_anc_iir_filter_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_set_anc_iir_filter_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_set_anc_iir_filter_req_ptr)->anc_instance); \
        *((addr) + 4) = (uint16)((accmd_set_anc_iir_filter_req_ptr)->path_id); \
        *((addr) + 5) = (uint16)((accmd_set_anc_iir_filter_req_ptr)->num_coeffs); \
        *((addr) + 6) = (uint16)((accmd_set_anc_iir_filter_req_ptr)->coeffs); \
    } while (0)

#define ACCMD_SET_ANC_IIR_FILTER_REQ_UNMARSHALL(addr, accmd_set_anc_iir_filter_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_set_anc_iir_filter_req_ptr)->header)); \
        (accmd_set_anc_iir_filter_req_ptr)->anc_instance = ACCMD_SET_ANC_IIR_FILTER_REQ_ANC_INSTANCE_GET(addr); \
        (accmd_set_anc_iir_filter_req_ptr)->path_id = ACCMD_SET_ANC_IIR_FILTER_REQ_PATH_ID_GET(addr); \
        (accmd_set_anc_iir_filter_req_ptr)->num_coeffs = ACCMD_SET_ANC_IIR_FILTER_REQ_NUM_COEFFS_GET(addr); \
        (accmd_set_anc_iir_filter_req_ptr)->coeffs = ACCMD_SET_ANC_IIR_FILTER_REQ_COEFFS_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Set_Anc_Lpf_Filter_Req

  DESCRIPTION
    From Client This command allows configuration of the ANC LPF filter for
    each ANC channel instance.

  MEMBERS
    anc_instance - The anc_instance parameter is an ID that identifies the ANC
                   channel instance to which the configuration is to be applied
                   (ANC0/ANC1).
    path_id      - The path_id parameter is an ID that specifies the ANC path
                   and hence the LPF filter instance to be configured ().
    shift1       - The shift1 parameter is the used to derive the first LPF
                   coefficient.
    shift2       - The shift2 parameter is the used to derive the second LPF
                   coefficient.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER       header;
    ACCMD_ANC_INSTANCE anc_instance;
    ACCMD_ANC_PATH     path_id;
    uint16             shift1;
    uint16             shift2;
} ACCMD_SET_ANC_LPF_FILTER_REQ;

/* The following macros take ACCMD_SET_ANC_LPF_FILTER_REQ *accmd_set_anc_lpf_filter_req_ptr or uint16 *addr */
#define ACCMD_SET_ANC_LPF_FILTER_REQ_ANC_INSTANCE_WORD_OFFSET (3)
#define ACCMD_SET_ANC_LPF_FILTER_REQ_ANC_INSTANCE_GET(addr) ((ACCMD_ANC_INSTANCE)*((addr) + 3))
#define ACCMD_SET_ANC_LPF_FILTER_REQ_ANC_INSTANCE_SET(addr, anc_instance) (*((addr) + 3) = (uint16)(anc_instance))
#define ACCMD_SET_ANC_LPF_FILTER_REQ_PATH_ID_WORD_OFFSET (4)
#define ACCMD_SET_ANC_LPF_FILTER_REQ_PATH_ID_GET(addr) ((ACCMD_ANC_PATH)*((addr) + 4))
#define ACCMD_SET_ANC_LPF_FILTER_REQ_PATH_ID_SET(addr, path_id) (*((addr) + 4) = (uint16)(path_id))
#define ACCMD_SET_ANC_LPF_FILTER_REQ_SHIFT1_WORD_OFFSET (5)
#define ACCMD_SET_ANC_LPF_FILTER_REQ_SHIFT1_GET(addr) (*((addr) + 5))
#define ACCMD_SET_ANC_LPF_FILTER_REQ_SHIFT1_SET(addr, shift1) (*((addr) + 5) = (uint16)(shift1))
#define ACCMD_SET_ANC_LPF_FILTER_REQ_SHIFT2_WORD_OFFSET (6)
#define ACCMD_SET_ANC_LPF_FILTER_REQ_SHIFT2_GET(addr) (*((addr) + 6))
#define ACCMD_SET_ANC_LPF_FILTER_REQ_SHIFT2_SET(addr, shift2) (*((addr) + 6) = (uint16)(shift2))
#define ACCMD_SET_ANC_LPF_FILTER_REQ_WORD_SIZE (7)
#define ACCMD_SET_ANC_LPF_FILTER_REQ_PACK(addr, anc_instance, path_id, shift1, shift2) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(anc_instance)); \
        *((addr) + 4) = (uint16)((uint16)(path_id)); \
        *((addr) + 5) = (uint16)((uint16)(shift1)); \
        *((addr) + 6) = (uint16)((uint16)(shift2)); \
    } while (0)

#define ACCMD_SET_ANC_LPF_FILTER_REQ_MARSHALL(addr, accmd_set_anc_lpf_filter_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_set_anc_lpf_filter_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_set_anc_lpf_filter_req_ptr)->anc_instance); \
        *((addr) + 4) = (uint16)((accmd_set_anc_lpf_filter_req_ptr)->path_id); \
        *((addr) + 5) = (uint16)((accmd_set_anc_lpf_filter_req_ptr)->shift1); \
        *((addr) + 6) = (uint16)((accmd_set_anc_lpf_filter_req_ptr)->shift2); \
    } while (0)

#define ACCMD_SET_ANC_LPF_FILTER_REQ_UNMARSHALL(addr, accmd_set_anc_lpf_filter_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_set_anc_lpf_filter_req_ptr)->header)); \
        (accmd_set_anc_lpf_filter_req_ptr)->anc_instance = ACCMD_SET_ANC_LPF_FILTER_REQ_ANC_INSTANCE_GET(addr); \
        (accmd_set_anc_lpf_filter_req_ptr)->path_id = ACCMD_SET_ANC_LPF_FILTER_REQ_PATH_ID_GET(addr); \
        (accmd_set_anc_lpf_filter_req_ptr)->shift1 = ACCMD_SET_ANC_LPF_FILTER_REQ_SHIFT1_GET(addr); \
        (accmd_set_anc_lpf_filter_req_ptr)->shift2 = ACCMD_SET_ANC_LPF_FILTER_REQ_SHIFT2_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Anc_Enable_Req

  DESCRIPTION
    From Client This command allows controls the enabling/disabling of ANC.

  MEMBERS
    anc_enable_0 - The anc_enable_0 parameter is a bit field that enables the
                   ANC input and output paths of the ANC0 instance.
    anc_enable_1 - The anc_enable_1 parameter is a bit field that enables the
                   ANC input and output paths of the ANC1 instance.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       anc_enable_0;
    uint16       anc_enable_1;
} ACCMD_STREAM_ANC_ENABLE_REQ;

/* The following macros take ACCMD_STREAM_ANC_ENABLE_REQ *accmd_stream_anc_enable_req_ptr or uint16 *addr */
#define ACCMD_STREAM_ANC_ENABLE_REQ_ANC_ENABLE_0_WORD_OFFSET (3)
#define ACCMD_STREAM_ANC_ENABLE_REQ_ANC_ENABLE_0_GET(addr) (*((addr) + 3))
#define ACCMD_STREAM_ANC_ENABLE_REQ_ANC_ENABLE_0_SET(addr, anc_enable_0) (*((addr) + 3) = (uint16)(anc_enable_0))
#define ACCMD_STREAM_ANC_ENABLE_REQ_ANC_ENABLE_1_WORD_OFFSET (4)
#define ACCMD_STREAM_ANC_ENABLE_REQ_ANC_ENABLE_1_GET(addr) (*((addr) + 4))
#define ACCMD_STREAM_ANC_ENABLE_REQ_ANC_ENABLE_1_SET(addr, anc_enable_1) (*((addr) + 4) = (uint16)(anc_enable_1))
#define ACCMD_STREAM_ANC_ENABLE_REQ_WORD_SIZE (5)
#define ACCMD_STREAM_ANC_ENABLE_REQ_PACK(addr, anc_enable_0, anc_enable_1) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(anc_enable_0)); \
        *((addr) + 4) = (uint16)((uint16)(anc_enable_1)); \
    } while (0)

#define ACCMD_STREAM_ANC_ENABLE_REQ_MARSHALL(addr, accmd_stream_anc_enable_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_anc_enable_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_anc_enable_req_ptr)->anc_enable_0); \
        *((addr) + 4) = (uint16)((accmd_stream_anc_enable_req_ptr)->anc_enable_1); \
    } while (0)

#define ACCMD_STREAM_ANC_ENABLE_REQ_UNMARSHALL(addr, accmd_stream_anc_enable_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_anc_enable_req_ptr)->header)); \
        (accmd_stream_anc_enable_req_ptr)->anc_enable_0 = ACCMD_STREAM_ANC_ENABLE_REQ_ANC_ENABLE_0_GET(addr); \
        (accmd_stream_anc_enable_req_ptr)->anc_enable_1 = ACCMD_STREAM_ANC_ENABLE_REQ_ANC_ENABLE_1_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Sidetone_En_Req

  DESCRIPTION
    From Client This command allows controls the enabling/disabling of
    "Uninterruptable" Sidetone.

  MEMBERS
    enable - The enable parameter enables/disables the "Uninterruptable"
             Sidetone.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       enable;
} ACCMD_STREAM_SIDETONE_EN_REQ;

/* The following macros take ACCMD_STREAM_SIDETONE_EN_REQ *accmd_stream_sidetone_en_req_ptr or uint16 *addr */
#define ACCMD_STREAM_SIDETONE_EN_REQ_ENABLE_WORD_OFFSET (3)
#define ACCMD_STREAM_SIDETONE_EN_REQ_ENABLE_GET(addr) (*((addr) + 3))
#define ACCMD_STREAM_SIDETONE_EN_REQ_ENABLE_SET(addr, enable) (*((addr) + 3) = (uint16)(enable))
#define ACCMD_STREAM_SIDETONE_EN_REQ_WORD_SIZE (4)
#define ACCMD_STREAM_SIDETONE_EN_REQ_PACK(addr, enable) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(enable)); \
    } while (0)

#define ACCMD_STREAM_SIDETONE_EN_REQ_MARSHALL(addr, accmd_stream_sidetone_en_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_sidetone_en_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_sidetone_en_req_ptr)->enable); \
    } while (0)

#define ACCMD_STREAM_SIDETONE_EN_REQ_UNMARSHALL(addr, accmd_stream_sidetone_en_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_sidetone_en_req_ptr)->header)); \
        (accmd_stream_sidetone_en_req_ptr)->enable = ACCMD_STREAM_SIDETONE_EN_REQ_ENABLE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Data_File_Allocate_Req

  DESCRIPTION
    Allocate buffer for data File

  MEMBERS
    Type      - Type of file
    Auto_free - flag to automatically delete file after use. When this is set
                the data file will be removed while destroying the associated
                operator if it is being loaded to an operator without
                Accmd_Data_File_DeAllocate_Req being called
    File_Size - Size of file in octets

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER       header;
    ACCMD_TYPE_OF_FILE Type;
    uint16             Auto_free;
    uint32             File_Size;
} ACCMD_DATA_FILE_ALLOCATE_REQ;

/* The following macros take ACCMD_DATA_FILE_ALLOCATE_REQ *accmd_data_file_allocate_req_ptr or uint16 *addr */
#define ACCMD_DATA_FILE_ALLOCATE_REQ_TYPE_WORD_OFFSET (3)
#define ACCMD_DATA_FILE_ALLOCATE_REQ_TYPE_GET(addr) ((ACCMD_TYPE_OF_FILE)*((addr) + 3))
#define ACCMD_DATA_FILE_ALLOCATE_REQ_TYPE_SET(addr, type) (*((addr) + 3) = (uint16)(type))
#define ACCMD_DATA_FILE_ALLOCATE_REQ_AUTO_FREE_WORD_OFFSET (4)
#define ACCMD_DATA_FILE_ALLOCATE_REQ_AUTO_FREE_GET(addr) (*((addr) + 4))
#define ACCMD_DATA_FILE_ALLOCATE_REQ_AUTO_FREE_SET(addr, auto_free) (*((addr) + 4) = (uint16)(auto_free))
#define ACCMD_DATA_FILE_ALLOCATE_REQ_FILE_SIZE_WORD_OFFSET (5)
#define ACCMD_DATA_FILE_ALLOCATE_REQ_FILE_SIZE_GET(addr)  \
    (((uint32)(*((addr) + 5)) | \
      ((uint32)(*((addr) + 6)) << 16)))
#define ACCMD_DATA_FILE_ALLOCATE_REQ_FILE_SIZE_SET(addr, file_size) do { \
        *((addr) + 5) = (uint16)((file_size) & 0xffff); \
        *((addr) + 6) = (uint16)((file_size) >> 16); } while (0)
#define ACCMD_DATA_FILE_ALLOCATE_REQ_WORD_SIZE (7)
#define ACCMD_DATA_FILE_ALLOCATE_REQ_PACK(addr, Type, Auto_free, File_Size) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Type)); \
        *((addr) + 4) = (uint16)((uint16)(Auto_free)); \
        *((addr) + 5) = (uint16)((uint16)((File_Size) & 0xffff)); \
        *((addr) + 6) = (uint16)(((File_Size) >> 16)); \
    } while (0)

#define ACCMD_DATA_FILE_ALLOCATE_REQ_MARSHALL(addr, accmd_data_file_allocate_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_data_file_allocate_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_data_file_allocate_req_ptr)->Type); \
        *((addr) + 4) = (uint16)((accmd_data_file_allocate_req_ptr)->Auto_free); \
        *((addr) + 5) = (uint16)(((accmd_data_file_allocate_req_ptr)->File_Size) & 0xffff); \
        *((addr) + 6) = (uint16)(((accmd_data_file_allocate_req_ptr)->File_Size) >> 16); \
    } while (0)

#define ACCMD_DATA_FILE_ALLOCATE_REQ_UNMARSHALL(addr, accmd_data_file_allocate_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_data_file_allocate_req_ptr)->header)); \
        (accmd_data_file_allocate_req_ptr)->Type = ACCMD_DATA_FILE_ALLOCATE_REQ_TYPE_GET(addr); \
        (accmd_data_file_allocate_req_ptr)->Auto_free = ACCMD_DATA_FILE_ALLOCATE_REQ_AUTO_FREE_GET(addr); \
        (accmd_data_file_allocate_req_ptr)->File_Size = ACCMD_DATA_FILE_ALLOCATE_REQ_FILE_SIZE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Data_File_Deallocate_Req

  DESCRIPTION
    Delete file with the requested ID

  MEMBERS
    File_Id - Id of the file

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       File_Id;
} ACCMD_DATA_FILE_DEALLOCATE_REQ;

/* The following macros take ACCMD_DATA_FILE_DEALLOCATE_REQ *accmd_data_file_deallocate_req_ptr or uint16 *addr */
#define ACCMD_DATA_FILE_DEALLOCATE_REQ_FILE_ID_WORD_OFFSET (3)
#define ACCMD_DATA_FILE_DEALLOCATE_REQ_FILE_ID_GET(addr) (*((addr) + 3))
#define ACCMD_DATA_FILE_DEALLOCATE_REQ_FILE_ID_SET(addr, file_id) (*((addr) + 3) = (uint16)(file_id))
#define ACCMD_DATA_FILE_DEALLOCATE_REQ_WORD_SIZE (4)
#define ACCMD_DATA_FILE_DEALLOCATE_REQ_PACK(addr, File_Id) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(File_Id)); \
    } while (0)

#define ACCMD_DATA_FILE_DEALLOCATE_REQ_MARSHALL(addr, accmd_data_file_deallocate_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_data_file_deallocate_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_data_file_deallocate_req_ptr)->File_Id); \
    } while (0)

#define ACCMD_DATA_FILE_DEALLOCATE_REQ_UNMARSHALL(addr, accmd_data_file_deallocate_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_data_file_deallocate_req_ptr)->header)); \
        (accmd_data_file_deallocate_req_ptr)->File_Id = ACCMD_DATA_FILE_DEALLOCATE_REQ_FILE_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_clock_Power_Save_Mode_Req

  DESCRIPTION
    This command allow the APP to register its Audio clock preference.

  MEMBERS
    Audio_power_Save_Mode - Power save operation Mode

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER           header;
    ACCMD_POWER_SAVE_MODES Audio_power_Save_Mode;
} ACCMD_CLOCK_POWER_SAVE_MODE_REQ;

/* The following macros take ACCMD_CLOCK_POWER_SAVE_MODE_REQ *accmd_clock_power_save_mode_req_ptr or uint16 *addr */
#define ACCMD_CLOCK_POWER_SAVE_MODE_REQ_AUDIO_POWER_SAVE_MODE_WORD_OFFSET (3)
#define ACCMD_CLOCK_POWER_SAVE_MODE_REQ_AUDIO_POWER_SAVE_MODE_GET(addr) ((ACCMD_POWER_SAVE_MODES)*((addr) + 3))
#define ACCMD_CLOCK_POWER_SAVE_MODE_REQ_AUDIO_POWER_SAVE_MODE_SET(addr, audio_power_save_mode) (*((addr) + 3) = (uint16)(audio_power_save_mode))
#define ACCMD_CLOCK_POWER_SAVE_MODE_REQ_WORD_SIZE (4)
#define ACCMD_CLOCK_POWER_SAVE_MODE_REQ_PACK(addr, Audio_power_Save_Mode) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Audio_power_Save_Mode)); \
    } while (0)

#define ACCMD_CLOCK_POWER_SAVE_MODE_REQ_MARSHALL(addr, accmd_clock_power_save_mode_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_clock_power_save_mode_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_clock_power_save_mode_req_ptr)->Audio_power_Save_Mode); \
    } while (0)

#define ACCMD_CLOCK_POWER_SAVE_MODE_REQ_UNMARSHALL(addr, accmd_clock_power_save_mode_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_clock_power_save_mode_req_ptr)->header)); \
        (accmd_clock_power_save_mode_req_ptr)->Audio_power_Save_Mode = ACCMD_CLOCK_POWER_SAVE_MODE_REQ_AUDIO_POWER_SAVE_MODE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Aux_Processor_Disable_Req

  DESCRIPTION
    Disable a secondary processor after boot. This provides an opportunity to
    override CoreEnable MIB key and reclaim the heap memory assigned for
    secondary processors during boot time.

  MEMBERS
    Processor_Id - Secondary processor ID.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Processor_Id;
} ACCMD_AUX_PROCESSOR_DISABLE_REQ;

/* The following macros take ACCMD_AUX_PROCESSOR_DISABLE_REQ *accmd_aux_processor_disable_req_ptr or uint16 *addr */
#define ACCMD_AUX_PROCESSOR_DISABLE_REQ_PROCESSOR_ID_WORD_OFFSET (3)
#define ACCMD_AUX_PROCESSOR_DISABLE_REQ_PROCESSOR_ID_GET(addr) (*((addr) + 3))
#define ACCMD_AUX_PROCESSOR_DISABLE_REQ_PROCESSOR_ID_SET(addr, processor_id) (*((addr) + 3) = (uint16)(processor_id))
#define ACCMD_AUX_PROCESSOR_DISABLE_REQ_WORD_SIZE (4)
#define ACCMD_AUX_PROCESSOR_DISABLE_REQ_PACK(addr, Processor_Id) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Processor_Id)); \
    } while (0)

#define ACCMD_AUX_PROCESSOR_DISABLE_REQ_MARSHALL(addr, accmd_aux_processor_disable_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_aux_processor_disable_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_aux_processor_disable_req_ptr)->Processor_Id); \
    } while (0)

#define ACCMD_AUX_PROCESSOR_DISABLE_REQ_UNMARSHALL(addr, accmd_aux_processor_disable_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_aux_processor_disable_req_ptr)->header)); \
        (accmd_aux_processor_disable_req_ptr)->Processor_Id = ACCMD_AUX_PROCESSOR_DISABLE_REQ_PROCESSOR_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Transform_From_Stream_Req

  DESCRIPTION
    Return the transform currently connected to a source or sink, if any;
    else 0.

  MEMBERS
    Sid    - Source or Sink

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       Sid;
} ACCMD_TRANSFORM_FROM_STREAM_REQ;

/* The following macros take ACCMD_TRANSFORM_FROM_STREAM_REQ *accmd_transform_from_stream_req_ptr or uint16 *addr */
#define ACCMD_TRANSFORM_FROM_STREAM_REQ_SID_WORD_OFFSET (3)
#define ACCMD_TRANSFORM_FROM_STREAM_REQ_SID_GET(addr) (*((addr) + 3))
#define ACCMD_TRANSFORM_FROM_STREAM_REQ_SID_SET(addr, sid) (*((addr) + 3) = (uint16)(sid))
#define ACCMD_TRANSFORM_FROM_STREAM_REQ_WORD_SIZE (4)
#define ACCMD_TRANSFORM_FROM_STREAM_REQ_PACK(addr, Sid) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Sid)); \
    } while (0)

#define ACCMD_TRANSFORM_FROM_STREAM_REQ_MARSHALL(addr, accmd_transform_from_stream_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_transform_from_stream_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_transform_from_stream_req_ptr)->Sid); \
    } while (0)

#define ACCMD_TRANSFORM_FROM_STREAM_REQ_UNMARSHALL(addr, accmd_transform_from_stream_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_transform_from_stream_req_ptr)->header)); \
        (accmd_transform_from_stream_req_ptr)->Sid = ACCMD_TRANSFORM_FROM_STREAM_REQ_SID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Set_Panic_Code_Req

  DESCRIPTION
    From Client An CSR internal command that sets the preserved block with
    the panic id and diatribe specified in the command.

  MEMBERS
    id       - panic id to set in preserved block
    diatribe - panic diatribe to set in preserved block

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       id;
    uint16       diatribe;
} ACCMD_SET_PANIC_CODE_REQ;

/* The following macros take ACCMD_SET_PANIC_CODE_REQ *accmd_set_panic_code_req_ptr or uint16 *addr */
#define ACCMD_SET_PANIC_CODE_REQ_ID_WORD_OFFSET (3)
#define ACCMD_SET_PANIC_CODE_REQ_ID_GET(addr) (*((addr) + 3))
#define ACCMD_SET_PANIC_CODE_REQ_ID_SET(addr, id) (*((addr) + 3) = (uint16)(id))
#define ACCMD_SET_PANIC_CODE_REQ_DIATRIBE_WORD_OFFSET (4)
#define ACCMD_SET_PANIC_CODE_REQ_DIATRIBE_GET(addr) (*((addr) + 4))
#define ACCMD_SET_PANIC_CODE_REQ_DIATRIBE_SET(addr, diatribe) (*((addr) + 4) = (uint16)(diatribe))
#define ACCMD_SET_PANIC_CODE_REQ_WORD_SIZE (5)
#define ACCMD_SET_PANIC_CODE_REQ_PACK(addr, id, diatribe) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(id)); \
        *((addr) + 4) = (uint16)((uint16)(diatribe)); \
    } while (0)

#define ACCMD_SET_PANIC_CODE_REQ_MARSHALL(addr, accmd_set_panic_code_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_set_panic_code_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_set_panic_code_req_ptr)->id); \
        *((addr) + 4) = (uint16)((accmd_set_panic_code_req_ptr)->diatribe); \
    } while (0)

#define ACCMD_SET_PANIC_CODE_REQ_UNMARSHALL(addr, accmd_set_panic_code_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_set_panic_code_req_ptr)->header)); \
        (accmd_set_panic_code_req_ptr)->id = ACCMD_SET_PANIC_CODE_REQ_ID_GET(addr); \
        (accmd_set_panic_code_req_ptr)->diatribe = ACCMD_SET_PANIC_CODE_REQ_DIATRIBE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Set_Fault_Code_Req

  DESCRIPTION
    From Client An CSR internal command that sets the preserved block with
    the fault id and diatribe specified in the command.

  MEMBERS
    id       - fault id to set in preserved block
    diatribe - fault diatribe to set in preserved block

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       id;
    uint16       diatribe;
} ACCMD_SET_FAULT_CODE_REQ;

/* The following macros take ACCMD_SET_FAULT_CODE_REQ *accmd_set_fault_code_req_ptr or uint16 *addr */
#define ACCMD_SET_FAULT_CODE_REQ_ID_WORD_OFFSET (3)
#define ACCMD_SET_FAULT_CODE_REQ_ID_GET(addr) (*((addr) + 3))
#define ACCMD_SET_FAULT_CODE_REQ_ID_SET(addr, id) (*((addr) + 3) = (uint16)(id))
#define ACCMD_SET_FAULT_CODE_REQ_DIATRIBE_WORD_OFFSET (4)
#define ACCMD_SET_FAULT_CODE_REQ_DIATRIBE_GET(addr) (*((addr) + 4))
#define ACCMD_SET_FAULT_CODE_REQ_DIATRIBE_SET(addr, diatribe) (*((addr) + 4) = (uint16)(diatribe))
#define ACCMD_SET_FAULT_CODE_REQ_WORD_SIZE (5)
#define ACCMD_SET_FAULT_CODE_REQ_PACK(addr, id, diatribe) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(id)); \
        *((addr) + 4) = (uint16)((uint16)(diatribe)); \
    } while (0)

#define ACCMD_SET_FAULT_CODE_REQ_MARSHALL(addr, accmd_set_fault_code_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_set_fault_code_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_set_fault_code_req_ptr)->id); \
        *((addr) + 4) = (uint16)((accmd_set_fault_code_req_ptr)->diatribe); \
    } while (0)

#define ACCMD_SET_FAULT_CODE_REQ_UNMARSHALL(addr, accmd_set_fault_code_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_set_fault_code_req_ptr)->header)); \
        (accmd_set_fault_code_req_ptr)->id = ACCMD_SET_FAULT_CODE_REQ_ID_GET(addr); \
        (accmd_set_fault_code_req_ptr)->diatribe = ACCMD_SET_FAULT_CODE_REQ_DIATRIBE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Set_Panic_Trigger_Req

  DESCRIPTION
    requests to trigger a panic in the Audio Subsystem

  MEMBERS

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
} ACCMD_SET_PANIC_TRIGGER_REQ;

/* The following macros take ACCMD_SET_PANIC_TRIGGER_REQ *accmd_set_panic_trigger_req_ptr or uint16 *addr */
#define ACCMD_SET_PANIC_TRIGGER_REQ_WORD_SIZE (3)


/*******************************************************************************

  NAME
    Accmd_Set_IPC_Interface_Test_Req

  DESCRIPTION
    From Client This command requests an Interprocessor interface test

  MEMBERS
    selector - The selector specifies the specifc interface test to be carried
               out

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint16       selector;
} ACCMD_SET_IPC_INTERFACE_TEST_REQ;

/* The following macros take ACCMD_SET_IPC_INTERFACE_TEST_REQ *accmd_set_ipc_interface_test_req_ptr or uint16 *addr */
#define ACCMD_SET_IPC_INTERFACE_TEST_REQ_SELECTOR_WORD_OFFSET (3)
#define ACCMD_SET_IPC_INTERFACE_TEST_REQ_SELECTOR_GET(addr) (*((addr) + 3))
#define ACCMD_SET_IPC_INTERFACE_TEST_REQ_SELECTOR_SET(addr, selector) (*((addr) + 3) = (uint16)(selector))
#define ACCMD_SET_IPC_INTERFACE_TEST_REQ_WORD_SIZE (4)
#define ACCMD_SET_IPC_INTERFACE_TEST_REQ_PACK(addr, selector) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(selector)); \
    } while (0)

#define ACCMD_SET_IPC_INTERFACE_TEST_REQ_MARSHALL(addr, accmd_set_ipc_interface_test_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_set_ipc_interface_test_req_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_set_ipc_interface_test_req_ptr)->selector); \
    } while (0)

#define ACCMD_SET_IPC_INTERFACE_TEST_REQ_UNMARSHALL(addr, accmd_set_ipc_interface_test_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_set_ipc_interface_test_req_ptr)->header)); \
        (accmd_set_ipc_interface_test_req_ptr)->selector = ACCMD_SET_IPC_INTERFACE_TEST_REQ_SELECTOR_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Call_Function_Req

  DESCRIPTION
    From Client This command implements an RPC mechanism for arbitrary
    functions in the firmware. The command carries a variable-length payload
    containing the function arguments in 32-bit fields.  It is up to the
    client to set this PDU up correctly. NOTE: this is only implemented for
    Kalimba32 targets

  MEMBERS
    address  - The call address of the function
    num_args - Number of arguments passed
    args     - List of arguments

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint32       address;
    uint8        num_args;
    uint32       args[ACCMD_PRIM_ANY_SIZE];
} ACCMD_CALL_FUNCTION_REQ;

/* The following macros take ACCMD_CALL_FUNCTION_REQ *accmd_call_function_req_ptr or uint16 *addr */
#define ACCMD_CALL_FUNCTION_REQ_ADDRESS_WORD_OFFSET (3)
#define ACCMD_CALL_FUNCTION_REQ_ADDRESS_GET(addr)  \
    (((uint32)(*((addr) + 3)) | \
      ((uint32)(*((addr) + 4)) << 16)))
#define ACCMD_CALL_FUNCTION_REQ_ADDRESS_SET(addr, address) do { \
        *((addr) + 3) = (uint16)((address) & 0xffff); \
        *((addr) + 4) = (uint16)((address) >> 16); } while (0)
#define ACCMD_CALL_FUNCTION_REQ_NUM_ARGS_WORD_OFFSET (5)
#define ACCMD_CALL_FUNCTION_REQ_NUM_ARGS_GET(addr) (((*((addr) + 5) & 0xff)))
#define ACCMD_CALL_FUNCTION_REQ_NUM_ARGS_SET(addr, num_args) (*((addr) + 5) =  \
                                                                  (uint16)((*((addr) + 5) & ~0xff) | (((num_args)) & 0xff)))
#define ACCMD_CALL_FUNCTION_REQ_ARGS_GET(addr)  \
    (((uint32)(*((addr) + 5)) | \
      ((uint32)(*((addr) + 6)) << 16)))
#define ACCMD_CALL_FUNCTION_REQ_ARGS_SET(addr, args) do { \
        *((addr) + 5) = (uint16)((args) & 0xffff); \
        *((addr) + 6) = (uint16)((args) >> 16); } while (0)
#define ACCMD_CALL_FUNCTION_REQ_WORD_SIZE (8)
#define ACCMD_CALL_FUNCTION_REQ_PACK(addr, address, num_args, args) \
    do { \
        *((addr) + 3) = (uint16)((uint16)((address) & 0xffff)); \
        *((addr) + 4) = (uint16)(((address) >> 16)); \
        *((addr) + 5) = (uint16)((uint16)(((num_args)) & 0xff) | \
                                 (uint16)((args) & 0xffff)); \
        *((addr) + 6) = (uint16)(((args) >> 16)); \
    } while (0)

#define ACCMD_CALL_FUNCTION_REQ_MARSHALL(addr, accmd_call_function_req_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_call_function_req_ptr)->header)); \
        *((addr) + 3) = (uint16)(((accmd_call_function_req_ptr)->address) & 0xffff); \
        *((addr) + 4) = (uint16)(((accmd_call_function_req_ptr)->address) >> 16); \
        *((addr) + 5) = (uint16)((((accmd_call_function_req_ptr)->num_args)) & 0xff) | \
                        (uint16)(((accmd_call_function_req_ptr)->args) & 0xffff); \
        *((addr) + 6) = (uint16)(((accmd_call_function_req_ptr)->args) >> 16); \
    } while (0)

#define ACCMD_CALL_FUNCTION_REQ_UNMARSHALL(addr, accmd_call_function_req_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_call_function_req_ptr)->header)); \
        (accmd_call_function_req_ptr)->address = ACCMD_CALL_FUNCTION_REQ_ADDRESS_GET(addr); \
        (accmd_call_function_req_ptr)->num_args = ACCMD_CALL_FUNCTION_REQ_NUM_ARGS_GET(addr); \
        (accmd_call_function_req_ptr)->args = ACCMD_CALL_FUNCTION_REQ_ARGS_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_No_Cmd_Resp

  DESCRIPTION
    From AudioSS an empty resp (header only)

  MEMBERS

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
} ACCMD_NO_CMD_RESP;

/* The following macros take ACCMD_NO_CMD_RESP *accmd_no_cmd_resp_ptr or uint16 *addr */
#define ACCMD_NO_CMD_RESP_WORD_SIZE (3)


/*******************************************************************************

  NAME
    Accmd_Stream_Get_Source_Resp

  DESCRIPTION
    From AudioSS This command acknowledges and responds to the
    Stream_Get_Source_Req command with the status of the request and sid
    representing the source ID

  MEMBERS
    Status    - The status of the request
    Source_ID - A sid referencing the requested source

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       Source_ID;
} ACCMD_STREAM_GET_SOURCE_RESP;

/* The following macros take ACCMD_STREAM_GET_SOURCE_RESP *accmd_stream_get_source_resp_ptr or uint16 *addr */
#define ACCMD_STREAM_GET_SOURCE_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_STREAM_GET_SOURCE_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_STREAM_GET_SOURCE_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_STREAM_GET_SOURCE_RESP_SOURCE_ID_WORD_OFFSET (4)
#define ACCMD_STREAM_GET_SOURCE_RESP_SOURCE_ID_GET(addr) (*((addr) + 4))
#define ACCMD_STREAM_GET_SOURCE_RESP_SOURCE_ID_SET(addr, source_id) (*((addr) + 4) = (uint16)(source_id))
#define ACCMD_STREAM_GET_SOURCE_RESP_WORD_SIZE (5)
#define ACCMD_STREAM_GET_SOURCE_RESP_PACK(addr, Status, Source_ID) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(Source_ID)); \
    } while (0)

#define ACCMD_STREAM_GET_SOURCE_RESP_MARSHALL(addr, accmd_stream_get_source_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_get_source_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_get_source_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_stream_get_source_resp_ptr)->Source_ID); \
    } while (0)

#define ACCMD_STREAM_GET_SOURCE_RESP_UNMARSHALL(addr, accmd_stream_get_source_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_get_source_resp_ptr)->header)); \
        (accmd_stream_get_source_resp_ptr)->Status = ACCMD_STREAM_GET_SOURCE_RESP_STATUS_GET(addr); \
        (accmd_stream_get_source_resp_ptr)->Source_ID = ACCMD_STREAM_GET_SOURCE_RESP_SOURCE_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Get_Sink_Resp

  DESCRIPTION
    From AudioSS This command acknowledges and responds to the
    Stream_Get_Sink_Req command with the status of the request and sid
    representing the source ID

  MEMBERS
    Status  - The status of the request
    Sink_ID - A sid referencing the requested source

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       Sink_ID;
} ACCMD_STREAM_GET_SINK_RESP;

/* The following macros take ACCMD_STREAM_GET_SINK_RESP *accmd_stream_get_sink_resp_ptr or uint16 *addr */
#define ACCMD_STREAM_GET_SINK_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_STREAM_GET_SINK_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_STREAM_GET_SINK_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_STREAM_GET_SINK_RESP_SINK_ID_WORD_OFFSET (4)
#define ACCMD_STREAM_GET_SINK_RESP_SINK_ID_GET(addr) (*((addr) + 4))
#define ACCMD_STREAM_GET_SINK_RESP_SINK_ID_SET(addr, sink_id) (*((addr) + 4) = (uint16)(sink_id))
#define ACCMD_STREAM_GET_SINK_RESP_WORD_SIZE (5)
#define ACCMD_STREAM_GET_SINK_RESP_PACK(addr, Status, Sink_ID) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(Sink_ID)); \
    } while (0)

#define ACCMD_STREAM_GET_SINK_RESP_MARSHALL(addr, accmd_stream_get_sink_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_get_sink_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_get_sink_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_stream_get_sink_resp_ptr)->Sink_ID); \
    } while (0)

#define ACCMD_STREAM_GET_SINK_RESP_UNMARSHALL(addr, accmd_stream_get_sink_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_get_sink_resp_ptr)->header)); \
        (accmd_stream_get_sink_resp_ptr)->Status = ACCMD_STREAM_GET_SINK_RESP_STATUS_GET(addr); \
        (accmd_stream_get_sink_resp_ptr)->Sink_ID = ACCMD_STREAM_GET_SINK_RESP_SINK_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Standard_Resp

  DESCRIPTION
    From AudioSS This command acknowledges the success (or failure) of
    Stream_Close_Source_Req

  MEMBERS
    Status - The status of the request

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
} ACCMD_STANDARD_RESP;

/* The following macros take ACCMD_STANDARD_RESP *accmd_standard_resp_ptr or uint16 *addr */
#define ACCMD_STANDARD_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_STANDARD_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_STANDARD_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_STANDARD_RESP_WORD_SIZE (4)
#define ACCMD_STANDARD_RESP_PACK(addr, Status) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
    } while (0)

#define ACCMD_STANDARD_RESP_MARSHALL(addr, accmd_standard_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_standard_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_standard_resp_ptr)->Status); \
    } while (0)

#define ACCMD_STANDARD_RESP_UNMARSHALL(addr, accmd_standard_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_standard_resp_ptr)->header)); \
        (accmd_standard_resp_ptr)->Status = ACCMD_STANDARD_RESP_STATUS_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Connect_Resp

  DESCRIPTION
    From AudioSS The success of the Stream_Connect_Req and the resulting
    transform connection ID

  MEMBERS
    Status       -
    Transform_ID - Streams transform id

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       Transform_ID;
} ACCMD_STREAM_CONNECT_RESP;

/* The following macros take ACCMD_STREAM_CONNECT_RESP *accmd_stream_connect_resp_ptr or uint16 *addr */
#define ACCMD_STREAM_CONNECT_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_STREAM_CONNECT_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_STREAM_CONNECT_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_STREAM_CONNECT_RESP_TRANSFORM_ID_WORD_OFFSET (4)
#define ACCMD_STREAM_CONNECT_RESP_TRANSFORM_ID_GET(addr) (*((addr) + 4))
#define ACCMD_STREAM_CONNECT_RESP_TRANSFORM_ID_SET(addr, transform_id) (*((addr) + 4) = (uint16)(transform_id))
#define ACCMD_STREAM_CONNECT_RESP_WORD_SIZE (5)
#define ACCMD_STREAM_CONNECT_RESP_PACK(addr, Status, Transform_ID) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(Transform_ID)); \
    } while (0)

#define ACCMD_STREAM_CONNECT_RESP_MARSHALL(addr, accmd_stream_connect_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_connect_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_connect_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_stream_connect_resp_ptr)->Transform_ID); \
    } while (0)

#define ACCMD_STREAM_CONNECT_RESP_UNMARSHALL(addr, accmd_stream_connect_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_connect_resp_ptr)->header)); \
        (accmd_stream_connect_resp_ptr)->Status = ACCMD_STREAM_CONNECT_RESP_STATUS_GET(addr); \
        (accmd_stream_connect_resp_ptr)->Transform_ID = ACCMD_STREAM_CONNECT_RESP_TRANSFORM_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_Firmware_Version_Resp

  DESCRIPTION
    From AudioSS gives audio subsystem version

  MEMBERS
    Status  -
    Version - audio subsystem version

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       Version;
} ACCMD_GET_FIRMWARE_VERSION_RESP;

/* The following macros take ACCMD_GET_FIRMWARE_VERSION_RESP *accmd_get_firmware_version_resp_ptr or uint16 *addr */
#define ACCMD_GET_FIRMWARE_VERSION_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_GET_FIRMWARE_VERSION_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_GET_FIRMWARE_VERSION_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_GET_FIRMWARE_VERSION_RESP_VERSION_WORD_OFFSET (4)
#define ACCMD_GET_FIRMWARE_VERSION_RESP_VERSION_GET(addr) (*((addr) + 4))
#define ACCMD_GET_FIRMWARE_VERSION_RESP_VERSION_SET(addr, version) (*((addr) + 4) = (uint16)(version))
#define ACCMD_GET_FIRMWARE_VERSION_RESP_WORD_SIZE (5)
#define ACCMD_GET_FIRMWARE_VERSION_RESP_PACK(addr, Status, Version) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(Version)); \
    } while (0)

#define ACCMD_GET_FIRMWARE_VERSION_RESP_MARSHALL(addr, accmd_get_firmware_version_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_get_firmware_version_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_get_firmware_version_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_get_firmware_version_resp_ptr)->Version); \
    } while (0)

#define ACCMD_GET_FIRMWARE_VERSION_RESP_UNMARSHALL(addr, accmd_get_firmware_version_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_get_firmware_version_resp_ptr)->header)); \
        (accmd_get_firmware_version_resp_ptr)->Status = ACCMD_GET_FIRMWARE_VERSION_RESP_STATUS_GET(addr); \
        (accmd_get_firmware_version_resp_ptr)->Version = ACCMD_GET_FIRMWARE_VERSION_RESP_VERSION_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Create_Operator_Resp

  DESCRIPTION
    From AudioSS Returns the operator ID if the capability has been
    successfully instantiated. Returns the error code if the operator was not
    created.

  MEMBERS
    Status              -
    opid_or_reason_code - Operator ID if successful, error reason code if failure

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       opid_or_reason_code;
} ACCMD_CREATE_OPERATOR_RESP;

/* The following macros take ACCMD_CREATE_OPERATOR_RESP *accmd_create_operator_resp_ptr or uint16 *addr */
#define ACCMD_CREATE_OPERATOR_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_CREATE_OPERATOR_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_CREATE_OPERATOR_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_CREATE_OPERATOR_RESP_OPID_OR_REASON_CODE_WORD_OFFSET (4)
#define ACCMD_CREATE_OPERATOR_RESP_OPID_OR_REASON_CODE_GET(addr) (*((addr) + 4))
#define ACCMD_CREATE_OPERATOR_RESP_OPID_OR_REASON_CODE_SET(addr, opid_or_reason_code) (*((addr) + 4) = (uint16)(opid_or_reason_code))
#define ACCMD_CREATE_OPERATOR_RESP_WORD_SIZE (5)
#define ACCMD_CREATE_OPERATOR_RESP_PACK(addr, Status, opid_or_reason_code) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(opid_or_reason_code)); \
    } while (0)

#define ACCMD_CREATE_OPERATOR_RESP_MARSHALL(addr, accmd_create_operator_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_create_operator_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_create_operator_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_create_operator_resp_ptr)->opid_or_reason_code); \
    } while (0)

#define ACCMD_CREATE_OPERATOR_RESP_UNMARSHALL(addr, accmd_create_operator_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_create_operator_resp_ptr)->header)); \
        (accmd_create_operator_resp_ptr)->Status = ACCMD_CREATE_OPERATOR_RESP_STATUS_GET(addr); \
        (accmd_create_operator_resp_ptr)->opid_or_reason_code = ACCMD_CREATE_OPERATOR_RESP_OPID_OR_REASON_CODE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Destroy_Operators_Resp

  DESCRIPTION
    From AudioSS Returns the number of Operators destroyed. If the number of
    operators destroyed is less than "Count" in the previous
    Accmd_Destroy_Operator_Req then an error reason code will also be returned

  MEMBERS
    Status      -
    Count       - The number of operators destroyed
    reason_code - Error reason code if not all operators destroyed

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       Count;
    uint16       reason_code;
} ACCMD_DESTROY_OPERATORS_RESP;

/* The following macros take ACCMD_DESTROY_OPERATORS_RESP *accmd_destroy_operators_resp_ptr or uint16 *addr */
#define ACCMD_DESTROY_OPERATORS_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_DESTROY_OPERATORS_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_DESTROY_OPERATORS_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_DESTROY_OPERATORS_RESP_COUNT_WORD_OFFSET (4)
#define ACCMD_DESTROY_OPERATORS_RESP_COUNT_GET(addr) (*((addr) + 4))
#define ACCMD_DESTROY_OPERATORS_RESP_COUNT_SET(addr, count) (*((addr) + 4) = (uint16)(count))
#define ACCMD_DESTROY_OPERATORS_RESP_REASON_CODE_WORD_OFFSET (5)
#define ACCMD_DESTROY_OPERATORS_RESP_REASON_CODE_GET(addr) (*((addr) + 5))
#define ACCMD_DESTROY_OPERATORS_RESP_REASON_CODE_SET(addr, reason_code) (*((addr) + 5) = (uint16)(reason_code))
#define ACCMD_DESTROY_OPERATORS_RESP_WORD_SIZE (6)
#define ACCMD_DESTROY_OPERATORS_RESP_PACK(addr, Status, Count, reason_code) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(Count)); \
        *((addr) + 5) = (uint16)((uint16)(reason_code)); \
    } while (0)

#define ACCMD_DESTROY_OPERATORS_RESP_MARSHALL(addr, accmd_destroy_operators_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_destroy_operators_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_destroy_operators_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_destroy_operators_resp_ptr)->Count); \
        *((addr) + 5) = (uint16)((accmd_destroy_operators_resp_ptr)->reason_code); \
    } while (0)

#define ACCMD_DESTROY_OPERATORS_RESP_UNMARSHALL(addr, accmd_destroy_operators_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_destroy_operators_resp_ptr)->header)); \
        (accmd_destroy_operators_resp_ptr)->Status = ACCMD_DESTROY_OPERATORS_RESP_STATUS_GET(addr); \
        (accmd_destroy_operators_resp_ptr)->Count = ACCMD_DESTROY_OPERATORS_RESP_COUNT_GET(addr); \
        (accmd_destroy_operators_resp_ptr)->reason_code = ACCMD_DESTROY_OPERATORS_RESP_REASON_CODE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Operator_Message_Resp

  DESCRIPTION
    From AudioSS Returns the original operator ID and a message response if
    the message has been correctly processed. If the message is not processed
    correctly an error code is returned.

  MEMBERS
    Status              -
    opid_or_reason_code - Operator ID if success, error code if failed
    response            - Contains the message response if command returns
                          successfully, undefined in failure case

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       opid_or_reason_code;
    uint16       response[ACCMD_PRIM_ANY_SIZE];
} ACCMD_OPERATOR_MESSAGE_RESP;

/* The following macros take ACCMD_OPERATOR_MESSAGE_RESP *accmd_operator_message_resp_ptr or uint16 *addr */
#define ACCMD_OPERATOR_MESSAGE_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_OPERATOR_MESSAGE_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_OPERATOR_MESSAGE_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_OPERATOR_MESSAGE_RESP_OPID_OR_REASON_CODE_WORD_OFFSET (4)
#define ACCMD_OPERATOR_MESSAGE_RESP_OPID_OR_REASON_CODE_GET(addr) (*((addr) + 4))
#define ACCMD_OPERATOR_MESSAGE_RESP_OPID_OR_REASON_CODE_SET(addr, opid_or_reason_code) (*((addr) + 4) = (uint16)(opid_or_reason_code))
#define ACCMD_OPERATOR_MESSAGE_RESP_RESPONSE_WORD_OFFSET (5)
#define ACCMD_OPERATOR_MESSAGE_RESP_RESPONSE_GET(addr) (*((addr) + 5))
#define ACCMD_OPERATOR_MESSAGE_RESP_RESPONSE_SET(addr, response) (*((addr) + 5) = (uint16)(response))
#define ACCMD_OPERATOR_MESSAGE_RESP_WORD_SIZE (6)
#define ACCMD_OPERATOR_MESSAGE_RESP_PACK(addr, Status, opid_or_reason_code, response) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(opid_or_reason_code)); \
        *((addr) + 5) = (uint16)((uint16)(response)); \
    } while (0)

#define ACCMD_OPERATOR_MESSAGE_RESP_MARSHALL(addr, accmd_operator_message_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_operator_message_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_operator_message_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_operator_message_resp_ptr)->opid_or_reason_code); \
        *((addr) + 5) = (uint16)((accmd_operator_message_resp_ptr)->response); \
    } while (0)

#define ACCMD_OPERATOR_MESSAGE_RESP_UNMARSHALL(addr, accmd_operator_message_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_operator_message_resp_ptr)->header)); \
        (accmd_operator_message_resp_ptr)->Status = ACCMD_OPERATOR_MESSAGE_RESP_STATUS_GET(addr); \
        (accmd_operator_message_resp_ptr)->opid_or_reason_code = ACCMD_OPERATOR_MESSAGE_RESP_OPID_OR_REASON_CODE_GET(addr); \
        (accmd_operator_message_resp_ptr)->response = ACCMD_OPERATOR_MESSAGE_RESP_RESPONSE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Start_Operators_Resp

  DESCRIPTION
    From AudioSS Returns the number of Operators started. If the number of
    operators started is less than "Count" in the previous
    Accmd_Start_Operators_Req then an error reason code will also be returned

  MEMBERS
    Status      -
    Count       - The number of operators started
    reason_code - Error reason code if not all operators were started

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       Count;
    uint16       reason_code;
} ACCMD_START_OPERATORS_RESP;

/* The following macros take ACCMD_START_OPERATORS_RESP *accmd_start_operators_resp_ptr or uint16 *addr */
#define ACCMD_START_OPERATORS_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_START_OPERATORS_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_START_OPERATORS_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_START_OPERATORS_RESP_COUNT_WORD_OFFSET (4)
#define ACCMD_START_OPERATORS_RESP_COUNT_GET(addr) (*((addr) + 4))
#define ACCMD_START_OPERATORS_RESP_COUNT_SET(addr, count) (*((addr) + 4) = (uint16)(count))
#define ACCMD_START_OPERATORS_RESP_REASON_CODE_WORD_OFFSET (5)
#define ACCMD_START_OPERATORS_RESP_REASON_CODE_GET(addr) (*((addr) + 5))
#define ACCMD_START_OPERATORS_RESP_REASON_CODE_SET(addr, reason_code) (*((addr) + 5) = (uint16)(reason_code))
#define ACCMD_START_OPERATORS_RESP_WORD_SIZE (6)
#define ACCMD_START_OPERATORS_RESP_PACK(addr, Status, Count, reason_code) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(Count)); \
        *((addr) + 5) = (uint16)((uint16)(reason_code)); \
    } while (0)

#define ACCMD_START_OPERATORS_RESP_MARSHALL(addr, accmd_start_operators_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_start_operators_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_start_operators_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_start_operators_resp_ptr)->Count); \
        *((addr) + 5) = (uint16)((accmd_start_operators_resp_ptr)->reason_code); \
    } while (0)

#define ACCMD_START_OPERATORS_RESP_UNMARSHALL(addr, accmd_start_operators_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_start_operators_resp_ptr)->header)); \
        (accmd_start_operators_resp_ptr)->Status = ACCMD_START_OPERATORS_RESP_STATUS_GET(addr); \
        (accmd_start_operators_resp_ptr)->Count = ACCMD_START_OPERATORS_RESP_COUNT_GET(addr); \
        (accmd_start_operators_resp_ptr)->reason_code = ACCMD_START_OPERATORS_RESP_REASON_CODE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stop_Operators_Resp

  DESCRIPTION
    From AudioSS Returns the number of Operators stopped. If the number of
    operators stopped is less than "Count" in the previous
    Accmd_Stop_Operators_Req then an error reason code will also be returned

  MEMBERS
    Status      -
    Count       - The number of operators stopped
    reason_code - Error reason code if not all operators stopped

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       Count;
    uint16       reason_code;
} ACCMD_STOP_OPERATORS_RESP;

/* The following macros take ACCMD_STOP_OPERATORS_RESP *accmd_stop_operators_resp_ptr or uint16 *addr */
#define ACCMD_STOP_OPERATORS_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_STOP_OPERATORS_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_STOP_OPERATORS_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_STOP_OPERATORS_RESP_COUNT_WORD_OFFSET (4)
#define ACCMD_STOP_OPERATORS_RESP_COUNT_GET(addr) (*((addr) + 4))
#define ACCMD_STOP_OPERATORS_RESP_COUNT_SET(addr, count) (*((addr) + 4) = (uint16)(count))
#define ACCMD_STOP_OPERATORS_RESP_REASON_CODE_WORD_OFFSET (5)
#define ACCMD_STOP_OPERATORS_RESP_REASON_CODE_GET(addr) (*((addr) + 5))
#define ACCMD_STOP_OPERATORS_RESP_REASON_CODE_SET(addr, reason_code) (*((addr) + 5) = (uint16)(reason_code))
#define ACCMD_STOP_OPERATORS_RESP_WORD_SIZE (6)
#define ACCMD_STOP_OPERATORS_RESP_PACK(addr, Status, Count, reason_code) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(Count)); \
        *((addr) + 5) = (uint16)((uint16)(reason_code)); \
    } while (0)

#define ACCMD_STOP_OPERATORS_RESP_MARSHALL(addr, accmd_stop_operators_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stop_operators_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stop_operators_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_stop_operators_resp_ptr)->Count); \
        *((addr) + 5) = (uint16)((accmd_stop_operators_resp_ptr)->reason_code); \
    } while (0)

#define ACCMD_STOP_OPERATORS_RESP_UNMARSHALL(addr, accmd_stop_operators_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stop_operators_resp_ptr)->header)); \
        (accmd_stop_operators_resp_ptr)->Status = ACCMD_STOP_OPERATORS_RESP_STATUS_GET(addr); \
        (accmd_stop_operators_resp_ptr)->Count = ACCMD_STOP_OPERATORS_RESP_COUNT_GET(addr); \
        (accmd_stop_operators_resp_ptr)->reason_code = ACCMD_STOP_OPERATORS_RESP_REASON_CODE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Reset_Operators_Resp

  DESCRIPTION
    From AudioSS Returns the number of Operators reset. If the number of
    operators reset is less than "Count" in the previous
    Accmd_Reset_Operators_Req then an error reason code will also be returned

  MEMBERS
    Status      -
    Count       - The number of operators reset
    reason_code - Error reason code if all operators have not been reset

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       Count;
    uint16       reason_code;
} ACCMD_RESET_OPERATORS_RESP;

/* The following macros take ACCMD_RESET_OPERATORS_RESP *accmd_reset_operators_resp_ptr or uint16 *addr */
#define ACCMD_RESET_OPERATORS_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_RESET_OPERATORS_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_RESET_OPERATORS_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_RESET_OPERATORS_RESP_COUNT_WORD_OFFSET (4)
#define ACCMD_RESET_OPERATORS_RESP_COUNT_GET(addr) (*((addr) + 4))
#define ACCMD_RESET_OPERATORS_RESP_COUNT_SET(addr, count) (*((addr) + 4) = (uint16)(count))
#define ACCMD_RESET_OPERATORS_RESP_REASON_CODE_WORD_OFFSET (5)
#define ACCMD_RESET_OPERATORS_RESP_REASON_CODE_GET(addr) (*((addr) + 5))
#define ACCMD_RESET_OPERATORS_RESP_REASON_CODE_SET(addr, reason_code) (*((addr) + 5) = (uint16)(reason_code))
#define ACCMD_RESET_OPERATORS_RESP_WORD_SIZE (6)
#define ACCMD_RESET_OPERATORS_RESP_PACK(addr, Status, Count, reason_code) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(Count)); \
        *((addr) + 5) = (uint16)((uint16)(reason_code)); \
    } while (0)

#define ACCMD_RESET_OPERATORS_RESP_MARSHALL(addr, accmd_reset_operators_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_reset_operators_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_reset_operators_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_reset_operators_resp_ptr)->Count); \
        *((addr) + 5) = (uint16)((accmd_reset_operators_resp_ptr)->reason_code); \
    } while (0)

#define ACCMD_RESET_OPERATORS_RESP_UNMARSHALL(addr, accmd_reset_operators_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_reset_operators_resp_ptr)->header)); \
        (accmd_reset_operators_resp_ptr)->Status = ACCMD_RESET_OPERATORS_RESP_STATUS_GET(addr); \
        (accmd_reset_operators_resp_ptr)->Count = ACCMD_RESET_OPERATORS_RESP_COUNT_GET(addr); \
        (accmd_reset_operators_resp_ptr)->reason_code = ACCMD_RESET_OPERATORS_RESP_REASON_CODE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_Firmware_Id_String_Resp

  DESCRIPTION
    From AudioSS This command carries a variable-length payload containing
    the ID string of the audio subsystem firmware.

  MEMBERS
    Status        -
    id_string_len - Length of the ID string, in uint16s
    id_string     - Audio subsystem ID string. This is a string consisting of
                    octets encoded in UTF-8, packed little-endian into 16-bit
                    words. If the string is an odd number of octets long, the
                    upper octet of the last word is zero.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       id_string_len;
    uint16       id_string[ACCMD_PRIM_ANY_SIZE];
} ACCMD_GET_FIRMWARE_ID_STRING_RESP;

/* The following macros take ACCMD_GET_FIRMWARE_ID_STRING_RESP *accmd_get_firmware_id_string_resp_ptr or uint16 *addr */
#define ACCMD_GET_FIRMWARE_ID_STRING_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_GET_FIRMWARE_ID_STRING_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_GET_FIRMWARE_ID_STRING_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_GET_FIRMWARE_ID_STRING_RESP_ID_STRING_LEN_WORD_OFFSET (4)
#define ACCMD_GET_FIRMWARE_ID_STRING_RESP_ID_STRING_LEN_GET(addr) (*((addr) + 4))
#define ACCMD_GET_FIRMWARE_ID_STRING_RESP_ID_STRING_LEN_SET(addr, id_string_len) (*((addr) + 4) = (uint16)(id_string_len))
#define ACCMD_GET_FIRMWARE_ID_STRING_RESP_ID_STRING_WORD_OFFSET (5)
#define ACCMD_GET_FIRMWARE_ID_STRING_RESP_ID_STRING_GET(addr) (*((addr) + 5))
#define ACCMD_GET_FIRMWARE_ID_STRING_RESP_ID_STRING_SET(addr, id_string) (*((addr) + 5) = (uint16)(id_string))
#define ACCMD_GET_FIRMWARE_ID_STRING_RESP_WORD_SIZE (6)
#define ACCMD_GET_FIRMWARE_ID_STRING_RESP_PACK(addr, Status, id_string_len, id_string) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(id_string_len)); \
        *((addr) + 5) = (uint16)((uint16)(id_string)); \
    } while (0)

#define ACCMD_GET_FIRMWARE_ID_STRING_RESP_MARSHALL(addr, accmd_get_firmware_id_string_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_get_firmware_id_string_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_get_firmware_id_string_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_get_firmware_id_string_resp_ptr)->id_string_len); \
        *((addr) + 5) = (uint16)((accmd_get_firmware_id_string_resp_ptr)->id_string); \
    } while (0)

#define ACCMD_GET_FIRMWARE_ID_STRING_RESP_UNMARSHALL(addr, accmd_get_firmware_id_string_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_get_firmware_id_string_resp_ptr)->header)); \
        (accmd_get_firmware_id_string_resp_ptr)->Status = ACCMD_GET_FIRMWARE_ID_STRING_RESP_STATUS_GET(addr); \
        (accmd_get_firmware_id_string_resp_ptr)->id_string_len = ACCMD_GET_FIRMWARE_ID_STRING_RESP_ID_STRING_LEN_GET(addr); \
        (accmd_get_firmware_id_string_resp_ptr)->id_string = ACCMD_GET_FIRMWARE_ID_STRING_RESP_ID_STRING_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Audio_prodtest_Resp

  DESCRIPTION
    Response message to production test messages.

  MEMBERS
    Status  - This is an accmd response so it contains an accmd status field
    test_id - The test ID that was started/stopped

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       test_id;
} ACCMD_AUDIO_PRODTEST_RESP;

/* The following macros take ACCMD_AUDIO_PRODTEST_RESP *accmd_audio_prodtest_resp_ptr or uint16 *addr */
#define ACCMD_AUDIO_PRODTEST_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_AUDIO_PRODTEST_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_AUDIO_PRODTEST_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_AUDIO_PRODTEST_RESP_TEST_ID_WORD_OFFSET (4)
#define ACCMD_AUDIO_PRODTEST_RESP_TEST_ID_GET(addr) (*((addr) + 4))
#define ACCMD_AUDIO_PRODTEST_RESP_TEST_ID_SET(addr, test_id) (*((addr) + 4) = (uint16)(test_id))
#define ACCMD_AUDIO_PRODTEST_RESP_WORD_SIZE (5)
#define ACCMD_AUDIO_PRODTEST_RESP_PACK(addr, Status, test_id) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(test_id)); \
    } while (0)

#define ACCMD_AUDIO_PRODTEST_RESP_MARSHALL(addr, accmd_audio_prodtest_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_audio_prodtest_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_audio_prodtest_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_audio_prodtest_resp_ptr)->test_id); \
    } while (0)

#define ACCMD_AUDIO_PRODTEST_RESP_UNMARSHALL(addr, accmd_audio_prodtest_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_audio_prodtest_resp_ptr)->header)); \
        (accmd_audio_prodtest_resp_ptr)->Status = ACCMD_AUDIO_PRODTEST_RESP_STATUS_GET(addr); \
        (accmd_audio_prodtest_resp_ptr)->test_id = ACCMD_AUDIO_PRODTEST_RESP_TEST_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Get_Info_Resp

  DESCRIPTION
    From AudioSS This command acknowledges and responds to the
    Stream_Get_Info_Req command with the status of the request and the value
    of the stream property requested.

  MEMBERS
    Status -
    value  - Value of the property previously requested

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint32       value;
} ACCMD_STREAM_GET_INFO_RESP;

/* The following macros take ACCMD_STREAM_GET_INFO_RESP *accmd_stream_get_info_resp_ptr or uint16 *addr */
#define ACCMD_STREAM_GET_INFO_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_STREAM_GET_INFO_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_STREAM_GET_INFO_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_STREAM_GET_INFO_RESP_VALUE_WORD_OFFSET (4)
#define ACCMD_STREAM_GET_INFO_RESP_VALUE_GET(addr)  \
    (((uint32)(*((addr) + 4)) | \
      ((uint32)(*((addr) + 5)) << 16)))
#define ACCMD_STREAM_GET_INFO_RESP_VALUE_SET(addr, value) do { \
        *((addr) + 4) = (uint16)((value) & 0xffff); \
        *((addr) + 5) = (uint16)((value) >> 16); } while (0)
#define ACCMD_STREAM_GET_INFO_RESP_WORD_SIZE (6)
#define ACCMD_STREAM_GET_INFO_RESP_PACK(addr, Status, value) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)((value) & 0xffff)); \
        *((addr) + 5) = (uint16)(((value) >> 16)); \
    } while (0)

#define ACCMD_STREAM_GET_INFO_RESP_MARSHALL(addr, accmd_stream_get_info_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_get_info_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_get_info_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)(((accmd_stream_get_info_resp_ptr)->value) & 0xffff); \
        *((addr) + 5) = (uint16)(((accmd_stream_get_info_resp_ptr)->value) >> 16); \
    } while (0)

#define ACCMD_STREAM_GET_INFO_RESP_UNMARSHALL(addr, accmd_stream_get_info_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_get_info_resp_ptr)->header)); \
        (accmd_stream_get_info_resp_ptr)->Status = ACCMD_STREAM_GET_INFO_RESP_STATUS_GET(addr); \
        (accmd_stream_get_info_resp_ptr)->value = ACCMD_STREAM_GET_INFO_RESP_VALUE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_Capabilities_Resp

  DESCRIPTION
    From AudioSS This command acknowledges and responds to the
    Get_Capabilities_Req command with a list of the capability IDs currently
    supported.

  MEMBERS
    Status                 -
    total_num_capabilities - Length of the list of capabilities, in uint16s
    num_capabilities       - Number of capabilities present in this message, in
                             uint16s
    capabilities           - List of capability IDs currently supported, one per
                             uint16.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       total_num_capabilities;
    uint16       num_capabilities;
    uint16       capabilities[ACCMD_PRIM_ANY_SIZE];
} ACCMD_GET_CAPABILITIES_RESP;

/* The following macros take ACCMD_GET_CAPABILITIES_RESP *accmd_get_capabilities_resp_ptr or uint16 *addr */
#define ACCMD_GET_CAPABILITIES_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_GET_CAPABILITIES_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_GET_CAPABILITIES_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_GET_CAPABILITIES_RESP_TOTAL_NUM_CAPABILITIES_WORD_OFFSET (4)
#define ACCMD_GET_CAPABILITIES_RESP_TOTAL_NUM_CAPABILITIES_GET(addr) (*((addr) + 4))
#define ACCMD_GET_CAPABILITIES_RESP_TOTAL_NUM_CAPABILITIES_SET(addr, total_num_capabilities) (*((addr) + 4) = (uint16)(total_num_capabilities))
#define ACCMD_GET_CAPABILITIES_RESP_NUM_CAPABILITIES_WORD_OFFSET (5)
#define ACCMD_GET_CAPABILITIES_RESP_NUM_CAPABILITIES_GET(addr) (*((addr) + 5))
#define ACCMD_GET_CAPABILITIES_RESP_NUM_CAPABILITIES_SET(addr, num_capabilities) (*((addr) + 5) = (uint16)(num_capabilities))
#define ACCMD_GET_CAPABILITIES_RESP_CAPABILITIES_WORD_OFFSET (6)
#define ACCMD_GET_CAPABILITIES_RESP_CAPABILITIES_GET(addr) (*((addr) + 6))
#define ACCMD_GET_CAPABILITIES_RESP_CAPABILITIES_SET(addr, capabilities) (*((addr) + 6) = (uint16)(capabilities))
#define ACCMD_GET_CAPABILITIES_RESP_WORD_SIZE (7)
#define ACCMD_GET_CAPABILITIES_RESP_PACK(addr, Status, total_num_capabilities, num_capabilities, capabilities) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(total_num_capabilities)); \
        *((addr) + 5) = (uint16)((uint16)(num_capabilities)); \
        *((addr) + 6) = (uint16)((uint16)(capabilities)); \
    } while (0)

#define ACCMD_GET_CAPABILITIES_RESP_MARSHALL(addr, accmd_get_capabilities_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_get_capabilities_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_get_capabilities_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_get_capabilities_resp_ptr)->total_num_capabilities); \
        *((addr) + 5) = (uint16)((accmd_get_capabilities_resp_ptr)->num_capabilities); \
        *((addr) + 6) = (uint16)((accmd_get_capabilities_resp_ptr)->capabilities); \
    } while (0)

#define ACCMD_GET_CAPABILITIES_RESP_UNMARSHALL(addr, accmd_get_capabilities_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_get_capabilities_resp_ptr)->header)); \
        (accmd_get_capabilities_resp_ptr)->Status = ACCMD_GET_CAPABILITIES_RESP_STATUS_GET(addr); \
        (accmd_get_capabilities_resp_ptr)->total_num_capabilities = ACCMD_GET_CAPABILITIES_RESP_TOTAL_NUM_CAPABILITIES_GET(addr); \
        (accmd_get_capabilities_resp_ptr)->num_capabilities = ACCMD_GET_CAPABILITIES_RESP_NUM_CAPABILITIES_GET(addr); \
        (accmd_get_capabilities_resp_ptr)->capabilities = ACCMD_GET_CAPABILITIES_RESP_CAPABILITIES_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Create_Operator_Ex_Resp

  DESCRIPTION
    From AudioSS Returns the operator ID if the capability has been
    successfully instantiated. Returns the error code if the operator was not
    created.

  MEMBERS
    Status              -
    opid_or_reason_code - Operator ID if successful, error reason code if failure

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       opid_or_reason_code;
} ACCMD_CREATE_OPERATOR_EX_RESP;

/* The following macros take ACCMD_CREATE_OPERATOR_EX_RESP *accmd_create_operator_ex_resp_ptr or uint16 *addr */
#define ACCMD_CREATE_OPERATOR_EX_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_CREATE_OPERATOR_EX_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_CREATE_OPERATOR_EX_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_CREATE_OPERATOR_EX_RESP_OPID_OR_REASON_CODE_WORD_OFFSET (4)
#define ACCMD_CREATE_OPERATOR_EX_RESP_OPID_OR_REASON_CODE_GET(addr) (*((addr) + 4))
#define ACCMD_CREATE_OPERATOR_EX_RESP_OPID_OR_REASON_CODE_SET(addr, opid_or_reason_code) (*((addr) + 4) = (uint16)(opid_or_reason_code))
#define ACCMD_CREATE_OPERATOR_EX_RESP_WORD_SIZE (5)
#define ACCMD_CREATE_OPERATOR_EX_RESP_PACK(addr, Status, opid_or_reason_code) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(opid_or_reason_code)); \
    } while (0)

#define ACCMD_CREATE_OPERATOR_EX_RESP_MARSHALL(addr, accmd_create_operator_ex_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_create_operator_ex_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_create_operator_ex_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_create_operator_ex_resp_ptr)->opid_or_reason_code); \
    } while (0)

#define ACCMD_CREATE_OPERATOR_EX_RESP_UNMARSHALL(addr, accmd_create_operator_ex_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_create_operator_ex_resp_ptr)->header)); \
        (accmd_create_operator_ex_resp_ptr)->Status = ACCMD_CREATE_OPERATOR_EX_RESP_STATUS_GET(addr); \
        (accmd_create_operator_ex_resp_ptr)->opid_or_reason_code = ACCMD_CREATE_OPERATOR_EX_RESP_OPID_OR_REASON_CODE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_Mem_Usage_Resp

  DESCRIPTION
    Response message containing informations about memory usage.

  MEMBERS
    Status       -
    heap_size    - The heap size in words.
    heap_current - Current heap usage in words.
    heap_min     - Minimum available heap in words.
    pool_size    - The pool memory size in words.
    pool_current - Current pool memory usage in words.
    pool_min     - Minimum available pool memory in words.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint32       heap_size;
    uint32       heap_current;
    uint32       heap_min;
    uint32       pool_size;
    uint32       pool_current;
    uint32       pool_min;
} ACCMD_GET_MEM_USAGE_RESP;

/* The following macros take ACCMD_GET_MEM_USAGE_RESP *accmd_get_mem_usage_resp_ptr or uint16 *addr */
#define ACCMD_GET_MEM_USAGE_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_GET_MEM_USAGE_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_GET_MEM_USAGE_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_GET_MEM_USAGE_RESP_HEAP_SIZE_WORD_OFFSET (4)
#define ACCMD_GET_MEM_USAGE_RESP_HEAP_SIZE_GET(addr)  \
    (((uint32)(*((addr) + 4)) | \
      ((uint32)(*((addr) + 5)) << 16)))
#define ACCMD_GET_MEM_USAGE_RESP_HEAP_SIZE_SET(addr, heap_size) do { \
        *((addr) + 4) = (uint16)((heap_size) & 0xffff); \
        *((addr) + 5) = (uint16)((heap_size) >> 16); } while (0)
#define ACCMD_GET_MEM_USAGE_RESP_HEAP_CURRENT_WORD_OFFSET (6)
#define ACCMD_GET_MEM_USAGE_RESP_HEAP_CURRENT_GET(addr)  \
    (((uint32)(*((addr) + 6)) | \
      ((uint32)(*((addr) + 7)) << 16)))
#define ACCMD_GET_MEM_USAGE_RESP_HEAP_CURRENT_SET(addr, heap_current) do { \
        *((addr) + 6) = (uint16)((heap_current) & 0xffff); \
        *((addr) + 7) = (uint16)((heap_current) >> 16); } while (0)
#define ACCMD_GET_MEM_USAGE_RESP_HEAP_MIN_WORD_OFFSET (8)
#define ACCMD_GET_MEM_USAGE_RESP_HEAP_MIN_GET(addr)  \
    (((uint32)(*((addr) + 8)) | \
      ((uint32)(*((addr) + 8 + 1)) << 16)))
#define ACCMD_GET_MEM_USAGE_RESP_HEAP_MIN_SET(addr, heap_min) do { \
        *((addr) + 8) = (uint16)((heap_min) & 0xffff); \
        *((addr) + 8 + 1) = (uint16)((heap_min) >> 16); } while (0)
#define ACCMD_GET_MEM_USAGE_RESP_POOL_SIZE_WORD_OFFSET (10)
#define ACCMD_GET_MEM_USAGE_RESP_POOL_SIZE_GET(addr)  \
    (((uint32)(*((addr) + 10)) | \
      ((uint32)(*((addr) + 11)) << 16)))
#define ACCMD_GET_MEM_USAGE_RESP_POOL_SIZE_SET(addr, pool_size) do { \
        *((addr) + 10) = (uint16)((pool_size) & 0xffff); \
        *((addr) + 11) = (uint16)((pool_size) >> 16); } while (0)
#define ACCMD_GET_MEM_USAGE_RESP_POOL_CURRENT_WORD_OFFSET (12)
#define ACCMD_GET_MEM_USAGE_RESP_POOL_CURRENT_GET(addr)  \
    (((uint32)(*((addr) + 12)) | \
      ((uint32)(*((addr) + 13)) << 16)))
#define ACCMD_GET_MEM_USAGE_RESP_POOL_CURRENT_SET(addr, pool_current) do { \
        *((addr) + 12) = (uint16)((pool_current) & 0xffff); \
        *((addr) + 13) = (uint16)((pool_current) >> 16); } while (0)
#define ACCMD_GET_MEM_USAGE_RESP_POOL_MIN_WORD_OFFSET (14)
#define ACCMD_GET_MEM_USAGE_RESP_POOL_MIN_GET(addr)  \
    (((uint32)(*((addr) + 14)) | \
      ((uint32)(*((addr) + 15)) << 16)))
#define ACCMD_GET_MEM_USAGE_RESP_POOL_MIN_SET(addr, pool_min) do { \
        *((addr) + 14) = (uint16)((pool_min) & 0xffff); \
        *((addr) + 15) = (uint16)((pool_min) >> 16); } while (0)
#define ACCMD_GET_MEM_USAGE_RESP_WORD_SIZE (16)
#define ACCMD_GET_MEM_USAGE_RESP_PACK(addr, Status, heap_size, heap_current, heap_min, pool_size, pool_current, pool_min) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)((heap_size) & 0xffff)); \
        *((addr) + 5) = (uint16)(((heap_size) >> 16)); \
        *((addr) + 6) = (uint16)((uint16)((heap_current) & 0xffff)); \
        *((addr) + 7) = (uint16)(((heap_current) >> 16)); \
        *((addr) + 8) = (uint16)((uint16)((heap_min) & 0xffff)); \
        *((addr) + 8 + 1) = (uint16)(((heap_min) >> 16)); \
        *((addr) + 10) = (uint16)((uint16)((pool_size) & 0xffff)); \
        *((addr) + 11) = (uint16)(((pool_size) >> 16)); \
        *((addr) + 12) = (uint16)((uint16)((pool_current) & 0xffff)); \
        *((addr) + 13) = (uint16)(((pool_current) >> 16)); \
        *((addr) + 14) = (uint16)((uint16)((pool_min) & 0xffff)); \
        *((addr) + 15) = (uint16)(((pool_min) >> 16)); \
    } while (0)

#define ACCMD_GET_MEM_USAGE_RESP_MARSHALL(addr, accmd_get_mem_usage_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_get_mem_usage_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_get_mem_usage_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)(((accmd_get_mem_usage_resp_ptr)->heap_size) & 0xffff); \
        *((addr) + 5) = (uint16)(((accmd_get_mem_usage_resp_ptr)->heap_size) >> 16); \
        *((addr) + 6) = (uint16)(((accmd_get_mem_usage_resp_ptr)->heap_current) & 0xffff); \
        *((addr) + 7) = (uint16)(((accmd_get_mem_usage_resp_ptr)->heap_current) >> 16); \
        *((addr) + 8) = (uint16)(((accmd_get_mem_usage_resp_ptr)->heap_min) & 0xffff); \
        *((addr) + 8 + 1) = (uint16)(((accmd_get_mem_usage_resp_ptr)->heap_min) >> 16); \
        *((addr) + 10) = (uint16)(((accmd_get_mem_usage_resp_ptr)->pool_size) & 0xffff); \
        *((addr) + 11) = (uint16)(((accmd_get_mem_usage_resp_ptr)->pool_size) >> 16); \
        *((addr) + 12) = (uint16)(((accmd_get_mem_usage_resp_ptr)->pool_current) & 0xffff); \
        *((addr) + 13) = (uint16)(((accmd_get_mem_usage_resp_ptr)->pool_current) >> 16); \
        *((addr) + 14) = (uint16)(((accmd_get_mem_usage_resp_ptr)->pool_min) & 0xffff); \
        *((addr) + 15) = (uint16)(((accmd_get_mem_usage_resp_ptr)->pool_min) >> 16); \
    } while (0)

#define ACCMD_GET_MEM_USAGE_RESP_UNMARSHALL(addr, accmd_get_mem_usage_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_get_mem_usage_resp_ptr)->header)); \
        (accmd_get_mem_usage_resp_ptr)->Status = ACCMD_GET_MEM_USAGE_RESP_STATUS_GET(addr); \
        (accmd_get_mem_usage_resp_ptr)->heap_size = ACCMD_GET_MEM_USAGE_RESP_HEAP_SIZE_GET(addr); \
        (accmd_get_mem_usage_resp_ptr)->heap_current = ACCMD_GET_MEM_USAGE_RESP_HEAP_CURRENT_GET(addr); \
        (accmd_get_mem_usage_resp_ptr)->heap_min = ACCMD_GET_MEM_USAGE_RESP_HEAP_MIN_GET(addr); \
        (accmd_get_mem_usage_resp_ptr)->pool_size = ACCMD_GET_MEM_USAGE_RESP_POOL_SIZE_GET(addr); \
        (accmd_get_mem_usage_resp_ptr)->pool_current = ACCMD_GET_MEM_USAGE_RESP_POOL_CURRENT_GET(addr); \
        (accmd_get_mem_usage_resp_ptr)->pool_min = ACCMD_GET_MEM_USAGE_RESP_POOL_MIN_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Clear_Mem_Watermarks_Resp

  DESCRIPTION
    Response message showing if the mem watermarks was successfully cleared.

  MEMBERS
    Status - Returns STATUS_OK if the operation was successful.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
} ACCMD_CLEAR_MEM_WATERMARKS_RESP;

/* The following macros take ACCMD_CLEAR_MEM_WATERMARKS_RESP *accmd_clear_mem_watermarks_resp_ptr or uint16 *addr */
#define ACCMD_CLEAR_MEM_WATERMARKS_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_CLEAR_MEM_WATERMARKS_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_CLEAR_MEM_WATERMARKS_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_CLEAR_MEM_WATERMARKS_RESP_WORD_SIZE (4)
#define ACCMD_CLEAR_MEM_WATERMARKS_RESP_PACK(addr, Status) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
    } while (0)

#define ACCMD_CLEAR_MEM_WATERMARKS_RESP_MARSHALL(addr, accmd_clear_mem_watermarks_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_clear_mem_watermarks_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_clear_mem_watermarks_resp_ptr)->Status); \
    } while (0)

#define ACCMD_CLEAR_MEM_WATERMARKS_RESP_UNMARSHALL(addr, accmd_clear_mem_watermarks_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_clear_mem_watermarks_resp_ptr)->header)); \
        (accmd_clear_mem_watermarks_resp_ptr)->Status = ACCMD_CLEAR_MEM_WATERMARKS_RESP_STATUS_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_Mips_Usage_Resp

  DESCRIPTION
    Response message holding the MIPS usage of an operator.

  MEMBERS
    Status       - Returns STATUS_OK if the operator exist.
    Sleep        - Sleep in thousandths.
    Count        - The number of operator mips in the list OP_MIPS_LIST. An
                   operator mips contains the operator id echoed with the mips
                   usage in thousandths.
    OP_MIPS_LIST - List containing the operators mips usage.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER        header;
    ACCMD_STATUS        Status;
    uint16              Sleep;
    uint16              Count;
    ACCMD_OP_MIPS_USAGE OP_MIPS_LIST[ACCMD_PRIM_ANY_SIZE];
} ACCMD_GET_MIPS_USAGE_RESP;

/* The following macros take ACCMD_GET_MIPS_USAGE_RESP *accmd_get_mips_usage_resp_ptr or uint16 *addr */
#define ACCMD_GET_MIPS_USAGE_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_GET_MIPS_USAGE_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_GET_MIPS_USAGE_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_GET_MIPS_USAGE_RESP_SLEEP_WORD_OFFSET (4)
#define ACCMD_GET_MIPS_USAGE_RESP_SLEEP_GET(addr) (*((addr) + 4))
#define ACCMD_GET_MIPS_USAGE_RESP_SLEEP_SET(addr, sleep) (*((addr) + 4) = (uint16)(sleep))
#define ACCMD_GET_MIPS_USAGE_RESP_COUNT_WORD_OFFSET (5)
#define ACCMD_GET_MIPS_USAGE_RESP_COUNT_GET(addr) (*((addr) + 5))
#define ACCMD_GET_MIPS_USAGE_RESP_COUNT_SET(addr, count) (*((addr) + 5) = (uint16)(count))
#define ACCMD_GET_MIPS_USAGE_RESP_OP_MIPS_LIST_WORD_OFFSET (6)
#define ACCMD_GET_MIPS_USAGE_RESP_OP_MIPS_LIST_GET(addr, op_mips_list_ptr) do {  \
        (op_mips_list_ptr)->_data[0] = *((addr) + 6); \
        (op_mips_list_ptr)->_data[1] = *((addr) + 7); } while (0)
#define ACCMD_GET_MIPS_USAGE_RESP_OP_MIPS_LIST_SET(addr, op_mips_list_ptr) do {  \
        *((addr) + 6) = (op_mips_list_ptr)->_data[0]; \
        *((addr) + 7) = (op_mips_list_ptr)->_data[1]; } while (0)
#define ACCMD_GET_MIPS_USAGE_RESP_WORD_SIZE (8)
#define ACCMD_GET_MIPS_USAGE_RESP_PACK(addr, Status, Sleep, Count, OP_MIPS_LIST_ptr) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(Sleep)); \
        *((addr) + 5) = (uint16)((uint16)(Count)); \
        *((addr) + 6) = (uint16)((op_mips_list_ptr)->_data[0]); \
        *((addr) + 7) = (uint16)((op_mips_list_ptr)->_data[1]); \
    } while (0)

#define ACCMD_GET_MIPS_USAGE_RESP_MARSHALL(addr, accmd_get_mips_usage_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_get_mips_usage_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_get_mips_usage_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_get_mips_usage_resp_ptr)->Sleep); \
        *((addr) + 5) = (uint16)((accmd_get_mips_usage_resp_ptr)->Count); \
        ACCMD_OP_MIPS_USAGE_MARSHALL((addr) + ACCMD_GET_MIPS_USAGE_RESP_OP_MIPS_LIST_WORD_OFFSET, &((accmd_get_mips_usage_resp_ptr)->OP_MIPS_LIST)); \
    } while (0)

#define ACCMD_GET_MIPS_USAGE_RESP_UNMARSHALL(addr, accmd_get_mips_usage_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_get_mips_usage_resp_ptr)->header)); \
        (accmd_get_mips_usage_resp_ptr)->Status = ACCMD_GET_MIPS_USAGE_RESP_STATUS_GET(addr); \
        (accmd_get_mips_usage_resp_ptr)->Sleep = ACCMD_GET_MIPS_USAGE_RESP_SLEEP_GET(addr); \
        (accmd_get_mips_usage_resp_ptr)->Count = ACCMD_GET_MIPS_USAGE_RESP_COUNT_GET(addr); \
        ACCMD_OP_MIPS_USAGE_UNMARSHALL((addr) + ACCMD_GET_MIPS_USAGE_RESP_OP_MIPS_LIST_WORD_OFFSET, &((accmd_get_mips_usage_resp_ptr)->OP_MIPS_LIST)); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Set_System_Stream_Rate_Resp

  DESCRIPTION
    Response after setting default streaming rate

  MEMBERS
    Status - Returns STATUS_OK for successfully changing system default

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
} ACCMD_SET_SYSTEM_STREAM_RATE_RESP;

/* The following macros take ACCMD_SET_SYSTEM_STREAM_RATE_RESP *accmd_set_system_stream_rate_resp_ptr or uint16 *addr */
#define ACCMD_SET_SYSTEM_STREAM_RATE_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_SET_SYSTEM_STREAM_RATE_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_SET_SYSTEM_STREAM_RATE_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_SET_SYSTEM_STREAM_RATE_RESP_WORD_SIZE (4)
#define ACCMD_SET_SYSTEM_STREAM_RATE_RESP_PACK(addr, Status) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
    } while (0)

#define ACCMD_SET_SYSTEM_STREAM_RATE_RESP_MARSHALL(addr, accmd_set_system_stream_rate_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_set_system_stream_rate_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_set_system_stream_rate_resp_ptr)->Status); \
    } while (0)

#define ACCMD_SET_SYSTEM_STREAM_RATE_RESP_UNMARSHALL(addr, accmd_set_system_stream_rate_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_set_system_stream_rate_resp_ptr)->header)); \
        (accmd_set_system_stream_rate_resp_ptr)->Status = ACCMD_SET_SYSTEM_STREAM_RATE_RESP_STATUS_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_System_Stream_Rate_Resp

  DESCRIPTION
    Get the default system streaming rate

  MEMBERS
    Status      - Returns STATUS_OK for successfull getting system default
    Sample_rate - System sampling rate

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint32       Sample_rate;
} ACCMD_GET_SYSTEM_STREAM_RATE_RESP;

/* The following macros take ACCMD_GET_SYSTEM_STREAM_RATE_RESP *accmd_get_system_stream_rate_resp_ptr or uint16 *addr */
#define ACCMD_GET_SYSTEM_STREAM_RATE_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_GET_SYSTEM_STREAM_RATE_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_GET_SYSTEM_STREAM_RATE_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_GET_SYSTEM_STREAM_RATE_RESP_SAMPLE_RATE_WORD_OFFSET (4)
#define ACCMD_GET_SYSTEM_STREAM_RATE_RESP_SAMPLE_RATE_GET(addr)  \
    (((uint32)(*((addr) + 4)) | \
      ((uint32)(*((addr) + 5)) << 16)))
#define ACCMD_GET_SYSTEM_STREAM_RATE_RESP_SAMPLE_RATE_SET(addr, sample_rate) do { \
        *((addr) + 4) = (uint16)((sample_rate) & 0xffff); \
        *((addr) + 5) = (uint16)((sample_rate) >> 16); } while (0)
#define ACCMD_GET_SYSTEM_STREAM_RATE_RESP_WORD_SIZE (6)
#define ACCMD_GET_SYSTEM_STREAM_RATE_RESP_PACK(addr, Status, Sample_rate) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)((Sample_rate) & 0xffff)); \
        *((addr) + 5) = (uint16)(((Sample_rate) >> 16)); \
    } while (0)

#define ACCMD_GET_SYSTEM_STREAM_RATE_RESP_MARSHALL(addr, accmd_get_system_stream_rate_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_get_system_stream_rate_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_get_system_stream_rate_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)(((accmd_get_system_stream_rate_resp_ptr)->Sample_rate) & 0xffff); \
        *((addr) + 5) = (uint16)(((accmd_get_system_stream_rate_resp_ptr)->Sample_rate) >> 16); \
    } while (0)

#define ACCMD_GET_SYSTEM_STREAM_RATE_RESP_UNMARSHALL(addr, accmd_get_system_stream_rate_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_get_system_stream_rate_resp_ptr)->header)); \
        (accmd_get_system_stream_rate_resp_ptr)->Status = ACCMD_GET_SYSTEM_STREAM_RATE_RESP_STATUS_GET(addr); \
        (accmd_get_system_stream_rate_resp_ptr)->Sample_rate = ACCMD_GET_SYSTEM_STREAM_RATE_RESP_SAMPLE_RATE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Disconnect_Resp

  DESCRIPTION
    Disconnect endpoints reply.

  MEMBERS
    Status        - Returns STATUS_OK for successfull disconnect
    Transform_ID1 - Returns transform id for successfully disconnected source
    Transform_ID2 - Returns transform id for successfully disconnected sink

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       Transform_ID1;
    uint16       Transform_ID2;
} ACCMD_STREAM_DISCONNECT_RESP;

/* The following macros take ACCMD_STREAM_DISCONNECT_RESP *accmd_stream_disconnect_resp_ptr or uint16 *addr */
#define ACCMD_STREAM_DISCONNECT_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_STREAM_DISCONNECT_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_STREAM_DISCONNECT_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_STREAM_DISCONNECT_RESP_TRANSFORM_ID1_WORD_OFFSET (4)
#define ACCMD_STREAM_DISCONNECT_RESP_TRANSFORM_ID1_GET(addr) (*((addr) + 4))
#define ACCMD_STREAM_DISCONNECT_RESP_TRANSFORM_ID1_SET(addr, transform_id1) (*((addr) + 4) = (uint16)(transform_id1))
#define ACCMD_STREAM_DISCONNECT_RESP_TRANSFORM_ID2_WORD_OFFSET (5)
#define ACCMD_STREAM_DISCONNECT_RESP_TRANSFORM_ID2_GET(addr) (*((addr) + 5))
#define ACCMD_STREAM_DISCONNECT_RESP_TRANSFORM_ID2_SET(addr, transform_id2) (*((addr) + 5) = (uint16)(transform_id2))
#define ACCMD_STREAM_DISCONNECT_RESP_WORD_SIZE (6)
#define ACCMD_STREAM_DISCONNECT_RESP_PACK(addr, Status, Transform_ID1, Transform_ID2) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(Transform_ID1)); \
        *((addr) + 5) = (uint16)((uint16)(Transform_ID2)); \
    } while (0)

#define ACCMD_STREAM_DISCONNECT_RESP_MARSHALL(addr, accmd_stream_disconnect_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_disconnect_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_disconnect_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_stream_disconnect_resp_ptr)->Transform_ID1); \
        *((addr) + 5) = (uint16)((accmd_stream_disconnect_resp_ptr)->Transform_ID2); \
    } while (0)

#define ACCMD_STREAM_DISCONNECT_RESP_UNMARSHALL(addr, accmd_stream_disconnect_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_disconnect_resp_ptr)->header)); \
        (accmd_stream_disconnect_resp_ptr)->Status = ACCMD_STREAM_DISCONNECT_RESP_STATUS_GET(addr); \
        (accmd_stream_disconnect_resp_ptr)->Transform_ID1 = ACCMD_STREAM_DISCONNECT_RESP_TRANSFORM_ID1_GET(addr); \
        (accmd_stream_disconnect_resp_ptr)->Transform_ID2 = ACCMD_STREAM_DISCONNECT_RESP_TRANSFORM_ID2_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Get_Sink_From_Source_Resp

  DESCRIPTION
    Find sink endpoint from stream source reply.

  MEMBERS
    Status - Returns STATUS_OK for successfull retrieval
    Sink   - Returns sink id

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       Sink;
} ACCMD_STREAM_GET_SINK_FROM_SOURCE_RESP;

/* The following macros take ACCMD_STREAM_GET_SINK_FROM_SOURCE_RESP *accmd_stream_get_sink_from_source_resp_ptr or uint16 *addr */
#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_RESP_SINK_WORD_OFFSET (4)
#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_RESP_SINK_GET(addr) (*((addr) + 4))
#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_RESP_SINK_SET(addr, sink) (*((addr) + 4) = (uint16)(sink))
#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_RESP_WORD_SIZE (5)
#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_RESP_PACK(addr, Status, Sink) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(Sink)); \
    } while (0)

#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_RESP_MARSHALL(addr, accmd_stream_get_sink_from_source_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_get_sink_from_source_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_get_sink_from_source_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_stream_get_sink_from_source_resp_ptr)->Sink); \
    } while (0)

#define ACCMD_STREAM_GET_SINK_FROM_SOURCE_RESP_UNMARSHALL(addr, accmd_stream_get_sink_from_source_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_get_sink_from_source_resp_ptr)->header)); \
        (accmd_stream_get_sink_from_source_resp_ptr)->Status = ACCMD_STREAM_GET_SINK_FROM_SOURCE_RESP_STATUS_GET(addr); \
        (accmd_stream_get_sink_from_source_resp_ptr)->Sink = ACCMD_STREAM_GET_SINK_FROM_SOURCE_RESP_SINK_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Stream_Get_Source_From_Sink_Resp

  DESCRIPTION
    Find source endpoint from stream sink reply.

  MEMBERS
    Status - Returns STATUS_OK for successfull retrieval
    Source - Returns source id

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       Source;
} ACCMD_STREAM_GET_SOURCE_FROM_SINK_RESP;

/* The following macros take ACCMD_STREAM_GET_SOURCE_FROM_SINK_RESP *accmd_stream_get_source_from_sink_resp_ptr or uint16 *addr */
#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_RESP_SOURCE_WORD_OFFSET (4)
#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_RESP_SOURCE_GET(addr) (*((addr) + 4))
#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_RESP_SOURCE_SET(addr, source) (*((addr) + 4) = (uint16)(source))
#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_RESP_WORD_SIZE (5)
#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_RESP_PACK(addr, Status, Source) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(Source)); \
    } while (0)

#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_RESP_MARSHALL(addr, accmd_stream_get_source_from_sink_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_stream_get_source_from_sink_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_stream_get_source_from_sink_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_stream_get_source_from_sink_resp_ptr)->Source); \
    } while (0)

#define ACCMD_STREAM_GET_SOURCE_FROM_SINK_RESP_UNMARSHALL(addr, accmd_stream_get_source_from_sink_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_stream_get_source_from_sink_resp_ptr)->header)); \
        (accmd_stream_get_source_from_sink_resp_ptr)->Status = ACCMD_STREAM_GET_SOURCE_FROM_SINK_RESP_STATUS_GET(addr); \
        (accmd_stream_get_source_from_sink_resp_ptr)->Source = ACCMD_STREAM_GET_SOURCE_FROM_SINK_RESP_SOURCE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Set_System_Framework_Resp

  DESCRIPTION
    From AudioSS Response after setting system parameters.

  MEMBERS
    status - The status of the request

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS status;
} ACCMD_SET_SYSTEM_FRAMEWORK_RESP;

/* The following macros take ACCMD_SET_SYSTEM_FRAMEWORK_RESP *accmd_set_system_framework_resp_ptr or uint16 *addr */
#define ACCMD_SET_SYSTEM_FRAMEWORK_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_SET_SYSTEM_FRAMEWORK_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_SET_SYSTEM_FRAMEWORK_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_SET_SYSTEM_FRAMEWORK_RESP_WORD_SIZE (4)
#define ACCMD_SET_SYSTEM_FRAMEWORK_RESP_PACK(addr, status) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(status)); \
    } while (0)

#define ACCMD_SET_SYSTEM_FRAMEWORK_RESP_MARSHALL(addr, accmd_set_system_framework_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_set_system_framework_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_set_system_framework_resp_ptr)->status); \
    } while (0)

#define ACCMD_SET_SYSTEM_FRAMEWORK_RESP_UNMARSHALL(addr, accmd_set_system_framework_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_set_system_framework_resp_ptr)->header)); \
        (accmd_set_system_framework_resp_ptr)->status = ACCMD_SET_SYSTEM_FRAMEWORK_RESP_STATUS_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_System_Framework_Resp

  DESCRIPTION
    From AudioSS This command acknowledges and responds to the
    System_Get_Framework_Req command with the status of the request and a
    message that contains its specific information.

  MEMBERS
    status  - The status of the request
    message - The message that contains specific information for each case.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS status;
    uint16       message[ACCMD_PRIM_ANY_SIZE];
} ACCMD_GET_SYSTEM_FRAMEWORK_RESP;

/* The following macros take ACCMD_GET_SYSTEM_FRAMEWORK_RESP *accmd_get_system_framework_resp_ptr or uint16 *addr */
#define ACCMD_GET_SYSTEM_FRAMEWORK_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_GET_SYSTEM_FRAMEWORK_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_GET_SYSTEM_FRAMEWORK_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_GET_SYSTEM_FRAMEWORK_RESP_MESSAGE_WORD_OFFSET (4)
#define ACCMD_GET_SYSTEM_FRAMEWORK_RESP_MESSAGE_GET(addr) (*((addr) + 4))
#define ACCMD_GET_SYSTEM_FRAMEWORK_RESP_MESSAGE_SET(addr, message) (*((addr) + 4) = (uint16)(message))
#define ACCMD_GET_SYSTEM_FRAMEWORK_RESP_WORD_SIZE (5)
#define ACCMD_GET_SYSTEM_FRAMEWORK_RESP_PACK(addr, status, message) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(status)); \
        *((addr) + 4) = (uint16)((uint16)(message)); \
    } while (0)

#define ACCMD_GET_SYSTEM_FRAMEWORK_RESP_MARSHALL(addr, accmd_get_system_framework_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_get_system_framework_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_get_system_framework_resp_ptr)->status); \
        *((addr) + 4) = (uint16)((accmd_get_system_framework_resp_ptr)->message); \
    } while (0)

#define ACCMD_GET_SYSTEM_FRAMEWORK_RESP_UNMARSHALL(addr, accmd_get_system_framework_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_get_system_framework_resp_ptr)->header)); \
        (accmd_get_system_framework_resp_ptr)->status = ACCMD_GET_SYSTEM_FRAMEWORK_RESP_STATUS_GET(addr); \
        (accmd_get_system_framework_resp_ptr)->message = ACCMD_GET_SYSTEM_FRAMEWORK_RESP_MESSAGE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_IPC_Interface_Test_Resp

  DESCRIPTION
    From AudioSS This command acknowledges and responds to the
    SET_IPC_INTERFACE_TEST_REQ command with the status of the request and a
    message that contains the test id.

  MEMBERS
    status  - The status of the request
    test_id - test_id

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS status;
    uint16       test_id[ACCMD_PRIM_ANY_SIZE];
} ACCMD_GET_IPC_INTERFACE_TEST_RESP;

/* The following macros take ACCMD_GET_IPC_INTERFACE_TEST_RESP *accmd_get_ipc_interface_test_resp_ptr or uint16 *addr */
#define ACCMD_GET_IPC_INTERFACE_TEST_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_GET_IPC_INTERFACE_TEST_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_GET_IPC_INTERFACE_TEST_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_GET_IPC_INTERFACE_TEST_RESP_TEST_ID_WORD_OFFSET (4)
#define ACCMD_GET_IPC_INTERFACE_TEST_RESP_TEST_ID_GET(addr) (*((addr) + 4))
#define ACCMD_GET_IPC_INTERFACE_TEST_RESP_TEST_ID_SET(addr, test_id) (*((addr) + 4) = (uint16)(test_id))
#define ACCMD_GET_IPC_INTERFACE_TEST_RESP_WORD_SIZE (5)
#define ACCMD_GET_IPC_INTERFACE_TEST_RESP_PACK(addr, status, test_id) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(status)); \
        *((addr) + 4) = (uint16)((uint16)(test_id)); \
    } while (0)

#define ACCMD_GET_IPC_INTERFACE_TEST_RESP_MARSHALL(addr, accmd_get_ipc_interface_test_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_get_ipc_interface_test_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_get_ipc_interface_test_resp_ptr)->status); \
        *((addr) + 4) = (uint16)((accmd_get_ipc_interface_test_resp_ptr)->test_id); \
    } while (0)

#define ACCMD_GET_IPC_INTERFACE_TEST_RESP_UNMARSHALL(addr, accmd_get_ipc_interface_test_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_get_ipc_interface_test_resp_ptr)->header)); \
        (accmd_get_ipc_interface_test_resp_ptr)->status = ACCMD_GET_IPC_INTERFACE_TEST_RESP_STATUS_GET(addr); \
        (accmd_get_ipc_interface_test_resp_ptr)->test_id = ACCMD_GET_IPC_INTERFACE_TEST_RESP_TEST_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Get_Operators_Resp

  DESCRIPTION
    From AudioSS This command acknowledges and responds to the
    Accmd_Get_Operators_Req command with a list of the operator
    IDs/capability IDs pairs currently instanciated in the system.

  MEMBERS
    Status              -
    total_num_operators - Total number of operators matching the filter in the
                          system, in uint16s
    num_operators       - Number of operators present in this message, in
                          uint16s
    payload             - List of capability IDs currently supported, one per
                          uint16.

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER              header;
    ACCMD_STATUS              Status;
    uint16                    total_num_operators;
    uint16                    num_operators;
    ACCMD_GET_OPERATORS_ENTRY payload[ACCMD_PRIM_ANY_SIZE];
} ACCMD_GET_OPERATORS_RESP;

/* The following macros take ACCMD_GET_OPERATORS_RESP *accmd_get_operators_resp_ptr or uint16 *addr */
#define ACCMD_GET_OPERATORS_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_GET_OPERATORS_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_GET_OPERATORS_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_GET_OPERATORS_RESP_TOTAL_NUM_OPERATORS_WORD_OFFSET (4)
#define ACCMD_GET_OPERATORS_RESP_TOTAL_NUM_OPERATORS_GET(addr) (*((addr) + 4))
#define ACCMD_GET_OPERATORS_RESP_TOTAL_NUM_OPERATORS_SET(addr, total_num_operators) (*((addr) + 4) = (uint16)(total_num_operators))
#define ACCMD_GET_OPERATORS_RESP_NUM_OPERATORS_WORD_OFFSET (5)
#define ACCMD_GET_OPERATORS_RESP_NUM_OPERATORS_GET(addr) (*((addr) + 5))
#define ACCMD_GET_OPERATORS_RESP_NUM_OPERATORS_SET(addr, num_operators) (*((addr) + 5) = (uint16)(num_operators))
#define ACCMD_GET_OPERATORS_RESP_PAYLOAD_WORD_OFFSET (6)
#define ACCMD_GET_OPERATORS_RESP_PAYLOAD_GET(addr, payload_ptr) do {  \
        (payload_ptr)->_data[0] = *((addr) + 6); \
        (payload_ptr)->_data[1] = *((addr) + 7); } while (0)
#define ACCMD_GET_OPERATORS_RESP_PAYLOAD_SET(addr, payload_ptr) do {  \
        *((addr) + 6) = (payload_ptr)->_data[0]; \
        *((addr) + 7) = (payload_ptr)->_data[1]; } while (0)
#define ACCMD_GET_OPERATORS_RESP_WORD_SIZE (8)
#define ACCMD_GET_OPERATORS_RESP_PACK(addr, Status, total_num_operators, num_operators, payload_ptr) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(total_num_operators)); \
        *((addr) + 5) = (uint16)((uint16)(num_operators)); \
        *((addr) + 6) = (uint16)((payload_ptr)->_data[0]); \
        *((addr) + 7) = (uint16)((payload_ptr)->_data[1]); \
    } while (0)

#define ACCMD_GET_OPERATORS_RESP_MARSHALL(addr, accmd_get_operators_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_get_operators_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_get_operators_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_get_operators_resp_ptr)->total_num_operators); \
        *((addr) + 5) = (uint16)((accmd_get_operators_resp_ptr)->num_operators); \
        ACCMD_GET_OPERATORS_ENTRY_MARSHALL((addr) + ACCMD_GET_OPERATORS_RESP_PAYLOAD_WORD_OFFSET, &((accmd_get_operators_resp_ptr)->payload)); \
    } while (0)

#define ACCMD_GET_OPERATORS_RESP_UNMARSHALL(addr, accmd_get_operators_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_get_operators_resp_ptr)->header)); \
        (accmd_get_operators_resp_ptr)->Status = ACCMD_GET_OPERATORS_RESP_STATUS_GET(addr); \
        (accmd_get_operators_resp_ptr)->total_num_operators = ACCMD_GET_OPERATORS_RESP_TOTAL_NUM_OPERATORS_GET(addr); \
        (accmd_get_operators_resp_ptr)->num_operators = ACCMD_GET_OPERATORS_RESP_NUM_OPERATORS_GET(addr); \
        ACCMD_GET_OPERATORS_ENTRY_UNMARSHALL((addr) + ACCMD_GET_OPERATORS_RESP_PAYLOAD_WORD_OFFSET, &((accmd_get_operators_resp_ptr)->payload)); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_PS_Read_Resp

  DESCRIPTION
    From Client The data returned may be incomplete. The requester can
    determine this from the ACCMD payload length and the Total_Len field (and
    the offset they requested, if any), and should send further PS_Read_Req
    with different offsets to fetch the remainder. This protocol is stateful
    for the requester: they are assumed to remember what key/offset this is
    about, so that's not included (this can be tracked via seq_no if
    necessary).

  MEMBERS
    Status    - The status of the request
    Total_Len - The length of the entire key, in uint16s (regardless of any
                offset in the Req). Use this to spot incomplete data.
    data      - the actual data; length implicit in ACCMD payload length

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       Total_Len;
    uint16       data[ACCMD_PRIM_ANY_SIZE];
} ACCMD_PS_READ_RESP;

/* The following macros take ACCMD_PS_READ_RESP *accmd_ps_read_resp_ptr or uint16 *addr */
#define ACCMD_PS_READ_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_PS_READ_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_PS_READ_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_PS_READ_RESP_TOTAL_LEN_WORD_OFFSET (4)
#define ACCMD_PS_READ_RESP_TOTAL_LEN_GET(addr) (*((addr) + 4))
#define ACCMD_PS_READ_RESP_TOTAL_LEN_SET(addr, total_len) (*((addr) + 4) = (uint16)(total_len))
#define ACCMD_PS_READ_RESP_DATA_WORD_OFFSET (5)
#define ACCMD_PS_READ_RESP_DATA_GET(addr) (*((addr) + 5))
#define ACCMD_PS_READ_RESP_DATA_SET(addr, data) (*((addr) + 5) = (uint16)(data))
#define ACCMD_PS_READ_RESP_WORD_SIZE (6)
#define ACCMD_PS_READ_RESP_PACK(addr, Status, Total_Len, data) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(Total_Len)); \
        *((addr) + 5) = (uint16)((uint16)(data)); \
    } while (0)

#define ACCMD_PS_READ_RESP_MARSHALL(addr, accmd_ps_read_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_ps_read_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_ps_read_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_ps_read_resp_ptr)->Total_Len); \
        *((addr) + 5) = (uint16)((accmd_ps_read_resp_ptr)->data); \
    } while (0)

#define ACCMD_PS_READ_RESP_UNMARSHALL(addr, accmd_ps_read_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_ps_read_resp_ptr)->header)); \
        (accmd_ps_read_resp_ptr)->Status = ACCMD_PS_READ_RESP_STATUS_GET(addr); \
        (accmd_ps_read_resp_ptr)->Total_Len = ACCMD_PS_READ_RESP_TOTAL_LEN_GET(addr); \
        (accmd_ps_read_resp_ptr)->data = ACCMD_PS_READ_RESP_DATA_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_PS_Write_Resp

  DESCRIPTION
    From Client This message indicates whether a Persistent Storage write
    operation was successful or not.

  MEMBERS
    Status - The status of the request

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
} ACCMD_PS_WRITE_RESP;

/* The following macros take ACCMD_PS_WRITE_RESP *accmd_ps_write_resp_ptr or uint16 *addr */
#define ACCMD_PS_WRITE_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_PS_WRITE_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_PS_WRITE_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_PS_WRITE_RESP_WORD_SIZE (4)
#define ACCMD_PS_WRITE_RESP_PACK(addr, Status) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
    } while (0)

#define ACCMD_PS_WRITE_RESP_MARSHALL(addr, accmd_ps_write_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_ps_write_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_ps_write_resp_ptr)->Status); \
    } while (0)

#define ACCMD_PS_WRITE_RESP_UNMARSHALL(addr, accmd_ps_write_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_ps_write_resp_ptr)->header)); \
        (accmd_ps_write_resp_ptr)->Status = ACCMD_PS_WRITE_RESP_STATUS_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Transform_From_Stream_Resp

  DESCRIPTION
    Transform returned in response to Accmd_Transform_From_Stream_Req.

  MEMBERS
    status       - The status of the request
    Transform_ID - Transform, or 0

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS status;
    uint16       Transform_ID;
} ACCMD_TRANSFORM_FROM_STREAM_RESP;

/* The following macros take ACCMD_TRANSFORM_FROM_STREAM_RESP *accmd_transform_from_stream_resp_ptr or uint16 *addr */
#define ACCMD_TRANSFORM_FROM_STREAM_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_TRANSFORM_FROM_STREAM_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_TRANSFORM_FROM_STREAM_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_TRANSFORM_FROM_STREAM_RESP_TRANSFORM_ID_WORD_OFFSET (4)
#define ACCMD_TRANSFORM_FROM_STREAM_RESP_TRANSFORM_ID_GET(addr) (*((addr) + 4))
#define ACCMD_TRANSFORM_FROM_STREAM_RESP_TRANSFORM_ID_SET(addr, transform_id) (*((addr) + 4) = (uint16)(transform_id))
#define ACCMD_TRANSFORM_FROM_STREAM_RESP_WORD_SIZE (5)
#define ACCMD_TRANSFORM_FROM_STREAM_RESP_PACK(addr, status, Transform_ID) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(status)); \
        *((addr) + 4) = (uint16)((uint16)(Transform_ID)); \
    } while (0)

#define ACCMD_TRANSFORM_FROM_STREAM_RESP_MARSHALL(addr, accmd_transform_from_stream_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_transform_from_stream_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_transform_from_stream_resp_ptr)->status); \
        *((addr) + 4) = (uint16)((accmd_transform_from_stream_resp_ptr)->Transform_ID); \
    } while (0)

#define ACCMD_TRANSFORM_FROM_STREAM_RESP_UNMARSHALL(addr, accmd_transform_from_stream_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_transform_from_stream_resp_ptr)->header)); \
        (accmd_transform_from_stream_resp_ptr)->status = ACCMD_TRANSFORM_FROM_STREAM_RESP_STATUS_GET(addr); \
        (accmd_transform_from_stream_resp_ptr)->Transform_ID = ACCMD_TRANSFORM_FROM_STREAM_RESP_TRANSFORM_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_clock_configure_Resp

  DESCRIPTION
    Response for Accmd_clock_configure_Req

  MEMBERS
    status             - The status of the request
    low_power_mode_clk - Operation Mode
    wake_on_mode_clk   - CPU clock variant
    active_mode_clk    - Kcodec clock variant

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS status;
    uint16       low_power_mode_clk;
    uint16       wake_on_mode_clk;
    uint16       active_mode_clk;
} ACCMD_CLOCK_CONFIGURE_RESP;

/* The following macros take ACCMD_CLOCK_CONFIGURE_RESP *accmd_clock_configure_resp_ptr or uint16 *addr */
#define ACCMD_CLOCK_CONFIGURE_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_CLOCK_CONFIGURE_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_CLOCK_CONFIGURE_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_CLOCK_CONFIGURE_RESP_LOW_POWER_MODE_CLK_WORD_OFFSET (4)
#define ACCMD_CLOCK_CONFIGURE_RESP_LOW_POWER_MODE_CLK_GET(addr) (*((addr) + 4))
#define ACCMD_CLOCK_CONFIGURE_RESP_LOW_POWER_MODE_CLK_SET(addr, low_power_mode_clk) (*((addr) + 4) = (uint16)(low_power_mode_clk))
#define ACCMD_CLOCK_CONFIGURE_RESP_WAKE_ON_MODE_CLK_WORD_OFFSET (5)
#define ACCMD_CLOCK_CONFIGURE_RESP_WAKE_ON_MODE_CLK_GET(addr) (*((addr) + 5))
#define ACCMD_CLOCK_CONFIGURE_RESP_WAKE_ON_MODE_CLK_SET(addr, wake_on_mode_clk) (*((addr) + 5) = (uint16)(wake_on_mode_clk))
#define ACCMD_CLOCK_CONFIGURE_RESP_ACTIVE_MODE_CLK_WORD_OFFSET (6)
#define ACCMD_CLOCK_CONFIGURE_RESP_ACTIVE_MODE_CLK_GET(addr) (*((addr) + 6))
#define ACCMD_CLOCK_CONFIGURE_RESP_ACTIVE_MODE_CLK_SET(addr, active_mode_clk) (*((addr) + 6) = (uint16)(active_mode_clk))
#define ACCMD_CLOCK_CONFIGURE_RESP_WORD_SIZE (7)
#define ACCMD_CLOCK_CONFIGURE_RESP_PACK(addr, status, low_power_mode_clk, wake_on_mode_clk, active_mode_clk) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(status)); \
        *((addr) + 4) = (uint16)((uint16)(low_power_mode_clk)); \
        *((addr) + 5) = (uint16)((uint16)(wake_on_mode_clk)); \
        *((addr) + 6) = (uint16)((uint16)(active_mode_clk)); \
    } while (0)

#define ACCMD_CLOCK_CONFIGURE_RESP_MARSHALL(addr, accmd_clock_configure_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_clock_configure_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_clock_configure_resp_ptr)->status); \
        *((addr) + 4) = (uint16)((accmd_clock_configure_resp_ptr)->low_power_mode_clk); \
        *((addr) + 5) = (uint16)((accmd_clock_configure_resp_ptr)->wake_on_mode_clk); \
        *((addr) + 6) = (uint16)((accmd_clock_configure_resp_ptr)->active_mode_clk); \
    } while (0)

#define ACCMD_CLOCK_CONFIGURE_RESP_UNMARSHALL(addr, accmd_clock_configure_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_clock_configure_resp_ptr)->header)); \
        (accmd_clock_configure_resp_ptr)->status = ACCMD_CLOCK_CONFIGURE_RESP_STATUS_GET(addr); \
        (accmd_clock_configure_resp_ptr)->low_power_mode_clk = ACCMD_CLOCK_CONFIGURE_RESP_LOW_POWER_MODE_CLK_GET(addr); \
        (accmd_clock_configure_resp_ptr)->wake_on_mode_clk = ACCMD_CLOCK_CONFIGURE_RESP_WAKE_ON_MODE_CLK_GET(addr); \
        (accmd_clock_configure_resp_ptr)->active_mode_clk = ACCMD_CLOCK_CONFIGURE_RESP_ACTIVE_MODE_CLK_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Data_File_Allocate_Resp

  DESCRIPTION
    Data file allocate sink reply

  MEMBERS
    Status  - Accmd status
    File_Id - Id of the file

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    ACCMD_STATUS Status;
    uint16       File_Id;
} ACCMD_DATA_FILE_ALLOCATE_RESP;

/* The following macros take ACCMD_DATA_FILE_ALLOCATE_RESP *accmd_data_file_allocate_resp_ptr or uint16 *addr */
#define ACCMD_DATA_FILE_ALLOCATE_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_DATA_FILE_ALLOCATE_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_DATA_FILE_ALLOCATE_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_DATA_FILE_ALLOCATE_RESP_FILE_ID_WORD_OFFSET (4)
#define ACCMD_DATA_FILE_ALLOCATE_RESP_FILE_ID_GET(addr) (*((addr) + 4))
#define ACCMD_DATA_FILE_ALLOCATE_RESP_FILE_ID_SET(addr, file_id) (*((addr) + 4) = (uint16)(file_id))
#define ACCMD_DATA_FILE_ALLOCATE_RESP_WORD_SIZE (5)
#define ACCMD_DATA_FILE_ALLOCATE_RESP_PACK(addr, Status, File_Id) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(Status)); \
        *((addr) + 4) = (uint16)((uint16)(File_Id)); \
    } while (0)

#define ACCMD_DATA_FILE_ALLOCATE_RESP_MARSHALL(addr, accmd_data_file_allocate_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_data_file_allocate_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_data_file_allocate_resp_ptr)->Status); \
        *((addr) + 4) = (uint16)((accmd_data_file_allocate_resp_ptr)->File_Id); \
    } while (0)

#define ACCMD_DATA_FILE_ALLOCATE_RESP_UNMARSHALL(addr, accmd_data_file_allocate_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_data_file_allocate_resp_ptr)->header)); \
        (accmd_data_file_allocate_resp_ptr)->Status = ACCMD_DATA_FILE_ALLOCATE_RESP_STATUS_GET(addr); \
        (accmd_data_file_allocate_resp_ptr)->File_Id = ACCMD_DATA_FILE_ALLOCATE_RESP_FILE_ID_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_clock_Power_Save_Mode_Resp

  DESCRIPTION
    Response for Accmd_clock_Power_Save_Mode_Req

  MEMBERS
    status                - The status of the request
    Audio_power_Save_Mode - Power save operation Mode

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER           header;
    ACCMD_STATUS           status;
    ACCMD_POWER_SAVE_MODES Audio_power_Save_Mode;
} ACCMD_CLOCK_POWER_SAVE_MODE_RESP;

/* The following macros take ACCMD_CLOCK_POWER_SAVE_MODE_RESP *accmd_clock_power_save_mode_resp_ptr or uint16 *addr */
#define ACCMD_CLOCK_POWER_SAVE_MODE_RESP_STATUS_WORD_OFFSET (3)
#define ACCMD_CLOCK_POWER_SAVE_MODE_RESP_STATUS_GET(addr) ((ACCMD_STATUS)*((addr) + 3))
#define ACCMD_CLOCK_POWER_SAVE_MODE_RESP_STATUS_SET(addr, status) (*((addr) + 3) = (uint16)(status))
#define ACCMD_CLOCK_POWER_SAVE_MODE_RESP_AUDIO_POWER_SAVE_MODE_WORD_OFFSET (4)
#define ACCMD_CLOCK_POWER_SAVE_MODE_RESP_AUDIO_POWER_SAVE_MODE_GET(addr) ((ACCMD_POWER_SAVE_MODES)*((addr) + 4))
#define ACCMD_CLOCK_POWER_SAVE_MODE_RESP_AUDIO_POWER_SAVE_MODE_SET(addr, audio_power_save_mode) (*((addr) + 4) = (uint16)(audio_power_save_mode))
#define ACCMD_CLOCK_POWER_SAVE_MODE_RESP_WORD_SIZE (5)
#define ACCMD_CLOCK_POWER_SAVE_MODE_RESP_PACK(addr, status, Audio_power_Save_Mode) \
    do { \
        *((addr) + 3) = (uint16)((uint16)(status)); \
        *((addr) + 4) = (uint16)((uint16)(Audio_power_Save_Mode)); \
    } while (0)

#define ACCMD_CLOCK_POWER_SAVE_MODE_RESP_MARSHALL(addr, accmd_clock_power_save_mode_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_clock_power_save_mode_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)((accmd_clock_power_save_mode_resp_ptr)->status); \
        *((addr) + 4) = (uint16)((accmd_clock_power_save_mode_resp_ptr)->Audio_power_Save_Mode); \
    } while (0)

#define ACCMD_CLOCK_POWER_SAVE_MODE_RESP_UNMARSHALL(addr, accmd_clock_power_save_mode_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_clock_power_save_mode_resp_ptr)->header)); \
        (accmd_clock_power_save_mode_resp_ptr)->status = ACCMD_CLOCK_POWER_SAVE_MODE_RESP_STATUS_GET(addr); \
        (accmd_clock_power_save_mode_resp_ptr)->Audio_power_Save_Mode = ACCMD_CLOCK_POWER_SAVE_MODE_RESP_AUDIO_POWER_SAVE_MODE_GET(addr); \
    } while (0)


/*******************************************************************************

  NAME
    Accmd_Call_Function_Resp

  DESCRIPTION
    From AudioSS This command indicates completion of an RPC request,
    carrying the return value, or at least the value in the register to which
    the return value would have been written if the function returned a
    value.  (It is up to the client to save or discard this value based on
    its knowledge of the function's signature.) NOTE: this is only
    implemented for Kalimba32 targets

  MEMBERS
    return_val -

*******************************************************************************/
typedef struct
{
    ACCMD_HEADER header;
    uint32       return_val;
} ACCMD_CALL_FUNCTION_RESP;

/* The following macros take ACCMD_CALL_FUNCTION_RESP *accmd_call_function_resp_ptr or uint16 *addr */
#define ACCMD_CALL_FUNCTION_RESP_RETURN_VAL_WORD_OFFSET (3)
#define ACCMD_CALL_FUNCTION_RESP_RETURN_VAL_GET(addr)  \
    (((uint32)(*((addr) + 3)) | \
      ((uint32)(*((addr) + 4)) << 16)))
#define ACCMD_CALL_FUNCTION_RESP_RETURN_VAL_SET(addr, return_val) do { \
        *((addr) + 3) = (uint16)((return_val) & 0xffff); \
        *((addr) + 4) = (uint16)((return_val) >> 16); } while (0)
#define ACCMD_CALL_FUNCTION_RESP_WORD_SIZE (5)
#define ACCMD_CALL_FUNCTION_RESP_PACK(addr, return_val) \
    do { \
        *((addr) + 3) = (uint16)((uint16)((return_val) & 0xffff)); \
        *((addr) + 4) = (uint16)(((return_val) >> 16)); \
    } while (0)

#define ACCMD_CALL_FUNCTION_RESP_MARSHALL(addr, accmd_call_function_resp_ptr) \
    do { \
        ACCMD_HEADER_MARSHALL((addr), &((accmd_call_function_resp_ptr)->header)); \
        *((addr) + 3) = (uint16)(((accmd_call_function_resp_ptr)->return_val) & 0xffff); \
        *((addr) + 4) = (uint16)(((accmd_call_function_resp_ptr)->return_val) >> 16); \
    } while (0)

#define ACCMD_CALL_FUNCTION_RESP_UNMARSHALL(addr, accmd_call_function_resp_ptr) \
    do { \
        ACCMD_HEADER_UNMARSHALL((addr), &((accmd_call_function_resp_ptr)->header)); \
        (accmd_call_function_resp_ptr)->return_val = ACCMD_CALL_FUNCTION_RESP_RETURN_VAL_GET(addr); \
    } while (0)


#endif /* ACCMD_PRIM_H__ */


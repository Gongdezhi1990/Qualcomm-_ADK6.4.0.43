/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.

FILE NAME
    operators.h

DESCRIPTION
    Kymera operator related helper functions.
    It consists of two APIs, low level one and higher level one.

    First API provides OperatorsCreate() function to create any operator and
    set of functions to configure operators properties. One function per property.

    Second API builds on top of low level one and provide function per operator to
    create and configure it in single call.
*/

#ifndef LIBS_OPERATORS_OPERATORS_H_
#define LIBS_OPERATORS_OPERATORS_H_

#include <operator.h>
#include <message.h>
#include <vmtypes.h>
#include <audio_sbc_encoder_params.h>
#include <audio_aptx_adaptive_encoder_params.h>
#include <app/ringtone/ringtone_if.h>

#define INVALID_OPERATOR ((Operator)0)

/* Default framework kick period */
#define DEFAULT_KICK_PERIOD (2000)

/* Low power graph kick period*/
#define LOW_POWER_GRAPH_KICK_PERIOD (7500)

/* Kick period macros */
#define KICK_PERIOD_MILLISECONDS        (2)
#define LATENCY_PER_KICK(latency_ms)    (latency_ms / KICK_PERIOD_MILLISECONDS)

/* Q6.10 format macros */
#define ONE_MILLISECOND_Q6_10                   (1 << 10)
#define MILLISECONDS_Q6_10(latency_ms)          (latency_ms * ONE_MILLISECOND_Q6_10)
#define MILLISECONDS_PER_KICK_Q6_10(latency_ms) ((uint16)(LATENCY_PER_KICK(MILLISECONDS_Q6_10(latency_ms))))

/* Q6.26 format macros */
#define ONE_MILLISECOND_Q6_26                   (1u << 26)
#define MILLISECONDS_Q6_26(latency_ms)          (latency_ms * ONE_MILLISECOND_Q6_26)
#define MILLISECONDS_PER_KICK_Q6_26(latency_ms) ((uint32)(LATENCY_PER_KICK(MILLISECONDS_Q6_26(latency_ms))))

typedef struct
{
    uint16 low_latency_0;
    uint16 low_latency_1;
    uint16 high_quality;
    uint16 tws_legacy;
} aptx_adaptive_ttp_in_ms_t;

typedef struct
{
    int16 primary_left;
    int16 primary_right;
    int16 secondary_left;
    int16 secondary_right;
} volume_trims_t;

typedef enum
{
    music_processing_mode_standby         = 1,
    music_processing_mode_full_processing = 2,
    music_processing_mode_passthrough     = 3
} music_processing_mode_t;

/*! @brief Built-in DSP capability IDs. 
   New entries in this enum must be added the config tool XML
   file sink_configure_dsp_capability_ids_def.xml. 
   Use config tool to remap built-in capability IDs to their downloadable equivalents. */
typedef enum
{
    capability_id_none                          = 0x00,
    capability_id_passthrough                   = 0x01,
    capability_id_sco_send                      = 0x03,
    capability_id_sco_receive                   = 0x04,
    capability_id_wbs_send                      = 0x05,
    capability_id_wbs_receive                   = 0x06,
    capability_id_mixer                         = 0x0a,
    capability_id_splitter                      = 0x13,
    capability_id_sbc_encoder                   = 0x14,
    capability_id_sbc_decoder                   = 0x16,
    capability_id_aac_decoder                   = 0x18,
    capability_id_aptx_decoder                  = 0x19,
    capability_id_cvc_hf_1mic_nb                = 0x1c,
    capability_id_cvc_receive_nb                = 0x1d,
    capability_id_cvc_hf_1mic_wb                = 0x1e,
    capability_id_cvc_receive_wb                = 0x1f,
    capability_id_cvc_hf_2mic_nb                = 0x20,
    capability_id_cvc_hf_2mic_wb                = 0x21,
    capability_id_cvc_hs_1mic_nb                = 0x23,
    capability_id_cvc_hs_1mic_wb                = 0x24,
    capability_id_cvc_hs_2mic_90deg_nb          = 0x25,
    capability_id_cvc_hs_2mic_90deg_wb          = 0x26,
    capability_id_cvc_hs_2mic_0deg_nb           = 0x27,
    capability_id_cvc_hs_2mic_0deg_wb           = 0x28,
    capability_id_cvc_spk_1mic_nb               = 0x29,
    capability_id_cvc_spk_1mic_wb               = 0x2a,
    capability_id_cvc_spk_2mic_0deg_nb          = 0x2d,
    capability_id_cvc_spk_2mic_0deg_wb          = 0x2e,
    capability_id_crossover_2band               = 0x33,
    capability_id_crossover_3band               = 0x34,
    capability_id_spdif_decoder                 = 0x36,
    capability_id_tone                          = 0x37,
    capability_id_ttp_passthrough               = 0x3c,
    capability_id_aptx_ll_decoder               = 0x3d,
    capability_id_aec_4mic                      = 0x43,
    capability_id_volume                        = 0x48,
    capability_id_peq                           = 0x49,
    capability_id_vse                           = 0x4a,
    capability_id_cvc_receive_uwb               = 0x53,
    capability_id_cvc_receive_swb               = 0x54,
    capability_id_cvc_hf_1_mic_uwb              = 0x56,
    capability_id_cvc_hf_1_mic_swb              = 0x57,
    capability_id_cvc_hf_2_mic_uwb              = 0x59,
    capability_id_cvc_hf_2_mic_swb              = 0x5a,
    capability_id_cvc_hs_1_mic_uwb              = 0x5c,
    capability_id_cvc_hs_1_mic_swb              = 0x5d,
    capability_id_cvc_hs_2_mic_uwb              = 0x5f,
    capability_id_cvc_hs_2_mic_swb              = 0x60,
    capability_id_cvc_hs_2_mic_binaural_uwb     = 0x62,
    capability_id_cvc_hs_2_mic_binaural_swb     = 0x63,
    capability_id_cvc_spk_1_mic_uwb             = 0x6b,
    capability_id_cvc_spk_1_mic_swb             = 0x6c,
    capability_id_cvc_spk_2_mic_uwb             = 0x6e,
    capability_id_cvc_spk_2_mic_swb             = 0x6f,
    capability_id_bass_enhance                  = 0x90,
    capability_id_compander                     = 0x92,
    capability_id_iir_resampler                 = 0x94,
    capability_id_vad                           = 0x95,
    capability_id_qva                           = 0x96,
    capability_id_rtp                           = 0x98,
    capability_id_source_sync                   = 0x99,
    capability_id_usb_audio_rx                  = 0x9a,
    capability_id_usb_audio_tx                  = 0x9b,
    capability_id_celt_encoder                  = 0x9c,
    capability_id_celt_decoder                  = 0x9d,
    capability_id_aptx_hd_decoder               = 0x9e,
    capability_id_aptx_mono_decoder             = 0xa9,
    capability_id_aptx_hd_mono_decoder          = 0xaa,
    capability_id_aptx_mono_decoder_no_autosync = 0xab,
    capability_id_aptx_hd_mono_decoder_no_autosync = 0xac,
    capability_id_aptx_demux                    = 0xb1,
    capability_id_anc_tuning                    = 0xb2,
    capability_id_rate_adjustment               = 0xb3,
    capability_id_async_wbs_encoder             = 0xb4,
    capability_id_switched_passthrough_consumer = 0xb6,
    capability_id_aptx_ad_demux                 = 0xb7,
    capability_id_aptx_ad_decoder               = 0xb8,
    capability_id_swbs_encode                   = 0xba,
    capability_id_swbs_decode                   = 0xbb,
    capability_id_graph_manager 			    = 0xbC,
    FORCE_ENUM_TO_MIN_16BIT(capability_id_t)
} capability_id_t;

typedef struct
{
    uint16 version_msb;
    uint16 version_lsb;
}capablity_version_t;

typedef enum
{
    splitter_mode_clone_input,
    splitter_mode_buffer_input
} splitter_working_mode_t;

typedef enum
{
    splitter_buffer_location_internal,
    splitter_buffer_location_sram
}splitter_buffer_location_t;

typedef enum
{
    splitter_output_streams_all,
    splitter_output_stream_0,
    splitter_output_stream_1
}splitter_output_stream_t;

typedef enum
{
    splitter_packing_unpacked,
    splitter_packing_packed
} splitter_packing_t;

typedef enum
{
    splitter_reframing_disable,
    splitter_reframing_enable
} splitter_reframing_enable_disable_t;


/*!
 * Codec types supported by the RTP decode capability
 */
typedef enum
{
    rtp_codec_type_aptx    = 0,
    rtp_codec_type_sbc     = 1,
    rtp_codec_type_atrac   = 2,
    rtp_codec_type_mp3     = 3,
    rtp_codec_type_aac     = 4,
    rtp_codec_type_aptx_hd = 5,
    rtp_codec_type_aptx_ad = 6
} rtp_codec_type_t;

typedef enum
{
    rtp_passthrough = 0, /* When no RTP header is present. */
    rtp_decode      = 1, /* Decodes the RTP header and calls the TTP module to get the time to play information. */
    rtp_strip       = 2, /* Decodes the RTP header and strips it. */
    rtp_ttp_only    = 3  /* Adds time to play information to encoded streams which have no RTP headers*/
} rtp_working_mode_t;


/* CELT decoder parameters. */
typedef struct
{
    uint32 sample_rate;
    uint16 frame_size;
} celt_encoder_params_t;

typedef enum
{
    operator_data_format_pcm,
    operator_data_format_encoded
} operator_data_format_t;

typedef enum
{
    operator_priority_lowest  = 0,
    operator_priority_low     = 1,
    operator_priority_medium  = 2,
    operator_priority_high    = 3,
    operator_priority_highest = 4
} operator_priority_t;

#define DEFAULT_OPERATOR_PRIORITY   (operator_priority_lowest)
typedef enum
{
    vad_working_mode_full_processing = 0x01,
    vad_working_mode_passthrough     = 0x02,
    vad_working_mode_force_trigger   = 0xff
}vad_working_mode_t;

typedef enum
{
    OPERATOR_PROCESSOR_ID_0 = 0,
    OPERATOR_PROCESSOR_ID_1 = 1
} operator_processor_id_t;

/*
    Structure for passing control settings
*/
typedef struct __operator_set_control_data
{
    unsigned control_id;
    unsigned value;
} operator_set_control_data_t;

/*
    Structure for passing channel information
*/
typedef struct __mixer_channel_gain
{
    unsigned channel_id;
    int gain;
} mixer_channel_gain_t;

/* types for passing parameter ids and values
*/
typedef uint16 standard_param_id_t;
typedef uint32 standard_param_value_t;

/*
    Structure for passing parameter values
*/
typedef struct
{
    standard_param_id_t id;
    standard_param_value_t value;
} standard_param_t;

typedef struct
{
    unsigned number_of_params;
    standard_param_t standard_params[];
} set_params_data_t;

typedef enum
{
    obpm_ok,
    obpm_too_big,
    obpm_not_ready,
    obpm_invalid_parameter,
    obpm_unsupport_control,
    obpm_unsupported_mode,
    obpm_unsupported_encoding
} obpm_result_state_t;

typedef struct
{
    obpm_result_state_t result;
    unsigned number_of_params;
    standard_param_t standard_params[];
} get_params_data_t;

/*
    Structure for a source sync source group
*/
typedef struct __source_sync_source_group
{
    bool        meta_data_required;
    bool        ttp_required;
    uint32      channel_mask;
} source_sync_source_group_t;

/*
    Structure for a source sync sink group
*/
typedef struct __source_sync_sink_group
{
    bool        meta_data_required;
    bool        rate_match;
    uint32      channel_mask;
} source_sync_sink_group_t;

/*
    Structure for a source sync route
*/
typedef struct __source_sync_route
{
    uint16      input_terminal;
    uint16      output_terminal;
    uint32      transition_samples;
    uint32      sample_rate;
    int16       gain;
} source_sync_route_t;

typedef enum
{
    qva_not_detected = 0x0,
    qva_detected = 0x01
} qva_detected_status_t;

/*
    Key for items which can be configured when an operator is created.

    NOTE. operators_setup_buffer_latency will not be enacted on creation but when
          OperatorsSetBufferSizeFromSampleRate is called as it requires sample
          rate to calculate the required buffer size.
*/
typedef enum
{
    operators_setup_buffer_size,
    operators_setup_parameter,
    operators_setup_buffer_latency,
    operators_setup_usb_config,
    operators_setup_sample_rate,
    operators_setup_switched_passthrough_set_format,
    operators_setup_swb_decode_codec_mode,
    operators_setup_swb_encode_codec_mode
} operator_setup_key_t;

typedef struct
{
    unsigned sample_rate;
    unsigned sample_size;
    unsigned number_of_channels;
} usb_config_t;

typedef enum
{
   spc_op_mode_passthrough = 0,
   spc_op_mode_consumer
}spc_mode_t;

typedef enum
{
   spc_op_format_encoded = 0,
   spc_op_format_pcm
}spc_format_t;

typedef enum
{
    spc_op_consume_all_inputs = 0,
    spc_op_select_passthrough_input_1,
    spc_op_select_passthrough_input_2,
    spc_op_select_passthrough_input_3,
    spc_op_select_passthrough_input_4,
    spc_op_select_passthrough_input_5,
    spc_op_select_passthrough_input_6,
    spc_op_select_passthrough_input_7,
    spc_op_select_passthrough_input_8
}spc_select_passthrough_input_t;

typedef enum
{
    swb_codec_mode_swb = 0,
    swb_codec_mode_uwb = 4
} swb_codec_mode_t;

/*
    Values to be applied when an item is configured
*/
typedef union
{
    unsigned buffer_size;
    unsigned buffer_latency;
    standard_param_t parameter;
    usb_config_t usb_config;
    unsigned sample_rate;
    spc_format_t spc_format;
    swb_codec_mode_t codec_mode;
} operator_setup_value_t;

/*
    Key/Value pair for operator configuration items
*/
typedef struct
{
    operator_setup_key_t key;
    operator_setup_value_t value;
} operator_setup_item_t;

/*
    A list of items to be configured when an operator is created.
*/
typedef struct
{
    unsigned num_items;
    const operator_setup_item_t* items;
} operator_setup_t;

typedef enum {
    aptx_ad_ll_0_ssrc_id = 0xA1,
    aptx_ad_ll_1_ssrc_id = 0xA2,
    aptx_ad_hq_ssrc_id = 0xA3,
    aptx_ad_tws_ssrc_id = 0xA4
} aptx_adaptive_ssrc_ids_t;

typedef struct
{
    aptx_adaptive_ssrc_ids_t ssrc_id;
    uint32  target_latency;
} aptx_ad_mode_notification_t;

/* Macros to help create operator_setup_item_t */
#define SOURCE_SYNC_PARAM_ID_LATENCY    (2)

#define OPERATORS_SETUP_SOURCE_SYNC_LATENCY(latency_ms) \
{ \
    .key = operators_setup_parameter, \
    .value = {.parameter = {SOURCE_SYNC_PARAM_ID_LATENCY, MILLISECONDS_PER_KICK_Q6_26(latency_ms)}} \
}

#define OPERATORS_SETUP_STANDARD_BUFFER_SIZE(size) \
{ \
    .key = operators_setup_buffer_size, \
    .value = {.buffer_size = (size)} \
}

#define OPERATORS_SETUP_STANDARD_BUFFER_SIZE_FROM_LATENCY(latency_ms) \
{ \
    .key = operators_setup_buffer_latency, \
    .value = {.buffer_latency = (latency_ms)} \
}

#define OPERATORS_SETUP_STANDARD_SAMPLE_RATE(rate) \
{ \
    .key = operators_setup_sample_rate, \
    .value = {.sample_rate = (rate)} \
}

#define OPERATORS_SETUP_SWITCHED_PASSTHROUGH_FORMAT(format) \
{ \
    .key = operators_setup_switched_passthrough_set_format, \
    .value = {.spc_format = (format)} \
}

/****************************************************************************
    Low level API
*/

/****************************************************************************
DESCRIPTION
    Enable the audio framework.
    It enables the framework, or it does not return at all.
*/
void OperatorsFrameworkEnable(void);

/****************************************************************************
DESCRIPTION
    Disable the audio framework.
    It disables the framework, or it does not return at all.
*/
void OperatorsFrameworkDisable(void);

/****************************************************************************
DESCRIPTION
    Set system wide kick period in microseconds.
    Valid values are between 1000 and 20000.
*/
bool OperatorsFrameworkSetKickPeriod(unsigned kick_period);

/****************************************************************************
DESCRIPTION
    Create the operator defined in id.
RETURNS
    A valid operator or INVALID_OPERATOR on failure.
*/
Operator OperatorsCreate(capability_id_t id, operator_processor_id_t processor_id, operator_priority_t priority);

/****************************************************************************
DESCRIPTION
    Create an operator and send a sequence of configuration messages as defined
    by the config parameter
RETURNS
    A valid operator or INVALID_OPERATOR on failure.
*/
Operator OperatorsCreateWithSetup(capability_id_t id, operator_processor_id_t processor_id, operator_priority_t priority, const operator_setup_t* config);

/*!
 * @brief Destroys all the operators passed as input, panics if it fails to do so.
 *        The operators must be stopped before they can be destroyed.
 *
 * @param operators Pointer to an array with the operators to be destroyed.
 * @param number_of_operators The length of operators array.
 */
void OperatorsDestroy(Operator *operators, unsigned number_of_operators);


/*!
 *  @brief    Set the resampler conversion rate.

    Sample rates are in Hz, but must come from a fixed set of rates :- 8000,
    11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000

    @param opid                 Operator to configure
    @param input_sample_rate    Sample rate of the input connection(s), from the 
                                supported range
    @param output_sample_rate   Desired sample rate of the output connection(s), 
                                from the supported range
*/
void OperatorsLegacyResamplerSetConversionRate(Operator opid, unsigned input_sample_rate, 
                                               unsigned output_sample_rate);

/****************************************************************************
DESCRIPTION
    Set IIR resampler conversion rate.
    Sample rates are in Hz.
*/
void OperatorsResamplerSetConversionRate(Operator opid, unsigned input_sample_rate, 
                                         unsigned output_sample_rate);

/****************************************************************************
DESCRIPTION
    Set tone to generate.
*/
void OperatorsToneSetNotes(Operator opid, const ringtone_note * tone);

/****************************************************************************
DESCRIPTION
    Set the splitter stream
*/
void OperatorsSplitterSetRunningStreams(Operator opid, splitter_output_stream_t stream_output, bool is_activate);

/****************************************************************************
DESCRIPTION
    Enable or disable second splitter output.
*/
void OperatorsSplitterEnableSecondOutput(Operator opid, bool is_second_output_active);

/****************************************************************************
DESCRIPTION
    activate splitter output
*/
void OperatorsSplitterActivateOutputStream(Operator opid, splitter_output_stream_t stream_output);

/****************************************************************************
DESCRIPTION
    deactivate splitter output
*/
void OperatorsSplitterDeactivateOutputStream(Operator opid, splitter_output_stream_t stream_output);

/****************************************************************************
DESCRIPTION
    Activate the streams after timestamp.The start_timestamp would give the exact position
    in the buffer from which the stream would start processing.
*/
void OperatorsSplitterActivateOutputStreamAfterTimestamp(Operator op, uint32 start_timestamp, splitter_output_stream_t stream_output);

/****************************************************************************
DESCRIPTION
    Set splitter data format.
    Data format is PCM or anything else.
*/
void OperatorsSplitterSetDataFormat(Operator opid, operator_data_format_t data_format);

/****************************************************************************
DESCRIPTION
    Set splitter working mode
    Data from the input buffer may be cloned to the output buffers, or it may
    be buffered separately. Data must be buffered separately to use external
    SRAM buffers or to pack data in the input buffer.
*/
void OperatorsSplitterSetWorkingMode(Operator op, splitter_working_mode_t mode);

/****************************************************************************
DESCRIPTION
    Set buffer location for splitter
    Data must be buffered separately to use external SRAM buffers or to pack
    data in the input buffer.
*/
void OperatorsSplitterSetBufferLocation(Operator op, splitter_buffer_location_t buf_loc);
/****************************************************************************
DESCRIPTION
    Data must be buffered separately in some cases like QVA/VAD
*/
void OperatorsSplitterBufferOutputStream(Operator op, splitter_output_stream_t stream);

/****************************************************************************
DESCRIPTION
    Set splitter packing
    Reduce buffer usage by packing only the most significant 16 bits of audio
    data into the splitter input buffer. This can only be used if the data 
    format has been set to operator_data_format_pcm and the working mode has
    been set to splitter_mode_buffer_input.
*/
void OperatorsSplitterSetPacking(Operator op, splitter_packing_t packing);

/****************************************************************************
DESCRIPTION
    Set if splitter needs to reframe the metadata or not
*/
void OperatorsSplitterSetMetadataReframing(Operator op, splitter_reframing_enable_disable_t state, uint16 size);

/****************************************************************************
DESCRIPTION
    Configure RTP working mode.
*/
void OperatorsRtpSetWorkingMode(Operator op, rtp_working_mode_t mode);

/****************************************************************************
DESCRIPTION
    Configure RTP to use/not use content protection.
*/
void OperatorsRtpSetContentProtection(Operator op, bool protection_enabled);

/****************************************************************************
DESCRIPTION
    Set RTP codec type.
*/
void OperatorsRtpSetCodecType(Operator op, rtp_codec_type_t codec_type);

/****************************************************************************
DESCRIPTION
    Set target TTP latency for all aptX Adaptive modes when in Non-Q2Q mode (as opposed to Q2Q).
    To be used on a RTP decode operator.
*/
void OperatorsRtpSetAptxAdaptiveTTPLatency(Operator rtp_op, aptx_adaptive_ttp_in_ms_t aptx_ad_ttp);

/****************************************************************************
DESCRIPTION
    Enable aptX Adaptive mode switch notifications when in Non-Q2Q mode (as opposed to Q2Q).
    Each time the mode changes, the specified Task will be called with a message that includes the
    current mode and target TTP latency used.
    The mapping between target TTP latency and mode has to be provided to the RTP decode operator beforehand
    using OperatorsRtpSetAptxAdaptiveTTPLatency().
    To be used on a RTP decode operator.
*/
void OperatorsRtpEnableAptxAdModeNotifications(Operator rtp_op, Task notification_handler);

/****************************************************************************
DESCRIPTION
    Set AAC Codec associated with an RTP capability.
*/
void OperatorsRtpSetAacCodec(Operator op, Operator aac_op);

/****************************************************************************
DESCRIPTION
    Set maximum RTP packet length (in octets) of an RTP capability.
*/
void OperatorsRtpSetMaximumPacketLength(Operator op, uint16 packet_length_in_octets);

/****************************************************************************
DESCRIPTION
    Set number of channels for each of mixer streams.
*/
void OperatorsMixerSetChannelsPerStream(Operator op, unsigned str1_ch, unsigned str2_ch, unsigned str3_ch);

/****************************************************************************
DESCRIPTION
    Set gain for each of mixer streams.
    Gain is represented in standard scaled dB.
    Valid values are 0 or negative integer.
    When the mixer is already running gain changes will ramp up/down over a
    number of samples controlled by OperatorsStandardSetNumberOfSamplesToRamp
*/
void OperatorsMixerSetGains(Operator op, int str1_gain, int str2_gain, int str3_gain);

/****************************************************************************
DESCRIPTION
    Set mixer primary stream. Primary stream must be connected before mixer is started.
    Valid values are 0, 1, 2.
*/
void OperatorsMixerSetPrimaryStream(Operator op, unsigned primary_stream);

/****************************************************************************
DESCRIPTION
    Set the number of samples over which to ramp up/down when changing input
    gain settings with OperatorsMixerSetGains
*/
void OperatorsMixerSetNumberOfSamplesToRamp(Operator op, unsigned number_of_samples);

/****************************************************************************
DESCRIPTION
    Set the master gain for the volume operator
    gain is specified in dB/60 and in the range of -90dB to 90dB
*/
void OperatorsVolumeSetMainGain(Operator op, int gain);

/****************************************************************************
DESCRIPTION
    Set the post gain for the volume operator
    gain is specified in dB/60 and in the range of -90dB to 90dB
*/
void OperatorsVolumeSetPostGain(Operator op, int gain);

/****************************************************************************
DESCRIPTION
    Send the control message to Mute/Unmute at the volume control
*/
void OperatorsVolumeMute(Operator op,bool enable);

/****************************************************************************
DESCRIPTION
    Set the aux gain for the volume operator
    gain is specified in dB/60 and in the range of -90dB to 90dB
*/
void OperatorsVolumeSetAuxGain(Operator op, int gain);

/****************************************************************************
DESCRIPTION
    Set the trims to apply to individual output channels of the volume operator
    trims are specified in dB/60 and in the range of -90dB to 90dB
*/
void OperatorsVolumeSetTrims(Operator op, const volume_trims_t* trims);

/****************************************************************************
DESCRIPTION
    Set the main and aux gain for the volume operator in a single control
    message.
    gain is specified in dB/60 and in the range of -90dB to 90dB
*/
void OperatorsVolumeSetMainAndAuxGain(Operator op, int gain);

/****************************************************************************
DESCRIPTION
    Send a message to the aec to configure the input and output rates.
 */
void OperatorsAecSetSampleRate(Operator op, unsigned in_rate, unsigned out_rate);

/****************************************************************************
DESCRIPTION
    Send a message to mute aec mic output.
 */
void OperatorsAecMuteMicOutput(Operator op, bool enable);

/****************************************************************************
DESCRIPTION
    Send a message to configure the AEC operator to use Time To Play (TTP
    information on the speaker output.
 */
void OperatorsAecEnableTtpGate(Operator op, bool enable, uint16 initial_delay_ms, bool control_drift);

/****************************************************************************
DESCRIPTION
    Send a message to the aec to configure the task period
 */
void OperatorsAecSetTaskPeriod(Operator op, uint16 period, uint16 decim_factor);

/****************************************************************************
DESCRIPTION
    Send a message to the spdif decoder to configure its output sample rate.
 */
void OperatorsSpdifSetOutputSampleRate(Operator op, unsigned sample_rate);

/****************************************************************************
DESCRIPTION
     Set Usb Audio RX config.
     Sample size is expressed in bytes.
 */
void OperatorsUsbAudioSetConfig(Operator op, usb_config_t config);

/****************************************************************************
DESCRIPTION
     Set SBC encoder encoding parameters.
 */
void OperatorsSbcEncoderSetEncodingParams(Operator op, sbc_encoder_params_t *params);

/****************************************************************************
DESCRIPTION
     Set mSBC encoder bitpool value
 */

void OperatorsMsbcEncoderSetBitpool(Operator op, uint16 bitpool_value);

/****************************************************************************
DESCRIPTION
    Send a message to the specified operator to set its mode.
    Only the Bass Enhance and Virtual Stereo Enhance Operators support this message.
    There are no checks on the operator type.
 */
void OperatorsSetMusicProcessingMode(Operator op, music_processing_mode_t mode);

/****************************************************************************
DESCRIPTION
    Send a message to the specified operator to set its input and output data format.
    This message is valid only for passthrough and ttp passthrough operators.
 */
void OperatorsSetPassthroughDataFormat(Operator op, operator_data_format_t data_format);

/****************************************************************************
DESCRIPTION
    Send a message to the specified operator to set its gain in dB.
    This message is valid only for the passthrough operator.
    The gain in dB must be scaled by multiplying by DB_DSP_SCALING_FACTOR.
 */
void OperatorsSetPassthroughGain(Operator op, int32 gain_db_scaled);


/****************************************************************************
DESCRIPTION
    Send a message to the specified operator to set its buffer size.
    Buffer size is expressed in bytes.
    Note:   This API is deprecated. Use OperatorsStandardSetBufferSizeWithFormat() instead.
 */
void OperatorsStandardSetBufferSize(Operator op, unsigned buffer_size);

/****************************************************************************
DESCRIPTION
    Send a message to the specified operator to set its buffer size based on the data format.
    Buffer size is expressed in bytes.
 */
void OperatorsStandardSetBufferSizeWithFormat(Operator op, unsigned buffer_size, operator_data_format_t format);

/****************************************************************************
DESCRIPTION
     Set mixer sample rate.
     Sample rate is in Hz.
 */
void OperatorsStandardSetSampleRate(Operator op, unsigned sample_rate);

/****************************************************************************
DESCRIPTION
     This function is used to help set up buffer size based on sample rate and
     required latency in milliseconds. Nothing will happen when this function is
     called unless config contains an entry for operators_setup_buffer_latency
 */
void OperatorsStandardSetBufferSizeFromSampleRate(Operator op, uint32 sample_rate, const operator_setup_t* setup);

/****************************************************************************
DESCRIPTION
    Send a generic Fade Out message to the specified operator. Depending upon
    the parameter, either an enable_fade_out or a disable_fade_out msg will be sent.
 */
void OperatorsStandardFadeOut(Operator op, bool enable);

/****************************************************************************
DESCRIPTION
    Send a control message to the specified operator
 */
void OperatorsStandardSetControl(Operator op, unsigned control_id, unsigned value);

/****************************************************************************
DESCRIPTION
    Send multiple control messages to the specified operator
 */
void OperatorsStandardSetControlMultiple(Operator op, unsigned count, const operator_set_control_data_t *controls);

/****************************************************************************
DESCRIPTION
    Get the capablity version of the specified operator
 */
capablity_version_t OperatorGetCapabilityVersion(Operator op);

/****************************************************************************
DESCRIPTION
    Set time to play latency. This specifies the delay after which audio should
    be played. The time_to_play value is expressed in microseconds.
 */
void OperatorsStandardSetTimeToPlayLatency(Operator op, uint32 ttp_latency);

/****************************************************************************
DESCRIPTION
    Tell the operator the minimum and maximum latency for time-to-play.
    The TTP generator will be reset if the latency exceeds these bounds.
    A zero value for the maximum latency means there is no upper bound.
    Minimum and maximum latency are expressed microseconds.
 */
void OperatorsStandardSetLatencyLimits(Operator op, uint32 minimum_latency, uint32 maximum_latency);

/****************************************************************************
DESCRIPTION
    Set the unique identifier associated with an operator.
 */
void OperatorsStandardSetUCID(Operator op, unsigned ucid);

/****************************************************************************
DESCRIPTION
    Sends a set of parameters to the specified operator.
    Note: Only 32 bit encoding of parameter values is supported.
 */
void OperatorsStandardSetParameters(Operator op, const set_params_data_t* set_params_data);

/****************************************************************************
DESCRIPTION
    Sends a list of parameters to the specified operator so that the operator can return their values.
    Note: Only 32 bit encoding of parameter values is supported.
 */
void OperatorsStandardGetParameters(Operator op, get_params_data_t* get_params_data);

/****************************************************************************
DESCRIPTION
    Set the gain of a specified CHANNELS
 */
void OperatorsMixerSetChannelsGains(Operator op,uint16 number_of_channels,const mixer_channel_gain_t *channels_gains);

/****************************************************************************
DESCRIPTION
    Set which inputs are members of a source sync sink group
 */
void OperatorsSourceSyncSetSinkGroups(Operator op, uint16 number_of_groups, const source_sync_sink_group_t* groups);

/****************************************************************************
DESCRIPTION
    Set which inputs are members of a source sync source group
 */
void OperatorsSourceSyncSetSourceGroups(Operator op, uint16 number_of_groups, const source_sync_source_group_t* groups);

/****************************************************************************
DESCRIPTION
    Set up routes through the source sync operator
 */
void OperatorsSourceSyncSetRoutes(Operator op, uint16 number_of_routes, const source_sync_route_t* routes);

/****************************************************************************
    Higher level API
*/

/****************************************************************************
DESCRIPTION
    Configure a resampler capability
 */
void OperatorsConfigureResampler(Operator op, unsigned input_sample_rate, unsigned output_sample_rate);

/****************************************************************************
DESCRIPTION
    Configure a tone generator capability.
 */
void OperatorsConfigureToneGenerator(Operator op, const ringtone_note *tone, Task listener);

/****************************************************************************
DESCRIPTION
    Configure a splitter capability and use SRAM for buffering   
    Note: If the board is having SRAM , this API shall be used with use_sram = TRUE
    OperatorsConfigureSplitter() shall be deprecated soon
 */
void OperatorsConfigureSplitterWithSram(Operator splitter_op, unsigned buffer_size, bool is_second_output_active,
        operator_data_format_t data_format,bool use_sram);
        
/****************************************************************************
DESCRIPTION
    Configure a splitter capability.
 */
void OperatorsConfigureSplitter(Operator op, unsigned buffer_size, bool is_second_output_active,
        operator_data_format_t data_format);

/****************************************************************************
DESCRIPTION
    Configure an sbc encoder capability.
 */
void OperatorsConfigureSbcEncoder(Operator op, sbc_encoder_params_t *params);

/****************************************************************************
DESCRIPTION
     Set Aptx Adaptive encoder encoding parameters.
 */
void OperatorsAptxAdEncoderSetEncodingParams(Operator op, aptxad_encoder_params_t *params);


/****************************************************************************
DESCRIPTION
    Configure an spdif decoder capability.
    message_handler is the task that will receive spdif decoders messages.
 */
void OperatorsConfigureSpdifDecoder(Operator op, Task message_handler);

/****************************************************************************
DESCRIPTION
    Configure an RTP capability.
 */
void OperatorsConfigureRtp(Operator op, rtp_codec_type_t codec_type, bool protection_enabled, rtp_working_mode_t mode);

/****************************************************************************
DESCRIPTION
    Configure a mixer capability.
 */
void OperatorsConfigureMixer(Operator op, unsigned sample_rate, unsigned primary_stream,
        int str1_gain, int str2_gain, int str3_gain,
        unsigned str1_ch, unsigned str2_ch, unsigned str3_ch);

/****************************************************************************
DESCRIPTION
    Configure a ttp passthrough capability. It can be used to add
    timestamps to the audio samples.
    ttp_latency is expressed in microseconds.
 */
void OperatorsConfigureTtpPassthrough(Operator op, unsigned ttp_latency, unsigned sample_rate,
        operator_data_format_t data_format);

/****************************************************************************
DESCRIPTION
    Configure an aec capability.
 */
void OperatorsConfigureAEC(Operator op, unsigned in_rate, unsigned out_rate);

/****************************************************************************
DESCRIPTION
    Configure a usb audio receive capability.
 */
void OperatorsConfigureUsbAudio(Operator usb_audio_op, usb_config_t config);

/****************************************************************************
DESCRIPTION
    Configure an QVA trigger phrase.
 */
void OperatorsConfigureQvaTriggerPhrase(Operator qva_op, DataFileID qva_model);

/****************************************************************************
DESCRIPTION
    Set QVA detection status
 */
void OperatorsSetQvaResetStatus(Operator qva_op, qva_detected_status_t detected_status);

/****************************************************************************
DESCRIPTION
    Helper function to create a set_params_data_t object
 */
set_params_data_t* OperatorsCreateSetParamsData(unsigned number_of_params);

/****************************************************************************
DESCRIPTION
    Helper function to create a get_params_data_t object
 */
get_params_data_t* OperatorsCreateGetParamsData(unsigned number_of_params);

/****************************************************************************
DESCRIPTION
    Configure a celt capability.
*/
void OperatorsCeltEncoderSetEncoderParams(Operator op, celt_encoder_params_t *params);

/****************************************************************************
DESCRIPTION
    Configure an QVA trigger phrase length.
 */
void OperatorsSetQvaMinMaxPhraseLen(Operator qva_op, unsigned min, unsigned max);

/****************************************************************************
DESCRIPTION
    Configure a Switched Passthrough Consumer Mode.
*/
void OperatorsSetSwitchedPassthruMode(Operator  spc_op, spc_mode_t mode);

/****************************************************************************
DESCRIPTION
    Configure a Switched Passthrough Consumer Format.
*/
void OperatorsSetSwitchedPassthruEncoding(Operator  spc_op, spc_format_t format);

/****************************************************************************
DESCRIPTION
    Parses the Operator message and returns the sturcture which contains the SSRC_ID and target latency.
*/
aptx_ad_mode_notification_t OperatorsRtpGetAptxAdModeNotificationInfo(const MessageFromOperator *op_msg);

/****************************************************************************
DESCRIPTION
    Configure SPC buffer size.
 */
void OperatorsSetSpcBufferSize(Operator spc_op, unsigned buffer_size);

/****************************************************************************
DESCRIPTION
    Configure a Switched Passthrough Consumer Format.
*/
void OperatorsSpcSelectPassthroughInput(Operator op, spc_select_passthrough_input_t input);

/****************************************************************************
DESCRIPTION
    Configure VAD working mode.
 */
void OperatorsVadSetWorkingMode(Operator op, vad_working_mode_t mode);

/****************************************************************************
DESCRIPTION
    Enable omni mode in multi-mic (2 or more) cVc send operator. The operator uses
    primary microphone for cVc processing.
 */
void OperatorsCvcSendEnableOmniMode(Operator cvc_snd_op);

/****************************************************************************
DESCRIPTION
    Disable omni mode in multi-mic (2 or more) cVc send operator.
 */
void OperatorsCvcSendDisableOmniMode(Operator cvc_snd_op);

/****************************************************************************
DESCRIPTION
    Configure a Passthrough capability.
 */
void OperatorsConfigurePassthrough(Operator op, unsigned buffer_size, operator_data_format_t data_format);

/****************************************************************************
DESCRIPTION
    Set the codec mode for the SWB encode operator.
 */
void OperatorsSwbEncodeSetCodecMode(Operator swb_encode_op, swb_codec_mode_t codec_mode);

/****************************************************************************
DESCRIPTION
    Set the codec mode for the SWB decode operator.
 */
void OperatorsSwbDecodeSetCodecMode(Operator swb_decode_op, swb_codec_mode_t codec_mode);

#endif /* LIBS_OPERATORS_OPERATORS_H_ */

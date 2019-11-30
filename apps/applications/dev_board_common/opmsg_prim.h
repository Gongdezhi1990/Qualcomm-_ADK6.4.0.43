/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*    */
/*****************************************************************************

            (c) Qualcomm Technologies International, Ltd. 2016
            Confidential information of Qualcomm

            Refer to LICENSE.txt included with this source for details
            on the license terms.

            WARNING: This is an auto-generated file!
                     DO NOT EDIT!

*****************************************************************************/
#ifndef OPMSG_PRIM_H
#define OPMSG_PRIM_H


#define OP_TERMINAL_CBOPS_PARAMETERS                                    (0x0001)
#define OP_TERMINAL_DATA_FORMAT                                         (0x0002)
#define OP_TERMINAL_DETAILS                                             (0x0003)
#define OP_TERMINAL_KICK_PERIOD                                         (0x0004)
#define OP_TERMINAL_BLOCK_SIZE                                          (0x0005)
#define OP_TERMINAL_PROC_TIME                                           (0x0006)
#define OP_TERMINAL_RATEMATCH_ABILITY                                   (0x0007)
#define OP_TERMINAL_RATEMATCH_RATE                                      (0x0008)
#define OP_TERMINAL_RATEMATCH_ENACTING                                  (0x0009)
#define OP_TERMINAL_RATEMATCH_ADJUSTMENT                                (0x000A)
#define SPLITTER_STREAM_0                                               (0x0001)
#define SPLITTER_STREAM_1                                               (0x0002)


/*******************************************************************************

  NAME
    OPMSG_AAC_ID

  DESCRIPTION
    AAC configuration messages

 VALUES
    SET_FRAME_TYPE - Sets the frame type  for AAC Decoder

*******************************************************************************/
typedef enum
{
    OPMSG_AAC_ID_SET_FRAME_TYPE = 0x0002
} OPMSG_AAC_ID;
/*******************************************************************************

  NAME
    OPMSG_AD2P_DEC_ID

  DESCRIPTION
    A2DP decoder configuration message

 VALUES
    CONTENT_PROTECTION_ENABLE - This message sets enables A2DP content protection
    OPMSG_SET_CTRL            - Sends a pointer to the endpoint stall flag

*******************************************************************************/
typedef enum
{
    OPMSG_AD2P_DEC_ID_CONTENT_PROTECTION_ENABLE = 0x0001,
    OPMSG_SET_CTRL = 0x00FE
} OPMSG_AD2P_DEC_ID;
/*******************************************************************************

  NAME
    OPMSG_AEC_REFERENCE_ID

  DESCRIPTION
    AEC Reference configuration messages

 VALUES
    SET_SAMPLE_RATES - This input message allows a user to set the input &
                       output sample rate.

*******************************************************************************/
typedef enum
{
    OPMSG_AEC_REFERENCE_ID_SET_SAMPLE_RATES = 0x00FE
} OPMSG_AEC_REFERENCE_ID;
/*******************************************************************************

  NAME
    OPMSG_CHANNEL_MIXER_ID

  DESCRIPTION
    Channel Mixer configuration messages

 VALUES
    SET_STREAM_PARAMETERS - Set the parameters for channel mixer

*******************************************************************************/
typedef enum
{
    OPMSG_CHANNEL_MIXER_ID_SET_STREAM_PARAMETERS = 0x0001
} OPMSG_CHANNEL_MIXER_ID;
/*******************************************************************************

  NAME
    OPMSG_COMMON_ID

  DESCRIPTION
    The set of Operator message IDs shared between all capabilities. Standard
    operator message IDs that all operators must support has the reserved
    range of 0x1000 - 0x1FFF. Common message IDs that an operator may choose
    to support has the reserved range of 0x2000 - 0x2FFF

 VALUES
    OPMSG_COMMON_ID_GET_CAPABILITY_VERSION
                   - Request the capability version of an operator. The response
                     contains the Major and Minor version number.
    OPMSG_COMMON_ID_FADEOUT_ENABLE
                   - This message starts the fade out function for the
                     capability.
    OPMSG_COMMON_ID_FADEOUT_DISABLE
                   - This message stops the fade out function for the
                     capability.
    OPMSG_COMMON_ID_SET_CONTROL
                   -
    OPMSG_COMMON_ID_GET_PARAMS
                   -
    OPMSG_COMMON_ID_GET_DEFAULTS
                   -
    OPMSG_COMMON_ID_SET_PARAMS
                   -
    OPMSG_COMMON_ID_GET_STATUS
                   -
    OPMSG_COMMON_ID_SET_UCID
                   - Message ID used to set use case ID.
    OPMSG_COMMON_ID_GET_LOGICAL_PS_ID
                   - Message ID used to get logical ID from set of cap, use case
                     and sub-block IDs.
    OPMSG_COMMON_CONFIGURE
                   - Configure message for an operator endpoint.
    OPMSG_COMMON_GET_CONFIGURATION
                   - Get configuration message for an operator endpoint.
    OPMSG_COMMON_GET_CLOCK_ID
                   - Get the clock id of a real operator endpoint. Operator
                     endpoints are only real when they are the start or end of a
                     chain.
    OPMSG_COMMON_ID_SET_BUFFER_SIZE
                   - Set the buffer size a capability should use, could be
                     interpreted as to request at connect or for some internal
                     use. Format is single word: an unsigned value for the
                     buffer size
    OPMSG_COMMON_ID_SET_TERMINAL_BUFFER_SIZE
                   - Set the buffer size a capability requests at a given
                     terminal or set of terminals. Format is 3 words: buffer
                     size to request, 16bit field sink terminals, 16bit field
                     source terminals. terminal bit fields bit 0...n represents
                     terminal 0..n.
    OPMSG_COMMON_SET_SAMPLE_RATE
                   - Set the sample rate of an operator Format(1word):
                     word0=fs/25,
    OPMSG_COMMON_SET_DATA_STREAM_BASED
                   - Tell the operator whether the data at its terminals can be
                     considered to be multi-channel streams. Operators that
                     support this do so because they can enable performance
                     (MIPS) improvements if this is true. Format(1word):
                     word0=boolean 1 = Data is multi-channel stream(s) 0 = Data
                     isn't
    OPMSG_COMMON_GET_RM_ABILITY
                   - Ask the operator whether it can perform ratematching.
                     Operators that can perform rate adjustment report in the
                     response a value from the stream RATEMATCHING_SUPPORT enum
    OPMSG_COMMON_SET_RM_ENACTING
                   - Tell the operator whether it is to perform rate adjustment,
                     or when to cease performing rate adjustment.
                     Format(2words): word0=boolean TRUE-perform rate adjustment,
                     FALSE-don't perform rate adjustment. word1=pointer to the
                     memory location where the amount of adjustment to apply can
                     be found
    OPMSG_COMMON_SET_TTP_LATENCY
                   - Tell the operator its target latency for time-to-play
                     calculations Format(2words): 32-bit latency in microseconds
    OPMSG_COMMON_SET_TTP_PARAMS
                   - Update the time-to-play parameters used by the operator

*******************************************************************************/
typedef enum
{
    OPMSG_COMMON_ID_GET_CAPABILITY_VERSION = 0x1000,
    OPMSG_COMMON_ID_FADEOUT_ENABLE = 0x2000,
    OPMSG_COMMON_ID_FADEOUT_DISABLE = 0x2001,
    OPMSG_COMMON_ID_SET_CONTROL = 0x2002,
    OPMSG_COMMON_ID_GET_PARAMS = 0x2003,
    OPMSG_COMMON_ID_GET_DEFAULTS = 0x2004,
    OPMSG_COMMON_ID_SET_PARAMS = 0x2005,
    OPMSG_COMMON_ID_GET_STATUS = 0x2006,
    OPMSG_COMMON_ID_SET_UCID = 0x2007,
    OPMSG_COMMON_ID_GET_LOGICAL_PS_ID = 0x2008,
    OPMSG_COMMON_CONFIGURE = 0x2009,
    OPMSG_COMMON_GET_CONFIGURATION = 0x200A,
    OPMSG_COMMON_GET_CLOCK_ID = 0x200B,
    OPMSG_COMMON_ID_SET_BUFFER_SIZE = 0x200C,
    OPMSG_COMMON_ID_SET_TERMINAL_BUFFER_SIZE = 0x200D,
    OPMSG_COMMON_SET_SAMPLE_RATE = 0x200E,
    OPMSG_COMMON_SET_DATA_STREAM_BASED = 0x200F,
    OPMSG_COMMON_GET_RM_ABILITY = 0x2010,
    OPMSG_COMMON_SET_RM_ENACTING = 0x2011,
    OPMSG_COMMON_SET_TTP_LATENCY = 0x2012,
    OPMSG_COMMON_SET_TTP_PARAMS = 0x2013
} OPMSG_COMMON_ID;
/*******************************************************************************

  NAME
    OPMSG_CONTROL

  DESCRIPTION
    The list of possibble contol Ids for set control operator message. Also
    the common override values.

 VALUES
    MODE_ID       - Control Id for the Mode control
    MUTE_ID       - Control Id for the Mute control
    OBPM_OVERRIDE - OBPM override bit mask

*******************************************************************************/
typedef enum
{
    OPMSG_CONTROL_MODE_ID = 0x1,
    OPMSG_CONTROL_MUTE_ID = 0x2
#define OPMSG_CONTROL_OBPM_OVERRIDE ((OPMSG_CONTROL) 0x8000)
} OPMSG_CONTROL;
/*******************************************************************************

  NAME
    OPMSG_DOWNLOAD_SELF_TEST_ID

  DESCRIPTION
    download_self_test configuration messages

 VALUES
    START_SELF_TEST - Runs the download self test

*******************************************************************************/
typedef enum
{
    OPMSG_DOWNLOAD_SELF_TEST_ID_START_SELF_TEST = 0x0001
} OPMSG_DOWNLOAD_SELF_TEST_ID;
/*******************************************************************************

  NAME
    OPMSG_FRAMEWORK_KEY

  DESCRIPTION
    Describes a common get/set system property messages.

 VALUES
    OPMSG_FRAMEWORK_GET_VERSION_SYS_MSG
                   -
    OPMSG_FRAMEWORK_GET_OPID_LIST_SYS_MSG
                   -
    OPMSG_FRAMEWORK_GET_CONID_LIST_SYS_MSG
                   -
    OPMSG_FRAMEWORK_SET_STREAM_RATE_SYS_MSG
                   -
    OPMSG_FRAMEWORK_GET_STREAM_RATE_SYS_MSG
                   -
    OPMSG_FRAMEWORK_SYS_SET
                   - Sets the system properties.
    OPMSG_FRAMEWORK_SYS_GET
                   - Gets the system properties.
    OPMSG_FRAMEWORK_GET_MEM_USAGE
                   - Querying the memory usage.
    OPMSG_FRAMEWORK_CLEAR_MEM_WATERMARKS
                   - Command to clear the memory watermarks. Regarding with the
                     response message it only contains a status signalling if
                     the instruction was successful. A standard baton response
                     massage is used to signal this.
    OPMSG_FRAMEWORK_GET_MIPS_USAGE
                   - Querying the mips usage for a list of operators. The first
                     field of the message is the length of the operator id list
                     followed by this field.
    OPMSG_FRAMEWORK_GET_CAPID_LIST
                   - Querying the list of supported capabilities.
    OPMSG_FRAMEWORK_GET_BUILD_ID_STRING
                   - Querying on BC the Kymera build identifier string.

*******************************************************************************/
typedef enum
{
    OPMSG_FRAMEWORK_GET_VERSION_SYS_MSG = 0x0001,
    OPMSG_FRAMEWORK_GET_OPID_LIST_SYS_MSG = 0x0002,
    OPMSG_FRAMEWORK_GET_CONID_LIST_SYS_MSG = 0x0003,
    OPMSG_FRAMEWORK_SET_STREAM_RATE_SYS_MSG = 0x0004,
    OPMSG_FRAMEWORK_GET_STREAM_RATE_SYS_MSG = 0x0005,
    OPMSG_FRAMEWORK_SYS_SET = 0x0006,
    OPMSG_FRAMEWORK_SYS_GET = 0x0007,
    OPMSG_FRAMEWORK_GET_MEM_USAGE = 0x00FB,
    OPMSG_FRAMEWORK_CLEAR_MEM_WATERMARKS = 0x00FC,
    OPMSG_FRAMEWORK_GET_MIPS_USAGE = 0x00FD,
    OPMSG_FRAMEWORK_GET_CAPID_LIST = 0x00FE,
    OPMSG_FRAMEWORK_GET_BUILD_ID_STRING = 0x00FF
} OPMSG_FRAMEWORK_KEY;
/*******************************************************************************

  NAME
    OPMSG_FROM_OP_SPDIF_DECODE_ID

  DESCRIPTION
    messages that S/PDIF Decode operator can send

 VALUES
    ADAPT_TO_NEW_RATE    - This message is sent to the client to when the sample
                           rate has changed. The client needs to adapt the
                           system to the new rate and once adaptation process is
                           complete it should inform the operator that the
                           output rate has changed via SET_OUTPUT_RATE message
    NEW_DECODER_REQUIRED - Tells the client that a new decoder is required, the
                           client must load the decoder and do all the
                           connections required and starts the decoder before
                           telling the operator that the decoder is ready to
                           use.
    NEW_DATA_TYPE        - New data type is being received, operator doesn't
                           require the client to do any specific action but
                           client might want to use this for other purposes.
    INPUT_RATE_VALID     - Tells the decoder whether the input is valid or no
                           . operator doesn't require the client to do any
                           specific action but client might want to use this for
                           other purposes.
    NEW_CHANNEL_STATUS   - NEW channel Status is forwarded to the client

*******************************************************************************/
typedef enum
{
    OPMSG_FROM_OP_SPDIF_DECODE_ID_ADAPT_TO_NEW_RATE = 0x1000,
    OPMSG_FROM_OP_SPDIF_DECODE_ID_NEW_DECODER_REQUIRED = 0x1001,
    OPMSG_FROM_OP_SPDIF_DECODE_ID_NEW_DATA_TYPE = 0x1002,
    OPMSG_FROM_OP_SPDIF_DECODE_ID_INPUT_RATE_VALID = 0x1003,
    OPMSG_FROM_OP_SPDIF_DECODE_ID_NEW_CHANNEL_STATUS = 0x1004
} OPMSG_FROM_OP_SPDIF_DECODE_ID;
/*******************************************************************************

  NAME
    OPMSG_IIR_RESAMPLER_ID

  DESCRIPTION
    IIR Resampler configuration messages

 VALUES
    SET_SAMPLE_RATES    - Sets the input & output sample rate Format(2words):
                          [in_rate/25, out_rate/25]
    SET_CONVERSION_RATE - Legacy resampler conversion rate opmsg. It has one
                          argument: conversion_rate = input_rate_index * 16 +
                          output_rate_index
    SET_CONFIG          - Set resampler configuration bitflags for low_mips &
                          dbl_precision one 16-bit word payload

*******************************************************************************/
typedef enum
{
    OPMSG_IIR_RESAMPLER_ID_SET_SAMPLE_RATES = 0x0001,
    OPMSG_IIR_RESAMPLER_ID_SET_CONVERSION_RATE = 0x0002,
    OPMSG_IIR_RESAMPLER_ID_SET_CONFIG = 0x0003
} OPMSG_IIR_RESAMPLER_ID;
/*******************************************************************************

  NAME
    OPMSG_INTERLEAVE_ID

  DESCRIPTION
    Messages for configuring Interleave and Deinterleave capabilities

 VALUES
    SET_DATA_FORMAT - This message changes the data format of operator's
                      terminals, all the input and output terminal will have the
                      same data type. The operator is agnostic about the data
                      type, but it is required to supply proper value to the
                      framework at the connection point. Default value is
                      AUDIO_DATA_FORMAT_FIXP.

*******************************************************************************/
typedef enum
{
    OPMSG_INTERLEAVE_ID_SET_DATA_FORMAT = 0x0002
} OPMSG_INTERLEAVE_ID;
/*******************************************************************************

  NAME
    OPMSG_MIXER_ID

  DESCRIPTION
    Mixer configuration messages

 VALUES
    SET_STREAM_GAINS     - Sets the gain of all the different inputs streams
    SET_STREAM_CHANNELS  - Sets the number of chains each stream can accept
    SET_RAMP_NUM_SAMPLES - Sets the number of samples to process during ramping
    SET_PRIMARY_STREAM   - Sets the new primary stream
    SET_CHANNEL_GAINS    - Sets the gain of multiple channels Format (n words):
                           - Number of channels to change gain - Pair of channel
                           number and new gain (as many as Number of channels to
                           change gain)
    SET_METADATA_STREAM  - Sets the stream whose metadata is propagated to the
                           output

*******************************************************************************/
typedef enum
{
    OPMSG_MIXER_ID_SET_STREAM_GAINS = 0x0001,
    OPMSG_MIXER_ID_SET_STREAM_CHANNELS = 0x0002,
    OPMSG_MIXER_ID_SET_RAMP_NUM_SAMPLES = 0x0003,
    OPMSG_MIXER_ID_SET_PRIMARY_STREAM = 0x0004,
    OPMSG_MIXER_ID_SET_CHANNEL_GAINS = 0x0005,
    OPMSG_MIXER_ID_SET_METADATA_STREAM = 0x0006
} OPMSG_MIXER_ID;
/*******************************************************************************

  NAME
    OPMSG_PASSTHROUGH_ID

  DESCRIPTION
    Basic passthrough configuration messages

 VALUES
    SET_WORKING_DATA        -
    CHANGE_INPUT_DATA_TYPE  - This input message allows a user to change data
                              type (see CS-215412).
    CHANGE_OUTPUT_DATA_TYPE - This output message allows a user to change data
                              type (see CS-215412).

*******************************************************************************/
typedef enum
{
    OPMSG_PASSTHROUGH_ID_SET_WORKING_DATA = 0x0002,
    OPMSG_PASSTHROUGH_ID_CHANGE_INPUT_DATA_TYPE = 0x000A,
    OPMSG_PASSTHROUGH_ID_CHANGE_OUTPUT_DATA_TYPE = 0x000B
} OPMSG_PASSTHROUGH_ID;
/*******************************************************************************

  NAME
    OPMSG_PEQ_ID

  DESCRIPTION
    PEQ configuration messages

 VALUES
    OVERRIDE_COEFFS - This input message allows a user to replace the transfer
                      function of the PEQ via a custom set of BIQUAD filter
                      coefficients.
    LOAD_CONFIG     - This input message allows a user to load a new PEQ
                      configuration

*******************************************************************************/
typedef enum
{
    OPMSG_PEQ_ID_OVERRIDE_COEFFS = 0x0001,
    OPMSG_PEQ_ID_LOAD_CONFIG = 0x0002
} OPMSG_PEQ_ID;
/*******************************************************************************

  NAME
    OPMSG_P_STORE

  DESCRIPTION
    Constants for the persistent storage mechanism.

 VALUES
    PARAMETER_SUB_ID      - ID for parameters in persistent store
    STATE_VARIABLE_SUB_ID - ID for state variables in persistent store

*******************************************************************************/
typedef enum
{
    OPMSG_P_STORE_PARAMETER_SUB_ID = 0x0,
    OPMSG_P_STORE_STATE_VARIABLE_SUB_ID = 0x1
} OPMSG_P_STORE;
/*******************************************************************************

  NAME
    OPMSG_RESAMPLER_ID

  DESCRIPTION
    Resampler configuration messages

 VALUES
    SET_CONVERSION_RATE     - This message sets the resampler conversion rate.
                              It has one argument: conversion_rate =
                              input_rate_index * 16 + output_rate_index
    SET_CUSTOM_RATE         - This message sets a custom sample rate conversion
                              by setting the internal parameters of the
                              resampler algorithm.
    SET_FILTER_COEFFICIENTS - This message fills the coefficients of an empty
                              filter created by the SET_CUSTOM_RATE message. It
                              should only be sent after a SET_CUSTOM_RATE
                              message.

*******************************************************************************/
typedef enum
{
    OPMSG_RESAMPLER_ID_SET_CONVERSION_RATE = 0x0002,
    OPMSG_RESAMPLER_ID_SET_CUSTOM_RATE = 0x0003,
    OPMSG_RESAMPLER_ID_SET_FILTER_COEFFICIENTS = 0x0004
} OPMSG_RESAMPLER_ID;
/*******************************************************************************

  NAME
    OPMSG_RESULT_STATES

  DESCRIPTION
    The Result codes for common Operator Messages:
    OPMSG_COMMON_ID_SET_CONTROL, OPMSG_COMMON_ID_GET_PARAMS,
    OPMSG_COMMON_ID_GET_DEFAULTS, OPMSG_COMMON_ID_SET_PARAMS,
    OPMSG_COMMON_ID_GET_STATUS, OPMSG_COMMON_ID_SET_UCID,
    OPMSG_COMMON_ID_GET_LOGICAL_PS_ID

 VALUES
    NORMAL_STATE              - Operation was successful.
    MESSAGE_TOO_BIG           - The response is too large to return.  Increase
                                the message size.
    PARAMETER_STATE_NOT_READY - The operator is waiting for parameters from
                                Persistent Storage
    INVALID_PARAMETER         - An invalid Parameter ID was specified
    UNSUPPORTED_CONTROL       - The Control ID is not recognized
    INVALID_CONTROL_VALUE     - The value of the requested control is invalid
    UNSUPPORTED_ENCODING      - Requested parameter encoding is not supported by
                                the target DSP

*******************************************************************************/
typedef enum
{
    OPMSG_RESULT_STATES_NORMAL_STATE = 0x0,
    OPMSG_RESULT_STATES_MESSAGE_TOO_BIG = 0x1,
    OPMSG_RESULT_STATES_PARAMETER_STATE_NOT_READY = 0x2,
    OPMSG_RESULT_STATES_INVALID_PARAMETER = 0x3,
    OPMSG_RESULT_STATES_UNSUPPORTED_CONTROL = 0x4,
    OPMSG_RESULT_STATES_INVALID_CONTROL_VALUE = 0x5,
    OPMSG_RESULT_STATES_UNSUPPORTED_ENCODING = 0x6
} OPMSG_RESULT_STATES;
/*******************************************************************************

  NAME
    OPMSG_RINGTONE_ID

  DESCRIPTION
    Ringtone Generator operator messages

 VALUES
    TONE_SEQ - Ringtone Generator tone definition message

*******************************************************************************/
typedef enum
{
    OPMSG_RINGTONE_ID_TONE_SEQ = 0x0001
} OPMSG_RINGTONE_ID;
/*******************************************************************************

  NAME
    OPMSG_RINGTONE_REPLY_ID

  DESCRIPTION
    Unsolicited message to indicate Host or VM no more tone data in DSP

 VALUES
    TONE_END - For more information see section 4.2 in BlueCore DSPManager
               Specification document

*******************************************************************************/
typedef enum
{
    OPMSG_RINGTONE_REPLY_ID_TONE_END = 0x0001
} OPMSG_RINGTONE_REPLY_ID;
/*******************************************************************************

  NAME
    OPMSG_RTP_DECODE_ID

  DESCRIPTION
    RTP decode configuration messages

 VALUES
    SET_WORKING_MODE           - Sets the working mode.
    SET_RTP_CODEC_TYPE         - Sets the rtp codec type.
    SET_RTP_CONTENT_PROTECTION - Sets the rtp content protection (enable or
                                 disable).

*******************************************************************************/
typedef enum
{
    OPMSG_RTP_DECODE_ID_SET_WORKING_MODE = 0x0001,
    OPMSG_RTP_DECODE_ID_SET_RTP_CODEC_TYPE = 0x0002,
    OPMSG_RTP_DECODE_ID_SET_RTP_CONTENT_PROTECTION = 0x0003
} OPMSG_RTP_DECODE_ID;
/*******************************************************************************

  NAME
    OPMSG_Reply_ID

  DESCRIPTION
    Unsolicited messages from operators.

 VALUES
    FADEOUT_DONE - For more information see section 4.2 in BlueCore DSPManager
                   Specification document

*******************************************************************************/
typedef enum
{
    OPMSG_REPLY_ID_FADEOUT_DONE = OPMSG_COMMON_ID_FADEOUT_ENABLE
} OPMSG_REPLY_ID;
/*******************************************************************************

  NAME
    OPMSG_SCO_RCV_ID

  DESCRIPTION
    SCO receive configuration messages

 VALUES
    FRAME_COUNTS      -
    FORCE_PLC_OFF     -
    PROC_TIME_MEASURE -
    SET_FROM_AIR_INFO -

*******************************************************************************/
typedef enum
{
    OPMSG_SCO_RCV_ID_FRAME_COUNTS = 0x0001,
    OPMSG_SCO_RCV_ID_FORCE_PLC_OFF = 0x0002,
    OPMSG_SCO_RCV_ID_PROC_TIME_MEASURE = 0x000C,
    OPMSG_SCO_RCV_ID_SET_FROM_AIR_INFO = 0x00FE
} OPMSG_SCO_RCV_ID;
/*******************************************************************************

  NAME
    OPMSG_SCO_SEND_ID

  DESCRIPTION
    SCO send configuration messages

 VALUES
    SET_TO_AIR_INFO -

*******************************************************************************/
typedef enum
{
    OPMSG_SCO_SEND_ID_SET_TO_AIR_INFO = 0x00FD
} OPMSG_SCO_SEND_ID;
/*******************************************************************************

  NAME
    OPMSG_SPDIF_DECODE_ID

  DESCRIPTION
    S/PDIF Decode configs and responses

 VALUES
    SET_SUPPORTED_DATA_TYPES - Use this to set the coded data types that are
                               supported by the s/pdif rx driver, by default no
                               coded data type will be supported.
    SET_OUTPUT_RATE          - Sets the expected output rate, This is normally
                               sent in response to message ADAPT_TO_NEW_RATE
                               from the driver, and tells the operator that the
                               system is ready for the new rate
    SET_DECODER_DATA_TYPE    - Tells the operator which data type can be decoded
                               by currently loaded decoder.
    NEW_CHSTS_FROM_EP        - New channel status word received from end point

*******************************************************************************/
typedef enum
{
    OPMSG_SPDIF_DECODE_ID_SET_SUPPORTED_DATA_TYPES = 0x0002,
    OPMSG_SPDIF_DECODE_ID_SET_OUTPUT_RATE = 0x0003,
    OPMSG_SPDIF_DECODE_ID_SET_DECODER_DATA_TYPE = 0x0004,
    OPMSG_SPDIF_DECODE_ID_NEW_CHSTS_FROM_EP = 0x00FE
} OPMSG_SPDIF_DECODE_ID;
/*******************************************************************************

  NAME
    OPMSG_SPLITTER_ID

  DESCRIPTION
    Splitter configuration messages

 VALUES
    SET_RUNNING_STREAMS - Sets the output streams of the mixer that are
                          considered running/active. Format is a bit field where
                          bit n = stream n
    SET_DATA_FORMAT     - Sets the expected input and output terminal data
                          formats

*******************************************************************************/
typedef enum
{
    OPMSG_SPLITTER_ID_SET_RUNNING_STREAMS = 0x0001,
    OPMSG_SPLITTER_ID_SET_DATA_FORMAT = 0x000A
} OPMSG_SPLITTER_ID;
/*******************************************************************************

  NAME
    OPMSG_WBS_DEC_ID

  DESCRIPTION
    Wideband speech decoding configuration messages

 VALUES
    FRAME_COUNTS      -
    FORCE_PLC_OFF     -
    PROC_TIME_MEASURE -
    SET_FROM_AIR_INFO -

*******************************************************************************/
typedef enum
{
    OPMSG_WBS_DEC_ID_FRAME_COUNTS = 0x0001,
    OPMSG_WBS_DEC_ID_FORCE_PLC_OFF = 0x0002,
    OPMSG_WBS_DEC_ID_PROC_TIME_MEASURE = 0x000C,
    OPMSG_WBS_DEC_ID_SET_FROM_AIR_INFO = 0x00FE
} OPMSG_WBS_DEC_ID;
/*******************************************************************************

  NAME
    OPMSG_WBS_ENC_ID

  DESCRIPTION
    Wideband speech encoding configuration messages

 VALUES
    SET_TO_AIR_INFO -

*******************************************************************************/
typedef enum
{
    OPMSG_WBS_ENC_ID_SET_TO_AIR_INFO = 0x00FD
} OPMSG_WBS_ENC_ID;


/*******************************************************************************

  NAME
    OPMSG_CVC_RCV_ID

  DESCRIPTION

*******************************************************************************/
typedef uint16 OPMSG_CVC_RCV_ID;
/*******************************************************************************

  NAME
    OPMSG_SBC_DEC_ID

  DESCRIPTION

*******************************************************************************/
typedef uint16 OPMSG_SBC_DEC_ID;


#define OPMSG_PRIM_ANY_SIZE 1

/*******************************************************************************

  NAME
    Opmsg_A2dp_Dec_Content_Protection_Enable

  DESCRIPTION
    A2DP Decoder Operator message for CONTENT_PROTECTION_ENABLE.

  MEMBERS
    message_id - message id
    enable_cp  - enable content protection for A2DP decoder

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} OPMSG_A2DP_DEC_CONTENT_PROTECTION_ENABLE;

/* The following macros take OPMSG_A2DP_DEC_CONTENT_PROTECTION_ENABLE *opmsg_a2dp_dec_content_protection_enable_ptr */
#define OPMSG_A2DP_DEC_CONTENT_PROTECTION_ENABLE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_A2DP_DEC_CONTENT_PROTECTION_ENABLE_MESSAGE_ID_GET(opmsg_a2dp_dec_content_protection_enable_ptr) ((OPMSG_AD2P_DEC_ID)(opmsg_a2dp_dec_content_protection_enable_ptr)->_data[0])
#define OPMSG_A2DP_DEC_CONTENT_PROTECTION_ENABLE_MESSAGE_ID_SET(opmsg_a2dp_dec_content_protection_enable_ptr, message_id) ((opmsg_a2dp_dec_content_protection_enable_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_A2DP_DEC_CONTENT_PROTECTION_ENABLE_ENABLE_CP_WORD_OFFSET (1)
#define OPMSG_A2DP_DEC_CONTENT_PROTECTION_ENABLE_ENABLE_CP_GET(opmsg_a2dp_dec_content_protection_enable_ptr) ((BOOL)(((opmsg_a2dp_dec_content_protection_enable_ptr)->_data[1] & 0x0)))
#define OPMSG_A2DP_DEC_CONTENT_PROTECTION_ENABLE_ENABLE_CP_SET(opmsg_a2dp_dec_content_protection_enable_ptr, enable_cp) ((opmsg_a2dp_dec_content_protection_enable_ptr)->_data[1] =  \
                                                                                                                             (uint16)(((opmsg_a2dp_dec_content_protection_enable_ptr)->_data[1] & ~0x0) | (((enable_cp)) & 0x0)))
#define OPMSG_A2DP_DEC_CONTENT_PROTECTION_ENABLE_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_A2DP_DEC_CONTENT_PROTECTION_ENABLE_CREATE(message_id, enable_cp) \
    (uint16)(message_id), \
    (uint16)(((enable_cp)) & 0x0)
#define OPMSG_A2DP_DEC_CONTENT_PROTECTION_ENABLE_PACK(opmsg_a2dp_dec_content_protection_enable_ptr, message_id, enable_cp) \
    do { \
        (opmsg_a2dp_dec_content_protection_enable_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_a2dp_dec_content_protection_enable_ptr)->_data[1] = (uint16)((uint16)(((enable_cp)) & 0x0)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Disable_Fadeout

  DESCRIPTION
    Operator common message for DISABLE_FADEOUT.

  MEMBERS
    message_id - message id

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} OPMSG_COMMON_DISABLE_FADEOUT;

/* The following macros take OPMSG_COMMON_DISABLE_FADEOUT *opmsg_common_disable_fadeout_ptr */
#define OPMSG_COMMON_DISABLE_FADEOUT_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_DISABLE_FADEOUT_MESSAGE_ID_GET(opmsg_common_disable_fadeout_ptr) ((OPMSG_COMMON_ID)(opmsg_common_disable_fadeout_ptr)->_data[0])
#define OPMSG_COMMON_DISABLE_FADEOUT_MESSAGE_ID_SET(opmsg_common_disable_fadeout_ptr, message_id) ((opmsg_common_disable_fadeout_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_DISABLE_FADEOUT_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_DISABLE_FADEOUT_CREATE(message_id) \
    (uint16)(message_id)
#define OPMSG_COMMON_DISABLE_FADEOUT_PACK(opmsg_common_disable_fadeout_ptr, message_id) \
    do { \
        (opmsg_common_disable_fadeout_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Enable_Fadeout

  DESCRIPTION
    Operator common message for ENABLE_FADEOUT.

  MEMBERS
    message_id - message id

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} OPMSG_COMMON_ENABLE_FADEOUT;

/* The following macros take OPMSG_COMMON_ENABLE_FADEOUT *opmsg_common_enable_fadeout_ptr */
#define OPMSG_COMMON_ENABLE_FADEOUT_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_ENABLE_FADEOUT_MESSAGE_ID_GET(opmsg_common_enable_fadeout_ptr) ((OPMSG_COMMON_ID)(opmsg_common_enable_fadeout_ptr)->_data[0])
#define OPMSG_COMMON_ENABLE_FADEOUT_MESSAGE_ID_SET(opmsg_common_enable_fadeout_ptr, message_id) ((opmsg_common_enable_fadeout_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_ENABLE_FADEOUT_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_ENABLE_FADEOUT_CREATE(message_id) \
    (uint16)(message_id)
#define OPMSG_COMMON_ENABLE_FADEOUT_PACK(opmsg_common_enable_fadeout_ptr, message_id) \
    do { \
        (opmsg_common_enable_fadeout_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Get_Capability_Version

  DESCRIPTION
    Operator common message for GET_CAPABILITY_VERSION.

  MEMBERS
    message_id - message id

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} OPMSG_COMMON_GET_CAPABILITY_VERSION;

/* The following macros take OPMSG_COMMON_GET_CAPABILITY_VERSION *opmsg_common_get_capability_version_ptr */
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_MESSAGE_ID_GET(opmsg_common_get_capability_version_ptr) ((OPMSG_COMMON_ID)(opmsg_common_get_capability_version_ptr)->_data[0])
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_MESSAGE_ID_SET(opmsg_common_get_capability_version_ptr, message_id) ((opmsg_common_get_capability_version_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_CREATE(message_id) \
    (uint16)(message_id)
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_PACK(opmsg_common_get_capability_version_ptr, message_id) \
    do { \
        (opmsg_common_get_capability_version_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Msg_Set_Data_Stream_Based

  DESCRIPTION
    Operator common message for SET_DATA_STREAM_BASED.

  MEMBERS
    message_id      - message id
    is_stream_based - is stream based

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} OPMSG_COMMON_MSG_SET_DATA_STREAM_BASED;

/* The following macros take OPMSG_COMMON_MSG_SET_DATA_STREAM_BASED *opmsg_common_msg_set_data_stream_based_ptr */
#define OPMSG_COMMON_MSG_SET_DATA_STREAM_BASED_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_MSG_SET_DATA_STREAM_BASED_MESSAGE_ID_GET(opmsg_common_msg_set_data_stream_based_ptr) ((OPMSG_COMMON_ID)(opmsg_common_msg_set_data_stream_based_ptr)->_data[0])
#define OPMSG_COMMON_MSG_SET_DATA_STREAM_BASED_MESSAGE_ID_SET(opmsg_common_msg_set_data_stream_based_ptr, message_id) ((opmsg_common_msg_set_data_stream_based_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_MSG_SET_DATA_STREAM_BASED_IS_STREAM_BASED_WORD_OFFSET (1)
#define OPMSG_COMMON_MSG_SET_DATA_STREAM_BASED_IS_STREAM_BASED_GET(opmsg_common_msg_set_data_stream_based_ptr) ((BOOL)(((opmsg_common_msg_set_data_stream_based_ptr)->_data[1] & 0x0)))
#define OPMSG_COMMON_MSG_SET_DATA_STREAM_BASED_IS_STREAM_BASED_SET(opmsg_common_msg_set_data_stream_based_ptr, is_stream_based) ((opmsg_common_msg_set_data_stream_based_ptr)->_data[1] =  \
                                                                                                                                     (uint16)(((opmsg_common_msg_set_data_stream_based_ptr)->_data[1] & ~0x0) | (((is_stream_based)) & 0x0)))
#define OPMSG_COMMON_MSG_SET_DATA_STREAM_BASED_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_MSG_SET_DATA_STREAM_BASED_CREATE(message_id, is_stream_based) \
    (uint16)(message_id), \
    (uint16)(((is_stream_based)) & 0x0)
#define OPMSG_COMMON_MSG_SET_DATA_STREAM_BASED_PACK(opmsg_common_msg_set_data_stream_based_ptr, message_id, is_stream_based) \
    do { \
        (opmsg_common_msg_set_data_stream_based_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_msg_set_data_stream_based_ptr)->_data[1] = (uint16)((uint16)(((is_stream_based)) & 0x0)); \
    } while (0)


/*******************************************************************************

  NAME
    OPMSG_MSG

  DESCRIPTION
    Operator message definition.

  MEMBERS
    sender_id  - sender id
    num_params - number of parameters in the payload
    payload    - Arguments

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_MSG;

/* The following macros take OPMSG_MSG *opmsg_msg_ptr */
#define OPMSG_MSG_SENDER_ID_WORD_OFFSET (0)
#define OPMSG_MSG_SENDER_ID_GET(opmsg_msg_ptr) ((opmsg_msg_ptr)->_data[0])
#define OPMSG_MSG_SENDER_ID_SET(opmsg_msg_ptr, sender_id) ((opmsg_msg_ptr)->_data[0] = (uint16)(sender_id))
#define OPMSG_MSG_NUM_PARAMS_WORD_OFFSET (1)
#define OPMSG_MSG_NUM_PARAMS_GET(opmsg_msg_ptr) ((opmsg_msg_ptr)->_data[1])
#define OPMSG_MSG_NUM_PARAMS_SET(opmsg_msg_ptr, num_params) ((opmsg_msg_ptr)->_data[1] = (uint16)(num_params))
#define OPMSG_MSG_PAYLOAD_WORD_OFFSET (2)
#define OPMSG_MSG_PAYLOAD_GET(opmsg_msg_ptr) ((opmsg_msg_ptr)->_data[2])
#define OPMSG_MSG_PAYLOAD_SET(opmsg_msg_ptr, payload) ((opmsg_msg_ptr)->_data[2] = (uint16)(payload))
#define OPMSG_MSG_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_MSG_CREATE(sender_id, num_params, payload) \
    (uint16)(sender_id), \
    (uint16)(num_params), \
    (uint16)(payload)
#define OPMSG_MSG_PACK(opmsg_msg_ptr, sender_id, num_params, payload) \
    do { \
        (opmsg_msg_ptr)->_data[0] = (uint16)((uint16)(sender_id)); \
        (opmsg_msg_ptr)->_data[1] = (uint16)((uint16)(num_params)); \
        (opmsg_msg_ptr)->_data[2] = (uint16)((uint16)(payload)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Aec_Set_Sample_Rates

  DESCRIPTION
    AEC operator message for SET_SAMPLE_RATES.

  MEMBERS
    message_id  - message id
    input_rate  - input rate
    output_rate - output rate

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_AEC_SET_SAMPLE_RATES;

/* The following macros take OPMSG_AEC_SET_SAMPLE_RATES *opmsg_aec_set_sample_rates_ptr */
#define OPMSG_AEC_SET_SAMPLE_RATES_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_AEC_SET_SAMPLE_RATES_MESSAGE_ID_GET(opmsg_aec_set_sample_rates_ptr) ((OPMSG_AEC_REFERENCE_ID)(opmsg_aec_set_sample_rates_ptr)->_data[0])
#define OPMSG_AEC_SET_SAMPLE_RATES_MESSAGE_ID_SET(opmsg_aec_set_sample_rates_ptr, message_id) ((opmsg_aec_set_sample_rates_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_AEC_SET_SAMPLE_RATES_INPUT_RATE_WORD_OFFSET (1)
#define OPMSG_AEC_SET_SAMPLE_RATES_INPUT_RATE_GET(opmsg_aec_set_sample_rates_ptr) ((opmsg_aec_set_sample_rates_ptr)->_data[1])
#define OPMSG_AEC_SET_SAMPLE_RATES_INPUT_RATE_SET(opmsg_aec_set_sample_rates_ptr, input_rate) ((opmsg_aec_set_sample_rates_ptr)->_data[1] = (uint16)(input_rate))
#define OPMSG_AEC_SET_SAMPLE_RATES_OUTPUT_RATE_WORD_OFFSET (2)
#define OPMSG_AEC_SET_SAMPLE_RATES_OUTPUT_RATE_GET(opmsg_aec_set_sample_rates_ptr) ((opmsg_aec_set_sample_rates_ptr)->_data[2])
#define OPMSG_AEC_SET_SAMPLE_RATES_OUTPUT_RATE_SET(opmsg_aec_set_sample_rates_ptr, output_rate) ((opmsg_aec_set_sample_rates_ptr)->_data[2] = (uint16)(output_rate))
#define OPMSG_AEC_SET_SAMPLE_RATES_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_AEC_SET_SAMPLE_RATES_CREATE(message_id, input_rate, output_rate) \
    (uint16)(message_id), \
    (uint16)(input_rate), \
    (uint16)(output_rate)
#define OPMSG_AEC_SET_SAMPLE_RATES_PACK(opmsg_aec_set_sample_rates_ptr, message_id, input_rate, output_rate) \
    do { \
        (opmsg_aec_set_sample_rates_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_aec_set_sample_rates_ptr)->_data[1] = (uint16)((uint16)(input_rate)); \
        (opmsg_aec_set_sample_rates_ptr)->_data[2] = (uint16)((uint16)(output_rate)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Ch_Mixer_Set_Parameters

  DESCRIPTION
    Channel Mixer operator message for SET_STREAM_PARAMETERS.

  MEMBERS
    message_id          - message id
    num_input_channels  - start index
    num_output_channels - count
    gain_table          - gain_table

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_CH_MIXER_SET_PARAMETERS;

/* The following macros take OPMSG_CH_MIXER_SET_PARAMETERS *opmsg_ch_mixer_set_parameters_ptr */
#define OPMSG_CH_MIXER_SET_PARAMETERS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_CH_MIXER_SET_PARAMETERS_MESSAGE_ID_GET(opmsg_ch_mixer_set_parameters_ptr) ((OPMSG_CHANNEL_MIXER_ID)(opmsg_ch_mixer_set_parameters_ptr)->_data[0])
#define OPMSG_CH_MIXER_SET_PARAMETERS_MESSAGE_ID_SET(opmsg_ch_mixer_set_parameters_ptr, message_id) ((opmsg_ch_mixer_set_parameters_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_CH_MIXER_SET_PARAMETERS_NUM_INPUT_CHANNELS_WORD_OFFSET (1)
#define OPMSG_CH_MIXER_SET_PARAMETERS_NUM_INPUT_CHANNELS_GET(opmsg_ch_mixer_set_parameters_ptr) ((opmsg_ch_mixer_set_parameters_ptr)->_data[1])
#define OPMSG_CH_MIXER_SET_PARAMETERS_NUM_INPUT_CHANNELS_SET(opmsg_ch_mixer_set_parameters_ptr, num_input_channels) ((opmsg_ch_mixer_set_parameters_ptr)->_data[1] = (uint16)(num_input_channels))
#define OPMSG_CH_MIXER_SET_PARAMETERS_NUM_OUTPUT_CHANNELS_WORD_OFFSET (2)
#define OPMSG_CH_MIXER_SET_PARAMETERS_NUM_OUTPUT_CHANNELS_GET(opmsg_ch_mixer_set_parameters_ptr) ((opmsg_ch_mixer_set_parameters_ptr)->_data[2])
#define OPMSG_CH_MIXER_SET_PARAMETERS_NUM_OUTPUT_CHANNELS_SET(opmsg_ch_mixer_set_parameters_ptr, num_output_channels) ((opmsg_ch_mixer_set_parameters_ptr)->_data[2] = (uint16)(num_output_channels))
#define OPMSG_CH_MIXER_SET_PARAMETERS_GAIN_TABLE_WORD_OFFSET (3)
#define OPMSG_CH_MIXER_SET_PARAMETERS_GAIN_TABLE_GET(opmsg_ch_mixer_set_parameters_ptr) ((opmsg_ch_mixer_set_parameters_ptr)->_data[3])
#define OPMSG_CH_MIXER_SET_PARAMETERS_GAIN_TABLE_SET(opmsg_ch_mixer_set_parameters_ptr, gain_table) ((opmsg_ch_mixer_set_parameters_ptr)->_data[3] = (uint16)(gain_table))
#define OPMSG_CH_MIXER_SET_PARAMETERS_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_CH_MIXER_SET_PARAMETERS_CREATE(message_id, num_input_channels, num_output_channels, gain_table) \
    (uint16)(message_id), \
    (uint16)(num_input_channels), \
    (uint16)(num_output_channels), \
    (uint16)(gain_table)
#define OPMSG_CH_MIXER_SET_PARAMETERS_PACK(opmsg_ch_mixer_set_parameters_ptr, message_id, num_input_channels, num_output_channels, gain_table) \
    do { \
        (opmsg_ch_mixer_set_parameters_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_ch_mixer_set_parameters_ptr)->_data[1] = (uint16)((uint16)(num_input_channels)); \
        (opmsg_ch_mixer_set_parameters_ptr)->_data[2] = (uint16)((uint16)(num_output_channels)); \
        (opmsg_ch_mixer_set_parameters_ptr)->_data[3] = (uint16)((uint16)(gain_table)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Get_Capability_Version_Resp

  DESCRIPTION
    Operator response for the op message for GET_CAPABILITY_VERSION.

  MEMBERS
    message_id  - message id
    version_msw - capability version msw
    version_lsw - capability version lsw

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_COMMON_GET_CAPABILITY_VERSION_RESP;

/* The following macros take OPMSG_COMMON_GET_CAPABILITY_VERSION_RESP *opmsg_common_get_capability_version_resp_ptr */
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_RESP_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_RESP_MESSAGE_ID_GET(opmsg_common_get_capability_version_resp_ptr) ((OPMSG_COMMON_ID)(opmsg_common_get_capability_version_resp_ptr)->_data[0])
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_RESP_MESSAGE_ID_SET(opmsg_common_get_capability_version_resp_ptr, message_id) ((opmsg_common_get_capability_version_resp_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_RESP_VERSION_MSW_WORD_OFFSET (1)
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_RESP_VERSION_MSW_GET(opmsg_common_get_capability_version_resp_ptr) ((opmsg_common_get_capability_version_resp_ptr)->_data[1])
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_RESP_VERSION_MSW_SET(opmsg_common_get_capability_version_resp_ptr, version_msw) ((opmsg_common_get_capability_version_resp_ptr)->_data[1] = (uint16)(version_msw))
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_RESP_VERSION_LSW_WORD_OFFSET (2)
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_RESP_VERSION_LSW_GET(opmsg_common_get_capability_version_resp_ptr) ((opmsg_common_get_capability_version_resp_ptr)->_data[2])
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_RESP_VERSION_LSW_SET(opmsg_common_get_capability_version_resp_ptr, version_lsw) ((opmsg_common_get_capability_version_resp_ptr)->_data[2] = (uint16)(version_lsw))
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_RESP_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_RESP_CREATE(message_id, version_msw, version_lsw) \
    (uint16)(message_id), \
    (uint16)(version_msw), \
    (uint16)(version_lsw)
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_RESP_PACK(opmsg_common_get_capability_version_resp_ptr, message_id, version_msw, version_lsw) \
    do { \
        (opmsg_common_get_capability_version_resp_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_get_capability_version_resp_ptr)->_data[1] = (uint16)((uint16)(version_msw)); \
        (opmsg_common_get_capability_version_resp_ptr)->_data[2] = (uint16)((uint16)(version_lsw)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Get_Defaults

  DESCRIPTION
    Operator common message for GET_DEFAULTS.

  MEMBERS
    message_id - message id
    offset     - offset
    range      - range

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_COMMON_GET_DEFAULTS;

/* The following macros take OPMSG_COMMON_GET_DEFAULTS *opmsg_common_get_defaults_ptr */
#define OPMSG_COMMON_GET_DEFAULTS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_GET_DEFAULTS_MESSAGE_ID_GET(opmsg_common_get_defaults_ptr) ((OPMSG_COMMON_ID)(opmsg_common_get_defaults_ptr)->_data[0])
#define OPMSG_COMMON_GET_DEFAULTS_MESSAGE_ID_SET(opmsg_common_get_defaults_ptr, message_id) ((opmsg_common_get_defaults_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_GET_DEFAULTS_OFFSET_WORD_OFFSET (1)
#define OPMSG_COMMON_GET_DEFAULTS_OFFSET_GET(opmsg_common_get_defaults_ptr) ((opmsg_common_get_defaults_ptr)->_data[1])
#define OPMSG_COMMON_GET_DEFAULTS_OFFSET_SET(opmsg_common_get_defaults_ptr, offset) ((opmsg_common_get_defaults_ptr)->_data[1] = (uint16)(offset))
#define OPMSG_COMMON_GET_DEFAULTS_RANGE_WORD_OFFSET (2)
#define OPMSG_COMMON_GET_DEFAULTS_RANGE_GET(opmsg_common_get_defaults_ptr) ((opmsg_common_get_defaults_ptr)->_data[2])
#define OPMSG_COMMON_GET_DEFAULTS_RANGE_SET(opmsg_common_get_defaults_ptr, range) ((opmsg_common_get_defaults_ptr)->_data[2] = (uint16)(range))
#define OPMSG_COMMON_GET_DEFAULTS_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_GET_DEFAULTS_CREATE(message_id, offset, range) \
    (uint16)(message_id), \
    (uint16)(offset), \
    (uint16)(range)
#define OPMSG_COMMON_GET_DEFAULTS_PACK(opmsg_common_get_defaults_ptr, message_id, offset, range) \
    do { \
        (opmsg_common_get_defaults_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_get_defaults_ptr)->_data[1] = (uint16)((uint16)(offset)); \
        (opmsg_common_get_defaults_ptr)->_data[2] = (uint16)((uint16)(range)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Get_Defaults_Resp

  DESCRIPTION
    Operator response for the message GET_DEFAULTS.

  MEMBERS
    message_id     - message id
    default_values - the default values

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_COMMON_GET_DEFAULTS_RESP;

/* The following macros take OPMSG_COMMON_GET_DEFAULTS_RESP *opmsg_common_get_defaults_resp_ptr */
#define OPMSG_COMMON_GET_DEFAULTS_RESP_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_GET_DEFAULTS_RESP_MESSAGE_ID_GET(opmsg_common_get_defaults_resp_ptr) ((OPMSG_COMMON_ID)(opmsg_common_get_defaults_resp_ptr)->_data[0])
#define OPMSG_COMMON_GET_DEFAULTS_RESP_MESSAGE_ID_SET(opmsg_common_get_defaults_resp_ptr, message_id) ((opmsg_common_get_defaults_resp_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_GET_DEFAULTS_RESP_DEFAULT_VALUES_WORD_OFFSET (1)
#define OPMSG_COMMON_GET_DEFAULTS_RESP_DEFAULT_VALUES_GET(opmsg_common_get_defaults_resp_ptr) ((opmsg_common_get_defaults_resp_ptr)->_data[1])
#define OPMSG_COMMON_GET_DEFAULTS_RESP_DEFAULT_VALUES_SET(opmsg_common_get_defaults_resp_ptr, default_values) ((opmsg_common_get_defaults_resp_ptr)->_data[1] = (uint16)(default_values))
#define OPMSG_COMMON_GET_DEFAULTS_RESP_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_GET_DEFAULTS_RESP_CREATE(message_id, default_values) \
    (uint16)(message_id), \
    (uint16)(default_values)
#define OPMSG_COMMON_GET_DEFAULTS_RESP_PACK(opmsg_common_get_defaults_resp_ptr, message_id, default_values) \
    do { \
        (opmsg_common_get_defaults_resp_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_get_defaults_resp_ptr)->_data[1] = (uint16)((uint16)(default_values)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Get_Logical_Ps_Id

  DESCRIPTION
    Operator common message for GET_LOGICAL_PS_ID.

  MEMBERS
    message_id - message id
    sid        - sid

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_COMMON_GET_LOGICAL_PS_ID;

/* The following macros take OPMSG_COMMON_GET_LOGICAL_PS_ID *opmsg_common_get_logical_ps_id_ptr */
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_MESSAGE_ID_GET(opmsg_common_get_logical_ps_id_ptr) ((OPMSG_COMMON_ID)(opmsg_common_get_logical_ps_id_ptr)->_data[0])
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_MESSAGE_ID_SET(opmsg_common_get_logical_ps_id_ptr, message_id) ((opmsg_common_get_logical_ps_id_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_SID_WORD_OFFSET (1)
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_SID_GET(opmsg_common_get_logical_ps_id_ptr) ((opmsg_common_get_logical_ps_id_ptr)->_data[1])
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_SID_SET(opmsg_common_get_logical_ps_id_ptr, sid) ((opmsg_common_get_logical_ps_id_ptr)->_data[1] = (uint16)(sid))
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_CREATE(message_id, sid) \
    (uint16)(message_id), \
    (uint16)(sid)
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_PACK(opmsg_common_get_logical_ps_id_ptr, message_id, sid) \
    do { \
        (opmsg_common_get_logical_ps_id_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_get_logical_ps_id_ptr)->_data[1] = (uint16)((uint16)(sid)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Get_Logical_Ps_Id_Resp

  DESCRIPTION
    Operator response for the message for GET_LOGICAL_PS_ID.

  MEMBERS
    message_id     - message id
    confirmed_ucid - confirmed ucid
    logical_ps_id  - logical PS ID
    ucid           - ucid
    sid            - sub-block ID

*******************************************************************************/
typedef struct
{
    uint16 _data[5];
} OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP;

/* The following macros take OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP *opmsg_common_get_logical_ps_id_resp_ptr */
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_MESSAGE_ID_GET(opmsg_common_get_logical_ps_id_resp_ptr) ((OPMSG_COMMON_ID)(opmsg_common_get_logical_ps_id_resp_ptr)->_data[0])
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_MESSAGE_ID_SET(opmsg_common_get_logical_ps_id_resp_ptr, message_id) ((opmsg_common_get_logical_ps_id_resp_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_CONFIRMED_UCID_WORD_OFFSET (1)
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_CONFIRMED_UCID_GET(opmsg_common_get_logical_ps_id_resp_ptr) ((opmsg_common_get_logical_ps_id_resp_ptr)->_data[1])
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_CONFIRMED_UCID_SET(opmsg_common_get_logical_ps_id_resp_ptr, confirmed_ucid) ((opmsg_common_get_logical_ps_id_resp_ptr)->_data[1] = (uint16)(confirmed_ucid))
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_LOGICAL_PS_ID_WORD_OFFSET (2)
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_LOGICAL_PS_ID_GET(opmsg_common_get_logical_ps_id_resp_ptr) ((opmsg_common_get_logical_ps_id_resp_ptr)->_data[2])
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_LOGICAL_PS_ID_SET(opmsg_common_get_logical_ps_id_resp_ptr, logical_ps_id) ((opmsg_common_get_logical_ps_id_resp_ptr)->_data[2] = (uint16)(logical_ps_id))
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_UCID_WORD_OFFSET (3)
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_UCID_GET(opmsg_common_get_logical_ps_id_resp_ptr) ((opmsg_common_get_logical_ps_id_resp_ptr)->_data[3])
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_UCID_SET(opmsg_common_get_logical_ps_id_resp_ptr, ucid) ((opmsg_common_get_logical_ps_id_resp_ptr)->_data[3] = (uint16)(ucid))
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_SID_WORD_OFFSET (4)
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_SID_GET(opmsg_common_get_logical_ps_id_resp_ptr) ((opmsg_common_get_logical_ps_id_resp_ptr)->_data[4])
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_SID_SET(opmsg_common_get_logical_ps_id_resp_ptr, sid) ((opmsg_common_get_logical_ps_id_resp_ptr)->_data[4] = (uint16)(sid))
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_WORD_SIZE (5)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_CREATE(message_id, confirmed_ucid, logical_ps_id, ucid, sid) \
    (uint16)(message_id), \
    (uint16)(confirmed_ucid), \
    (uint16)(logical_ps_id), \
    (uint16)(ucid), \
    (uint16)(sid)
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_PACK(opmsg_common_get_logical_ps_id_resp_ptr, message_id, confirmed_ucid, logical_ps_id, ucid, sid) \
    do { \
        (opmsg_common_get_logical_ps_id_resp_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_get_logical_ps_id_resp_ptr)->_data[1] = (uint16)((uint16)(confirmed_ucid)); \
        (opmsg_common_get_logical_ps_id_resp_ptr)->_data[2] = (uint16)((uint16)(logical_ps_id)); \
        (opmsg_common_get_logical_ps_id_resp_ptr)->_data[3] = (uint16)((uint16)(ucid)); \
        (opmsg_common_get_logical_ps_id_resp_ptr)->_data[4] = (uint16)((uint16)(sid)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Get_Params

  DESCRIPTION
    Operator common message for GET_PARAMS.

  MEMBERS
    message_id - message id
    offset     - offset
    range      - range

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_COMMON_GET_PARAMS;

/* The following macros take OPMSG_COMMON_GET_PARAMS *opmsg_common_get_params_ptr */
#define OPMSG_COMMON_GET_PARAMS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_GET_PARAMS_MESSAGE_ID_GET(opmsg_common_get_params_ptr) ((OPMSG_COMMON_ID)(opmsg_common_get_params_ptr)->_data[0])
#define OPMSG_COMMON_GET_PARAMS_MESSAGE_ID_SET(opmsg_common_get_params_ptr, message_id) ((opmsg_common_get_params_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_GET_PARAMS_OFFSET_WORD_OFFSET (1)
#define OPMSG_COMMON_GET_PARAMS_OFFSET_GET(opmsg_common_get_params_ptr) ((opmsg_common_get_params_ptr)->_data[1])
#define OPMSG_COMMON_GET_PARAMS_OFFSET_SET(opmsg_common_get_params_ptr, offset) ((opmsg_common_get_params_ptr)->_data[1] = (uint16)(offset))
#define OPMSG_COMMON_GET_PARAMS_RANGE_WORD_OFFSET (2)
#define OPMSG_COMMON_GET_PARAMS_RANGE_GET(opmsg_common_get_params_ptr) ((opmsg_common_get_params_ptr)->_data[2])
#define OPMSG_COMMON_GET_PARAMS_RANGE_SET(opmsg_common_get_params_ptr, range) ((opmsg_common_get_params_ptr)->_data[2] = (uint16)(range))
#define OPMSG_COMMON_GET_PARAMS_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_GET_PARAMS_CREATE(message_id, offset, range) \
    (uint16)(message_id), \
    (uint16)(offset), \
    (uint16)(range)
#define OPMSG_COMMON_GET_PARAMS_PACK(opmsg_common_get_params_ptr, message_id, offset, range) \
    do { \
        (opmsg_common_get_params_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_get_params_ptr)->_data[1] = (uint16)((uint16)(offset)); \
        (opmsg_common_get_params_ptr)->_data[2] = (uint16)((uint16)(range)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Get_Params_Resp

  DESCRIPTION
    Operator response to the op message GET_PARAMS.

  MEMBERS
    message_id - message id
    parameters - the parameters requested

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_COMMON_GET_PARAMS_RESP;

/* The following macros take OPMSG_COMMON_GET_PARAMS_RESP *opmsg_common_get_params_resp_ptr */
#define OPMSG_COMMON_GET_PARAMS_RESP_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_GET_PARAMS_RESP_MESSAGE_ID_GET(opmsg_common_get_params_resp_ptr) ((OPMSG_COMMON_ID)(opmsg_common_get_params_resp_ptr)->_data[0])
#define OPMSG_COMMON_GET_PARAMS_RESP_MESSAGE_ID_SET(opmsg_common_get_params_resp_ptr, message_id) ((opmsg_common_get_params_resp_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_GET_PARAMS_RESP_PARAMETERS_WORD_OFFSET (1)
#define OPMSG_COMMON_GET_PARAMS_RESP_PARAMETERS_GET(opmsg_common_get_params_resp_ptr) ((opmsg_common_get_params_resp_ptr)->_data[1])
#define OPMSG_COMMON_GET_PARAMS_RESP_PARAMETERS_SET(opmsg_common_get_params_resp_ptr, parameters) ((opmsg_common_get_params_resp_ptr)->_data[1] = (uint16)(parameters))
#define OPMSG_COMMON_GET_PARAMS_RESP_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_GET_PARAMS_RESP_CREATE(message_id, parameters) \
    (uint16)(message_id), \
    (uint16)(parameters)
#define OPMSG_COMMON_GET_PARAMS_RESP_PACK(opmsg_common_get_params_resp_ptr, message_id, parameters) \
    do { \
        (opmsg_common_get_params_resp_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_get_params_resp_ptr)->_data[1] = (uint16)((uint16)(parameters)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Msg_Configure

  DESCRIPTION
    Operator common message for CONFIGURE.

  MEMBERS
    message_id - message id
    key        - key for the type of the configure
    value      - value applied to the operator endpoint

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_COMMON_MSG_CONFIGURE;

/* The following macros take OPMSG_COMMON_MSG_CONFIGURE *opmsg_common_msg_configure_ptr */
#define OPMSG_COMMON_MSG_CONFIGURE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_MSG_CONFIGURE_MESSAGE_ID_GET(opmsg_common_msg_configure_ptr) ((OPMSG_COMMON_ID)(opmsg_common_msg_configure_ptr)->_data[0])
#define OPMSG_COMMON_MSG_CONFIGURE_MESSAGE_ID_SET(opmsg_common_msg_configure_ptr, message_id) ((opmsg_common_msg_configure_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_MSG_CONFIGURE_KEY_WORD_OFFSET (1)
#define OPMSG_COMMON_MSG_CONFIGURE_KEY_GET(opmsg_common_msg_configure_ptr) ((opmsg_common_msg_configure_ptr)->_data[1])
#define OPMSG_COMMON_MSG_CONFIGURE_KEY_SET(opmsg_common_msg_configure_ptr, key) ((opmsg_common_msg_configure_ptr)->_data[1] = (uint16)(key))
#define OPMSG_COMMON_MSG_CONFIGURE_VALUE_WORD_OFFSET (2)
#define OPMSG_COMMON_MSG_CONFIGURE_VALUE_GET(opmsg_common_msg_configure_ptr)  \
    (((uint32)((opmsg_common_msg_configure_ptr)->_data[2]) | \
      ((uint32)((opmsg_common_msg_configure_ptr)->_data[3]) << 16)))
#define OPMSG_COMMON_MSG_CONFIGURE_VALUE_SET(opmsg_common_msg_configure_ptr, value) do { \
        (opmsg_common_msg_configure_ptr)->_data[2] = (uint16)((value) & 0xffff); \
        (opmsg_common_msg_configure_ptr)->_data[3] = (uint16)((value) >> 16); } while (0)
#define OPMSG_COMMON_MSG_CONFIGURE_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_MSG_CONFIGURE_CREATE(message_id, key, value) \
    (uint16)(message_id), \
    (uint16)(key), \
    (uint16)((value) & 0xffff), \
    (uint16)((value) >> 16)
#define OPMSG_COMMON_MSG_CONFIGURE_PACK(opmsg_common_msg_configure_ptr, message_id, key, value) \
    do { \
        (opmsg_common_msg_configure_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_msg_configure_ptr)->_data[1] = (uint16)((uint16)(key)); \
        (opmsg_common_msg_configure_ptr)->_data[2] = (uint16)((uint16)((value) & 0xffff)); \
        (opmsg_common_msg_configure_ptr)->_data[3] = (uint16)(((value) >> 16)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Msg_Set_Sample_Rate

  DESCRIPTION
    Operator common message for SET_SAMPLE_RATE.

  MEMBERS
    message_id  - message id
    sample_rate - sample rate

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_COMMON_MSG_SET_SAMPLE_RATE;

/* The following macros take OPMSG_COMMON_MSG_SET_SAMPLE_RATE *opmsg_common_msg_set_sample_rate_ptr */
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_MESSAGE_ID_GET(opmsg_common_msg_set_sample_rate_ptr) ((OPMSG_COMMON_ID)(opmsg_common_msg_set_sample_rate_ptr)->_data[0])
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_MESSAGE_ID_SET(opmsg_common_msg_set_sample_rate_ptr, message_id) ((opmsg_common_msg_set_sample_rate_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_SAMPLE_RATE_WORD_OFFSET (1)
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_SAMPLE_RATE_GET(opmsg_common_msg_set_sample_rate_ptr)  \
    (((uint32)((opmsg_common_msg_set_sample_rate_ptr)->_data[1]) | \
      ((uint32)((opmsg_common_msg_set_sample_rate_ptr)->_data[2]) << 16)))
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_SAMPLE_RATE_SET(opmsg_common_msg_set_sample_rate_ptr, sample_rate) do { \
        (opmsg_common_msg_set_sample_rate_ptr)->_data[1] = (uint16)((sample_rate) & 0xffff); \
        (opmsg_common_msg_set_sample_rate_ptr)->_data[2] = (uint16)((sample_rate) >> 16); } while (0)
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_CREATE(message_id, sample_rate) \
    (uint16)(message_id), \
    (uint16)((sample_rate) & 0xffff), \
    (uint16)((sample_rate) >> 16)
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_PACK(opmsg_common_msg_set_sample_rate_ptr, message_id, sample_rate) \
    do { \
        (opmsg_common_msg_set_sample_rate_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_msg_set_sample_rate_ptr)->_data[1] = (uint16)((uint16)((sample_rate) & 0xffff)); \
        (opmsg_common_msg_set_sample_rate_ptr)->_data[2] = (uint16)(((sample_rate) >> 16)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Msg_Set_TTP_Latency

  DESCRIPTION
    Operator common message for SET_TTP_LATENCY.

  MEMBERS
    message_id - message id
    latency_ms - Most-significant 16 bits of target latency, in microseconds
    latency_ls - Least-significant 16 bits of target latency, in microseconds

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_COMMON_MSG_SET_TTP_LATENCY;

/* The following macros take OPMSG_COMMON_MSG_SET_TTP_LATENCY *opmsg_common_msg_set_ttp_latency_ptr */
#define OPMSG_COMMON_MSG_SET_TTP_LATENCY_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_MSG_SET_TTP_LATENCY_MESSAGE_ID_GET(opmsg_common_msg_set_ttp_latency_ptr) ((OPMSG_COMMON_ID)(opmsg_common_msg_set_ttp_latency_ptr)->_data[0])
#define OPMSG_COMMON_MSG_SET_TTP_LATENCY_MESSAGE_ID_SET(opmsg_common_msg_set_ttp_latency_ptr, message_id) ((opmsg_common_msg_set_ttp_latency_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_MSG_SET_TTP_LATENCY_LATENCY_MS_WORD_OFFSET (1)
#define OPMSG_COMMON_MSG_SET_TTP_LATENCY_LATENCY_MS_GET(opmsg_common_msg_set_ttp_latency_ptr) ((opmsg_common_msg_set_ttp_latency_ptr)->_data[1])
#define OPMSG_COMMON_MSG_SET_TTP_LATENCY_LATENCY_MS_SET(opmsg_common_msg_set_ttp_latency_ptr, latency_ms) ((opmsg_common_msg_set_ttp_latency_ptr)->_data[1] = (uint16)(latency_ms))
#define OPMSG_COMMON_MSG_SET_TTP_LATENCY_LATENCY_LS_WORD_OFFSET (2)
#define OPMSG_COMMON_MSG_SET_TTP_LATENCY_LATENCY_LS_GET(opmsg_common_msg_set_ttp_latency_ptr) ((opmsg_common_msg_set_ttp_latency_ptr)->_data[2])
#define OPMSG_COMMON_MSG_SET_TTP_LATENCY_LATENCY_LS_SET(opmsg_common_msg_set_ttp_latency_ptr, latency_ls) ((opmsg_common_msg_set_ttp_latency_ptr)->_data[2] = (uint16)(latency_ls))
#define OPMSG_COMMON_MSG_SET_TTP_LATENCY_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_MSG_SET_TTP_LATENCY_CREATE(message_id, latency_ms, latency_ls) \
    (uint16)(message_id), \
    (uint16)(latency_ms), \
    (uint16)(latency_ls)
#define OPMSG_COMMON_MSG_SET_TTP_LATENCY_PACK(opmsg_common_msg_set_ttp_latency_ptr, message_id, latency_ms, latency_ls) \
    do { \
        (opmsg_common_msg_set_ttp_latency_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_msg_set_ttp_latency_ptr)->_data[1] = (uint16)((uint16)(latency_ms)); \
        (opmsg_common_msg_set_ttp_latency_ptr)->_data[2] = (uint16)((uint16)(latency_ls)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Msg_Set_TTP_Params

  DESCRIPTION
    Operator common message for SET_TTP_PARAMS.

  MEMBERS
    message_id     - message id
    filter_gain_ms - Most-significant 16 bits of filter gain value
    filter_gain_ls - Least-significant 16 bits of filter gain value
    error_scale_ms - Most-significant 16 bits of error scale value
    error_scale_ls - Least-significant 16 bits of error scale value
    startup_time   - Startup time, in milliseconds

*******************************************************************************/
typedef struct
{
    uint16 _data[6];
} OPMSG_COMMON_MSG_SET_TTP_PARAMS;

/* The following macros take OPMSG_COMMON_MSG_SET_TTP_PARAMS *opmsg_common_msg_set_ttp_params_ptr */
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_MESSAGE_ID_GET(opmsg_common_msg_set_ttp_params_ptr) ((OPMSG_COMMON_ID)(opmsg_common_msg_set_ttp_params_ptr)->_data[0])
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_MESSAGE_ID_SET(opmsg_common_msg_set_ttp_params_ptr, message_id) ((opmsg_common_msg_set_ttp_params_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_FILTER_GAIN_MS_WORD_OFFSET (1)
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_FILTER_GAIN_MS_GET(opmsg_common_msg_set_ttp_params_ptr) ((opmsg_common_msg_set_ttp_params_ptr)->_data[1])
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_FILTER_GAIN_MS_SET(opmsg_common_msg_set_ttp_params_ptr, filter_gain_ms) ((opmsg_common_msg_set_ttp_params_ptr)->_data[1] = (uint16)(filter_gain_ms))
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_FILTER_GAIN_LS_WORD_OFFSET (2)
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_FILTER_GAIN_LS_GET(opmsg_common_msg_set_ttp_params_ptr) ((opmsg_common_msg_set_ttp_params_ptr)->_data[2])
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_FILTER_GAIN_LS_SET(opmsg_common_msg_set_ttp_params_ptr, filter_gain_ls) ((opmsg_common_msg_set_ttp_params_ptr)->_data[2] = (uint16)(filter_gain_ls))
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_ERROR_SCALE_MS_WORD_OFFSET (3)
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_ERROR_SCALE_MS_GET(opmsg_common_msg_set_ttp_params_ptr) ((opmsg_common_msg_set_ttp_params_ptr)->_data[3])
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_ERROR_SCALE_MS_SET(opmsg_common_msg_set_ttp_params_ptr, error_scale_ms) ((opmsg_common_msg_set_ttp_params_ptr)->_data[3] = (uint16)(error_scale_ms))
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_ERROR_SCALE_LS_WORD_OFFSET (4)
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_ERROR_SCALE_LS_GET(opmsg_common_msg_set_ttp_params_ptr) ((opmsg_common_msg_set_ttp_params_ptr)->_data[4])
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_ERROR_SCALE_LS_SET(opmsg_common_msg_set_ttp_params_ptr, error_scale_ls) ((opmsg_common_msg_set_ttp_params_ptr)->_data[4] = (uint16)(error_scale_ls))
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_STARTUP_TIME_WORD_OFFSET (5)
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_STARTUP_TIME_GET(opmsg_common_msg_set_ttp_params_ptr) ((opmsg_common_msg_set_ttp_params_ptr)->_data[5])
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_STARTUP_TIME_SET(opmsg_common_msg_set_ttp_params_ptr, startup_time) ((opmsg_common_msg_set_ttp_params_ptr)->_data[5] = (uint16)(startup_time))
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_WORD_SIZE (6)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_CREATE(message_id, filter_gain_ms, filter_gain_ls, error_scale_ms, error_scale_ls, startup_time) \
    (uint16)(message_id), \
    (uint16)(filter_gain_ms), \
    (uint16)(filter_gain_ls), \
    (uint16)(error_scale_ms), \
    (uint16)(error_scale_ls), \
    (uint16)(startup_time)
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_PACK(opmsg_common_msg_set_ttp_params_ptr, message_id, filter_gain_ms, filter_gain_ls, error_scale_ms, error_scale_ls, startup_time) \
    do { \
        (opmsg_common_msg_set_ttp_params_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_msg_set_ttp_params_ptr)->_data[1] = (uint16)((uint16)(filter_gain_ms)); \
        (opmsg_common_msg_set_ttp_params_ptr)->_data[2] = (uint16)((uint16)(filter_gain_ls)); \
        (opmsg_common_msg_set_ttp_params_ptr)->_data[3] = (uint16)((uint16)(error_scale_ms)); \
        (opmsg_common_msg_set_ttp_params_ptr)->_data[4] = (uint16)((uint16)(error_scale_ls)); \
        (opmsg_common_msg_set_ttp_params_ptr)->_data[5] = (uint16)((uint16)(startup_time)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Set_Buffer_Size

  DESCRIPTION
    Operator common message for SET_BUFFER_SIZE.

  MEMBERS
    message_id  - message id
    buffer_size - buffer size

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_COMMON_SET_BUFFER_SIZE;

/* The following macros take OPMSG_COMMON_SET_BUFFER_SIZE *opmsg_common_set_buffer_size_ptr */
#define OPMSG_COMMON_SET_BUFFER_SIZE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_SET_BUFFER_SIZE_MESSAGE_ID_GET(opmsg_common_set_buffer_size_ptr) ((OPMSG_COMMON_ID)(opmsg_common_set_buffer_size_ptr)->_data[0])
#define OPMSG_COMMON_SET_BUFFER_SIZE_MESSAGE_ID_SET(opmsg_common_set_buffer_size_ptr, message_id) ((opmsg_common_set_buffer_size_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_SET_BUFFER_SIZE_BUFFER_SIZE_WORD_OFFSET (1)
#define OPMSG_COMMON_SET_BUFFER_SIZE_BUFFER_SIZE_GET(opmsg_common_set_buffer_size_ptr) ((opmsg_common_set_buffer_size_ptr)->_data[1])
#define OPMSG_COMMON_SET_BUFFER_SIZE_BUFFER_SIZE_SET(opmsg_common_set_buffer_size_ptr, buffer_size) ((opmsg_common_set_buffer_size_ptr)->_data[1] = (uint16)(buffer_size))
#define OPMSG_COMMON_SET_BUFFER_SIZE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_SET_BUFFER_SIZE_CREATE(message_id, buffer_size) \
    (uint16)(message_id), \
    (uint16)(buffer_size)
#define OPMSG_COMMON_SET_BUFFER_SIZE_PACK(opmsg_common_set_buffer_size_ptr, message_id, buffer_size) \
    do { \
        (opmsg_common_set_buffer_size_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_set_buffer_size_ptr)->_data[1] = (uint16)((uint16)(buffer_size)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Set_Control

  DESCRIPTION
    Operator common message for SET_CONTROL.

  MEMBERS
    message_id - message id
    control_id - control id
    value      - value

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_COMMON_SET_CONTROL;

/* The following macros take OPMSG_COMMON_SET_CONTROL *opmsg_common_set_control_ptr */
#define OPMSG_COMMON_SET_CONTROL_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_SET_CONTROL_MESSAGE_ID_GET(opmsg_common_set_control_ptr) ((OPMSG_COMMON_ID)(opmsg_common_set_control_ptr)->_data[0])
#define OPMSG_COMMON_SET_CONTROL_MESSAGE_ID_SET(opmsg_common_set_control_ptr, message_id) ((opmsg_common_set_control_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_SET_CONTROL_CONTROL_ID_WORD_OFFSET (1)
#define OPMSG_COMMON_SET_CONTROL_CONTROL_ID_GET(opmsg_common_set_control_ptr) ((opmsg_common_set_control_ptr)->_data[1])
#define OPMSG_COMMON_SET_CONTROL_CONTROL_ID_SET(opmsg_common_set_control_ptr, control_id) ((opmsg_common_set_control_ptr)->_data[1] = (uint16)(control_id))
#define OPMSG_COMMON_SET_CONTROL_VALUE_WORD_OFFSET (2)
#define OPMSG_COMMON_SET_CONTROL_VALUE_GET(opmsg_common_set_control_ptr)  \
    (((uint32)((opmsg_common_set_control_ptr)->_data[2]) | \
      ((uint32)((opmsg_common_set_control_ptr)->_data[3]) << 16)))
#define OPMSG_COMMON_SET_CONTROL_VALUE_SET(opmsg_common_set_control_ptr, value) do { \
        (opmsg_common_set_control_ptr)->_data[2] = (uint16)((value) & 0xffff); \
        (opmsg_common_set_control_ptr)->_data[3] = (uint16)((value) >> 16); } while (0)
#define OPMSG_COMMON_SET_CONTROL_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_SET_CONTROL_CREATE(message_id, control_id, value) \
    (uint16)(message_id), \
    (uint16)(control_id), \
    (uint16)((value) & 0xffff), \
    (uint16)((value) >> 16)
#define OPMSG_COMMON_SET_CONTROL_PACK(opmsg_common_set_control_ptr, message_id, control_id, value) \
    do { \
        (opmsg_common_set_control_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_set_control_ptr)->_data[1] = (uint16)((uint16)(control_id)); \
        (opmsg_common_set_control_ptr)->_data[2] = (uint16)((uint16)((value) & 0xffff)); \
        (opmsg_common_set_control_ptr)->_data[3] = (uint16)(((value) >> 16)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Set_Params

  DESCRIPTION
    Operator common message for SET_PARAMS.

  MEMBERS
    message_id - message id
    offset     - offset
    range      - range
    values     - values

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_COMMON_SET_PARAMS;

/* The following macros take OPMSG_COMMON_SET_PARAMS *opmsg_common_set_params_ptr */
#define OPMSG_COMMON_SET_PARAMS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_SET_PARAMS_MESSAGE_ID_GET(opmsg_common_set_params_ptr) ((OPMSG_COMMON_ID)(opmsg_common_set_params_ptr)->_data[0])
#define OPMSG_COMMON_SET_PARAMS_MESSAGE_ID_SET(opmsg_common_set_params_ptr, message_id) ((opmsg_common_set_params_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_SET_PARAMS_OFFSET_WORD_OFFSET (1)
#define OPMSG_COMMON_SET_PARAMS_OFFSET_GET(opmsg_common_set_params_ptr) ((opmsg_common_set_params_ptr)->_data[1])
#define OPMSG_COMMON_SET_PARAMS_OFFSET_SET(opmsg_common_set_params_ptr, offset) ((opmsg_common_set_params_ptr)->_data[1] = (uint16)(offset))
#define OPMSG_COMMON_SET_PARAMS_RANGE_WORD_OFFSET (2)
#define OPMSG_COMMON_SET_PARAMS_RANGE_GET(opmsg_common_set_params_ptr) ((opmsg_common_set_params_ptr)->_data[2])
#define OPMSG_COMMON_SET_PARAMS_RANGE_SET(opmsg_common_set_params_ptr, range) ((opmsg_common_set_params_ptr)->_data[2] = (uint16)(range))
#define OPMSG_COMMON_SET_PARAMS_VALUES_WORD_OFFSET (3)
#define OPMSG_COMMON_SET_PARAMS_VALUES_GET(opmsg_common_set_params_ptr) ((opmsg_common_set_params_ptr)->_data[3])
#define OPMSG_COMMON_SET_PARAMS_VALUES_SET(opmsg_common_set_params_ptr, values) ((opmsg_common_set_params_ptr)->_data[3] = (uint16)(values))
#define OPMSG_COMMON_SET_PARAMS_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_SET_PARAMS_CREATE(message_id, offset, range, values) \
    (uint16)(message_id), \
    (uint16)(offset), \
    (uint16)(range), \
    (uint16)(values)
#define OPMSG_COMMON_SET_PARAMS_PACK(opmsg_common_set_params_ptr, message_id, num_blocks, offset, range, parameter_value_msw, parameter_value_lsw) \
    do { \
        (opmsg_common_set_params_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_set_params_ptr)->_data[1] = (uint16)((uint16)(num_blocks)); \
        (opmsg_common_set_params_ptr)->_data[2] = (uint16)((uint16)(offset)); \
        (opmsg_common_set_params_ptr)->_data[3] = (uint16)((uint16)(range)); \
        (opmsg_common_set_params_ptr)->_data[4] = (uint16)((uint16)(parameter_value_msw)); \
        (opmsg_common_set_params_ptr)->_data[5] = (uint16)((uint16)(parameter_value_lsw)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Set_Terminal_Buffer_Size

  DESCRIPTION
    Operator common message for SET_TERMINAL_BUFFER_SIZE.

  MEMBERS
    message_id  - message id
    buffer_size - buffer size
    sinks       - sinks
    sources     - sources

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE;

/* The following macros take OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE *opmsg_common_set_terminal_buffer_size_ptr */
#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_MESSAGE_ID_GET(opmsg_common_set_terminal_buffer_size_ptr) ((OPMSG_COMMON_ID)(opmsg_common_set_terminal_buffer_size_ptr)->_data[0])
#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_MESSAGE_ID_SET(opmsg_common_set_terminal_buffer_size_ptr, message_id) ((opmsg_common_set_terminal_buffer_size_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_BUFFER_SIZE_WORD_OFFSET (1)
#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_BUFFER_SIZE_GET(opmsg_common_set_terminal_buffer_size_ptr) ((opmsg_common_set_terminal_buffer_size_ptr)->_data[1])
#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_BUFFER_SIZE_SET(opmsg_common_set_terminal_buffer_size_ptr, buffer_size) ((opmsg_common_set_terminal_buffer_size_ptr)->_data[1] = (uint16)(buffer_size))
#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_SINKS_WORD_OFFSET (2)
#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_SINKS_GET(opmsg_common_set_terminal_buffer_size_ptr) ((opmsg_common_set_terminal_buffer_size_ptr)->_data[2])
#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_SINKS_SET(opmsg_common_set_terminal_buffer_size_ptr, sinks) ((opmsg_common_set_terminal_buffer_size_ptr)->_data[2] = (uint16)(sinks))
#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_SOURCES_WORD_OFFSET (3)
#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_SOURCES_GET(opmsg_common_set_terminal_buffer_size_ptr) ((opmsg_common_set_terminal_buffer_size_ptr)->_data[3])
#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_SOURCES_SET(opmsg_common_set_terminal_buffer_size_ptr, sources) ((opmsg_common_set_terminal_buffer_size_ptr)->_data[3] = (uint16)(sources))
#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_CREATE(message_id, buffer_size, sinks, sources) \
    (uint16)(message_id), \
    (uint16)(buffer_size), \
    (uint16)(sinks), \
    (uint16)(sources)
#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_PACK(opmsg_common_set_terminal_buffer_size_ptr, message_id, buffer_size, sinks, sources) \
    do { \
        (opmsg_common_set_terminal_buffer_size_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_set_terminal_buffer_size_ptr)->_data[1] = (uint16)((uint16)(buffer_size)); \
        (opmsg_common_set_terminal_buffer_size_ptr)->_data[2] = (uint16)((uint16)(sinks)); \
        (opmsg_common_set_terminal_buffer_size_ptr)->_data[3] = (uint16)((uint16)(sources)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Common_Set_Ucid

  DESCRIPTION
    Operator common message for SET_UCID.

  MEMBERS
    message_id - message id
    ucid       - ucid

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_COMMON_SET_UCID;

/* The following macros take OPMSG_COMMON_SET_UCID *opmsg_common_set_ucid_ptr */
#define OPMSG_COMMON_SET_UCID_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_SET_UCID_MESSAGE_ID_GET(opmsg_common_set_ucid_ptr) ((OPMSG_COMMON_ID)(opmsg_common_set_ucid_ptr)->_data[0])
#define OPMSG_COMMON_SET_UCID_MESSAGE_ID_SET(opmsg_common_set_ucid_ptr, message_id) ((opmsg_common_set_ucid_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_SET_UCID_UCID_WORD_OFFSET (1)
#define OPMSG_COMMON_SET_UCID_UCID_GET(opmsg_common_set_ucid_ptr) ((opmsg_common_set_ucid_ptr)->_data[1])
#define OPMSG_COMMON_SET_UCID_UCID_SET(opmsg_common_set_ucid_ptr, ucid) ((opmsg_common_set_ucid_ptr)->_data[1] = (uint16)(ucid))
#define OPMSG_COMMON_SET_UCID_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_SET_UCID_CREATE(message_id, ucid) \
    (uint16)(message_id), \
    (uint16)(ucid)
#define OPMSG_COMMON_SET_UCID_PACK(opmsg_common_set_ucid_ptr, message_id, ucid) \
    do { \
        (opmsg_common_set_ucid_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_set_ucid_ptr)->_data[1] = (uint16)((uint16)(ucid)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Iir_Resampler_Set_Config

  DESCRIPTION
    IIR Resampler operator message for SET_CONFIG.

  MEMBERS
    message_id   - message id
    config_flags - configuration flags

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_IIR_RESAMPLER_SET_CONFIG;

/* The following macros take OPMSG_IIR_RESAMPLER_SET_CONFIG *opmsg_iir_resampler_set_config_ptr */
#define OPMSG_IIR_RESAMPLER_SET_CONFIG_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_IIR_RESAMPLER_SET_CONFIG_MESSAGE_ID_GET(opmsg_iir_resampler_set_config_ptr) ((OPMSG_IIR_RESAMPLER_ID)(opmsg_iir_resampler_set_config_ptr)->_data[0])
#define OPMSG_IIR_RESAMPLER_SET_CONFIG_MESSAGE_ID_SET(opmsg_iir_resampler_set_config_ptr, message_id) ((opmsg_iir_resampler_set_config_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_IIR_RESAMPLER_SET_CONFIG_CONFIG_FLAGS_WORD_OFFSET (1)
#define OPMSG_IIR_RESAMPLER_SET_CONFIG_CONFIG_FLAGS_GET(opmsg_iir_resampler_set_config_ptr) ((opmsg_iir_resampler_set_config_ptr)->_data[1])
#define OPMSG_IIR_RESAMPLER_SET_CONFIG_CONFIG_FLAGS_SET(opmsg_iir_resampler_set_config_ptr, config_flags) ((opmsg_iir_resampler_set_config_ptr)->_data[1] = (uint16)(config_flags))
#define OPMSG_IIR_RESAMPLER_SET_CONFIG_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_IIR_RESAMPLER_SET_CONFIG_CREATE(message_id, config_flags) \
    (uint16)(message_id), \
    (uint16)(config_flags)
#define OPMSG_IIR_RESAMPLER_SET_CONFIG_PACK(opmsg_iir_resampler_set_config_ptr, message_id, config_flags) \
    do { \
        (opmsg_iir_resampler_set_config_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_iir_resampler_set_config_ptr)->_data[1] = (uint16)((uint16)(config_flags)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Iir_Resampler_Set_Conversion_Rate

  DESCRIPTION
    IIR Resampler operator message for SET_CONVERSION_RATE.

  MEMBERS
    message_id      - message id
    conversion_rate - conversion rate

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_IIR_RESAMPLER_SET_CONVERSION_RATE;

/* The following macros take OPMSG_IIR_RESAMPLER_SET_CONVERSION_RATE *opmsg_iir_resampler_set_conversion_rate_ptr */
#define OPMSG_IIR_RESAMPLER_SET_CONVERSION_RATE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_IIR_RESAMPLER_SET_CONVERSION_RATE_MESSAGE_ID_GET(opmsg_iir_resampler_set_conversion_rate_ptr) ((OPMSG_IIR_RESAMPLER_ID)(opmsg_iir_resampler_set_conversion_rate_ptr)->_data[0])
#define OPMSG_IIR_RESAMPLER_SET_CONVERSION_RATE_MESSAGE_ID_SET(opmsg_iir_resampler_set_conversion_rate_ptr, message_id) ((opmsg_iir_resampler_set_conversion_rate_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_IIR_RESAMPLER_SET_CONVERSION_RATE_CONVERSION_RATE_WORD_OFFSET (1)
#define OPMSG_IIR_RESAMPLER_SET_CONVERSION_RATE_CONVERSION_RATE_GET(opmsg_iir_resampler_set_conversion_rate_ptr) ((opmsg_iir_resampler_set_conversion_rate_ptr)->_data[1])
#define OPMSG_IIR_RESAMPLER_SET_CONVERSION_RATE_CONVERSION_RATE_SET(opmsg_iir_resampler_set_conversion_rate_ptr, conversion_rate) ((opmsg_iir_resampler_set_conversion_rate_ptr)->_data[1] = (uint16)(conversion_rate))
#define OPMSG_IIR_RESAMPLER_SET_CONVERSION_RATE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_IIR_RESAMPLER_SET_CONVERSION_RATE_CREATE(message_id, conversion_rate) \
    (uint16)(message_id), \
    (uint16)(conversion_rate)
#define OPMSG_IIR_RESAMPLER_SET_CONVERSION_RATE_PACK(opmsg_iir_resampler_set_conversion_rate_ptr, message_id, conversion_rate) \
    do { \
        (opmsg_iir_resampler_set_conversion_rate_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_iir_resampler_set_conversion_rate_ptr)->_data[1] = (uint16)((uint16)(conversion_rate)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Mixer_Set_Metadata_Stream

  DESCRIPTION
    Mixer operator message for SET_METADATA_STREAM.

  MEMBERS
    message_id     - message id
    tracks_primary - Boolean indicating whether the metadata stream should track
                     the primary stream. 0 = FALSE, 1 = TRUE, other values are
                     not valid
    stream_number  - the number of the stream to propagate metadata from

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_MIXER_SET_METADATA_STREAM;

/* The following macros take OPMSG_MIXER_SET_METADATA_STREAM *opmsg_mixer_set_metadata_stream_ptr */
#define OPMSG_MIXER_SET_METADATA_STREAM_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_MIXER_SET_METADATA_STREAM_MESSAGE_ID_GET(opmsg_mixer_set_metadata_stream_ptr) ((OPMSG_MIXER_ID)(opmsg_mixer_set_metadata_stream_ptr)->_data[0])
#define OPMSG_MIXER_SET_METADATA_STREAM_MESSAGE_ID_SET(opmsg_mixer_set_metadata_stream_ptr, message_id) ((opmsg_mixer_set_metadata_stream_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_MIXER_SET_METADATA_STREAM_TRACKS_PRIMARY_WORD_OFFSET (1)
#define OPMSG_MIXER_SET_METADATA_STREAM_TRACKS_PRIMARY_GET(opmsg_mixer_set_metadata_stream_ptr) ((opmsg_mixer_set_metadata_stream_ptr)->_data[1])
#define OPMSG_MIXER_SET_METADATA_STREAM_TRACKS_PRIMARY_SET(opmsg_mixer_set_metadata_stream_ptr, tracks_primary) ((opmsg_mixer_set_metadata_stream_ptr)->_data[1] = (uint16)(tracks_primary))
#define OPMSG_MIXER_SET_METADATA_STREAM_STREAM_NUMBER_WORD_OFFSET (2)
#define OPMSG_MIXER_SET_METADATA_STREAM_STREAM_NUMBER_GET(opmsg_mixer_set_metadata_stream_ptr) ((opmsg_mixer_set_metadata_stream_ptr)->_data[2])
#define OPMSG_MIXER_SET_METADATA_STREAM_STREAM_NUMBER_SET(opmsg_mixer_set_metadata_stream_ptr, stream_number) ((opmsg_mixer_set_metadata_stream_ptr)->_data[2] = (uint16)(stream_number))
#define OPMSG_MIXER_SET_METADATA_STREAM_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_MIXER_SET_METADATA_STREAM_CREATE(message_id, tracks_primary, stream_number) \
    (uint16)(message_id), \
    (uint16)(tracks_primary), \
    (uint16)(stream_number)
#define OPMSG_MIXER_SET_METADATA_STREAM_PACK(opmsg_mixer_set_metadata_stream_ptr, message_id, tracks_primary, stream_number) \
    do { \
        (opmsg_mixer_set_metadata_stream_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_mixer_set_metadata_stream_ptr)->_data[1] = (uint16)((uint16)(tracks_primary)); \
        (opmsg_mixer_set_metadata_stream_ptr)->_data[2] = (uint16)((uint16)(stream_number)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Mixer_Set_Primary_Stream

  DESCRIPTION
    Mixer operator message for SET_PRIMARY_STREAM.

  MEMBERS
    message_id    - message id
    stream_number - the number of the stream to be set primary

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_MIXER_SET_PRIMARY_STREAM;

/* The following macros take OPMSG_MIXER_SET_PRIMARY_STREAM *opmsg_mixer_set_primary_stream_ptr */
#define OPMSG_MIXER_SET_PRIMARY_STREAM_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_MIXER_SET_PRIMARY_STREAM_MESSAGE_ID_GET(opmsg_mixer_set_primary_stream_ptr) ((OPMSG_MIXER_ID)(opmsg_mixer_set_primary_stream_ptr)->_data[0])
#define OPMSG_MIXER_SET_PRIMARY_STREAM_MESSAGE_ID_SET(opmsg_mixer_set_primary_stream_ptr, message_id) ((opmsg_mixer_set_primary_stream_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_MIXER_SET_PRIMARY_STREAM_STREAM_NUMBER_WORD_OFFSET (1)
#define OPMSG_MIXER_SET_PRIMARY_STREAM_STREAM_NUMBER_GET(opmsg_mixer_set_primary_stream_ptr) ((opmsg_mixer_set_primary_stream_ptr)->_data[1])
#define OPMSG_MIXER_SET_PRIMARY_STREAM_STREAM_NUMBER_SET(opmsg_mixer_set_primary_stream_ptr, stream_number) ((opmsg_mixer_set_primary_stream_ptr)->_data[1] = (uint16)(stream_number))
#define OPMSG_MIXER_SET_PRIMARY_STREAM_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_MIXER_SET_PRIMARY_STREAM_CREATE(message_id, stream_number) \
    (uint16)(message_id), \
    (uint16)(stream_number)
#define OPMSG_MIXER_SET_PRIMARY_STREAM_PACK(opmsg_mixer_set_primary_stream_ptr, message_id, stream_number) \
    do { \
        (opmsg_mixer_set_primary_stream_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_mixer_set_primary_stream_ptr)->_data[1] = (uint16)((uint16)(stream_number)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Mixer_Set_Ramp_Num_Samples

  DESCRIPTION
    Mixer operator message for SET_RAMP_NUM_SAMPLES.

  MEMBERS
    message_id      - message id
    num_samples_msb - number of samples ms 8 bits
    num_samples_lsb - number of samples ls 16 bits

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_MIXER_SET_RAMP_NUM_SAMPLES;

/* The following macros take OPMSG_MIXER_SET_RAMP_NUM_SAMPLES *opmsg_mixer_set_ramp_num_samples_ptr */
#define OPMSG_MIXER_SET_RAMP_NUM_SAMPLES_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_MIXER_SET_RAMP_NUM_SAMPLES_MESSAGE_ID_GET(opmsg_mixer_set_ramp_num_samples_ptr) ((OPMSG_MIXER_ID)(opmsg_mixer_set_ramp_num_samples_ptr)->_data[0])
#define OPMSG_MIXER_SET_RAMP_NUM_SAMPLES_MESSAGE_ID_SET(opmsg_mixer_set_ramp_num_samples_ptr, message_id) ((opmsg_mixer_set_ramp_num_samples_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_MIXER_SET_RAMP_NUM_SAMPLES_NUM_SAMPLES_MSB_WORD_OFFSET (1)
#define OPMSG_MIXER_SET_RAMP_NUM_SAMPLES_NUM_SAMPLES_MSB_GET(opmsg_mixer_set_ramp_num_samples_ptr) ((opmsg_mixer_set_ramp_num_samples_ptr)->_data[1])
#define OPMSG_MIXER_SET_RAMP_NUM_SAMPLES_NUM_SAMPLES_MSB_SET(opmsg_mixer_set_ramp_num_samples_ptr, num_samples_msb) ((opmsg_mixer_set_ramp_num_samples_ptr)->_data[1] = (uint16)(num_samples_msb))
#define OPMSG_MIXER_SET_RAMP_NUM_SAMPLES_NUM_SAMPLES_LSB_WORD_OFFSET (2)
#define OPMSG_MIXER_SET_RAMP_NUM_SAMPLES_NUM_SAMPLES_LSB_GET(opmsg_mixer_set_ramp_num_samples_ptr) ((opmsg_mixer_set_ramp_num_samples_ptr)->_data[2])
#define OPMSG_MIXER_SET_RAMP_NUM_SAMPLES_NUM_SAMPLES_LSB_SET(opmsg_mixer_set_ramp_num_samples_ptr, num_samples_lsb) ((opmsg_mixer_set_ramp_num_samples_ptr)->_data[2] = (uint16)(num_samples_lsb))
#define OPMSG_MIXER_SET_RAMP_NUM_SAMPLES_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_MIXER_SET_RAMP_NUM_SAMPLES_CREATE(message_id, num_samples_msb, num_samples_lsb) \
    (uint16)(message_id), \
    (uint16)(num_samples_msb), \
    (uint16)(num_samples_lsb)
#define OPMSG_MIXER_SET_RAMP_NUM_SAMPLES_PACK(opmsg_mixer_set_ramp_num_samples_ptr, message_id, num_samples_msb, num_samples_lsb) \
    do { \
        (opmsg_mixer_set_ramp_num_samples_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_mixer_set_ramp_num_samples_ptr)->_data[1] = (uint16)((uint16)(num_samples_msb)); \
        (opmsg_mixer_set_ramp_num_samples_ptr)->_data[2] = (uint16)((uint16)(num_samples_lsb)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Mixer_Set_Stream_Channels

  DESCRIPTION
    Mixer operator message for SET_STREAM_CHANNELS.

  MEMBERS
    message_id           - message id
    stream1_num_channels - stream 1 number of channels
    stream2_num_channels - stream 2 number of channels
    stream3_num_channels - stream 3 number of channels

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_MIXER_SET_STREAM_CHANNELS;

/* The following macros take OPMSG_MIXER_SET_STREAM_CHANNELS *opmsg_mixer_set_stream_channels_ptr */
#define OPMSG_MIXER_SET_STREAM_CHANNELS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_MIXER_SET_STREAM_CHANNELS_MESSAGE_ID_GET(opmsg_mixer_set_stream_channels_ptr) ((OPMSG_MIXER_ID)(opmsg_mixer_set_stream_channels_ptr)->_data[0])
#define OPMSG_MIXER_SET_STREAM_CHANNELS_MESSAGE_ID_SET(opmsg_mixer_set_stream_channels_ptr, message_id) ((opmsg_mixer_set_stream_channels_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_MIXER_SET_STREAM_CHANNELS_STREAM1_NUM_CHANNELS_WORD_OFFSET (1)
#define OPMSG_MIXER_SET_STREAM_CHANNELS_STREAM1_NUM_CHANNELS_GET(opmsg_mixer_set_stream_channels_ptr) ((opmsg_mixer_set_stream_channels_ptr)->_data[1])
#define OPMSG_MIXER_SET_STREAM_CHANNELS_STREAM1_NUM_CHANNELS_SET(opmsg_mixer_set_stream_channels_ptr, stream1_num_channels) ((opmsg_mixer_set_stream_channels_ptr)->_data[1] = (uint16)(stream1_num_channels))
#define OPMSG_MIXER_SET_STREAM_CHANNELS_STREAM2_NUM_CHANNELS_WORD_OFFSET (2)
#define OPMSG_MIXER_SET_STREAM_CHANNELS_STREAM2_NUM_CHANNELS_GET(opmsg_mixer_set_stream_channels_ptr) ((opmsg_mixer_set_stream_channels_ptr)->_data[2])
#define OPMSG_MIXER_SET_STREAM_CHANNELS_STREAM2_NUM_CHANNELS_SET(opmsg_mixer_set_stream_channels_ptr, stream2_num_channels) ((opmsg_mixer_set_stream_channels_ptr)->_data[2] = (uint16)(stream2_num_channels))
#define OPMSG_MIXER_SET_STREAM_CHANNELS_STREAM3_NUM_CHANNELS_WORD_OFFSET (3)
#define OPMSG_MIXER_SET_STREAM_CHANNELS_STREAM3_NUM_CHANNELS_GET(opmsg_mixer_set_stream_channels_ptr) ((opmsg_mixer_set_stream_channels_ptr)->_data[3])
#define OPMSG_MIXER_SET_STREAM_CHANNELS_STREAM3_NUM_CHANNELS_SET(opmsg_mixer_set_stream_channels_ptr, stream3_num_channels) ((opmsg_mixer_set_stream_channels_ptr)->_data[3] = (uint16)(stream3_num_channels))
#define OPMSG_MIXER_SET_STREAM_CHANNELS_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_MIXER_SET_STREAM_CHANNELS_CREATE(message_id, stream1_num_channels, stream2_num_channels, stream3_num_channels) \
    (uint16)(message_id), \
    (uint16)(stream1_num_channels), \
    (uint16)(stream2_num_channels), \
    (uint16)(stream3_num_channels)
#define OPMSG_MIXER_SET_STREAM_CHANNELS_PACK(opmsg_mixer_set_stream_channels_ptr, message_id, stream1_num_channels, stream2_num_channels, stream3_num_channels) \
    do { \
        (opmsg_mixer_set_stream_channels_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_mixer_set_stream_channels_ptr)->_data[1] = (uint16)((uint16)(stream1_num_channels)); \
        (opmsg_mixer_set_stream_channels_ptr)->_data[2] = (uint16)((uint16)(stream2_num_channels)); \
        (opmsg_mixer_set_stream_channels_ptr)->_data[3] = (uint16)((uint16)(stream3_num_channels)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Mixer_Set_Stream_Gains

  DESCRIPTION
    Mixer operator message for SET_STREAM_GAINS.

  MEMBERS
    message_id   - message id
    stream1_gain - stream 1 gain
    stream2_gain - stream 2 gain
    stream3_gain - stream 3 gain

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_MIXER_SET_STREAM_GAINS;

/* The following macros take OPMSG_MIXER_SET_STREAM_GAINS *opmsg_mixer_set_stream_gains_ptr */
#define OPMSG_MIXER_SET_STREAM_GAINS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_MIXER_SET_STREAM_GAINS_MESSAGE_ID_GET(opmsg_mixer_set_stream_gains_ptr) ((OPMSG_MIXER_ID)(opmsg_mixer_set_stream_gains_ptr)->_data[0])
#define OPMSG_MIXER_SET_STREAM_GAINS_MESSAGE_ID_SET(opmsg_mixer_set_stream_gains_ptr, message_id) ((opmsg_mixer_set_stream_gains_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_MIXER_SET_STREAM_GAINS_STREAM1_GAIN_WORD_OFFSET (1)
#define OPMSG_MIXER_SET_STREAM_GAINS_STREAM1_GAIN_GET(opmsg_mixer_set_stream_gains_ptr) ((opmsg_mixer_set_stream_gains_ptr)->_data[1])
#define OPMSG_MIXER_SET_STREAM_GAINS_STREAM1_GAIN_SET(opmsg_mixer_set_stream_gains_ptr, stream1_gain) ((opmsg_mixer_set_stream_gains_ptr)->_data[1] = (uint16)(stream1_gain))
#define OPMSG_MIXER_SET_STREAM_GAINS_STREAM2_GAIN_WORD_OFFSET (2)
#define OPMSG_MIXER_SET_STREAM_GAINS_STREAM2_GAIN_GET(opmsg_mixer_set_stream_gains_ptr) ((opmsg_mixer_set_stream_gains_ptr)->_data[2])
#define OPMSG_MIXER_SET_STREAM_GAINS_STREAM2_GAIN_SET(opmsg_mixer_set_stream_gains_ptr, stream2_gain) ((opmsg_mixer_set_stream_gains_ptr)->_data[2] = (uint16)(stream2_gain))
#define OPMSG_MIXER_SET_STREAM_GAINS_STREAM3_GAIN_WORD_OFFSET (3)
#define OPMSG_MIXER_SET_STREAM_GAINS_STREAM3_GAIN_GET(opmsg_mixer_set_stream_gains_ptr) ((opmsg_mixer_set_stream_gains_ptr)->_data[3])
#define OPMSG_MIXER_SET_STREAM_GAINS_STREAM3_GAIN_SET(opmsg_mixer_set_stream_gains_ptr, stream3_gain) ((opmsg_mixer_set_stream_gains_ptr)->_data[3] = (uint16)(stream3_gain))
#define OPMSG_MIXER_SET_STREAM_GAINS_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_MIXER_SET_STREAM_GAINS_CREATE(message_id, stream1_gain, stream2_gain, stream3_gain) \
    (uint16)(message_id), \
    (uint16)(stream1_gain), \
    (uint16)(stream2_gain), \
    (uint16)(stream3_gain)
#define OPMSG_MIXER_SET_STREAM_GAINS_PACK(opmsg_mixer_set_stream_gains_ptr, message_id, stream1_gain, stream2_gain, stream3_gain) \
    do { \
        (opmsg_mixer_set_stream_gains_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_mixer_set_stream_gains_ptr)->_data[1] = (uint16)((uint16)(stream1_gain)); \
        (opmsg_mixer_set_stream_gains_ptr)->_data[2] = (uint16)((uint16)(stream2_gain)); \
        (opmsg_mixer_set_stream_gains_ptr)->_data[3] = (uint16)((uint16)(stream3_gain)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Msg_Iir_Resampler_Set_Sample_Rates

  DESCRIPTION
    IIR Resampler operator message for SET_SAMPLE_RATES.

  MEMBERS
    message_id  - message id
    input_rate  - input rate divided by 25
    output_rate - output rate divided by 25

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES;

/* The following macros take OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES *opmsg_msg_iir_resampler_set_sample_rates_ptr */
#define OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES_MESSAGE_ID_GET(opmsg_msg_iir_resampler_set_sample_rates_ptr) ((OPMSG_IIR_RESAMPLER_ID)(opmsg_msg_iir_resampler_set_sample_rates_ptr)->_data[0])
#define OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES_MESSAGE_ID_SET(opmsg_msg_iir_resampler_set_sample_rates_ptr, message_id) ((opmsg_msg_iir_resampler_set_sample_rates_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES_INPUT_RATE_WORD_OFFSET (1)
#define OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES_INPUT_RATE_GET(opmsg_msg_iir_resampler_set_sample_rates_ptr) ((opmsg_msg_iir_resampler_set_sample_rates_ptr)->_data[1])
#define OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES_INPUT_RATE_SET(opmsg_msg_iir_resampler_set_sample_rates_ptr, input_rate) ((opmsg_msg_iir_resampler_set_sample_rates_ptr)->_data[1] = (uint16)(input_rate))
#define OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES_OUTPUT_RATE_WORD_OFFSET (2)
#define OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES_OUTPUT_RATE_GET(opmsg_msg_iir_resampler_set_sample_rates_ptr) ((opmsg_msg_iir_resampler_set_sample_rates_ptr)->_data[2])
#define OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES_OUTPUT_RATE_SET(opmsg_msg_iir_resampler_set_sample_rates_ptr, output_rate) ((opmsg_msg_iir_resampler_set_sample_rates_ptr)->_data[2] = (uint16)(output_rate))
#define OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES_CREATE(message_id, input_rate, output_rate) \
    (uint16)(message_id), \
    (uint16)(input_rate), \
    (uint16)(output_rate)
#define OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES_PACK(opmsg_msg_iir_resampler_set_sample_rates_ptr, message_id, input_rate, output_rate) \
    do { \
        (opmsg_msg_iir_resampler_set_sample_rates_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_msg_iir_resampler_set_sample_rates_ptr)->_data[1] = (uint16)((uint16)(input_rate)); \
        (opmsg_msg_iir_resampler_set_sample_rates_ptr)->_data[2] = (uint16)((uint16)(output_rate)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Resampler_Filter_Coefficients

  DESCRIPTION
    Resampler operator message for FILTER_COEFFICIENTS.

  MEMBERS
    message_id  - message id
    start_index - start index
    count       - count
    packed_data - packed data

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_RESAMPLER_FILTER_COEFFICIENTS;

/* The following macros take OPMSG_RESAMPLER_FILTER_COEFFICIENTS *opmsg_resampler_filter_coefficients_ptr */
#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_MESSAGE_ID_GET(opmsg_resampler_filter_coefficients_ptr) ((OPMSG_RESAMPLER_ID)(opmsg_resampler_filter_coefficients_ptr)->_data[0])
#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_MESSAGE_ID_SET(opmsg_resampler_filter_coefficients_ptr, message_id) ((opmsg_resampler_filter_coefficients_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_START_INDEX_WORD_OFFSET (1)
#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_START_INDEX_GET(opmsg_resampler_filter_coefficients_ptr) ((opmsg_resampler_filter_coefficients_ptr)->_data[1])
#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_START_INDEX_SET(opmsg_resampler_filter_coefficients_ptr, start_index) ((opmsg_resampler_filter_coefficients_ptr)->_data[1] = (uint16)(start_index))
#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_COUNT_WORD_OFFSET (2)
#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_COUNT_GET(opmsg_resampler_filter_coefficients_ptr) ((opmsg_resampler_filter_coefficients_ptr)->_data[2])
#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_COUNT_SET(opmsg_resampler_filter_coefficients_ptr, count) ((opmsg_resampler_filter_coefficients_ptr)->_data[2] = (uint16)(count))
#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_PACKED_DATA_WORD_OFFSET (3)
#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_PACKED_DATA_GET(opmsg_resampler_filter_coefficients_ptr) ((opmsg_resampler_filter_coefficients_ptr)->_data[3])
#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_PACKED_DATA_SET(opmsg_resampler_filter_coefficients_ptr, packed_data) ((opmsg_resampler_filter_coefficients_ptr)->_data[3] = (uint16)(packed_data))
#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_CREATE(message_id, start_index, count, packed_data) \
    (uint16)(message_id), \
    (uint16)(start_index), \
    (uint16)(count), \
    (uint16)(packed_data)
#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_PACK(opmsg_resampler_filter_coefficients_ptr, message_id, start_index, count, packed_data) \
    do { \
        (opmsg_resampler_filter_coefficients_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_resampler_filter_coefficients_ptr)->_data[1] = (uint16)((uint16)(start_index)); \
        (opmsg_resampler_filter_coefficients_ptr)->_data[2] = (uint16)((uint16)(count)); \
        (opmsg_resampler_filter_coefficients_ptr)->_data[3] = (uint16)((uint16)(packed_data)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Resampler_Set_Conversion_Rate

  DESCRIPTION
    Resampler operator message for SET_CONVERSION_RATE.

  MEMBERS
    message_id      - message id
    conversion_rate - conversion rate

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_RESAMPLER_SET_CONVERSION_RATE;

/* The following macros take OPMSG_RESAMPLER_SET_CONVERSION_RATE *opmsg_resampler_set_conversion_rate_ptr */
#define OPMSG_RESAMPLER_SET_CONVERSION_RATE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_RESAMPLER_SET_CONVERSION_RATE_MESSAGE_ID_GET(opmsg_resampler_set_conversion_rate_ptr) ((OPMSG_RESAMPLER_ID)(opmsg_resampler_set_conversion_rate_ptr)->_data[0])
#define OPMSG_RESAMPLER_SET_CONVERSION_RATE_MESSAGE_ID_SET(opmsg_resampler_set_conversion_rate_ptr, message_id) ((opmsg_resampler_set_conversion_rate_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_RESAMPLER_SET_CONVERSION_RATE_CONVERSION_RATE_WORD_OFFSET (1)
#define OPMSG_RESAMPLER_SET_CONVERSION_RATE_CONVERSION_RATE_GET(opmsg_resampler_set_conversion_rate_ptr) ((opmsg_resampler_set_conversion_rate_ptr)->_data[1])
#define OPMSG_RESAMPLER_SET_CONVERSION_RATE_CONVERSION_RATE_SET(opmsg_resampler_set_conversion_rate_ptr, conversion_rate) ((opmsg_resampler_set_conversion_rate_ptr)->_data[1] = (uint16)(conversion_rate))
#define OPMSG_RESAMPLER_SET_CONVERSION_RATE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_RESAMPLER_SET_CONVERSION_RATE_CREATE(message_id, conversion_rate) \
    (uint16)(message_id), \
    (uint16)(conversion_rate)
#define OPMSG_RESAMPLER_SET_CONVERSION_RATE_PACK(opmsg_resampler_set_conversion_rate_ptr, message_id, conversion_rate) \
    do { \
        (opmsg_resampler_set_conversion_rate_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_resampler_set_conversion_rate_ptr)->_data[1] = (uint16)((uint16)(conversion_rate)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Resampler_Set_Custom_Rate

  DESCRIPTION
    Resampler operator message for SET_CUSTOM_RATE.

  MEMBERS
    message_id        - message id
    int_up_rate       - internal parameters int up rate
    filter_size       - filter size
    frac_down_rate_ms - internal parameters frac up rate
    frac_down_rate_ls - internal parameters frac down rate ls
    int_down_rate     - internal parameters int down rate

*******************************************************************************/
typedef struct
{
    uint16 _data[6];
} OPMSG_RESAMPLER_SET_CUSTOM_RATE;

/* The following macros take OPMSG_RESAMPLER_SET_CUSTOM_RATE *opmsg_resampler_set_custom_rate_ptr */
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_MESSAGE_ID_GET(opmsg_resampler_set_custom_rate_ptr) ((OPMSG_RESAMPLER_ID)(opmsg_resampler_set_custom_rate_ptr)->_data[0])
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_MESSAGE_ID_SET(opmsg_resampler_set_custom_rate_ptr, message_id) ((opmsg_resampler_set_custom_rate_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_INT_UP_RATE_WORD_OFFSET (1)
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_INT_UP_RATE_GET(opmsg_resampler_set_custom_rate_ptr) ((opmsg_resampler_set_custom_rate_ptr)->_data[1])
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_INT_UP_RATE_SET(opmsg_resampler_set_custom_rate_ptr, int_up_rate) ((opmsg_resampler_set_custom_rate_ptr)->_data[1] = (uint16)(int_up_rate))
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_FILTER_SIZE_WORD_OFFSET (2)
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_FILTER_SIZE_GET(opmsg_resampler_set_custom_rate_ptr) ((opmsg_resampler_set_custom_rate_ptr)->_data[2])
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_FILTER_SIZE_SET(opmsg_resampler_set_custom_rate_ptr, filter_size) ((opmsg_resampler_set_custom_rate_ptr)->_data[2] = (uint16)(filter_size))
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_FRAC_DOWN_RATE_MS_WORD_OFFSET (3)
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_FRAC_DOWN_RATE_MS_GET(opmsg_resampler_set_custom_rate_ptr) ((opmsg_resampler_set_custom_rate_ptr)->_data[3])
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_FRAC_DOWN_RATE_MS_SET(opmsg_resampler_set_custom_rate_ptr, frac_down_rate_ms) ((opmsg_resampler_set_custom_rate_ptr)->_data[3] = (uint16)(frac_down_rate_ms))
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_FRAC_DOWN_RATE_LS_WORD_OFFSET (4)
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_FRAC_DOWN_RATE_LS_GET(opmsg_resampler_set_custom_rate_ptr) ((opmsg_resampler_set_custom_rate_ptr)->_data[4])
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_FRAC_DOWN_RATE_LS_SET(opmsg_resampler_set_custom_rate_ptr, frac_down_rate_ls) ((opmsg_resampler_set_custom_rate_ptr)->_data[4] = (uint16)(frac_down_rate_ls))
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_INT_DOWN_RATE_WORD_OFFSET (5)
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_INT_DOWN_RATE_GET(opmsg_resampler_set_custom_rate_ptr) ((opmsg_resampler_set_custom_rate_ptr)->_data[5])
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_INT_DOWN_RATE_SET(opmsg_resampler_set_custom_rate_ptr, int_down_rate) ((opmsg_resampler_set_custom_rate_ptr)->_data[5] = (uint16)(int_down_rate))
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_WORD_SIZE (6)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_CREATE(message_id, int_up_rate, filter_size, frac_down_rate_ms, frac_down_rate_ls, int_down_rate) \
    (uint16)(message_id), \
    (uint16)(int_up_rate), \
    (uint16)(filter_size), \
    (uint16)(frac_down_rate_ms), \
    (uint16)(frac_down_rate_ls), \
    (uint16)(int_down_rate)
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_PACK(opmsg_resampler_set_custom_rate_ptr, message_id, int_up_rate, filter_size, frac_down_rate_ms, frac_down_rate_ls, int_down_rate) \
    do { \
        (opmsg_resampler_set_custom_rate_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_resampler_set_custom_rate_ptr)->_data[1] = (uint16)((uint16)(int_up_rate)); \
        (opmsg_resampler_set_custom_rate_ptr)->_data[2] = (uint16)((uint16)(filter_size)); \
        (opmsg_resampler_set_custom_rate_ptr)->_data[3] = (uint16)((uint16)(frac_down_rate_ms)); \
        (opmsg_resampler_set_custom_rate_ptr)->_data[4] = (uint16)((uint16)(frac_down_rate_ls)); \
        (opmsg_resampler_set_custom_rate_ptr)->_data[5] = (uint16)((uint16)(int_down_rate)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Rtp_Set_Codec_Type

  DESCRIPTION
    Rtp operator message for SET_RTP_CODEC_TYPE.

  MEMBERS
    message_id - message id
    codec_type - working mode (APTX = 0, SBC = 1, ATRAC = 2, MP3 = 3, AAC = 4 )

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_RTP_SET_CODEC_TYPE;

/* The following macros take OPMSG_RTP_SET_CODEC_TYPE *opmsg_rtp_set_codec_type_ptr */
#define OPMSG_RTP_SET_CODEC_TYPE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_RTP_SET_CODEC_TYPE_MESSAGE_ID_GET(opmsg_rtp_set_codec_type_ptr) ((OPMSG_RTP_DECODE_ID)(opmsg_rtp_set_codec_type_ptr)->_data[0])
#define OPMSG_RTP_SET_CODEC_TYPE_MESSAGE_ID_SET(opmsg_rtp_set_codec_type_ptr, message_id) ((opmsg_rtp_set_codec_type_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_RTP_SET_CODEC_TYPE_CODEC_TYPE_WORD_OFFSET (1)
#define OPMSG_RTP_SET_CODEC_TYPE_CODEC_TYPE_GET(opmsg_rtp_set_codec_type_ptr) ((opmsg_rtp_set_codec_type_ptr)->_data[1])
#define OPMSG_RTP_SET_CODEC_TYPE_CODEC_TYPE_SET(opmsg_rtp_set_codec_type_ptr, codec_type) ((opmsg_rtp_set_codec_type_ptr)->_data[1] = (uint16)(codec_type))
#define OPMSG_RTP_SET_CODEC_TYPE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_RTP_SET_CODEC_TYPE_CREATE(message_id, codec_type) \
    (uint16)(message_id), \
    (uint16)(codec_type)
#define OPMSG_RTP_SET_CODEC_TYPE_PACK(opmsg_rtp_set_codec_type_ptr, message_id, codec_type) \
    do { \
        (opmsg_rtp_set_codec_type_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_rtp_set_codec_type_ptr)->_data[1] = (uint16)((uint16)(codec_type)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Rtp_Set_Content_Protection

  DESCRIPTION
    Rtp operator message for setting the content protection.

  MEMBERS
    message_id                 - message id
    content_protection_enabled - 0 = content protection disabled, 1 = content
                                 protection enabled

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_RTP_SET_CONTENT_PROTECTION;

/* The following macros take OPMSG_RTP_SET_CONTENT_PROTECTION *opmsg_rtp_set_content_protection_ptr */
#define OPMSG_RTP_SET_CONTENT_PROTECTION_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_RTP_SET_CONTENT_PROTECTION_MESSAGE_ID_GET(opmsg_rtp_set_content_protection_ptr) ((OPMSG_RTP_DECODE_ID)(opmsg_rtp_set_content_protection_ptr)->_data[0])
#define OPMSG_RTP_SET_CONTENT_PROTECTION_MESSAGE_ID_SET(opmsg_rtp_set_content_protection_ptr, message_id) ((opmsg_rtp_set_content_protection_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_RTP_SET_CONTENT_PROTECTION_CONTENT_PROTECTION_ENABLED_WORD_OFFSET (1)
#define OPMSG_RTP_SET_CONTENT_PROTECTION_CONTENT_PROTECTION_ENABLED_GET(opmsg_rtp_set_content_protection_ptr) ((opmsg_rtp_set_content_protection_ptr)->_data[1])
#define OPMSG_RTP_SET_CONTENT_PROTECTION_CONTENT_PROTECTION_ENABLED_SET(opmsg_rtp_set_content_protection_ptr, content_protection_enabled) ((opmsg_rtp_set_content_protection_ptr)->_data[1] = (uint16)(content_protection_enabled))
#define OPMSG_RTP_SET_CONTENT_PROTECTION_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_RTP_SET_CONTENT_PROTECTION_CREATE(message_id, content_protection_enabled) \
    (uint16)(message_id), \
    (uint16)(content_protection_enabled)
#define OPMSG_RTP_SET_CONTENT_PROTECTION_PACK(opmsg_rtp_set_content_protection_ptr, message_id, content_protection_enabled) \
    do { \
        (opmsg_rtp_set_content_protection_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_rtp_set_content_protection_ptr)->_data[1] = (uint16)((uint16)(content_protection_enabled)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Rtp_Set_Working_Mode

  DESCRIPTION
    Rtp operator message for SET_WORKING_MODE.

  MEMBERS
    message_id   - message id
    working_mode - working mode (PASSTHROUGH = 0, DECODE = 1, STRIP = 2)

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_RTP_SET_WORKING_MODE;

/* The following macros take OPMSG_RTP_SET_WORKING_MODE *opmsg_rtp_set_working_mode_ptr */
#define OPMSG_RTP_SET_WORKING_MODE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_RTP_SET_WORKING_MODE_MESSAGE_ID_GET(opmsg_rtp_set_working_mode_ptr) ((OPMSG_RTP_DECODE_ID)(opmsg_rtp_set_working_mode_ptr)->_data[0])
#define OPMSG_RTP_SET_WORKING_MODE_MESSAGE_ID_SET(opmsg_rtp_set_working_mode_ptr, message_id) ((opmsg_rtp_set_working_mode_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_RTP_SET_WORKING_MODE_WORKING_MODE_WORD_OFFSET (1)
#define OPMSG_RTP_SET_WORKING_MODE_WORKING_MODE_GET(opmsg_rtp_set_working_mode_ptr) ((opmsg_rtp_set_working_mode_ptr)->_data[1])
#define OPMSG_RTP_SET_WORKING_MODE_WORKING_MODE_SET(opmsg_rtp_set_working_mode_ptr, working_mode) ((opmsg_rtp_set_working_mode_ptr)->_data[1] = (uint16)(working_mode))
#define OPMSG_RTP_SET_WORKING_MODE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_RTP_SET_WORKING_MODE_CREATE(message_id, working_mode) \
    (uint16)(message_id), \
    (uint16)(working_mode)
#define OPMSG_RTP_SET_WORKING_MODE_PACK(opmsg_rtp_set_working_mode_ptr, message_id, working_mode) \
    do { \
        (opmsg_rtp_set_working_mode_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_rtp_set_working_mode_ptr)->_data[1] = (uint16)((uint16)(working_mode)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Splitter_Set_Data_Format

  DESCRIPTION
    Splitter operator message for SET_DATA_FORMAT.

  MEMBERS
    message_id - message id
    data_type  - data type

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_SPLITTER_SET_DATA_FORMAT;

/* The following macros take OPMSG_SPLITTER_SET_DATA_FORMAT *opmsg_splitter_set_data_format_ptr */
#define OPMSG_SPLITTER_SET_DATA_FORMAT_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPLITTER_SET_DATA_FORMAT_MESSAGE_ID_GET(opmsg_splitter_set_data_format_ptr) ((OPMSG_SPLITTER_ID)(opmsg_splitter_set_data_format_ptr)->_data[0])
#define OPMSG_SPLITTER_SET_DATA_FORMAT_MESSAGE_ID_SET(opmsg_splitter_set_data_format_ptr, message_id) ((opmsg_splitter_set_data_format_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPLITTER_SET_DATA_FORMAT_DATA_TYPE_WORD_OFFSET (1)
#define OPMSG_SPLITTER_SET_DATA_FORMAT_DATA_TYPE_GET(opmsg_splitter_set_data_format_ptr) ((opmsg_splitter_set_data_format_ptr)->_data[1])
#define OPMSG_SPLITTER_SET_DATA_FORMAT_DATA_TYPE_SET(opmsg_splitter_set_data_format_ptr, data_type) ((opmsg_splitter_set_data_format_ptr)->_data[1] = (uint16)(data_type))
#define OPMSG_SPLITTER_SET_DATA_FORMAT_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPLITTER_SET_DATA_FORMAT_CREATE(message_id, data_type) \
    (uint16)(message_id), \
    (uint16)(data_type)
#define OPMSG_SPLITTER_SET_DATA_FORMAT_PACK(opmsg_splitter_set_data_format_ptr, message_id, data_type) \
    do { \
        (opmsg_splitter_set_data_format_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_splitter_set_data_format_ptr)->_data[1] = (uint16)((uint16)(data_type)); \
    } while (0)


/*******************************************************************************

  NAME
    Opmsg_Splitter_Set_Running_Streams

  DESCRIPTION
    Splitter operator message for SET_RUNNING_STREAMS.

  MEMBERS
    message_id      - message id
    running_streams - Bit field indicating the streams to treat as running. Bit
                      n = stream n

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_SPLITTER_SET_RUNNING_STREAMS;

/* The following macros take OPMSG_SPLITTER_SET_RUNNING_STREAMS *opmsg_splitter_set_running_streams_ptr */
#define OPMSG_SPLITTER_SET_RUNNING_STREAMS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPLITTER_SET_RUNNING_STREAMS_MESSAGE_ID_GET(opmsg_splitter_set_running_streams_ptr) ((OPMSG_SPLITTER_ID)(opmsg_splitter_set_running_streams_ptr)->_data[0])
#define OPMSG_SPLITTER_SET_RUNNING_STREAMS_MESSAGE_ID_SET(opmsg_splitter_set_running_streams_ptr, message_id) ((opmsg_splitter_set_running_streams_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPLITTER_SET_RUNNING_STREAMS_RUNNING_STREAMS_WORD_OFFSET (1)
#define OPMSG_SPLITTER_SET_RUNNING_STREAMS_RUNNING_STREAMS_GET(opmsg_splitter_set_running_streams_ptr) ((opmsg_splitter_set_running_streams_ptr)->_data[1])
#define OPMSG_SPLITTER_SET_RUNNING_STREAMS_RUNNING_STREAMS_SET(opmsg_splitter_set_running_streams_ptr, running_streams) ((opmsg_splitter_set_running_streams_ptr)->_data[1] = (uint16)(running_streams))
#define OPMSG_SPLITTER_SET_RUNNING_STREAMS_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPLITTER_SET_RUNNING_STREAMS_CREATE(message_id, running_streams) \
    (uint16)(message_id), \
    (uint16)(running_streams)
#define OPMSG_SPLITTER_SET_RUNNING_STREAMS_PACK(opmsg_splitter_set_running_streams_ptr, message_id, running_streams) \
    do { \
        (opmsg_splitter_set_running_streams_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_splitter_set_running_streams_ptr)->_data[1] = (uint16)((uint16)(running_streams)); \
    } while (0)


#endif /* OPMSG_PRIM_H */


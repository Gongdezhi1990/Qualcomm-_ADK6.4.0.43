/*****************************************************************************

            (c) Qualcomm Technologies International, Ltd. 2019
            Confidential information of Qualcomm

            Refer to LICENSE.txt included with this source for details
            on the license terms.

            WARNING: This is an auto-generated file!
                     DO NOT EDIT!

*****************************************************************************/
#ifndef OPMSG_PRIM_H
#define OPMSG_PRIM_H


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
    SET_TASK_PERIOD              - AEC REFERENCE operator by default runs per
                                   1ms task period, this message is for changing
                                   the default task period and is mainly to
                                   reduce the cpu usage by the operator. There
                                   are some limitations, see the description of
                                   the message for details.
    SAME_INPUT_OUTPUT_CLK_SOURCE - tell the operator that the back-end of the
                                   operator are coming from same clock source.
                                   This is for optimisation purpose and it's
                                   recommended to be enabled for use cases where
                                   speaker input and microphone output are
                                   synchronised (e.g. SCO and USB voice use
                                   cases). Note: Send/Resend this message when
                                   all microphone input/output and REFERENCE
                                   output are disconnected.
    MUTE_MIC_OUTPUT              - mutes/unmutes microphone output of AEC
                                   Reference
    ENABLE_SPKR_INPUT_GATE       - Enables AEC reference to have a TTP based
                                   gate at init phase, so no input is fed to
                                   speaker side until the latency has reached a
                                   predefined level. NOTE: This message is no
                                   longer supported/required, AEC Reference
                                   capability now supports full TTP playback in
                                   speaker path.
    SET_SAMPLE_RATES             - This input message allows a user to set the
                                   input & output sample rate.

*******************************************************************************/
typedef enum
{
    OPMSG_AEC_REFERENCE_ID_SET_TASK_PERIOD = 0x0007,
    OPMSG_AEC_REFERENCE_ID_SAME_INPUT_OUTPUT_CLK_SOURCE = 0x0008,
    OPMSG_AEC_REFERENCE_ID_MUTE_MIC_OUTPUT = 0x0009,
    OPMSG_AEC_REFERENCE_ID_ENABLE_SPKR_INPUT_GATE = 0x000A,
    OPMSG_AEC_REFERENCE_ID_SET_SAMPLE_RATES = 0x00FE
} OPMSG_AEC_REFERENCE_ID;
/*******************************************************************************

  NAME
    OPMSG_ANC_TUNING_ID

  DESCRIPTION
    ANC tuning configuration messages

 VALUES
    FRONTEND_CONFIG - Sets the mic and speaker configuration for the ANC tuning
                      capability

*******************************************************************************/
typedef enum
{
    OPMSG_ANC_TUNING_ID_FRONTEND_CONFIG = 0x0001
} OPMSG_ANC_TUNING_ID;
/*******************************************************************************

  NAME
    OPMSG_APTX_ADAPTIVE_ENC_ID

  DESCRIPTION
    Aptx Adaptive Encoder configuration messages

 VALUES
    SET_ENCODING_PARAMS - This message is used for setting the parameters that
                          Apt Adaptive uses for encoding the audio/voice as an
                          Aptx adaptive frame
    SET_BIT_RATE        - This message is used for setting the bit rate for Aptx
                          Adaptive Encoder

*******************************************************************************/
typedef enum
{
    OPMSG_APTX_ADAPTIVE_ENC_ID_SET_ENCODING_PARAMS = 0x0001,
    OPMSG_APTX_ADAPTIVE_ENC_ID_SET_BIT_RATE = 0x0002
} OPMSG_APTX_ADAPTIVE_ENC_ID;
/*******************************************************************************

  NAME
    OPMSG_ASYNC_WBS_DEC_ID

  DESCRIPTION
    Asynchronous WBS configuration messages

 VALUES
    FRAME_COUNTS      -
    FORCE_PLC_OFF     -
    SET_BITPOOL_VALUE -
    PROC_TIME_MEASURE -
    SET_FROM_AIR_INFO -

*******************************************************************************/
typedef enum
{
    OPMSG_ASYNC_WBS_DEC_ID_FRAME_COUNTS = 0x0001,
    OPMSG_ASYNC_WBS_DEC_ID_FORCE_PLC_OFF = 0x0002,
    OPMSG_ASYNC_WBS_DEC_ID_SET_BITPOOL_VALUE = 0x0003,
    OPMSG_ASYNC_WBS_DEC_ID_PROC_TIME_MEASURE = 0x000C,
    OPMSG_ASYNC_WBS_DEC_ID_SET_FROM_AIR_INFO = 0x00FE
} OPMSG_ASYNC_WBS_DEC_ID;
/*******************************************************************************

  NAME
    OPMSG_ASYNC_WBS_ENC_ID

  DESCRIPTION
    Asynchronous WBS configuration messages

 VALUES
    SET_BITPOOL_VALUE -
    SET_TO_AIR_INFO   -

*******************************************************************************/
typedef enum
{
    OPMSG_ASYNC_WBS_ENC_ID_SET_BITPOOL_VALUE = 0x0001,
    OPMSG_ASYNC_WBS_ENC_ID_SET_TO_AIR_INFO = 0x00FD
} OPMSG_ASYNC_WBS_ENC_ID;
/*******************************************************************************

  NAME
    OPMSG_ASYNC_WBS_ID

  DESCRIPTION
    Asynchronous WBS configuration messages

 VALUES
    SET_BITPOOL_VALUE -

*******************************************************************************/
typedef enum
{
    OPMSG_ASYNC_WBS_ID_SET_BITPOOL_VALUE = 0x00FC
} OPMSG_ASYNC_WBS_ID;
/*******************************************************************************

  NAME
    OPMSG_CELT_ENC_ID

  DESCRIPTION
    SBC Encoder configuration messages

 VALUES
    SET_ENCODING_PARAM - This message is used for setting the parameters that
                         CELT uses for encoding the audio as an CELT frame

*******************************************************************************/
typedef enum
{
    OPMSG_CELT_ENC_ID_SET_ENCODING_PARAM = 0x0001
} OPMSG_CELT_ENC_ID;
/*******************************************************************************

  NAME
    OPMSG_CHANNEL_MIXER_ID

  DESCRIPTION
    Channel Mixer configuration messages

 VALUES
    SET_CH_MIXER_PARAMETERS - Set the parameters for channel mixer

*******************************************************************************/
typedef enum
{
    OPMSG_CHANNEL_MIXER_ID_SET_CH_MIXER_PARAMETERS = 0x0001
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
    OPMSG_COMMON_SET_METADATA_DELAY
                   - Set the algorithmic delay of the operator processing in
                     samples
    OPMSG_COMMON_SET_LATENCY_LIMITS
                   - Tell the operator the minimun and maximum latency for
                     time-to-play. The TTP generator will be reset if the
                     latency exceeds these bounds. A zero value for the maximum
                     latency means there is no upper bound. Format(4words):
                     32-bit min, max latency in microseconds
    OPMSG_COMMON_GET_VOICE_QUALITY
                   - Tell the cvc send voice quality
    OPMSG_COMMON_SET_RATE_ADJUST_CURRENT_RATE
                   - Message to set current warp rate of a standalone rate
                     adjust operator, the warp rate is in chip fractional
                     format, and it shows how much the input rate will be
                     adjusted. E.g. a rate of 0.001 will slow down the consuming
                     of input by 0.1%, likewise a rate of -0.001 will speed of
                     the consuming by 0.1%. Note: This message is intended to be
                     used by the framework only, and shall not be used directly
                     by user.
    OPMSG_COMMON_SET_RATE_ADJUST_TARGET_RATE
                   - Message to set the target rate of a standalone rate adjust
                     operator. The format and meaning of this message is the
                     same as for OPMSG_COMMON_SET_RATE_ADJUST_CURRENT_RATE,
                     however when using this message the operator will not
                     change the current rate immediately, instead it will
                     gradually move the current rate towards target rate value.
                     Note: This message is intended to be used by the framework
                     only, and shall not be used directly by user.
    OPMSG_COMMON_SET_RATE_ADJUST_PASSTHROUGH_MODE
                   - Message to set/clear the passthrough mode of a standalone
                     rate adjust operator Note: This message is intended to be
                     used by the framework only, and shall not be used directly
                     by user.

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
    OPMSG_COMMON_SET_TTP_PARAMS = 0x2013,
    OPMSG_COMMON_SET_METADATA_DELAY = 0x2014,
    OPMSG_COMMON_SET_LATENCY_LIMITS = 0x2015,
    OPMSG_COMMON_GET_VOICE_QUALITY = 0x2016,
    OPMSG_COMMON_SET_RATE_ADJUST_CURRENT_RATE = 0x2017,
    OPMSG_COMMON_SET_RATE_ADJUST_TARGET_RATE = 0x2018,
    OPMSG_COMMON_SET_RATE_ADJUST_PASSTHROUGH_MODE = 0x2019
} OPMSG_COMMON_ID;
/*******************************************************************************

  NAME
    OPMSG_CONFIGURATION_KEYS

  DESCRIPTION
    Configuration keys of the operator endpoint.

 VALUES
    OPMSG_OP_TERMINAL_CBOPS_PARAMETERS
                   - Key for passing cbops parameters.
    OPMSG_OP_TERMINAL_DATA_FORMAT
                   - Used for getting the data format of the operator endpoint.
    OPMSG_OP_TERMINAL_DETAILS
                   - Details of the endpoint. Currently only used to check if
                     the operator endpoint is real.
    OPMSG_OP_TERMINAL_KICK_PERIOD
                   - The operator terminal kick period
    OPMSG_OP_TERMINAL_BLOCK_SIZE
                   - Terminal block size
    OPMSG_OP_TERMINAL_PROC_TIME
                   - Processing time of upstream operator.
    OPMSG_OP_TERMINAL_RATEMATCH_ABILITY
                   - Request the type of ratematching support the terminal of
                     the operator possesses.
    OPMSG_OP_TERMINAL_RATEMATCH_RATE
                   - Request the measured rate used for ratematching.
    OPMSG_OP_TERMINAL_RATEMATCH_ENACTING
                   - Ratematching enactment setting enable(d)/disable(d).
    OPMSG_OP_TERMINAL_RATEMATCH_ADJUSTMENT
                   - Apply a rate adjustment value.
    OPMSG_OP_TERMINAL_RATEMATCH_MEASUREMENT
                   - Request a rate triple for ratematching from a non-enacting
                     endpoint.
    OPMSG_OP_TERMINAL_RATEMATCH_REFERENCE
                   - Apply a rate triple from the reference endpoint to an
                     enacting endpoint.

*******************************************************************************/
typedef enum
{
    OPMSG_OP_TERMINAL_CBOPS_PARAMETERS = 0x0001,
    OPMSG_OP_TERMINAL_DATA_FORMAT = 0x0002,
    OPMSG_OP_TERMINAL_DETAILS = 0x0003,
    OPMSG_OP_TERMINAL_KICK_PERIOD = 0x0004,
    OPMSG_OP_TERMINAL_BLOCK_SIZE = 0x0005,
    OPMSG_OP_TERMINAL_PROC_TIME = 0x0006,
    OPMSG_OP_TERMINAL_RATEMATCH_ABILITY = 0x0007,
    OPMSG_OP_TERMINAL_RATEMATCH_RATE = 0x0008,
    OPMSG_OP_TERMINAL_RATEMATCH_ENACTING = 0x0009,
    OPMSG_OP_TERMINAL_RATEMATCH_ADJUSTMENT = 0x000A,
    OPMSG_OP_TERMINAL_RATEMATCH_MEASUREMENT = 0x000B,
    OPMSG_OP_TERMINAL_RATEMATCH_REFERENCE = 0x000C
} OPMSG_CONFIGURATION_KEYS;
/*******************************************************************************

  NAME
    OPMSG_CONSUMER_ID

  DESCRIPTION
    Consumer capability configuration messages

 VALUES
    CHANGE_INPUT_DATA_TYPE - This input message allows a user to change data
                             type for the consumer capability.
    ENABLE_BACK_KICK       - Boolean argument, enables back kicking from the
                             test consumer capability.

*******************************************************************************/
typedef enum
{
    OPMSG_CONSUMER_ID_CHANGE_INPUT_DATA_TYPE = 0x000A,
    OPMSG_CONSUMER_ID_ENABLE_BACK_KICK = 0x000B
} OPMSG_CONSUMER_ID;
/*******************************************************************************

  NAME
    OPMSG_CONTROL

  DESCRIPTION
    The list of possible contol Ids for set control operator message. Also
    the common override values.

 VALUES
    MODE_ID       - Control Id for the Mode control
    MUTE_ID       - Control Id for the Mute control
    OMNI_ID       - Control Id for the Omni-mode control
    DUMMY_ID      - Dummy value to force the enumeration size to 16-bit
    OBPM_OVERRIDE - OBPM override bit mask

*******************************************************************************/
typedef enum
{
    OPMSG_CONTROL_MODE_ID = 0x1,
    OPMSG_CONTROL_MUTE_ID = 0x2,
    OPMSG_CONTROL_OMNI_ID = 0x3,
    OPMSG_CONTROL_DUMMY_ID = 0x7fff
#define OPMSG_CONTROL_OBPM_OVERRIDE ((OPMSG_CONTROL) 0x8000)
} OPMSG_CONTROL;
/*******************************************************************************

  NAME
    OPMSG_CVC_SEND_MODE

  DESCRIPTION
    CVC Send mode type: mode can be one of the following:

 VALUES
    STANDBY         - Silences the output.
    FULL_PROC       - Performs the enabled cVc processing.
    LOWVOLUME       - Reduces the processing when the volume is below a
                      threshold to reduce power consumption. Only applicable to
                      headset Capabilities.
    PASS_THRU_LEFT  - Passes the primary input to the output with minimal
                      processing.
    PASS_THRU_RIGHT - Passes the secondary input to the output with minimal
                      processing. For single microphone Capabilities, the
                      primary microphone is used as per pass-through (left)
                      mode.
    PASS_THRU_MIC3  - Passes the 3rd MIC input to the output with minimal
                      processing.
    PASS_THRU_MIC4  - Passes the 4th MIC input to the output with minimal
                      processing.

*******************************************************************************/
typedef enum
{
    OPMSG_CVC_SEND_MODE_STANDBY = 0x0001,
    OPMSG_CVC_SEND_MODE_FULL_PROC = 0x0002,
    OPMSG_CVC_SEND_MODE_LOWVOLUME = 0x0003,
    OPMSG_CVC_SEND_MODE_PASS_THRU_LEFT = 0x0004,
    OPMSG_CVC_SEND_MODE_PASS_THRU_RIGHT = 0x0005,
    OPMSG_CVC_SEND_MODE_PASS_THRU_MIC3 = 0x0006,
    OPMSG_CVC_SEND_MODE_PASS_THRU_MIC4 = 0x0007
} OPMSG_CVC_SEND_MODE;
/*******************************************************************************

  NAME
    OPMSG_DELAY_ID

  DESCRIPTION
    DELAY configuration messages

 VALUES
    SET_EXTERNAL_MEMORY - Allows host to set extern memory for delay .

*******************************************************************************/
typedef enum
{
    OPMSG_DELAY_ID_SET_EXTERNAL_MEMORY = 0x0001
} OPMSG_DELAY_ID;
/*******************************************************************************

  NAME
    OPMSG_DOWNLOAD_DIRECT_ACCESS_TEST_ID

  DESCRIPTION
    Direct access action messages

 VALUES
    FILE_OPEN  - Opens a file: it gets a filename as input, requests the file
                 details from the Apps SubSystem, and sets a callback which
                 updates the file record in the Audio SubSystem, passing a file
                 handle back to the user
    FILE_READ  - Reads data from a file with the parameters: file handle
                 obtained at FILE_OPEN, amount to read [octets], offset within
                 the file [octets]
    FILE_CLOSE - Closes a file identified by the file handle obtained at
                 FILE_OPEN

*******************************************************************************/
typedef enum
{
    OPMSG_DOWNLOAD_DIRECT_ACCESS_TEST_ID_FILE_OPEN = 0x0001,
    OPMSG_DOWNLOAD_DIRECT_ACCESS_TEST_ID_FILE_READ = 0x0002,
    OPMSG_DOWNLOAD_DIRECT_ACCESS_TEST_ID_FILE_CLOSE = 0x0003
} OPMSG_DOWNLOAD_DIRECT_ACCESS_TEST_ID;
/*******************************************************************************

  NAME
    OPMSG_DOWNLOAD_PS_TEST_ID

  DESCRIPTION
    pskey download command messages

 VALUES
    ENTRY_READ_REQ   - Entry read request
    ENTRY_WRITE_REQ  - Entry write request
    ENTRY_DELETE_REQ - Entry delete request
    DELETE_REQ       - Delete rank request
    EXECUTE_REQ      - Request to execute already stored commands

*******************************************************************************/
typedef enum
{
    OPMSG_DOWNLOAD_PS_TEST_ID_ENTRY_READ_REQ = 0x0001,
    OPMSG_DOWNLOAD_PS_TEST_ID_ENTRY_WRITE_REQ = 0x0002,
    OPMSG_DOWNLOAD_PS_TEST_ID_ENTRY_DELETE_REQ = 0x0003,
    OPMSG_DOWNLOAD_PS_TEST_ID_DELETE_REQ = 0x0004,
    OPMSG_DOWNLOAD_PS_TEST_ID_EXECUTE_REQ = 0x0005
} OPMSG_DOWNLOAD_PS_TEST_ID;
/*******************************************************************************

  NAME
    OPMSG_DOWNLOAD_PS_TEST_RESP_ID

  DESCRIPTION
    pskey download response messages

 VALUES
    ENTRY_READ   - Entry read response
    ENTRY_WRITE  - Entry write response
    ENTRY_DELETE - Entry delete response
    DELETE       - Delete rank response
    EXECUTE      - Execute response

*******************************************************************************/
typedef enum
{
    OPMSG_DOWNLOAD_PS_TEST_RESP_ID_ENTRY_READ = 0x0006,
    OPMSG_DOWNLOAD_PS_TEST_RESP_ID_ENTRY_WRITE = 0x0007,
    OPMSG_DOWNLOAD_PS_TEST_RESP_ID_ENTRY_DELETE = 0x0008,
    OPMSG_DOWNLOAD_PS_TEST_RESP_ID_DELETE = 0x0009,
    OPMSG_DOWNLOAD_PS_TEST_RESP_ID_EXECUTE = 0x000A
} OPMSG_DOWNLOAD_PS_TEST_RESP_ID;
/*******************************************************************************

  NAME
    OPMSG_DOWNLOAD_PS_TEST_RESP_IND_ID

  DESCRIPTION
    Unsolicited message to indicate execution of the pskey operations

 VALUES
    ENTRY_READ   - Indication that the data from the pskey entry read are
                   included in the message
    ENTRY_WRITE  - Indication that pskey entry write is done
    ENTRY_DELETE - Indication that pskey entry delete is done
    DELETE       - Indication that pskey rank delete is done
    EXECUTE      - Indication that ps_execute is completed and all stored
                   messages are executed

*******************************************************************************/
typedef enum
{
    OPMSG_DOWNLOAD_PS_TEST_RESP_IND_ID_ENTRY_READ = 0x000B,
    OPMSG_DOWNLOAD_PS_TEST_RESP_IND_ID_ENTRY_WRITE = 0x000C,
    OPMSG_DOWNLOAD_PS_TEST_RESP_IND_ID_ENTRY_DELETE = 0x000D,
    OPMSG_DOWNLOAD_PS_TEST_RESP_IND_ID_DELETE = 0x000E,
    OPMSG_DOWNLOAD_PS_TEST_RESP_IND_ID_EXECUTE = 0x000F
} OPMSG_DOWNLOAD_PS_TEST_RESP_IND_ID;
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
    OPMSG_FRAMEWORK_SET_BDADDR
                   - Complement to GET_BDADDR
    OPMSG_FRAMEWORK_GET_BDADDR
                   - Special message from audio to apps0. Apps0 is expected to
                     trap/intercept a message of this type, and return BD ADDR
                     to audio. See EC-831.

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
    OPMSG_FRAMEWORK_GET_BUILD_ID_STRING = 0x00FF,
    OPMSG_FRAMEWORK_SET_BDADDR = 0x00ac,
    OPMSG_FRAMEWORK_GET_BDADDR = 0x00ad
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
    INPUT_RATE_VALID     - Tells the decoder whether the input is valid or not.
                           operator doesn't require the client to do any
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
    OPMSG_GET_CONFIG_TERMINAL_DETAILS

  DESCRIPTION
    Type of operator endpoint

 VALUES
    NONE             - Neither real nor rate match aware
    REAL             - Real operator endpoint end of chain
    RATE_MATCH_AWARE - Uses the rate manager interface although not a real
                       operator endpoint

*******************************************************************************/
typedef enum
{
    OPMSG_GET_CONFIG_TERMINAL_DETAILS_NONE = 0x0000,
    OPMSG_GET_CONFIG_TERMINAL_DETAILS_REAL = 0x0001,
    OPMSG_GET_CONFIG_TERMINAL_DETAILS_RATE_MATCH_AWARE = 0x0002
} OPMSG_GET_CONFIG_TERMINAL_DETAILS;
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
    OPMSG_KSP_ID

  DESCRIPTION

 VALUES
    SET_STREAM_TRANSFORMS - The operator message tells the operator which
                            transform buffers in the running graph it should
                            sniff. message must be sent before starting the
                            operator.

*******************************************************************************/
typedef enum
{
    OPMSG_KSP_ID_SET_STREAM_TRANSFORMS = 0x0001
} OPMSG_KSP_ID;
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
    GET_CONFIG           - Get mixer configurations

*******************************************************************************/
typedef enum
{
    OPMSG_MIXER_ID_SET_STREAM_GAINS = 0x0001,
    OPMSG_MIXER_ID_SET_STREAM_CHANNELS = 0x0002,
    OPMSG_MIXER_ID_SET_RAMP_NUM_SAMPLES = 0x0003,
    OPMSG_MIXER_ID_SET_PRIMARY_STREAM = 0x0004,
    OPMSG_MIXER_ID_SET_CHANNEL_GAINS = 0x0005,
    OPMSG_MIXER_ID_SET_METADATA_STREAM = 0x0006,
    OPMSG_MIXER_ID_GET_CONFIG = 0x0007
} OPMSG_MIXER_ID;
/*******************************************************************************

  NAME
    OPMSG_OP_CLIENT_REPLY_ID

  DESCRIPTION
    Unsolicited messages from operators may be redirected to an operator
    client. They will be transposed to the range 0x3000 - 0x3FFF, so that the
    framework can toggle the ID. The last values of the space are special
    values for other information forwarded by the framework to the operator
    client, such as the delegated operators and the replies to commands sent
    by the operator client (opmsgs or start/stop/reset commands).

 VALUES
    MASK                - This value is the mask to use to transform an
                          unsolicited message to a forwarded message
                          (OP_CLIENT_REPLY).
    VA_TRIGGER          - This message is sent 1) by a VAD operator to its
                          client when the VAD operator detects audio activity,
                          2) by a VTD operator to its client when the VTD
                          operator detects a trigger phrase, 3) by a VCI
                          operator to its client when the VCI operator detects a
                          voice command.
    VA_NEGATIVE_TRIGGER - This message is sent 1) by a VTD operator to its
                          client when the VTD operator detects NO trigger
                          phrase, 2) by a VCI operator to its client when the
                          VCI operator detects NO voice command.
    AOV_LP_NOTIFICATION - Notification from the AOV that we are entering
                          LowPower mode. Application client sends the
                          lp_activate to AOV (through WOS). The AOV processes
                          it, changes the clock and sends a notification to the
                          operator client in the form of an operator message
                          with this id.
    AOV_RESPONSE        - Response to any request sent by the operator client to
                          the AOV. Operator client sends a request to the AOV,
                          ex. a clock change. The AOV processes it and fills in
                          a response which is sent on an operator message with
                          this id.
    MESSAGE_RESPONSE    - Response to an operator message sent by the operator
                          client. Operator client sends a message to a delegated
                          operator, which processes it and fills in a response.
                          The Framework redirects this response to the operator
                          client in the form of an operator message with this
                          id.
    COMMAND_RESPONSE    - Response to a start/stop/reset request sent by the
                          operator client. Operator client sends a request to a
                          list of delegated operators. The framework processes
                          it and sends a response to the operator client in the
                          form of an operator message with this id.
    DELEGATED_OPERATORS - List of the operator ids delegated to the operator
                          client. Apps promotes an operator to be an operator
                          client, delegating a list of operators to its
                          management. The framework forwards this list of
                          operator ids to the operator client in the form of an
                          operator message with this id. The order of the
                          operator ids in the list may be agreed with the
                          relevant plug-in in Apps. The implementation of an
                          operator client may choose to ignore this message and
                          use another operator specific message instead.

*******************************************************************************/
typedef enum
{
    OPMSG_OP_CLIENT_REPLY_ID_MASK = 0x3000,
    OPMSG_OP_CLIENT_REPLY_ID_VA_TRIGGER = 0x3002,
    OPMSG_OP_CLIENT_REPLY_ID_VA_NEGATIVE_TRIGGER = 0x3003,
    OPMSG_OP_CLIENT_REPLY_ID_AOV_LP_NOTIFICATION = 0x3FFB,
    OPMSG_OP_CLIENT_REPLY_ID_AOV_RESPONSE = 0x3FFC,
    OPMSG_OP_CLIENT_REPLY_ID_MESSAGE_RESPONSE = 0x3FFD,
    OPMSG_OP_CLIENT_REPLY_ID_COMMAND_RESPONSE = 0x3FFE,
    OPMSG_OP_CLIENT_REPLY_ID_DELEGATED_OPERATORS = 0x3FFF
} OPMSG_OP_CLIENT_REPLY_ID;
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
    OPMSG_QVA_ID

  DESCRIPTION
    QVA configuration messages

 VALUES
    RESET_STATUS                   - Resets QVA status
    MODE_CHANGE                    - Sets the operating MODE for the QVA
    TRIGGER_PHRASE_LOAD            - Requests QVA to use the listed trigger
                                     phrase files
    TRIGGER_PHRASE_UNLOAD          - Requests QVA to unload the listed trigger
                                     phrase files
    STRIP_TRIGGER_PHRASE_FLAG      - Requests QVA to pass through audio data
                                     only after detecting the end of trigger
                                     phrase
    SET_MIN_MAX_TRIGGER_PHRASE_LEN - When detecting the start and the end of the
                                     trigger phrase timestamp the length can be
                                     limited.

*******************************************************************************/
typedef enum
{
    OPMSG_QVA_ID_RESET_STATUS = 0x0000,
    OPMSG_QVA_ID_MODE_CHANGE = 0x0001,
    OPMSG_QVA_ID_TRIGGER_PHRASE_LOAD = 0x0002,
    OPMSG_QVA_ID_TRIGGER_PHRASE_UNLOAD = 0x0003,
    OPMSG_QVA_ID_STRIP_TRIGGER_PHRASE_FLAG = 0x0004,
    OPMSG_QVA_ID_SET_MIN_MAX_TRIGGER_PHRASE_LEN = 0x0005
} OPMSG_QVA_ID;
/*******************************************************************************

  NAME
    OPMSG_QVA_MODE

  DESCRIPTION
    QVA mode type: mode can be one of the following:

 VALUES
    FULL_PROC     - QVA tries to detect key phrase
    PASS_THRU     - QVA simply passes data through
    PURGE_DATA    - Consumes all data at input with no processing
    FORCE_TRIGGER - For test purposes only, it forces QVA to behave as if speech
                    is detected

*******************************************************************************/
typedef enum
{
    OPMSG_QVA_MODE_FULL_PROC = 0x0001,
    OPMSG_QVA_MODE_PASS_THRU = 0x0002,
    OPMSG_QVA_MODE_PURGE_DATA = 0x0003,
    OPMSG_QVA_MODE_FORCE_TRIGGER = 0x00FF
} OPMSG_QVA_MODE;
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
    TONE_PARAM - Ringtone Generator tone definition message

*******************************************************************************/
typedef enum
{
    OPMSG_RINGTONE_ID_TONE_PARAM = 0x0001
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
    SET_WORKING_MODE                - Sets the working mode.
    SET_RTP_CODEC_TYPE              - Sets the rtp codec type.
    SET_RTP_CONTENT_PROTECTION      - Sets the rtp content protection (enable or
                                      disable).
    SET_AAC_DECODER                 - Sets the AAC codec for RTP decode. The AAC
                                      codec must be an AAC decode operator which
                                      will give sample count estimates for RTP
                                      decode.
    SET_MAX_PACKET_LENGTH           - Sets a maximum output packet length for
                                      unframed input data. This is only used for
                                      the aptX "TTP-only" mode.
    SET_AAC_UTILITY                 - Sets the AAC utility to use for
                                      calculation of number of samples in a
                                      packet.
    SET_SRC_LATENCY_MAPPING         - Sets the source ID to latency mapping used
                                      for aptX adaptive.
    SET_PACKING                     - To save some space the latency buffer can
                                      be packed to an internal buffer. NORE:
                                      Only works if DATAFORMAT_32 is not
                                      defined.
    SET_LATENCY_CHANGE_NOTIFICATION - Enables latency change nofitication for
                                      aptX adaptive

*******************************************************************************/
typedef enum
{
    OPMSG_RTP_DECODE_ID_SET_WORKING_MODE = 0x0001,
    OPMSG_RTP_DECODE_ID_SET_RTP_CODEC_TYPE = 0x0002,
    OPMSG_RTP_DECODE_ID_SET_RTP_CONTENT_PROTECTION = 0x0003,
    OPMSG_RTP_DECODE_ID_SET_AAC_DECODER = 0x0004,
    OPMSG_RTP_DECODE_ID_SET_MAX_PACKET_LENGTH = 0x0005,
    OPMSG_RTP_DECODE_ID_SET_AAC_UTILITY = 0x0006,
    OPMSG_RTP_DECODE_ID_SET_SRC_LATENCY_MAPPING = 0x0007,
    OPMSG_RTP_DECODE_ID_SET_PACKING = 0x0008,
    OPMSG_RTP_DECODE_ID_SET_LATENCY_CHANGE_NOTIFICATION = 0x0009
} OPMSG_RTP_DECODE_ID;
/*******************************************************************************

  NAME
    OPMSG_Reply_ID

  DESCRIPTION
    Unsolicited messages from operators.

 VALUES
    VA_TRIGGER          - This message is sent 1) by a VAD operator to its
                          client when the VAD operator detects audio activity,
                          2) by a VTD operator to its client when the VTD
                          operator detects a trigger phrase, 3) by a VCI
                          operator to its client when the VCI operator detects a
                          voice command.
    VA_NEGATIVE_TRIGGER - This message is sent 1) by a VTD operator to its
                          client when the VTD operator detects NO trigger
                          phrase, 2) by a VCI operator to its client when the
                          VCI operator detects NO voice command.
    LATENCY_CHANGED     - This  message is sent by the RTP DECODE operator
                          whenever there is an latency change due to an aptX
                          adaptive source change.
    FADEOUT_DONE        - For more information see section 4.2 in BlueCore
                          DSPManager Specification document

*******************************************************************************/
typedef enum
{
    OPMSG_REPLY_ID_VA_TRIGGER = 0x0002,
    OPMSG_REPLY_ID_VA_NEGATIVE_TRIGGER = 0x0003,
    OPMSG_REPLY_ID_LATENCY_CHANGED = 0x0005,
    OPMSG_REPLY_ID_FADEOUT_DONE = OPMSG_COMMON_ID_FADEOUT_ENABLE
} OPMSG_REPLY_ID;
/*******************************************************************************

  NAME
    OPMSG_SBC_ENC_ID

  DESCRIPTION
    SBC Encoder configuration messages

 VALUES
    SET_ENCODING_PARAMS - This message is used for setting the parameters that
                          SBC uses for encoding the audio as an SBC frame

*******************************************************************************/
typedef enum
{
    OPMSG_SBC_ENC_ID_SET_ENCODING_PARAMS = 0x0001
} OPMSG_SBC_ENC_ID;
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
    OPMSG_SPC_ID

  DESCRIPTION
    Switched Passthrough Consumer configuration messages

 VALUES
    TRANSITION         - Request the SPC to transition to another mode.
    SET_DATA_FORMAT    - Sets the expected input and output terminal data
                         formats
    SET_BUFFERING      - Sets the internal buffer size in words which is used
                         for buffering the incoming data and metadata. By
                         default is 0 meaning that there is no buffreing.
    SET_TRANSITION     - Transition form consume to passthrough when time stampo
                         is bigger than the given value. The timestamp is 32bit
                         value divided into msb and lsb.
    SELECT_PASSTHROUGH - Select which input to passthrough

*******************************************************************************/
typedef enum
{
    OPMSG_SPC_ID_TRANSITION = 0x0001,
    OPMSG_SPC_ID_SET_DATA_FORMAT = 0x0002,
    OPMSG_SPC_ID_SET_BUFFERING = 0x0003,
    OPMSG_SPC_ID_SET_TRANSITION = 0x0004,
    OPMSG_SPC_ID_SELECT_PASSTHROUGH = 0x0005
} OPMSG_SPC_ID;
/*******************************************************************************

  NAME
    OPMSG_SPC_MODE

  DESCRIPTION
    Enumerates the Switched Passthrough Consumer operating modes.

 VALUES
    PASSTHROUGH - Copy all data from the input to the output.
    CONSUMER    - Discard all data from the input.

*******************************************************************************/
typedef enum
{
    OPMSG_SPC_MODE_PASSTHROUGH = 0x0,
    OPMSG_SPC_MODE_CONSUMER = 0x1
} OPMSG_SPC_MODE;
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
    OPMSG_SPLITTER_BUFFER_LOCATION

  DESCRIPTION
    Buffer locations supported by the splitter operator.

 VALUES
    OPMSG_SPLITTER_INTERNALLY - All data is saved in the internal memory of the
                                DSP. This is the default setting when
                                OPMSG_SPLITTER_BUFFER_DATA is activated.
    OPMSG_SPLITTER_SRAM       - The bufferd data is saved to an external SRAM.

*******************************************************************************/
typedef enum
{
    OPMSG_SPLITTER_INTERNALLY = 0x0000,
    OPMSG_SPLITTER_SRAM = 0x0001
} OPMSG_SPLITTER_BUFFER_LOCATION;
/*******************************************************************************

  NAME
    OPMSG_SPLITTER_ID

  DESCRIPTION
    Splitter configuration messages

 VALUES
    SET_RUNNING_STREAMS              - Sets the output streams of the splitter
                                       that are considered running/active or
                                       inactive. Format is a bit field where bit
                                       n = stream n. Streams marked as 1 will be
                                       activated while streams marked as 0 will
                                       be deactivated.
    ACTIVATE_STREAMS                 - Activates the output streams of the
                                       splitter. All active streams will pass
                                       data to the output. Format is a bit field
                                       where bit n = stream n. Streams marked as
                                       1 will be activated while streams marked
                                       as 0 will be ignored (state remains the
                                       same).
    ACTIVATE_STREAMS_AFTER_TIMESTAMP - Activates the output streams of the
                                       splitter after the given time to play
                                       timestamp. This message is only available
                                       when the working mode is BUFFER_DATA and
                                       only streams on buffer/hold can be
                                       activated after timestamp. Format is a
                                       bit field where bit n = stream n. Streams
                                       marked as 1 will be activated after the
                                       timestamp while streams marked as 0 will
                                       be ignored (state remains the same).
    DEACTIVATE_STREAMS               - Deactivates the output streams of the
                                       splitter. Deactivated streams will
                                       discard data and they can be
                                       disconnected. Format is a bit field where
                                       bit n = stream n. Streams marked as 1
                                       will be deactivated while streams marked
                                       as 0 will be ignored (state remains the
                                       same).
    BUFFER_STREAMS                   - Sets the streams of the splitter which
                                       will be buffered/hold. Buffering stream
                                       will hold as much data as possible in the
                                       internal buffer, but the data is not
                                       available at the output until the stream
                                       is activated. Format is a bit field where
                                       bit n = stream n. Streams marked as 1
                                       will start to buffer data while streams
                                       marked as 0 will be ignored (state
                                       remains the same).
    SET_WORKING_MODE                 - Sets the working mode for the splitter
                                       operator. The following modes are
                                       supported: CLONE_BUFFER - The input
                                       buffers are cloned at the outputs. The
                                       data is buffered int the transform
                                       buffers. BUFFER_DATA - The inputs and
                                       outputs are different buffers and the
                                       data is buffered in a separate internal
                                       buffer.
    SET_BUFFERING_LOCATION           - Sets the buffering location for the
                                       splitter operator. The following
                                       locations are supported: INTERNALLY - By
                                       default the buffered data is saved
                                       locally in the DSP memory. SRAM - The
                                       data is saved to an external SRAM.
    SET_PACKING                      - Sets the packing mode for the splitter
                                       operator. This can only be set if the
                                       data format is audio:
                                       AUDIO_DATA_FORMAT_FIXP or
                                       AUDIO_DATA_FORMAT_FIXP_WITH_METADATA The
                                       following modes are supported: UNPACKED -
                                       The audio data is saved as it is. PACKED
                                       - Only the MSB 16 bit of the audio sample
                                       is saved.
    SET_REFRAMING                    - Sets the  metadata reframing mode and
                                       size for the splitter operator. The
                                       following modes are supported:
                                       OPMSG_SPLITTER_REFRAMING_DISABLED -
                                       Metadata tags are transported unchanged.
                                       OPMSG_SPLITTER_REFRAMING_ENABLED -
                                       Metadata tags are reframed to a minimum
                                       size.
    SET_DATA_FORMAT                  - Sets the expected input and output
                                       terminal data formats

*******************************************************************************/
typedef enum
{
    OPMSG_SPLITTER_ID_SET_RUNNING_STREAMS = 0x0001,
    OPMSG_SPLITTER_ID_ACTIVATE_STREAMS = 0x0002,
    OPMSG_SPLITTER_ID_ACTIVATE_STREAMS_AFTER_TIMESTAMP = 0x0003,
    OPMSG_SPLITTER_ID_DEACTIVATE_STREAMS = 0x0004,
    OPMSG_SPLITTER_ID_BUFFER_STREAMS = 0x0005,
    OPMSG_SPLITTER_ID_SET_WORKING_MODE = 0x0006,
    OPMSG_SPLITTER_ID_SET_BUFFERING_LOCATION = 0x0007,
    OPMSG_SPLITTER_ID_SET_PACKING = 0x0008,
    OPMSG_SPLITTER_ID_SET_REFRAMING = 0x0009,
    OPMSG_SPLITTER_ID_SET_DATA_FORMAT = 0x000A
} OPMSG_SPLITTER_ID;
/*******************************************************************************

  NAME
    OPMSG_SPLITTER_PACKING

  DESCRIPTION
    To save memory the audio formats can be backed. A packed audio will lose
    the LSB 16 bit.

 VALUES
    OPMSG_SPLITTER_UNPACKED - The audio data will be saved as it is. This is the
                              default setting when OPMSG_SPLITTER_BUFFER_DATA is
                              activated.
    OPMSG_SPLITTER_PACKED   - Only the MSB 16 bit of the audio sample is saved.
                              Therefore the LSB 16 bit is lost.

*******************************************************************************/
typedef enum
{
    OPMSG_SPLITTER_UNPACKED = 0x0000,
    OPMSG_SPLITTER_PACKED = 0x0001
} OPMSG_SPLITTER_PACKING;
/*******************************************************************************

  NAME
    OPMSG_SPLITTER_REFRAMING

  DESCRIPTION
    Reframing modes.

 VALUES
    OPMSG_SPLITTER_REFRAMING_DISABLED - Metadata tags are transported unchanged.
    OPMSG_SPLITTER_REFRAMING_ENABLED  - Metadata tags transported to the output
                                        are resized to a minimum size.

*******************************************************************************/
typedef enum
{
    OPMSG_SPLITTER_REFRAMING_DISABLED = 0x0000,
    OPMSG_SPLITTER_REFRAMING_ENABLED = 0x0001
} OPMSG_SPLITTER_REFRAMING;
/*******************************************************************************

  NAME
    OPMSG_SPLITTER_RUNNING_STREAMS

  DESCRIPTION
    Bitfield values used by the Set Running Streams splitter message

 VALUES
    OPMSG_SPLITTER_STREAM_0 - Bit representing stream 0 (even terminal ids)
    OPMSG_SPLITTER_STREAM_1 - Bit representing stream 1 (odd terminal ids)

*******************************************************************************/
typedef enum
{
    OPMSG_SPLITTER_STREAM_0 = 0x0001,
    OPMSG_SPLITTER_STREAM_1 = 0x0002
} OPMSG_SPLITTER_RUNNING_STREAMS;
/*******************************************************************************

  NAME
    OPMSG_SPLITTER_WORKING_MODES

  DESCRIPTION
    The working modes supported by the splitter operator.

 VALUES
    OPMSG_SPLITTER_CLONE_BUFFER - The input buffer of the splitter is cloned at
                                  the outputs. This is the default behaviour.
    OPMSG_SPLITTER_BUFFER_DATA  - The input data will be buffer the operator in
                                  a separate buffer. The buffer can be external
                                  or internal (please see
                                  OPMSG_SPLITTER_BUFFER_LOCATION). ALso note
                                  that audio streams can be packed (form more
                                  info see ).

*******************************************************************************/
typedef enum
{
    OPMSG_SPLITTER_CLONE_BUFFER = 0x0000,
    OPMSG_SPLITTER_BUFFER_DATA = 0x0001
} OPMSG_SPLITTER_WORKING_MODES;
/*******************************************************************************

  NAME
    OPMSG_SRC_SYNC_ID

  DESCRIPTION
    SRC_SYNC configuration messages

 VALUES
    SET_ROUTES        - Setup signal routes through operator.
    GET_ROUTES        - Gets signal routes through operator.
    SET_SINK_GROUPS   - Group sinks together for processing.
    SET_SOURCE_GROUPS - Group sources together for processing.
    SET_TRACE_ENABLE  -

*******************************************************************************/
typedef enum
{
    OPMSG_SRC_SYNC_ID_SET_ROUTES = 0x0001,
    OPMSG_SRC_SYNC_ID_GET_ROUTES = 0x0002,
    OPMSG_SRC_SYNC_ID_SET_SINK_GROUPS = 0x0003,
    OPMSG_SRC_SYNC_ID_SET_SOURCE_GROUPS = 0x0004,
    OPMSG_SRC_SYNC_ID_SET_TRACE_ENABLE = 0x0010
} OPMSG_SRC_SYNC_ID;
/*******************************************************************************

  NAME
    OPMSG_STALL_DROP_ID

  DESCRIPTION

 VALUES
    STALL_DROP_CONFIGURE         -
    STALL_DROP_SET_TIMING_OUTPUT -
    STALL_DROP_SET_DELAY_PARAMS  -

*******************************************************************************/
typedef enum
{
    OPMSG_STALL_DROP_ID_STALL_DROP_CONFIGURE = 0x0001,
    OPMSG_STALL_DROP_ID_STALL_DROP_SET_TIMING_OUTPUT = 0x0002,
    OPMSG_STALL_DROP_ID_STALL_DROP_SET_DELAY_PARAMS = 0x0003
} OPMSG_STALL_DROP_ID;
/*******************************************************************************

  NAME
    OPMSG_USB_AUDIO_ID

  DESCRIPTION
    Messages for configuring usb audio operators, This is for both TX and RX
    operators

 VALUES
    SET_CONNECTION_CONFIG - This message sends the usb connection config to the
                            operator.

*******************************************************************************/
typedef enum
{
    OPMSG_USB_AUDIO_ID_SET_CONNECTION_CONFIG = 0x0002
} OPMSG_USB_AUDIO_ID;
/*******************************************************************************

  NAME
    OPMSG_VAD_ID

  DESCRIPTION
    VAD configuration messages

 VALUES
    MODE_CHANGE - Sets the operating MODE for the VAD

*******************************************************************************/
typedef enum
{
    OPMSG_VAD_ID_MODE_CHANGE = 0x0001
} OPMSG_VAD_ID;
/*******************************************************************************

  NAME
    OPMSG_VAD_MODE

  DESCRIPTION
    VAD mode type: mode can be one of the following:

 VALUES
    FULL_PROC     - VAD tries to detect speech
    PASS_THRU     - VAD simply passes data through
    FORCE_TRIGGER - For test purposes only, it forces VAD to behave as if speech
                    is detected

*******************************************************************************/
typedef enum
{
    OPMSG_VAD_MODE_FULL_PROC = 0x0001,
    OPMSG_VAD_MODE_PASS_THRU = 0x0002,
    OPMSG_VAD_MODE_FORCE_TRIGGER = 0x00FF
} OPMSG_VAD_MODE;
/*******************************************************************************

  NAME
    OPMSG_VA_GM_ID

  DESCRIPTION
    VA_Graph_Manager configuration messages

 VALUES
    SET_GRAPH_LOAD      - Sets the MIPS load of the VA graph
    SET_SPLITTER_OFFSET - Sets the time offset to send to the splitter at VAD
                          trigger

*******************************************************************************/
typedef enum
{
    OPMSG_VA_GM_ID_SET_GRAPH_LOAD = 0x0001,
    OPMSG_VA_GM_ID_SET_SPLITTER_OFFSET = 0x0002
} OPMSG_VA_GM_ID;
/*******************************************************************************

  NAME
    OPMSG_VA_GM_LOAD

  DESCRIPTION
    VA Graph Manager MIPS load of the VA graph: mode can be one of the
    following:

 VALUES
    FULL      - Graph load requires full cpu power.
    MEDIUM    - Graph load requires normal cpu power.
    LOW       - Graph load requires low cpu power.
    DONT_CARE - Ignore graph load.

*******************************************************************************/
typedef enum
{
    OPMSG_VA_GM_LOAD_FULL = 0x0001,
    OPMSG_VA_GM_LOAD_MEDIUM = 0x0002,
    OPMSG_VA_GM_LOAD_LOW = 0x003,
    OPMSG_VA_GM_LOAD_DONT_CARE = 0x00FF
} OPMSG_VA_GM_LOAD;
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
    SET_TO_AIR_INFO   -
    FORWARD_ALL_KICKS -

*******************************************************************************/
typedef enum
{
    OPMSG_WBS_ENC_ID_SET_TO_AIR_INFO = 0x00FD,
    OPMSG_WBS_ENC_ID_FORWARD_ALL_KICKS = 0x00FF
} OPMSG_WBS_ENC_ID;
/*******************************************************************************

  NAME
    Opmsg_Source_Sync_Set_Sink_Groups_Flag

  DESCRIPTION
    Flags to add to the MSW of a sink group definition

 VALUES
    RATE_ADJUST - Enable rate adjustment for this sink group
    PURGE       - Consume all data from this sink group if none of the terminals
                  are routed
    METADATA    - Support metadata on this sink group

*******************************************************************************/
typedef enum
{
    OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_FLAG_RATE_ADJUST = 0x0800,
    OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_FLAG_PURGE = 0x1000
#define OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_FLAG_METADATA ((OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_FLAG) 0x8000)
} OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_FLAG;
/*******************************************************************************

  NAME
    Opmsg_Source_Sync_Set_Source_Groups_Flag

  DESCRIPTION
    Flags to add to the MSW of a source group definition

 VALUES
    PROVIDE_TTP - Provide TTP on this source group
    METADATA    - Support metadata on this source group

*******************************************************************************/
typedef enum
{
    OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_FLAG_PROVIDE_TTP = 0x4000
#define OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_FLAG_METADATA ((OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_FLAG) 0x8000)
} OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_FLAG;


/*******************************************************************************

  NAME
    OPMSG_CVC_RCV_ID

  DESCRIPTION

*******************************************************************************/
typedef uint16 OPMSG_CVC_RCV_ID;


#define OPMSG_PRIM_ANY_SIZE 1

/*******************************************************************************

  NAME
    OPMSG_QVA_MODE_CHANGE

  DESCRIPTION
    Sets the operating MODE for the QVA

  MEMBERS
    message_id   - message id
    working_mode - The mode is of the type OPMSG_QVA_MODE

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_QVA_MODE_CHANGE;

/* The following macros take OPMSG_QVA_MODE_CHANGE *opmsg_qva_mode_change_ptr */
#define OPMSG_QVA_MODE_CHANGE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_QVA_MODE_CHANGE_MESSAGE_ID_GET(opmsg_qva_mode_change_ptr) ((OPMSG_QVA_ID)(opmsg_qva_mode_change_ptr)->_data[0])
#define OPMSG_QVA_MODE_CHANGE_MESSAGE_ID_SET(opmsg_qva_mode_change_ptr, message_id) ((opmsg_qva_mode_change_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_QVA_MODE_CHANGE_WORKING_MODE_WORD_OFFSET (1)
#define OPMSG_QVA_MODE_CHANGE_WORKING_MODE_GET(opmsg_qva_mode_change_ptr) ((OPMSG_QVA_MODE)(opmsg_qva_mode_change_ptr)->_data[1])
#define OPMSG_QVA_MODE_CHANGE_WORKING_MODE_SET(opmsg_qva_mode_change_ptr, working_mode) ((opmsg_qva_mode_change_ptr)->_data[1] = (uint16)(working_mode))
#define OPMSG_QVA_MODE_CHANGE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_QVA_MODE_CHANGE_CREATE(message_id, working_mode) \
    (uint16)(message_id), \
    (uint16)(working_mode)
#define OPMSG_QVA_MODE_CHANGE_PACK(opmsg_qva_mode_change_ptr, message_id, working_mode) \
    do { \
        (opmsg_qva_mode_change_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_qva_mode_change_ptr)->_data[1] = (uint16)((uint16)(working_mode)); \
    } while (0)

#define OPMSG_QVA_MODE_CHANGE_MARSHALL(addr, opmsg_qva_mode_change_ptr) memcpy((void *)(addr), (void *)(opmsg_qva_mode_change_ptr), 2)
#define OPMSG_QVA_MODE_CHANGE_UNMARSHALL(addr, opmsg_qva_mode_change_ptr) memcpy((void *)(opmsg_qva_mode_change_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    OPMSG_VAD_MODE_CHANGE

  DESCRIPTION
    Sets the operating MODE for the VAD

  MEMBERS
    message_id   - message id
    working_mode - The mode is of the type OPMSG_VAD_MODE

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_VAD_MODE_CHANGE;

/* The following macros take OPMSG_VAD_MODE_CHANGE *opmsg_vad_mode_change_ptr */
#define OPMSG_VAD_MODE_CHANGE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_VAD_MODE_CHANGE_MESSAGE_ID_GET(opmsg_vad_mode_change_ptr) ((OPMSG_VAD_ID)(opmsg_vad_mode_change_ptr)->_data[0])
#define OPMSG_VAD_MODE_CHANGE_MESSAGE_ID_SET(opmsg_vad_mode_change_ptr, message_id) ((opmsg_vad_mode_change_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_VAD_MODE_CHANGE_WORKING_MODE_WORD_OFFSET (1)
#define OPMSG_VAD_MODE_CHANGE_WORKING_MODE_GET(opmsg_vad_mode_change_ptr) ((OPMSG_VAD_MODE)(opmsg_vad_mode_change_ptr)->_data[1])
#define OPMSG_VAD_MODE_CHANGE_WORKING_MODE_SET(opmsg_vad_mode_change_ptr, working_mode) ((opmsg_vad_mode_change_ptr)->_data[1] = (uint16)(working_mode))
#define OPMSG_VAD_MODE_CHANGE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_VAD_MODE_CHANGE_CREATE(message_id, working_mode) \
    (uint16)(message_id), \
    (uint16)(working_mode)
#define OPMSG_VAD_MODE_CHANGE_PACK(opmsg_vad_mode_change_ptr, message_id, working_mode) \
    do { \
        (opmsg_vad_mode_change_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_vad_mode_change_ptr)->_data[1] = (uint16)((uint16)(working_mode)); \
    } while (0)

#define OPMSG_VAD_MODE_CHANGE_MARSHALL(addr, opmsg_vad_mode_change_ptr) memcpy((void *)(addr), (void *)(opmsg_vad_mode_change_ptr), 2)
#define OPMSG_VAD_MODE_CHANGE_UNMARSHALL(addr, opmsg_vad_mode_change_ptr) memcpy((void *)(opmsg_vad_mode_change_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    OPMSG_VA_GM_SET_GRAPH_LOAD

  DESCRIPTION
    Sets the operating MODE for the VAD

  MEMBERS
    message_id - message id
    load       - The mode is of the type OPMSG_VAD_MODE

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_VA_GM_SET_GRAPH_LOAD;

/* The following macros take OPMSG_VA_GM_SET_GRAPH_LOAD *opmsg_va_gm_set_graph_load_ptr */
#define OPMSG_VA_GM_SET_GRAPH_LOAD_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_VA_GM_SET_GRAPH_LOAD_MESSAGE_ID_GET(opmsg_va_gm_set_graph_load_ptr) ((OPMSG_VA_GM_ID)(opmsg_va_gm_set_graph_load_ptr)->_data[0])
#define OPMSG_VA_GM_SET_GRAPH_LOAD_MESSAGE_ID_SET(opmsg_va_gm_set_graph_load_ptr, message_id) ((opmsg_va_gm_set_graph_load_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_VA_GM_SET_GRAPH_LOAD_LOAD_WORD_OFFSET (1)
#define OPMSG_VA_GM_SET_GRAPH_LOAD_LOAD_GET(opmsg_va_gm_set_graph_load_ptr) ((OPMSG_VA_GM_LOAD)(opmsg_va_gm_set_graph_load_ptr)->_data[1])
#define OPMSG_VA_GM_SET_GRAPH_LOAD_LOAD_SET(opmsg_va_gm_set_graph_load_ptr, load) ((opmsg_va_gm_set_graph_load_ptr)->_data[1] = (uint16)(load))
#define OPMSG_VA_GM_SET_GRAPH_LOAD_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_VA_GM_SET_GRAPH_LOAD_CREATE(message_id, load) \
    (uint16)(message_id), \
    (uint16)(load)
#define OPMSG_VA_GM_SET_GRAPH_LOAD_PACK(opmsg_va_gm_set_graph_load_ptr, message_id, load) \
    do { \
        (opmsg_va_gm_set_graph_load_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_va_gm_set_graph_load_ptr)->_data[1] = (uint16)((uint16)(load)); \
    } while (0)

#define OPMSG_VA_GM_SET_GRAPH_LOAD_MARSHALL(addr, opmsg_va_gm_set_graph_load_ptr) memcpy((void *)(addr), (void *)(opmsg_va_gm_set_graph_load_ptr), 2)
#define OPMSG_VA_GM_SET_GRAPH_LOAD_UNMARSHALL(addr, opmsg_va_gm_set_graph_load_ptr) memcpy((void *)(opmsg_va_gm_set_graph_load_ptr), (void *)(addr), 2)


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

#define OPMSG_A2DP_DEC_CONTENT_PROTECTION_ENABLE_MARSHALL(addr, opmsg_a2dp_dec_content_protection_enable_ptr) memcpy((void *)(addr), (void *)(opmsg_a2dp_dec_content_protection_enable_ptr), 1)
#define OPMSG_A2DP_DEC_CONTENT_PROTECTION_ENABLE_UNMARSHALL(addr, opmsg_a2dp_dec_content_protection_enable_ptr) memcpy((void *)(opmsg_a2dp_dec_content_protection_enable_ptr), (void *)(addr), 1)


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

#define OPMSG_COMMON_DISABLE_FADEOUT_MARSHALL(addr, opmsg_common_disable_fadeout_ptr) memcpy((void *)(addr), (void *)(opmsg_common_disable_fadeout_ptr), 1)
#define OPMSG_COMMON_DISABLE_FADEOUT_UNMARSHALL(addr, opmsg_common_disable_fadeout_ptr) memcpy((void *)(opmsg_common_disable_fadeout_ptr), (void *)(addr), 1)


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

#define OPMSG_COMMON_ENABLE_FADEOUT_MARSHALL(addr, opmsg_common_enable_fadeout_ptr) memcpy((void *)(addr), (void *)(opmsg_common_enable_fadeout_ptr), 1)
#define OPMSG_COMMON_ENABLE_FADEOUT_UNMARSHALL(addr, opmsg_common_enable_fadeout_ptr) memcpy((void *)(opmsg_common_enable_fadeout_ptr), (void *)(addr), 1)


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

#define OPMSG_COMMON_GET_CAPABILITY_VERSION_MARSHALL(addr, opmsg_common_get_capability_version_ptr) memcpy((void *)(addr), (void *)(opmsg_common_get_capability_version_ptr), 1)
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_UNMARSHALL(addr, opmsg_common_get_capability_version_ptr) memcpy((void *)(opmsg_common_get_capability_version_ptr), (void *)(addr), 1)


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

#define OPMSG_COMMON_MSG_SET_DATA_STREAM_BASED_MARSHALL(addr, opmsg_common_msg_set_data_stream_based_ptr) memcpy((void *)(addr), (void *)(opmsg_common_msg_set_data_stream_based_ptr), 1)
#define OPMSG_COMMON_MSG_SET_DATA_STREAM_BASED_UNMARSHALL(addr, opmsg_common_msg_set_data_stream_based_ptr) memcpy((void *)(opmsg_common_msg_set_data_stream_based_ptr), (void *)(addr), 1)


/*******************************************************************************

  NAME
    Opmsg_Mixer_Get_Config

  DESCRIPTION
    Mixer operator message for GET_CONFIG

  MEMBERS
    message_id - message id

*******************************************************************************/
typedef struct
{
    uint16 _data[1];
} OPMSG_MIXER_GET_CONFIG;

/* The following macros take OPMSG_MIXER_GET_CONFIG *opmsg_mixer_get_config_ptr */
#define OPMSG_MIXER_GET_CONFIG_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_MIXER_GET_CONFIG_MESSAGE_ID_GET(opmsg_mixer_get_config_ptr) ((OPMSG_MIXER_ID)(opmsg_mixer_get_config_ptr)->_data[0])
#define OPMSG_MIXER_GET_CONFIG_MESSAGE_ID_SET(opmsg_mixer_get_config_ptr, message_id) ((opmsg_mixer_get_config_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_MIXER_GET_CONFIG_WORD_SIZE (1)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_MIXER_GET_CONFIG_CREATE(message_id) \
    (uint16)(message_id)
#define OPMSG_MIXER_GET_CONFIG_PACK(opmsg_mixer_get_config_ptr, message_id) \
    do { \
        (opmsg_mixer_get_config_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
    } while (0)

#define OPMSG_MIXER_GET_CONFIG_MARSHALL(addr, opmsg_mixer_get_config_ptr) memcpy((void *)(addr), (void *)(opmsg_mixer_get_config_ptr), 1)
#define OPMSG_MIXER_GET_CONFIG_UNMARSHALL(addr, opmsg_mixer_get_config_ptr) memcpy((void *)(opmsg_mixer_get_config_ptr), (void *)(addr), 1)


/*******************************************************************************

  NAME
    Opmsg_SPC_Change_Mode

  DESCRIPTION
    Request the SPC to transition to another mode.

  MEMBERS
    message_id - message id
    new_mode   - Operating mode to transition to.

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_SPC_CHANGE_MODE;

/* The following macros take OPMSG_SPC_CHANGE_MODE *opmsg_spc_change_mode_ptr */
#define OPMSG_SPC_CHANGE_MODE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPC_CHANGE_MODE_MESSAGE_ID_GET(opmsg_spc_change_mode_ptr) ((OPMSG_SPC_ID)(opmsg_spc_change_mode_ptr)->_data[0])
#define OPMSG_SPC_CHANGE_MODE_MESSAGE_ID_SET(opmsg_spc_change_mode_ptr, message_id) ((opmsg_spc_change_mode_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPC_CHANGE_MODE_NEW_MODE_WORD_OFFSET (1)
#define OPMSG_SPC_CHANGE_MODE_NEW_MODE_GET(opmsg_spc_change_mode_ptr) ((OPMSG_SPC_MODE)(opmsg_spc_change_mode_ptr)->_data[1])
#define OPMSG_SPC_CHANGE_MODE_NEW_MODE_SET(opmsg_spc_change_mode_ptr, new_mode) ((opmsg_spc_change_mode_ptr)->_data[1] = (uint16)(new_mode))
#define OPMSG_SPC_CHANGE_MODE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPC_CHANGE_MODE_CREATE(message_id, new_mode) \
    (uint16)(message_id), \
    (uint16)(new_mode)
#define OPMSG_SPC_CHANGE_MODE_PACK(opmsg_spc_change_mode_ptr, message_id, new_mode) \
    do { \
        (opmsg_spc_change_mode_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_spc_change_mode_ptr)->_data[1] = (uint16)((uint16)(new_mode)); \
    } while (0)

#define OPMSG_SPC_CHANGE_MODE_MARSHALL(addr, opmsg_spc_change_mode_ptr) memcpy((void *)(addr), (void *)(opmsg_spc_change_mode_ptr), 2)
#define OPMSG_SPC_CHANGE_MODE_UNMARSHALL(addr, opmsg_spc_change_mode_ptr) memcpy((void *)(opmsg_spc_change_mode_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    OPMSG_ANC_TUNING_FRONTEND_CONFIG

  DESCRIPTION
    Sets the frontend configuration for the ANC tuning capability

  MEMBERS
    message_id - message id
    is_stereo  - dac and fb mon feeds are stereo (TRUE) or mono (FALSE)
    is_two_mic - number of ANC mics per instance is two mic (TRUE) or one (MIC)

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_ANC_TUNING_FRONTEND_CONFIG;

/* The following macros take OPMSG_ANC_TUNING_FRONTEND_CONFIG *opmsg_anc_tuning_frontend_config_ptr */
#define OPMSG_ANC_TUNING_FRONTEND_CONFIG_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_ANC_TUNING_FRONTEND_CONFIG_MESSAGE_ID_GET(opmsg_anc_tuning_frontend_config_ptr) ((OPMSG_ANC_TUNING_ID)(opmsg_anc_tuning_frontend_config_ptr)->_data[0])
#define OPMSG_ANC_TUNING_FRONTEND_CONFIG_MESSAGE_ID_SET(opmsg_anc_tuning_frontend_config_ptr, message_id) ((opmsg_anc_tuning_frontend_config_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_ANC_TUNING_FRONTEND_CONFIG_IS_STEREO_WORD_OFFSET (1)
#define OPMSG_ANC_TUNING_FRONTEND_CONFIG_IS_STEREO_GET(opmsg_anc_tuning_frontend_config_ptr) ((opmsg_anc_tuning_frontend_config_ptr)->_data[1])
#define OPMSG_ANC_TUNING_FRONTEND_CONFIG_IS_STEREO_SET(opmsg_anc_tuning_frontend_config_ptr, is_stereo) ((opmsg_anc_tuning_frontend_config_ptr)->_data[1] = (uint16)(is_stereo))
#define OPMSG_ANC_TUNING_FRONTEND_CONFIG_IS_TWO_MIC_WORD_OFFSET (2)
#define OPMSG_ANC_TUNING_FRONTEND_CONFIG_IS_TWO_MIC_GET(opmsg_anc_tuning_frontend_config_ptr) ((opmsg_anc_tuning_frontend_config_ptr)->_data[2])
#define OPMSG_ANC_TUNING_FRONTEND_CONFIG_IS_TWO_MIC_SET(opmsg_anc_tuning_frontend_config_ptr, is_two_mic) ((opmsg_anc_tuning_frontend_config_ptr)->_data[2] = (uint16)(is_two_mic))
#define OPMSG_ANC_TUNING_FRONTEND_CONFIG_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_ANC_TUNING_FRONTEND_CONFIG_CREATE(message_id, is_stereo, is_two_mic) \
    (uint16)(message_id), \
    (uint16)(is_stereo), \
    (uint16)(is_two_mic)
#define OPMSG_ANC_TUNING_FRONTEND_CONFIG_PACK(opmsg_anc_tuning_frontend_config_ptr, message_id, is_stereo, is_two_mic) \
    do { \
        (opmsg_anc_tuning_frontend_config_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_anc_tuning_frontend_config_ptr)->_data[1] = (uint16)((uint16)(is_stereo)); \
        (opmsg_anc_tuning_frontend_config_ptr)->_data[2] = (uint16)((uint16)(is_two_mic)); \
    } while (0)

#define OPMSG_ANC_TUNING_FRONTEND_CONFIG_MARSHALL(addr, opmsg_anc_tuning_frontend_config_ptr) memcpy((void *)(addr), (void *)(opmsg_anc_tuning_frontend_config_ptr), 3)
#define OPMSG_ANC_TUNING_FRONTEND_CONFIG_UNMARSHALL(addr, opmsg_anc_tuning_frontend_config_ptr) memcpy((void *)(opmsg_anc_tuning_frontend_config_ptr), (void *)(addr), 3)


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

#define OPMSG_MSG_MARSHALL(addr, opmsg_msg_ptr) memcpy((void *)(addr), (void *)(opmsg_msg_ptr), 3)
#define OPMSG_MSG_UNMARSHALL(addr, opmsg_msg_ptr) memcpy((void *)(opmsg_msg_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN

  DESCRIPTION
    Currently only the end of the trigger phrase can be detected accurately.
    Therefore to limit the start of the trigger phrase timestamp trigger
    phase length limits are sent to the operator.

  MEMBERS
    message_id - message id
    min_len    - Minimum trigger phrase length in milliseconds.
    max_len    - Maximum trigger phrase length in milliseconds.

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN;

/* The following macros take OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN *opmsg_qva_set_min_max_trigger_phrase_len_ptr */
#define OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN_MESSAGE_ID_GET(opmsg_qva_set_min_max_trigger_phrase_len_ptr) ((OPMSG_QVA_ID)(opmsg_qva_set_min_max_trigger_phrase_len_ptr)->_data[0])
#define OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN_MESSAGE_ID_SET(opmsg_qva_set_min_max_trigger_phrase_len_ptr, message_id) ((opmsg_qva_set_min_max_trigger_phrase_len_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN_MIN_LEN_WORD_OFFSET (1)
#define OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN_MIN_LEN_GET(opmsg_qva_set_min_max_trigger_phrase_len_ptr) ((opmsg_qva_set_min_max_trigger_phrase_len_ptr)->_data[1])
#define OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN_MIN_LEN_SET(opmsg_qva_set_min_max_trigger_phrase_len_ptr, min_len) ((opmsg_qva_set_min_max_trigger_phrase_len_ptr)->_data[1] = (uint16)(min_len))
#define OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN_MAX_LEN_WORD_OFFSET (2)
#define OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN_MAX_LEN_GET(opmsg_qva_set_min_max_trigger_phrase_len_ptr) ((opmsg_qva_set_min_max_trigger_phrase_len_ptr)->_data[2])
#define OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN_MAX_LEN_SET(opmsg_qva_set_min_max_trigger_phrase_len_ptr, max_len) ((opmsg_qva_set_min_max_trigger_phrase_len_ptr)->_data[2] = (uint16)(max_len))
#define OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN_CREATE(message_id, min_len, max_len) \
    (uint16)(message_id), \
    (uint16)(min_len), \
    (uint16)(max_len)
#define OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN_PACK(opmsg_qva_set_min_max_trigger_phrase_len_ptr, message_id, min_len, max_len) \
    do { \
        (opmsg_qva_set_min_max_trigger_phrase_len_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_qva_set_min_max_trigger_phrase_len_ptr)->_data[1] = (uint16)((uint16)(min_len)); \
        (opmsg_qva_set_min_max_trigger_phrase_len_ptr)->_data[2] = (uint16)((uint16)(max_len)); \
    } while (0)

#define OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN_MARSHALL(addr, opmsg_qva_set_min_max_trigger_phrase_len_ptr) memcpy((void *)(addr), (void *)(opmsg_qva_set_min_max_trigger_phrase_len_ptr), 3)
#define OPMSG_QVA_SET_MIN_MAX_TRIGGER_PHRASE_LEN_UNMARSHALL(addr, opmsg_qva_set_min_max_trigger_phrase_len_ptr) memcpy((void *)(opmsg_qva_set_min_max_trigger_phrase_len_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    OPMSG_QVA_STRIP_TRIGGER_PHRASE_FLAG

  DESCRIPTION
    Requests QVA to pass through audio data only after detecting the end of
    trigger phrase. This effectively strips the trigger phrase from any audio
    command that includes it.

  MEMBERS
    message_id - message id
    flag_value - Can be 0 or 1. If set to 1 then we strip the trigger phrase
                 from the incoming audio command. Setting to zero disables this
                 functionality.

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_QVA_STRIP_TRIGGER_PHRASE_FLAG;

/* The following macros take OPMSG_QVA_STRIP_TRIGGER_PHRASE_FLAG *opmsg_qva_strip_trigger_phrase_flag_ptr */
#define OPMSG_QVA_STRIP_TRIGGER_PHRASE_FLAG_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_QVA_STRIP_TRIGGER_PHRASE_FLAG_MESSAGE_ID_GET(opmsg_qva_strip_trigger_phrase_flag_ptr) ((OPMSG_QVA_ID)(opmsg_qva_strip_trigger_phrase_flag_ptr)->_data[0])
#define OPMSG_QVA_STRIP_TRIGGER_PHRASE_FLAG_MESSAGE_ID_SET(opmsg_qva_strip_trigger_phrase_flag_ptr, message_id) ((opmsg_qva_strip_trigger_phrase_flag_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_QVA_STRIP_TRIGGER_PHRASE_FLAG_FLAG_VALUE_WORD_OFFSET (1)
#define OPMSG_QVA_STRIP_TRIGGER_PHRASE_FLAG_FLAG_VALUE_GET(opmsg_qva_strip_trigger_phrase_flag_ptr) ((opmsg_qva_strip_trigger_phrase_flag_ptr)->_data[1])
#define OPMSG_QVA_STRIP_TRIGGER_PHRASE_FLAG_FLAG_VALUE_SET(opmsg_qva_strip_trigger_phrase_flag_ptr, flag_value) ((opmsg_qva_strip_trigger_phrase_flag_ptr)->_data[1] = (uint16)(flag_value))
#define OPMSG_QVA_STRIP_TRIGGER_PHRASE_FLAG_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_QVA_STRIP_TRIGGER_PHRASE_FLAG_CREATE(message_id, flag_value) \
    (uint16)(message_id), \
    (uint16)(flag_value)
#define OPMSG_QVA_STRIP_TRIGGER_PHRASE_FLAG_PACK(opmsg_qva_strip_trigger_phrase_flag_ptr, message_id, flag_value) \
    do { \
        (opmsg_qva_strip_trigger_phrase_flag_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_qva_strip_trigger_phrase_flag_ptr)->_data[1] = (uint16)((uint16)(flag_value)); \
    } while (0)

#define OPMSG_QVA_STRIP_TRIGGER_PHRASE_FLAG_MARSHALL(addr, opmsg_qva_strip_trigger_phrase_flag_ptr) memcpy((void *)(addr), (void *)(opmsg_qva_strip_trigger_phrase_flag_ptr), 2)
#define OPMSG_QVA_STRIP_TRIGGER_PHRASE_FLAG_UNMARSHALL(addr, opmsg_qva_strip_trigger_phrase_flag_ptr) memcpy((void *)(opmsg_qva_strip_trigger_phrase_flag_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    OPMSG_QVA_TRIGGER_PHRASE_LOAD

  DESCRIPTION
    Requests QVA to use the listed trigger phrase files

  MEMBERS
    message_id   - message id
    num_files    - Number of files to be loaded. Currently QVA supports loading
                   only 1 file at a time
    file_id_list - List of file  IDs to be loaded. Currently QVA supports
                   loading only 1 file at a time

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_QVA_TRIGGER_PHRASE_LOAD;

/* The following macros take OPMSG_QVA_TRIGGER_PHRASE_LOAD *opmsg_qva_trigger_phrase_load_ptr */
#define OPMSG_QVA_TRIGGER_PHRASE_LOAD_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_QVA_TRIGGER_PHRASE_LOAD_MESSAGE_ID_GET(opmsg_qva_trigger_phrase_load_ptr) ((OPMSG_QVA_ID)(opmsg_qva_trigger_phrase_load_ptr)->_data[0])
#define OPMSG_QVA_TRIGGER_PHRASE_LOAD_MESSAGE_ID_SET(opmsg_qva_trigger_phrase_load_ptr, message_id) ((opmsg_qva_trigger_phrase_load_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_QVA_TRIGGER_PHRASE_LOAD_NUM_FILES_WORD_OFFSET (1)
#define OPMSG_QVA_TRIGGER_PHRASE_LOAD_NUM_FILES_GET(opmsg_qva_trigger_phrase_load_ptr) ((opmsg_qva_trigger_phrase_load_ptr)->_data[1])
#define OPMSG_QVA_TRIGGER_PHRASE_LOAD_NUM_FILES_SET(opmsg_qva_trigger_phrase_load_ptr, num_files) ((opmsg_qva_trigger_phrase_load_ptr)->_data[1] = (uint16)(num_files))
#define OPMSG_QVA_TRIGGER_PHRASE_LOAD_FILE_ID_LIST_WORD_OFFSET (2)
#define OPMSG_QVA_TRIGGER_PHRASE_LOAD_FILE_ID_LIST_GET(opmsg_qva_trigger_phrase_load_ptr) ((opmsg_qva_trigger_phrase_load_ptr)->_data[2])
#define OPMSG_QVA_TRIGGER_PHRASE_LOAD_FILE_ID_LIST_SET(opmsg_qva_trigger_phrase_load_ptr, file_id_list) ((opmsg_qva_trigger_phrase_load_ptr)->_data[2] = (uint16)(file_id_list))
#define OPMSG_QVA_TRIGGER_PHRASE_LOAD_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_QVA_TRIGGER_PHRASE_LOAD_CREATE(message_id, num_files, file_id_list) \
    (uint16)(message_id), \
    (uint16)(num_files), \
    (uint16)(file_id_list)
#define OPMSG_QVA_TRIGGER_PHRASE_LOAD_PACK(opmsg_qva_trigger_phrase_load_ptr, message_id, num_files, file_id_list) \
    do { \
        (opmsg_qva_trigger_phrase_load_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_qva_trigger_phrase_load_ptr)->_data[1] = (uint16)((uint16)(num_files)); \
        (opmsg_qva_trigger_phrase_load_ptr)->_data[2] = (uint16)((uint16)(file_id_list)); \
    } while (0)

#define OPMSG_QVA_TRIGGER_PHRASE_LOAD_MARSHALL(addr, opmsg_qva_trigger_phrase_load_ptr) memcpy((void *)(addr), (void *)(opmsg_qva_trigger_phrase_load_ptr), 3)
#define OPMSG_QVA_TRIGGER_PHRASE_LOAD_UNMARSHALL(addr, opmsg_qva_trigger_phrase_load_ptr) memcpy((void *)(opmsg_qva_trigger_phrase_load_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    OPMSG_QVA_TRIGGER_PHRASE_UNLOAD

  DESCRIPTION
    Requests QVA to unload the listed trigger phrase files

  MEMBERS
    message_id   - message id
    num_files    - Number of files to be unloaded. Currently QVA supports
                   un-loading only 1 file at a time
    file_id_list - List of file  IDs to be unloaded. Currently QVA supports
                   un-loading only 1 file at a time

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_QVA_TRIGGER_PHRASE_UNLOAD;

/* The following macros take OPMSG_QVA_TRIGGER_PHRASE_UNLOAD *opmsg_qva_trigger_phrase_unload_ptr */
#define OPMSG_QVA_TRIGGER_PHRASE_UNLOAD_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_QVA_TRIGGER_PHRASE_UNLOAD_MESSAGE_ID_GET(opmsg_qva_trigger_phrase_unload_ptr) ((OPMSG_QVA_ID)(opmsg_qva_trigger_phrase_unload_ptr)->_data[0])
#define OPMSG_QVA_TRIGGER_PHRASE_UNLOAD_MESSAGE_ID_SET(opmsg_qva_trigger_phrase_unload_ptr, message_id) ((opmsg_qva_trigger_phrase_unload_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_QVA_TRIGGER_PHRASE_UNLOAD_NUM_FILES_WORD_OFFSET (1)
#define OPMSG_QVA_TRIGGER_PHRASE_UNLOAD_NUM_FILES_GET(opmsg_qva_trigger_phrase_unload_ptr) ((opmsg_qva_trigger_phrase_unload_ptr)->_data[1])
#define OPMSG_QVA_TRIGGER_PHRASE_UNLOAD_NUM_FILES_SET(opmsg_qva_trigger_phrase_unload_ptr, num_files) ((opmsg_qva_trigger_phrase_unload_ptr)->_data[1] = (uint16)(num_files))
#define OPMSG_QVA_TRIGGER_PHRASE_UNLOAD_FILE_ID_LIST_WORD_OFFSET (2)
#define OPMSG_QVA_TRIGGER_PHRASE_UNLOAD_FILE_ID_LIST_GET(opmsg_qva_trigger_phrase_unload_ptr) ((opmsg_qva_trigger_phrase_unload_ptr)->_data[2])
#define OPMSG_QVA_TRIGGER_PHRASE_UNLOAD_FILE_ID_LIST_SET(opmsg_qva_trigger_phrase_unload_ptr, file_id_list) ((opmsg_qva_trigger_phrase_unload_ptr)->_data[2] = (uint16)(file_id_list))
#define OPMSG_QVA_TRIGGER_PHRASE_UNLOAD_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_QVA_TRIGGER_PHRASE_UNLOAD_CREATE(message_id, num_files, file_id_list) \
    (uint16)(message_id), \
    (uint16)(num_files), \
    (uint16)(file_id_list)
#define OPMSG_QVA_TRIGGER_PHRASE_UNLOAD_PACK(opmsg_qva_trigger_phrase_unload_ptr, message_id, num_files, file_id_list) \
    do { \
        (opmsg_qva_trigger_phrase_unload_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_qva_trigger_phrase_unload_ptr)->_data[1] = (uint16)((uint16)(num_files)); \
        (opmsg_qva_trigger_phrase_unload_ptr)->_data[2] = (uint16)((uint16)(file_id_list)); \
    } while (0)

#define OPMSG_QVA_TRIGGER_PHRASE_UNLOAD_MARSHALL(addr, opmsg_qva_trigger_phrase_unload_ptr) memcpy((void *)(addr), (void *)(opmsg_qva_trigger_phrase_unload_ptr), 3)
#define OPMSG_QVA_TRIGGER_PHRASE_UNLOAD_UNMARSHALL(addr, opmsg_qva_trigger_phrase_unload_ptr) memcpy((void *)(opmsg_qva_trigger_phrase_unload_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    OPMSG_VA_GM_SET_SPLITTER_OFFSET

  DESCRIPTION
    Sets the time offset to send to the splitter at VAD trigger

  MEMBERS
    message_id - message id
    OFFSET     - The time offset to send to the splitter, in milliseconds

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_VA_GM_SET_SPLITTER_OFFSET;

/* The following macros take OPMSG_VA_GM_SET_SPLITTER_OFFSET *opmsg_va_gm_set_splitter_offset_ptr */
#define OPMSG_VA_GM_SET_SPLITTER_OFFSET_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_VA_GM_SET_SPLITTER_OFFSET_MESSAGE_ID_GET(opmsg_va_gm_set_splitter_offset_ptr) ((OPMSG_VA_GM_ID)(opmsg_va_gm_set_splitter_offset_ptr)->_data[0])
#define OPMSG_VA_GM_SET_SPLITTER_OFFSET_MESSAGE_ID_SET(opmsg_va_gm_set_splitter_offset_ptr, message_id) ((opmsg_va_gm_set_splitter_offset_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_VA_GM_SET_SPLITTER_OFFSET_OFFSET_WORD_OFFSET (1)
#define OPMSG_VA_GM_SET_SPLITTER_OFFSET_OFFSET_GET(opmsg_va_gm_set_splitter_offset_ptr) ((opmsg_va_gm_set_splitter_offset_ptr)->_data[1])
#define OPMSG_VA_GM_SET_SPLITTER_OFFSET_OFFSET_SET(opmsg_va_gm_set_splitter_offset_ptr, offset) ((opmsg_va_gm_set_splitter_offset_ptr)->_data[1] = (uint16)(offset))
#define OPMSG_VA_GM_SET_SPLITTER_OFFSET_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_VA_GM_SET_SPLITTER_OFFSET_CREATE(message_id, OFFSET) \
    (uint16)(message_id), \
    (uint16)(OFFSET)
#define OPMSG_VA_GM_SET_SPLITTER_OFFSET_PACK(opmsg_va_gm_set_splitter_offset_ptr, message_id, OFFSET) \
    do { \
        (opmsg_va_gm_set_splitter_offset_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_va_gm_set_splitter_offset_ptr)->_data[1] = (uint16)((uint16)(OFFSET)); \
    } while (0)

#define OPMSG_VA_GM_SET_SPLITTER_OFFSET_MARSHALL(addr, opmsg_va_gm_set_splitter_offset_ptr) memcpy((void *)(addr), (void *)(opmsg_va_gm_set_splitter_offset_ptr), 2)
#define OPMSG_VA_GM_SET_SPLITTER_OFFSET_UNMARSHALL(addr, opmsg_va_gm_set_splitter_offset_ptr) memcpy((void *)(opmsg_va_gm_set_splitter_offset_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Aac_Set_Frame_Type

  DESCRIPTION
    Sets the frame type  for AAC Decoder

  MEMBERS
    message_id - message id
    frame_type - frame types mp4 (0), adts (1), latm (2)

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_AAC_SET_FRAME_TYPE;

/* The following macros take OPMSG_AAC_SET_FRAME_TYPE *opmsg_aac_set_frame_type_ptr */
#define OPMSG_AAC_SET_FRAME_TYPE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_AAC_SET_FRAME_TYPE_MESSAGE_ID_GET(opmsg_aac_set_frame_type_ptr) ((OPMSG_AAC_ID)(opmsg_aac_set_frame_type_ptr)->_data[0])
#define OPMSG_AAC_SET_FRAME_TYPE_MESSAGE_ID_SET(opmsg_aac_set_frame_type_ptr, message_id) ((opmsg_aac_set_frame_type_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_AAC_SET_FRAME_TYPE_FRAME_TYPE_WORD_OFFSET (1)
#define OPMSG_AAC_SET_FRAME_TYPE_FRAME_TYPE_GET(opmsg_aac_set_frame_type_ptr) ((opmsg_aac_set_frame_type_ptr)->_data[1])
#define OPMSG_AAC_SET_FRAME_TYPE_FRAME_TYPE_SET(opmsg_aac_set_frame_type_ptr, frame_type) ((opmsg_aac_set_frame_type_ptr)->_data[1] = (uint16)(frame_type))
#define OPMSG_AAC_SET_FRAME_TYPE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_AAC_SET_FRAME_TYPE_CREATE(message_id, frame_type) \
    (uint16)(message_id), \
    (uint16)(frame_type)
#define OPMSG_AAC_SET_FRAME_TYPE_PACK(opmsg_aac_set_frame_type_ptr, message_id, frame_type) \
    do { \
        (opmsg_aac_set_frame_type_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_aac_set_frame_type_ptr)->_data[1] = (uint16)((uint16)(frame_type)); \
    } while (0)

#define OPMSG_AAC_SET_FRAME_TYPE_MARSHALL(addr, opmsg_aac_set_frame_type_ptr) memcpy((void *)(addr), (void *)(opmsg_aac_set_frame_type_ptr), 2)
#define OPMSG_AAC_SET_FRAME_TYPE_UNMARSHALL(addr, opmsg_aac_set_frame_type_ptr) memcpy((void *)(opmsg_aac_set_frame_type_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Aec_Enable_Spkr_Input_Gate

  DESCRIPTION
    AEC operator message for ENABLE_SPKR_INPUT_GATE message.

  MEMBERS
    message_id              - message id
    enable                  - enables the speaker gate feature
    initial_delay           - initial delay in opening the gate (in ms) 0 will
                              be interpreted as default 100ms and, a minimum of
                              50ms and maximum of 300ms will apply.
    post_gate_drift_control - enables to control latency drift after openning
                              the gate.

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_AEC_ENABLE_SPKR_INPUT_GATE;

/* The following macros take OPMSG_AEC_ENABLE_SPKR_INPUT_GATE *opmsg_aec_enable_spkr_input_gate_ptr */
#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_MESSAGE_ID_GET(opmsg_aec_enable_spkr_input_gate_ptr) ((OPMSG_AEC_REFERENCE_ID)(opmsg_aec_enable_spkr_input_gate_ptr)->_data[0])
#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_MESSAGE_ID_SET(opmsg_aec_enable_spkr_input_gate_ptr, message_id) ((opmsg_aec_enable_spkr_input_gate_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_ENABLE_WORD_OFFSET (1)
#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_ENABLE_GET(opmsg_aec_enable_spkr_input_gate_ptr) ((opmsg_aec_enable_spkr_input_gate_ptr)->_data[1])
#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_ENABLE_SET(opmsg_aec_enable_spkr_input_gate_ptr, enable) ((opmsg_aec_enable_spkr_input_gate_ptr)->_data[1] = (uint16)(enable))
#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_INITIAL_DELAY_WORD_OFFSET (2)
#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_INITIAL_DELAY_GET(opmsg_aec_enable_spkr_input_gate_ptr) ((opmsg_aec_enable_spkr_input_gate_ptr)->_data[2])
#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_INITIAL_DELAY_SET(opmsg_aec_enable_spkr_input_gate_ptr, initial_delay) ((opmsg_aec_enable_spkr_input_gate_ptr)->_data[2] = (uint16)(initial_delay))
#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_POST_GATE_DRIFT_CONTROL_WORD_OFFSET (3)
#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_POST_GATE_DRIFT_CONTROL_GET(opmsg_aec_enable_spkr_input_gate_ptr) ((opmsg_aec_enable_spkr_input_gate_ptr)->_data[3])
#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_POST_GATE_DRIFT_CONTROL_SET(opmsg_aec_enable_spkr_input_gate_ptr, post_gate_drift_control) ((opmsg_aec_enable_spkr_input_gate_ptr)->_data[3] = (uint16)(post_gate_drift_control))
#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_CREATE(message_id, enable, initial_delay, post_gate_drift_control) \
    (uint16)(message_id), \
    (uint16)(enable), \
    (uint16)(initial_delay), \
    (uint16)(post_gate_drift_control)
#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_PACK(opmsg_aec_enable_spkr_input_gate_ptr, message_id, enable, initial_delay, post_gate_drift_control) \
    do { \
        (opmsg_aec_enable_spkr_input_gate_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_aec_enable_spkr_input_gate_ptr)->_data[1] = (uint16)((uint16)(enable)); \
        (opmsg_aec_enable_spkr_input_gate_ptr)->_data[2] = (uint16)((uint16)(initial_delay)); \
        (opmsg_aec_enable_spkr_input_gate_ptr)->_data[3] = (uint16)((uint16)(post_gate_drift_control)); \
    } while (0)

#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_MARSHALL(addr, opmsg_aec_enable_spkr_input_gate_ptr) memcpy((void *)(addr), (void *)(opmsg_aec_enable_spkr_input_gate_ptr), 4)
#define OPMSG_AEC_ENABLE_SPKR_INPUT_GATE_UNMARSHALL(addr, opmsg_aec_enable_spkr_input_gate_ptr) memcpy((void *)(opmsg_aec_enable_spkr_input_gate_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    Opmsg_Aec_Mute_Mic_Output

  DESCRIPTION
    AEC operator message for MUTE_MIC_OUTPUT.

  MEMBERS
    message_id - message id
    enable     - any non zero value will mute the output

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_AEC_MUTE_MIC_OUTPUT;

/* The following macros take OPMSG_AEC_MUTE_MIC_OUTPUT *opmsg_aec_mute_mic_output_ptr */
#define OPMSG_AEC_MUTE_MIC_OUTPUT_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_AEC_MUTE_MIC_OUTPUT_MESSAGE_ID_GET(opmsg_aec_mute_mic_output_ptr) ((OPMSG_AEC_REFERENCE_ID)(opmsg_aec_mute_mic_output_ptr)->_data[0])
#define OPMSG_AEC_MUTE_MIC_OUTPUT_MESSAGE_ID_SET(opmsg_aec_mute_mic_output_ptr, message_id) ((opmsg_aec_mute_mic_output_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_AEC_MUTE_MIC_OUTPUT_ENABLE_WORD_OFFSET (1)
#define OPMSG_AEC_MUTE_MIC_OUTPUT_ENABLE_GET(opmsg_aec_mute_mic_output_ptr) ((opmsg_aec_mute_mic_output_ptr)->_data[1])
#define OPMSG_AEC_MUTE_MIC_OUTPUT_ENABLE_SET(opmsg_aec_mute_mic_output_ptr, enable) ((opmsg_aec_mute_mic_output_ptr)->_data[1] = (uint16)(enable))
#define OPMSG_AEC_MUTE_MIC_OUTPUT_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_AEC_MUTE_MIC_OUTPUT_CREATE(message_id, enable) \
    (uint16)(message_id), \
    (uint16)(enable)
#define OPMSG_AEC_MUTE_MIC_OUTPUT_PACK(opmsg_aec_mute_mic_output_ptr, message_id, enable) \
    do { \
        (opmsg_aec_mute_mic_output_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_aec_mute_mic_output_ptr)->_data[1] = (uint16)((uint16)(enable)); \
    } while (0)

#define OPMSG_AEC_MUTE_MIC_OUTPUT_MARSHALL(addr, opmsg_aec_mute_mic_output_ptr) memcpy((void *)(addr), (void *)(opmsg_aec_mute_mic_output_ptr), 2)
#define OPMSG_AEC_MUTE_MIC_OUTPUT_UNMARSHALL(addr, opmsg_aec_mute_mic_output_ptr) memcpy((void *)(opmsg_aec_mute_mic_output_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Aec_Same_Input_Output_Clock_Source

  DESCRIPTION
    AEC operator message for SAME_INPUT_OUTPUT_CLOCK_SOURCE.

  MEMBERS
    message_id   - message id
    same_clk_src - any non zero value means microphone output has the same clock
                   source as speaker input.

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_AEC_SAME_INPUT_OUTPUT_CLOCK_SOURCE;

/* The following macros take OPMSG_AEC_SAME_INPUT_OUTPUT_CLOCK_SOURCE *opmsg_aec_same_input_output_clock_source_ptr */
#define OPMSG_AEC_SAME_INPUT_OUTPUT_CLOCK_SOURCE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_AEC_SAME_INPUT_OUTPUT_CLOCK_SOURCE_MESSAGE_ID_GET(opmsg_aec_same_input_output_clock_source_ptr) ((OPMSG_AEC_REFERENCE_ID)(opmsg_aec_same_input_output_clock_source_ptr)->_data[0])
#define OPMSG_AEC_SAME_INPUT_OUTPUT_CLOCK_SOURCE_MESSAGE_ID_SET(opmsg_aec_same_input_output_clock_source_ptr, message_id) ((opmsg_aec_same_input_output_clock_source_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_AEC_SAME_INPUT_OUTPUT_CLOCK_SOURCE_SAME_CLK_SRC_WORD_OFFSET (1)
#define OPMSG_AEC_SAME_INPUT_OUTPUT_CLOCK_SOURCE_SAME_CLK_SRC_GET(opmsg_aec_same_input_output_clock_source_ptr) ((opmsg_aec_same_input_output_clock_source_ptr)->_data[1])
#define OPMSG_AEC_SAME_INPUT_OUTPUT_CLOCK_SOURCE_SAME_CLK_SRC_SET(opmsg_aec_same_input_output_clock_source_ptr, same_clk_src) ((opmsg_aec_same_input_output_clock_source_ptr)->_data[1] = (uint16)(same_clk_src))
#define OPMSG_AEC_SAME_INPUT_OUTPUT_CLOCK_SOURCE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_AEC_SAME_INPUT_OUTPUT_CLOCK_SOURCE_CREATE(message_id, same_clk_src) \
    (uint16)(message_id), \
    (uint16)(same_clk_src)
#define OPMSG_AEC_SAME_INPUT_OUTPUT_CLOCK_SOURCE_PACK(opmsg_aec_same_input_output_clock_source_ptr, message_id, same_clk_src) \
    do { \
        (opmsg_aec_same_input_output_clock_source_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_aec_same_input_output_clock_source_ptr)->_data[1] = (uint16)((uint16)(same_clk_src)); \
    } while (0)

#define OPMSG_AEC_SAME_INPUT_OUTPUT_CLOCK_SOURCE_MARSHALL(addr, opmsg_aec_same_input_output_clock_source_ptr) memcpy((void *)(addr), (void *)(opmsg_aec_same_input_output_clock_source_ptr), 2)
#define OPMSG_AEC_SAME_INPUT_OUTPUT_CLOCK_SOURCE_UNMARSHALL(addr, opmsg_aec_same_input_output_clock_source_ptr) memcpy((void *)(opmsg_aec_same_input_output_clock_source_ptr), (void *)(addr), 2)


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

#define OPMSG_AEC_SET_SAMPLE_RATES_MARSHALL(addr, opmsg_aec_set_sample_rates_ptr) memcpy((void *)(addr), (void *)(opmsg_aec_set_sample_rates_ptr), 3)
#define OPMSG_AEC_SET_SAMPLE_RATES_UNMARSHALL(addr, opmsg_aec_set_sample_rates_ptr) memcpy((void *)(opmsg_aec_set_sample_rates_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    Opmsg_Aec_Set_Task_Period

  DESCRIPTION
    AEC operator message for SET_TASK_PERIOD.

  MEMBERS
    message_id   - message id
    task_period  - AEC Reference task period in microseconds, task_period must
                   be a factor of 1000000(1 second), the operator will accept
                   one the following values: 1000, 1250, 1600, 2000, 2500, 3125,
                   4000 and 5000 Note: AEC REFERENCE task period should not
                   exceed system kick period.
    decim_factor - This will enable the operator to run side tone mixing more
                   often hence achieving better latency performance. task_period
                   must be devisable by this factor. This value will only be
                   effective for Hydra architectures. Don't set a decim_factore
                   if sidetone isn't required because it will add some overhead
                   to operator's cpu usage.

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_AEC_SET_TASK_PERIOD;

/* The following macros take OPMSG_AEC_SET_TASK_PERIOD *opmsg_aec_set_task_period_ptr */
#define OPMSG_AEC_SET_TASK_PERIOD_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_AEC_SET_TASK_PERIOD_MESSAGE_ID_GET(opmsg_aec_set_task_period_ptr) ((OPMSG_AEC_REFERENCE_ID)(opmsg_aec_set_task_period_ptr)->_data[0])
#define OPMSG_AEC_SET_TASK_PERIOD_MESSAGE_ID_SET(opmsg_aec_set_task_period_ptr, message_id) ((opmsg_aec_set_task_period_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_AEC_SET_TASK_PERIOD_TASK_PERIOD_WORD_OFFSET (1)
#define OPMSG_AEC_SET_TASK_PERIOD_TASK_PERIOD_GET(opmsg_aec_set_task_period_ptr) ((opmsg_aec_set_task_period_ptr)->_data[1])
#define OPMSG_AEC_SET_TASK_PERIOD_TASK_PERIOD_SET(opmsg_aec_set_task_period_ptr, task_period) ((opmsg_aec_set_task_period_ptr)->_data[1] = (uint16)(task_period))
#define OPMSG_AEC_SET_TASK_PERIOD_DECIM_FACTOR_WORD_OFFSET (2)
#define OPMSG_AEC_SET_TASK_PERIOD_DECIM_FACTOR_GET(opmsg_aec_set_task_period_ptr) ((opmsg_aec_set_task_period_ptr)->_data[2])
#define OPMSG_AEC_SET_TASK_PERIOD_DECIM_FACTOR_SET(opmsg_aec_set_task_period_ptr, decim_factor) ((opmsg_aec_set_task_period_ptr)->_data[2] = (uint16)(decim_factor))
#define OPMSG_AEC_SET_TASK_PERIOD_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_AEC_SET_TASK_PERIOD_CREATE(message_id, task_period, decim_factor) \
    (uint16)(message_id), \
    (uint16)(task_period), \
    (uint16)(decim_factor)
#define OPMSG_AEC_SET_TASK_PERIOD_PACK(opmsg_aec_set_task_period_ptr, message_id, task_period, decim_factor) \
    do { \
        (opmsg_aec_set_task_period_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_aec_set_task_period_ptr)->_data[1] = (uint16)((uint16)(task_period)); \
        (opmsg_aec_set_task_period_ptr)->_data[2] = (uint16)((uint16)(decim_factor)); \
    } while (0)

#define OPMSG_AEC_SET_TASK_PERIOD_MARSHALL(addr, opmsg_aec_set_task_period_ptr) memcpy((void *)(addr), (void *)(opmsg_aec_set_task_period_ptr), 3)
#define OPMSG_AEC_SET_TASK_PERIOD_UNMARSHALL(addr, opmsg_aec_set_task_period_ptr) memcpy((void *)(opmsg_aec_set_task_period_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    Opmsg_Aptx_Adaptive_Enc_Set_Bit_Rate

  DESCRIPTION
    APTX ADAPTIVE Encoder Operator message for changing the bit rate for
    encoding.

  MEMBERS
    message_id      - message id
    bit_rate_change - bit rate

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_APTX_ADAPTIVE_ENC_SET_BIT_RATE;

/* The following macros take OPMSG_APTX_ADAPTIVE_ENC_SET_BIT_RATE *opmsg_aptx_adaptive_enc_set_bit_rate_ptr */
#define OPMSG_APTX_ADAPTIVE_ENC_SET_BIT_RATE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_APTX_ADAPTIVE_ENC_SET_BIT_RATE_MESSAGE_ID_GET(opmsg_aptx_adaptive_enc_set_bit_rate_ptr) ((OPMSG_APTX_ADAPTIVE_ENC_ID)(opmsg_aptx_adaptive_enc_set_bit_rate_ptr)->_data[0])
#define OPMSG_APTX_ADAPTIVE_ENC_SET_BIT_RATE_MESSAGE_ID_SET(opmsg_aptx_adaptive_enc_set_bit_rate_ptr, message_id) ((opmsg_aptx_adaptive_enc_set_bit_rate_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_APTX_ADAPTIVE_ENC_SET_BIT_RATE_BIT_RATE_CHANGE_WORD_OFFSET (1)
#define OPMSG_APTX_ADAPTIVE_ENC_SET_BIT_RATE_BIT_RATE_CHANGE_GET(opmsg_aptx_adaptive_enc_set_bit_rate_ptr) ((opmsg_aptx_adaptive_enc_set_bit_rate_ptr)->_data[1])
#define OPMSG_APTX_ADAPTIVE_ENC_SET_BIT_RATE_BIT_RATE_CHANGE_SET(opmsg_aptx_adaptive_enc_set_bit_rate_ptr, bit_rate_change) ((opmsg_aptx_adaptive_enc_set_bit_rate_ptr)->_data[1] = (uint16)(bit_rate_change))
#define OPMSG_APTX_ADAPTIVE_ENC_SET_BIT_RATE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_APTX_ADAPTIVE_ENC_SET_BIT_RATE_CREATE(message_id, bit_rate_change) \
    (uint16)(message_id), \
    (uint16)(bit_rate_change)
#define OPMSG_APTX_ADAPTIVE_ENC_SET_BIT_RATE_PACK(opmsg_aptx_adaptive_enc_set_bit_rate_ptr, message_id, bit_rate_change) \
    do { \
        (opmsg_aptx_adaptive_enc_set_bit_rate_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_aptx_adaptive_enc_set_bit_rate_ptr)->_data[1] = (uint16)((uint16)(bit_rate_change)); \
    } while (0)

#define OPMSG_APTX_ADAPTIVE_ENC_SET_BIT_RATE_MARSHALL(addr, opmsg_aptx_adaptive_enc_set_bit_rate_ptr) memcpy((void *)(addr), (void *)(opmsg_aptx_adaptive_enc_set_bit_rate_ptr), 2)
#define OPMSG_APTX_ADAPTIVE_ENC_SET_BIT_RATE_UNMARSHALL(addr, opmsg_aptx_adaptive_enc_set_bit_rate_ptr) memcpy((void *)(opmsg_aptx_adaptive_enc_set_bit_rate_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Aptx_Adaptive_Enc_Set_Encoding_Params

  DESCRIPTION
    APTX ADAPTIVE Encoder Operator message for configuring the encoding
    parameters used for encoding.

  MEMBERS
    message_id        - message id
    primary_profile   - primary profile
    secondary_profile - secondary profile
    channels          - Channel to use
    bit_rate          - Bit rate to use
    sample_rate       - Sample rate to use

*******************************************************************************/
typedef struct
{
    uint16 _data[6];
} OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS;

/* The following macros take OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS *opmsg_aptx_adaptive_enc_set_encoding_params_ptr */
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_MESSAGE_ID_GET(opmsg_aptx_adaptive_enc_set_encoding_params_ptr) ((OPMSG_APTX_ADAPTIVE_ENC_ID)(opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[0])
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_MESSAGE_ID_SET(opmsg_aptx_adaptive_enc_set_encoding_params_ptr, message_id) ((opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_PRIMARY_PROFILE_WORD_OFFSET (1)
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_PRIMARY_PROFILE_GET(opmsg_aptx_adaptive_enc_set_encoding_params_ptr) ((opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[1])
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_PRIMARY_PROFILE_SET(opmsg_aptx_adaptive_enc_set_encoding_params_ptr, primary_profile) ((opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[1] = (uint16)(primary_profile))
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_SECONDARY_PROFILE_WORD_OFFSET (2)
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_SECONDARY_PROFILE_GET(opmsg_aptx_adaptive_enc_set_encoding_params_ptr) ((opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[2])
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_SECONDARY_PROFILE_SET(opmsg_aptx_adaptive_enc_set_encoding_params_ptr, secondary_profile) ((opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[2] = (uint16)(secondary_profile))
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_CHANNELS_WORD_OFFSET (3)
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_CHANNELS_GET(opmsg_aptx_adaptive_enc_set_encoding_params_ptr) ((opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[3])
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_CHANNELS_SET(opmsg_aptx_adaptive_enc_set_encoding_params_ptr, channels) ((opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[3] = (uint16)(channels))
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_BIT_RATE_WORD_OFFSET (4)
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_BIT_RATE_GET(opmsg_aptx_adaptive_enc_set_encoding_params_ptr) ((opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[4])
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_BIT_RATE_SET(opmsg_aptx_adaptive_enc_set_encoding_params_ptr, bit_rate) ((opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[4] = (uint16)(bit_rate))
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_SAMPLE_RATE_WORD_OFFSET (5)
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_SAMPLE_RATE_GET(opmsg_aptx_adaptive_enc_set_encoding_params_ptr) ((opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[5])
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_SAMPLE_RATE_SET(opmsg_aptx_adaptive_enc_set_encoding_params_ptr, sample_rate) ((opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[5] = (uint16)(sample_rate))
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_WORD_SIZE (6)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_CREATE(message_id, primary_profile, secondary_profile, channels, bit_rate, sample_rate) \
    (uint16)(message_id), \
    (uint16)(primary_profile), \
    (uint16)(secondary_profile), \
    (uint16)(channels), \
    (uint16)(bit_rate), \
    (uint16)(sample_rate)
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_PACK(opmsg_aptx_adaptive_enc_set_encoding_params_ptr, message_id, primary_profile, secondary_profile, channels, bit_rate, sample_rate) \
    do { \
        (opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[1] = (uint16)((uint16)(primary_profile)); \
        (opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[2] = (uint16)((uint16)(secondary_profile)); \
        (opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[3] = (uint16)((uint16)(channels)); \
        (opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[4] = (uint16)((uint16)(bit_rate)); \
        (opmsg_aptx_adaptive_enc_set_encoding_params_ptr)->_data[5] = (uint16)((uint16)(sample_rate)); \
    } while (0)

#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_MARSHALL(addr, opmsg_aptx_adaptive_enc_set_encoding_params_ptr) memcpy((void *)(addr), (void *)(opmsg_aptx_adaptive_enc_set_encoding_params_ptr), 6)
#define OPMSG_APTX_ADAPTIVE_ENC_SET_ENCODING_PARAMS_UNMARSHALL(addr, opmsg_aptx_adaptive_enc_set_encoding_params_ptr) memcpy((void *)(opmsg_aptx_adaptive_enc_set_encoding_params_ptr), (void *)(addr), 6)


/*******************************************************************************

  NAME
    Opmsg_Async_Wbs_Dec_Set_Bitpool_Value

  DESCRIPTION
    SBC Encoder Operator message for configuring the encoding parameters used
    for encoding.

  MEMBERS
    message_id - message id
    bitpool    - Bitpool size to use

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_ASYNC_WBS_DEC_SET_BITPOOL_VALUE;

/* The following macros take OPMSG_ASYNC_WBS_DEC_SET_BITPOOL_VALUE *opmsg_async_wbs_dec_set_bitpool_value_ptr */
#define OPMSG_ASYNC_WBS_DEC_SET_BITPOOL_VALUE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_ASYNC_WBS_DEC_SET_BITPOOL_VALUE_MESSAGE_ID_GET(opmsg_async_wbs_dec_set_bitpool_value_ptr) ((OPMSG_ASYNC_WBS_DEC_ID)(opmsg_async_wbs_dec_set_bitpool_value_ptr)->_data[0])
#define OPMSG_ASYNC_WBS_DEC_SET_BITPOOL_VALUE_MESSAGE_ID_SET(opmsg_async_wbs_dec_set_bitpool_value_ptr, message_id) ((opmsg_async_wbs_dec_set_bitpool_value_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_ASYNC_WBS_DEC_SET_BITPOOL_VALUE_BITPOOL_WORD_OFFSET (1)
#define OPMSG_ASYNC_WBS_DEC_SET_BITPOOL_VALUE_BITPOOL_GET(opmsg_async_wbs_dec_set_bitpool_value_ptr) ((opmsg_async_wbs_dec_set_bitpool_value_ptr)->_data[1])
#define OPMSG_ASYNC_WBS_DEC_SET_BITPOOL_VALUE_BITPOOL_SET(opmsg_async_wbs_dec_set_bitpool_value_ptr, bitpool) ((opmsg_async_wbs_dec_set_bitpool_value_ptr)->_data[1] = (uint16)(bitpool))
#define OPMSG_ASYNC_WBS_DEC_SET_BITPOOL_VALUE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_ASYNC_WBS_DEC_SET_BITPOOL_VALUE_CREATE(message_id, bitpool) \
    (uint16)(message_id), \
    (uint16)(bitpool)
#define OPMSG_ASYNC_WBS_DEC_SET_BITPOOL_VALUE_PACK(opmsg_async_wbs_dec_set_bitpool_value_ptr, message_id, bitpool) \
    do { \
        (opmsg_async_wbs_dec_set_bitpool_value_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_async_wbs_dec_set_bitpool_value_ptr)->_data[1] = (uint16)((uint16)(bitpool)); \
    } while (0)

#define OPMSG_ASYNC_WBS_DEC_SET_BITPOOL_VALUE_MARSHALL(addr, opmsg_async_wbs_dec_set_bitpool_value_ptr) memcpy((void *)(addr), (void *)(opmsg_async_wbs_dec_set_bitpool_value_ptr), 2)
#define OPMSG_ASYNC_WBS_DEC_SET_BITPOOL_VALUE_UNMARSHALL(addr, opmsg_async_wbs_dec_set_bitpool_value_ptr) memcpy((void *)(opmsg_async_wbs_dec_set_bitpool_value_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Async_Wbs_Enc_Set_Bitpool_Value

  DESCRIPTION
    SBC Encoder Operator message for configuring the encoding parameters used
    for encoding.

  MEMBERS
    message_id - message id
    bitpool    - Bitpool size to use

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_ASYNC_WBS_ENC_SET_BITPOOL_VALUE;

/* The following macros take OPMSG_ASYNC_WBS_ENC_SET_BITPOOL_VALUE *opmsg_async_wbs_enc_set_bitpool_value_ptr */
#define OPMSG_ASYNC_WBS_ENC_SET_BITPOOL_VALUE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_ASYNC_WBS_ENC_SET_BITPOOL_VALUE_MESSAGE_ID_GET(opmsg_async_wbs_enc_set_bitpool_value_ptr) ((OPMSG_ASYNC_WBS_ENC_ID)(opmsg_async_wbs_enc_set_bitpool_value_ptr)->_data[0])
#define OPMSG_ASYNC_WBS_ENC_SET_BITPOOL_VALUE_MESSAGE_ID_SET(opmsg_async_wbs_enc_set_bitpool_value_ptr, message_id) ((opmsg_async_wbs_enc_set_bitpool_value_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_ASYNC_WBS_ENC_SET_BITPOOL_VALUE_BITPOOL_WORD_OFFSET (1)
#define OPMSG_ASYNC_WBS_ENC_SET_BITPOOL_VALUE_BITPOOL_GET(opmsg_async_wbs_enc_set_bitpool_value_ptr) ((opmsg_async_wbs_enc_set_bitpool_value_ptr)->_data[1])
#define OPMSG_ASYNC_WBS_ENC_SET_BITPOOL_VALUE_BITPOOL_SET(opmsg_async_wbs_enc_set_bitpool_value_ptr, bitpool) ((opmsg_async_wbs_enc_set_bitpool_value_ptr)->_data[1] = (uint16)(bitpool))
#define OPMSG_ASYNC_WBS_ENC_SET_BITPOOL_VALUE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_ASYNC_WBS_ENC_SET_BITPOOL_VALUE_CREATE(message_id, bitpool) \
    (uint16)(message_id), \
    (uint16)(bitpool)
#define OPMSG_ASYNC_WBS_ENC_SET_BITPOOL_VALUE_PACK(opmsg_async_wbs_enc_set_bitpool_value_ptr, message_id, bitpool) \
    do { \
        (opmsg_async_wbs_enc_set_bitpool_value_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_async_wbs_enc_set_bitpool_value_ptr)->_data[1] = (uint16)((uint16)(bitpool)); \
    } while (0)

#define OPMSG_ASYNC_WBS_ENC_SET_BITPOOL_VALUE_MARSHALL(addr, opmsg_async_wbs_enc_set_bitpool_value_ptr) memcpy((void *)(addr), (void *)(opmsg_async_wbs_enc_set_bitpool_value_ptr), 2)
#define OPMSG_ASYNC_WBS_ENC_SET_BITPOOL_VALUE_UNMARSHALL(addr, opmsg_async_wbs_enc_set_bitpool_value_ptr) memcpy((void *)(opmsg_async_wbs_enc_set_bitpool_value_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Async_Wbs_Enc_Set_Frames_Per_Packet

  DESCRIPTION
    SBC Encoder Operator message for configuring the encoding parameters used
    for encoding.

  MEMBERS
    message_id - message id
    max_frames - Max frames in a BT packet

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_ASYNC_WBS_ENC_SET_FRAMES_PER_PACKET;

/* The following macros take OPMSG_ASYNC_WBS_ENC_SET_FRAMES_PER_PACKET *opmsg_async_wbs_enc_set_frames_per_packet_ptr */
#define OPMSG_ASYNC_WBS_ENC_SET_FRAMES_PER_PACKET_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_ASYNC_WBS_ENC_SET_FRAMES_PER_PACKET_MESSAGE_ID_GET(opmsg_async_wbs_enc_set_frames_per_packet_ptr) ((OPMSG_ASYNC_WBS_ENC_ID)(opmsg_async_wbs_enc_set_frames_per_packet_ptr)->_data[0])
#define OPMSG_ASYNC_WBS_ENC_SET_FRAMES_PER_PACKET_MESSAGE_ID_SET(opmsg_async_wbs_enc_set_frames_per_packet_ptr, message_id) ((opmsg_async_wbs_enc_set_frames_per_packet_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_ASYNC_WBS_ENC_SET_FRAMES_PER_PACKET_MAX_FRAMES_WORD_OFFSET (1)
#define OPMSG_ASYNC_WBS_ENC_SET_FRAMES_PER_PACKET_MAX_FRAMES_GET(opmsg_async_wbs_enc_set_frames_per_packet_ptr) ((opmsg_async_wbs_enc_set_frames_per_packet_ptr)->_data[1])
#define OPMSG_ASYNC_WBS_ENC_SET_FRAMES_PER_PACKET_MAX_FRAMES_SET(opmsg_async_wbs_enc_set_frames_per_packet_ptr, max_frames) ((opmsg_async_wbs_enc_set_frames_per_packet_ptr)->_data[1] = (uint16)(max_frames))
#define OPMSG_ASYNC_WBS_ENC_SET_FRAMES_PER_PACKET_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_ASYNC_WBS_ENC_SET_FRAMES_PER_PACKET_CREATE(message_id, max_frames) \
    (uint16)(message_id), \
    (uint16)(max_frames)
#define OPMSG_ASYNC_WBS_ENC_SET_FRAMES_PER_PACKET_PACK(opmsg_async_wbs_enc_set_frames_per_packet_ptr, message_id, max_frames) \
    do { \
        (opmsg_async_wbs_enc_set_frames_per_packet_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_async_wbs_enc_set_frames_per_packet_ptr)->_data[1] = (uint16)((uint16)(max_frames)); \
    } while (0)

#define OPMSG_ASYNC_WBS_ENC_SET_FRAMES_PER_PACKET_MARSHALL(addr, opmsg_async_wbs_enc_set_frames_per_packet_ptr) memcpy((void *)(addr), (void *)(opmsg_async_wbs_enc_set_frames_per_packet_ptr), 2)
#define OPMSG_ASYNC_WBS_ENC_SET_FRAMES_PER_PACKET_UNMARSHALL(addr, opmsg_async_wbs_enc_set_frames_per_packet_ptr) memcpy((void *)(opmsg_async_wbs_enc_set_frames_per_packet_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_BTAddr_Operator_Message

  DESCRIPTION
    Message to operator.

  MEMBERS
    message_id      - message id
    bt_addr_nap     - btaddr nap
    bt_addr_uap_lap - btaddr uap and msb lap
    bt_addr_lap     - btaddr lap

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_BTADDR_OPERATOR_MESSAGE;

/* The following macros take OPMSG_BTADDR_OPERATOR_MESSAGE *opmsg_btaddr_operator_message_ptr */
#define OPMSG_BTADDR_OPERATOR_MESSAGE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_BTADDR_OPERATOR_MESSAGE_MESSAGE_ID_GET(opmsg_btaddr_operator_message_ptr) ((OPMSG_FRAMEWORK_KEY)(opmsg_btaddr_operator_message_ptr)->_data[0])
#define OPMSG_BTADDR_OPERATOR_MESSAGE_MESSAGE_ID_SET(opmsg_btaddr_operator_message_ptr, message_id) ((opmsg_btaddr_operator_message_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_BTADDR_OPERATOR_MESSAGE_BT_ADDR_NAP_WORD_OFFSET (1)
#define OPMSG_BTADDR_OPERATOR_MESSAGE_BT_ADDR_NAP_GET(opmsg_btaddr_operator_message_ptr) ((opmsg_btaddr_operator_message_ptr)->_data[1])
#define OPMSG_BTADDR_OPERATOR_MESSAGE_BT_ADDR_NAP_SET(opmsg_btaddr_operator_message_ptr, bt_addr_nap) ((opmsg_btaddr_operator_message_ptr)->_data[1] = (uint16)(bt_addr_nap))
#define OPMSG_BTADDR_OPERATOR_MESSAGE_BT_ADDR_UAP_LAP_WORD_OFFSET (2)
#define OPMSG_BTADDR_OPERATOR_MESSAGE_BT_ADDR_UAP_LAP_GET(opmsg_btaddr_operator_message_ptr) ((opmsg_btaddr_operator_message_ptr)->_data[2])
#define OPMSG_BTADDR_OPERATOR_MESSAGE_BT_ADDR_UAP_LAP_SET(opmsg_btaddr_operator_message_ptr, bt_addr_uap_lap) ((opmsg_btaddr_operator_message_ptr)->_data[2] = (uint16)(bt_addr_uap_lap))
#define OPMSG_BTADDR_OPERATOR_MESSAGE_BT_ADDR_LAP_WORD_OFFSET (3)
#define OPMSG_BTADDR_OPERATOR_MESSAGE_BT_ADDR_LAP_GET(opmsg_btaddr_operator_message_ptr) ((opmsg_btaddr_operator_message_ptr)->_data[3])
#define OPMSG_BTADDR_OPERATOR_MESSAGE_BT_ADDR_LAP_SET(opmsg_btaddr_operator_message_ptr, bt_addr_lap) ((opmsg_btaddr_operator_message_ptr)->_data[3] = (uint16)(bt_addr_lap))
#define OPMSG_BTADDR_OPERATOR_MESSAGE_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_BTADDR_OPERATOR_MESSAGE_CREATE(message_id, bt_addr_nap, bt_addr_uap_lap, bt_addr_lap) \
    (uint16)(message_id), \
    (uint16)(bt_addr_nap), \
    (uint16)(bt_addr_uap_lap), \
    (uint16)(bt_addr_lap)
#define OPMSG_BTADDR_OPERATOR_MESSAGE_PACK(opmsg_btaddr_operator_message_ptr, message_id, bt_addr_nap, bt_addr_uap_lap, bt_addr_lap) \
    do { \
        (opmsg_btaddr_operator_message_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_btaddr_operator_message_ptr)->_data[1] = (uint16)((uint16)(bt_addr_nap)); \
        (opmsg_btaddr_operator_message_ptr)->_data[2] = (uint16)((uint16)(bt_addr_uap_lap)); \
        (opmsg_btaddr_operator_message_ptr)->_data[3] = (uint16)((uint16)(bt_addr_lap)); \
    } while (0)

#define OPMSG_BTADDR_OPERATOR_MESSAGE_MARSHALL(addr, opmsg_btaddr_operator_message_ptr) memcpy((void *)(addr), (void *)(opmsg_btaddr_operator_message_ptr), 4)
#define OPMSG_BTADDR_OPERATOR_MESSAGE_UNMARSHALL(addr, opmsg_btaddr_operator_message_ptr) memcpy((void *)(opmsg_btaddr_operator_message_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    Opmsg_Celt_Enc_Set_Encoding_Params

  DESCRIPTION
    CELT Encoder Operator message for configuring the encoding parameters
    used for encoding.

  MEMBERS
    message_id - message id
    mode       - Number of subbands to use
    frame_size - Number of blocks per frame
    channels   - Bitpool size to use

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_CELT_ENC_SET_ENCODING_PARAMS;

/* The following macros take OPMSG_CELT_ENC_SET_ENCODING_PARAMS *opmsg_celt_enc_set_encoding_params_ptr */
#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_MESSAGE_ID_GET(opmsg_celt_enc_set_encoding_params_ptr) ((OPMSG_CELT_ENC_ID)(opmsg_celt_enc_set_encoding_params_ptr)->_data[0])
#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_MESSAGE_ID_SET(opmsg_celt_enc_set_encoding_params_ptr, message_id) ((opmsg_celt_enc_set_encoding_params_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_MODE_WORD_OFFSET (1)
#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_MODE_GET(opmsg_celt_enc_set_encoding_params_ptr) ((opmsg_celt_enc_set_encoding_params_ptr)->_data[1])
#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_MODE_SET(opmsg_celt_enc_set_encoding_params_ptr, mode) ((opmsg_celt_enc_set_encoding_params_ptr)->_data[1] = (uint16)(mode))
#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_FRAME_SIZE_WORD_OFFSET (2)
#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_FRAME_SIZE_GET(opmsg_celt_enc_set_encoding_params_ptr) ((opmsg_celt_enc_set_encoding_params_ptr)->_data[2])
#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_FRAME_SIZE_SET(opmsg_celt_enc_set_encoding_params_ptr, frame_size) ((opmsg_celt_enc_set_encoding_params_ptr)->_data[2] = (uint16)(frame_size))
#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_CHANNELS_WORD_OFFSET (3)
#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_CHANNELS_GET(opmsg_celt_enc_set_encoding_params_ptr) ((opmsg_celt_enc_set_encoding_params_ptr)->_data[3])
#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_CHANNELS_SET(opmsg_celt_enc_set_encoding_params_ptr, channels) ((opmsg_celt_enc_set_encoding_params_ptr)->_data[3] = (uint16)(channels))
#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_CREATE(message_id, mode, frame_size, channels) \
    (uint16)(message_id), \
    (uint16)(mode), \
    (uint16)(frame_size), \
    (uint16)(channels)
#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_PACK(opmsg_celt_enc_set_encoding_params_ptr, message_id, mode, frame_size, channels) \
    do { \
        (opmsg_celt_enc_set_encoding_params_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_celt_enc_set_encoding_params_ptr)->_data[1] = (uint16)((uint16)(mode)); \
        (opmsg_celt_enc_set_encoding_params_ptr)->_data[2] = (uint16)((uint16)(frame_size)); \
        (opmsg_celt_enc_set_encoding_params_ptr)->_data[3] = (uint16)((uint16)(channels)); \
    } while (0)

#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_MARSHALL(addr, opmsg_celt_enc_set_encoding_params_ptr) memcpy((void *)(addr), (void *)(opmsg_celt_enc_set_encoding_params_ptr), 4)
#define OPMSG_CELT_ENC_SET_ENCODING_PARAMS_UNMARSHALL(addr, opmsg_celt_enc_set_encoding_params_ptr) memcpy((void *)(opmsg_celt_enc_set_encoding_params_ptr), (void *)(addr), 4)


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

#define OPMSG_CH_MIXER_SET_PARAMETERS_MARSHALL(addr, opmsg_ch_mixer_set_parameters_ptr) memcpy((void *)(addr), (void *)(opmsg_ch_mixer_set_parameters_ptr), 4)
#define OPMSG_CH_MIXER_SET_PARAMETERS_UNMARSHALL(addr, opmsg_ch_mixer_set_parameters_ptr) memcpy((void *)(opmsg_ch_mixer_set_parameters_ptr), (void *)(addr), 4)


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

#define OPMSG_COMMON_GET_CAPABILITY_VERSION_RESP_MARSHALL(addr, opmsg_common_get_capability_version_resp_ptr) memcpy((void *)(addr), (void *)(opmsg_common_get_capability_version_resp_ptr), 3)
#define OPMSG_COMMON_GET_CAPABILITY_VERSION_RESP_UNMARSHALL(addr, opmsg_common_get_capability_version_resp_ptr) memcpy((void *)(opmsg_common_get_capability_version_resp_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    Opmsg_Common_Get_Defaults

  DESCRIPTION
    Operator common message for GET_DEFAULTS.

  MEMBERS
    message_id - message id
    num_blocks - number of parameter blocks that are intended to be retrieved.
                 Each block is defined by a start offset (param id) and a range
                 (number of params).
    offset     - offset
    range      - range

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_COMMON_GET_DEFAULTS;

/* The following macros take OPMSG_COMMON_GET_DEFAULTS *opmsg_common_get_defaults_ptr */
#define OPMSG_COMMON_GET_DEFAULTS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_GET_DEFAULTS_MESSAGE_ID_GET(opmsg_common_get_defaults_ptr) ((OPMSG_COMMON_ID)(opmsg_common_get_defaults_ptr)->_data[0])
#define OPMSG_COMMON_GET_DEFAULTS_MESSAGE_ID_SET(opmsg_common_get_defaults_ptr, message_id) ((opmsg_common_get_defaults_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_GET_DEFAULTS_NUM_BLOCKS_WORD_OFFSET (1)
#define OPMSG_COMMON_GET_DEFAULTS_NUM_BLOCKS_GET(opmsg_common_get_defaults_ptr) ((opmsg_common_get_defaults_ptr)->_data[1])
#define OPMSG_COMMON_GET_DEFAULTS_NUM_BLOCKS_SET(opmsg_common_get_defaults_ptr, num_blocks) ((opmsg_common_get_defaults_ptr)->_data[1] = (uint16)(num_blocks))
#define OPMSG_COMMON_GET_DEFAULTS_OFFSET_WORD_OFFSET (2)
#define OPMSG_COMMON_GET_DEFAULTS_OFFSET_GET(opmsg_common_get_defaults_ptr) ((opmsg_common_get_defaults_ptr)->_data[2])
#define OPMSG_COMMON_GET_DEFAULTS_OFFSET_SET(opmsg_common_get_defaults_ptr, offset) ((opmsg_common_get_defaults_ptr)->_data[2] = (uint16)(offset))
#define OPMSG_COMMON_GET_DEFAULTS_RANGE_WORD_OFFSET (3)
#define OPMSG_COMMON_GET_DEFAULTS_RANGE_GET(opmsg_common_get_defaults_ptr) ((opmsg_common_get_defaults_ptr)->_data[3])
#define OPMSG_COMMON_GET_DEFAULTS_RANGE_SET(opmsg_common_get_defaults_ptr, range) ((opmsg_common_get_defaults_ptr)->_data[3] = (uint16)(range))
#define OPMSG_COMMON_GET_DEFAULTS_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_GET_DEFAULTS_CREATE(message_id, num_blocks, offset, range) \
    (uint16)(message_id), \
    (uint16)(num_blocks), \
    (uint16)(offset), \
    (uint16)(range)
#define OPMSG_COMMON_GET_DEFAULTS_PACK(opmsg_common_get_defaults_ptr, message_id, num_blocks, offset, range) \
    do { \
        (opmsg_common_get_defaults_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_get_defaults_ptr)->_data[1] = (uint16)((uint16)(num_blocks)); \
        (opmsg_common_get_defaults_ptr)->_data[2] = (uint16)((uint16)(offset)); \
        (opmsg_common_get_defaults_ptr)->_data[3] = (uint16)((uint16)(range)); \
    } while (0)

#define OPMSG_COMMON_GET_DEFAULTS_MARSHALL(addr, opmsg_common_get_defaults_ptr) memcpy((void *)(addr), (void *)(opmsg_common_get_defaults_ptr), 4)
#define OPMSG_COMMON_GET_DEFAULTS_UNMARSHALL(addr, opmsg_common_get_defaults_ptr) memcpy((void *)(opmsg_common_get_defaults_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    Opmsg_Common_Get_Defaults_Resp

  DESCRIPTION
    Operator response for the message GET_DEFAULTS.

  MEMBERS
    message_id    - message id
    num_blocks    - number of parameter blocks that were requested. Each block
                    is defined by a start offset (param id) and a range (number
                    of params).
    offset        - offset of first parameter retrieved
    range         - number of parameters retrieved
    parameter_msw - The most significant 16 bits of the first parameter
                    retrieved
    parameter_lsw - The least significant 16 bits of the first parameter
                    retrieved. This split up of the parameter value is repeated
                    for all subsequent parameters that were retrieved

*******************************************************************************/
typedef struct
{
    uint16 _data[6];
} OPMSG_COMMON_GET_DEFAULTS_RESP;

/* The following macros take OPMSG_COMMON_GET_DEFAULTS_RESP *opmsg_common_get_defaults_resp_ptr */
#define OPMSG_COMMON_GET_DEFAULTS_RESP_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_GET_DEFAULTS_RESP_MESSAGE_ID_GET(opmsg_common_get_defaults_resp_ptr) ((OPMSG_COMMON_ID)(opmsg_common_get_defaults_resp_ptr)->_data[0])
#define OPMSG_COMMON_GET_DEFAULTS_RESP_MESSAGE_ID_SET(opmsg_common_get_defaults_resp_ptr, message_id) ((opmsg_common_get_defaults_resp_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_GET_DEFAULTS_RESP_NUM_BLOCKS_WORD_OFFSET (1)
#define OPMSG_COMMON_GET_DEFAULTS_RESP_NUM_BLOCKS_GET(opmsg_common_get_defaults_resp_ptr) ((opmsg_common_get_defaults_resp_ptr)->_data[1])
#define OPMSG_COMMON_GET_DEFAULTS_RESP_NUM_BLOCKS_SET(opmsg_common_get_defaults_resp_ptr, num_blocks) ((opmsg_common_get_defaults_resp_ptr)->_data[1] = (uint16)(num_blocks))
#define OPMSG_COMMON_GET_DEFAULTS_RESP_OFFSET_WORD_OFFSET (2)
#define OPMSG_COMMON_GET_DEFAULTS_RESP_OFFSET_GET(opmsg_common_get_defaults_resp_ptr) ((opmsg_common_get_defaults_resp_ptr)->_data[2])
#define OPMSG_COMMON_GET_DEFAULTS_RESP_OFFSET_SET(opmsg_common_get_defaults_resp_ptr, offset) ((opmsg_common_get_defaults_resp_ptr)->_data[2] = (uint16)(offset))
#define OPMSG_COMMON_GET_DEFAULTS_RESP_RANGE_WORD_OFFSET (3)
#define OPMSG_COMMON_GET_DEFAULTS_RESP_RANGE_GET(opmsg_common_get_defaults_resp_ptr) ((opmsg_common_get_defaults_resp_ptr)->_data[3])
#define OPMSG_COMMON_GET_DEFAULTS_RESP_RANGE_SET(opmsg_common_get_defaults_resp_ptr, range) ((opmsg_common_get_defaults_resp_ptr)->_data[3] = (uint16)(range))
#define OPMSG_COMMON_GET_DEFAULTS_RESP_PARAMETER_MSW_WORD_OFFSET (4)
#define OPMSG_COMMON_GET_DEFAULTS_RESP_PARAMETER_MSW_GET(opmsg_common_get_defaults_resp_ptr) ((opmsg_common_get_defaults_resp_ptr)->_data[4])
#define OPMSG_COMMON_GET_DEFAULTS_RESP_PARAMETER_MSW_SET(opmsg_common_get_defaults_resp_ptr, parameter_msw) ((opmsg_common_get_defaults_resp_ptr)->_data[4] = (uint16)(parameter_msw))
#define OPMSG_COMMON_GET_DEFAULTS_RESP_PARAMETER_LSW_WORD_OFFSET (5)
#define OPMSG_COMMON_GET_DEFAULTS_RESP_PARAMETER_LSW_GET(opmsg_common_get_defaults_resp_ptr) ((opmsg_common_get_defaults_resp_ptr)->_data[5])
#define OPMSG_COMMON_GET_DEFAULTS_RESP_PARAMETER_LSW_SET(opmsg_common_get_defaults_resp_ptr, parameter_lsw) ((opmsg_common_get_defaults_resp_ptr)->_data[5] = (uint16)(parameter_lsw))
#define OPMSG_COMMON_GET_DEFAULTS_RESP_WORD_SIZE (6)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_GET_DEFAULTS_RESP_CREATE(message_id, num_blocks, offset, range, parameter_msw, parameter_lsw) \
    (uint16)(message_id), \
    (uint16)(num_blocks), \
    (uint16)(offset), \
    (uint16)(range), \
    (uint16)(parameter_msw), \
    (uint16)(parameter_lsw)
#define OPMSG_COMMON_GET_DEFAULTS_RESP_PACK(opmsg_common_get_defaults_resp_ptr, message_id, num_blocks, offset, range, parameter_msw, parameter_lsw) \
    do { \
        (opmsg_common_get_defaults_resp_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_get_defaults_resp_ptr)->_data[1] = (uint16)((uint16)(num_blocks)); \
        (opmsg_common_get_defaults_resp_ptr)->_data[2] = (uint16)((uint16)(offset)); \
        (opmsg_common_get_defaults_resp_ptr)->_data[3] = (uint16)((uint16)(range)); \
        (opmsg_common_get_defaults_resp_ptr)->_data[4] = (uint16)((uint16)(parameter_msw)); \
        (opmsg_common_get_defaults_resp_ptr)->_data[5] = (uint16)((uint16)(parameter_lsw)); \
    } while (0)

#define OPMSG_COMMON_GET_DEFAULTS_RESP_MARSHALL(addr, opmsg_common_get_defaults_resp_ptr) memcpy((void *)(addr), (void *)(opmsg_common_get_defaults_resp_ptr), 6)
#define OPMSG_COMMON_GET_DEFAULTS_RESP_UNMARSHALL(addr, opmsg_common_get_defaults_resp_ptr) memcpy((void *)(opmsg_common_get_defaults_resp_ptr), (void *)(addr), 6)


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

#define OPMSG_COMMON_GET_LOGICAL_PS_ID_MARSHALL(addr, opmsg_common_get_logical_ps_id_ptr) memcpy((void *)(addr), (void *)(opmsg_common_get_logical_ps_id_ptr), 2)
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_UNMARSHALL(addr, opmsg_common_get_logical_ps_id_ptr) memcpy((void *)(opmsg_common_get_logical_ps_id_ptr), (void *)(addr), 2)


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

#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_MARSHALL(addr, opmsg_common_get_logical_ps_id_resp_ptr) memcpy((void *)(addr), (void *)(opmsg_common_get_logical_ps_id_resp_ptr), 5)
#define OPMSG_COMMON_GET_LOGICAL_PS_ID_RESP_UNMARSHALL(addr, opmsg_common_get_logical_ps_id_resp_ptr) memcpy((void *)(opmsg_common_get_logical_ps_id_resp_ptr), (void *)(addr), 5)


/*******************************************************************************

  NAME
    Opmsg_Common_Get_Params

  DESCRIPTION
    Operator common message for GET_PARAMS.

  MEMBERS
    message_id - message id
    num_blocks - number of parameter blocks that are intended to be retrieved.
                 Each block is defined by a start offset (param id) and a range
                 (number of params).
    offset     - offset
    range      - range

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_COMMON_GET_PARAMS;

/* The following macros take OPMSG_COMMON_GET_PARAMS *opmsg_common_get_params_ptr */
#define OPMSG_COMMON_GET_PARAMS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_GET_PARAMS_MESSAGE_ID_GET(opmsg_common_get_params_ptr) ((OPMSG_COMMON_ID)(opmsg_common_get_params_ptr)->_data[0])
#define OPMSG_COMMON_GET_PARAMS_MESSAGE_ID_SET(opmsg_common_get_params_ptr, message_id) ((opmsg_common_get_params_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_GET_PARAMS_NUM_BLOCKS_WORD_OFFSET (1)
#define OPMSG_COMMON_GET_PARAMS_NUM_BLOCKS_GET(opmsg_common_get_params_ptr) ((opmsg_common_get_params_ptr)->_data[1])
#define OPMSG_COMMON_GET_PARAMS_NUM_BLOCKS_SET(opmsg_common_get_params_ptr, num_blocks) ((opmsg_common_get_params_ptr)->_data[1] = (uint16)(num_blocks))
#define OPMSG_COMMON_GET_PARAMS_OFFSET_WORD_OFFSET (2)
#define OPMSG_COMMON_GET_PARAMS_OFFSET_GET(opmsg_common_get_params_ptr) ((opmsg_common_get_params_ptr)->_data[2])
#define OPMSG_COMMON_GET_PARAMS_OFFSET_SET(opmsg_common_get_params_ptr, offset) ((opmsg_common_get_params_ptr)->_data[2] = (uint16)(offset))
#define OPMSG_COMMON_GET_PARAMS_RANGE_WORD_OFFSET (3)
#define OPMSG_COMMON_GET_PARAMS_RANGE_GET(opmsg_common_get_params_ptr) ((opmsg_common_get_params_ptr)->_data[3])
#define OPMSG_COMMON_GET_PARAMS_RANGE_SET(opmsg_common_get_params_ptr, range) ((opmsg_common_get_params_ptr)->_data[3] = (uint16)(range))
#define OPMSG_COMMON_GET_PARAMS_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_GET_PARAMS_CREATE(message_id, num_blocks, offset, range) \
    (uint16)(message_id), \
    (uint16)(num_blocks), \
    (uint16)(offset), \
    (uint16)(range)
#define OPMSG_COMMON_GET_PARAMS_PACK(opmsg_common_get_params_ptr, message_id, num_blocks, offset, range) \
    do { \
        (opmsg_common_get_params_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_get_params_ptr)->_data[1] = (uint16)((uint16)(num_blocks)); \
        (opmsg_common_get_params_ptr)->_data[2] = (uint16)((uint16)(offset)); \
        (opmsg_common_get_params_ptr)->_data[3] = (uint16)((uint16)(range)); \
    } while (0)

#define OPMSG_COMMON_GET_PARAMS_MARSHALL(addr, opmsg_common_get_params_ptr) memcpy((void *)(addr), (void *)(opmsg_common_get_params_ptr), 4)
#define OPMSG_COMMON_GET_PARAMS_UNMARSHALL(addr, opmsg_common_get_params_ptr) memcpy((void *)(opmsg_common_get_params_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    Opmsg_Common_Get_Params_Resp

  DESCRIPTION
    Operator response to the op message GET_PARAMS.

  MEMBERS
    message_id    - message id
    num_blocks    - number of parameter blocks that were requested. Each block
                    is defined by a start offset (param id) and a range (number
                    of params).
    offset        - offset of first parameter retrieved
    range         - number of parameters retrieved
    parameter_msw - The most significant 16 bits of the first parameter
                    retrieved
    parameter_lsw - The least significant 16 bits of the first parameter
                    retrieved. This split up of the parameter value is repeated
                    for all subsequent parameters that were retrieved

*******************************************************************************/
typedef struct
{
    uint16 _data[6];
} OPMSG_COMMON_GET_PARAMS_RESP;

/* The following macros take OPMSG_COMMON_GET_PARAMS_RESP *opmsg_common_get_params_resp_ptr */
#define OPMSG_COMMON_GET_PARAMS_RESP_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_GET_PARAMS_RESP_MESSAGE_ID_GET(opmsg_common_get_params_resp_ptr) ((OPMSG_COMMON_ID)(opmsg_common_get_params_resp_ptr)->_data[0])
#define OPMSG_COMMON_GET_PARAMS_RESP_MESSAGE_ID_SET(opmsg_common_get_params_resp_ptr, message_id) ((opmsg_common_get_params_resp_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_GET_PARAMS_RESP_NUM_BLOCKS_WORD_OFFSET (1)
#define OPMSG_COMMON_GET_PARAMS_RESP_NUM_BLOCKS_GET(opmsg_common_get_params_resp_ptr) ((opmsg_common_get_params_resp_ptr)->_data[1])
#define OPMSG_COMMON_GET_PARAMS_RESP_NUM_BLOCKS_SET(opmsg_common_get_params_resp_ptr, num_blocks) ((opmsg_common_get_params_resp_ptr)->_data[1] = (uint16)(num_blocks))
#define OPMSG_COMMON_GET_PARAMS_RESP_OFFSET_WORD_OFFSET (2)
#define OPMSG_COMMON_GET_PARAMS_RESP_OFFSET_GET(opmsg_common_get_params_resp_ptr) ((opmsg_common_get_params_resp_ptr)->_data[2])
#define OPMSG_COMMON_GET_PARAMS_RESP_OFFSET_SET(opmsg_common_get_params_resp_ptr, offset) ((opmsg_common_get_params_resp_ptr)->_data[2] = (uint16)(offset))
#define OPMSG_COMMON_GET_PARAMS_RESP_RANGE_WORD_OFFSET (3)
#define OPMSG_COMMON_GET_PARAMS_RESP_RANGE_GET(opmsg_common_get_params_resp_ptr) ((opmsg_common_get_params_resp_ptr)->_data[3])
#define OPMSG_COMMON_GET_PARAMS_RESP_RANGE_SET(opmsg_common_get_params_resp_ptr, range) ((opmsg_common_get_params_resp_ptr)->_data[3] = (uint16)(range))
#define OPMSG_COMMON_GET_PARAMS_RESP_PARAMETER_MSW_WORD_OFFSET (4)
#define OPMSG_COMMON_GET_PARAMS_RESP_PARAMETER_MSW_GET(opmsg_common_get_params_resp_ptr) ((opmsg_common_get_params_resp_ptr)->_data[4])
#define OPMSG_COMMON_GET_PARAMS_RESP_PARAMETER_MSW_SET(opmsg_common_get_params_resp_ptr, parameter_msw) ((opmsg_common_get_params_resp_ptr)->_data[4] = (uint16)(parameter_msw))
#define OPMSG_COMMON_GET_PARAMS_RESP_PARAMETER_LSW_WORD_OFFSET (5)
#define OPMSG_COMMON_GET_PARAMS_RESP_PARAMETER_LSW_GET(opmsg_common_get_params_resp_ptr) ((opmsg_common_get_params_resp_ptr)->_data[5])
#define OPMSG_COMMON_GET_PARAMS_RESP_PARAMETER_LSW_SET(opmsg_common_get_params_resp_ptr, parameter_lsw) ((opmsg_common_get_params_resp_ptr)->_data[5] = (uint16)(parameter_lsw))
#define OPMSG_COMMON_GET_PARAMS_RESP_WORD_SIZE (6)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_GET_PARAMS_RESP_CREATE(message_id, num_blocks, offset, range, parameter_msw, parameter_lsw) \
    (uint16)(message_id), \
    (uint16)(num_blocks), \
    (uint16)(offset), \
    (uint16)(range), \
    (uint16)(parameter_msw), \
    (uint16)(parameter_lsw)
#define OPMSG_COMMON_GET_PARAMS_RESP_PACK(opmsg_common_get_params_resp_ptr, message_id, num_blocks, offset, range, parameter_msw, parameter_lsw) \
    do { \
        (opmsg_common_get_params_resp_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_get_params_resp_ptr)->_data[1] = (uint16)((uint16)(num_blocks)); \
        (opmsg_common_get_params_resp_ptr)->_data[2] = (uint16)((uint16)(offset)); \
        (opmsg_common_get_params_resp_ptr)->_data[3] = (uint16)((uint16)(range)); \
        (opmsg_common_get_params_resp_ptr)->_data[4] = (uint16)((uint16)(parameter_msw)); \
        (opmsg_common_get_params_resp_ptr)->_data[5] = (uint16)((uint16)(parameter_lsw)); \
    } while (0)

#define OPMSG_COMMON_GET_PARAMS_RESP_MARSHALL(addr, opmsg_common_get_params_resp_ptr) memcpy((void *)(addr), (void *)(opmsg_common_get_params_resp_ptr), 6)
#define OPMSG_COMMON_GET_PARAMS_RESP_UNMARSHALL(addr, opmsg_common_get_params_resp_ptr) memcpy((void *)(opmsg_common_get_params_resp_ptr), (void *)(addr), 6)


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

#define OPMSG_COMMON_MSG_CONFIGURE_MARSHALL(addr, opmsg_common_msg_configure_ptr) memcpy((void *)(addr), (void *)(opmsg_common_msg_configure_ptr), 4)
#define OPMSG_COMMON_MSG_CONFIGURE_UNMARSHALL(addr, opmsg_common_msg_configure_ptr) memcpy((void *)(opmsg_common_msg_configure_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    Opmsg_Common_Msg_Set_Latency_Limits

  DESCRIPTION
    Operator common message for SET_LATENCY_LIMITS.

  MEMBERS
    message_id     - message id
    min_latency_ms - Most-significant 16 bits of latency low limit, in
                     microseconds
    min_latency_ls - Least-significant 16 bits of  latency low limit, in
                     microseconds
    max_latency_ms - Most-significant 16 bits of latency high limit, in
                     microseconds
    max_latency_ls - Least-significant 16 bits of  latency high limit, in
                     microseconds

*******************************************************************************/
typedef struct
{
    uint16 _data[5];
} OPMSG_COMMON_MSG_SET_LATENCY_LIMITS;

/* The following macros take OPMSG_COMMON_MSG_SET_LATENCY_LIMITS *opmsg_common_msg_set_latency_limits_ptr */
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_MESSAGE_ID_GET(opmsg_common_msg_set_latency_limits_ptr) ((OPMSG_COMMON_ID)(opmsg_common_msg_set_latency_limits_ptr)->_data[0])
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_MESSAGE_ID_SET(opmsg_common_msg_set_latency_limits_ptr, message_id) ((opmsg_common_msg_set_latency_limits_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_MIN_LATENCY_MS_WORD_OFFSET (1)
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_MIN_LATENCY_MS_GET(opmsg_common_msg_set_latency_limits_ptr) ((opmsg_common_msg_set_latency_limits_ptr)->_data[1])
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_MIN_LATENCY_MS_SET(opmsg_common_msg_set_latency_limits_ptr, min_latency_ms) ((opmsg_common_msg_set_latency_limits_ptr)->_data[1] = (uint16)(min_latency_ms))
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_MIN_LATENCY_LS_WORD_OFFSET (2)
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_MIN_LATENCY_LS_GET(opmsg_common_msg_set_latency_limits_ptr) ((opmsg_common_msg_set_latency_limits_ptr)->_data[2])
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_MIN_LATENCY_LS_SET(opmsg_common_msg_set_latency_limits_ptr, min_latency_ls) ((opmsg_common_msg_set_latency_limits_ptr)->_data[2] = (uint16)(min_latency_ls))
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_MAX_LATENCY_MS_WORD_OFFSET (3)
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_MAX_LATENCY_MS_GET(opmsg_common_msg_set_latency_limits_ptr) ((opmsg_common_msg_set_latency_limits_ptr)->_data[3])
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_MAX_LATENCY_MS_SET(opmsg_common_msg_set_latency_limits_ptr, max_latency_ms) ((opmsg_common_msg_set_latency_limits_ptr)->_data[3] = (uint16)(max_latency_ms))
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_MAX_LATENCY_LS_WORD_OFFSET (4)
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_MAX_LATENCY_LS_GET(opmsg_common_msg_set_latency_limits_ptr) ((opmsg_common_msg_set_latency_limits_ptr)->_data[4])
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_MAX_LATENCY_LS_SET(opmsg_common_msg_set_latency_limits_ptr, max_latency_ls) ((opmsg_common_msg_set_latency_limits_ptr)->_data[4] = (uint16)(max_latency_ls))
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_WORD_SIZE (5)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_CREATE(message_id, min_latency_ms, min_latency_ls, max_latency_ms, max_latency_ls) \
    (uint16)(message_id), \
    (uint16)(min_latency_ms), \
    (uint16)(min_latency_ls), \
    (uint16)(max_latency_ms), \
    (uint16)(max_latency_ls)
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_PACK(opmsg_common_msg_set_latency_limits_ptr, message_id, min_latency_ms, min_latency_ls, max_latency_ms, max_latency_ls) \
    do { \
        (opmsg_common_msg_set_latency_limits_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_msg_set_latency_limits_ptr)->_data[1] = (uint16)((uint16)(min_latency_ms)); \
        (opmsg_common_msg_set_latency_limits_ptr)->_data[2] = (uint16)((uint16)(min_latency_ls)); \
        (opmsg_common_msg_set_latency_limits_ptr)->_data[3] = (uint16)((uint16)(max_latency_ms)); \
        (opmsg_common_msg_set_latency_limits_ptr)->_data[4] = (uint16)((uint16)(max_latency_ls)); \
    } while (0)

#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_MARSHALL(addr, opmsg_common_msg_set_latency_limits_ptr) memcpy((void *)(addr), (void *)(opmsg_common_msg_set_latency_limits_ptr), 5)
#define OPMSG_COMMON_MSG_SET_LATENCY_LIMITS_UNMARSHALL(addr, opmsg_common_msg_set_latency_limits_ptr) memcpy((void *)(opmsg_common_msg_set_latency_limits_ptr), (void *)(addr), 5)


/*******************************************************************************

  NAME
    Opmsg_Common_Msg_Set_Metadata_Delay

  DESCRIPTION
    Operator common message for setting algorithmic delay to apply to
    metadata. This ensures metadata tags remain aligned with the processed
    samples.

  MEMBERS
    message_id - message id
    delay      - The number of audio samples of delay introduced by the operator

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_COMMON_MSG_SET_METADATA_DELAY;

/* The following macros take OPMSG_COMMON_MSG_SET_METADATA_DELAY *opmsg_common_msg_set_metadata_delay_ptr */
#define OPMSG_COMMON_MSG_SET_METADATA_DELAY_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_MSG_SET_METADATA_DELAY_MESSAGE_ID_GET(opmsg_common_msg_set_metadata_delay_ptr) ((OPMSG_COMMON_ID)(opmsg_common_msg_set_metadata_delay_ptr)->_data[0])
#define OPMSG_COMMON_MSG_SET_METADATA_DELAY_MESSAGE_ID_SET(opmsg_common_msg_set_metadata_delay_ptr, message_id) ((opmsg_common_msg_set_metadata_delay_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_MSG_SET_METADATA_DELAY_DELAY_WORD_OFFSET (1)
#define OPMSG_COMMON_MSG_SET_METADATA_DELAY_DELAY_GET(opmsg_common_msg_set_metadata_delay_ptr) ((opmsg_common_msg_set_metadata_delay_ptr)->_data[1])
#define OPMSG_COMMON_MSG_SET_METADATA_DELAY_DELAY_SET(opmsg_common_msg_set_metadata_delay_ptr, delay) ((opmsg_common_msg_set_metadata_delay_ptr)->_data[1] = (uint16)(delay))
#define OPMSG_COMMON_MSG_SET_METADATA_DELAY_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_MSG_SET_METADATA_DELAY_CREATE(message_id, delay) \
    (uint16)(message_id), \
    (uint16)(delay)
#define OPMSG_COMMON_MSG_SET_METADATA_DELAY_PACK(opmsg_common_msg_set_metadata_delay_ptr, message_id, delay) \
    do { \
        (opmsg_common_msg_set_metadata_delay_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_msg_set_metadata_delay_ptr)->_data[1] = (uint16)((uint16)(delay)); \
    } while (0)

#define OPMSG_COMMON_MSG_SET_METADATA_DELAY_MARSHALL(addr, opmsg_common_msg_set_metadata_delay_ptr) memcpy((void *)(addr), (void *)(opmsg_common_msg_set_metadata_delay_ptr), 2)
#define OPMSG_COMMON_MSG_SET_METADATA_DELAY_UNMARSHALL(addr, opmsg_common_msg_set_metadata_delay_ptr) memcpy((void *)(opmsg_common_msg_set_metadata_delay_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Common_Msg_Set_Rate_Adjust_Current_Rate

  DESCRIPTION
    Operator common message for OPMSG_COMMON_SET_RATE_ADJUST_CURRENT_RATE

  MEMBERS
    message_id - message id
    rate       - current warp rate, in chip fractional format (for 24-bit
                 architectures this can be either sign or zero extended to 32
                 bits) Note: MSW16 first

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_COMMON_MSG_SET_RATE_ADJUST_CURRENT_RATE;

/* The following macros take OPMSG_COMMON_MSG_SET_RATE_ADJUST_CURRENT_RATE *opmsg_common_msg_set_rate_adjust_current_rate_ptr */
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_CURRENT_RATE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_CURRENT_RATE_MESSAGE_ID_GET(opmsg_common_msg_set_rate_adjust_current_rate_ptr) ((OPMSG_COMMON_ID)(opmsg_common_msg_set_rate_adjust_current_rate_ptr)->_data[0])
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_CURRENT_RATE_MESSAGE_ID_SET(opmsg_common_msg_set_rate_adjust_current_rate_ptr, message_id) ((opmsg_common_msg_set_rate_adjust_current_rate_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_CURRENT_RATE_RATE_WORD_OFFSET (1)
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_CURRENT_RATE_RATE_GET(opmsg_common_msg_set_rate_adjust_current_rate_ptr)  \
    (((uint32)((opmsg_common_msg_set_rate_adjust_current_rate_ptr)->_data[1]) | \
      ((uint32)((opmsg_common_msg_set_rate_adjust_current_rate_ptr)->_data[2]) << 16)))
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_CURRENT_RATE_RATE_SET(opmsg_common_msg_set_rate_adjust_current_rate_ptr, rate) do { \
        (opmsg_common_msg_set_rate_adjust_current_rate_ptr)->_data[1] = (uint16)((rate) & 0xffff); \
        (opmsg_common_msg_set_rate_adjust_current_rate_ptr)->_data[2] = (uint16)((rate) >> 16); } while (0)
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_CURRENT_RATE_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_CURRENT_RATE_CREATE(message_id, rate) \
    (uint16)(message_id), \
    (uint16)((rate) & 0xffff), \
    (uint16)((rate) >> 16)
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_CURRENT_RATE_PACK(opmsg_common_msg_set_rate_adjust_current_rate_ptr, message_id, rate) \
    do { \
        (opmsg_common_msg_set_rate_adjust_current_rate_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_msg_set_rate_adjust_current_rate_ptr)->_data[1] = (uint16)((uint16)((rate) & 0xffff)); \
        (opmsg_common_msg_set_rate_adjust_current_rate_ptr)->_data[2] = (uint16)(((rate) >> 16)); \
    } while (0)

#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_CURRENT_RATE_MARSHALL(addr, opmsg_common_msg_set_rate_adjust_current_rate_ptr) memcpy((void *)(addr), (void *)(opmsg_common_msg_set_rate_adjust_current_rate_ptr), 3)
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_CURRENT_RATE_UNMARSHALL(addr, opmsg_common_msg_set_rate_adjust_current_rate_ptr) memcpy((void *)(opmsg_common_msg_set_rate_adjust_current_rate_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    Opmsg_Common_Msg_Set_Rate_Adjust_Passthrough_Mode

  DESCRIPTION
    Operator common message for OPMSG_COMMON_SET_RATE_ADJUST_PASSTHROUGH_MODE

  MEMBERS
    message_id - message id
    mode       - passthrough mode Any non-zero value will set the operator into
                 passthrough mode

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_COMMON_MSG_SET_RATE_ADJUST_PASSTHROUGH_MODE;

/* The following macros take OPMSG_COMMON_MSG_SET_RATE_ADJUST_PASSTHROUGH_MODE *opmsg_common_msg_set_rate_adjust_passthrough_mode_ptr */
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_PASSTHROUGH_MODE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_PASSTHROUGH_MODE_MESSAGE_ID_GET(opmsg_common_msg_set_rate_adjust_passthrough_mode_ptr) ((OPMSG_COMMON_ID)(opmsg_common_msg_set_rate_adjust_passthrough_mode_ptr)->_data[0])
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_PASSTHROUGH_MODE_MESSAGE_ID_SET(opmsg_common_msg_set_rate_adjust_passthrough_mode_ptr, message_id) ((opmsg_common_msg_set_rate_adjust_passthrough_mode_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_PASSTHROUGH_MODE_MODE_WORD_OFFSET (1)
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_PASSTHROUGH_MODE_MODE_GET(opmsg_common_msg_set_rate_adjust_passthrough_mode_ptr) ((opmsg_common_msg_set_rate_adjust_passthrough_mode_ptr)->_data[1])
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_PASSTHROUGH_MODE_MODE_SET(opmsg_common_msg_set_rate_adjust_passthrough_mode_ptr, mode) ((opmsg_common_msg_set_rate_adjust_passthrough_mode_ptr)->_data[1] = (uint16)(mode))
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_PASSTHROUGH_MODE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_PASSTHROUGH_MODE_CREATE(message_id, mode) \
    (uint16)(message_id), \
    (uint16)(mode)
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_PASSTHROUGH_MODE_PACK(opmsg_common_msg_set_rate_adjust_passthrough_mode_ptr, message_id, mode) \
    do { \
        (opmsg_common_msg_set_rate_adjust_passthrough_mode_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_msg_set_rate_adjust_passthrough_mode_ptr)->_data[1] = (uint16)((uint16)(mode)); \
    } while (0)

#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_PASSTHROUGH_MODE_MARSHALL(addr, opmsg_common_msg_set_rate_adjust_passthrough_mode_ptr) memcpy((void *)(addr), (void *)(opmsg_common_msg_set_rate_adjust_passthrough_mode_ptr), 2)
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_PASSTHROUGH_MODE_UNMARSHALL(addr, opmsg_common_msg_set_rate_adjust_passthrough_mode_ptr) memcpy((void *)(opmsg_common_msg_set_rate_adjust_passthrough_mode_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Common_Msg_Set_Rate_Adjust_Target_Rate

  DESCRIPTION
    Operator common message for OPMSG_COMMON_SET_RATE_ADJUST_TARGET_RATE

  MEMBERS
    message_id - message id
    rate       - target warp rate, in chip fractional format (for 24-bit
                 architectures this can be either sign or zero extended to 32
                 bits) Note: MSW16 first

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_COMMON_MSG_SET_RATE_ADJUST_TARGET_RATE;

/* The following macros take OPMSG_COMMON_MSG_SET_RATE_ADJUST_TARGET_RATE *opmsg_common_msg_set_rate_adjust_target_rate_ptr */
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_TARGET_RATE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_TARGET_RATE_MESSAGE_ID_GET(opmsg_common_msg_set_rate_adjust_target_rate_ptr) ((OPMSG_COMMON_ID)(opmsg_common_msg_set_rate_adjust_target_rate_ptr)->_data[0])
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_TARGET_RATE_MESSAGE_ID_SET(opmsg_common_msg_set_rate_adjust_target_rate_ptr, message_id) ((opmsg_common_msg_set_rate_adjust_target_rate_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_TARGET_RATE_RATE_WORD_OFFSET (1)
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_TARGET_RATE_RATE_GET(opmsg_common_msg_set_rate_adjust_target_rate_ptr)  \
    (((uint32)((opmsg_common_msg_set_rate_adjust_target_rate_ptr)->_data[1]) | \
      ((uint32)((opmsg_common_msg_set_rate_adjust_target_rate_ptr)->_data[2]) << 16)))
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_TARGET_RATE_RATE_SET(opmsg_common_msg_set_rate_adjust_target_rate_ptr, rate) do { \
        (opmsg_common_msg_set_rate_adjust_target_rate_ptr)->_data[1] = (uint16)((rate) & 0xffff); \
        (opmsg_common_msg_set_rate_adjust_target_rate_ptr)->_data[2] = (uint16)((rate) >> 16); } while (0)
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_TARGET_RATE_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_TARGET_RATE_CREATE(message_id, rate) \
    (uint16)(message_id), \
    (uint16)((rate) & 0xffff), \
    (uint16)((rate) >> 16)
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_TARGET_RATE_PACK(opmsg_common_msg_set_rate_adjust_target_rate_ptr, message_id, rate) \
    do { \
        (opmsg_common_msg_set_rate_adjust_target_rate_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_msg_set_rate_adjust_target_rate_ptr)->_data[1] = (uint16)((uint16)((rate) & 0xffff)); \
        (opmsg_common_msg_set_rate_adjust_target_rate_ptr)->_data[2] = (uint16)(((rate) >> 16)); \
    } while (0)

#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_TARGET_RATE_MARSHALL(addr, opmsg_common_msg_set_rate_adjust_target_rate_ptr) memcpy((void *)(addr), (void *)(opmsg_common_msg_set_rate_adjust_target_rate_ptr), 3)
#define OPMSG_COMMON_MSG_SET_RATE_ADJUST_TARGET_RATE_UNMARSHALL(addr, opmsg_common_msg_set_rate_adjust_target_rate_ptr) memcpy((void *)(opmsg_common_msg_set_rate_adjust_target_rate_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    Opmsg_Common_Msg_Set_Sample_Rate

  DESCRIPTION
    Operator common message for SET_SAMPLE_RATE.

  MEMBERS
    message_id  - message id
    sample_rate - sample rate (fs/25)

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_COMMON_MSG_SET_SAMPLE_RATE;

/* The following macros take OPMSG_COMMON_MSG_SET_SAMPLE_RATE *opmsg_common_msg_set_sample_rate_ptr */
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_MESSAGE_ID_GET(opmsg_common_msg_set_sample_rate_ptr) ((OPMSG_COMMON_ID)(opmsg_common_msg_set_sample_rate_ptr)->_data[0])
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_MESSAGE_ID_SET(opmsg_common_msg_set_sample_rate_ptr, message_id) ((opmsg_common_msg_set_sample_rate_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_SAMPLE_RATE_WORD_OFFSET (1)
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_SAMPLE_RATE_GET(opmsg_common_msg_set_sample_rate_ptr) ((opmsg_common_msg_set_sample_rate_ptr)->_data[1])
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_SAMPLE_RATE_SET(opmsg_common_msg_set_sample_rate_ptr, sample_rate) ((opmsg_common_msg_set_sample_rate_ptr)->_data[1] = (uint16)(sample_rate))
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_CREATE(message_id, sample_rate) \
    (uint16)(message_id), \
    (uint16)(sample_rate)
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_PACK(opmsg_common_msg_set_sample_rate_ptr, message_id, sample_rate) \
    do { \
        (opmsg_common_msg_set_sample_rate_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_msg_set_sample_rate_ptr)->_data[1] = (uint16)((uint16)(sample_rate)); \
    } while (0)

#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_MARSHALL(addr, opmsg_common_msg_set_sample_rate_ptr) memcpy((void *)(addr), (void *)(opmsg_common_msg_set_sample_rate_ptr), 2)
#define OPMSG_COMMON_MSG_SET_SAMPLE_RATE_UNMARSHALL(addr, opmsg_common_msg_set_sample_rate_ptr) memcpy((void *)(opmsg_common_msg_set_sample_rate_ptr), (void *)(addr), 2)


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

#define OPMSG_COMMON_MSG_SET_TTP_LATENCY_MARSHALL(addr, opmsg_common_msg_set_ttp_latency_ptr) memcpy((void *)(addr), (void *)(opmsg_common_msg_set_ttp_latency_ptr), 3)
#define OPMSG_COMMON_MSG_SET_TTP_LATENCY_UNMARSHALL(addr, opmsg_common_msg_set_ttp_latency_ptr) memcpy((void *)(opmsg_common_msg_set_ttp_latency_ptr), (void *)(addr), 3)


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

#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_MARSHALL(addr, opmsg_common_msg_set_ttp_params_ptr) memcpy((void *)(addr), (void *)(opmsg_common_msg_set_ttp_params_ptr), 6)
#define OPMSG_COMMON_MSG_SET_TTP_PARAMS_UNMARSHALL(addr, opmsg_common_msg_set_ttp_params_ptr) memcpy((void *)(opmsg_common_msg_set_ttp_params_ptr), (void *)(addr), 6)


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

#define OPMSG_COMMON_SET_BUFFER_SIZE_MARSHALL(addr, opmsg_common_set_buffer_size_ptr) memcpy((void *)(addr), (void *)(opmsg_common_set_buffer_size_ptr), 2)
#define OPMSG_COMMON_SET_BUFFER_SIZE_UNMARSHALL(addr, opmsg_common_set_buffer_size_ptr) memcpy((void *)(opmsg_common_set_buffer_size_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Common_Set_Control

  DESCRIPTION
    Operator common message for SET_CONTROL.

  MEMBERS
    message_id - message id
    num_blocks - number of controls to be set in a message
    control_id - control id
    value_msw  - most significant 16 bits of the control value
    value_lsw  - least significant 16 bits of the control value

*******************************************************************************/
typedef struct
{
    uint16 _data[5];
} OPMSG_COMMON_SET_CONTROL;

/* The following macros take OPMSG_COMMON_SET_CONTROL *opmsg_common_set_control_ptr */
#define OPMSG_COMMON_SET_CONTROL_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_SET_CONTROL_MESSAGE_ID_GET(opmsg_common_set_control_ptr) ((OPMSG_COMMON_ID)(opmsg_common_set_control_ptr)->_data[0])
#define OPMSG_COMMON_SET_CONTROL_MESSAGE_ID_SET(opmsg_common_set_control_ptr, message_id) ((opmsg_common_set_control_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_SET_CONTROL_NUM_BLOCKS_WORD_OFFSET (1)
#define OPMSG_COMMON_SET_CONTROL_NUM_BLOCKS_GET(opmsg_common_set_control_ptr) ((opmsg_common_set_control_ptr)->_data[1])
#define OPMSG_COMMON_SET_CONTROL_NUM_BLOCKS_SET(opmsg_common_set_control_ptr, num_blocks) ((opmsg_common_set_control_ptr)->_data[1] = (uint16)(num_blocks))
#define OPMSG_COMMON_SET_CONTROL_CONTROL_ID_WORD_OFFSET (2)
#define OPMSG_COMMON_SET_CONTROL_CONTROL_ID_GET(opmsg_common_set_control_ptr) ((opmsg_common_set_control_ptr)->_data[2])
#define OPMSG_COMMON_SET_CONTROL_CONTROL_ID_SET(opmsg_common_set_control_ptr, control_id) ((opmsg_common_set_control_ptr)->_data[2] = (uint16)(control_id))
#define OPMSG_COMMON_SET_CONTROL_VALUE_MSW_WORD_OFFSET (3)
#define OPMSG_COMMON_SET_CONTROL_VALUE_MSW_GET(opmsg_common_set_control_ptr) ((opmsg_common_set_control_ptr)->_data[3])
#define OPMSG_COMMON_SET_CONTROL_VALUE_MSW_SET(opmsg_common_set_control_ptr, value_msw) ((opmsg_common_set_control_ptr)->_data[3] = (uint16)(value_msw))
#define OPMSG_COMMON_SET_CONTROL_VALUE_LSW_WORD_OFFSET (4)
#define OPMSG_COMMON_SET_CONTROL_VALUE_LSW_GET(opmsg_common_set_control_ptr) ((opmsg_common_set_control_ptr)->_data[4])
#define OPMSG_COMMON_SET_CONTROL_VALUE_LSW_SET(opmsg_common_set_control_ptr, value_lsw) ((opmsg_common_set_control_ptr)->_data[4] = (uint16)(value_lsw))
#define OPMSG_COMMON_SET_CONTROL_WORD_SIZE (5)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_SET_CONTROL_CREATE(message_id, num_blocks, control_id, value_msw, value_lsw) \
    (uint16)(message_id), \
    (uint16)(num_blocks), \
    (uint16)(control_id), \
    (uint16)(value_msw), \
    (uint16)(value_lsw)
#define OPMSG_COMMON_SET_CONTROL_PACK(opmsg_common_set_control_ptr, message_id, num_blocks, control_id, value_msw, value_lsw) \
    do { \
        (opmsg_common_set_control_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_set_control_ptr)->_data[1] = (uint16)((uint16)(num_blocks)); \
        (opmsg_common_set_control_ptr)->_data[2] = (uint16)((uint16)(control_id)); \
        (opmsg_common_set_control_ptr)->_data[3] = (uint16)((uint16)(value_msw)); \
        (opmsg_common_set_control_ptr)->_data[4] = (uint16)((uint16)(value_lsw)); \
    } while (0)

#define OPMSG_COMMON_SET_CONTROL_MARSHALL(addr, opmsg_common_set_control_ptr) memcpy((void *)(addr), (void *)(opmsg_common_set_control_ptr), 5)
#define OPMSG_COMMON_SET_CONTROL_UNMARSHALL(addr, opmsg_common_set_control_ptr) memcpy((void *)(opmsg_common_set_control_ptr), (void *)(addr), 5)


/*******************************************************************************

  NAME
    Opmsg_Common_Set_Params

  DESCRIPTION
    Operator common message for SET_PARAMS.

  MEMBERS
    message_id          - message id
    num_blocks          - number of parameter blocks that were need to be set.
                          Each block is defined by a start offset (param id) and
                          a range (number of params).
    offset              - offset of first parameter to be set
    range               - number of parameters to be set
    parameter_value_msw - The most significant 16 bits of the first parameter
                          that has to be set
    parameter_value_lsw - The least significant 16 bits of the first parameter
                          that has to be set. This split up of the parameter
                          value is repeated for all subsequent parameters that
                          need to be set

*******************************************************************************/
typedef struct
{
    uint16 _data[6];
} OPMSG_COMMON_SET_PARAMS;

/* The following macros take OPMSG_COMMON_SET_PARAMS *opmsg_common_set_params_ptr */
#define OPMSG_COMMON_SET_PARAMS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_COMMON_SET_PARAMS_MESSAGE_ID_GET(opmsg_common_set_params_ptr) ((OPMSG_COMMON_ID)(opmsg_common_set_params_ptr)->_data[0])
#define OPMSG_COMMON_SET_PARAMS_MESSAGE_ID_SET(opmsg_common_set_params_ptr, message_id) ((opmsg_common_set_params_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_COMMON_SET_PARAMS_NUM_BLOCKS_WORD_OFFSET (1)
#define OPMSG_COMMON_SET_PARAMS_NUM_BLOCKS_GET(opmsg_common_set_params_ptr) ((opmsg_common_set_params_ptr)->_data[1])
#define OPMSG_COMMON_SET_PARAMS_NUM_BLOCKS_SET(opmsg_common_set_params_ptr, num_blocks) ((opmsg_common_set_params_ptr)->_data[1] = (uint16)(num_blocks))
#define OPMSG_COMMON_SET_PARAMS_OFFSET_WORD_OFFSET (2)
#define OPMSG_COMMON_SET_PARAMS_OFFSET_GET(opmsg_common_set_params_ptr) ((opmsg_common_set_params_ptr)->_data[2])
#define OPMSG_COMMON_SET_PARAMS_OFFSET_SET(opmsg_common_set_params_ptr, offset) ((opmsg_common_set_params_ptr)->_data[2] = (uint16)(offset))
#define OPMSG_COMMON_SET_PARAMS_RANGE_WORD_OFFSET (3)
#define OPMSG_COMMON_SET_PARAMS_RANGE_GET(opmsg_common_set_params_ptr) ((opmsg_common_set_params_ptr)->_data[3])
#define OPMSG_COMMON_SET_PARAMS_RANGE_SET(opmsg_common_set_params_ptr, range) ((opmsg_common_set_params_ptr)->_data[3] = (uint16)(range))
#define OPMSG_COMMON_SET_PARAMS_PARAMETER_VALUE_MSW_WORD_OFFSET (4)
#define OPMSG_COMMON_SET_PARAMS_PARAMETER_VALUE_MSW_GET(opmsg_common_set_params_ptr) ((opmsg_common_set_params_ptr)->_data[4])
#define OPMSG_COMMON_SET_PARAMS_PARAMETER_VALUE_MSW_SET(opmsg_common_set_params_ptr, parameter_value_msw) ((opmsg_common_set_params_ptr)->_data[4] = (uint16)(parameter_value_msw))
#define OPMSG_COMMON_SET_PARAMS_PARAMETER_VALUE_LSW_WORD_OFFSET (5)
#define OPMSG_COMMON_SET_PARAMS_PARAMETER_VALUE_LSW_GET(opmsg_common_set_params_ptr) ((opmsg_common_set_params_ptr)->_data[5])
#define OPMSG_COMMON_SET_PARAMS_PARAMETER_VALUE_LSW_SET(opmsg_common_set_params_ptr, parameter_value_lsw) ((opmsg_common_set_params_ptr)->_data[5] = (uint16)(parameter_value_lsw))
#define OPMSG_COMMON_SET_PARAMS_WORD_SIZE (6)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_COMMON_SET_PARAMS_CREATE(message_id, num_blocks, offset, range, parameter_value_msw, parameter_value_lsw) \
    (uint16)(message_id), \
    (uint16)(num_blocks), \
    (uint16)(offset), \
    (uint16)(range), \
    (uint16)(parameter_value_msw), \
    (uint16)(parameter_value_lsw)
#define OPMSG_COMMON_SET_PARAMS_PACK(opmsg_common_set_params_ptr, message_id, num_blocks, offset, range, parameter_value_msw, parameter_value_lsw) \
    do { \
        (opmsg_common_set_params_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_common_set_params_ptr)->_data[1] = (uint16)((uint16)(num_blocks)); \
        (opmsg_common_set_params_ptr)->_data[2] = (uint16)((uint16)(offset)); \
        (opmsg_common_set_params_ptr)->_data[3] = (uint16)((uint16)(range)); \
        (opmsg_common_set_params_ptr)->_data[4] = (uint16)((uint16)(parameter_value_msw)); \
        (opmsg_common_set_params_ptr)->_data[5] = (uint16)((uint16)(parameter_value_lsw)); \
    } while (0)

#define OPMSG_COMMON_SET_PARAMS_MARSHALL(addr, opmsg_common_set_params_ptr) memcpy((void *)(addr), (void *)(opmsg_common_set_params_ptr), 6)
#define OPMSG_COMMON_SET_PARAMS_UNMARSHALL(addr, opmsg_common_set_params_ptr) memcpy((void *)(opmsg_common_set_params_ptr), (void *)(addr), 6)


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

#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_MARSHALL(addr, opmsg_common_set_terminal_buffer_size_ptr) memcpy((void *)(addr), (void *)(opmsg_common_set_terminal_buffer_size_ptr), 4)
#define OPMSG_COMMON_SET_TERMINAL_BUFFER_SIZE_UNMARSHALL(addr, opmsg_common_set_terminal_buffer_size_ptr) memcpy((void *)(opmsg_common_set_terminal_buffer_size_ptr), (void *)(addr), 4)


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

#define OPMSG_COMMON_SET_UCID_MARSHALL(addr, opmsg_common_set_ucid_ptr) memcpy((void *)(addr), (void *)(opmsg_common_set_ucid_ptr), 2)
#define OPMSG_COMMON_SET_UCID_UNMARSHALL(addr, opmsg_common_set_ucid_ptr) memcpy((void *)(opmsg_common_set_ucid_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Consumer_Change_Input_Data_Type

  DESCRIPTION
    CONSUMER operator message for CHANGE_INPUT_DATA_TYPE.

  MEMBERS
    message_id - message id
    data_type  - data type

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_CONSUMER_CHANGE_INPUT_DATA_TYPE;

/* The following macros take OPMSG_CONSUMER_CHANGE_INPUT_DATA_TYPE *opmsg_consumer_change_input_data_type_ptr */
#define OPMSG_CONSUMER_CHANGE_INPUT_DATA_TYPE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_CONSUMER_CHANGE_INPUT_DATA_TYPE_MESSAGE_ID_GET(opmsg_consumer_change_input_data_type_ptr) ((OPMSG_CONSUMER_ID)(opmsg_consumer_change_input_data_type_ptr)->_data[0])
#define OPMSG_CONSUMER_CHANGE_INPUT_DATA_TYPE_MESSAGE_ID_SET(opmsg_consumer_change_input_data_type_ptr, message_id) ((opmsg_consumer_change_input_data_type_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_CONSUMER_CHANGE_INPUT_DATA_TYPE_DATA_TYPE_WORD_OFFSET (1)
#define OPMSG_CONSUMER_CHANGE_INPUT_DATA_TYPE_DATA_TYPE_GET(opmsg_consumer_change_input_data_type_ptr) ((opmsg_consumer_change_input_data_type_ptr)->_data[1])
#define OPMSG_CONSUMER_CHANGE_INPUT_DATA_TYPE_DATA_TYPE_SET(opmsg_consumer_change_input_data_type_ptr, data_type) ((opmsg_consumer_change_input_data_type_ptr)->_data[1] = (uint16)(data_type))
#define OPMSG_CONSUMER_CHANGE_INPUT_DATA_TYPE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_CONSUMER_CHANGE_INPUT_DATA_TYPE_CREATE(message_id, data_type) \
    (uint16)(message_id), \
    (uint16)(data_type)
#define OPMSG_CONSUMER_CHANGE_INPUT_DATA_TYPE_PACK(opmsg_consumer_change_input_data_type_ptr, message_id, data_type) \
    do { \
        (opmsg_consumer_change_input_data_type_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_consumer_change_input_data_type_ptr)->_data[1] = (uint16)((uint16)(data_type)); \
    } while (0)

#define OPMSG_CONSUMER_CHANGE_INPUT_DATA_TYPE_MARSHALL(addr, opmsg_consumer_change_input_data_type_ptr) memcpy((void *)(addr), (void *)(opmsg_consumer_change_input_data_type_ptr), 2)
#define OPMSG_CONSUMER_CHANGE_INPUT_DATA_TYPE_UNMARSHALL(addr, opmsg_consumer_change_input_data_type_ptr) memcpy((void *)(opmsg_consumer_change_input_data_type_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Consumer_Enable_Back_Kick

  DESCRIPTION
    CONSUMER operator message for ENABLE_BACK_KICK.

  MEMBERS
    message_id       - message id
    enable_back_kick - enable back kicks

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_CONSUMER_ENABLE_BACK_KICK;

/* The following macros take OPMSG_CONSUMER_ENABLE_BACK_KICK *opmsg_consumer_enable_back_kick_ptr */
#define OPMSG_CONSUMER_ENABLE_BACK_KICK_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_CONSUMER_ENABLE_BACK_KICK_MESSAGE_ID_GET(opmsg_consumer_enable_back_kick_ptr) ((OPMSG_CONSUMER_ID)(opmsg_consumer_enable_back_kick_ptr)->_data[0])
#define OPMSG_CONSUMER_ENABLE_BACK_KICK_MESSAGE_ID_SET(opmsg_consumer_enable_back_kick_ptr, message_id) ((opmsg_consumer_enable_back_kick_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_CONSUMER_ENABLE_BACK_KICK_ENABLE_BACK_KICK_WORD_OFFSET (1)
#define OPMSG_CONSUMER_ENABLE_BACK_KICK_ENABLE_BACK_KICK_GET(opmsg_consumer_enable_back_kick_ptr) ((opmsg_consumer_enable_back_kick_ptr)->_data[1])
#define OPMSG_CONSUMER_ENABLE_BACK_KICK_ENABLE_BACK_KICK_SET(opmsg_consumer_enable_back_kick_ptr, enable_back_kick) ((opmsg_consumer_enable_back_kick_ptr)->_data[1] = (uint16)(enable_back_kick))
#define OPMSG_CONSUMER_ENABLE_BACK_KICK_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_CONSUMER_ENABLE_BACK_KICK_CREATE(message_id, enable_back_kick) \
    (uint16)(message_id), \
    (uint16)(enable_back_kick)
#define OPMSG_CONSUMER_ENABLE_BACK_KICK_PACK(opmsg_consumer_enable_back_kick_ptr, message_id, enable_back_kick) \
    do { \
        (opmsg_consumer_enable_back_kick_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_consumer_enable_back_kick_ptr)->_data[1] = (uint16)((uint16)(enable_back_kick)); \
    } while (0)

#define OPMSG_CONSUMER_ENABLE_BACK_KICK_MARSHALL(addr, opmsg_consumer_enable_back_kick_ptr) memcpy((void *)(addr), (void *)(opmsg_consumer_enable_back_kick_ptr), 2)
#define OPMSG_CONSUMER_ENABLE_BACK_KICK_UNMARSHALL(addr, opmsg_consumer_enable_back_kick_ptr) memcpy((void *)(opmsg_consumer_enable_back_kick_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_From_Op_Spdif_Decode_Adapt_To_New_Rate

  DESCRIPTION
    This message is sent to the client when the sample rate has changed. The
    client needs to adapt the system to the new rate and once adaptation
    process is complete it should inform the operator that the output rate
    has changed via SET_OUTPUT_RATE message

  MEMBERS
    message_id - message id
    new_rate   - new rate in 25Hz units

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_FROM_OP_SPDIF_DECODE_ADAPT_TO_NEW_RATE;

/* The following macros take OPMSG_FROM_OP_SPDIF_DECODE_ADAPT_TO_NEW_RATE *opmsg_from_op_spdif_decode_adapt_to_new_rate_ptr */
#define OPMSG_FROM_OP_SPDIF_DECODE_ADAPT_TO_NEW_RATE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_FROM_OP_SPDIF_DECODE_ADAPT_TO_NEW_RATE_MESSAGE_ID_GET(opmsg_from_op_spdif_decode_adapt_to_new_rate_ptr) ((OPMSG_FROM_OP_SPDIF_DECODE_ID)(opmsg_from_op_spdif_decode_adapt_to_new_rate_ptr)->_data[0])
#define OPMSG_FROM_OP_SPDIF_DECODE_ADAPT_TO_NEW_RATE_MESSAGE_ID_SET(opmsg_from_op_spdif_decode_adapt_to_new_rate_ptr, message_id) ((opmsg_from_op_spdif_decode_adapt_to_new_rate_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_FROM_OP_SPDIF_DECODE_ADAPT_TO_NEW_RATE_NEW_RATE_WORD_OFFSET (1)
#define OPMSG_FROM_OP_SPDIF_DECODE_ADAPT_TO_NEW_RATE_NEW_RATE_GET(opmsg_from_op_spdif_decode_adapt_to_new_rate_ptr) ((opmsg_from_op_spdif_decode_adapt_to_new_rate_ptr)->_data[1])
#define OPMSG_FROM_OP_SPDIF_DECODE_ADAPT_TO_NEW_RATE_NEW_RATE_SET(opmsg_from_op_spdif_decode_adapt_to_new_rate_ptr, new_rate) ((opmsg_from_op_spdif_decode_adapt_to_new_rate_ptr)->_data[1] = (uint16)(new_rate))
#define OPMSG_FROM_OP_SPDIF_DECODE_ADAPT_TO_NEW_RATE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_FROM_OP_SPDIF_DECODE_ADAPT_TO_NEW_RATE_CREATE(message_id, new_rate) \
    (uint16)(message_id), \
    (uint16)(new_rate)
#define OPMSG_FROM_OP_SPDIF_DECODE_ADAPT_TO_NEW_RATE_PACK(opmsg_from_op_spdif_decode_adapt_to_new_rate_ptr, message_id, new_rate) \
    do { \
        (opmsg_from_op_spdif_decode_adapt_to_new_rate_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_from_op_spdif_decode_adapt_to_new_rate_ptr)->_data[1] = (uint16)((uint16)(new_rate)); \
    } while (0)

#define OPMSG_FROM_OP_SPDIF_DECODE_ADAPT_TO_NEW_RATE_MARSHALL(addr, opmsg_from_op_spdif_decode_adapt_to_new_rate_ptr) memcpy((void *)(addr), (void *)(opmsg_from_op_spdif_decode_adapt_to_new_rate_ptr), 2)
#define OPMSG_FROM_OP_SPDIF_DECODE_ADAPT_TO_NEW_RATE_UNMARSHALL(addr, opmsg_from_op_spdif_decode_adapt_to_new_rate_ptr) memcpy((void *)(opmsg_from_op_spdif_decode_adapt_to_new_rate_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_From_Op_Spdif_Decode_Input_Rate_Valid

  DESCRIPTION
    Tells the decoder whether the input is valid or not. operator doesn't
    require the client to do any specific action but client might want to use
    this for other purposes.

  MEMBERS
    message_id - message id
    input_rate - will be 0 for invalid input else the sampling rate of the input

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_FROM_OP_SPDIF_DECODE_INPUT_RATE_VALID;

/* The following macros take OPMSG_FROM_OP_SPDIF_DECODE_INPUT_RATE_VALID *opmsg_from_op_spdif_decode_input_rate_valid_ptr */
#define OPMSG_FROM_OP_SPDIF_DECODE_INPUT_RATE_VALID_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_FROM_OP_SPDIF_DECODE_INPUT_RATE_VALID_MESSAGE_ID_GET(opmsg_from_op_spdif_decode_input_rate_valid_ptr) ((OPMSG_FROM_OP_SPDIF_DECODE_ID)(opmsg_from_op_spdif_decode_input_rate_valid_ptr)->_data[0])
#define OPMSG_FROM_OP_SPDIF_DECODE_INPUT_RATE_VALID_MESSAGE_ID_SET(opmsg_from_op_spdif_decode_input_rate_valid_ptr, message_id) ((opmsg_from_op_spdif_decode_input_rate_valid_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_FROM_OP_SPDIF_DECODE_INPUT_RATE_VALID_INPUT_RATE_WORD_OFFSET (1)
#define OPMSG_FROM_OP_SPDIF_DECODE_INPUT_RATE_VALID_INPUT_RATE_GET(opmsg_from_op_spdif_decode_input_rate_valid_ptr)  \
    (((uint32)((opmsg_from_op_spdif_decode_input_rate_valid_ptr)->_data[1]) | \
      ((uint32)((opmsg_from_op_spdif_decode_input_rate_valid_ptr)->_data[2]) << 16)))
#define OPMSG_FROM_OP_SPDIF_DECODE_INPUT_RATE_VALID_INPUT_RATE_SET(opmsg_from_op_spdif_decode_input_rate_valid_ptr, input_rate) do { \
        (opmsg_from_op_spdif_decode_input_rate_valid_ptr)->_data[1] = (uint16)((input_rate) & 0xffff); \
        (opmsg_from_op_spdif_decode_input_rate_valid_ptr)->_data[2] = (uint16)((input_rate) >> 16); } while (0)
#define OPMSG_FROM_OP_SPDIF_DECODE_INPUT_RATE_VALID_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_FROM_OP_SPDIF_DECODE_INPUT_RATE_VALID_CREATE(message_id, input_rate) \
    (uint16)(message_id), \
    (uint16)((input_rate) & 0xffff), \
    (uint16)((input_rate) >> 16)
#define OPMSG_FROM_OP_SPDIF_DECODE_INPUT_RATE_VALID_PACK(opmsg_from_op_spdif_decode_input_rate_valid_ptr, message_id, input_rate) \
    do { \
        (opmsg_from_op_spdif_decode_input_rate_valid_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_from_op_spdif_decode_input_rate_valid_ptr)->_data[1] = (uint16)((uint16)((input_rate) & 0xffff)); \
        (opmsg_from_op_spdif_decode_input_rate_valid_ptr)->_data[2] = (uint16)(((input_rate) >> 16)); \
    } while (0)

#define OPMSG_FROM_OP_SPDIF_DECODE_INPUT_RATE_VALID_MARSHALL(addr, opmsg_from_op_spdif_decode_input_rate_valid_ptr) memcpy((void *)(addr), (void *)(opmsg_from_op_spdif_decode_input_rate_valid_ptr), 3)
#define OPMSG_FROM_OP_SPDIF_DECODE_INPUT_RATE_VALID_UNMARSHALL(addr, opmsg_from_op_spdif_decode_input_rate_valid_ptr) memcpy((void *)(opmsg_from_op_spdif_decode_input_rate_valid_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    Opmsg_From_Op_Spdif_Decode_New_Channel_Status

  DESCRIPTION
    NEW channel Status is forwarded to the client

  MEMBERS
    message_id     - message id
    channel        - the channel that this channel status report message is for
    channel_status - list channel status words (exactly 12 words)

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_FROM_OP_SPDIF_DECODE_NEW_CHANNEL_STATUS;

/* The following macros take OPMSG_FROM_OP_SPDIF_DECODE_NEW_CHANNEL_STATUS *opmsg_from_op_spdif_decode_new_channel_status_ptr */
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_CHANNEL_STATUS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_CHANNEL_STATUS_MESSAGE_ID_GET(opmsg_from_op_spdif_decode_new_channel_status_ptr) ((OPMSG_FROM_OP_SPDIF_DECODE_ID)(opmsg_from_op_spdif_decode_new_channel_status_ptr)->_data[0])
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_CHANNEL_STATUS_MESSAGE_ID_SET(opmsg_from_op_spdif_decode_new_channel_status_ptr, message_id) ((opmsg_from_op_spdif_decode_new_channel_status_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_CHANNEL_STATUS_CHANNEL_WORD_OFFSET (1)
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_CHANNEL_STATUS_CHANNEL_GET(opmsg_from_op_spdif_decode_new_channel_status_ptr) ((opmsg_from_op_spdif_decode_new_channel_status_ptr)->_data[1])
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_CHANNEL_STATUS_CHANNEL_SET(opmsg_from_op_spdif_decode_new_channel_status_ptr, channel) ((opmsg_from_op_spdif_decode_new_channel_status_ptr)->_data[1] = (uint16)(channel))
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_CHANNEL_STATUS_CHANNEL_STATUS_WORD_OFFSET (2)
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_CHANNEL_STATUS_CHANNEL_STATUS_GET(opmsg_from_op_spdif_decode_new_channel_status_ptr) ((opmsg_from_op_spdif_decode_new_channel_status_ptr)->_data[2])
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_CHANNEL_STATUS_CHANNEL_STATUS_SET(opmsg_from_op_spdif_decode_new_channel_status_ptr, channel_status) ((opmsg_from_op_spdif_decode_new_channel_status_ptr)->_data[2] = (uint16)(channel_status))
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_CHANNEL_STATUS_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_CHANNEL_STATUS_CREATE(message_id, channel, channel_status) \
    (uint16)(message_id), \
    (uint16)(channel), \
    (uint16)(channel_status)
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_CHANNEL_STATUS_PACK(opmsg_from_op_spdif_decode_new_channel_status_ptr, message_id, channel, channel_status) \
    do { \
        (opmsg_from_op_spdif_decode_new_channel_status_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_from_op_spdif_decode_new_channel_status_ptr)->_data[1] = (uint16)((uint16)(channel)); \
        (opmsg_from_op_spdif_decode_new_channel_status_ptr)->_data[2] = (uint16)((uint16)(channel_status)); \
    } while (0)

#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_CHANNEL_STATUS_MARSHALL(addr, opmsg_from_op_spdif_decode_new_channel_status_ptr) memcpy((void *)(addr), (void *)(opmsg_from_op_spdif_decode_new_channel_status_ptr), 3)
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_CHANNEL_STATUS_UNMARSHALL(addr, opmsg_from_op_spdif_decode_new_channel_status_ptr) memcpy((void *)(opmsg_from_op_spdif_decode_new_channel_status_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    Opmsg_From_Op_Spdif_Decode_New_Data_Type

  DESCRIPTION
    New data type is being received, operator doesn't require the client to
    do any specific action but client might want to use this for other
    purposes.

  MEMBERS
    message_id    - message id
    new_data_type - new data type

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_FROM_OP_SPDIF_DECODE_NEW_DATA_TYPE;

/* The following macros take OPMSG_FROM_OP_SPDIF_DECODE_NEW_DATA_TYPE *opmsg_from_op_spdif_decode_new_data_type_ptr */
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DATA_TYPE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DATA_TYPE_MESSAGE_ID_GET(opmsg_from_op_spdif_decode_new_data_type_ptr) ((OPMSG_FROM_OP_SPDIF_DECODE_ID)(opmsg_from_op_spdif_decode_new_data_type_ptr)->_data[0])
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DATA_TYPE_MESSAGE_ID_SET(opmsg_from_op_spdif_decode_new_data_type_ptr, message_id) ((opmsg_from_op_spdif_decode_new_data_type_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DATA_TYPE_NEW_DATA_TYPE_WORD_OFFSET (1)
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DATA_TYPE_NEW_DATA_TYPE_GET(opmsg_from_op_spdif_decode_new_data_type_ptr) ((opmsg_from_op_spdif_decode_new_data_type_ptr)->_data[1])
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DATA_TYPE_NEW_DATA_TYPE_SET(opmsg_from_op_spdif_decode_new_data_type_ptr, new_data_type) ((opmsg_from_op_spdif_decode_new_data_type_ptr)->_data[1] = (uint16)(new_data_type))
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DATA_TYPE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DATA_TYPE_CREATE(message_id, new_data_type) \
    (uint16)(message_id), \
    (uint16)(new_data_type)
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DATA_TYPE_PACK(opmsg_from_op_spdif_decode_new_data_type_ptr, message_id, new_data_type) \
    do { \
        (opmsg_from_op_spdif_decode_new_data_type_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_from_op_spdif_decode_new_data_type_ptr)->_data[1] = (uint16)((uint16)(new_data_type)); \
    } while (0)

#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DATA_TYPE_MARSHALL(addr, opmsg_from_op_spdif_decode_new_data_type_ptr) memcpy((void *)(addr), (void *)(opmsg_from_op_spdif_decode_new_data_type_ptr), 2)
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DATA_TYPE_UNMARSHALL(addr, opmsg_from_op_spdif_decode_new_data_type_ptr) memcpy((void *)(opmsg_from_op_spdif_decode_new_data_type_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_From_Op_Spdif_Decode_New_Decoder_required

  DESCRIPTION
    Tells the client that a new decoder is required, the client must load the
    decoder and do all the connections required and starts the decoder before
    telling the operator that the decoder is ready to use.

  MEMBERS
    message_id    - message id
    new_data_type - new data type

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_FROM_OP_SPDIF_DECODE_NEW_DECODER_REQUIRED;

/* The following macros take OPMSG_FROM_OP_SPDIF_DECODE_NEW_DECODER_REQUIRED *opmsg_from_op_spdif_decode_new_decoder_required_ptr */
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DECODER_REQUIRED_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DECODER_REQUIRED_MESSAGE_ID_GET(opmsg_from_op_spdif_decode_new_decoder_required_ptr) ((OPMSG_FROM_OP_SPDIF_DECODE_ID)(opmsg_from_op_spdif_decode_new_decoder_required_ptr)->_data[0])
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DECODER_REQUIRED_MESSAGE_ID_SET(opmsg_from_op_spdif_decode_new_decoder_required_ptr, message_id) ((opmsg_from_op_spdif_decode_new_decoder_required_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DECODER_REQUIRED_NEW_DATA_TYPE_WORD_OFFSET (1)
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DECODER_REQUIRED_NEW_DATA_TYPE_GET(opmsg_from_op_spdif_decode_new_decoder_required_ptr) ((opmsg_from_op_spdif_decode_new_decoder_required_ptr)->_data[1])
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DECODER_REQUIRED_NEW_DATA_TYPE_SET(opmsg_from_op_spdif_decode_new_decoder_required_ptr, new_data_type) ((opmsg_from_op_spdif_decode_new_decoder_required_ptr)->_data[1] = (uint16)(new_data_type))
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DECODER_REQUIRED_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DECODER_REQUIRED_CREATE(message_id, new_data_type) \
    (uint16)(message_id), \
    (uint16)(new_data_type)
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DECODER_REQUIRED_PACK(opmsg_from_op_spdif_decode_new_decoder_required_ptr, message_id, new_data_type) \
    do { \
        (opmsg_from_op_spdif_decode_new_decoder_required_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_from_op_spdif_decode_new_decoder_required_ptr)->_data[1] = (uint16)((uint16)(new_data_type)); \
    } while (0)

#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DECODER_REQUIRED_MARSHALL(addr, opmsg_from_op_spdif_decode_new_decoder_required_ptr) memcpy((void *)(addr), (void *)(opmsg_from_op_spdif_decode_new_decoder_required_ptr), 2)
#define OPMSG_FROM_OP_SPDIF_DECODE_NEW_DECODER_REQUIRED_UNMARSHALL(addr, opmsg_from_op_spdif_decode_new_decoder_required_ptr) memcpy((void *)(opmsg_from_op_spdif_decode_new_decoder_required_ptr), (void *)(addr), 2)


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

#define OPMSG_IIR_RESAMPLER_SET_CONFIG_MARSHALL(addr, opmsg_iir_resampler_set_config_ptr) memcpy((void *)(addr), (void *)(opmsg_iir_resampler_set_config_ptr), 2)
#define OPMSG_IIR_RESAMPLER_SET_CONFIG_UNMARSHALL(addr, opmsg_iir_resampler_set_config_ptr) memcpy((void *)(opmsg_iir_resampler_set_config_ptr), (void *)(addr), 2)


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

#define OPMSG_IIR_RESAMPLER_SET_CONVERSION_RATE_MARSHALL(addr, opmsg_iir_resampler_set_conversion_rate_ptr) memcpy((void *)(addr), (void *)(opmsg_iir_resampler_set_conversion_rate_ptr), 2)
#define OPMSG_IIR_RESAMPLER_SET_CONVERSION_RATE_UNMARSHALL(addr, opmsg_iir_resampler_set_conversion_rate_ptr) memcpy((void *)(opmsg_iir_resampler_set_conversion_rate_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Interleave_Set_Data_Format

  DESCRIPTION
    Interleave operator message SET_DATA_FORMAT.

  MEMBERS
    message_id  - message id
    data_format - data format

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_INTERLEAVE_SET_DATA_FORMAT;

/* The following macros take OPMSG_INTERLEAVE_SET_DATA_FORMAT *opmsg_interleave_set_data_format_ptr */
#define OPMSG_INTERLEAVE_SET_DATA_FORMAT_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_INTERLEAVE_SET_DATA_FORMAT_MESSAGE_ID_GET(opmsg_interleave_set_data_format_ptr) ((OPMSG_INTERLEAVE_ID)(opmsg_interleave_set_data_format_ptr)->_data[0])
#define OPMSG_INTERLEAVE_SET_DATA_FORMAT_MESSAGE_ID_SET(opmsg_interleave_set_data_format_ptr, message_id) ((opmsg_interleave_set_data_format_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_INTERLEAVE_SET_DATA_FORMAT_DATA_FORMAT_WORD_OFFSET (1)
#define OPMSG_INTERLEAVE_SET_DATA_FORMAT_DATA_FORMAT_GET(opmsg_interleave_set_data_format_ptr) ((opmsg_interleave_set_data_format_ptr)->_data[1])
#define OPMSG_INTERLEAVE_SET_DATA_FORMAT_DATA_FORMAT_SET(opmsg_interleave_set_data_format_ptr, data_format) ((opmsg_interleave_set_data_format_ptr)->_data[1] = (uint16)(data_format))
#define OPMSG_INTERLEAVE_SET_DATA_FORMAT_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_INTERLEAVE_SET_DATA_FORMAT_CREATE(message_id, data_format) \
    (uint16)(message_id), \
    (uint16)(data_format)
#define OPMSG_INTERLEAVE_SET_DATA_FORMAT_PACK(opmsg_interleave_set_data_format_ptr, message_id, data_format) \
    do { \
        (opmsg_interleave_set_data_format_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_interleave_set_data_format_ptr)->_data[1] = (uint16)((uint16)(data_format)); \
    } while (0)

#define OPMSG_INTERLEAVE_SET_DATA_FORMAT_MARSHALL(addr, opmsg_interleave_set_data_format_ptr) memcpy((void *)(addr), (void *)(opmsg_interleave_set_data_format_ptr), 2)
#define OPMSG_INTERLEAVE_SET_DATA_FORMAT_UNMARSHALL(addr, opmsg_interleave_set_data_format_ptr) memcpy((void *)(opmsg_interleave_set_data_format_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Ksp_Set_Stream_Transforms

  DESCRIPTION
    KSP operator message for SET_STREAM_TRANSFORMS mesage ID.

  MEMBERS
    message_id   - message id
    stream_id    - Stream ID, operators can potentially support up to 4
                   concurrent streams, however current implementation might
                   support less than 4. Consult operator's documentation for the
                   limit on number of concurrent streams. A seperate message
                   must be sent for each stream.
    n_channels   - Number of channels present in this stream, each steam can
                   potentioally support up to 8 synchronised channels, however
                   current implementation might support less than 8 channels in
                   a stream. Consult operator's documentation for the limit on
                   number of channels per stream.
    data_type    - Data type of the stream. One of these values should be used:
                   type       value  transmits      suitable for
                   DATA16     0      Lower 16 bits  16-bit encoded streams
                   PCM16      1      Upper 16 bits  linear PCM streams
                   PCM24      2      Upper 24 bits  linear PCM streams
                   PCM32      3      Upper 32 bits  linear PCM streams
                   DATA32     4      Lower 32 bits  32-bit encoded streams
                   Reserved   5,6,7
    nr_samples   - Number of samples/channel that are read and packetised by the
                   operator. If set to 0 operator will choose a suitable number.
                   Maximum 256 samples.
    channel_info - If non-zero operator will transmit some channel specific
                   metadata in each packet. Current implementation might not
                   support it, set it to 0 in that case.
    transforms   - Array of transform ids associated to channels in the stream.
                   These are the connection IDs of the to-be sniffed transforms.

*******************************************************************************/
typedef struct
{
    uint16 _data[7];
} OPMSG_KSP_SET_STREAM_TRANSFORMS;

/* The following macros take OPMSG_KSP_SET_STREAM_TRANSFORMS *opmsg_ksp_set_stream_transforms_ptr */
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_MESSAGE_ID_GET(opmsg_ksp_set_stream_transforms_ptr) ((OPMSG_KSP_ID)(opmsg_ksp_set_stream_transforms_ptr)->_data[0])
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_MESSAGE_ID_SET(opmsg_ksp_set_stream_transforms_ptr, message_id) ((opmsg_ksp_set_stream_transforms_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_STREAM_ID_WORD_OFFSET (1)
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_STREAM_ID_GET(opmsg_ksp_set_stream_transforms_ptr) ((opmsg_ksp_set_stream_transforms_ptr)->_data[1])
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_STREAM_ID_SET(opmsg_ksp_set_stream_transforms_ptr, stream_id) ((opmsg_ksp_set_stream_transforms_ptr)->_data[1] = (uint16)(stream_id))
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_N_CHANNELS_WORD_OFFSET (2)
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_N_CHANNELS_GET(opmsg_ksp_set_stream_transforms_ptr) ((opmsg_ksp_set_stream_transforms_ptr)->_data[2])
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_N_CHANNELS_SET(opmsg_ksp_set_stream_transforms_ptr, n_channels) ((opmsg_ksp_set_stream_transforms_ptr)->_data[2] = (uint16)(n_channels))
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_DATA_TYPE_WORD_OFFSET (3)
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_DATA_TYPE_GET(opmsg_ksp_set_stream_transforms_ptr) ((opmsg_ksp_set_stream_transforms_ptr)->_data[3])
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_DATA_TYPE_SET(opmsg_ksp_set_stream_transforms_ptr, data_type) ((opmsg_ksp_set_stream_transforms_ptr)->_data[3] = (uint16)(data_type))
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_NR_SAMPLES_WORD_OFFSET (4)
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_NR_SAMPLES_GET(opmsg_ksp_set_stream_transforms_ptr) ((opmsg_ksp_set_stream_transforms_ptr)->_data[4])
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_NR_SAMPLES_SET(opmsg_ksp_set_stream_transforms_ptr, nr_samples) ((opmsg_ksp_set_stream_transforms_ptr)->_data[4] = (uint16)(nr_samples))
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_CHANNEL_INFO_WORD_OFFSET (5)
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_CHANNEL_INFO_GET(opmsg_ksp_set_stream_transforms_ptr) ((opmsg_ksp_set_stream_transforms_ptr)->_data[5])
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_CHANNEL_INFO_SET(opmsg_ksp_set_stream_transforms_ptr, channel_info) ((opmsg_ksp_set_stream_transforms_ptr)->_data[5] = (uint16)(channel_info))
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_TRANSFORMS_WORD_OFFSET (6)
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_TRANSFORMS_GET(opmsg_ksp_set_stream_transforms_ptr) ((opmsg_ksp_set_stream_transforms_ptr)->_data[6])
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_TRANSFORMS_SET(opmsg_ksp_set_stream_transforms_ptr, transforms) ((opmsg_ksp_set_stream_transforms_ptr)->_data[6] = (uint16)(transforms))
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_WORD_SIZE (7)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_CREATE(message_id, stream_id, n_channels, data_type, nr_samples, channel_info, transforms) \
    (uint16)(message_id), \
    (uint16)(stream_id), \
    (uint16)(n_channels), \
    (uint16)(data_type), \
    (uint16)(nr_samples), \
    (uint16)(channel_info), \
    (uint16)(transforms)
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_PACK(opmsg_ksp_set_stream_transforms_ptr, message_id, stream_id, n_channels, data_type, nr_samples, channel_info, transforms) \
    do { \
        (opmsg_ksp_set_stream_transforms_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_ksp_set_stream_transforms_ptr)->_data[1] = (uint16)((uint16)(stream_id)); \
        (opmsg_ksp_set_stream_transforms_ptr)->_data[2] = (uint16)((uint16)(n_channels)); \
        (opmsg_ksp_set_stream_transforms_ptr)->_data[3] = (uint16)((uint16)(data_type)); \
        (opmsg_ksp_set_stream_transforms_ptr)->_data[4] = (uint16)((uint16)(nr_samples)); \
        (opmsg_ksp_set_stream_transforms_ptr)->_data[5] = (uint16)((uint16)(channel_info)); \
        (opmsg_ksp_set_stream_transforms_ptr)->_data[6] = (uint16)((uint16)(transforms)); \
    } while (0)

#define OPMSG_KSP_SET_STREAM_TRANSFORMS_MARSHALL(addr, opmsg_ksp_set_stream_transforms_ptr) memcpy((void *)(addr), (void *)(opmsg_ksp_set_stream_transforms_ptr), 7)
#define OPMSG_KSP_SET_STREAM_TRANSFORMS_UNMARSHALL(addr, opmsg_ksp_set_stream_transforms_ptr) memcpy((void *)(opmsg_ksp_set_stream_transforms_ptr), (void *)(addr), 7)


/*******************************************************************************

  NAME
    Opmsg_Mixer_Set_Channel_Gains

  DESCRIPTION
    Mixer operator message for SET_CHANNEL_GAINS.

  MEMBERS
    message_id             - message id
    number_channels_change - Number of channels to change
    channel_num            - Channel number to change
    new_channel_gain       - new gain for channel number

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_MIXER_SET_CHANNEL_GAINS;

/* The following macros take OPMSG_MIXER_SET_CHANNEL_GAINS *opmsg_mixer_set_channel_gains_ptr */
#define OPMSG_MIXER_SET_CHANNEL_GAINS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_MIXER_SET_CHANNEL_GAINS_MESSAGE_ID_GET(opmsg_mixer_set_channel_gains_ptr) ((OPMSG_MIXER_ID)(opmsg_mixer_set_channel_gains_ptr)->_data[0])
#define OPMSG_MIXER_SET_CHANNEL_GAINS_MESSAGE_ID_SET(opmsg_mixer_set_channel_gains_ptr, message_id) ((opmsg_mixer_set_channel_gains_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_MIXER_SET_CHANNEL_GAINS_NUMBER_CHANNELS_CHANGE_WORD_OFFSET (1)
#define OPMSG_MIXER_SET_CHANNEL_GAINS_NUMBER_CHANNELS_CHANGE_GET(opmsg_mixer_set_channel_gains_ptr) ((opmsg_mixer_set_channel_gains_ptr)->_data[1])
#define OPMSG_MIXER_SET_CHANNEL_GAINS_NUMBER_CHANNELS_CHANGE_SET(opmsg_mixer_set_channel_gains_ptr, number_channels_change) ((opmsg_mixer_set_channel_gains_ptr)->_data[1] = (uint16)(number_channels_change))
#define OPMSG_MIXER_SET_CHANNEL_GAINS_CHANNEL_NUM_WORD_OFFSET (2)
#define OPMSG_MIXER_SET_CHANNEL_GAINS_CHANNEL_NUM_GET(opmsg_mixer_set_channel_gains_ptr) ((opmsg_mixer_set_channel_gains_ptr)->_data[2])
#define OPMSG_MIXER_SET_CHANNEL_GAINS_CHANNEL_NUM_SET(opmsg_mixer_set_channel_gains_ptr, channel_num) ((opmsg_mixer_set_channel_gains_ptr)->_data[2] = (uint16)(channel_num))
#define OPMSG_MIXER_SET_CHANNEL_GAINS_NEW_CHANNEL_GAIN_WORD_OFFSET (3)
#define OPMSG_MIXER_SET_CHANNEL_GAINS_NEW_CHANNEL_GAIN_GET(opmsg_mixer_set_channel_gains_ptr) ((opmsg_mixer_set_channel_gains_ptr)->_data[3])
#define OPMSG_MIXER_SET_CHANNEL_GAINS_NEW_CHANNEL_GAIN_SET(opmsg_mixer_set_channel_gains_ptr, new_channel_gain) ((opmsg_mixer_set_channel_gains_ptr)->_data[3] = (uint16)(new_channel_gain))
#define OPMSG_MIXER_SET_CHANNEL_GAINS_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_MIXER_SET_CHANNEL_GAINS_CREATE(message_id, number_channels_change, channel_num, new_channel_gain) \
    (uint16)(message_id), \
    (uint16)(number_channels_change), \
    (uint16)(channel_num), \
    (uint16)(new_channel_gain)
#define OPMSG_MIXER_SET_CHANNEL_GAINS_PACK(opmsg_mixer_set_channel_gains_ptr, message_id, number_channels_change, channel_num, new_channel_gain) \
    do { \
        (opmsg_mixer_set_channel_gains_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_mixer_set_channel_gains_ptr)->_data[1] = (uint16)((uint16)(number_channels_change)); \
        (opmsg_mixer_set_channel_gains_ptr)->_data[2] = (uint16)((uint16)(channel_num)); \
        (opmsg_mixer_set_channel_gains_ptr)->_data[3] = (uint16)((uint16)(new_channel_gain)); \
    } while (0)

#define OPMSG_MIXER_SET_CHANNEL_GAINS_MARSHALL(addr, opmsg_mixer_set_channel_gains_ptr) memcpy((void *)(addr), (void *)(opmsg_mixer_set_channel_gains_ptr), 4)
#define OPMSG_MIXER_SET_CHANNEL_GAINS_UNMARSHALL(addr, opmsg_mixer_set_channel_gains_ptr) memcpy((void *)(opmsg_mixer_set_channel_gains_ptr), (void *)(addr), 4)


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

#define OPMSG_MIXER_SET_METADATA_STREAM_MARSHALL(addr, opmsg_mixer_set_metadata_stream_ptr) memcpy((void *)(addr), (void *)(opmsg_mixer_set_metadata_stream_ptr), 3)
#define OPMSG_MIXER_SET_METADATA_STREAM_UNMARSHALL(addr, opmsg_mixer_set_metadata_stream_ptr) memcpy((void *)(opmsg_mixer_set_metadata_stream_ptr), (void *)(addr), 3)


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

#define OPMSG_MIXER_SET_PRIMARY_STREAM_MARSHALL(addr, opmsg_mixer_set_primary_stream_ptr) memcpy((void *)(addr), (void *)(opmsg_mixer_set_primary_stream_ptr), 2)
#define OPMSG_MIXER_SET_PRIMARY_STREAM_UNMARSHALL(addr, opmsg_mixer_set_primary_stream_ptr) memcpy((void *)(opmsg_mixer_set_primary_stream_ptr), (void *)(addr), 2)


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

#define OPMSG_MIXER_SET_RAMP_NUM_SAMPLES_MARSHALL(addr, opmsg_mixer_set_ramp_num_samples_ptr) memcpy((void *)(addr), (void *)(opmsg_mixer_set_ramp_num_samples_ptr), 3)
#define OPMSG_MIXER_SET_RAMP_NUM_SAMPLES_UNMARSHALL(addr, opmsg_mixer_set_ramp_num_samples_ptr) memcpy((void *)(opmsg_mixer_set_ramp_num_samples_ptr), (void *)(addr), 3)


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

#define OPMSG_MIXER_SET_STREAM_CHANNELS_MARSHALL(addr, opmsg_mixer_set_stream_channels_ptr) memcpy((void *)(addr), (void *)(opmsg_mixer_set_stream_channels_ptr), 4)
#define OPMSG_MIXER_SET_STREAM_CHANNELS_UNMARSHALL(addr, opmsg_mixer_set_stream_channels_ptr) memcpy((void *)(opmsg_mixer_set_stream_channels_ptr), (void *)(addr), 4)


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

#define OPMSG_MIXER_SET_STREAM_GAINS_MARSHALL(addr, opmsg_mixer_set_stream_gains_ptr) memcpy((void *)(addr), (void *)(opmsg_mixer_set_stream_gains_ptr), 4)
#define OPMSG_MIXER_SET_STREAM_GAINS_UNMARSHALL(addr, opmsg_mixer_set_stream_gains_ptr) memcpy((void *)(opmsg_mixer_set_stream_gains_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    Opmsg_Msg_Download_Direct_Access_Test_File_Close

  DESCRIPTION
    Download direct access test operator message for File Close.

  MEMBERS
    message_id  - message id
    file_handle - File handle received from File_Open

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_CLOSE;

/* The following macros take OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_CLOSE *opmsg_msg_download_direct_access_test_file_close_ptr */
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_CLOSE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_CLOSE_MESSAGE_ID_GET(opmsg_msg_download_direct_access_test_file_close_ptr) ((OPMSG_DOWNLOAD_DIRECT_ACCESS_TEST_ID)(opmsg_msg_download_direct_access_test_file_close_ptr)->_data[0])
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_CLOSE_MESSAGE_ID_SET(opmsg_msg_download_direct_access_test_file_close_ptr, message_id) ((opmsg_msg_download_direct_access_test_file_close_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_CLOSE_FILE_HANDLE_WORD_OFFSET (1)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_CLOSE_FILE_HANDLE_GET(opmsg_msg_download_direct_access_test_file_close_ptr)  \
    (((uint32)((opmsg_msg_download_direct_access_test_file_close_ptr)->_data[1]) | \
      ((uint32)((opmsg_msg_download_direct_access_test_file_close_ptr)->_data[2]) << 16)))
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_CLOSE_FILE_HANDLE_SET(opmsg_msg_download_direct_access_test_file_close_ptr, file_handle) do { \
        (opmsg_msg_download_direct_access_test_file_close_ptr)->_data[1] = (uint16)((file_handle) & 0xffff); \
        (opmsg_msg_download_direct_access_test_file_close_ptr)->_data[2] = (uint16)((file_handle) >> 16); } while (0)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_CLOSE_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_CLOSE_CREATE(message_id, file_handle) \
    (uint16)(message_id), \
    (uint16)((file_handle) & 0xffff), \
    (uint16)((file_handle) >> 16)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_CLOSE_PACK(opmsg_msg_download_direct_access_test_file_close_ptr, message_id, file_handle) \
    do { \
        (opmsg_msg_download_direct_access_test_file_close_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_msg_download_direct_access_test_file_close_ptr)->_data[1] = (uint16)((uint16)((file_handle) & 0xffff)); \
        (opmsg_msg_download_direct_access_test_file_close_ptr)->_data[2] = (uint16)(((file_handle) >> 16)); \
    } while (0)

#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_CLOSE_MARSHALL(addr, opmsg_msg_download_direct_access_test_file_close_ptr) memcpy((void *)(addr), (void *)(opmsg_msg_download_direct_access_test_file_close_ptr), 3)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_CLOSE_UNMARSHALL(addr, opmsg_msg_download_direct_access_test_file_close_ptr) memcpy((void *)(opmsg_msg_download_direct_access_test_file_close_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    Opmsg_Msg_Download_Direct_Access_Test_File_Open

  DESCRIPTION
    Download direct access test operator message for File Open.

  MEMBERS
    message_id      - message id
    filename_length - in octets including the Null termination; if
                      length(filename) including the Null termination is odd
                      there will be an extra Null character in the "filename"
                      not reported in this field
    filename        - Null terminated string packed as pairs of little endian
                      uint16s

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_OPEN;

/* The following macros take OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_OPEN *opmsg_msg_download_direct_access_test_file_open_ptr */
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_OPEN_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_OPEN_MESSAGE_ID_GET(opmsg_msg_download_direct_access_test_file_open_ptr) ((OPMSG_DOWNLOAD_DIRECT_ACCESS_TEST_ID)(opmsg_msg_download_direct_access_test_file_open_ptr)->_data[0])
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_OPEN_MESSAGE_ID_SET(opmsg_msg_download_direct_access_test_file_open_ptr, message_id) ((opmsg_msg_download_direct_access_test_file_open_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_OPEN_FILENAME_LENGTH_WORD_OFFSET (1)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_OPEN_FILENAME_LENGTH_GET(opmsg_msg_download_direct_access_test_file_open_ptr) ((opmsg_msg_download_direct_access_test_file_open_ptr)->_data[1])
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_OPEN_FILENAME_LENGTH_SET(opmsg_msg_download_direct_access_test_file_open_ptr, filename_length) ((opmsg_msg_download_direct_access_test_file_open_ptr)->_data[1] = (uint16)(filename_length))
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_OPEN_FILENAME_WORD_OFFSET (2)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_OPEN_FILENAME_GET(opmsg_msg_download_direct_access_test_file_open_ptr) ((opmsg_msg_download_direct_access_test_file_open_ptr)->_data[2])
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_OPEN_FILENAME_SET(opmsg_msg_download_direct_access_test_file_open_ptr, filename) ((opmsg_msg_download_direct_access_test_file_open_ptr)->_data[2] = (uint16)(filename))
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_OPEN_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_OPEN_CREATE(message_id, filename_length, filename) \
    (uint16)(message_id), \
    (uint16)(filename_length), \
    (uint16)(filename)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_OPEN_PACK(opmsg_msg_download_direct_access_test_file_open_ptr, message_id, filename_length, filename) \
    do { \
        (opmsg_msg_download_direct_access_test_file_open_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_msg_download_direct_access_test_file_open_ptr)->_data[1] = (uint16)((uint16)(filename_length)); \
        (opmsg_msg_download_direct_access_test_file_open_ptr)->_data[2] = (uint16)((uint16)(filename)); \
    } while (0)

#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_OPEN_MARSHALL(addr, opmsg_msg_download_direct_access_test_file_open_ptr) memcpy((void *)(addr), (void *)(opmsg_msg_download_direct_access_test_file_open_ptr), 3)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_OPEN_UNMARSHALL(addr, opmsg_msg_download_direct_access_test_file_open_ptr) memcpy((void *)(opmsg_msg_download_direct_access_test_file_open_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    Opmsg_Msg_Download_Direct_Access_Test_File_Read

  DESCRIPTION
    Download direct access test operator message for File Read.

  MEMBERS
    message_id  - message id
    file_handle - File handle received from File_Open
    amount      - Amount of data to read from file [octets]
    offset      - Offset within the file [octets]

*******************************************************************************/
typedef struct
{
    uint16 _data[7];
} OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ;

/* The following macros take OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ *opmsg_msg_download_direct_access_test_file_read_ptr */
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_MESSAGE_ID_GET(opmsg_msg_download_direct_access_test_file_read_ptr) ((OPMSG_DOWNLOAD_DIRECT_ACCESS_TEST_ID)(opmsg_msg_download_direct_access_test_file_read_ptr)->_data[0])
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_MESSAGE_ID_SET(opmsg_msg_download_direct_access_test_file_read_ptr, message_id) ((opmsg_msg_download_direct_access_test_file_read_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_FILE_HANDLE_WORD_OFFSET (1)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_FILE_HANDLE_GET(opmsg_msg_download_direct_access_test_file_read_ptr)  \
    (((uint32)((opmsg_msg_download_direct_access_test_file_read_ptr)->_data[1]) | \
      ((uint32)((opmsg_msg_download_direct_access_test_file_read_ptr)->_data[2]) << 16)))
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_FILE_HANDLE_SET(opmsg_msg_download_direct_access_test_file_read_ptr, file_handle) do { \
        (opmsg_msg_download_direct_access_test_file_read_ptr)->_data[1] = (uint16)((file_handle) & 0xffff); \
        (opmsg_msg_download_direct_access_test_file_read_ptr)->_data[2] = (uint16)((file_handle) >> 16); } while (0)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_AMOUNT_WORD_OFFSET (3)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_AMOUNT_GET(opmsg_msg_download_direct_access_test_file_read_ptr)  \
    (((uint32)((opmsg_msg_download_direct_access_test_file_read_ptr)->_data[3]) | \
      ((uint32)((opmsg_msg_download_direct_access_test_file_read_ptr)->_data[4]) << 16)))
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_AMOUNT_SET(opmsg_msg_download_direct_access_test_file_read_ptr, amount) do { \
        (opmsg_msg_download_direct_access_test_file_read_ptr)->_data[3] = (uint16)((amount) & 0xffff); \
        (opmsg_msg_download_direct_access_test_file_read_ptr)->_data[4] = (uint16)((amount) >> 16); } while (0)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_OFFSET_WORD_OFFSET (5)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_OFFSET_GET(opmsg_msg_download_direct_access_test_file_read_ptr)  \
    (((uint32)((opmsg_msg_download_direct_access_test_file_read_ptr)->_data[5]) | \
      ((uint32)((opmsg_msg_download_direct_access_test_file_read_ptr)->_data[6]) << 16)))
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_OFFSET_SET(opmsg_msg_download_direct_access_test_file_read_ptr, offset) do { \
        (opmsg_msg_download_direct_access_test_file_read_ptr)->_data[5] = (uint16)((offset) & 0xffff); \
        (opmsg_msg_download_direct_access_test_file_read_ptr)->_data[6] = (uint16)((offset) >> 16); } while (0)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_WORD_SIZE (7)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_CREATE(message_id, file_handle, amount, offset) \
    (uint16)(message_id), \
    (uint16)((file_handle) & 0xffff), \
    (uint16)((file_handle) >> 16), \
    (uint16)((amount) & 0xffff), \
    (uint16)((amount) >> 16), \
    (uint16)((offset) & 0xffff), \
    (uint16)((offset) >> 16)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_PACK(opmsg_msg_download_direct_access_test_file_read_ptr, message_id, file_handle, amount, offset) \
    do { \
        (opmsg_msg_download_direct_access_test_file_read_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_msg_download_direct_access_test_file_read_ptr)->_data[1] = (uint16)((uint16)((file_handle) & 0xffff)); \
        (opmsg_msg_download_direct_access_test_file_read_ptr)->_data[2] = (uint16)(((file_handle) >> 16)); \
        (opmsg_msg_download_direct_access_test_file_read_ptr)->_data[3] = (uint16)((uint16)((amount) & 0xffff)); \
        (opmsg_msg_download_direct_access_test_file_read_ptr)->_data[4] = (uint16)(((amount) >> 16)); \
        (opmsg_msg_download_direct_access_test_file_read_ptr)->_data[5] = (uint16)((uint16)((offset) & 0xffff)); \
        (opmsg_msg_download_direct_access_test_file_read_ptr)->_data[6] = (uint16)(((offset) >> 16)); \
    } while (0)

#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_MARSHALL(addr, opmsg_msg_download_direct_access_test_file_read_ptr) memcpy((void *)(addr), (void *)(opmsg_msg_download_direct_access_test_file_read_ptr), 7)
#define OPMSG_MSG_DOWNLOAD_DIRECT_ACCESS_TEST_FILE_READ_UNMARSHALL(addr, opmsg_msg_download_direct_access_test_file_read_ptr) memcpy((void *)(opmsg_msg_download_direct_access_test_file_read_ptr), (void *)(addr), 7)


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

#define OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES_MARSHALL(addr, opmsg_msg_iir_resampler_set_sample_rates_ptr) memcpy((void *)(addr), (void *)(opmsg_msg_iir_resampler_set_sample_rates_ptr), 3)
#define OPMSG_MSG_IIR_RESAMPLER_SET_SAMPLE_RATES_UNMARSHALL(addr, opmsg_msg_iir_resampler_set_sample_rates_ptr) memcpy((void *)(opmsg_msg_iir_resampler_set_sample_rates_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    Opmsg_Op_Client_AOV_LP

  DESCRIPTION
    Notification from AOV that we are entering low power mode.

  MEMBERS
    message_id - message id
    active     - Activation status: 1 enabled, 0 disabled

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_OP_CLIENT_AOV_LP;

/* The following macros take OPMSG_OP_CLIENT_AOV_LP *opmsg_op_client_aov_lp_ptr */
#define OPMSG_OP_CLIENT_AOV_LP_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_OP_CLIENT_AOV_LP_MESSAGE_ID_GET(opmsg_op_client_aov_lp_ptr) ((OPMSG_OP_CLIENT_REPLY_ID)(opmsg_op_client_aov_lp_ptr)->_data[0])
#define OPMSG_OP_CLIENT_AOV_LP_MESSAGE_ID_SET(opmsg_op_client_aov_lp_ptr, message_id) ((opmsg_op_client_aov_lp_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_OP_CLIENT_AOV_LP_ACTIVE_WORD_OFFSET (1)
#define OPMSG_OP_CLIENT_AOV_LP_ACTIVE_GET(opmsg_op_client_aov_lp_ptr) ((opmsg_op_client_aov_lp_ptr)->_data[1])
#define OPMSG_OP_CLIENT_AOV_LP_ACTIVE_SET(opmsg_op_client_aov_lp_ptr, active) ((opmsg_op_client_aov_lp_ptr)->_data[1] = (uint16)(active))
#define OPMSG_OP_CLIENT_AOV_LP_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_OP_CLIENT_AOV_LP_CREATE(message_id, active) \
    (uint16)(message_id), \
    (uint16)(active)
#define OPMSG_OP_CLIENT_AOV_LP_PACK(opmsg_op_client_aov_lp_ptr, message_id, active) \
    do { \
        (opmsg_op_client_aov_lp_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_op_client_aov_lp_ptr)->_data[1] = (uint16)((uint16)(active)); \
    } while (0)

#define OPMSG_OP_CLIENT_AOV_LP_MARSHALL(addr, opmsg_op_client_aov_lp_ptr) memcpy((void *)(addr), (void *)(opmsg_op_client_aov_lp_ptr), 2)
#define OPMSG_OP_CLIENT_AOV_LP_UNMARSHALL(addr, opmsg_op_client_aov_lp_ptr) memcpy((void *)(opmsg_op_client_aov_lp_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Op_Client_AOV_Response

  DESCRIPTION
    Response from AOV to Graph Manager request.

  MEMBERS
    message_id - message id
    status     - Response status

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_OP_CLIENT_AOV_RESPONSE;

/* The following macros take OPMSG_OP_CLIENT_AOV_RESPONSE *opmsg_op_client_aov_response_ptr */
#define OPMSG_OP_CLIENT_AOV_RESPONSE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_OP_CLIENT_AOV_RESPONSE_MESSAGE_ID_GET(opmsg_op_client_aov_response_ptr) ((OPMSG_OP_CLIENT_REPLY_ID)(opmsg_op_client_aov_response_ptr)->_data[0])
#define OPMSG_OP_CLIENT_AOV_RESPONSE_MESSAGE_ID_SET(opmsg_op_client_aov_response_ptr, message_id) ((opmsg_op_client_aov_response_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_OP_CLIENT_AOV_RESPONSE_STATUS_WORD_OFFSET (1)
#define OPMSG_OP_CLIENT_AOV_RESPONSE_STATUS_GET(opmsg_op_client_aov_response_ptr) ((opmsg_op_client_aov_response_ptr)->_data[1])
#define OPMSG_OP_CLIENT_AOV_RESPONSE_STATUS_SET(opmsg_op_client_aov_response_ptr, status) ((opmsg_op_client_aov_response_ptr)->_data[1] = (uint16)(status))
#define OPMSG_OP_CLIENT_AOV_RESPONSE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_OP_CLIENT_AOV_RESPONSE_CREATE(message_id, status) \
    (uint16)(message_id), \
    (uint16)(status)
#define OPMSG_OP_CLIENT_AOV_RESPONSE_PACK(opmsg_op_client_aov_response_ptr, message_id, status) \
    do { \
        (opmsg_op_client_aov_response_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_op_client_aov_response_ptr)->_data[1] = (uint16)((uint16)(status)); \
    } while (0)

#define OPMSG_OP_CLIENT_AOV_RESPONSE_MARSHALL(addr, opmsg_op_client_aov_response_ptr) memcpy((void *)(addr), (void *)(opmsg_op_client_aov_response_ptr), 2)
#define OPMSG_OP_CLIENT_AOV_RESPONSE_UNMARSHALL(addr, opmsg_op_client_aov_response_ptr) memcpy((void *)(opmsg_op_client_aov_response_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Op_Client_Command_Response

  DESCRIPTION
    Response to an operator message sent by the operator client.

  MEMBERS
    message_id - message id
    count      - number of operators handled successfully until error or success
    status     - response status
    err_code   - Error code

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_OP_CLIENT_COMMAND_RESPONSE;

/* The following macros take OPMSG_OP_CLIENT_COMMAND_RESPONSE *opmsg_op_client_command_response_ptr */
#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_MESSAGE_ID_GET(opmsg_op_client_command_response_ptr) ((OPMSG_OP_CLIENT_REPLY_ID)(opmsg_op_client_command_response_ptr)->_data[0])
#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_MESSAGE_ID_SET(opmsg_op_client_command_response_ptr, message_id) ((opmsg_op_client_command_response_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_COUNT_WORD_OFFSET (1)
#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_COUNT_GET(opmsg_op_client_command_response_ptr) ((opmsg_op_client_command_response_ptr)->_data[1])
#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_COUNT_SET(opmsg_op_client_command_response_ptr, count) ((opmsg_op_client_command_response_ptr)->_data[1] = (uint16)(count))
#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_STATUS_WORD_OFFSET (2)
#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_STATUS_GET(opmsg_op_client_command_response_ptr) ((opmsg_op_client_command_response_ptr)->_data[2])
#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_STATUS_SET(opmsg_op_client_command_response_ptr, status) ((opmsg_op_client_command_response_ptr)->_data[2] = (uint16)(status))
#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_ERR_CODE_WORD_OFFSET (3)
#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_ERR_CODE_GET(opmsg_op_client_command_response_ptr) ((opmsg_op_client_command_response_ptr)->_data[3])
#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_ERR_CODE_SET(opmsg_op_client_command_response_ptr, err_code) ((opmsg_op_client_command_response_ptr)->_data[3] = (uint16)(err_code))
#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_CREATE(message_id, count, status, err_code) \
    (uint16)(message_id), \
    (uint16)(count), \
    (uint16)(status), \
    (uint16)(err_code)
#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_PACK(opmsg_op_client_command_response_ptr, message_id, count, status, err_code) \
    do { \
        (opmsg_op_client_command_response_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_op_client_command_response_ptr)->_data[1] = (uint16)((uint16)(count)); \
        (opmsg_op_client_command_response_ptr)->_data[2] = (uint16)((uint16)(status)); \
        (opmsg_op_client_command_response_ptr)->_data[3] = (uint16)((uint16)(err_code)); \
    } while (0)

#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_MARSHALL(addr, opmsg_op_client_command_response_ptr) memcpy((void *)(addr), (void *)(opmsg_op_client_command_response_ptr), 4)
#define OPMSG_OP_CLIENT_COMMAND_RESPONSE_UNMARSHALL(addr, opmsg_op_client_command_response_ptr) memcpy((void *)(opmsg_op_client_command_response_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    Opmsg_Op_Client_Delegated_Operators

  DESCRIPTION
    Unsolicited message forwarded to the operator client.

  MEMBERS
    message_id - message id
    operators  - operators[] array

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_OP_CLIENT_DELEGATED_OPERATORS;

/* The following macros take OPMSG_OP_CLIENT_DELEGATED_OPERATORS *opmsg_op_client_delegated_operators_ptr */
#define OPMSG_OP_CLIENT_DELEGATED_OPERATORS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_OP_CLIENT_DELEGATED_OPERATORS_MESSAGE_ID_GET(opmsg_op_client_delegated_operators_ptr) ((OPMSG_OP_CLIENT_REPLY_ID)(opmsg_op_client_delegated_operators_ptr)->_data[0])
#define OPMSG_OP_CLIENT_DELEGATED_OPERATORS_MESSAGE_ID_SET(opmsg_op_client_delegated_operators_ptr, message_id) ((opmsg_op_client_delegated_operators_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_OP_CLIENT_DELEGATED_OPERATORS_OPERATORS_WORD_OFFSET (1)
#define OPMSG_OP_CLIENT_DELEGATED_OPERATORS_OPERATORS_GET(opmsg_op_client_delegated_operators_ptr) ((opmsg_op_client_delegated_operators_ptr)->_data[1])
#define OPMSG_OP_CLIENT_DELEGATED_OPERATORS_OPERATORS_SET(opmsg_op_client_delegated_operators_ptr, operators) ((opmsg_op_client_delegated_operators_ptr)->_data[1] = (uint16)(operators))
#define OPMSG_OP_CLIENT_DELEGATED_OPERATORS_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_OP_CLIENT_DELEGATED_OPERATORS_CREATE(message_id, operators) \
    (uint16)(message_id), \
    (uint16)(operators)
#define OPMSG_OP_CLIENT_DELEGATED_OPERATORS_PACK(opmsg_op_client_delegated_operators_ptr, message_id, operators) \
    do { \
        (opmsg_op_client_delegated_operators_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_op_client_delegated_operators_ptr)->_data[1] = (uint16)((uint16)(operators)); \
    } while (0)

#define OPMSG_OP_CLIENT_DELEGATED_OPERATORS_MARSHALL(addr, opmsg_op_client_delegated_operators_ptr) memcpy((void *)(addr), (void *)(opmsg_op_client_delegated_operators_ptr), 2)
#define OPMSG_OP_CLIENT_DELEGATED_OPERATORS_UNMARSHALL(addr, opmsg_op_client_delegated_operators_ptr) memcpy((void *)(opmsg_op_client_delegated_operators_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Op_Client_Message_Response

  DESCRIPTION
    Response to an operator message sent by the operator client.

  MEMBERS
    message_id   - message id
    source_op_id - External op ID of the source operator
    status       - response status
    payload      - payload[] array

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_OP_CLIENT_MESSAGE_RESPONSE;

/* The following macros take OPMSG_OP_CLIENT_MESSAGE_RESPONSE *opmsg_op_client_message_response_ptr */
#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_MESSAGE_ID_GET(opmsg_op_client_message_response_ptr) ((OPMSG_OP_CLIENT_REPLY_ID)(opmsg_op_client_message_response_ptr)->_data[0])
#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_MESSAGE_ID_SET(opmsg_op_client_message_response_ptr, message_id) ((opmsg_op_client_message_response_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_SOURCE_OP_ID_WORD_OFFSET (1)
#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_SOURCE_OP_ID_GET(opmsg_op_client_message_response_ptr) ((opmsg_op_client_message_response_ptr)->_data[1])
#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_SOURCE_OP_ID_SET(opmsg_op_client_message_response_ptr, source_op_id) ((opmsg_op_client_message_response_ptr)->_data[1] = (uint16)(source_op_id))
#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_STATUS_WORD_OFFSET (2)
#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_STATUS_GET(opmsg_op_client_message_response_ptr) ((opmsg_op_client_message_response_ptr)->_data[2])
#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_STATUS_SET(opmsg_op_client_message_response_ptr, status) ((opmsg_op_client_message_response_ptr)->_data[2] = (uint16)(status))
#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_PAYLOAD_WORD_OFFSET (3)
#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_PAYLOAD_GET(opmsg_op_client_message_response_ptr) ((opmsg_op_client_message_response_ptr)->_data[3])
#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_PAYLOAD_SET(opmsg_op_client_message_response_ptr, payload) ((opmsg_op_client_message_response_ptr)->_data[3] = (uint16)(payload))
#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_CREATE(message_id, source_op_id, status, payload) \
    (uint16)(message_id), \
    (uint16)(source_op_id), \
    (uint16)(status), \
    (uint16)(payload)
#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_PACK(opmsg_op_client_message_response_ptr, message_id, source_op_id, status, payload) \
    do { \
        (opmsg_op_client_message_response_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_op_client_message_response_ptr)->_data[1] = (uint16)((uint16)(source_op_id)); \
        (opmsg_op_client_message_response_ptr)->_data[2] = (uint16)((uint16)(status)); \
        (opmsg_op_client_message_response_ptr)->_data[3] = (uint16)((uint16)(payload)); \
    } while (0)

#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_MARSHALL(addr, opmsg_op_client_message_response_ptr) memcpy((void *)(addr), (void *)(opmsg_op_client_message_response_ptr), 4)
#define OPMSG_OP_CLIENT_MESSAGE_RESPONSE_UNMARSHALL(addr, opmsg_op_client_message_response_ptr) memcpy((void *)(opmsg_op_client_message_response_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    Opmsg_Op_Client_Unsolicited_Message

  DESCRIPTION
    Unsolicited message forwarded to the operator client.

  MEMBERS
    message_id   - message id
    source_op_id - External op ID of the source operator
    payload      - payload[] array

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE;

/* The following macros take OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE *opmsg_op_client_unsolicited_message_ptr */
#define OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE_MESSAGE_ID_GET(opmsg_op_client_unsolicited_message_ptr) ((OPMSG_OP_CLIENT_REPLY_ID)(opmsg_op_client_unsolicited_message_ptr)->_data[0])
#define OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE_MESSAGE_ID_SET(opmsg_op_client_unsolicited_message_ptr, message_id) ((opmsg_op_client_unsolicited_message_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE_SOURCE_OP_ID_WORD_OFFSET (1)
#define OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE_SOURCE_OP_ID_GET(opmsg_op_client_unsolicited_message_ptr) ((opmsg_op_client_unsolicited_message_ptr)->_data[1])
#define OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE_SOURCE_OP_ID_SET(opmsg_op_client_unsolicited_message_ptr, source_op_id) ((opmsg_op_client_unsolicited_message_ptr)->_data[1] = (uint16)(source_op_id))
#define OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE_PAYLOAD_WORD_OFFSET (2)
#define OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE_PAYLOAD_GET(opmsg_op_client_unsolicited_message_ptr) ((opmsg_op_client_unsolicited_message_ptr)->_data[2])
#define OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE_PAYLOAD_SET(opmsg_op_client_unsolicited_message_ptr, payload) ((opmsg_op_client_unsolicited_message_ptr)->_data[2] = (uint16)(payload))
#define OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE_CREATE(message_id, source_op_id, payload) \
    (uint16)(message_id), \
    (uint16)(source_op_id), \
    (uint16)(payload)
#define OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE_PACK(opmsg_op_client_unsolicited_message_ptr, message_id, source_op_id, payload) \
    do { \
        (opmsg_op_client_unsolicited_message_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_op_client_unsolicited_message_ptr)->_data[1] = (uint16)((uint16)(source_op_id)); \
        (opmsg_op_client_unsolicited_message_ptr)->_data[2] = (uint16)((uint16)(payload)); \
    } while (0)

#define OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE_MARSHALL(addr, opmsg_op_client_unsolicited_message_ptr) memcpy((void *)(addr), (void *)(opmsg_op_client_unsolicited_message_ptr), 3)
#define OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE_UNMARSHALL(addr, opmsg_op_client_unsolicited_message_ptr) memcpy((void *)(opmsg_op_client_unsolicited_message_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    Opmsg_Op_Client_VA_Negative_Trigger

  DESCRIPTION
    Positive trigger from a VA operator forwarded to the operator client.

  MEMBERS
    message_id   - message id
    source_op_id - External op ID of the source operator

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_OP_CLIENT_VA_NEGATIVE_TRIGGER;

/* The following macros take OPMSG_OP_CLIENT_VA_NEGATIVE_TRIGGER *opmsg_op_client_va_negative_trigger_ptr */
#define OPMSG_OP_CLIENT_VA_NEGATIVE_TRIGGER_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_OP_CLIENT_VA_NEGATIVE_TRIGGER_MESSAGE_ID_GET(opmsg_op_client_va_negative_trigger_ptr) ((OPMSG_OP_CLIENT_REPLY_ID)(opmsg_op_client_va_negative_trigger_ptr)->_data[0])
#define OPMSG_OP_CLIENT_VA_NEGATIVE_TRIGGER_MESSAGE_ID_SET(opmsg_op_client_va_negative_trigger_ptr, message_id) ((opmsg_op_client_va_negative_trigger_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_OP_CLIENT_VA_NEGATIVE_TRIGGER_SOURCE_OP_ID_WORD_OFFSET (1)
#define OPMSG_OP_CLIENT_VA_NEGATIVE_TRIGGER_SOURCE_OP_ID_GET(opmsg_op_client_va_negative_trigger_ptr) ((opmsg_op_client_va_negative_trigger_ptr)->_data[1])
#define OPMSG_OP_CLIENT_VA_NEGATIVE_TRIGGER_SOURCE_OP_ID_SET(opmsg_op_client_va_negative_trigger_ptr, source_op_id) ((opmsg_op_client_va_negative_trigger_ptr)->_data[1] = (uint16)(source_op_id))
#define OPMSG_OP_CLIENT_VA_NEGATIVE_TRIGGER_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_OP_CLIENT_VA_NEGATIVE_TRIGGER_CREATE(message_id, source_op_id) \
    (uint16)(message_id), \
    (uint16)(source_op_id)
#define OPMSG_OP_CLIENT_VA_NEGATIVE_TRIGGER_PACK(opmsg_op_client_va_negative_trigger_ptr, message_id, source_op_id) \
    do { \
        (opmsg_op_client_va_negative_trigger_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_op_client_va_negative_trigger_ptr)->_data[1] = (uint16)((uint16)(source_op_id)); \
    } while (0)

#define OPMSG_OP_CLIENT_VA_NEGATIVE_TRIGGER_MARSHALL(addr, opmsg_op_client_va_negative_trigger_ptr) memcpy((void *)(addr), (void *)(opmsg_op_client_va_negative_trigger_ptr), 2)
#define OPMSG_OP_CLIENT_VA_NEGATIVE_TRIGGER_UNMARSHALL(addr, opmsg_op_client_va_negative_trigger_ptr) memcpy((void *)(opmsg_op_client_va_negative_trigger_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Op_Client_VA_Trigger

  DESCRIPTION
    Positive trigger from a VA operator forwarded to the operator client.

  MEMBERS
    message_id   - message id
    source_op_id - External op ID of the source operator

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_OP_CLIENT_VA_TRIGGER;

/* The following macros take OPMSG_OP_CLIENT_VA_TRIGGER *opmsg_op_client_va_trigger_ptr */
#define OPMSG_OP_CLIENT_VA_TRIGGER_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_OP_CLIENT_VA_TRIGGER_MESSAGE_ID_GET(opmsg_op_client_va_trigger_ptr) ((OPMSG_OP_CLIENT_REPLY_ID)(opmsg_op_client_va_trigger_ptr)->_data[0])
#define OPMSG_OP_CLIENT_VA_TRIGGER_MESSAGE_ID_SET(opmsg_op_client_va_trigger_ptr, message_id) ((opmsg_op_client_va_trigger_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_OP_CLIENT_VA_TRIGGER_SOURCE_OP_ID_WORD_OFFSET (1)
#define OPMSG_OP_CLIENT_VA_TRIGGER_SOURCE_OP_ID_GET(opmsg_op_client_va_trigger_ptr) ((opmsg_op_client_va_trigger_ptr)->_data[1])
#define OPMSG_OP_CLIENT_VA_TRIGGER_SOURCE_OP_ID_SET(opmsg_op_client_va_trigger_ptr, source_op_id) ((opmsg_op_client_va_trigger_ptr)->_data[1] = (uint16)(source_op_id))
#define OPMSG_OP_CLIENT_VA_TRIGGER_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_OP_CLIENT_VA_TRIGGER_CREATE(message_id, source_op_id) \
    (uint16)(message_id), \
    (uint16)(source_op_id)
#define OPMSG_OP_CLIENT_VA_TRIGGER_PACK(opmsg_op_client_va_trigger_ptr, message_id, source_op_id) \
    do { \
        (opmsg_op_client_va_trigger_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_op_client_va_trigger_ptr)->_data[1] = (uint16)((uint16)(source_op_id)); \
    } while (0)

#define OPMSG_OP_CLIENT_VA_TRIGGER_MARSHALL(addr, opmsg_op_client_va_trigger_ptr) memcpy((void *)(addr), (void *)(opmsg_op_client_va_trigger_ptr), 2)
#define OPMSG_OP_CLIENT_VA_TRIGGER_UNMARSHALL(addr, opmsg_op_client_va_trigger_ptr) memcpy((void *)(opmsg_op_client_va_trigger_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Op_Unsolicited_Message

  DESCRIPTION
    Unsolicited message from operator.

  MEMBERS
    op_id      - operator id
    client_id  - client id
    message_id - message id
    length     - length of payload array
    payload    - payload[] array

*******************************************************************************/
typedef struct
{
    uint16 _data[5];
} OPMSG_OP_UNSOLICITED_MESSAGE;

/* The following macros take OPMSG_OP_UNSOLICITED_MESSAGE *opmsg_op_unsolicited_message_ptr */
#define OPMSG_OP_UNSOLICITED_MESSAGE_OP_ID_WORD_OFFSET (0)
#define OPMSG_OP_UNSOLICITED_MESSAGE_OP_ID_GET(opmsg_op_unsolicited_message_ptr) ((opmsg_op_unsolicited_message_ptr)->_data[0])
#define OPMSG_OP_UNSOLICITED_MESSAGE_OP_ID_SET(opmsg_op_unsolicited_message_ptr, op_id) ((opmsg_op_unsolicited_message_ptr)->_data[0] = (uint16)(op_id))
#define OPMSG_OP_UNSOLICITED_MESSAGE_CLIENT_ID_WORD_OFFSET (1)
#define OPMSG_OP_UNSOLICITED_MESSAGE_CLIENT_ID_GET(opmsg_op_unsolicited_message_ptr) ((opmsg_op_unsolicited_message_ptr)->_data[1])
#define OPMSG_OP_UNSOLICITED_MESSAGE_CLIENT_ID_SET(opmsg_op_unsolicited_message_ptr, client_id) ((opmsg_op_unsolicited_message_ptr)->_data[1] = (uint16)(client_id))
#define OPMSG_OP_UNSOLICITED_MESSAGE_MESSAGE_ID_WORD_OFFSET (2)
#define OPMSG_OP_UNSOLICITED_MESSAGE_MESSAGE_ID_GET(opmsg_op_unsolicited_message_ptr) ((opmsg_op_unsolicited_message_ptr)->_data[2])
#define OPMSG_OP_UNSOLICITED_MESSAGE_MESSAGE_ID_SET(opmsg_op_unsolicited_message_ptr, message_id) ((opmsg_op_unsolicited_message_ptr)->_data[2] = (uint16)(message_id))
#define OPMSG_OP_UNSOLICITED_MESSAGE_LENGTH_WORD_OFFSET (3)
#define OPMSG_OP_UNSOLICITED_MESSAGE_LENGTH_GET(opmsg_op_unsolicited_message_ptr) ((opmsg_op_unsolicited_message_ptr)->_data[3])
#define OPMSG_OP_UNSOLICITED_MESSAGE_LENGTH_SET(opmsg_op_unsolicited_message_ptr, length) ((opmsg_op_unsolicited_message_ptr)->_data[3] = (uint16)(length))
#define OPMSG_OP_UNSOLICITED_MESSAGE_PAYLOAD_WORD_OFFSET (4)
#define OPMSG_OP_UNSOLICITED_MESSAGE_PAYLOAD_GET(opmsg_op_unsolicited_message_ptr) ((opmsg_op_unsolicited_message_ptr)->_data[4])
#define OPMSG_OP_UNSOLICITED_MESSAGE_PAYLOAD_SET(opmsg_op_unsolicited_message_ptr, payload) ((opmsg_op_unsolicited_message_ptr)->_data[4] = (uint16)(payload))
#define OPMSG_OP_UNSOLICITED_MESSAGE_WORD_SIZE (5)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_OP_UNSOLICITED_MESSAGE_CREATE(op_id, client_id, message_id, length, payload) \
    (uint16)(op_id), \
    (uint16)(client_id), \
    (uint16)(message_id), \
    (uint16)(length), \
    (uint16)(payload)
#define OPMSG_OP_UNSOLICITED_MESSAGE_PACK(opmsg_op_unsolicited_message_ptr, op_id, client_id, message_id, length, payload) \
    do { \
        (opmsg_op_unsolicited_message_ptr)->_data[0] = (uint16)((uint16)(op_id)); \
        (opmsg_op_unsolicited_message_ptr)->_data[1] = (uint16)((uint16)(client_id)); \
        (opmsg_op_unsolicited_message_ptr)->_data[2] = (uint16)((uint16)(message_id)); \
        (opmsg_op_unsolicited_message_ptr)->_data[3] = (uint16)((uint16)(length)); \
        (opmsg_op_unsolicited_message_ptr)->_data[4] = (uint16)((uint16)(payload)); \
    } while (0)

#define OPMSG_OP_UNSOLICITED_MESSAGE_MARSHALL(addr, opmsg_op_unsolicited_message_ptr) memcpy((void *)(addr), (void *)(opmsg_op_unsolicited_message_ptr), 5)
#define OPMSG_OP_UNSOLICITED_MESSAGE_UNMARSHALL(addr, opmsg_op_unsolicited_message_ptr) memcpy((void *)(opmsg_op_unsolicited_message_ptr), (void *)(addr), 5)


/*******************************************************************************

  NAME
    Opmsg_Passthrough_Change_Input_Data_Type

  DESCRIPTION
    Basic Passthrough operator message for CHANGE_INPUT_DATA_TYPE.

  MEMBERS
    message_id - message id
    data_type  - data type

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_PASSTHROUGH_CHANGE_INPUT_DATA_TYPE;

/* The following macros take OPMSG_PASSTHROUGH_CHANGE_INPUT_DATA_TYPE *opmsg_passthrough_change_input_data_type_ptr */
#define OPMSG_PASSTHROUGH_CHANGE_INPUT_DATA_TYPE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_PASSTHROUGH_CHANGE_INPUT_DATA_TYPE_MESSAGE_ID_GET(opmsg_passthrough_change_input_data_type_ptr) ((OPMSG_PASSTHROUGH_ID)(opmsg_passthrough_change_input_data_type_ptr)->_data[0])
#define OPMSG_PASSTHROUGH_CHANGE_INPUT_DATA_TYPE_MESSAGE_ID_SET(opmsg_passthrough_change_input_data_type_ptr, message_id) ((opmsg_passthrough_change_input_data_type_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_PASSTHROUGH_CHANGE_INPUT_DATA_TYPE_DATA_TYPE_WORD_OFFSET (1)
#define OPMSG_PASSTHROUGH_CHANGE_INPUT_DATA_TYPE_DATA_TYPE_GET(opmsg_passthrough_change_input_data_type_ptr) ((opmsg_passthrough_change_input_data_type_ptr)->_data[1])
#define OPMSG_PASSTHROUGH_CHANGE_INPUT_DATA_TYPE_DATA_TYPE_SET(opmsg_passthrough_change_input_data_type_ptr, data_type) ((opmsg_passthrough_change_input_data_type_ptr)->_data[1] = (uint16)(data_type))
#define OPMSG_PASSTHROUGH_CHANGE_INPUT_DATA_TYPE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_PASSTHROUGH_CHANGE_INPUT_DATA_TYPE_CREATE(message_id, data_type) \
    (uint16)(message_id), \
    (uint16)(data_type)
#define OPMSG_PASSTHROUGH_CHANGE_INPUT_DATA_TYPE_PACK(opmsg_passthrough_change_input_data_type_ptr, message_id, data_type) \
    do { \
        (opmsg_passthrough_change_input_data_type_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_passthrough_change_input_data_type_ptr)->_data[1] = (uint16)((uint16)(data_type)); \
    } while (0)

#define OPMSG_PASSTHROUGH_CHANGE_INPUT_DATA_TYPE_MARSHALL(addr, opmsg_passthrough_change_input_data_type_ptr) memcpy((void *)(addr), (void *)(opmsg_passthrough_change_input_data_type_ptr), 2)
#define OPMSG_PASSTHROUGH_CHANGE_INPUT_DATA_TYPE_UNMARSHALL(addr, opmsg_passthrough_change_input_data_type_ptr) memcpy((void *)(opmsg_passthrough_change_input_data_type_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Passthrough_Change_Output_Data_Type

  DESCRIPTION
    Basic Passthrough operator message for CHANGE_OUTPUT_DATA_TYPE.

  MEMBERS
    message_id - message id
    data_type  - data type

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_PASSTHROUGH_CHANGE_OUTPUT_DATA_TYPE;

/* The following macros take OPMSG_PASSTHROUGH_CHANGE_OUTPUT_DATA_TYPE *opmsg_passthrough_change_output_data_type_ptr */
#define OPMSG_PASSTHROUGH_CHANGE_OUTPUT_DATA_TYPE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_PASSTHROUGH_CHANGE_OUTPUT_DATA_TYPE_MESSAGE_ID_GET(opmsg_passthrough_change_output_data_type_ptr) ((OPMSG_PASSTHROUGH_ID)(opmsg_passthrough_change_output_data_type_ptr)->_data[0])
#define OPMSG_PASSTHROUGH_CHANGE_OUTPUT_DATA_TYPE_MESSAGE_ID_SET(opmsg_passthrough_change_output_data_type_ptr, message_id) ((opmsg_passthrough_change_output_data_type_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_PASSTHROUGH_CHANGE_OUTPUT_DATA_TYPE_DATA_TYPE_WORD_OFFSET (1)
#define OPMSG_PASSTHROUGH_CHANGE_OUTPUT_DATA_TYPE_DATA_TYPE_GET(opmsg_passthrough_change_output_data_type_ptr) ((opmsg_passthrough_change_output_data_type_ptr)->_data[1])
#define OPMSG_PASSTHROUGH_CHANGE_OUTPUT_DATA_TYPE_DATA_TYPE_SET(opmsg_passthrough_change_output_data_type_ptr, data_type) ((opmsg_passthrough_change_output_data_type_ptr)->_data[1] = (uint16)(data_type))
#define OPMSG_PASSTHROUGH_CHANGE_OUTPUT_DATA_TYPE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_PASSTHROUGH_CHANGE_OUTPUT_DATA_TYPE_CREATE(message_id, data_type) \
    (uint16)(message_id), \
    (uint16)(data_type)
#define OPMSG_PASSTHROUGH_CHANGE_OUTPUT_DATA_TYPE_PACK(opmsg_passthrough_change_output_data_type_ptr, message_id, data_type) \
    do { \
        (opmsg_passthrough_change_output_data_type_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_passthrough_change_output_data_type_ptr)->_data[1] = (uint16)((uint16)(data_type)); \
    } while (0)

#define OPMSG_PASSTHROUGH_CHANGE_OUTPUT_DATA_TYPE_MARSHALL(addr, opmsg_passthrough_change_output_data_type_ptr) memcpy((void *)(addr), (void *)(opmsg_passthrough_change_output_data_type_ptr), 2)
#define OPMSG_PASSTHROUGH_CHANGE_OUTPUT_DATA_TYPE_UNMARSHALL(addr, opmsg_passthrough_change_output_data_type_ptr) memcpy((void *)(opmsg_passthrough_change_output_data_type_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Peq_Override_Coeffs

  DESCRIPTION
    PEQ operator message OVERRIDE_COEFFS.

  MEMBERS
    message_id        - message id
    num_stages        - number of stages
    gain_exponent_msb - MSB of gain_exponent
    gain_exponent_lsb - LSB of gain_exponent
    gain_mantisa_msb  - MSB of gain_mantisa
    gain_mantisa_lsb  - LSB of gain_mantisa
    payload           - coeffs_scales

*******************************************************************************/
typedef struct
{
    uint16 _data[7];
} OPMSG_PEQ_OVERRIDE_COEFFS;

/* The following macros take OPMSG_PEQ_OVERRIDE_COEFFS *opmsg_peq_override_coeffs_ptr */
#define OPMSG_PEQ_OVERRIDE_COEFFS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_PEQ_OVERRIDE_COEFFS_MESSAGE_ID_GET(opmsg_peq_override_coeffs_ptr) ((OPMSG_PEQ_ID)(opmsg_peq_override_coeffs_ptr)->_data[0])
#define OPMSG_PEQ_OVERRIDE_COEFFS_MESSAGE_ID_SET(opmsg_peq_override_coeffs_ptr, message_id) ((opmsg_peq_override_coeffs_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_PEQ_OVERRIDE_COEFFS_NUM_STAGES_WORD_OFFSET (1)
#define OPMSG_PEQ_OVERRIDE_COEFFS_NUM_STAGES_GET(opmsg_peq_override_coeffs_ptr) ((opmsg_peq_override_coeffs_ptr)->_data[1])
#define OPMSG_PEQ_OVERRIDE_COEFFS_NUM_STAGES_SET(opmsg_peq_override_coeffs_ptr, num_stages) ((opmsg_peq_override_coeffs_ptr)->_data[1] = (uint16)(num_stages))
#define OPMSG_PEQ_OVERRIDE_COEFFS_GAIN_EXPONENT_MSB_WORD_OFFSET (2)
#define OPMSG_PEQ_OVERRIDE_COEFFS_GAIN_EXPONENT_MSB_GET(opmsg_peq_override_coeffs_ptr) ((opmsg_peq_override_coeffs_ptr)->_data[2])
#define OPMSG_PEQ_OVERRIDE_COEFFS_GAIN_EXPONENT_MSB_SET(opmsg_peq_override_coeffs_ptr, gain_exponent_msb) ((opmsg_peq_override_coeffs_ptr)->_data[2] = (uint16)(gain_exponent_msb))
#define OPMSG_PEQ_OVERRIDE_COEFFS_GAIN_EXPONENT_LSB_WORD_OFFSET (3)
#define OPMSG_PEQ_OVERRIDE_COEFFS_GAIN_EXPONENT_LSB_GET(opmsg_peq_override_coeffs_ptr) ((opmsg_peq_override_coeffs_ptr)->_data[3])
#define OPMSG_PEQ_OVERRIDE_COEFFS_GAIN_EXPONENT_LSB_SET(opmsg_peq_override_coeffs_ptr, gain_exponent_lsb) ((opmsg_peq_override_coeffs_ptr)->_data[3] = (uint16)(gain_exponent_lsb))
#define OPMSG_PEQ_OVERRIDE_COEFFS_GAIN_MANTISA_MSB_WORD_OFFSET (4)
#define OPMSG_PEQ_OVERRIDE_COEFFS_GAIN_MANTISA_MSB_GET(opmsg_peq_override_coeffs_ptr) ((opmsg_peq_override_coeffs_ptr)->_data[4])
#define OPMSG_PEQ_OVERRIDE_COEFFS_GAIN_MANTISA_MSB_SET(opmsg_peq_override_coeffs_ptr, gain_mantisa_msb) ((opmsg_peq_override_coeffs_ptr)->_data[4] = (uint16)(gain_mantisa_msb))
#define OPMSG_PEQ_OVERRIDE_COEFFS_GAIN_MANTISA_LSB_WORD_OFFSET (5)
#define OPMSG_PEQ_OVERRIDE_COEFFS_GAIN_MANTISA_LSB_GET(opmsg_peq_override_coeffs_ptr) ((opmsg_peq_override_coeffs_ptr)->_data[5])
#define OPMSG_PEQ_OVERRIDE_COEFFS_GAIN_MANTISA_LSB_SET(opmsg_peq_override_coeffs_ptr, gain_mantisa_lsb) ((opmsg_peq_override_coeffs_ptr)->_data[5] = (uint16)(gain_mantisa_lsb))
#define OPMSG_PEQ_OVERRIDE_COEFFS_PAYLOAD_WORD_OFFSET (6)
#define OPMSG_PEQ_OVERRIDE_COEFFS_PAYLOAD_GET(opmsg_peq_override_coeffs_ptr) ((opmsg_peq_override_coeffs_ptr)->_data[6])
#define OPMSG_PEQ_OVERRIDE_COEFFS_PAYLOAD_SET(opmsg_peq_override_coeffs_ptr, payload) ((opmsg_peq_override_coeffs_ptr)->_data[6] = (uint16)(payload))
#define OPMSG_PEQ_OVERRIDE_COEFFS_WORD_SIZE (7)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_PEQ_OVERRIDE_COEFFS_CREATE(message_id, num_stages, gain_exponent_msb, gain_exponent_lsb, gain_mantisa_msb, gain_mantisa_lsb, payload) \
    (uint16)(message_id), \
    (uint16)(num_stages), \
    (uint16)(gain_exponent_msb), \
    (uint16)(gain_exponent_lsb), \
    (uint16)(gain_mantisa_msb), \
    (uint16)(gain_mantisa_lsb), \
    (uint16)(payload)
#define OPMSG_PEQ_OVERRIDE_COEFFS_PACK(opmsg_peq_override_coeffs_ptr, message_id, num_stages, gain_exponent_msb, gain_exponent_lsb, gain_mantisa_msb, gain_mantisa_lsb, payload) \
    do { \
        (opmsg_peq_override_coeffs_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_peq_override_coeffs_ptr)->_data[1] = (uint16)((uint16)(num_stages)); \
        (opmsg_peq_override_coeffs_ptr)->_data[2] = (uint16)((uint16)(gain_exponent_msb)); \
        (opmsg_peq_override_coeffs_ptr)->_data[3] = (uint16)((uint16)(gain_exponent_lsb)); \
        (opmsg_peq_override_coeffs_ptr)->_data[4] = (uint16)((uint16)(gain_mantisa_msb)); \
        (opmsg_peq_override_coeffs_ptr)->_data[5] = (uint16)((uint16)(gain_mantisa_lsb)); \
        (opmsg_peq_override_coeffs_ptr)->_data[6] = (uint16)((uint16)(payload)); \
    } while (0)

#define OPMSG_PEQ_OVERRIDE_COEFFS_MARSHALL(addr, opmsg_peq_override_coeffs_ptr) memcpy((void *)(addr), (void *)(opmsg_peq_override_coeffs_ptr), 7)
#define OPMSG_PEQ_OVERRIDE_COEFFS_UNMARSHALL(addr, opmsg_peq_override_coeffs_ptr) memcpy((void *)(opmsg_peq_override_coeffs_ptr), (void *)(addr), 7)


/*******************************************************************************

  NAME
    Opmsg_QVA_Reset_Status

  DESCRIPTION
    Reset the QVA status

  MEMBERS
    message_id       - message id
    qva_reset_status -

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_QVA_RESET_STATUS;

/* The following macros take OPMSG_QVA_RESET_STATUS *opmsg_qva_reset_status_ptr */
#define OPMSG_QVA_RESET_STATUS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_QVA_RESET_STATUS_MESSAGE_ID_GET(opmsg_qva_reset_status_ptr) ((OPMSG_QVA_ID)(opmsg_qva_reset_status_ptr)->_data[0])
#define OPMSG_QVA_RESET_STATUS_MESSAGE_ID_SET(opmsg_qva_reset_status_ptr, message_id) ((opmsg_qva_reset_status_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_QVA_RESET_STATUS_QVA_RESET_STATUS_WORD_OFFSET (1)
#define OPMSG_QVA_RESET_STATUS_QVA_RESET_STATUS_GET(opmsg_qva_reset_status_ptr) ((opmsg_qva_reset_status_ptr)->_data[1])
#define OPMSG_QVA_RESET_STATUS_QVA_RESET_STATUS_SET(opmsg_qva_reset_status_ptr, qva_reset_status) ((opmsg_qva_reset_status_ptr)->_data[1] = (uint16)(qva_reset_status))
#define OPMSG_QVA_RESET_STATUS_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_QVA_RESET_STATUS_CREATE(message_id, qva_reset_status) \
    (uint16)(message_id), \
    (uint16)(qva_reset_status)
#define OPMSG_QVA_RESET_STATUS_PACK(opmsg_qva_reset_status_ptr, message_id, qva_reset_status) \
    do { \
        (opmsg_qva_reset_status_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_qva_reset_status_ptr)->_data[1] = (uint16)((uint16)(qva_reset_status)); \
    } while (0)

#define OPMSG_QVA_RESET_STATUS_MARSHALL(addr, opmsg_qva_reset_status_ptr) memcpy((void *)(addr), (void *)(opmsg_qva_reset_status_ptr), 2)
#define OPMSG_QVA_RESET_STATUS_UNMARSHALL(addr, opmsg_qva_reset_status_ptr) memcpy((void *)(opmsg_qva_reset_status_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_QVA_Trigger_Channel_Info

  DESCRIPTION
    Trigger details per each channel.

  MEMBERS
    trigger_confidence    - confidence score for trigger
    start_timestamp_upper - upper half of the trigger start timestamp
    start_timestamp_lower - lower half of the trigger start timestamp
    end_timestamp_upper   - upper half of the trigger end timestamp
    end_timestamp_lower   - lower half of the trigger end timestamp

*******************************************************************************/
typedef struct
{
    uint16 _data[5];
} OPMSG_QVA_TRIGGER_CHANNEL_INFO;

/* The following macros take OPMSG_QVA_TRIGGER_CHANNEL_INFO *opmsg_qva_trigger_channel_info_ptr */
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_TRIGGER_CONFIDENCE_WORD_OFFSET (0)
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_TRIGGER_CONFIDENCE_GET(opmsg_qva_trigger_channel_info_ptr) ((opmsg_qva_trigger_channel_info_ptr)->_data[0])
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_TRIGGER_CONFIDENCE_SET(opmsg_qva_trigger_channel_info_ptr, trigger_confidence) ((opmsg_qva_trigger_channel_info_ptr)->_data[0] = (uint16)(trigger_confidence))
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_START_TIMESTAMP_UPPER_WORD_OFFSET (1)
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_START_TIMESTAMP_UPPER_GET(opmsg_qva_trigger_channel_info_ptr) ((opmsg_qva_trigger_channel_info_ptr)->_data[1])
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_START_TIMESTAMP_UPPER_SET(opmsg_qva_trigger_channel_info_ptr, start_timestamp_upper) ((opmsg_qva_trigger_channel_info_ptr)->_data[1] = (uint16)(start_timestamp_upper))
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_START_TIMESTAMP_LOWER_WORD_OFFSET (2)
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_START_TIMESTAMP_LOWER_GET(opmsg_qva_trigger_channel_info_ptr) ((opmsg_qva_trigger_channel_info_ptr)->_data[2])
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_START_TIMESTAMP_LOWER_SET(opmsg_qva_trigger_channel_info_ptr, start_timestamp_lower) ((opmsg_qva_trigger_channel_info_ptr)->_data[2] = (uint16)(start_timestamp_lower))
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_END_TIMESTAMP_UPPER_WORD_OFFSET (3)
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_END_TIMESTAMP_UPPER_GET(opmsg_qva_trigger_channel_info_ptr) ((opmsg_qva_trigger_channel_info_ptr)->_data[3])
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_END_TIMESTAMP_UPPER_SET(opmsg_qva_trigger_channel_info_ptr, end_timestamp_upper) ((opmsg_qva_trigger_channel_info_ptr)->_data[3] = (uint16)(end_timestamp_upper))
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_END_TIMESTAMP_LOWER_WORD_OFFSET (4)
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_END_TIMESTAMP_LOWER_GET(opmsg_qva_trigger_channel_info_ptr) ((opmsg_qva_trigger_channel_info_ptr)->_data[4])
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_END_TIMESTAMP_LOWER_SET(opmsg_qva_trigger_channel_info_ptr, end_timestamp_lower) ((opmsg_qva_trigger_channel_info_ptr)->_data[4] = (uint16)(end_timestamp_lower))
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_WORD_SIZE (5)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_CREATE(trigger_confidence, start_timestamp_upper, start_timestamp_lower, end_timestamp_upper, end_timestamp_lower) \
    (uint16)(trigger_confidence), \
    (uint16)(start_timestamp_upper), \
    (uint16)(start_timestamp_lower), \
    (uint16)(end_timestamp_upper), \
    (uint16)(end_timestamp_lower)
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_PACK(opmsg_qva_trigger_channel_info_ptr, trigger_confidence, start_timestamp_upper, start_timestamp_lower, end_timestamp_upper, end_timestamp_lower) \
    do { \
        (opmsg_qva_trigger_channel_info_ptr)->_data[0] = (uint16)((uint16)(trigger_confidence)); \
        (opmsg_qva_trigger_channel_info_ptr)->_data[1] = (uint16)((uint16)(start_timestamp_upper)); \
        (opmsg_qva_trigger_channel_info_ptr)->_data[2] = (uint16)((uint16)(start_timestamp_lower)); \
        (opmsg_qva_trigger_channel_info_ptr)->_data[3] = (uint16)((uint16)(end_timestamp_upper)); \
        (opmsg_qva_trigger_channel_info_ptr)->_data[4] = (uint16)((uint16)(end_timestamp_lower)); \
    } while (0)

#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_MARSHALL(addr, opmsg_qva_trigger_channel_info_ptr) memcpy((void *)(addr), (void *)(opmsg_qva_trigger_channel_info_ptr), 5)
#define OPMSG_QVA_TRIGGER_CHANNEL_INFO_UNMARSHALL(addr, opmsg_qva_trigger_channel_info_ptr) memcpy((void *)(opmsg_qva_trigger_channel_info_ptr), (void *)(addr), 5)


/*******************************************************************************

  NAME
    Opmsg_Reply_Latency_Changed

  DESCRIPTION
    Rtp Unsolicited operator message for Latency Change Notification.

  MEMBERS
    source_id      - source id
    target_latency - source id

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_REPLY_LATENCY_CHANGED;

/* The following macros take OPMSG_REPLY_LATENCY_CHANGED *opmsg_reply_latency_changed_ptr */
#define OPMSG_REPLY_LATENCY_CHANGED_SOURCE_ID_WORD_OFFSET (0)
#define OPMSG_REPLY_LATENCY_CHANGED_SOURCE_ID_GET(opmsg_reply_latency_changed_ptr)  \
    (((uint32)((opmsg_reply_latency_changed_ptr)->_data[0]) | \
      ((uint32)((opmsg_reply_latency_changed_ptr)->_data[1]) << 16)))
#define OPMSG_REPLY_LATENCY_CHANGED_SOURCE_ID_SET(opmsg_reply_latency_changed_ptr, source_id) do { \
        (opmsg_reply_latency_changed_ptr)->_data[0] = (uint16)((source_id) & 0xffff); \
        (opmsg_reply_latency_changed_ptr)->_data[1] = (uint16)((source_id) >> 16); } while (0)
#define OPMSG_REPLY_LATENCY_CHANGED_TARGET_LATENCY_WORD_OFFSET (2)
#define OPMSG_REPLY_LATENCY_CHANGED_TARGET_LATENCY_GET(opmsg_reply_latency_changed_ptr)  \
    (((uint32)((opmsg_reply_latency_changed_ptr)->_data[2]) | \
      ((uint32)((opmsg_reply_latency_changed_ptr)->_data[3]) << 16)))
#define OPMSG_REPLY_LATENCY_CHANGED_TARGET_LATENCY_SET(opmsg_reply_latency_changed_ptr, target_latency) do { \
        (opmsg_reply_latency_changed_ptr)->_data[2] = (uint16)((target_latency) & 0xffff); \
        (opmsg_reply_latency_changed_ptr)->_data[3] = (uint16)((target_latency) >> 16); } while (0)
#define OPMSG_REPLY_LATENCY_CHANGED_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_REPLY_LATENCY_CHANGED_CREATE(source_id, target_latency) \
    (uint16)((source_id) & 0xffff), \
    (uint16)((source_id) >> 16), \
    (uint16)((target_latency) & 0xffff), \
    (uint16)((target_latency) >> 16)
#define OPMSG_REPLY_LATENCY_CHANGED_PACK(opmsg_reply_latency_changed_ptr, source_id, target_latency) \
    do { \
        (opmsg_reply_latency_changed_ptr)->_data[0] = (uint16)((uint16)((source_id) & 0xffff)); \
        (opmsg_reply_latency_changed_ptr)->_data[1] = (uint16)(((source_id) >> 16)); \
        (opmsg_reply_latency_changed_ptr)->_data[2] = (uint16)((uint16)((target_latency) & 0xffff)); \
        (opmsg_reply_latency_changed_ptr)->_data[3] = (uint16)(((target_latency) >> 16)); \
    } while (0)

#define OPMSG_REPLY_LATENCY_CHANGED_MARSHALL(addr, opmsg_reply_latency_changed_ptr) memcpy((void *)(addr), (void *)(opmsg_reply_latency_changed_ptr), 4)
#define OPMSG_REPLY_LATENCY_CHANGED_UNMARSHALL(addr, opmsg_reply_latency_changed_ptr) memcpy((void *)(opmsg_reply_latency_changed_ptr), (void *)(addr), 4)


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

#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_MARSHALL(addr, opmsg_resampler_filter_coefficients_ptr) memcpy((void *)(addr), (void *)(opmsg_resampler_filter_coefficients_ptr), 4)
#define OPMSG_RESAMPLER_FILTER_COEFFICIENTS_UNMARSHALL(addr, opmsg_resampler_filter_coefficients_ptr) memcpy((void *)(opmsg_resampler_filter_coefficients_ptr), (void *)(addr), 4)


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

#define OPMSG_RESAMPLER_SET_CONVERSION_RATE_MARSHALL(addr, opmsg_resampler_set_conversion_rate_ptr) memcpy((void *)(addr), (void *)(opmsg_resampler_set_conversion_rate_ptr), 2)
#define OPMSG_RESAMPLER_SET_CONVERSION_RATE_UNMARSHALL(addr, opmsg_resampler_set_conversion_rate_ptr) memcpy((void *)(opmsg_resampler_set_conversion_rate_ptr), (void *)(addr), 2)


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

#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_MARSHALL(addr, opmsg_resampler_set_custom_rate_ptr) memcpy((void *)(addr), (void *)(opmsg_resampler_set_custom_rate_ptr), 6)
#define OPMSG_RESAMPLER_SET_CUSTOM_RATE_UNMARSHALL(addr, opmsg_resampler_set_custom_rate_ptr) memcpy((void *)(opmsg_resampler_set_custom_rate_ptr), (void *)(addr), 6)


/*******************************************************************************

  NAME
    Opmsg_Ringtone_Generator_Tone_Param

  DESCRIPTION
    Ringtone Generator message for defining tones.

  MEMBERS
    message_id      - message id
    tone_parameters - tone parameters in sequences

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_RINGTONE_GENERATOR_TONE_PARAM;

/* The following macros take OPMSG_RINGTONE_GENERATOR_TONE_PARAM *opmsg_ringtone_generator_tone_param_ptr */
#define OPMSG_RINGTONE_GENERATOR_TONE_PARAM_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_RINGTONE_GENERATOR_TONE_PARAM_MESSAGE_ID_GET(opmsg_ringtone_generator_tone_param_ptr) ((OPMSG_RINGTONE_ID)(opmsg_ringtone_generator_tone_param_ptr)->_data[0])
#define OPMSG_RINGTONE_GENERATOR_TONE_PARAM_MESSAGE_ID_SET(opmsg_ringtone_generator_tone_param_ptr, message_id) ((opmsg_ringtone_generator_tone_param_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_RINGTONE_GENERATOR_TONE_PARAM_TONE_PARAMETERS_WORD_OFFSET (1)
#define OPMSG_RINGTONE_GENERATOR_TONE_PARAM_TONE_PARAMETERS_GET(opmsg_ringtone_generator_tone_param_ptr) ((opmsg_ringtone_generator_tone_param_ptr)->_data[1])
#define OPMSG_RINGTONE_GENERATOR_TONE_PARAM_TONE_PARAMETERS_SET(opmsg_ringtone_generator_tone_param_ptr, tone_parameters) ((opmsg_ringtone_generator_tone_param_ptr)->_data[1] = (uint16)(tone_parameters))
#define OPMSG_RINGTONE_GENERATOR_TONE_PARAM_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_RINGTONE_GENERATOR_TONE_PARAM_CREATE(message_id, tone_parameters) \
    (uint16)(message_id), \
    (uint16)(tone_parameters)
#define OPMSG_RINGTONE_GENERATOR_TONE_PARAM_PACK(opmsg_ringtone_generator_tone_param_ptr, message_id, tone_parameters) \
    do { \
        (opmsg_ringtone_generator_tone_param_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_ringtone_generator_tone_param_ptr)->_data[1] = (uint16)((uint16)(tone_parameters)); \
    } while (0)

#define OPMSG_RINGTONE_GENERATOR_TONE_PARAM_MARSHALL(addr, opmsg_ringtone_generator_tone_param_ptr) memcpy((void *)(addr), (void *)(opmsg_ringtone_generator_tone_param_ptr), 2)
#define OPMSG_RINGTONE_GENERATOR_TONE_PARAM_UNMARSHALL(addr, opmsg_ringtone_generator_tone_param_ptr) memcpy((void *)(opmsg_ringtone_generator_tone_param_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Rtp_Set_AAC_Codec

  DESCRIPTION
    Rtp operator message for setting the AAC codec.

  MEMBERS
    message_id  - message id
    operator_id - The operator ID of the AAC decoder.

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_RTP_SET_AAC_CODEC;

/* The following macros take OPMSG_RTP_SET_AAC_CODEC *opmsg_rtp_set_aac_codec_ptr */
#define OPMSG_RTP_SET_AAC_CODEC_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_RTP_SET_AAC_CODEC_MESSAGE_ID_GET(opmsg_rtp_set_aac_codec_ptr) ((OPMSG_RTP_DECODE_ID)(opmsg_rtp_set_aac_codec_ptr)->_data[0])
#define OPMSG_RTP_SET_AAC_CODEC_MESSAGE_ID_SET(opmsg_rtp_set_aac_codec_ptr, message_id) ((opmsg_rtp_set_aac_codec_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_RTP_SET_AAC_CODEC_OPERATOR_ID_WORD_OFFSET (1)
#define OPMSG_RTP_SET_AAC_CODEC_OPERATOR_ID_GET(opmsg_rtp_set_aac_codec_ptr) ((opmsg_rtp_set_aac_codec_ptr)->_data[1])
#define OPMSG_RTP_SET_AAC_CODEC_OPERATOR_ID_SET(opmsg_rtp_set_aac_codec_ptr, operator_id) ((opmsg_rtp_set_aac_codec_ptr)->_data[1] = (uint16)(operator_id))
#define OPMSG_RTP_SET_AAC_CODEC_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_RTP_SET_AAC_CODEC_CREATE(message_id, operator_id) \
    (uint16)(message_id), \
    (uint16)(operator_id)
#define OPMSG_RTP_SET_AAC_CODEC_PACK(opmsg_rtp_set_aac_codec_ptr, message_id, operator_id) \
    do { \
        (opmsg_rtp_set_aac_codec_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_rtp_set_aac_codec_ptr)->_data[1] = (uint16)((uint16)(operator_id)); \
    } while (0)

#define OPMSG_RTP_SET_AAC_CODEC_MARSHALL(addr, opmsg_rtp_set_aac_codec_ptr) memcpy((void *)(addr), (void *)(opmsg_rtp_set_aac_codec_ptr), 2)
#define OPMSG_RTP_SET_AAC_CODEC_UNMARSHALL(addr, opmsg_rtp_set_aac_codec_ptr) memcpy((void *)(opmsg_rtp_set_aac_codec_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Rtp_Set_AAC_Utility

  DESCRIPTION
    Rtp operator message for setting the AAC samples in packet utility.

  MEMBERS
    message_id  - message id
    utility_sel - Utility selector (AAC_LC_OVER_LATM = 0, GENERIC = 1).

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_RTP_SET_AAC_UTILITY;

/* The following macros take OPMSG_RTP_SET_AAC_UTILITY *opmsg_rtp_set_aac_utility_ptr */
#define OPMSG_RTP_SET_AAC_UTILITY_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_RTP_SET_AAC_UTILITY_MESSAGE_ID_GET(opmsg_rtp_set_aac_utility_ptr) ((OPMSG_RTP_DECODE_ID)(opmsg_rtp_set_aac_utility_ptr)->_data[0])
#define OPMSG_RTP_SET_AAC_UTILITY_MESSAGE_ID_SET(opmsg_rtp_set_aac_utility_ptr, message_id) ((opmsg_rtp_set_aac_utility_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_RTP_SET_AAC_UTILITY_UTILITY_SEL_WORD_OFFSET (1)
#define OPMSG_RTP_SET_AAC_UTILITY_UTILITY_SEL_GET(opmsg_rtp_set_aac_utility_ptr) ((opmsg_rtp_set_aac_utility_ptr)->_data[1])
#define OPMSG_RTP_SET_AAC_UTILITY_UTILITY_SEL_SET(opmsg_rtp_set_aac_utility_ptr, utility_sel) ((opmsg_rtp_set_aac_utility_ptr)->_data[1] = (uint16)(utility_sel))
#define OPMSG_RTP_SET_AAC_UTILITY_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_RTP_SET_AAC_UTILITY_CREATE(message_id, utility_sel) \
    (uint16)(message_id), \
    (uint16)(utility_sel)
#define OPMSG_RTP_SET_AAC_UTILITY_PACK(opmsg_rtp_set_aac_utility_ptr, message_id, utility_sel) \
    do { \
        (opmsg_rtp_set_aac_utility_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_rtp_set_aac_utility_ptr)->_data[1] = (uint16)((uint16)(utility_sel)); \
    } while (0)

#define OPMSG_RTP_SET_AAC_UTILITY_MARSHALL(addr, opmsg_rtp_set_aac_utility_ptr) memcpy((void *)(addr), (void *)(opmsg_rtp_set_aac_utility_ptr), 2)
#define OPMSG_RTP_SET_AAC_UTILITY_UNMARSHALL(addr, opmsg_rtp_set_aac_utility_ptr) memcpy((void *)(opmsg_rtp_set_aac_utility_ptr), (void *)(addr), 2)


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

#define OPMSG_RTP_SET_CODEC_TYPE_MARSHALL(addr, opmsg_rtp_set_codec_type_ptr) memcpy((void *)(addr), (void *)(opmsg_rtp_set_codec_type_ptr), 2)
#define OPMSG_RTP_SET_CODEC_TYPE_UNMARSHALL(addr, opmsg_rtp_set_codec_type_ptr) memcpy((void *)(opmsg_rtp_set_codec_type_ptr), (void *)(addr), 2)


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

#define OPMSG_RTP_SET_CONTENT_PROTECTION_MARSHALL(addr, opmsg_rtp_set_content_protection_ptr) memcpy((void *)(addr), (void *)(opmsg_rtp_set_content_protection_ptr), 2)
#define OPMSG_RTP_SET_CONTENT_PROTECTION_UNMARSHALL(addr, opmsg_rtp_set_content_protection_ptr) memcpy((void *)(opmsg_rtp_set_content_protection_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Rtp_Set_Latency_Change_Notification

  DESCRIPTION
    Rtp operator message for SET_LATENCY_CHANGE_NOTIFICATION

  MEMBERS
    message_id            - message id
    enable_latency_notify - enable or disable latency change notification for
                            aptx adaptive 0 = Disable Latency change
                            notification. 1 = Enable Latency change notofication.

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_RTP_SET_LATENCY_CHANGE_NOTIFICATION;

/* The following macros take OPMSG_RTP_SET_LATENCY_CHANGE_NOTIFICATION *opmsg_rtp_set_latency_change_notification_ptr */
#define OPMSG_RTP_SET_LATENCY_CHANGE_NOTIFICATION_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_RTP_SET_LATENCY_CHANGE_NOTIFICATION_MESSAGE_ID_GET(opmsg_rtp_set_latency_change_notification_ptr) ((OPMSG_RTP_DECODE_ID)(opmsg_rtp_set_latency_change_notification_ptr)->_data[0])
#define OPMSG_RTP_SET_LATENCY_CHANGE_NOTIFICATION_MESSAGE_ID_SET(opmsg_rtp_set_latency_change_notification_ptr, message_id) ((opmsg_rtp_set_latency_change_notification_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_RTP_SET_LATENCY_CHANGE_NOTIFICATION_ENABLE_LATENCY_NOTIFY_WORD_OFFSET (1)
#define OPMSG_RTP_SET_LATENCY_CHANGE_NOTIFICATION_ENABLE_LATENCY_NOTIFY_GET(opmsg_rtp_set_latency_change_notification_ptr) ((opmsg_rtp_set_latency_change_notification_ptr)->_data[1])
#define OPMSG_RTP_SET_LATENCY_CHANGE_NOTIFICATION_ENABLE_LATENCY_NOTIFY_SET(opmsg_rtp_set_latency_change_notification_ptr, enable_latency_notify) ((opmsg_rtp_set_latency_change_notification_ptr)->_data[1] = (uint16)(enable_latency_notify))
#define OPMSG_RTP_SET_LATENCY_CHANGE_NOTIFICATION_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_RTP_SET_LATENCY_CHANGE_NOTIFICATION_CREATE(message_id, enable_latency_notify) \
    (uint16)(message_id), \
    (uint16)(enable_latency_notify)
#define OPMSG_RTP_SET_LATENCY_CHANGE_NOTIFICATION_PACK(opmsg_rtp_set_latency_change_notification_ptr, message_id, enable_latency_notify) \
    do { \
        (opmsg_rtp_set_latency_change_notification_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_rtp_set_latency_change_notification_ptr)->_data[1] = (uint16)((uint16)(enable_latency_notify)); \
    } while (0)

#define OPMSG_RTP_SET_LATENCY_CHANGE_NOTIFICATION_MARSHALL(addr, opmsg_rtp_set_latency_change_notification_ptr) memcpy((void *)(addr), (void *)(opmsg_rtp_set_latency_change_notification_ptr), 2)
#define OPMSG_RTP_SET_LATENCY_CHANGE_NOTIFICATION_UNMARSHALL(addr, opmsg_rtp_set_latency_change_notification_ptr) memcpy((void *)(opmsg_rtp_set_latency_change_notification_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Rtp_Set_Max_Packet_Length

  DESCRIPTION
    Rtp operator message for setting the maximum packet length.

  MEMBERS
    message_id - message id
    length     - The maximum output packet length,in octets.

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_RTP_SET_MAX_PACKET_LENGTH;

/* The following macros take OPMSG_RTP_SET_MAX_PACKET_LENGTH *opmsg_rtp_set_max_packet_length_ptr */
#define OPMSG_RTP_SET_MAX_PACKET_LENGTH_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_RTP_SET_MAX_PACKET_LENGTH_MESSAGE_ID_GET(opmsg_rtp_set_max_packet_length_ptr) ((OPMSG_RTP_DECODE_ID)(opmsg_rtp_set_max_packet_length_ptr)->_data[0])
#define OPMSG_RTP_SET_MAX_PACKET_LENGTH_MESSAGE_ID_SET(opmsg_rtp_set_max_packet_length_ptr, message_id) ((opmsg_rtp_set_max_packet_length_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_RTP_SET_MAX_PACKET_LENGTH_LENGTH_WORD_OFFSET (1)
#define OPMSG_RTP_SET_MAX_PACKET_LENGTH_LENGTH_GET(opmsg_rtp_set_max_packet_length_ptr) ((opmsg_rtp_set_max_packet_length_ptr)->_data[1])
#define OPMSG_RTP_SET_MAX_PACKET_LENGTH_LENGTH_SET(opmsg_rtp_set_max_packet_length_ptr, length) ((opmsg_rtp_set_max_packet_length_ptr)->_data[1] = (uint16)(length))
#define OPMSG_RTP_SET_MAX_PACKET_LENGTH_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_RTP_SET_MAX_PACKET_LENGTH_CREATE(message_id, length) \
    (uint16)(message_id), \
    (uint16)(length)
#define OPMSG_RTP_SET_MAX_PACKET_LENGTH_PACK(opmsg_rtp_set_max_packet_length_ptr, message_id, length) \
    do { \
        (opmsg_rtp_set_max_packet_length_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_rtp_set_max_packet_length_ptr)->_data[1] = (uint16)((uint16)(length)); \
    } while (0)

#define OPMSG_RTP_SET_MAX_PACKET_LENGTH_MARSHALL(addr, opmsg_rtp_set_max_packet_length_ptr) memcpy((void *)(addr), (void *)(opmsg_rtp_set_max_packet_length_ptr), 2)
#define OPMSG_RTP_SET_MAX_PACKET_LENGTH_UNMARSHALL(addr, opmsg_rtp_set_max_packet_length_ptr) memcpy((void *)(opmsg_rtp_set_max_packet_length_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Rtp_Set_Packing

  DESCRIPTION
    RTP operator message for SET_PACKING

  MEMBERS
    message_id - message id
    packing    - The following configs are supported: 0 - The latency buffer
                 will be the output buffer without packing. 1 - An internal
                 buffer is created to pack the data to 32 bit. NORE: If
                 DATAFORMAT_32 is defined there is no need to pack the data.

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_RTP_SET_PACKING;

/* The following macros take OPMSG_RTP_SET_PACKING *opmsg_rtp_set_packing_ptr */
#define OPMSG_RTP_SET_PACKING_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_RTP_SET_PACKING_MESSAGE_ID_GET(opmsg_rtp_set_packing_ptr) ((OPMSG_RTP_DECODE_ID)(opmsg_rtp_set_packing_ptr)->_data[0])
#define OPMSG_RTP_SET_PACKING_MESSAGE_ID_SET(opmsg_rtp_set_packing_ptr, message_id) ((opmsg_rtp_set_packing_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_RTP_SET_PACKING_PACKING_WORD_OFFSET (1)
#define OPMSG_RTP_SET_PACKING_PACKING_GET(opmsg_rtp_set_packing_ptr) ((opmsg_rtp_set_packing_ptr)->_data[1])
#define OPMSG_RTP_SET_PACKING_PACKING_SET(opmsg_rtp_set_packing_ptr, packing) ((opmsg_rtp_set_packing_ptr)->_data[1] = (uint16)(packing))
#define OPMSG_RTP_SET_PACKING_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_RTP_SET_PACKING_CREATE(message_id, packing) \
    (uint16)(message_id), \
    (uint16)(packing)
#define OPMSG_RTP_SET_PACKING_PACK(opmsg_rtp_set_packing_ptr, message_id, packing) \
    do { \
        (opmsg_rtp_set_packing_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_rtp_set_packing_ptr)->_data[1] = (uint16)((uint16)(packing)); \
    } while (0)

#define OPMSG_RTP_SET_PACKING_MARSHALL(addr, opmsg_rtp_set_packing_ptr) memcpy((void *)(addr), (void *)(opmsg_rtp_set_packing_ptr), 2)
#define OPMSG_RTP_SET_PACKING_UNMARSHALL(addr, opmsg_rtp_set_packing_ptr) memcpy((void *)(opmsg_rtp_set_packing_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Rtp_Set_Src_Latency_Mapping

  DESCRIPTION
    RTP operator message for SET_SRC_LATENCY_MAPPING

  MEMBERS
    message_id - message id
    count      -
    entries    - <count> records each of 2 32-bit words: First word is Source ID
                 Second word is corresponding TTP latency Each MS 16 bits first
                 A zero latency indicates that the RTP timestamp should be used
                 directly as a playback time

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_RTP_SET_SRC_LATENCY_MAPPING;

/* The following macros take OPMSG_RTP_SET_SRC_LATENCY_MAPPING *opmsg_rtp_set_src_latency_mapping_ptr */
#define OPMSG_RTP_SET_SRC_LATENCY_MAPPING_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_RTP_SET_SRC_LATENCY_MAPPING_MESSAGE_ID_GET(opmsg_rtp_set_src_latency_mapping_ptr) ((OPMSG_RTP_DECODE_ID)(opmsg_rtp_set_src_latency_mapping_ptr)->_data[0])
#define OPMSG_RTP_SET_SRC_LATENCY_MAPPING_MESSAGE_ID_SET(opmsg_rtp_set_src_latency_mapping_ptr, message_id) ((opmsg_rtp_set_src_latency_mapping_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_RTP_SET_SRC_LATENCY_MAPPING_COUNT_WORD_OFFSET (1)
#define OPMSG_RTP_SET_SRC_LATENCY_MAPPING_COUNT_GET(opmsg_rtp_set_src_latency_mapping_ptr) ((opmsg_rtp_set_src_latency_mapping_ptr)->_data[1])
#define OPMSG_RTP_SET_SRC_LATENCY_MAPPING_COUNT_SET(opmsg_rtp_set_src_latency_mapping_ptr, count) ((opmsg_rtp_set_src_latency_mapping_ptr)->_data[1] = (uint16)(count))
#define OPMSG_RTP_SET_SRC_LATENCY_MAPPING_ENTRIES_WORD_OFFSET (2)
#define OPMSG_RTP_SET_SRC_LATENCY_MAPPING_ENTRIES_GET(opmsg_rtp_set_src_latency_mapping_ptr)  \
    (((uint32)((opmsg_rtp_set_src_latency_mapping_ptr)->_data[2]) | \
      ((uint32)((opmsg_rtp_set_src_latency_mapping_ptr)->_data[3]) << 16)))
#define OPMSG_RTP_SET_SRC_LATENCY_MAPPING_ENTRIES_SET(opmsg_rtp_set_src_latency_mapping_ptr, entries) do { \
        (opmsg_rtp_set_src_latency_mapping_ptr)->_data[2] = (uint16)((entries) & 0xffff); \
        (opmsg_rtp_set_src_latency_mapping_ptr)->_data[3] = (uint16)((entries) >> 16); } while (0)
#define OPMSG_RTP_SET_SRC_LATENCY_MAPPING_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_RTP_SET_SRC_LATENCY_MAPPING_CREATE(message_id, count, entries) \
    (uint16)(message_id), \
    (uint16)(count), \
    (uint16)((entries) & 0xffff), \
    (uint16)((entries) >> 16)
#define OPMSG_RTP_SET_SRC_LATENCY_MAPPING_PACK(opmsg_rtp_set_src_latency_mapping_ptr, message_id, count, entries) \
    do { \
        (opmsg_rtp_set_src_latency_mapping_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_rtp_set_src_latency_mapping_ptr)->_data[1] = (uint16)((uint16)(count)); \
        (opmsg_rtp_set_src_latency_mapping_ptr)->_data[2] = (uint16)((uint16)((entries) & 0xffff)); \
        (opmsg_rtp_set_src_latency_mapping_ptr)->_data[3] = (uint16)(((entries) >> 16)); \
    } while (0)

#define OPMSG_RTP_SET_SRC_LATENCY_MAPPING_MARSHALL(addr, opmsg_rtp_set_src_latency_mapping_ptr) memcpy((void *)(addr), (void *)(opmsg_rtp_set_src_latency_mapping_ptr), 4)
#define OPMSG_RTP_SET_SRC_LATENCY_MAPPING_UNMARSHALL(addr, opmsg_rtp_set_src_latency_mapping_ptr) memcpy((void *)(opmsg_rtp_set_src_latency_mapping_ptr), (void *)(addr), 4)


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

#define OPMSG_RTP_SET_WORKING_MODE_MARSHALL(addr, opmsg_rtp_set_working_mode_ptr) memcpy((void *)(addr), (void *)(opmsg_rtp_set_working_mode_ptr), 2)
#define OPMSG_RTP_SET_WORKING_MODE_UNMARSHALL(addr, opmsg_rtp_set_working_mode_ptr) memcpy((void *)(opmsg_rtp_set_working_mode_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_SPC_Select_Passthrough

  DESCRIPTION
    Request the SPC to use a different input

  MEMBERS
    message_id - message id
    new_input  - Select input to pass through. 0 = consume, 1 = first input (0)

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_SPC_SELECT_PASSTHROUGH;

/* The following macros take OPMSG_SPC_SELECT_PASSTHROUGH *opmsg_spc_select_passthrough_ptr */
#define OPMSG_SPC_SELECT_PASSTHROUGH_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPC_SELECT_PASSTHROUGH_MESSAGE_ID_GET(opmsg_spc_select_passthrough_ptr) ((OPMSG_SPC_ID)(opmsg_spc_select_passthrough_ptr)->_data[0])
#define OPMSG_SPC_SELECT_PASSTHROUGH_MESSAGE_ID_SET(opmsg_spc_select_passthrough_ptr, message_id) ((opmsg_spc_select_passthrough_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPC_SELECT_PASSTHROUGH_NEW_INPUT_WORD_OFFSET (1)
#define OPMSG_SPC_SELECT_PASSTHROUGH_NEW_INPUT_GET(opmsg_spc_select_passthrough_ptr) ((opmsg_spc_select_passthrough_ptr)->_data[1])
#define OPMSG_SPC_SELECT_PASSTHROUGH_NEW_INPUT_SET(opmsg_spc_select_passthrough_ptr, new_input) ((opmsg_spc_select_passthrough_ptr)->_data[1] = (uint16)(new_input))
#define OPMSG_SPC_SELECT_PASSTHROUGH_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPC_SELECT_PASSTHROUGH_CREATE(message_id, new_input) \
    (uint16)(message_id), \
    (uint16)(new_input)
#define OPMSG_SPC_SELECT_PASSTHROUGH_PACK(opmsg_spc_select_passthrough_ptr, message_id, new_input) \
    do { \
        (opmsg_spc_select_passthrough_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_spc_select_passthrough_ptr)->_data[1] = (uint16)((uint16)(new_input)); \
    } while (0)

#define OPMSG_SPC_SELECT_PASSTHROUGH_MARSHALL(addr, opmsg_spc_select_passthrough_ptr) memcpy((void *)(addr), (void *)(opmsg_spc_select_passthrough_ptr), 2)
#define OPMSG_SPC_SELECT_PASSTHROUGH_UNMARSHALL(addr, opmsg_spc_select_passthrough_ptr) memcpy((void *)(opmsg_spc_select_passthrough_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_SPC_Set_Buffering

  DESCRIPTION
    Switched Passthrough Consumer operator message for Set_Buffering.

  MEMBERS
    message_id  - message id
    buffer_size - The size of the buffer in words.

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_SPC_SET_BUFFERING;

/* The following macros take OPMSG_SPC_SET_BUFFERING *opmsg_spc_set_buffering_ptr */
#define OPMSG_SPC_SET_BUFFERING_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPC_SET_BUFFERING_MESSAGE_ID_GET(opmsg_spc_set_buffering_ptr) ((OPMSG_SPC_ID)(opmsg_spc_set_buffering_ptr)->_data[0])
#define OPMSG_SPC_SET_BUFFERING_MESSAGE_ID_SET(opmsg_spc_set_buffering_ptr, message_id) ((opmsg_spc_set_buffering_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPC_SET_BUFFERING_BUFFER_SIZE_WORD_OFFSET (1)
#define OPMSG_SPC_SET_BUFFERING_BUFFER_SIZE_GET(opmsg_spc_set_buffering_ptr) ((opmsg_spc_set_buffering_ptr)->_data[1])
#define OPMSG_SPC_SET_BUFFERING_BUFFER_SIZE_SET(opmsg_spc_set_buffering_ptr, buffer_size) ((opmsg_spc_set_buffering_ptr)->_data[1] = (uint16)(buffer_size))
#define OPMSG_SPC_SET_BUFFERING_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPC_SET_BUFFERING_CREATE(message_id, buffer_size) \
    (uint16)(message_id), \
    (uint16)(buffer_size)
#define OPMSG_SPC_SET_BUFFERING_PACK(opmsg_spc_set_buffering_ptr, message_id, buffer_size) \
    do { \
        (opmsg_spc_set_buffering_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_spc_set_buffering_ptr)->_data[1] = (uint16)((uint16)(buffer_size)); \
    } while (0)

#define OPMSG_SPC_SET_BUFFERING_MARSHALL(addr, opmsg_spc_set_buffering_ptr) memcpy((void *)(addr), (void *)(opmsg_spc_set_buffering_ptr), 2)
#define OPMSG_SPC_SET_BUFFERING_UNMARSHALL(addr, opmsg_spc_set_buffering_ptr) memcpy((void *)(opmsg_spc_set_buffering_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_SPC_Set_Data_Format

  DESCRIPTION
    Switched Passthrough Consumer operator message for SET_DATA_FORMAT.

  MEMBERS
    message_id - message id
    data_type  - data type

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_SPC_SET_DATA_FORMAT;

/* The following macros take OPMSG_SPC_SET_DATA_FORMAT *opmsg_spc_set_data_format_ptr */
#define OPMSG_SPC_SET_DATA_FORMAT_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPC_SET_DATA_FORMAT_MESSAGE_ID_GET(opmsg_spc_set_data_format_ptr) ((OPMSG_SPC_ID)(opmsg_spc_set_data_format_ptr)->_data[0])
#define OPMSG_SPC_SET_DATA_FORMAT_MESSAGE_ID_SET(opmsg_spc_set_data_format_ptr, message_id) ((opmsg_spc_set_data_format_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPC_SET_DATA_FORMAT_DATA_TYPE_WORD_OFFSET (1)
#define OPMSG_SPC_SET_DATA_FORMAT_DATA_TYPE_GET(opmsg_spc_set_data_format_ptr) ((opmsg_spc_set_data_format_ptr)->_data[1])
#define OPMSG_SPC_SET_DATA_FORMAT_DATA_TYPE_SET(opmsg_spc_set_data_format_ptr, data_type) ((opmsg_spc_set_data_format_ptr)->_data[1] = (uint16)(data_type))
#define OPMSG_SPC_SET_DATA_FORMAT_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPC_SET_DATA_FORMAT_CREATE(message_id, data_type) \
    (uint16)(message_id), \
    (uint16)(data_type)
#define OPMSG_SPC_SET_DATA_FORMAT_PACK(opmsg_spc_set_data_format_ptr, message_id, data_type) \
    do { \
        (opmsg_spc_set_data_format_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_spc_set_data_format_ptr)->_data[1] = (uint16)((uint16)(data_type)); \
    } while (0)

#define OPMSG_SPC_SET_DATA_FORMAT_MARSHALL(addr, opmsg_spc_set_data_format_ptr) memcpy((void *)(addr), (void *)(opmsg_spc_set_data_format_ptr), 2)
#define OPMSG_SPC_SET_DATA_FORMAT_UNMARSHALL(addr, opmsg_spc_set_data_format_ptr) memcpy((void *)(opmsg_spc_set_data_format_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_SPC_Set_Transition

  DESCRIPTION
    Switched Passthrough Consumer operator message for Start Passthrough From.

  MEMBERS
    message_id   - message id
    timestamp_ms - Most-significant 16 bits of the timestamp, in microseconds
    timestamp_ls - Least-significant 16 bits of the timestamp, in microseconds

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_SPC_SET_TRANSITION;

/* The following macros take OPMSG_SPC_SET_TRANSITION *opmsg_spc_set_transition_ptr */
#define OPMSG_SPC_SET_TRANSITION_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPC_SET_TRANSITION_MESSAGE_ID_GET(opmsg_spc_set_transition_ptr) ((OPMSG_SPC_ID)(opmsg_spc_set_transition_ptr)->_data[0])
#define OPMSG_SPC_SET_TRANSITION_MESSAGE_ID_SET(opmsg_spc_set_transition_ptr, message_id) ((opmsg_spc_set_transition_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPC_SET_TRANSITION_TIMESTAMP_MS_WORD_OFFSET (1)
#define OPMSG_SPC_SET_TRANSITION_TIMESTAMP_MS_GET(opmsg_spc_set_transition_ptr) ((opmsg_spc_set_transition_ptr)->_data[1])
#define OPMSG_SPC_SET_TRANSITION_TIMESTAMP_MS_SET(opmsg_spc_set_transition_ptr, timestamp_ms) ((opmsg_spc_set_transition_ptr)->_data[1] = (uint16)(timestamp_ms))
#define OPMSG_SPC_SET_TRANSITION_TIMESTAMP_LS_WORD_OFFSET (2)
#define OPMSG_SPC_SET_TRANSITION_TIMESTAMP_LS_GET(opmsg_spc_set_transition_ptr) ((opmsg_spc_set_transition_ptr)->_data[2])
#define OPMSG_SPC_SET_TRANSITION_TIMESTAMP_LS_SET(opmsg_spc_set_transition_ptr, timestamp_ls) ((opmsg_spc_set_transition_ptr)->_data[2] = (uint16)(timestamp_ls))
#define OPMSG_SPC_SET_TRANSITION_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPC_SET_TRANSITION_CREATE(message_id, timestamp_ms, timestamp_ls) \
    (uint16)(message_id), \
    (uint16)(timestamp_ms), \
    (uint16)(timestamp_ls)
#define OPMSG_SPC_SET_TRANSITION_PACK(opmsg_spc_set_transition_ptr, message_id, timestamp_ms, timestamp_ls) \
    do { \
        (opmsg_spc_set_transition_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_spc_set_transition_ptr)->_data[1] = (uint16)((uint16)(timestamp_ms)); \
        (opmsg_spc_set_transition_ptr)->_data[2] = (uint16)((uint16)(timestamp_ls)); \
    } while (0)

#define OPMSG_SPC_SET_TRANSITION_MARSHALL(addr, opmsg_spc_set_transition_ptr) memcpy((void *)(addr), (void *)(opmsg_spc_set_transition_ptr), 3)
#define OPMSG_SPC_SET_TRANSITION_UNMARSHALL(addr, opmsg_spc_set_transition_ptr) memcpy((void *)(opmsg_spc_set_transition_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    Opmsg_Sbc_Enc_Set_Encoding_Params

  DESCRIPTION
    SBC Encoder Operator message for configuring the encoding parameters used
    for encoding.

  MEMBERS
    message_id        - message id
    no_subbands       - Number of subbands to use
    no_blocks         - Number of blocks per frame
    bitpool           - Bitpool size to use
    sampling_freq     - Sampling frequency in the format fs/25
    channel_mode      - 0 = Mono, 1 = Dual Mono, 2 = Stereo, 3 = Joint Stereo
    allocation_method - Allocation method: 0 = Loudness, 1 = SNR

*******************************************************************************/
typedef struct
{
    uint16 _data[7];
} OPMSG_SBC_ENC_SET_ENCODING_PARAMS;

/* The following macros take OPMSG_SBC_ENC_SET_ENCODING_PARAMS *opmsg_sbc_enc_set_encoding_params_ptr */
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_MESSAGE_ID_GET(opmsg_sbc_enc_set_encoding_params_ptr) ((OPMSG_SBC_ENC_ID)(opmsg_sbc_enc_set_encoding_params_ptr)->_data[0])
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_MESSAGE_ID_SET(opmsg_sbc_enc_set_encoding_params_ptr, message_id) ((opmsg_sbc_enc_set_encoding_params_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_NO_SUBBANDS_WORD_OFFSET (1)
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_NO_SUBBANDS_GET(opmsg_sbc_enc_set_encoding_params_ptr) ((opmsg_sbc_enc_set_encoding_params_ptr)->_data[1])
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_NO_SUBBANDS_SET(opmsg_sbc_enc_set_encoding_params_ptr, no_subbands) ((opmsg_sbc_enc_set_encoding_params_ptr)->_data[1] = (uint16)(no_subbands))
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_NO_BLOCKS_WORD_OFFSET (2)
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_NO_BLOCKS_GET(opmsg_sbc_enc_set_encoding_params_ptr) ((opmsg_sbc_enc_set_encoding_params_ptr)->_data[2])
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_NO_BLOCKS_SET(opmsg_sbc_enc_set_encoding_params_ptr, no_blocks) ((opmsg_sbc_enc_set_encoding_params_ptr)->_data[2] = (uint16)(no_blocks))
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_BITPOOL_WORD_OFFSET (3)
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_BITPOOL_GET(opmsg_sbc_enc_set_encoding_params_ptr) ((opmsg_sbc_enc_set_encoding_params_ptr)->_data[3])
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_BITPOOL_SET(opmsg_sbc_enc_set_encoding_params_ptr, bitpool) ((opmsg_sbc_enc_set_encoding_params_ptr)->_data[3] = (uint16)(bitpool))
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_SAMPLING_FREQ_WORD_OFFSET (4)
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_SAMPLING_FREQ_GET(opmsg_sbc_enc_set_encoding_params_ptr) ((opmsg_sbc_enc_set_encoding_params_ptr)->_data[4])
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_SAMPLING_FREQ_SET(opmsg_sbc_enc_set_encoding_params_ptr, sampling_freq) ((opmsg_sbc_enc_set_encoding_params_ptr)->_data[4] = (uint16)(sampling_freq))
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_CHANNEL_MODE_WORD_OFFSET (5)
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_CHANNEL_MODE_GET(opmsg_sbc_enc_set_encoding_params_ptr) ((opmsg_sbc_enc_set_encoding_params_ptr)->_data[5])
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_CHANNEL_MODE_SET(opmsg_sbc_enc_set_encoding_params_ptr, channel_mode) ((opmsg_sbc_enc_set_encoding_params_ptr)->_data[5] = (uint16)(channel_mode))
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_ALLOCATION_METHOD_WORD_OFFSET (6)
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_ALLOCATION_METHOD_GET(opmsg_sbc_enc_set_encoding_params_ptr) ((opmsg_sbc_enc_set_encoding_params_ptr)->_data[6])
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_ALLOCATION_METHOD_SET(opmsg_sbc_enc_set_encoding_params_ptr, allocation_method) ((opmsg_sbc_enc_set_encoding_params_ptr)->_data[6] = (uint16)(allocation_method))
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_WORD_SIZE (7)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_CREATE(message_id, no_subbands, no_blocks, bitpool, sampling_freq, channel_mode, allocation_method) \
    (uint16)(message_id), \
    (uint16)(no_subbands), \
    (uint16)(no_blocks), \
    (uint16)(bitpool), \
    (uint16)(sampling_freq), \
    (uint16)(channel_mode), \
    (uint16)(allocation_method)
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_PACK(opmsg_sbc_enc_set_encoding_params_ptr, message_id, no_subbands, no_blocks, bitpool, sampling_freq, channel_mode, allocation_method) \
    do { \
        (opmsg_sbc_enc_set_encoding_params_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_sbc_enc_set_encoding_params_ptr)->_data[1] = (uint16)((uint16)(no_subbands)); \
        (opmsg_sbc_enc_set_encoding_params_ptr)->_data[2] = (uint16)((uint16)(no_blocks)); \
        (opmsg_sbc_enc_set_encoding_params_ptr)->_data[3] = (uint16)((uint16)(bitpool)); \
        (opmsg_sbc_enc_set_encoding_params_ptr)->_data[4] = (uint16)((uint16)(sampling_freq)); \
        (opmsg_sbc_enc_set_encoding_params_ptr)->_data[5] = (uint16)((uint16)(channel_mode)); \
        (opmsg_sbc_enc_set_encoding_params_ptr)->_data[6] = (uint16)((uint16)(allocation_method)); \
    } while (0)

#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_MARSHALL(addr, opmsg_sbc_enc_set_encoding_params_ptr) memcpy((void *)(addr), (void *)(opmsg_sbc_enc_set_encoding_params_ptr), 7)
#define OPMSG_SBC_ENC_SET_ENCODING_PARAMS_UNMARSHALL(addr, opmsg_sbc_enc_set_encoding_params_ptr) memcpy((void *)(opmsg_sbc_enc_set_encoding_params_ptr), (void *)(addr), 7)


/*******************************************************************************

  NAME
    Opmsg_Sco_Common_Rcv_Force_Plc_Off

  DESCRIPTION
    SCO common receive operator message for FORCE_PLC_OFF.

  MEMBERS
    message_id    - message id
    force_plc_off - set/unset force PLC off

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_SCO_COMMON_RCV_FORCE_PLC_OFF;

/* The following macros take OPMSG_SCO_COMMON_RCV_FORCE_PLC_OFF *opmsg_sco_common_rcv_force_plc_off_ptr */
#define OPMSG_SCO_COMMON_RCV_FORCE_PLC_OFF_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SCO_COMMON_RCV_FORCE_PLC_OFF_MESSAGE_ID_GET(opmsg_sco_common_rcv_force_plc_off_ptr) ((opmsg_sco_common_rcv_force_plc_off_ptr)->_data[0])
#define OPMSG_SCO_COMMON_RCV_FORCE_PLC_OFF_MESSAGE_ID_SET(opmsg_sco_common_rcv_force_plc_off_ptr, message_id) ((opmsg_sco_common_rcv_force_plc_off_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SCO_COMMON_RCV_FORCE_PLC_OFF_FORCE_PLC_OFF_WORD_OFFSET (1)
#define OPMSG_SCO_COMMON_RCV_FORCE_PLC_OFF_FORCE_PLC_OFF_GET(opmsg_sco_common_rcv_force_plc_off_ptr) ((opmsg_sco_common_rcv_force_plc_off_ptr)->_data[1])
#define OPMSG_SCO_COMMON_RCV_FORCE_PLC_OFF_FORCE_PLC_OFF_SET(opmsg_sco_common_rcv_force_plc_off_ptr, force_plc_off) ((opmsg_sco_common_rcv_force_plc_off_ptr)->_data[1] = (uint16)(force_plc_off))
#define OPMSG_SCO_COMMON_RCV_FORCE_PLC_OFF_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SCO_COMMON_RCV_FORCE_PLC_OFF_CREATE(message_id, force_plc_off) \
    (uint16)(message_id), \
    (uint16)(force_plc_off)
#define OPMSG_SCO_COMMON_RCV_FORCE_PLC_OFF_PACK(opmsg_sco_common_rcv_force_plc_off_ptr, message_id, force_plc_off) \
    do { \
        (opmsg_sco_common_rcv_force_plc_off_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_sco_common_rcv_force_plc_off_ptr)->_data[1] = (uint16)((uint16)(force_plc_off)); \
    } while (0)

#define OPMSG_SCO_COMMON_RCV_FORCE_PLC_OFF_MARSHALL(addr, opmsg_sco_common_rcv_force_plc_off_ptr) memcpy((void *)(addr), (void *)(opmsg_sco_common_rcv_force_plc_off_ptr), 2)
#define OPMSG_SCO_COMMON_RCV_FORCE_PLC_OFF_UNMARSHALL(addr, opmsg_sco_common_rcv_force_plc_off_ptr) memcpy((void *)(opmsg_sco_common_rcv_force_plc_off_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Sco_Common_Rcv_Frame_Counts

  DESCRIPTION
    SCO common receive operator message for FRAME_COUNTS.

  MEMBERS
    message_id   - message id
    frame_counts - frame counts

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_SCO_COMMON_RCV_FRAME_COUNTS;

/* The following macros take OPMSG_SCO_COMMON_RCV_FRAME_COUNTS *opmsg_sco_common_rcv_frame_counts_ptr */
#define OPMSG_SCO_COMMON_RCV_FRAME_COUNTS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SCO_COMMON_RCV_FRAME_COUNTS_MESSAGE_ID_GET(opmsg_sco_common_rcv_frame_counts_ptr) ((opmsg_sco_common_rcv_frame_counts_ptr)->_data[0])
#define OPMSG_SCO_COMMON_RCV_FRAME_COUNTS_MESSAGE_ID_SET(opmsg_sco_common_rcv_frame_counts_ptr, message_id) ((opmsg_sco_common_rcv_frame_counts_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SCO_COMMON_RCV_FRAME_COUNTS_FRAME_COUNTS_WORD_OFFSET (1)
#define OPMSG_SCO_COMMON_RCV_FRAME_COUNTS_FRAME_COUNTS_GET(opmsg_sco_common_rcv_frame_counts_ptr) ((opmsg_sco_common_rcv_frame_counts_ptr)->_data[1])
#define OPMSG_SCO_COMMON_RCV_FRAME_COUNTS_FRAME_COUNTS_SET(opmsg_sco_common_rcv_frame_counts_ptr, frame_counts) ((opmsg_sco_common_rcv_frame_counts_ptr)->_data[1] = (uint16)(frame_counts))
#define OPMSG_SCO_COMMON_RCV_FRAME_COUNTS_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SCO_COMMON_RCV_FRAME_COUNTS_CREATE(message_id, frame_counts) \
    (uint16)(message_id), \
    (uint16)(frame_counts)
#define OPMSG_SCO_COMMON_RCV_FRAME_COUNTS_PACK(opmsg_sco_common_rcv_frame_counts_ptr, message_id, frame_counts) \
    do { \
        (opmsg_sco_common_rcv_frame_counts_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_sco_common_rcv_frame_counts_ptr)->_data[1] = (uint16)((uint16)(frame_counts)); \
    } while (0)

#define OPMSG_SCO_COMMON_RCV_FRAME_COUNTS_MARSHALL(addr, opmsg_sco_common_rcv_frame_counts_ptr) memcpy((void *)(addr), (void *)(opmsg_sco_common_rcv_frame_counts_ptr), 2)
#define OPMSG_SCO_COMMON_RCV_FRAME_COUNTS_UNMARSHALL(addr, opmsg_sco_common_rcv_frame_counts_ptr) memcpy((void *)(opmsg_sco_common_rcv_frame_counts_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Sco_Common_Rcv_Set_From_Air_Info

  DESCRIPTION
    SCO common receive operator message for SET_FROM_AIR_INFO.

  MEMBERS
    message_id - message id
    pkt_size   - frame size
    exp_pkts   - expected packets
    t_esco     - t_esco

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO;

/* The following macros take OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO *opmsg_sco_common_rcv_set_from_air_info_ptr */
#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_MESSAGE_ID_GET(opmsg_sco_common_rcv_set_from_air_info_ptr) ((opmsg_sco_common_rcv_set_from_air_info_ptr)->_data[0])
#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_MESSAGE_ID_SET(opmsg_sco_common_rcv_set_from_air_info_ptr, message_id) ((opmsg_sco_common_rcv_set_from_air_info_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_PKT_SIZE_WORD_OFFSET (1)
#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_PKT_SIZE_GET(opmsg_sco_common_rcv_set_from_air_info_ptr) ((opmsg_sco_common_rcv_set_from_air_info_ptr)->_data[1])
#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_PKT_SIZE_SET(opmsg_sco_common_rcv_set_from_air_info_ptr, pkt_size) ((opmsg_sco_common_rcv_set_from_air_info_ptr)->_data[1] = (uint16)(pkt_size))
#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_EXP_PKTS_WORD_OFFSET (2)
#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_EXP_PKTS_GET(opmsg_sco_common_rcv_set_from_air_info_ptr) ((opmsg_sco_common_rcv_set_from_air_info_ptr)->_data[2])
#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_EXP_PKTS_SET(opmsg_sco_common_rcv_set_from_air_info_ptr, exp_pkts) ((opmsg_sco_common_rcv_set_from_air_info_ptr)->_data[2] = (uint16)(exp_pkts))
#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_T_ESCO_WORD_OFFSET (3)
#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_T_ESCO_GET(opmsg_sco_common_rcv_set_from_air_info_ptr) ((opmsg_sco_common_rcv_set_from_air_info_ptr)->_data[3])
#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_T_ESCO_SET(opmsg_sco_common_rcv_set_from_air_info_ptr, t_esco) ((opmsg_sco_common_rcv_set_from_air_info_ptr)->_data[3] = (uint16)(t_esco))
#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_CREATE(message_id, pkt_size, exp_pkts, t_esco) \
    (uint16)(message_id), \
    (uint16)(pkt_size), \
    (uint16)(exp_pkts), \
    (uint16)(t_esco)
#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_PACK(opmsg_sco_common_rcv_set_from_air_info_ptr, message_id, pkt_size, exp_pkts, t_esco) \
    do { \
        (opmsg_sco_common_rcv_set_from_air_info_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_sco_common_rcv_set_from_air_info_ptr)->_data[1] = (uint16)((uint16)(pkt_size)); \
        (opmsg_sco_common_rcv_set_from_air_info_ptr)->_data[2] = (uint16)((uint16)(exp_pkts)); \
        (opmsg_sco_common_rcv_set_from_air_info_ptr)->_data[3] = (uint16)((uint16)(t_esco)); \
    } while (0)

#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_MARSHALL(addr, opmsg_sco_common_rcv_set_from_air_info_ptr) memcpy((void *)(addr), (void *)(opmsg_sco_common_rcv_set_from_air_info_ptr), 4)
#define OPMSG_SCO_COMMON_RCV_SET_FROM_AIR_INFO_UNMARSHALL(addr, opmsg_sco_common_rcv_set_from_air_info_ptr) memcpy((void *)(opmsg_sco_common_rcv_set_from_air_info_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    Opmsg_Sco_Send_Set_To_Air_Info

  DESCRIPTION
    SCO send operator message for SET_TO_AIR_INFO.

  MEMBERS
    message_id - message id
    frame_size - frame size

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_SCO_SEND_SET_TO_AIR_INFO;

/* The following macros take OPMSG_SCO_SEND_SET_TO_AIR_INFO *opmsg_sco_send_set_to_air_info_ptr */
#define OPMSG_SCO_SEND_SET_TO_AIR_INFO_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SCO_SEND_SET_TO_AIR_INFO_MESSAGE_ID_GET(opmsg_sco_send_set_to_air_info_ptr) ((OPMSG_SCO_SEND_ID)(opmsg_sco_send_set_to_air_info_ptr)->_data[0])
#define OPMSG_SCO_SEND_SET_TO_AIR_INFO_MESSAGE_ID_SET(opmsg_sco_send_set_to_air_info_ptr, message_id) ((opmsg_sco_send_set_to_air_info_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SCO_SEND_SET_TO_AIR_INFO_FRAME_SIZE_WORD_OFFSET (1)
#define OPMSG_SCO_SEND_SET_TO_AIR_INFO_FRAME_SIZE_GET(opmsg_sco_send_set_to_air_info_ptr) ((opmsg_sco_send_set_to_air_info_ptr)->_data[1])
#define OPMSG_SCO_SEND_SET_TO_AIR_INFO_FRAME_SIZE_SET(opmsg_sco_send_set_to_air_info_ptr, frame_size) ((opmsg_sco_send_set_to_air_info_ptr)->_data[1] = (uint16)(frame_size))
#define OPMSG_SCO_SEND_SET_TO_AIR_INFO_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SCO_SEND_SET_TO_AIR_INFO_CREATE(message_id, frame_size) \
    (uint16)(message_id), \
    (uint16)(frame_size)
#define OPMSG_SCO_SEND_SET_TO_AIR_INFO_PACK(opmsg_sco_send_set_to_air_info_ptr, message_id, frame_size) \
    do { \
        (opmsg_sco_send_set_to_air_info_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_sco_send_set_to_air_info_ptr)->_data[1] = (uint16)((uint16)(frame_size)); \
    } while (0)

#define OPMSG_SCO_SEND_SET_TO_AIR_INFO_MARSHALL(addr, opmsg_sco_send_set_to_air_info_ptr) memcpy((void *)(addr), (void *)(opmsg_sco_send_set_to_air_info_ptr), 2)
#define OPMSG_SCO_SEND_SET_TO_AIR_INFO_UNMARSHALL(addr, opmsg_sco_send_set_to_air_info_ptr) memcpy((void *)(opmsg_sco_send_set_to_air_info_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Source_Sync_Get_Routes

  DESCRIPTION
    Source sync operator message for GET_ROUTES.

  MEMBERS
    message_id  - message id
    routes_mask - Bitmask of source indices whose routes are requested

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_SOURCE_SYNC_GET_ROUTES;

/* The following macros take OPMSG_SOURCE_SYNC_GET_ROUTES *opmsg_source_sync_get_routes_ptr */
#define OPMSG_SOURCE_SYNC_GET_ROUTES_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SOURCE_SYNC_GET_ROUTES_MESSAGE_ID_GET(opmsg_source_sync_get_routes_ptr) ((OPMSG_SRC_SYNC_ID)(opmsg_source_sync_get_routes_ptr)->_data[0])
#define OPMSG_SOURCE_SYNC_GET_ROUTES_MESSAGE_ID_SET(opmsg_source_sync_get_routes_ptr, message_id) ((opmsg_source_sync_get_routes_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SOURCE_SYNC_GET_ROUTES_ROUTES_MASK_WORD_OFFSET (1)
#define OPMSG_SOURCE_SYNC_GET_ROUTES_ROUTES_MASK_GET(opmsg_source_sync_get_routes_ptr)  \
    (((uint32)((opmsg_source_sync_get_routes_ptr)->_data[1]) | \
      ((uint32)((opmsg_source_sync_get_routes_ptr)->_data[2]) << 16)))
#define OPMSG_SOURCE_SYNC_GET_ROUTES_ROUTES_MASK_SET(opmsg_source_sync_get_routes_ptr, routes_mask) do { \
        (opmsg_source_sync_get_routes_ptr)->_data[1] = (uint16)((routes_mask) & 0xffff); \
        (opmsg_source_sync_get_routes_ptr)->_data[2] = (uint16)((routes_mask) >> 16); } while (0)
#define OPMSG_SOURCE_SYNC_GET_ROUTES_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SOURCE_SYNC_GET_ROUTES_CREATE(message_id, routes_mask) \
    (uint16)(message_id), \
    (uint16)((routes_mask) & 0xffff), \
    (uint16)((routes_mask) >> 16)
#define OPMSG_SOURCE_SYNC_GET_ROUTES_PACK(opmsg_source_sync_get_routes_ptr, message_id, routes_mask) \
    do { \
        (opmsg_source_sync_get_routes_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_source_sync_get_routes_ptr)->_data[1] = (uint16)((uint16)((routes_mask) & 0xffff)); \
        (opmsg_source_sync_get_routes_ptr)->_data[2] = (uint16)(((routes_mask) >> 16)); \
    } while (0)

#define OPMSG_SOURCE_SYNC_GET_ROUTES_MARSHALL(addr, opmsg_source_sync_get_routes_ptr) memcpy((void *)(addr), (void *)(opmsg_source_sync_get_routes_ptr), 3)
#define OPMSG_SOURCE_SYNC_GET_ROUTES_UNMARSHALL(addr, opmsg_source_sync_get_routes_ptr) memcpy((void *)(opmsg_source_sync_get_routes_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    Opmsg_Source_Sync_Get_Routes_Resp

  DESCRIPTION
    Source sync operator response message for GET_ROUTES.

  MEMBERS
    message_id  - message id
    routes_mask - Bitmask of source indices whose routes were requested
    routes      - records of 3 words each: packed ( bit 15 source_connected, bit
                  8-14 source_index, bit 7 sink_connected, bit 0-6 sink_index ),
                  sample_rate (units of 25Hz, or 0 if not connected), gain
                  (units of 1/60 db)

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_SOURCE_SYNC_GET_ROUTES_RESP;

/* The following macros take OPMSG_SOURCE_SYNC_GET_ROUTES_RESP *opmsg_source_sync_get_routes_resp_ptr */
#define OPMSG_SOURCE_SYNC_GET_ROUTES_RESP_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SOURCE_SYNC_GET_ROUTES_RESP_MESSAGE_ID_GET(opmsg_source_sync_get_routes_resp_ptr) ((OPMSG_SRC_SYNC_ID)(opmsg_source_sync_get_routes_resp_ptr)->_data[0])
#define OPMSG_SOURCE_SYNC_GET_ROUTES_RESP_MESSAGE_ID_SET(opmsg_source_sync_get_routes_resp_ptr, message_id) ((opmsg_source_sync_get_routes_resp_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SOURCE_SYNC_GET_ROUTES_RESP_ROUTES_MASK_WORD_OFFSET (1)
#define OPMSG_SOURCE_SYNC_GET_ROUTES_RESP_ROUTES_MASK_GET(opmsg_source_sync_get_routes_resp_ptr)  \
    (((uint32)((opmsg_source_sync_get_routes_resp_ptr)->_data[1]) | \
      ((uint32)((opmsg_source_sync_get_routes_resp_ptr)->_data[2]) << 16)))
#define OPMSG_SOURCE_SYNC_GET_ROUTES_RESP_ROUTES_MASK_SET(opmsg_source_sync_get_routes_resp_ptr, routes_mask) do { \
        (opmsg_source_sync_get_routes_resp_ptr)->_data[1] = (uint16)((routes_mask) & 0xffff); \
        (opmsg_source_sync_get_routes_resp_ptr)->_data[2] = (uint16)((routes_mask) >> 16); } while (0)
#define OPMSG_SOURCE_SYNC_GET_ROUTES_RESP_ROUTES_WORD_OFFSET (3)
#define OPMSG_SOURCE_SYNC_GET_ROUTES_RESP_ROUTES_GET(opmsg_source_sync_get_routes_resp_ptr) ((opmsg_source_sync_get_routes_resp_ptr)->_data[3])
#define OPMSG_SOURCE_SYNC_GET_ROUTES_RESP_ROUTES_SET(opmsg_source_sync_get_routes_resp_ptr, routes) ((opmsg_source_sync_get_routes_resp_ptr)->_data[3] = (uint16)(routes))
#define OPMSG_SOURCE_SYNC_GET_ROUTES_RESP_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SOURCE_SYNC_GET_ROUTES_RESP_CREATE(message_id, routes_mask, routes) \
    (uint16)(message_id), \
    (uint16)((routes_mask) & 0xffff), \
    (uint16)((routes_mask) >> 16), \
    (uint16)(routes)
#define OPMSG_SOURCE_SYNC_GET_ROUTES_RESP_PACK(opmsg_source_sync_get_routes_resp_ptr, message_id, routes_mask, routes) \
    do { \
        (opmsg_source_sync_get_routes_resp_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_source_sync_get_routes_resp_ptr)->_data[1] = (uint16)((uint16)((routes_mask) & 0xffff)); \
        (opmsg_source_sync_get_routes_resp_ptr)->_data[2] = (uint16)(((routes_mask) >> 16)); \
        (opmsg_source_sync_get_routes_resp_ptr)->_data[3] = (uint16)((uint16)(routes)); \
    } while (0)

#define OPMSG_SOURCE_SYNC_GET_ROUTES_RESP_MARSHALL(addr, opmsg_source_sync_get_routes_resp_ptr) memcpy((void *)(addr), (void *)(opmsg_source_sync_get_routes_resp_ptr), 4)
#define OPMSG_SOURCE_SYNC_GET_ROUTES_RESP_UNMARSHALL(addr, opmsg_source_sync_get_routes_resp_ptr) memcpy((void *)(opmsg_source_sync_get_routes_resp_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    Opmsg_Source_Sync_Set_Routes

  DESCRIPTION
    Source sync operator message for SET_ROUTES. Sources which do not appear
    in the message are not affected. To remove a route, specify
    sample_rate=0, in this case sink_index and gain are ignored.

  MEMBERS
    message_id    - message id
    number_routes -
    routes        - records of 5 words each: source_index (0-23, unique in
                    message), sink_index (0-23), sample_rate (in units of 25Hz)
                    or 0 to remove a route, gain (in units of 1/60 db),
                    transition_samples (0 for immediate switch)

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_SOURCE_SYNC_SET_ROUTES;

/* The following macros take OPMSG_SOURCE_SYNC_SET_ROUTES *opmsg_source_sync_set_routes_ptr */
#define OPMSG_SOURCE_SYNC_SET_ROUTES_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SOURCE_SYNC_SET_ROUTES_MESSAGE_ID_GET(opmsg_source_sync_set_routes_ptr) ((OPMSG_SRC_SYNC_ID)(opmsg_source_sync_set_routes_ptr)->_data[0])
#define OPMSG_SOURCE_SYNC_SET_ROUTES_MESSAGE_ID_SET(opmsg_source_sync_set_routes_ptr, message_id) ((opmsg_source_sync_set_routes_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SOURCE_SYNC_SET_ROUTES_NUMBER_ROUTES_WORD_OFFSET (1)
#define OPMSG_SOURCE_SYNC_SET_ROUTES_NUMBER_ROUTES_GET(opmsg_source_sync_set_routes_ptr) ((opmsg_source_sync_set_routes_ptr)->_data[1])
#define OPMSG_SOURCE_SYNC_SET_ROUTES_NUMBER_ROUTES_SET(opmsg_source_sync_set_routes_ptr, number_routes) ((opmsg_source_sync_set_routes_ptr)->_data[1] = (uint16)(number_routes))
#define OPMSG_SOURCE_SYNC_SET_ROUTES_ROUTES_WORD_OFFSET (2)
#define OPMSG_SOURCE_SYNC_SET_ROUTES_ROUTES_GET(opmsg_source_sync_set_routes_ptr) ((opmsg_source_sync_set_routes_ptr)->_data[2])
#define OPMSG_SOURCE_SYNC_SET_ROUTES_ROUTES_SET(opmsg_source_sync_set_routes_ptr, routes) ((opmsg_source_sync_set_routes_ptr)->_data[2] = (uint16)(routes))
#define OPMSG_SOURCE_SYNC_SET_ROUTES_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SOURCE_SYNC_SET_ROUTES_CREATE(message_id, number_routes, routes) \
    (uint16)(message_id), \
    (uint16)(number_routes), \
    (uint16)(routes)
#define OPMSG_SOURCE_SYNC_SET_ROUTES_PACK(opmsg_source_sync_set_routes_ptr, message_id, number_routes, routes) \
    do { \
        (opmsg_source_sync_set_routes_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_source_sync_set_routes_ptr)->_data[1] = (uint16)((uint16)(number_routes)); \
        (opmsg_source_sync_set_routes_ptr)->_data[2] = (uint16)((uint16)(routes)); \
    } while (0)

#define OPMSG_SOURCE_SYNC_SET_ROUTES_MARSHALL(addr, opmsg_source_sync_set_routes_ptr) memcpy((void *)(addr), (void *)(opmsg_source_sync_set_routes_ptr), 3)
#define OPMSG_SOURCE_SYNC_SET_ROUTES_UNMARSHALL(addr, opmsg_source_sync_set_routes_ptr) memcpy((void *)(opmsg_source_sync_set_routes_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    Opmsg_Source_Sync_Set_Sink_Groups

  DESCRIPTION
    Source sync operator message for SET_SINK_GROUPS. The list of groups in
    the message replaces any previously configured sink groups.<br> Every
    sink which is connected when the message is processed should appear in
    one of the groups. Sink indices which do not appear in a specified group
    are placed in an automatically created fallback group, and a warning will
    be written to the log. Likewise if a sink is connected later, and its
    index is not covered by a sink group, it will be added to the fallback
    group and a warning will be issued.<br> If any bit position is set in
    more than one group, the entire message is ignored.

  MEMBERS
    message_id    - message id
    number_groups - Number of groups in this message
    groups        - Bitmasks of sink indices for each group, 32 bit in 2 16-bit
                    words, MSW first:<br> Bit 31: Metadata enabled,<br> Bits
                    0..23: Sink terminal membership, bit n corresponds to sink
                    terminal n.

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_SOURCE_SYNC_SET_SINK_GROUPS;

/* The following macros take OPMSG_SOURCE_SYNC_SET_SINK_GROUPS *opmsg_source_sync_set_sink_groups_ptr */
#define OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_MESSAGE_ID_GET(opmsg_source_sync_set_sink_groups_ptr) ((OPMSG_SRC_SYNC_ID)(opmsg_source_sync_set_sink_groups_ptr)->_data[0])
#define OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_MESSAGE_ID_SET(opmsg_source_sync_set_sink_groups_ptr, message_id) ((opmsg_source_sync_set_sink_groups_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_NUMBER_GROUPS_WORD_OFFSET (1)
#define OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_NUMBER_GROUPS_GET(opmsg_source_sync_set_sink_groups_ptr) ((opmsg_source_sync_set_sink_groups_ptr)->_data[1])
#define OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_NUMBER_GROUPS_SET(opmsg_source_sync_set_sink_groups_ptr, number_groups) ((opmsg_source_sync_set_sink_groups_ptr)->_data[1] = (uint16)(number_groups))
#define OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_GROUPS_WORD_OFFSET (2)
#define OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_GROUPS_GET(opmsg_source_sync_set_sink_groups_ptr)  \
    (((uint32)((opmsg_source_sync_set_sink_groups_ptr)->_data[2]) | \
      ((uint32)((opmsg_source_sync_set_sink_groups_ptr)->_data[3]) << 16)))
#define OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_GROUPS_SET(opmsg_source_sync_set_sink_groups_ptr, groups) do { \
        (opmsg_source_sync_set_sink_groups_ptr)->_data[2] = (uint16)((groups) & 0xffff); \
        (opmsg_source_sync_set_sink_groups_ptr)->_data[3] = (uint16)((groups) >> 16); } while (0)
#define OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_CREATE(message_id, number_groups, groups) \
    (uint16)(message_id), \
    (uint16)(number_groups), \
    (uint16)((groups) & 0xffff), \
    (uint16)((groups) >> 16)
#define OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_PACK(opmsg_source_sync_set_sink_groups_ptr, message_id, number_groups, groups) \
    do { \
        (opmsg_source_sync_set_sink_groups_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_source_sync_set_sink_groups_ptr)->_data[1] = (uint16)((uint16)(number_groups)); \
        (opmsg_source_sync_set_sink_groups_ptr)->_data[2] = (uint16)((uint16)((groups) & 0xffff)); \
        (opmsg_source_sync_set_sink_groups_ptr)->_data[3] = (uint16)(((groups) >> 16)); \
    } while (0)

#define OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_MARSHALL(addr, opmsg_source_sync_set_sink_groups_ptr) memcpy((void *)(addr), (void *)(opmsg_source_sync_set_sink_groups_ptr), 4)
#define OPMSG_SOURCE_SYNC_SET_SINK_GROUPS_UNMARSHALL(addr, opmsg_source_sync_set_sink_groups_ptr) memcpy((void *)(opmsg_source_sync_set_sink_groups_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    Opmsg_Source_Sync_Set_Source_Groups

  DESCRIPTION
    Source sync operator message for SET_SOURCE_GROUPS. The list of groups in
    the message replaces any previously configured source groups.<br> Source
    groups have a timing property which can be strict or stallable. Full
    downstream buffers on any connected channel in any strict timing source
    group will hold up any data flow through the operator. On the other hand,
    full downstream buffers on connected channels in a stallable source group
    will cause only data to channels in this group to be held up, which may
    cause these channels to lose some data. There must be at least one strict
    timing source group. Metadata may not be supported on stallable source
    groups.<br> All strict timing source groups receive the same amount of
    data in terms of time. Stallable source groups receive the same amount
    unless they are detected as stalled.<br> Each source group receives one
    shared stream of metadata. The operator will only forward metadata to one
    of the source channels in a group.<br> If any bit position is set in more
    than one group, the entire message is ignored.

  MEMBERS
    message_id    - message id
    number_groups - Number of groups in this message
    groups        - Bitmasks of sink indices for each group, 32 bit in 2 16-bit
                    words, MSW first:<br> Bit 31: Metadata enabled (on targets
                    which support it),<br> Bit 30: TTP enabled (on targets which
                    support it),<br> Bit 29: Stallable,<br> Bits 0..23: Source
                    terminal membership, bit n corresponds to source terminal n.

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS;

/* The following macros take OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS *opmsg_source_sync_set_source_groups_ptr */
#define OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_MESSAGE_ID_GET(opmsg_source_sync_set_source_groups_ptr) ((OPMSG_SRC_SYNC_ID)(opmsg_source_sync_set_source_groups_ptr)->_data[0])
#define OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_MESSAGE_ID_SET(opmsg_source_sync_set_source_groups_ptr, message_id) ((opmsg_source_sync_set_source_groups_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_NUMBER_GROUPS_WORD_OFFSET (1)
#define OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_NUMBER_GROUPS_GET(opmsg_source_sync_set_source_groups_ptr) ((opmsg_source_sync_set_source_groups_ptr)->_data[1])
#define OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_NUMBER_GROUPS_SET(opmsg_source_sync_set_source_groups_ptr, number_groups) ((opmsg_source_sync_set_source_groups_ptr)->_data[1] = (uint16)(number_groups))
#define OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_GROUPS_WORD_OFFSET (2)
#define OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_GROUPS_GET(opmsg_source_sync_set_source_groups_ptr)  \
    (((uint32)((opmsg_source_sync_set_source_groups_ptr)->_data[2]) | \
      ((uint32)((opmsg_source_sync_set_source_groups_ptr)->_data[3]) << 16)))
#define OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_GROUPS_SET(opmsg_source_sync_set_source_groups_ptr, groups) do { \
        (opmsg_source_sync_set_source_groups_ptr)->_data[2] = (uint16)((groups) & 0xffff); \
        (opmsg_source_sync_set_source_groups_ptr)->_data[3] = (uint16)((groups) >> 16); } while (0)
#define OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_CREATE(message_id, number_groups, groups) \
    (uint16)(message_id), \
    (uint16)(number_groups), \
    (uint16)((groups) & 0xffff), \
    (uint16)((groups) >> 16)
#define OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_PACK(opmsg_source_sync_set_source_groups_ptr, message_id, number_groups, groups) \
    do { \
        (opmsg_source_sync_set_source_groups_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_source_sync_set_source_groups_ptr)->_data[1] = (uint16)((uint16)(number_groups)); \
        (opmsg_source_sync_set_source_groups_ptr)->_data[2] = (uint16)((uint16)((groups) & 0xffff)); \
        (opmsg_source_sync_set_source_groups_ptr)->_data[3] = (uint16)(((groups) >> 16)); \
    } while (0)

#define OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_MARSHALL(addr, opmsg_source_sync_set_source_groups_ptr) memcpy((void *)(addr), (void *)(opmsg_source_sync_set_source_groups_ptr), 4)
#define OPMSG_SOURCE_SYNC_SET_SOURCE_GROUPS_UNMARSHALL(addr, opmsg_source_sync_set_source_groups_ptr) memcpy((void *)(opmsg_source_sync_set_source_groups_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    Opmsg_Spdif_Decode_Set_Decoder_Data_Type

  DESCRIPTION
    Tells the operator which data type can be decoded by currently loaded
    decoder.

  MEMBERS
    message_id            - message id
    decoder_data_type     - decoder data type
    decoder_opid          - opid of the decoder
    decoder_nrof_channels - decoder's numbe of output channels

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE;

/* The following macros take OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE *opmsg_spdif_decode_set_decoder_data_type_ptr */
#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_MESSAGE_ID_GET(opmsg_spdif_decode_set_decoder_data_type_ptr) ((OPMSG_SPDIF_DECODE_ID)(opmsg_spdif_decode_set_decoder_data_type_ptr)->_data[0])
#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_MESSAGE_ID_SET(opmsg_spdif_decode_set_decoder_data_type_ptr, message_id) ((opmsg_spdif_decode_set_decoder_data_type_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_DECODER_DATA_TYPE_WORD_OFFSET (1)
#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_DECODER_DATA_TYPE_GET(opmsg_spdif_decode_set_decoder_data_type_ptr) ((opmsg_spdif_decode_set_decoder_data_type_ptr)->_data[1])
#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_DECODER_DATA_TYPE_SET(opmsg_spdif_decode_set_decoder_data_type_ptr, decoder_data_type) ((opmsg_spdif_decode_set_decoder_data_type_ptr)->_data[1] = (uint16)(decoder_data_type))
#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_DECODER_OPID_WORD_OFFSET (2)
#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_DECODER_OPID_GET(opmsg_spdif_decode_set_decoder_data_type_ptr) ((opmsg_spdif_decode_set_decoder_data_type_ptr)->_data[2])
#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_DECODER_OPID_SET(opmsg_spdif_decode_set_decoder_data_type_ptr, decoder_opid) ((opmsg_spdif_decode_set_decoder_data_type_ptr)->_data[2] = (uint16)(decoder_opid))
#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_DECODER_NROF_CHANNELS_WORD_OFFSET (3)
#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_DECODER_NROF_CHANNELS_GET(opmsg_spdif_decode_set_decoder_data_type_ptr) ((opmsg_spdif_decode_set_decoder_data_type_ptr)->_data[3])
#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_DECODER_NROF_CHANNELS_SET(opmsg_spdif_decode_set_decoder_data_type_ptr, decoder_nrof_channels) ((opmsg_spdif_decode_set_decoder_data_type_ptr)->_data[3] = (uint16)(decoder_nrof_channels))
#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_CREATE(message_id, decoder_data_type, decoder_opid, decoder_nrof_channels) \
    (uint16)(message_id), \
    (uint16)(decoder_data_type), \
    (uint16)(decoder_opid), \
    (uint16)(decoder_nrof_channels)
#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_PACK(opmsg_spdif_decode_set_decoder_data_type_ptr, message_id, decoder_data_type, decoder_opid, decoder_nrof_channels) \
    do { \
        (opmsg_spdif_decode_set_decoder_data_type_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_spdif_decode_set_decoder_data_type_ptr)->_data[1] = (uint16)((uint16)(decoder_data_type)); \
        (opmsg_spdif_decode_set_decoder_data_type_ptr)->_data[2] = (uint16)((uint16)(decoder_opid)); \
        (opmsg_spdif_decode_set_decoder_data_type_ptr)->_data[3] = (uint16)((uint16)(decoder_nrof_channels)); \
    } while (0)

#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_MARSHALL(addr, opmsg_spdif_decode_set_decoder_data_type_ptr) memcpy((void *)(addr), (void *)(opmsg_spdif_decode_set_decoder_data_type_ptr), 4)
#define OPMSG_SPDIF_DECODE_SET_DECODER_DATA_TYPE_UNMARSHALL(addr, opmsg_spdif_decode_set_decoder_data_type_ptr) memcpy((void *)(opmsg_spdif_decode_set_decoder_data_type_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    Opmsg_Spdif_Decode_Set_Output_Rate

  DESCRIPTION
    Sets the expected output rate, This is normally sent in response to
    message ADAPT_TO_NEW_RATE from the driver, and tells the operator that
    the system is ready for the new rate

  MEMBERS
    message_id  - message id
    output_rate - output rate in 25Hz units

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_SPDIF_DECODE_SET_OUTPUT_RATE;

/* The following macros take OPMSG_SPDIF_DECODE_SET_OUTPUT_RATE *opmsg_spdif_decode_set_output_rate_ptr */
#define OPMSG_SPDIF_DECODE_SET_OUTPUT_RATE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPDIF_DECODE_SET_OUTPUT_RATE_MESSAGE_ID_GET(opmsg_spdif_decode_set_output_rate_ptr) ((OPMSG_SPDIF_DECODE_ID)(opmsg_spdif_decode_set_output_rate_ptr)->_data[0])
#define OPMSG_SPDIF_DECODE_SET_OUTPUT_RATE_MESSAGE_ID_SET(opmsg_spdif_decode_set_output_rate_ptr, message_id) ((opmsg_spdif_decode_set_output_rate_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPDIF_DECODE_SET_OUTPUT_RATE_OUTPUT_RATE_WORD_OFFSET (1)
#define OPMSG_SPDIF_DECODE_SET_OUTPUT_RATE_OUTPUT_RATE_GET(opmsg_spdif_decode_set_output_rate_ptr) ((opmsg_spdif_decode_set_output_rate_ptr)->_data[1])
#define OPMSG_SPDIF_DECODE_SET_OUTPUT_RATE_OUTPUT_RATE_SET(opmsg_spdif_decode_set_output_rate_ptr, output_rate) ((opmsg_spdif_decode_set_output_rate_ptr)->_data[1] = (uint16)(output_rate))
#define OPMSG_SPDIF_DECODE_SET_OUTPUT_RATE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPDIF_DECODE_SET_OUTPUT_RATE_CREATE(message_id, output_rate) \
    (uint16)(message_id), \
    (uint16)(output_rate)
#define OPMSG_SPDIF_DECODE_SET_OUTPUT_RATE_PACK(opmsg_spdif_decode_set_output_rate_ptr, message_id, output_rate) \
    do { \
        (opmsg_spdif_decode_set_output_rate_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_spdif_decode_set_output_rate_ptr)->_data[1] = (uint16)((uint16)(output_rate)); \
    } while (0)

#define OPMSG_SPDIF_DECODE_SET_OUTPUT_RATE_MARSHALL(addr, opmsg_spdif_decode_set_output_rate_ptr) memcpy((void *)(addr), (void *)(opmsg_spdif_decode_set_output_rate_ptr), 2)
#define OPMSG_SPDIF_DECODE_SET_OUTPUT_RATE_UNMARSHALL(addr, opmsg_spdif_decode_set_output_rate_ptr) memcpy((void *)(opmsg_spdif_decode_set_output_rate_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Spdif_Decode_Set_Supported_Data_Types

  DESCRIPTION
    Use this to set the coded data types that are supported by the s/pdif rx
    driver, by default no coded data type will be supported.

  MEMBERS
    message_id                  - message id
    supported_data_types_group1 - bitmap containing first 16 supported data
                                  types
    supported_data_types_group2 - bitmap containing second 16 supported data
                                  types
    user_data_type_period       - user specific data type period (for test only)

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES;

/* The following macros take OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES *opmsg_spdif_decode_set_supported_data_types_ptr */
#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_MESSAGE_ID_GET(opmsg_spdif_decode_set_supported_data_types_ptr) ((OPMSG_SPDIF_DECODE_ID)(opmsg_spdif_decode_set_supported_data_types_ptr)->_data[0])
#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_MESSAGE_ID_SET(opmsg_spdif_decode_set_supported_data_types_ptr, message_id) ((opmsg_spdif_decode_set_supported_data_types_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_SUPPORTED_DATA_TYPES_GROUP1_WORD_OFFSET (1)
#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_SUPPORTED_DATA_TYPES_GROUP1_GET(opmsg_spdif_decode_set_supported_data_types_ptr) ((opmsg_spdif_decode_set_supported_data_types_ptr)->_data[1])
#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_SUPPORTED_DATA_TYPES_GROUP1_SET(opmsg_spdif_decode_set_supported_data_types_ptr, supported_data_types_group1) ((opmsg_spdif_decode_set_supported_data_types_ptr)->_data[1] = (uint16)(supported_data_types_group1))
#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_SUPPORTED_DATA_TYPES_GROUP2_WORD_OFFSET (2)
#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_SUPPORTED_DATA_TYPES_GROUP2_GET(opmsg_spdif_decode_set_supported_data_types_ptr) ((opmsg_spdif_decode_set_supported_data_types_ptr)->_data[2])
#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_SUPPORTED_DATA_TYPES_GROUP2_SET(opmsg_spdif_decode_set_supported_data_types_ptr, supported_data_types_group2) ((opmsg_spdif_decode_set_supported_data_types_ptr)->_data[2] = (uint16)(supported_data_types_group2))
#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_USER_DATA_TYPE_PERIOD_WORD_OFFSET (3)
#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_USER_DATA_TYPE_PERIOD_GET(opmsg_spdif_decode_set_supported_data_types_ptr) ((opmsg_spdif_decode_set_supported_data_types_ptr)->_data[3])
#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_USER_DATA_TYPE_PERIOD_SET(opmsg_spdif_decode_set_supported_data_types_ptr, user_data_type_period) ((opmsg_spdif_decode_set_supported_data_types_ptr)->_data[3] = (uint16)(user_data_type_period))
#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_CREATE(message_id, supported_data_types_group1, supported_data_types_group2, user_data_type_period) \
    (uint16)(message_id), \
    (uint16)(supported_data_types_group1), \
    (uint16)(supported_data_types_group2), \
    (uint16)(user_data_type_period)
#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_PACK(opmsg_spdif_decode_set_supported_data_types_ptr, message_id, supported_data_types_group1, supported_data_types_group2, user_data_type_period) \
    do { \
        (opmsg_spdif_decode_set_supported_data_types_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_spdif_decode_set_supported_data_types_ptr)->_data[1] = (uint16)((uint16)(supported_data_types_group1)); \
        (opmsg_spdif_decode_set_supported_data_types_ptr)->_data[2] = (uint16)((uint16)(supported_data_types_group2)); \
        (opmsg_spdif_decode_set_supported_data_types_ptr)->_data[3] = (uint16)((uint16)(user_data_type_period)); \
    } while (0)

#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_MARSHALL(addr, opmsg_spdif_decode_set_supported_data_types_ptr) memcpy((void *)(addr), (void *)(opmsg_spdif_decode_set_supported_data_types_ptr), 4)
#define OPMSG_SPDIF_DECODE_SET_SUPPORTED_DATA_TYPES_UNMARSHALL(addr, opmsg_spdif_decode_set_supported_data_types_ptr) memcpy((void *)(opmsg_spdif_decode_set_supported_data_types_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    Opmsg_Splitter_Activate_Streams

  DESCRIPTION
    Splitter operator message to set which outputs are active.

  MEMBERS
    message_id - message id
    streams    - Bit field indicating the streams to activate. Bit n = stream n

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_SPLITTER_ACTIVATE_STREAMS;

/* The following macros take OPMSG_SPLITTER_ACTIVATE_STREAMS *opmsg_splitter_activate_streams_ptr */
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_MESSAGE_ID_GET(opmsg_splitter_activate_streams_ptr) ((OPMSG_SPLITTER_ID)(opmsg_splitter_activate_streams_ptr)->_data[0])
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_MESSAGE_ID_SET(opmsg_splitter_activate_streams_ptr, message_id) ((opmsg_splitter_activate_streams_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_STREAMS_WORD_OFFSET (1)
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_STREAMS_GET(opmsg_splitter_activate_streams_ptr) ((opmsg_splitter_activate_streams_ptr)->_data[1])
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_STREAMS_SET(opmsg_splitter_activate_streams_ptr, streams) ((opmsg_splitter_activate_streams_ptr)->_data[1] = (uint16)(streams))
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_CREATE(message_id, streams) \
    (uint16)(message_id), \
    (uint16)(streams)
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_PACK(opmsg_splitter_activate_streams_ptr, message_id, streams) \
    do { \
        (opmsg_splitter_activate_streams_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_splitter_activate_streams_ptr)->_data[1] = (uint16)((uint16)(streams)); \
    } while (0)

#define OPMSG_SPLITTER_ACTIVATE_STREAMS_MARSHALL(addr, opmsg_splitter_activate_streams_ptr) memcpy((void *)(addr), (void *)(opmsg_splitter_activate_streams_ptr), 2)
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_UNMARSHALL(addr, opmsg_splitter_activate_streams_ptr) memcpy((void *)(opmsg_splitter_activate_streams_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Splitter_Activate_Streams_After_Timestamp

  DESCRIPTION
    Splitter operator message to activate streams after a given timestamp.

  MEMBERS
    message_id   - message id
    streams      - Bit field indicating the streams to activate after timestamp.
                   Bit n = stream n
    timestamp_ms - Most-significant 16 bits of the timestamp, in microseconds
    timestamp_ls - Least-significant 16 bits of the timestamp, in microseconds

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP;

/* The following macros take OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP *opmsg_splitter_activate_streams_after_timestamp_ptr */
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_MESSAGE_ID_GET(opmsg_splitter_activate_streams_after_timestamp_ptr) ((OPMSG_SPLITTER_ID)(opmsg_splitter_activate_streams_after_timestamp_ptr)->_data[0])
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_MESSAGE_ID_SET(opmsg_splitter_activate_streams_after_timestamp_ptr, message_id) ((opmsg_splitter_activate_streams_after_timestamp_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_STREAMS_WORD_OFFSET (1)
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_STREAMS_GET(opmsg_splitter_activate_streams_after_timestamp_ptr) ((opmsg_splitter_activate_streams_after_timestamp_ptr)->_data[1])
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_STREAMS_SET(opmsg_splitter_activate_streams_after_timestamp_ptr, streams) ((opmsg_splitter_activate_streams_after_timestamp_ptr)->_data[1] = (uint16)(streams))
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_TIMESTAMP_MS_WORD_OFFSET (2)
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_TIMESTAMP_MS_GET(opmsg_splitter_activate_streams_after_timestamp_ptr) ((opmsg_splitter_activate_streams_after_timestamp_ptr)->_data[2])
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_TIMESTAMP_MS_SET(opmsg_splitter_activate_streams_after_timestamp_ptr, timestamp_ms) ((opmsg_splitter_activate_streams_after_timestamp_ptr)->_data[2] = (uint16)(timestamp_ms))
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_TIMESTAMP_LS_WORD_OFFSET (3)
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_TIMESTAMP_LS_GET(opmsg_splitter_activate_streams_after_timestamp_ptr) ((opmsg_splitter_activate_streams_after_timestamp_ptr)->_data[3])
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_TIMESTAMP_LS_SET(opmsg_splitter_activate_streams_after_timestamp_ptr, timestamp_ls) ((opmsg_splitter_activate_streams_after_timestamp_ptr)->_data[3] = (uint16)(timestamp_ls))
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_CREATE(message_id, streams, timestamp_ms, timestamp_ls) \
    (uint16)(message_id), \
    (uint16)(streams), \
    (uint16)(timestamp_ms), \
    (uint16)(timestamp_ls)
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_PACK(opmsg_splitter_activate_streams_after_timestamp_ptr, message_id, streams, timestamp_ms, timestamp_ls) \
    do { \
        (opmsg_splitter_activate_streams_after_timestamp_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_splitter_activate_streams_after_timestamp_ptr)->_data[1] = (uint16)((uint16)(streams)); \
        (opmsg_splitter_activate_streams_after_timestamp_ptr)->_data[2] = (uint16)((uint16)(timestamp_ms)); \
        (opmsg_splitter_activate_streams_after_timestamp_ptr)->_data[3] = (uint16)((uint16)(timestamp_ls)); \
    } while (0)

#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_MARSHALL(addr, opmsg_splitter_activate_streams_after_timestamp_ptr) memcpy((void *)(addr), (void *)(opmsg_splitter_activate_streams_after_timestamp_ptr), 4)
#define OPMSG_SPLITTER_ACTIVATE_STREAMS_AFTER_TIMESTAMP_UNMARSHALL(addr, opmsg_splitter_activate_streams_after_timestamp_ptr) memcpy((void *)(opmsg_splitter_activate_streams_after_timestamp_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    Opmsg_Splitter_Buffer_Streams

  DESCRIPTION
    Splitter operator message for BUFFER_STREAMS.

  MEMBERS
    message_id - message id
    streams    - Bit field indicating the streams to deactivate. Bit n = stream
                 n

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_SPLITTER_BUFFER_STREAMS;

/* The following macros take OPMSG_SPLITTER_BUFFER_STREAMS *opmsg_splitter_buffer_streams_ptr */
#define OPMSG_SPLITTER_BUFFER_STREAMS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPLITTER_BUFFER_STREAMS_MESSAGE_ID_GET(opmsg_splitter_buffer_streams_ptr) ((OPMSG_SPLITTER_ID)(opmsg_splitter_buffer_streams_ptr)->_data[0])
#define OPMSG_SPLITTER_BUFFER_STREAMS_MESSAGE_ID_SET(opmsg_splitter_buffer_streams_ptr, message_id) ((opmsg_splitter_buffer_streams_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPLITTER_BUFFER_STREAMS_STREAMS_WORD_OFFSET (1)
#define OPMSG_SPLITTER_BUFFER_STREAMS_STREAMS_GET(opmsg_splitter_buffer_streams_ptr) ((opmsg_splitter_buffer_streams_ptr)->_data[1])
#define OPMSG_SPLITTER_BUFFER_STREAMS_STREAMS_SET(opmsg_splitter_buffer_streams_ptr, streams) ((opmsg_splitter_buffer_streams_ptr)->_data[1] = (uint16)(streams))
#define OPMSG_SPLITTER_BUFFER_STREAMS_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPLITTER_BUFFER_STREAMS_CREATE(message_id, streams) \
    (uint16)(message_id), \
    (uint16)(streams)
#define OPMSG_SPLITTER_BUFFER_STREAMS_PACK(opmsg_splitter_buffer_streams_ptr, message_id, streams) \
    do { \
        (opmsg_splitter_buffer_streams_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_splitter_buffer_streams_ptr)->_data[1] = (uint16)((uint16)(streams)); \
    } while (0)

#define OPMSG_SPLITTER_BUFFER_STREAMS_MARSHALL(addr, opmsg_splitter_buffer_streams_ptr) memcpy((void *)(addr), (void *)(opmsg_splitter_buffer_streams_ptr), 2)
#define OPMSG_SPLITTER_BUFFER_STREAMS_UNMARSHALL(addr, opmsg_splitter_buffer_streams_ptr) memcpy((void *)(opmsg_splitter_buffer_streams_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Splitter_Deactivate_Streams

  DESCRIPTION
    Splitter operator message for DEACTIVATE_STREAMS.

  MEMBERS
    message_id - message id
    streams    - Bit field indicating the streams to deactivate. Bit n = stream
                 n

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_SPLITTER_DEACTIVATE_STREAMS;

/* The following macros take OPMSG_SPLITTER_DEACTIVATE_STREAMS *opmsg_splitter_deactivate_streams_ptr */
#define OPMSG_SPLITTER_DEACTIVATE_STREAMS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPLITTER_DEACTIVATE_STREAMS_MESSAGE_ID_GET(opmsg_splitter_deactivate_streams_ptr) ((OPMSG_SPLITTER_ID)(opmsg_splitter_deactivate_streams_ptr)->_data[0])
#define OPMSG_SPLITTER_DEACTIVATE_STREAMS_MESSAGE_ID_SET(opmsg_splitter_deactivate_streams_ptr, message_id) ((opmsg_splitter_deactivate_streams_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPLITTER_DEACTIVATE_STREAMS_STREAMS_WORD_OFFSET (1)
#define OPMSG_SPLITTER_DEACTIVATE_STREAMS_STREAMS_GET(opmsg_splitter_deactivate_streams_ptr) ((opmsg_splitter_deactivate_streams_ptr)->_data[1])
#define OPMSG_SPLITTER_DEACTIVATE_STREAMS_STREAMS_SET(opmsg_splitter_deactivate_streams_ptr, streams) ((opmsg_splitter_deactivate_streams_ptr)->_data[1] = (uint16)(streams))
#define OPMSG_SPLITTER_DEACTIVATE_STREAMS_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPLITTER_DEACTIVATE_STREAMS_CREATE(message_id, streams) \
    (uint16)(message_id), \
    (uint16)(streams)
#define OPMSG_SPLITTER_DEACTIVATE_STREAMS_PACK(opmsg_splitter_deactivate_streams_ptr, message_id, streams) \
    do { \
        (opmsg_splitter_deactivate_streams_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_splitter_deactivate_streams_ptr)->_data[1] = (uint16)((uint16)(streams)); \
    } while (0)

#define OPMSG_SPLITTER_DEACTIVATE_STREAMS_MARSHALL(addr, opmsg_splitter_deactivate_streams_ptr) memcpy((void *)(addr), (void *)(opmsg_splitter_deactivate_streams_ptr), 2)
#define OPMSG_SPLITTER_DEACTIVATE_STREAMS_UNMARSHALL(addr, opmsg_splitter_deactivate_streams_ptr) memcpy((void *)(opmsg_splitter_deactivate_streams_ptr), (void *)(addr), 2)


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

#define OPMSG_SPLITTER_SET_DATA_FORMAT_MARSHALL(addr, opmsg_splitter_set_data_format_ptr) memcpy((void *)(addr), (void *)(opmsg_splitter_set_data_format_ptr), 2)
#define OPMSG_SPLITTER_SET_DATA_FORMAT_UNMARSHALL(addr, opmsg_splitter_set_data_format_ptr) memcpy((void *)(opmsg_splitter_set_data_format_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Splitter_Set_Location

  DESCRIPTION
    Splitter operator message for SET_BUFFRING_LOCATION.

  MEMBERS
    message_id - message id
    location   - The following locations are supported:
                 OPMSG_SPLITTER_INTERNALLY - The buffered data is saved locally.
                 This is the default when OPMSG_SPLITTER_BUFFER_DATA is
                 activated. OPMSG_SPLITTER_SRAM - The data is sent to an
                 external SRAM.

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_SPLITTER_SET_LOCATION;

/* The following macros take OPMSG_SPLITTER_SET_LOCATION *opmsg_splitter_set_location_ptr */
#define OPMSG_SPLITTER_SET_LOCATION_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPLITTER_SET_LOCATION_MESSAGE_ID_GET(opmsg_splitter_set_location_ptr) ((OPMSG_SPLITTER_ID)(opmsg_splitter_set_location_ptr)->_data[0])
#define OPMSG_SPLITTER_SET_LOCATION_MESSAGE_ID_SET(opmsg_splitter_set_location_ptr, message_id) ((opmsg_splitter_set_location_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPLITTER_SET_LOCATION_LOCATION_WORD_OFFSET (1)
#define OPMSG_SPLITTER_SET_LOCATION_LOCATION_GET(opmsg_splitter_set_location_ptr) ((opmsg_splitter_set_location_ptr)->_data[1])
#define OPMSG_SPLITTER_SET_LOCATION_LOCATION_SET(opmsg_splitter_set_location_ptr, location) ((opmsg_splitter_set_location_ptr)->_data[1] = (uint16)(location))
#define OPMSG_SPLITTER_SET_LOCATION_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPLITTER_SET_LOCATION_CREATE(message_id, location) \
    (uint16)(message_id), \
    (uint16)(location)
#define OPMSG_SPLITTER_SET_LOCATION_PACK(opmsg_splitter_set_location_ptr, message_id, location) \
    do { \
        (opmsg_splitter_set_location_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_splitter_set_location_ptr)->_data[1] = (uint16)((uint16)(location)); \
    } while (0)

#define OPMSG_SPLITTER_SET_LOCATION_MARSHALL(addr, opmsg_splitter_set_location_ptr) memcpy((void *)(addr), (void *)(opmsg_splitter_set_location_ptr), 2)
#define OPMSG_SPLITTER_SET_LOCATION_UNMARSHALL(addr, opmsg_splitter_set_location_ptr) memcpy((void *)(opmsg_splitter_set_location_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Splitter_Set_Packing

  DESCRIPTION
    Splitter operator message for SET_PACKING. Packing can only be set if the
    data format is audio AUDIO_DATA_FORMAT_FIXP or
    AUDIO_DATA_FORMAT_FIXP_WITH_METADATA.

  MEMBERS
    message_id - message id
    packing    - The following locations are supported: OPMSG_SPLITTER_UNPACKED
                 - The audio samples are saved without losing precision.
                 OPMSG_SPLITTER_PACKED - Only the MSB 16 bit of the audio sample
                 is saved.

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_SPLITTER_SET_PACKING;

/* The following macros take OPMSG_SPLITTER_SET_PACKING *opmsg_splitter_set_packing_ptr */
#define OPMSG_SPLITTER_SET_PACKING_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPLITTER_SET_PACKING_MESSAGE_ID_GET(opmsg_splitter_set_packing_ptr) ((OPMSG_SPLITTER_ID)(opmsg_splitter_set_packing_ptr)->_data[0])
#define OPMSG_SPLITTER_SET_PACKING_MESSAGE_ID_SET(opmsg_splitter_set_packing_ptr, message_id) ((opmsg_splitter_set_packing_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPLITTER_SET_PACKING_PACKING_WORD_OFFSET (1)
#define OPMSG_SPLITTER_SET_PACKING_PACKING_GET(opmsg_splitter_set_packing_ptr) ((opmsg_splitter_set_packing_ptr)->_data[1])
#define OPMSG_SPLITTER_SET_PACKING_PACKING_SET(opmsg_splitter_set_packing_ptr, packing) ((opmsg_splitter_set_packing_ptr)->_data[1] = (uint16)(packing))
#define OPMSG_SPLITTER_SET_PACKING_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPLITTER_SET_PACKING_CREATE(message_id, packing) \
    (uint16)(message_id), \
    (uint16)(packing)
#define OPMSG_SPLITTER_SET_PACKING_PACK(opmsg_splitter_set_packing_ptr, message_id, packing) \
    do { \
        (opmsg_splitter_set_packing_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_splitter_set_packing_ptr)->_data[1] = (uint16)((uint16)(packing)); \
    } while (0)

#define OPMSG_SPLITTER_SET_PACKING_MARSHALL(addr, opmsg_splitter_set_packing_ptr) memcpy((void *)(addr), (void *)(opmsg_splitter_set_packing_ptr), 2)
#define OPMSG_SPLITTER_SET_PACKING_UNMARSHALL(addr, opmsg_splitter_set_packing_ptr) memcpy((void *)(opmsg_splitter_set_packing_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Splitter_Set_Reframing

  DESCRIPTION
    Splitter operator message for SET_REFRAMING.

  MEMBERS
    message_id - message id
    mode       - The following modes are supported:
                 OPMSG_SPLITTER_REFRAMING_DISABLED - Reframing timestamps are
                 disabled. OPMSG_SPLITTER_REFRAMING_ENABLED - Reframing
                 timestamps are enabled.
    size       - The minimum tag size length in octets if the mode is
                 OPMSG_SPLITTER_REFRAMING_ENABLED.

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_SPLITTER_SET_REFRAMING;

/* The following macros take OPMSG_SPLITTER_SET_REFRAMING *opmsg_splitter_set_reframing_ptr */
#define OPMSG_SPLITTER_SET_REFRAMING_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPLITTER_SET_REFRAMING_MESSAGE_ID_GET(opmsg_splitter_set_reframing_ptr) ((OPMSG_SPLITTER_ID)(opmsg_splitter_set_reframing_ptr)->_data[0])
#define OPMSG_SPLITTER_SET_REFRAMING_MESSAGE_ID_SET(opmsg_splitter_set_reframing_ptr, message_id) ((opmsg_splitter_set_reframing_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPLITTER_SET_REFRAMING_MODE_WORD_OFFSET (1)
#define OPMSG_SPLITTER_SET_REFRAMING_MODE_GET(opmsg_splitter_set_reframing_ptr) ((opmsg_splitter_set_reframing_ptr)->_data[1])
#define OPMSG_SPLITTER_SET_REFRAMING_MODE_SET(opmsg_splitter_set_reframing_ptr, mode) ((opmsg_splitter_set_reframing_ptr)->_data[1] = (uint16)(mode))
#define OPMSG_SPLITTER_SET_REFRAMING_SIZE_WORD_OFFSET (2)
#define OPMSG_SPLITTER_SET_REFRAMING_SIZE_GET(opmsg_splitter_set_reframing_ptr) ((opmsg_splitter_set_reframing_ptr)->_data[2])
#define OPMSG_SPLITTER_SET_REFRAMING_SIZE_SET(opmsg_splitter_set_reframing_ptr, size) ((opmsg_splitter_set_reframing_ptr)->_data[2] = (uint16)(size))
#define OPMSG_SPLITTER_SET_REFRAMING_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPLITTER_SET_REFRAMING_CREATE(message_id, mode, size) \
    (uint16)(message_id), \
    (uint16)(mode), \
    (uint16)(size)
#define OPMSG_SPLITTER_SET_REFRAMING_PACK(opmsg_splitter_set_reframing_ptr, message_id, mode, size) \
    do { \
        (opmsg_splitter_set_reframing_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_splitter_set_reframing_ptr)->_data[1] = (uint16)((uint16)(mode)); \
        (opmsg_splitter_set_reframing_ptr)->_data[2] = (uint16)((uint16)(size)); \
    } while (0)

#define OPMSG_SPLITTER_SET_REFRAMING_MARSHALL(addr, opmsg_splitter_set_reframing_ptr) memcpy((void *)(addr), (void *)(opmsg_splitter_set_reframing_ptr), 3)
#define OPMSG_SPLITTER_SET_REFRAMING_UNMARSHALL(addr, opmsg_splitter_set_reframing_ptr) memcpy((void *)(opmsg_splitter_set_reframing_ptr), (void *)(addr), 3)


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

#define OPMSG_SPLITTER_SET_RUNNING_STREAMS_MARSHALL(addr, opmsg_splitter_set_running_streams_ptr) memcpy((void *)(addr), (void *)(opmsg_splitter_set_running_streams_ptr), 2)
#define OPMSG_SPLITTER_SET_RUNNING_STREAMS_UNMARSHALL(addr, opmsg_splitter_set_running_streams_ptr) memcpy((void *)(opmsg_splitter_set_running_streams_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Splitter_Set_Working_Mode

  DESCRIPTION
    Splitter operator message for SET_WORKING_MODE.

  MEMBERS
    message_id   - message id
    working_mode - The following modes are supported:
                   OPMSG_SPLITTER_CLONE_BUFFER - By default the input buffers is
                   cloned at the output. OPMSG_SPLITTER_BUFFER_DATA - The input
                   and output buffers are different while data is saved
                   internally.

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_SPLITTER_SET_WORKING_MODE;

/* The following macros take OPMSG_SPLITTER_SET_WORKING_MODE *opmsg_splitter_set_working_mode_ptr */
#define OPMSG_SPLITTER_SET_WORKING_MODE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SPLITTER_SET_WORKING_MODE_MESSAGE_ID_GET(opmsg_splitter_set_working_mode_ptr) ((OPMSG_SPLITTER_ID)(opmsg_splitter_set_working_mode_ptr)->_data[0])
#define OPMSG_SPLITTER_SET_WORKING_MODE_MESSAGE_ID_SET(opmsg_splitter_set_working_mode_ptr, message_id) ((opmsg_splitter_set_working_mode_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SPLITTER_SET_WORKING_MODE_WORKING_MODE_WORD_OFFSET (1)
#define OPMSG_SPLITTER_SET_WORKING_MODE_WORKING_MODE_GET(opmsg_splitter_set_working_mode_ptr) ((opmsg_splitter_set_working_mode_ptr)->_data[1])
#define OPMSG_SPLITTER_SET_WORKING_MODE_WORKING_MODE_SET(opmsg_splitter_set_working_mode_ptr, working_mode) ((opmsg_splitter_set_working_mode_ptr)->_data[1] = (uint16)(working_mode))
#define OPMSG_SPLITTER_SET_WORKING_MODE_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SPLITTER_SET_WORKING_MODE_CREATE(message_id, working_mode) \
    (uint16)(message_id), \
    (uint16)(working_mode)
#define OPMSG_SPLITTER_SET_WORKING_MODE_PACK(opmsg_splitter_set_working_mode_ptr, message_id, working_mode) \
    do { \
        (opmsg_splitter_set_working_mode_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_splitter_set_working_mode_ptr)->_data[1] = (uint16)((uint16)(working_mode)); \
    } while (0)

#define OPMSG_SPLITTER_SET_WORKING_MODE_MARSHALL(addr, opmsg_splitter_set_working_mode_ptr) memcpy((void *)(addr), (void *)(opmsg_splitter_set_working_mode_ptr), 2)
#define OPMSG_SPLITTER_SET_WORKING_MODE_UNMARSHALL(addr, opmsg_splitter_set_working_mode_ptr) memcpy((void *)(opmsg_splitter_set_working_mode_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Src_Sync_Set_Trace_Enable

  DESCRIPTION
    SRC_SYNC operator message for SET_TRACE_ENABLE.

  MEMBERS
    message_id       - message id
    trace_enable_msb - MSB of trace enable
    trace_enable_lsb - LSB of trace enable

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_SRC_SYNC_SET_TRACE_ENABLE;

/* The following macros take OPMSG_SRC_SYNC_SET_TRACE_ENABLE *opmsg_src_sync_set_trace_enable_ptr */
#define OPMSG_SRC_SYNC_SET_TRACE_ENABLE_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_SRC_SYNC_SET_TRACE_ENABLE_MESSAGE_ID_GET(opmsg_src_sync_set_trace_enable_ptr) ((OPMSG_SRC_SYNC_ID)(opmsg_src_sync_set_trace_enable_ptr)->_data[0])
#define OPMSG_SRC_SYNC_SET_TRACE_ENABLE_MESSAGE_ID_SET(opmsg_src_sync_set_trace_enable_ptr, message_id) ((opmsg_src_sync_set_trace_enable_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_SRC_SYNC_SET_TRACE_ENABLE_TRACE_ENABLE_MSB_WORD_OFFSET (1)
#define OPMSG_SRC_SYNC_SET_TRACE_ENABLE_TRACE_ENABLE_MSB_GET(opmsg_src_sync_set_trace_enable_ptr) ((opmsg_src_sync_set_trace_enable_ptr)->_data[1])
#define OPMSG_SRC_SYNC_SET_TRACE_ENABLE_TRACE_ENABLE_MSB_SET(opmsg_src_sync_set_trace_enable_ptr, trace_enable_msb) ((opmsg_src_sync_set_trace_enable_ptr)->_data[1] = (uint16)(trace_enable_msb))
#define OPMSG_SRC_SYNC_SET_TRACE_ENABLE_TRACE_ENABLE_LSB_WORD_OFFSET (2)
#define OPMSG_SRC_SYNC_SET_TRACE_ENABLE_TRACE_ENABLE_LSB_GET(opmsg_src_sync_set_trace_enable_ptr) ((opmsg_src_sync_set_trace_enable_ptr)->_data[2])
#define OPMSG_SRC_SYNC_SET_TRACE_ENABLE_TRACE_ENABLE_LSB_SET(opmsg_src_sync_set_trace_enable_ptr, trace_enable_lsb) ((opmsg_src_sync_set_trace_enable_ptr)->_data[2] = (uint16)(trace_enable_lsb))
#define OPMSG_SRC_SYNC_SET_TRACE_ENABLE_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_SRC_SYNC_SET_TRACE_ENABLE_CREATE(message_id, trace_enable_msb, trace_enable_lsb) \
    (uint16)(message_id), \
    (uint16)(trace_enable_msb), \
    (uint16)(trace_enable_lsb)
#define OPMSG_SRC_SYNC_SET_TRACE_ENABLE_PACK(opmsg_src_sync_set_trace_enable_ptr, message_id, trace_enable_msb, trace_enable_lsb) \
    do { \
        (opmsg_src_sync_set_trace_enable_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_src_sync_set_trace_enable_ptr)->_data[1] = (uint16)((uint16)(trace_enable_msb)); \
        (opmsg_src_sync_set_trace_enable_ptr)->_data[2] = (uint16)((uint16)(trace_enable_lsb)); \
    } while (0)

#define OPMSG_SRC_SYNC_SET_TRACE_ENABLE_MARSHALL(addr, opmsg_src_sync_set_trace_enable_ptr) memcpy((void *)(addr), (void *)(opmsg_src_sync_set_trace_enable_ptr), 3)
#define OPMSG_SRC_SYNC_SET_TRACE_ENABLE_UNMARSHALL(addr, opmsg_src_sync_set_trace_enable_ptr) memcpy((void *)(opmsg_src_sync_set_trace_enable_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    Opmsg_Unsolicited_QVA_Trigger

  DESCRIPTION
    Unsolicited message with trigger information.

  MEMBERS
    number_of_channels  - number of channels
    ch0_trigger_details - trigger detail for ch0
    ch1_trigger_details - trigger detail for ch1
    ch2_trigger_details - trigger detail for ch2
    ch3_trigger_details - trigger detail for ch3

*******************************************************************************/
typedef struct
{
    uint16 _data[21];
} OPMSG_UNSOLICITED_QVA_TRIGGER;

/* The following macros take OPMSG_UNSOLICITED_QVA_TRIGGER *opmsg_unsolicited_qva_trigger_ptr */
#define OPMSG_UNSOLICITED_QVA_TRIGGER_NUMBER_OF_CHANNELS_WORD_OFFSET (0)
#define OPMSG_UNSOLICITED_QVA_TRIGGER_NUMBER_OF_CHANNELS_GET(opmsg_unsolicited_qva_trigger_ptr) ((opmsg_unsolicited_qva_trigger_ptr)->_data[0])
#define OPMSG_UNSOLICITED_QVA_TRIGGER_NUMBER_OF_CHANNELS_SET(opmsg_unsolicited_qva_trigger_ptr, number_of_channels) ((opmsg_unsolicited_qva_trigger_ptr)->_data[0] = (uint16)(number_of_channels))
#define OPMSG_UNSOLICITED_QVA_TRIGGER_CH0_TRIGGER_DETAILS_WORD_OFFSET (1)
#define OPMSG_UNSOLICITED_QVA_TRIGGER_CH1_TRIGGER_DETAILS_WORD_OFFSET (6)
#define OPMSG_UNSOLICITED_QVA_TRIGGER_CH2_TRIGGER_DETAILS_WORD_OFFSET (11)
#define OPMSG_UNSOLICITED_QVA_TRIGGER_CH3_TRIGGER_DETAILS_WORD_OFFSET (16)
#define OPMSG_UNSOLICITED_QVA_TRIGGER_WORD_SIZE (21)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_UNSOLICITED_QVA_TRIGGER_CREATE(number_of_channels) \
    (uint16)(number_of_channels)
#define OPMSG_UNSOLICITED_QVA_TRIGGER_PACK(opmsg_unsolicited_qva_trigger_ptr, number_of_channels) \
    do { \
        (opmsg_unsolicited_qva_trigger_ptr)->_data[0] = (uint16)((uint16)(number_of_channels)); \
    } while (0)

#define OPMSG_UNSOLICITED_QVA_TRIGGER_MARSHALL(addr, opmsg_unsolicited_qva_trigger_ptr) memcpy((void *)(addr), (void *)(opmsg_unsolicited_qva_trigger_ptr), 21)
#define OPMSG_UNSOLICITED_QVA_TRIGGER_UNMARSHALL(addr, opmsg_unsolicited_qva_trigger_ptr) memcpy((void *)(opmsg_unsolicited_qva_trigger_ptr), (void *)(addr), 21)


/*******************************************************************************

  NAME
    Opmsg_Usb_Audio_Set_Connection_Config

  DESCRIPTION
    sends the usb connection config to the operator

  MEMBERS
    message_id     - message id
    data_format    - data format
    sample_rate    - sample rate (in 25Hz units)
    nrof_channels  - number of channels
    subframe_size  - subframe size in bits
    bit_resolution - bit resolution in bits

*******************************************************************************/
typedef struct
{
    uint16 _data[6];
} OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG;

/* The following macros take OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG *opmsg_usb_audio_set_connection_config_ptr */
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_MESSAGE_ID_GET(opmsg_usb_audio_set_connection_config_ptr) ((OPMSG_USB_AUDIO_ID)(opmsg_usb_audio_set_connection_config_ptr)->_data[0])
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_MESSAGE_ID_SET(opmsg_usb_audio_set_connection_config_ptr, message_id) ((opmsg_usb_audio_set_connection_config_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_DATA_FORMAT_WORD_OFFSET (1)
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_DATA_FORMAT_GET(opmsg_usb_audio_set_connection_config_ptr) ((opmsg_usb_audio_set_connection_config_ptr)->_data[1])
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_DATA_FORMAT_SET(opmsg_usb_audio_set_connection_config_ptr, data_format) ((opmsg_usb_audio_set_connection_config_ptr)->_data[1] = (uint16)(data_format))
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_SAMPLE_RATE_WORD_OFFSET (2)
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_SAMPLE_RATE_GET(opmsg_usb_audio_set_connection_config_ptr) ((opmsg_usb_audio_set_connection_config_ptr)->_data[2])
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_SAMPLE_RATE_SET(opmsg_usb_audio_set_connection_config_ptr, sample_rate) ((opmsg_usb_audio_set_connection_config_ptr)->_data[2] = (uint16)(sample_rate))
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_NROF_CHANNELS_WORD_OFFSET (3)
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_NROF_CHANNELS_GET(opmsg_usb_audio_set_connection_config_ptr) ((opmsg_usb_audio_set_connection_config_ptr)->_data[3])
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_NROF_CHANNELS_SET(opmsg_usb_audio_set_connection_config_ptr, nrof_channels) ((opmsg_usb_audio_set_connection_config_ptr)->_data[3] = (uint16)(nrof_channels))
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_SUBFRAME_SIZE_WORD_OFFSET (4)
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_SUBFRAME_SIZE_GET(opmsg_usb_audio_set_connection_config_ptr) ((opmsg_usb_audio_set_connection_config_ptr)->_data[4])
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_SUBFRAME_SIZE_SET(opmsg_usb_audio_set_connection_config_ptr, subframe_size) ((opmsg_usb_audio_set_connection_config_ptr)->_data[4] = (uint16)(subframe_size))
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_BIT_RESOLUTION_WORD_OFFSET (5)
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_BIT_RESOLUTION_GET(opmsg_usb_audio_set_connection_config_ptr) ((opmsg_usb_audio_set_connection_config_ptr)->_data[5])
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_BIT_RESOLUTION_SET(opmsg_usb_audio_set_connection_config_ptr, bit_resolution) ((opmsg_usb_audio_set_connection_config_ptr)->_data[5] = (uint16)(bit_resolution))
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_WORD_SIZE (6)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_CREATE(message_id, data_format, sample_rate, nrof_channels, subframe_size, bit_resolution) \
    (uint16)(message_id), \
    (uint16)(data_format), \
    (uint16)(sample_rate), \
    (uint16)(nrof_channels), \
    (uint16)(subframe_size), \
    (uint16)(bit_resolution)
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_PACK(opmsg_usb_audio_set_connection_config_ptr, message_id, data_format, sample_rate, nrof_channels, subframe_size, bit_resolution) \
    do { \
        (opmsg_usb_audio_set_connection_config_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_usb_audio_set_connection_config_ptr)->_data[1] = (uint16)((uint16)(data_format)); \
        (opmsg_usb_audio_set_connection_config_ptr)->_data[2] = (uint16)((uint16)(sample_rate)); \
        (opmsg_usb_audio_set_connection_config_ptr)->_data[3] = (uint16)((uint16)(nrof_channels)); \
        (opmsg_usb_audio_set_connection_config_ptr)->_data[4] = (uint16)((uint16)(subframe_size)); \
        (opmsg_usb_audio_set_connection_config_ptr)->_data[5] = (uint16)((uint16)(bit_resolution)); \
    } while (0)

#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_MARSHALL(addr, opmsg_usb_audio_set_connection_config_ptr) memcpy((void *)(addr), (void *)(opmsg_usb_audio_set_connection_config_ptr), 6)
#define OPMSG_USB_AUDIO_SET_CONNECTION_CONFIG_UNMARSHALL(addr, opmsg_usb_audio_set_connection_config_ptr) memcpy((void *)(opmsg_usb_audio_set_connection_config_ptr), (void *)(addr), 6)


/*******************************************************************************

  NAME
    Opmsg_Wbs_Enc_Forward_All_Kicks

  DESCRIPTION
    WBS_ENC operator message for FORWARD_ALL_KICKS.

  MEMBERS
    message_id        - message id
    forward_all_kicks - forward all kicks enabled

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_WBS_ENC_FORWARD_ALL_KICKS;

/* The following macros take OPMSG_WBS_ENC_FORWARD_ALL_KICKS *opmsg_wbs_enc_forward_all_kicks_ptr */
#define OPMSG_WBS_ENC_FORWARD_ALL_KICKS_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_WBS_ENC_FORWARD_ALL_KICKS_MESSAGE_ID_GET(opmsg_wbs_enc_forward_all_kicks_ptr) ((OPMSG_WBS_ENC_ID)(opmsg_wbs_enc_forward_all_kicks_ptr)->_data[0])
#define OPMSG_WBS_ENC_FORWARD_ALL_KICKS_MESSAGE_ID_SET(opmsg_wbs_enc_forward_all_kicks_ptr, message_id) ((opmsg_wbs_enc_forward_all_kicks_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_WBS_ENC_FORWARD_ALL_KICKS_FORWARD_ALL_KICKS_WORD_OFFSET (1)
#define OPMSG_WBS_ENC_FORWARD_ALL_KICKS_FORWARD_ALL_KICKS_GET(opmsg_wbs_enc_forward_all_kicks_ptr) ((opmsg_wbs_enc_forward_all_kicks_ptr)->_data[1])
#define OPMSG_WBS_ENC_FORWARD_ALL_KICKS_FORWARD_ALL_KICKS_SET(opmsg_wbs_enc_forward_all_kicks_ptr, forward_all_kicks) ((opmsg_wbs_enc_forward_all_kicks_ptr)->_data[1] = (uint16)(forward_all_kicks))
#define OPMSG_WBS_ENC_FORWARD_ALL_KICKS_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_WBS_ENC_FORWARD_ALL_KICKS_CREATE(message_id, forward_all_kicks) \
    (uint16)(message_id), \
    (uint16)(forward_all_kicks)
#define OPMSG_WBS_ENC_FORWARD_ALL_KICKS_PACK(opmsg_wbs_enc_forward_all_kicks_ptr, message_id, forward_all_kicks) \
    do { \
        (opmsg_wbs_enc_forward_all_kicks_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_wbs_enc_forward_all_kicks_ptr)->_data[1] = (uint16)((uint16)(forward_all_kicks)); \
    } while (0)

#define OPMSG_WBS_ENC_FORWARD_ALL_KICKS_MARSHALL(addr, opmsg_wbs_enc_forward_all_kicks_ptr) memcpy((void *)(addr), (void *)(opmsg_wbs_enc_forward_all_kicks_ptr), 2)
#define OPMSG_WBS_ENC_FORWARD_ALL_KICKS_UNMARSHALL(addr, opmsg_wbs_enc_forward_all_kicks_ptr) memcpy((void *)(opmsg_wbs_enc_forward_all_kicks_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_Wbs_Enc_Set_To_Air_Info

  DESCRIPTION
    WBS_ENC operator message for SET_TO_AIR_INFO.

  MEMBERS
    message_id - message id
    frame_size - frame size

*******************************************************************************/
typedef struct
{
    uint16 _data[2];
} OPMSG_WBS_ENC_SET_TO_AIR_INFO;

/* The following macros take OPMSG_WBS_ENC_SET_TO_AIR_INFO *opmsg_wbs_enc_set_to_air_info_ptr */
#define OPMSG_WBS_ENC_SET_TO_AIR_INFO_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_WBS_ENC_SET_TO_AIR_INFO_MESSAGE_ID_GET(opmsg_wbs_enc_set_to_air_info_ptr) ((OPMSG_WBS_ENC_ID)(opmsg_wbs_enc_set_to_air_info_ptr)->_data[0])
#define OPMSG_WBS_ENC_SET_TO_AIR_INFO_MESSAGE_ID_SET(opmsg_wbs_enc_set_to_air_info_ptr, message_id) ((opmsg_wbs_enc_set_to_air_info_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_WBS_ENC_SET_TO_AIR_INFO_FRAME_SIZE_WORD_OFFSET (1)
#define OPMSG_WBS_ENC_SET_TO_AIR_INFO_FRAME_SIZE_GET(opmsg_wbs_enc_set_to_air_info_ptr) ((opmsg_wbs_enc_set_to_air_info_ptr)->_data[1])
#define OPMSG_WBS_ENC_SET_TO_AIR_INFO_FRAME_SIZE_SET(opmsg_wbs_enc_set_to_air_info_ptr, frame_size) ((opmsg_wbs_enc_set_to_air_info_ptr)->_data[1] = (uint16)(frame_size))
#define OPMSG_WBS_ENC_SET_TO_AIR_INFO_WORD_SIZE (2)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_WBS_ENC_SET_TO_AIR_INFO_CREATE(message_id, frame_size) \
    (uint16)(message_id), \
    (uint16)(frame_size)
#define OPMSG_WBS_ENC_SET_TO_AIR_INFO_PACK(opmsg_wbs_enc_set_to_air_info_ptr, message_id, frame_size) \
    do { \
        (opmsg_wbs_enc_set_to_air_info_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_wbs_enc_set_to_air_info_ptr)->_data[1] = (uint16)((uint16)(frame_size)); \
    } while (0)

#define OPMSG_WBS_ENC_SET_TO_AIR_INFO_MARSHALL(addr, opmsg_wbs_enc_set_to_air_info_ptr) memcpy((void *)(addr), (void *)(opmsg_wbs_enc_set_to_air_info_ptr), 2)
#define OPMSG_WBS_ENC_SET_TO_AIR_INFO_UNMARSHALL(addr, opmsg_wbs_enc_set_to_air_info_ptr) memcpy((void *)(opmsg_wbs_enc_set_to_air_info_ptr), (void *)(addr), 2)


/*******************************************************************************

  NAME
    Opmsg_download_ps_test_delete_req

  DESCRIPTION
    pskey delete rank request message

  MEMBERS
    message_id   - command code
    delete_ident - ID value
    delete_rank  - Key Low Word

*******************************************************************************/
typedef struct
{
    uint16 _data[3];
} OPMSG_DOWNLOAD_PS_TEST_DELETE_REQ;

/* The following macros take OPMSG_DOWNLOAD_PS_TEST_DELETE_REQ *opmsg_download_ps_test_delete_req_ptr */
#define OPMSG_DOWNLOAD_PS_TEST_DELETE_REQ_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_DOWNLOAD_PS_TEST_DELETE_REQ_MESSAGE_ID_GET(opmsg_download_ps_test_delete_req_ptr) ((OPMSG_DOWNLOAD_PS_TEST_ID)(opmsg_download_ps_test_delete_req_ptr)->_data[0])
#define OPMSG_DOWNLOAD_PS_TEST_DELETE_REQ_MESSAGE_ID_SET(opmsg_download_ps_test_delete_req_ptr, message_id) ((opmsg_download_ps_test_delete_req_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_DOWNLOAD_PS_TEST_DELETE_REQ_DELETE_IDENT_WORD_OFFSET (1)
#define OPMSG_DOWNLOAD_PS_TEST_DELETE_REQ_DELETE_IDENT_GET(opmsg_download_ps_test_delete_req_ptr) ((opmsg_download_ps_test_delete_req_ptr)->_data[1])
#define OPMSG_DOWNLOAD_PS_TEST_DELETE_REQ_DELETE_IDENT_SET(opmsg_download_ps_test_delete_req_ptr, delete_ident) ((opmsg_download_ps_test_delete_req_ptr)->_data[1] = (uint16)(delete_ident))
#define OPMSG_DOWNLOAD_PS_TEST_DELETE_REQ_DELETE_RANK_WORD_OFFSET (2)
#define OPMSG_DOWNLOAD_PS_TEST_DELETE_REQ_DELETE_RANK_GET(opmsg_download_ps_test_delete_req_ptr) ((opmsg_download_ps_test_delete_req_ptr)->_data[2])
#define OPMSG_DOWNLOAD_PS_TEST_DELETE_REQ_DELETE_RANK_SET(opmsg_download_ps_test_delete_req_ptr, delete_rank) ((opmsg_download_ps_test_delete_req_ptr)->_data[2] = (uint16)(delete_rank))
#define OPMSG_DOWNLOAD_PS_TEST_DELETE_REQ_WORD_SIZE (3)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_DOWNLOAD_PS_TEST_DELETE_REQ_CREATE(message_id, delete_ident, delete_rank) \
    (uint16)(message_id), \
    (uint16)(delete_ident), \
    (uint16)(delete_rank)
#define OPMSG_DOWNLOAD_PS_TEST_DELETE_REQ_PACK(opmsg_download_ps_test_delete_req_ptr, message_id, delete_ident, delete_rank) \
    do { \
        (opmsg_download_ps_test_delete_req_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_download_ps_test_delete_req_ptr)->_data[1] = (uint16)((uint16)(delete_ident)); \
        (opmsg_download_ps_test_delete_req_ptr)->_data[2] = (uint16)((uint16)(delete_rank)); \
    } while (0)

#define OPMSG_DOWNLOAD_PS_TEST_DELETE_REQ_MARSHALL(addr, opmsg_download_ps_test_delete_req_ptr) memcpy((void *)(addr), (void *)(opmsg_download_ps_test_delete_req_ptr), 3)
#define OPMSG_DOWNLOAD_PS_TEST_DELETE_REQ_UNMARSHALL(addr, opmsg_download_ps_test_delete_req_ptr) memcpy((void *)(opmsg_download_ps_test_delete_req_ptr), (void *)(addr), 3)


/*******************************************************************************

  NAME
    Opmsg_download_ps_test_entry_delete_req

  DESCRIPTION
    pskey delete entry request message

  MEMBERS
    message_id   - command code
    delete_ident - ID value
    delete_key_l - Key Low Word
    delete_key_h - Key Hi Word

*******************************************************************************/
typedef struct
{
    uint16 _data[4];
} OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ;

/* The following macros take OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ *opmsg_download_ps_test_entry_delete_req_ptr */
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_MESSAGE_ID_GET(opmsg_download_ps_test_entry_delete_req_ptr) ((OPMSG_DOWNLOAD_PS_TEST_ID)(opmsg_download_ps_test_entry_delete_req_ptr)->_data[0])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_MESSAGE_ID_SET(opmsg_download_ps_test_entry_delete_req_ptr, message_id) ((opmsg_download_ps_test_entry_delete_req_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_DELETE_IDENT_WORD_OFFSET (1)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_DELETE_IDENT_GET(opmsg_download_ps_test_entry_delete_req_ptr) ((opmsg_download_ps_test_entry_delete_req_ptr)->_data[1])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_DELETE_IDENT_SET(opmsg_download_ps_test_entry_delete_req_ptr, delete_ident) ((opmsg_download_ps_test_entry_delete_req_ptr)->_data[1] = (uint16)(delete_ident))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_DELETE_KEY_L_WORD_OFFSET (2)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_DELETE_KEY_L_GET(opmsg_download_ps_test_entry_delete_req_ptr) ((opmsg_download_ps_test_entry_delete_req_ptr)->_data[2])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_DELETE_KEY_L_SET(opmsg_download_ps_test_entry_delete_req_ptr, delete_key_l) ((opmsg_download_ps_test_entry_delete_req_ptr)->_data[2] = (uint16)(delete_key_l))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_DELETE_KEY_H_WORD_OFFSET (3)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_DELETE_KEY_H_GET(opmsg_download_ps_test_entry_delete_req_ptr) ((opmsg_download_ps_test_entry_delete_req_ptr)->_data[3])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_DELETE_KEY_H_SET(opmsg_download_ps_test_entry_delete_req_ptr, delete_key_h) ((opmsg_download_ps_test_entry_delete_req_ptr)->_data[3] = (uint16)(delete_key_h))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_WORD_SIZE (4)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_CREATE(message_id, delete_ident, delete_key_l, delete_key_h) \
    (uint16)(message_id), \
    (uint16)(delete_ident), \
    (uint16)(delete_key_l), \
    (uint16)(delete_key_h)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_PACK(opmsg_download_ps_test_entry_delete_req_ptr, message_id, delete_ident, delete_key_l, delete_key_h) \
    do { \
        (opmsg_download_ps_test_entry_delete_req_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_download_ps_test_entry_delete_req_ptr)->_data[1] = (uint16)((uint16)(delete_ident)); \
        (opmsg_download_ps_test_entry_delete_req_ptr)->_data[2] = (uint16)((uint16)(delete_key_l)); \
        (opmsg_download_ps_test_entry_delete_req_ptr)->_data[3] = (uint16)((uint16)(delete_key_h)); \
    } while (0)

#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_MARSHALL(addr, opmsg_download_ps_test_entry_delete_req_ptr) memcpy((void *)(addr), (void *)(opmsg_download_ps_test_entry_delete_req_ptr), 4)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_DELETE_REQ_UNMARSHALL(addr, opmsg_download_ps_test_entry_delete_req_ptr) memcpy((void *)(opmsg_download_ps_test_entry_delete_req_ptr), (void *)(addr), 4)


/*******************************************************************************

  NAME
    Opmsg_download_ps_test_entry_read_req

  DESCRIPTION
    pskey read entry request message

  MEMBERS
    message_id - message id
    read_id    - ID value
    read_key_l - Key Low Word
    read_key_h - Key Hi Word
    read_rank  - Rank

*******************************************************************************/
typedef struct
{
    uint16 _data[5];
} OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ;

/* The following macros take OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ *opmsg_download_ps_test_entry_read_req_ptr */
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_MESSAGE_ID_GET(opmsg_download_ps_test_entry_read_req_ptr) ((OPMSG_DOWNLOAD_PS_TEST_ID)(opmsg_download_ps_test_entry_read_req_ptr)->_data[0])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_MESSAGE_ID_SET(opmsg_download_ps_test_entry_read_req_ptr, message_id) ((opmsg_download_ps_test_entry_read_req_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_READ_ID_WORD_OFFSET (1)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_READ_ID_GET(opmsg_download_ps_test_entry_read_req_ptr) ((opmsg_download_ps_test_entry_read_req_ptr)->_data[1])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_READ_ID_SET(opmsg_download_ps_test_entry_read_req_ptr, read_id) ((opmsg_download_ps_test_entry_read_req_ptr)->_data[1] = (uint16)(read_id))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_READ_KEY_L_WORD_OFFSET (2)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_READ_KEY_L_GET(opmsg_download_ps_test_entry_read_req_ptr) ((opmsg_download_ps_test_entry_read_req_ptr)->_data[2])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_READ_KEY_L_SET(opmsg_download_ps_test_entry_read_req_ptr, read_key_l) ((opmsg_download_ps_test_entry_read_req_ptr)->_data[2] = (uint16)(read_key_l))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_READ_KEY_H_WORD_OFFSET (3)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_READ_KEY_H_GET(opmsg_download_ps_test_entry_read_req_ptr) ((opmsg_download_ps_test_entry_read_req_ptr)->_data[3])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_READ_KEY_H_SET(opmsg_download_ps_test_entry_read_req_ptr, read_key_h) ((opmsg_download_ps_test_entry_read_req_ptr)->_data[3] = (uint16)(read_key_h))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_READ_RANK_WORD_OFFSET (4)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_READ_RANK_GET(opmsg_download_ps_test_entry_read_req_ptr) ((opmsg_download_ps_test_entry_read_req_ptr)->_data[4])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_READ_RANK_SET(opmsg_download_ps_test_entry_read_req_ptr, read_rank) ((opmsg_download_ps_test_entry_read_req_ptr)->_data[4] = (uint16)(read_rank))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_WORD_SIZE (5)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_CREATE(message_id, read_id, read_key_l, read_key_h, read_rank) \
    (uint16)(message_id), \
    (uint16)(read_id), \
    (uint16)(read_key_l), \
    (uint16)(read_key_h), \
    (uint16)(read_rank)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_PACK(opmsg_download_ps_test_entry_read_req_ptr, message_id, read_id, read_key_l, read_key_h, read_rank) \
    do { \
        (opmsg_download_ps_test_entry_read_req_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_download_ps_test_entry_read_req_ptr)->_data[1] = (uint16)((uint16)(read_id)); \
        (opmsg_download_ps_test_entry_read_req_ptr)->_data[2] = (uint16)((uint16)(read_key_l)); \
        (opmsg_download_ps_test_entry_read_req_ptr)->_data[3] = (uint16)((uint16)(read_key_h)); \
        (opmsg_download_ps_test_entry_read_req_ptr)->_data[4] = (uint16)((uint16)(read_rank)); \
    } while (0)

#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_MARSHALL(addr, opmsg_download_ps_test_entry_read_req_ptr) memcpy((void *)(addr), (void *)(opmsg_download_ps_test_entry_read_req_ptr), 5)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_READ_REQ_UNMARSHALL(addr, opmsg_download_ps_test_entry_read_req_ptr) memcpy((void *)(opmsg_download_ps_test_entry_read_req_ptr), (void *)(addr), 5)


/*******************************************************************************

  NAME
    Opmsg_download_ps_test_entry_write_req

  DESCRIPTION
    pskey write entry request message

  MEMBERS
    message_id         - message id
    write_id           - ID value
    write_key_l        - Key Low Word
    write_key_h        - Key Hi Word
    write_rank         - Rank
    write_total_length - Total Length of the message
    write_offset       - Offset
    payload            - 1st data position

*******************************************************************************/
typedef struct
{
    uint16 _data[8];
} OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ;

/* The following macros take OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ *opmsg_download_ps_test_entry_write_req_ptr */
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_MESSAGE_ID_WORD_OFFSET (0)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_MESSAGE_ID_GET(opmsg_download_ps_test_entry_write_req_ptr) ((OPMSG_DOWNLOAD_PS_TEST_ID)(opmsg_download_ps_test_entry_write_req_ptr)->_data[0])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_MESSAGE_ID_SET(opmsg_download_ps_test_entry_write_req_ptr, message_id) ((opmsg_download_ps_test_entry_write_req_ptr)->_data[0] = (uint16)(message_id))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_ID_WORD_OFFSET (1)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_ID_GET(opmsg_download_ps_test_entry_write_req_ptr) ((opmsg_download_ps_test_entry_write_req_ptr)->_data[1])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_ID_SET(opmsg_download_ps_test_entry_write_req_ptr, write_id) ((opmsg_download_ps_test_entry_write_req_ptr)->_data[1] = (uint16)(write_id))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_KEY_L_WORD_OFFSET (2)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_KEY_L_GET(opmsg_download_ps_test_entry_write_req_ptr) ((opmsg_download_ps_test_entry_write_req_ptr)->_data[2])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_KEY_L_SET(opmsg_download_ps_test_entry_write_req_ptr, write_key_l) ((opmsg_download_ps_test_entry_write_req_ptr)->_data[2] = (uint16)(write_key_l))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_KEY_H_WORD_OFFSET (3)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_KEY_H_GET(opmsg_download_ps_test_entry_write_req_ptr) ((opmsg_download_ps_test_entry_write_req_ptr)->_data[3])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_KEY_H_SET(opmsg_download_ps_test_entry_write_req_ptr, write_key_h) ((opmsg_download_ps_test_entry_write_req_ptr)->_data[3] = (uint16)(write_key_h))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_RANK_WORD_OFFSET (4)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_RANK_GET(opmsg_download_ps_test_entry_write_req_ptr) ((opmsg_download_ps_test_entry_write_req_ptr)->_data[4])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_RANK_SET(opmsg_download_ps_test_entry_write_req_ptr, write_rank) ((opmsg_download_ps_test_entry_write_req_ptr)->_data[4] = (uint16)(write_rank))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_TOTAL_LENGTH_WORD_OFFSET (5)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_TOTAL_LENGTH_GET(opmsg_download_ps_test_entry_write_req_ptr) ((opmsg_download_ps_test_entry_write_req_ptr)->_data[5])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_TOTAL_LENGTH_SET(opmsg_download_ps_test_entry_write_req_ptr, write_total_length) ((opmsg_download_ps_test_entry_write_req_ptr)->_data[5] = (uint16)(write_total_length))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_OFFSET_WORD_OFFSET (6)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_OFFSET_GET(opmsg_download_ps_test_entry_write_req_ptr) ((opmsg_download_ps_test_entry_write_req_ptr)->_data[6])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WRITE_OFFSET_SET(opmsg_download_ps_test_entry_write_req_ptr, write_offset) ((opmsg_download_ps_test_entry_write_req_ptr)->_data[6] = (uint16)(write_offset))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_PAYLOAD_WORD_OFFSET (7)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_PAYLOAD_GET(opmsg_download_ps_test_entry_write_req_ptr) ((opmsg_download_ps_test_entry_write_req_ptr)->_data[7])
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_PAYLOAD_SET(opmsg_download_ps_test_entry_write_req_ptr, payload) ((opmsg_download_ps_test_entry_write_req_ptr)->_data[7] = (uint16)(payload))
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_WORD_SIZE (8)
/*lint -e(773) allow unparenthesized*/
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_CREATE(message_id, write_id, write_key_l, write_key_h, write_rank, write_total_length, write_offset, payload) \
    (uint16)(message_id), \
    (uint16)(write_id), \
    (uint16)(write_key_l), \
    (uint16)(write_key_h), \
    (uint16)(write_rank), \
    (uint16)(write_total_length), \
    (uint16)(write_offset), \
    (uint16)(payload)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_PACK(opmsg_download_ps_test_entry_write_req_ptr, message_id, write_id, write_key_l, write_key_h, write_rank, write_total_length, write_offset, payload) \
    do { \
        (opmsg_download_ps_test_entry_write_req_ptr)->_data[0] = (uint16)((uint16)(message_id)); \
        (opmsg_download_ps_test_entry_write_req_ptr)->_data[1] = (uint16)((uint16)(write_id)); \
        (opmsg_download_ps_test_entry_write_req_ptr)->_data[2] = (uint16)((uint16)(write_key_l)); \
        (opmsg_download_ps_test_entry_write_req_ptr)->_data[3] = (uint16)((uint16)(write_key_h)); \
        (opmsg_download_ps_test_entry_write_req_ptr)->_data[4] = (uint16)((uint16)(write_rank)); \
        (opmsg_download_ps_test_entry_write_req_ptr)->_data[5] = (uint16)((uint16)(write_total_length)); \
        (opmsg_download_ps_test_entry_write_req_ptr)->_data[6] = (uint16)((uint16)(write_offset)); \
        (opmsg_download_ps_test_entry_write_req_ptr)->_data[7] = (uint16)((uint16)(payload)); \
    } while (0)

#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_MARSHALL(addr, opmsg_download_ps_test_entry_write_req_ptr) memcpy((void *)(addr), (void *)(opmsg_download_ps_test_entry_write_req_ptr), 8)
#define OPMSG_DOWNLOAD_PS_TEST_ENTRY_WRITE_REQ_UNMARSHALL(addr, opmsg_download_ps_test_entry_write_req_ptr) memcpy((void *)(opmsg_download_ps_test_entry_write_req_ptr), (void *)(addr), 8)


#endif /* OPMSG_PRIM_H */


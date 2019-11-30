/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/* This header fragment is to be included by faultids.h
 * It cannot stand alone.
 * Contains all the audio-specific faultids; hydra common ids remain in hydra_faultids.h
 */
    /**
     * The configured PCM bit clock rate can't be used for some reason.
     * Argument is the configured rate.
     */
    FAULT_AUDIO_PCM_INVALID_CLOCK_RATE = 0x0001,

    /**
     * Internal error: attempted to destroy an audio buffer while it is
     * still in use. Argument is the buffer handle.
     */
    FAULT_AUDIO_IN_USE_PCM_BUFFER_DESTROYED = 0x0002,

    /**
     * ADC calibration timeout. (Not used on Kymera.)
     */
    FAULT_AUDIO_ADC_CALIBBRATION_TIMEOUT = 0x0003,

    /**
     * Attempt to set an unsupported configuration MIB Key or an invalid
     * value for MIB key for audio hardware. Argument contains information
     * which depends on call site.
     */
    FAULT_AUDIO_INVALID_CONFIG = 0x0004,

    /**
     * Audio resource manager initialisation failed. The argument gives the
     * specific resource manager instance that failed.
     */
    FAULT_AUDIO_RES_MGR_INIT_FAILED = 0x0005,

    /**
     * The patch loader received a session create failure
     */
    FAULT_AUDIO_PATCH_LOADER_SESSION_FAIL = 0x0006,

    /**
     * The patch loader received a read fail on an open file from fileserv
     */
    FAULT_AUDIO_PATCH_LOADER_READ_FAIL = 0x0007,

    /**
     * The patch loader received a close fail on an open file from fileserv
     */
    FAULT_AUDIO_PATCH_LOADER_CLOSE_FAIL = 0x0008,

    /**
     * The patch loader received a session destroy failure from fileserv
     */
    FAULT_AUDIO_PATCH_LOADER_SESSION_DESTROY_FAIL = 0x0009,

    /**
     * The patch loader received an incorrect msg_id from fileserv
     * The argument is the faulty msg_id
     */
    FAULT_AUDIO_PATCH_LOADER_BAD_MSG = 0x000a,

    /**
     * Miscellaneous catch-all: some part of the system asked another
     * part to do something currently unsupported. Probably an internal
     * design error.
     */
    FAULT_AUDIO_UNSUPPORTED = 0x000b,

    /**
     * The patch loader received a zero-length read.
     * It probably found the end of the file too early.
     * The argument is the amount of data (in 16-bit words) expected
     * but not read.
     */
    FAULT_AUDIO_PATCH_LOADER_READ_EMPTY = 0x000c,

    /**
     * The filename in the patch header didn't match the requested filename.
     */
    FAULT_AUDIO_PATCH_LOADER_WRONG_FILENAME = 0x000d,

    /**
     * The patch loader tried to put code at an invalid PM RAM address.
     */
    FAULT_AUDIO_PATCH_LOADER_INVALID_ADDRESS = 0x000e,

    /**
     * The ACCMD transmit queue is over 50% full; argument is the number of
     * queued messages.
     */
    FAULT_AUDIO_ACCMD_TX_QUEUE_HIGH_USAGE = 0x000f,

    /**
     * An ACCMD service received a response message when it had not issued a
     * request; argument is the cmdId of the message that was received.
     */
    FAULT_AUDIO_ACCMD_UNEXPECTED_RESP_RECEIVED = 0x0010,

    /**
     * An ACCMD service received a response message which was incorrectly
     * formed.
     * This probably means that the message was the wrong size for the cmdId.
     * Argument is the cmdId.
     */
    FAULT_AUDIO_ACCMD_BAD_PDU_RESP_RECEIVED = 0x0011,

    /**
     * An ACCMD service received an invalid sequence number in a command.
     * This probably means that a response to a request sent to the client
     * came back with the wrong sequence number. Argument is the offending
     * sequence number.
     */
    FAULT_AUDIO_ACCMD_INVALID_SEQ_NO = 0x0012,

    /**
     * An ACCMD was received but could not be queued to the ACCMD task queue,
     * possibly due to memory exhaustion. Argument is the cmdId.
     */
    FAULT_AUDIO_ACCMD_RX_MSG_INVALID = 0x0013,

    /**
     * An ACCMD response message was received but the firmware does not know
     * how to handle this type of response message. Argument is the cmdId.
     */
    FAULT_AUDIO_ACCMD_UNHANDLED_RESP_RECEIVED = 0x0014,

    /**
     * A larger-than-necessary dynamic memory allocation happened
     * because there were no free blocks of the smallest suitable size.
     * The argument is the number of words requested.
     * (Not currently emitted by Kymera builds.)
     */
    FAULT_PMALLOC_OVERSIZE_ALLOCATION = 0x0015,

    /**
     * Couldn't find the end of an operator chain.
     * (Not currently emitted by Kymera.)
     */
    FAULT_AUDIO_STREAM_CHAIN_TOO_LONG = 0x0016,

    /**
     * Firmware has discovered itself to be running on a different hardware
     * revision to that which it was built for.
     */
    FAULT_AUDIO_WRONG_HARDWARE_VERSION = 0x0017,

    /**
     * Lost synchronisation with the received SCO packet stream from the
     * Bluetooth LC. (This is not a condition directly relating to what is
     * happening over the air.)
     */
    FAULT_AUDIO_SCO_METADATA_SYNC_LOST = 0x0018,

    /**
     * We were asked to set a buffer offset bigger than the buffer itself.
     * This isn't actually fatal as the buffer hardware will automatically
     * handle circular addressing, but in reality it probably indicates an
     * error. Argument is the offending offset.
     */
    FAULT_AUDIO_BUFFER_OFFSET_TOO_BIG = 0x0019,

    /**
     * Streams couldn't do a stream connect due to the configuration
     * of the audio endpoint
     */
    FAULT_AUDIO_STREAM_CONNECT_FAIL_ENDPOINT_FORMAT_MISMATCH = 0x001a,

    /**
     * A stream endpoint was accessed by a command connection that wasn't
     * its owner. Argument identifies the attempted accessor.
     */
    FAULT_AUDIO_STREAM_ENDPOINT_ACCESS_VIOLATION = 0x001b,

    /**
     * Couldn't get the buffer we required for a prodtest ACCMD
     * (not used on Kymera).
     */
    FAULT_AUDIO_PROD_TEST_BUFFER_SIZE_INCORRECT_FAIL = 0x001c,

    /**
     * Debug command failed to send (not used on Kymera).
     */
    FAULT_AUDIO_DEBUG_COULD_NOT_TRANSMIT = 0x001d,

    /**
     * Incoming ACCMD request seen with reserved bits nonzero.
     * Argument is the complete 16-bit word which includes the sequence
     * number and reserved fields.
     * (Usually this means the sender believes the sequence number occupies
     * this whole word.)
     */
    FAULT_AUDIO_ACCMD_RESERVED_FIELD_NONZERO = 0x001e,

    /**
     * Some operators need to be configured to learn their block sizes before
     * their endpoints are connected; streams will request the input and output
     * block sizes at connection.
     * This signals an attempt to connect before the operator received the
     * configuration.
     * (not used on Kymera)
     */
    FAULT_AUDIO_OPERATOR_BLOCK_SIZES_UNKNOWN = 0x001f,

    /**
     * The config loader received a session create failure
     * (not used on Kymera)
     */
    FAULT_AUDIO_CFG_LOADER_SESSION_FAIL = 0x20,

    /**
     * The config loader received a read fail on an open file from fileserv
     * (not used on Kymera)
     */
    FAULT_AUDIO_CFG_LOADER_READ_FAIL = 0x21,

    /**
     * The config loader received a zero-length read. It probably found the end
     * of the file too early. The argument is the amount of data (in 16-bit
     * words) expected but not read.
     * (not used on Kymera)
     */
    FAULT_AUDIO_CFG_LOADER_READ_EMPTY = 0x22,

    /**
     * The filename in the config header didn't match the requested filename.
     * (not used on Kymera)
     */
    FAULT_AUDIO_CFG_LOADER_WRONG_FILENAME = 0x0023,

    /**
     * The config loader received a close fail on an open file from fileserv
     * (not used on Kymera)
     */
    FAULT_AUDIO_CFG_LOADER_CLOSE_FAIL = 0x0024,

    /**
     * The config loader received a session destroy failure from fileserv
     * (not used on Kymera)
     */
    FAULT_AUDIO_CFG_LOADER_SESSION_DESTROY_FAIL = 0x0025,

    /**
     * The config loader received an incorrect msg_id from fileserv
     * The argument is the faulty msg_id
     * (not used on Kymera)
     */
    FAULT_AUDIO_CFG_LOADER_BAD_MSG = 0x0026,

    /**
     * Attempt to connect synchronised audio stream endpoints to SCO chains
     * with different endpoint_deadlines
     * (not used on Kymera)
     */
    FAULT_AUDIO_STREAM_SYNC_SCO_DEADLINE_NOT_SAME = 0x0027,

    /**
     * Attempt to connect synchronised audio stream endpoints to chains
     * with different kick times
     * (not used on Kymera)
     */
    FAULT_AUDIO_STREAM_SYNC_KICK_TIMES_NOT_SAME = 0x0028,

    /**
     * Found an ENDPOINT which is non-real with the type non-operator
     * (not used on Kymera)
     */
    FAULT_AUDIO_UNSUPPORTED_ENDPOINT_TYPE = 0x0029,

    /**
     * Rate matching does not work properly due to incorrect setting
     * (not used on Kymera)
     */
    FAULT_AUDIO_RATE_MATCHING_INCORRECT_SETTING = 0x002A,

    /**
     * Streams can't calculate an appropriate buffer priming level
     * (not used on Kymera)
     */
    FAULT_AUDIO_INCALCULABLE_PRIMING_LEVEL = 0x002B,

    /*
     * The requested PCM DATA format is unsupported
     */
    FAULT_AUDIO_PCM_DATA_FORMAT_UNSUPPORTED = 0x002C,

    /**
     * The requested PCM SAMPLE format is unsupported
     */
    FAULT_AUDIO_PCM_SAMPLE_FORMAT_UNSUPPORTED = 0x002D,

    /**
     * We got a VML error from the BAC
     */
    FAULT_AUDIO_BAC_VML_ERROR = 0x002E,

    /**
     * We got a memory-lookup error from the BAC
     */
    FAULT_AUDIO_BAC_MEM_ERROR = 0x002F,

    /**
     * Warp_and_shift failed to allocate a temporary cbuffer to run 'in-place'
     * (not used on Kymera)
     */
    FAULT_AUDIO_A2DP_RATE_MATCHING_MEM_ALLOCATION_FAILURE = 0x0030,

    /**
     * An attempted dynamic memory allocation failed.
     * The argument is the number of words requested.
     */
    FAULT_AUDIO_INSUFFICIENT_MEMORY = 0x0032,

    /**
     * Audio configuration tried to set invalid ADC input termination.
     * (no longer used)
     */
    FAULT_AUDIO_INVALID_INPUT_TERMINATION = 0x0033,

    /**
     * An attempt was made to connect two endpoints that are both trying to
     * provide a buffer.
     */
    FAULT_AUDIO_BOTH_ENDPOINTS_SUPPLYING_BUFFER = 0x0034,

    /**
     * The maximum number of shared memory users was reached and a shared memory
     * request failed. The argument is the shared memory id whose limit was hit.
     */
    FAULT_AUDIO_SHARED_MEM_USER_LIMIT = 0x0035,

    /**
     * Can not find a free BlueCore PS key to write; PS is full.
     */
    FAULT_AUDIO_PS_FULL = 0x0036,

    /**
     * A BlueCore PS write failed in a way that potentially endangers integrity
     * of the PS filesystem.
     */
    FAULT_AUDIO_PS_WRITE_FAILURE = 0x0037,

    /**
     * SCO kick time looked implausible so was rebased.
     * Probably due to late or corrupted wallclock.
     * Argument is new kick time.
     */
    FAULT_AUDIO_SCO_TIMER_REBASED = 0x0038,

    /**
     * SCO packet size is larger than what we can handle. Argument is length
     */
    FAULT_AUDIO_PACKET_SIZE_LARGER_THAN_MANAGEABLE_LENGTH = 0x0039,

    /**
     * The ratematch manager couldn't traverse the whole graph as it contains
     * too many operators. Argument contains first unvisited operator.
     */
    FAULT_AUDIO_RM_GRAPH_TOO_BIG = 0x003A,

    /**
     * The SCO manager wallclock update request failed; this usually happens
     * when the SCO link goes away before the DSP has been informed.
     * (BlueCore-specific.)
     */
    FAULT_AUDIO_SCO_MGR_WCLOCK_REQ_FAILED = 0x003B,

    /**
     * An attempt was made to remove a downloaded capability bundle that
     * wasn't in the internal database.
     */
    FAULT_AUDIO_CAP_DOWNLOAD_MGR_UNRECOGNISED_KCS = 0x003C,

    /**
     * Trying to remove a capability bundle which still has at least one
     * capability instantiated.
     */
    FAULT_AUDIO_CAP_DOWNLOAD_MGR_REMOVE_KCS_WHILE_OP_EXISTS = 0x003D,

    /**
     * Internal state machine failure. Capability download manager was trying
     * to perform an invalid operation. The parameter indicates the state that
     * produced the error
     */
    FAULT_AUDIO_CAP_DOWNLOAD_MGR_INTERNAL_ERROR_STATE = 0x003E,

    /**
     * An error ocurred while fetching data from the KCS
     */
    FAULT_AUDIO_CAP_DOWNLOAD_MGR_FETCH_ERROR = 0x003F,

    /**
     * Internal parser state machine failure. Capability download manager was trying
     * to perform an invalid operation while parsing the KCS file. The parameter indicates
     * the error as defined by enum STATE_EXIT_CODE in cap_download_manager_parser.c
     */
    FAULT_AUDIO_CAP_DOWNLOAD_MGR_PARSER_ERROR = 0x0040,

    /**
     * Internal capability download manager utility module error. Capability download manager
     * was trying to perform an invalid operation while processing a KCS file. The parameter
     * indicates the error as defined by enum ERROR_CODE in cap_download_manager_util.c
     */
    FAULT_AUDIO_CAP_DOWNLOAD_MGR_UTIL_ERROR = 0x0041,

    /**
     * In the SCO packet stream received from the Bluetooth LC, the metadata
     * reserved field was not 0 as expected. Perhaps an internal error with the
     * SCO metadata transform. Cannot be provoked over the air.
     */
    FAULT_AUDIO_INVALID_SCO_METADATA_RESERVE_FIELD_NOT_ZERO = 0x0042,

    /**
     * In the SCO packet stream received from the Bluetooth LC, the status
     * field was not one of the expected values. This is an internal error
     * which cannot be provoked over the air.
     */
    FAULT_AUDIO_INVALID_SCO_METADATA_STATE = 0x0043,

    /**
     * Narrowband SCO had issues copying data to the output buffer. This might
     * mean that the check for the available output space (or the packet data
     * length) is incorrect.
     */
    FAULT_AUDIO_INVALID_SCO_NB_COPY_PACKET_FAILED = 0x0044,

    /**
     * Incorrect wideband-SCO (WBS) validate return value.
     */
    FAULT_AUDIO_INVALID_WBS_VALIDATE_RESULT = 0x0045,

    /**
     * Incorrect WBS validate and decode return value combination.
     */
    FAULT_AUDIO_INVALID_WBS_VALIDATE_DECODE_RESULT = 0x0046,

    /**
     * On BlueCore, a message was received from the XAP and insufficient RAM
     * was available to allocate the message and handle it.
     */
    FAULT_AUDIO_BC_MESSAGE_ALLOCATION_FAILED = 0x0047,

    /**
     * The ACCMD transmit queue has run out of capacity; argument is queue size
     */
    FAULT_AUDIO_ACCMD_TX_QUEUE_FULL = 0x0048,

    /**
     * A connection failed because an in-place chain
     * would have exceeded MAX_INPLACE_LENGTH; argument is max length.
     */
    FAULT_AUDIO_IN_PLACE_CHAIN_TOO_LONG = 0x0049,

    /**
     * During a disconnect operation one side refused to disconnect due to its
     * state and it was not possible to reconnect the other side as it was
     * before the request was sent.
     */
    FAULT_AUDIO_RECONNECT_DURING_DISCONNECT_FAILED = 0x004A,

    /**
     * The timed playback module is unable to meet the requested latency.
     * This fault is emitted when multiple packets are dealt with late.
     */
    FAULT_UNACHIEVABLE_LATENCY = 0x004B,

    /**
     * The RTP decoder capability could not find the the SBC sync byte
     * in the header.
     */
    FAULT_RTP_SBC_SYNC_LOST = 0x004C,

    /**
     * A decoder capability couldn't enable ratematching due to insufficient
     * resources being available. Argument is the ID of the operator.
     */
    FAULT_AUDIO_CODEC_RM_OUT_OF_RAM = 0x004D,

    /**
     * Invalid buffer details access.
     * (This fault is no longer emitted by new firmware, and often did not
     * indicate a real problem in old firmware.)
     */
    FAULT_AUDIO_INVALID_BUF_DETAILS_ACCESS = 0x004E,

    /**
     * Some metadata was lost as there wasn't enough memory available for the
     * tag.
     */
    FAULT_AUDIO_METADATA_TAG_ALLOCATION_FAILED = 0x004F,

    /**
     * Some metadata private data was lost as there wasn't enough memory
     * available for the private data. Argument is the requested allocation.
     */
    FAULT_AUDIO_METADATA_DATA_ALLOCATION_FAILED = 0x0050,

    /**
     * Internal error: attempt to set invalid usable_octets on a buffer.
     * Argument is the wrong usable_octets.
     */
    FAULT_AUDIO_METADATA_USABLE_OCTETS_INVALID = 0x0051,

    /**
     * The built-in profiler has a limit on the number of entities (operators)
     * it can profile.
     * Emit this fault if we exceed this upper limit.
     */
    FAULT_AUDIO_PROFILER_HANDLER_LIST_TOO_LONG = 0x0052,

    /**
     * The second processor P1 has not responded to watchdog pings from P0.
     * Argument is the number of attempts P0 has made. (No longer used)
     */
    FAULT_AUDIO_REMOTE_PROCESSOR_NOT_RESPONDING = 0x0053,

    /**
     * An attempt has been made to query the licence manager before a
     * conection to it has been established.
     */
    FAULT_AUDIO_LICENSE_CHECK_NOT_AVAILABLE = 0x0054,

    /**
     * SCO metadata not enabled in the connection buffer
     */
    FAULT_SCO_METADATA_NOT_PRESENT = 0x0055,

    /**
     * The transform ID passed as a parameter in the kip connect
     * response handler is not identical to the transform ID as
     * stored in state/context when starting execution of the
     * connect command. P1 is not playing by the rules!
     */
    FAULT_AUDIO_MULTICORE_CONNECT_INVALID_STATE = 0x0056,

    /**
     * For Audio IPC to report a fault. Typically when receiving
     * fewer bytes than expected (IPC or KIP protocol error) or
     * IPC message buffer full. Argument == IPC status (error code).
     */
    FAULT_AUDIO_MULTICORE_IPC_ERROR = 0x0057,

    /**
     * A passthrough capability is running with an unsupported mismatch
     * between input and output formats. Argument is operator ID.
     */
    FAULT_AUDIO_PASSTHROUGH_FORMAT_MISMATCH = 0x0058,

    /**
     * A call to buff_metadata_append attempted to add a tag which
     * is not aligned the next expected tag's index
     */
    FAULT_AUDIO_METADATA_APPEND_MISALIGNED = 0x0059,

    /**
     * A call to buff_metadata_append attempted to insert more
     * octets than the available space.
     */
    FAULT_AUDIO_METADATA_APPEND_EXCEEDS_SPACE = 0x005a,

    /**
     * A call to buff_metadata_remove attempted to consume more
     * metadata than available.
     */
    FAULT_AUDIO_METADATA_REMOVE_EXCEEDS_AVAILABLE = 0x005b,

    /**
     * When looking for shadow (inter-processor) endpoint associated with
     * a metadata channel, none was found. For example, for a metadata channel
     * with ID of 0x18F, an endpoint with ID 0x180 is expected to
     * be found.
     */
    FAULT_AUDIO_METADATA_PARTNER_EP_MISSING = 0x005c,

    /**
     * During stream_connect, one of the endpoints involved supplied a buffer
     * which was too small to meet the other endpoint's requirement.
     * Argument is the supplied buffer size.
     */
    FAULT_AUDIO_SUPPLIED_BUFFER_TOO_SMALL = 0x005d,

    /**
     * When there's not enough space in the shared buffer for KIP to pass a
     * metadata tag to another processor. Argument is tag's size.
     */
    FAULT_AUDIO_METADATA_KIP_BUFFER_NOT_ENOUGH_SPACE = 0x005e,

    /**
     * Raised when trying to call HAL functions with invalid arguments.
     */
    FAULT_AUDIO_INVALID_HAL_PARAMETER = 0x005f,

    /**
     * Raised when a Source Sync operator attempts to write to a disconnected output
     */
    FAULT_AUDIO_SRC_SYNC_WRITE_UNCONNECTED_TERMINAL = 0x0060,

    /* Hydra common fault IDs start at 0x1000 */

    /*
     * 0x4000 is the start of debug fault IDs that should never occur in
     * production code. These are for events in the following categories:
     *   + things that really are impossible (yeah, right)
     *   + temporary faults in place of recovery code
     * Really, all of these should be become either real or removed.
     *
     * These should only be emitted with the right debug options enabled.
     */
    /**
     * Buffer was attempted to be primed with an amount larger than buffer size.
     */
    FAULT_AUDIO_DEBUG_BUFF_PRIMING_ABOVE_BUFF_SIZE = 0x4000,

    /**
     * Asked to discard more data than was available in a buffer.
     * Argument: amount of data available.
     */
    FAULT_AUDIO_DEBUG_BUFFER_NOT_ENOUGH_DATA = 0x4001,

    /**
     * Asked to fill a buffer with more data than space was available for.
     * Argument: amount of space available.
     */
    FAULT_AUDIO_DEBUG_BUFFER_NOT_ENOUGH_SPACE = 0x4002,

    /**
     * An operation was attempted on the wrong kind of buffer.
     * Argument: buffer descriptor.
     */
    FAULT_AUDIO_DEBUG_BUFFER_ACCESS = 0x4003,

    /**
     * Not enough space in an audio data service metadata buffer.
     * This is an internal error (the service design assumes that the
     * buffer is big enough and does not specify recovery from this
     * condition).
     * Argument: amount of space available.
     */
    FAULT_AUDIO_DATA_SERVICE_META_BUFFER_NOT_ENOUGH_SPACE = 0x4004,

    /**
     * The in-place buffer list doesn't have direction or is set twice.
     * This is only generated if IN_PLACE_DEBUG is defined.
     */
    FAULT_AUDIO_IN_PLACE_LIST_DIRECTION_ERROR = 0x4005,

    /**
     * A secondary core stream endpoints may not have destroyed during
     * a connection failure
     */
    FAULT_AUDIO_STREAM_ENDPOINT_DESTROY_ERROR = 0x4006,

    /**
     * One or more transforms at one core have been disconnected but
     * the corresponding KIP transforms at another core have note been
     * disconnected results in a disconnected error. This partial
     * transform disconnect is irrecoverable.
     */
    FAULT_AUDIO_STREAM_TRANSFORM_DISCONNNECT_ERROR = 0x4007,


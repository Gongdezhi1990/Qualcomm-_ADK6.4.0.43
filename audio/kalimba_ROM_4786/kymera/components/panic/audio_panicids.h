/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/* This header fragment is to be included by panicids.h
 * It cannot stand alone.
 * Contains all the audio-specific panicids; hydra common ids remain in hydra_panicids.h
 * For any panic cases that don't exactly match existing ones below (in terms of meaning and
 * platform they were defined on), add new names & IDs continuing from last ID in the appropriate existing "range",
 * instead of re-using "similar" existing definitions.
 * Current ranges are:
 *      0x0000 - 0x0FFF:  audio panic IDs including ones used by platform-independent modules.
 *                        TODO: possibly separate platform-independent (e.g. streams) panic codes from e.g.
 *                              Amber audio subsystem ones, if possible (this would break historic values).
 *      0x1000 - 0x3FFF:  currently for Hydra codes - this range later, if needed, can be narrowed
 *      0x4000 - ...:  debug panic IDs that never occur in production code
 */

    /**
     * An invalid stream en0dpoint ID (SID) was seen. This suggests the
     * value has become corrupted. The argument is the invalid SID.
     */
    PANIC_AUDIO_INVALID_SID = 0x0001,

    /**
     * Invalid voice slot manager (VSM) state in the audio driver module
     */
    PANIC_AUDIO_VSM_INVALID_STATE = 0x0002,

    /**
     * Internal error: unexpected NULL pointer in audio driver module.
     */
    PANIC_AUDIO_INVALID_TOKEN = 0x0003,

    /**
     * RAMRUN code too big (not currently used)
     */
    PANIC_AUDIO_RAMRUN_CODE_TOO_BIG = 0x0004,

    /**
     * The scheduler received a NULL pointer to a message (probably out
     * of memory).
     * (No longer used, see PANIC_OXYGOS_SCHED_MSG_IS_NULL.)
     */
    PANIC_AUDIO_SCHED_MSG_IS_NULL = 0x0005,

    /**
     * A call to pmalloc or shared memory requested zero bytes of
     * memory
     */
    PANIC_AUDIO_REQ_ZERO_MEMORY = 0x0006,

    /**
     * Malloc Memory pool control information invalid
     */
    PANIC_AUDIO_INVALID_POOL_INFO = 0x0007,

    /**
     * An invalid timer value was specified for a scheduled event
     */
    PANIC_AUDIO_INVALID_TIME = 0x0008,

    /**
     * An invalid task ID has been found in the scheduler
     * (No longer used, see PANIC_OXYGOS_INVALID_TASK_ID.)
     */
    PANIC_AUDIO_INVALID_TASK_ID = 0x0009,

    /**
     * A Null handler has been found in the scheduler
     * (No longer used, see PANIC_OXYGOS_NULL_HANDLER.)
     */
    PANIC_AUDIO_NULL_HANDLER = 0x000A,

    /**
     * An invalid scheduler queue number
     * (No longer used, see PANIC_OXYGOS_INVALID_QUEUE_NUMBER.)
     */
    PANIC_AUDIO_INVALID_QUEUE_NUMBER = 0x000B,

    /**
     * The bit position of the background task being raised is invalid
     * This panic ID is now deprecated
     * (See also PANIC_OXYGOS_INVALID_BACKGROUND_INTERRUPT_BIT_POSITION)
     */
    PANIC_AUDIO_INVALID_BACKGROUND_INTERRUPT_BIT_POSITION = 0x000C,

    /**
     * The number of scheduled messages does not match the number of messages
     * found in the scheduler
     * (No longer used, see PANIC_OXYGOS_INVALID_MESSAGE_COUNT.)
     */
    PANIC_AUDIO_INVALID_MESSAGE_COUNT = 0x000D,

    /**
     * Internal error: an invalid audio interface hardware type has been
     * referenced internally.
     * The argument is the invalid audio hardware type.
     */
    PANIC_AUDIO_INVALID_HARDWARE_TYPE = 0x000E,

    /**
     * Internal error: an attempt has been made to request SCO buffers for
     * a SCO handle that already has buffers associated with it.
     */
    PANIC_AUDIO_SCO_BUFFERS_ALREADY_EXIST = 0x000F,

    /**
     * Internal error: an attempt was made to do something with an MMU
     * handle that doesn't exist or is of inappropriate type.
     */
    PANIC_AUDIO_INVALID_MMU_HANDLE = 0x0010,

    /**
     * Something called the traditional Kalimba assembler error handler
     * $error, for some reason.
     * The argument is the value of rLink at entry to $error,
     * which should uniquely identify the caller.
     * Note that this value is 24 bits internally, and is stored as such in
     * the audio subsystem's preserve block, but only the LS 16 bits make it
     * out in messages to Curator / Host.
     */
    PANIC_AUDIO_LIBRARY_ERROR = 0x0011,

    /**
     * An interrupt fired without having a handler installed.
     */
    PANIC_AUDIO_UNHANDLED_INTERRUPT = 0x0012,

    /**
     * Software triggered an exception (which manifests like an interrupt).
     * See the EXCEPTION_TYPE and possibly EXT_EXCEPTION_TYPE registers for
     * the kind of exception; these are usually memory access exceptions,
     * such as a failure to meet address alignment requirements.
     * The argument is the program counter where the exception fired; note
     * that it may not be exact, due to pipelining.
     * Note that this value is 24 bits internally, and is stored as such in
     * the audio subsystem's preserve block, but only the LS 16 bits make it
     * out in messages to Curator / Host.
     */
    PANIC_AUDIO_SW_EXCEPTION = 0x0013,

    /**
     * A stack overflow exception fired.
     * The argument is the value of STACK_OVERFLOW_PC after the exception
     * (the instruction which generated the exception, approximately).
     * Note that this value is 24 bits internally, and is stored as such in
     * the audio subsystem's preserve block, but only the LS 16 bits make it
     * out in messages to Curator / Host.
     */
    PANIC_AUDIO_STACK_EXCEPTION = 0x0014,

    /**
     * Internal error: attempt to connect a remote buffer directly to audio
     * interface hardware
     */
    PANIC_AUDIO_REMOTE_BUFFER_CONNECTING_TO_AUDIO_PORT = 0x0015,

    /**
     * Something went wrong when copying data in 'prodtest'.
     * (Not used in Kymera.)
     */
    PANIC_AUDIO_PROD_TEST_DATA_READ_WRITE_FAIL = 0x0016,

    /**
     * An attempt to modify the offset on a protected remote MMU handle
     */
    PANIC_AUDIO_MODIFY_ATTEMPT_ON_PROTECTED_MMU_HANDLE = 0x0017,

    /**
     * One of the memory heaps (DM or PM) was unable to allocate the
     * requested memory. Argument is the size of the failed request.
     */
    PANIC_AUDIO_HEAP_EXHAUSTION = 0x0018,

    /**
     * pfree was passed an invalid pointer; argument is the bad pointer
     */
    PANIC_AUDIO_FREE_INVALID = 0x0019,

    /**
     * The message count limit was exceeded
     */
    PANIC_AUDIO_TOO_MANY_MESSAGES = 0x001A,

    /**
     * A timer fired but the handler function was NULL
     */
    PANIC_AUDIO_TIMER_NO_HANDLER = 0x001B,

    /**
     * The requested MMU buffer size is bigger than we can support
     * Argument is the requested size, in octets.
     */
    PANIC_AUDIO_MMU_TOO_LARGE = 0x001C,

    /**
     * We ran out of hardware (BAC) buffer handles
     */
    PANIC_AUDIO_BUFFER_HANDLES_EXHAUSTED = 0x001D,

    /**
     * Dynamic task API has been passed invalid parameters
     * (No longer used, see PANIC_OXYGOS_DYNAMIC_TASK_INVALID_PARAMS.)
     */
    PANIC_AUDIO_DYNAMIC_TASK_INVALID_PARAMS = 0x001E,

    /**
     * Too many dynamic tasks in the system
     * (No longer used, see PANIC_OXYGOS_TOO_MANY_DYNAMIC_TASKS.)
     */
    PANIC_AUDIO_TOO_MANY_DYNAMIC_TASKS = 0x001F,

    /**
     * Internal error: unknown message to ACCMD task
     */
    PANIC_AUDIO_ACCMD_TASK_UNKNOWN_MSG = 0x0020,

    /**
     * Internal error: invalid message to ACCMD task
     */
    PANIC_AUDIO_ACCMD_TASK_INVALID_MSG = 0x0021,

    /**
     * Internal error: message received for invalid ACCMD connection
     */
    PANIC_AUDIO_ACCMD_INVALID_CONIDX = 0x0022,

    /**
     * The ACCMD transmit queue has run out of capacity; argument is queue size
     */
    PANIC_AUDIO_ACCMD_TX_QUEUE_FULL = 0x0023,

    /**
     * An attempt to send an (ACCMD-related) message over ISP has failed
     */
    PANIC_AUDIO_ISP_SEND_FAILED = 0x0024,

    /**
     * Timer Panics: Attempt to schedule a new timer with invalid parameters
     */
    PANIC_AUDIO_TIMER_INVALID_PARAMS = 0x0025,

    /**
     * Timer Panics: Attempt to update a timer that was not created to be periodic
     */
    PANIC_AUDIO_TIMER_NOT_REPEATING_EVENT = 0x0026,

    /**
     * Timer Panics: Attempt to do a busy wait for invalid time
     */
    PANIC_AUDIO_TIMER_INVALID_DELAY = 0x0027,

    /**
     * Opmgr is missing the function table for the capability specified in the
     * argument.
     */
    PANIC_AUDIO_OPERATOR_MISSING_FUNCTIONS = 0x0028,

    /**
     * Opmgr reset failed. Probably indicates a lack of memory. Argument
     * contains the operator ID that failed to reset
     */
    PANIC_AUDIO_OPMGR_RESET_FAILED = 0x0029,

    /**
     * Opmgr created an operator but the source/sink count of the operator is
     * not within the permitted range
     */
    PANIC_AUDIO_SRC_SINK_CNT_OUT_OF_RANGE = 0x002a,

    /**
     * An operator sent an unsolicited message to the host but Opmgr detected
     * that the message was not delivered. This is probably due to destination
     * ACCMD connection having been closed. The argument contains the operator
     * number that sent the message (note internal numbering format not
     * external)
     */
    PANIC_AUDIO_MESSAGE_FROM_OPERATOR_FAILED = 0x002b,

    /**
     * Something attempted to use a configuration key or endpoint ID which
     * does not exist.
     */
    PANIC_AUDIO_INVALID_KEY_OR_ENDPOINT_ID = 0x002c,

    /**
     * We handled an operator_bgint, but the context pointer was invalid.
     */
    PANIC_AUDIO_INVALID_OPERATOR_CONTEXT = 0x002d,

    /**
     * An invalid endpoint somehow got used for scheduling a chain in streams
     */
    PANIC_AUDIO_INVALID_ENDPOINT_IN_SCHEDULE_CONTEXT = 0x002e,

    /**
     * (can't remember what this was, but it's no longer used)
     */
    PANIC_AUDIO_SCHEDULING_FAILED_BADLY = 0x002f,

    /**
     * A SCO record entry was not found for the hci_handle.
     * The argument contains the hci_handle.
     */
    PANIC_AUDIO_SCO_INVALID_HCI_HANDLE = 0x0030,

    /**
     * The wallclock counter published by the Bluetooth subsystem indicates
     * that it is being updated, but audio firmware has timed out waiting
     * for the update to complete. This probably indicates the BTSS not
     * behaving properly or audio reading the buffer wrong.
     * The argument contains the hci_handle of the SCO link whose wallclock
     * caused the panic.
     */
    PANIC_AUDIO_WALLCLOCK_UPDATE_TIMEOUT = 0x0031,

    /**
     * A transform somehow got into an invalid state.
     * The argument contains the transform_id (internal).
     */
    PANIC_AUDIO_STREAM_TRANSFORM_INVALID_STATE = 0x0032,

    /**
     * A transform that "shouldn't" be kicked has just
     * been kicked.
     * The argument contains the internal transform ID.
     */
    PANIC_AUDIO_STREAM_TRANSFORM_KICKED_INCORRECTLY =0x0033,

    /**
     * A transform couldn't be configured when it should have.
     * The argument contains the internal transform ID.
     */
    PANIC_AUDIO_STREAM_TRANSFORM_CONFIGURATION_FAIL = 0x0034,

    /**
     * A SCO type endpoint was used wrongly, or a SCO operation
     * was attempted on a non-SCO endpoint.
     */
    PANIC_AUDIO_STREAM_INVALID_SCO_ENDPOINT = 0x0035,

    /**
     * Somehow our recursive depth didn't return to 0 when leaving
     * the stream timing calculations. This will probably be
     * due to one of the recursive functions not reducing the
     * depth on some code path.
     */
    PANIC_AUDIO_STREAM_FAILED_TO_UNWIND_RECURSION_FULLY = 0x0036,

    /**
     * An invalid voice slot ID has been detected. The argument contains the
     * invalid voice slot ID.
     */
    PANIC_AUDIO_VOICE_SLOT_ID_INVALID = 0x0037,

    /**
     * An unexpected voice slot mask has been detected. The argument contains
     * the unexpected voice slot mask.
     */
    PANIC_AUDIO_VOICE_SLOT_MASK_INVALID = 0x0038,

    /**
     * An invalid/unrecogised endpoint type was found.
     */
    PANIC_AUDIO_STREAM_INVALID_ENDPOINT_TYPE = 0x0039,

    /**
     * A cache-execution exception fired.
     * The argument is the value of rIntLink after the exception interrupt.
     * Note that this value is 24 bits internally, and is stored as such in
     * the audio subsystem's preserve block, but only the LS 16 bits make it
     * out in messages to Curator / Host.
     */
    PANIC_AUDIO_CACHE_EXCEPTION = 0x003A,

    /**
     * A sid was used incorrectly in the context of the audio drivers.
     */
    PANIC_AUDIO_SID_ROUTING_INCORRECT = 0x003B,

    /**
     * Internal error: an unexpected NULL pointer to ACCMD_ASYNC_DETAILS was
     * encountered.
     */
    PANIC_AUDIO_ACCMD_ASYNC_DETAILS_NULL = 0x003C,

    /**
     * An invalid or missing parameter or option IE was encountered in a
     * response from the Bluetooth subsystem during the start up of the
     * to-air shunt service.
     * The argument contains the sstag of the service instance.
     */
    PANIC_AUDIO_INVALID_TO_AIR_SHUNT_SERVICE_IE = 0x003D,

    /**
     * An instance of the to-air shunt service could not be stopped.
     * The argument contains the sstag of the service instance.
     */
    PANIC_AUDIO_FAILED_TO_STOP_TO_AIR_SHUNT_SERVICE = 0x003E,

    /**
     * A badly formed to-air shunt auxiliary message was received. This could
     * be due to unrecognised/inconsistent data or a wrong message length.
     * The argument contains the received auxiliary message ID.
     */
    PANIC_AUDIO_BADLY_FORMED_TASAUX_MSG_RECEIVED = 0x003F,

    /**
     * A to-air shunt endpoint was unable to be closed on receipt of a
     * TASAUX_SIGNAL_ID_ACL_DROPPED auxiliary command channel message.
     * The argument contains the external ID of the endpoint.
     */
    PANIC_AUDIO_FAILED_TO_CLOSE_TO_AIR_SHUNT_ENDPOINT = 0x0040,

    /**
     * An operator could not be stopped, and we needed to stop it. If this
     * occurs an ACCMD service that owns the operator in the argument is
     * probably going away.
     */
    PANIC_AUDIO_OPERATOR_STOP_FAILED = 0x0041,

    /**
     * A running prodtest could not be stopped. If this occurs then the
     * prodtest resources are in an unknown state.
     * (not used on Kymera)
     */
    PANIC_AUDIO_PRODTEST_STOP_FAILED = 0x0042,

    /**
     * A buffer monitoring operation was attempted on a non-audio endpoint.
     */
    PANIC_AUDIO_STREAM_MONITOR_ENDPOINT_NOT_SUPPORTED = 0x0043,

    /**
     * We failed to set a stream monitor because we didn't have any monitors
     * free.
     */
    PANIC_AUDIO_STREAM_MONITOR_TOO_FEW_RESOURCES = 0x0044,

    /**
     * The operator capability ID hasn't been set in the capability structure
     */
    PANIC_AUDIO_OPERATOR_CAP_TYPE_NOT_SET = 0x0045,

    /**
     * An unknown capability ID has been encountered in an operator's data
     * structure.
     * (This should never happen, so may indicate memory corruption.)
     */
    PANIC_AUDIO_OPMGR_CAPABILITY_UNKNOWN = 0x0046,

    /**
     * Internal error: attempt to configure a non-existent audio interface
     */
    PANIC_AUDIO_CONFIGURING_NONEXISTENT_INTERFACE = 0X0047,

    /* Unused 0x0048
     * Previously PANIC_AUDIO_STREAM_FORCE_CLOSE_CALLED_WITH_BAD_DEVICE
     * Never actually used in Kymera, and unlikely to conflict with Amber use
     * So this can probably be safely recycled
     */

    /**
     * An instance of the from-air shunt service could not be stopped.
     * The argument contains the sstag of the service instance.
     */
    PANIC_AUDIO_FAILED_TO_STOP_FROM_AIR_SHUNT_SERVICE = 0x0049,

    /**
     * An invalid or missing parameter or option IE was encountered in a
     * response from the BTSS during the start up of the from-air shunt service.
     * The argument contains the sstag of the service instance.
     */
    PANIC_AUDIO_INVALID_FROM_AIR_SHUNT_SERVICE_IE = 0x004A,

    /**
     * A badly formed from-air shunt auxiliary message was received. This could
     * be due to unrecognised/inconsistent data or a wrong message length.
     * The argument contains the received auxiliary message ID.
     */
    PANIC_AUDIO_BADLY_FORMED_FASAUX_MSG_RECEIVED = 0x004B,

    /**
     * A from-air shunt endpoint was unable to be closed.
     * The argument contains the SSTAG.
     */
    PANIC_AUDIO_INVALID_SSTAG_FOR_SHUNT_ENDPOINT = 0x004C,

    /**
     * An instance of the Bluetooth Information service could not be stopped.
     * The argument contains the sstag of the service instance.
     */
    PANIC_AUDIO_FAILED_TO_STOP_BT_INFO_SERVICE = 0x004D,

    /**
     * Stopping stream monitor interrupts failed.
     */
    PANIC_AUDIO_STREAM_MONITOR_STOP_FAILED = 0x004E,

    /**
     * Kymera composite connection ID is invalid (sender and/or receiver ID part)
     */
    PANIC_AUDIO_INVALID_CONNECTION_ID = 0x004F,

    /**
     * The SCO manager message handler received a non-SCO related message
     */
    PANIC_AUDIO_SCO_MGR_UNKNOWN_MSG = 0x0050,

    /**
     * A SCO related kalimba comms message was received when one isn't expected.
     */
    PANIC_AUDIO_UNEXPECTED_SCO_MSG = 0x0051,

    /**
     * Some generic unsolicited message sending failed. For specific ones, like message
     * from operator, use their dedicated panic ID instead of this.
     */
    PANIC_AUDIO_UNSOLICITED_MESSAGE_FAILED = 0x0052,

    /**
     * During stream_connect, one of the endpoints asked for a non-software
     * buffer, on a platform where such a connection is not supported.
     */
    PANIC_AUDIO_NOT_SW_BUFFER = 0x0053,

    /**
     * Invalid Transform. Transform not found in the Transformlist
     */
    PANIC_AUDIO_INVALID_TRANSFORM = 0x0054,

    /**
     * Invalid Endpoint. The endpoint is not in the endpoint list.
     */
    PANIC_AUDIO_INVALID_ENDPOINT = 0x0055,

    /**
     * Transform cannot be disconnected
     */
    PANIC_AUDIO_TRANSFORM_DISCONNECT_FAILURE = 0x0056,

    /**
     * Insufficient RAM to clear up a connection's operators
     */
    PANIC_AUDIO_OPMGR_DESTROY_BY_CONID_NO_RESOURCES = 0x0057,

    /**
     * The operator task handler received a message with invalid ID
     */
    PANIC_AUDIO_OPMGR_INVALID_MSG_ID = 0x0058,

    /**
     * The operator task has no operator data
     */
    PANIC_AUDIO_OPMGR_NO_OP_DATA = 0x0059,

    /**
     * The operator task has no data
     */
    PANIC_AUDIO_OPMGR_NO_TASK_DATA = 0x005A,

    /**
     * The operator hasn't created any response message.
     * Probably the op structure is corrupted
     */
    PANIC_AUDIO_OPMGR_NO_RESPONSE_FROM_OPERATOR = 0x005B,

    /**
     * Two attempts were made to allocate shared memory with the same ID
     * (argument) but incompatible size or location.
     */
    PANIC_AUDIO_SHARED_MEM_INVALID_MALLOC = 0x005C,

    /**
     * No shared memory was found with the given address.
     */
    PANIC_AUDIO_SHARED_MEM_FREE_INVALID_POINTER = 0x005D,

    /**
     * No shared memory was found with the given ID.
     */
    PANIC_AUDIO_SHARED_MEM_FREE_INVALID_ID = 0x005E,

    /**
     * Not enough memory for creating an operator response.
     */
    PANIC_AUDIO_NOT_ENOUGH_MEMORY_FOR_OPERATOR = 0x005F,

    /**
     * Most probably the shared_memory_id enumeration was not updated
     * after a new shared memory was added to enc_shared_malloc_table
     * or dec_shared_malloc_table or wbs_shared_malloc_table.
     */
    PANIC_AUDIO_WBS_SHARED_MEM_IDS_NOT_MATCH = 0x0060,

    /**
     * An attempt was made to commit more scratch memory than
     * the amount reserved by the current task.
     */
    PANIC_AUDIO_SCRATCH_MEMORY_OVERCOMMITTED = 0x0061,

    /**
     * An invalid request was made to the scratch memory system;
     * e.g., attempting to reserve memory without registering first.
     */
    PANIC_AUDIO_SCRATCH_MEMORY_BAD_REQUEST = 0x0062,

    /**
     * A-law and mu-law cbops conversions were requested simultaneously.
     * This is not a valid combination.
     */
    PANIC_AUDIO_INVALID_CBOP_COMBINATION = 0x0063,

    /**
     * An unrecognised message was passed to the BlueCore audio endpoint
     * message handler.
     */
    PANIC_AUDIO_UNRECOGNISED_AUDIO_MESSAGE = 0x0064,

    /**
     * Jumped into the startup code at a time other than hardware
     * subsystem boot.
     * This is often the result of calling a NULL function pointer.
     * The argument is the value of rLink at the time this was detected.
     * If nonzero, it's probably a clue to who called a NULL function
     * pointer. If zero, we probably hit this panic because someone popped
     * a zero off the stack into rLink and then did 'rts'... in that case
     * you'll have to find the culprit by some other means.
     */
    PANIC_AUDIO_UNEXPECTED_RESTART = 0x0065,

    /**
     * An invalid audio hardware instance has been referenced internally.
     * The argument is the invalid audio hardware instance.
     */
    PANIC_AUDIO_INVALID_HARDWARE_INSTANCE = 0x0066,

    /**
     * An invalid configure key was sent to an endpoint. The endpoint
     * did not expect to receive this message in its current state.
     */
    PANIC_AUDIO_STREAM_INVALID_CONFIGURE_KEY = 0x0067,

    /**
     * BlueCore: integrity of persistent store information is broken. Most
     * likely that DSP PS keys have been altered by an external entity while
     * DSP was running, or already in init phase the information about PS keys
     * could not be built correctly due to some system failure.
     */
    PANIC_AUDIO_PS_CORRUPT_INFO = 0x0068,

    /**
     * A cbops chain / section is about to be used but the head structure
     * is not initialised.
     */
    PANIC_AUDIO_CBOPS_HEAD_NOT_INITIALISED = 0x0069,

    /**
     * Some cbops structure value has been detected to be outside its normal limits.
     */
    PANIC_AUDIO_CBOPS_OUT_OF_RANGE = 0x006A,

    /**
     * Deferred endpoint creation sequence error.
     * Either we tried to create another endpoint while one was still in progress,
     * or there was no suitable data to process the completion.
     */
    PANIC_AUDIO_ENDPOINT_CREATION_SEQUENCE_ERROR = 0x006B,

    /**
     * The ratematching manager can't realise a valid solution to the graph's
     * ratematching needs.
     */
    PANIC_AUDIO_RATEMATCHING_LIMITATION = 0x006C,

    /**
     * Registered handler for BAC monitor interrupt is NULL or invalid.
     */
    PANIC_AUDIO_BAC_MONITOR_HANDLER_INVALID = 0x006D,

    /**
     * The operator returned FALSE but also provided a response message.
     * This is a capability API usage error.
     */
    PANIC_AUDIO_OPMGR_INVALID_RESPONSE_FROM_OPERATOR = 0x006E,

    /**
     * Deferred endpoint close sequence error.
     */
    PANIC_AUDIO_ENDPOINT_CLOSE_SEQUENCE_ERROR = 0x006F,

    /**
     * Operator endpoint couldn't find the task of its operator.
     */
    PANIC_AUDIO_OPERATOR_HAS_NO_TASK = 0x0070,

    /**
     * Parameter checks in S/PDIF endpoint failed.
     */
    PANIC_SPDIF_ENDPOINT_INVALID_PARAM =  0x0071,

    /**
     * CSRatlas7: APB target returned an error.
     * We have probably tried to access a non-existing address (or protected) using
     * Keyhole.
     */
    PANIC_AUDIO_APB_SLV_ERROR = 0x0072,

    /**
     * The accessed device is invalid/unsupported.
     */
    PANIC_AUDIO_INVALID_DEVICE_ID = 0x0073,

    /**
     * Attempt made to rate-match an endpoint with itself.
     */
    PANIC_AUDIO_RATEMATCHING_SAME_ENDPOINT = 0x0074,

    /**
     * No buffer was associated with the in-place transform.
     */
    PANIC_AUDIO_IN_PLACE_DISCONNECT = 0x0075,

    /**
     * Setting up the kick for the in-place chain failed.
     */
    PANIC_AUDIO_IN_PLACE_KICK_SETUP_FAILED = 0x0076,

    /**
     * Cancelling the kick for the in-place chain failed.
     */
    PANIC_AUDIO_IN_PLACE_KICK_CANCEL_FAILED = 0x0077,

    /**
     * Aligning a buffer's metadata with its data read/write offsets is
     * only currently supported for pure software buffers.
     */
    PANIC_AUDIO_METADATA_HW_BUFFERS_NOT_SUPPORTED = 0x0078,

    /**
     * octet_buffers currently only support certain packing types.
     */
    PANIC_AUDIO_OCTET_BUFFER_UNSUPPORTED_PACKING = 0x0079,

    /**
     * The RTP decode capability encountered a codec payload type it does
     * not support.
     */
    PANIC_AUDIO_RTP_UNSUPPORTED_CODEC = 0x007A,

    /* Unused 0x007B. But this was previously allocated for audio resource manager
     * as PANIC_AUDIO_RES_MGR_INIT_FAILED and released to customers in Alpha ADK
     * Do not reallocate the panic ID until Alpha ADK falls out of use */

    /* Unused 0x007C. But this was previously allocated for audio resource manager
     * as PANIC_AUDIO_RES_MGR_INVALID_PIN_RESOURCE and released to customers in
     * Alpha ADK Do not reallocate the panic ID until Alpha ADK falls out of use */

    /**
     * Audio resource manager unable to send a release request.
     */
    PANIC_AUDIO_RES_MGR_UNABLE_TO_RELEASE = 0x007D,

    /**
     * Audio resource manager got unexpected message from subres.
     */
    PANIC_AUDIO_RES_MGR_UNSUPPORTED_SUBRES_OP = 0x007E,

    /**
     * Internal error in MCLK manager (setup).
     */
    PANIC_AUDIO_MCLK_SETUP_FAILURE = 0x007F,

    /**
     * Internal error in MCLK manager (sequence of events).
     */
    PANIC_AUDIO_MCLK_ACTIVATION_SEQUENCE_ERROR=0x0080,

    /**
     * Insufficient memory for response message
     */
    PANIC_AUDIO_KIP_NO_MEMORY_FOR_RESPONSE_MSG = 0x0081,

    /**
     * Private (per-processor) memory corruption detected.
     */
    PANIC_AUDIO_PRIVATE_MEMORY_CORRUPTED = 0x0082,

    /**
     * Inter-processor comms look up table error.
     */
    PANIC_IPC_LUT_CORRUPTED = 0x0083,

    /**
     * Inter-processor comms failure of some sort.
     */
    PANIC_IPC_FAILED = 0x0084,

    /**
     * Attempt to release a hardware semaphore that has already been
     * released.
     */
    PANIC_IPC_HWSEW_ALREADY_RELEASED = 0x0085,

    /**
     * Entering an entry into the LUT failed.
     */
    PANIC_IPC_LUT_SETLOOKUP_FAILED = 0x0086,

    /**
     * Reading an entry from the LUT failed
     */
    PANIC_IPC_LUT_GETLOOKUP_FAILED = 0x0087,

    /**
     * Invalid processor ID
     */
    PANIC_IPC_INVALID_PROCID = 0x0088,

    /* Unused 0x0089. But this was previously allocated for audio dual-core
     * as PANIC_AUDIO_KIP_CONNECT_INVALID_STATE and released to customers in
     * ADK 6. Do not reallocate the panic ID until ADK 6 falls out of use */

    /**
     * Invalid heterogeneous (multi-processor) operator list state; context
     * not retrieved from conid.
     */
    PANIC_AUDIO_OPMGR_OPLIST_LOST_CONTEXT = 0x008A,

    /**
     * Audio data service failed to allocate a metadata tag.
     */
    PANIC_AUDIO_DATA_METADATA_FAILED = 0x008B,

    /**
     * Not enough memory for creating a timed playback instance.
     */
    PANIC_AUDIO_NOT_ENOUGH_MEMORY_FOR_TIMED_PLAYBACK = 0x008C,

    /**
     * The initialisation of the timed playback module failed.
     */
    PANIC_AUDIO_TIMED_PLAYBACK_INIT_FAIL = 0x008D,

    /**
     * The read octet offset and the write octet offset are not equal.
     * cbuffer_copy_16bit_be_zero_shift_ex does not support such a copy.
     */
    PANIC_AUDIO_CBUFFER_EX_COPY_UNEQUAL_OFFSET = 0x008E,

    /**
     * Attempted to use too many exported constant tables.
     */
    PANIC_AUDIO_TOO_MANY_DIFFERENT_CONSTANTS = 0x008F,

    /**
     * Find operator pointer for a given operator ID failed.
     */
    PANIC_AUDIO_OPLIST_INVALID_OPID = 0x0090,

    /**
     * No memory to store context/state.
     */
    PANIC_AUDIO_KIP_NO_MEMORY_TO_STORE_CONTEXT = 0x0091,

    /**
     * QCA669x: COMMS max payload size violation
     */
    PANIC_AUDIO_COMMS_MSG_MAX_PAYLOAD_SIZE_VIOLATION = 0x0092,

    /**
     * QCA669x: COMMS payload pointer is invalid
     */
    PANIC_AUDIO_COMMS_MSG_PAYLOAD_PTR_INVALID = 0x0093,

    /* Efuse forbids use of requested feature */
    PANIC_AUDIO_EFUSE_FEATURE_DISALLOWED = 0x0094,

    /* ANC failed to enable/disable due to sequencing error */
    PANIC_AUDIO_ANC_ENABLE_ERROR = 0x0095,

    /*
     * The BULK interface limit was exceeded
     */
    PANIC_AUDIO_TOO_MANY_BULK_INTERFACES = 0x0096,

    /**
     * Audio amplifier over current detected.
     * The analogue macro has disabled itself automatically to
     * prevent damage to the chip.
     */
    PANIC_AUDIO_OVER_CURRENT_DETECTED = 0x0097,

    /**
     * VCI operator has no operator connected on sink terminal 0.
     */
    PANIC_AUDIO_AOV_VCI_SINK_NOT_CONNECTED = 0x0098,

    /**
     * VTD operator has no operator connected on sink terminal 0.
     */
    PANIC_AUDIO_AOV_VTD_SINK_NOT_CONNECTED = 0x0099,

    /**
     * AOV graph missing a required endpoint.
     */
    PANIC_AUDIO_AOV_GRAPH_ENDPOINT_ABSENT = 0x009A,

    /**
     * An ACCMD connection id corresponding to an
     * ACCMD global stag can't be found.
     */
    PANIC_AUDIO_AOV_ACCMD_CONN_ID_NOT_FOUND = 0x009B,

    /* *******************  Hydra common panic IDs start at 0x1000 ********************** */


    /*
     * This is the start of debug panic IDs that should never occur in
     * production code. These are for events in the following categories:
     *   + things that really are impossible (yeah, right)
     *   + temporary panics in place of recovery code
     * Really, all of these should be become either real, documented panics or
     * faults (with recovery).
     *
     *   + special panics for debugging modes
     * These should only be emitted with the right debug options enabled.
     */

    /**
     * Assert has detected a problem.
     */
    PANIC_AUDIO_DEBUG_ASSERT_FAILED = 0x4000,

    /**
     * 24 Bit number is oversize
     */
    PANIC_AUDIO_DEBUG_NOT_24_BITS = 0x4001,

    /**
     * Streams has requested an operation on an endpoint for which
     * opmgr can't find the corresponding operator
     */
    PANIC_AUDIO_OPERATOR_NOT_FOUND = 0x4004,

    /**
     * The operator has requested an endpoint data type that is not
     * supported or endpoint has not specified a data type
     */
    PANIC_AUDIO_OPMGR_INVALID_DATA_TYPE = 0x4005,

    /**
     * An attempt has been made to set a timer for a time which
     * has long expired
     */
    PANIC_AUDIO_TIMER_TOO_OLD = 0x4006,

    /**
     * Memory debug has detected corruption
     */
    PANIC_AUDIO_DEBUG_MEMORY_CORRUPTION = 0x4007,

    /**
     * An operator associated with an ACCMD connection could not
     * be destroyed
     */
    PANIC_AUDIO_OPERATOR_DESTROY_FAILED = 0x4008,

    /**
     * The special ACCMD which exists to deliberately trigger a panic in the
     * audio subsystem for testing purposes has done so.
     */
    PANIC_AUDIO_ACCMD_PANIC_TRIGGER = 0x4009,

    /**
     * An endpoint was asked to disconnect while it was still running. There
     * shouldn't be a code path that allows this.
     */
    PANIC_AUDIO_UNEXPECTED_ENDPOINT_DISCONNECT = 0x400A,

    /**
     * A BlueCore SCO request message failed. This shouldn't happen unless we
     * sent something illogical.
     */
    PANIC_AUDIO_SCO_MGR_REQ_FAILED = 0x400B,

    /**
     * Stream dummy start was called.
     */
    PANIC_AUDIO_STREAM_INVALID_START = 0x400C,

    /**
     * Stream dummy stop was called.
     */
    PANIC_AUDIO_STREAM_INVALID_STOP = 0x400D,

    /**
     * Stream code path somehow allowed operator connect to be called for an
     * already connected terminal.
     */
    PANIC_AUDIO_ALREADY_CONNECTED_OPERATOR_TERMINAL = 0x400E,

    /**
     * Stream code path somehow allowed operator disconnect to be called for an
     * already disconnected terminal.
     */
    PANIC_AUDIO_ALREADY_DISCONNECTED_OPERATOR_TERMINAL = 0x400F,

    /**
     * Audio endpoint processing took much longer than expected.
     */
    PANIC_AUDIO_PROCESSING_OVERRUN = 0x4010,

    /**
     * ACCMD connection index not found.
     */
    PANIC_AUDIO_ACCMD_CONIDX_NOT_FOUND = 0x4011,

    /**
     * AOV fsm has seen an invalid clock change request.
     */
    PANIC_AUDIO_AOV_UNEXPECTED_CLOCK_TRANSITION_REQUEST = 0x4012,

    /**
     * AOV Duty timer is in use when it shouldn't be.
     */
    PANIC_AUDIO_AOV_DUTY_TIMER_ERROR = 0x4013,

    /**
     * AOV fsm entered error state.
     */
    PANIC_AUDIO_AOV_IN_ERROR_STATE = 0x4014,


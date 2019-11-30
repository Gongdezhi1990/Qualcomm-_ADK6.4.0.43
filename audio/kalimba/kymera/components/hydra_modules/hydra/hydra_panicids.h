/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/* This header fragment is to be included by panicids.h
 * It cannot stand alone
 */
    /**
     * The code that blocks and unblocks interrupts has lost state. This
     * probably means that code has called one of the unblock_...() functions
     * without first calling the corresponding block_...() function. */
    PANIC_HYDRA_INTERRUPT_BLOCK = 0x1001,

    /**
     * Interrupts have been blocked for longer than allowed. This probably
     * indicates a software design problem. */
    PANIC_HYDRA_INTERRUPT_TOO_LONG = 0x1002,

    /**
     * The software has run out of hardware event sources to use for
     * generating software interrupts. */
    PANIC_HYDRA_TOO_MANY_SW_INTERRUPTS = 0x1003,

    /**
     * An attempt was made to unconfigure a software interrupt that is not
     * currently configured.
     */
    PANIC_HYDRA_INVALID_SW_INTERRUPT = 0x1004,

    /**
     * The processor private memory manager (pmalloc) has run out of memory.
     * The paramter is the number of chars requested.
     */
    PANIC_HYDRA_PRIVATE_MEMORY_EXHAUSTION = 0x1005,

    /**
     * The configuration used to create the private memory manager's (pmalloc)
     * memory pools is invalid. */
    PANIC_HYDRA_INVALID_PRIVATE_MEMORY_CONFIG = 0x1006,

    /**
     * An invalid pointer has been passed to the private memory manager
     * (pmalloc), probably to pfree(). */
    PANIC_HYDRA_INVALID_PRIVATE_MEMORY_POINTER = 0x1007,

    /**
     * This id is for local test code only. DO NOT CHECK IN CODE USING THIS ID
     */
    PANIC_HYDRA_MYSTERY = 0x1008,

    /**
     * Scheduler related badness
     */
    PANIC_HYDRA_QUEUE_ACCESS_VIOLATION = 0x1009,

    /**
     * Scheduler related badness
     */
    PANIC_HYDRA_TOO_MANY_TIMED_EVENTS = 0x100a,

    /**
     * The software is attempting to preserve more data across a reset then
     * space has been reserved for. This indicates an internal inconsistency
     * within the software. */
    PANIC_HYDRA_PRESERVED_MEMORY_TOO_LARGE = 0x100b,

    /**
     * The software dereferenced a NULL function pointer. */
    PANIC_HYDRA_BRANCH_THROUGH_ZERO = 0x100c,

    /**
     * An unrecognised reset type was requested. This probably means that
     * there is an incosistency in the software. */
    PANIC_HYDRA_UNKNOWN_RESET = 0x100d,

    /**
     * The processor was unexpectedly reset by the watchdog timer. This
     * probably means that the software entered an infinite loop or the
     * processor started executing incorrect code. */
    PANIC_HYDRA_WATCHDOG_RESET = 0x100e,

    /**
     * The watchdog code has lost state. This probably means that calls to
     * the normal and exclusive watchdog functions have been incorrectly
     * interleaved. */
    PANIC_HYDRA_WATCHDOG_EXCLUSIVE = 0x100f,

    /**
     * The processor was unexpectedly halted. This is caused by wd_halt()
     * being called without a preceding call to wd_pre_halt(). */
    PANIC_HYDRA_WATCHDOG_HALT = 0x1010,

    /**
     * The software stack has overflowed. This is likely to mean that either
     * too little memory was allocated (at compile time) to the stack, or
     * something went badly wrong (at run time). */
    PANIC_HYDRA_STACK_OVERFLOW = 0x1011,

    /**
     * With the DEBUG_MMU_PROC_PORT_SAVE option, restored buffer info was trashed*/
    PANIC_HYDRA_MMU_BAD_SAVE_INFO         = 0x1012,

    /**
     * A run time assertion failed */
    PANIC_HYDRA_ASSERTION_FAILED          = 0x1013,

    /**
     * An invalid MMU buffer handle has been passed to one of the buffer
     * handling functions. This is probably due to a handle being used after
     * its buffer has been destroyed. */
    PANIC_HYDRA_INVALID_MMU_HANDLE = 0x1014,

    /**
     * An out-of-range or badly aligned MMU buffer index has been passed to
     * one of the buffer handling functions.*/
    PANIC_HYDRA_INVALID_MMU_INDEX = 0x1015,

    /**
     * An unsuitable MMU port identifier has been passed to one of the buffer
     * handling functions. */
    PANIC_HYDRA_INVALID_MMU_PORT = 0x1016,

    /**
     * An attempt to allocate memory for storing data within an MMU buffer
     * failed due to there being insufficient free memory */
    PANIC_HYDRA_MMU_FREE_PAGE_EXHAUSTION = 0x1017,

    /**
     * An attempt was made to allocate memory to a region of an MMU buffer
     * that already had memory assigned */
    PANIC_HYDRA_MMU_PAGE_ALREADY_ALLOCATED = 0x1018,

    /**
     * An attempt was made to free memory from a region of an MMU buffer that
     * did not have memory assigned */
    PANIC_HYDRA_MMU_PAGE_NOT_ALLOCATED = 0x1019,

    /**
     * Pmalloc can't give memory requested
     */
    PANIC_HYDRA_HEAP_EXHAUSTION    = 0x101b,

    /**
     * Scheduler related badness
     */
    PANIC_HYDRA_TOO_MANY_MESSAGES  = 0x101c,

    /**
     * Scheduler related badness
     */
    PANIC_HYDRA_INVALID_RUNLEVEL   = 0x101d,

    /**
     * Scheduler related badness
     */
    PANIC_HYDRA_UNKNOWN_TASK       = 0x101e,

    /**
     * Scheduler related badness
     */
    PANIC_HYDRA_UNKNOWN_QUEUE      = 0x101f,

    /**
     * An attempt was made to access non-existent MMU buffer memory,
     * and no pages were free to map in. */
    PANIC_HYDRA_SEG_FAULT = 0x1020,

    /**
     * A request was made for a buffer size bigger than the MMU supports */
    PANIC_HYDRA_MMU_TOO_LARGE = 0x1021,

    /**
     * A timed interrupt has been requested but no callback function was
     * specified. */
    PANIC_HYDRA_ITIME_NO_HANDLER = 0x1022,

    /**
     * A timed interrupt occurred excessively late. */
    PANIC_HYDRA_ITIME_EXCESSIVE_LATENCY = 0x1023,

    /**
     * An itime event claimed to be set but was not found in the list */
    PANIC_HYDRA_ITIME_EVENT_NOT_FOUND = 0x1024,

    /**
     * An attempt to create an SDIO buffer failed */
    PANIC_HYDRA_HOSTIF_SDIO_BUFFER_CREATION_FAILED = 0x1025,

    /**
     * An attempt to create a UART buffer failed */
    PANIC_HYDRA_HOSTIF_UART_BUFFER_CREATION_FAILED = 0x1026,

    /**
     * A required buffer could not be created */
    PANIC_HYDRA_BUFFER_CREATION_FAILED = 0x1028,

    /**
     * Call made to fault_diatribe() provoked panic() because PanicOnFault is set. */
    PANIC_HYDRA_ON_FAULT = 0x1029,

    /**
     * Panic when slow clock calibration fails in dorm_hibernate */
    PANIC_HYDRA_DORM_CALCULATION     = 0x1038,

    /**
     * Too many random numbers were requested in one go. */
    PANIC_HYDRA_RAND_INVALID_REQUEST = 0x103e,

    /**
     * A required MIB value was missing */
    PANIC_HYDRA_MIB_MISSING_DEFAULT = 0x1043,

    /**
     * Attempt to configure a buffer that was already set
     */
    PANIC_SUBMSG_BUFFER_ALREADY_SET = 0x104f,
    /**
     * Attempt to configure a channel out of the allowed range
     */
    PANIC_SUBMSG_CHANNEL_TOO_LARGE = 0x1050,
    /**
     * Length of PDU is too short or too long to transmit
     */
    PANIC_SUBMSG_INVALID_TX_LENGTH = 0x1051,
    /**
     * Reported length of received PDU is too short or too long
     */
    PANIC_SUBMSG_INVALID_RX_LENGTH = 0x1052,
    /**
     * Destination subsystem of PDU is not handled
     */
    PANIC_SUBMSG_INVALID_DESTINATION = 0x1053,
    /**
     * Attempt to remove invalid subsystem
     */
    PANIC_SUBMSG_INVALID_SUBSYSTEM = 0x1054,
    /**
     * Destination subsystem of PDU is known but is not yet configured
     */
    PANIC_SUBMSG_DESTINATION_NOT_CONFIGURED = 0x1055,
    /**
     * Source subsystem of PDU is not yet configured
     */
    PANIC_SUBMSG_SOURCE_NOT_CONFIGURED = 0x1056,
    /**
     * Channel for PDU is invalid (not configured).
     * This can also indicate the wrong API was used for
     * a particular port/channel (i.e. the data interface
     * for out-of-band reports, or the report interface
     * for in-band data).
     */
    PANIC_SUBMSG_INVALID_CHANNEL = 0x1057,
    /**
     * A TX interrupt was received when no transmission was active
     */
    PANIC_SUBMSG_TX_NOT_ACTIVE = 0x1058,
    /**
     * A TX interrupt was received when the transmit queue was empty
     */
    PANIC_SUBMSG_TX_QUEUE_EMPTY = 0x1059,
    /**
     * On reset there were data channel(s) not associated with a
     * subsystem.
     */
    PANIC_SUBMSG_EXTRANEOUS_CHANNELS = 0x105a,
    /**
     * Attempt to poll for transmission confirmations when polling
     * mode is not enabled.  All confirmations are handled by
     * interrupt, so this is a fatal error.
     */
    PANIC_SUBMSG_NOT_POLLING = 0x105b,
    /**
     * Overrun when attempting to write to a buffer used for messaging.
     * The remote subsystem has not yet finished reading up to the
     * point to which the local subsystem wishes to write.
     */
    PANIC_SUBMSG_BUFFER_OVERRUN = 0x105c,
    /**
     * Mismatch between transmission and transmission response.
     * The tag passed back with a transmit response is not
     * the one associated with the message at the front of the queue.
     */
    PANIC_SUBMSG_TX_RESPONSE_MISMATCH = 0x105d,

    /**
     * The length of an argument type in 16-bit words is not that
     * expected by a CCP function that deals with the type.  This is
     * probably because the type has a different length on the target
     * platform than on the XAP.
     *
     * The argument gives the CCP signal ID.
     */
    PANIC_CCP_BAD_TYPE_LENGTH = 0x105e,

    /**
     * A CCP function was called with an array length that
     * was out of range.
     */
    PANIC_CCP_BAD_ARRAY_LENGTH = 0x105f,

    /**
     * A CCP "recv" function is being called for the wrong PDU type.
     * The argument gives the CCP signal ID.
     */
    PANIC_CCP_BAD_RECV_TYPE = 0x1060,

    /**
     * An attempt was made to send a message to an inactive subsystem.
     * If the message was intentional, the subsystem should have been
     * turned on before the message was passed to submsg.
     */
    PANIC_SUBMSG_SUBSYSTEM_DOWN = 0x1061,

    /**
     * An attempt to read a message from the hardware failed.
     */
    PANIC_SUBMSG_RX_READ_FAILED = 0x1062,

    /**
     * Hydra TXBus Spin Wait Timed out */
    PANIC_HYDRA_TXBUS_WAIT_TIMED_OUT  = 0x1063,

    /**
     * A bad operation on an array of service information elements was
     * detected.
     */
    PANIC_SERVICE_BAD_IE = 0x1064,

    /**
     * A service instance in a message from \c globserv was not
     * found locally.  The argument is the service tag.
     */
    PANIC_SUBSERV_INSTANCE_NOT_FOUND = 0x1065,
    /**
     * A service instance was found internally to be in an invalid
     * state for the operation being performed.
     */
    PANIC_SUBSERV_INVALID_STATE = 0x1066,
    /**
     * A local request for a service operation was received
     * which referred to a non-existent or inappropriate service
     * instance.
     */
    PANIC_SUBSERV_INVALID_INSTANCE = 0x1067,
    /**
     * A function called from subserv returned an invalid value. 
     */
    PANIC_SUBSERV_INVALID_RETURN_VALUE = 0x1068,
    /**
     * A call to subserv supplied a service parameter with an
     * invalid type.
     */
    PANIC_SUBSERV_INVALID_PARAMETER_TYPE = 0x1069,
    /**
     * A value for a service parameter should have been filled
     * in but wasn't.
     */
    PANIC_SUBSERV_MISSING_PARAMETER_VALUE = 0x106a,
    /**
     * \c subserv received an information element with a bad format.
     * It was discarded with contumely.
     */
    PANIC_SUBSERV_BAD_IE = 0x106b,
    /**
     * \c subserv received IEs in an inappropriate order: an
     * option IE was before the last parameter IE.
     */
    PANIC_SUBSERV_MISPLACED_IE = 0x106c,
    /**
     * \c subserv received a parameter IE of a type it doesn't handle.
     */
    PANIC_SUBSERV_BAD_IE_TYPE = 0x106d,
    /**
     * \c subserv received a parameter IE with an index that was
     * too large.  This probably means the original start request
     * didn't pass enough parameters.
     */
    PANIC_SUBSERV_BAD_IE_INDEX = 0x106e,
    /**
     * \c subserv received a parameter IE that it didn't expect
     * at that point, for example it was already fully negotiated.
     */
    PANIC_SUBSERV_BAD_IE_STATE = 0x106f,

    /**
     * A CCP message passed to \c subserv had a bad format
     * that should have been picked up by the handler at
     * a higher level.
     */
    PANIC_SUBSERV_BAD_MESSAGE = 0x1070,


    /**
     * An inappropriate link type was used when establishing a
     * \c submsg link.
     */
    PANIC_SUBMSG_BAD_LINK_TYPE = 0x1071,
    /**
     * Resources were not available to set up a submsg link.
     * This is only seen if \c SUBMSG_PANIC_ON_FAILURE is defined,
     * else functions gracefully return \c FALSE.
     */
    PANIC_SUBMSG_LINK_SETUP_FAILED = 0x1072,
    /**
     * A submsg operation was attempted using an ISP port that
     * was not in an appropriate state.
     */
    PANIC_SUBMSG_INVALID_PORT = 0x1073,
    /**
     * An attempt was made to add an ISP port to submsg that already
     * exists.  This indicates the previous use was not properly
     * cleared.
     */
    PANIC_SUBMSG_PORT_EXISTS = 0x1074,
    /**
     * Resources were not available to send a frame.
     * This is only used if SUBMSG_PANIC_ON_FAILURE is defined,
     * otherwise the function simply returns FALSE.
     */
    PANIC_SUBMSG_TX_NO_RESOURCES = 0x1075,
    /**
     * The direction for a link was not TX or RX.
     */
    PANIC_SUBMSG_INVALID_DIRECTION = 0x1076,
    /**
     * The link type for a \c submsg link was invalid.
     */
    PANIC_SUBMSG_INVALID_LINK_TYPE = 0x1077,

    /**
     * Fileserv could not obtain an MMU handle
     */
    PANIC_FILESERV_NO_MMU_HANDLE = 0x1078,

    /**
     * Not enough memory was available for the memory abstraction
     * module \c memabs when \c MEMABS_NODDY was in use.
     * This may indicate the memory region has not been initialised.
     */
    PANIC_MEMABS_NODDY_EXHAUSTION = 0x1079,
    /**
     * An attempt was made to set or use an invalid memory allocation
     * strategy in the memabs module.  The argument is the
     * strategy requested.
     */
    PANIC_MEMABS_BAD_STRATEGY = 0x107a,
    /**
     * An attempt was made to allocate or free memory using
     * the memabs private pool strategy with an invalid ID.
     * The argument is the ID.
     */
    PANIC_MEMABS_BAD_ID = 0x107b,
    /**
     * An attempt was made to allocate memory using the memabs private
     * pool strategy where the size of pool elements was greater
     * than the requested size. The argument is the private pool MEMABS_ID.
     */
    PANIC_MEMABS_BAD_POOL = 0x107c,
    /**
     * An attempt was made to free a pointer using the memabs private
     * pool strategy that did not correspond to an element of the pool.
     * The argument is the id of the pool.
     */
    PANIC_MEMABS_BAD_FREE = 0x107d,
    /**
     * An attempt was made to allocate memory using the memabs private
     * pool strategy but the private pool was exhausted.  The argument
     * gives the ID of the private pool.
     */
    PANIC_MEMABS_PRIVATE_POOL_EXHAUSTION = 0x107e,
    /**
     * An attempt was made to allocate memory using the memabs runtime
     * strategy, but the underlying allocator ran out of memory.
     * The argument gives the ID passed, though this may not be
     * relevant to the runtime allocation strategy currently in use.
     */
    PANIC_MEMABS_RUNTIME_EXHAUSTION = 0x107f,

    /**
     * \c subevt on the Curator received a panic report from another
     * subsystem, but owing to truncation and the presence of headers
     * no panic ID could be read.  This is likely to be a rare
     * to non-existent condition as it implies the full two-word ISP
     * header was used.  Note that this code is only used by the
     * Curator, but in a place where it appears to come from a
     * subsystem, hence is part of the Hydra panic ID set.
     */
    PANIC_SUBEVT_PANIC_HYDRA_MYSTERY = 0x1080,

    /**
     * A patch with an invalid ID was called.
     * The argument is the invalid ID.
     */
    PANIC_HYDRA_SW_PATCH_INVALID_ID = 0x1081,
    /**
     * A patch function was not found in the table.
     * The argument is the ID for the patch.
     */
    PANIC_HYDRA_SW_PATCH_MISSING_ENTRY = 0x1082,

    /*
     * \isp_router has been asked to create a link but no context storage
     * has been supplied in the call.
     */
    PANIC_ISP_NO_CONTEXT_SUPPLIED = 0x1083,

    /*
     * subsleep was passed an inappropriate message by the main
     * CCP router.  The argument is the message ID, if it could
     * be deduced, else 0xFFFF.
     */
    PANIC_SUBSLEEP_BAD_MESSAGE = 0x1084,
    /*
     * subsleep failed to send a message.
     * This is used for crucial messages sent in a state where
     * the system is not expected to be too busy to send a message.
     * The argument is the signal ID.
     */
    PANIC_SUBSLEEP_MESSAGE_FAILED = 0x1085,
    /*
     * subsleep received a wake response from the Curator when
     * it wasn't expecting it.  This indicates something fatally
     * bad has gone wrong with the sequencing of the deep sleep
     * procedure.  There is no argument.
     */
    PANIC_SUBSLEEP_BAD_SEQUENCE = 0x1086,
    /**
     * The SSSM private memory pool can not satisfy a request for memory.
     * The argument is the number of chars unsuccessfully requested.
     */
    PANIC_SSSM_MEMPOOL_EXHAUSTION = 0x1087,
    /**
     * A general panic to be sent by conman
     * when an attempt to patch has failed in a way it shouldn't
     * This includes things such as most failures from ftp,
     * other than no file found or no session created
     */
    PANIC_CONMAN_PATCH_FAILED = 0x1088,
    /**
     * Failed to queue an SSSM event. (e.g. heap exhausted)
     * The argument is the event_type.
     */
    PANIC_SSSM_EVENTQ_FAILED = 0x1089,

    /**
     * The interrupt priority blocking hardware has raised an error
     */
    PANIC_HYDRA_INT_PRIORITY_HW_ERROR = 0x108a,

    /**
     * The remote memory access hardware has raised an error
     */
    PANIC_HYDRA_REMOTE_ACCESS_HW_ERROR = 0x108b,

    /**
     * A problem has occurred when writing a Transfer Descriptor
     * in the transfer/event descriptor code.
     */
    PANIC_HYDRA_TD_BUFFER_ERROR = 0x108c,
    /**
     * A problem has occurred when reading an Event Descriptor
     * in the transfer/event descriptor code
     */
    PANIC_HYDRA_ED_BUFFER_ERROR = 0x108d,

    /**
     * Either the stack check hardware has detected corruption,
     * or (unlikely) we have hit a breakpoint without the
     * debugger attached, or we have tried to dereference a 
     * NULL pointer.
     */
    PANIC_HYDRA_STACK_CHECK_HW_ERROR = 0x108e,

    /**
     * subsleep was told deep sleep was ready when the
     * logical state didn't allow this.  This means there
     * was a bad sequence of calls to subsleep functions.
     */
    PANIC_SUBSLEEP_NOT_READY = 0x108f,

    /**
     * On initialisation, submsg discovered that the subsystem
     * type for which the subsystem was compiled disagreed with
     * the hardware bus address for that subsystem.  The argument
     * is the firmware's notion of the bus address for which it
     * was compiled.  As submsg cannot run, the argument needs to
     * be extracted from the firmware with a debugger.
     */
    PANIC_SUBMSG_SUBSYSTEM_MISMATCH = 0x1090,

    /**
     * Host requested a deliberate panic by sending ccp "ALIVE"
     * to this subsystem. Used to test panic recovery.
     */
    PANIC_TEST = 0x1091,

    /**
     * Subserv received a confirmation for a service start on the
     * consumer and tried to complete automatic message channel
     * set up, but this failed.  This left the service in an
     * inconsistent state.
     */
    PANIC_SUBSERV_MESSAGE_CHANNEL_FAILED = 0x1092,

    /**
     * subres received a message from the Curator or a release request
     * from the local subsystem referring to a resource that was not
     * recorded in its local database.  The argument is the resource
     * identifier (type CCP_RESOURCE_ID).
     */
    PANIC_SUBRES_RESOURCE_MISSING = 0x1093,

    /**
     * subres recevied a request from the local subsystem to release
     * a resource that was in its local database, but not in
     * an appropriate state, for example the resource had not
     * yet been granted by \c globres.  The argument is
     * the resource identifier (type CCP_RESOURCE_ID).
     */
    PANIC_SUBRES_BAD_RESOURCE_STATE = 0x1094,

    /**
     * subres received a query response that could not be matched
     * against a query.  The argument is the resource ID in the
     * received response.
     */
    PANIC_SUBRES_BAD_QUERY = 0x1095,

    /**
     * subres failed to send an intersubsystem message to the Curator
     * at a point where graceful failure was not possible.  The
     * argument is the resource ID in the failed message.
     */
    PANIC_SUBRES_MESSAGE_FAILURE = 0x1096,

    /* The following PL_TIMER and OXYGOS panics arrived in the Hydra common
     * code from the Audio firmware.  They are now logically common because
     * they are also used by the Apps fw.  The same is true of the OXYGOS
     * panics later in the list. */

    /**
     * A timer fired but the handler function was NULL
     */
    PANIC_PL_TIMER_NO_HANDLER = 0x1097,

    /**
     * Timer Panics: Attempt to update a timer that was not created to be
     * periodic
     */
    PANIC_PL_TIMER_NOT_REPEATING_EVENT = 0x1098,

    /**
     * Timer Panics: Attempt to do a busy wait for invalid time
     */
    PANIC_PL_TIMER_INVALID_DELAY = 0x1099,

    /**
     * The scheduler recieved a NULL pointer to a message (probably out of
     * memory)
     */
    PANIC_OXYGOS_SCHED_MSG_IS_NULL = 0x109A,

    /**
     * An invalid task ID has been found in the scheduler
     */
    PANIC_OXYGOS_INVALID_TASK_ID = 0x109B,

    /**
     * A Null handler has been found in the scheduler
     */
    PANIC_OXYGOS_NULL_HANDLER = 0x109C,

    /**
     * An invalid scheduler queue number
     */
    PANIC_OXYGOS_INVALID_QUEUE_NUMBER = 0x109D,

    /**
     * The bit position of the background task being raised is invalid
     */
    PANIC_OXYGOS_INVALID_BACKGROUND_INTERRUPT_BIT_POSITION = 0x109E,

    /**
     * The number of scheduled messages does not match the number of messages
     * found in the scheduler
     */
    PANIC_OXYGOS_INVALID_MESSAGE_COUNT = 0x109F,

    /**
     * The randcache module failed to request random numbers
     * via subres, probably indicating low memory resources
     * on the local subsystem.
     */
    PANIC_RANDCACHE_SUBRES_FAILED = 0x10a0,
    /**
     * The randcache module received a callback from subres
     * with an operation it didn't support, given by the argument.
     */
    PANIC_RANDCACHE_BAD_SUBRES_OP = 0x10a1,
    /**
     * The randcache module received a resource response with
     * a result code indicating the resource was unavailable
     * in a fatal way, e.g. the Curator did not support it.
     * The argument is the result code.
     */
    PANIC_RANDCACHE_BAD_RESOURCE_RESULT = 0x10a2,
    /**
     * Not enough random numbers were available from the randcache module.
     * This error is only seen if the RANDCACHE_PANIC_ON_EMPTY
     * definition is in effect.
     */
    PANIC_RANDCACHE_CACHE_EXHAUSTED = 0x10a3,

    /**
     * The timed event module miscounted timed events: no timed
     * events were present but the count was non-zero.
     */
    PANIC_HYDRA_TIMED_EVENT_MISCOUNT = 0x10a4,

    /**
     * The number of pending messages in \c sched was wrong.
     */
    PANIC_HYDRA_BAD_MESSAGE_COUNT = 0x10a5,

    /**
     * The submsg MMU abstraction was used in a mode where the
     * called function was not available.
     */
    PANIC_SUBMSG_MMU_FUNCTION_UNAVAILABLE = 0x10a6,

    /**
     * \c sub_bit_config was passed a PDU it didn't recognise.
     */
    PANIC_SUB_BIT_CONFIG_BAD_PDU = 0x10a7,
  
    /**
     * The USB hardware has reported an unrecoverable error. The
     * argument is (event_group << 8 | event_index) (see CS-205548-DD, 
     * ch. 12).
     */
    PANIC_HOSTIO_USB_HW_ERROR = 0x10a8,

    /**
     * The USB event handling layer has failed.  The argument is
     * an internal indicating the status buffer that was being
     * processed.
     */
    PANIC_HOSTIO_USB_EVENT_HANDLER_ERROR = 0x10a9,

    /**
     * An endpoint's transfer descriptor buffer has overflowed.
     * This means that packets are being queued up faster than the
     * hardware can send them.  The argument is the firmware
     * endpoint index.
     */
    PANIC_HOSTIO_USB_TD_BUF_OVERFLOW = 0x10aa,

    /**
     * When the host reads a packet from the chip in multiple transfers,
     * all but the last must be a multiple of the sdio block size 
     * (usually 512 bytes). If multiple transfers are used that don't
     * comply (e.g. when the host buffer size is set smaller than the 
     * largest to-host PDU) than this panic will be generated. 
     * The recommendation is to set the to-host length in the 
     * ptap_queue_pair_attach_req command to 4096.
     */
    PANIC_HOSTIO_SDIO_UNEXPECTED_TO_HOST_TRANSFER = 0x10ab,

    /**
     * Dynamic task API has been passed invalid parameters
     */
    PANIC_OXYGOS_DYNAMIC_TASK_INVALID_PARAMS = 0x10ac,

    /**
     * Too many dynamic tasks in the system
     */
    PANIC_OXYGOS_TOO_MANY_DYNAMIC_TASKS = 0x10ad,

    /**
     * The message count limit was exceeded
     */
    PANIC_OXYGOS_TOO_MANY_MESSAGES = 0x10ae,

    /**
     * Asked to do something unsupported
     */
    PANIC_GENERAL_ERROR = 0x10af,

    /**
     * No handle for this interrupt
     */
    PANIC_UNHANDLED_INTERRUPT = 0x10b0,

    /**
     * Unexpected software interrupt
     */
    PANIC_SW_EXCEPTION = 0x10b1,

    /**
     * Run out of MMU handles
     */
    PANIC_HYDRA_MMU_HANDLES_EXHAUSTED = 0x10b2,

    /** The shared memory manager (smalloc) has run out of memory. */
    PANIC_SHARED_MEMORY_EXHAUSTION = 0x10b3,

    /** An invalid pointer has been passed to the shared memory manager
      * (smalloc), probably to sfree(). */
    PANIC_INVALID_SHARED_MEMORY_POINTER = 0x10b4,

    /** The sfree function has been called with a pointer that does not point
     * inside the smalloc pool.
     * Argument is the pointer that was passed in.
     */
    PANIC_SMALLOC_FREE_INVALID_PTR = 0x10b5,
    /** The sfree function has been called with a pointer whose underlying block
     * does not contain a valid block size, and therefore, it is considered
     * invalid. */
    PANIC_SMALLOC_FREE_INVALID_BLOCK = 0x10b6,

    /** The sfree function has been called with a block whose pool has no blocks
     * allocated. */
    PANIC_SMALLOC_FREE_NOT_ALLOCATED = 0x10b7,

    /** A service on which another service is marked as depending has crashed.
     * We have no option at present but to crash the subsystem providing the
     * dependent service.  The diatribe is the stag of the dependency service.
     */
    PANIC_SUBSERV_DEPENDENCY_CRASHED = 0x10b8,

    /**
     * The host has not read the to-host protocol buffer within a reasonable
     * time or an attempt was made to send a packet that was too large for
     * the buffer. The timeout is set in the config value
     * ISP_ROUTER_HOSTIO_TIMEOUT_US that can be found in config.common
     * (at the time of writing).
     * The diatribe is the size of ISP packet that was being attempted.
     */
    PANIC_ISP_HOST_NOT_REPONDING_OR_PACKET_TOO_BIG = 0x10b9,

    /**
     * The hostio module failed to request Curator USB hub
     * via subres, either due to the request send failed or because
     * Curator USB hub resource was not available.
     */
    PANIC_HOSTIO_USB_HUB_SUBRES_FAILED = 0x10ba,

    /**
     * Sending a TXBus interrupt through the TXBus interrupt
     * hardware has timed out. The diatribe is the time, in usec
     * that was spent waiting for the pending send transaction
     * to complete.
     */
    PANIC_BUS_INTERRUPT_HW_SEND_TIMED_OUT = 0x10bb,

    /**
     * A pointer which is already in the list of free blocks has been passed
     * to pfree(). */
    PANIC_HYDRA_FREE_ALREADY_DEALLOCATED_MEMORY = 0x10bc,

    /**
     * Sending a TXBus transaction through the bus_message
     * hardware has timed out. The diatribe is the value from
     * the BUS_MESSAGE_STATUS register after the timeout.
     */
    PANIC_BUS_MESSAGE_HW_SEND_TIMED_OUT = 0x10bd,

    /** A call to subres from the subres grouping layer failed before even
     * attempting to communicate with the Curator */
    PANIC_SUBRES_GROUP_LOCAL_FAILURE = 0x10be,

    /** An attempt to configure a PIO via subres produced a failure response
     * from the Curator.  The response is in the diatribe, and is of type
     * \c CCP_UPDATE_PIO_CONFIG_STATUS
     */
    PANIC_SUBRES_PIO_CONFIG_BAD_RESPONSE_STATUS = 0x10bf,



    /**
     * NOTE: 0x17ff is the highest panic code in the CSR range
     * CSR add codes from the lowest free number towards this high one
     */
    /**
     * NOTE: 0x1fff is the highest panic code in the SCSC range
     * SCSC add codes from the this down to the lowest
     */
    /**
     * A request for PMU information was made with an invalid
     * callback function.
     *
     * Argument 0: callback function was invalid.
     *
     * Argument 1: callback function was inconsistent.  Only one
     * callback function may be in use at once.
     */
    PANIC_SUBPMU_INVALID_CALLBACK = 0x1fff,
    /**
     * A request for PMU information failed because the CCP
     * command could not be sent.
     */
    PANIC_SUBPMU_CCP_FAILURE = 0x1ffe,
    /**
     * A division by zero occurred in the \c fixed16 module.
     */
    PANIC_FIXED16_DIVIDE_BY_ZERO = 0x1ffd,
    /**
     * Message data was sent via a subserv message channel that was
     * not ready for use.  It is also possible that no message channel
     * for transmission is present at all.
     */
    PANIC_SUBSERV_MC_NOT_READY = 0x1ffc,
    /**
     * NOTE: 0x1800 is the lowest panic code in the SCSC range
     */

    /*
     * Some inconsistency was detected in the stream subsystem.
     */
    PANIC_STREAM_NO_SUCH_SINK = 0x402c,
    PANIC_STREAM_MOVE_INTERNAL_FAILURE = 0x402d,
    PANIC_STREAM_CONFIGURE_WHEN_LIVE = 0x402e,
    PANIC_STREAM_TRANSFORM_IS_HELPER = 0x4030,
    PANIC_STREAM_UDP_METADATA_INCONSISTENT = 0x4031,

    /*
     * FILE EDITORS READ THIS
     *
     * Place additions above this comment.
     *
     * Additions to this file visible in public areas must be
     * made on the Hydra main branch.  If you have submitted
     * this file to the Curator main branch without submitting
     * to the Hydra main branch you have committed an offence.
     *
     * Updates *from* Hydra should where possible be en bloc rather
     * than piecemeal, i.e. please pull in the complete set of updates,
     * not just the ones you need.
     */

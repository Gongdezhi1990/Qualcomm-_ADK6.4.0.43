/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/** \file
 *
 *  This is the main public project header for the \c ipc library.
 *
 */
/****************************************************************************
Include Files
*/

#ifndef IPC_H
#define IPC_H

#include "types.h"
#include "buffer/buffer_ipc.h"
#include "stream/stream.h"
#ifdef INSTALL_IPC_INTERFACE_TEST
#include "test/if_test/ipc_if_test.h"
#endif

#include "ipc_kip.h"

/****************************************************************************
Public Type Declarations
*/

typedef enum IPC_FRW_TYPE
{
    IPC_FRW_TYPE_MONOLITHIC = 11,
    IPC_FRW_TYPE_KYMERA     = 12,

} IPC_FRW_TYPE;

typedef void ipc_rx_ret;

typedef struct
{
    uint16         signal_size;
    void *         signal_ptr;

} ipc_signal;

typedef enum IPC_EVENT
{
    IPC_REGISTERED                       = 0,
    IPC_SETUP_READY                      = 1,
    IPC_SETUP_COMPLETE                   = 2,
    IPC_SIG_CONFIGURED                   = 3,
    IPC_SIG_CONFIGURE_IND                = 4,
    IPC_DATA_CHANNEL_ACTIVATED           = 5,
    IPC_DATA_CHANNEL_DEACTIVATED         = 6,
    IPC_TEARDOWN_COMPLETED               = 7,
    IPC_TEARDOWN_IND                     = 8,
    IPC_MSGHANDLER_FAILED                = 9,
    IPC_WATCHDOG_RECV                    = 10,
    IPC_WATCHDOG_COMPLETED               = 11,
    IPC_RESET_REQUEST                    = 12,
    IPC_RESET_COMPLETED                  = 13,
    IPC_ERROR_IND                        = 14,

} IPC_EVENT;

typedef void (*IPC_MESSAGE_NOTIFICATION_HANDLER)(uint16 channel_id, ipc_msg *msg);

/*
 * error_type in reported IPC faults in debuglog
 */
typedef enum 
{
   IPC_ERROR_LEVEL_IPC_HDRREAD          = 0x101,
   IPC_ERROR_LEVEL_IPC_EXTREAD          = 0x102,
   IPC_ERROR_LEVEL_IPC_INTREAD          = 0x103,
   IPC_ERROR_LEVEL_IPC_INT              = 0x104,
   IPC_ERROR_LEVEL_IPC_EXTWRITE         = 0x201,
   IPC_ERROR_LEVEL_IPC_INTWRITE         = 0x202,
   IPC_ERROR_LEVEL_IFCALLBACK           = 0x301,
   IPC_ERROR_LEVEL_UNKNOWN              = 0x901
} IPC_ERROR_LEVEL_TYPE;

/*
 * Optional parameters for IPC_INTERFACE_CALLBACK
 */
typedef struct
{
    IPC_STATUS             ipc_status;

} ipc_event_params_default;

typedef struct
{
    IPC_STATUS             ipc_status;
    uint16                 signal_channel_id;
    uint16                 signal_id;

} ipc_event_params_signal;

typedef struct
{
    IPC_STATUS             ipc_status;
    uint16                 signal_channel_id;
    uint16                 signal_id;
    IPC_PROCESSOR_ID_NUM   remote_proc_id;

} ipc_event_params_signal_ind;

typedef struct
{
    IPC_STATUS             ipc_status;
    IPC_STATUS             remote_status;
    uint16                 data_channel_id;

} ipc_event_params_deactivated;

typedef struct
{
    IPC_STATUS             ipc_status;
    uint16                 data_channel_id;
    IPC_PROCESSOR_ID_NUM   remote_proc_id;
    uint16                 params_size;
    void                   *params;

} ipc_event_params_activated;

typedef struct
{
    IPC_STATUS             ipc_status;
    uint16                 msg_channel_id;

} ipc_event_params_msgchan;

typedef struct
{
    IPC_STATUS             ipc_status;
} ipc_event_params_watchdog;

typedef struct
{
    uint16                 error_type;
    uint16                 error_level;
    uint16                 error_code;
    uint16                 remote_proc_id;

} ipc_event_params_error_ind;


/*
 * In case of a need to add additional optional event parameters and types,
 * note that every event enum IPC_EVENT has its own optional parameters type.
 */
typedef ipc_event_params_default            ipc_event_registered_params_t;                             /* IPC_REGISTERED */
typedef ipc_event_params_default            ipc_event_setup_ready_params_t;                            /* IPC_SETUP_READY */
typedef ipc_event_params_default            ipc_event_setup_complete_params_t;                         /* IPC_SETUP_COMPLETE */
typedef ipc_event_params_signal             ipc_event_sig_configured_params_t;                         /* IPC_SIG_CONFIGURED */
typedef ipc_event_params_signal_ind         ipc_event_sig_configure_ind_params_t;                      /* IPC_SIG_CONFIGURE_IND */
typedef ipc_event_params_activated          ipc_event_data_channel_activated_params_t;                 /* IPC_DATA_CHANNEL_ACTIVATED */
typedef ipc_event_params_deactivated        ipc_event_data_channel_deactivated_params_t;               /* IPC_DATA_CHANNEL_DEACTIVATED */
typedef ipc_event_params_default            ipc_event_msghandler_failed_params_t;                      /* IPC_MSGHANDLER_FAILED */
typedef ipc_event_params_msgchan            ipc_event_teardown_completed_params_t;                     /* IPC_TEARDOWN_COMPLETED */
typedef ipc_event_params_msgchan            ipc_event_teardown_ind_params_t;                           /* IPC_TEARDOWN_IND */
typedef ipc_event_params_watchdog           ipc_event_watchdog_ind_params_t;                           /* IPC_WATCHDOG_IND*/
typedef ipc_event_params_watchdog           ipc_event_watchdog_completed_params_t;                     /* IPC_WATCHDOG_COMPLETE*/
typedef ipc_event_params_msgchan            ipc_event_reset_request_params_t;                          /* IPC_RESET_REQUEST/COMPLETED */
typedef ipc_event_params_error_ind          ipc_event_params_error_ind_t;                              /* IPC_ERROR_IND */

/*
 * The 'optional_params' parameter will point to per-function specific function
 * with fields relevant to the function.
 */
typedef IPC_STATUS (*IPC_INTERFACE_CALLBACK)(IPC_EVENT ipc_event, void *optional_params);

/*
 * General signal handler if no dedicated handler was installed for a signal.
 */
typedef IPC_STATUS (*IPC_SIGNAL_HANDLER)(IPC_PROCESSOR_ID_NUM remote_proc_id, uint16 signal_id, ipc_signal *sigind);

/*
 * Dedicated signal handler installed by function 'setup_static_signal'.
 */
typedef IPC_STATUS (*IPC_DEDICATED_SIGNAL_HANDLER)(IPC_PROCESSOR_ID_NUM remote_proc_id, uint16 signal_id, ipc_signal *sigind);

typedef enum IPC_MSG_TYPE
{
    IPC_MSG_TYPE_INTERNAL = 0,
    IPC_MSG_TYPE_EXTERNAL = 1

} IPC_MSG_TYPE;

typedef enum IPC_MSG_PAYLOAD_SRC
{
    IPC_MSG_PAYLOAD_UNKNOWN = 0,
    IPC_MSG_PAYLOAD_KIP     = 1

} IPC_MSG_PAYLOAD_SRC;

typedef enum IPC_SIGCHANTYPE
{
    IPC_SIGCHANTYPE_DYNAMIC   = 0,
    IPC_SIGCHANTYPE_DEDICATED = 1

} IPC_SIGCHANTYPE;

/*
 * Active or sleep enumeration for 'ipc_[s|g]et_shallow_sleep_status'
 * It shares values with 'IPC_SETUP_STATE' in 'ipc_private.h'. Make
 * sure that entries are unique between both 'IPC_SETUP_STATE' and
 * 'IPC_ACTIVE_STATE'
 */
typedef enum _IPC_ACTIVE_STATE
{
    IPC_CPU_STOPPED                     = 0x0000,
    IPC_CPU_SLEEPING                    = 0x0001,
    IPC_CPU_ACTIVE                      = 0x0002,

} IPC_ACTIVE_STATE;


/****************************************************************************
Public Constant and macros
*/

/*
 * Msg id request/response masking
 */
#define IPC_MSGID_RESPMASK        0x8000

#define ipc_set_req(msgid)                                           \
                      ((msgid)&(~(IPC_MSGID_RESPMASK)))

#define ipc_set_resp(msgid)                                          \
                      ((msgid)|(IPC_MSGID_RESPMASK))

#define ipc_is_msgid_resp(msgid)                                     \
                      (((msgid)&(IPC_MSGID_RESPMASK))?TRUE:FALSE)

/****************************************************************************
Public Variable Declarations
*/

/****************************************************************************
Public Function Declarations
*/

/**
 * \brief Initialise IPC data structures
 *
 * \param[in] local_proc_id     - Local processor ID. Allowed numbers are 0 - 3.
 * \param[in] framework_type    - Framework type (monolithic = 11, kymera = 12)
 * \param[in] framework_version - Framework version (top byte major version, bottom byte minor version, e.g. 0x0100 for v1.0)
 * \param[in] callback          - Caller provided callback function
 *
 * \return  0 if successfully initialised IPC data structures else error code
 */
extern IPC_STATUS ipc_register_comms(IPC_PROCESSOR_ID_NUM local_proc_id, uint16 framework_type, uint16 framework_version, IPC_INTERFACE_CALLBACK callback);

/**
 * \brief Setup IPC communications
 *
 * \param[in]  remote_proc_id - Remote processor ID. Allowed numbers are 0 - 3.
 * \param[in]  msg_callback   - Caller provided callback function to process message events
 * \param[in]  num_signals    - Number of signal channels/events to support
 * \param[in]  sig_callback   - Caller provided callback function to process signal events
 * \param[out] channel_id     - The channel id of the messaging channel that was setup
 *
 * \return  0 if successfully setup IPC communications else error code
 */
extern IPC_STATUS ipc_setup_comms(IPC_PROCESSOR_ID_NUM remote_proc_id, IPC_MESSAGE_NOTIFICATION_HANDLER msg_callback, uint16 num_signals, IPC_SIGNAL_HANDLER sig_callback, uint16 *channel_id);

/**
 * \brief Teardown IPC communications
 *
 * Undoes all the allocations and configurations done by 'ipc_setup_comms' and 'ipc_register_comms'.
 * It may only be called after all other processors have been through teardown themselves and are
 * all powered off. Can only be called on P0.
 *
 * \return  0 if successful teardown of IPC communications else error code
 */
extern IPC_STATUS ipc_unregister_comms(void);

/**
 * \brief Start a processor through keyhole
 *
 * \param[in] remote_proc_id - Remote processor ID. Allowed numbers are 1 - 3. Primary processor is 0.
 * \param[in] target         - Boot up location ( ROM = 0, SQIF = 1)
 *
 * \return  0 if successfully started the processor else error code
 */
extern IPC_STATUS ipc_start_processor(IPC_PROCESSOR_ID_NUM remote_proc_id, IPC_NVMEM_WIN_CONFIG_TYPE target);

/*
 * \brief Forcefully stop a processor through keyhole
 *
 * \param[in] remote_proc_id - Remote processor ID. Allowed numbers are 1 - 3. Primary processor is 0.
 */
extern IPC_STATUS ipc_stop_processor(IPC_PROCESSOR_ID_NUM remote_proc_id);

/*
 * \brief Request a processor to shutdown. This will tear off ipc link as well.
 *
 * \param[in] remote_proc_id - Remote processir ID. Allowed numbers are 1 - 3. Primary processor is 0.
 */
extern IPC_STATUS ipc_poweroff_processor(IPC_PROCESSOR_ID_NUM remote_proc_id);

/**
 * \brief Seeks a particular sequence number has been transmitted
 *
 * \param[in] channel id - Message channel ID (CS-336170-DD Figure 14 in 10.1.3)
 * \param[in] seqno      - The IPC message sequence ID
 *
 * \return  0 if successfully seeked message else error code
 */
extern IPC_STATUS ipc_seek_send_messages(uint16 channel_id, uint16 seqno);

/**
 * \brief Enable watchdog pings
 *        Note that this function assumes the specified remote processor has
 *        already started. The caller must ensure that this is the case.
 *
 * \param[in] remote_proc_id   - Remote processor ID. Allowed numbers are 1 - 3.
 * \param[in] watchdog_timeout - The watchdog timeout in ms
 * \param[in] callback         - Call upon timeout
 *
 * \return  0 if successfully enabled watchdog pings else error code
 */
extern IPC_STATUS ipc_watchdog_ping(IPC_PROCESSOR_ID_NUM remote_proc_id);

/**
 * \brief Sets up a signalling channel dedicated for the signal ID.
 *
 * \param[in]  signal id      - Caller supplied signal id (see CS-336170-DD section 10.1.4.2)
 * \param[in]  priority       - Signal priority
 * \param[in]  callback       - Signal handler (callback)
 * \param[out] signal_channel - Allocated signal channel (if successful)
 *                              Signal channel numbers are 0 - 'num_signals-1' (usually num_signals is 32, total of dedicated + static signals)
 *
 * \return  0 if successfully sent external message else error code
 */
extern IPC_STATUS ipc_setup_static_signal(uint16 signal_id, uint16 priority, IPC_DEDICATED_SIGNAL_HANDLER callback, uint16 *signal_channel);

/**
 * \brief Resets a signalling channel to use for dynamic signals.
 *
 * \param[in] signal_channel - Signal channel numbers are 0 - 'num_signals-1' (usually num_signals is 32, total of dedicated + static signals)
 *
 * \return  0 if successfully reset signal channel else error code
 */
extern IPC_STATUS ipc_reset_signal_channel(uint16 signal_channel_id);

/**
 * \brief Raise a signal to a partner processor.
 *
 * \param[in] remote_proc_id - Remote processor ID (0 - 3).
 * \param[in] signal id      - Caller supplied signal id (see CS-336170-DD section 10.1.4.2)
 * \param[in] sigreq         - Caller supplied signal value (ipc_signal type)
 *                             The signal value is expected to exist in shared memory.
 *                             No copying will occur, just the pointer and size are forwarded
 *                             to the remote processor.
 *
 * \return  0 if successfully raised signal else error code
 */
extern IPC_STATUS ipc_raise_signal(IPC_PROCESSOR_ID_NUM remote_proc_id, uint16 signal_id, ipc_signal *sigreq);

/**
 * \brief Creates a data channel. Create the port of it, the request is for the first channel.
 *        Maximum of 16 channels (8 in either direction).
 *
 * \param[in]  port_id            - Port ID.
 * \param[in]  channel_number     - Data channel ID (0..127).
 * \param[in]  IPC_DATA_DIRECTION - Direction: channel read or write.
 * \param[in]  cbuffer            - If provided use that cbuffer. If NULL, function must allocate cbuffer.
 * \param[out] data_channel_id    - Channel id of the data channel created
 *
 * \return  0 if successfully created data channel else error code
 */
extern IPC_STATUS ipc_create_data_channel(uint16 port_id, uint16 channel_number, IPC_DATA_DIRECTION direction, uint16 *data_channel_id);

/**
 * \brief Destroy a channel. For the last channel, destroy the port too.
 *
 * \param[in] data_channel_id - Data channel ID (0..127).
 *
 * \return  0 if successfully destroyed data channel else error code
 */
extern IPC_STATUS ipc_destroy_data_channel(uint16 data_channel_id);

/**
 * \brief  Activate a new data channel for use with partner processor 'remote_proc_id'.
 *         Callers of this function can specify an opaque data block to be transferred
 *         to the secondary processor
 *
 * \param  data_channel_id [IN] - data channel ID (from ipc_create_data_channel)
 * \param  remote_proc_id  [IN] - secondary processor to communicate activation to
 * \param  cbuffer         [IN] - pointer to caller supplied circular buffer
 * \param  create_cbuffer   [IN] - if true, IPC allocates a new cbuffer in the shared
 *                                memory and syncs it with the passed cbuffer.
 *                                Otherwise the passes buffer is used directly in
 *                                the channel.
 * \param  param_size      [IN] - size of parameter block in bytes.
 * \param  params          [IN] - pointer to caller supplied parameter block.
 *
 * \return Return IPC_SUCCESS if channel activated successfully, error if not.
 *         The interface callback will be called upon receiving reply from
 *         the target secondary processor handling the activation event.
 */
extern IPC_STATUS ipc_activate_data_channel(uint16 data_channel_id, uint16 remote_proc_id, tCbuffer *cbuffer, bool create_cbuffer, uint16 param_size, void *params);

/**
 * \brief Deactivate the data channel. The data port as well while destroying the last channel
 * of a port.
 *
 * \param[in] data_channel_id - Data channel ID (0..127).
 *
 * \return  0 if successfully deactivated data channel else error code
 */
extern IPC_STATUS ipc_deactivate_data_channel(uint16 data_channel_id);

/**
 * \brief Return the cbuffer associated with a data channel.
 *
 * \param[in] data_channel_id - Data channel ID (0..127).
 *
 * \return  NULL if data_channel_id not found or if stored cbuffer of data channel is NULL.
 *          Non-NULL if data_channel_id valid and stored cbuffer of data channel is non-NULL.
 */
extern tCbuffer *ipc_data_channel_get_cbuffer(uint16 data_channel_id);

/**

 * \brief  Synchronise a data channel with a cbuffer.
 *         If the data channel and cbuffer share the same data buffer, then
 *         update the read and write pointers, so that the structs are now
 *         synchronised. If the data buffers are different, copy all data
 *         from the cbuffer to the data channel.
 *
 *         Purpose is to sync two cbuffers using the same data buffer.
 *         Some operators will have buffers created for all channels even though
 *         it has connected only one channel, such as mixer,
 *         splitter, resampler. Real endpoints (e.g. A2DP source endpoint) will also have
 *         its buffers. This buffer must be used through the dual-core data channel, rather
 *         than creating a new cbuffer. But for cbuffers accessed by two cpus the cbuffer
 *         structure must be in shared memory, accessible by both cpus.
 *         In these cases, we keep a duplicate cbuffer structure: the original one in
 *         private memory, and a second one in shared memory (created in ipc_activate_
 *         data_channel), using the same data buffer. The two cbuffer structures must be
 *         synced from time to time; ipc_data_channel_write_sync syncs the shared cbuffer
 *         struct of the data channel to a private cbuffer; ipc_data_channel_read_sync
 *         syncs a private cbuffer to the shared cbuffer struct of the data channel.
 *
 * \param  src_cbuffer      [IN] - cbuffer to read from
 * \param  dst_data_channel [IN] - data channel ID (from ipc_create_data_channel)
 *                                 whose cbuffer to write to
 *
 * \return Return IPC_SUCCESS if successfully synchronised cbuffers, else error.
 */
extern IPC_STATUS ipc_data_channel_write_sync(tCbuffer *src_cbuffer, uint16 dst_data_channel);

/**
 * \brief  Synchronise a cbuffer with a data channel.
 *         If the cbuffer and data channel share the same data buffer, then
 *         update the read and write pointers, so that the structs are now
 *         synchronised. If the data buffers are different, copy all data
 *         from the data channel to the cbuffer.
 *
 *         See also comments under ipc_data_channel_write_sync above.
 *
 * \param  src_data_channel [IN] - data channel ID (from ipc_create_data_channel)
 *                                 whose cbuffer to read from
 * \param  dst_cbuffer      [IN] - cbuffer to write to
 *
 * \return Return IPC_SUCCESS if successfully synchronised cbuffers, else error.
 */
extern IPC_STATUS ipc_data_channel_read_sync(uint16 src_data_channel, tCbuffer *dst_cbuffer);

/**
 * \brief  get the shared static address from the lookup table
 *
 * \param[in] key               - The key defined in IPC_LUT_EXT_ID
 * \param[out] addr             - The address associated with key in the (key, address) pair in the IPC LUT
 *
 * \return  IPC_SUCCESS if lookup address was found, any other return value indicates an error.
 */
extern IPC_STATUS ipc_get_lookup_addr(IPC_LUT_EXT_ID key, uintptr_t *addr);

/**
 * \brief  set the shared static address into the lookup table
 *
 * \param[in] key               - The key defined in IPC_LUT_EXT_ID
 * \param[in] addr              - The address associated with key, to set in the (key, address) pair in the IPC LUT
 *
 * \return  IPC_SUCCESS if the address was successfully entered in the IPC LUT for the specified key,
 *          any other return value indicates an error.
 */
extern IPC_STATUS ipc_set_lookup_addr(IPC_LUT_EXT_ID key, uintptr_t addr);

/**
 * \brief  Find active/sleep state of secondary processors
 *
 * \return  TRUE when any secondary processor (e.g. not P0) is running (stopped nor shallow sleeping).
 */
extern bool ipc_aux_proc_active(void);

/**
 * \brief  Store the shallow sleep status of the secondary processor
 *
 * \param[in] proc_id           - Remote processor ID (1 - 3).
 * \param[in] status            - The status to record
 * \param[in] wakeup_time       - The wakeup_time to record
 *
 * \return  Set the shared memory location for the processor that this processor is either entering or exiting
 *          shallow sleep, otherwise set it to the earliest wakeup time it would like to wakeup.
 */
extern IPC_STATUS ipc_set_shallow_sleep_status(IPC_PROCESSOR_ID_NUM proc_id, IPC_ACTIVE_STATE state, TIME wakeup_time);

/**
 * \brief  Returns the shallow sleep status of the secondary processor
 *
 * \param[in] proc_id           - Remote processor ID (1 - 3).
 *
 * \return  Get the value in the shared memory location for the processor that this processor is either entering
 *           or exiting shallow sleep.
 */
extern IPC_ACTIVE_STATE ipc_get_shallow_sleep_status(IPC_PROCESSOR_ID_NUM proc_id);

/**
 * \brief  Returns whether the secondary processor has started
 *         That is, it verifies whether the processor is in a state
 *         ready to communicate with other processors. If not, it is
 *         either off, or still establishing communications, or in
 *         teardown.
 *
 * \param[in] proc_id           - Remote processor ID (1 - 3).
 *
 * \return  TRUE if secondary processor has started, FALSE if not.
 */
extern bool ipc_aux_proc_has_started(uint16 proc_id);

/**
 * \brief  Returns whether the debugger support should be used.
 *         The ADK debugger sets a bit in a register to indicate
 *         whether the firmware should enable debugger support.
 *
 * \return  TRUE if debugger mode is required, FALSE if not.
 */
extern bool ipc_support_debugger(IPC_PROCESSOR_ID_NUM remote_proc_id);

#ifndef GEN_BG_INT
extern ipc_rx_ret ipc_rx_bg(void);
#endif /* GEN_BG_INT */

#endif /* IPC_H */

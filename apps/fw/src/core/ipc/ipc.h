/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#ifndef IPC_H_
#define IPC_H_

/* Do not #include anything before this check.
 * Redefining BUFFER before including this file can cause inconsistent
 * structure definitions. */
#ifdef BUFFER
#error "ipc.h must not be included after a redefinition of BUFFER"
#endif

#include "hydra/hydra.h"
#include "ipc/ipc_prim.h"
#include "ipc/ipc_msg_types.h"
#include "timed_event/rtime.h"
#include "sched/sched.h"

struct BUFFER;
struct BUFFER_MSG_ss;

#define TO_STREAM_ID(src_or_snk) ((uint16)(((uint32)(src_or_snk)) & 0xffff))
#define SINK_TO_ID(snk) TO_STREAM_ID(snk)
#define SOURCE_TO_ID(src) TO_STREAM_ID(src)
#define TRANSFORM_TO_ID(tfm) TO_STREAM_ID(tfm)
#define SINK_FROM_ID(id) ((Sink)(uint32)(id))
#define SOURCE_FROM_ID(id)  ((Source)(uint32)(id))
#define TRANSFORM_FROM_ID(id) ((Transform)(uint32)(id))


/**
 * On P0, sets up two MMU handles: one (P0->P1) at a conventional reserved index
 * (MMU_INDEX_RESERVED_IPC) which is known to P1 and one (P1->P0) at an
 * arbitrary index. It writes the size of the P0->P1 buffer and the handle and
 * size of the P1->P0 buffer to the P0->P1 buffer.
 * *This function must be called on P0 before P1 is started.*
 *
 * On P1, sets up a single-page buffer at the reserved handle index and then
 * reads the size and the other buffer's handle and size from it, subsequently
 * correcting the buffer size and creating a buffer structure for the P1->P0
 * buffer.
 *
 * Once P1 has completed execution of this function, inter-process communication
 * is available.  Before this point P0 is able to write message to P1, but it
 * will not see them until ipc_init() has completed.
 *
 * \ingroup ipc_init
 */
extern void ipc_init(void);
/**
 * Non-blocking send: copies the supplied message into the IPC send buffer and
 * raises the IPC interrupt on the other processor.
 *
 * @param msg_id ID of message to send
 * @param msg Message body (note: the ID field is automatically set)
 * @param len_bytes Length of message in bytes (if \p payload is NULL, must be
 * 0!). Must be a multiple of 4.
 *
 * \ingroup ipc_send
 */

extern void ipc_send(IPC_SIGNAL_ID msg_id, const void *msg,
                     uint16 len_bytes);

/**
 * Non-blocking out-of-band send: creates an \c IPC_TUNNELLED_PRIM_OUTBAND
 * pointing at the supplied payload and submits it via \c ipc_send().
 *
 * \note Unlike \c ipc_send, the supplied pointer and length refer to the
 * payload only
 *
 * @param msg_id ID of message to send.
 * @param payload Arbitrary pointer to data.  The data are not touched by ipc;
 * only the pointer value itself is delivered.
 * @param payload_len_bytes Length of message in bytes
 *
 * \ingroup ipc_send
 */
extern void ipc_send_outband(IPC_SIGNAL_ID msg_id, void *payload,
                                                    uint32 payload_len_bytes);

/**
 * Send an IPC_BOOL_RSP with the given signal ID and the given boolean value
 * @param msg_id Signal ID (most likely a RSP)
 * @param val The boolean value to send
 */
extern void ipc_send_bool(IPC_SIGNAL_ID msg_id, bool val);

/**
 * Send an IPC_UINT16_RSP with the given signal ID and the given uint16 value
 * @param msg_id Signal ID (most likely a RSP)
 * @param val The uint16 value to send
 */
extern void ipc_send_uint16(IPC_SIGNAL_ID msg_id, uint16 val);

/**
 * Send an IPC_INT16_RSP with the given signal ID and the given int16 value
 * @param msg_id Signal ID (most likely a RSP)
 * @param val The int16 value to send
 */
extern void ipc_send_int16(IPC_SIGNAL_ID msg_id, int16 val);

/**
 * Helper function to make it convenient to signal frees of pointers over IPC
 * @param free_id Whether to do a pfree or an sfree
 * @param ptr The (remote) pointer to be free'd
 *
 * \ingroup ipc_send
 */
extern void ipc_send_free(IPC_SIGNAL_ID free_id, void *ptr);

/**
 * Helper macro to request a pfree
 * @param ptr The (remote) pointer to be pfree'd
 *
 * \ingroup ipc_send
 */
#define ipc_send_pfree(ptr) ipc_send_free(IPC_SIGNAL_ID_PFREE, ptr)

/**
 * Send a signal that doesn't require any additional data.
 * @param sig_id  The Signal ID.
 */
extern void ipc_send_signal(IPC_SIGNAL_ID sig_id);

/**
 * Helper function to make it convenient to request smallocs from P1
 * @param size_bytes Number of bytes to allocate
 * @param panic_on_failure What to do if NULL comes back
 * @return The allocated pointer
 *
 * \note Unlike other send functions, this function also does a (blocking)
 * receive
 *
 * \ingroup ipc_send
 */
extern void *ipc_send_smalloc(uint32 size_bytes, bool panic_on_failure);

/**
 * P1's equivalent of snew
 * @param type Type to allocate one of
 *
 * \ingroup ipc_send
 */
#define ipc_snew(type) \
    (type *)ipc_send_smalloc(sizeof(type), TRUE)
/**
 * P1's equivalent of xsnew
 * @param type Type to allocate one of
 *
 * \ingroup ipc_send
 */
#define ipc_xsnew(type) \
    (type *)ipc_send_smalloc(sizeof(type), FALSE)

/**
 * Helper macro to request a pfree
 * @param ptr The (remote) pointer to be sfree'd
 *
 * \ingroup ipc_send
 */
#define ipc_send_sfree(ptr) \
    ipc_send_free(IPC_SIGNAL_ID_SFREE, ptr)

/**
 * Main function for sending PIO related requests to P0.
 * @param req Pointer to request message.
 * @param signal Signal identifier indicating which operation is wanted.
 * @note req may be overwritten with useful data.
 */
extern void ipc_send_pio(IPC_PIO *req, IPC_SIGNAL_ID signal);

/**
 * Helper macro for acquiring several PIOs.
 * @param req Pointer to request message indicating which PIOs to acquire.
 */
#define ipc_send_pio_acquire(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_ACQUIRE)

/**
 * Helper macro for releasing several PIOs.
 * @param req Pointer to request message indicating which PIOs to release.
 */
#define ipc_send_pio_release(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_RELEASE)

/**
 * Helper macro for setting the local Apps owner for several PIOs.
 * @param req Pointer to request message indicating which PIOs to change.
 */
#define ipc_send_pio_set_owner(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_SET_OWNER)

/**
 * Helper macro for getting the local Apps owner of several PIOs.
 * @param req Pointer to request message indicating which PIOs to get.
 * @note req->value will contain a mask indicating the ownership
 * (0 - P0, 1 - P1).
 */  
#define ipc_send_pio_get_owner(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_GET_OWNER)

/**
 * Helper macro for enabling/disabling pull resistors for several PIOs.
 * @param req Pointer to request message indicating which PIOs to change.
 */ 
#define ipc_send_pio_set_pull_en(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_SET_PULL_EN)

/**
 * Helper macro for getting the pull resistors status of several PIOs.
 * @param req Pointer to request message indicating which PIOs to get.
 * @note req->value will contain a mask indicating the pull resistors status
 * (0 - disabled, 1 - enabled).
 */ 
#define ipc_send_pio_get_pull_en(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_GET_PULL_EN)

/**
 * Helper macro for changing pull resistors direction for several PIOs.
 * @param req Pointer to request message indicating which PIOs to change.
 */ 
#define ipc_send_pio_set_pull_dir(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_SET_PULL_DIR)

/**
 * Helper macro for getting the pull resistors status of several PIOs.
 * @param req Pointer to request message indicating which PIOs to get.
 * @note req->value will contain a mask indicating the pull resistors direction
 * (0 - low, 1 - high).
 */  
#define ipc_send_pio_get_pull_dir(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_GET_PULL_DIR)

/**
 * Helper macro for changing pull resistors strength for several PIOs.
 * @param req Pointer to request message indicating which PIOs to change.
 */     
#define ipc_send_pio_set_pull_str(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_SET_PULL_STR)

/**
 * Helper macro for getting the pull resistors strength of several PIOs.
 * @param req Pointer to request message indicating which PIOs to get.
 * @note req->value will contain a mask indicating the pull resistors strength
 * (0 - weak, 1 - strong).
 */ 
#define ipc_send_pio_get_pull_str(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_GET_PULL_STR)

/**
 * Helper macro for getting the unused PIOs.
 * @param req Pointer to request message indicating which PIOs to get.
 * @note req->value will contain a mask indicating the pull resistors strength
 * (0 - in use, 1 - available).
 */   
#define ipc_send_pio_get_unused(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_GET_UNUSED)

/**
 * Helper macro to mux PIOs to other subsystems.
 * @param req Pointer to request message indicating the PIOs and the
 * subsystem to mux them to.
 * @note req->value needs to contain the mux setting, not a mask.
 */
#define ipc_send_pio_set_pio_mux(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_SET_PIO_MUX)

/**
 * Helper macro for getting the PIO mux.
 * @param req Pointer to request message indicating the PIO to get.
 * @note req->value will contain the mux setting.
 */
#define ipc_send_pio_get_pio_mux(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_GET_PIO_MUX)

/**
 * Helper macro to change the PAD mux. This changes the user between several
 * hardware peripherals and subsystem control.
 * @param req Pointer to request message indicating which PIO PAD muxes to
 * change and the ID of the owner to assign them to (see \c IPC_PIO_PAD_MUX_ID).
 * @note req->value needs to contain the mux setting, not a mask.
 */
#define ipc_send_pio_set_pad_mux(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_SET_PAD_MUX)

/**
 * Helper macro to get the PAD mux. This indicates the user between several
 * hardware peripherals and subsystem control.
 * @param req Pointer to request message indicating which PIO PAD mux to get
 * @note req->value will contain the mux setting, not a mask.
 */
#define ipc_send_pio_get_pad_mux(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_GET_PAD_MUX)

/**
 * Helper macro to set the drive strength for a PIO. This selects a drive
 * strength ID from \c pin_drive_strength_id.
 * @param req Pointer to request message indicating which PIO to change and the
 * ID of the drive strength.
 * @note req->value needs to contain the drive strength ID, not a mask.
 */
#define ipc_send_pio_set_drive_strength(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_SET_DRIVE_STRENGTH)

/**
 * Helper macro to get the drive strength of a PIO. This returns a drive
 * strength ID from \c pin_drive_strength_id.
 * @param req Pointer to request message indicating which PIO drive strength to
 * get.
 * @note req->value will contain the drive strength ID, not a mask.
 */
#define ipc_send_pio_get_drive_strength(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_GET_DRIVE_STRENGTH)

/**
 * Helper macro for enabling/disabling sticky pull resistors for several PIOs.
 * @param req Pointer to request message indicating which PIOs to change.
 */     
#define ipc_send_pio_set_sticky(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_SET_STICKY)

/**
 * Helper macro for getting the sticky pull resistors of several PIOs.
 * @param req Pointer to request message indicating which PIOs to get.
 * @note req->value will contain a mask indicating the pull resistors sticky
 * setting (0 - disabled, 1 - enabled).
 */ 
#define ipc_send_pio_get_sticky(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_GET_STICKY)

/**
 * Helper macro for enabling/disabling slew for several PIOs.
 * @param req Pointer to request message indicating which PIOs to change.
 */     
#define ipc_send_pio_set_slew(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_SET_SLEW)

/**
 * Helper macro for getting the slew setting of several PIOs.
 * @param req Pointer to request message indicating which PIOs to get.
 * @note req->value will contain a mask indicating the slew setting
 * (0 - disabled, 1 - enabled).
 */ 
#define ipc_send_pio_get_slew(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_GET_SLEW)

/**
 * Helper macro for setting the XIO mode for several PIOs.
 * @param req Pointer to request message indicating which PIOs to change.
 */     
#define ipc_send_pio_set_xio_mode(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_SET_XIO_MODE)

/**
 * Helper macro for getting the XIO mode of several PIOs.
 * @param req Pointer to request message indicating which PIOs to get.
 * @note req->value will contain a mask indicating the XIO mode.
 */ 
#define ipc_send_pio_get_xio_mode(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_GET_XIO_MODE)

/**
 * Helper macro for setting the drive enable for several PIOs.
 * @param req Pointer to request message indicating which PIOs to change.
 */     
#define ipc_send_pio_set_drive_enable(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_SET_DRIVE_ENABLE)

/**
 * Helper macro for getting the drive enable of several PIOs.
 * @param req Pointer to request message indicating which PIOs to get.
 * @note req->value will contain a mask indicating the drive enable.
 */ 
#define ipc_send_pio_get_drive_enable(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_GET_DRIVE_ENABLE)

/**
 * Helper macro for setting the drive for several PIOs.
 * @param req Pointer to request message indicating which PIOs to change.
 */     
#define ipc_send_pio_set_drive(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_SET_DRIVE)

/**
 * Helper macro for getting the drive of several PIOs.
 * @param req Pointer to request message indicating which PIOs to get.
 * @note req->value will contain a mask indicating the drive.
 */ 
#define ipc_send_pio_get_drive(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_GET_DRIVE)

/**
 * Helper macro for setting the bitserial PIO muxes in the host interface block.
 * @param req Pointer to request message indicating which PIOs to set.
 * @note req->value will contain the bitserial instance and signal.
 */ 
#define ipc_send_pio_set_func_bitserial(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_SET_FUNC_BITSERIAL)

/**
 * Helper macro for setting the uart PIO muxes in the host interface block.
 * @param req Pointer to request message indicating which PIOs to set.
 * @note req->value will contain the bitserial instance and signal.
 */ 
#define ipc_send_pio_set_func_uart(req) \
    ipc_send_pio(req, IPC_SIGNAL_ID_PIO_SET_FUNC_UART)

/**
 * Send a fault report to P0.
 * @param id Fault ID.
 * @param n Number of occurrences.
 * @param t Timestamp.
 * @param arg Fault argument.
 */
extern void ipc_send_fault(uint16 id, uint16 n, TIME t, uint32 arg);

/**
 * Send a \ref IPC_SIGNAL_ID_P1_DEEP_SLEEP_MSG to P0
 * @param p1_sleep TRUE: P1 requires deep sleep, FALSE: P1 doesn't require deep sleep
 * @param earliest_wake_up_time Earliest time for wake up (in microseconds)
 * @param latest_wake_up_time Latest time for wake up (in microseconds)
  * \ingroup ipc_send
 */
extern void ipc_send_p1_deep_sleep_msg(bool p1_sleep, uint32 earliest_wake_up_time, uint32 latest_wake_up_time);

/**
 * Handler for memory access fault messages from P0 to P1.
 * @param id Signal ID
 * @param msg Message body
 */

extern void ipc_memory_access_fault_handler(IPC_SIGNAL_ID id, const void *msg);


/**
 * Helper function to make it convenient to signal tail frees on \c BUFFERs
 * @param buf Pointer to the \c BUFFER to free data from.  Must be in shared
 * memory!
 * @param new_tail New index of oldest non-freed data
 *
 * \ingroup ipc_send
 */
void ipc_send_buf_raw_update_tail_free(BUFFER *buf, uint16 new_tail);

/**
 * Helper function to make it convenient to signal tail frees on \c BUFFERs
 * @param msg_buf Pointer to the \c BUFFER_MSG to free data from.  Must be in
 * shared memory!
 *
 * \ingroup ipc_send
 */
void ipc_send_buf_update_behind_free(BUFFER_MSG *msg_buf);

/**
 * Helper function to make it convenient to send Bluestack primitives
 * @param protocol Protocol tag, as \#defined in bluetooth.h
 * @param prim Pointer to the primitive to be sent
 *
 */
extern void ipc_send_bluestack(uint16 protocol, void *prim);

/**
 * Helper function to make it convenient to send scheduler messages to remote
 * queues
 * @param remote_qid QID of remote queue
 * @param mi Message integer
 * @param mv  Message blob
 */
extern void ipc_send_sched(qid remote_qid, uint16 mi, void *mv);


/**
 * Blocking receive: shallow sleeps until either a message is seen with the
 * supplied ID.  Returns a copy of the received payload in a local pmalloc'd
 * block.  Ownership is assumed by the caller.
 *
 * Note: the receive handler underlying this call will process everything else
 * it finds in the receive buffer and then return control to this function.
 * Hence non-blocking message handlers should avoid making blocking IPC
 * calls themselves, to avoid inadvertently blocking out any current blocking
 * call for a long time.
 *
 * @param msg_id IPC message to receive
 * @param blocking_msg Pointer to pre-allocated space for the expected message,
 * or NULL if the message should be pmalloc'd internally
 * @return Pointer to the message body: simply equal to \p blocking_msg if this
 * is not NULL, else points to an internally pmalloc'd copy of the message
 *
 * \ingroup ipc_recv
 */
extern void *ipc_recv(IPC_SIGNAL_ID msg_id, void *blocking_msg);

/**
 * Variant of ipc_recv for when the receiving of the blocking response message
 * should not be pre-empted by high-prioity handlers
 *
 * @param msg_id IPC message to receive
 * @param blocking_msg Pointer to pre-allocated space for the expected message,
 * or NULL if the message should be pmalloc'd internally
 * @return Pointer to the message body: simply equal to \p blocking_msg if this
 * is not NULL, else points to an internally pmalloc'd copy of the message
 *
 * \ingroup ipc_recv
 */
extern void *ipc_recv_atomic(IPC_SIGNAL_ID msg_id, void *blocking_msg);
/**
 * Signature of receive callbacks.  These are passed:
 * @param msg_id ID of the received message
 * @param msg The received message
 *
 * @note Dynamic callbacks should not post blocking receives in case they are
 * invoked while another blocking receive is in progress, causing a
 * "double-block".  More generally, IPC callbacks should not take too long in
 * execution: ideally they would just be leaf functions.
 *
 * @note The pointer to the message becomes invalid as soon as the callback
 * returns
 *
 * \ingroup ipc_recv
 */
typedef void (*IPC_RECV_CB)(IPC_SIGNAL_ID msg_id, const void *msg);

/**
 * Non-blocking receive to request the framework to pass a single response of the
 * given ID to the given callback.  To simulate a permanent handler, the callback
 * should call this function again during execution to re-post itself.
 * @param recv_id The message ID that will trigger the callback
 * @param cb The callback itself
 *
 * \ingroup ipc_recv
 */
extern void ipc_recv_cb(IPC_SIGNAL_ID recv_id, IPC_RECV_CB cb);

#ifdef CHIP_DEF_P1_SQIF_SHALLOW_SLEEP_WA_B_195036
/**
 * Get the difference between the location of p0 code in flash and p1 code in
 * flash. Used for translating const pointers from p1 to p0.
 * @return The offset between p0 and p1 code memory in the flash device
 */
extern uint32 ipc_get_p1_flash_offset(void);
#endif /* CHIP_DEF_P1_SQIF_SHALLOW_SLEEP_WA_B_195036 */

/**
 * Send a panic report to P0.
 * @param id Panic ID.
 * @param t Timestamp.
 * @param arg Panic argument.
 */
extern void ipc_send_panic(uint16 id, TIME t, uint32 arg);

/**
 * Is the IPC currently blocking waiting for a "fast" response message?
 * @return TRUE if so, else FALSE
 */
extern bool ipc_disallow_high_priority_handler_calls(void);

/**
 * Notify IPC that a high-priority message handler is about to run/has just
 * finished running synchronously within the IPC handler.  This must be used
 * to bracket handler functions that call "fast" traps.
 * \param start_not_stop TRUE indicates the handler is about to start, FALSE
 * that it is about to stop.
 */
extern void ipc_high_priority_handler_running(bool start_not_stop);

/**
 * Indicate that IPC should not treat calls as recursive, at least at the current
 * level
 */
extern void ipc_stop_recursive(void);

/**
 * Handler for all PIO related IPC signals
 * @param id Signal ID
 * @param msg Message body, including header
 * 
 * \ingroup ipc_recv_impl
 */  
extern void ipc_pio_msg_handler(IPC_SIGNAL_ID id, const IPC_PIO *msg);

/**
 * @brief Initialise the IPC receive buffer mapping policy.
 *
 * The IPC receive buffer has two modes for handling its pages.
 * It either frees them once it's done with them, or it leaves
 * them around to be overwritten later. This function initialises
 * the mode based on the IpcLeaveRecvBufferPagesMapped MIB key.
 *
 * This function must not be called until the MIB module has been
 * initialised with keys from the config files.
 */
extern void ipc_recv_buffer_mapping_policy_init(void);


#endif /* IPC_H_ */
/*@}*/

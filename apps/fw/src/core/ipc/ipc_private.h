/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#ifndef IPC_PRIVATE_H_
#define IPC_PRIVATE_H_

#define IO_DEFS_MODULE_KALIMBA_INTERPROC_INT
#include "ipc/ipc.h"
#include "hydra/hydra.h"
#include "hydra/hydra_types.h"
#include "buffer/buffer.h"
#include "panic/panic.h"
#include "hydra_log/hydra_log.h"
#include "assert.h"
#include "sched/sched.h"
#include "int/int.h"
#include "pmalloc/pmalloc.h"
#include "hal/hal_macros.h"
#include "dorm/dorm.h"
#include "utils/utils_bits_and_bobs.h"
#include "fault/fault.h"
#include "memory_map.h"         /* For redirection of const pointers */
#include "trap_api/trap_api.h"
#include "trap_version/trap_version.h"

/**
 * The size of the IPC send and receive buffers.
 * The implementation is tolerant of overflowing the IPC buffers: 
 * it puts messages it can’t insert into a linked list until it can insert them. 
 * Hence we trade off buffer size against pmalloc memory. 
 * Not only buffer-size is important, since a buffer can be logically full 
 * (contains 16 messages) before it is physically full. 
 * Enhancing the BUFFER_MSG class to fall back on pmalloc for additional logical 
 * slots, increases implementation complexity, and we have chosen not to.
 *
 * \ingroup ipc_impl
 */
 
#define IPC_BUFFER_SIZE MMU_BUFFER_SIZE_512

/**
 * The maximum number of messages to process in one invocation of the receive
 * handler
 *
 * \ingroup ipc_recv_impl
 */
#define IPC_MAX_RECV_MSGS 10

#define hal_set_reg_interproc_event_1 hal_set_reg_p1_to_p0_interproc_event_1
#define hal_set_reg_interproc_event_2 hal_set_reg_p1_to_p0_interproc_event_2

/**
 * Type for implementing the queue of receive callbacks
 *
 * \ingroup ipc_recv_impl
 */
typedef struct IPC_RECV_CB_QUEUE
{
    struct IPC_RECV_CB_QUEUE *next; /**< Linked list impl. */
    IPC_SIGNAL_ID            recv_id; /**< ID triggering this callback */
    IPC_RECV_CB              cb; /**< Callback to trigger */
    uint32                   nest_level; /**< Level at which the response should
                                              be returned */
} IPC_RECV_CB_QUEUE;

/**
 * Type for implementing the queue of pending messages, i.e. those that have
 * been posted for sending but haven't made it into the buffer yet due to a lack
 * of space
 *
 * \ingroup ipc_send_impl
 */
typedef struct IPC_MSG_QUEUE
{
    struct IPC_MSG_QUEUE *next; /**< Linked list impl. */
    IPC_SIGNAL_ID         msg_id; /**< Message ID */
    void                 *msg;    /**< Message body */
    uint16                length_bytes; /**< Message length */
} IPC_MSG_QUEUE;

/**
 * Top-level storage for IPC internal data
 *
 * \ingroup ipc_impl
 */
typedef struct
{
    BUFFER_MSG *send; /**< The send message buffer */
    BUFFER_MSG *recv; /**< The receive message buffer */
    volatile bool pending; /**< Internal fg/bg comms */
    IPC_RECV_CB_QUEUE *recv_cb; /**< Linked list of current receive callbacks */
    IPC_MSG_QUEUE *send_queue; /**< Linked list of messages waiting for send
                                    buffer space */
#ifdef CHIP_DEF_P1_SQIF_SHALLOW_SLEEP_WA_B_195036
    /** Difference between location of p0 code in flash and p1 code in flash.
     * Used for translating const pointer from p1 to p0
     */
    uint32  p1_pm_flash_offset_from_p0;
#endif /* CHIP_DEF_P1_SQIF_SHALLOW_SLEEP_WA_B_195036 */
    bool atomic_rsp_pending; /**< An "atomic" blocking trap is waiting for its
                                 response message: further blocking traps
                                 may not be invoked */
    IPC_MSG_QUEUE *postponed_hp_triggers; /**< Messages that can't be consumed
                                  yet because the handler can't be run until the
                                  atomic trap is complete */
    IPC_MSG_QUEUE *out_of_order_rsps; /**< Messages that are blocking responses
                                  to outer calls to ipc_recv */
    uint8 nest_level;
} IPC_DATA;

/**
 * IPC implementation data instance
 *
 * \ingroup ipc_impl
 */
extern IPC_DATA ipc_data;

/**
 * Macro to encapsulate the code for sending a raw pointer in the IPC buffer,
 * rather than a proper IPC message.  Used at initialisation time.
 *
 * \ingroup ipc_send_impl
 */
#define IPC_SEND_POINTER(ptr) \
    do {\
        uint8 *to_px = buf_map_front_msg(ipc_data.send);\
        assert(sizeof(uint32) == sizeof(void *));\
        *((uint32 *)to_px) = (uint32)(ptr);\
        buf_add_to_front(ipc_data.send, sizeof(void *));\
    } while (0)

/**
 * Macro to encapsulate the code for synchronously receiving a raw pointer in
 * the IPC buffer, rather than via a proper IPC message.  Used at initialisation
 * time.
 *
 * \ingroup ipc_recv_impl
 */
#define IPC_RECV_POINTER(ptr, type) \
    do {\
        const uint8 *from_px = buf_map_back_msg(ipc_data.recv); \
        assert(buf_get_back_msg_len(ipc_data.recv) == sizeof(uint32)); \
        (ptr) = (type *)(*((const uint32 *)from_px)); \
        buf_update_back(ipc_data.recv); \
        buf_update_behind_free(ipc_data.recv); \
    } while (0)

/**
 * Macro to encapsulate the code for sending a raw uint32 in the IPC buffer,
 * rather than a proper IPC message.  Used at initialisation time.
 *
 * \ingroup ipc_send_impl
 */
#define IPC_SEND_VALUE(value) \
    do {\
        uint8 *to_px = buf_map_front_msg(ipc_data.send);\
        *((uint32 *)to_px) = (value);\
        buf_add_to_front(ipc_data.send, sizeof(uint32));\
    } while (0)

/**
 * Macro to encapsulate the code for synchronously receiving a raw uint32 in
 * the IPC buffer, rather than via a proper IPC message.  Used at initialisation
 * time.
 *
 * \ingroup ipc_recv_impl
 */
#define IPC_RECV_VALUE(value) \
    do {\
        const uint8 *from_px = buf_map_back_msg(ipc_data.recv); \
        assert(buf_get_back_msg_len(ipc_data.recv) == sizeof(uint32)); \
        (value) = *((const uint32 *)from_px); \
        buf_update_back(ipc_data.recv); \
        buf_update_behind_free(ipc_data.recv); \
    } while (0)

#endif /* IPC_PRIVATE_H_ */

/**
 * Post as many messages from the "back-up" queue as possible.
 *
 * \note This function must be called with interrupts blocked!
 *
 * @return TRUE if the queue is now empty, else FALSE
 *
 * \ingroup ipc_send_impl
 */
extern bool ipc_clear_queue(void);

/**
 * Helper function to place a message on a given IPC_MSG_QUEUE
 *
 * \param pqueue Pointer to the queue to place the message on
 * \param msg_id The IPC signal
 * \param msg The message body
 * \param len_bytes The message length in bytes
 */
extern void ipc_queue_msg_core(IPC_MSG_QUEUE **pqueue, IPC_SIGNAL_ID msg_id,
        const void *msg, uint16 len_bytes);


/**
 * Process everything in the receive buffer.  If the current blocking_msg_id is
 * encountered amongst them, save a copy of the payload in a local pmalloc block
 * and return it to the caller after everything has been processed.  Ownership
 * of the memory is passed to the caller.
 *
 * @param blocking_msg Pointer to pre-allocated space for any expected message,
 * or NULL if the message should be pmalloc'd internally.  If this function
 * isn't being called in the context of blocking, the value is irrelevant,
 * except that it will be returned unchanged, so it is highly recommended to
 * pass NULL, so that the case where an blocking message is erroneously received
 * can be detected (e.g. see \c ipc_background_handler())
 * @return Pointer to the blocking message body, if any: equal to
 * \p blocking_msg if this is not NULL, else points to an internally pmalloc'd
 * copy of the message if there was one, else NULL
 *
 * \ingroup icp_recv_impl
 */
extern void *ipc_recv_handler(void *blocking_msg);


/**
 * Handler for MMU and buffer related IPC signals
 * @param id Signal ID
 * @param msg Message body, including header
 *
 * \ingroup ipc_recv_impl
 */
extern void ipc_mmu_msg_handler(IPC_SIGNAL_ID id, const void *msg);

/**
 * Handler for pmalloc and smalloc related IPC signals
 * @param id Signal ID
 * @param msg Message body, including header
 *
 * \ingroup ipc_recv_impl
 */
extern void ipc_malloc_msg_handler(IPC_SIGNAL_ID id, const void *msg);

/**
 * Handler for fault/panic related IPC signals.
 * @param id Signal ID
 * @param msg Message body, including header
 */
extern void ipc_fault_panic_msg_handler(IPC_SIGNAL_ID id, const IPC_FAULT_PANIC *msg);


/**
 * Handler for test tunnel primitives
 * @param id Signal ID (= \c IPC_SIGNAL_ID_TEST_TUNNEL_PRIM)
 * @param msg Message body, including header
 * @param msg_length_bytes Total length of the message in bytes
 *
 * \ingroup ipc_recv_impl
 */
extern void ipc_test_tunnel_handler(IPC_SIGNAL_ID id, const void *msg,
                                                    uint16 msg_length_bytes);

/**
 * Handler for seep sleep related IPC signals
 * @param id Signal ID
 * @param msg Message body, including header
 *
 * \ingroup ipc_recv_impl
 */
void ipc_deep_sleep_msg_handler(IPC_SIGNAL_ID id, const void *msg);

/**
 * Handler for Bluestack primitives
 * @param id Signal ID (= \c IPC_SIGNAL_ID_BLUESTACK_PRIM)
 * @param msg Message body
 *
 * \ingroup ipc_recv_impl
 */
extern void ipc_bluestack_handler(IPC_SIGNAL_ID id, const void *msg);

/**
 * Handler for scheduler message primitives
 * @param id Signal ID (= \c IPC_SIGNAL_ID_SCHED_MSG_PRIM)
 * @param msg Message body
 *
 * \ingroup ipc_recv_impl
 */
extern void ipc_sched_handler(IPC_SIGNAL_ID id, const void *msg);

/**
 * Handler for app standard messages
 * @param id Signal ID
 * @param msg Message body
 * @param msg_len Length of @p msg in bytes
 * @return FALSE if message handler is high-priority but the IPC is currently
 * blocking on an atomic response; otherwise TRUE
 *
 * \ingroup ipc_recv_impl
 */
extern bool ipc_trap_api_handler(IPC_SIGNAL_ID id, const void *msg, uint16 msg_len);

/**
 * Handler for app 'fast' messages
 * @param id Signal ID
 * @param msg Message body
 * @return FALSE if message handler is high-priority but the IPC is currently
 * blocking on an atomic response; otherwise TRUE
 *
 * \ingroup ipc_recv_impl
 */
extern bool ipc_trap_api_fast_handler(IPC_SIGNAL_ID id, const void *msg);

/**
 * Handler for stream related messages from P0 to P1.
 * @param id Signal ID
 * @param msg Message body
 */
extern void ipc_stream_handler(IPC_SIGNAL_ID id, const void *msg);



/**
 * Initialise the IPC Appcmd-based tests.
 *
 * \ingroup ipc_test
 */
extern void ipc_test_init(void);

/**
 * Test handler function for hi_pri_stream_handler_test
 * \param t Task message posted to
 * \param id Message ID
 * \param m Message body
 */
extern void ipc_test_high_priority_handler(Task t, MessageId id, Message m);

/**
 * Panic interrupt handler.
 *
 * \ingroup ipc_impl
 */
extern void panic_interrupt_handler(void);

/**
 * Struct used to store panic data.
 *
 * \ingroup ipc_impl
 */
typedef struct
{
    panicid p0_deathbed_confession;
    DIATRIBE_TYPE p0_diatribe;
    TIME p0_t;
    panicid p1_deathbed_confession;
    DIATRIBE_TYPE p1_diatribe;
    TIME p1_t;
} PANIC_DATA;

/**
 * Panic data instance
 *
 * \ingroup ipc_impl
 */
extern PANIC_DATA *panic_data;

/**
 * Send an IPC message passing the trap API version information
 */
void ipc_send_trap_api_version_info(void);

/**
 * Handler for the trap API version information message
 */
void ipc_trap_api_version_prim_handler(IPC_SIGNAL_ID id, const void *prim);


/**
 * Leave IPC receive buffer pages mapped instead of freeing them.
 *
 * Once a page has finished being used the IPC receive buffer can either
 * free the page or leave it to be reused later on. Freeing the page saves
 * on RAM at the expense of making it more difficult to debug the IPC module.
 *
 * If the IpcLeaveRecvBufferPagesMapped MIB key is set this function should
 * be called to leave the IPC receive buffer pages mapped in, otherwise they
 * will be freed.
 */
void ipc_leave_recv_buffer_pages_mapped(void);

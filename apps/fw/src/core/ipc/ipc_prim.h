/* Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 *
 * Primitive declarations for IPC
 */
#ifndef IPC_PRIM_H__
#define IPC_PRIM_H__

/* Do not #include anything before this check.
 * Redefining BUFFER before including this file can cause inconsistent
 * structure definitions. */
#ifdef BUFFER
#error "ipc_prim.h must not be included after a redefinition of BUFFER"
#endif

#include "hydra/hydra_types.h"
#include "buffer/buffer.h"
#include "ipc/ipc_msg_types.h"
#include "app/bluestack/bluetooth.h" /* For TP_BDADDR_T */
#include "itime/itime.h"
#include "app/adc/adc_if.h"
#include "app/voltsense/voltsense_if.h"
#include <operator.h>

typedef enum
{
    IPC_SIGNAL_ID_NULL =                                                     0,
    IPC_SIGNAL_ID_MMU_HANDLE_ALLOC_REQ =                                     1,
    IPC_SIGNAL_ID_MMU_HANDLE_ALLOC_RSP =                                     2,
    IPC_SIGNAL_ID_MMU_HANDLE_FREE =                                          3,
    IPC_SIGNAL_ID_BUF_TAIL_FREE =                                            4,
    IPC_SIGNAL_ID_BUF_UPDATE_BEHIND_FREE =                                   5,
    IPC_SIGNAL_ID_PFREE =                                                    6,
    IPC_SIGNAL_ID_SMALLOC_REQ =                                              7,
    IPC_SIGNAL_ID_SMALLOC_RSP =                                              8,
    IPC_SIGNAL_ID_SFREE =                                                    9,
    IPC_SIGNAL_ID_BLUESTACK_PRIM =                                           10,
    IPC_SIGNAL_ID_BLUESTACK_PRIM_RECEIVED =                                  11,
    IPC_SIGNAL_ID_TEST_TUNNEL_PRIM =                                         12,
    IPC_SIGNAL_ID_SCHED_MSG_PRIM =                                           13,
    IPC_SIGNAL_ID_APP_MSG =                                                  14,
    IPC_SIGNAL_ID_APP_SINK_SOURCE_MSG =                                      15,
    IPC_SIGNAL_ID_PIO_SET_OWNER =                                            16,
    IPC_SIGNAL_ID_PIO_GET_OWNER =                                            17,
    IPC_SIGNAL_ID_PIO_SET_PULL_EN =                                          18,
    IPC_SIGNAL_ID_PIO_GET_PULL_EN =                                          19,
    IPC_SIGNAL_ID_PIO_SET_PULL_DIR =                                         20,
    IPC_SIGNAL_ID_PIO_GET_PULL_DIR =                                         21,
    IPC_SIGNAL_ID_PIO_SET_PULL_STR =                                         22,
    IPC_SIGNAL_ID_PIO_GET_PULL_STR =                                         23,
    IPC_SIGNAL_ID_PIO_GET_UNUSED =                                           24,
    IPC_SIGNAL_ID_PIO_RSP =                                                  25,
    IPC_SIGNAL_ID_P1_DEEP_SLEEP_MSG =                                        26, /**< Signal from P1 to P0 to inform about P1 deep sleep status */
    IPC_SIGNAL_ID_FAULT =                                                    27,
    IPC_SIGNAL_ID_SIGNAL_INTERPROC_EVENT =                                   28,
    IPC_SIGNAL_ID_PIO_SET_PIO_MUX =                                          36,
    IPC_SIGNAL_ID_STREAM_DESTROYED =                                         37,
    IPC_SIGNAL_ID_DEEP_SLEEP_WAKEUP_SOURCE =                                 40,
    IPC_SIGNAL_ID_OPERATORS_DESTROYED =                                      41,
    IPC_SIGNAL_ID_TRAP_API_VERSION =                                         42,
    IPC_SIGNAL_ID_STREAM_UART_SINK     =                                     43,
    IPC_SIGNAL_ID_STREAM_UART_SINK_RSP =                                     44,
    IPC_SIGNAL_ID_APP_MSG_TO_HANDLER =                                       45,
    IPC_SIGNAL_ID_PIO_SET_PAD_MUX =                                          46,
    IPC_SIGNAL_ID_TESTTRAP_BT_REQ =                                          47,
    IPC_SIGNAL_ID_TESTTRAP_BT_RSP =                                          48,
    IPC_SIGNAL_ID_IPC_LEAVE_RECV_BUFFER_PAGES_MAPPED =                       49,
    IPC_SIGNAL_ID_PIO_GET_PAD_MUX =                                          50,
    IPC_SIGNAL_ID_PIO_GET_PIO_MUX =                                          51,
    IPC_SIGNAL_ID_PIO_SET_DRIVE_STRENGTH =                                   52,
    IPC_SIGNAL_ID_PIO_GET_DRIVE_STRENGTH =                                   53,
    IPC_SIGNAL_ID_PIO_SET_STICKY =                                           54,
    IPC_SIGNAL_ID_PIO_GET_STICKY =                                           55,
    IPC_SIGNAL_ID_PIO_SET_SLEW =                                             56,
    IPC_SIGNAL_ID_PIO_GET_SLEW =                                             57,
    IPC_SIGNAL_ID_PIO_ACQUIRE =                                              58,
    IPC_SIGNAL_ID_PIO_RELEASE =                                              59,
    IPC_SIGNAL_ID_PIO_SET_XIO_MODE =                                         60,
    IPC_SIGNAL_ID_PIO_GET_XIO_MODE =                                         61,
    IPC_SIGNAL_ID_PIO_SET_DRIVE_ENABLE =                                     62,
    IPC_SIGNAL_ID_PIO_GET_DRIVE_ENABLE =                                     63,
    IPC_SIGNAL_ID_PIO_SET_DRIVE =                                            64,
    IPC_SIGNAL_ID_PIO_GET_DRIVE =                                            65,
    IPC_SIGNAL_ID_PIO_SET_FUNC_BITSERIAL =                                   66,
    IPC_SIGNAL_ID_PIO_SET_FUNC_UART =                                        67,
    IPC_SIGNAL_ID_SD_MMC_SLOT_INIT_REQ =                                     68,
    IPC_SIGNAL_ID_SD_MMC_SLOT_INIT_RSP =                                     69,
    IPC_SIGNAL_ID_SD_MMC_READ_DATA_REQ =                                     70,
    IPC_SIGNAL_ID_SD_MMC_READ_DATA_RSP =                                     71,
    IPC_SIGNAL_ID_MEMORY_ACCESS_FAULT_INFO =                                 72,
    IPC_SIGNAL_ID_APP_MESSAGE_RECEIVED =                                     73,
#include "ipc/gen/ipc_trap_api_signals.h"
} IPC_SIGNAL_ID;


/**
 * Substructure defining the header of IPC messages
 * \ingroup ipc_prim
 */
typedef struct
{
    IPC_SIGNAL_ID id;
} IPC_HEADER;

/**
 * An IPC message that doesn't need any data, just the signal ID.
 */
typedef struct
{
    IPC_HEADER header;
} IPC_SIGNAL;

/**
 * DIR TO_P0 Ask P0 to allocate an MMU handle of the specified size. The size
 * is indicated via the \c MMU_BUFFER_SIZE enumeration
 * \ingroup ipc_prim
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_MMU_HANDLE_ALLOC_REQ */
    mmu_buffer_size      size;
} IPC_MMU_HANDLE_ALLOC_REQ;

/**
 * DIR FROM_P0 Pass an allocated MMU handle for use by P1
 * \ingroup ipc_prim
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_MMU_HANDLE_ALLOC_RSP */
    mmu_handle      hdl;
} IPC_MMU_HANDLE_ALLOC_RSP;


/**
 * DIR TO_P0 Ask P0 to free an MMU handle that P1 is no longer using
 * \ingroup ipc_prim
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_MMU_HANDLE_FREE */
    mmu_handle      hdl;
} IPC_MMU_HANDLE_FREE;


/**
 * DIR TO_P0 Request P0 to perform a buf_tail_free call
 * \ingroup ipc_prim
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_BUF_TAIL_FREE */
    BUFFER    *buffer; /**< Pointer to the BUFFER to be updated.  This must be
                            in shared memory */
    uint16     new_tail; /**< New tail to set */
} IPC_BUF_TAIL_FREE;

/**
 * DIR TO_P0 Request P0 to perform a buf_update_behind_free call
 * \ingroup ipc_prim
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_BUF_UPDATE_BEHIND_FREE */
    BUFFER_MSG  *buffer_msg; /**< Pointer to the BUFFER_MSG to be updated */
} IPC_BUF_UPDATE_BEHIND_FREE;


/**
 * DIR BOTH Request the other processor to free the given pointer
 * \ingroup ipc_prim
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_PFREE  or \c IPC_SIGNAL_ID_SFREE*/
    void *pointer;
} IPC_FREE;

/**
 * DIR TO_P0 Request P0 to smalloc a pointer of the specified size
 * \ingroup ipc_prim
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_SMALLOC_REQ */
    uint32     size_bytes; /**< Number of bytes to be allocated */
} IPC_SMALLOC_REQ;

/**
 * DIR FROM_P0 Response from P0 with an smalloc'd pointer
 * \ingroup ipc_prim
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_SMALLOC_RSP */
    void      *pointer; /**< Allocated pointer, NULL if allocation failed */
} IPC_SMALLOC_RSP;


/**
 * DIR BOTH Carrier for a bluestack primitive.
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_BLUESTACK_PRIM or
                                 \c IPC_SIGNAL_ID_BLUESTACK_PRIM_RECEIVED */
    uint16 protocol;
    void *prim;
} IPC_BLUESTACK_PRIM;

/**
 * DIR FROM_P1 message free indication is sent to P0.
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_APP_MESSAGE_RECEIVED */
    uint16 id;         /**< Message id that has been received */
    void *msg;         /**< Message body */
} IPC_APP_MESSAGE_RECEIVED;

/**
 * DIR BOTH Carrier for a scheduler message bound for a remote queue
 */
typedef struct
{
    IPC_HEADER header;
    uint32 qid;
    uint16 mi;
    void *mv;
} IPC_SCHED_MSG_PRIM;

/**
 * DIR BOTH Arbitrary primitive passed explicitly to or from the customer
 * app. The primitive itself is opaque as far as this protocol is concerned.
 * The payload is copied into the data field
 * \ingroup ipc_prim
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_BLUESTACK_PRIM,
                               = \c IPC_SIGNAL_ID_TEST_TUNNEL_PRIM */
    uint32 length;
    uint32 data[1];   /**< Arbitrary length data: this field must be last in
                           the structure! */
} IPC_TUNNELLED_PRIM_INBAND;

/**
 * DIR BOTH Arbitrary primitive passed explicitly to or from the customer
 * app. The primitive itself is opaque as far as this protocol is concerned
 *
 * \note The payload is not const because it needs to be possible for the
 * receiver to pfree it (i.e. call ipc_send_pfree())
 * \ingroup ipc_prim
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_TEST_TUNNEL_PRIM */
    uint32 length;
    void *payload;
} IPC_TUNNELLED_PRIM_OUTBAND;

/**
 * DIR FROM_P0 Carrier for a message to be delivered to a registered App message
 * handler task
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_APP_MSG */
    IPC_MSG_TYPE hdlr; /**< ID of the handler channel to post the message to */
    uint16 id;         /**< Message ID */
    void *message;     /**< Message body */
    bool allow_duplicates;  /**< Should the handler task's queue be filtered
                                 for pre-existing messages like this one? */
} IPC_APP_MSG_PRIM;

/**
 * DIR FROM_P0 Carrier for a message to be delivered to an unregistered App
 * message handler task
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_APP_MSG_TO_HANDLER */
    void *handler;     /**< Handler to post the message to. This is normally a
                            P1 task of type Task */
    uint16 id;         /**< Message ID */
    bool allow_duplicates;  /**< Should the handler task's queue be filtered
                                 for pre-existing messages like this one? */
    uint8 message[1];  /**< Message body must be stored immediately after
                            this structure in memory by allocating
                            offsetof(IPC_APP_MSG_TO_HANDLER_PRIM, message) +
                            message_length */
} IPC_APP_MSG_TO_HANDLER_PRIM;

/**
 * DIR FROM_P0 Carrier for a message to be delivered to a registered App Sink
 * or Source handler task
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_APP_SINK_SOURCE_MSG */
    uint16 stream_id;  /**< ID of the sink or source to post the message to */
    uint16 id;         /**< Message ID */
    void *message;     /**< Message body */
    bool allow_duplicates;  /**< Should the handler task's queue be filtered
                                 for pre-existing messages like this one? */
} IPC_APP_SINK_SOURCE_MSG_PRIM;

/**
 * DIR BOTH Gets and sets PIO pull enable, pull direction, pull strength and
 * owner settings.
 */
typedef struct
{
    IPC_HEADER header;     /**< id = \c IPC_SIGNAL_ID_PIO_GET or
                                     \c IPC_SIGNAL_ID_PIO_SET */
    uint16 bank;           /**< Bank ID */
    uint32 mask;           /**< Bit mask indicating which PIOs to change or
                                request information about */
    uint32 value;          /**< Depending on the signal ID, this bit mask
                                indicates: if a pull resistor is enabled or
                                going to be enabled (0-disable, 1-enable), the
                                current or requested pull direction (0-low,
                                1-high), current or requested pull strength
                                (0-weak, 1-strong), if the sender is the PIO
                                owner or a request for/release of PIO
                                ownership, which subsystem will control a PIO,
                                which hardware peripheral will control a PIO
                                (see \c IPC_PIO_PAD_MUX_ID). */
} IPC_PIO;

/**
 * Type definition for the IDs accepted by the PAD mux. Please note that not
 * all of these are available for all PIOs. Also, for firmware controlled
 * PIOs \c IPC_PIO_PAD_MUX_CORE_PIO needs to be used.
 */
typedef enum
{
    IPC_PIO_PAD_MUX_CORE_PIO = 0,
    IPC_PIO_PAD_MUX_JANITOR_PIO,
    IPC_PIO_PAD_MUX_T_BRIDGE,
    IPC_PIO_PAD_MUX_DEBUG_SPI,
    IPC_PIO_PAD_MUX_SDIO_DEVICE,
    IPC_PIO_PAD_MUX_ULPI,
    IPC_PIO_PAD_MUX_AUDIO_PCM,
    IPC_PIO_PAD_MUX_AUDIO_PWM,
    IPC_PIO_PAD_MUX_MCLK,
    IPC_PIO_PAD_MUX_CLK32K,
    IPC_PIO_PAD_MUX_SDIO_HOST,
    IPC_PIO_PAD_MUX_SQIF,
    IPC_PIO_PAD_MUX_USB
} IPC_PIO_PAD_MUX_ID;

/**
 * Type definition for the IDs accepted by the PIO mux. For firmware controlled
 * PIOs \c IPX_PIO_MUX_APPS needs to be used.
 */
typedef enum
{
    IPC_PIO_MUX_CURATOR = 0,
    IPC_PIO_MUX_HOST,
    IPC_PIO_MUX_BT,
    IPC_PIO_MUX_AUDIO,
    IPC_PIO_MUX_APPS,
    IPC_PIO_MUX_PIO_CTRL,
    IPC_PIO_MUX_LED_CTRL
} IPC_PIO_MUX_ID;

/**
 * DIR FROM_P1 Fault or panic indication from P1.
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_FAULT */
    uint16 id;         /**< Fault/Panic ID */
    uint16 n;          /**< Number of reports of the fault ID. Not used for panics. */
    TIME t;            /**< Fault/Panic timestamp */
    uint32 arg;        /**< Fault/Panic argument */
} IPC_FAULT_PANIC;

/**
 * DIR FROM_P1 Deep sleep indication from P1, indicating if P1 can enter deep sleep and wake up time.
 * The indication is sent independently from the state of deep sleep activation with the customer trap command.
 */
typedef struct
{
    IPC_HEADER header;                  /**< id = \c IPC_SIGNAL_ID_P1_DEEP_SLEEP_MSG */
    bool       p1_sleep;                /**< TRUE: P1 requests sleeping; FALSE: P1 doesn't request sleeping */
    uint32     earliest_wake_up_time;   /**< Earliest wake up time in microseconds, only valid if \c p1_sleep is True */
    uint32     latest_wake_up_time;     /**< Latest wake up time in microseconds, only valid if \c p1_sleep is True */
} IPC_P1_DEEP_SLEEP_MSG_PRIM;

/**
 * When received generates an inter-processor interrupt for P0 if received by P1
 * or P1 if received by P0.
 */
typedef struct
{
    IPC_HEADER header;
} IPC_SIGNAL_INTERPROC_EVENT_PRIM;

/**
 * DIR FROM_P0 Inform P1 that a stream has been closed.
 * \ingroup ipc_prim
 */
typedef struct
{
    IPC_HEADER header;        /**< id =
                                    \c IPC_SIGNAL_ID_STREAM_DESTROYED */
    uint16 source;            /**< The source of the stream that has been closed. */
    uint16 sink;              /**< The sink of the stream that has been closed. */
} IPC_STREAM_DESTROYED;

/**
 * DIR FROM_P1 Deep sleep wakeup source configuration.
 * \ingroup ipc_prim
 */
typedef struct
{
    IPC_HEADER header;  /**< id = \c IPC_SIGNAL_ID_DEEP_SLEEP_WAKEUP_SOURCE */
    uint32 wake_source; /**< Set of bits where the low bits come from the
                             enumeration \c deep_sleep_wakeup_mask indicating
                             sources the subsystem would like to wake it if
                             they occur. */
    uint32 mask;        /**< Indicates which wake_source bit is valid to update
                             the wake up sources configuration */
} IPC_DEEP_SLEEP_WAKEUP_SOURCE;

/**
 * DIR FROM_P0 Inform P1 that operator have been destroyed.
 * \ingroup ipc_prim
 */
typedef struct
{
    IPC_HEADER header;      /**< id = \c IPC_SIGNAL_ID_OPERATORS_DESTROYED */
    uint16  num_operators;  /**< Number of operators in op[] list */
    Operator *op_list;      /**< Pointer to list of operators that
                                 have been destroyed. */
} IPC_OPERATORS_DESTROYED;

/**
 * Carrier for a uart primitive.
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_STREAM_UART_SINK or
                                 \c IPC_SIGNAL_ID_STREAM_UART_SINK_RSP */
} IPC_STREAM_UART_SINK;

/**
 * Indicates the trap API version information
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_TRAP_API_VERSION */
    /** Major, Minor and Tertiary version numbers of the Trap API */
    uint32  version[3];
    /** Length of the \c trapset_bitmap array */
    uint32  trapset_bitmap_dwords;
    /** Bitmap of trapsets that are supported by the firmware of the
     * sending processor. The actual length of the array is given by
     * \c trapset_bitmap_dwords */
    const uint32  *trapset_bitmap;
} IPC_TRAP_API_VERSION_INFO;

/**
 * DIR FROM_P1 Request P0 to send RADIOTEST cmd to BT subsystem.
 */
typedef struct
{
    IPC_HEADER header;      /**< id = \c IPC_SIGNAL_ID_TESTTRAP_BT_REQ */
    uint16 len;             /**< Payload length */
    uint16 padding;         /**< Padding to ensure offsetof(cmd) is 32-bit aligned */
    uint16 cmd[1];          /**< BCCMD payload, struct hack */
} IPC_TESTTRAP_BT_REQ;

/**
 * Carrier for a SD MMC slot init primitive.
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_SD_MMC_SLOT_INIT_REQ or
                                 \c IPC_SIGNAL_ID_SD_MMC_SLOT_INIT_RSP */
    bool init;
} IPC_SD_MMC_SLOT_INIT;

/**
 * Carrier for a SD MMC read data primitive.
 */
typedef struct
{
    IPC_HEADER header; /**< id = \c IPC_SIGNAL_ID_SD_MMC_READ_DATA_REQ or
                                 \c IPC_SIGNAL_ID_SD_MMC_READ_DATA_RSP */
    uint8 slot;
    uint8 *buff;
    uint32 start_block;
    uint32 blocks_count;
} IPC_SD_MMC_READ_DATA;

/**
 * DIR FROM_P0 Inform P1 memory access exception information.
 * \ingroup ipc_prim
 */
typedef struct
{
    IPC_HEADER header;        /**< id =
                                    \c IPC_SIGNAL_ID_MEMORY_ACCESS_FAULT_INFO */
    uint32 pc;
    uint32 address;
    uint8 type;
    uint8 areaidx;
    uint8 count;
} IPC_MEMORY_ACCESS_FAULT_INFO;

#include "ipc/gen/ipc_trap_api_prims.h"

#endif /* IPC_PRIM_H__ */

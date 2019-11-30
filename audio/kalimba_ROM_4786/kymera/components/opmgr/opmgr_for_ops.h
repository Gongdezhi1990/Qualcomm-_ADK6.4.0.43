/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  opmgr_for_ops.h
 * \ingroup opmgr
 *
 * Operator Manager public header file. <br>
 * This file contains public opmgr functions and types that are used also by operators. <br>
 *
 */

#ifndef OPMGR_FOR_OPS_H
#define OPMGR_FOR_OPS_H

#include "types.h"
#include "buffer.h"
#include "stream/stream.h"
#include "status_prim.h"
#include "platform/profiler_c.h"
#include "opmgr/opmgr.h"
#include "rate/rate_types.h"

#define TERMINAL_NUM_MASK            0x003F

/* TODO- this for now defined to match current (temp?) convention in EP-to-terminal translation */
#define TERMINAL_SINK_MASK              (1 << 23)

/* Changes the terminal direction. */
#define SWAP_TERMINAL_DIRECTION(TERMINAL)   ((TERMINAL)^(TERMINAL_SINK_MASK))

/* Changes the channel number. */
#define CHANGE_TERMINAL_CHANNEL(TERMINAL, CHANNEL)   ( ((TERMINAL)&(~TERMINAL_NUM_MASK))|(CHANNEL) )

/** Bitfield definitions for returning the terminals that an operator touched */
#define TOUCHED_NOTHING (0)

#define TOUCHED_SINK_0 (1)
#define TOUCHED_SINK_1 (1 << 1)
#define TOUCHED_SINK_2 (1 << 2)
#define TOUCHED_SINK_3 (1 << 3)
#define TOUCHED_SINK_4 (1 << 4)
#define TOUCHED_SINK_5 (1 << 5)
#define TOUCHED_SINK_6 (1 << 6)
#define TOUCHED_SINK_7 (1 << 7)
#define TOUCHED_SINK_8 (1 << 8)
#define TOUCHED_SINK_9 (1 << 9)
#define TOUCHED_SINK_10 (1 << 10)
#define TOUCHED_SINK_11 (1 << 11)
#define TOUCHED_SINK_12 (1 << 12)
#define TOUCHED_SINK_13 (1 << 13)
#define TOUCHED_SINK_14 (1 << 14)
#define TOUCHED_SINK_15 (1 << 15)
#define TOUCHED_SINK_16 (1 << 16)
#define TOUCHED_SINK_17 (1 << 17)
#define TOUCHED_SINK_18 (1 << 18)
#define TOUCHED_SINK_19 (1 << 19)
#define TOUCHED_SINK_20 (1 << 20)
#define TOUCHED_SINK_21 (1 << 21)
#define TOUCHED_SINK_22 (1 << 22)
#define TOUCHED_SINK_23 (1 << 23)
#if (DAWTH == 32)
#define TOUCHED_SINK_24 (1 << 24)
#define TOUCHED_SINK_25 (1 << 25)
#define TOUCHED_SINK_26 (1 << 26)
#define TOUCHED_SINK_27 (1 << 27)
#define TOUCHED_SINK_28 (1 << 28)
#define TOUCHED_SINK_29 (1 << 29)
#define TOUCHED_SINK_30 (1 << 30)
#define TOUCHED_SINK_31 (1 << 31)
#endif /* DAWTH == 32 */

#define TOUCHED_SOURCE_0 (1)
#define TOUCHED_SOURCE_1 (1 << 1)
#define TOUCHED_SOURCE_2 (1 << 2)
#define TOUCHED_SOURCE_3 (1 << 3)
#define TOUCHED_SOURCE_4 (1 << 4)
#define TOUCHED_SOURCE_5 (1 << 5)
#define TOUCHED_SOURCE_6 (1 << 6)
#define TOUCHED_SOURCE_7 (1 << 7)
#define TOUCHED_SOURCE_8 (1 << 8)
#define TOUCHED_SOURCE_9 (1 << 9)
#define TOUCHED_SOURCE_10 (1 << 10)
#define TOUCHED_SOURCE_11 (1 << 11)
#define TOUCHED_SOURCE_12 (1 << 12)
#define TOUCHED_SOURCE_13 (1 << 13)
#define TOUCHED_SOURCE_14 (1 << 14)
#define TOUCHED_SOURCE_15 (1 << 15)
#define TOUCHED_SOURCE_16 (1 << 16)
#define TOUCHED_SOURCE_17 (1 << 17)
#define TOUCHED_SOURCE_18 (1 << 18)
#define TOUCHED_SOURCE_19 (1 << 19)
#define TOUCHED_SOURCE_20 (1 << 20)
#define TOUCHED_SOURCE_21 (1 << 21)
#define TOUCHED_SOURCE_22 (1 << 22)
#define TOUCHED_SOURCE_23 (1 << 23)
#if (DAWTH == 32)
#define TOUCHED_SOURCE_24 (1 << 24)
#define TOUCHED_SOURCE_25 (1 << 25)
#define TOUCHED_SOURCE_26 (1 << 26)
#define TOUCHED_SOURCE_27 (1 << 27)
#define TOUCHED_SOURCE_28 (1 << 28)
#define TOUCHED_SOURCE_29 (1 << 29)
#define TOUCHED_SOURCE_30 (1 << 30)
#define TOUCHED_SOURCE_31 (1 << 31)
#endif /* DAWTH == 32 */

/****************************************************************************
Type Declarations
*/


struct OPERATOR_DATA;

/** Function pointer prototype of message handler functions. */
typedef bool (*handler_function)(struct OPERATOR_DATA *cap_data, void *message_body,
              unsigned *response_id, void **response_data);

#ifdef INSTALL_OPERATOR_CREATE_PENDING
/* Support for asynchronous operator functions.
   This is not currently intended for customer usage */
typedef enum
{
    HANDLER_FAILED = FALSE,
    HANDLER_COMPLETE = TRUE,
    HANDLER_INCOMPLETE
} PENDABLE_OP_HANDLER_RETURN;

/* Callback function that may be returned by operator function in response_data */
typedef void (*pending_operator_cb)(struct OPERATOR_DATA *cap_data, 
                uint16 msgId,void *msg,tRoutingInfo *routing,             
                unsigned supplied_id);

/* Function pointer prototype for message handler functions that can pend. */
typedef PENDABLE_OP_HANDLER_RETURN (*handler_function_with_cb)
                    (struct OPERATOR_DATA *cap_data, void *message_body,
                     unsigned *response_id, void **response_data);
#endif

/** Structure for capabilities to return the terminals they touched in */
typedef struct
{
    /** Bitfield representing the source terminals that were written to and
     * should be kicked */
    unsigned sources;
    /** Bitfield representing the sink terminals that should be kicked as the
     * capability is starved. */
    unsigned sinks;
} TOUCHED_TERMINALS;

/** Struct used by capabilities to communicate their entry points to opmgr. */
/* Do not forget to update enum OPCMD_ID after modifying this structure. */
typedef struct
{
    handler_function op_create;
    handler_function op_destroy;
    handler_function op_start;
    handler_function op_stop;
    handler_function op_reset;
    handler_function op_connect;
    handler_function op_disconnect;
    handler_function op_buffer_details;
    handler_function op_data_format;
    handler_function op_get_sched_info;
} handler_lookup_struct;

/**
 * Message IDs
 */
/* The values in the enum must match the order of the fields in the
 * previous structure. There should be no gaps.
 * Except for 2 special values that should be kept at the end of the
 * range, the last value should always be OPCMD_LAST_ID. */
typedef enum
{
    OPCMD_CREATE = 0x0000,
    OPCMD_DESTROY = 0x0001,
    OPCMD_START = 0x0002,
    OPCMD_STOP = 0x0003,
    OPCMD_RESET = 0x0004,
    OPCMD_CONNECT = 0x0005,
    OPCMD_DISCONNECT = 0x0006,
    OPCMD_BUFFER_DETAILS = 0x0007,
    OPCMD_DATA_FORMAT = 0x0008,
    OPCMD_GET_SCHED_INFO = 0x0009,
    OPCMD_TEST = 0x000A,
    OPCMD_ARRAY_SIZE = 0x000B, /* Do not forget to update this value. */

    OPCMD_MESSAGE = 0xFFFD,
    OPCMD_FROM_OPERATOR = 0xFFFE,
    OPCMD_INVALID = 0xFFFF,
} OPCMD_ID;

/* The array is used by opmgr to access the member functions quickly when
   given OPCMD_ID */
typedef union
{
    handler_function      by_index[OPCMD_ARRAY_SIZE];
    handler_lookup_struct by_member;
} handler_lookup_table_union;

/** Forward declaration of OP_OPMSG_RSP_PAYLOAD. */
typedef struct op_opmsg_rsp_payload OP_OPMSG_RSP_PAYLOAD;

/** Function pointer prototype of operator message handler functions. */
typedef bool (*opmsg_handler_function)(struct OPERATOR_DATA *op_data, void *message_body,
        unsigned *response_length, OP_OPMSG_RSP_PAYLOAD **response_data);

/** Structure of an operator message function table entry */
typedef struct
{
    /** Opmsg ID associated with the handler */
    unsigned id;

    /** Function pointer of the function that is to be used to service the operator message */
    opmsg_handler_function handler;
} opmsg_handler_lookup_table_entry;


/** Static capability data */
typedef struct CAPABILITY_DATA
{
    /** Capability ID */
    unsigned id;

    /** Version information - hi and lo parts */
    unsigned version_msw;
    unsigned version_lsw;

    /** Max number of sinks/inputs and sources/outputs */
    unsigned max_sinks;
    unsigned max_sources;

    /** Pointer to message handler function table */
    const handler_lookup_struct *handler_table;

    /** Pointer to operator message handler function table */
    const opmsg_handler_lookup_table_entry *opmsg_handler_table;

    /** Pointer to data processing function */
    void (*process_data)(struct OPERATOR_DATA*, TOUCHED_TERMINALS*);

    /** Processing time information */
    unsigned processing_time;

    /** Size of capability-specific per-instance data */
    unsigned instance_data_size;

} CAPABILITY_DATA;


/** Operator states */
typedef enum
{
    OP_RUNNING,
    OP_NOT_RUNNING
    /* ... TODO */
} OP_STATE;


/**
 * Forward declaration of an internal structure used by opmgr for managing
 * propagation of kicks between operators.
 */
typedef struct KPT KP_TABLE;

/** Standard operator data structure, with capability-specific data pointer */

/* Warning - structure offsets are defined in capability_common_asm.h
 * Changing anything here will probably need matching changes there too 
 */
typedef struct OPERATOR_DATA
{
    /** Connection ID - TODO: possibly partially redundant with client ID */
    unsigned int con_id;

    /** operator ID */
    unsigned id;

    /** Task ID */
    taskid task_id;

    /** Creator client ID */
    unsigned creator_client_id;

    /** Creator processor ID */
    unsigned int processor_id; 

    /** Pointer to static capability data */
    const CAPABILITY_DATA *cap_data;

    /** Flag indicating running / stopped etc. state */
    OP_STATE state;

    /** Field indicating the direction in wich the kicks are ignored. */
    STOP_KICK stop_chain_kicks;

    /** Table caching operators & endpoints that may need to recieve a kick
     * after this operator has run.
     */
    KP_TABLE *kick_propagation_table;

#ifdef PROFILER_ON
    /**
     * Pointer to the profiler of the operator. This will measure the MIPS
     * usage of the process_data function of the operator.
     */
    profiler *profiler;
#endif

    /** Pointer to a next operator in a list, e.g. kept my OpMgr */
    struct OPERATOR_DATA* next;
    /** to save a few instructions and a memory read in redirection */
    void (*local_process_data)(struct OPERATOR_DATA*, TOUCHED_TERMINALS*);

    /** Pointer for sub-classing a capability*/
    void *cap_class_ext;

    /** Some extra data needed by specific instance */
    void* extra_op_data;

} OPERATOR_DATA;

/** Standard response message structure */
typedef struct
{
    unsigned op_id;
    /** STATUS_KYMERA is the enum of status values */
    unsigned status;
    union{
        unsigned data;
        unsigned err_code;
    }resp_data;
}OP_STD_RSP;


/** Scheduler information response message structure */
typedef struct
{
    unsigned op_id;
    /** STATUS_KYMERA is the enum of status values */
    unsigned status;
    unsigned block_size;
    unsigned run_period;
    bool locally_clocked;
}OP_SCHED_INFO_RSP;


/** Buffer details response message structure */
typedef struct
{
    /** The ID of the operator the response is from */
    unsigned op_id;
    /** Whether the request was successful or encountered an error */
    unsigned status;

    /** Flag indicating if the operator terminal wants to supply the buffer for the connection */
    bool supplies_buffer:1;
    /** Flag indicating if the operator terminal can be overridden by the endpoint
     * being connected to. */
    bool can_override:1;
    /** Flag indicating if the operator terminal needs to override the endpoint
     * being connected to */
    bool needs_override:1;
    /** Flag indicating if the operator terminal can run in-place re-using the
     * same buffer at a corresponding input/output terminals */
    bool runs_in_place:1;
#ifdef INSTALL_METADATA
    bool supports_metadata:1;
    tCbuffer *metadata_buffer;
#endif /* INSTALL_METADATA */

    /** This union is discriminated by the flags the combination of flags that have been set */
    union buffer_op_info_union{
        /** The minimum size of buffer being requested to form the connection */
        unsigned buffer_size;
        /** The buffer supplied when supplies_buffer is TRUE */
        tCbuffer *buffer;

        /** The buffer parameters being requested when runs_in_place is TRUE */
        struct{
            /**
             * The in place terminal shows where the operator would run in pace on for
             * the asked endpoint. */
            unsigned int in_place_terminal;
            /** The minimum buffer size in words needed for the operator. */
            unsigned int size;
            /** A pointer to the buffer that the in_place_terminal is connected to.
             * NULL if the terminal is not yet connected. */
            tCbuffer *buffer;
        }in_place_buff_params;
    }b;
}OP_BUF_DETAILS_RSP;

/** Get configuration message response structure */
typedef struct
{
    unsigned op_id;
    unsigned status;
    unsigned length;
    uint32 value;
}OP_GET_CONFIG_RSP;

/** Header structure for operator command message (OPCMD_MESSAGE). */
typedef struct
{
    /**client_id - To allow operators to determine the message source,*/
    unsigned client_id;
    /** length of the remaining content */
    unsigned length;
}OPCMD_MSG_HEADER;

/** Operator message header structure. */
typedef struct
{
    OPCMD_MSG_HEADER cmd_header;
    /** operator message ID */
    unsigned msg_id;
}OPMSG_HEADER;

/* Macros for accessing the header fields for the operator command message.*/
#define OPMGR_GET_OPCMD_MESSAGE_CLIENT_ID(x)  ((x)->cmd_header.client_id)
#define OPMGR_GET_OPCMD_MESSAGE_LENGTH(x)     ((x)->cmd_header.length)
#define OPMGR_GET_OPCMD_MESSAGE_MSG_ID(x)     ((x)->msg_id)

/** Get config result for key OPMSG_OP_TERMINAL_RATEMATCH_MEASUREMENT */
typedef struct
{
    /** Approximate (averaged) sample period deviation */
    int sp_deviation;

    /** Measurement triple in rate library format */
    RATE_RELATIVE_RATE measurement;
} OPMSG_GET_CONFIG_RM_MEASUREMENT;

/** Get config result, forward declared in opmgr.h */
union OPMSG_GET_CONFIG_RESULT
{
    /** Default result is one word */
    uint32 value;

    /** Result for OPMSG_OP_TERMINAL_RATEMATCH_MEASUREMENT */
    OPMSG_GET_CONFIG_RM_MEASUREMENT rm_measurement;
};

/** Get configuration message structure */
typedef struct
{
    OPMSG_HEADER header;
    /** Key- specify the type of the get configuration. */
    unsigned key;
    /** Pointer which will be populated with the asked configuration value. */
    OPMSG_GET_CONFIG_RESULT* result;
}OPMSG_GET_CONFIG;

/** Configure parameters for key OPMSG_OP_TERMINAL_RATEMATCH_REFERENCE */
typedef struct
{
    /** Legacy adjustment */
    int32 ratio;

    /** Optional reference triple */
    RATE_RELATIVE_RATE ref;
}
OPMSG_CONFIGURE_RM_REFERENCE;

/** Configure message structure */
typedef struct
{
    OPMSG_HEADER header;
    /** Key- specify the type of the configure. */
    unsigned key;
    /** The configure value which will be applied to the operator endpoint.
     * For some keys, this is a pointer to a struct. */
    uint32 value;
}OPMSG_CONFIGURE;

/** Generic opmsg REQ structure - header followed by (any) payload */
typedef struct
{
    OPMSG_HEADER header;

    /** Some payload */
    unsigned payload[];
}OP_MSG_REQ;

/* Macros for accessing the header fields for the operator message.*/
#define OPMGR_GET_OPMSG_MSG_ID(x)             ((x)->header.msg_id)
#define OPMGR_GET_OPMSG_CLIENT_ID(x)          ((x)->header.cmd_header.client_id)
#define OPMGR_GET_OPMSG_LENGTH(x)             ((x)->header.cmd_header.length)


/**
 * 'Set control' message data block
 */
typedef struct
{
    unsigned control_id;
    unsigned msw;
    unsigned lsw;   
}OPMSG_SET_CONTROL_BLOCK;

/**
 * 'Set control' message structre
 */
typedef struct
{
    OPMSG_HEADER header;
    unsigned num_blocks;          
    OPMSG_SET_CONTROL_BLOCK   data[];
}OPMSG_SET_CONTROL_MSG;

/**
 * 'Get parameters' message data block
 */
typedef struct 
{
    unsigned param_id;
    unsigned num_params;
}OPMSG_PARAM_BLOCK;

/**
 * 'Get parameters' message structure definiton
 */
typedef struct
{
    OPMSG_HEADER header;
    unsigned num_blocks;
    OPMSG_PARAM_BLOCK data_block[];
}OPMSG_PARAM_MSG;

/**
 * 'Set parameters' message data block
 */
typedef struct 
{
    unsigned param_id;
    unsigned num_params;
    unsigned *values;       /* Pointer to the 3:2 encoded parameter values */
}OPMSG_SET_PARAM_BLOCK;

/**
 * 'Set parameters' message structure definition
 */
typedef struct
{
    OPMSG_HEADER header;
    unsigned num_blocks;
    OPMSG_SET_PARAM_BLOCK data_block[];
}OPMSG_SET_PARAM_MSG;

/**
 * 'Get status' message structure definition
 */
typedef struct
{
    OPMSG_HEADER header;
    unsigned stat_config;
}OPMSG_GET_STATUS_MSG;
  
/**
 * 'Set UCID' message structure definition
 */
typedef struct
{
    OPMSG_HEADER header;
    unsigned ucid;
}OPMSG_SET_UCID_MSG;

/**
 * 'Get LOGICAL PS ID' message structure definition
 */
typedef struct
{
    OPMSG_HEADER header;
    unsigned sid;
}OPMSG_GET_LOGICAL_PS_ID_MSG;

/** Common message IDs for any operators that use the described functionality. We don't
 * call these "standard" IDs because not every operator uses the related features. 
 */
typedef enum
{
    MSG_FROM_OP_FADEOUT_DONE = 0x2000, /* it is matched with the historic value */
    MSG_FROM_OP_XXX_YYY /* TODO - whatever other messages */
} COMMON_MSG_FROM_OP_ID;


/** Operator messages - response structure */
/* NOTE: length is 1 or more -  at least an op. msg ID / key ID is present in the payload */
typedef struct
{
    unsigned op_id;
    unsigned status;
    unsigned length;
    unsigned payload[];
} OP_OPMSG_RSP;

/** Variable length opmsg response payload (an opmsg ID, a
 *  capability- & opmsg-specific status word, followed by optional generic payload data.
 */
struct op_opmsg_rsp_payload
{
    unsigned msg_id;
    union
    {
        /* Only the CPS response messages use the status word */
        struct
        {
            unsigned status;
            unsigned data[];
        } cps;
        unsigned raw_data[1];
    } u;
};

/* Payload size in words, with just message ID */
#define OPMSG_RSP_PAYLOAD_SIZE_RAW_DATA(x) ((x) + 1)

/* Payload size in words, with message ID and status */
#define OPMSG_RSP_PAYLOAD_SIZE_CPS_DATA(x) ((x) + 2)



/****************************************************************************
Macro Declarations
*/
/** Convert an internal opid into an external opid */
#define INT_TO_EXT_OPID(id) (STREAM_EP_OP_BIT | (id << STREAM_EP_OPID_POSN))

/** Convert an external opid into an internal opid */
#define EXT_TO_INT_OPID(id) get_opid_from_opidep(id)

/** Convert an internal opid into an external sink endpoint */
#define INT_TO_EXT_SINK(id) (STREAM_EP_OP_SINK | (id << STREAM_EP_OPID_POSN))

/* Helper macros for opmsg field lookup
 * These are necessary because the structure definitions and associated macros 
 * in opmsg_prim are wrong for at least some platforms
 */
#define OPCMD_MSG_HEADER_SIZE (sizeof(OPCMD_MSG_HEADER)/sizeof(unsigned int))

/* macro for reading a word from opmsg with an offset from a named field */
#define OPMSG_FIELD_GET_FROM_OFFSET(msg, msgtype, field, offset) ((uint16)(((unsigned int *)msg)[msgtype##_##field##_WORD_OFFSET + OPCMD_MSG_HEADER_SIZE + offset]))

/* get a uint16 opmsg field */
#define OPMSG_FIELD_GET(msg, msgtype, field) OPMSG_FIELD_GET_FROM_OFFSET(msg, msgtype, field, 0)

/* get a uint32 opmsg filed */
#define OPMSG_FIELD_GET32(msg, msgtype, field) (((uint32)OPMSG_FIELD_GET_FROM_OFFSET(msg, msgtype, field, 0)) + \
                                                (((uint32)OPMSG_FIELD_GET_FROM_OFFSET(msg, msgtype, field, 1))<<16))

/****************************************************************************
Public Function Definitions
*/

/**
 * \brief Helper function for kicking the operator in the background from an
 * abnormal source. e.g. some special interrupt the operator setup
 * 
 * \param op_data The data structure of the operator to kick
 */
static inline void opmgr_kick_operator(OPERATOR_DATA *op_data)
{
    raise_bg_int(op_data->task_id);
}

/**
 * \brief Helper function for issue kicks from an operator
 * 
 * \param op_data       The data structure of the operator to kick
          source_kicks  Mask of sources to kick
          sink_kicks    Mask of sinks to kick
 */
void opmgr_kick_from_operator(OPERATOR_DATA *op_data,unsigned source_kicks,unsigned sink_kicks);

extern OPERATOR_DATA* get_op_data_from_id(unsigned int id);

#endif  /* OPMGR_FOR_OPS_H */

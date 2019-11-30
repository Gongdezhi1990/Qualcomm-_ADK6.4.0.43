/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  base_op.h
 * \ingroup capabilities
 *
 * Multi-channel Base operator public header file. <br>
 *
 */

#ifndef MULTI_CHAN_BASE_OP_H
#define MULTI_CHAN_BASE_OP_H

/****************************************************************************
Include Files
*/
#include "opmgr/opmgr_for_ops.h"

/****************************************************************************
Public Constant Declarations
*/

#define MULTI_CHAN_DEFAULT_BLOCK_SIZE  1

#define MULTI_CHAN_INT_PART_BITS 5

/**
 * TTP debug messages and macros.
 */
#ifdef MULTI_CHAN_OP_DEBUG

#define MULTI_CHAN_DBG_MSG1(x, a)             L0_DBG_MSG1(x, a)
#define MULTI_CHAN_DBG_MSG2(x, a, b)          L0_DBG_MSG2(x, a, b)
#define MULTI_CHAN_DBG_MSG3(x, a, b, c)       L0_DBG_MSG3(x, a, b, c)
#define MULTI_CHAN_DBG_MSG4(x, a, b, c, d)    L0_DBG_MSG4(x, a, b, c, d)

#else  /* MULTI_CHAN_OP_DEBUG */

#define MULTI_CHAN_DBG_MSG1(x, a)             ((void)0)
#define MULTI_CHAN_DBG_MSG2(x, a, b)          ((void)0)
#define MULTI_CHAN_DBG_MSG3(x, a, b, c)       ((void)0)
#define MULTI_CHAN_DBG_MSG4(x, a, b, c, d)    ((void)0)

#endif /* MULTI_CHAN_OP_DEBUG */


/****************************************************************************
Public Type Declarations
*/

/* Configuration Flags */
#define MULTI_INPLACE_FLAG    0x1
#define MULTI_HOT_CONN_FLAG   0x2
#define MULTI_STREAM_FLAG     0x4
#define MULTI_METADATA_FLAG   0x8

/* Connection state (legacy) - TODO  Move to CPS */
typedef enum
{
    OPSTATE_INTERNAL_CONNECTED=1,
    OPSTATE_INTERNAL_READY=2
}OPSTATE_INTERNAL;

/**
 * Type of tags supported by the operator.
 */
typedef enum
{
    UNSUPPORTED_TAG = 0,
    TIMESTAMPED_TAG,
    TIME_OF_ARRIVAL_TAG,
    VOID_TAG,
    EMPTY_TAG
} multi_chan_tag_types;


/**
 *  Structure to store information from a tag.
 */
typedef struct
{
    /** input sample rate used for tag transfers */
    unsigned in_rate;
    
    /** Timestamp or time of arrival from the last metadata tag processed */
    unsigned timestamp;

    /** Sample period adjustment value from the last metadata tag */
    unsigned spa;

    /** Samples read since the last metadata tag */
    unsigned samples_after;

    /* Error offset ID from last tag */
    unsigned err_offset_id;
} multi_chan_ttp_last_tag;

/* Basic channel descriptor  */
typedef struct multi_chan_channel_struc
{  
   struct multi_chan_channel_struc *next_active;      /* Linked list of active channels */
   tCbuffer *sink_buffer_ptr;                         /* Sink tBuffer    (tCbuffer*)*/
   tCbuffer *source_buffer_ptr;                       /* Source tBuffer  (tCbuffer*)*/   
}MULTI_CHANNEL_CHANNEL_STRUC;

/* Constructor and destructor prototypes */
typedef bool (*MULTI_CHANNEL_CREATE_FN)(OPERATOR_DATA*,MULTI_CHANNEL_CHANNEL_STRUC *,unsigned);
typedef void (*MULTI_CHANNEL_DESTROY_FN)(OPERATOR_DATA*,MULTI_CHANNEL_CHANNEL_STRUC *);

typedef unsigned (*MULTICHAN_DATA_ADJUST_FN)(MULTI_CHANNEL_CHANNEL_STRUC*,unsigned);


/* multi-channel definition plus channel descriptor */
typedef struct multi_channel_def
{
   unsigned config;                 /* Capability support flags*/

   unsigned chan_obj_size;          /* Size of the extended channel structure */
   unsigned buffer_size;            /* Capability desired buffer size */
   unsigned block_size;             /* Capability desired block size */

   unsigned active_chan_mask;       /* Bitwise flag of connected channels */

   MULTI_CHANNEL_CREATE_FN  create_fn;    /* Active Channel constructor */
   MULTI_CHANNEL_DESTROY_FN destroy_fn;   /* Active channel destructor */

#ifdef INSTALL_METADATA
    tCbuffer *metadata_ip_buffer;   /** The input buffer with metadata to transport from */
    tCbuffer *metadata_op_buffer;   /** The output buffer with metadata to transport to */
    unsigned  metadata_delay;       /** Delay to add/remove on start/stop */
    multi_chan_tag_types   last_tag_type;
 
#endif /* INSTALL_METADATA */

   /* Channel data */
   MULTI_CHANNEL_CHANNEL_STRUC  *first_active;  /* Pointer to first active channel */
   unsigned                     chan_data[];   /* Channel array of size max sources for capability */
}MULTI_CHANNEL_DEF;


/****************************************************************************
Public Function Declarations
*/

/**
 * \brief Create and initialize channel definition 
 *
 * \param  op_data      Pointer to operator structure
 * \param  in_place     Specify in-place buffer support
 * \param  hot_connect  Specify connect/disconnect while running support
 * \param  chan_size    Size of capability specific channel structure
 *
 * \return - result TRUE if success
 */

bool multi_channel_create(OPERATOR_DATA* op_data,unsigned flags,unsigned chan_size);

/**
 * \brief Destroy channel definition
 *
 * \param  op_data      Pointer to operator structure
 */
void multi_channel_detroy(OPERATOR_DATA *op_data);

/**
 * \brief Connect terminal
 *
 * \param  op_data           Pointer to operator structure
 * \param  message_data      Pointer to message payload
 * \param  response_id       Pointer to response message ID
 * \param  response_data     Pointer to response payload
 *
 * \return - result TRUE if response generated 
 */
bool multi_channel_connect(OPERATOR_DATA *op_data,void *message_data, unsigned *response_id, void **response_data);

/**
 * \brief Disconnect terminal
 *
 * \param  op_data           Pointer to operator structure
 * \param  message_data      Pointer to message payload
 * \param  response_id       Pointer to response message ID
 * \param  response_data     Pointer to response payload
 *
 * \return - result TRUE if response generated 
 */
bool multi_channel_disconnect(OPERATOR_DATA *op_data,void *message_data, unsigned *response_id, void **response_data);

/**
 * \brief Handle schedule info request
 *
 * \param  op_data           Pointer to operator structure
 * \param  message_data      Pointer to message payload
 * \param  response_id       Pointer to response message ID
 * \param  response_data     Pointer to response payload
 *
 * \return - result TRUE if response generated 
 */
bool multi_channel_sched_info(OPERATOR_DATA *op_data,void *message_data, unsigned *response_id, void **response_data);

/**
 * \brief Handle buffer details request
 *
 * \param  op_data           Pointer to operator structure
 * \param  message_data      Pointer to message payload
 * \param  response_id       Pointer to response message ID
 * \param  response_data     Pointer to response payload
 *
 * \return - result TRUE if response generated 
 */
bool multi_channel_buffer_details(OPERATOR_DATA *op_data,void *message_data, unsigned *response_id, void **response_data);

/**
 * \brief Handle Start operator
 *
 * \param  op_data           Pointer to operator structure
 * \param  message_data      Pointer to message payload
 * \param  response_id       Pointer to response message ID
 * \param  response_data     Pointer to response payload
 *
 * \return - result TRUE if response generated 
 */
bool multi_channel_start(OPERATOR_DATA *op_data,void *message_data, unsigned *response_id, void **response_data);

/**
 * \brief  Handle Stop operator
 *
 * \param  op_data           Pointer to operator structure
 * \param  message_data      Pointer to message payload
 * \param  response_id       Pointer to response message ID
 * \param  response_data     Pointer to response payload
 *
 * \return - result TRUE if response generated 
 */
bool multi_channel_stop(OPERATOR_DATA *op_data,void *message_data, unsigned *response_id, void **response_data);

/**
 * \brief  Handle Reset operator
 *
 * \param  op_data           Pointer to operator structure
 * \param  message_data      Pointer to message payload
 * \param  response_id       Pointer to response message ID
 * \param  response_data     Pointer to response payload
 *
 * \return - result TRUE if response generated 
 */
bool multi_channel_reset(OPERATOR_DATA *op_data,void *message_data, unsigned *response_id, void **response_data);


/**
 * \brief stream based enable/disable operator message
 *
 * \param  op_data           Pointer to operator structure
 * \param  message_data      Pointer to message payload
 * \param  response_id       Pointer to response message ID
 * \param  response_data     Pointer to response payload
 *
 * \return - result TRUE if response generated 
 */
bool multi_channel_stream_based(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

/**
 * \brief set buffer size operator message
 *
 * \param  op_data           Pointer to operator structure
 * \param  message_data      Pointer to message payload
 * \param  response_id       Pointer to response message ID
 * \param  response_data     Pointer to response payload
 *
 * \return - result TRUE if response generated
 */
bool multi_channel_opmsg_set_buffer_size(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

/**
 * \brief Check for data to process and handle kicks
 *
 * \param  op_data      Pointer to operator structure
 * \param  touched      Pointer to kick status
 * \param  space_func   Function to adjust amount space
 * \param  data_func    Function to adjust amount data
 *
 * \return - result samples to process
 */
int multi_channel_check_buffers_adjusted(OPERATOR_DATA *op_data,TOUCHED_TERMINALS *touched,MULTICHAN_DATA_ADJUST_FN space_func,MULTICHAN_DATA_ADJUST_FN data_func);
/**
 * \brief Check for data to process and handle kicks
 *
 * \param  op_data      Pointer to operator structure
 * \param  touched      Pointer to kick status
 *
 * \return - result samples to process
 */
static inline unsigned multi_channel_check_buffers(OPERATOR_DATA *op_data,TOUCHED_TERMINALS *touched)
{
     /* Return amount of data to process */
    return multi_channel_check_buffers_adjusted(op_data,touched,NULL,NULL); /* use generic function to minimize duplicate code */
}


/**
 * \brief Set channel constructor & destructor
 *
 * \param  op_data      Pointer to operator structure
 * \param  create_fn    Pointer to channel constructor function
 * \param  destroy_fn   Pointer to channel destructor function
 *
 */
void multi_channel_set_callbacks(OPERATOR_DATA *op_data, MULTI_CHANNEL_CREATE_FN create_fn,MULTI_CHANNEL_DESTROY_FN destroy_fn);

/**
 * \brief Set buffer size for channels
 *
 * \param  op_data      Pointer to operator structure
 * \param  buffer_size  Buffer size
 *
 */
void multi_channel_set_buffer_size(OPERATOR_DATA *op_data, unsigned buffer_size);

/**
 * \brief Set block size for channels
 *
 * \param  op_data      Pointer to operator structure
 * \param  block_size   Block size
 *
 */
void multi_channel_set_block_size(OPERATOR_DATA *op_data, unsigned block_size);

/**
 * \brief copy or mute streams
 *
 * \param  first_stream          Pointer to first active stream
 * \param  samples_to_process    Number of samples to process
 * \param  bMute                 Mute flag 
 *
 */
void multi_channel_copy_mute(MULTI_CHANNEL_CHANNEL_STRUC *first_stream, unsigned samples_to_process,unsigned bMute);

/**
 * \brief advance buffers by amount
 *
 * \param  first_stream    Pointer to first active stream
 * \param  consumed        Number of samples to advance sink
 * \param  produced        Number of samples to advance source
 *
 */
void multi_channel_advance_buffers(MULTI_CHANNEL_CHANNEL_STRUC *first_stream, unsigned consumed,unsigned produced);

/**
 * \brief Get number of channels
 *
 * \param  op_data      Pointer to operator structure
 *
 * \return - number of channels
 */
unsigned multi_channel_get_channel_count(OPERATOR_DATA *op_data);


/**
 * \brief Get bitwise flag of active channels
 *
 * \param  op_data      Pointer to operator structure
 *
 * \return - bitwise flag of active channel
 */
static inline unsigned multi_channel_active_channels(OPERATOR_DATA *op_data)
{
   MULTI_CHANNEL_DEF *chan_def = (MULTI_CHANNEL_DEF*)op_data->cap_class_ext;
   return chan_def->active_chan_mask;
}

/**
 * \brief Get pointer to first active channel
 *
 * \param  op_data      Pointer to operator structure
 *
 * \return - Pointer to active channel
 */
static inline MULTI_CHANNEL_CHANNEL_STRUC* multi_channel_first_active_channel(OPERATOR_DATA *op_data)
{
   MULTI_CHANNEL_DEF *chan_def = (MULTI_CHANNEL_DEF*)op_data->cap_class_ext;
   return chan_def->first_active;
}

/**
 * \brief Get pointer to channel definition
 *
 * \param  op_data      Pointer to operator structure
 *
 * \return - Pointer to active channel
 */
static inline MULTI_CHANNEL_DEF* multi_channel_get_channel_def(OPERATOR_DATA *op_data)
{
   return (MULTI_CHANNEL_DEF*)op_data->cap_class_ext;
}


#ifdef INSTALL_METADATA
/**
 * \brief propagate metadata
 *
 * \param  op_data      Pointer to operator structure
 * \param  amount       Transfer amount
 *
 * \return - none
 */
void multi_channel_metadata_propagate(OPERATOR_DATA *op_data,unsigned amount);

/**
 * \brief propagate metadata 
 *
 * \param  op_data            Pointer to operator structure
 * \param  consumed           Transfer amount for sink
 * \param  produced           Transfer amount for source
 * \param  last_tag           TTP last metadata tag info
 *
 * \return - none
 */
void multi_channel_metadata_transfer(OPERATOR_DATA *op_data,unsigned consumed,unsigned produced,multi_chan_ttp_last_tag *last_tag);

/**
 * \brief Set the delay for metadata
 *
 * \param  op_data      Pointer to operator structure
 * \param  delay        Delay in samples
 *
 * \return - none
 */
static inline void multi_channel_set_metadata_delay(OPERATOR_DATA *op_data,unsigned delay)
{
   MULTI_CHANNEL_DEF *chan_def = (MULTI_CHANNEL_DEF*)op_data->cap_class_ext;
   chan_def->metadata_delay =  delay*OCTETS_PER_SAMPLE;
}

unsigned multi_channel_metadata_limit_consumption(OPERATOR_DATA *op_data,TOUCHED_TERMINALS *touched,unsigned consumed);

#endif

#endif /* MULTI_CHAN_BASE_OP_H */

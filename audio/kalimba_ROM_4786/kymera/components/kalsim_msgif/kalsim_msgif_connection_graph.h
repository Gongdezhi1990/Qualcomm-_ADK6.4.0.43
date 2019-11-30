/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file kalsim_msgif_connection_graph.h
 *
 * \section kalsim_msgif_connection_graph USAGE
 * This file describes the connection graph that will be set up in the test.
 * 
 */

 /****************************************************************************
 Include Files
 */
#ifndef KALSIM_MSGIF_CONNECTION_GRAPH_H
#define KALSIM_MSGIF_CONNECTION_GRAPH_H

#include "accmd/accmd.h"
#include "hydra_types.h"
#include "types.h"

/****************************************************************************
Public Macro Definitions
*/

/**
 * Number of real source or sink endpoints. Must be at least 1, max of 4
 * Assumed that endpoints will be of same type (PCM/I2S) and on same instance. If more
 * than one endpoint is requested, the slot/channel will be incremented */
#define MAX_SRC_EP    2
#define MAX_SINK_EP   2
#define ENDPOINT_TYPE       ACCMD_STREAM_DEVICE_PCM
#define ENDPOINT_INSTANCE   ACCMD_AUDIO_INSTANCE_0

/**
 * Number of operators. Must be at least 1 */
#define MAX_OP 8

#define MAX_OP_MSGS  5

/**
 * How long operators should run for, in msec. msgif intiates stop and clean up
 * connections after this interval.
 */
#define RUN_TIME 1000

/**
 * Specify if the endpoints need to be synchronised or not. Comment this out if the
 * test does not require endpoint synchronisation. If defined, all the source endpoints
 * will be synchronised with each other and all the sink endpoints will be synchronised
 * with each other.
 */
#define SYNC_ENDPOINTS
/****************************************************************************
Public Type Declarations
*/

/**
 * Type of endpoint, which is used by the msgif state machine to generate source/sink
 * id when connecting up the graph.
 */
typedef enum
{
    ep_real,
    ep_operator,
    ep_max
}ep_type;

/**
 * Stream config key value pairs.
 */
typedef struct config_msg_ts
{
    ACCMD_CONFIG_KEY config_key;
    uint32       value;
}config_msg;


/**
 * Operator descriptor describes the required operator, including cap_id, number of
 * source & sink endpoints. Each operator needs to be configured before connection using
 * a set of operator messages. The list of messages to be used is given by an index into
 * the opmsg_list.
 */
typedef struct operator_desc_ts
{
    CAP_ID capid;             /* cap id  */
    unsigned int op_msg_type; /* index in opmsg_list*/
    unsigned int num_src_ep;  /* num of src endpoints */
    unsigned int num_sink_ep; /* number of sink endpoints */
}operator_desc;

/**
 * operator message is operator specific. It can be specified as an array of 16-bit words
 * with length as a header.
 */
typedef struct operator_msg_desc_ts
{
    uint16 length;
    uint16   msg[];
}operator_msg_desc;

/**
 * Set of operator messages to be sent to a specific operator. Defines by the number of
 * messages (max MAX_OP_MSGS) and pointers to the messages.
 */
typedef struct operator_msg_table_ts
{
    unsigned int num_msgs;
    const uint16 *msg_table[MAX_OP_MSGS];
}operator_msg_table;

/**
 * connect desc describes a connection in the graph. It defines the source and sink, which
 * is an index in the source/sink for real endpoints or in the operator list for operator
 * endpoints. Sub index gives the connection index within the operator.
 */
typedef struct connect_desc_ts
{
    ep_type src_ep_type;
    unsigned int src_index;
    unsigned int src_sub_index;
    ep_type sink_ep_type;
    unsigned int sink_index;
    unsigned int sink_sub_index;
}connect_desc;

/****************************************************************************
Public constant definitions
*/

#if MAX_OP
/* List of operators to be created. To add more, increase MAX_OP and add the capability
 * id to the table below */
const operator_desc operator_list[MAX_OP] = {
#if MAX_OP > 1
            {CAP_ID_BASIC_PASS, 1, 1, 1},
            {CAP_ID_BASIC_PASS, 1, 1, 1},
            {CAP_ID_BASIC_PASS, 1, 1, 1},
            {CAP_ID_BASIC_PASS, 1, 1, 1},
            {CAP_ID_BASIC_PASS, 1, 1, 1},
            {CAP_ID_BASIC_PASS, 1, 1, 1},
            {CAP_ID_BASIC_PASS, 1, 1, 1},
#endif
            {CAP_ID_BASIC_PASS, 1, 1, 1}
};
#endif /* MAX_OP */

/* List of config messages sent to the source/sink endpoints. Here we assume that all
 * sinks and sources are of the same type (PCM, I2S etc.) and all are in the same hardware
 * instance. As a result, we only need to send stream configs to one of the source/sink
 * and we choose to send stream config to the 1st source
 */
const config_msg config_msg_list[] = {
        {ACCMD_CONFIG_KEY_STREAM_PCM_SYNC_RATE, 48000l},
        {ACCMD_CONFIG_KEY_STREAM_PCM_MASTER_CLOCK_RATE, 3072000l},
        {ACCMD_CONFIG_KEY_STREAM_PCM_MASTER_MODE, 0x00000001l},
        {ACCMD_CONFIG_KEY_STREAM_PCM_SLOT_COUNT, 0x00000004l},
        {ACCMD_CONFIG_KEY_STREAM_PCM_SAMPLE_FORMAT, 0x00000001l},
        {ACCMD_CONFIG_KEY_STREAM_PCM_SAMPLE_RISING_EDGE_ENABLE, 0x00000000l},
        {0,0} /* End with 0, 0 */
};

/*
 * Operator message list.
 * NOTE: This is still TBD.
 */
const uint16 change_input_data_type_16bigendian_data[] = {
        2,  /* length */
        10, /* Change Input data type message id */
        0   /* data format 16 bit big endian data */
};

const uint16 change_input_data_type_leftaligned_audio[] = {
        2,  /* length */
        10, /* Change Input data type message id */
        1   /* Q1.23 or Q1.31 left aligned audio */
};

const uint16 change_output_data_type_16bigendian_data[] = {
        2,  /* length */
        11, /* Change Output data type message id */
        0   /* data format 16 bit big endian data */
};

const uint16 change_output_data_type_leftaligned_audio[] = {
        2,  /* length */
        11, /* Change Output data type message id */
        1   /* Q1.23 or Q1.31 left aligned audio */
};

const operator_msg_table opmsg_list[] = {
        { 0, /* Number of messages to be sent for this type of operator */
          {NULL, NULL, NULL, NULL, NULL} /* Array of message pointers, change number if MAX_OP_MSGS changes  */
        },
        /* Free list, add more here */
        { 2, /* Number of messages to be sent for this type of operator */
          { change_input_data_type_leftaligned_audio,
            change_output_data_type_leftaligned_audio,
            NULL, NULL, NULL } /* Array of message pointers, change number if MAX_OP_MSGS changes  */
        }
};

/*
 * Audio source & sinks are real endpoints and their
 * sink/source id is used as is. Operator sink & source ids need to be generated from
 * the operator id using
 *   source_id = operator_id + connection + 0x2000
 *   sink_id   = operator_id + connection + 0xA000
 *   where, connection is the input/output "port" or connection index.
 * See ACCMD Commands spec section on stream_connect_req/resp for more details.
 */
const connect_desc connection_list[] = {
#if MAX_OP
        { ep_real, 0, 0, ep_operator, 0, 0 },
        { ep_operator, 0, 0, ep_operator, 1, 0 },
        { ep_operator, 1, 0, ep_operator, 2, 0 },
        { ep_operator, 2, 0, ep_operator, 3, 0 },
        { ep_operator, 3, 0, ep_real, 0, 0 },
        { ep_real, 1, 0, ep_operator, 4, 0 },
        { ep_operator, 4, 0, ep_operator, 5, 0 },
        { ep_operator, 5, 0, ep_operator, 6, 0 },
        { ep_operator, 6, 0, ep_operator, 7, 0 },
        { ep_operator, 7, 0, ep_real, 1, 0 },
#else
        { ep_real, 0, 0, ep_real, 0, 0 },
#endif /* MAX_OP */
        /* Finish with ep_max to ep_max connection entry */
        {ep_max, 0, 0, ep_max, 0, 0 }
};

#endif /* KALSIM_MSGIF_CONNECTION_GRAPH_H */

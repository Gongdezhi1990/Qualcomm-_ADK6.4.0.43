/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/** \file
 *
 *  This is the main public project header for the \c ipc LUT library.
 *
 */
/****************************************************************************
Include Files
*/

#ifndef IPC_MACROS_H
#define IPC_MACROS_H

/****************************************************************************
Public Type Declarations
*/

/****************************************************************************
Public Constant and macros
*/

/*
 * Macros for Message Queues and Message format header
 */
#define IPC_MSGHDR_PROTOCOL_BIT       6
#define IPC_MSGHDR_HDRTYPE_BIT        4
#define IPC_MSGHDR_INTEXT_BIT         3

#define ipc_set_msg_header(ipc_version, msgtype, payload)     \
                      (0xC400 | ((ipc_version)<<IPC_MSGHDR_PROTOCOL_BIT) | (((msgtype)&1)<<IPC_MSGHDR_INTEXT_BIT) | ((payload)&7))

#define ipc_get_msg_header_version(header)                    \
                      (((header) >> IPC_MSGHDR_PROTOCOL_BIT) & 0xFF)

#define ipc_get_msg_header_hdrtype(header)                    \
                      (((header) >> IPC_MSGHDR_HDRTYPE_BIT) & 0x03)

#define ipc_get_msg_header_msgtype(header)                    \
                      (((header) >> IPC_MSGHDR_INTEXT_BIT) & 0x01)

#define ipc_get_msg_header_payloadtype(header)                \
                      ( (header)       & 0x07)

/*
 * Macros for Message Channel channel id
 */
#define IPC_CHANID_INACTIVE             0x00
#define IPC_CHANID_INPROGRESS           0x01
#define IPC_CHANID_ACTIVE               0x03
#define IPC_CHANID_STATE_BIT            14
#define IPC_CHANID_OWNER_BIT            11
#define IPC_CHANID_PARTICIPANT_BIT      8

#define ipc_set_chanid(chan_state, chan_owner, chan_ptpt, offset)     \
                      ( (((chan_state)&3)<<IPC_CHANID_STATE_BIT) | (((chan_owner)&7)<<IPC_CHANID_OWNER_BIT) | (((chan_ptpt)&7)<<IPC_CHANID_PARTICIPANT_BIT) | ((offset)&15) )

#define ipc_get_chanid_state(chanid)                          \
                      (((chanid)>>IPC_CHANID_STATE_BIT)&3)

#define ipc_get_chanid_owner(chanid)                          \
                      (((chanid)>>IPC_CHANID_OWNER_BIT)&7)

#define ipc_get_chanid_participant(chanid)                    \
                      (((chanid)>>IPC_CHANID_PARTICIPANT_BIT)&7)

#define ipc_get_chanid_remote_proc(chanid) \
               (ipc_get_chanid_owner(chanid) == hal_get_reg_processor_id())? \
                ipc_get_chanid_participant(chanid): ipc_get_chanid_owner(chanid)


#define ipc_get_chanid_offset(chanid)                         \
                      ((chanid)&15)

/*
 * Macros for Queue ID
 */
#define IPC_MESSAGE_QUEUE_DEST_BIT      0
#define IPC_MESSAGE_QUEUE_SRC_BIT       3

#define ipc_set_queue_id(src,dst)                            \
                      (((src&7)<<IPC_MESSAGE_QUEUE_SRC_BIT) | (dst&7))

#define ipc_get_queue_id_dest(qid)                             \
                      ((qid)&7)

#define ipc_get_queue_id_src(qid)                              \
                      (((qid)>>IPC_MESSAGE_QUEUE_SRC_BIT)&7)


/*
 * Macros for signal id
 */
#define IPC_SIGTYPE_BIT                 15
#define IPC_SIGSOURCE_BIT               12

#define ipc_set_signalid(sig_type, sig_source, signal)        \
                      ( (((sig_type)&1)<<IPC_SIGTYPE_BIT) | (((sig_source)&7)<<IPC_SIGSOURCE_BIT) | ((signal)&0x0FFF) )

#define ipc_get_signalid_type(sigid)                          \
                      (((sigid)>>IPC_SIGTYPE_BIT)&1)

#define ipc_get_signalid_source(sigid)                        \
                      (((sigid)>>IPC_SIGSOURCE_BIT)&7)

#define ipc_get_signalid_signal(sigid)                        \
                      ((sigid)&0xFFF)

/*
 * Macros for data channel id
 */

#define IPC_MAX_DATA_PORTS              32
#define IPC_MAX_DATA_CHANNELS           16

#define IPC_DATA_CHANNEL_PORT_BIT       8
#define IPC_DATA_CHANNEL_DIR_BIT        7

#define IPC_DATA_CHANNEL_PORT_MASK      0xFF
#define IPC_DATA_CHANNEL_NUM_MASK       0x7F
#define IPC_DATA_CHANNEL_DIR_MASK       1U

#define ipc_set_data_channelid(channel_number, direction, port)      \
                      ( (((port)&IPC_DATA_CHANNEL_PORT_MASK)<<IPC_DATA_CHANNEL_PORT_BIT) |        \
                        (((direction)&IPC_DATA_CHANNEL_DIR_MASK)<<IPC_DATA_CHANNEL_DIR_BIT) |     \
                         ((channel_number)&IPC_DATA_CHANNEL_NUM_MASK) )

#define ipc_get_data_channelid_port(chanid)                          \
                      (((chanid)>>IPC_DATA_CHANNEL_PORT_BIT)&IPC_DATA_CHANNEL_PORT_MASK)

#define ipc_get_data_channelid_dir(chanid)                           \
                      (((chanid)>>IPC_DATA_CHANNEL_DIR_BIT)&IPC_DATA_CHANNEL_DIR_MASK)

#define ipc_get_data_channelid_channum(chanid)                       \
                      ((chanid)&IPC_DATA_CHANNEL_NUM_MASK)

#define ipc_invert_chanid_dir(chanid)                                \
                      ((chanid) ^ (1U<<IPC_DATA_CHANNEL_DIR_BIT))


/****************************************************************************
Public Variable Declarations
*/

/****************************************************************************
Public Function Declarations
*/

#endif /* IPC_MACROS_H */


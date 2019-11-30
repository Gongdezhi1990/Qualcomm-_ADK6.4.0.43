/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/

#ifndef _OP_CHANNEL_LIST_H_
#define _OP_CHANNEL_LIST_H_

#include "buffer/buffer.h"

#define OP_CHANNEL_LIST_SOURCE               0
#define OP_CHANNEL_LIST_SINK                 1

#define OP_CHANNEL_LIST_CONNECTION_INVALID  -1

typedef struct OP_CHANNEL_LIST
{
    /** source and sink terminal buffers for this channel */
    tCbuffer *pterminal_buf[2];
    
    /** terminal index for this channel */
    unsigned index;
    
    /** next channel in the linked-list */
    struct OP_CHANNEL_LIST* next;
    
} OP_CHANNEL_LIST;


#ifdef INSTALL_METADATA
/**
 * \brief   add a new source or sink connection to the channel list
 *
 * \param   list - address of a pointer to the channel list head
 * \param   pterminal_buf - pointer to terminal cbuffer structure to connect
 * \param   index - index of terminal to connect
 * \param   sink_flag - 1 if connecting a sink / 0 if connecting a source
 * \param   metadata_buff - Location where the operator's metadata buffer is tracked.
 *          This should correspond to the ip/op depending on the sink_flag value.
 *
 * \return  TRUE if connection was successful
 */
bool op_channel_list_connect(OP_CHANNEL_LIST** list, tCbuffer *pterminal_buf, unsigned index, unsigned sink_flag, tCbuffer **metadata_buff);
#else
/**
 * \brief   add a new source or sink connection to the channel list
 *
 * \param   list - address of a pointer to the channel list head
 * \param   pterminal_buf - pointer to terminal cbuffer structure to connect
 * \param   index - index of terminal to connect
 * \param   sink_flag - 1 if connecting a sink / 0 if connecting a source
 *
 * \return  TRUE if connection was successful
 */
bool op_channel_list_connect(OP_CHANNEL_LIST** list, tCbuffer *pterminal_buf, unsigned index, unsigned sink_flag);
#endif /* INSTALL_METADATA */

#ifdef INSTALL_METADATA
/**
 * \brief   remove a source or sink connection from the channel list
 *
 * \param   list - address of a pointer to the channel list head
 * \param   index - index of terminal to disconnect
 * \param   sink_flag - 1 if disconnecting a sink / 0 if disconnecting a source
 * \param   metadata_buff - Location where the operator's metadata buffer is tracked.
 *          This should correspond to the ip/op depending on the sink_flag value.
 *
 * \return  TRUE if disconnection was successful
 */
bool op_channel_list_disconnect(OP_CHANNEL_LIST** list, unsigned index, unsigned sink_flag, tCbuffer **metadata_buff);
#else
/**
 * \brief   remove a source or sink connection from the channel list
 *
 * \param   list - address of a pointer to the channel list head
 * \param   index - index of terminal to disconnect
 * \param   sink_flag - 1 if disconnecting a sink / 0 if disconnecting a source
 *
 * \return  TRUE if disconnection was successful
 */
bool op_channel_list_disconnect(OP_CHANNEL_LIST** list, unsigned index, unsigned sink_flag);
#endif /* INSTALL_METADATA */

/**
 * \brief   count the number of fully connected channels
 *
 *  for an entry to be fully connected, both source and sink pointer must be set
 *  this function will return -1 if any list entry is only half connected
 *
 * \param   list - address of a pointer to the channel list head
 *
 * \return  number of connected channels, or -1 if any invalid
 */
int op_channel_list_num_connected(OP_CHANNEL_LIST** list);

/**
 * \brief   free all allocated data in the channel list
 *
 * \param   list - address of a pointer to the channel list head
 */
void op_channel_list_free_list(OP_CHANNEL_LIST** list);

#endif  /* _OP_CHANNEL_LIST_H_ */

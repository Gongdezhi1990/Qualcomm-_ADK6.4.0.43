/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */
/*!
  @file broadcast_msg_client.c
  @brief Utility functions for sending Broadcast library messages to client
         task.
*/

#include "broadcast_private.h"
#include "broadcast_msg_client.h"
#include "gatt_broadcast_server_uuids.h"

#include <message.h>
#include <panic.h>
#include <sink.h>
#include <source.h>

/*! @brief Send BROADCAST_INIT_CFM message to client task, */
void broadcast_msg_client_init_cfm(Task client_task, BROADCAST* lib,
                                   broadcast_status status)
{
    MESSAGE_MAKE(message, BROADCAST_INIT_CFM_T);
    message->broadcast = lib;
    message->status = status;
    MessageSend(client_task, BROADCAST_INIT_CFM, message);
}

/*! @brief Send BROADCAST_DESTROY_CFM message to client task, */
void broadcast_msg_client_destroy_cfm(Task client_task, broadcast_status status)
{
    MESSAGE_MAKE(message, BROADCAST_DESTROY_CFM_T);
    message->status = status;
    MessageSend(client_task, BROADCAST_DESTROY_CFM, message);
}

/*! @brief Send BROADCAST_CONFIGURE_CFM message to client task, */
void broadcast_msg_client_config_cfm(BROADCAST* lib,
                                     broadcast_status status)
{
    MESSAGE_MAKE(message, BROADCAST_CONFIGURE_CFM_T);
    message->broadcast = lib;
    message->status = status;
    MessageSend(lib->client_task, BROADCAST_CONFIGURE_CFM, message);
}

/*! @brief Send BROADCAST_STATUS_IND message to client task, */
void broadcast_msg_client_status_ind(BROADCAST* lib,
                                     broadcast_status status)
{
    MESSAGE_MAKE(message, BROADCAST_STATUS_IND_T);
    message->broadcast = lib;
    message->status = status;
    MessageSend(lib->client_task, BROADCAST_STATUS_IND, message);
}

/*! @brief Send BROADCAST_START_BROADCAST_CFM message to client task, */
void broadcast_msg_client_start_broadcast_cfm(BROADCAST* lib,
                                              broadcast_status status,
                                              broadcast_mode start_mode,
                                              Sink sink)
{
    MESSAGE_MAKE(message, BROADCAST_START_BROADCAST_CFM_T);
    message->broadcast = lib;
    message->status = status;
    message->mode = start_mode;
    message->sink = sink;
    MessageSend(lib->client_task, BROADCAST_START_BROADCAST_CFM, message);
}

/*! @brief Send BROADCAST_STOP_BROADCAST_CFM message to client task, */
void broadcast_msg_client_stop_broadcast_cfm(BROADCAST* lib,
                                             broadcast_mode mode,
                                             broadcast_status status)
{
    MESSAGE_MAKE(message, BROADCAST_STOP_BROADCAST_CFM_T);
    message->broadcast = lib;
    message->status = status;
    message->mode = mode;
    MessageSend(lib->client_task, BROADCAST_STOP_BROADCAST_CFM, message);
}

/*! @brief Send BROADCAST_STOP_BROADCAST_IND message to client task. */
void broadcast_msg_client_stop_broadcast_ind(BROADCAST* lib)
{
    MESSAGE_MAKE(message, BROADCAST_STOP_BROADCAST_IND_T);
    message->broadcast = lib;
    MessageSend(lib->client_task, BROADCAST_STOP_BROADCAST_IND, message);
}

/*! @brief Send BROADCAST_START_RECEIVER_CFM message to client task, */
void broadcast_msg_client_start_receiver_cfm(BROADCAST* lib,
                                             broadcast_status status,
                                             broadcast_mode mode,
                                             Source source,
                                             receiver_csb_params* params)
{
    MESSAGE_MAKE(message, BROADCAST_START_RECEIVER_CFM_T);
    message->broadcast = lib;
    message->status = status;
    message->mode = mode;
    message->source = source;
    if (params)
    {
        message->csb_params = *params;
    }
    MessageSend(lib->client_task, BROADCAST_START_RECEIVER_CFM, message);
}

/*! @brief Send BROADCAST_STOP_RECEIVER_CFM message to client task, */
void broadcast_msg_client_stop_receiver_cfm(BROADCAST* lib,
                                            broadcast_status status)
{
    MESSAGE_MAKE(message, BROADCAST_STOP_RECEIVER_CFM_T);
    message->broadcast = lib;
    message->status = status;
    MessageSend(lib->client_task, BROADCAST_STOP_RECEIVER_CFM, message);
}

/*! @brief Send BROADCAST_STOP_RECEIVER_IND message to client task, */
void broadcast_msg_client_stop_receiver_ind(BROADCAST* lib)
{
    MESSAGE_MAKE(message, BROADCAST_STOP_RECEIVER_IND_T);
    message->broadcast = lib;
    MessageSend(lib->client_task, BROADCAST_STOP_RECEIVER_IND, message);
}

/*! @brief Send BROADCAST_AFH_UPDATE_IND message to client task. */
void broadcast_msg_client_afh_update_ind(BROADCAST* lib,
                                         AfhMap map,
                                         uint32 clock)
{
    MESSAGE_MAKE(message, BROADCAST_AFH_UPDATE_IND_T);
    
    message->broadcast = lib;
    memmove(&message->channel_map, map, sizeof(message->channel_map));
    message->clock = clock;
    MessageSend(lib->client_task, BROADCAST_AFH_UPDATE_IND, message);
}

/*! @brief Send BROADCAST_AFH_CHANNEL_MAP_CHANGED_IND message to client task. */ 
void broadcast_msg_client_afh_channel_map_changed_ind(BROADCAST* lib,
                                                      AfhMap map)
{
    MESSAGE_MAKE(message, BROADCAST_AFH_CHANNEL_MAP_CHANGED_IND_T);
    
    memmove(&message->channel_map, map, sizeof(message->channel_map));
    MessageSend(lib->client_task, BROADCAST_AFH_CHANNEL_MAP_CHANGED_IND, message);
}

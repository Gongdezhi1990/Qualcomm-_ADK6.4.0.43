/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */
/*!
  @file broadcast_msg_client.h
  @brief Interface to utility functions for sending Broadcast library 
         messages to client task.
*/

#ifndef BROADCAST_MSG_CLIENT_H_
#define BROADCAST_MSG_CLIENT_H_

#include "broadcast_private.h"

#include <message.h>
#include <sink.h>
#include <source.h>

/*!
    @brief Send BROADCAST_INIT_CFM message to client task.

    @param client_task [IN] Task registered with the broadcast library instance. 
    @param status [IN] Result of the initialisation operation.
*/
void broadcast_msg_client_init_cfm(Task client_task, BROADCAST* lib,
                                   broadcast_status status);

/*!
    @brief Send BROADCAST_DESTROY_CFM message to client task.

    @param client_task [IN] Task registered with the broadcast library instance. 
    @param status [IN] Result of the destruction operation.
*/
void broadcast_msg_client_destroy_cfm(Task client_task, broadcast_status status);

/*!
    @brief Send BROADCAST_CONFIGURE_CFM message to client task.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param status [IN] Result of the operation.
*/
void broadcast_msg_client_config_cfm(BROADCAST* lib,
                                     broadcast_status status);

/*!
    @brief Send BROADCAST_STATUS_IND message to client task.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param status [IN] Result of the operation.
*/
void broadcast_msg_client_status_ind(BROADCAST* lib, broadcast_status status);

/*!
    @brief Send BROADCAST_START_BROADCAST_CFM message to client task.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param mode [IN] The mode in which the broadcast was started.
    @param status [IN] Result of the operation.
*/
void broadcast_msg_client_start_broadcast_cfm(BROADCAST* lib,
                                              broadcast_status status,
                                              broadcast_mode start_mode,
                                              Sink sink);

/*!
    @brief Send BROADCAST_STOP_BROADCAST_CFM message to client task.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param mode [IN] The mode in which the broadcast was stopped.
    @param status [IN] Result of the operation.
*/
void broadcast_msg_client_stop_broadcast_cfm(BROADCAST* lib,
                                             broadcast_mode mode,
                                             broadcast_status status);

/*! @brief Send BROADCAST_STOP_BROADCAST_IND message to client task.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
*/
void broadcast_msg_client_stop_broadcast_ind(BROADCAST* lib);

/*!
    @brief Send BROADCAST_START_RECEIVER_CFM message to client task.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param status [IN] Result of the operation.
    @param mode [IN] The mode in which the receive was started.
    @param source [IN] Stream source of the data received from the Broadcaster. 
    @param params [IN] Pointer to the CSB receiver parameters.
*/
void broadcast_msg_client_start_receiver_cfm(BROADCAST* lib,
                                             broadcast_status status,
                                             broadcast_mode mode,
                                             Source source,
                                             receiver_csb_params* params);

/*!
    @brief Send BROADCAST_STOP_RECEIVER_CFM message to client task.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param status [IN] Result of the operation.
*/
void broadcast_msg_client_stop_receiver_cfm(BROADCAST* lib,
                                            broadcast_status status);

/*!
    @brief Send BROADCAST_STOP_RECEIVER_IND message to client task.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
*/
void broadcast_msg_client_stop_receiver_ind(BROADCAST* lib);

/*!
    @brief Send BROADCAST_AFH_UPDATE_IND message to client task.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param map [IN] New channel map calculated by firmware.
    @param clock [IN] Clock instant at which the new map was calculated.
*/
void broadcast_msg_client_afh_update_ind(BROADCAST* lib,
                                         AfhMap map,
                                         uint32 clock);

/*!
    @brief Send BROADCAST_AFH_CHANNEL_MAP_CHANGED_IND message to client task.  

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param map [IN] New channel map that is now applied and in use for the CSB link.
*/
void broadcast_msg_client_afh_channel_map_changed_ind(BROADCAST* lib,
                                                      AfhMap map);
#endif /* BROADCAST_MSG_CLIENT_H_ */ 



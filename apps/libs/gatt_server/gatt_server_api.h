/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_server_api.h
    
DESCRIPTION
    Contains routines responsible for building & sending external messages.
*/

#ifndef GATT_SERVER_API_H_
#define GATT_SERVER_API_H_

/* Firmware headers */
#include <csrtypes.h>
#include <message.h>
#include <panic.h>
#include <stdlib.h>

/* GATT Server headers */
#include "gatt_server.h"

/* Macros for creating messages */
#define MAKE_GATT_SERVER_MESSAGE(TYPE) MESSAGE_MAKE(message,TYPE##_T)

/*******************************************************************************
NAME
    gattServerSendReadClientConfigInd
    
DESCRIPTION
    Build and send a GATT_SERVER_READ_CLIENT_CONFIG_IND message.
    
PARAMETERS
    task        The task to send the message to.
    cid         The connection ID of the device requesting read of descriptor.
    handle      The Handle being accessed.

RETURNS
    TRUE if the message was sent, FALSE otherwise.
*/
bool gattServerSendReadClientConfigInd(Task task, uint16 cid, uint16  handle);


/*******************************************************************************
NAME
    gattServerSendWriteClientConfigInd
    
DESCRIPTION
    Build and send a GATT_SERVER_WRITE_CLIENT_CONFIG_IND message.
    
PARAMETERS
    task        The task to send the message to.
    cid         The connection ID of the device requesting read of descriptor.
    value       The value the remote device wants to write to the descriptor.
    
RETURNS
    TRUE if the message was sent, FALSE otherwise.
*/
bool gattServerSendWriteClientConfigInd(Task task, uint16 cid, uint16 value);


/*******************************************************************************
NAME
    gattServerSendServiceChangedIndicationCfm
    
DESCRIPTION
    Build and send a GATT_SERVER_SERVICE_CHANGED_INDICATION_CFM message.
    
PARAMETERS
    task        The task to send the message to.
    cid         The connection ID of the device requesting read of descriptor.
    status      The GATT Status to add to the message.
    
RETURNS
    TRUE if the message was sent, FALSE otherwise.
*/
bool gattServerSendServiceChangedIndicationCfm(Task task,
                                               uint16 cid,
                                               gatt_status_t status);

#endif

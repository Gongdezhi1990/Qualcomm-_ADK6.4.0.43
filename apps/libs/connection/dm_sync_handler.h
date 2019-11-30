/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_sync_handler.h
    
DESCRIPTION

*/

#ifndef    CONNECTION_DM_SYNC_HANDLER_H_
#define    CONNECTION_DM_SYNC_HANDLER_H_

#include <app/bluestack/dm_prim.h>
#include "connection_private.h"

#ifndef CL_EXCLUDE_SYNC

/* If we don't get a DM_SYNC_REGISTER_CFM by the time this expires, assume it failed */
#define SYNC_REGISTER_TIMEOUT    (1000)

/* If we don't get a DM_SYNC_UNREGISTER_CFM by the time this expires, assume it failed */
#define SYNC_UNREGISTER_TIMEOUT    (1000)


/****************************************************************************
NAME    
    connectionHandleSyncRegisterReq

DESCRIPTION
    Register the task as utilising Synchronous connections. This registers it with 
    BlueStack so on an incoming Synchronous connection the task will be asked whether
    its willing to accept it.

RETURNS
    void
*/
void connectionHandleSyncRegisterReq(const CL_INTERNAL_SYNC_REGISTER_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleSyncRegisterCfm
    
DESCRIPTION
    Task has been sucessfully registered for receiving Synchronous connection 
    notifications - inform the client.
    
RETURNS
    void
*/
void connectionHandleSyncRegisterCfm(const DM_SYNC_REGISTER_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleSyncRegisterTimeoutInd
    
DESCRIPTION
    Task has not been registered for receiving Synchronous connection 
    notifications - inform the client.
    
RETURNS
    void
*/
void connectionHandleSyncRegisterTimeoutInd(const CL_INTERNAL_SYNC_REGISTER_TIMEOUT_IND_T *ind);


/****************************************************************************
NAME    
    connectionHandleSyncUnregisterReq

DESCRIPTION
    Unregister task with BlueStack indicating it is no longer interested in
    being notified about incoming Synchronous connections.

RETURNS
    void
*/
void connectionHandleSyncUnregisterReq(const CL_INTERNAL_SYNC_UNREGISTER_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleSyncUnregisterCfm

DESCRIPTION
    Task has been sucessfully unregistered from receiving Synchronous connection 
    notifications - inform the client.

RETURNS
    void
*/
void connectionHandleSyncUnregisterCfm(const DM_SYNC_UNREGISTER_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleSyncUnregisterTimeoutInd

DESCRIPTION
    Task has not been unregistered from receiving Synchronous connection 
    notifications - inform the client.

RETURNS
    void
*/
void connectionHandleSyncUnregisterTimeoutInd(const CL_INTERNAL_SYNC_UNREGISTER_TIMEOUT_IND_T *ind);


/****************************************************************************
NAME    
    connectionHandleSyncConnectReq

DESCRIPTION
    Request to open a Synchronous connection to a remote device.

RETURNS
    void
*/
void connectionHandleSyncConnectReq(const CL_INTERNAL_SYNC_CONNECT_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleSyncConnectCfm

DESCRIPTION
    Response to the Synchronous connection request indicating either that a Synchronous
    connection has been opened or that the attempt has failed.

RETURNS
    void
*/
void connectionHandleSyncConnectCfm(Task theAppTask, const DM_SYNC_CONNECT_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleSyncConnectInd

DESCRIPTION
    Indication that the remote device wishes to open a Synchronous connection.

RETURNS
    void
*/
void connectionHandleSyncConnectInd(const DM_SYNC_CONNECT_IND_T *ind);


/****************************************************************************
NAME    
    connectionHandleSyncConnectCompleteInd

DESCRIPTION
    Indication that the remote devices wish to open a Synchronous connection has completed.

RETURNS
    void
*/
void connectionHandleSyncConnectCompleteInd(Task theAppTask, const DM_SYNC_CONNECT_COMPLETE_IND_T *ind);


/****************************************************************************
NAME    
    connectionHandleSyncConnectRes

DESCRIPTION
    Response accepting (or not) an incoming Synchronous connection.

RETURNS
    void
*/
void connectionHandleSyncConnectRes(const CL_INTERNAL_SYNC_CONNECT_RES_T *res);


/****************************************************************************
NAME    
    connectionHandleSyncDisconnectReq

DESCRIPTION
    Request to disconnect an existing Synchronous connection.

RETURNS
    void
*/
void connectionHandleSyncDisconnectReq(const CL_INTERNAL_SYNC_DISCONNECT_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleSyncDisconnectInd

DESCRIPTION
    Indication that the Synchronous connection has been disconnected. This is 
    received by both sides, regardless of which one initiated the disconnect.

RETURNS
    void
*/
void connectionHandleSyncDisconnectInd(const DM_SYNC_DISCONNECT_IND_T *ind);


/****************************************************************************
NAME    
    connectionHandleSyncDisconnectCfm

DESCRIPTION
    Confirmation that the Synchronous connection has been disconnected. This is 
    received by both sides, regardless of which one initiated the disconnect <--- is this correct?

RETURNS
    void
*/
void connectionHandleSyncDisconnectCfm(const DM_SYNC_DISCONNECT_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleSyncRenegotiateReq

DESCRIPTION
    Request to change the connection parameters of an existing Synchronous connection.

RETURNS
    void
*/
void connectionHandleSyncRenegotiateReq(const CL_INTERNAL_SYNC_RENEGOTIATE_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleSyncRenegotiateInd

DESCRIPTION
    Indication that remote device has changed the connection parameters of an existing 
    Synchronous connection.

RETURNS
    void
*/
void connectionHandleSyncRenegotiateInd(const DM_SYNC_RENEGOTIATE_IND_T *ind);


/****************************************************************************
NAME    
    connectionHandleSyncRenegotiateCfm

DESCRIPTION
    Confirmation of local device's attempt to change the connection parameters of an existing 
    Synchronous connection.


RETURNS
    void
*/
void connectionHandleSyncRenegotiateCfm(const DM_SYNC_RENEGOTIATE_CFM_T *cfm);

#endif
#endif    /* CONNECTION_DM_SYNC_HANDLER_H_ */

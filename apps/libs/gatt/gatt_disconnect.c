/****************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_disconnect.c        

DESCRIPTION
    Functions for the disconnection.
        GattDisconnectRequest() ->
        GATT_DISCONNECT_IND_T <-

NOTES

*/

/***************************************************************************
    Header Files
*/

#include "gatt.h"
#include "gatt_private.h"

#include <vm.h>

/****************************************************************************
NAME
    GattDisconnectRequest
           
DESCRIPTION
   API function. 

RETURN

*/
void GattDisconnectRequest(uint16 cid)
{
    MAKE_GATT_MESSAGE(GATT_INTERNAL_DISCONNECT_REQ);
    message->cid                = cid;
    MessageSend(gattGetTask(), GATT_INTERNAL_DISCONNECT_REQ, message);
}

/****************************************************************************
NAME
    gattHandleInternalDisconnectReq
           
DESCRIPTION
   Handle the internal disconnection request. 

RETURN

*/
void gattHandleInternalDisconnectReq(const GATT_INTERNAL_DISCONNECT_REQ_T *req)
{
    /* Disconnect crossover - the CID could already have been removed from the 
       connection list by a Disconnect Ind from the other device. Therefore,
       do nothing!
     */
    if (gattCidIsValid(req->cid))
    {
        /* Disconnect is a non-blocking message scenario. */
        MAKE_ATT_PRIM(ATT_DISCONNECT_REQ);
        prim->cid = req->cid;
        VmSendAttPrim(prim);
    }
}

/****************************************************************************
NAME
    gattHandleAttDisonnectInd

DESCRIPTION
    Handles the ATT_DISCONNECT_IND from BlueStack. 

RETURNS

*/
void gattHandleAttDisconnectInd(const ATT_DISCONNECT_IND_T *ind)
{
    cid_map_t *conn = gattFindConn(ind->cid);

    if (conn)
    {
        MAKE_GATT_MESSAGE(GATT_INTERNAL_DELETE_CONN);

        message->cid = ind->cid;
        message->theAppTask = conn->task;

        switch (ind->reason)
        {
            case L2CA_DISCONNECT_NORMAL:
                message->status = gatt_status_success;
                break;
            case L2CA_DISCONNECT_LINK_LOSS:
                message->status = gatt_status_link_loss;
                break;
            default:
                message->status = gatt_status_abnormal_disconnection;
                GATT_DEBUG_INFO((
                            "Disconnect Ind Reason: 0x%04x\n", 
                            ind->reason
                            ));
                break;
        }

        /* stop using this connection */
        conn->closing = TRUE;
        
        /* send message to ourself telling to delete the connection, but
         * queue the message after any pending scenario are completed.
         */
        MessageSendConditionallyOnTask(
            gattGetTask(),
            GATT_INTERNAL_DELETE_CONN,
            message,
            &conn->data.app);
    }
    else
    {
        GATT_DEBUG_INFO((
                    "gattHandleAttDisconnectInd: cid 0x%04x not found\n", 
                    ind->cid
                    ));
    }
}

/****************************************************************************
NAME
    gattHandleInternalDeleteConn

DESCRIPTION

    Send the GATT_DISCONNECT_IND on to the app task which initiated the
    connection and then deleted the cid_map for this connection.

RETURNS

*/
void gattHandleInternalDeleteConn(const GATT_INTERNAL_DELETE_CONN_T *msg)
{
    MAKE_GATT_MESSAGE(GATT_DISCONNECT_IND);
    
    message->cid = msg->cid;
    message->status = msg->status;

    MessageSend(msg->theAppTask, GATT_DISCONNECT_IND, message);
    gattDeleteCid(msg->cid);
}


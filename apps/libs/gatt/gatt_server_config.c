/****************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_server_config.c

DESCRIPTION
    Functions to handle GATT Server Configuration sub-procedures.

    This module implements support for the following GATT sub-procedures:
    - Exchange MTU for the client
    - Exchange MTU for the server

NOTES

*/

/***************************************************************************
    Header Files
*/

#include "gatt.h"
#include "gatt_private.h"

#include <stdlib.h>
#include <vm.h>

#if (GATT_FEATURES & GATT_EXCHANGE_MTU)

/*
 * Exchange MTU for the client side
 */

static void gattSendExchangeMtuCfm(Task task,
                                   gatt_status_t status,
                                   uint16 cid,
                                   int16 mtu);

void GattExchangeMtuRequest(Task theAppTask, uint16 cid, uint16 mtu)
{
    cid_map_t *conn = gattFindConn(cid);

    if (conn && !conn->closing)
    {
        MAKE_GATT_MESSAGE(GATT_INTERNAL_EXCHANGE_MTU_REQ);

#ifdef GATT_DEBUG_LIB
        if (mtu > 0 && mtu < GATT_DEFAULT_MTU)
            GATT_DEBUG(
                ("GattExchangeMtuRequest: Invalid MTU range (1..22) %d\n", mtu)
                );
#endif

        message->common.task = theAppTask;
        message->common.cid = cid;
        message->mtu = mtu;

        MessageSendConditionallyOnTask(
            gattGetTask(),
            GATT_INTERNAL_EXCHANGE_MTU_REQ,
            message,
            &conn->data.app);
    }
    else
    {
        gattSendExchangeMtuCfm(theAppTask, gatt_status_invalid_cid, cid, 0);
    }
}

/****************************************************************************
NAME
    gattSendExchangeMtuCfm

DESCRIPTION
    Send the GATT_EXCHANGE_MTU_CFM message.

RETURNS

*/
static void gattSendExchangeMtuCfm(Task task,
                                   gatt_status_t status,
                                   uint16 cid,
                                   int16 mtu)
{
    MAKE_GATT_MESSAGE(GATT_EXCHANGE_MTU_CFM);

    message->status = gatt_message_status(status);
    message->cid    = cid;
    message->mtu    = mtu;

    MessageSend(task, GATT_EXCHANGE_MTU_CFM, message);
}

/****************************************************************************
NAME
    gattHandleInternalExchangeMtuReq
           
DESCRIPTION
    Handle the internal Exchange MTU Req 

RETURN

*/
void gattHandleInternalExchangeMtuReq(const GATT_INTERNAL_EXCHANGE_MTU_REQ_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->common.cid);

    /* allow exchange mtu only once */
    if (conn->mtu_exchanged)
    {
        gattSendExchangeMtuCfm(m->common.task,
                               gatt_status_mtu_already_exchanged,
                               m->common.cid, 0);
        return;
    }
    
    gattSetConnState(conn, m->common.task, gatt_ms_exchange_mtu);

    gatt_exchange_mtu_req(m->common.cid, m->mtu);
}


/****************************************************************************
NAME
    gattHandleAttExchangeMtuCfm

DESCRIPTION
    Handles the ATT_EXCHANGE_MTU_CFM from BlueStack. 

RETURNS

*/
void gattHandleAttExchangeMtuCfm(const ATT_EXCHANGE_MTU_CFM_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->cid);

    /* send confirmation to the application */
    gattSendExchangeMtuCfm(conn->data.app, m->result, m->cid, m->mtu);

    /* do not allow second exchange */
    conn->mtu_exchanged = TRUE;
    conn->mtu = m->mtu;
    
    gattSetConnState(conn, NULL, gatt_ms_none);
}

#endif /* GATT_EXCHANGE_MTU */

/*
 * Exchange MTU for server side - mandatory
 */

void GattExchangeMtuResponse(uint16 cid, uint16 mtu)
{
    gatt_exchange_mtu_rsp(cid, mtu);
}

/****************************************************************************
NAME
    gattHandleAttExchangeMtuInd

DESCRIPTION
    Handles the ATT_EXCHANGE_MTU_IND from BlueStack. 

RETURNS

*/
void gattHandleAttExchangeMtuInd(gattState *theGatt,
                                 const ATT_EXCHANGE_MTU_IND_T *m)
{
    MAKE_GATT_MESSAGE(GATT_EXCHANGE_MTU_IND);

    message->cid = m->cid;
    message->mtu = m->client_mtu;

    MessageSend(theGatt->theAppTask, GATT_EXCHANGE_MTU_IND, message);
}

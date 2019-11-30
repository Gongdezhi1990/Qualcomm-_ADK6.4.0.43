/****************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_notification_indication.c        

DESCRIPTION
    Handle the Notification and Indication GATT message scenarios, both 
    server and client side.

NOTES

*/

/***************************************************************************
    Header Files
*/

#include "gatt.h"
#include "gatt_private.h"

#include <string.h>
#include <stdlib.h>
#include <vm.h>

/****************************************************************************
NAME
    gattSendCfm

DESCRIPTION
    Send an Indication or Notification Cfm message, depending on the the 
    flags set (Indication or Notification
RETURNS

*/

static void gattSendCfm(
    Task task, 
    gatt_status_t status, 
    uint16 cid,
    uint16 flags,
    uint16 handle
    )
{
    /* NOTIFICATION and INDICATION message are identical in structure. */
    MessageId id = GATT_INDICATION_CFM;
    MAKE_GATT_MESSAGE(GATT_INDICATION_CFM);

    message->status = status;
    message->cid = cid;
    message->handle = handle;

    if (flags == ATT_HANDLE_VALUE_NOTIFICATION)
        id = GATT_NOTIFICATION_CFM;

    MessageSend(task, id, message);
}

/****************************************************************************
NAME
    GattNotification

DESCRIPTION
    Send a Notification Req to the client. Does not update the ATT database.

RETURNS

*/
void GattNotificationRequest(
        Task theAppTask, 
        uint16 cid, 
        uint16 handle, 
        uint16 size_value,
        const uint8 *value
        )
{
    cid_map_t *conn = gattFindConn(cid);
    
    if(conn && !conn->closing)
    {
        MAKE_GATT_MESSAGE_WITH_VALUE(
            GATT_INTERNAL_HANDLE_VALUE_NTF, 
            size_value,
            value
            );

        message->theAppTask = theAppTask;
        message->cid        = cid;
        message->handle     = handle;
        message->flags      = ATT_HANDLE_VALUE_NOTIFICATION;

        MessageSendConditionallyOnTask(
                gattGetTask(),
                GATT_INTERNAL_HANDLE_VALUE_NTF,
                message,
                &conn->data.app
                );
    }
    else
    {
        /* Device record was not found, send error */
        gattSendCfm(
            theAppTask, 
            gatt_status_device_not_found,
            cid,
            ATT_HANDLE_VALUE_NOTIFICATION,
            handle
            );
    }
}

/****************************************************************************
NAME
    GattIndicationRequest

DESCRIPTION
    Send an Indication (no response) to the client. Does not update the ATT
    database.

RETURNS

*/
void GattIndicationRequest(
        Task theAppTask, 
        uint16 cid, 
        uint16 handle, 
        uint16 size_value,
        const uint8 *value
        )
{
    cid_map_t *conn = gattFindConn(cid);
    
    if(conn && !conn->closing)
    {
        MAKE_GATT_MESSAGE_WITH_VALUE(
            GATT_INTERNAL_HANDLE_VALUE_REQ, 
            size_value,
            value
            );

        message->theAppTask = theAppTask;
        message->cid        = cid;
        message->handle     = handle;
        message->flags      = ATT_HANDLE_VALUE_INDICATION;

        MessageSendConditionallyOnTask(
                gattGetTask(),
                GATT_INTERNAL_HANDLE_VALUE_REQ,
                message,
                &conn->data.app
                );
    }
    else
    {
        /* Device record was not found, send error */
        gattSendCfm(
            theAppTask, 
            gatt_status_device_not_found,
            cid,
            ATT_HANDLE_VALUE_INDICATION,
            handle
            );
    }
}

/****************************************************************************
NAME
    gattHandleInternalHandleValueNtf

DESCRIPTION
    Handle the Internal Handle Value Ntf message, which is only used 
    for Notifications. 

RETURNS

*/
void gattHandleInternalHandleValueNtf(
        const GATT_INTERNAL_HANDLE_VALUE_NTF_T *req
        )
{
    if ( gattCidIsValid(req->cid) )
    {
        gatt_msg_scen_t msg_scen = gatt_ms_notification_req;
        cid_map_t *conn = PanicNull(gattFindConn(req->cid));
            
        MAKE_ATT_PRIM(ATT_HANDLE_VALUE_NTF);

        gattSetConnState(conn, req->theAppTask, msg_scen);
       
        prim->cid           = req->cid;
        prim->handle        = req->handle;
        prim->flags         = req->flags;
        prim->size_value    = req->size_value;
        
        if (req->size_value)
        {
            uint8 *value = (uint8 *)PanicUnlessMalloc(req->size_value);
            memmove(value, req->value, req->size_value);
            prim->value = VmGetHandleFromPointer(value);
        }
        else
        {
            prim->value = 0;    /* Unlikely */
        }

        VmSendAttPrim(prim);
    }
    else 
    {
        gattSendCfm(
            req->theAppTask,
            gatt_status_invalid_cid,
            req->cid,
            req->flags,
            0
            );
    }
}

/****************************************************************************
NAME
    gattHandleInternalHandleValueReq

DESCRIPTION
    Handle the Internal Handle Value Request message, which is only used
    for indications. 

RETURNS

*/
void gattHandleInternalHandleValueReq(const GATT_INTERNAL_HANDLE_VALUE_REQ_T *req)
{
    if ( gattCidIsValid(req->cid) )
    {
        gatt_msg_scen_t msg_scen = gatt_ms_indication_req;
        conn_data_t* data = (conn_data_t*)PanicNull(gattGetCidData(req->cid));
        cid_map_t *conn = gattFindConnOrPanic(req->cid);
            
        MAKE_ATT_PRIM(ATT_HANDLE_VALUE_REQ);

        gattSetConnState(conn, req->theAppTask, msg_scen);
       
        prim->cid           = req->cid;
        prim->handle        = req->handle;
        prim->flags         = req->flags;
        prim->size_value    = req->size_value;
        
        if (req->size_value)
        {
            uint8 *value = (uint8 *)PanicUnlessMalloc(req->size_value);
            memmove(value, req->value, req->size_value);
            prim->value = VmGetHandleFromPointer(value);
        }
        else
        {
            prim->value = 0;    /* Unlikely */
        }

        /* Stash the flags to determine the CFM message later. */
        data->req.notif_ind.notif_ind_flags = req->flags;
        data->req.notif_ind.handle = req->handle;
        
        VmSendAttPrim(prim);
    }
    else 
    {
        gattSendCfm(
            req->theAppTask,
            gatt_status_invalid_cid,
            req->cid,
            req->flags,
            0
            );
    }
}

/****************************************************************************
NAME
    gattAttHandleValueNtfCfm

DESCRIPTION
    Handle the ATT Value Ntf message. 

RETURNS

*/
void gattHandleAttHandleValueNtfCfm(const ATT_HANDLE_VALUE_NTF_CFM_T *cfm)
{
    if ( gattCidIsValid(cfm->cid) )
    {
        gatt_status_t status = gatt_status_success;
        cid_map_t *conn = PanicNull(gattFindConn(cfm->cid));

        if (cfm->result)
        {
            if (cfm->result == ATT_RESULT_SUCCESS_SENT)
                status = gatt_status_success_sent;
            else
            {
                status = gatt_status_failure;
                GATT_DEBUG_INFO((
                    "gattAttHandleValueCfm: ATT result 0x%04x\n", 
                    cfm->result
                    ));
            }
        }
        
        /* From context, the flags must be for NOTIFICATION. */
        gattSendCfm(
            gattGetCidMappedTask(cfm->cid),
            status,
            cfm->cid,
            ATT_HANDLE_VALUE_NOTIFICATION,
            cfm->handle
            );

        /* Don't unlock until failure or success */
        if (status != gatt_status_success_sent)
        {
            gattSetConnState(conn, NULL, gatt_ms_none);
        }
    }
    else
        GATT_DEBUG_INFO((
            "gattAttHandleValueCfm: invalid cid 0x%04x\n", 
            cfm->cid
            ));
}

/****************************************************************************
NAME
    gattAttHandleValueCfm

DESCRIPTION
    Handle the Handle ATT Value Cfm message. 

RETURNS

*/
void gattHandleAttHandleValueCfm(const ATT_HANDLE_VALUE_CFM_T *cfm)
{
    if ( gattCidIsValid(cfm->cid) )
    {
        gatt_status_t status = gatt_status_success;
        conn_data_t *cd = (conn_data_t *)PanicNull(gattGetCidData(cfm->cid));
        cid_map_t *conn = gattFindConnOrPanic(cfm->cid);

        if (cfm->result)
        {
            
            if (cfm->result == ATT_RESULT_SUCCESS_SENT)
            {
                status = gatt_status_success_sent;
            }
            else
            {
                status = gatt_status_failure;
                GATT_DEBUG_INFO((
                    "gattAttHandleValueCfm: ATT result 0x%04x\n", 
                    cfm->result
                    ));
            }
        }
        
        gattSendCfm(
            gattGetCidMappedTask(cfm->cid),
            status,
            cfm->cid,
            cd->req.notif_ind.notif_ind_flags,
            cd->req.notif_ind.handle
            );

        /* Don't unlock until failure or success */
        if (status != gatt_status_success_sent)
        {
            gattSetConnState(conn, NULL, gatt_ms_none);
        }
    }
    else
    {
        GATT_DEBUG_INFO((
            "gattAttHandleValueCfm: invalid cid 0x%04x\n", 
            cfm->cid
            ));
    }
}

/****************************************************************************
NAME
    gattAttHandleValueind

DESCRIPTION
    Handle the ATT Handle Value Indication message. Could be a GATT
    Notification or Indication, depending on the flags. 

RETURNS

*/
void gattHandleAttHandleValueInd(const ATT_HANDLE_VALUE_IND_T *ind)
{
    uint8 *value = NULL;
    if (ind->size_value)
        value = (uint8 *) VmGetPointerFromHandle(ind->value);   

    if ( gattCidIsValid(ind->cid) )
    {
        /* Notification and Indication message structures are identical,
         * so use NOTFICATION and adjust the message id accordingly.
         */
        MessageId id = GATT_NOTIFICATION_IND;
        
        MAKE_GATT_MESSAGE_WITH_VALUE(
            GATT_NOTIFICATION_IND, 
            ind->size_value,
            value
            );

        message->cid = ind->cid;
        message->handle = ind->handle;

        if (ind->flags == ATT_HANDLE_VALUE_INDICATION)
            id = GATT_INDICATION_IND;

        MessageSend( gattGetCidMappedTask(ind->cid), id, message );
    }
    else    
    {
        GATT_DEBUG_INFO((
            "gattAttHandleValueInd: invalid cid 0x%04x\n", 
            ind->cid
            ));
    }

    free(value);
}

/****************************************************************************
NAME
    GattIndicationResponse

DESCRIPTION
    API function in response to GATT_INDICATION_IND. Non-locking, sends 
    response immediately.

RETURNS

*/
void GattIndicationResponse(uint16 cid)
{
    if( gattCidIsValid(cid) )
    {
        MAKE_ATT_PRIM(ATT_HANDLE_VALUE_RSP);

        prim->cid = cid;

        VmSendAttPrim(prim);
    }
    else
    {
        GATT_DEBUG_INFO((
            "GattIndicationResponse: invalid cid 0x%04x\n", 
            cid
            ));
    }
}



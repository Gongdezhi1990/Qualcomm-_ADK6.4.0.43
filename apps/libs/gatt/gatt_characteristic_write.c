/*************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_characteristic_write.c

DESCRIPTION
    Functions to handle GATT Characteristic Write sub-procedures.

    This module implements support for the following GATT sub-procedures:
    - Write Without Response

NOTES
*/

#include "gatt_private.h"

#include <stdlib.h>
#include <string.h>
#include <vm.h>
#ifndef HYDRACORE
#include <memory.h>
#endif /* HYDRACORE */

static void handleInternalWriteWithoutResponse(
    cid_map_t *conn,
    const GATT_INTERNAL_WRITE_REQ_T *m);

static void handleInternalWriteReqLong(
    cid_map_t *conn,
    const GATT_INTERNAL_WRITE_REQ_T *m);

static void handleInternalWriteReqPrepare(
    cid_map_t *conn,
    const GATT_INTERNAL_WRITE_REQ_T *m);

#if (GATT_FEATURES & (GATT_WRITE_COMMAND | \
                      GATT_WRITE_SIGNED | \
                      GATT_WRITE | \
                      GATT_WRITE_LONG | \
                      GATT_WRITE_RELIABLE))

static void internalWriteReq(Task theAppTask,
                             uint16 id,
                             uint16 cid,
                             uint16 handle,
                             uint16 offs,
                             uint16 size_value,
                             const uint8 *value)
{
    cid_map_t *conn = gattFindConn(cid);

    if (conn && !conn->closing)
    {
        MAKE_GATT_MESSAGE_WITH_VALUE(GATT_INTERNAL_WRITE_REQ,
                                     size_value, value);
        
        message->common.task = theAppTask;
        message->common.cid = cid;
        message->handle = handle;
        message->offs = offs;
        message->id = id;

        MessageSendConditionallyOnTask(
            gattGetTask(),
            GATT_INTERNAL_WRITE_REQ,
            message,
            &conn->data.app);
    }
    else
    {
        MAKE_GATT_MESSAGE(GATT_WRITE_CHARACTERISTIC_VALUE_CFM);

        message->cid = cid;
        message->handle = handle;
        message->status = gatt_status_invalid_cid;

        switch (id)
        {
#if (GATT_FEATURES & GATT_WRITE_COMMAND)
            case gatt_ms_write_without_response:
                id = GATT_WRITE_WITHOUT_RESPONSE_CFM;
                break;
#endif

#if (GATT_FEATURES & GATT_WRITE_SIGNED)
            case gatt_ms_signed_write_without_response:
                id = GATT_SIGNED_WRITE_WITHOUT_RESPONSE_CFM;
                break;
#endif

#if (GATT_FEATURES & GATT_WRITE)
            case gatt_ms_write_characteristic_value:
                id = GATT_WRITE_CHARACTERISTIC_VALUE_CFM;
                break;
#endif

#if (GATT_FEATURES & GATT_WRITE_LONG)
            case gatt_ms_write_long_characteristic_value:
                id = GATT_WRITE_LONG_CHARACTERISTIC_VALUE_CFM;
                break;
#endif

#if (GATT_FEATURES & GATT_WRITE_RELIABLE)
            case gatt_ms_prepare_write:
                id = GATT_RELIABLE_WRITE_PREPARE_CFM;
                break;
#endif
                
            default:
                /* horror error which can't happen, ever. */
                Panic();
        }
        
        MessageSend(theAppTask, id, message);
    }
}
#endif /* WRITE_COMMAND | WRITE_SIGNED | WRITE | WRITE_LONG */

#if (GATT_FEATURES & GATT_WRITE_COMMAND)
void GattWriteWithoutResponseRequest(Task theAppTask,
                                     uint16 cid,
                                     uint16 handle,
                                     uint16 size_value,
                                     const uint8 *value)
{
    internalWriteReq(theAppTask, gatt_ms_write_without_response, cid,
                     handle, 0 /* offs */, size_value, value);
}
#endif

#if (GATT_FEATURES & GATT_WRITE_SIGNED)
void GattSignedWriteWithoutResponseRequest(Task theAppTask,
                                           uint16 cid,
                                           uint16 handle,
                                           uint16 size_value,
                                           const uint8 *value)
{
    internalWriteReq(theAppTask, gatt_ms_signed_write_without_response, cid,
                     handle, 0 /* offs */, size_value, value);
}
#endif

#if (GATT_FEATURES & GATT_WRITE)
void GattWriteCharacteristicValueRequest(Task theAppTask,
                                         uint16 cid,
                                         uint16 handle,
                                         uint16 size_value,
                                         const uint8 *value)
{
    internalWriteReq(theAppTask, gatt_ms_write_characteristic_value, cid,
                     handle, 0 /* offs */, size_value, value);
}
#endif

#if (GATT_FEATURES & (GATT_WRITE_COMMAND | \
                      GATT_WRITE_SIGNED | \
                      GATT_WRITE | \
                      GATT_WRITE_LONG | \
                      GATT_WRITE_RELIABLE))
/*************************************************************************
NAME    
    gattHandleInternalWriteReq
    
DESCRIPTION
    This function is handler for GATT_INTERNAL_WRITE_REQ internal message.
    
RETURNS
    
*/
void gattHandleInternalWriteReq(const GATT_INTERNAL_WRITE_REQ_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->common.cid); /* never NULL */
    STASH(conn, stash, WRITE_CHARACTERISTIC_VALUE);
    uint16 flags;

    switch (m->id)
    {
#if (GATT_FEATURES & GATT_WRITE_COMMAND)        
        case gatt_ms_write_without_response:
            handleInternalWriteWithoutResponse(conn, m);
            return;
#endif

#if (GATT_FEATURES & GATT_WRITE_SIGNED)
        case gatt_ms_signed_write_without_response:
            flags = ATT_WRITE_COMMAND | ATT_WRITE_SIGNED;
            break;
#endif
            
#if (GATT_FEATURES & GATT_WRITE)
        case gatt_ms_write_characteristic_value:
            flags = ATT_WRITE_REQUEST;
            break;
#endif

#if (GATT_FEATURES & GATT_WRITE_LONG)
        case gatt_ms_write_long_characteristic_value:
            handleInternalWriteReqLong(conn, m);
            return;
#endif
            
#if (GATT_FEATURES & GATT_WRITE_RELIABLE)
        case gatt_ms_prepare_write:
            handleInternalWriteReqPrepare(conn, m);
            return;
#endif
            
        default:
            /* horror error which can't happen, ever. */
            flags = 0;
            Panic();
            
    }
    
    gattSetConnState(conn, m->common.task, (gatt_msg_scen_t)m->id);

    gatt_write_req(m->common.cid, m->handle, flags, m->size_value, m->value);

    /* Create the response message */
    MAKE_STASH(conn, stash, WRITE_CHARACTERISTIC_VALUE);

    stash->cid = m->common.cid;
    stash->handle = m->handle;    
}

/*************************************************************************
NAME    
    gattHandleAttWriteCfm
    
DESCRIPTION
    This function handles Bluestack response for ATT_WRITE_REQ.
    
RETURNS
    
*/
void gattHandleAttWriteCfm(const ATT_WRITE_CFM_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->cid); /* never NULL */
    STASH(conn, stash, WRITE_CHARACTERISTIC_VALUE);
    uint16 id;

    stash->status = gatt_message_status(m->result);

    switch (conn->data.scenario)
    {
#if (GATT_FEATURES & GATT_WRITE_SIGNED)
        case gatt_ms_signed_write_without_response:
            id = GATT_SIGNED_WRITE_WITHOUT_RESPONSE_CFM;
            break;
#endif

#if (GATT_FEATURES & GATT_WRITE)
        case gatt_ms_write_characteristic_value:
            id = GATT_WRITE_CHARACTERISTIC_VALUE_CFM;
            break;
#endif
            
        default:
            /* horror error which can't happen, ever. */
            id = 0;
            Panic();
    }
    
    gatt_message_send(conn, id);

    gattSetConnState(conn, NULL, gatt_ms_none);
}
#endif /* GATT_WRITE_COMMAND | GATT_WRITE_SIGNED | GATT_WRITE */

#if (GATT_FEATURES & GATT_WRITE_COMMAND)
/*************************************************************************
NAME    
    handleInternalWriteWithoutResponse
    
DESCRIPTION
    This function is handler for GATT_INTERNAL_WRITE_REQ internal message
    during WriteWithoutResponse.
    
RETURNS
    
*/
static void handleInternalWriteWithoutResponse(cid_map_t *conn,
                                       const GATT_INTERNAL_WRITE_REQ_T *m)
{
    gattSetConnState(conn, m->common.task, gatt_ms_write_without_response);
    gatt_write_cmd(m->common.cid, m->handle, m->size_value, m->value);
}

/*************************************************************************
NAME    
    gattHandleAttWriteCmdCfm
    
DESCRIPTION
    This function handles the Bluestack response for ATT_WRITE_CMD
    during WriteWithoutResponse.
    
RETURNS
    
*/
void gattHandleAttWriteCmdCfm(const ATT_WRITE_CMD_CFM_T *m)
{
    cid_map_t *conn = PanicNull(gattFindConn(m->cid)); /* never NULL */
    STASH(conn, stash, WRITE_WITHOUT_RESPONSE);
    MAKE_STASH(conn, stash, WRITE_WITHOUT_RESPONSE);
    
    stash->cid = m->cid;
    stash->status = gatt_message_status(m->result);
    stash->handle = (uint16) m->handle;

    gatt_message_send(conn, GATT_WRITE_WITHOUT_RESPONSE_CFM);

    gattSetConnState(conn, NULL, gatt_ms_none);   
}
#endif

#if (GATT_FEATURES & GATT_WRITE_LONG)
void GattWriteLongCharacteristicValueRequest(Task theAppTask,
                                             uint16 cid,
                                             uint16 handle,
                                             uint16 size_value,
                                             const uint8 *value)
{
    internalWriteReq(theAppTask, gatt_ms_write_long_characteristic_value, cid,
                     handle, 0 /* offs */, size_value, value);
}

/*************************************************************************
NAME    
    handleInternalWriteReqLong
    
DESCRIPTION
    This function is handler for GATT_INTERNAL_WRITE_REQ internal message
    during Write Long Characteristic Value.
    
RETURNS
    
*/
static void handleInternalWriteReqLong(cid_map_t *conn,
                                       const GATT_INTERNAL_WRITE_REQ_T *m)
{
    STASH(conn, stash, WRITE_LONG_CHARACTERISTIC_VALUE);
    gatt_write_reliable_t *data = &conn->data.req.write_reliable;
    
    gattSetConnState(conn, m->common.task, m->id);

    data->handle = m->handle;
    data->offs = m->offs;
    data->size_value = m->size_value;
    data->value = PanicUnlessMalloc(m->size_value);
    memmove(data->value, m->value, m->size_value);
    
    gatt_prepare_write_req(m->common.cid, m->handle, 0 /* offset */,
                           m->size_value, m->value);

    /* Create the response message */
    MAKE_STASH(conn, stash, WRITE_LONG_CHARACTERISTIC_VALUE);

    stash->cid = m->common.cid;
    stash->handle = m->handle;
    stash->status = gatt_status_success;
}

/*************************************************************************
NAME    
    handleAttPrepareWriteCfmLong
    
DESCRIPTION
    This function handles Bluestack response for ATT_PREPARE_WRITE_REQ
    during Long Characteristic Write.
    
RETURNS
    
*/
static void handleAttPrepareWriteCfmLong(cid_map_t *conn,
                                         const ATT_PREPARE_WRITE_CFM_T *m)
{
    gatt_write_reliable_t *data = &conn->data.req.write_reliable;

    if (m->result == ATT_RESULT_SUCCESS)
    {
        data->offs += MAX_PREPARE_SIZE(conn->mtu);

        /* send next packet */
        if (data->offs < data->size_value)
        {
            gatt_prepare_write_req(m->cid, data->handle, data->offs,
                                   data->size_value - data->offs,
                                   data->value + data->offs);
        }

        /* execute write */
        else
        {
            gatt_execute_write_req(m->cid, ATT_EXECUTE_WRITE);
        }
    }

    /* cancel ongoing write queue */
    else if (data->offs)
    {
        STASH(conn, stash, WRITE_LONG_CHARACTERISTIC_VALUE);

        stash->status = gatt_message_status(m->result);
        gatt_execute_write_req(m->cid, ATT_EXECUTE_CANCEL);
    }

    /* send failure */
    else
    {
        STASH(conn, stash, WRITE_LONG_CHARACTERISTIC_VALUE);

        stash->status = gatt_message_status(m->result);
        gatt_message_send(conn, GATT_WRITE_LONG_CHARACTERISTIC_VALUE_CFM);
        gattSetConnState(conn, NULL, gatt_ms_none);

        free(data->value);
    }
    
    /* free the data */
    free(VmGetPointerFromHandle(m->value));
}

/*************************************************************************
NAME    
    handleAttExecuteWriteCfmLong
    
DESCRIPTION
    This function handles Bluestack response for ATT_EXECUTE_WRITE_REQ
    during Write Long Characteristic.
    
RETURNS
    
*/
static void handleAttExecuteWriteCfmLong(cid_map_t *conn,
                                         const ATT_EXECUTE_WRITE_CFM_T *m)
{
    gatt_write_reliable_t *data = &conn->data.req.write_reliable;
    STASH(conn, stash, WRITE_LONG_CHARACTERISTIC_VALUE);

    if (stash->status == gatt_status_success)
    {
        /* update status to match final execute status */
        stash->status = gatt_message_status(m->result);
    }
    
    gatt_message_send(conn, GATT_WRITE_LONG_CHARACTERISTIC_VALUE_CFM);
    gattSetConnState(conn, NULL, gatt_ms_none);

    free(data->value);
}
#endif /* GATT_WRITE_LONG */

#if (GATT_FEATURES & GATT_WRITE_RELIABLE)
void GattReliableWritePrepareRequest(Task theAppTask,
                                     uint16 cid,
                                     uint16 handle,
                                     uint16 offset,
                                     uint16 size_value,
                                     const uint8 *value)
{
    internalWriteReq(theAppTask, gatt_ms_prepare_write, cid,
                     handle, offset, size_value, value);
}

/*************************************************************************
NAME    
    handleInternalWriteReqPrepare
    
DESCRIPTION
    This function is handler for GATT_INTERNAL_WRITE_REQ internal message
    during Reliable Write prepare.
    
RETURNS
    
*/
static void handleInternalWriteReqPrepare(cid_map_t *conn,
                                          const GATT_INTERNAL_WRITE_REQ_T *m)
{
    STASH(conn, stash, RELIABLE_WRITE_PREPARE);
    gatt_write_reliable_t *data = &conn->data.req.write_reliable;
    
    gattSetConnState(conn, m->common.task, m->id);

    data->handle = m->handle;
    data->offs = m->offs;
    data->size_value = m->size_value;
    data->value = PanicUnlessMalloc(m->size_value);
    memmove(data->value, m->value, m->size_value);
    
    gatt_prepare_write_req(m->common.cid, m->handle, m->offs,
                           m->size_value, m->value);

    /* Create the response message */
    MAKE_STASH(conn, stash, RELIABLE_WRITE_PREPARE);

    stash->cid = m->common.cid;
    stash->handle = m->handle;
    stash->status = gatt_status_success;
}

/*************************************************************************
NAME    
    handleAttPrepareWriteCfmPrepare
    
DESCRIPTION
    This function handles Bluestack response for ATT_PREPARE_WRITE_REQ
    during Prepare Write.
    
RETURNS
    
*/
static void handleAttPrepareWriteCfmPrepare(cid_map_t *conn,
                                            const ATT_PREPARE_WRITE_CFM_T *m)
{
    gatt_write_reliable_t *data = &conn->data.req.write_reliable;
    uint8 *vmptr;
    STASH(conn, stash, RELIABLE_WRITE_PREPARE);

    /* convert the data pointer */
    vmptr = VmGetPointerFromHandle(m->value);
    
    if (m->result == ATT_RESULT_SUCCESS &&
        /* check that the value matches */
        (m->offset != data->offs ||
         m->size_value != data->size_value ||
         memcmp8(vmptr, data->value, m->size_value)))
    {
        stash->status = gatt_status_value_mismatch;
    }
    else
    {
        stash->status = gatt_message_status(m->result);
    }

    /* send the response */
    gatt_message_send(conn, GATT_RELIABLE_WRITE_PREPARE_CFM);
    gattSetConnState(conn, NULL, gatt_ms_none);

    /* free the stored data */
    free(data->value);

    /* free the data pointer */
    free(vmptr);
}

/****************************************************************************
NAME
    sendReliableWriteExecuteCfm

DESCRIPTION
    Send the GATT_RELIABLE_WRITE_EXECUTE_CFM message.

RETURNS

*/
static void sendReliableWriteExecuteCfm(Task task,
                                        uint16 cid,
                                        uint16 handle, 
                                        gatt_status_t status)
{
    MAKE_GATT_MESSAGE(GATT_RELIABLE_WRITE_EXECUTE_CFM);

    message->cid = cid;
    message->handle = handle;
    message->status = gatt_message_status(status);

    MessageSend(task, GATT_RELIABLE_WRITE_EXECUTE_CFM, message);
}

void GattReliableWriteExecuteRequest(Task theAppTask,
                                     uint16 cid,
                                     bool execute)
{
    cid_map_t *conn = gattFindConn(cid);

    if (conn && !conn->closing)
    {
        MAKE_GATT_MESSAGE(GATT_INTERNAL_EXECUTE_WRITE_REQ);

        message->common.task = theAppTask;
        message->common.cid = cid;
        message->flags = execute ? ATT_EXECUTE_WRITE : ATT_EXECUTE_CANCEL;

        MessageSendConditionallyOnTask(
            gattGetTask(),
            GATT_INTERNAL_EXECUTE_WRITE_REQ,
            message,
            &conn->data.app);
    }
    else
    {
        sendReliableWriteExecuteCfm(theAppTask, cid, 0 /* handle */,
                                    gatt_status_invalid_cid);
    }
}

/*************************************************************************
NAME    
    gattHandleInternalExecuteWriteReq
    
DESCRIPTION
    This function is handler for GATT_INTERNAL_EXECUTE_WRITE_REQ internal
    message.
    
RETURNS
    
*/
void gattHandleInternalExecuteWriteReq(const GATT_INTERNAL_EXECUTE_WRITE_REQ_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->common.cid); /* never NULL */
    
    gattSetConnState(conn, m->common.task, gatt_ms_execute_write);
    gatt_execute_write_req(m->common.cid, m->flags);
}

/*************************************************************************
NAME    
    handleAttExecuteWriteCfmExecute
    
DESCRIPTION
    This function handles Bluestack response for ATT_EXECUTE_WRITE_REQ
    during Execute Write.
    
RETURNS
    
*/
static void handleAttExecuteWriteCfmExecute(cid_map_t *conn,
                                            const ATT_EXECUTE_WRITE_CFM_T *m)
{
    sendReliableWriteExecuteCfm(conn->data.app, m->cid, m->handle, m->result);
    gattSetConnState(conn, NULL, gatt_ms_none);
}
#endif /* GATT_WRITE_RELIABLE */

#if (GATT_FEATURES & (GATT_WRITE_LONG | \
                      GATT_WRITE_RELIABLE))
/*************************************************************************
NAME    
    gattHandleAttPrepareWriteCfm
    
DESCRIPTION
    This function handles Bluestack response for ATT_PREPARE_WRITE_REQ.
    
RETURNS
    
*/
void gattHandleAttPrepareWriteCfm(const ATT_PREPARE_WRITE_CFM_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->cid); /* never NULL */

    switch (conn->data.scenario)
    {
#if (GATT_FEATURES & GATT_WRITE_LONG)
        case gatt_ms_write_long_characteristic_value:
            handleAttPrepareWriteCfmLong(conn, m);
            return;
#endif
            
#if (GATT_FEATURES & GATT_WRITE_RELIABLE)
        case gatt_ms_prepare_write:
            handleAttPrepareWriteCfmPrepare(conn, m);
            return;
#endif
            
        default:
            /* horror error which can't happen, ever. */
            Panic();            
    }
}

/*************************************************************************
NAME    
    gattHandleAttExecuteWriteCfm
    
DESCRIPTION
    This function handles Bluestack response for ATT_EXECUTE_WRITE_REQ.
    
RETURNS
    
*/
void gattHandleAttExecuteWriteCfm(const ATT_EXECUTE_WRITE_CFM_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->cid); /* never NULL */

    switch (conn->data.scenario)
    {
#if (GATT_FEATURES & GATT_WRITE_LONG)
        case gatt_ms_write_long_characteristic_value:
            handleAttExecuteWriteCfmLong(conn, m);
            return;
#endif
            
#if (GATT_FEATURES & GATT_WRITE_RELIABLE)
        case gatt_ms_execute_write:
            handleAttExecuteWriteCfmExecute(conn, m);
            return;
#endif
            
        default:
            /* horror error which can't happen, ever. */
            Panic();            
    }
}
#endif /* GATT_WRITE_LONG | GATT_WRITE_RELIABLE */

/*************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_descriptor_discovery.c

DESCRIPTION
    Functions to handle GATT Descriptor Discovery sub-procedures.

    This module implements support for the following GATT sub-procedures:
    - Discover All Characteristic Descriptors

NOTES
*/

#include "gatt_private.h"

#include <stdlib.h>
#include <string.h>
#include <vm.h>

#if (GATT_FEATURES & GATT_DISC_ALL_DESCRIPTORS)
void GattDiscoverAllCharacteristicDescriptorsRequest(Task theAppTask,
                                                     uint16 cid,
                                                     uint16 start,
                                                     uint16 end)
{
    cid_map_t *conn = gattFindConn(cid);

    if (conn && !conn->closing)
    {
        MAKE_GATT_MESSAGE(GATT_INTERNAL_DISCOVER_ALL_DESCRIPTORS_REQ);

        message->common.task = theAppTask;
        message->common.cid = cid;
        message->start = start;
        message->end = end;

        MessageSendConditionallyOnTask(
            gattGetTask(),
            GATT_INTERNAL_DISCOVER_ALL_DESCRIPTORS_REQ,
            message,
            &conn->data.app);
    }
    else
    {
        MAKE_GATT_MESSAGE(GATT_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM);

        memset(message, 0,
               sizeof(GATT_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T));
        message->cid = cid;
        message->status = gatt_status_invalid_cid;
        
        MessageSend(theAppTask,
                    GATT_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM, message);
    }
}

/*************************************************************************
NAME    
    gattHandleInternalDiscoverAllDescriptorsReq
    
DESCRIPTION
    This function is handler for GATT_DISCOVER_ALL_DESCRIPTORS_REQ
    internal message.
    
RETURNS
    
*/
void gattHandleInternalDiscoverAllDescriptorsReq(
    const GATT_INTERNAL_DISCOVER_ALL_DESCRIPTORS_REQ_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->common.cid); /* never NULL */
    
    gattSetConnState(conn, m->common.task, gatt_ms_discover_all_descriptors);

    conn->data.req.discover_all_descriptors.start = m->start;
    conn->data.req.discover_all_descriptors.end = m->end;
    
    gatt_find_info_req(m->common.cid,
                       conn->data.req.discover_all_descriptors.start,
                       conn->data.req.discover_all_descriptors.end);
}

/*************************************************************************
NAME    
    gattHandleAttFindInfoCfm
    
DESCRIPTION
    This function handles Bluestack response for ATT_FIND_INFO_REQ.
    
RETURNS
    
*/
void gattHandleAttFindInfoCfm(const ATT_FIND_INFO_CFM_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->cid); /* never NULL */
    STASH(conn, stash, DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS);
    bool more;
    bool send;

    if (!gatt_message_prepare(
            conn,
            &more, &send, m->result,
            m->handle >= conn->data.req.discover_all_descriptors.end,
            stash ? &stash->more_to_come : NULL,
            GATT_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM))
    {
        return;
    }

    /* Create the message */
    MAKE_STASH(conn, stash, DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS);

    stash->cid = m->cid;
    stash->handle = m->handle;
    stash->uuid_type = (gatt_uuid_type_t)m->uuid_type;
    memmove(stash->uuid, m->uuid, GATT_UUID_SIZE);
    stash->more_to_come = more;
    stash->status = gatt_message_status(m->result);
    
    /* schedule new round for getting more information */
    if (!send)
    {
        gatt_find_info_req(m->cid,
                           m->handle + 1,
                           conn->data.req.discover_all_descriptors.end);
    }
    
    /* only sent the message if we are not waiting for more information */
    else
    {
        gatt_message_send(conn,
                          GATT_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM);

        if (!more)
        {
            gattSetConnState(conn, NULL, gatt_ms_none);
        }
    }
}

#endif /* GATT_DISC_ALL_DESCRIPTORS */

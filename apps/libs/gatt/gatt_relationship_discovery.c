/*************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_relationship_discovery.c

DESCRIPTION
    Functions to handle GATT Relationship Discovery sub-procedures.

    This module implements support for the following GATT sub-procedures:
    - Find Included Services

NOTES
*/

#include "gatt_private.h"
#include <stdlib.h>
#include <string.h>
#include <vm.h>

#if (GATT_FEATURES & GATT_RELATIONSHIP_DISCOVERY)

static const uint32 uuid_include = ATT_UUID_INCLUDE;

void GattFindIncludedServicesRequest(Task theAppTask,
                                     uint16 cid,
                                     uint16 start,
                                     uint16 end)
{
    cid_map_t *conn = gattFindConn(cid);
    
    if (conn && !conn->closing)
    {
        MAKE_GATT_MESSAGE(GATT_INTERNAL_FIND_INCLUDES_REQ);

        message->common.task = theAppTask;
        message->common.cid = cid;
        message->start = start;
        message->end = end;
        
        MessageSendConditionallyOnTask(
            gattGetTask(),
            GATT_INTERNAL_FIND_INCLUDES_REQ,
            message,
            &conn->data.app);
    }
    else
    {
        MAKE_GATT_MESSAGE(GATT_FIND_INCLUDED_SERVICES_CFM);

        memset(message, 0, sizeof(GATT_FIND_INCLUDED_SERVICES_CFM_T));
        message->cid = cid;
        message->status = gatt_status_invalid_cid;
        
        MessageSend(theAppTask, GATT_FIND_INCLUDED_SERVICES_CFM, message);
    }
}

/*************************************************************************
NAME    
    gattHandleInternalFindIncludesReq
    
DESCRIPTION
    This function is handler for ATT_FIND_INCLUDES_REQ internal message.
    
RETURNS
    
*/
void gattHandleInternalFindIncludesReq(const GATT_INTERNAL_FIND_INCLUDES_REQ_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->common.cid); /* never NULL */
    
    gattSetConnState(conn, m->common.task, gatt_ms_find_includes);

    conn->data.req.find_includes.start = m->start;
    conn->data.req.find_includes.end = m->end;
    
    gatt_read_by_type_req(m->common.cid,
                          conn->data.req.find_includes.start,
                          conn->data.req.find_includes.end,
                          gatt_uuid16, &uuid_include);
}

/*************************************************************************
NAME    
    gattHandleAttReadByTypeCfmInclude
    
DESCRIPTION
    This function handles Bluestack response for ATT_READ_BY_TYPE_REQ
    during Relationship Discovery.
    
RETURNS
    
*/
void gattHandleAttReadByTypeCfmInclude(const ATT_READ_BY_TYPE_CFM_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->cid); /* never NULL */
    STASH(conn, stash, FIND_INCLUDED_SERVICES);
    uint8 *data;
    uint8 *p;
    bool more;
    bool send;

    p = data = VmGetPointerFromHandle(m->value);

    if (!gatt_message_prepare(conn,
                              &more, &send, m->result,
                              m->handle >= conn->data.req.find_includes.end,
                              stash ? &stash->more_to_come : NULL,
                              GATT_FIND_INCLUDED_SERVICES_CFM))
    {
        free(data);
        return;
    }

    /* Create the message */
    MAKE_STASH(conn, stash, FIND_INCLUDED_SERVICES);

    stash->cid = m->cid;
    stash->more_to_come = more;
    stash->status = gatt_message_status(m->result);

    /* read values from the response */
    if (stash->status == ATT_RESULT_SUCCESS)
    {
        stash->handle = gatt_read16(&p);
        stash->end = gatt_read16(&p);

        gatt_get_att_uuid(stash->uuid, &stash->uuid_type,
                          m->size_value - 4, p);

        if ((stash->uuid_type == gatt_uuid16)
                || (stash->uuid_type == gatt_uuid32))
        {
            /* schedule new round for getting more information */
            if (!send)
                gatt_read_by_type_req(
                    m->cid,
                    m->handle + 1,
                    conn->data.req.find_includes.end,
                    stash->uuid_type,
                    &uuid_include
                    );
        }
        else if (m->result == ATT_RESULT_SUCCESS_MORE)
        {
            /* We are seriously doomed!
             * Remote has more than one 128-bit include (default MTU
             * can have 5, and we'll have to queue all of those before
             * we can proceed reading the UUIDs. */
            /* TODO: B-107496: implement a proper way of handling this, for
             * now just send the message without UUID */
        }
        else
        {
            /* read 128-bit UUID */
            gatt_read_req(m->cid, stash->handle);

            /* update starting pointer */
            conn->data.req.find_includes.start = m->handle + 1;
            send = FALSE; 
        }
    }
    
    /* only sent the message if we are not waiting for more information */
    if (send)
    {
        gatt_message_send(conn, GATT_FIND_INCLUDED_SERVICES_CFM);

        if (!more)
        {
            gattSetConnState(conn, NULL, gatt_ms_none);
        }
    }
    
    free(data);
}

/*************************************************************************
NAME    
    gattHandleAttReadCfmInclude
    
DESCRIPTION
    This function handles Bluestack response for ATT_READ_REQ during
    Relationship Discovery.
    
RETURNS
    
*/
void gattHandleAttReadCfmInclude(const ATT_READ_CFM_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->cid); /* never NULL */
    STASH(conn, stash, FIND_INCLUDED_SERVICES);
    uint8 *data = VmGetPointerFromHandle(m->value);

    if (stash)
    {
        gatt_get_att_uuid(stash->uuid, &stash->uuid_type, m->size_value, data);

        if (stash->more_to_come)
        {
            gatt_read_by_type_req(m->cid,
                                  conn->data.req.find_includes.start,
                                  conn->data.req.find_includes.end,
                                  ATT_UUID16, &uuid_include);
        }
        else
        {
            gatt_message_send(conn, GATT_FIND_INCLUDED_SERVICES_CFM);
            gattSetConnState(conn, NULL, gatt_ms_none);
        }        
    }
    
    free(data);
}

#endif /* GATT_FEATURE_RELATIONSHIP_DISCOVERY */

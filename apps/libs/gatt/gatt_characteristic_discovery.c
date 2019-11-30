/*************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_characteristic_discovery.c

DESCRIPTION
    Functions to handle GATT Characteristic Discovery sub-procedures.

    This module implements support for the following GATT sub-procedures:
    - Discover All Characteristics of a Service
    - Discover Characteristics by UUID

NOTES
*/

#include "gatt_private.h"

#include <stdlib.h>
#include <string.h>
#include <vm.h>

#if (GATT_FEATURES & GATT_CHARACTERISTIC_DISCOVERY)

static const uint32 uuid_characteristic = ATT_UUID_CHARACTERISTIC;

void GattDiscoverAllCharacteristicsRequest(Task theAppTask,
                                           uint16 cid,
                                           uint16 start,
                                           uint16 end)
{
    cid_map_t *conn = gattFindConn(cid);

    if (conn && !conn->closing)
    {
        MAKE_GATT_MESSAGE(GATT_INTERNAL_DISCOVER_ALL_CHARACTERISTICS_REQ);

        message->common.task = theAppTask;
        message->common.cid = cid;
        message->start = start;
        message->end = end;

        MessageSendConditionallyOnTask(
            gattGetTask(),
            GATT_INTERNAL_DISCOVER_ALL_CHARACTERISTICS_REQ,
            message,
            &conn->data.app);
    }
    else
    {
        MAKE_GATT_MESSAGE(GATT_DISCOVER_ALL_CHARACTERISTICS_CFM);

        memset(message, 0, sizeof(GATT_DISCOVER_ALL_CHARACTERISTICS_CFM_T));
        message->cid = cid;
        message->status = gatt_status_invalid_cid;
        
        MessageSend(theAppTask,
                    GATT_DISCOVER_ALL_CHARACTERISTICS_CFM,
                    message);
    }
}

/*************************************************************************
NAME    
    gattHandleGattDiscoverAllCharacteristicsReq
    
DESCRIPTION
    This function is handler for GATT_DISCOVER_ALL_CHARACTERISTICS_REQ
    internal message.
    
RETURNS
    
*/
void gattHandleGattDiscoverAllCharacteristicsReq(
    const GATT_INTERNAL_DISCOVER_ALL_CHARACTERISTICS_REQ_T *m)
{
    cid_map_t *conn = gattFindConn(m->common.cid);
    
    gattSetConnState(conn, m->common.task,
                     gatt_ms_discover_all_characteristics);

    conn->data.req.discover_all_characteristics.start = m->start;
    conn->data.req.discover_all_characteristics.end = m->end;
    
    gatt_read_by_type_req(m->common.cid,
                          conn->data.req.discover_all_characteristics.start,
                          conn->data.req.discover_all_characteristics.end,
                          gatt_uuid16, &uuid_characteristic);
}

/*************************************************************************
NAME    
    gattHandleAttReadByTypeCfmCharacteristic
    
DESCRIPTION
    This function handles Bluestack response for ATT_READ_BY_TYPE_REQ
    during Relationship Discovery.
    
RETURNS
    
*/
void gattHandleAttReadByTypeCfmCharacteristic(const ATT_READ_BY_TYPE_CFM_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->cid); /* never NULL */
    STASH(conn, stash, DISCOVER_ALL_CHARACTERISTICS);
    uint8 *data;
    uint8 *p;
    bool more;
    bool send;

    p = data = VmGetPointerFromHandle(m->value);

    if (!gatt_message_prepare(conn,
                              &more, &send, m->result,
                              m->handle >= conn->data.req.discover_all_characteristics.end,
                              stash ? &stash->more_to_come : NULL,
                              GATT_DISCOVER_ALL_CHARACTERISTICS_CFM))
    {
        free(data);
        return;
    }

    /* Create the message */
    MAKE_STASH(conn, stash, DISCOVER_ALL_CHARACTERISTICS);

    stash->cid = m->cid;
    stash->more_to_come = more;
    stash->status = gatt_message_status(m->result);
    stash->declaration = m->handle;

    /* read values from the response */
    if (stash->status == ATT_RESULT_SUCCESS)
    {
        stash->properties = gatt_read8(&p);
        stash->handle = gatt_read16(&p);

        gatt_get_att_uuid(stash->uuid, &stash->uuid_type,
                          m->size_value - 3, p);

    }
    
    /* schedule new round for getting more information */
    if (!send)
    {
        gatt_read_by_type_req(m->cid,
                              m->handle + 1,
                              conn->data.req.discover_all_characteristics.end,
                              gatt_uuid16, &uuid_characteristic);
    }
    
    /* only sent the message if we are not waiting for more information */
    else
    {
        gatt_message_send(conn, GATT_DISCOVER_ALL_CHARACTERISTICS_CFM);

        if (!more)
        {
            gattSetConnState(conn, NULL, gatt_ms_none);
        }
    }
    
    free(data);
}

#endif /* GATT_FEATURE_RELATIONSHIP_DISCOVERY */

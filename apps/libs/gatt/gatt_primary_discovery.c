/*************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_primary_discovery.c

DESCRIPTION
    Functions to handle GATT Primary Service Discovery sub-procedures.

    This module implements support for the following GATT sub-procedures:
    - Discover All Primary Services
    - Discover Primary Service by Service UUID

NOTES
*/

#include "gatt_private.h"
#include <service.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <vm.h>

#if (GATT_FEATURES & GATT_PRIMARY_DISCOVERY)

static const uint32 uuid_primary_service = ATT_UUID_PRI_SERVICE;

/* search pattern for sdp search
 * DataElSeq - See Core V4.0 SDP Specification P221 Sec 3.2 & 3.3 */
static const uint8 sdp_search[] = {
    0x35, 0x03,         /* DataElHeader(Type:0b00110 Size:0b101), 3 bytes */
    0x19, 0x00, 0x07    /* UUID16(Type:0b00011 Size:0b001), ATT 0x0007 */
};

/* search pattern for sdp attributes
 * DataElSeq - See Core V4.0 SDP Specification P221 Sec 3.2 & 3.3 */
static const uint8 sdp_search_attr[] = {
    0x35, 0x06,         /* DataElHeader(Type:0b00110 Size:0b101), 6 bytes */
    0x09, 0x00, 0x04,   /* UINT16(Type:0b00001 Size:0b001),
                                            ProtocolDescriptorList 0x0004 */
    0x09, 0x00, 0x01    /* UINT16(Type:0b00001 Size:0b001),
                                            ServiceClassIdList 0x0001 */
};

#define SDC_MAX_ATTRIBUTES      20
/* DataElement Header for UUID16 (Type:0b00011 Size:0b001)
 * See Core V4.0 SDP Specification P221 Sec 3.2 & 3.3 */
#define SDP_TYPE_UUID16         0x19
/* DataElement Header for UUID16 (Type:0b00011 Size:0b010)
 * See Core V4.0 SDP Specification P221 Sec 3.2 & 3.3 */
#define SDP_TYPE_UUID32         0x1A
/* DataElement Header for UUID16 (Type:0b00011 Size:0b100)
 * See Core V4.0 SDP Specification P221 Sec 3.2 & 3.3 */
#define SDP_TYPE_UUID128        0x1c
#define SDP_SEARCH_SIZE_OFFS    1       /* offset for sequence length */

/*************************************************************************
NAME    
    gattHandleClSdpServiceSearchAttributeCfm
    
DESCRIPTION
    This function handles Connection library response for SDP Service
    Search Attribute request.
    
RETURNS
    
*/
void gattHandleClSdpServiceSearchAttributeCfm(
    const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *m)
{
    uint16 id = 0;
    cid_map_t *conn = (cid_map_t *)PanicNull(gattFindConnAddr(&m->bd_addr));/* never NULL */

    /* Use GATT_DISCOVER_ALL_BREDR_SERVICES_CFM to populate the response.
     * The message is mangled later to it's correct form based on what
     * procedure we were running. */
    MAKE_GATT_MESSAGE(GATT_DISCOVER_ALL_BREDR_SERVICES_CFM);

    message->bd_addr = m->bd_addr;
    message->more_to_come = m->more_to_come;
    message->status = gatt_status_attr_not_found;
    
    if (m->status == sdp_response_success)
    {
        ServiceDataType type;
        Region sdp;
        Region val;

        /* If we have correctly formatted response it is:
         *
         *  0 UINT16 0x0004 (ProtocolDescriptorList)
         *  1 elSeq
         *  2     elSeq
         *  3         UUID   0x0100 (L2CAP)
         *  4         UINT16 0x001f (ATT PSM)
         *  5     elSeq
         *  6         UUID   0x0007 (ATT)
         *  7         UINT16 start_handle
         *  8         UINT16 end_handle
         *  9 UINT16 0x0001 (ServiceClassIDList)
         * 10 elSeq
         * 11     UUID service_uuid
         */
        const ServiceDataType expect[] = {
            sdtUnsignedInteger, sdtSequence,
            sdtSequence, sdtUUID, sdtUnsignedInteger,
            sdtSequence, sdtUUID, sdtUnsignedInteger, sdtUnsignedInteger,

            sdtUnsignedInteger, sdtSequence, sdtUUID };
        uint16 i;
        
        sdp.begin = m->attributes;
        sdp.end = m->attributes + m->size_attributes;

        /* assume the response is valid and do only minimal verify */
        for (i = 0; i < sizeof(expect) / sizeof(ServiceDataType); i++)
        {
            if (   !ServiceGetValue(&sdp, &type, &val) 
                || type != (ServiceDataType)expect[i])
            {
                break;
            }

            /* take values we are interested in */
            switch (i)
            {
                case  7:
                    message->handle = RegionReadUint16(&val);
                    break;
                    
                case  8: 
                    message->end = RegionReadUint16(&val);
                    break;
                    
                case 11:
                    gatt_get_sdp_uuid(message->uuid,
                                      &message->uuid_type,
                                      &val);
                    message->status = gatt_status_success;
            }

            /* go inside the sequence */
            if (type == sdtSequence) sdp.begin = val.begin;
        }        
    }

    /* in case of error reset values */
    else 
    {
        message->handle = 0;
        message->end = 0;
        message->uuid_type = gatt_uuid_none;
        memset(message->uuid, 0, GATT_UUID_SIZE);
    }

    switch (conn->data.scenario)
    {
#if (GATT_FEATURES & GATT_DISC_ALL_PRIMARY_SERVICES)
        case gatt_ms_discover_all_bredr_services:
            id = GATT_DISCOVER_ALL_BREDR_SERVICES_CFM;
            break;
#endif

#if (GATT_FEATURES & GATT_DISC_PRIMARY_SERVICE)
        case gatt_ms_discover_bredr_service:
            id = GATT_DISCOVER_BREDR_SERVICE_CFM;
            break;
#endif

        default:
            /* horror error which can't happen, ever. */
            id = 0;
            Panic();            
    }
    
    MessageSend(conn->data.app, id, message);
    
    if (!m->more_to_come)
    {
        /* delete temporary connection instance */
        if (conn->data.scenario == gatt_ms_discover_all_bredr_services ||
            conn->data.scenario == gatt_ms_discover_bredr_service)
            gattDeleteCid(conn->cid);

        gattSetConnState(conn, NULL, gatt_ms_none);
    }
}

/*************************************************************************
NAME    
    get_temp_conn
    
DESCRIPTION
    This function is used to create a temporary connection instance for
    SDP searches when there is no existing connection.
    
RETURNS
    connection pointer or NULL
*/
static cid_map_t *get_temp_conn(Task task)
{
    gattState *gatt = (gattState*)gattGetTask();
    uint16 cid = 0x003f; /* maximum fixed cid */
    uint16 i;
    
    /* create special connection instance for the SDP search */
    for (i = 1; i < MAX_ATT_CONNECTIONS; i++)
    {
        if (gatt->u.cid_map[i].cid < cid)
        {
            cid = gatt->u.cid_map[i].cid - 1;
        }
    }

    return gattAddCid(cid, task);
}

#endif

#if (GATT_FEATURES & GATT_DISC_ALL_PRIMARY_SERVICES)

void GattDiscoverAllPrimaryServicesRequest(Task theAppTask, uint16 cid)
{
    cid_map_t *conn = gattFindConn(cid);

    if (conn && !conn->closing)
    {
        MAKE_GATT_MESSAGE(GATT_INTERNAL_DISCOVER_ALL_PRIMARY_SERVICES_REQ);

        message->common.task = theAppTask;
        message->common.cid = cid;

        MessageSendConditionallyOnTask(
            gattGetTask(),
            GATT_INTERNAL_DISCOVER_ALL_PRIMARY_SERVICES_REQ,
            message,
            &conn->data.app);
    }
    else
    {
        MAKE_GATT_MESSAGE(GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM);

        memset(message, 0, sizeof(GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM_T));
        message->cid = cid;
        message->status = gatt_status_invalid_cid;
        
        MessageSend(theAppTask,
                    GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM,
                    message);
    }
}

/*************************************************************************
NAME    
    gattHandleInternalDiscoverAllPrimaryServices_req
    
DESCRIPTION
    This function is handler for
    GATT_INTERNAL_DISCOVER_ALL_PRIMARY_SERVICES_REQ internal message.
    
RETURNS
    
*/
void gattHandleInternalDiscoverAllPrimaryServicesReq(
    const GATT_INTERNAL_DISCOVER_ALL_PRIMARY_SERVICES_REQ_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->common.cid); /* never NULL */
        
    gattSetConnState(conn,
                     m->common.task,
                     gatt_ms_discover_all_primary_services);


    gatt_read_by_group_type_req(
            m->common.cid,
            GATT_HANDLE_MIN,
            GATT_HANDLE_MAX,
            gatt_uuid16,
            &uuid_primary_service
            );
}

/*************************************************************************
NAME    
    gattHandleAttReadByGroupTypeCfm
    
DESCRIPTION
    This function handles Bluestack response for
    ATT_READ_BY_GROUP_TYPE_REQ.
    
RETURNS
    
*/
void gattHandleAttReadByGroupTypeCfm(const ATT_READ_BY_GROUP_TYPE_CFM_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->cid); /* never NULL */
    STASH(conn, stash, DISCOVER_ALL_PRIMARY_SERVICES);
    uint8 *data;
    bool more;
    bool send;

    data = VmGetPointerFromHandle(m->value);

    if (!gatt_message_prepare(conn,
                              &more, &send, m->result,
                              m->end == GATT_HANDLE_MAX,
                              stash ? &stash->more_to_come : NULL,
                              GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM))
    {
        free(data);
        return;
    }

    /* Create the message */
    MAKE_STASH(conn, stash, DISCOVER_ALL_PRIMARY_SERVICES);
    stash->cid = m->cid;
    stash->handle = m->handle;
    stash->end = m->end;
    stash->more_to_come = more;
    stash->status = gatt_message_status(m->result);
    gatt_get_att_uuid(stash->uuid, &stash->uuid_type, m->size_value, data);

    /* schedule new round for getting more information */
    if (!send)
    {
        gatt_read_by_group_type_req(
                m->cid,
                m->end + 1,
                GATT_HANDLE_MAX,
                gatt_uuid16, 
                &uuid_primary_service
                );
    }
    
    /* only sent the message if we are not waiting for more information */
    else 
    {
        gatt_message_send(conn, GATT_DISCOVER_ALL_PRIMARY_SERVICES_CFM);

        if (!more)
        {
            gattSetConnState(conn, NULL, gatt_ms_none);
        }
    }
    
    free(data);
}

#endif /* DISC_ALL_PRIMARY_SERVICES */

#if (GATT_FEATURES & GATT_DISC_PRIMARY_SERVICE)

void GattDiscoverPrimaryServiceRequest(Task theAppTask,
                                       uint16 cid,
                                       gatt_uuid_type_t uuid_type,
                                       const gatt_uuid_t *uuid)
{
    cid_map_t *conn = gattFindConn(cid);
    gatt_status_t err;

    /* check that cid exists */
    if (!conn || conn->closing)
    {
        err = gatt_status_invalid_cid;
    }
    else if ((uuid_type < gatt_uuid16) || (uuid_type > gatt_uuid128))
    {
        err = gatt_status_invalid_uuid;
    }
    else
    {
        MAKE_GATT_MESSAGE(GATT_INTERNAL_DISCOVER_PRIMARY_SERVICE_REQ);

        /* send internal request */
        
        message->common.task = theAppTask;
        message->common.cid = cid;
        message->uuid_type = uuid_type;
        memmove(message->uuid, uuid, GATT_UUID_SIZE);
    
        MessageSendConditionallyOnTask(
            gattGetTask(),
            GATT_INTERNAL_DISCOVER_PRIMARY_SERVICE_REQ,
            message,
            &conn->data.app);

        return;
        
    }
    
    /* send error */
    {
        MAKE_GATT_MESSAGE(GATT_DISCOVER_PRIMARY_SERVICE_CFM);

        memset(message, 0, sizeof(GATT_DISCOVER_PRIMARY_SERVICE_CFM_T));
        message->cid = cid;
        message->status = err;

        MessageSend(theAppTask, GATT_DISCOVER_PRIMARY_SERVICE_CFM, message);
    }
}

/*************************************************************************
NAME    
    search_sdp_service
    
DESCRIPTION
    This function is used to initiate SDP search request for a specific
    service UUID.
    
RETURNS
    
*/
static void search_sdp_service(const bdaddr *addr,
                               gatt_uuid_type_t uuid_type,
                               const gatt_uuid_t *uuidc)
{
    uint16 len;
    uint16 i;
    uint8 *search;
    uint8 *p;
    gatt_uuid_t uuid[4];

    memmove(uuid, uuidc, GATT_UUID_SIZE);
    
    /* allocate maximum possible data we need */
    search = PanicUnlessMalloc(sizeof(sdp_search) + 1 + 16);
    memmove(search, sdp_search, sizeof(sdp_search));
    p = search + sizeof(sdp_search);

    if (uuid_type == gatt_uuid16)
    {
        len = 1 + 2; /* type + UUID16 */
        *p++ = SDP_TYPE_UUID16;
        uuid[0] <<= 16; /* move to upper word for later byte copy*/
    }
    else if (uuid_type == gatt_uuid32)
    {
        len = 1 + 4; /* type + UUID32 */
        *p++ = SDP_TYPE_UUID32;
    }
    else
    {
        len = 1 + 16; /* type + UUID128 */
        *p++ = SDP_TYPE_UUID128;
    }

    /* copy the UUID */
    for (i = 0; i < 4; i++)
    {
        *p++ = (uuid[i] >> 24) & 0xff;
        *p++ = (uuid[i] >> 16) & 0xff;
        *p++ = (uuid[i] >>  8) & 0xff;
        *p++ = (uuid[i]      ) & 0xff;            
    }
        
    search[SDP_SEARCH_SIZE_OFFS] += len;
        
    ConnectionSdpServiceSearchAttributeRequest(
        gattGetTask(), addr, SDC_MAX_ATTRIBUTES,
        sizeof(sdp_search) + len, search,
        sizeof(sdp_search_attr), sdp_search_attr);
}

/*************************************************************************
NAME    
    gattHandleInternalDiscoverPrimaryServiceReq
    
DESCRIPTION
    This function is handler for GATT_INTERNAL_DISCOVER_PRIMARY_SERVICE_REQ
    internal message.
    
RETURNS
    
*/
void gattHandleInternalDiscoverPrimaryServiceReq(
    const GATT_INTERNAL_DISCOVER_PRIMARY_SERVICE_REQ_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->common.cid);
    
    gattSetConnState(conn, m->common.task, gatt_ms_discover_primary_service);

    conn->data.req.discover_primary_service.uuid_type = m->uuid_type;
    memmove(conn->data.req.discover_primary_service.uuid,
            m->uuid,
            GATT_UUID_SIZE);
    
    gatt_find_by_type_value_req(
        m->common.cid, GATT_HANDLE_MIN, GATT_HANDLE_MAX,
        ATT_UUID_PRI_SERVICE,
        conn->data.req.discover_primary_service.uuid_type,
        conn->data.req.discover_primary_service.uuid);
}

/*************************************************************************
NAME    
    gattHandleAttFindByTypeValueCfm
    
DESCRIPTION
    This function handles Bluestack response for
    ATT_FIND_BY_TYPE_VALUE_REQ.
    
RETURNS
    
*/
void gattHandleAttFindByTypeValueCfm(const ATT_FIND_BY_TYPE_VALUE_CFM_T *m)
{
    cid_map_t *conn = gattFindConnOrPanic(m->cid); /* never NULL */
    STASH(conn, stash, DISCOVER_PRIMARY_SERVICE);
    
    bool more = FALSE;
    bool send = TRUE; /* create and send a message */

    if (!gatt_message_prepare(conn,
                              &more, &send, m->result,
                              m->end == GATT_HANDLE_MAX,
                              stash ? &stash->more_to_come : NULL,
                              GATT_DISCOVER_PRIMARY_SERVICE_CFM))
    {
        gattSetConnState(conn, NULL, gatt_ms_none);
        return;
    }
    
    /* Create the message */
    MAKE_STASH(conn, stash, DISCOVER_PRIMARY_SERVICE);
    stash->cid = m->cid;
    stash->handle = m->handle;
    stash->end = m->end;
    stash->uuid_type = conn->data.req.discover_primary_service.uuid_type;
    memmove(stash->uuid,
            conn->data.req.discover_primary_service.uuid,
            GATT_UUID_SIZE);
    stash->more_to_come = more;
    stash->status = gatt_message_status(m->result);

    /* schedule new round for getting more information */
    if (!send)
    {
        gatt_find_by_type_value_req(
            m->cid, m->end + 1, GATT_HANDLE_MAX,
            ATT_UUID_PRI_SERVICE,
            conn->data.req.discover_primary_service.uuid_type,
            conn->data.req.discover_primary_service.uuid);
    }
    
    /* only sent the message if we are not waiting for more information */
    else
    {
        gatt_message_send(conn, GATT_DISCOVER_PRIMARY_SERVICE_CFM);

        if (!more)
        {
            gattSetConnState(conn, NULL, gatt_ms_none);
        }
    }
}

#endif /* DISC_PRIMARY_SERVICE */

#if (GATT_FEATURES & GATT_DISC_ALL_PRIMARY_SERVICES)

void GattDiscoverAllBrEdrServicesRequest(Task theAppTask, bdaddr *bd_addr)
{
    cid_map_t *conn = get_temp_conn(theAppTask);

    if (!conn)
    {
        MAKE_GATT_MESSAGE(GATT_DISCOVER_ALL_BREDR_SERVICES_CFM);

        memset(message, 0, sizeof(GATT_DISCOVER_ALL_BREDR_SERVICES_CFM_T));
        message->bd_addr = *bd_addr;
        message->status = gatt_status_max_connections;
        
        MessageSend(theAppTask, GATT_DISCOVER_ALL_BREDR_SERVICES_CFM, message);
        return;
    }

    /* store the address */
    conn->data.req.discover_all_bredr_services.addr = *bd_addr;

    gattSetConnState(conn,
                     theAppTask,
                     gatt_ms_discover_all_bredr_services);

    /* launch the search */
    ConnectionSdpServiceSearchAttributeRequest(
        gattGetTask(), bd_addr, SDC_MAX_ATTRIBUTES,
        sizeof(sdp_search), sdp_search,
        sizeof(sdp_search_attr), sdp_search_attr);
}

#endif /* DISC_ALL_PRIMARY_SERVICES */

#if (GATT_FEATURES & GATT_DISC_PRIMARY_SERVICE)
void GattDiscoverBrEdrServiceRequest(Task theAppTask,
                                     bdaddr *bd_addr,
                                     gatt_uuid_type_t uuid_type,
                                     const gatt_uuid_t *uuid)
{
    cid_map_t *conn = NULL;

    if (   (   (uuid_type < gatt_uuid16) 
            || (uuid_type > gatt_uuid128)) 
        || (conn = get_temp_conn(theAppTask)) == NULL)
    {
        MAKE_GATT_MESSAGE(GATT_DISCOVER_BREDR_SERVICE_CFM);

        memset(message, 0, sizeof(GATT_DISCOVER_BREDR_SERVICE_CFM_T));
        message->bd_addr = *bd_addr;
        message->status =
            conn ? gatt_status_max_connections : gatt_status_invalid_uuid;

        MessageSend(theAppTask, GATT_DISCOVER_BREDR_SERVICE_CFM, message);
        return;
    }

    /* store the address */
    conn->data.req.discover_bredr_service.addr = *bd_addr;

    gattSetConnState(conn,
                     theAppTask,
                     gatt_ms_discover_bredr_service);

    /* launch the search */
    search_sdp_service(bd_addr, uuid_type, uuid);
}

#endif /* DISC_PRIMARY_SERVICE */

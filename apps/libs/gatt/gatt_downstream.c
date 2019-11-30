/*************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_downstream.c

DESCRIPTION
    Functions to send downstream ATT primitives to Bluestack.

NOTES
*/

#include "gatt_private.h"
#include <stdlib.h>
#include <string.h>
#include <vm.h>

/* CASE macro for type conversion switch statements */
#define CASE(from, to) case (from): \
    if((from) == (to)) \
        goto coerce; \
    else \
        return (to);

/*************************************************************************
NAME    
    gatt_uuid_type_to_att
    
DESCRIPTION
    Convert the gatt_uuid_type_t enum type to the ATT uint16 defined equivalant.
    
RETURNS
    The uint16_t equivalant of the gatt_uuid_id.
    
*/
static uint16 gatt_uuid_type_to_att(gatt_uuid_type_t uuid_type)
{
    switch(uuid_type)
    {
        CASE(gatt_uuid_none, ATT_UUID_NONE);
        CASE(gatt_uuid16, ATT_UUID16);
        CASE(gatt_uuid128, ATT_UUID128);
        CASE(gatt_uuid32, ATT_UUID32);
        coerce: return (int16_t) uuid_type;
        default: return ATT_UUID_NONE;
    }
}

#if (GATT_FEATURES & GATT_EXCHANGE_MTU)

/*************************************************************************
NAME    
    gatt_exchange_mtu_req
    
DESCRIPTION
    Send ATT_EXCHANGE_MTU_REQ to Bluestack.
    
RETURNS
    
*/
void gatt_exchange_mtu_req(uint16 cid, uint16 mtu)
{
    MAKE_ATT_PRIM(ATT_EXCHANGE_MTU_REQ);
    
    prim->cid = cid;
    prim->mtu = mtu;

    VmSendAttPrim(prim);
}
#endif /* GATT_EXCHANGE_MTU */

/*************************************************************************
NAME    
    gatt_exchange_mtu_rsp
    
DESCRIPTION
    Send ATT_EXCHANGE_MTU_RSP to Bluestack.
    
RETURNS
    
*/
void gatt_exchange_mtu_rsp(uint16 cid, uint16 mtu)
{
    MAKE_ATT_PRIM(ATT_EXCHANGE_MTU_RSP);
    
    prim->cid = cid;
    prim->server_mtu = mtu;

    VmSendAttPrim(prim);
}

#if (GATT_FEATURES & GATT_PRIMARY_DISCOVERY)
/*************************************************************************
NAME    
    gatt_read_by_group_type_req
    
DESCRIPTION
    Send ATT_READ_BY_GROUP_TYPE_REQ to Bluestack.
    
RETURNS
    
*/
void gatt_read_by_group_type_req(uint16 cid,
                                 uint16 start,
                                 uint16 end,
                                 gatt_uuid_type_t uuid_type,
                                 const gatt_uuid_t *uuid)
{
    MAKE_ATT_PRIM(ATT_READ_BY_GROUP_TYPE_REQ);
    
    prim->cid = cid;
    prim->start = start;
    prim->end = end;
    prim->group_type = gatt_uuid_type_to_att(uuid_type);

    memmove(prim->group, uuid, GATT_UUID_SIZE);

    VmSendAttPrim(prim);
}

/*************************************************************************
NAME    
    gatt_find_by_type_value_req
    
DESCRIPTION
    Send ATT_FIND_BY_TYPE_VALUE_REQ to Bluestack.

    GATT uses this only for searching for UUID in value so the value
    parameter is UUID.

RETURNS
    
*/
void gatt_find_by_type_value_req(uint16 cid,
                                 uint16 start,
                                 uint16 end,
                                 uint16 type,
                                 gatt_uuid_type_t uuid_type,
                                 const gatt_uuid_t *uuid)
{
    uint32 u[4];
    uint32 *p;
    uint16 i;
    MAKE_ATT_PRIM(ATT_FIND_BY_TYPE_VALUE_REQ);
    
    prim->cid = cid;
    prim->start = start;
    prim->end = end;
    prim->uuid = type;

    if (uuid_type == gatt_uuid16)
    {
        prim->size_value = 2U; /* UUID16 = 2 bytes ( 16 / 8 )*/
        p = &u[0]; /* point to end of UUID16 */
    }
    else if (uuid_type == gatt_uuid32)
    {
        prim->size_value = 4U; /* UUID32 = 4 bytes ( 32 / 8 )*/
        p = &u[0]; /* point to end of UUID32 */
    }
    else
    {
        prim->size_value = 16U; /* UUID128 = 16 bytes ( 128 / 8 )*/
        p = &u[3]; /* point to end of UUID128 */
    }

    /* copy uuid */
    memmove(u, uuid, GATT_UUID_SIZE);
    
    /* allocate room for filter */
    prim->value = PanicUnlessMalloc(prim->size_value);
    
    /* copy pos bytes from the end to the beginning, i.e. swapping byte
     * order from big endian to little endian */
    for (i = 0; i < prim->size_value; i++)
    {
        prim->value[i] = *p & 0xff;

        /* jump to next index every time bit 2 changes position */
        if ((i & 0x3) == 0x03)
        {
            p--; /* move to the previous index */
        }
        else
        {
            *p >>= 8; /* shift to the next part of p */
        }
    }

    prim->value = VmGetHandleFromPointer(prim->value);

    VmSendAttPrim(prim);
}
#endif /* GATT_PRIMARY_DISCOVERY */

#if (GATT_FEATURES & (GATT_FIND_INCLUDED_SERVICES | \
                      GATT_DISC_ALL_CHARACTERISTICS | \
                      GATT_READ_BY_TYPE))
/*************************************************************************
NAME    
    gatt_read_by_type_req
    
DESCRIPTION
    Send ATT_READ_BY_TYPE_REQ to Bluestack.
    
RETURNS
    
*/
void gatt_read_by_type_req(uint16 cid,
                           uint16 start,
                           uint16 end,
                           gatt_uuid_type_t uuid_type,
                           const gatt_uuid_t *uuid)
{
    MAKE_ATT_PRIM(ATT_READ_BY_TYPE_REQ);
    
    prim->cid = cid;
    prim->start = start;
    prim->end = end;
    prim->uuid_type = gatt_uuid_type_to_att(uuid_type);

    memmove(prim->uuid, uuid, GATT_UUID_SIZE);

    VmSendAttPrim(prim);
}
#endif /* FIND_INCLUDED_SERVICES | DISC_ALL_CHARACTERISTICS | READ_BY_TYPE */

#if (GATT_FEATURES & (GATT_FIND_INCLUDED_SERVICES | GATT_READ))
/*************************************************************************
NAME    
    gatt_read_req
    
DESCRIPTION
    Send ATT_READ_REQ to Bluestack.
    
RETURNS
    
*/
void gatt_read_req(uint16 cid,
                   uint16 handle)
{
    MAKE_ATT_PRIM(ATT_READ_REQ);
    
    prim->cid = cid;
    prim->handle = handle;

    VmSendAttPrim(prim);
}
#endif /* GATT_FIND_INCLUDED_SERVICES | GATT_READ */

#if (GATT_FEATURES & (GATT_WRITE_SIGNED | \
                      GATT_WRITE))
/*************************************************************************
NAME    
    gatt_write_req
    
DESCRIPTION
    Send ATT_WRITE_REQ to Bluestack.
    
RETURNS
    
*/
void gatt_write_req(uint16 cid,
                    uint16 handle,
                    uint16 flags,
                    uint16 size_value,
                    const uint8 *value)
{
    MAKE_ATT_PRIM(ATT_WRITE_REQ);
    
    prim->cid = cid;
    prim->handle = handle;
    prim->flags = flags;
    prim->size_value = size_value;

    prim->value = PanicUnlessMalloc(size_value);
    memmove(prim->value, value, size_value);
    prim->value = VmGetHandleFromPointer(prim->value);

    VmSendAttPrim(prim);
}
#endif /* WRITE_COMMAND | WRITE_SIGNED | WRITE */

#if (GATT_FEATURES & GATT_WRITE_COMMAND)
/*************************************************************************
NAME    
    gatt_write_cmd
    
DESCRIPTION
    Send ATT_WRITE_CMD to Bluestack. The context field is not used.
    
RETURNS
    
*/
void gatt_write_cmd(uint16 cid,
                    uint16 handle,
                    uint16 size_value,
                    const uint8 *value)
{
    MAKE_ATT_PRIM(ATT_WRITE_CMD);
    
    prim->cid = cid;
    prim->handle = handle;
    prim->flags = ATT_WRITE_COMMAND;
    prim->size_value = size_value;

    prim->value = PanicUnlessMalloc(size_value);
    memmove(prim->value, value, size_value);
    prim->value = VmGetHandleFromPointer(prim->value);

    VmSendAttPrim(prim);
}
#endif /* WRITE_COMMAND */

#if (GATT_FEATURES & GATT_WRITE_LONG)
/*************************************************************************
NAME    
    gatt_write_req
    
DESCRIPTION
    Send ATT_WRITE_REQ to Bluestack.
    
RETURNS
    
*/
void gatt_prepare_write_req(uint16 cid,
                            uint16 handle,
                            uint16 offs,
                            uint16 size_value,
                            const uint8 *value)
{
    MAKE_ATT_PRIM(ATT_PREPARE_WRITE_REQ);
    
    prim->cid = cid;
    prim->handle = handle;
    prim->offset = offs;
    prim->size_value = size_value;

    prim->value = PanicUnlessMalloc(size_value);
    memmove(prim->value, value, size_value);
    prim->value = VmGetHandleFromPointer(prim->value);

    VmSendAttPrim(prim);
}

/*************************************************************************
NAME    
    gatt_write_req
    
DESCRIPTION
    Send ATT_WRITE_REQ to Bluestack.
    
RETURNS
    
*/
void gatt_execute_write_req(uint16 cid,
                            uint16 flags)
{
    MAKE_ATT_PRIM(ATT_EXECUTE_WRITE_REQ);
    
    prim->cid = cid;
    prim->flags = flags;

    VmSendAttPrim(prim);
}
#endif /* WRITE_LONG */

#if (GATT_FEATURES & GATT_DESCRIPTOR_DISCOVERY)
/*************************************************************************
NAME    
    gatt_find_info_req
    
DESCRIPTION
    Send ATT_FIND_INFO_REQ to Bluestack.
    
RETURNS
    
*/
void gatt_find_info_req(uint16 cid,
                        uint16 start,
                        uint16 end)
{
    MAKE_ATT_PRIM(ATT_FIND_INFO_REQ);
    
    prim->cid = cid;
    prim->start = start;
    prim->end = end;

    VmSendAttPrim(prim);
}
#endif /* GATT_DESCRIPTOR_DISCOVERY */

#if (GATT_FEATURES & GATT_READ_LONG)
/*************************************************************************
NAME    
    gatt_read_blob_req
    
DESCRIPTION
    Send ATT_READ_BLOB_REQ to Bluestack.
    
RETURNS
    
*/
void gatt_read_blob_req(uint16 cid,
                        uint16 offs,
                        uint16 handle)
{
    MAKE_ATT_PRIM(ATT_READ_BLOB_REQ);
    
    prim->cid = cid;
    prim->offset = offs;
    prim->handle = handle;

    VmSendAttPrim(prim);
}
#endif

#if (GATT_FEATURES & GATT_READ_MULTIPLE)
/*************************************************************************
NAME    
    gatt_read_multi_req
    
DESCRIPTION
    Send ATT_READ_MULTI_REQ to Bluestack.
    
RETURNS
    
*/
void gatt_read_multi_req(uint16 cid,
                        uint16 size_handles,
                        const uint16 *handles)
{
    size_t array_size = size_handles * sizeof(uint16_t);
    MAKE_ATT_PRIM(ATT_READ_MULTI_REQ);

    prim->cid = cid;
    prim->size_handles = size_handles;

    prim->handles = (uint16_t*)PanicUnlessMalloc(array_size);
    memmove(prim->handles, handles, array_size);
    prim->handles = (uint16_t*)VmGetHandleFromPointer(prim->handles);

    VmSendAttPrim(prim);
}
#endif

/****************************************************************************
Copyright (c) 2016 - 2017  Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_common.c
DESCRIPTION
    Library to implement a general purpose configuration facility.
    Provide getter and setter functionality to configuration data.
*/

#include <bdaddr.h>
#include "iap2_private.h"
#include "iap2_link.h"
#include "iap2_packet.h"

iap2_link *Iap2GetLinkForBdaddr(bdaddr *remote_bdaddr)
{
    iap2_link *link = NULL;

    if (iap2_lib)
    {
        link = iap2_lib->links;

        while (link)
        {
            if (BdaddrIsSame(remote_bdaddr, &link->remote_addr))
                break;

            link = link->next;
        }
    }

    return link;
}

iap2_status_t Iap2GetBdaddrForLink(iap2_link *link, bdaddr *remote_bdaddr)
{
    if (iap2LinkValidate(link))
    {
        if (remote_bdaddr)
            *remote_bdaddr = link->remote_addr;
        return iap2_status_success;
    }

    return iap2_status_link_not_found;
}

bool Iap2CtrlParamReadUint8(uint8 *payload, uint16 size_payload, uint16 param_id, uint8 *value)
{
    uint16 read_size;
    uint8 *read_ptr;

    if (Iap2CtrlParamGetReadAddress(payload, size_payload, param_id, &read_size, &read_ptr) &&
        read_size == 1)
    {
        if (value)
            *value = *read_ptr;

        return TRUE;
    }

    return FALSE;
}

bool Iap2CtrlParamReadUint16(uint8 *payload, uint16 size_payload, uint16 param_id, uint16 *value)
{
    uint16 read_size;
    uint8 *read_ptr;

    if (Iap2CtrlParamGetReadAddress(payload, size_payload, param_id, &read_size, &read_ptr) &&
        read_size == 2)
    {
        if (value)
            *value = ((uint16)read_ptr[0] << 8) | (uint16)read_ptr[1];

        return TRUE;
    }

    return FALSE;
}

bool Iap2CtrlParamReadUint32(uint8 *payload, uint16 size_payload, uint16 param_id, uint32 *value)
{
    uint16 read_size;
    uint8 *read_ptr;

    if (Iap2CtrlParamGetReadAddress(payload, size_payload, param_id, &read_size, &read_ptr) &&
        read_size == 4)
    {
        if (value)
            *value = ((uint32)read_ptr[0] << 24) |
                     ((uint32)read_ptr[1] << 16) |
                     ((uint32)read_ptr[2] << 8) |
                     (uint32)read_ptr[3];

        return TRUE;
    }

    return FALSE;
}

bool Iap2CtrlParamGetReadAddress(uint8 *payload, uint16 size_payload, uint16 param_id, uint16 *read_size, uint8 **read_ptr)
{
    bool success = FALSE;
    uint16 i = 0;
    uint16 param_size;

    while (payload && i < size_payload)
    {
        param_size = ((uint16)payload[i] << 8) | payload[i + 1];

        if (param_size >= IAP2_CTRL_PARAM_HEADER_LEN &&
            param_id == (((uint16)payload[i + 2] << 8) | payload[i + 3]))
        {
            if (read_size)
                *read_size = param_size - IAP2_CTRL_PARAM_HEADER_LEN;

            if (read_ptr)
                *read_ptr = (param_size > IAP2_CTRL_PARAM_HEADER_LEN) ? &payload[i + 4] : NULL;

            success = TRUE;
            break;
        }

        i += param_size;
    }

    return success;
}

uint16 Iap2CtrlParamWriteUint8(uint8 *buffer, uint16 param_id, uint8 value)
{
    uint16 param_len = IAP2_CTRL_PARAM_HEADER_LEN + 1;

    if (buffer)
    {
        *buffer++ = (param_len >> 8) & 0xff;
        *buffer++ = param_len & 0xff;
        *buffer++ = (param_id >> 8) & 0xff;
        *buffer++ = param_id & 0xff;
        *buffer++ = value;
    }

    return param_len;
}

uint16 Iap2CtrlParamWriteUint16(uint8 *buffer, uint16 param_id, uint16 value)
{
    uint16 param_len = IAP2_CTRL_PARAM_HEADER_LEN + 2;

    if (buffer)
    {
        *buffer++ = (param_len >> 8) & 0xff;
        *buffer++ = param_len & 0xff;
        *buffer++ = (param_id >> 8) & 0xff;
        *buffer++ = param_id & 0xff;
        *buffer++ = (value >> 8) & 0xff;
        *buffer++ = value & 0xff;
    }

    return param_len;
}

uint16 Iap2CtrlParamWriteUint32(uint8 *buffer, uint16 param_id, uint32 value)
{
    uint16 param_len = IAP2_CTRL_PARAM_HEADER_LEN + 4;

    if (buffer)
    {
        *buffer++ = (param_len >> 8) & 0xff;
        *buffer++ = param_len & 0xff;
        *buffer++ = (param_id >> 8) & 0xff;
        *buffer++ = param_id & 0xff;
        *buffer++ = (value >> 24) & 0xff;
        *buffer++ = (value >> 16) & 0xff;
        *buffer++ = (value >> 8) & 0xff;
        *buffer++ = value & 0xff;
    }

    return param_len;
}

uint16 Iap2CtrlParamWriteString(uint8 *buffer, uint16 param_id, const char *string)
{
    uint16 param_len = IAP2_CTRL_PARAM_HEADER_LEN + strlen(string) + 1;

    if (buffer)
    {
        *buffer++ = (param_len >> 8) & 0xff;
        *buffer++ = param_len & 0xff;
        *buffer++ = (param_id >> 8) & 0xff;
        *buffer++ = param_id & 0xff;
        if (string)
            memmove(buffer, string, strlen(string) + 1);
    }

    return param_len;
}

uint16 Iap2CtrlParamWriteBlob(uint8 *buffer, uint16 param_id, uint8 *data, uint16 size_data)
{
    uint16 param_len = IAP2_CTRL_PARAM_HEADER_LEN + size_data;
    uint16 write_len = 0;

    if (buffer)
    {
        *buffer++ = (param_len >> 8) & 0xff;
        *buffer++ = param_len & 0xff;
        *buffer++ = (param_id >> 8) & 0xff;
        *buffer++ = param_id & 0xff;
        write_len = 4;
        if (size_data && data)
        {
            memmove(buffer, data, size_data);
            write_len += size_data;
        }

        return write_len;
    }

    return param_len;
}

uint8 iap2Checksum(uint8 *data, uint16 size)
{
    uint8 checksum = 0;
    while (size--)
        checksum += *data++;
    return (checksum & 0xff);
}


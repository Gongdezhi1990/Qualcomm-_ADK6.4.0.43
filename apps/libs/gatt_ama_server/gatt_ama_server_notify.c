/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/* Part of 6.2 */

#include "gatt_ama_server_private.h"
#include "gatt_ama_server_db.h"
#include "gatt_ama_server_access.h"
#include <gatt_ama_server.h>

extern GAMASS *AmaServer;

/****************************************************************************/
bool GattAmaServerSendNotification(const GAMASS *ama, uint16 cid, uint16 handle,
                                   uint16 length, uint8 *data)
{
    /* Validate this instance */
    if(ama == NULL)
    {
        GATT_AMA_SERVER_PANIC(("GAMASS: Null instance"));
    }

    /* Validate the input parameters */
    if ((cid == 0) || (length == 0) || (data == NULL))
    {
        return FALSE;
    }

    handle = HANDLE_AMA_ALEXA_RX_CHAR;

    /* Send notification to GATT Manager */
    GattManagerRemoteClientNotify((Task)&ama->lib_task, cid, handle, length, data);
    return TRUE;
}

bool SendAmaNotification(uint8 *data, uint16 length)
{
    uint16 tx_mtu_size = GattGetMaxTxDataLength(AmaServer->cid);

    /* The connection id must be invalid at this point */
    if (tx_mtu_size == 0)
    {
        GATT_AMA_SERVER_DEBUG_INFO(("AMA: Invalid MTU, drop notification\n"));
        return FALSE;
    }

    while(length)
    {
        uint16 bytesToSend = MIN(length, tx_mtu_size);

        /* Send notification to GATT Manager */
        GattManagerRemoteClientNotify((Task)&AmaServer->lib_task, AmaServer->cid, HANDLE_AMA_ALEXA_RX_CHAR, bytesToSend, data);

        data += bytesToSend;
        length -= bytesToSend;
    }

    return TRUE;
}


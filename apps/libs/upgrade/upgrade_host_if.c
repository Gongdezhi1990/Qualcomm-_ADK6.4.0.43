/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade_host_if.c

DESCRIPTION

NOTES

*/


#include <stdlib.h>
#include <string.h>

#include <panic.h>
#include <print.h>

#include <upgrade.h>
#include <byte_utils.h>
#include "upgrade_ctx.h"
#include "upgrade_host_if.h"
#include "upgrade_host_if_data.h"

void UpgradeHostIFClientConnect(Task clientTask)
{
    UpgradeCtxGet()->clientTask = clientTask;
}

/****************************************************************************
NAME
    UpgradeHostIFClientSendData

DESCRIPTION
    Send a data packet to a connected upgrade client.

*/
void UpgradeHostIFClientSendData(uint8 *data, uint16 dataSize)
{
    if(UpgradeCtxGet()->transportTask)
    {
        UPGRADE_TRANSPORT_DATA_IND_T *dataInd;

        PRINT(("UpgradeHostIFClientSendData 0x%p\n", (void *)UpgradeCtxGet()->transportTask));

        dataInd = (UPGRADE_TRANSPORT_DATA_IND_T *)PanicUnlessMalloc(sizeof(*dataInd) + dataSize - 1);
        
        PRINT(("UpgradeHostIFClientSendData len %d\n", dataSize));

        ByteUtilsMemCpyToStream(dataInd->data, data, dataSize);

        free(data);

        dataInd->size_data = dataSize;

        dataInd->is_data_state = UpgradeIsPartitionDataState();

        MessageSend(UpgradeCtxGet()->transportTask, UPGRADE_TRANSPORT_DATA_IND, dataInd);
    }
    else
    {
        free(data);
    }
}

/****************************************************************************
NAME
    UpgradeHostIFTransportConnect

DESCRIPTION
    Process an upgrade connect request from a client.
    Send a response to the transport in a UPGRADE_TRANSPORT_CONNECT_CFM.

    If Upgrade library has not been initialised return 
    upgrade_status_unexpected_error.

    If an upgrade client is already connected return
    upgrade_status_already_connected_warning.
    
    Otherwise return upgrade_status_success.

*/
void UpgradeHostIFTransportConnect(Task transportTask, bool need_data_cfm, bool request_multiple_blocks)
{
    MESSAGE_MAKE(connectCfm, UPGRADE_TRANSPORT_CONNECT_CFM_T);

    if (!UpgradeIsInitialised())
    {
        /*! @todo Should really add a new error type here? */
        connectCfm->status = upgrade_status_unexpected_error;
    }
    else if(UpgradeCtxGet()->transportTask)
    {
        connectCfm->status = upgrade_status_already_connected_warning;
    }
    else
    {
        connectCfm->status = upgrade_status_success;
        UpgradeCtxGet()->transportTask = transportTask;
        UpgradeCtxGet()->need_data_cfm = need_data_cfm;
        UpgradeCtxGet()->request_multiple_blocks = request_multiple_blocks;
    }

    MessageSend(transportTask, UPGRADE_TRANSPORT_CONNECT_CFM, connectCfm);
}

/****************************************************************************
NAME
    UpgradeHostIFProcessDataRequest

DESCRIPTION
    Process a data packet from an Upgrade client.
    Send a response to the transport in a UPGRADE_TRANSPORT_DATA_CFM.

    If Upgrade library has not been initialised or there is no
    transport connected, do nothing.
*/
void UpgradeHostIFProcessDataRequest(uint8 *data, uint16 dataSize)
{
    UPGRADE_TRANSPORT_DATA_CFM_T *dataCfm;

    PRINT(("UpgradeHostIFProcessDataRequest\n"));

    if (!UpgradeIsInitialised()
        || UpgradeCtxGet()->transportTask == 0)
        return;

    UpgradeHostIFDataBuildIncomingMsg(UpgradeCtxGet()->clientTask, data, dataSize);

    if (UpgradeCtxGet()->need_data_cfm)
    {
        dataCfm = (UPGRADE_TRANSPORT_DATA_CFM_T *) PanicUnlessMalloc(sizeof(UPGRADE_TRANSPORT_DATA_CFM_T));
        memset(dataCfm, 0, sizeof(UPGRADE_TRANSPORT_DATA_CFM_T));

        dataCfm->status = 0;
        if(dataSize && data != NULL)
        {
            dataCfm->packet_type = data[0];    /* See UpgradeMsgHost */
        }

        MessageSend(UpgradeCtxGet()->transportTask, UPGRADE_TRANSPORT_DATA_CFM, dataCfm);
    }
}


/****************************************************************************
NAME
    UpgradeHostIFTransportDisconnect

DESCRIPTION
    Process a disconnect request from an Upgrade client.
    Send a response to the transport in a UPGRADE_TRANSPORT_DISCONNECT_CFM.

    If Upgrade library has not been initialised or there is no
*/
void UpgradeHostIFTransportDisconnect(void)
{

    if (UpgradeIsInitialised()
        && UpgradeCtxGet()->transportTask)
    {
        MESSAGE_MAKE(disconnectCfm, UPGRADE_TRANSPORT_DISCONNECT_CFM_T);

        disconnectCfm->status = 0;

        MessageSend(UpgradeCtxGet()->transportTask, UPGRADE_TRANSPORT_DISCONNECT_CFM, disconnectCfm);

        UpgradeCtxGet()->transportTask = 0;
    }
}

/****************************************************************************
NAME
    UpgradeHostIFTransportInUse

DESCRIPTION
    Return an indication of whether or not the
*/
bool UpgradeHostIFTransportInUse(void)
{
    if (UpgradeIsInitialised() && UpgradeCtxGet()->transportTask)
    {
        return TRUE;
    }

    return FALSE;
}

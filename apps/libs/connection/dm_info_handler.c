/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_info_handler.c

DESCRIPTION
    This is the informational primitive management entity and is used to
    route responses back to the task that initiated the request.

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "dm_info_handler.h"

#include <bdaddr.h>
#include <vm.h>
#include <string.h>
#include <sink.h>


/* Send a link quality cfm message to the client */
static void sendLinkQualityCfm(
        Task task,
        hci_status status,
        uint8 link_quality,
        Sink sink
        )
{
    if (task)
    {
        /* Create and send the message */
        MAKE_CL_MESSAGE(CL_DM_LINK_QUALITY_CFM);
        message->status = status;
        message->link_quality = link_quality;
        message->sink = sink;
        MessageSend(task, CL_DM_LINK_QUALITY_CFM, message);
    }
}


/* Send an RSSI cfm message to the client */
static void sendRssiCfm(Task task, hci_status status, uint8 rssi, Sink sink)
{
    if (task)
    {
        /* Create and send the message */
        MAKE_CL_MESSAGE(CL_DM_RSSI_CFM);
        message->status = status;
        message->rssi = rssi;
        message->sink = sink;
        MessageSend(task, CL_DM_RSSI_CFM, message);
    }
}

/* Send an RSSI cfm message to the client */
static void sendRssiBdaddrCfm(Task task,
                              hci_status status,
                              uint8 rssi,
                              const TP_BD_ADDR_T *tp_addrt)
{
    if (task)
    {
        /* Create and send the message */
        MAKE_CL_MESSAGE(CL_DM_RSSI_BDADDR_CFM);
        message->status = status;
        message->rssi = rssi;
        BdaddrConvertTpBluestackToVm(&message->tpaddr, tp_addrt);
        MessageSend(task, CL_DM_RSSI_BDADDR_CFM, message);
    }
}

/* Send an clock offset cfm message to the client */
static void sendClockOffsetCfm(
        Task task,
        hci_status status,
        uint16 offset,
        Sink sink
        )
{
    if (task)
    {
        /* Create and send the message */
        MAKE_CL_MESSAGE(CL_DM_CLOCK_OFFSET_CFM);
        message->status = status;
        message->sink = sink;
        message->clock_offset = offset;
        MessageSend(task, CL_DM_CLOCK_OFFSET_CFM, message);
    }
}


/* Send a remote supported features cfm message to the client */
static void sendRemoteSupportedFeaturesCfm(
        Task task,
        hci_status status,
        const uint16 *features,
        Sink sink
        )
{
    if (task)
    {
        /* Create and send the message */
        MAKE_CL_MESSAGE(CL_DM_REMOTE_FEATURES_CFM);
        message->status = status;
        message->sink = sink;

        if (features)
            memmove(message->features, features, sizeof(message->features));
        else
            *message->features = 0;

        MessageSend(task, CL_DM_REMOTE_FEATURES_CFM, message);
    }
}


/* Send a remote supported extended features cfm message to the client */
static void sendRemoteSupportedExtendedFeaturesCfm(
        Task task,
        hci_status status,
        uint8 page_num,
        uint8 max_page_num,
        const uint16 *lmp_ext_features,
        bdaddr* bd_addr
        )
{
    if (task)
    {
        /* Create and send the message */
        MAKE_CL_MESSAGE(CL_DM_REMOTE_EXTENDED_FEATURES_CFM);
        /* Fill in bd_addr if valid one could be obtained */
        if(bd_addr != NULL)
            message->bd_addr = *bd_addr;
        else
            BdaddrSetZero(&message->bd_addr);
        
        message->status = status;
        message->page_num = page_num;
        message->max_page_num = max_page_num;       

        if (lmp_ext_features)
            memmove(message->lmp_ext_features, lmp_ext_features, 4);
        else
            *message->lmp_ext_features = 0;

        MessageSend(task, CL_DM_REMOTE_EXTENDED_FEATURES_CFM, message);
    }
}


/****************************************************************************
NAME
    connectionHandleReadAddrRequest

DESCRIPTION
    Handle an internal request to read the local bluetooth address

RETURNS
    void
*/
void connectionHandleReadAddrRequest(
        connectionReadInfoState *state,
        const CL_INTERNAL_DM_READ_BD_ADDR_REQ_T *req
        )
{
    if (!state->stateInfoLock)
    {
        /* Set the lock */
        state->stateInfoLock = req->theAppTask;
        state->sink = 0;

        /* Response not outstanding so issue request */
        {
            MAKE_PRIM_C(DM_HCI_READ_BD_ADDR_REQ);
            VmSendDmPrim(prim);
        }
    }
    else
    {
        /* Lock set so queue up the request */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_BD_ADDR_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_DM_READ_BD_ADDR_REQ,
                message,
                &state->stateInfoLock
                );
    }
}


/****************************************************************************
NAME
    connectionHandleReadBdAddrComplete

DESCRIPTION
    Handle a read bd addr complete event

RETURNS
    void
*/
void connectionHandleReadBdAddrComplete(
        connectionReadInfoState *state,
        const DM_HCI_READ_BD_ADDR_CFM_T *cfm
        )
{
    if (state->stateInfoLock)
    {
        /* Send the result to the client */
        MAKE_CL_MESSAGE(CL_DM_LOCAL_BD_ADDR_CFM);
        message->status = connectionConvertHciStatus(cfm->status);
        BdaddrConvertBluestackToVm(&message->bd_addr, &cfm->bd_addr);
        MessageSend(state->stateInfoLock, CL_DM_LOCAL_BD_ADDR_CFM, message);

        /* Reset the lock */
        state->stateInfoLock = 0;
    }
}


/****************************************************************************
NAME
    connectionHandleReadLinkQualityRequest

DESCRIPTION
    Issue a request to read the link quality on a particular connection.

RETURNS
    void
*/
void connectionHandleReadLinkQualityRequest(
        connectionReadInfoState *state,
        const CL_INTERNAL_DM_READ_LINK_QUALITY_REQ_T *req
        )
{
    /* Check the resource lock */
    if (!state->stateInfoLock)
    {
        tp_bdaddr tpaddr;

        /* Check we got a valid addr */
        if (SinkGetBdAddr(req->sink, &tpaddr))
        {
            /* Response not outstanding so issue request */
            MAKE_PRIM_C(DM_HCI_GET_LINK_QUALITY_REQ);
            BdaddrConvertVmToBluestack(&prim->bd_addr, &tpaddr.taddr.addr);
            VmSendDmPrim(prim);

            /* Set the lock */
            state->stateInfoLock = req->theAppTask;
            state->sink = req->sink;
        }
        else
        {
            /* Send an error to the app as it didn't pass in a valid sink */
            sendLinkQualityCfm(
                    req->theAppTask,
                    hci_error_no_connection,
                    0,
                    req->sink
                    );
        }
    }
    else
    {
        /* Lock set so queue up the request */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_LINK_QUALITY_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_DM_READ_LINK_QUALITY_REQ,
                message,
                &state->stateInfoLock
                );
    }
}


/****************************************************************************
NAME
    connectionHandleReadLinkQualityComplete

DESCRIPTION
    Confirm containing the link quality if the read request succeeded.

RETURNS
    void
*/
void connectionHandleReadLinkQualityComplete(
        connectionReadInfoState *state,
        const DM_HCI_GET_LINK_QUALITY_CFM_T *cfm
        )
{
    /* Let the app know the outcome of the read request */
    sendLinkQualityCfm(
            state->stateInfoLock,
            connectionConvertHciStatus(cfm->status),
            cfm->link_quality,
            state->sink
            );

    /* Reset the lock */
    state->stateInfoLock = 0;
    state->sink = 0;
}


/****************************************************************************
NAME
    connectionHandleReadRssiRequest
DESCRIPTION
    Request to read the RSSI on a particular connection. Handles both:
    - ConnectionGetRssi(...) - req->sink = sink, req->tpaddr = ignored
    - ConnectionGettRssiBdaddr(...) - req->sink = NULL, req->tpaddr = tpaddr

RETURNS
    void
*/
void connectionHandleReadRssiRequest(
        connectionReadInfoState                 *state,
        const CL_INTERNAL_DM_READ_RSSI_REQ_T    *req
        )
{
    /* Check the resource lock */
    if (!state->stateInfoLock)
    {
        /* if sink is not NULL, then retrieve address for sink (if valid)
         * and send DM prim for that retrieved address.
         */
        tp_bdaddr tpaddr;

        /* If there is a sink, check the device address for that sink is valid.
         */
        if (req->sink && !SinkGetBdAddr(req->sink, &tpaddr))
        {
            /* Send an error to the app as it didn't pass in a valid sink. */
            sendRssiCfm(req->theAppTask, hci_error_no_connection, 0, req->sink);
        }
        /* Sink is valid and the device address for that sink has been retrieved
         * OR sink is NULL and the tpaddr passed in the req message must be
         * used.
         */
        else
        {
            MAKE_PRIM_C(DM_HCI_READ_RSSI_REQ);

            /* Sink is set so use device address retrieved for that sink. */
            if (req->sink)
                BdaddrConvertTpVmToBluestack(&prim->tp_addrt, &tpaddr);
            /* Sink is NULL so use the device address passed in req message. */
            else
                BdaddrConvertTpVmToBluestack(&prim->tp_addrt, &req->tpaddr);

            VmSendDmPrim(prim);

            /* Set the lock */
            state->stateInfoLock = req->theAppTask;
            state->sink = req->sink;
        }
    }
    else
    {
        /* Lock set so queue up the request */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_RSSI_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_DM_READ_RSSI_REQ,
                message,
                &state->stateInfoLock
                );
    }
}


/****************************************************************************
NAME
    connectionHandleReadRssiComplete

DESCRIPTION
    Confirm containing the RSSI value if the read request succeeded.

RETURNS
    void
*/
void connectionHandleReadRssiComplete(
        connectionReadInfoState *state,
        const DM_HCI_READ_RSSI_CFM_T *cfm
        )
{
    /* Pass the result to the app */
    if (state->sink)
    {
        sendRssiCfm(state->stateInfoLock,
                    connectionConvertHciStatus(cfm->status),
                    cfm->rssi,
                    state->sink);
    }
    else
    {
        sendRssiBdaddrCfm(state->stateInfoLock,
                          connectionConvertHciStatus(cfm->status),
                          cfm->rssi,
                          &cfm->tp_addrt);
    }

    /* Reset the lock */
    state->stateInfoLock = 0;
    state->sink = 0;
}


/****************************************************************************
NAME
    connectionHandleReadclkOffsetRequest

DESCRIPTION
    Request to read the clock offset of a remote device.

RETURNS
    void
*/
void connectionHandleReadclkOffsetRequest(
        connectionReadInfoState *state,
        const CL_INTERNAL_DM_READ_CLK_OFFSET_REQ_T *req
        )
{
    /* Check the resource lock */
    if (!state->stateInfoLock)
    {
        tp_bdaddr tpaddr;

        /* Check we got a valid addr */
        if (SinkGetBdAddr(req->sink, &tpaddr))
        {
            /* Response not outstanding so issue request */
            MAKE_PRIM_C(DM_HCI_READ_CLOCK_OFFSET_REQ);
            BdaddrConvertVmToBluestack(&prim->bd_addr, &tpaddr.taddr.addr);
            VmSendDmPrim(prim);

            /* Set the lock */
            state->stateInfoLock = req->theAppTask;
            state->sink = req->sink;
        }
        else
        {
            /* Send an error to the app as it didn't pass in a valid sink */
            sendClockOffsetCfm(
                    req->theAppTask,
                    hci_error_no_connection,
                    0,
                    req->sink
                    );
        }
    }
    else
    {
        /* Lock set so queue up the request */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_CLK_OFFSET_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_DM_READ_CLK_OFFSET_REQ,
                message,
                &state->stateInfoLock
                );
    }
}


/****************************************************************************
NAME
    connectionHandleReadClkOffsetComplete

DESCRIPTION
    Confirm containing the remote clock offset if the read request succeeded.

RETURNS
    void
*/
void connectionHandleReadClkOffsetComplete(
        connectionReadInfoState *state,
        const DM_HCI_READ_CLOCK_OFFSET_CFM_T *cfm
        )
{
    /* Tell the application about the outcome of the read clock offset request.
     */
    sendClockOffsetCfm(
            state->stateInfoLock,
            connectionConvertHciStatus(cfm->status),
            cfm->clock_offset,
            state->sink
            );

    /* Reset the lock */
    state->stateInfoLock = 0;
    state->sink = 0;
}


/****************************************************************************
NAME
    connectionHandleReadRemoteSupportedFeaturesRequest

DESCRIPTION
    Request to read the supported features of a remote device.

RETURNS
    void
*/
void connectionHandleReadRemoteSupportedFeaturesRequest(
        connectionReadInfoState *state,
        const CL_INTERNAL_DM_READ_REMOTE_SUPP_FEAT_REQ_T *req
        )
{
    /* Check the resource lock */
    if (!state->stateInfoLock)
    {
        tp_bdaddr tpaddr;

        /* Check we got a valid addr */
        if (SinkGetBdAddr(req->sink, &tpaddr))
        {
                /* Response not outstanding so issue request */
                MAKE_PRIM_C(DM_HCI_READ_REMOTE_SUPP_FEATURES_REQ);
                BdaddrConvertVmToBluestack(&prim->bd_addr, &tpaddr.taddr.addr);
                VmSendDmPrim(prim);

                /* Set the lock */
                state->stateInfoLock = req->theAppTask;
                state->sink = req->sink;
        }
        else
        {
            /* Send an error to the app as it didn't pass in a valid sink */
            sendRemoteSupportedFeaturesCfm(
                    req->theAppTask,
                    hci_error_no_connection,
                    0,
                    req->sink
                    );
        }
    }
    else
    {
        /* Lock set so queue up the request */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_REMOTE_SUPP_FEAT_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_DM_READ_REMOTE_SUPP_FEAT_REQ,
                message,
                &state->stateInfoLock
                );
    }
}


/****************************************************************************
NAME
    connectionHandleReadRemoteSupportedFeaturesCfm

DESCRIPTION
    Confirm containing the remote supported features, if the read request
    succeeded.

RETURNS
    void
*/
void connectionHandleReadRemoteSupportedFeaturesCfm(
        connectionReadInfoState *state,
        const DM_HCI_READ_REMOTE_SUPP_FEATURES_CFM_T *cfm
        )
{
    /* Send message up to the client only if the lock is set */
    if (state->stateInfoLock)
    {
        /* Need to check the lock since we get an unsolicited remote features
         * cfm message every time an ACL is opened because BlueStack obtains
         * the remote features automatically.
         */
        sendRemoteSupportedFeaturesCfm(
                state->stateInfoLock,
                connectionConvertHciStatus(cfm->status),
                cfm->features,
                state->sink
                );
    }

    /* Reset the lock */
    state->stateInfoLock = 0;
    state->sink = 0;
}


/****************************************************************************
NAME
    connectionHandleReadRemoteSupportedExtFeaturesRequest

DESCRIPTION
    Request to read the supported extende features of a remote device.

RETURNS
    void
*/
void connectionHandleReadRemoteSupportedExtFeaturesRequest(
        connectionReadInfoState *state,
        const CL_INTERNAL_DM_READ_REMOTE_SUPP_EXT_FEAT_REQ_T *req
        )
{
    /* Check the resource lock */
    if (!state->stateInfoLock)
    {
        /* Response not outstanding so issue request */
        MAKE_PRIM_C(DM_HCI_READ_REMOTE_EXT_FEATURES_REQ);
        BdaddrConvertVmToBluestack(&prim->bd_addr, &req->bd_addr);
        prim->page_num = req->page_num;
        VmSendDmPrim(prim);

        /* Set the lock */
        state->stateInfoLock = req->theAppTask;
    }
    else
    {
        /* Lock set so queue up the request */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_REMOTE_SUPP_EXT_FEAT_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_DM_READ_REMOTE_SUPP_EXT_FEAT_REQ,
                message, &state->stateInfoLock
                );
    }   
}


/****************************************************************************
NAME
    connectionHandleReadRemoteSupportedExtFeaturesCfm

DESCRIPTION
    Confirm containing the remote supported extended features, if the read
    request succeeded.

RETURNS
    void
*/
void connectionHandleReadRemoteSupportedExtFeaturesCfm(
        connectionReadInfoState *state,
        const DM_HCI_READ_REMOTE_EXT_FEATURES_CFM_T *cfm
        )
{
    bdaddr  addr;
    /* Send message up to the client only if the lock is set */
    if (state->stateInfoLock)
    {
        BdaddrConvertBluestackToVm(&addr, &cfm->bd_addr);
        
        /* Need to check the lock since we get an unsolicited extended remote
         * features cfm message every time an ACL is opened because BlueStack
         * obtains the extended remote features automatically.
         */
        sendRemoteSupportedExtendedFeaturesCfm(
                state->stateInfoLock,
                connectionConvertHciStatus(cfm->status),
                cfm->page_num,
                cfm->max_page_num,
                (const uint16 *) &(cfm->lmp_ext_features),
                &addr
                );
    }

    /* Reset the lock */
    state->stateInfoLock = 0;
    state->sink = 0;
}


/****************************************************************************
NAME
    connectionHandleReadLocalVersionRequest

DESCRIPTION
    Request to read the version information of the local device.

RETURNS
    void
*/
void connectionHandleReadLocalVersionRequest(
        connectionReadInfoState *state,
        const CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ_T *req
        )
{
    /* Check the resource lock */
    if (!state->stateInfoLock)
    {
        /* Response not outstanding so issue request */
        MAKE_PRIM_C(DM_HCI_READ_LOCAL_VER_INFO_REQ);
        VmSendDmPrim(prim);

        /* Set the lock */
        state->stateInfoLock = req->theAppTask;
    }
    else
    {
        /* Lock set so queue up the request */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ,
                message,
                &state->stateInfoLock
                );
    }
}


/****************************************************************************
NAME
    connectionHandleReadLocalVersionCfm

DESCRIPTION
    Confirm containing the local version information, if the read request
    succeeded.

RETURNS
    void
*/
void connectionHandleReadLocalVersionCfm(
        connectionReadInfoState *state,
        const DM_HCI_READ_LOCAL_VER_INFO_CFM_T *cfm
        )
{
    if(state->stateInfoLock == connectionGetCmTask())
    {

        /* We got here as part of intialisation, continue version setup */
        if(cfm->hci_version >= HCI_VER_2_0)
        {
            /* Firmware supports 2.1 so can setup the BT version in BlueStack */
            MAKE_CL_MESSAGE(CL_INTERNAL_DM_SET_BT_VERSION_REQ);
            message->theAppTask = connectionGetCmTask();

            switch(cfm->hci_version)
            {
                case HCI_VER_2_0:
                    message->version = BT_VERSION_2p0;
                    break;
                case HCI_VER_2_1:
                    message->version = BT_VERSION_2p1;
                    break;
                case HCI_VER_3_0:
                    message->version = BT_VERSION_3p0;
                    break;
                case HCI_VER_4_0:
                    message->version = BT_VERSION_4p0;
                    break;
                case HCI_VER_4_1:
                    message->version = BT_VERSION_4p1;
                    break;
                case HCI_VER_4_2:
                    message->version = BT_VERSION_4p2;
                    break;
                case HCI_VER_5_0:
                    message->version = BT_VERSION_5p0;
                    break;
                case HCI_VER_5_1:
                    message->version = BT_VERSION_5p1;
                    break;
                default:
                message->version = BT_VERSION_MAX;
                    break;
            }
            MessageSend(
                    connectionGetCmTask(),
                    CL_INTERNAL_DM_SET_BT_VERSION_REQ, message
                    );
        }
        else
        {
            /* Nothing more to be done for version init, continue init process.
             */
            state->version = bluetooth_unknown;
            connectionSendInternalInitCfm(connectionInitVer);
        }
    }
    else if (state->stateInfoLock)
    {
        /* Create and send the message */
        MAKE_CL_MESSAGE(CL_DM_LOCAL_VERSION_CFM);
        message->hciVersion = connectionConvertHciVersion(cfm->hci_version);
        message->hciRevision = cfm->hci_revision;
        message->status = connectionConvertHciStatus(cfm->status);
        message->lmpVersion       = cfm->lmp_version;
        message->manufacturerName = cfm->manuf_name;
        message->lmpSubVersion    = cfm->lmp_subversion;

        MessageSend(state->stateInfoLock, CL_DM_LOCAL_VERSION_CFM, message);
    }
    /* Reset the lock */
    state->stateInfoLock = 0;
    state->sink = 0;
}


/****************************************************************************
NAME
    connectionHandleReadRemoteVersionRequest

DESCRIPTION
    Request to read the version information of a remote device.

RETURNS
    void
*/
void connectionHandleReadRemoteVersionRequest(
        connectionReadInfoState *state,
        const CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ_T *req
        )
{
    /* Check the resource lock */
    if (!state->stateInfoLock)
    {
        /* Response not outstanding so issue request */
        MAKE_PRIM_C(DM_HCI_READ_REMOTE_VER_INFO_REQ);
        BdaddrConvertTpVmToBluestack(&prim->tp_addrt, &req->tpaddr);
        VmSendDmPrim(prim);

        /* Set the lock */
        state->stateInfoLock = req->theAppTask;
    }
    else
    {
        /* Lock set so queue up the request */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ,
                message, &state->stateInfoLock
                );
    }
}


/****************************************************************************
NAME
    connectionHandleReadRemoteVersionCfm

DESCRIPTION
    Confirm containing the remote version information, if the read request
    succeeded.

RETURNS
    void
*/
void connectionHandleReadRemoteVersionCfm(
        connectionReadInfoState *state,
        const DM_HCI_READ_REMOTE_VER_INFO_CFM_T *cfm
        )
{
    if (state->stateInfoLock)
    {
        /* Create and send the message */
        MAKE_CL_MESSAGE(CL_DM_REMOTE_VERSION_CFM);
        message->status = connectionConvertHciStatus(cfm->status);
        message->lmpVersion       = cfm->LMP_version;
        message->manufacturerName = cfm->manufacturer_name;
        message->lmpSubVersion    = cfm->LMP_subversion;

        MessageSend(state->stateInfoLock, CL_DM_REMOTE_VERSION_CFM, message);

    /* Reset the lock */
    state->stateInfoLock = 0;
    state->sink = 0;
    }
}


/****************************************************************************
NAME
    connectionHandleSetBtVersionReq

DESCRIPTION
    Handle setting BT Version

RETURNS
    void
*/
void connectionHandleSetBtVersionReq(
        connectionReadInfoState *state,
        const CL_INTERNAL_DM_SET_BT_VERSION_REQ_T *req
        )
{
    if(!state->stateInfoLock)
    {
        state->stateInfoLock = req->theAppTask;
        {
        MAKE_PRIM_T(DM_SET_BT_VERSION_REQ);
        prim->version = req->version;
        VmSendDmPrim(prim);
        }
    }
    else
    {
        /* Lock set so queue up the request */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_SET_BT_VERSION_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_DM_SET_BT_VERSION_REQ,
                message,
                &state->stateInfoLock
                );
    }
}


/****************************************************************************
NAME
    connectionHandleSetBtVersionCfm

DESCRIPTION
    Confirm containing the request status and current BT Version

RETURNS
    void
*/
void connectionHandleSetBtVersionCfm(
        connectionReadInfoState *state,
        const DM_SET_BT_VERSION_CFM_T *cfm
        )
{
    if(state->stateInfoLock)
    {
        if(state->stateInfoLock == connectionGetCmTask())
        {
            /* We finished initialising bt version, continue init process */
            connectionSendInternalInitCfm(connectionInitVer);
        }
        else
        {
            MAKE_CL_MESSAGE(CL_DM_READ_BT_VERSION_CFM);
            message->status = connectionConvertHciStatus(cfm->status);
            message->version = connectionConvertBtVersion(cfm->version);
            MessageSend(
                    state->stateInfoLock,
                    CL_DM_READ_BT_VERSION_CFM,
                    message
                    );
        }
    }

    state->stateInfoLock = 0;
    state->version = cfm->version;
}


/****************************************************************************
NAME
    connectionHandleDmHciModeChangeEvent

DESCRIPTION
    An indication from BlueStack that an ACL has been opened. Some clients
    may need this information so pass the indication up to the task
    registered as the "app task" (we don't know who else to pass this to!).

RETURNS
    void
*/
void connectionHandleDmHciModeChangeEvent(
        Task task,
        const DM_HCI_MODE_CHANGE_EVENT_IND_T *ev
        )
{
    const msg_filter *msgFilter = connectionGetMsgFilter();
    if (task && (msgFilter[0] & msg_group_mode_change))
    {
        /* Check mode change was successful */
        if (
                (ev->status == HCI_SUCCESS) &&
                (
                    (ev->mode == HCI_BT_MODE_ACTIVE) ||
                    (ev->mode == HCI_BT_MODE_SNIFF)
                )
           )
        {
            MAKE_CL_MESSAGE(CL_DM_MODE_CHANGE_EVENT);
            BdaddrConvertBluestackToVm(&message->bd_addr, &ev->bd_addr);

            /* Convert HCI mode into connection library power mode */
            switch (ev->mode)
            {
                case HCI_BT_MODE_ACTIVE:
                    message->mode = lp_active;
                    break;
                case HCI_BT_MODE_SNIFF:
                    message->mode = lp_sniff;
                    break;
                default:
                    Panic();
                    break;
            }
            message->interval = ev->length;
            MessageSend(task, CL_DM_MODE_CHANGE_EVENT, message);
        }
    }
}

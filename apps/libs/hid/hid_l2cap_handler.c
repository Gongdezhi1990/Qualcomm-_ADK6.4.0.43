/****************************************************************************
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    hid_l2cap_handler.c
DESCRIPTION
    Handles the L2CAP messages from the connection library.
*/

#include <assert.h>

#include "hid.h"
#include "hid_init.h"
#include "hid_l2cap_handler.h"
#include "hid_common.h"
#include "hid_connect.h"
#include "hid_profile_handler.h"

/****************************************************************************
NAME
    hidHandleL2capRegisterCfm
DESCRIPTION
    This function is called on receipt of an CL_L2CAP_REGISTER_CFM.
*/
void hidHandleL2capRegisterCfm(HID_LIB *hidLib, const CL_L2CAP_REGISTER_CFM_T *cfm)
{
    HID_PRINT(("hidHandleL2capRegisterCfm\n"));

    /* Check if registration was successful */
    if (cfm->status == success)
    {
        /* Set correct PSM initialised flag */
        if (cfm->psm == HID_PSM_CONTROL)
            hidLib->control_psm = TRUE;
        else if (cfm->psm == HID_PSM_INTERRUPT)
            hidLib->interrupt_psm = TRUE;
        else
            Panic();

        /* Send internal init confirm (only sent if initialisation complete) */
        hidSendInternalInitCfm(hidLib, hid_init_success);
    }
    else
    {
        /* Send internal init confirm */
        hidSendInternalInitCfm(hidLib, hid_init_l2cap_reg_fail);
    }
}

/****************************************************************************
NAME
    hidHandleL2capConnectInd
DESCRIPTION
    This function is called on receipt of a CL_L2CAP_CONNECT_IND message.
    This message indicates that a remote device is attempting to establish
    an L2CAP connection to this device.
*/
void hidHandleL2capConnectInd(HID_LIB *hid_lib, const CL_L2CAP_CONNECT_IND_T *ind)
{
    HID *hid;
    HID_PRINT(("hidHandleL2capConnectInd, psm=%d\n", ind->psm));

    hid = hidFindFromBddr(hid_lib, &ind->bd_addr);
    if (hid !=NULL && ind->psm == HID_PSM_CONTROL)
    {
       /* Reject incoming connection when we are already connecting with the remote and recieve a connect indication */
        ConnectionL2capConnectResponse(&hid_lib->task, FALSE, ind->psm, ind->connection_id, ind->identifier, 0, NULL);
    }

    if (hid == NULL)
        hid = hidCreate(hid_lib, &ind->bd_addr);
    if (hid != NULL)
    {
        MAKE_HID_MESSAGE(HID_INTERNAL_CONNECT_IND);

        /* Send message to HID instance */
        message->bd_addr = ind->bd_addr;
        message->psm = ind->psm;
        message->connection_id = ind->connection_id;
        message->identifier = ind->identifier;
        MessageSend(&hid->task, (ind->psm == HID_PSM_CONTROL) ? HID_INTERNAL_CONNECT_CONTROL_IND : HID_INTERNAL_CONNECT_INTERRUPT_IND, message);
    }
    else
    {
        /* Reject incoming connection */
        ConnectionL2capConnectResponse(&hid_lib->task, FALSE, ind->psm, ind->connection_id, ind->identifier, 0, NULL);
    }
}

/****************************************************************************
NAME
    hidHandleL2capConnectCfm
DESCRIPTION
    This function is called on receipt of a CL_L2CAP_CONNECT_CFM message
    indicating the outcome of the connect attempt.
*/
void hidHandleL2capConnectCfmRemote(HID *hid, const CL_L2CAP_CONNECT_CFM_T *cfm)
{
    HID_PRINT(("hidHandleL2capConnectCfmRemote, hid=%p, psm=%d, mtu=%d, status=%d\n", (void *)hid, cfm->psm_local, cfm->mtu_remote, cfm->status));

    /* Check if sucessful connection */
    if (cfm->status == l2cap_connect_success)
    {
        /* Move connection to connected state */
        hidConnConnected(hid, cfm->psm_local, cfm->sink, cfm->mtu_remote);

        /* Check if both interrupt and control channels connected */
        if (hidConnIsConnected(hid, HID_PSM_CONTROL) && hidConnIsConnected(hid, HID_PSM_INTERRUPT))
        {
            /* Move to connected state */
            hidSetState(hid, hidConnected);
        }
        else if (hidConnIsConnected(hid, HID_PSM_CONTROL) && hidConnIsDisconnected(hid, HID_PSM_INTERRUPT))
        {
            /* Start connection timer */
            MessageSendLater(&hid->task, HID_INTERNAL_CONNECT_TIMEOUT_IND, 0, D_SEC(HID_CONNECT_TIMEOUT));
        }
    }
    else if (cfm->status >= l2cap_connect_failed)
    {
        /* Store connect failure reason */
        hid->connect_status = hid_connect_failed;

        /* Move connection to disconnected state */
        hidConnDisconnectedPsm(hid, cfm->psm_local);

        /* Check if we are already disconnected */
        if (hidConnIsDisconnected(hid, HID_PSM_CONTROL) && hidConnIsDisconnected(hid, HID_PSM_INTERRUPT))
        {
            /* Move to disconnected state */
            hidSetState(hid, hidDisconnected);
        }
        else
        {
            /* Move to disconnecting failed state */
            hidSetState(hid, hidDisconnectingFailed);
        }
    }
    /* Ignores l2cap_connect_pending status */
}

/****************************************************************************
NAME
    hidHandleL2capConnectCfmLocal
DESCRIPTION
    This function is called on receipt of a CL_L2CAP_CONNECT_CFM message
    indicating the outcome of the connect attempt.
*/
void hidHandleL2capConnectCfmLocal(HID *hid, const CL_L2CAP_CONNECT_CFM_T *cfm)
{
    HID_PRINT(("hidHandleL2capConnectCfmLocal, hid=%p, psm=%d, mtu=%d, status=%d\n", (void *)hid, cfm->psm_local, cfm->mtu_remote, cfm->status));

    /* Check if connection pending */
    if (cfm->status == l2cap_connect_pending)
    {
        /* do nothing yet... */
    }
    /* Check if sucessful connection */
    else if (cfm->status == l2cap_connect_success)
    {
        /* Move connection to connected state */
        hidConnConnected(hid, cfm->psm_local, cfm->sink, cfm->mtu_remote);

        /* Check if both interrupt and control channels connected */
        if (hidConnIsConnected(hid, HID_PSM_CONTROL) && hidConnIsConnected(hid, HID_PSM_INTERRUPT))
        {
            /* Move to connected state */
            hidSetState(hid, hidConnected);
        }
        else if (hidConnIsConnected(hid, HID_PSM_CONTROL) && hidConnIsDisconnected(hid, HID_PSM_INTERRUPT))
        {
            hidConnConnecting(hid, HID_PSM_INTERRUPT, 0, 0);

            /* Connect L2CAP interrupt channel using application specified conftab */
            ConnectionL2capConnectRequest(&hid->task, &hid->remote_addr,
                                          HID_PSM_INTERRUPT, HID_PSM_INTERRUPT,
                                          hid->hid_conftab_length, hid->hid_conftab);
        }
    }
    else if (cfm->status >= l2cap_connect_failed)
    {
        /* Store connect failure reason */
        hid->connect_status = hid_connect_failed;

        /* Move connection to disconnected state */
        hidConnDisconnectedPsm(hid, cfm->psm_local);

        /* Check if we are already disconnected */
        if (hidConnIsDisconnected(hid, HID_PSM_CONTROL) && hidConnIsDisconnected(hid, HID_PSM_INTERRUPT))
        {
            /* Move to disconnected state */
            hidSetState(hid, hidDisconnected);
        }
        else
        {
            /* Move to disconnecting failed state */
            hidSetState(hid, hidDisconnectingFailed);
        }
    }
    /* Ignores l2cap_connect_pending status */
}


/****************************************************************************
NAME
    hidHandleL2capConnectCfmDisconnecting
DESCRIPTION
    This function is called on receipt of a CL_L2CAP_CONNECT_CFM message
    indicating the outcome of the connect attempt.
*/
void hidHandleL2capConnectCfmDisconnecting(HID *hid, const CL_L2CAP_CONNECT_CFM_T *cfm)
{
    HID_PRINT(("hidHandleL2capConnectCfmDisconnecting, hid=%p, psm=%d, mtu=%d, status=%d\n", (void *)hid, cfm->psm_local, cfm->mtu_remote, cfm->status));

    /* Check if sucessful connection */
    if (cfm->status == l2cap_connect_success)
    {
        /* Move connection to connected state */
        hidConnConnected(hid, cfm->psm_local, cfm->sink, cfm->mtu_remote);

        /* Disconnect connection */
        hidConnDisconnect(hid, cfm->psm_local);
    }
    else if (cfm->status >= l2cap_connect_failed)
    {
        /* Move connection to disconnected state */
        hidConnDisconnectedPsm(hid, cfm->psm_local);

        /* Check if we are already disconnected */
        if (hidConnIsDisconnected(hid, HID_PSM_CONTROL) && hidConnIsDisconnected(hid, HID_PSM_INTERRUPT))
        {
            /* Move to disconnected state */
            hidSetState(hid, hidDisconnected);
        }
    }
    /* Ignores l2cap_connect_pending status */
}

/****************************************************************************
NAME
    hidHandleL2capDisconnectIndConnecting
DESCRIPTION
    This function is called on receipt of a CL_L2CAP_DISCONNECT_IND message.
    This message indicates that a L2CAP connection has been disconnected.
*/
void hidHandleL2capDisconnectIndConnecting(HID *hid, const CL_L2CAP_DISCONNECT_IND_T *ind)
{
    HID_PRINT(("hidHandleL2capDisconnectIndConnecting, hid=%p\n", (void *)hid));

    /* Store connect status */
    hid->connect_status = hid_connect_disconnected;

    /* Mark connection as disconnected */
    hidConnDisconnected(hid, ind->sink);

    /* Respond to disconnect indication */
    ConnectionL2capDisconnectResponse(ind->identifier, ind->sink);

    /* Check if we are already disconnected */
    if (hidConnIsDisconnected(hid, HID_PSM_CONTROL) && hidConnIsDisconnected(hid, HID_PSM_INTERRUPT))
    {
        /* Move to disconnected state */
        hidSetState(hid, hidDisconnected);
    }
    else
    {
        /* Move to disconnecting failed state */
        hidSetState(hid, hidDisconnectingFailed);
    }
}

/****************************************************************************
NAME
    hidHandleL2capDisconnectInd
DESCRIPTION
    This function is called on receipt of a CL_L2CAP_DISCONNECT_IND message.
    This message indicates that a L2CAP connection has been disconnected.
*/
void hidHandleL2capDisconnectIndConnected(HID *hid, const CL_L2CAP_DISCONNECT_IND_T *ind)
{
    HID_PRINT(("hidHandleL2capDisconnectIndConnected, hid=%p\n", (void *)hid));

    /* Store disconnect status */
    switch (ind->status)
    {
        case l2cap_disconnect_successful:
            hid->disconnect_status = hid_disconnect_success;
            break;
        case l2cap_disconnect_timed_out:
            hid->disconnect_status = hid_disconnect_timeout;
            break;
        case l2cap_disconnect_link_loss:
            hid->disconnect_status = hid_disconnect_link_loss;
            break;
        case l2cap_disconnect_error:
        default:
            hid->disconnect_status = hid_disconnect_error;
            break;
    }

    /* Mark connection as disconnected */
    hidConnDisconnected(hid, ind->sink);

    /* Respond to disconnect indication */
    ConnectionL2capDisconnectResponse(ind->identifier, ind->sink);

    /* Move to disconnecting remote state */
    hidSetState(hid, hidDisconnectingRemote);
}

/****************************************************************************
NAME
    hidHandleL2capDisconnectIndDisconnecting
DESCRIPTION
    This function is called on receipt of a CL_L2CAP_DISCONNECT_IND message.
    This message indicates that a L2CAP connection has been disconnected.
*/
void hidHandleL2capDisconnectIndDisconnecting(HID *hid, const CL_L2CAP_DISCONNECT_IND_T *ind)
{
    HID_PRINT(("hidHandleL2capDisconnectIndDisconnecting, hid=%p\n", (void *)hid));

    /* Mark connection as disconnected */
    hidConnDisconnected(hid, ind->sink);

    /* Respond to disconnect indication */
    ConnectionL2capDisconnectResponse(ind->identifier, ind->sink);

    /* Check if both connections are now disconnected */
    if (hidConnIsDisconnected(hid, HID_PSM_CONTROL) && hidConnIsDisconnected(hid, HID_PSM_INTERRUPT))
    {
        /* Move to disconnected state */
        hidSetState(hid, hidDisconnected);
    }
}

/****************************************************************************
NAME
    hidHandleL2capDisconnectCfmDisconnecting
DESCRIPTION
    This function is called on receipt of a CL_L2CAP_DISCONNECT_CFM message.
    This message indicates that a L2CAP disconnection which was initated by a local host has been completed.
*/
void hidHandleL2capDisconnectCfmDisconnecting(HID *hid, const CL_L2CAP_DISCONNECT_CFM_T *cfm)
{
    HID_PRINT(("hidHandleL2capDisconnectCfmDisconnecting, hid=%p\n", (void *)hid));

    /* Mark connection as disconnected */
    hidConnDisconnected(hid, cfm->sink);

    /* Check if both connections are now disconnected */
    if (hidConnIsDisconnected(hid, HID_PSM_CONTROL) && hidConnIsDisconnected(hid, HID_PSM_INTERRUPT))
    {
        /* Move to disconnected state */
        hidSetState(hid, hidDisconnected);
    }
}

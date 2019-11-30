/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_con_manager.c
\brief      Connection Manager State Machine
*/

#include "av_headset.h"
#include "av_headset_sdp.h"
#include "av_headset_log.h"
#include "av_headset_con_manager.h"

#include <message.h>
#include <panic.h>
#include <app/bluestack/dm_prim.h>

/*! @{ Macros to make connection manager messages. */
#define MAKE_CONMAN_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);
#define MAKE_PRIM_T(TYPE) MESSAGE_MAKE(prim,TYPE##_T); prim->type = TYPE;
#define MAKE_PRIM_C(TYPE) MESSAGE_MAKE(prim,TYPE##_T); prim->common.op_code = TYPE; prim->common.length = sizeof(TYPE##_T);
/*! @} */

static void appConManagerSetDeviceState(conManagerDevice *device, conManagerAclState state)
{
    device->state = state;
    device->lock = (state & ACL_STATE_LOCK);

    DEBUG_LOGF("appConManagerSetDeviceState, device %x,%x,%lx, state %u, lock %u",
               device->addr.nap, device->addr.uap, device->addr.lap,
               device->state, device->lock);
}

static conManagerDevice *appConManagerFindDeviceFromBdAddr(const bdaddr *addr)
{
    conManagerTaskData *theConMgr = appGetConManager();
    for (int i = 0; i < CON_MANAGER_MAX_DEVICES; i++)
    {
        conManagerDevice *device = &theConMgr->devices[i];
        if (BdaddrIsSame(&device->addr, addr))
            return device;
    }
    return NULL;
}


/*! \brief Check if there are any CONNECTED links.
*/
bool appConManagerAnyLinkConnected(void)
{
    conManagerTaskData *theConMgr = appGetConManager();

    for (int i = 0; i < CON_MANAGER_MAX_DEVICES; i++)
    {
        conManagerDevice *device = &theConMgr->devices[i];
        if (device->state == ACL_CONNECTED)
        {
            DEBUG_LOGF("appConManagerAnyLinkConnected %04X %02X %06lX IS CONNECTED",
                device->addr.nap,device->addr.uap,device->addr.lap);
            return TRUE;
        }
    }
    return FALSE;
}


/*! \brief Query if a device is currently connected. */
bool appConManagerIsConnected(const bdaddr *addr)
{
    const conManagerDevice *device = appConManagerFindDeviceFromBdAddr(addr);
    return device ? device->state == ACL_CONNECTED : FALSE;
}

/*! \brief Query if a ACL to device was locally initiated. */
bool appConManagerIsAclLocal(const bdaddr *addr)
{
    const conManagerDevice *device = appConManagerFindDeviceFromBdAddr(addr);
    return device ? device->local : FALSE;
}

/*! \brief Set or clear locally initiated ACL flag for a device. */
static void appConManagerSetAclLocal(const bdaddr *addr, bool local)
{
    conManagerDevice *device = appConManagerFindDeviceFromBdAddr(addr);
    if (device)
        device->local = local;
}

static conManagerDevice *appConManageGetNewDevice(const bdaddr *addr, conManagerAclState state, bool is_local, bool is_ble)
{
    conManagerTaskData *theConMgr = appGetConManager();
    for (int i = 0; i < CON_MANAGER_MAX_DEVICES; i++)
    {
        conManagerDevice *device = &theConMgr->devices[i];
        if (BdaddrIsZero(&device->addr))
        {
            device->addr = *addr;
            appConManagerSetDeviceState(device, state);
            device->users = 0;
            device->local = is_local;
            device->ble = is_ble;
            device->lpState.pt_index = POWERTABLE_UNASSIGNED;
            return device;
        }
    }
    return NULL;
}

/*! \brief Add (or update) device to devices list. */
static conManagerDevice *appConManagerAddDevice(const bdaddr *addr, conManagerAclState state, bool is_local, bool is_ble)
{
    conManagerDevice *device = appConManagerFindDeviceFromBdAddr(addr);
    if (device)
    {
        appConManagerSetDeviceState(device, state);
        return device;
    }
    else
    {
        device = appConManageGetNewDevice(addr, state, is_local, is_ble);
        if (device == NULL)
        {
            conManagerTaskData *theConMgr = appGetConManager();

            /* Flush any devices in ACL_DISCONNECTED_LINK_LOSS state */
            for (int i = 0; i < CON_MANAGER_MAX_DEVICES; i++)
            {
                device = &theConMgr->devices[i];
                if (device->state == ACL_DISCONNECTED_LINK_LOSS)
                {
                    appConManagerSetDeviceState(device, ACL_DISCONNECTED);
                    BdaddrSetZero(&device->addr);
                }
            }
            device = appConManageGetNewDevice(addr, state, is_local, is_ble);
        }

        if (device)
        {
            return device;
        }

        /* No space */
        Panic();
    }

    return NULL;
}

/*! \brief Remove device from the connected devices list. */
static bool appConManagerRemoveDevice(const bdaddr *addr)
{
    conManagerDevice *device = appConManagerFindDeviceFromBdAddr(addr);
    if (device)
    {
        appConManagerSetDeviceState(device, ACL_DISCONNECTED);
        BdaddrSetZero(&device->addr);
        device->users = 0;
        return TRUE;
    }
    else
    {
        DEBUG_LOGF("appConManagerRemoveDevice, %x,%x,%lx, not found!",
                   addr->nap, addr->uap, addr->lap);
        return FALSE;
    }
}

/* Send DM_ACL_CLOSE_REQ */
void appConManagerSendCloseAclRequest(const bdaddr *addr, bool force)
{
    if (force)
        ConnectionDmAclDetach(addr, HCI_ERROR_OETC_USER, FALSE);
    else
        ConnectionDmAclClose(addr, FALSE);
}


/* Implementation of create ACL function. 
   This includes a BLE flag so can be used to create BLE and BREDR connections
   if needed. */
static uint16 *appConManagerCreateAclImpl(const bdaddr *addr, bool is_ble)
{
    uint32 page_timeout = appConfigDefaultPageTimeout();

    /* setup page timeout depending on the type of device the connection is for */
    if (appDeviceIsPeer(addr))
    {
        page_timeout = appConfigIsLeft() ? appConfigLeftEarbudPageTimeout() :
                                           appConfigRightEarbudPageTimeout();
    }

    /* Attempt to find existing device */
    conManagerDevice *device = appConManagerFindDeviceFromBdAddr(addr);
    if (device)
    {
        if (device->state != ACL_DISCONNECTED_LINK_LOSS)
        {
            device->users += 1;
            DEBUG_LOGF("appConManagerCreateAclImpl, %x,%x,%lx, found device, state %u, lock %u, users %u",
                       device->addr.nap, device->addr.uap, device->addr.lap,
                       device->state, device->lock, device->users);

            /* Return pointer to lock, may or may not be set depending on ACL state */
            return &device->lock;
        }
        else
        {
            /* Increase page timeout as device was previously disconnected due to link-loss */
            page_timeout *= appDeviceIsPeer(addr) ? appConfigEarbudLinkLossPageTimeoutMultiplier() :
                                                    appConfigHandsetLinkLossPageTimeoutMultiplier();
            if (page_timeout > 0xFFFF)
                page_timeout = 0xFFFF;

            DEBUG_LOGF("appConManagerCreateAclImpl, link-loss device, increasing page timeout to %u ms", page_timeout * 625UL / 1000UL);

            /* Reset device state */
            appConManagerSetDeviceState(device, ACL_DISCONNECTED);
        }
    }

    /* Create new device */
    device = appConManagerAddDevice(addr, ACL_CONNECTING, TRUE, is_ble);
    device->users += 1;

    DEBUG_LOGF("appConManagerCreateAclImpl, %x,%x,%lx, create device, state %u, lock %u, users %u",
               device->addr.nap, device->addr.uap, device->addr.lap,
               device->state, device->lock, device->users);


    /* Temporary direct use of DM_PRIMs until connection library API is created.
     * there is an API for page timeout, but we want to ensure it is set before using
     * the DM_ACK_OPEN_REQ. */
    {
        MAKE_PRIM_C(DM_HCI_WRITE_PAGE_TIMEOUT_REQ);
        prim->page_timeout = (uint16)page_timeout;
        VmSendDmPrim(prim);
    }
    {
        /* Send DM_ACL_OPEN_REQ to open ACL manually */
        MAKE_PRIM_T(DM_ACL_OPEN_REQ);
        prim->addrt.type = TBDADDR_PUBLIC;
        prim->flags = 0;
        BdaddrConvertVmToBluestack(&prim->addrt.addr, &device->addr);
        VmSendDmPrim(prim);
    }

    /* Return pointer to lock, will always be set */
    return &device->lock;
}


uint16 *appConManagerCreateAcl(const bdaddr *addr)
{
    return appConManagerCreateAclImpl(addr, FALSE);
}


/*! \brief Release ownership on ACL */
void appConManagerReleaseAcl(const bdaddr *addr)
{
    /* Attempt to find existing device */
    conManagerDevice *device = appConManagerFindDeviceFromBdAddr(addr);
    if (device)
    {
        if (device->users)
            device->users -= 1;

        DEBUG_LOGF("appConManagerReleaseAcl, %x,%x,%lx, state %u, lock %u, users %u",
                   device->addr.nap, device->addr.uap, device->addr.lap,
                   device->state, device->lock, device->users);

        if (!device->users)
            appConManagerSendCloseAclRequest(addr, FALSE);
    }
}

/*! \brief Send CON_MANAGER_CONNECTION_IND message to all registered clients. */
static void appConManagerMsgConnectionInd(const bdaddr *addr, bool connected, bool ble,
                                          hci_status reason)
{
    conManagerTaskData *theConMgr = appGetConManager();

    MAKE_CONMAN_MESSAGE(CON_MANAGER_CONNECTION_IND);
    message->bd_addr = *addr;
    message->connected = connected;
    message->ble = ble;
    message->reason = reason;
    appTaskListMessageSend(theConMgr->connection_client_tasks, CON_MANAGER_CONNECTION_IND, message);
}

/*! \brief Handle service search attributes confirmation message

    Handles the results of a service search attributes request.  If the request
    was successful, the TWS+ version is extracted and stored in the device database.
    If there was no search data it is assumed the device is a standard device.
*/
static void appHandleClSdpServiceSearchAttributeCfm(conManagerTaskData *theConMgr,
                                                    const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm)
{
    UNUSED(theConMgr);

    DEBUG_LOGF("appHandleClSdpServiceSearchAttributeCfm, status %d", cfm->status);

    if (cfm->status == sdp_response_success)
    {
        /* Received response, so extract TWS version from attribute */
        uint16 tws_version = DEVICE_TWS_STANDARD;
        appSdpFindTwsVersion(cfm->attributes, cfm->attributes + cfm->size_attributes, &tws_version);
        appDeviceSetTwsVersion(&cfm->bd_addr, tws_version);

        DEBUG_LOGF("appHandleClSdpServiceSearchAttributeCfm, TWS+ device %x,%x,%lx, version %d",
                     cfm->bd_addr.nap, cfm->bd_addr.uap, cfm->bd_addr.lap, tws_version);
    }
    else if (cfm->status == sdp_no_response_data)
    {
        DEBUG_LOGF("appHandleClSdpServiceSearchAttributeCfm, standard device %x,%x,%lx",
                     cfm->bd_addr.nap, cfm->bd_addr.uap, cfm->bd_addr.lap);

        /* No response data, so handset doesn't have UUID and/or version attribute, therefore
           treat as standard handset */
        appDeviceSetTwsVersion(&cfm->bd_addr, DEVICE_TWS_STANDARD);
    }

    /* Attempt to find existing device */
    conManagerDevice *device = appConManagerFindDeviceFromBdAddr(&cfm->bd_addr);
    if (device)
    {
        /* Check device in in ACL_CONNECTED_SDP_SEARCH state */
        if (device->state == ACL_CONNECTED_SDP_SEARCH)
        {
            /* Indicate to clients the connection to this device is up */
            appConManagerMsgConnectionInd(&device->addr, TRUE, device->ble, hci_success);

            /* Move to ACL_CONNECTED state */
            appConManagerSetDeviceState(device, ACL_CONNECTED);
        }
    }
}

/*  ACL opened indication handler

    If a new ACL is opened successfully and it is to a handset (where the TWS+
    version needs to be checked everytime) a service attribute search is started.
*/
static void appConManagerHandleClDmAclOpenedIndication(const CL_DM_ACL_OPENED_IND_T *ind)
{
    DEBUG_LOGF("appConManagerHandleClDmAclOpenedIndication, status %d, incoming %u, %x,%x,%lx[%d] Flags:%x",
               ind->status, (ind->flags & DM_ACL_FLAG_INCOMING) ? 1 : 0,
               ind->bd_addr.addr.nap, ind->bd_addr.addr.uap, ind->bd_addr.addr.lap,ind->bd_addr.type,ind->flags);

    if (ind->status == hci_success)
    {
        const bool is_local = !!(~ind->flags & DM_ACL_FLAG_INCOMING);
        const bool is_ble = !!(ind->flags & DM_ACL_FLAG_ULP);

        /* Update local ACL flag */
        appConManagerSetAclLocal(&ind->bd_addr.addr, is_local);

        /* Set default link supervision timeout if locally inititated (i.e. we're master) */
        if (is_local)
            appLinkPolicyUpdateLinkSupervisionTimeout(&ind->bd_addr.addr);

        /* Check if this is a BREDR BDADDR for handset */
        if (   !(ind->flags & DM_ACL_FLAG_ULP) 
            && appDeviceIsHandset(&ind->bd_addr.addr))
        {
            DEBUG_LOG("appConManagerHandleClDmAclOpenedIndication, handset");

#ifndef DISABLE_TWS_PLUS
            /* Add this device to list of connected devices */
            appConManagerAddDevice(&ind->bd_addr.addr, ACL_CONNECTED_SDP_SEARCH, is_local, is_ble);

            /* Perform service search for TWS+ Source UUID and version attribute */
            appConManagerQueryHandsetTwsVersion(&ind->bd_addr.addr);
#else
            /* TWS+ disabled so assume so handset is standard */
            appDeviceSetTwsVersion(&ind->bd_addr.addr, DEVICE_TWS_STANDARD);

            /* Add this device to list of connected devices */
            appConManagerAddDevice(&ind->bd_addr.addr, ACL_CONNECTED, is_local, is_ble);

            /* Indicate to clients the connection to this device is up */
            appConManagerMsgConnectionInd(&ind->bd_addr.addr, TRUE, is_ble, hci_success);
#endif
        }
        else
        {
            DEBUG_LOG("appConManagerHandleClDmAclOpenedIndication, peer or unknown");

            /* Add this device to list of connected devices */
            appConManagerAddDevice(&ind->bd_addr.addr, ACL_CONNECTED, is_local, is_ble);

            /* For non-handset devices indicate to clients the connection to this
             * device is up */
            appConManagerMsgConnectionInd(&ind->bd_addr.addr, TRUE, is_ble, hci_success);
        }
    }
    else if (ind->status == hci_error_max_nr_of_acl)
    {
        DEBUG_LOG("appConManagerHandleClDmAclOpenedIndication, status %d, waiting");
    }
    else
    {
        /* Remove this device from list of connected devices */
        appConManagerRemoveDevice(&ind->bd_addr.addr);
    }
}

/*! \brief ACL closed indication handler
*/
static void appConManagerHandleClDmAclClosedIndication(const CL_DM_ACL_CLOSED_IND_T *ind)
{
    bool    was_ble = FALSE;

    DEBUG_LOGF("appConManagerHandleClDmAclClosedIndication, status %d, %x,%x,%lx", ind->status, ind->taddr.addr.nap, ind->taddr.addr.uap, ind->taddr.addr.lap);

    /* Check if this BDADDR is for handset */
    if (   TYPED_BDADDR_PUBLIC == ind->taddr.type
        && appDeviceIsHandset(&ind->taddr.addr))
    {
        DEBUG_LOG("appConManagerHandleClDmAclClosedIndication, handset");
    }

    /* If connection timeout/link-loss move to special disconnected state, so that re-opening ACL
     * will use longer page timeout */
    conManagerDevice *device = appConManagerFindDeviceFromBdAddr(&ind->taddr.addr);
    if (device)
    {
        was_ble = device->ble;
    }

    if (ind->status == hci_error_conn_timeout && device)
    {
        appConManagerSetDeviceState(device, ACL_DISCONNECTED_LINK_LOSS);
        device->users = 0;
    }
    else
    {
        /* Remove this device from list of connected devices */
        appConManagerRemoveDevice(&ind->taddr.addr);
    }

    /* Indicate to client the connection to this device has gone */
    appConManagerMsgConnectionInd(&ind->taddr.addr, FALSE, was_ble, ind->status);
}

/*! \brief Handle authentication.
 */
static void appConManagerHandleClSmAuthoriseIndication(const CL_SM_AUTHORISE_IND_T *ind)
{
    conManagerTaskData *theConMgr = appGetConManager();
    bool authorise = FALSE;
    bool ble_connection = (ind->protocol_id == protocol_le_l2cap);

    DEBUG_LOGF("appConManagerHandleClSmAuthoriseIndication, protocol %d, channel %d, incoming %d BLE:%d",
                 ind->protocol_id, ind->channel, ind->incoming, ble_connection);

    /* Only deal with BREDR connections */
    if (!ble_connection)
    {
        /* Always allow connection from peer */
        if (appDeviceIsPeer(&ind->bd_addr))
        {
            DEBUG_LOG("appConManagerHandleClSmAuthoriseIndication, ALLOW peer");
            authorise = TRUE;
        }
        else 
        {
            bool handset = appDeviceIsHandset(&ind->bd_addr);

            if (handset && theConMgr->handset_connect_allowed)
            {
                bdaddr handset_bd_addr, peer_handset_bd_addr;
                bool peer_sync = appPeerSyncIsComplete();
                bool peer_handset_tws = appPeerSyncIsPeerHandsetTws();
                bool peer_handset;

                appDeviceGetHandsetBdAddr(&handset_bd_addr);
                appPeerSyncGetPeerHandsetAddr(&peer_handset_bd_addr);
                peer_handset = !BdaddrIsZero(&peer_handset_bd_addr);

                DEBUG_LOGF("appConManagerHandleClSmAuthoriseIndication incoming: %x,%x,%lx", ind->bd_addr.nap, ind->bd_addr.uap, ind->bd_addr.lap);
                DEBUG_LOGF("appConManagerHandleClSmAuthoriseIndication handset:  %x,%x,%lx", handset_bd_addr.nap, handset_bd_addr.uap, handset_bd_addr.lap);
                DEBUG_LOGF("appConManagerHandleClSmAuthoriseIndication peer:     %x,%x,%lx", peer_handset_bd_addr.nap, peer_handset_bd_addr.uap, peer_handset_bd_addr.lap);

                if ((ind->protocol_id == protocol_l2cap) && (ind->channel == 1))
                {
                    /* SDP L2CAP, always allow */
                    DEBUG_LOG("c, ALLOW handset SDP (bredr)");
                    authorise = TRUE;
                }
                else if (appDeviceIsHandsetConnected() && BdaddrIsSame(&handset_bd_addr, &ind->bd_addr))
                {
                    /* NB: appDeviceIsHandsetConnected() will be set on an ACL connection */
                    DEBUG_LOGF("appConManagerHandleClSmAuthoriseIndication peer_sync %d peer_handset %d peer_handset_tws %d",
                               peer_sync, peer_handset, peer_handset_tws);

                    /* Check the status of the peer handset*/
                    if (peer_sync && peer_handset)
                    {
                        if (peer_handset_tws && BdaddrIsSame(&peer_handset_bd_addr, &ind->bd_addr))
                        {
                            DEBUG_LOG("appConManagerHandleClSmAuthoriseIndication, ALLOW same TWS+ handset as peer (bredr)");
                            authorise = TRUE;
                        }
                        else
                        {
                            DEBUG_LOG("appConManagerHandleClSmAuthoriseIndication, REJECT peer has TWS handset connected (bredr)");
                            authorise = FALSE;
                        }
                    }
                    else
                    {
                        DEBUG_LOG("appConManagerHandleClSmAuthoriseIndication, ALLOW same handset (bredr)");
                        authorise = TRUE;
                    }

                }
                else
                {
                    DEBUG_LOG("appConManagerHandleClSmAuthoriseIndication, REJECT as a different handset (bredr)");
                    authorise = FALSE;
                }
            }
            else
            {
                DEBUG_LOG("appConManagerHandleClSmAuthoriseIndication, REJECT. not peer or allowed:%d handset:%d (bredr)",
                            theConMgr->handset_connect_allowed, handset);
            }
        }
    }
    else
    {
        DEBUG_LOG("appConManagerHandleClSmAuthoriseIndication, REJECT as BLE");
    }

    ConnectionSmAuthoriseResponse(&ind->bd_addr, ind->protocol_id, ind->channel, ind->incoming, authorise);
}


bool appConManagerHandleConnectionLibraryMessages(MessageId id,Message message, bool already_handled)
{
    switch (id)
    {
        case CL_SM_AUTHORISE_IND:
            if (!already_handled)
            {
                appConManagerHandleClSmAuthoriseIndication((CL_SM_AUTHORISE_IND_T *)message);
            }
            return TRUE;

        case CL_DM_ACL_OPENED_IND:
            appConManagerHandleClDmAclOpenedIndication((CL_DM_ACL_OPENED_IND_T *)message);
            return TRUE;
        
        case CL_DM_ACL_CLOSED_IND:
            appConManagerHandleClDmAclClosedIndication((CL_DM_ACL_CLOSED_IND_T *)message);
            return TRUE;

        case CL_DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND:
        {
            CL_DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND_T *ind = (CL_DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND_T *)message;
        
            ConnectionDmBleAcceptConnectionParUpdateResponse(TRUE, &ind->taddr,
                                                             ind->id,
                                                             ind->conn_interval_min, ind->conn_interval_max,
                                                             ind->conn_latency,
                                                             ind->supervision_timeout);
            return TRUE;
        }

    }
    return already_handled;
}



/*! \brief Perform SDP query of TWS version of handset
*/
void appConManagerQueryHandsetTwsVersion(const bdaddr *bd_addr)
{
    conManagerTaskData *theConMgr = appGetConManager();
    ConnectionSdpServiceSearchAttributeRequest(&theConMgr->task, bd_addr, 0x32,
                                               appSdpGetTwsSourceServiceSearchRequestSize(), appSdpGetTwsSourceServiceSearchRequest(),
                                               appSdpGetTwsSourceAttributeSearchRequestSize(), appSdpGetTwsSourceAttributeSearchRequest());
}

/*! \brief Connection manager message handler.
 */
static void appConManagerHandleMessage(Task task, MessageId id, Message message)
{
    conManagerTaskData *theConMgr = (conManagerTaskData *)task;

    switch (id)
    {
        case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
            appHandleClSdpServiceSearchAttributeCfm(theConMgr, (CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *)message);
            return;

        case PAIRING_HANDSET_PAIR_CFM:
            return;
    }
}

/*! \brief Configure role switch policy.
 
    Currently we want to never automatically ask for a role switch nor
    refuse a request for a role switch.
 */
static void appConManagerSetupRoleSwitchPolicy(void)
{
    static uint16 connectionDmRsTable = 0;
    MAKE_PRIM_T(DM_LP_WRITE_ROLESWITCH_POLICY_REQ);
    prim->version = 0;
    prim->length = 1;
    prim->rs_table = (uint16 *)&connectionDmRsTable;
    VmSendDmPrim(prim);
}

/*! \brief Initialise the connection manager module.
 */
void appConManagerInit(void)
{
    conManagerTaskData *theConMgr = appGetConManager();
    
    DEBUG_LOG("appConManagerInit");

    memset(theConMgr, 0, sizeof(*theConMgr));

    /* Set up task handler */
    theConMgr->task.handler = appConManagerHandleMessage;

    /* create a task list to track tasks interested in connection
     * event indications */
    theConMgr->connection_client_tasks = appTaskListInit();

    /* setup role switch policy */
    appConManagerSetupRoleSwitchPolicy();
}

/*! \brief Register a client task to receive notifications of connections.
    \param[in] client_task Task which will receive CON_MANAGER_CONNECTION_IND message
 */
void appConManagerRegisterConnectionsClient(Task client_task)
{
    conManagerTaskData *theConMgr = appGetConManager();
    appTaskListAddTask(theConMgr->connection_client_tasks, client_task);
}

/*! \brief Set the link policy per-connection state. */
void appConManagerSetLpState(const bdaddr *addr, const lpPerConnectionState *lpState)
{
    conManagerDevice *device = appConManagerFindDeviceFromBdAddr(addr);
    if (device)
    {
        device->lpState = *lpState;
    }
}

/*! \brief Get the link policy per-connection state. */
void appConManagerGetLpState(const bdaddr *addr, lpPerConnectionState *lpState)
{
    conManagerDevice *device = appConManagerFindDeviceFromBdAddr(addr);
    if (device)
    {
        *lpState = device->lpState;
    }
}


bool appConManagerLpStateSame(const lpPerConnectionState *state_a,
                              const lpPerConnectionState *state_b)
{
    return    state_a->pt_index == state_b->pt_index
           && state_a->table_set_used == state_b->table_set_used;
}


/*! \brief Control if handset connections are allowed. */
void appConManagerAllowHandsetConnect(bool allowed)
{
    conManagerTaskData *theConMgr = appGetConManager();
    theConMgr->handset_connect_allowed = allowed;
}


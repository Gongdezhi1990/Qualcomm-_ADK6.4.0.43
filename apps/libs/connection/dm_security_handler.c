/****************************************************************************
Copyright (c) 2004 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_security_handler.c

DESCRIPTION
    This files contains the implementation of the Security Entity API for
    the Connection Library.  The Application Task and/or Profile Libraries
    make calls to these functions to manage all Bluetooth security related
    functionality.  In order to correctly manage the Connection Library
    state machine calls to these API's result in a private message being
    posted to the main Connection Library task handler.  Depending upon the
    current state of the Connection Library these messages are handled
    appropriately.

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "init.h"
#include "connection_tdl.h"
#include "dm_security_handler.h"

#include <bdaddr.h>
#include <string.h>
#include <vm.h>
#include <sink.h>
#include <stream.h>
#include <ps.h>
#include <vmal.h>
#include <message.h>

#define AUTH_BITS_SET(AUTH_REQ, BITS) \
    bool auth_bits_set = (AUTH_REQ & BITS) ? TRUE:FALSE;


/*****************************************************************************/
static void aclOpen(const bdaddr* bd_addr)
{
    MAKE_PRIM_T(DM_ACL_OPEN_REQ);
    prim->addrt.type = TBDADDR_PUBLIC;
    BdaddrConvertVmToBluestack(&prim->addrt.addr, bd_addr);
    VmSendDmPrim(prim);
}


/*****************************************************************************/
static void aclClose(const bdaddr* bd_addr, uint16 flags, uint8 reason)
{
    MAKE_PRIM_T(DM_ACL_CLOSE_REQ);
    prim->addrt.type = TBDADDR_PUBLIC;
    BdaddrConvertVmToBluestack(&prim->addrt.addr, bd_addr);
    prim->flags=flags;
    prim->reason=reason;
    VmSendDmPrim(prim);
}

/****************************************************************************
NAME
    startBonding

DESCRIPTION
    This function is called to send a DM_SM_BONDING_REQ to Bluestack

RETURNS

*/
static void startBonding(const bdaddr* bd_addr)
{
    MAKE_PRIM_T(DM_SM_BONDING_REQ);
    prim->addrt.type = TBDADDR_PUBLIC;
    BdaddrConvertVmToBluestack(&prim->addrt.addr, bd_addr);
    VmSendDmPrim(prim);
}


/****************************************************************************
NAME
    cancelBonding

DESCRIPTION
    This function is called to send a DM_SM_BONDING_CANCEL_REQ to Bluestack

RETURNS

*/
static void cancelBonding(const bdaddr* bd_addr, bool force)
{
    MAKE_PRIM_T(DM_SM_BONDING_CANCEL_REQ);

    prim->addrt.type = TBDADDR_PUBLIC;
    BdaddrConvertVmToBluestack(&prim->addrt.addr, bd_addr);

    if (force)
        prim->flags = DM_ACL_FLAG_FORCE;
    else
        prim->flags = 0;

    VmSendDmPrim(prim);
}


/****************************************************************************
NAME
    connectionSendAuthenticateCfm

DESCRIPTION
    This function is called to send a CL_SM_AUTHENTICATE_CFM message to the
    specified task.  This indicates that a pairing operation has completed.
    The message confirms the Bluetooth Device Address of the device just
    paired with and a result code

RETURNS

*/
static void connectionSendAuthenticateCfm(
                Task task,
                const bdaddr* bd_addr,
                authentication_status status,
                cl_sm_link_key_type key_type,
                bool bonded
                )
{
    /*
        This message indicates that a pairing operation has completed.
        The message confirms the Bluetooth device address of the device just
        paired with and a result code indicating the outcome of the pairing
        attempt.
    */
    if (task)
    {
        MAKE_CL_MESSAGE(CL_SM_AUTHENTICATE_CFM);
        message->bd_addr = *bd_addr;
        message->status = status;
        message->key_type = key_type;
        message->bonded = bonded;
        MessageSend(task, CL_SM_AUTHENTICATE_CFM, message);
    }
}


/****************************************************************************
    This function is called when a CL_INTERNAL_SM_SET_SC_MODE_CFM arrives at
    the CL with a destination task of the CL.  This will occur as a result of
    DM_SM_SET_SEC_MODE_REQ being sent from within the CL.  This message
    is sent during the Authentication process.
*/
static void handleInternalSetSecurityModeCfm(
                connectionSmState* smState,
                const DM_SM_INIT_CFM_T* cfm
                )
{
    /* Check to determine if Authentication is active */
    if(smState->authReqLock)
    {
        /* If the Peer Bluetooth Device Address is non zero then establish
           an ACL connection to start the Authentication Process */
        if(cfm->status == HCI_SUCCESS)
        {
            /* If the Peer Bluetooth Device Address is non zero then...*/
            if(!BdaddrIsZero(&smState->authRemoteAddr))
                aclOpen(&smState->authRemoteAddr);
            else
                /* Reset the Authentication request lock */
                smState->authReqLock = 0;
        }
    }
}


/*****************************************************************************/
static void endAuthentication(
                connectionSmState* smState,
                authentication_status status
                )
{
    /* If Authentication has failed then let the application know about it */
    if(status != auth_status_success)
    {
        connectionSendAuthenticateCfm(
                smState->authReqLock,
                &smState->authRemoteAddr,
                status,
                cl_sm_link_key_none,
                FALSE
                );
    }
    /* If Authentication timed out, ensure ACL connection is closed */
    if(status == auth_status_timeout)
    {
        if( (smState->security_mode != sec_mode4_ssp)
                && (smState->security_mode != sec_mode2_service) )
            aclClose(&smState->authRemoteAddr, 0, 0);
        else
            cancelBonding(&smState->authRemoteAddr, FALSE);
    }
    else
    {
        /* Cancel any pending authentication timeout */
        (void) MessageCancelFirst(
                    connectionGetCmTask(),
                    CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND
                    );
    }

    /* Zero storage of peer device */
    BdaddrSetZero(&smState->authRemoteAddr);

    /* Restore the security settings, the fact that the peer Bluetooth Device
       Address is zero is used to decode the fact that Authentication has
       ended. The resulting CFM handler will reset the Authentication Lock. */
    if( (smState->security_mode != sec_mode4_ssp)
            && (smState->security_mode != sec_mode2_service) )
        ConnectionSmSetSecurityMode(
                connectionGetCmTask(),
                smState->security_mode,
                smState->enc_mode
                );
    else
        smState->authReqLock = 0;

}


/****************************************************************************/
static void sendAclOpenedIndToClient(
                    Task task,
                    TYPED_BD_ADDR_T *addr,
                    uint16 flags,
                    uint24_t dev_class,
                    hci_status status,
                    DM_ACL_BLE_CONN_PARAMS_T *ble
                    )
{
    const msg_filter *msgFilter = connectionGetMsgFilter();

    if (task && (msgFilter[0] & msg_group_acl))
    {
        MAKE_CL_MESSAGE(CL_DM_ACL_OPENED_IND);

        BdaddrConvertTypedBluestackToVm(&message->bd_addr, addr);
        message->incoming = (flags & DM_ACL_FLAG_INCOMING) ? TRUE : FALSE;
        message->dev_class = dev_class;
        message->status = status;
        message->flags = flags;

        if (flags & DM_ACL_FLAG_ULP)
        {
            message->conn_interval       = ble->conn_interval;
            message->conn_latency        = ble->conn_latency;
            message->supervision_timeout = ble->supervision_timeout;
            message->clock_accuracy      = ble->clock_accuracy;
        }
        else
        {
            message->conn_interval       = 0;
            message->conn_latency        = 0;
            message->supervision_timeout = 0;
            message->clock_accuracy      = 0;
        }

        MessageSend(task, CL_DM_ACL_OPENED_IND, message);
    }
}

/****************************************************************************
NAME
    connectionHandleSmAddDeviceCfm

DESCRIPTION
    This function is called whenever a device is added to the Bluestack
    Security Managers device database and the connection library is
    initialising.

RETURNS

*/
void connectionHandleSmAddDeviceCfm(
        connectionSmState* smState,
        const DM_SM_ADD_DEVICE_CFM_T* cfm
        )
{
    /*
       During initialisation we expect to see a CFM for every device added to
       the database from the entries in the trusted device list
    */
    if(cfm->status == HCI_SUCCESS)
    {
        smState->deviceCount++;
        if(smState->deviceCount == smState->noDevices)
        {
            /* Confirm Trusted Device List has been initialised */
            connectionSendInternalInitCfm(connectionInitComplete);
        }
    }
}

/****************************************************************************
NAME
    connectionHandleSmAddDeviceCfmReady

DESCRIPTION
    This function is called whenever a device is added to the Bluestack
    Security Managers device database and the connection library has been
    initialised.

RETURNS

*/
void connectionHandleSmAddDeviceCfmReady(
        connectionSmState* smState,
        const DM_SM_ADD_DEVICE_CFM_T* cfm
        )
{
    if (smState->deviceReqLock)
    {
        bdaddr  addr;

        /* Convert from CSR to CCL format */
        BdaddrConvertBluestackToVm(&addr, &cfm->addrt.addr);

        /* Send message to application */
        {
            MessageId id =
                (smState->deviceReqType == device_req_add_authorised) ?
                    CL_SM_ADD_AUTH_DEVICE_CFM : CL_SM_SET_TRUST_LEVEL_CFM;

            /* CL_SM_ADD_AUTH_DEVICE_CFM has the same structure as
             * CL_SM_SET_TRUST_LEVEL_CFM.
             */
            MAKE_CL_MESSAGE(CL_SM_ADD_AUTH_DEVICE_CFM);
            message->bd_addr = addr;
            message->status = (cfm->status) ? fail : success;
            MessageSend(
                    smState->deviceReqLock,
                    id,
                    message
                    );
        }
    }

    /* Reset lock */
    smState->deviceReqLock = 0;
}


/****************************************************************************
NAME
    connectionHandleSmAccessInd

DESCRIPTION
    This function is called on receipt of a DM_SM_ACCESS_IND from Bluestack

RETURNS

*/
void connectionHandleSmAccessInd(
        connectionSdpState* sdpState,
        const DM_SM_ACCESS_IND_T* ind
        )
{
    bdaddr bd_addr;
    BdaddrConvertBluestackToVm(&bd_addr, &ind->conn_setup.connection.addrt.addr);

    /* If this is not an indication for SDP and the address matches one we're
     * searching...
     */
    if  (
         !(
           (
            ind->conn_setup.connection.service.protocol_id == SEC_PROTOCOL_L2CAP 
            ||
            ind->conn_setup.connection.service.protocol_id == SEC_PROTOCOL_LE_L2CAP
           )
           &&
           ind->conn_setup.connection.service.channel == 1
          )
          &&
          BdaddrIsSame(&bd_addr, &sdpState->sdpServerAddr)
        )
    {
        /* If the connection lib has a search open we're SDP Pinging */
        if(sdpState->sdpLock == connectionGetCmTask())
        {
            /* If we are waiting for a search result... */
            if(sdpState->sdpSearchLock == connectionGetCmTask())
            {
                /* Free the search lock so the result is binned and we don't
                 * send a new request.
                */
                sdpState->sdpSearchLock = 0;
            }
            else
            {
                /* If we're not waiting for a result there must be a pending
                 * message .
                 */
                MessageCancelFirst
                    (connectionGetCmTask(),
                     CL_INTERNAL_SDP_SERVICE_SEARCH_REQ
                     );
                sdpState->sdpSearchLock = 0;
            }

        /* Close the search */
        ConnectionSdpCloseSearchRequest(connectionGetCmTask());
        }
    }
}

/****************************************************************************
NAME
    connectionHandleSmKeyReqInd

DESCRIPTION
    This function is called on receipt of a DM_SM_KEY_REQUEST_IND
    from the Bluestack Security Manager.

    Bluestack only sends this if the SM database does not contain the link
    key. As we always register devices this means that we do not really have
    the link key. For BR/EDR - reject the request. For BLE - echo back the
    security requirements.

RETURNS

*/
void connectionHandleSmKeyReqInd(const DM_SM_KEY_REQUEST_IND_T* ind)
{
    MAKE_PRIM_T(DM_SM_KEY_REQUEST_NEG_RSP);

    prim->addrt    = ind->addrt;
    prim->key_type = ind->key_type;

    if (ind->key_type == DM_SM_KEY_ENC_BREDR)
    {
        if(!ind->rhsf && ind->initiated_outgoing)
        {
            bdaddr  addr;
            BdaddrConvertBluestackToVm(&addr, &ind->addrt.addr);

            ConnectionSdpOpenSearchRequest(connectionGetCmTask(), &addr);
        }
    }

    VmSendDmPrim(prim);
}


/****************************************************************************
NAME
    handleBredrKey

DESCRIPTION
    For a BR/EDR key, work out if it is bonding and interact with the
    BR/EDR authentication state machine.

RETURNS
    bool - indicates if the device is bonded or not.

*/
static bool handleBredrKey(
        Task                            theAppTask,
        connectionSmState               *smState,
        const BD_ADDR_T                 *bd_addr,
        const DM_SM_KEY_ENC_BREDR_T     *enc_bredr
        )
{
    bool bonded = FALSE;

    cl_sm_link_key_type key_type =
        connectionConvertLinkKeyType( enc_bredr->link_key_type );

    bdaddr    addr;

    /* Check if we're bonding */
    AUTH_BITS_SET(
            smState->authentication_requirements,
            ( HCI_MITM_NOT_REQUIRED_GENERAL_BONDING |
              HCI_MITM_NOT_REQUIRED_DEDICATED_BONDING |
              AUTH_REQ_UNKNOWN )
            );


    /* If legacy pairing or bonding (and not debug key) store key */
    if  (
        (key_type == cl_sm_link_key_legacy) ||
        (auth_bits_set && (key_type != cl_sm_link_key_debug))
        )
    {
        bonded = TRUE;
    }

    /* For legacy devs this is where authentication ends */
    if  (
        ( ( smState->security_mode == sec_mode4_ssp ) ||
                ( smState->security_mode == sec_mode2_service ) ) &&
                key_type == cl_sm_link_key_legacy
        )
    {
        endAuthentication(smState, auth_status_success);
    }

    /* Reset authentication requirements + send cfm to app */
    smState->authentication_requirements = AUTH_REQ_UNKNOWN;

    BdaddrConvertBluestackToVm(&addr, bd_addr);
    connectionSendAuthenticateCfm(
            theAppTask,
            &addr,
            auth_status_success,
            key_type,
            bonded
            );

    return bonded;
}
/****************************************************************************
NAME
    connectionGetSinksFromBdAddr

DESCRIPTION
    Returns pointer to array of pointers of Sinks for tpaddr
    *number_sinks contains number of elements in returned array.
    Caller must free returned array.

RETURNS

*/
static Sink* connectionGetSinksFromBdAddr(
        uint16* number_sinks,
        tp_bdaddr *tpaddr)
{
    /* Try and get the streams, increase sink_list size until it is big
       enough or a panic occurs.
     */
    Sink *sink_list = NULL;
    uint16 num_sinks;

    for(num_sinks = 5; ; num_sinks += 5)
    {
        sink_list = (Sink *)PanicNull(calloc(num_sinks, sizeof(Sink)));
        if(StreamSinksFromBdAddr(&num_sinks, sink_list, tpaddr))
        {
            *number_sinks = num_sinks;
            return sink_list;
        }
        free(sink_list);
    }
}



/****************************************************************************
NAME
    handleLeKey

DESCRIPTION
    For LE link, send the link security SC/non-SC to the application.

RETURNS
    void.

*/
#ifndef DISABLE_BLE
static void handleLeKey(
        Task   theAppTask,    
        const bdaddr* bd_addr,
        const uint16_t  security_requirements
        )
{
    if (theAppTask)
    {        
        MAKE_CL_MESSAGE(CL_SM_BLE_LINK_SECURITY_IND);
        message->bd_addr = *bd_addr;
        message->le_link_sc = security_requirements & DM_SM_SECURITY_SECURE_CONNECTIONS ? TRUE : FALSE;
        MessageSend(theAppTask, CL_SM_BLE_LINK_SECURITY_IND, message);
    }
}
#endif

/****************************************************************************
NAME
    connectionHandleSmKeysInd

DESCRIPTION
    This function is called on receipt of a DM_SM_KEYS_IND
    from the Bluestack Security Manager. This indicates that a pairing
    operation has been completed.  
    
    If bonding, the link keys will be stored.
    - For BR/EDR this is determined by the smState->authentication_requirements
      bits set (Authentication/Authorisation), and the type of BR/EDR link key.
    - For LE this is determined by the keys.security_requirements containing
      DM_SM_SECURITY_BONDING flag.

    NOTE: For Cross Key Derivation of the BR/EDR link key, if there are no
    smState->authentication_requirements bits set (AUTH_REQ_UNKNOWN), then
    the BLE keys.security_requirements flag for DM_SM_SECURITY_BONDING is used
    to determine if bonding, and therefore storing the link key.

RETURNS

*/
void connectionHandleSmKeysInd(
            Task theAppTask,
            connectionSmState   *smState,
            const DM_SM_KEYS_IND_T    *ind
            )
{
    uint16                  key_idx;
    uint16                  new_idx=0;
    bool                    bonded = FALSE;
    uint16                  key_type = DM_SM_KEY_NONE;
    const TYPED_BD_ADDR_T   *addrt_to_use;
#ifndef DISABLE_BLE
    bool                    le_key= FALSE;    
#endif    

    /* Use calloc to 0 all fields */
    DM_SM_ADD_DEVICE_REQ_T *prim =
        (DM_SM_ADD_DEVICE_REQ_T *)
            PanicNull( calloc( 1, sizeof(DM_SM_ADD_DEVICE_REQ_T) ) );

    /* Work out which address field from the DM_SM_KEYS_IND message to use.
     * If 'id_addrt' is all zero, then privacy is not used and the 'addrt'
     * address indicates the public/static address of the remote device.
     * Otherwise, the 'id_addrt' indicates the public/static address.
     */
    if  (
            !ind->id_addrt.addr.lap &&
            !ind->id_addrt.addr.uap &&
            !ind->id_addrt.addr.nap
        )
    {
        addrt_to_use = &ind->addrt;
    }
    else
    {
        addrt_to_use = &ind->id_addrt;
        /* Allocate memory for caching the Permanent address.
         * Bluestack shall send DM_SM_KEYS_IND twice during CTKD, in that
         * case the permanent address should be cached only once.
         */ 
        if (smState->permanent_taddr == NULL) 
        {
            /* Allocate memory for caching the Permanent address.*/
            smState->permanent_taddr = (typed_bdaddr*)
                PanicUnlessMalloc(sizeof(typed_bdaddr));

            BdaddrConvertTypedBluestackToVm(
                    smState->permanent_taddr, 
                    &ind->id_addrt
                    );
        }
    }

    prim->type = DM_SM_ADD_DEVICE_REQ;
    prim->addrt = *addrt_to_use;
    prim->trust = DM_SM_TRUST_UNCHANGED;
    prim->privacy = DM_SM_PRIVACY_UNCHANGED;

    prim->keys.security_requirements = ind->keys.security_requirements;
    prim->keys.encryption_key_size = ind->keys.encryption_key_size;

    for (key_idx=0; key_idx<DM_SM_MAX_NUM_KEYS; key_idx++)
    {
        key_type = ind->keys.present >> (DM_SM_NUM_KEY_BITS * key_idx)
                    & DM_SM_KEY_MASK;


        /* Convert keys from handles to pointers. Could use up to 5 slots BUT
         * In practice the maximum is 2 and probably only ever 1 at a time.
         */
        switch(key_type)
        {
            case DM_SM_KEY_ENC_BREDR:
                prim->keys.u[new_idx].enc_bredr =
                    (DM_SM_KEY_ENC_BREDR_T *) (
                        PanicNull(
                            VmGetPointerFromHandle(
                                ind->keys.u[key_idx].enc_bredr
                                )
                            )
                        );

                prim->keys.present |=
                    DM_SM_KEY_ENC_BREDR << (DM_SM_NUM_KEY_BITS * new_idx++);

                break;

            case DM_SM_KEY_ENC_CENTRAL:
                prim->keys.u[new_idx].enc_central =
                    (DM_SM_KEY_ENC_CENTRAL_T *) (
                        PanicNull(
                            VmGetPointerFromHandle(
                                ind->keys.u[key_idx].enc_central
                                )
                            )
                        );

                prim->keys.present |=
                    DM_SM_KEY_ENC_CENTRAL << (DM_SM_NUM_KEY_BITS * new_idx++);

                break;

            /* DIV is a uint16 so no memory needs to be allocated for
             * handling it BUT it does need to be stored so that the
             * next DIV can be calculated after a power cycle.
             */
            case DM_SM_KEY_DIV:
                PsStore(
                    PSKEY_SM_DIV_STATE,
                    &ind->keys.u[key_idx].div,
                    PS_SIZE_ADJ(sizeof(uint16_t))
                    );

                prim->keys.u[new_idx].div = ind->keys.u[key_idx].div;

                prim->keys.present |=
                    DM_SM_KEY_DIV << (DM_SM_NUM_KEY_BITS * new_idx++);

                break;

            case DM_SM_KEY_ID:
                prim->keys.u[new_idx].id =
                    (DM_SM_KEY_ID_T *) (
                        PanicNull(
                            VmGetPointerFromHandle(
                                ind->keys.u[key_idx].id
                                )
                            )
                        );

                prim->keys.present |=
                    DM_SM_KEY_ID << (DM_SM_NUM_KEY_BITS * new_idx++);

                break;

            case DM_SM_KEY_SIGN:
                prim->keys.u[new_idx].sign =
                    (DM_SM_KEY_SIGN_T *) (
                        PanicNull(
                            VmGetPointerFromHandle(
                                ind->keys.u[key_idx].sign
                                )
                            )
                        );

                prim->keys.present |=
                    DM_SM_KEY_SIGN << (DM_SM_NUM_KEY_BITS * new_idx++);

                break;

            /* Nothing to be done for DM_SM_KEY_NONE. */
            default:
                break;
        }

        if (key_type == DM_SM_KEY_ENC_BREDR)
        {
            /* If security is being done through the SM then this is for GATT
             * over BR/EDR and the keys.security_requirements flags indicate
             * if bonding is being done, not the authentication requirement 
             * settings.
             */
            if (smState->sm_security)
            {
                /* for GATT over BR/EDR or CTKD the key type can never be
                 * LEGACY or DEBUG.
                 */ 
                bdaddr    addr;
                cl_sm_link_key_type cl_key_type; 

                bonded = 
                    (ind->keys.security_requirements & DM_SM_SECURITY_BONDING)
                        ? TRUE : FALSE;

                cl_key_type =
                    connectionConvertLinkKeyType(
                            prim->keys.u[key_idx].enc_bredr->link_key_type
                            );
                
                BdaddrConvertBluestackToVm(&addr, &addrt_to_use->addr);

                connectionSendAuthenticateCfm(
                        theAppTask,
                        &addr,
                        auth_status_success,
                        cl_key_type,
                        bonded
                        );
            }
            else /* This is BR/EDR authentication and bonding. */
            {
               bonded = handleBredrKey(
                   theAppTask,
                   smState,
                   &addrt_to_use->addr,
                   prim->keys.u[key_idx].enc_bredr
                   );
            }
            /* if non-bonding and there is already a bonded link key for
             * this device then delete it from the TDL (if not a priority
             * device).
             */
            if (!bonded)
            {
                connectionAuthDeleteDeviceFromTdl(addrt_to_use);
            }
        }
        /* For BLE, check the keys.security_requirements for the BONDING flag.
         */
        else if (key_type != DM_SM_KEY_NONE)
        {
            bonded = (ind->keys.security_requirements & DM_SM_SECURITY_BONDING) 
                ? TRUE : FALSE;
#ifndef DISABLE_BLE            
            le_key = TRUE;
#endif            
        }
    }

#ifndef DISABLE_BLE
    if(le_key)
    {
        bdaddr addr;

        BdaddrConvertBluestackToVm(&addr, &addrt_to_use->addr);	 
        handleLeKey(
                theAppTask,
                &addr,
                prim->keys.security_requirements
                );
    }
#endif
    /* 
        If not bonded we will not be sending the message.

        Save some processing time by wrapping the final message prep
        within the same if-block.
    */
    if (bonded)
    {
        prim->keys.present |= DM_SM_KEYS_UPDATE_EXISTING;

        connectionAuthUpdateTdl(addrt_to_use, &prim->keys);

        /* Convert the Key VM slot memory pointers to firmware handles */
        for (key_idx=0; key_idx<DM_SM_MAX_NUM_KEYS; key_idx++)
        {
            key_type = prim->keys.present >> (DM_SM_NUM_KEY_BITS * key_idx)
                        & DM_SM_KEY_MASK;

            if (key_type != DM_SM_KEY_DIV && key_type != DM_SM_KEY_NONE)
                prim->keys.u[key_idx].none =
                    VmGetHandleFromPointer(prim->keys.u[key_idx].none);
        }

        VmSendDmPrim(prim);
    }
    else /* Free the BS Message we have been constructing. */
    {
        /* First free the individual link key memory allocated from handles. */
        for (key_idx=0; key_idx<DM_SM_MAX_NUM_KEYS; key_idx++)
        {
            key_type = prim->keys.present >> (DM_SM_NUM_KEY_BITS * key_idx)
                        & DM_SM_KEY_MASK;

            if (key_type != DM_SM_KEY_DIV && key_type != DM_SM_KEY_NONE)
                free(prim->keys.u[key_idx].none);
        }

        /* Now free the prim iteself. */
        free(prim);
    }
}


/****************************************************************************
NAME
    connectionHandleSmBondingCfm

DESCRIPTION
    This function is called on receipt of a DM_SM_BONDING_CFM
    from the Bluestack Security Manager.  This indicates that a pairing
    operation has been completed.  For a successful pairing complete,
    connection Library would have received DM_SM_KEY_IND before this message
    and completed the Authentication procedure. Handle this message only in
    failure case since Bluestack will not send a DM_SM_KEY_IND.

RETURNS

*/
void connectionHandleSmBondingCfm(
        connectionSmState* smState,
        const DM_SM_BONDING_CFM_T* cfm
        )
{
     bdaddr    addr;

     BdaddrConvertBluestackToVm(&addr, &cfm->addrt.addr);

    /* Check to determine if Authentication is active.
     * This message is expected only on ssp_mode4 & service_mode2
     */
    if(smState->authReqLock &&
            ( (smState->security_mode == sec_mode4_ssp)
                    || (smState->security_mode == sec_mode2_service) ))
    {
        authentication_status status =
            connectionConvertAuthStatus(cfm->status);
        /* Success cases, Authentication ends with legacy devices on receiving
         * the DM_SM_KEY_IND. End the Authentication only for failure.
         */
        if(status != auth_status_success)
        {
            /* Check Authentication failed with the right remote device*/
             if(BdaddrIsSame(&smState->authRemoteAddr, &addr))
                endAuthentication(smState, status);
        }
    }
}

/****************************************************************************
NAME
    connectionHandleSmPinReqInd

DESCRIPTION
    This function is called on receipt of a DM_SM_PIN_REQUEST_IND
    from the Bluestack Security Manager.

RETURNS

*/
void connectionHandleSmPinReqInd(
        Task theAppTask,
        const DM_SM_PIN_REQUEST_IND_T* ind
        )
{
    /* Send a message to the Application requesting a PIN code */
    MAKE_CL_MESSAGE(CL_SM_PIN_CODE_IND);
    BdaddrConvertTypedBluestackToVm(&message->taddr, &ind->addrt);
    MessageSend(theAppTask, CL_SM_PIN_CODE_IND, message);
}


/****************************************************************************
NAME
    connectionHandleSmIoCapReqInd

DESCRIPTION
    This function is called on receipt of a DM_SM_IO_CAPABILITY_REQUEST_IND
    from the Bluestack Security Manager.

RETURNS

*/
void connectionHandleSmIoCapReqInd(
        Task theAppTask,
        connectionSmState* smState,
        const DM_SM_IO_CAPABILITY_REQUEST_IND_T* ind
        )
{
    MAKE_CL_MESSAGE(CL_SM_IO_CAPABILITY_REQ_IND);

    smState->sm_security =
        (ind->flags & DM_SM_FLAGS_SECURITY_MANAGER) ? TRUE : FALSE;

    BdaddrConvertTpBluestackToVm(&message->tpaddr, &ind->tp_addrt);

    message->sm_over_bredr =
	(ind->flags & DM_SM_FLAGS_SMP_OVER_BREDR) ? TRUE : FALSE;	

    MessageSend(theAppTask, CL_SM_IO_CAPABILITY_REQ_IND, message);
}

/****************************************************************************
NAME
    connectionHandleSmIoCapResInd

DESCRIPTION
    This function is called on receipt of a DM_SM_IO_CAPABILITY_RESPONSE_IND
    from the Bluestack Security Manager.

RETURNS

*/
void connectionHandleSmIoCapResInd(
        Task theAppTask,
        connectionSmState* smState,
        const DM_SM_IO_CAPABILITY_RESPONSE_IND_T* ind
        )
{
    MAKE_CL_MESSAGE(CL_SM_REMOTE_IO_CAPABILITY_IND);

    message->authentication_requirements =
        connectionConvertAuthenticationRequirements(
                ind->authentication_requirements
                );
    {

        /* Check we don't already have an internal auth requirement */
        AUTH_BITS_SET(smState->authentication_requirements, AUTH_REQ_UNKNOWN);

        /* If remote dev initiated */
        if(auth_bits_set)
        {
            /* Remember it's requirements (for the dedicated bonding case) */
            smState->authentication_requirements =
                                    ind->authentication_requirements;
        }
    }

    /* If security is being handled by SM, NOT LM, and this device is NOT the
     * initiator (it is the responder).
     */
    smState->responder =    (
                            (ind->flags & DM_SM_FLAGS_SECURITY_MANAGER) &&
                            !(ind->flags & DM_SM_FLAGS_INITIATOR)
                            ) ? TRUE : FALSE;

    if  (smState->responder)
        /* Store the key_distribution for the Io Capability Request Response. */
        smState->key_distribution = ind->key_distribution;

    message->io_capability = connectionConvertIoCapability(ind->io_capability);
    message->oob_data_present = ind->oob_data_present;

    BdaddrConvertTpBluestackToVm(&message->tpaddr, &ind->tp_addrt);

    MessageSend(theAppTask, CL_SM_REMOTE_IO_CAPABILITY_IND, message);
}


/****************************************************************************
NAME
    connectionHandleSmUserConfirmationReqInd

DESCRIPTION
    This function is called on receipt of a DM_SM_USER_CONFIRMATION_REQUEST_IND
    from the Bluestack Security Manager.

RETURNS

*/
void connectionHandleSmUserConfirmationReqInd(
        Task theAppTask,
        const DM_SM_USER_CONFIRMATION_REQUEST_IND_T* ind
        )
{
    MAKE_CL_MESSAGE(CL_SM_USER_CONFIRMATION_REQ_IND);
    BdaddrConvertTpBluestackToVm(&message->tpaddr, &ind->tp_addrt);
    message->numeric_value = ind->numeric_value;
    message->response_required = ind->flags & DM_SM_FLAGS_RESPONSE_REQUIRED;
    MessageSend(theAppTask, CL_SM_USER_CONFIRMATION_REQ_IND,message);
}


/****************************************************************************
NAME
    connectionHandleSmUserPasskeyReqInd

DESCRIPTION
    This function is called on receipt of a DM_SM_USER_PASSKEY_REQUEST_IND
    from the Bluestack Security Manager.

RETURNS

*/
void connectionHandleSmUserPasskeyReqInd(
        Task theAppTask,
        const DM_SM_USER_PASSKEY_REQUEST_IND_T* ind
        )
{
    MAKE_CL_MESSAGE(CL_SM_USER_PASSKEY_REQ_IND);
    BdaddrConvertTpBluestackToVm(&message->tpaddr, &ind->tp_addrt);
    MessageSend(theAppTask, CL_SM_USER_PASSKEY_REQ_IND,message);
}


/****************************************************************************
NAME
    connectionHandleSmUserPasskeyNotificationInd

DESCRIPTION
    This function is called on receipt of a DM_SM_USER_PASSKEY_NOTIFICATION_IND
    from the Bluestack Security Manager.

RETURNS

*/
void connectionHandleSmUserPasskeyNotificationInd(
        Task theAppTask,
        const DM_SM_USER_PASSKEY_NOTIFICATION_IND_T* ind
        )
{
    MAKE_CL_MESSAGE(CL_SM_USER_PASSKEY_NOTIFICATION_IND);
    BdaddrConvertTpBluestackToVm(&message->tpaddr, &ind->tp_addrt);
    message->passkey = ind->passkey;
    MessageSend(theAppTask, CL_SM_USER_PASSKEY_NOTIFICATION_IND,message);
}


/****************************************************************************
NAME
    connectionHandleSmKeypressNotificationInd

DESCRIPTION
    This function is called on receipt of a DM_SM_KEYPRESS_NOTIFICATION_IND
    from the Bluestack Security Manager.

RETURNS

*/
void connectionHandleSmKeypressNotificationInd(
        Task theAppTask,
        const DM_SM_KEYPRESS_NOTIFICATION_IND_T* ind
        )
{
    MAKE_CL_MESSAGE(CL_SM_KEYPRESS_NOTIFICATION_IND);
    BdaddrConvertTpBluestackToVm(&message->tpaddr, &ind->tp_addrt);
    message->type = connectionConvertKeypressType(ind->notification_type);
    MessageSend(theAppTask, CL_SM_KEYPRESS_NOTIFICATION_IND,message);
}

/****************************************************************************
NAME
    connectionHandleSmChangeLinkKeyReq

DESCRIPTION
    This function is called on receipt of a CL_INTERNAL_SM_CHANGE_LINK_KEY_REQ
    message from the connection lib.

RETURNS

*/
void connectionHandleSmChangeLinkKeyReq(
        const CL_INTERNAL_SM_CHANGE_LINK_KEY_REQ_T* req
        )
{
    tp_bdaddr tpaddr;
    if (SinkGetBdAddr(req->sink, &tpaddr))
    {
        /* Send DM_HCI_CHANGE_LINK_KEY message to Bluestack */
        MAKE_PRIM_C(DM_HCI_CHANGE_CONN_LINK_KEY_REQ);
        BdaddrConvertVmToBluestack(&prim->bd_addr, &tpaddr.taddr.addr);
        VmSendDmPrim(prim);
    }
    else
        CL_DEBUG(("Invalid sink: %d\n", (int)req->sink));
}

/****************************************************************************
NAME
    connectionHandleSmAuthoriseInd

DESCRIPTION
    This function is called on receipt of a DM_SM_AUTHORISE_IND from the
    Bluestack Security Manager.

RETURNS

*/
void connectionHandleSmAuthoriseInd(
        Task theAppTask,
        const DM_SM_AUTHORISE_IND_T* ind
        )
{
    /* Send a message to the Application requesting authorisation for the
       device to connect */
    MAKE_CL_MESSAGE(CL_SM_AUTHORISE_IND);
    BdaddrConvertBluestackToVm(&message->bd_addr, &ind->cs.connection.addrt.addr);
    message->protocol_id =
        connectionConvertProtocolId_t(ind->cs.connection.service.protocol_id);
    message->channel = ind->cs.connection.service.channel;
    message->incoming = ind->cs.incoming;
    MessageSend(theAppTask, CL_SM_AUTHORISE_IND, message);
}


/****************************************************************************
NAME
    connectionHandleSmSimplePairingCompleteInd

DESCRIPTION
    This function is called on receipt of a DM_SM_SIMPLE_PAIRING_COMPLETE_IND
    from the Bluestack Security Manager.

RETURNS

*/
void connectionHandleSmSimplePairingCompleteInd(
        Task theAppTask,
        connectionSmState* smState,
        const DM_SM_SIMPLE_PAIRING_COMPLETE_IND_T* ind
        )
{
#ifndef DISABLE_BLE
    /* If this is using SM security then assume this is a BLE connection. */
    if (ind->flags & DM_SM_FLAGS_SECURITY_MANAGER)
    {
        /* Send the COMPLETE to the app */
        MAKE_CL_MESSAGE(CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND);
        message->status = (ind->status) ? fail : success;
        BdaddrConvertTpBluestackToVm(&message->tpaddr, &ind->tp_addrt);
        message->flags = ind->flags;

        if (smState->permanent_taddr)
        {
           message->permanent_taddr = *smState->permanent_taddr;
           free(smState->permanent_taddr);
           smState->permanent_taddr = 0;
        }
        else
        {
            BdaddrTypedSetEmpty(&message->permanent_taddr);
        }

        MessageSend(theAppTask, CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND, message);
    }
    else
#endif /* DISABLE_BLE */
    {
        bdaddr bd_addr; /* TODO: This may change if SPC is sent for BR/EDR */
        BdaddrConvertBluestackToVm(&bd_addr, &ind->tp_addrt.addrt.addr);

        if(ind->status != HCI_SUCCESS)
        {
            if(!smState->authReqLock)
                connectionSendAuthenticateCfm(
                        theAppTask,
                        &bd_addr,
                        connectionConvertAuthStatus(ind->status),
                        cl_sm_link_key_none,
                        FALSE
                        );

            /* Authentication finished, forget the requirements */
            smState->authentication_requirements = AUTH_REQ_UNKNOWN;
        }

        if(smState->authReqLock)
        {
            endAuthentication(smState, connectionConvertAuthStatus(ind->status));
        }
    }
}


/****************************************************************************
NAME
    connectionHandleDmAclOpenCfm

DESCRIPTION
    This function is called when confirmation that a previously requested
    open ACL connection has completed
RETURNS

*/
void connectionHandleDmAclOpenCfm(
        connectionSmState* smState,
        const DM_ACL_OPEN_CFM_T* cfm
        )
{
    /* Check to determine if Authentication is active */
    if(smState->authReqLock)
    {
        /*
           We are currently in the process of performing Authentication with a
           remote device.  At this point if the ACL connection was successful
           then we have successfully paired
        */
        if(cfm->success)
        {
            bdaddr  addr;
            BdaddrConvertBluestackToVm(&addr, &cfm->addrt.addr);

            /* Provided this is the correct device */
            if(BdaddrIsSame(&smState->authRemoteAddr, &addr))
                /* Authentication successful, close the ACL link */
                aclClose(&smState->authRemoteAddr, 0, 0);
        }
        else
        {
            /* End Authentication and notify application task */
            endAuthentication(smState, auth_status_fail);
        }
    }
}


/*****************************************************************************/
void connectionHandleDmAclOpenInd(
        Task task,
        connectionSmState* smState,
        const DM_ACL_OPENED_IND_T *ind
        )
{
    sendAclOpenedIndToClient(
        task,
        (TYPED_BD_ADDR_T *) &ind->addrt,
        ind->flags,
        ind->dev_class,
        connectionConvertHciStatus(ind->status),
        (DM_ACL_BLE_CONN_PARAMS_T *) &ind->ble_conn_params
        );

    if(smState->authReqLock && ( (smState->security_mode == sec_mode4_ssp)
            || (smState->security_mode == sec_mode2_service) ))
    {
        authentication_status status =
                    connectionConvertAuthStatus(ind->status);
        /*
           We are currently in the process of performing Authentication with a
           remote device.  At this point if the ACL open failed authentication
           failed.
        */
        if(status == auth_status_timeout)
        {
            /* Cancel any pending authentication timeout */
            (void) MessageCancelFirst(
                        connectionGetCmTask(),
                        CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND
                        );
            /* End Authentication and notify application task */
            endAuthentication(smState, status);
        }
    }
}


/*****************************************************************************/
void connectionHandleDmAclClosedInd(
        Task task,
        connectionSmState *smState,
        const DM_ACL_CLOSED_IND_T *ind
        )
{
    typed_bdaddr taddr;
    const msg_filter *msgFilter = connectionGetMsgFilter();
    hci_status status =  connectionConvertHciStatus(ind->reason);

    BdaddrConvertTypedBluestackToVm(&taddr, &ind->addrt);

    /* Check to determine if Authentication is active  - BR/EDR connection */
    if(smState->authReqLock)
    {
        /* Authentication is active, an ACL connection to the peer device has
         * just been dropped, therefore Authentication has ended. */
        if(BdaddrIsSame(&smState->authRemoteAddr, &taddr.addr))
            /* In mode 4 we end auth on the bonding cfm and on mode-2 (legacy key) we
             * end it on receiving the link key. so don't handle here */
            if( (smState->security_mode != sec_mode4_ssp)
                    && (smState->security_mode != sec_mode2_service) )
            {
                /* End Authentication */
                endAuthentication(smState, auth_status_success);
            }
    }

    if (task && (msgFilter[0] & msg_group_acl))
    {
        MAKE_CL_MESSAGE(CL_DM_ACL_CLOSED_IND);
        message->taddr = taddr;
        message->status = status;
        MessageSend(task, CL_DM_ACL_CLOSED_IND, message);
    }
}

/****************************************************************************
NAME
    connectionHandleReadLocalOobDataReq

DESCRIPTION
    This function is called on receipt of a
    CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ message

RETURNS

*/
void connectionHandleReadLocalOobDataReq(
        connectionReadInfoState* infoState,
        connectionSmState* smState,
        const CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ_T* req
        )
{
    /* Check we can handle reading OOB data */
    if(infoState->version >= bluetooth2_1)
    {
        if(!smState->authReqLock)
        {
            /* Use the authReqLock to avoid getting new OOB data in the middle
               of authentication */
            smState->authReqLock = req->task;
            /* Send request to Bluestack */
            {
                MAKE_PRIM_T(DM_SM_READ_LOCAL_OOB_DATA_REQ);
                switch(req->transport)
                {
                    case TRANSPORT_BREDR_ACL:
                        prim->tp_type = BREDR_ACL;
                        break;
                    case TRANSPORT_BLE_ACL:
                        prim->tp_type = LE_ACL;
                        break;
                    default:
                        prim->tp_type = NO_PHYSICAL_TRANSPORT;
                        break;
                }
                VmSendDmPrim(prim);
            }
        }
        else
        {
            /* Authentication procedure in progress, post a conditional
             * internal message to queue request for processing later.
             */
            MAKE_CL_MESSAGE(CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ);
            COPY_CL_MESSAGE(req, message);
            MessageSendConditionallyOnTask(
                    connectionGetCmTask(),
                    CL_INTERNAL_SM_READ_LOCAL_OOB_DATA_REQ,
                    message,
                    &smState->authReqLock
                    );
        }
    }
    else
    {
        /* Send fail cfm to app - unsupported feature */
        MAKE_CL_MESSAGE(CL_SM_READ_LOCAL_OOB_DATA_CFM);
        message->status = hci_error_unsupported_feature;
        message->oob_hash_c[0] = 0;
        message->oob_rand_r[0] = 0;
        MessageSend(req->task, CL_SM_READ_LOCAL_OOB_DATA_CFM, message);
    }
}


/****************************************************************************
NAME
    connectionHandleReadLocalOobDataCfm

DESCRIPTION
    This function is called on receipt of a DM_SM_READ_LOCAL_OOB_DATA_CFM
    message.

RETURNS

*/
void connectionHandleReadLocalOobDataCfm(
        connectionSmState* smState,
        const DM_SM_READ_LOCAL_OOB_DATA_CFM_T* cfm
        )
{
    if(smState->authReqLock)
    {
        MAKE_CL_MESSAGE(CL_SM_READ_LOCAL_OOB_DATA_CFM);

        message->status = connectionConvertHciStatus(cfm->status);

        message->oob_data = oob_data_none;
        if (cfm->oob_data_present & DM_SM_OOB_DATA_P192)
            message->oob_data |= oob_data_p192;
        if (cfm->oob_data_present & DM_SM_OOB_DATA_P256)
            message->oob_data |= oob_data_p256;

        switch(cfm->tp_type) 
        {
            case BREDR_ACL:
                message->transport = TRANSPORT_BREDR_ACL;
                break;
            case LE_ACL:
                message->transport = TRANSPORT_BLE_ACL;
                break;
            default:
                message->transport = TRANSPORT_NONE;
                break;
        }

        if(cfm->status == HCI_SUCCESS)
        {
            /* Get the data from the primitive */
            uint8* oob_data = VmGetPointerFromHandle(cfm->oob_hash_c);
            memmove(message->oob_hash_c, oob_data, CL_SIZE_OOB_DATA);
            free(oob_data);

            oob_data = VmGetPointerFromHandle(cfm->oob_rand_r);
            memmove(message->oob_rand_r, oob_data, CL_SIZE_OOB_DATA);
            free(oob_data);
        }
        else
        {
            memset(message->oob_hash_c, 0, CL_SIZE_OOB_DATA);
            memset(message->oob_rand_r, 0, CL_SIZE_OOB_DATA);
        }

        MessageSend(
                smState->authReqLock,
                CL_SM_READ_LOCAL_OOB_DATA_CFM,
                message
                );
    }

    smState->authReqLock = 0;
}


/****************************************************************************
NAME
    connectionHandleAuthenticationReq

DESCRIPTION
    This function is called on receipt of a CM_AUTHENTICATE_REQ message

RETURNS

*/
void connectionHandleAuthenticationReq(
        connectionSmState* smState,
        const CL_INTERNAL_SM_AUTHENTICATION_REQ_T* req
        )
{
    if(!smState->authReqLock)
    {
        /* Lock Authentication */
        smState->authReqLock = req->task;

        /* Remove device from the trusted device list */
        (void) ConnectionSmDeleteAuthDevice(&req->bd_addr);

        /* Send a timeout message result in a timeout if Authentication fails.
         */
        {
            MAKE_CL_MESSAGE(CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND);
            message->theAppTask = req->task;
            MessageSendLater(
                    connectionGetCmTask(),
                    CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND,
                    message,
                    req->timeout
                    );
        }

        /* Store the Bluetooth device address of the device we are trying to
           Authenticate with */
        smState->authRemoteAddr = req->bd_addr;

        /* Set the security mode to SEC_MODE3_LINK (Wait for CFM) or
         * SEC_MODE4_SSP if 2.1 dev
         */
        if((smState->security_mode != sec_mode4_ssp)
                && (smState->security_mode != sec_mode2_service))
        {
            ConnectionSmSetSecurityMode(
                    connectionGetCmTask(),
                    sec_mode3_link,
                    smState->enc_mode
                    );
        }
        else
        {
            smState->authentication_requirements =
                            HCI_MITM_NOT_REQUIRED_DEDICATED_BONDING;
            startBonding(&smState->authRemoteAddr);
        }
    }
    else
    {
        /* Authentication procedure in progress, post a conditional internal
           message to queue request for processing later */
        CL_INTERNAL_SM_AUTHENTICATION_REQ_T *message =
            PanicUnlessNew(CL_INTERNAL_SM_AUTHENTICATION_REQ_T);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_SM_AUTHENTICATION_REQ,
                message,
                &smState->authReqLock
                );
    }
}


/****************************************************************************
NAME
    connectionHandleCancelAuthenticationReq

DESCRIPTION
    This function is called on receipt of a
    CL_INTERNAL_SM_CANCEL_AUTHENTICATION_REQ message.

RETURNS

*/
void connectionHandleCancelAuthenticationReq(
        connectionReadInfoState* infoState,
        connectionSmState* smState,
        const CL_INTERNAL_SM_CANCEL_AUTHENTICATION_REQ_T* req
        )
{
    /* Check the cancel request comes from the task that initiated bonding */
    if(smState->authReqLock == req->task)
    {
        /* Check firmware supports the command before we go any further */
        if( (infoState->version >= bluetooth2_1)
                || ( (infoState->version == bluetooth2_0)
                && (smState->security_mode == sec_mode2_service) ) )
        {
            /* Cancel bonding, locks will be tidied up on pairing complete */
            cancelBonding(&smState->authRemoteAddr, req->force);
        }
        else
        {
            /* Cancel bonding by closing the ACL */
            if(req->force)
                aclClose(&smState->authRemoteAddr, 0, 0);
        }
    }
}


/****************************************************************************
NAME
    connectionHandleAuthenticationTimeout

DESCRIPTION
    This function is called on receipt of a
    CL_INTERNAL_SM_AUTHENTICATION_TIMEOUT_IND message.  This indicates that
    that a request to pair to a remote device has timed out.  Let the source
    task know

RETURNS

*/
void connectionHandleAuthenticationTimeout(connectionSmState* smState)
{
    /* End Authentication and notify application task */
    endAuthentication(smState, auth_status_timeout);
}

/****************************************************************************
NAME
    handleSecurityInitReq

DESCRIPTION
    This function is called on receipt of an CL_INTERNAL_SM_INIT_REQ message,
    which only occurs when connection library is in the initialising state.

RETURNS

*/
void handleSecurityInitReq(
        connectionReadInfoState             *state,
        const CL_INTERNAL_SM_INIT_REQ_T     *req
        )
{
    MAKE_PRIM_T(DM_SM_INIT_REQ);

    prim->options = req->options;
    prim->config =  req->config;

    prim->security_level_default = 0; /* not set  here*/
    prim->security_mode = connectionConvertSecurityMode_t(req->security_mode);
    prim->write_auth_enable =
                   connectionConvertWriteAuthEnable_t(req->write_auth_enable);
    prim->mode3_enc = (uint8) req->mode3_enc;

#ifndef DISABLE_BLE
    if (state->version >= bluetooth4_0)

    {
        /* If this key is empty, then this device has not been run before.
         * Do not set the DM_SM_INIT_SM_STATE option so that IR and ER
         * are generated for the first time.
         */
        memset(&prim->sm_key_state, 0, sizeof(DM_SM_KEY_STATE_T));

        prim->sm_div_state = 0;

        /* SIGNING is not implemented. When it is, the 'Counter' field
         * will need to be retrieved from a PSKEY as well.
         */
        prim->sm_sign_counter = 0;

        if (
            PsRetrieve(
                PSKEY_SM_KEY_STATE_IR_ER,
                &prim->sm_key_state,
                PS_SIZE_ADJ(sizeof(DM_SM_KEY_STATE_T))
                )
            )
        {
            prim->options |= DM_SM_INIT_SM_STATE;

            if (
                !PsRetrieve(
                    PSKEY_SM_DIV_STATE,
                    &prim->sm_div_state,
                    PS_SIZE_ADJ(sizeof(uint16_t))
                    )
                )
            {
                Panic();
            }
        }
    }
#else /* DISABLE_BLE */
    UNUSED(state);
#endif /* DISABLE_BLE */

    VmSendDmPrim(prim);
}

/****************************************************************************
NAME
    connectionHandleSecurityInitCfm

DESCRIPTION
    This function is called on receipt of an CL_INTERNAL_SM_INIT_REQ message,
    which only occurs when connection library is in the initialising state.

    It then send an internal state message so that the initialisation can move
    to the next state.
RETURNS

*/
void handleSecurityInitCfm(
        connectionReadInfoState *state,
        const DM_SM_INIT_CFM_T  *cfm
        )
{
    /* Anything other than 0 and something has gone wrong. */
    if (cfm->status)
        Panic();

#ifndef DISABLE_BLE
    if (state->version >= bluetooth4_0)
    {
        PsStore(
            PSKEY_SM_KEY_STATE_IR_ER,
            &cfm->sm_key_state,
            PS_SIZE_ADJ(sizeof(DM_SM_KEY_STATE_T))
            );

        /* If no Div key is generated before the next power-cycle then this
         * still needs to be stored.
         */
        PsStore(
            PSKEY_SM_DIV_STATE,
            &cfm->sm_div_state,
            PS_SIZE_ADJ(sizeof(uint16_t))
            );
        /* SIGNING is not supported so the sm_sign_counter is not stored. */
    }
#else /* DISABLE_BLE */
    UNUSED(state);
#endif /* DISABLE_BLE */

    /* Proceed to the next part of connection library initialisation */
    connectionSendInternalInitCfm(connectionInitSm);
}



/****************************************************************************
NAME
    handleSetSecurityModeReq

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_SM_SET_SC_MODE_REQ message.

RETURNS

*/
void handleSetSecurityModeReq(
        connectionSmState* smState,
        const CL_INTERNAL_SM_SET_SC_MODE_REQ_T* req
        )
{
    if(!smState->setSecurityModeLock)
    {
        /* Record the current mode */
        if(!smState->authReqLock)
        {
            smState->security_mode = req->mode;
            smState->enc_mode = req->mode3_enc;
        }

        /* indicate the message type so that the correct CFM is generated when
           the DM_SM_INIT_CFM is returned from Bluestack.
         */
        smState->sm_init_msg = sm_init_set_security_mode;

        /* In order to return the CFM back to the source task we need to
           lock further requests from being sent to Bluestack until the CFM
           for this instance is returned */
        smState->setSecurityModeLock = req->theAppTask;

        /* Send set sec mode primitive to Bluestack */
        {
            MAKE_PRIM_T(DM_SM_INIT_REQ);
            prim->options = DM_SM_INIT_SECURITY_MODE |
                            DM_SM_INIT_MODE3_ENC |
                            DM_SM_INIT_CONFIG;
            prim->config =
                DM_SM_SEC_MODE_CONFIG_LEGACY_AUTO_PAIR_MISSING_LINK_KEY;
            prim->security_level_default = 0; /* not set  here*/
            prim->security_mode = connectionConvertSecurityMode_t(req->mode);
            prim->write_auth_enable = 0;
            prim->mode3_enc = (uint8) req->mode3_enc;
            VmSendDmPrim(prim);
        }
    }
    else
    {
        /* Message still outstanding, conditionally sent a private message
           to be consumed on the outstanding message request being completed */
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_SET_SC_MODE_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_SM_SET_SC_MODE_REQ,
                message,
                &smState->setSecurityModeLock
                );
    }
}


/****************************************************************************
NAME
    handleSetDefaultSecurityLevelReq

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_SM_SET_DEFAULT_SECURITY_LEVEL_REQ message.

RETURNS

*/
void handleSetSspSecurityLevelReq(
        connectionSmState* smState,
        const CL_INTERNAL_SM_SET_SSP_SECURITY_LEVEL_REQ_T* req
        )
{
    if(req->ssp_sec_level < ssp_secl_level_unknown)
    {
        if(req->protocol_id == protocol_unknown)
        {
            /* Set default security level */
            MAKE_PRIM_T(DM_SM_INIT_REQ);

            /* indicate the message type so that the correct CFM is generated
             * when the DM_SM_INIT_CFM is returned from Bluestack.
             */
            smState->sm_init_msg = sm_init_set_security_default;

            prim->options = DM_SM_INIT_SECURITY_LEVEL_DEFAULT;
            prim->security_mode = 0;
            prim->security_level_default =
                connectionConvertSspSecurityLevel_t(
                        req->ssp_sec_level,
                        TRUE,
                        req->authorised,
                        req->disable_legacy
                        );
            prim->config = 0;
            prim->mode3_enc = 0;
            prim->write_auth_enable = 0;
            VmSendDmPrim(prim);
        }
        else
        {
            /* Set protocol security level */
            MAKE_PRIM_T(DM_SM_REGISTER_REQ);
            prim->service.protocol_id =
                             connectionConvertProtocolId(req->protocol_id);
            prim->service.channel = req->channel;
                prim->outgoing_ok = req->outgoing_ok;

                prim->security_level =
                    connectionConvertSspSecurityLevel_t(
                            req->ssp_sec_level,
                            req->outgoing_ok,
                            req->authorised,
                            req->disable_legacy
                            );
                VmSendDmPrim(prim);
        }
    }
}

/****************************************************************************
NAME
    handleSecModeConfigReq

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ message.

RETURNS

*/
void handleSecModeConfigReq(
        connectionSmState* smState,
        connectionReadInfoState* infoState,
        const CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ_T* req
        )
{
    /* Make sure the firmware can support the sec mode config primitive */
    if(infoState->version != bluetooth_unknown)
    {
        if(!smState->setSecurityModeLock)
        {
            /* Configure SM */
            MAKE_PRIM_T(DM_SM_INIT_REQ);

            /* indicate the message type so that the correct CFM is generated
             * when the DM_SM_INIT_CFM is returned from Bluestack.
             */
            smState->sm_init_msg = sm_init_set_security_config;

            prim->options = DM_SM_INIT_CONFIG | DM_SM_INIT_WRITE_AUTH_ENABLE;
            prim->security_mode = 0;
            prim->security_level_default = 0;

            /* Devs need access DM_SM_SEC_MODE_CONFIG_SEND_ACCESS_IND inds to
             * stop SDP ping.
             */
            prim->config = 0;
            if(req->debug_keys)
                prim->config |= DM_SM_SEC_MODE_CONFIG_DEBUG_MODE;
            if(req->legacy_auto_pair_key_missing)
                prim->config |=
                    DM_SM_SEC_MODE_CONFIG_LEGACY_AUTO_PAIR_MISSING_LINK_KEY;

            prim->write_auth_enable =
                connectionConvertWriteAuthEnable_t(req->write_auth_enable);

            prim->mode3_enc = 0;

            smState->setSecurityModeLock = req->theAppTask;
            VmSendDmPrim(prim);
        }
        else
        {
            /* Message still outstanding, conditionally sent a private message
             * to be consumed on the outstanding message request being
             * completed.
             */
            MAKE_CL_MESSAGE(CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ);
            COPY_CL_MESSAGE(req, message);
            MessageSendConditionallyOnTask(
                    connectionGetCmTask(),
                    CL_INTERNAL_SM_SEC_MODE_CONFIG_REQ,
                    message,
                    &smState->setSecurityModeLock
                    );
        }
    }
    else
    {
        /* Send sec mode config fail message to app */
        MAKE_CL_MESSAGE(CL_SM_SEC_MODE_CONFIG_CFM);
        message->success = FALSE;
        message->wae = cl_sm_wae_never;
        message->indications = FALSE;
        message->debug_keys = FALSE;
        MessageSend(req->theAppTask,CL_SM_SEC_MODE_CONFIG_CFM,message);
    }
}


/****************************************************************************
NAME
    connectionHandleSetSecurityModeCfm

DESCRIPTION
    This function is called on receipt of an DM_SM_INIT_CFM message.

RETURNS

*/
void connectionHandleSetSecurityModeCfm(
        connectionSmState* smState,
        const DM_SM_INIT_CFM_T* cfm
        )
{
    if(smState->setSecurityModeLock == connectionGetCmTask())
    {
        /* Request originated within the CL, handle internally */
        handleInternalSetSecurityModeCfm(smState, cfm);
    }
    else
    {
        if (smState->setSecurityModeLock)
        {
            /* Send CFM to source task */
            MAKE_CL_MESSAGE(CL_SM_SECURITY_LEVEL_CFM);
            message->success = cfm->status ? FALSE:TRUE;
            MessageSend(
                    smState->setSecurityModeLock,
                    CL_SM_SECURITY_LEVEL_CFM,
                    message
                    );
        }
    }

    /* Reset lock */
    smState->setSecurityModeLock = 0;
}


/****************************************************************************
NAME
    connectionHandleConfigureSecurityCfm

DESCRIPTION
    This function is called on receipt of an
    DM_SM_INIT_CFM message.

RETURNS

*/
void connectionHandleConfigureSecurityCfm(
        connectionSmState* smState,
        const DM_SM_INIT_CFM_T* cfm
        )
{
    if(smState->setSecurityModeLock)
    {
        if(smState->setSecurityModeLock != connectionGetCmTask())
        {
            /* Send CFM to source task */
            MAKE_CL_MESSAGE(CL_SM_SEC_MODE_CONFIG_CFM);
            message->success = cfm->status ? FALSE:TRUE;
            message->wae = cfm->write_auth_enable;
            message->indications = FALSE;
            message->debug_keys =
                (cfm->config & DM_SM_SEC_MODE_CONFIG_DEBUG_MODE) ? TRUE:FALSE;
            MessageSend(
                    smState->setSecurityModeLock,
                    CL_SM_SEC_MODE_CONFIG_CFM,
                    message
                    );
        }
    }
    smState->setSecurityModeLock = 0;
}


/****************************************************************************
NAME
    handleRegisterReq

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_SM_REGISTER_REQ message.

RETURNS

*/
void handleRegisterReq(const CL_INTERNAL_SM_REGISTER_REQ_T* req)
{
    MAKE_PRIM_T(DM_SM_REGISTER_REQ);
    prim->context = 0;
    prim->service.protocol_id = connectionConvertProtocolId(req->protocol_id);
    prim->service.channel = req->channel;
    prim->outgoing_ok = req->outgoing_ok;
    prim->security_level = (dm_security_level_t)req->security_level;
    VmSendDmPrim(prim);
}


/****************************************************************************
NAME
    handleUnRegisterReq

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_SM_UNREGISTER_REQ message.

RETURNS

*/
void handleUnRegisterReq(const CL_INTERNAL_SM_UNREGISTER_REQ_T* req)
{
    MAKE_PRIM_T(DM_SM_UNREGISTER_REQ);
    prim->context = 0;
    prim->service.protocol_id = connectionConvertProtocolId(req->protocol_id);
    prim->service.channel = req->channel;
    VmSendDmPrim(prim);
}


/****************************************************************************
NAME
    handleRegisterOutgoingReq

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_SM_REGISTER_OUTGOING_REQ message.

RETURNS

*/
void handleRegisterOutgoingReq(
        const CL_INTERNAL_SM_REGISTER_OUTGOING_REQ_T* req
        )
{
    MAKE_PRIM_T(DM_SM_REGISTER_OUTGOING_REQ);
    prim->context = CTX_FROM_TASK(req->theAppTask);

    prim->connection.addrt.type = TBDADDR_PUBLIC;
    BdaddrConvertVmToBluestack(&prim->connection.addrt.addr, &req->bd_addr);
    prim->connection.service.protocol_id =
                        connectionConvertProtocolId(req->protocol_id);
    prim->connection.service.channel = req->remote_channel;
    prim->security_level = (dm_security_level_t)req->outgoing_security_level;
    VmSendDmPrim(prim);
}

/****************************************************************************
NAME
    handleRegisterOutgoingCfm

DESCRIPTION
    This function is called on receipt of an DM_SM_REGISTER_OUTGOING_CFM
    message. Only CFM for an outgoing RFCOMM security channel are sent
    back to the task identified in the 'context' field.

RETURNS

*/
void handleRegisterOutgoingCfm(const DM_SM_REGISTER_OUTGOING_CFM_T* cfm)
{
    if  (
        cfm->context &&
        cfm->connection.service.protocol_id == SEC_PROTOCOL_RFCOMM
        )
    {
        MAKE_CL_MESSAGE(CL_SM_REGISTER_OUTGOING_SERVICE_CFM);
        BdaddrConvertBluestackToVm(&message->bd_addr,&cfm->connection.addrt.addr);
        message->security_channel = cfm->connection.service.channel;
        MessageSend(
                (Task)cfm->context,
                CL_SM_REGISTER_OUTGOING_SERVICE_CFM,
                message
                );

    }
}


/****************************************************************************
NAME
    handleUnRegisterOutgoingReq

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_SM_UNREGISTER_OUTGOING_REQ message.

RETURNS

*/
void handleUnRegisterOutgoingReq(
        const CL_INTERNAL_SM_UNREGISTER_OUTGOING_REQ_T* req
        )
{
    MAKE_PRIM_T(DM_SM_UNREGISTER_OUTGOING_REQ);
    prim->context = 0;
    prim->connection.addrt.type = TBDADDR_PUBLIC;
    BdaddrConvertVmToBluestack(&prim->connection.addrt.addr, &req->bd_addr);
    prim->connection.service.protocol_id =
                            connectionConvertProtocolId(req->protocol_id);
    prim->connection.service.channel= req->channel;
    VmSendDmPrim(prim);
}


/****************************************************************************
NAME
    handleEncryptReq

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_SM_ENCRYPT_REQ message.

RETURNS

*/
void handleEncryptReq(
        connectionSmState *smState,
        const CL_INTERNAL_SM_ENCRYPT_REQ_T* req
        )
{
    if(!smState->encryptReqLock)
    {
        tp_bdaddr tpaddr;

        /* Can't do anything if the sink is invalid. */
        if (SinkGetBdAddr(req->sink, &tpaddr))
        {
            MAKE_PRIM_T(DM_SM_ENCRYPT_REQ);

            /* Store the task id so we know who to route the response to. */
            smState->encryptReqLock = req->theAppTask;
            smState->sink = req->sink;

            BdaddrConvertVmToBluestack(&prim->bd_addr, &tpaddr.taddr.addr);
            prim->encrypt = req->encrypt;
            VmSendDmPrim(prim);
        }
        else
        {
            CL_DEBUG(("Invalid sink: %d\n", (int)req->sink));
        }
    }
    else
    {
        /*
            Message still outstanding, conditionally sent a private message
            to be consumed on the outstanding message request being completed.
        */
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_ENCRYPT_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_SM_ENCRYPT_REQ,
                message,
                &smState->encryptReqLock
                );
    }
}


/****************************************************************************
NAME
    connectionHandleEncryptCfm

DESCRIPTION
    Handle the confirm message informing us of the outcome of the encrypt
    request.

RETURNS

*/
void connectionHandleEncryptCfm(
        connectionSmState *smState,
        const DM_SM_ENCRYPT_CFM_T *cfm
        )
{
    if (smState->encryptReqLock)
    {
        MAKE_CL_MESSAGE(CL_SM_ENCRYPT_CFM);

        if (cfm->success)
            message->status = success;
        else
            message->status = fail;

        message->sink = smState->sink;
        message->encrypted = cfm->encrypted;
        message->encrypt_type =
                          connectionConvertEncryptionType(cfm->encrypt_type);
        MessageSend(smState->encryptReqLock, CL_SM_ENCRYPT_CFM, message);
    }

    /* Reset the resource lock */
    smState->encryptReqLock = 0;
    smState->sink = 0;
}


/****************************************************************************
NAME
    handleEncryptionKeyRefreshReq

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ message.

RETURNS

*/
void handleEncryptionKeyRefreshReq(
        connectionReadInfoState* infoState,
        const CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ_T* req
        )
{
    if(infoState->version != bluetooth_unknown)
    {
        MAKE_PRIM_C(DM_HCI_REFRESH_ENCRYPTION_KEY_REQ);
        BdaddrConvertTypedVmToBluestack(&prim->addrt, &req->taddr);
        VmSendDmPrim(prim);
    }
}


/****************************************************************************/
static void sendEncryptionKeyRefreshInd(
        Task clientTask,
        hci_status status,
        Sink sink,
        const tp_bdaddr *tpaddr
        )
{
    MAKE_CL_MESSAGE(CL_SM_ENCRYPTION_KEY_REFRESH_IND);
    message->status = status;
    message->sink = sink;
    message->tpaddr = *tpaddr;
    MessageSend(clientTask, CL_SM_ENCRYPTION_KEY_REFRESH_IND, message);
}


/****************************************************************************
NAME
    connectionHandleEncryptionKeyRefreshInd

DESCRIPTION
    Handle the encryption key refresh indication

RETURNS

*/

/* TODO: How can you send an IND to a task with a BLE link (and no sink).
 *
 * There is a private gatt library function gattGetCidMappedTask(). This
 * could be used, in conjunction with the already public Gatt function
 * GattGetCidForBdaddr(), to get the task and then send them the indication.
 *
 * Unfortunately, this would require that the GATT library is linked, and a
 * GATT task intialised. So far, encryption change IND for BLE links has
 * not been required for applications.
 */
void connectionHandleEncryptionKeyRefreshInd(
        const DM_HCI_REFRESH_ENCRYPTION_KEY_IND_T *ind
        )
{
    tp_bdaddr tpaddr;
    uint16 i = 0;
    uint16 number_sinks;
    hci_status status = connectionConvertHciStatus(ind->status);
    Sink *sink_list;
    bool ind_sent = FALSE;

    /* Convert the address in the ind */
    BdaddrConvertTpBluestackToVm(&tpaddr, &ind->tp_addrt);

    sink_list = connectionGetSinksFromBdAddr(&number_sinks, &tpaddr);

    /* Send a message to each task informing it of the change in
     * encryption status
     */
    for (i=0; i < number_sinks; i++)
    {
        /* Check we have a sink */
        if(sink_list[i])
        {
            /* Get the associated task */
            Task task = MessageStreamGetTaskFromSink(sink_list[i]);

            if (task)
            {
                sendEncryptionKeyRefreshInd(
                        task,
                        status,
                        sink_list[i],
                        &tpaddr
                        );

                ind_sent = TRUE;
            }
        }
    }

    /* If the IND has not been sent to a task associated to any related sinks
     * then send the ind to the task which initiated the CL
     */
    if (!ind_sent)
    {
        sendEncryptionKeyRefreshInd(
                connectionGetAppTask(),
                status,
                0,
                &tpaddr
                );
    }

    /* Free the sink list */
    free(sink_list);
}


/****************************************************************************
NAME
    handlePinRequestRes

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_SM_PIN_REQUEST_RES message.

RETURNS

*/
void handlePinRequestRes(const CL_INTERNAL_SM_PIN_REQUEST_RES_T* res)
{
    MAKE_PRIM_T(DM_SM_PIN_REQUEST_RSP);
    BdaddrConvertTypedVmToBluestack(&prim->addrt, &res->taddr);

    /* Reject pin if length is zero or it exceeds MAX_HCI_PIN_LENGTH */
    if((res->pin_length > 0) && (res->pin_length <= HCI_MAX_PIN_LENGTH))
    {
         prim->pin_length = res->pin_length;
         memmove(prim->pin, res->pin, res->pin_length);
    }
    else
    {
         prim->pin_length = 0;
    }

    VmSendDmPrim(prim);
}


/****************************************************************************
NAME
    handleIoCapabilityRequestRes

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES message.

RETURNS

*/
void handleIoCapabilityRequestRes(
        connectionSmState* smState,
        const CL_INTERNAL_SM_IO_CAPABILITY_REQUEST_RES_T* res
        )
{
    cl_sm_io_capability io_capability = res->io_capability;
    uint8 authentication_requirements = 0;

    /* If Security is handled by the security manager (BLE link) */
    if (smState->sm_security)
    {
        if (res->bonding)
            authentication_requirements |= DM_SM_SECURITY_BONDING;

        if (res->mitm)
            authentication_requirements |= DM_SM_SECURITY_MITM_PROTECTION;
    }
    else /* Otherwise, security is being handled by the LM. */
    {
        /* Check if either we or the remote dev are doing dedicated bonding */
        AUTH_BITS_SET(
                smState->authentication_requirements,
                HCI_MITM_NOT_REQUIRED_DEDICATED_BONDING
                );

        if(res->bonding)
        {
            if(auth_bits_set)
            {
                authentication_requirements =
                    res->mitm ?
                        HCI_MITM_REQUIRED_DEDICATED_BONDING :
                        HCI_MITM_NOT_REQUIRED_DEDICATED_BONDING;
            }
            else
            {
                authentication_requirements =
                    res->mitm ?
                        HCI_MITM_REQUIRED_GENERAL_BONDING :
                        HCI_MITM_NOT_REQUIRED_GENERAL_BONDING;
            }
        }
        else
        {
            if(auth_bits_set)
            {
                /* Reject response */
                io_capability = cl_sm_reject_request;
                authentication_requirements = AUTH_REQ_UNKNOWN;

            }
            else
            {
                authentication_requirements =
                    res->mitm ?
                        HCI_MITM_REQUIRED_NO_BONDING :
                        HCI_MITM_NOT_REQUIRED_NO_BONDING;
            }
        }
    }

	if(res->tpaddr.transport == TRANSPORT_BLE_ACL)
	{
		authentication_requirements |= DM_SM_SECURITY_SECURE_CONNECTIONS;
	}

    smState->authentication_requirements = authentication_requirements;

    if(io_capability != cl_sm_reject_request)
    {
        MAKE_PRIM_T(DM_SM_IO_CAPABILITY_REQUEST_RSP);

        BdaddrConvertTpVmToBluestack(&prim->tp_addrt, &res->tpaddr);

        prim->io_capability = connectionConvertIoCapability_t(io_capability);

        if (res->mitm == mitm_flexible)
           DM_SM_SET_MITM_FLEXIBLE(prim);

        prim->authentication_requirements = authentication_requirements;
        prim->oob_data_present = res->oob_data;

        if (res->oob_hash_c)
            prim->oob_hash_c = VmGetHandleFromPointer(res->oob_hash_c);
        else
            prim->oob_hash_c = NULL;

        if (res->oob_rand_r)
            prim->oob_rand_r = VmGetHandleFromPointer(res->oob_rand_r);
        else
            prim->oob_rand_r = NULL;

        /* if the SM is handling security and there are keys to distribute. */
        if (smState->sm_security && res->key_distribution)
        {
            /* If we are the Responder, Logical AND our key_distribution with
             * the initiator's - we can only unset bits they have set.
             */
            if (smState->responder)
            {
                prim->key_distribution =
                    smState->key_distribution & res->key_distribution;
            }
            else /* We are the initiator and set the key distribution. */
            {
                prim->key_distribution = res->key_distribution;
            }
        }
        else
        {
            prim->key_distribution = 0;
        }

        smState->responder = FALSE;

        VmSendDmPrim(prim);
     }
     else
     {
        MAKE_PRIM_T(DM_SM_IO_CAPABILITY_REQUEST_NEG_RSP);
        BdaddrConvertTpVmToBluestack(&prim->tp_addrt, &res->tpaddr);

        /* The CL must send an error code appropriate to the transport type. */
        prim->reason = (res->tpaddr.transport == TRANSPORT_BLE_ACL) ?
            DM_SM_ERROR_PAIRING_NOT_SUPPORTED : HCI_ERROR_PAIRING_NOT_ALLOWED;

        VmSendDmPrim(prim);
     }
}


/****************************************************************************
NAME
    handleUserConfirmationRequestRes

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_USER_CONFIRMATION_REQUEST_RES message.

RETURNS

*/
void handleUserConfirmationRequestRes(
        const CL_INTERNAL_SM_USER_CONFIRMATION_REQUEST_RES_T* res
        )
{
    if(res->confirm)
    {
        MAKE_PRIM_T(DM_SM_USER_CONFIRMATION_REQUEST_RSP);
        BdaddrConvertTpVmToBluestack(&prim->tp_addrt, &res->tpaddr);
        VmSendDmPrim(prim);
    }
    else
    {
        MAKE_PRIM_T(DM_SM_USER_CONFIRMATION_REQUEST_NEG_RSP);
        BdaddrConvertTpVmToBluestack(&prim->tp_addrt, &res->tpaddr);
        VmSendDmPrim(prim);
    }
}


/****************************************************************************
NAME
    handleUserPasskeyRequestRes

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_USER_PASSKEY_REQUEST_RES message.

RETURNS

*/
void handleUserPasskeyRequestRes(
        const CL_INTERNAL_SM_USER_PASSKEY_REQUEST_RES_T* res
        )
{
    if(!res->cancelled)
    {
        MAKE_PRIM_T(DM_SM_USER_PASSKEY_REQUEST_RSP);
        BdaddrConvertTpVmToBluestack(&prim->tp_addrt, &res->tpaddr);
        prim->numeric_value = res->numeric_value;
        VmSendDmPrim(prim);
    }
    else
    {
        MAKE_PRIM_T(DM_SM_USER_PASSKEY_REQUEST_NEG_RSP);
        BdaddrConvertTpVmToBluestack(&prim->tp_addrt, &res->tpaddr);
        VmSendDmPrim(prim);
    }
}


/****************************************************************************
NAME
    handleSendKeypressNotificationReq

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_SEND_KEYPRESS_NOTIFICATION_REQ message.

RETURNS

*/
void handleSendKeypressNotificationReq(
        const CL_INTERNAL_SM_SEND_KEYPRESS_NOTIFICATION_REQ_T* req
        )
{
    MAKE_PRIM_T(DM_SM_SEND_KEYPRESS_NOTIFICATION_REQ)
    BdaddrConvertTpVmToBluestack(&prim->tp_addrt, &req->tpaddr);
    prim->notification_type = connectionConvertKeypressType_t(req->type);
    VmSendDmPrim(prim);
}


/****************************************************************************
NAME
    handleSetTrustLevelReq

DESCRIPTION
    This function is called on receipt of an CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ
     message.

RETURNS

*/
void handleSetTrustLevelReq(
        connectionSmState *smState,
        const CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ_T* req
        )
{
    if (smState->deviceReqLock)
    {
        /*
            Message still outstanding, conditionally sent a private message
            to be consumed on the outstanding message request being completed.
        */
        CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ_T *message =
            PanicUnlessNew(CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ_T);

        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_SM_SET_TRUST_LEVEL_REQ,
                message,
                &smState->deviceReqLock
                );
    }
    else
    {
        if ( !connectionAuthSetTrustLevel(&req->bd_addr, req->trusted) )
        {
            CL_SM_SET_TRUST_LEVEL_CFM_T *message =
                PanicUnlessNew(CL_SM_SET_TRUST_LEVEL_CFM_T);
            message->bd_addr = req->bd_addr;
            message->status = fail;
            MessageSend(req->theAppTask, CL_SM_SET_TRUST_LEVEL_CFM, message);
        }
        else
        {
            /* Set lock */
            smState->deviceReqType = device_req_set_trusted;
            smState->deviceReqLock = req->theAppTask;
        }

    }
}


/****************************************************************************
NAME
    handleAuthoriseRes

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_SM_AUTHORISE_RES message.

RETURNS

*/
void handleAuthoriseRes(const CL_INTERNAL_SM_AUTHORISE_RES_T* res)
{
    MAKE_PRIM_T(DM_SM_AUTHORISE_RSP);
    prim->cs.connection.addrt.type = TBDADDR_PUBLIC;
    BdaddrConvertVmToBluestack(&prim->cs.connection.addrt.addr, &res->bd_addr);
    prim->cs.connection.service.protocol_id =
                             connectionConvertProtocolId(res->protocol_id);
    prim->cs.connection.service.channel = res->channel;
    prim->cs.incoming = res->incoming;

    if (res->authorised)
        prim->authorisation = DM_SM_AUTHORISE_ACCEPT_ONCE;
    else
        prim->authorisation = DM_SM_AUTHORISE_REJECT_ONCE;
    VmSendDmPrim(prim);
}


/****************************************************************************
NAME
    handleAddAuthDeviceReq

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ to add a device to the trusted device
    list.

    Use PanicUnlessNew() instead of MAKE_CL_MESSAGE to try and reduce stack
    usage.

RETURNS

*/
void handleAddAuthDeviceReq(
        connectionSmState *smState,
        const CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ_T *req
        )
{
    if (smState->deviceReqLock)
    {
        /*
            Message still outstanding, conditionally sent a private message
            to be consumed on the outstanding message request being completed.
        */
        CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ_T *message =
            PanicUnlessNew(CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ_T);

        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_SM_ADD_AUTH_DEVICE_REQ,
                message,
                &smState->deviceReqLock
                );
    }
    else
    {
        /* Add device, check if failed locally */
        if  ( !connectionAuthAddDevice(req) )
        {
            CL_SM_ADD_AUTH_DEVICE_CFM_T *message =
                PanicUnlessNew(CL_SM_ADD_AUTH_DEVICE_CFM_T);
            message->bd_addr = req->bd_addr;
            message->status = fail;
            MessageSend(req->theAppTask, CL_SM_ADD_AUTH_DEVICE_CFM, message);
        }
        else
        {
            /* Set lock */
            smState->deviceReqType = device_req_add_authorised;
            smState->deviceReqLock = req->theAppTask;
        }
    }
}


/****************************************************************************
NAME
    handleGetAuthDeviceReq

DESCRIPTION
    This function is called on receipt of an
    CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ to get a device from the trusted device
    list.

RETURNS

*/
void handleGetAuthDeviceReq(
        connectionSmState *smState,
        const CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ_T *req
        )
{
    if (smState->deviceReqLock)
    {
        /*
            Message still outstanding, conditionally sent a private message
            to be consumed on the outstanding message request being completed.
        */
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_SM_GET_AUTH_DEVICE_REQ,
                message,
                &smState->deviceReqLock
                );
    }
    else
    {
        uint16 linkkey[BREDR_KEY_SIZE];
        cl_sm_link_key_type link_key_type;
        uint16 trusted;
        MAKE_CL_MESSAGE_WITH_LEN(CL_SM_GET_AUTH_DEVICE_CFM, BREDR_KEY_SIZE * sizeof(uint16));

        message->bd_addr = req->bd_addr;

        if  (
            connectionAuthGetDevice(
                &req->bd_addr,
                &link_key_type,
                &linkkey[0],
                &trusted
                )
            )
        {
            message->status = success;
            message->trusted = trusted;
            message->link_key_type = link_key_type;
            message->size_link_key = BREDR_KEY_SIZE;
            memmove(&message->link_key[0], &linkkey[0], sizeof(linkkey));
        }
        else
        {
            message->status = fail;
            message->link_key_type = cl_sm_link_key_none;
        }

        MessageSend(req->theAppTask, CL_SM_GET_AUTH_DEVICE_CFM, message);
    }
}


/****************************************************************************/
static void sendEncryptionChangeInd(
        Task clientTask,
        Sink sink,
        bool encrypted,
        uint16 encrypt_type,
        const tp_bdaddr *tpaddr
        )
{
    MAKE_CL_MESSAGE(CL_SM_ENCRYPTION_CHANGE_IND);
    message->sink = sink;
    message->encrypted = encrypted;
    message->encrypt_type = connectionConvertEncryptionType(encrypt_type);
    message->tpaddr = *tpaddr;
    MessageSend(clientTask, CL_SM_ENCRYPTION_CHANGE_IND, message);
}


/****************************************************************************/
void connectionHandleEncryptionChange(const DM_SM_ENCRYPTION_CHANGE_IND_T *ind)
{
    tp_bdaddr tpaddr;
    uint16 i = 0;
    uint16 number_sinks;
    Sink *sink_list = NULL;
    bool ind_sent = FALSE;

    /* Convert the address in the ind */
    BdaddrConvertTpBluestackToVm(&tpaddr, &ind->tp_addrt);

    sink_list = connectionGetSinksFromBdAddr(&number_sinks, &tpaddr);

    /* Send a message to each task informing it of the change in encryption
     * status.
     */
    for (i=0; i < number_sinks; i++)
    {
        /* Check we have a sink */
        if(sink_list[i])
        {
            /* Get the associated task */
            Task task = MessageStreamGetTaskFromSink(sink_list[i]);

            if (task)
            {
                /* Send encryption change message to that task */
                sendEncryptionChangeInd(
                        task,
                        sink_list[i],
                        ind->encrypted,
                        ind->encrypt_type,
                        &tpaddr
                        );

                ind_sent = TRUE;
            }
        }
    }

    /* If the IND has not been sent to a task associated to any related sinks
     * then send the ind to the task which initiated the CL
     */
    if (!ind_sent)
    {
        sendEncryptionChangeInd(
                connectionGetAppTask(),
                0,
                ind->encrypted,
                ind->encrypt_type,
                &tpaddr
                );
    }

    /* Free the sink list */
    free(sink_list);
}


/***************************************************************************
NAME
    connectionSmHandleGetAttributeReq

DESCRIPTION
    Called to request a read of attributes from persistent store
*/
void connectionSmHandleGetAttributeReq(
        Task appTask,
        const CL_INTERNAL_SM_GET_ATTRIBUTE_REQ_T * req
        )
{
    connectionAuthGetAttribute(
            appTask,
            req->ps_base,
            req->addr_type,
            &req->bd_addr,
            req->size_psdata
            );
}


/***************************************************************************
NAME
    connectionSmHandleGetIndexedAttributeReq

DESCRIPTION
    Called to request a read of attributes from persistent store
*/
void connectionSmHandleGetIndexedAttributeReq(
        Task appTask,
        const CL_INTERNAL_SM_GET_INDEXED_ATTRIBUTE_REQ_T * req
        )
{
    connectionAuthGetIndexedAttribute(
            appTask,
            req->ps_base,
            req->index,
            req->size_psdata
            );
}

/***************************************************************************
NAME
    connectionDmHandleAclOpen

DESCRIPTION
    Called on receipt of CL_INTERNAL_DM_ACL_OPEN_REQ to ACL
    attach on device, identified by a specific BD_ADDR.
    See DM_ACL_OPEN_REQ in dm_prim.h.
*/
void connectionDmHandleAclOpen(
        const CL_INTERNAL_DM_ACL_OPEN_REQ_T *req
        )
{
    aclOpen(&req->bd_addr);
}

/***************************************************************************
NAME
    connectionDmHandleAclClose

DESCRIPTION
    Called on receipt of CL_INTERNAL_DM_ACL_CLOSE_REQ to close ACL
    with device, identified by a specific BD_ADDR, or to detach all
    ACL links depending on the flags set. See DM_ACL_CLOSE_REQ in dm_prim.h.
*/
void connectionDmHandleAclClose(
        const CL_INTERNAL_DM_ACL_CLOSE_REQ_T *req
        )
{
    aclClose(&req->bd_addr, req->flags, req->reason);
}

/***************************************************************************
NAME
    connectionHandleAPTExpiredInd

DESCRIPTION
    Called on receipt of DM_HCI_AUTHENTICATED_PAYLOAD_TIMEOUT_EXPIRED_IND_T
    to inform the application that an authenticated payload has not been
    received on a link within the current timeout period.
*/
void connectionHandleAPTExpiredInd(
        Task appTask,
        const DM_HCI_AUTHENTICATED_PAYLOAD_TIMEOUT_EXPIRED_IND_T *ind )
{
    if( appTask )
    {
        MAKE_CL_MESSAGE(CL_DM_APT_IND);
        BdaddrConvertTpBluestackToVm(&message->tpaddr, &ind->tp_addrt);
        MessageSend(appTask, CL_DM_APT_IND, message);
    }
}


/****************************************************************************
NAME    
    connectionHandleReadAPTReq

DESCRIPTION
    Read the current value of the authenticated payload timeout from the controller.
*/
void connectionHandleReadAPTReq(const CL_INTERNAL_DM_READ_APT_REQ_T *req)
{
    MAKE_PRIM_C(DM_HCI_READ_AUTHENTICATED_PAYLOAD_TIMEOUT_REQ);
    BdaddrConvertTpVmToBluestack(&prim->tp_addrt, &req->tpaddr);
    VmSendDmPrim(prim);
}

/****************************************************************************
NAME	
    connectionHandleReadAPTCfm

DESCRIPTION
    This function handles a read authenticated payload timeout result.
*/
void connectionHandleReadAPTCfm(
        Task appTask,
        const DM_HCI_READ_AUTHENTICATED_PAYLOAD_TIMEOUT_CFM_T *cfm)
{
    MAKE_CL_MESSAGE(CL_DM_READ_APT_CFM);
    BdaddrConvertTpBluestackToVm(&message->tpaddr, &cfm->tp_addrt);
    message->status = connectionConvertHciStatus(cfm->status);
    message->apt = cfm->authenticated_payload_timeout;
    MessageSend(appTask, CL_DM_READ_APT_CFM, message);
                
}
/****************************************************************************
NAME    
    connectionHandleWriteAPTReq

DESCRIPTION
    Set the per-link value of the authenticated payload timeout.
*/
void connectionHandleWriteAPTReq(const CL_INTERNAL_DM_WRITE_APT_REQ_T *req)
{
    MAKE_PRIM_T(DM_SM_WRITE_AUTHENTICATED_PAYLOAD_TIMEOUT_REQ);
    BdaddrConvertTpVmToBluestack(&prim->tp_addrt, &req->tpaddr);
    prim->authenticated_payload_timeout = req->apt;
    prim->route_event = connectionConvertAPTRouteEvent(req->apt_route_event);
    VmSendDmPrim(prim);
}

/****************************************************************************
NAME	
    connectionHandleWriteAPTCfm

DESCRIPTION
    This function handles a write authenticated payload timeout result.
*/
void connectionHandleWriteAPTCfm(
        Task appTask,
        const DM_SM_WRITE_AUTHENTICATED_PAYLOAD_TIMEOUT_CFM_T *cfm)
{
    MAKE_CL_MESSAGE(CL_DM_WRITE_APT_CFM);
    BdaddrConvertTpBluestackToVm(&message->tpaddr, &cfm->tp_addrt);
    message->status = connectionConvertHciStatus(cfm->status);
    MessageSend(appTask, CL_DM_WRITE_APT_CFM, message);
}

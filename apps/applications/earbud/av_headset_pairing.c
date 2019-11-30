/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_pairing.c
\brief      Pairing task
*/

#include <panic.h>
#include <connection.h>
#include <ps.h>
#include <string.h>
#include <cryptovm.h>

#include "av_headset.h"
#include "av_headset_scan_manager.h"
#include "av_headset_sdp.h"
#include "av_headset_peer_signalling.h"
#include "av_headset_log.h"


/*! Macro to insert UUID into EIR data, order of octets swapped as EIR data is little-endian */
#define EIR_UUID128_2(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)  p,o,n,m,l,k,j,i,h,g,f,e,d,c,b,a
#define EIR_UUID128(uuid)  EIR_UUID128_2(uuid)

/*! Macro for simplifying creating messages */
#define MAKE_PAIRING_MESSAGE(TYPE) \
    TYPE##_T *message = PanicUnlessNew(TYPE##_T);

/*! Macro to make message with variable length for array fields. */
#define MAKE_PAIRING_MESSAGE_WITH_LEN(TYPE, LEN) \
    TYPE##_T *message = (TYPE##_T *) PanicUnlessMalloc(sizeof(TYPE##_T) + LEN);

/*! Maximum packet size of a DM3 packet  */
#define MAX_PACKET_SIZE_DM3 (121)
/*! Maximum size of content in an extended inquiry response (EIR) packet */
#define EIR_MAX_SIZE        (MAX_PACKET_SIZE_DM3)

/*! \name Tags that can be used in an extended inquiry response (EIR) */
#define EIR_TYPE_LOCAL_NAME_COMPLETE        (0x09)
#define EIR_TYPE_LOCAL_NAME_SHORTENED       (0x08)
#define EIR_TYPE_UUID16_COMPLETE            (0x03)
#define EIR_TYPE_UUID128_COMPLETE           (0x07)
#define EIR_TYPE_MANUFACTURER_SPECIFIC      (0xFF)
#define EIR_SIZE_MANUFACTURER_SPECIFIC      (10)  /* Doesn't include size field */

/*
 * Function Prototypes
 */
static void appPairingSetState(pairingTaskData *thePairing, pairingState state);
static void appPairingFindHandsetsNeedingLinkKeyTx(void);
static void appPairingEnterCheckingHandsetLinkkeys(pairingTaskData* thePairing);

/*! Number of bytes in the 128-bit salt. */
#define SALT_SIZE       16
/*! Offset into SALT for start of peer earbud address NAP */
#define OFFSET_SALT_NAP 10
/*! Offset into SALT for start of peer earbud address UAP */
#define OFFSET_SALT_UAP 12
/*! Offset into SALT for start of peer earbud address LAP */
#define OFFSET_SALT_LAP 13

/*! @brief Zero and write the peer earbud BT address into last 6 bytes of the SALT.

    The salt is 0x00000000000000000000xxxxxxxxxxxx
    Where the 6 bytes are LAP (3 bytes big-endian), UAP (1 byte), NAP (2 bytes big-endian)
*/
static void appSetupSaltWithPeerAddress(uint8 *salt, const bdaddr *peer_addr)
{
    memset(salt, 0, SALT_SIZE);
    salt[OFFSET_SALT_NAP]   = (peer_addr->nap >> 8) & 0xFF;
    salt[OFFSET_SALT_NAP+1] = peer_addr->nap & 0xFF;
    salt[OFFSET_SALT_UAP]   = peer_addr->uap & 0xFF;
    salt[OFFSET_SALT_LAP]   = (peer_addr->lap >> 16) & 0xFF;
    salt[OFFSET_SALT_LAP+1] = (peer_addr->lap >> 8) & 0xFF;
    salt[OFFSET_SALT_LAP+2] = peer_addr->lap & 0xFF;
}

static void appPairingInitialiseEir(const uint8 *local_name, uint16 size_local_name)
{
    static const uint8 eir_16bit_uuids[] =
    {
        EIR_TYPE_UUID16_COMPLETE, /* Complete list of 16-bit Service Class UUIDs */
#ifdef INCLUDE_HFP
        0x1E, 0x11,     /* HFP 0x111E */
        0x08, 0x11,     /* HSP 0x1108 */
#endif
#ifdef INCLUDE_AV
        0x0B, 0x11,     /* AudioSink 0x110B */
        0x0D, 0x11,     /* A2DP 0x110D */
        0x0E, 0x11      /* AVRCP 0x110E */
#endif
    };

#ifndef DISABLE_TWS_PLUS
    static const uint8 eir_128bit_uuids[] =
    {
        EIR_TYPE_UUID128_COMPLETE, /* Complete list of 128-bit Service Class UUIDs */
        EIR_UUID128(UUID_TWS_SINK_SERVICE)
    };
#endif
    
    uint8 *const eir = (uint8 *)PanicUnlessMalloc(EIR_MAX_SIZE + 1);
    uint8 *const eir_end = eir + EIR_MAX_SIZE + 1;
    uint8 *eir_ptr = eir;
    uint16 eir_space;
    bdaddr peer_bdaddr;

    /* Get peer device address, default to all 0's if we haven't paired */
    BdaddrSetZero(&peer_bdaddr);
    appDeviceGetPeerBdAddr(&peer_bdaddr);

    /* Copy 16 bit UUIDs into EIR packet */
    *eir_ptr++ = sizeof(eir_16bit_uuids);
    memmove(eir_ptr, eir_16bit_uuids, sizeof(eir_16bit_uuids));
    eir_ptr += sizeof(eir_16bit_uuids);

#ifndef DISABLE_TWS_PLUS
    /* Copy 128 bit UUIDs into EIR packet */
    *eir_ptr++ = sizeof(eir_128bit_uuids);
    memmove(eir_ptr, eir_128bit_uuids, sizeof(eir_128bit_uuids));
    eir_ptr += sizeof(eir_128bit_uuids);
    
    /* Add TWS+ EIR data */
    *eir_ptr++ = EIR_SIZE_MANUFACTURER_SPECIFIC;
    *eir_ptr++ = EIR_TYPE_MANUFACTURER_SPECIFIC;
    *eir_ptr++ = (appConfigBtSigCompanyId() >> 0) & 0xFF;  /* LSB of CompanyID */
    *eir_ptr++ = (appConfigBtSigCompanyId() >> 8) & 0xFF;  /* MSB of CompanyID */
    *eir_ptr++ = 0x20;                              /* Form factor for BD_ADDR */
    *eir_ptr++ = (peer_bdaddr.lap >>  0) & 0xFF;    /* LSB of LAP */
    *eir_ptr++ = (peer_bdaddr.lap >>  8) & 0xFF;
    *eir_ptr++ = (peer_bdaddr.lap >> 16) & 0xFF;    /* MSB of LAP */
    *eir_ptr++ = peer_bdaddr.uap;                   /* UAP */
    *eir_ptr++ = (peer_bdaddr.nap >>  0) & 0xFF;    /* LSB of NAP */
    *eir_ptr++ = (peer_bdaddr.nap >>  8) & 0xFF;    /* MSB of NAP */
#endif

    /* Calculate space for local device name */
    eir_space = (eir_end - eir_ptr) - 3;  /* Take 3 extra from space for type and size fields and zero terminator */

    /* Check if name need trucating */
    if (eir_space < size_local_name)
    {
        /* Store header for truncated name */
        *eir_ptr++ = eir_space + 1;
        *eir_ptr++ = EIR_TYPE_LOCAL_NAME_SHORTENED;

        /* Clip size of name to space available */
        size_local_name = eir_space;
    }
    else
    {
        /* Store header for complete name */
        *eir_ptr++ = size_local_name + 1;
        *eir_ptr++ = EIR_TYPE_LOCAL_NAME_COMPLETE;
    }

    /* Copy local device name into EIR packet */
    memmove(eir_ptr, local_name, size_local_name);
    eir_ptr += size_local_name;

    /* Add termination character */
    *eir_ptr++ = 0x00;

    /* Write EIR data */
    ConnectionWriteEirData(FALSE, eir_ptr - eir, eir);

    /* Free the EIR data */
    free(eir);
}


static void appPairingRegisterServiceRecord(pairingTaskData *thePairing)
{
    /* Unregister service record if there's one already registered */
    if (thePairing->tws_sink_service_handle)
    {
        /* Increment SDP lock before unregistering to stall any SDP searches */
        thePairing->sdp_lock++;
        ConnectionUnregisterServiceRecord(&thePairing->task, thePairing->tws_sink_service_handle);
    }
    else
    {
        appDeviceAttributes attributes;
        bdaddr bd_addr = {0, 0, 0};

        /* Make copy of service secord */
        uint8 *record = PanicUnlessMalloc(appSdpGetTwsSinkServiceRecordSize());
        memcpy(record, appSdpGetTwsSinkServiceRecord(), appSdpGetTwsSinkServiceRecordSize());

        /* Find address of peer device */
        appDeviceGetPeerAttributes(&bd_addr, &attributes);

        /* Write peer device address into service record */
        appSdpSetTwsSinkServiceRecordPeerBdAddr(record, &bd_addr);

        DEBUG_LOGF("appPairingRegisterServiceRecord, bd_addr %04x,%02x,%06lx", bd_addr.nap, bd_addr.uap, bd_addr.lap);

        /* Register service record */
        ConnectionRegisterServiceRecord(&thePairing->task, appSdpGetTwsSinkServiceRecordSize(), record);
    }
}


static void appPairingEnterInitialising(pairingTaskData *thePairing)
{
    DEBUG_LOG("appPairingEnterInitialising");

    /* Register SDP record */
    appPairingRegisterServiceRecord(thePairing);
}

/*! @brief Actions to take when leaving #PAIRING_STATE_INITIALISING. */
static void appPairingExitInitialising(pairingTaskData *thePairing)
{
    UNUSED(thePairing);

    DEBUG_LOG("appPairingExitInitialising");

    /* starting up, check if we have any outstanding handset link keys
     * which we still need to send to our peer earbud. */
    appPairingTransmitHandsetLinkKeysReq();
}

/*! \brief Find handsets that still need link key sent to peer.
 */
static void appPairingEnterCheckingHandsetLinkkeys(pairingTaskData* thePairing)
{
    /* search for handsets that still require link key to be sent to peer */
    appPairingFindHandsetsNeedingLinkKeyTx();

    /* no more potential for conflicts reading device manager/attribute state
     * so return to idle and permit other pairing operations */
    appPairingSetState(thePairing, PAIRING_STATE_IDLE);
}

static void appPairingEnterIdle(pairingTaskData *thePairing)
{
    DEBUG_LOG("appPairingEnterIdle");

    /* Send message to disable page and inquiry scan after being idle for a period of time */
    MessageSendLater(&thePairing->task, PAIRING_INTERNAL_DISABLE_SCAN, NULL, D_SEC(appConfigPairingScanDisableDelay()));

    /* Unlock pairing and permit a pairing operation */
    thePairing->pairing_lock = 0;
}


static void appPairingExitIdle(pairingTaskData *thePairing)
{
    DEBUG_LOG("appPairingExitIdle");

    /* Cancel any delayed message to disable page and inquiry scan */
    MessageCancelAll(&thePairing->task, PAIRING_INTERNAL_DISABLE_SCAN);

    /* No longer idle, starting a pairing operation so need to be connectable */
    appScanManagerEnablePageScan(SCAN_MAN_USER_PAIRING, SCAN_MAN_PARAMS_TYPE_FAST);

    /* Lock pairing now that a pairing operation is underway */
    thePairing->pairing_lock = 1;
}


static void appPairingEnterPeerInquiry(pairingTaskData *thePairing)
{
    DEBUG_LOG("appPairingEnterPeerInquiry");

    /* Reset address & peak RSSI value */
    BdaddrSetZero(&thePairing->bd_addr[0]);
    BdaddrSetZero(&thePairing->bd_addr[1]);
    thePairing->inquiry_rssi[0] = thePairing->inquiry_rssi[1] = appConfigPeerPairingMinRssi() - appConfigPeerPairingMinRssiDelta();

    /* Set inquiry Tx power, RSSI mode and start periodic inquiry */
    /*ConnectionWriteInquiryTx(-40);*/
    ConnectionWriteInquiryMode(&thePairing->task, inquiry_mode_rssi);

    /* Start inquiry */
    ConnectionInquire(&thePairing->task, 0x9E8B30, 20,
                      5,
                      AUDIO_MAJOR_SERV_CLASS | RENDER_MAJOR_SERV_CLASS |
                      AV_MAJOR_DEVICE_CLASS | HEADSET_MINOR_DEVICE_CLASS);

    /* Start pairing timeout */
    MessageCancelFirst(&thePairing->task, PAIRING_INTERNAL_TIMEOUT_IND);
    if (thePairing->is_user_initiated && appConfigPeerPairingTimeout())
        MessageSendLater(&thePairing->task, PAIRING_INTERNAL_TIMEOUT_IND, 0, D_SEC(appConfigPeerPairingTimeout()));
    else if (!thePairing->is_user_initiated && appConfigAutoPeerPairingTimeout())
        MessageSendLater(&thePairing->task, PAIRING_INTERNAL_TIMEOUT_IND, 0, D_SEC(appConfigAutoPeerPairingTimeout()));

    /* Indicate peer pairing active */
    appUiPeerPairingActive(thePairing->is_user_initiated);
}


static void appPairingExitPeerInquiry(pairingTaskData *thePairing)
{
    DEBUG_LOG("appPairingExitPeerInquiry");

    /* Cancel periodic inquiry */
    ConnectionInquireCancel(&thePairing->task);

    /* Cancel pairing timeout */
    MessageCancelFirst(&thePairing->task, PAIRING_INTERNAL_TIMEOUT_IND);

    /* Cancel inquiry indication */
    appUiPeerPairingInactive();
}


static void appPairingEnterPeerDiscoverable(pairingTaskData *thePairing)
{
    static const uint32 iac_array[] = { 0x9E8B30 };

    DEBUG_LOG("appPairingEnterPeerDiscoverable");

    /* Inquiry Scan on LIAC only */
    ConnectionWriteInquiryAccessCode(&thePairing->task, iac_array, 1);

    /* Enable inquiry scan do other earbud can find us */
    appScanManagerEnableInquiryScan(SCAN_MAN_USER_PAIRING, SCAN_MAN_PARAMS_TYPE_FAST);

    /* Start pairing timeout */
    MessageCancelFirst(&thePairing->task, PAIRING_INTERNAL_TIMEOUT_IND);
    if (thePairing->is_user_initiated && appConfigPeerPairingTimeout())
        MessageSendLater(&thePairing->task, PAIRING_INTERNAL_TIMEOUT_IND, 0, D_SEC(appConfigPeerPairingTimeout()));
    else if (!thePairing->is_user_initiated && appConfigAutoPeerPairingTimeout())
        MessageSendLater(&thePairing->task, PAIRING_INTERNAL_TIMEOUT_IND, 0, D_SEC(appConfigAutoPeerPairingTimeout()));

    /* Indicate peer pairing active */
    appUiPeerPairingActive(thePairing->is_user_initiated);
}


static void appPairingExitPeerDiscoverable(pairingTaskData *thePairing)
{
    DEBUG_LOG("appPairingExitPeerDiscoverable");

    /* No longer discoverable, disable inquiry scan */
    appScanManagerDisableInquiryScan(SCAN_MAN_USER_PAIRING);

    /* Cancel pairing timeout */
    MessageCancelFirst(&thePairing->task, PAIRING_INTERNAL_TIMEOUT_IND);

    /* Cancel inquiry indication */
    appUiPeerPairingInactive();
}


static void appPairingEnterPeerSdpSearch(pairingTaskData *thePairing)
{
    DEBUG_LOG("appPairingEnterPeerSdpSearch");

    /* Perform SDP search */
    ConnectionSdpServiceSearchAttributeRequest(&thePairing->task, &thePairing->bd_addr[0], 0x32,
                                               appSdpGetTwsSinkServiceSearchRequestSize(), appSdpGetTwsSinkServiceSearchRequest(),
                                               appSdpGetTwsSinkAttributeSearchRequestSize(), appSdpGetTwsSinkAttributeSearchRequest());
}


static void appPairingEnterPeerSdpSearchAuthenticated(pairingTaskData *thePairing)
{
    DEBUG_LOG("appPairingEnterPeerSdpSearchAuthenticated");

    /* Perform SDP search */
    ConnectionSdpServiceSearchAttributeRequest(&thePairing->task, &thePairing->bd_addr[0], 0x32,
                                               appSdpGetTwsSinkServiceSearchRequestSize(), appSdpGetTwsSinkServiceSearchRequest(),
                                               appSdpGetTwsSinkAttributeSearchRequestSize(), appSdpGetTwsSinkAttributeSearchRequest());
}


static void appPairingEnterPeerAuthenticate(pairingTaskData *thePairing)
{
    DEBUG_LOGF("appPairingEnterPeerAuthenticate, authenticating %04x,%02x,%06lx", thePairing->bd_addr[0].nap, thePairing->bd_addr[0].uap, thePairing->bd_addr[0].lap);

    /* Device supports TWS+ service, so initiate pairing */
    ConnectionSmAuthenticate(appGetAppTask(), &thePairing->bd_addr[0], 90);
}


static void appPairingEnterPeerScoFwdSdpSearch(pairingTaskData *thePairing)
{
    DEBUG_LOG("appPairingEnterPeerScoFwdSdpSearch");

    /* Perform SDP search */
    ConnectionSdpServiceSearchAttributeRequest(&thePairing->task, &thePairing->bd_addr[0], 0x32,
                                               appSdpGetScoFwdServiceSearchRequestSize(), appSdpGetScoFwdServiceSearchRequest(),
                                               appSdpGetScoFwdFeaturesAttributeSearchRequestSize(), appSdpGetScoFwdFeaturesAttributeSearchRequest());
}


static void appPairingEnterHandsetDiscoverable(pairingTaskData *thePairing)
{
    static const uint32 iac_array[] = { 0x9E8B33 };

    DEBUG_LOG("appPairingEnterHandsetDiscoverable");

    /* Inquiry Scan on GIAC only */
    ConnectionWriteInquiryAccessCode(&thePairing->task, iac_array, 1);

    /* Enable inquiry scan so handset can find us */
    appScanManagerEnableInquiryScan(SCAN_MAN_USER_PAIRING, SCAN_MAN_PARAMS_TYPE_FAST);

    /* Start pairing timeout */
    MessageCancelFirst(&thePairing->task, PAIRING_INTERNAL_TIMEOUT_IND);
    if (thePairing->is_user_initiated && appConfigHandsetPairingTimeout())
        MessageSendLater(&thePairing->task, PAIRING_INTERNAL_TIMEOUT_IND, 0, D_SEC(appConfigHandsetPairingTimeout()));
    else if (!thePairing->is_user_initiated && appConfigAutoHandsetPairingTimeout())
        MessageSendLater(&thePairing->task, PAIRING_INTERNAL_TIMEOUT_IND, 0, D_SEC(appConfigAutoHandsetPairingTimeout()));

    /* Show pairing on LEDs */
    appUiPairingActive(thePairing->is_user_initiated);
}


static void appPairingExitHandsetDiscoverable(pairingTaskData *thePairing)
{
    DEBUG_LOG("appPairingExitHandsetDiscoverable");

    /* No longer discoverable, disable inquiry scan */
    appScanManagerDisableInquiryScan(SCAN_MAN_USER_PAIRING);

    /* Cancel pairing timeout */
    MessageCancelFirst(&thePairing->task, PAIRING_INTERNAL_TIMEOUT_IND);

    /* Stop pairing indication on LEDs */
    appUiPairingInactive(thePairing->is_user_initiated);
}

static void appPairingEnterHandsetSdpSearchAuthenticated(pairingTaskData *thePairing)
{
    DEBUG_LOG("appPairingEnterHandsetSdpSearchAuthenticated");
    
    /* Perform SDP search */
    ConnectionSdpServiceSearchAttributeRequest(&thePairing->task, &thePairing->bd_addr[0], 0x32,
                                               appSdpGetTwsSourceServiceSearchRequestSize(), appSdpGetTwsSourceServiceSearchRequest(),
                                               appSdpGetTwsSourceAttributeSearchRequestSize(), appSdpGetTwsSourceAttributeSearchRequest());
}

/*! \brief Set Pairing FSM state

    Called to change state.  Handles calling the state entry and exit
    functions for the new and old states.
*/
static void appPairingSetState(pairingTaskData *thePairing, pairingState state)
{
    DEBUG_LOGF("appPairSetState(%d)", state);

    switch (thePairing->state)
    {
        case PAIRING_STATE_INITIALISING:
            appPairingExitInitialising(thePairing);
            break;

        case PAIRING_STATE_IDLE:
            appPairingExitIdle(thePairing);
            break;

        case PAIRING_STATE_PEER_INQUIRY:
            appPairingExitPeerInquiry(thePairing);
            break;

        case PAIRING_STATE_PEER_DISCOVERABLE:
            appPairingExitPeerDiscoverable(thePairing);
            break;

        case PAIRING_STATE_HANDSET_DISCOVERABLE:
            appPairingExitHandsetDiscoverable(thePairing);
            break;

        default:
            break;
    }

    /* Set new state */
    thePairing->state = state;

    /* Handle state entry functions */
    switch (state)
    {
        case PAIRING_STATE_INITIALISING:
            appPairingEnterInitialising(thePairing);
            break;

        case PAIRING_STATE_IDLE:
            appPairingEnterIdle(thePairing);
            break;

        case PAIRING_STATE_PEER_INQUIRY:
            appPairingEnterPeerInquiry(thePairing);
            break;

        case PAIRING_STATE_PEER_SDP_SEARCH:
            appPairingEnterPeerSdpSearch(thePairing);
            break;

        case PAIRING_STATE_PEER_AUTHENTICATE:
            appPairingEnterPeerAuthenticate(thePairing);
            break;

        case PAIRING_STATE_PEER_DISCOVERABLE:
            appPairingEnterPeerDiscoverable(thePairing);
            break;

        case PAIRING_STATE_PEER_SDP_SEARCH_AUTHENTICATED:
            appPairingEnterPeerSdpSearchAuthenticated(thePairing);
            break;

        case PAIRING_STATE_PEER_SCO_FWD_SDP_SEARCH:
            appPairingEnterPeerScoFwdSdpSearch(thePairing);
            break;

        case PAIRING_STATE_HANDSET_DISCOVERABLE:
            appPairingEnterHandsetDiscoverable(thePairing);
            break;

        case PAIRING_STATE_CHECKING_HANDSET_LINKKEYS:
            appPairingEnterCheckingHandsetLinkkeys(thePairing);
            break;

        case PAIRING_STATE_HANDSET_SDP_SEARCH_AUTHENTICATED:
            appPairingEnterHandsetSdpSearchAuthenticated(thePairing);
            break;

        default:
            break;
    }
}


/*! \brief Get Pairing FSM state

    Returns current state of the Pairing FSM.
*/
static pairingState appPairingGetState(pairingTaskData *thePairing)
{
    return thePairing->state;
}


static void appPairingPeerComplete(pairingTaskData *thePairing, pairingStatus status, const bdaddr *bd_addr, uint16 version)
{
    if (thePairing->client_task)
    {
        MAKE_PAIRING_MESSAGE(PAIRING_PEER_PAIR_CFM);
        if (bd_addr)
            message->peer_bd_addr = *bd_addr;
        else
            BdaddrSetZero(&message->peer_bd_addr);
        message->status = status;
        message->peer_version_major = version >> 8;
        message->peer_version_minor = version & 0xFF;
        MessageSend(thePairing->client_task, PAIRING_PEER_PAIR_CFM, message);
    }

    if ((status != pairingSuccess) && (status != pairingPeerCancelled))
        appUiPeerPairingError();
    else
    {
        /* Update link supervision timeout now we know the device is an Earbud */
        appLinkPolicyUpdateLinkSupervisionTimeout(bd_addr);
    }

    /* Move back to 'idle' state */
    appPairingSetState(thePairing, PAIRING_STATE_IDLE);
}

static void appPairingPeerUpdate(pairingTaskData *thePairing)
{
    appDeviceAttributes attributes;

    /* Add attributes */
    appDeviceInitAttributes(&attributes);
    attributes.type = DEVICE_TYPE_EARBUD;
    attributes.tws_version = thePairing->peer_tws_version;
    appDeviceSetAttributes(&thePairing->bd_addr[0], &attributes);

    /* Request local name the response will cause EIR data to be updated */
    ConnectionReadLocalName(&thePairing->task);

    /* Mark device as a priority device, so that it never falls off
     * the bottom of the trusted device list */
    ConnectionAuthSetPriorityDevice(&thePairing->bd_addr[0], TRUE);

    /* Update TWS+ service record */
    appPairingRegisterServiceRecord(thePairing);
}


static void appPairingHandsetComplete(pairingTaskData *thePairing, pairingStatus status, const bdaddr *bd_addr)
{
    if (thePairing->client_task)
    {
        MAKE_PAIRING_MESSAGE(PAIRING_HANDSET_PAIR_CFM);
        if (bd_addr)
            message->handset_bd_addr = *bd_addr;
        else
            BdaddrSetZero(&message->handset_bd_addr);
        message->status = status;
        MessageSend(thePairing->client_task, PAIRING_HANDSET_PAIR_CFM, message);
    }

    if (pairingHandsetSuccess == status)
    {
#ifndef DISABLE_TWS_PLUS
        PanicFalse(bd_addr != NULL);
        /* Perform service search for TWS+ Source UUID and version attribute */
        appConManagerQueryHandsetTwsVersion(bd_addr);
#endif
        appUiPairingComplete();
    }
    else
    {
        if (status != pairingHandsetCancelled)
            appUiPairingFailed();
    }

    /* Move back to 'idle' state */
    appPairingSetState(thePairing, PAIRING_STATE_IDLE);
}

/*! @brief Create attributes for a handset device. */
static void appPairingHandsetUpdate(bdaddr* handset_addr, uint16 tws_version, uint8 flags)
{
    appDeviceAttributes attributes;

    /* TODO may need to update this function to look for existing attributes
     * in case device already exists, if we have attributes (flags?) that we
     * don't want to forget. Currently this function will clear everything. */

    /* Add attributes, set flag indicating just paired with handset, this
       will temporarily prevent us from initiating connections to the handset */
    appDeviceInitAttributes(&attributes);
    attributes.type = DEVICE_TYPE_HANDSET;
    attributes.tws_version = tws_version;
    attributes.flags |= flags | DEVICE_FLAGS_JUST_PAIRED;
    appDeviceSetAttributes(handset_addr, &attributes);
}


static void appHandleClSdpRegisterCfm(pairingTaskData *thePairing,
                                      const CL_SDP_REGISTER_CFM_T *cfm)
{
    DEBUG_LOGF("appHandleClSdpRegisterCfm, status %d", cfm->status);

    switch (appPairingGetState(thePairing))
    {
        case PAIRING_STATE_INITIALISING:
        {
            if (cfm->status == sds_status_success)
            {
                /* Registration of service record was sucessful, so store service handle
                 * for use later on */
                PanicNotZero(thePairing->tws_sink_service_handle);
                thePairing->tws_sink_service_handle = cfm->service_handle;

                /* Inform main task that pairing is initialised */
                MessageSend(appGetAppTask(), PAIRING_INIT_CFM, NULL);

                /* Move to 'idle' state */
                appPairingSetState(thePairing, PAIRING_STATE_IDLE);
            }
            else
                Panic();
        }
        break;

        case PAIRING_STATE_PEER_AUTHENTICATE:
        case PAIRING_STATE_PEER_DISCOVERABLE:
        case PAIRING_STATE_PEER_SDP_SEARCH_AUTHENTICATED:
        {
            if (cfm->status == sds_status_success)
            {
                /* Registration of updated service record was successful, so update
                 * service handle for use later on */
                PanicNotZero(thePairing->tws_sink_service_handle);
                thePairing->tws_sink_service_handle = cfm->service_handle;

                if (appConfigScoForwardingEnabled())
                {
                    /* Check if peer supports SCO Forwarding with or without MIC forwarding */
                    appPairingSetState(thePairing, PAIRING_STATE_PEER_SCO_FWD_SDP_SEARCH);
                }
                else
                {
                    /* Send confirmation to main task */
                    appPairingPeerComplete(thePairing, pairingSuccess, &thePairing->bd_addr[0], thePairing->peer_tws_version);
                }
            }
            else
                Panic();
        }
        break;

        default:
            Panic();
    }

    /* Decrement SDP lock now that the record has been registered */
    thePairing->sdp_lock--;
}


static void appHandleClSdpUnregisterCfm(pairingTaskData *thePairing,
                                        const CL_SDP_UNREGISTER_CFM_T *cfm)
{
    DEBUG_LOGF("appHandleClSdpUnregisterCfm, status %d", cfm->status);

    if (cfm->status == sds_status_success)
    {
        if (thePairing->tws_sink_service_handle == cfm->service_handle)
        {
            thePairing->tws_sink_service_handle = 0;

            /* Re-register service record with new peer device address */
            appPairingRegisterServiceRecord(thePairing);
        }
        else if (cfm->status == sds_status_pending)
        {
            /* Wait for final confirmation message */
        }
        else
            Panic();
    }
}


static void appHandleClSdpServiceSearchAttributeCfm(pairingTaskData *thePairing,
                                                    const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm)
{
    DEBUG_LOGF("appHandleClSdpServiceSearchAttributeCfm, status %d", cfm->status);

    switch (appPairingGetState(thePairing))
    {
        case PAIRING_STATE_PEER_SDP_SEARCH:
        case PAIRING_STATE_PEER_SDP_SEARCH_AUTHENTICATED:
        {
            /* Extract TWS version number */
            thePairing->peer_tws_version = 0x0000;
            if (cfm->status == sdp_response_success && cfm->size_attributes &&
                appSdpFindTwsVersion(cfm->attributes, cfm->attributes + cfm->size_attributes, &thePairing->peer_tws_version))
            {
                DEBUG_LOGF("appHandleClSdpServiceSearchAttributeCfm, TWS Version %04x", thePairing->peer_tws_version);

                /* Major version number must match to allow pairing */
                if ((thePairing->peer_tws_version & 0xFF00) == (DEVICE_TWS_VERSION & 0xFF00))
                {
                    if (appPairingGetState(thePairing) == PAIRING_STATE_PEER_SDP_SEARCH)
                    {
                        /* Move to 'peer authenticate' state to start authentication */
                        appPairingSetState(thePairing, PAIRING_STATE_PEER_AUTHENTICATE);
                    }
                    else
                    {
                        /* Update attributes and service record */
                        appPairingPeerUpdate(thePairing);
                    }
                }
                else
                {
                    /* Send confirmation with error to main task */
                    appPairingPeerComplete(thePairing, pairingPeerVersionMismatch, &cfm->bd_addr, thePairing->peer_tws_version);
                }
            }
            else
            {
                /* SDP search failed, errors apart from no response data kicks off a retry */
                if (cfm->status != sdp_no_response_data)
                {
                    /* Increment counter, try again if haven't hit limit */
                    thePairing->sdp_search_attempts += 1;
                    if (thePairing->sdp_search_attempts < 5)
                    {
                        /* Send message to retry after 1 second */
                        MessageSendLater(&thePairing->task, PAIRING_INTERNAL_PEER_SDP_SEARCH, NULL, D_SEC(1));
                    }
                    else
                    {
                        /* Send confirmation with error to main task */
                        appPairingPeerComplete(thePairing, pairingNoPeerServiceRecord, &cfm->bd_addr, 0x0000);
                    }
                }
                else
                {
                    /* Send confirmation with error to main task */
                    appPairingPeerComplete(thePairing, pairingNoPeerServiceRecord, &cfm->bd_addr, 0x0000);
                }
            }
        }
        break;

        case PAIRING_STATE_HANDSET_SDP_SEARCH_AUTHENTICATED:
        {
            if (BdaddrIsSame(&thePairing->bd_addr[0], &cfm->bd_addr))
            {
                if (cfm->status == sdp_response_success)
                {
                    /* Received response, so extract TWS version from attribute */
                    uint16 tws_version = DEVICE_TWS_STANDARD;
                    appSdpFindTwsVersion(cfm->attributes, cfm->attributes + cfm->size_attributes, &tws_version);
                    appDeviceSetTwsVersion(&cfm->bd_addr, tws_version);
            
                    DEBUG_LOGF("appHandleClSdpServiceSearchAttributeCfm, TWS+ device %x,%x,%lx, version %d",
                                 cfm->bd_addr.nap, cfm->bd_addr.uap, cfm->bd_addr.lap, tws_version);
    
                    /* Send confirmation to main task */
                    appPairingHandsetComplete(thePairing, pairingHandsetSuccess, &cfm->bd_addr);
                }
                else if (cfm->status == sdp_no_response_data)
                {
                    DEBUG_LOGF("appHandleClSdpServiceSearchAttributeCfm, standard device %x,%x,%lx",
                                 cfm->bd_addr.nap, cfm->bd_addr.uap, cfm->bd_addr.lap);
            
                    /* No response data, so handset doesn't have UUID and/or version attribute, therefore
                       treat as standard handset */
                    appDeviceSetTwsVersion(&cfm->bd_addr, DEVICE_TWS_STANDARD);
    
                    /* Send confirmation to main task */
                    appPairingHandsetComplete(thePairing, pairingHandsetSuccess, &cfm->bd_addr);
                }
                else
                {
                    /* Increment counter, try again if haven't hit limit */
                    thePairing->sdp_search_attempts += 1;
                    if (thePairing->sdp_search_attempts < 5)
                    {
                        /* Send message to retry after 1 second */
                        MessageSendLater(&thePairing->task, PAIRING_INTERNAL_HANDSET_SDP_SEARCH, NULL, D_SEC(1));
                    }
                    else
                    {
                        /* SDP search failed even after retries */
                        appPairingHandsetComplete(thePairing, pairingHandsetUnknown, &cfm->bd_addr);
                    }
                }
            }
        }
        break;
        
        case PAIRING_STATE_IDLE:
        {
            /* Extract TWS version number */
            uint16 tws_version = 0x0000;
            if (cfm->status == sdp_response_success && cfm->size_attributes &&
                appSdpFindTwsVersion(cfm->attributes, cfm->attributes + cfm->size_attributes, &tws_version))

            DEBUG_LOGF("appHandleClSdpServiceSearchAttributeCfm, TWS Version %04x", tws_version);
        }
        break;

        case PAIRING_STATE_PEER_SCO_FWD_SDP_SEARCH:
        {
            uint16 supported_features;
            if (cfm->status == sdp_response_success && cfm->size_attributes &&
                appSdpGetScoFwdSupportedFeatures(cfm->attributes, cfm->attributes + cfm->size_attributes, &supported_features))
            {
                DEBUG_LOGF("appHandleClSdpServiceSearchAttributeCfm, peer SCO forward features %04x", supported_features);
                appDeviceSetScoForwardFeatures(&cfm->bd_addr, supported_features);
            }
            else
                DEBUG_LOG("appHandleClSdpServiceSearchAttributeCfm, no peer SCO forward features");

            /* Send confirmation to main task */
            appPairingPeerComplete(thePairing, pairingSuccess, &thePairing->bd_addr[0], thePairing->peer_tws_version);
        }
        break;

        default:
            Panic();
    }
}

static void appPairingUpdateLinkMode(const bdaddr *bd_addr, cl_sm_link_key_type key_type)
{
    /* Check if the key_type generated is p256. If yes then set the
    * attribute.mode to sink_mode_unknown. Once the encryption type is known in
    * CL_SM_ENCRYPTION_CHANGE_IND or  CL_SM_ENCRYPT_CFM message,device
    * attributes will be updated accordingly with proper mode.
    * Update the device attributes with this information to be reused later.
    */
    if((key_type == cl_sm_link_key_unauthenticated_p256) ||
        (key_type == cl_sm_link_key_authenticated_p256))
    {
        DEBUG_LOG("appPairingHandleClSmAuthenticateConfirm: link_mode UNKNOWN");
        appDeviceSetLinkMode(bd_addr, DEVICE_LINK_MODE_UNKNOWN);
    }
    else
    {
        DEBUG_LOG("appPairingHandleClSmAuthenticateConfirm: link_mode NO_SECURE_CONNECTION");
        appDeviceSetLinkMode(bd_addr, DEVICE_LINK_MODE_NO_SECURE_CONNECTION);
    }
}

void appPairingGenerateLinkKey(const bdaddr *bd_addr, const uint16 *lk_packed, uint32 key_id_in, uint16 *lk_derived)
{
    uint8 salt[SALT_SIZE];
    uint8 lk_unpacked[AES_CMAC_BLOCK_SIZE];
    uint8 key_h7[AES_CMAC_BLOCK_SIZE];
    uint8 key_h6[AES_CMAC_BLOCK_SIZE];
    uint32_t key_id = ((key_id_in & 0xFF000000UL) >> 24) +
                      ((key_id_in & 0x00FF0000UL) >> 8) +
                      ((key_id_in & 0x0000FF00UL) << 8) +
                      ((key_id_in & 0x000000FFUL) << 24);

    DEBUG_LOGF("Key Id: %08lx, reversed %08lx", key_id_in, key_id);

    DEBUG_PRINT("LK In: ");
    for (int lk_i = 0; lk_i < 8; lk_i++)
        DEBUG_PRINTF("%02x ", lk_packed[lk_i]);
    DEBUG_PRINT("\n");

    appSetupSaltWithPeerAddress(salt, bd_addr);
    DEBUG_PRINT("Salt: ");
    for (int salt_i = 0; salt_i < SALT_SIZE; salt_i++)
        DEBUG_PRINTF("%02x ", salt[salt_i]);
    DEBUG_PRINT("\n");

    /* Unpack the link key: 0xCCDD 0xAABB -> 0xAA 0xBB 0xCC 0xDD */
    for (int lk_i = 0; lk_i < (AES_CMAC_BLOCK_SIZE / 2); lk_i++)
    {
        lk_unpacked[(lk_i * 2) + 0] = (lk_packed[7 - lk_i] >> 8) & 0xFF;
        lk_unpacked[(lk_i * 2) + 1] = (lk_packed[7 - lk_i] >> 0) & 0xFF;
    }
    DEBUG_PRINT("LK In: ");
    for (int lk_i = 0; lk_i < AES_CMAC_BLOCK_SIZE; lk_i++)
        DEBUG_PRINTF("%02x ", lk_unpacked[lk_i]);
    DEBUG_PRINT("\n");

    CryptoVmH7(lk_unpacked, salt, key_h7);
    DEBUG_PRINT("H7: ");
    for (int h7_i = 0; h7_i < AES_CMAC_BLOCK_SIZE; h7_i++)
        DEBUG_PRINTF("%02x ", key_h7[h7_i]);
    DEBUG_PRINT("\n");

    CryptoVmH6(key_h7, key_id, key_h6);
    DEBUG_PRINT("H6: ");
    for (int h6_i = 0; h6_i < AES_CMAC_BLOCK_SIZE; h6_i++)
        DEBUG_PRINTF("%02x ", key_h6[h6_i]);
    DEBUG_PRINT("\n");

    /* Pack the link key: 0xAA 0xBB 0xCC 0xDD -> 0xCCDD 0xAABB */
    for (int lk_i = 0; lk_i < (AES_CMAC_BLOCK_SIZE / 2); lk_i++)
    {
        lk_derived[7 - lk_i] = (key_h6[(lk_i * 2) + 0] << 8) |
                               (key_h6[(lk_i * 2) + 1] << 0);
    }

    DEBUG_PRINT("LK Out: ");
    for (int lk_i = 0; lk_i < 8; lk_i++)
        DEBUG_PRINTF("%02x ", lk_derived[lk_i]);
    DEBUG_PRINT("\n");
}

/*! \brief Use link key for handset to derive key for peer earbud.
 */
static void appPairingHandleClSmGetAuthDeviceConfirm(pairingTaskData* thePairing,
                                                     CL_SM_GET_AUTH_DEVICE_CFM_T *cfm)
{
    DEBUG_LOGF("appPairingHandleClSmGetAuthDeviceConfirm %d", cfm->status);
    UNUSED(thePairing);

    if ((cfm->status == success) && (cfm->size_link_key == 8))
    {
        appDeviceAttributes attr;
        bdaddr peer_addr;

        if (appDeviceGetPeerAttributes(&peer_addr, &attr))
        {
            uint16 lk_out[8];

            appPairingGenerateLinkKey(&peer_addr, cfm->link_key, appConfigTwsKeyId(), lk_out);

            /* ask peer signalling to send the new link key to the peer */
            DEBUG_LOG("appPairingGenerateLinkKey derived link key - sending to peer");
            thePairing->outstanding_peer_sig_req++;
            appPeerSigLinkKeyToPeerRequest(&thePairing->task, &peer_addr, &cfm->bd_addr,
                                         lk_out, cfm->size_link_key);
        }
        else
        {
            /* No peer device */
        }
    }
    else
    {
        /* Failed to find link key data for handset, we just added it
         * this is bad. */
        Panic();
    }
}


/*! \brief Request creation of link key for peer earbud to handset link.
 */
static void appPairingDerivePeerHandsetLinkKey(bdaddr* handset_addr)
{
    appDeviceAttributes attributes;
    bdaddr bd_addr = {0, 0, 0};
    pairingTaskData* thePairing = appGetPairing();

     /* Only start key derivation (which will attempt to send the key
      * to the peer) if we are actually paired with a peer earbud.
      * It is fine to abort, the handset will be marked in the device
      * manager as requiring the link key and we'll send it once we
      * get a paired peer when the peer signalling comes up. */
    if (appDeviceGetPeerAttributes(&bd_addr, &attributes))
    {
        /* request link key for handset */
        ConnectionSmGetAuthDevice(&thePairing->task, handset_addr);
    }
}

/*! \brief Handle authentication confirmation

    The firmware has indicated that authentication with the remote device
    has completed, we should only receive this message in the pairing states.

    If we're pairing with a peer earbud, delete any previous pairing and then
    update the TWS+ service record with the address of the peer earbud.

    If we're pairing with a handset start SDP search to see if phone supports
    TWS+.
*/
static void appPairingHandleClSmAuthenticateConfirm(const CL_SM_AUTHENTICATE_CFM_T *cfm)
{
    pairingTaskData *thePairing = appGetPairing();

    DEBUG_LOGF("appPairingHandleClSmAuthenticateCfm, state %d, status %d, bonded %d", appPairingGetState(thePairing), cfm->status, cfm->bonded);

    switch (appPairingGetState(thePairing))
    {
        case PAIRING_STATE_PEER_AUTHENTICATE:
        case PAIRING_STATE_PEER_PENDING_AUTHENTICATION:
        {
            /* If authenticate was successful, delete any existing earbud pairing and
             * add attributes for new pairing */
            if (cfm->status == auth_status_success)
            {
                appDeviceAttributes attributes;
                bdaddr bd_addr = {0, 0, 0};

                /* Check if we have previously paired with an earbud */
                if (appDeviceGetPeerAttributes(&bd_addr, &attributes))
                {
                    /* Check it's actually a different earbud */
                    if (!BdaddrIsSame(&bd_addr, &cfm->bd_addr))
                    {
                        DEBUG_LOGF("appPairingHandleClSmAuthenticateCfm, deleting bd_addr %04x,%02x,%06lx", bd_addr.nap, bd_addr.uap, bd_addr.lap);

                        /* Delete pairing */
                        appDeviceDelete(&bd_addr);
                    }
                }

                if(cfm->bonded)
                {
                    /* Update the device link mode based on the key type */
                    appPairingUpdateLinkMode(&cfm->bd_addr, cfm->key_type);
                }

                /* If we're in 'peer authenticate' state we've now completed pairing,
                 * so we can go ahead and write device attributes and update TWS+
                 * service record with peer earbud's address
                 * If we're in 'peer discoverable' state we still need to perform SDP
                 * search to get peer earbuds TWS+ version */
                if (appPairingGetState(thePairing) == PAIRING_STATE_PEER_AUTHENTICATE)
                {
                    /* Update attributes and service record */
                    appPairingPeerUpdate(thePairing);
                }
                else
                {
                    /* Store device address of authenticated earbud */
                    thePairing->bd_addr[0] = cfm->bd_addr;

                    /* Move to 'peer SDP search authenticated' state to start SDP search */
                    appPairingSetState(thePairing, PAIRING_STATE_PEER_SDP_SEARCH_AUTHENTICATED);
                }
            }
            else
            {
                /* Send confirmation with error to main task */
                appPairingPeerComplete(thePairing, pairingPeerAuthenticationFailed, &cfm->bd_addr, 0x0000);
            }
        }
        break;

        case PAIRING_STATE_HANDSET_PENDING_AUTHENTICATION:
        {
            if (cfm->status == auth_status_success)
            {
                /* Add attributes, set pre paired flag if this address is known */
                if (BdaddrIsSame(&cfm->bd_addr, &thePairing->bd_addr[0]))
                    appPairingHandsetUpdate(&cfm->bd_addr, DEVICE_TWS_UNKNOWN, DEVICE_FLAGS_PRE_PAIRED_HANDSET);
                else
                    appPairingHandsetUpdate(&cfm->bd_addr, DEVICE_TWS_UNKNOWN, 0);

                if (cfm->bonded)
                {
                    /* Update the device link mode based on the key type */
                    appPairingUpdateLinkMode(&cfm->bd_addr, cfm->key_type);

                    /* Wait for TWS version, store BT address of authenticated device */
                    thePairing->bd_addr[0] = cfm->bd_addr;
#ifndef DISABLE_TWS_PLUS
                    appPairingSetState(thePairing, PAIRING_STATE_HANDSET_SDP_SEARCH_AUTHENTICATED);
#else
                    /* TWS+ disabled, so assume handset is standard handset */
                    appDeviceSetTwsVersion(&cfm->bd_addr, DEVICE_TWS_STANDARD);
    
                    /* Send confirmation to main task */
                    appPairingHandsetComplete(thePairing, pairingHandsetSuccess, &cfm->bd_addr);
#endif
                }
            }
            else
            {
                /* Decrement our pairing attempts*/
                thePairing->handset_pairing_attempts--;

                DEBUG_LOGF("appPairingHandleClSmAuthenticateCfm, status %d attempt left %d", cfm->status, thePairing->handset_pairing_attempts);
                /* Try again to pair when Paging timeout or authentication failure received */
                if ( (cfm->status == auth_status_timeout || cfm->status == auth_status_fail) &&
                     (thePairing->handset_pairing_attempts > 0) )
                {
                    smTaskData *sm = appGetSm();

                    // Retry only once again if this is an authentication failure
                    if(cfm->status == auth_status_fail)
                    {
                        thePairing->handset_pairing_attempts = 1;
                    }

                    // Reset to IDLE state to start again a clean pairing
                    appPairingSetState(thePairing, PAIRING_STATE_IDLE);

                    // Trigger a new pairing attempt with updated retry count
                    appPairingHandsetPairAddress(&sm->task, &cfm->bd_addr, thePairing->handset_pairing_attempts);
                }
                else
                {
                    /* Send confirmation with error to main task */
                    appPairingHandsetComplete(thePairing, pairingHandsetAuthenticationFailed, &cfm->bd_addr);
                }
            }
        }
        break;

        default:
        {
            if (cfm->status == auth_status_success)
                DEBUG_LOG("appPairingHandleClSmAuthenticateCfm, unexpected authentication");
        }
        break;
    }
}


/*! \brief Handle authorisation request for unknown device

    This function is called to handle an authorisation request for a unknown
    device.

    TODO: Update "In all cases we authorise the device, and if we're in pairing
    mode we mark the device as trusted."
*/
static bool appPairingHandleClSmAuthoriseIndication(const CL_SM_AUTHORISE_IND_T *ind)
{
    pairingTaskData *thePairing = appGetPairing();

    DEBUG_LOGF("appPairingHandleClSmAuthoriseIndication, state %d, protocol %d, channel %d, incoming %d",
                   appPairingGetState(thePairing), ind->protocol_id, ind->channel, ind->incoming);

    /*! \todo BLE: The pairing module doesn't handle LE connections at present */
    if (ind->protocol_id == protocol_le_l2cap)
    {
        return FALSE;
    }

    /* Special case handling for incoming SDP */
    if ((ind->protocol_id == protocol_l2cap) && (ind->channel == 1) && ind->incoming && thePairing->sdp_lock)
    {
        /* Hold off authorising SDP if we're in the process of re-registering SDP, send message back to
           us conditionally on SDK lock being cleared */
        MAKE_PAIRING_MESSAGE(CL_SM_AUTHORISE_IND);
        *message = *ind;
        MessageSendConditionally(appGetAppTask(), CL_SM_AUTHORISE_IND, message, &thePairing->sdp_lock);

        /* Return indicating CL_SM_AUTHORISE_IND will be handled by us */
        return TRUE;
    }
    else if ((ind->protocol_id == protocol_l2cap) && (ind->channel == 1))
    {
        /* SDP L2CAP, always allow */
        ConnectionSmAuthoriseResponse(&ind->bd_addr, ind->protocol_id, ind->channel, ind->incoming, TRUE);

        /* Return indicating CL_SM_AUTHORISE_IND will be handled by us */
        return TRUE;
    }
    else if (appPairingGetState(thePairing) == PAIRING_STATE_PEER_SDP_SEARCH_AUTHENTICATED)
    {
        /* Waiting for SDP search when pairing with peer, should allow peer to connect */
        ConnectionSmAuthoriseResponse(&ind->bd_addr, ind->protocol_id, ind->channel, ind->incoming, TRUE);

        /* Return indicating CL_SM_AUTHORISE_IND will be handled by us */
        return TRUE;
    }
    else if (appDeviceIsPeer(&ind->bd_addr))
    {
        /* Connection is to peer, so nothing to do with handset pairing. Let 
           connection mananger deal with authorisation by returning FALSE
           indicating CL_SM_AUTHORISE_IND hasn't been handled, caller has to 
           deal with it */
        return FALSE;
    }
    else if (appPairingGetState(thePairing) == PAIRING_STATE_HANDSET_DISCOVERABLE)
    {
        /* handset pairing in progress and it is early enough to cancel it
         * cancel it and accept the connection */
        appPairingHandsetPairCancel();
        ConnectionSmAuthoriseResponse(&ind->bd_addr, ind->protocol_id, ind->channel, ind->incoming, TRUE);

        /* Return indicating CL_SM_AUTHORISE_IND will be handled by us */
        return TRUE;
    }
    else if (appPairingGetState(thePairing) == PAIRING_STATE_HANDSET_PENDING_AUTHENTICATION)
    {
        /* handset pairing in progress, but too late to cancel the pairing,
         * refuse the connection */
        ConnectionSmAuthoriseResponse(&ind->bd_addr, ind->protocol_id, ind->channel, ind->incoming, FALSE);

        /* Return indicating CL_SM_AUTHORISE_IND will be handled by us */
        return TRUE;
    }
    else if (appPairingGetState(thePairing) == PAIRING_STATE_HANDSET_SDP_SEARCH_AUTHENTICATED)
    {
        if (BdaddrIsSame(&thePairing->bd_addr[0], &ind->bd_addr))
        {
            /* Handset pairing in progress, address matches so handset is
             * connecting quicker than our SDP search, allow connection */
            ConnectionSmAuthoriseResponse(&ind->bd_addr, ind->protocol_id, ind->channel, ind->incoming, TRUE);
        }
        else
        {
            /* Handset pairing in progress, but too late to cancel the pairing,
             * refuse the connection */
            ConnectionSmAuthoriseResponse(&ind->bd_addr, ind->protocol_id, ind->channel, ind->incoming, FALSE);
        }

        /* Return indicating CL_SM_AUTHORISE_IND will be handled by us */
        return TRUE;
    }
    else
    {
        /* Return indicating CL_SM_AUTHORISE_IND hasn't been handled, caller has to deal with it */
        return FALSE;
    }
}


/*! \brief Handle IO capabilities request

    This function is called when the remote device wants to know the headsets IO
    capabilities.  If we are pairing we respond indicating the headset has no input
    or ouput capabilities, if we're not pairing then just reject the request.
*/
static void appPairingHandleClSmIoCapabilityReqIndication(const CL_SM_IO_CAPABILITY_REQ_IND_T *ind)
{
    pairingTaskData *thePairing = appGetPairing();
    bool accept = FALSE;
    uint16 key_dist = KEY_DIST_NONE;

    DEBUG_LOG("appPairingHandleClSmIoCapabilityReqIndication Pairing state %d. Type:%d Transp:%d sm_over_BREDR:%d",
                    appPairingGetState(thePairing), ind->tpaddr.taddr.type,
                    ind->tpaddr.transport,ind->sm_over_bredr);

    switch (appPairingGetState(thePairing))
    {
        case PAIRING_STATE_PEER_AUTHENTICATE:
            accept = TRUE;
            break;

        case PAIRING_STATE_HANDSET_PENDING_AUTHENTICATION:
            accept = TRUE;
            break;

        case PAIRING_STATE_PEER_DISCOVERABLE:
            accept = TRUE;
            appPairingSetState(thePairing, PAIRING_STATE_PEER_PENDING_AUTHENTICATION);
            break;

        case PAIRING_STATE_HANDSET_DISCOVERABLE:
            accept = TRUE;
            appPairingSetState(thePairing, PAIRING_STATE_HANDSET_PENDING_AUTHENTICATION);
            break;

#ifdef APP_SECURE_CONNECTIONS
            /* If using secure connections, we can produce BLE pairing immediately after
               BREDR pairing has completed */
        case PAIRING_STATE_HANDSET_SDP_SEARCH_AUTHENTICATED:
#endif
        /* If we have secure connections enabled, this is when cross transport
            pairing for our peer earbud arives. 
        case PAIRING_STATE_PEER_SCO_FWD_SDP_SEARCH: */
        case PAIRING_STATE_IDLE:
            if (ind->tpaddr.transport == TRANSPORT_BLE_ACL)
            {
                bool random_address = (ind->tpaddr.taddr.type == TYPED_BDADDR_RANDOM);
                bool existing =  appDeviceFindBdAddrAttributes(&ind->tpaddr.taddr.addr, NULL);

                /* Clear expected address for BLE pairing. We should not have a crossover
                   but in any case, safest to remove any pending */
                memset(&thePairing->pending_ble_address,0,sizeof(thePairing->pending_ble_address));

                DEBUG_LOG("appPairingHandleClSmIoCapabilityReqIndication Handling BLE pairing. Addr %06x",ind->tpaddr.taddr.addr.lap);
                DEBUG_LOG("appPairingHandleClSmIoCapabilityReqIndication. random:%d ble_permission:%d sm_over_bredr:%d",
                    random_address, thePairing->ble_permission, ind->sm_over_bredr );

                    /* BLE pairing is not allowed */
                if (thePairing->ble_permission == pairingBleDisallowed)
                {
                    break;
                }
                    /* Eliminate this being a valid Cross Transport Key derivation(CTKD) */
                if (ind->sm_over_bredr && !existing)
                {
                    break;
                }
                    /* Static address only allowed in one permission mode, anything else is OK now */
                if (   !random_address && !existing 
                    && thePairing->ble_permission != pairingBleAllowAll)
                {
                    break;
                }

                accept = TRUE;

                key_dist = (KEY_DIST_RESPONDER_ENC_CENTRAL |
                            KEY_DIST_RESPONDER_ID |
                            KEY_DIST_INITIATOR_ENC_CENTRAL |
                            KEY_DIST_INITIATOR_ID);

                thePairing->pending_ble_address = ind->tpaddr.taddr;
            }
            break;

        default:
            break;
    }

    DEBUG_LOGF("appPairingHandleClSmIoCapabilityReqIndication, accept %d", accept);

    ConnectionSmIoCapabilityResponse(&ind->tpaddr,
                                     accept ? cl_sm_io_cap_no_input_no_output : cl_sm_reject_request,
                                     mitm_not_required,
                                     TRUE,
                                     key_dist,
                                     oob_data_none,
                                     0,
                                     0);
}

static void appPairingHandleClSmRemoteIoCapabilityIndication(const CL_SM_REMOTE_IO_CAPABILITY_IND_T *ind)
{
    UNUSED(ind);
    DEBUG_LOGF("appPairingHandleClSmRemoteIoCapabilityIndication %lx auth:%x io:%x oib:%x", ind->tpaddr.taddr.addr.lap,
                                                                       ind->authentication_requirements,
                                                                       ind->io_capability,
                                                                       ind->oob_data_present);
}


/************************************************************************/
static void appHandleClDmInquireResult(pairingTaskData *thePairing, const CL_DM_INQUIRE_RESULT_T *result)
{
    switch (appPairingGetState(thePairing))
    {
        case PAIRING_STATE_PEER_INQUIRY:
        {
            if (result->status == inquiry_status_result)
            {
                DEBUG_LOGF("appHandleClDmInquireResult, bdaddr %04x,%02x,%06lx rssi %d cod %lx",
                            result->bd_addr.nap,
                            result->bd_addr.uap,
                            result->bd_addr.lap,
                            result->rssi,
                            result->dev_class);

                /* Cache result if RSSI is higher */
                if (result->rssi > thePairing->inquiry_rssi[0])
                {
                    /* Check if address is different from previous peak */
                    if (!BdaddrIsSame(&result->bd_addr, &thePairing->bd_addr[0]))
                    {
                        /* Store previous peak RSSI */
                        thePairing->inquiry_rssi[1] = thePairing->inquiry_rssi[0];
                        thePairing->bd_addr[1] = thePairing->bd_addr[0];

                        /* Store new address */
                       thePairing->bd_addr[0] = result->bd_addr;
                    }

                    /* Store peak RSSI */
                    thePairing->inquiry_rssi[0] = result->rssi;
                }
                else if (result->rssi > thePairing->inquiry_rssi[1])
                {
                    /* Check if address is different from peak */
                    if (!BdaddrIsSame(&result->bd_addr, &thePairing->bd_addr[0]))
                    {
                        /* Store next highest RSSI */
                        thePairing->inquiry_rssi[1] = result->rssi;
                    }
                }
            }
            else
            {
                DEBUG_LOGF("appHandleClDmInquireResult, complete, status %d, bdaddr %x,%x,%lx, rssi %d, next_rssi %d",
                            result->status,
                            thePairing->bd_addr[0].nap,
                            thePairing->bd_addr[0].uap,
                            thePairing->bd_addr[0].lap,
                            thePairing->inquiry_rssi[0],
                            thePairing->inquiry_rssi[1]);

                /* Attempt to connect to device with highest RSSI */
                if (!BdaddrIsZero(&thePairing->bd_addr[0]))
                {
                    /* Check if RSSI peak is sufficently higher than next */
                    if ((thePairing->inquiry_rssi[1] - thePairing->inquiry_rssi[0]) <= -appConfigPeerPairingMinRssiDelta())
                    {
                        DEBUG_LOG("appHandleClDmInquireResult, performing SDP service search");

                        /* Move to 'peer sdp search' state */
                        appPairingSetState(thePairing, PAIRING_STATE_PEER_SDP_SEARCH);
                    }
                    else
                    {
                        DEBUG_LOG("appHandleClDmInquireResult, Inquiry error, too many devices nearby or unable to create new instance");

                        /* Send confirmation with error to main task */
                        if (thePairing->is_user_initiated)
                            appPairingPeerComplete(thePairing, pairingNoPeerFound, NULL, 0x0000);
                        else
                        {
                            /* Restart inquiry */
                            ConnectionInquire(&thePairing->task, 0x9E8B30, 20,
                                              5,
                                              AUDIO_MAJOR_SERV_CLASS | RENDER_MAJOR_SERV_CLASS |
                                              AV_MAJOR_DEVICE_CLASS | HEADSET_MINOR_DEVICE_CLASS);
                        }
                    }
                }
                else
                {
                    /* Restart inquiry */
                    ConnectionInquire(&thePairing->task, 0x9E8B30, 20,
                                      5,
                                      AUDIO_MAJOR_SERV_CLASS | RENDER_MAJOR_SERV_CLASS |
                                      AV_MAJOR_DEVICE_CLASS | HEADSET_MINOR_DEVICE_CLASS);
                }
            }
        }
        break;

        default:
        {
            /* Ignore inquire result */
        }
        break;
    }
}


/*! \brief Local name request completed

    The request for the local device name has completed, pass the name to
    appInquiryEirInit() to allow the Extended Inquiry Response to be initialised.
*/
static void appHandleClDmLocalNameComplete(CL_DM_LOCAL_NAME_COMPLETE_T *msg)
{
    DEBUG_LOGF("appHandleClDmLocalNameComplete, status %d", msg->status);

    /* Initialise pairing module, this will set EIR data */
    appPairingInitialiseEir(msg->local_name, msg->size_local_name);
}



static void appHandleInternalPeerPairRequest(pairingTaskData *thePairing, PAIR_REQ_T *req)
{
    DEBUG_LOGF("appHandleInternalPeerPairRequest, state %d", appPairingGetState(thePairing));

    switch (appPairingGetState(thePairing))
    {
        case PAIRING_STATE_IDLE:
        {
            /* Store client task */
            thePairing->client_task = req->client_task;
            thePairing->is_user_initiated = req->is_user_initiated;

            /* Reset SDP search attempts count */
            thePairing->sdp_search_attempts = 0;

            if (appConfigIsLeft())
            {
                /* Move to 'peer inquiry' state to start inquiry */
                appPairingSetState(thePairing, PAIRING_STATE_PEER_INQUIRY);
            }
            else
            {
                /* Move to 'peer discoverable' state so peer device can discover us */
                appPairingSetState(thePairing, PAIRING_STATE_PEER_DISCOVERABLE);
            }
        }
        break;

        default:
        {
            MAKE_PAIRING_MESSAGE(PAIRING_PEER_PAIR_CFM);

            /* Send confirmation with error to main task */
            message->status = pairingNotReady;
            MessageSend(req->client_task, PAIRING_PEER_PAIR_CFM, message);
        }
        break;
    }
}

/*! \brief Handle request to start pairing with a handset. */
static void appHandleInternalHandsetPairRequest(pairingTaskData *thePairing, PAIR_REQ_T *req)
{
    DEBUG_LOGF("appHandleInternalHandsetPairRequest, state %d", appPairingGetState(thePairing));

    switch (appPairingGetState(thePairing))
    {
        case PAIRING_STATE_IDLE:
        {
            /* Store client task */
            thePairing->client_task = req->client_task;

            /* Reset SDP search attempts count */
            thePairing->sdp_search_attempts = 0;                        

            /* Store address of handset to pair with, 0 we should go discoverable */
            thePairing->bd_addr[0] = req->addr;

            /* no address, go discoverable for inquiry process */
            if (BdaddrIsZero(&req->addr))
            {
                /* Move to 'discoverable' state to start inquiry & page scan */
                appPairingSetState(thePairing, PAIRING_STATE_HANDSET_DISCOVERABLE);
            }
            else
            {                
                /* Set the pairing attempts count */
                thePairing->handset_pairing_attempts = req->num_attempts;

                /* Address of handset known, just start authentication with 90 second timeout */
                ConnectionSmAuthenticate(appGetAppTask(), &req->addr, 90);
                appPairingSetState(thePairing, PAIRING_STATE_HANDSET_PENDING_AUTHENTICATION);
            }
        }
        break;

        default:
        {
            MAKE_PAIRING_MESSAGE(PAIRING_HANDSET_PAIR_CFM);

            /* Send confirmation with error to main task */
            message->status = pairingNotReady;
            MessageSend(req->client_task, PAIRING_HANDSET_PAIR_CFM, message);
        }
        break;
    }
}


static void appHandleInternalTimeoutIndications(pairingTaskData *thePairing)
{
    switch (appPairingGetState(thePairing))
    {
        case PAIRING_STATE_HANDSET_DISCOVERABLE:
        {
            /* Send confirmation with error to main task */
            appPairingHandsetComplete(thePairing, pairingHandsetTimeout, NULL);
        }
        break;

        case PAIRING_STATE_PEER_INQUIRY:
        case PAIRING_STATE_PEER_DISCOVERABLE:
        {
            /* Send confirmation with error to main task */
            appPairingPeerComplete(thePairing, pairingPeerTimeout, NULL, thePairing->peer_tws_version);
        }
        break;

        default:
            break;
    }
}

/*! \brief Handle request to cancel peer pairing. */
static void appHandleInternalPeerPairCancel(pairingTaskData* thePairing)
{
    DEBUG_LOG("appHandleInternalPeerPairCancel");

    switch (appPairingGetState(thePairing))
    {
        case PAIRING_STATE_PEER_INQUIRY:
        case PAIRING_STATE_PEER_SDP_SEARCH:
        case PAIRING_STATE_PEER_DISCOVERABLE:
            /* just send complete message with cancelled status, there is an auto
             * transition back to idle after sending the message */
            appPairingPeerComplete(thePairing, pairingPeerCancelled, NULL, 0x0000);
            break;
        default:
            break;
    }
}

/*! \brief Handle request to cancel handset pairing. */
static void appHandleInternalHandsetPairCancel(pairingTaskData* thePairing)
{
    DEBUG_LOG("appHandleInternalHandsetPairCancel");

    switch (appPairingGetState(thePairing))
    {
        case PAIRING_STATE_HANDSET_DISCOVERABLE:
            /* just send complete message with cancelled status, there is an auto
             * transition back to idle after sending the message */
            appPairingHandsetComplete(thePairing, pairingHandsetCancelled, NULL);
            break;
        default:
            break;
    }
}

static void appHandleInternalPeerSdpSearch(pairingTaskData *thePairing)
{
    DEBUG_LOG("appHandleInternalPeerSdpSearch");

    switch (appPairingGetState(thePairing))
    {
        case PAIRING_STATE_PEER_SDP_SEARCH:
        case PAIRING_STATE_PEER_SDP_SEARCH_AUTHENTICATED:
            /* Re-enter state to re-start SDP search */
            appPairingSetState(thePairing, appPairingGetState(thePairing));
            break;
        default:
            break;
    }
}


static void appHandleInternalHandsetSdpSearch(pairingTaskData *thePairing)
{
    DEBUG_LOG("appHandleInternalHandsetSdpSearch");

    switch (appPairingGetState(thePairing))
    {
        case PAIRING_STATE_HANDSET_SDP_SEARCH_AUTHENTICATED:
            /* Re-enter state to re-start SDP search */
            appPairingSetState(thePairing, appPairingGetState(thePairing));
            break;
        default:
            break;
    }
}


/*! \brief Handle request to check if any handset links key still need to be sent to peer. */
static void appHandleInternalCheckHandsetLinkkeys(pairingTaskData* thePairing)
{
    switch (appPairingGetState(thePairing))
    {
        case PAIRING_STATE_IDLE:
            /* only valid in idle state */
            appPairingSetState(thePairing, PAIRING_STATE_CHECKING_HANDSET_LINKKEYS);
        break;

        default:
            DEBUG_LOGF("appHandleInternalCheckHandsetLinkkeys bad state %d",
                         appPairingGetState(thePairing));
        break;
    }
}


/*! \brief Handle receipt of message to disable scanning.
 */
static void appHandleInternalDisableScan(void)
{
    DEBUG_LOG("appHandleInternalDisableScan");

    /* Pairing is idle, no inquiry or page scan needed */
    appScanManagerDisableInquiryPageScan(SCAN_MAN_USER_PAIRING);
}


/*! \brief Handle receipt of handset link key from peer headset and store in PDL.
 */
static void appPairingHandlePeerSigLinkKeyRxInd(pairingTaskData* thePairing,
                                                PEER_SIG_LINK_KEY_RX_IND_T *ind)
{
    DEBUG_LOGF("appPairingHandlePeerSigLinkKeyRxInd %d", ind->status);

    if (ind->status == peerSigStatusSuccess)
    {
        /* Store the link key in the PDL */
        ConnectionSmAddAuthDevice(&thePairing->task, &ind->handset_addr,
                                  FALSE,
                                  TRUE,
                                  cl_sm_link_key_unauthenticated_p192, /* TODO is this right? */
                                  ind->key_len,
                                  ind->key);
    }
    else
    {
        /* We shouldn't get non-success message so panic we got a code bug */
        Panic();
    }
}

/*! \brief Handle result of attempt to send handset link key to peer headset. */
static void appPairingHandlePeerSigLinkKeyTxConfirm(pairingTaskData* thePairing,
                                                    PEER_SIG_LINK_KEY_TX_CFM_T *cfm)
{
    UNUSED(thePairing);

    DEBUG_LOGF("appPairingHandlePeerSigLinkKeyTxConfirm %d", cfm->status);

    if (cfm->status == peerSigStatusSuccess)
    {
        /* update device manager that we have successfully sent
         * handset link key to peer headset, clear the flag */
        appDeviceSetHandsetLinkKeyTxReqd(&cfm->handset_addr, FALSE);
    }
    else
    {
        DEBUG_LOG("Failed to send handset link key to peer earbud!");
    }
    thePairing->outstanding_peer_sig_req--;
}

static void appPairingHandlePeerSigPairHandsetConfirm(pairingTaskData* thePairing, PEER_SIG_PAIR_HANDSET_CFM_T *cfm)
{
    DEBUG_LOGF("appPairingHandlePeerSigPairHandsetConfirm %d", cfm->status);

    if (cfm->status == peerSigStatusSuccess)
    {
        appDeviceSetHandsetAddressForwardReq(&cfm->handset_addr, FALSE);
    }
    else
    {
        DEBUG_LOG("Failed to send standard handset address to peer earbud");
    }
    thePairing->outstanding_peer_sig_req--;
}


/*! \brief Handle confirmation of adding link key for handset to PDL
 */
static void appPairingHandleClSmAddAuthDeviceConfirm(CL_SM_ADD_AUTH_DEVICE_CFM_T *cfm)
{
    DEBUG_LOGF("appPairingHandleClSmAddAuthDeviceConfirm %d", cfm->status);

    /* Complete setup by adding device attributes for the handset
     * default to TWS+ */
    appPairingHandsetUpdate(&cfm->bd_addr, DEVICE_TWS_VERSION, DEVICE_FLAGS_PRE_PAIRED_HANDSET);

    /* Set event indicating we've received a handset link-key, this will start peer
     * synchronisation which will ultimately result in us connecting to the handset */
    appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_RX_HANDSET_LINKKEY);
}


/*! \brief Handler for CL_SM_SEC_MODE_CONFIG_CFM.

    Handle Security mode confirmation */
static void appPairingHandleClSmSecurityModeConfigConfirm(const CL_SM_SEC_MODE_CONFIG_CFM_T *cfm)
{
    DEBUG_LOG("appPairingHandleClSmSecurityModeConfigConfirm");

    /* Check if setting security mode was successful */
    if (!cfm->success)
        Panic();
}


/*  Handle Pin code indication

    The firmware has indicated that it requires the pin code, we should
    only send the pin code response if the application is in one of the
    pairing mode, otherwise we send a NULL pin code to reject the request.
*/
static void appPairingHandleClPinCodeIndication(const CL_SM_PIN_CODE_IND_T *ind)
{
    uint16 pin_length = 0;
    uint8 pin[16];

    DEBUG_LOG("appPairingHandleClPinCodeIndication");

    /* Only respond to pin indication request when pairing or inquiry */
    if (appSmIsPairing())
    {
        /* Attempt to retrieve fixed pin in PS, if not reject pairing */
        pin_length = PsFullRetrieve(PSKEY_FIXED_PIN, pin, sizeof(pin));
        if (pin_length > 16)
            pin_length = 0;

        /* Respond to the PIN code request */
        ConnectionSmPinCodeResponse(&ind->taddr, pin_length, pin);
    }
    else
    {
        /* Respond to the PIN code request with empty pin code */
        ConnectionSmPinCodeResponse(&ind->taddr, 0, (uint8 *)"");
    }
}


/*  Handle the user passkey confirmation.

    This function is called to handle confirmation for the user that the passkey
    matches, since the headset doesn't have a display we just send a reject
    immediately.
*/
static void appPairingHandleClSmUserConfirmationReqIndication(const CL_SM_USER_CONFIRMATION_REQ_IND_T *ind)
{
    DEBUG_LOG("appPairingHandleClSmUserConfirmationReqIndication");

    ConnectionSmUserConfirmationResponse(&ind->tpaddr, FALSE);
}


/*! \brief Handler for CL_SM_USER_PASSKEY_REQ_IND.
*/
static void appPairingHandleClSmUserPasskeyReqIndication(const CL_SM_USER_PASSKEY_REQ_IND_T *ind)
{
    DEBUG_LOG("appPairingHandleClSmUserPasskeyReqIndication");

    ConnectionSmUserPasskeyResponse(&ind->tpaddr, TRUE, 0);
}


/*! \brief Handler for CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND_T . */
static void appPairingHandleClSmBleSimplePairingCompleteInd(const CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND_T *ind)
{
    pairingTaskData *thePairing = appGetPairing();
    bool current_request = BdaddrTypedIsSame(&thePairing->pending_ble_address,
                                             &ind->tpaddr.taddr);
    bool any_pending = !BdaddrTypedIsEmpty(&thePairing->pending_ble_address);
    pairingBlePermission permission = thePairing->ble_permission;

    DEBUG_LOG("appPairingHandleClSmBleSimplePairingCompleteInd Any:%d Matches:%d Permission %d",
                    any_pending, current_request, permission);

    if (any_pending && current_request && permission > pairingBleDisallowed)
    {
        bool hset = appDeviceIsHandset(&ind->permanent_taddr.addr);
        bool permanent = !BdaddrTypedIsEmpty(&ind->permanent_taddr);

        DEBUG_LOG("appPairingHandleClSmBleSimplePairingCompleteInd Handset:%d Permanent:%d",
                    hset, permanent);

        if (!hset)
        {
            hset = appDeviceTypeIsHandset(&ind->permanent_taddr.addr);
            DEBUG_LOG("appPairingHandleClSmBleSimplePairingCompleteInd. Note device is a handset, but no TWS information");
        }

        if (   (permission == pairingBleOnlyPairedHandsets && hset)
            || (permission == pairingBleAllowOnlyResolvable && permanent)
            || (permission == pairingBleAllowAll))
        {
            if (hset || permanent)
            {
                /* Mark this device as having a resolvable address */
                appDeviceSetBlePairing(&ind->permanent_taddr.addr, TRUE);
            }
            DEBUG_LOG("appPairingHandleClSmBleSimplePairingCompleteInd Link is expected");
        }
        else
        {
            uint16 cid = GattGetCidForBdaddr(&ind->tpaddr.taddr);

            if (cid && cid != INVALID_CID)
            {
                DEBUG_LOG("appPairingHandleClSmBleSimplePairingCompleteInd disconnect GATT cid %d",cid);
                GattDisconnectRequest(cid);
            }

            if (permanent)
            {
                ConnectionSmDeleteAuthDeviceReq(ind->permanent_taddr.type, &ind->permanent_taddr.addr);
            }
            else
            {
                ConnectionSmDeleteAuthDeviceReq(ind->tpaddr.taddr.type, &ind->tpaddr.taddr.addr);
            }
        }
        memset(&thePairing->pending_ble_address,0,sizeof(thePairing->pending_ble_address));
    }
    
}


bool appPairingHandleConnectionLibraryMessages(MessageId id,Message message, bool already_handled)
{
    switch (id)
    {
        case CL_SM_SEC_MODE_CONFIG_CFM:
            appPairingHandleClSmSecurityModeConfigConfirm((CL_SM_SEC_MODE_CONFIG_CFM_T *)message);
            return TRUE;

        case CL_SM_PIN_CODE_IND:
            appPairingHandleClPinCodeIndication((CL_SM_PIN_CODE_IND_T *)message);
            return TRUE;

        case CL_SM_AUTHENTICATE_CFM:
            appPairingHandleClSmAuthenticateConfirm((CL_SM_AUTHENTICATE_CFM_T *)message);
            return TRUE;

        case CL_SM_AUTHORISE_IND:
            return appPairingHandleClSmAuthoriseIndication((CL_SM_AUTHORISE_IND_T *)message);

        case CL_SM_IO_CAPABILITY_REQ_IND:
            appPairingHandleClSmIoCapabilityReqIndication((CL_SM_IO_CAPABILITY_REQ_IND_T *)message);
            return TRUE;

        case CL_SM_USER_CONFIRMATION_REQ_IND:
            appPairingHandleClSmUserConfirmationReqIndication((CL_SM_USER_CONFIRMATION_REQ_IND_T *)message);
            return TRUE;
    
        case CL_SM_REMOTE_IO_CAPABILITY_IND:
            appPairingHandleClSmRemoteIoCapabilityIndication((CL_SM_REMOTE_IO_CAPABILITY_IND_T *)message);
            return TRUE;

        case CL_SM_USER_PASSKEY_REQ_IND:
            appPairingHandleClSmUserPasskeyReqIndication((CL_SM_USER_PASSKEY_REQ_IND_T *)message);
            return TRUE;

        case CL_SM_USER_PASSKEY_NOTIFICATION_IND:
        case CL_SM_KEYPRESS_NOTIFICATION_IND:
        case CL_DM_WRITE_APT_CFM:
                /* These messages are associated with pairing, although as 
                   indications they required no handling */
            return TRUE;

        case CL_DM_LINK_SUPERVISION_TIMEOUT_IND:
            return TRUE;
            
        case CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND:
            appPairingHandleClSmBleSimplePairingCompleteInd((const CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND_T *)message);
            return TRUE;
    }
    return already_handled;
}


/*! @brief Find all known handsets that still require link key to be sent to peer earbud.
 */
static void appPairingFindHandsetsNeedingLinkKeyTx(void)
{
    pairingTaskData *thePairing = appGetPairing();
    int index = 0;
    appDeviceAttributes attr;
    bdaddr handset_addr;

    DEBUG_LOG("appPairingFindHandsetsNeedingLinkKeyTx");

    /* iterate through handset devices, for each we find start link key
     * derivation if it is a TWS+ handset and we still need to send key
     * to peer earbud */
    while (appDeviceGetHandsetAttributes(&handset_addr, &attr, &index))
    {
        if (   appDeviceTwsVersion(&handset_addr) == DEVICE_TWS_VERSION
            && appDeviceHandsetAttrIsLinkKeyTxReqd(&attr))
        {
            DEBUG_LOG("appPairingFindHandsetsNeedingLinkKeyTx need to TX link key");
            appPairingDerivePeerHandsetLinkKey(&handset_addr);
        }

        if (   appDeviceTwsVersion(&handset_addr) == DEVICE_TWS_STANDARD
            && appDeviceHandsetAttrIsAddressForwardReqd(&attr))
        {
            DEBUG_LOG("appPairingFindHandsetsNeedingLinkKeyTx need to forward handset address");
            bdaddr peer_addr;
            if (appDeviceGetPeerBdAddr(&peer_addr))
            {
                thePairing->outstanding_peer_sig_req++;
                appPeerSigTxPairHandsetRequest(&thePairing->task, &peer_addr, &handset_addr);
            }
        }

        /* keep looking for more handset devices */
        index++;
    }
}


/*! \brief Message Handler

    This function is the main message handler for the pairing module.
*/
static void appPairingHandleMessage(Task task, MessageId id, Message message)
{
    pairingTaskData *thePairing = (pairingTaskData *)task;

    switch (id)
    {
        case CL_SDP_REGISTER_CFM:
            appHandleClSdpRegisterCfm(thePairing, (CL_SDP_REGISTER_CFM_T *)message);
            return;

        case CL_SDP_UNREGISTER_CFM:
            appHandleClSdpUnregisterCfm(thePairing, (CL_SDP_UNREGISTER_CFM_T *)message);
            return;

        case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
            appHandleClSdpServiceSearchAttributeCfm(thePairing, (CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *)message);
            return;

        case CL_DM_INQUIRE_RESULT:
            appHandleClDmInquireResult(thePairing, (CL_DM_INQUIRE_RESULT_T *)message);
            return;

        case CL_DM_LOCAL_NAME_COMPLETE:
            appHandleClDmLocalNameComplete((CL_DM_LOCAL_NAME_COMPLETE_T *)message);
            return;

        case CL_DM_WRITE_INQUIRY_MODE_CFM:
        case CL_DM_WRITE_INQUIRY_ACCESS_CODE_CFM:
            return;

        case CL_SM_GET_AUTH_DEVICE_CFM:
            appPairingHandleClSmGetAuthDeviceConfirm(thePairing, (CL_SM_GET_AUTH_DEVICE_CFM_T*)message);
            return;

        case PAIRING_INTERNAL_PEER_PAIR_REQ:
            appHandleInternalPeerPairRequest(thePairing, (PAIR_REQ_T *)message);
            return;

        case PAIRING_INTERNAL_HANDSET_PAIR_REQ:
            appHandleInternalHandsetPairRequest(thePairing, (PAIR_REQ_T *)message);
            return;

        case PAIRING_INTERNAL_CHECK_HANDSET_LINKKEYS:
            appHandleInternalCheckHandsetLinkkeys(thePairing);
            return;

        case PAIRING_INTERNAL_TIMEOUT_IND:
            appHandleInternalTimeoutIndications(thePairing);
            return;

        case PAIRING_INTERNAL_PEER_PAIR_CANCEL:
            appHandleInternalPeerPairCancel(thePairing);
            return;

        case PAIRING_INTERNAL_HANDSET_PAIR_CANCEL:
            appHandleInternalHandsetPairCancel(thePairing);
            return;

        case PAIRING_INTERNAL_PEER_SDP_SEARCH:
            appHandleInternalPeerSdpSearch(thePairing);
            return;

        case PAIRING_INTERNAL_HANDSET_SDP_SEARCH:
            appHandleInternalHandsetSdpSearch(thePairing);
            return;

        case PAIRING_INTERNAL_DISABLE_SCAN:
            appHandleInternalDisableScan();
            return;

        case PEER_SIG_LINK_KEY_RX_IND:
            appPairingHandlePeerSigLinkKeyRxInd(thePairing, (PEER_SIG_LINK_KEY_RX_IND_T*)message);
            return;

        case PEER_SIG_LINK_KEY_TX_CFM:
            appPairingHandlePeerSigLinkKeyTxConfirm(thePairing, (PEER_SIG_LINK_KEY_TX_CFM_T*)message);
            return;

        case PEER_SIG_PAIR_HANDSET_CFM:
            appPairingHandlePeerSigPairHandsetConfirm(thePairing, (PEER_SIG_PAIR_HANDSET_CFM_T*)message);
            return;

        case CL_SM_ADD_AUTH_DEVICE_CFM:
            appPairingHandleClSmAddAuthDeviceConfirm((CL_SM_ADD_AUTH_DEVICE_CFM_T*)message);
            return;            
    }

    appHandleUnexpected(id);
}


void appPairingInit(void)
{
    pairingTaskData *thePairing = appGetPairing();

    DEBUG_LOG("appPairingInit");

    /* Set up task handler */
    thePairing->task.handler = appPairingHandleMessage;

    /* Initialise state */
    thePairing->state = PAIRING_STATE_NULL;
    thePairing->tws_sink_service_handle = 0;
    thePairing->sdp_lock = 1;
    thePairing->pairing_lock = 1;    
    thePairing->handset_pairing_attempts = 0;
    appPairingSetState(thePairing, PAIRING_STATE_INITIALISING);
    thePairing->ble_permission = pairingBleOnlyPairedHandsets;

    /* Get device name so that we can initialise EIR response */
    ConnectionReadLocalName(&thePairing->task);

    /* register with peer signalling for notification of handset
     * link key arrival, should the other earbud pair with a new handset */
    appPeerSigLinkKeyTaskRegister(&thePairing->task);

    /* register pairing as a client of the peer signalling task, it means
     * we will may get PEER_SIG_CONNECTION_IND messages if the signalling
     * channel becomes available again to retry sending the link key */
    appPeerSigClientRegister(&thePairing->task);
}

/*! \brief Pair with peer earbud, using RSSI pairing. */
void appPairingPeerPair(Task client_task, bool is_user_initiated)
{
    MAKE_PAIRING_MESSAGE(PAIR_REQ);
    pairingTaskData *thePairing = appGetPairing();
    message->client_task = client_task;
    message->is_user_initiated = is_user_initiated;
    BdaddrSetZero(&message->addr);
    MessageSendConditionally(&thePairing->task, PAIRING_INTERNAL_PEER_PAIR_REQ,
                             message, &thePairing->pairing_lock);
}

/*! \brief Pair with a handset, where inquiry is required. */
void appPairingHandsetPair(Task client_task, bool is_user_initiated)
{
    MAKE_PAIRING_MESSAGE(PAIR_REQ);
    pairingTaskData *thePairing = appGetPairing();
    message->client_task = client_task;
    message->is_user_initiated = is_user_initiated;
    message->num_attempts = 0;
    BdaddrSetZero(&message->addr);
    MessageSendConditionally(&thePairing->task, PAIRING_INTERNAL_HANDSET_PAIR_REQ,
                             message, &thePairing->pairing_lock);
}

/*! \brief Pair with a handset where the address is already known.
 */
void appPairingHandsetPairAddress(Task client_task, bdaddr* handset_addr, uint16 num_attempts)
{
    MAKE_PAIRING_MESSAGE(PAIR_REQ);
    pairingTaskData *thePairing = appGetPairing();
    message->client_task = client_task;
    message->addr = *handset_addr;    
    message->is_user_initiated = FALSE;
    message->num_attempts = num_attempts;
    MessageSendConditionally(&thePairing->task, PAIRING_INTERNAL_HANDSET_PAIR_REQ,
                             message, &thePairing->pairing_lock);
}

/*! @brief Cancel a handset pairing.
 */
void appPairingHandsetPairCancel(void)
{
    pairingTaskData *thePairing = appGetPairing();
    MessageSend(&thePairing->task, PAIRING_INTERNAL_HANDSET_PAIR_CANCEL, NULL);
}

/*! @brief Cancel a peer pairing.
 */
void appPairingPeerPairCancel(void)
{
    pairingTaskData *thePairing = appGetPairing();
    MessageSend(&thePairing->task, PAIRING_INTERNAL_PEER_PAIR_CANCEL, NULL);
}

/*! \brief Request the pairing module checks if any paired handset still need link keys TX to peer. */
void appPairingTransmitHandsetLinkKeysReq(void)
{
    pairingTaskData *thePairing = appGetPairing();

    /* don't do this again if already in the process */
    if (thePairing->outstanding_peer_sig_req)
        return;

    /* wait until any in progress pairing operation is completed, i.e. back in
     * the idle state */
    MessageCancelAll(&thePairing->task, PAIRING_INTERNAL_CHECK_HANDSET_LINKKEYS);
    MessageSendConditionally(&thePairing->task, PAIRING_INTERNAL_CHECK_HANDSET_LINKKEYS, NULL,
                             &thePairing->pairing_lock);
}

/*
 * TEST FUNCTIONS
 */
void appPairingSetHandsetLinkTxReqd(void)
{
    appDeviceAttributes attr;
    bdaddr handset_addr;

    DEBUG_LOG("Setting handset link key TX is required");
    appDeviceGetHandsetAttributes(&handset_addr, &attr, NULL);
    appDeviceSetHandsetLinkKeyTxReqd(&handset_addr, TRUE);
}

void appPairingClearHandsetLinkTxReqd(void)
{
    appDeviceAttributes attr;
    bdaddr handset_addr;

    DEBUG_LOG("Setting handset link key TX is required");
    appDeviceGetHandsetAttributes(&handset_addr, &attr, NULL);
    appDeviceSetHandsetLinkKeyTxReqd(&handset_addr, FALSE);
}


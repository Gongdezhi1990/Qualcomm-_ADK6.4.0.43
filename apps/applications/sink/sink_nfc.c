/****************************************************************************
Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
  

*/

/*****************************************************************************/
/*!

\ingroup sink_app

\brief  Implementation file to handle nfc connection library messages

@{
*/
#include "sink_nfc.h"
#include "sink_inquiry.h"
#include "sink_statemanager.h"
#include "sink_audio.h"
#include "assert.h"
#include "connection.h"
#include "sink_config.h"
#include "sink_slc.h"
#include "sink_init.h"
#include "sink_scan.h"

#if defined(ENABLE_ADK_NFC)

#include "nfc/nfc_prim.h"
#include "nfc_cl.h" /* !< contains NFC connection lib function prototypes */

/* PRIVATE MACROS DEFINITIONS ************************************************/
/**
 * Ref: Bluetooth core specification 4.2, section 4.2.2 connectable mode
 * page 304 and
 * Table 4.2 Page Scan Parameter for connection speed scenarios page 305
 * Fast R1 TGap(106) = 100ms   , 100/0.625=160 0xA0
 * Fast R1 TGap(101) = 10.625ms, 10.625/0.625=17 or 0x11=HCI_PAGESCAN_WINDOW_MIN */
#define NFC_HCI_PAGESCAN_INTERVAL (0xA0)
#define NFC_HCI_PAGESCAN_WINDOW   (0x11)
#define NFC_HCI_PAGESCAN_TIMEOUT  (3000)
#define NFC_HCI_CONN_PAGESCAN_TO  (60000)

#ifdef DEBUG_NFC
    #define NFC_DEBUG(x) DEBUG(x)
#else
    #define NFC_DEBUG(x)
#endif
#define NFC_I_STR "NFC: "
#define NFC_W_STR "NFC:**WARN "

/*#define NFC_DEBUG(x) DEBUG(x)*/

#define SINK_NFC_ASSERT(assertion) \
    ((assertion)? (void)0 : Panic())

/* PRIVATE VARIABLE DEFINITIONS **********************************************/
static bool NfcFastConnect = TRUE;

typedef enum sink_nfc_state_tag
{
    NFC_IDLE = 0,
    FAST_INQ_PAGE_SCANNING,
    NFC_CONNECTED,
  	CONN_ALLOW_AG2_PAIRING
} SINK_NFC_STATE;
static SINK_NFC_STATE currentSinkNfcState;

/*#define DEBUG_NFC_FSM 1*/
#define NFC_FSM_STR "NFC-FSM: "

#if defined(DEBUG_NFC_FSM) && defined(DEBUG_NFC)
#define NFC_FSM_DEBUG(x) DEBUG(x)
static const char *nfcStateName[] = { "IDLE", "INQ_PAGE_SCANNING", "CONNECTED", "CONN_ALLOW_AG2_PAIRING" };
static const char *sinkNfcGetStateName(void)
{
    return nfcStateName[currentSinkNfcState];
}

#else
#define NFC_FSM_DEBUG(x)

#endif

static void sinkNfcSetState(SINK_NFC_STATE nfcState)
{
    NFC_FSM_DEBUG((NFC_FSM_STR"os: %s ns: %s\n", nfcStateName[currentSinkNfcState], nfcStateName[nfcState]));
    currentSinkNfcState = nfcState;
}
static SINK_NFC_STATE sinkNfcGetState(void)
{
    return currentSinkNfcState;
}

/* PRIVATE FUNCTION DECLARATIONS *********************************************/

/*!
    @brief Start Fast Connect
    @warning Experimental code

    @param task main task
*/
static void sinkNfcStartFastConnect(Task task);

/*!
    @brief Stop Fast Connect
    @warning Experimental code

    @param task NULL when the timer has expired.  The running timer will be
    cancelled otherwise.
*/
static void sinkNfcStopFastConnect(Task task);

/*!
    @brief same as sinkNfcStopFastConnect with FSM state change

    @param task same as sinkNfcStopFastConnect
*/
static void sinkNfcStopFastConnectCommon(Task task);

/*!
    @brief disallow AG2 to pair and stop NFC_CL_DISALLOW_AG2_PAIRING_IND timer

    @param task use NULL when the timer has expired.  The running timer will be
    cancelled using the "task" otherwise.
*/
static void sinkNfcDisallowAG2Pairing(Task task);

/*!
    @brief allow AG2 to pair and start NFC_CL_DISALLOW_AG2_PAIRING_IND timer
    - Pairing is allowed while the timer is running.

    @param task used to start the NFC_CL_DISALLOW_AG2_PAIRING_IND timer
*/
static void sinkNfcAllowAG2Pairing(Task task);

/*****************************************************************************/
/*!
    @brief This function is called when the NFC TAG has been activated by the
    NFC reader and is in the process of being read.
    An imminent BT connection (optional pairing) or disconnection will occur.

    @param task the Sink App task
*/
static void sinkNfcHandleTagReadStarted(Task task);

/*****************************************************************************/
/*!
    @brief called when the NFC FIELD has been detected by the NFC HW

    @param task the Sink App task
*/
static void sinkNfcHandleCarrierOn(Task task);

/* PRIVATE FUNCTION DEFINITIONS **********************************************/
static void sinkNfcAllowAG2Pairing(Task task)
{
    NFC_FSM_DEBUG((NFC_FSM_STR"ev NFC_CL_ALLOW_AG2_PAIRING_REQ in St:%s(%d)\n", sinkNfcGetStateName(),stateManagerIsConnected()));
    switch (sinkNfcGetState())
    {
    case NFC_CONNECTED:
        sinkNfcSetState(CONN_ALLOW_AG2_PAIRING);
        MessageSendLater(task, NFC_CL_DISALLOW_AG2_PAIRING_IND, NULL, NFC_HCI_CONN_PAGESCAN_TO);
        sinkEnableMultipointConnectable();
        break;
    case NFC_IDLE: /* fallthrough */
    case FAST_INQ_PAGE_SCANNING: /* fallthrough */
    case CONN_ALLOW_AG2_PAIRING: /* fallthrough */
    default:
        break;
    }
}

static void sinkNfcDisallowAG2PairingCommon(Task task)
{
    if (NULL != task)
    {
        MessageCancelAll(task, NFC_CL_DISALLOW_AG2_PAIRING_IND);
    }
    /* ### Note: the code could stop scanning here. e.g. call
       sinkDisableConnectable. */
}
static void sinkNfcDisallowAG2Pairing(Task task)
{
    if (NULL!=task)
    {
        NFC_FSM_DEBUG((NFC_FSM_STR"ev NFC_CL_DISALLOW_AG2_PAIRING_IND(Stopped) in St:%s(%d)\n", sinkNfcGetStateName(),stateManagerIsConnected()));
    }
    else
    {
        NFC_FSM_DEBUG((NFC_FSM_STR"ev NFC_CL_DISALLOW_AG2_PAIRING_IND(Expired) in St:%s(%d)\n", sinkNfcGetStateName(),stateManagerIsConnected()));
    }

    switch (sinkNfcGetState())
    {
    case CONN_ALLOW_AG2_PAIRING:
        sinkNfcDisallowAG2PairingCommon(task);
        sinkNfcSetState(stateManagerIsConnected()==TRUE ? NFC_CONNECTED : NFC_IDLE);
        break;
    case NFC_IDLE: /* fallthrough */
    case FAST_INQ_PAGE_SCANNING: /* fallthrough */
    case NFC_CONNECTED: /* fallthrough */
    default:
        break;
    }
}

static void sinkNfcStartFastConnect(Task task)
{
    if(TRUE  == NfcFastConnect)
    {
        NFC_FSM_DEBUG((NFC_FSM_STR"ev NFC_CL_START_FAST_PAGESCAN_REQ in St:%s(%d)\n", sinkNfcGetStateName(),stateManagerIsConnected()));
        switch (sinkNfcGetState())
        {
        case NFC_IDLE:
            sinkNfcSetState(FAST_INQ_PAGE_SCANNING);
            /* Call ConnectionWritePagescanActivity after stateManagerEnterConnDiscoverableState
             * NOTE: a timer (e.g. 10s) needs to be STARTED to restore the DEFAULT page scan. */
            ConnectionWritePagescanActivity(NFC_HCI_PAGESCAN_INTERVAL,
                                            NFC_HCI_PAGESCAN_WINDOW);
            ConnectionWritePageScanType(hci_scan_type_interlaced);
            MessageSendLater(task, NFC_CL_STOP_FAST_PAGESCAN_IND_ID, NULL, NFC_HCI_PAGESCAN_TIMEOUT);
            break;
        case FAST_INQ_PAGE_SCANNING:  /* fallthrough */
        case NFC_CONNECTED:  /* fallthrough */
        case CONN_ALLOW_AG2_PAIRING:  /* fallthrough */
        default:
            break;
        }
    }
}

static void sinkNfcStopFastConnectCommon(Task task)
{
    if (TRUE == NfcFastConnect)
    {
        radio_config_type radioConfig;
        sinkInquiryGetRadioConfig(&radioConfig);

        ConnectionWritePagescanActivity(radioConfig.page_scan_interval, radioConfig.page_scan_window);
        ConnectionWritePageScanType(hci_scan_type_standard);
        /* Cancel FAST Page Scan timeout message */
        if (NULL != task)
        {
            MessageCancelAll(task, NFC_CL_STOP_FAST_PAGESCAN_IND_ID);
        }
    }
}
static void sinkNfcStopFastConnect(Task task)
{
    if (NULL!=task)
    {
        NFC_FSM_DEBUG((NFC_FSM_STR"ev NFC_CL_STOP_FAST_PAGESCAN_IND(Stopped) in St:%s(%d)\n", sinkNfcGetStateName(),stateManagerIsConnected()));
    }
    else
    {
        NFC_FSM_DEBUG((NFC_FSM_STR"ev NFC_CL_STOP_FAST_PAGESCAN_IND(Expired) in St:%s(%d)\n", sinkNfcGetStateName(),stateManagerIsConnected()));
    }
    if (TRUE == NfcFastConnect)
    {
        switch (sinkNfcGetState())
        {
        case FAST_INQ_PAGE_SCANNING:
            sinkNfcStopFastConnectCommon(task);
            sinkNfcSetState(stateManagerIsConnected()==TRUE ? NFC_CONNECTED : NFC_IDLE);
            break;
        case NFC_IDLE:/* fallthrough */
        case NFC_CONNECTED:/* fallthrough */
        case CONN_ALLOW_AG2_PAIRING:/* fallthrough */
        default:
            break;
        }
    }
}

/**
 * The code logic should be kept the same as
 * handleUsrNFCTagDetected  (see below) without the fast
 * paging. It is anticipated that the commonality will be
 * merged in the future. */
static void sinkNfcHandleCarrierOnAndTagReadStarted(Task task,
                                                    bool readStarted)
{
   /* if not connected to an AG, go straight into pairing mode */
    switch(stateManagerGetState())/* < deviceConnected)*/
    {
    case deviceLimbo:
        MessageSend(task, EventUsrPowerOn, NULL);
        /*### Expected NFC signals 
          Case1:
            CarrierOn      --->
          Case 2 :
            CarrierOn      --->
            TagReadStarted --->
         
          The state must be changed from "deviceLimbo" as in case 2 to two EventUsrPowerOn
          messages will be sent.  resulting in two "Power On" audio prompt.
         
          The state for Case 2 must be changed to start page scanning(e.g. Connectable).
          The phone is expected after reading the NFC tag successfully to "Page" NOT
          "Inquire" (see BT spec). (e.g. call stateManagerEnterConnectableState(TRUE))
         
          Unfortunatly some old phones break the spec so for compatibility the sink moves
          ConnectableDiscoverable state. To be revised. */
        stateManagerEnterConnDiscoverableState(TRUE);
        sinkNfcStartFastConnect(task);

        NFC_DEBUG((NFC_I_STR"Wake On NFC readStarted:%d\n", readStarted));
        /*SINK_NFC_ASSERT(readStarted==FALSE);*/
        break;
    case deviceConnectable:
        stateManagerEnterConnDiscoverableState(TRUE);
        sinkNfcStartFastConnect(task);
        break;
    case deviceConnDiscoverable: /* No need to enter discoverable state */
        sinkNfcStartFastConnect(task);
        break;
    default: /* >= deviceConnected)*/
        /* NO fast FastConnect here as this would be too intensive so trigger
         * the default page scan (### Note: interleave scan would also speed up
         * the BT connection time) and authorise pairing request.
         *
         * This code should be run only when the tag has been READ
         * a) this will filter the CARRIER_DETECTION when the AG is moving away
         *    from tag after it has been read.
         *
         * b) the user may also decide to leave the phone on the tag until it is
         *    steaming then move the phone away.
         *
         * c) It is also slightly safer as paging request will be accepted immediatly
         *    The disavantage is it will make the connections with AG2 slower.
         *    It can also fail if the phone give up sending the paging request. */
        if (TRUE==readStarted)
        {
            sinkNfcAllowAG2Pairing(task);
        }
        break;
    }
}

static void sinkNfcHandleCarrierOn(Task task)
{
    NFC_DEBUG((NFC_I_STR"NFC_CL_CARRIER_ON_IND\n"));
    sinkNfcHandleCarrierOnAndTagReadStarted(task, FALSE);
}

static void sinkNfcHandleTagReadStarted(Task task)
{
    NFC_DEBUG((NFC_I_STR"NFC_CL_TAG_READ_STARTED_IND\n"));
    sinkNfcHandleCarrierOnAndTagReadStarted(task, TRUE);
}

/* PUBLIC FUNCTION DEFINITIONS ***********************************************/
void handleNfcClMessage (Task task, MessageId id, Message message)
{
    const NFC_CL_PRIM *p_nfc_cl = (const NFC_CL_PRIM *) message;

    UNUSED(task);
    switch (id)
    {
    case NFC_CL_CONFIG_CNF_ID:
        SINK_NFC_ASSERT(IS_VALID_PTR(p_nfc_cl));
        NFC_DEBUG((NFC_I_STR"NFC_CL_CONFIG_CNF: nfc_cl_status=%d mode=%d ch:%d\n",
                   p_nfc_cl->m.config_cnf.nfc_cl_status,
                   p_nfc_cl->m.config_cnf.nfc_config.mode,
                   p_nfc_cl->m.config_cnf.nfc_config.ch_service));
        sinkNfcEnableFastConnect();
        if(NFC_CL_READY == p_nfc_cl->m.config_cnf.nfc_cl_status)
        {
            ConnectionReadLocalAddr(task);
        }
        break;
    case NFC_CL_WRITE_CH_CARRIERS_CNF_ID:
        SINK_NFC_ASSERT(IS_VALID_PTR(p_nfc_cl));
        NFC_DEBUG((NFC_I_STR"NFC_CL_WRITE_CH_CARRIERS_CNF status=%d\n", p_nfc_cl->status));
        break;
    case NFC_CL_TAG_READ_STARTED_IND_ID: /* Static Handover ONLY */
        /* NFC_DEBUG Print in function below */
        sinkNfcHandleTagReadStarted(task);
        break;
    case NFC_CL_CARRIER_ON_IND_ID:
        /* NFC_DEBUG Print in function below */
        sinkNfcHandleCarrierOn(task);  /* Gamble on the phone reading the entire tag so start scanning */
        break;
    case NFC_CL_CARRIER_LOSS_IND_ID:
        NFC_DEBUG((NFC_I_STR"NFC_CL_CARRIER_LOSS_IND\n"));
        break;
    case NFC_CL_SELECTED_IND_ID:
        NFC_DEBUG((NFC_I_STR"NFC_CL_SELECTED_IND\n"));
        break;
    case NFC_CL_STOP_FAST_PAGESCAN_IND_ID:
        /* NFC_DEBUG Print in function below */
        sinkNfcStopFastConnect(NULL);
        break;
    case NFC_CL_DISALLOW_AG2_PAIRING_IND:
        /* NFC_DEBUG Print in function below */
        sinkNfcDisallowAG2Pairing(NULL);
        break;
    case NFC_CL_HANDOVER_CARRIER_IND_ID: /* Negotiated handover ONLY */
        NFC_DEBUG((NFC_I_STR"NFC_CL_HANDOVER_CARRIER_IND\n"));
        break;
    case NFC_CL_HANDOVER_COMPLETE_IND_ID: /* Negotiated handover ONLY */
        NFC_DEBUG((NFC_I_STR"NFC_CL_HANDOVER_COMPLETE_IND\n"));
        break;
    default:
        NFC_DEBUG((NFC_I_STR"Rx Unhandled NFC_CL (msg id=%d)\n",id));
        break;
    }
}

/* SINK NFC MESSAGE TO CONFIGURE NFC *****************************************/
void sinkNfcEnableFastConnect(void)
{
    NfcFastConnect = TRUE;
}

void sinkNfcDisableFastConnect(void)
{
    NfcFastConnect = FALSE;
}

void sinkNfcTagConfigReq(Task client_nfc_cl_task)
{
    NFC_CL_CONFIG_REQ sinkNfcClConfigReq;
    sinkNfcClConfigReq.nfcClientRecvTask = client_nfc_cl_task;
    sinkNfcClConfigReq.send_carrier_on_ind   = TRUE;
    sinkNfcClConfigReq.send_carrier_loss_ind = FALSE;
    sinkNfcClConfigReq.send_selected_ind     = FALSE;
    sinkNfcClConfigReq.nfc_config.mode = NFC_VM_TT2;
    sinkNfcClConfigReq.nfc_config.ch_service = NFC_VM_LLCP_NONE;
    sinkNfcClConfigReq.nfc_config.snep_service = NFC_VM_LLCP_NONE;
    (void) NfcClConfigReq(&sinkNfcClConfigReq);
}

void sinkNfcHandleLocalBDAddr(Task cl_dm_task, const CL_DM_LOCAL_BD_ADDR_CFM_T *bdaddr_cfm)
{
    bdaddr local_bdaddr;

    SINK_NFC_ASSERT(IS_VALID_PTR(bdaddr_cfm));
    local_bdaddr = bdaddr_cfm->bd_addr;
    NfcClEncBtCarBDAddr(&local_bdaddr);
    ConnectionReadLocalName(cl_dm_task);
}

void sinkNfcHandleLocalName(Task cl_dm_task,
                            const CL_DM_LOCAL_NAME_COMPLETE_T *name_complete)
{
    const uint8 *local_name;
    uint8 size_local_name;

    UNUSED(cl_dm_task);
    SINK_NFC_ASSERT(IS_VALID_PTR(name_complete));
    local_name = name_complete->local_name;
    size_local_name = (uint8) name_complete->size_local_name;
    NfcClEncBtCarLocalName(local_name, size_local_name);
}

void sinkNfcSetClassOfDevice(Task cl_dm_task)
{
     /* The device class is set in the PSkeys
        For a quick test it can be set to  ={0x14, 0x04, 0x20 };
        SrvCl=Audio, Major=Audio/Video, Minor=Loudspeaker */
    uint8 class_of_device_str[CLASS_OF_DEVICE_SIZE];
    uint32 class_of_device = sinkInitGetDeviceClassConfig();

    UNUSED(cl_dm_task);
    if (0==class_of_device)
    {
        NFC_DEBUG((NFC_W_STR"DeviceClass no set, check pskey\n"));
    }
    else
    {
        class_of_device_str[0] = (uint8)(class_of_device & 0xFF);
        class_of_device_str[1] = (uint8)(class_of_device>>8 & 0xFF);
        class_of_device_str[2] = (uint8)(class_of_device>>16 & 0xFF);
        NfcClEncBtCarClassOfDevice(class_of_device_str);
    }
}

void sinkNfcWriteChCarriersReq(void)
{
    NFC_DEBUG((NFC_I_STR"WriteChCarriersReq\n"));
    (void)NfcClWriteChCarriersReq(BT_CAR_ID, NONE_CAR_ID);
}

/* EXTERNAL SINK NFC FSM MESSAGE ACTIONS *************************************/
void sinkNfcBTDeviceConnectedInd(Task task)
{
    NFC_FSM_DEBUG((NFC_FSM_STR"ev NFC_BT_DEVICE_CONNECTED_IND in St:%s(%d)\n", sinkNfcGetStateName(),stateManagerIsConnected()));
    switch (sinkNfcGetState())
    {
    case NFC_IDLE:
        sinkNfcSetState(NFC_CONNECTED);
        break;
    case FAST_INQ_PAGE_SCANNING:
        sinkNfcStopFastConnectCommon(task);
        sinkNfcSetState(NFC_CONNECTED);
        break;
    case CONN_ALLOW_AG2_PAIRING:
        /* NFC_BT_DEVICE_CONNECTED_IND can be received instead of AUTHENTICATION_SUCCESS_IND
          when AG2 was already paired. */
        /* AVRCP_SPEC_v14 mentions that
           - stop is C1 (Mandatory)
           - pause is O (Optional)
           so calling sinkAvrcpStop should in theory always work. sinkAvrcpPause
           should also work unless the user is using a very old phone. */
        sinkAvrcpStop();
        sinkNfcDisallowAG2PairingCommon(task);
        sinkNfcSetState(NFC_CONNECTED);
        break;
    case NFC_CONNECTED:/* fallthrough */
    default:
        break;
    }
}

void sinkNfcBTDeviceDisconnectedInd(Task task)
{
    NFC_FSM_DEBUG((NFC_FSM_STR"ev NFC_BT_DEVICE_DISCONNECTED_IND in St:%s(%d)\n", sinkNfcGetStateName(),stateManagerIsConnected()));
    if(FALSE == stateManagerIsConnected())
    {
        sinkNfcStopFastConnectCommon(task);
        sinkNfcDisallowAG2Pairing(task); /* Cleanup: Only allowed in "CONN_ALLOW_AG2_PAIRING" state */
        sinkNfcSetState(NFC_IDLE);
    }
    /* else: Stay in CONNECTED states. For instance: a second is trying to
       connect and the first device disconnect at the same time */
}

bool sinkNfcIsSecondAGPairingAllowed(void)
{
    bool result = (CONN_ALLOW_AG2_PAIRING==sinkNfcGetState()) && stateManagerIsConnected();
    NFC_DEBUG((NFC_I_STR"IsSecondAGPairingAllowed=%d\n",result));
    return result;
}

void sinkNfcAuthenticationSuccessInd(Task task)
{
    NFC_FSM_DEBUG((NFC_FSM_STR"ev NFC_AUTHENTICATION_SUCCESS_IND in St:%s(%d)\n", sinkNfcGetStateName(),stateManagerIsConnected()));
    switch (sinkNfcGetState())
    {
    case CONN_ALLOW_AG2_PAIRING:
        sinkAvrcpStop();
        sinkNfcDisallowAG2PairingCommon(task);
        sinkNfcSetState(NFC_CONNECTED);
        break;
    case NFC_IDLE:/* fallthrough */
    case FAST_INQ_PAGE_SCANNING:/* fallthrough */
    case NFC_CONNECTED:/* fallthrough */
    default:
        break;
    }
}

bool sinkNfcIsMessageIdNfc(MessageId id)
{
    return IS_VALID_NFC_CL_MSG_ID(id);
}

#endif /* ENABLE_ADK_NFC */

/** ADK 4.x code
 *
 *  NFC tag detected, determine action based on current
 *  connection state
 *      case EventUsrNFCTagDetected:
 *
 *  The code below should be replaced something similar to
 *  sinkNfcHandleTagReadStarted(task) to enable fast connection.
 *  This would be triggered on the detection of NFC Field from
 *  the phone. The main difference is that the Sink app has NO
 *  guarantee that the reader has actually read the tag.
 *
 *  For instance: the end user may just have waved his/her phone
 *  next to the tag but yet activated the tag or started reading
 *  the content of it.
 *
 *  The TagReadStarted function has the advantage of informing
 *  the sink app that the PHONE has almost completed reading the
 *  content of the Sink App TAG */
void handleUsrNFCTagDetected(Task task,  Message message)
{
    (void)task;
    (void)message;
    /* if not connected to an AG, go straight into pairing mode */
    switch(stateManagerGetState())/* < deviceConnected)*/
    {
    case deviceLimbo: /* fallthrough */
        MessageSend(task, EventUsrPowerOn, NULL);
        break;
    case deviceConnectable: /* fallthrough */
    case deviceConnDiscoverable:
        sinkInquirySetInquirySession(inquiry_session_normal);
        stateManagerEnterConnDiscoverableState( FALSE );
        break;
    default: /* >= deviceConnected)*/
        break;
    }
}

/** @}*/

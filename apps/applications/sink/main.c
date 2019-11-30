/*
Copyright (c) 2005 - 2019 Qualcomm Technologies International, Ltd.
*/
/**
\file

\ingroup sink_app

\brief  This is main file for the application software for a sink device

*/
/**
 \addtogroup sink_app
 \{
*/
/****************************************************************************
    Header files
*/

#include "sink_sport_health_sm.h"
#include "sink_sport_health.h"
#include "sink_tap_ui.h"

#include <message.h>
#include <hfp.h>
#include <local_device.h>
#include <transport_manager.h>

#include "sink_private_data.h"
#include "sink_main_task.h"

#ifndef HOSTED_TEST_ENVIRONMENT
#include "sink_pmalloc_pools.h"
#endif

#include "sink_init.h"
#include "sink_auth.h"
#include "sink_scan.h"
#include "sink_slc.h"
#include "sink_ahi.h"
#include "sink_accessory.h"
#include "sink_hfp_data.h"
#include "sink_inquiry.h"
#include "sink_devicemanager.h"
#include "sink_link_policy.h"
#include "sink_indicators.h"
#include "sink_hf_indicators.h"
#include "sink_dut.h"
#include "sink_pio.h"
#include "sink_multipoint.h"
#include "sink_led_manager.h"
#include "sink_buttonmanager.h"
#include "sink_configmanager.h"
#include "sink_events.h"
#include "sink_statemanager.h"
#include "sink_states.h"
#include "sink_powermanager.h"
#include "sink_callmanager.h"
#include "sink_csr_features.h"
#include "sink_usb.h"
#include "sink_usb_debug.h"
#include "sink_display.h"
#include "sink_speech_recognition.h"
#include "sink_a2dp.h"
#include "sink_config.h"
#include "sink_audio_routing.h"
#include "sink_partymode.h"
#include "sink_leds.h"
#include "sink_led_err.h"
#include "sink_upgrade.h"
#include "sink_fm.h"
#include "sink_anc.h"
#include "sink_ba.h"
#include "sink_auto_power_off.h"
#include "sink_bootmode.h"
#include "sink_bredr.h"
#include "sink_audio_ttp.h"
#include "sink_va_if.h"
#include "sink_dormant.h"
#include "sink_dsp_capabilities.h"
#include "sink_downloadable_caps.h"
#include "sink_music_processing.h"
#ifdef ENABLE_BISTO
#include "bisto/sink2bisto.h"
#include "sink_process_event.h"
#endif /* ENABLE_BISTO */

#include <os.h>
#include <operator.h>
#include <resource_manager.h>
/* BLE related headers */

#include "sink_ble.h"
#include "sink_ble_gap.h"
#include "sink_ba_ble_gap.h"
#include "sink_ble_advertising.h"
#include "sink_ble_scanning.h"
#include "sink_gatt_server_ias.h"
#include "sink_gatt_client_ias.h"
#include "sink_gatt_client.h"

#include "sink_gatt_hid_qualification.h"
#include "sink_gatt_server_lls.h"
#include "sink_gatt_server_hrs.h"
#include "sink_ble_sc.h"

#ifdef ENABLE_IR_REMOTE
#include "sink_ir_remote_control.h"
#endif

#include "sink_gaia.h"
#ifdef ENABLE_GAIA
#include "gaia.h"
#endif
#ifdef ENABLE_PBAP
#include "sink_pbap.h"
#endif
#ifdef ENABLE_MAPC
#include "sink_mapc.h"
#endif
#include "sink_avrcp.h"
#include "sink_swat.h"

#ifdef ENABLE_PEER
#include "sink_peer.h"
#endif

#ifdef ENABLE_FAST_PAIR
#include "sink_fast_pair.h"
#endif

#include "sink_linkloss.h"
#include "sink_sc.h"

#include "sink_avrcp_qualification.h"
#include "sink_peer_qualification.h"

#include "sink_hid.h"

#include "sink_volume.h"
#include "sink_tones.h"
#include "sink_audio_prompts.h"
#include "sink_audio_indication.h"
#include "sink_event_queue.h"

#include "sink_audio.h"
#include "sink_at_commands.h"

#include "sink_watchdog.h"
#include "vm.h"

#include "sink_nfc.h"
#include "config_definition.h"


#include <config_store.h>
#include <library.h>
#include <bdaddr.h>
#include <connection.h>
#include <panic.h>
#include <ps.h>
#include <stdlib.h>
#include <stdio.h>
#include <stream.h>
#include <boot.h>
#include <string.h>
#include <audio.h>
#include <sink.h>
#include <kalimba_standard_messages.h>
#include <audio_plugin_if.h>
#include <print.h>
#include <loader.h>
#include <pio_common.h>
#include <ahi.h>
#include <config_store.h>
#include <spp_common.h>
#include "ahi_test.h"
#include "sink_spp_qualification.h"
#include "sink_dfu_ps.h"

#include <display.h>
#include <display_plugin_if.h>

#ifdef DEBUG_MAIN
    #define MAIN_DEBUG(x) DEBUG(x)
#else
    #define MAIN_DEBUG(x)
#endif

#ifdef DEBUG_MAIN_L1
    #define MAIN_DEBUG_L1(x) DEBUG(x)
#else
    #define MAIN_DEBUG_L1(x)
#endif

#if defined(DEBUG_MAIN) || defined(DEBUG_MAIN_L1)
    #define TRUE_OR_FALSE(x)  ((x) ? 'T':'F')
#endif 

#define IS_SOURCE_CONNECTED ((!usbIsAttached()) && (!analogAudioConnected()) && \
                             (!spdifAudioConnected()) && (!sinkFmIsFmRxOn()) && (!i2sAudioConnected()))

/* Pairing timeout action */
enum
{
    PAIRTIMEOUT_CONNECTABLE         = 0,
    PAIRTIMEOUT_POWER_OFF           = 1,
    PAIRTIMEOUT_POWER_OFF_IF_NO_PDL = 2
};

void app_handler(Task task, MessageId id, Message message);

static void handleHFPStatusCFM ( hfp_lib_status pStatus ) ;
static void sinkConnectionInit(void);


#ifdef HOSTED_TEST_ENVIRONMENT
extern void _sink_init(void);
#else
extern void _init(void);
#endif
static void IndicateEvent(MessageId id);
static bool voiceDialProcessEvent(hfp_link_priority hfp_link);

/*************************************************************************
NAME
    sinkSend

DESCRIPTION
    Send event to main task

RETURNS
    void
*/
void sinkSend(sinkEvents_t event)
{
    MessageSend(&theSink.task, event, NULL);
}

/*************************************************************************
NAME
    sinkCancelAndSendLater

DESCRIPTION
    Cancel queued messages
    Send event to main task after delay

RETURNS
    void
*/
void sinkCancelAndSendLater(sinkEvents_t event, uint32 delay)
{
    MessageCancelAll(&theSink.task, event);
    MessageSendLater(&theSink.task, event, NULL, delay);
}

/*************************************************************************
NAME
    sinkCancelAndIfDelaySendLater

DESCRIPTION
    Cancel queued messages
    If delay is non-zero, send event to main task after delay

RETURNS
    void
*/
void sinkCancelAndIfDelaySendLater(sinkEvents_t event, uint32 delay)
{
    MessageCancelAll(&theSink.task, event);
    
    if (delay)
    {
        MessageSendLater(&theSink.task, event, NULL, delay);
    }
}

/*************************************************************************
NAME
    sinkCancelMessage

DESCRIPTION
    Cancel queued messages

RETURNS
    void
*/
static void sinkCancelMessage(sinkEvents_t event)
{
    MessageCancelAll(&theSink.task, event);
}

/*************************************************************************
NAME
    handleCLMessage

DESCRIPTION
    Function to handle the CL Lib messages - these are independent of state

RETURNS

*/
static void handleCLMessage ( Task task, MessageId id, Message message )
{
    MAIN_DEBUG(("CL[%x]\n",id)) ;

    UNUSED(task); /* Can be used depending on compile-time definitions */

    switch(id)
    {
        case CL_INIT_CFM:
            MAIN_DEBUG_L1(("CL_INIT_CFM [%d]\n" , ((CL_INIT_CFM_T*)message)->status ));

            if(((const CL_INIT_CFM_T*)message)->status == success)
            {
                LocalDeviceInit(task);
#ifdef ENABLE_PEER
                if (VmGetResetSource() != UNEXPECTED_RESET)
                {
                    peerPurgeTemporaryPairing();
                }
#endif

                /* Initialize the transport manager. */
                TransportMgrInit();

#ifdef ENABLE_GAIA
                /* Initialise Gaia with a concurrent connection limit of 1 */
                GaiaInit(task, 1);
#endif
            }
            else
            {
                Panic();
            }
            sinkNfcTagConfigReq(&theSink.task);
        break;
        case CL_DM_WRITE_INQUIRY_MODE_CFM:
            /* Read the local name to put in our EIR data */
            ConnectionReadInquiryTx(&theSink.task);
        break;
        case CL_DM_READ_INQUIRY_TX_CFM:
            sinkInquirySetInquiryTx(((const CL_DM_READ_INQUIRY_TX_CFM_T*)message)->tx_power);
            ConnectionReadLocalName(&theSink.task);
        break;
        case CL_DM_LOCAL_NAME_COMPLETE:
            MAIN_DEBUG_L1(("CL_DM_LOCAL_NAME_COMPLETE\n"));
            sinkNfcHandleLocalName(task, (const CL_DM_LOCAL_NAME_COMPLETE_T *)message);
            sinkNfcSetClassOfDevice(task);
            sinkNfcWriteChCarriersReq();
            sinkAccessoryUpdateDeviceName((const CL_DM_LOCAL_NAME_COMPLETE_T*)message);
            /* Write EIR data and initialise the codec task */
            sinkWriteEirData((const CL_DM_LOCAL_NAME_COMPLETE_T*)message);
            sinkAccessoryHwInit();

            sinkSportHealthSMGetLocalAddr();
            break;

        case CL_SM_SEC_MODE_CONFIG_CFM:
            MAIN_DEBUG_L1(("CL_SM_SEC_MODE_CONFIG_CFM\n"));
            /* Remember if debug keys are on or off */
           sinkDataSetDebugKeyEnabledFlag(((const CL_SM_SEC_MODE_CONFIG_CFM_T*)message)->debug_keys);
        break;
        case CL_SM_PIN_CODE_IND:
            MAIN_DEBUG_L1(("CL_SM_PIN_IND\n"));
            sinkHandlePinCodeInd((const CL_SM_PIN_CODE_IND_T*) message);
        break;
        case CL_SM_USER_CONFIRMATION_REQ_IND:
            MAIN_DEBUG_L1(("CL_SM_USER_CONFIRMATION_REQ_IND\n"));
            sinkHandleUserConfirmationInd((const CL_SM_USER_CONFIRMATION_REQ_IND_T*) message);
        break;
        case CL_SM_USER_PASSKEY_REQ_IND:
            MAIN_DEBUG_L1(("CL_SM_USER_PASSKEY_REQ_IND\n"));
            sinkHandleUserPasskeyInd((const CL_SM_USER_PASSKEY_REQ_IND_T*) message);
        break;
        case CL_SM_USER_PASSKEY_NOTIFICATION_IND:
            MAIN_DEBUG_L1(("CL_SM_USER_PASSKEY_NOTIFICATION_IND\n"));
            sinkHandleUserPasskeyNotificationInd((const CL_SM_USER_PASSKEY_NOTIFICATION_IND_T*) message);
        break;
        case CL_SM_KEYPRESS_NOTIFICATION_IND:
        break;
        case CL_SM_REMOTE_IO_CAPABILITY_IND:
            MAIN_DEBUG_L1(("CL_SM_IO_CAPABILITY_IND\n"));
            sinkHandleRemoteIoCapabilityInd((const CL_SM_REMOTE_IO_CAPABILITY_IND_T*)message);
        break;
        case CL_SM_IO_CAPABILITY_REQ_IND:
            MAIN_DEBUG_L1(("CL_SM_IO_CAPABILITY_REQ_IND\n"));
            sinkHandleIoCapabilityInd((const CL_SM_IO_CAPABILITY_REQ_IND_T*) message);
        break;
        case CL_SM_AUTHORISE_IND:
            MAIN_DEBUG_L1(("CL_SM_AUTHORISE_IND\n"));
            sinkHandleAuthoriseInd((const CL_SM_AUTHORISE_IND_T*) message);
        break;
        case CL_SM_AUTHENTICATE_CFM:
            MAIN_DEBUG_L1(("CL_SM_AUTHENTICATE_CFM\n"));
            sinkHandleAuthenticateCfm((const CL_SM_AUTHENTICATE_CFM_T*) message);
        break;
#if defined ENABLE_PEER && defined ENABLE_SUBWOOFER
#error ENABLE_PEER and ENABLE_SUBWOOFER are mutually exclusive due to their use of CL_SM_GET_AUTH_DEVICE_CFM
#endif
#ifdef ENABLE_SUBWOOFER
        case CL_SM_GET_AUTH_DEVICE_CFM: /* This message should only be sent for subwoofer devices */
            MAIN_DEBUG_L1(("CL_SM_GET_AUTH_DEVICE_CFM\n"));
            if (sinkDataGetDisplayLinkKeys())
            {
                deviceManagerExtractDeviceLinkKey((const CL_SM_GET_AUTH_DEVICE_CFM_T*) message);
            }
            else
            {
                handleSubwooferGetAuthDevice((const CL_SM_GET_AUTH_DEVICE_CFM_T*) message);
            }
        break;
#endif
#ifdef ENABLE_PEER
        case CL_SM_GET_AUTH_DEVICE_CFM:
            MAIN_DEBUG_L1(("CL_SM_GET_AUTH_DEVICE_CFM\n"));
            if (sinkDataGetDisplayLinkKeys())
            {
                deviceManagerExtractDeviceLinkKey((const CL_SM_GET_AUTH_DEVICE_CFM_T*) message);
            }
            else
            {
                handleGetAuthDeviceCfm((const CL_SM_GET_AUTH_DEVICE_CFM_T *)message);
            }
        break;
        case CL_SM_ADD_AUTH_DEVICE_CFM:
            MAIN_DEBUG_L1(("CL_SM_ADD_AUTH_DEVICE_CFM\n"));
            handleAddAuthDeviceCfm((const CL_SM_ADD_AUTH_DEVICE_CFM_T *)message);
        break;
#elif !defined ENABLE_SUBWOOFER
        /* General case when ENABLE_PEER and ENABLE_SUBWOOFER are not defined */
        case CL_SM_GET_AUTH_DEVICE_CFM:
            MAIN_DEBUG_L1(("CL_SM_GET_AUTH_DEVICE_CFM\n"));
            if (sinkDataGetDisplayLinkKeys())
            {
                deviceManagerExtractDeviceLinkKey((const CL_SM_GET_AUTH_DEVICE_CFM_T*) message);
            }
        break;
#endif

        case CL_DM_REMOTE_FEATURES_CFM:
            MAIN_DEBUG_L1(("HS : Supported Features\n")) ;
        break ;
        case CL_DM_REMOTE_EXTENDED_FEATURES_CFM:
            MAIN_DEBUG_L1(("HS : Supported Extended Features\n")) ;
        break;
        case CL_DM_INQUIRE_RESULT:
            MAIN_DEBUG_L1(("HS : Inquiry Result\n"));
            inquiryHandleResult((CL_DM_INQUIRE_RESULT_T*)message);
        break;
        case CL_SM_GET_ATTRIBUTE_CFM:
            MAIN_DEBUG_L1(("HS : CL_SM_GET_ATTRIBUTE_CFM Vol:%d \n",
                            ((const CL_SM_GET_ATTRIBUTE_CFM_T *)(message))->psdata[0]));
        break;
        case CL_SM_GET_INDEXED_ATTRIBUTE_CFM:
            MAIN_DEBUG_L1(("HS: CL_SM_GET_INDEXED_ATTRIBUTE_CFM[%d]\n" ,
                            ((const CL_SM_GET_INDEXED_ATTRIBUTE_CFM_T*)message)->status)) ;
        break ;

        case CL_DM_LOCAL_BD_ADDR_CFM:
            MAIN_DEBUG_L1(("CL_DM_LOCAL_BD_ADDR_CFM\n"));
            sinkNfcHandleLocalBDAddr(task, (const CL_DM_LOCAL_BD_ADDR_CFM_T *)message);

            sinkSportHealthSMHandleBdAddrCfm(message);
        break;

        case CL_DM_ROLE_IND:
            linkPolicyHandleRoleInd((const CL_DM_ROLE_IND_T *)message);
        break;
        case CL_DM_ROLE_CFM:
            linkPolicyHandleRoleCfm((const CL_DM_ROLE_CFM_T *)message);
        break;
        case CL_SM_SET_TRUST_LEVEL_CFM:
            MAIN_DEBUG_L1(("HS : CL_SM_SET_TRUST_LEVEL_CFM status %x\n",
                            ((const CL_SM_SET_TRUST_LEVEL_CFM_T*)message)->status));
        break;
        case CL_DM_ACL_OPENED_IND:
            MAIN_DEBUG_L1(("HS : ACL Opened\n"));

            if(((CL_DM_ACL_OPENED_IND_T*)message)->status == hci_success)
            {
                sinkStartAutoPowerOffTimer();
            }
        break;
        case CL_DM_ACL_CLOSED_IND:
            MAIN_DEBUG_L1(("HS : ACL Closed\n"));
#ifdef ENABLE_AVRCP
            if(avrcpAvrcpIsEnabled())
            {
                sinkAvrcpAclClosed(((const CL_DM_ACL_CLOSED_IND_T *)message)->taddr.addr);
            }
#endif
        break;

/* BLE Messages */
        case CL_DM_BLE_ADVERTISING_REPORT_IND:
        {
            MAIN_DEBUG_L1(("CL_DM_BLE_ADVERTISING_REPORT_IND\n"));
            bleHandleScanResponse((const CL_DM_BLE_ADVERTISING_REPORT_IND_T *)message);
        }
        break;
        case CL_DM_BLE_SET_ADVERTISING_DATA_CFM:
        {
            bleHandleSetAdvertisingData( (const CL_DM_BLE_SET_ADVERTISING_DATA_CFM_T*)message );
        }
        break;
        case CL_DM_BLE_SET_ADVERTISING_PARAMS_CFM:
        {
            MAIN_DEBUG_L1(("CL_DM_BLE_SET_ADVERTISING_PARAMS_CFM\n"));
            sinkBleGapTriggerAdvertising();
        }
        break;
        case CL_DM_BLE_SECURITY_CFM:
        {
            MAIN_DEBUG_L1(("CL_DM_BLE_SECURITY_CFM [%x]\n",
                            ((const CL_DM_BLE_SECURITY_CFM_T*)message)->status));
        }
        break;
        case CL_DM_BLE_SET_CONNECTION_PARAMETERS_CFM:
        {
            MAIN_DEBUG_L1(("CL_DM_BLE_SET_CONNECTION_PARAMETERS_CFM [%x]\n",
                            ((const CL_DM_BLE_SET_CONNECTION_PARAMETERS_CFM_T*)message)->status));
        }
        break;
        case CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM:
        {
            MAIN_DEBUG_L1(("CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM [%x]\n",
                            ((const CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM_T*)message)->status));
        }
        break;
        case CL_DM_BLE_SET_SCAN_PARAMETERS_CFM:
        {
            MAIN_DEBUG_L1(("CL_DM_BLE_SET_SCAN_PARAMETERS_CFM [%x]\n",
                            ((const CL_DM_BLE_SET_SCAN_PARAMETERS_CFM_T*)message)->status));
        }
        break;
        case CL_DM_BLE_SET_SCAN_RESPONSE_DATA_CFM:
        {
            MAIN_DEBUG_L1(("CL_DM_BLE_SET_SCAN_RESPONSE_DATA_CFM\n"));
        }
        break;
        case CL_DM_BLE_READ_WHITE_LIST_SIZE_CFM:
        {
            MAIN_DEBUG_L1(("CL_DM_BLE_READ_WHITE_LIST_SIZE_CFM num[%d]\n",
                            ((const CL_DM_BLE_READ_WHITE_LIST_SIZE_CFM_T*)message)->white_list_size));
        }
        break;
        case CL_DM_BLE_CLEAR_WHITE_LIST_CFM:
        {
            MAIN_DEBUG_L1(("CL_DM_BLE_CLEAR_WHITE_LIST_CFM\n"));
        }
        break;
        case CL_DM_BLE_ADD_DEVICE_TO_WHITE_LIST_CFM:
        {
            MAIN_DEBUG_L1(("CL_DM_BLE_ADD_DEVICE_TO_WHITE_LIST_CFM status[%u]\n",
                            ((const CL_DM_BLE_ADD_DEVICE_TO_WHITE_LIST_CFM_T*)message)->status));
            sinkBleGapAddDeviceWhiteListCfm((const CL_DM_BLE_ADD_DEVICE_TO_WHITE_LIST_CFM_T*)message);
        }
        break;
        case CL_DM_BLE_REMOVE_DEVICE_FROM_WHITE_LIST_CFM:
        {
            MAIN_DEBUG_L1(("CL_DM_BLE_REMOVE_DEVICE_FROM_WHITE_LIST_CFM\n"));
        }
        break;
        case CL_DM_BLE_CONFIGURE_LOCAL_ADDRESS_CFM:
        {
            MAIN_DEBUG_L1(("CL_DM_BLE_CONFIGURE_LOCAL_ADDRESS_CFM\n"));
        }
        break;
        case CL_SM_BLE_READ_RANDOM_ADDRESS_CFM:
        {
            MAIN_DEBUG_L1(("CL_SM_BLE_READ_RANDOM_ADDRESS_CFM status[%u]\n",
                            ((const CL_SM_BLE_READ_RANDOM_ADDRESS_CFM_T*)message)->status));
            sinkBleGapReadRandomAddressCfm((const CL_SM_BLE_READ_RANDOM_ADDRESS_CFM_T*)message);
        }
        break;
        case CL_DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND:
        {
            const CL_DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND_T * req = (const CL_DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND_T*)message;
            bool accept = TRUE;

            MAIN_DEBUG_L1(("CL_DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND conn_interval_min [0x%x], conn_interval_max [0x%x] \n", req->conn_interval_min, req->conn_interval_max));

            /* Based on the allowed range, accept or reject the connection parameter updates. */
            if(SinkBleConnectionParameterIsOutOfRange(req))
            {
                accept = FALSE;
            }

#ifdef ENABLE_FAST_PAIR 
            /*This is applicable for first time pairing. Device is placed in pairing mode */
             if(stateManagerGetState() == deviceConnDiscoverable)
            {
                accept = TRUE;
            }
#endif
      
            ConnectionDmBleAcceptConnectionParUpdateResponse(accept, &req->taddr, req->id,
                                                req->conn_interval_min, req->conn_interval_max,
                                                req->conn_latency, req->supervision_timeout);
        }
        break;
        case CL_DM_BLE_CONNECTION_UPDATE_COMPLETE_IND:
        {
            MAIN_DEBUG_L1(("CL_DM_BLE_CONNECTION_UPDATE_COMPLETE_IND status %d, CI [0x%x] \n", ((const CL_DM_BLE_CONNECTION_UPDATE_COMPLETE_IND_T*)message)->status,
                                                                    ((const CL_DM_BLE_CONNECTION_UPDATE_COMPLETE_IND_T*)message)->conn_interval));
            ConnectionDmBleConnectionUpdateCompleteInd((const CL_DM_BLE_CONNECTION_UPDATE_COMPLETE_IND_T*)message);
        }
        break;

        case CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND:
        {
            MAIN_DEBUG_L1(("CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND [0x%x]\n",
                            ((const CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND_T*)message)->status));
                sinkBleSimplePairingCompleteInd((const CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND_T*)message);
        }
        break;
        case CL_DM_BLE_ADVERTISING_PARAM_UPDATE_IND:
        {
            MAIN_DEBUG_L1(("CL_DM_BLE_ADVERTISING_PARAM_UPDATE_IND\n"));
        }
        break;

        case CL_DM_APT_IND:
        {
            MAIN_DEBUG_L1(("CL_DM_APT_IND - APT expire indication \n"));
        }
        break;
        case CL_DM_READ_APT_CFM:
        {
            MAIN_DEBUG_L1(("CL_DM_READ_APT_CFM [%x]\n", ((CL_DM_READ_APT_CFM_T*)message)->apt));
        }
        break;
        case CL_SM_ENCRYPT_CFM:
        {
            MAIN_DEBUG_L1(("CL_SM_ENCRYPT_CFM \n"));
            sinkHandleBrEdrEncryptionCfm((const CL_SM_ENCRYPT_CFM_T*)message);
        }
        break;

        case CL_SM_BLE_LINK_SECURITY_IND:
        {
            MAIN_DEBUG_L1(("CL_SM_BLE_LINK_SECURITY_IND\n"));

            sinkBleLinkSecurityInd((const CL_SM_BLE_LINK_SECURITY_IND_T*)message);
        }
        break;

        case CL_SM_ENCRYPTION_CHANGE_IND:
        {
            MAIN_DEBUG_L1(("CL_SM_ENCRYPTION_CHANGE_IND \n"));
            sinkHandleBrEdrEncryptionChangeInd((const CL_SM_ENCRYPTION_CHANGE_IND_T*)message);
            sinkHandleBleEncryptionChangeInd((const CL_SM_ENCRYPTION_CHANGE_IND_T*)message);
        }
        break;

        case CL_DM_DUT_CFM:
        {
            MAIN_DEBUG_L1(("CL_DM_DUT_CFM [%x]\n", ((const CL_DM_DUT_CFM_T*)message)->status));
        }
        break;
#ifdef ENABLE_FAST_PAIR
        case CL_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM:
        case CL_CRYPTO_HASH_CFM:
        case CL_CRYPTO_ENCRYPT_CFM:
        case CL_CRYPTO_DECRYPT_CFM:
        {
            MAIN_DEBUG_L1(("CL_CRYPTO_IND\n"));

            sinkFastPairHandleMessages(task, id, message);
        }
        break;

#endif
            /* filter connection library messages */
        case CL_SDP_REGISTER_CFM:
        case CL_DM_WRITE_INQUIRY_ACCESS_CODE_CFM:
        case CL_DM_LINK_SUPERVISION_TIMEOUT_IND:
        break;

        /*all unhandled connection lib messages end up here*/
        default :
            MAIN_DEBUG_L1(("Sink - Unhandled CL msg[%x]\n", id));
        break ;
    }
}


static bool eventToBeIndicatedBeforeProcessing(const MessageId id)
{
    if(id == EventUsrMainOutMuteOn)
    {
        return TRUE;
    }
    return FALSE;
}

static void sinkRouteAudio(void)
{
    if(sinkAudioIsManualSrcSelectionEnabled())
    {
        if(!sinkAudioGetDefaultAudioSource())
        {
            audioSwitchToAudioSource(sinkAudioGetRequestedAudioSource());
        }
        else
        {
            audioSwitchToAudioSource((audio_sources)sinkAudioGetDefaultAudioSource());
        }
    }
    else
    {
        audioUpdateAudioRouting();
    }
}
static bool sinkPowerProcessEventPower(const MessageId EventPower)
{
    bool indicate_event = TRUE;

    if(peerProcessEvent(EventPower))
    {
        indicate_event = FALSE;
    }
    else
    {
        switch(EventPower)
        {
            case EventUsrPowerOn:
            case EventSysPowerOnPanic:
                MAIN_DEBUG(("HS: Power On\n" )) ;
                if (stateManagerGetState() == deviceLimbo)
                {
                    displaySetState(TRUE);
                    displayShowText(DISPLAYSTR_HELLO, strlen(DISPLAYSTR_HELLO), DISPLAY_TEXT_SCROLL_SCROLL, 1000, 2000, FALSE, 10, SINK_TEXT_TYPE_DEVICE_STATE);
                    displayUpdateVolume((VOLUME_NUM_VOICE_STEPS * sinkHfpDataGetDefaultVolume())/sinkVolumeGetNumberOfVolumeSteps(audio_output_group_main));
                    /* update battery display */
                    displayUpdateBatteryLevel(powerManagerIsChargerConnected());
                }

                sinkAncHandlePowerOn();

                stateManagerPowerOn();
                /* Indicate now "Power On" voice prompt before audio
                   plugins kick in, derisking playing audio with low volume.*/
                IndicateEvent(EventPower);
                indicate_event = FALSE;

                configManagerReadSessionData();
                {
                    sinkRouteAudio();
                }

                sinkFmSetFmRxOn(FALSE);

                sinkDataSetPowerUpNoConnectionFlag(TRUE);

                if(powerManagerIsVthmCritical())
                {
                    MessageSend(&theSink.task, EventUsrPowerOff, 0);
                }

                powerManagerReadVbat(battery_level_initial_reading);

                if(sinkDataGetEncryptionRefreshTimeout() != 0)
                {
                    MessageSendLater(&theSink.task, EventSysRefreshEncryption, 0, D_MIN(sinkDataGetEncryptionRefreshTimeout()));
                }

                if ( sinkDataCheckDisablePowerOffAfterPowerOn() )
                {
                    sinkDataSetPoweroffEnabled(FALSE);
                    MAIN_DEBUG(("DIS[%x]\n" , sinkDataGetDisablePoweroffAfterPoweronTimeout() )) ;
                    MessageSendLater ( &theSink.task , EventSysEnablePowerOff , 0 , D_SEC ( sinkDataGetDisablePoweroffAfterPoweronTimeout()) ) ;
                }
                else
                {
                    sinkDataSetPoweroffEnabled(TRUE);
                }

#ifdef ENABLE_SUBWOOFER
                /* check to see if there is a paired subwoofer device, if not kick off an inquiry scan */
                MessageSend(&theSink.task, EventSysSubwooferCheckPairing, 0);
#endif

                if(sinkDataDefragCheckTimout())
                {
                    MessageSendLater(&theSink.task, EventSysCheckDefrag, 0, D_SEC(sinkDataDefragCheckTimout()));
                }

                /* generate event to drive EQ indicator */
                sinkAudioEQOperatingStateIndication();

                MessageSend(&theSink.task, EventSysUpdateDevicesConnectedStatus, NULL);
            
                 /* Power on BLE */
                 sinkBlePowerOnEvent();

                sinkBroadcastAudioHandleUserPowerOn();
                
                PioDriveUserPio(PioGetUserPioState());

                sinkSportHealthEnableDefault();

                break ;

            case EventSysPeerGeneratedPowerOff:
            case EventUsrPowerOff:
                /* Reset the silence detected flag in case if the audio silence detected (i.e., set to 1) and before
                 * the timer expiry if user pressed the EventUsrPowerOff eventt.*/
                if(sinkAudioIsSilenceDetected())
                {
                    sinkAudioSetSilence(FALSE);
                }

                MAIN_DEBUG(("HS: PowerOff - En[%c]\n" , TRUE_OR_FALSE(sinkDataIsPowerOffEnabled()))) ;

                /* don't indicate event if already in limbo state */
                if(stateManagerGetState() == deviceLimbo)
                {
                    indicate_event = FALSE ;
                }

                /* only power off if timer has expired or battery is low and the charger isn't connected or temperature high */
                if ( sinkDataIsPowerOffEnabled() || ((!powerManagerIsChargerConnected() || 
                        sinkDataAllowAutomaticPowerOffWhenCharging()) && powerManagerIsVbatCritical()) || powerManagerIsVthmCritical())
                {
                    /* store current volume levels for non bluetooth volumes */
                    configManagerWriteSessionData () ;

                    configManagerWriteDspData();

                    PioDriveUserPio(FALSE);
                    
                    audioSwitchToAudioSource(audio_source_none);

                    stateManagerEnterLimboState(limboShutDown);
                    AuthResetConfirmationFlags();

                    sinkBroadcastAudioPowerOff();

                    VolumeUpdateMuteStatusAllOutputs(FALSE);
                    VolumeSetHfpMicrophoneGain(hfp_invalid_link, MICROPHONE_MUTE_OFF);

                    sinkClearQueueudEvent();

                    sinkStopAutoPowerOffTimer();

                    if(sinkDataGetEncryptionRefreshTimeout() != 0)
                    {
                        MessageCancelAll ( &theSink.task, EventSysRefreshEncryption) ;
                    }

                    MessageCancelAll (&theSink.task, EventSysCheckDefrag);
                    MessageCancelAll (&theSink.task, EventSysDefrag);

                    MessageCancelAll (&theSink.task, EventSysPairingFail);
#ifdef ENABLE_PEER
                    MessageCancelAll(&theSink.task , EventSysA2DPPeerLinkLossTimeout);
                    setA2dpPeerLinkLossReconnect(FALSE);
#endif

#ifdef ENABLE_AVRCP
                    /* cancel any queued ff or rw requests */
                    MessageCancelAll (&theSink.task, EventUsrAvrcpFastForwardPress);
                    MessageCancelAll (&theSink.task, EventUsrAvrcpRewindPress);
#endif
#ifdef ENABLE_SPEECH_RECOGNITION
                    /* if speech recognition is in tuning mode stop it */
                    if(speechRecognitionTuningIsActive())
                    {
                        speechRecognitionStop();
                        speechRecognitionSetTuningActive(FALSE);
                    }
#endif
                    if(sinkFmIsFmRxOn())
                    {
                        MessageSend(&theSink.task, EventUsrFmRxOff, 0);
                    }

#ifdef ENABLE_LINKKEY_DISPLAY
                    displayHideLinkKey();
#endif
                    /* keep the display on if charging */
                    if (powerManagerIsChargerConnected() && (stateManagerGetState() == deviceLimbo) )
                    {
                        displaySetState(TRUE);
                        displayUpdateVolume(0);
                        displayUpdateBatteryLevel(TRUE);
                    }
                    else
                    {
                        displaySetState(FALSE);
                    }

#ifdef ENABLE_GAIA
                    if (!gaia_get_remain_connected())
                    {
                        gaiaDisconnect();
                    }
#endif

#ifdef ENABLE_FAST_PAIR
                    /* Reset the counter while powering off*/
                    sinkFastPairResetFailCount();
                    sinkFastPairDisconnectLELink();
#endif
                    /* generate event to drive EQ indicator */
                    sinkAudioEQOperatingStateIndication();

                    /* Power off BLE if not in BA mode*/
                    sinkBlePowerOffEvent();

                    sinkAncHandlePowerOff();

                    MessageSend(&theSink.task, EventSysUpdateDevicesConnectedStatus, NULL);

                    deviceManagerResetAudioPromptPlayDeviceAttribute();

                    sinkSportHealthSMShutDown();
                }
                else
                {
                    indicate_event = FALSE ;
                }
        }
    }
    return indicate_event;
}

/*******************************************************************************
NAME
    handleDeviceAndAudioRoutingInfoEvents

DESCRIPTION
    This handler function indicate to user if the device(peer/ag) is connected
    or disconnected as well as it also indicate audio routing information to
    user via LEDs,

RETURNS
    void

*/
static void handleDeviceAndAudioRoutingInfoEvents(const MessageId id)
{
    /*Just print event here fir debug purpose so that we know which event
      is being fed to IndicateEvent() after switch statement */
    MAIN_DEBUG_L1(("Main : handleDeviceAndAudioRoutingInfoEvents() - Event %x\n",id));
    switch(id)
    {
        case EventSysUpdateDevicesConnectedStatus:
            handleEventSysUpdateDevicesConnectedStatus();
        break;

        /* at least one AG or no AG is connected to the device */
        case EventSysAgSourceConnected:
        case EventSysAllAgSourcesDisconnected:
        break;

#ifdef ENABLE_PEER
        /* at least one Peer is connected to the device */
        case EventSysPeerConnected:
            MAIN_DEBUG(( "HS : EventSysPeerConnected\n" ));
            break;

        /* no Peers are connected to the device */
        case EventSysPeerDisconnected:
            MAIN_DEBUG(( "HS : EventSysPeerDisconnected\n" ));
            peerSendAudioRoutingInformationToUser();
        break;

        /* Below events are generated when TWS audio routing is updated and this device
            becomes the corresponding channel(as per event) of a stereo TWS setup */
        case EventSysPeerAudioRoutingStereo:
        case EventSysPeerAudioRoutingLeft:
        case EventSysPeerAudioRoutingRight:
        case EventSysPeerAudioRoutingDownmix:
            break;

        /* at least one AG or no AG is connected to the Peer device */
        case EventSysPeerAgSourceConnected:
        case EventSysPeerAllAgSourcesDisconnected:
            break;

        case EventSysEnableAudioActive:
            MAIN_DEBUG(( "HS : EventSysEnableAudioActive( PIO_AUDIO_ACTIVE, %d )\n", (int)sinkAudioIsAudioRouted() ));
            audioUpdateAudioActivePio();
            break;
#endif
    }
}

static void disconnectAgResendOrError( sinkEvents_t eventToResend )
{
    /* disconnect the device considered to be least active, check if sucessful */
    if(peerDisconnectLowestPriorityAG())
    {
        /* re-send the event allowing enough time for the device to be disconnected */
        MessageSendLater ( &theSink.task , eventToResend , 0 , ALLOW_DEVICE_TO_DISCONNECT_DELAY) ;
    }
    /* request to disconnect AG failed, show error */
    else
    {
        MessageSend ( &theSink.task , EventSysError , 0 ) ;
    }
}

/*************************************************************************
NAME
    handleUEMessage

DESCRIPTION
    handles messages from the User Events

RETURNS

*/
static void handleUEMessage  ( Task task, MessageId id, Message message )
{
    /* Event state control is done by the config - we will be in the right state for the message
    therefore messages need only be passed to the relative handlers unless configurable */
    sinkState lState = stateManagerGetState() ;

    /*if we do not want the event received to be indicated then set this to FALSE*/
    bool lIndicateEvent = TRUE;
    bool lResetAutoSwitchOff = FALSE;

    /* If there has been an error with the configuraiton data, do not process
       any user events. */
    if (LedsInErrorMode() && (id <= EVENTS_USR_LAST_EVENT) && (id != EventUsrChargerDisconnected))
    {
        MAIN_DEBUG(("HS: In Error State - Ignoring User Event\n"));
        return;
    }

    /* Reset the auto switch off timer when either BT device disconnects. */
    if((id == EventSysPrimaryDeviceDisconnected) || (id == EventSysSecondaryDeviceDisconnected))
    {
        /* postpone auto switch-off */
        lResetAutoSwitchOff = TRUE;
    }

    /* Deal with user generated Event specific actions*/
    if (id < EVENTS_SYS_MESSAGE_BASE)
    {
        /*cancel any missed call indicator on a user event (button press)*/
        MessageCancelAll(task , EventSysMissedCall ) ;

        /* postpone auto switch-off */
        lResetAutoSwitchOff = TRUE;

        /* check for led timeout/re-enable */
        LedManagerCheckTimeoutState();

#ifdef ENABLE_GAIA
        gaiaReportUserEvent(id);
#endif
    }

    /* Indicate Audio routing and device connecting/disconnecting status*/
    if(((id >= EventSysPeerConnected) && (id <= EventSysPeerAllAgSourcesDisconnected)) || 
	    (id == EventSysEnableAudioActive))
    {
        handleDeviceAndAudioRoutingInfoEvents(id);
    }

    if(eventToBeIndicatedBeforeProcessing(id))
    {
        IndicateEvent(id);
    }

    MAIN_DEBUG (("UE[%x]\n", id));

    /* The configurable Events*/
    switch ( id )
    {
        case (EventSysResetWatchdog):
            watchdogReset();
        break;
        case (EventUsrDebugKeysToggle):
            MAIN_DEBUG(("HS: Toggle Debug Keys\n"));
            /* If the device has debug keys enabled then toggle on/off */
            ConnectionSmSecModeConfig(&theSink.task, cl_sm_wae_acl_none, !sinkDataIsDebugKeyEnabled(), TRUE);
        break;

        case EventUsrPowerOff:
        case EventUsrPowerOn:
        case EventSysPowerOnPanic:
        case EventSysPeerGeneratedPowerOff:
            lIndicateEvent = sinkPowerProcessEventPower(id);
            break ;
        case (EventUsrInitateVoiceRecognition):
        case (EventUsrInitateVoiceDial):
            MAIN_DEBUG(("HS: InitVoiceDial [%d]\n", sinkHfpDataGetVoiceRecognitionIsActive()));
                /*Toggle the voice dial behaviour depending on whether we are currently active*/
            if ( sinkDataIsPowerOffEnabled() )
            {
#ifdef ENABLE_BISTO
                if (!sinkVaEventHandler( id ))
                    sinkCommonProcessEvent(id, message);
#else
                lIndicateEvent = voiceDialProcessEvent(hfp_primary_link);
#endif
            }
            else
            {
                lIndicateEvent = FALSE ;
            }
        break ;
        case (EventUsrInitateVoiceDial_AG2):
            MAIN_DEBUG(("HS: InitVoiceDial AG2[%d]\n", sinkHfpDataGetVoiceRecognitionIsActive()));
                /*Toggle the voice dial behaviour depending on whether we are currently active*/
            if (sinkDataIsPowerOffEnabled() )
            {
#ifdef ENABLE_BISTO
                if (!sinkVaEventHandler( id ))
                    sinkCommonProcessEvent(id, message);
#else
                lIndicateEvent = voiceDialProcessEvent(hfp_secondary_link);
#endif
            }
            else
            {
                lIndicateEvent = FALSE ;
            }
        break ;
        case (EventUsrLastNumberRedial):
            MAIN_DEBUG(("HS: LNR\n" )) ;

            if ( sinkDataIsPowerOffEnabled() )
            {
                if (sinkHfpDataIsLNRCancelsVoiceDialIfActive())
                {
                    if (sinkHfpDataGetVoiceRecognitionIsActive() )
                    {
                        MessageSend(&theSink.task , EventUsrInitateVoiceDial , 0 ) ;
                        lIndicateEvent = FALSE ;
                    }
                    else
                    {
                        /* LNR on AG 1 */
                        sinkInitiateLNR(hfp_primary_link) ;
                    }
                }
                else
                {
                   /* LNR on AG 1 */
                    sinkInitiateLNR(hfp_primary_link) ;
                }
            }
            else
            {
                lIndicateEvent = FALSE ;
            }
        break ;
        case (EventUsrLastNumberRedial_AG2):
            MAIN_DEBUG(("HS: LNR AG2\n" )) ;
            if ( sinkDataIsPowerOffEnabled() )
            {
                if (sinkHfpDataIsLNRCancelsVoiceDialIfActive())
                {
                    if (sinkHfpDataGetVoiceRecognitionIsActive() )
                    {
                        MessageSend(&theSink.task , EventUsrInitateVoiceDial , 0 ) ;
                        lIndicateEvent = FALSE ;
                    }
                    else
                    {
                        /* LNR on AG 2 */
                        sinkInitiateLNR(hfp_secondary_link) ;
                    }
                }
                else
                {
                   /* LNR on AG 2 */
                   sinkInitiateLNR(hfp_secondary_link) ;
                }
            }
            else
            {
                lIndicateEvent = FALSE ;
            }
        break ;

        case (EventHidQualificationConnect):
        case (EventHidQualificationLimitedDiscoverable):
        case (EventUsrHidSelfiePress):
        case (EventUsrHidSelfieRelease):
            lIndicateEvent = sinkHidUserEvents(id);
        break;
        case (EventUsrAnswer):
            MAIN_DEBUG(("HS: Answer\n" )) ;
            /* don't indicate event if not in incoming call state as answer event is used
               for some of the multipoint three way calling operations which generate unwanted
               tones */
            if(stateManagerGetState() != deviceIncomingCallEstablish) lIndicateEvent = FALSE ;

            /* Call the HFP lib function, this will determine the AT cmd to send
               depending on whether the profile instance is HSP or HFP compliant. */
#ifdef ENABLE_BISTO
            if (!sinkVaEventHandler( id ))
                sinkCommonProcessEvent(id, message);
#else
            sinkAnswerOrRejectCall( TRUE );
#endif
        break ;
        case (EventUsrReject):
            MAIN_DEBUG(("HS: Reject\n" )) ;
            /* Reject incoming call - only valid for instances of HFP */
#ifdef ENABLE_BISTO
            if (!sinkVaEventHandler( id ))
                sinkCommonProcessEvent(id, message);
#else
            sinkAnswerOrRejectCall( FALSE );
#endif
        break ;
        case (EventUsrCancelEnd):
            MAIN_DEBUG(("HS: CancelEnd\n" )) ;
            /* Terminate the current ongoing call process */
#ifdef ENABLE_BISTO
            if (!sinkVaEventHandler( id ))
                sinkCommonProcessEvent(id, message);
#else
            sinkHangUpCall();
#endif
        break ;
        case (EventUsrTransferToggle):
            MAIN_DEBUG(("HS: Transfer\n" )) ;
            sinkTransferToggle();
        break ;
        case EventSysCheckForAudioTransfer :
            MAIN_DEBUG(("HS: Check Aud Tx\n")) ;
            sinkCheckForAudioTransfer(((AUDIO_TRANSFER_MESSAGE_T *)message)->priority);
            break ;

        case EventUsrMicrophoneMuteToggle:
        case EventUsrMicrophoneMuteOn:
        case EventUsrMicrophoneMuteOff:
        case EventUsrVolumeOrientationNormal:
        case EventUsrVolumeOrientationInvert:
        case EventUsrVolumeOrientationToggle:
        case EventUsrMainOutVolumeUp:
        case EventUsrMainOutVolumeDown:
        case EventUsrAuxOutVolumeUp:
        case EventUsrAuxOutVolumeDown:
        case EventUsrMainOutMuteOn:
        case EventUsrMainOutMuteOff:
        case EventUsrMainOutMuteToggle:
        case EventUsrAuxOutMuteOn:
        case EventUsrAuxOutMuteOff:
        case EventUsrAuxOutMuteToggle:
        case EventSysVolumeMax:
        case EventSysVolumeMin:
        case EventSysVolumeAndSourceChangeTimer:
        case EventSysPeerGeneratedMainMuteOn:
        case EventSysPeerGeneratedMainMuteOff:
        case EventSysUpdateMainVolume:
        case EventSysUpdateMainVolumeLocallyOnly:
        case EventSysPeerSelfGeneratedUpdateMainVolume:
        case EventSysPeerRemoteGeneratedUpdateMainVolume:
            MAIN_DEBUG(("Sys/Usr Volume Event\n"));
            id = sinkVolumeModifyEventAccordingToVolumeOrientation(id);
            lIndicateEvent = sinkVolumeProcessEventVolume(id);
            break;

        case (EventSysEnterPairingEmptyPDL):
        case (EventUsrEnterPairing):
            MAIN_DEBUG(("HS: EnterPair [%d]\n" , lState )) ;

            lIndicateEvent = FALSE ;
            
            /*go into pairing mode*/
            if(lState != deviceLimbo)
            {
                sinkBleBondableEvent();
                
                if(lState != deviceConnDiscoverable)
                {
                    sinkInquirySetInquirySession(inquiry_session_normal);
                    stateManagerEnterConnDiscoverableState( TRUE );
#ifdef ENABLE_FAST_PAIR
                    if(stateManagerGetState() == deviceConnDiscoverable)
                        sinkFastPairSetDiscoverabilityStatus(user_triggered_discoverability);
#endif
                    lIndicateEvent = TRUE;
                }
            }
        break ;
#ifdef ENABLE_FAST_PAIR
        case EventSysFastPairTimeout:
            {
                MAIN_DEBUG(("EventSysFastPairTimeout\n"));
                sinkFastPairSetFastPairState(fast_pair_state_idle);
            }
            break;

        case EventSysExitDiscoverableMode:
            {
                MAIN_DEBUG(("EventSysExitDiscoverableMode\n"));
                sinkBleExitDiscoverableModeEvent();
            }
            break;

        case EventSysFastPairDiscoverabilityTimeout:
            {
                MAIN_DEBUG(("EventSysFastPairDiscoverabilityTimeout\n"));
                sinkFastPairHandleDiscoverability();
            }
            break;
#endif
        case EventUsrBAStartAssociation:
            {
                MAIN_DEBUG(("EventUsrBroadcastAudioStartAssociation\n"));
                sinkBroadcastAudioStartAssociation();
            }
            break;
        case EventUsrBASwitchToNormalMode:
            MAIN_DEBUG(( "BA : Switch to Sink App Mode \n"));
            lIndicateEvent = sinkBroadcastAudioChangeMode(sink_ba_appmode_normal);
            break;
        case EventUsrBASwitchToBroadcaster:
            MAIN_DEBUG(( "BA : Switch to Broadcster Mode \n"));
            lIndicateEvent = sinkBroadcastAudioChangeMode(sink_ba_appmode_broadcaster);
            break;
        case EventUsrBASwitchToReceiver:
            MAIN_DEBUG(( "BA : Switch to Receiver \n"));
            lIndicateEvent = sinkBroadcastAudioChangeMode(sink_ba_appmode_receiver);
            break;
        case EventUsrBASetEcSchemeTo25:
            MAIN_DEBUG(( "BA : Set Erasure Coding Scheme to 2.5 \n"));
            sinkBroadcastAudioSetEcScheme(EC_SCHEME_2_5);
            (void)sinkBroadcastAudioChangeMode(sink_ba_appmode_broadcaster);
            break;
        case EventUsrBASetEcSchemeTo39:
            MAIN_DEBUG(( "BA : Set Erasure Coding Scheme to 3.9 \n"));
            sinkBroadcastAudioSetEcScheme(EC_SCHEME_3_9);
            (void)sinkBroadcastAudioChangeMode(sink_ba_appmode_broadcaster);
            break;
        case EventUsrCancelPairing:
            MAIN_DEBUG(("HS: Cancel Pairing\n")) ;
            MessageCancelAll(&theSink.task, EventSysPairingFail);
            MessageSend(&theSink.task, EventSysPairingFail, 0);

            sinkSportHealthSMDeviceStateCheck(EventUsrCancelPairing);
            break;

        case (EventSysPairingFail):
            /*we have failed to pair in the alloted time - return to the connectable state*/
            MAIN_DEBUG(("HS: Pairing Fail\n")) ;
            if (lState != deviceTestMode)
            {
                inquiryStop();
                switch (sinkDataGetActionOnParingmodeTimeout())
                {
                    case PAIRTIMEOUT_POWER_OFF:
                    {
                        if (!sinkFmIsFmRxOn())
                        {
                            if ( deviceManagerNumConnectedDevs() == 0 )
                            {
                                MessageSend ( task , EventUsrPowerOff , 0) ;
                            }
                            else
                            {
                                sinkDisableDiscoverable();
                                /* when not configured to stay disconverable at all times */
                                if(!sinkDataIsDiscoverableAtAllTimes())
                                {
                                    /* return to connected mode */
                                    stateManagerEnterConnectedState();
                                }
#ifdef ENABLE_PEER
                            /* Attempt peer connection, only if Broadcast audio is not active */
                            if(!sinkBroadcastAudioIsActive())
                            {							
                                /* Attempt to establish connection with Peer */
                                peerConnectPeer();
                            }							
#endif
                            }
                        }
                    }
                        break;
                    case PAIRTIMEOUT_POWER_OFF_IF_NO_PDL:
                        /* Power off if no entries in PDL list */
                        if (ConnectionTrustedDeviceListSize() == 0)
                        {
                            if (!sinkFmIsFmRxOn())
                            {
                                MessageSend ( task , EventUsrPowerOff , 0) ;
                            }
                        }
                        else
                        {
                            /* when not configured to stay disconverable at all times */
                            if(!sinkDataIsDiscoverableAtAllTimes())
                            {
                                /* enter connectable mode */
                                stateManagerEnterConnectableState(TRUE);
                            }
#ifdef ENABLE_PEER
                            /* Attempt peer connection, only if Broadcast audio is not active */
                            if(!sinkBroadcastAudioIsActive())
                                /* Attempt to establish connection with Peer */
                                peerConnectPeer();
#endif
                        }
                        break;
                    case PAIRTIMEOUT_CONNECTABLE:
                    default:
                        /* when not configured to stay disconverable at all times */
                        if(!sinkDataIsDiscoverableAtAllTimes())
                        {
                            /* Check if we were connected before*/
                            if(deviceManagerNumConnectedDevs() == 0)
                            {
                                /* enter connectable state */
                                stateManagerEnterConnectableState(TRUE);
                            }
                            else
                            {
                                /* return to connected mode */
                                stateManagerEnterConnectedState();
                            }
                        }
#ifdef ENABLE_PEER
                        /* Attempt peer connection, only if Broadcast audio is not active */
                        if(!sinkBroadcastAudioIsActive())
                            /* Attempt to establish connection with Peer */
                            peerConnectPeer();
#endif
                        break;
                }
            }
            /* have attempted to connect following a power on and failed so clear the power up connection flag */
            sinkDataSetPowerUpNoConnectionFlag(FALSE);

        break ;
        case ( EventSysPairingSuccessful):
            MAIN_DEBUG(("HS: Pairing Successful\n")) ;
            if (lState == deviceConnDiscoverable)
            {
                stateManagerEnterConnectableState(FALSE);
            }
        break ;

        case EventUsrEstablishPeerConnection:
            MAIN_DEBUG(("HS: Establish peer Connection\n"));
#ifdef ENABLE_PEER
                /* Attempt to establish connection with Peer */
                peerConnectPeer();
#endif
        break ;

        case ( EventUsrConfirmationAccept ):
            MAIN_DEBUG(("HS: Pairing Correct Res\n" )) ;
            sinkPairingAcceptRes();
        break;
        case ( EventUsrConfirmationReject ):
            MAIN_DEBUG(("HS: Pairing Reject Res\n" )) ;
            sinkPairingRejectRes();
        break;
        case ( EventUsrEstablishSLC ) :
                /* Make sure we're not using the Panic action */
                sinkDataSetPanicReconnect(FALSE);
                /* Fall through */
        case ( EventSysEstablishSLCOnPanic ):

#ifdef ENABLE_SUBWOOFER
            /* if performing a subwoofer inquiry scan then cancel the SLC connect request
               this will resume after the inquiry scan has completed */
            if(sinkInquiryCheckInqActionSub())
            {
                lIndicateEvent = FALSE;
                break;
            }
#endif
            /* check we are not already connecting before starting */
            {
                MAIN_DEBUG(("EventUsrEstablishSLC\n")) ;

                slcEstablishSLCRequest() ;

                /* don't indicate the event at first power up if the use different event at power on
                   feature bit is enabled, this enables the establish slc event to be used for the second manual
                   connection request */
                if(stateManagerGetState() == deviceConnectable)
                {
                    /* send message to do indicate a start of paging process when in connectable state */
                    MessageSend(&theSink.task, EventSysStartPagingInConnState ,0);
                }
            }
        break ;
        case ( EventUsrRssiPair ):
            MAIN_DEBUG(("HS: RSSI Pair\n"));
            lIndicateEvent = inquiryPair( inquiry_session_normal, TRUE );
        break;
        case ( EventSysRssiResume ):
            MAIN_DEBUG(("HS: RSSI Resume\n"));
            inquiryResume();
        break;
        case ( EventSysRssiPairReminder ):
            MAIN_DEBUG(("HS: RSSI Pair Reminder\n"));
            if (stateManagerGetState() != deviceLimbo )
                MessageSendLater(&theSink.task, EventSysRssiPairReminder, 0, D_SEC(INQUIRY_REMINDER_TIMEOUT_SECS));
            else
                lIndicateEvent = FALSE;

        break;
        case ( EventSysRssiPairTimeout ):
            MAIN_DEBUG(("HS: RSSI Pair Timeout\n"));
            inquiryTimeout();
        break;
        case ( EventSysRefreshEncryption ):
            MAIN_DEBUG(("HS: Refresh Encryption\n"));
            {
                uint8 k;
                Sink sink;
                Sink audioSink;
                /* For each profile */
                for(k=0;k<MAX_PROFILES;k++)
                {
                    MAIN_DEBUG(("Profile %d: ",k));
                    /* If profile is connected */
                    if((HfpLinkGetSlcSink((hfp_link_priority)(k + 1), &sink)) && sink)
                    {
                        /* If profile has no valid SCO sink associated with it */
                        HfpLinkGetAudioSink((hfp_link_priority)(k + hfp_primary_link), &audioSink);
                        if(!SinkIsValid(audioSink))
                        {
                            MAIN_DEBUG(("Key Refreshed\n"));
                            /* Refresh the encryption key */
                            ConnectionSmEncryptionKeyRefreshSink(sink);
                        }
#ifdef DEBUG_MAIN
                        else
                        {
                            MAIN_DEBUG(("Key Not Refreshed, SCO Active\n"));
                        }
                    }
                    else
                    {
                        MAIN_DEBUG(("Key Not Refreshed, SLC Not Active\n"));
#endif
                    }
                }
                MessageSendLater(&theSink.task, EventSysRefreshEncryption, 0, D_MIN(sinkDataGetEncryptionRefreshTimeout()));
            }
        break;

        /* 60 second timer has triggered to disable connectable mode in multipoint
            connection mode */
        case ( EventSysConnectableTimeout ) :
#ifdef ENABLE_SUBWOOFER
            if(!SwatGetSignallingSink(sinkSwatGetDevId()))
            {
                MAIN_DEBUG(("SM: disable Connectable Cancelled due to lack of subwoofer\n" ));
                break;
            }
#endif
#ifdef ENABLE_PARTYMODE
            /* leave headset connectable when using party mode */
            if(!(sinkPartymodeIsEnabled()))
#endif
            {
                /* only disable connectable mode if at least one hfp instance is connected */
                if(deviceManagerNumConnectedDevs())
                {
                    MAIN_DEBUG(("SM: disable Connectable \n" ));
                    /* disable connectability */
                    sinkDisableConnectable();
                }
            }
        break;

        case ( EventSysLEDEventComplete ) :
            /*the message is a ptr to the event we have completed*/
            MAIN_DEBUG(("HS : LEDEvCmp[%x]\n" ,  (( LMEndMessage_t *)message)->Event  )) ;

            LedsSetLedCurrentIndicateEvent(FALSE);
            switch ( (( const LMEndMessage_t *)message)->Event )
            {
                case (EventUsrResetPairedDeviceList) :
                {      /*then the reset has been completed*/
                    MessageSend(&theSink.task , EventSysResetComplete , 0 ) ;

                        /*power cycle if required*/
                    if ((sinkDataIsPowerOffAfterPDLReset() )&&
                        (stateManagerGetState() > deviceLimbo ))
                    {
                        MAIN_DEBUG(("HS: Reboot After Reset\n")) ;
                        if (!sinkFmIsFmRxOn())
                        {
                            MessageSend ( &theSink.task , EventUsrPowerOff , 0 ) ;
                        }
                    }
                }
                break ;

                case EventUsrPowerOff:
                {
                    /* Determine if a reset is required because the PS needs defragmentation */
                    configManagerDefragIfRequired();

                    /*allows a reset of the device for those designs which keep the chip permanently powered on*/
                    if (sinkDataIsResetAfterPowerOff() )
                    {
                        MAIN_DEBUG(("Reset\n"));
                        /* Reboot always - set the same boot mode; this triggers the target to reboot.*/
                        BootSetMode(BootGetMode());
                    }

                    sinkEventQueueDelete();

                    if(powerManagerIsVthmCritical())
                        stateManagerUpdateLimboState();

                }
                break ;

                default:
                break ;
            }

            if (LedManagerQueueLedEvents())
            {
                    /*if there is a queueud event*/
                if (LedManagerQueuedEvent())
                {
                    MAIN_DEBUG(("HS : Play Q'd Ev [%x]\n", LedManagerQueuedEvent()));
                    LedManagerIndicateQueuedEvent();
                }
                else
                {
                    /* restart state indication, the state is retrieved inside the function */
                    LedManagerIndicateExtendedState ();
                }
            }
            else
                LedManagerIndicateExtendedState ();

        break ;
        case (EventSysAutoSwitchOff):
            lResetAutoSwitchOff = sinkHandleAutoPowerOff(task, lState);
            break;

        case (EventUsrChargerConnected):
        {
            MAIN_DEBUG(("HS: Charger Connected\n"));
            powerManagerChargerConnected();

            if (stateManagerIsShuttingDown())
            {
                stateManagerEnterLimboState(limboWakeUp);
            }

            if ( lState == deviceLimbo )
            {
                stateManagerUpdateLimboState();
                displaySetState(TRUE);
            }

            /* indicate battery charging on the display */
            displayUpdateBatteryLevel(TRUE);
        }
        break;
        case (EventUsrChargerDisconnected):
        {
            MAIN_DEBUG(("HS: Charger Disconnected\n"));
            powerManagerChargerDisconnected();

            displayRemoveText(SINK_TEXT_TYPE_CHARGER_STATE);
            powerManagerChargerError(FALSE);

            /* if in limbo state, schedule a power off event */
            if ( lState == deviceLimbo )
            {
                /* cancel existing limbo timeout and rescheduled another limbo timeout */
                sinkCancelAndSendLater(EventSysLimboTimeout, D_SEC(sinkDataGetLimboTimeout()));
                stateManagerEnterLimboState(limboShutDown);
            }

            if (stateManagerIsShuttingDown())
            {
                /* turn off the display if shutting down and no longer charging */
                displaySetState(FALSE);
            }
            else
            {
                /* update battery display */
                displayUpdateBatteryLevel(FALSE);
            }
        }
        break;
        case (EventSysChargerError):
        {
            MAIN_DEBUG(("HS: Charger Error\n"));
            displayShowSimpleText("Charger Error", SINK_TEXT_TYPE_CHARGER_STATE);
            powerManagerChargerError(TRUE);
            powerManagerUpdateChargeCurrent();
        }
        break;
        case (EventUsrQuickChargeStatInterrupt):
        {
            MAIN_DEBUG(("HS: Quick Charge Stat Interrupt\n"));
            powerManagerServiceQuickChargeStatInterrupt();
        }
        break;
        case (EventUsrResetPairedDeviceList):
            {
                /* NOTE: For Soundbar application,
                 * devices considered as "protected" (i.e BLE
                 * remote or subwoofer) will not be deleted in this
                 * event.*/

                MAIN_DEBUG(("HS: --Reset PDL--")) ;
                if ( stateManagerIsConnected () )
                {
                   /*disconnect any connected HFP profiles*/
                   sinkDisconnectAllSlc();
                   /*disconnect any connected A2DP/AVRCP profiles*/
                   disconnectAllA2dpAvrcp(TRUE);
                }

                deviceManagerRemoveAllDevices();

#ifdef ENABLE_PEER
                /* Ensure permanently paired Peer device is placed back into PDL */
                AuthInitPermanentPairing();
#endif

                if(INQUIRY_ON_PDL_RESET)
                    MessageSend(&theSink.task, EventUsrRssiPair, 0);

#ifdef ENABLE_FAST_PAIR
                sinkFastPairDeleteAccountKeys();
#endif
            }
        break ;
        case ( EventSysLimboTimeout ):
            {
                /*we have received a power on timeout - shutdown*/
                MAIN_DEBUG_L1(("HS: EvLimbo TIMEOUT\n")) ;
                if (lState != deviceTestMode)
                {
                    /* If Dormant mode is not supported, Limbo to power off from here */
                    /* Otherwise Dormant mode is supposed to take care of this */
                    if (!sinkDormantIsSupported())
                    {
                        stateManagerEnterLimboState(limboShutDown) ;
                    }
                    stateManagerUpdateLimboState();
                }
            }
        break ;
        case EventSysSLCDisconnected:
                MAIN_DEBUG(("HS: EvSLCDisconnect\n")) ;
            {
                sinkHfpDataSetVoiceRecognitionIsActive(FALSE);
                MessageCancelAll ( &theSink.task , EventSysNetworkOrServiceNotPresent ) ;
            }
        break ;
        case (EventSysLinkLoss):
            MAIN_DEBUG(("HS: Link Loss\n")) ;
            {
                conn_mask mask;
                bdaddr  *linkloss_bd_addr;
                /* should the device have been powered off prior to a linkloss event being
                   generated, this can happen if a link loss has occurred within 5 seconds
                   of power off, ensure the device does not attempt to reconnet from limbo mode */
                if(stateManagerGetState()== deviceLimbo)
                    lIndicateEvent = FALSE;
                /* get the link loss bd address */
                    linkloss_bd_addr = sinkDataGetLinkLossBDaddr();
                /* Only get the profiles for the device which has a link loss.
                   If it is not specified fallback to all connected profiles. */
                if (linkloss_bd_addr && !BdaddrIsZero(linkloss_bd_addr))
                {
                    mask = deviceManagerProfilesConnected(linkloss_bd_addr);
                }
                else
                {
                    mask = deviceManagerGetProfilesConnected();
                }

                MAIN_DEBUG(("MAIN:    mask 0x%x linkLossReminderTime %u protection %u routed_audio 0x%p\n",
                    mask, sinkHfpDataGetLinkLossReminderTime(),
                    (sinkDataGetStreamProtectionState() == linkloss_stream_protection_on),
                    (void *)sinkAudioGetRoutedAudioSink()));

                /* The hfp library will generate repeat link loss events but
                   the a2dp library doesn't, so only repeat it here if:
                   a2dp only is in use, peer link loss stream protection is
                   not on (or there is no routed audio), or the link loss interval is > 0. */
                if (((!sinkHfpDataGetSupportedProfile()) || (!(mask & conn_hfp) && (mask & conn_a2dp)))
                    && (linkloss_bd_addr
                        && !(linklossIsStreamProtected(linkloss_bd_addr) && sinkAudioIsAudioRouted()))
                    && (sinkHfpDataGetLinkLossReminderTime() != 0))
                {
                    linklossSendLinkLossTone(linkloss_bd_addr, D_SEC(sinkHfpDataGetLinkLossReminderTime()));

                    /* update the number and type of devices connected */
                    MessageSend(&theSink.task, EventSysUpdateDevicesConnectedStatus, NULL);
                }
                if(sinkDataGetAutoSwitchOffTimeout() !=0)
                {
                    MAIN_DEBUG(("APD restart\n"));                    
                    sinkStartAutoPowerOffTimer();
                }
                SinkVaHandleLinkLoss();
            }
        break ;
        case (EventSysMuteReminder) :
            MAIN_DEBUG(("HS: Mute Remind\n")) ;
            /*arrange the next mute reminder tone*/
            MessageSendLater( &theSink.task , EventSysMuteReminder , 0 ,D_SEC(sinkHfpDataGetMuteRemindTime() ) );

            /* only play the mute reminder tone if AG currently having its audio routed is in mute state */
            if(!VolumePlayMuteToneQuery())
                lIndicateEvent = FALSE;
        break;

        case EventUsrBatteryLevelRequest:
          MAIN_DEBUG(("EventUsrBatteryLevelRequest\n")) ;
          powerManagerReadVbat(battery_level_user);
        break;

        case EventSysBatteryCritical:
            MAIN_DEBUG(("HS: EventSysBatteryCritical\n")) ;
            /* Test code for HF indicator Battery Level */
#ifdef TEST_HF_INDICATORS
            hfIndicatorNotify(hf_battery_level, 1);
#endif /* TEST_HF_INDICATORS*/
        break;

        case EventSysBatteryLow:
            MAIN_DEBUG(("HS: EventSysBatteryLow\n")) ;
            /* Test code for HF indicator Battery Level */
#ifdef TEST_HF_INDICATORS
            hfIndicatorNotify(hf_battery_level, 10);
#endif /* TEST_HF_INDICATORS*/
        break;

        case EventSysGasGauge0 :
        case EventSysGasGauge1 :
        case EventSysGasGauge2 :
        case EventSysGasGauge3 :
            MAIN_DEBUG(("HS: EventSysGasGauge%d\n", id - EventSysGasGauge0)) ;
        break ;

        case EventSysBatteryOk:
            MAIN_DEBUG_L1(("HS: EventSysBatteryOk\n")) ;
            /* Test code for HF indicator Battery Level */
#ifdef TEST_HF_INDICATORS
            hfIndicatorNotify(hf_battery_level, 100);
#endif /* TEST_HF_INDICATORS*/
        break;

        case EventSysChargeInProgress:
            MAIN_DEBUG_L1(("HS: EventSysChargeInProgress\n")) ;
            displayShowSimpleText("Charging", SINK_TEXT_TYPE_CHARGER_STATE);
        break;

        case EventSysChargeComplete:
            MAIN_DEBUG(("HS: EventSysChargeComplete\n")) ;
            displayShowSimpleText("Charged", SINK_TEXT_TYPE_CHARGER_STATE);
        break;

        case EventSysChargeDisabled:
            MAIN_DEBUG(("HS: EventSysChargeDisabled\n"));
            if (!powerManagerHasChargerError())
                displayRemoveText(SINK_TEXT_TYPE_CHARGER_STATE);
        break;

        case EventUsrEnterDUTState :
        {
            MAIN_DEBUG(("EnterDUTState \n")) ;
            stateManagerEnterTestModeState();
        }
        break;
        case EventUsrEnterDutMode :
        {
            MAIN_DEBUG(("Enter DUT Mode \n")) ;
            if (lState !=deviceTestMode)
            {
                MessageSend( task , EventUsrEnterDUTState, 0 ) ;
            }
            enterDutMode () ;
        }
        break;

        case EventUsrEnterTXContTestMode :
        {
            MAIN_DEBUG(("Enter TX Cont \n")) ;
            if (lState !=deviceTestMode)
            {
                MessageSend( task , EventUsrEnterDUTState , 0 ) ;
            }
            enterTxContinuousTestMode() ;
        }
        break ;

        case EventSysNetworkOrServiceNotPresent:
            {       /*only bother to repeat this indication if it is not 0*/
                sinkCancelAndIfDelaySendLater(EventSysNetworkOrServiceNotPresent,
                                        D_SEC(sinkHfpDataGetNWSIndicatorRepeatTime())) ;
                MAIN_DEBUG(("HS: NO NETWORK [%d]\n", sinkHfpDataGetNWSIndicatorRepeatTime() )) ;
            }
        break ;
        case EventSysNetworkOrServicePresent:
            {
                MessageCancelAll ( task , EventSysNetworkOrServiceNotPresent ) ;
                MAIN_DEBUG(("HS: YES NETWORK\n")) ;
            }
        break ;
        case EventUsrLedsOnOffToggle  :
            MAIN_DEBUG(("HS: Toggle EN_DIS LEDS ")) ;
            MAIN_DEBUG(("HS: Tog Was[%c]\n" , TRUE_OR_FALSE(LedManagerIsEnabled())));

            LedManagerToggleLEDS();
            MAIN_DEBUG(("HS: Tog Now[%c]\n" , TRUE_OR_FALSE(LedManagerIsEnabled())));

            break ;
        case EventUsrLedsOn:
            MAIN_DEBUG(("HS: Enable LEDS\n")) ;
            LedManagerEnableLEDS ( ) ;
            peerUpdateLedIndication(TRUE);
            break ;
        case EventUsrLedsOff:
            MAIN_DEBUG(("HS: Disable LEDS\n")) ;
            LedManagerDisableLEDS ( ) ;
            peerUpdateLedIndication(FALSE);
            break ;
        case EventSysCancelLedIndication:
            MAIN_DEBUG(("HS: Disable LED indication\n")) ;
            LedManagerResetLEDIndications ( ) ;
            break ;
        case EventSysCallAnswered:
            MAIN_DEBUG(("HS: EventSysCallAnswered\n")) ;
        break;

        case EventSysSLCConnected:
        case EventSysSLCConnectedAfterPowerOn:

            MAIN_DEBUG(("HS: EventSysSLCConnected\n")) ;
            /*if there is a queued event - we might want to know*/
            sinkRecallQueuedEvent();

            /* postpone auto switch-off */
            lResetAutoSwitchOff = TRUE;
        break;

        case EventSysPrimaryDeviceConnected:
        case EventSysSecondaryDeviceConnected:
            MAIN_DEBUG(("HS:Device Connected [%c]\n " , (id - EventSysPrimaryDeviceConnected)? 'S' : 'P'  )); 
            sinkNfcBTDeviceConnectedInd(task);
            MessageSend(&theSink.task, EventSysUpdateDevicesConnectedStatus, NULL);
            break;
        case EventSysPrimaryDeviceDisconnected:
        case EventSysSecondaryDeviceDisconnected:
            /*used for indication purposes only*/
            MAIN_DEBUG(("HS:Device Disconnected [%c]\n " , (id - EventSysPrimaryDeviceDisconnected)? 'S' : 'P'  ));
            sinkNfcBTDeviceDisconnectedInd(task);
            MessageSend(&theSink.task, EventSysUpdateDevicesConnectedStatus, NULL);
#ifdef ENABLE_LINKKEY_DISPLAY
            displayHideLinkKey();
#endif
        break;
        case EventSysLongTimer:

        case EventSysVLongTimer:
           if (lState == deviceLimbo)
           {
               lIndicateEvent = FALSE ;
           }
        break ;
            /*these events have no required action directly associated with them  */
             /*they are received here so that LED patterns and Tones can be assigned*/
        case EventSysSCOLinkOpen :
            MAIN_DEBUG(("EventSysSCOLinkOpen\n")) ;
        break ;
        case EventSysSCOLinkClose:
            MAIN_DEBUG(("EventSysSCOLinkClose\n")) ;
        break ;
        case EventSysEndOfCall :
            MAIN_DEBUG(("EventSysEndOfCall\n")) ;
            displayRemoveText(SINK_TEXT_TYPE_CALLER_INFO);
            displayRemoveText(SINK_TEXT_TYPE_DEVICE_STATE);
        break;
        case EventSysResetComplete:
            MAIN_DEBUG(("EventSysResetComplete\n")) ;
        break ;
        case EventSysError:
            MAIN_DEBUG(("EventSysError\n")) ;
        break;
        case EventSysReconnectFailed:
            MAIN_DEBUG(("EventSysReconnectFailed\n")) ;
        break;

#ifdef THREE_WAY_CALLING
        case EventUsrThreeWayReleaseAllHeld:
            MAIN_DEBUG(("HS3 : RELEASE ALL\n"));
            /* release the held call */
            MpReleaseAllHeld();
        break;
        case EventUsrThreeWayAcceptWaitingReleaseActive:
            MAIN_DEBUG(("HS3 : ACCEPT & RELEASE\n"));
            MpAcceptWaitingReleaseActive();
        break ;
        case EventUsrThreeWayAcceptWaitingHoldActive  :
            MAIN_DEBUG(("HS3 : ACCEPT & HOLD\n"));
            /* three way calling not available in multipoint usage */
            MpAcceptWaitingHoldActive();
        break ;
        case EventUsrThreeWayAddHeldTo3Way  :
            MAIN_DEBUG(("HS3 : ADD HELD to 3WAY\n"));
            /* check to see if a conference call can be created, more than one call must be on the same AG */
            MpHandleConferenceCall(TRUE);
        break ;
        case EventUsrThreeWayConnect2Disconnect:
            MAIN_DEBUG(("HS3 : EXPLICIT TRANSFER\n"));
            /* check to see if a conference call can be created, more than one call must be on the same AG */
            MpHandleConferenceCall(FALSE);
        break ;
#endif
        case (EventSysEnablePowerOff):
        {
            MAIN_DEBUG(("HS: EventSysEnablePowerOff \n")) ;
            sinkDataSetPoweroffEnabled(TRUE);
        }
        break;
        case EventUsrPlaceIncomingCallOnHold:
            sinkPlaceIncomingCallOnHold();
        break ;

        case EventUsrAcceptHeldIncomingCall:
            sinkAcceptHeldIncomingCall();
        break ;
        case EventUsrRejectHeldIncomingCall:
            sinkRejectHeldIncomingCall();
        break;

        case EventUsrEnterDFUMode:
        {
            MAIN_DEBUG(("EventUsrEnterDFUMode\n")) ;
            BootSetMode(BOOTMODE_DFU);
        }
        break;
        case EventSysServiceModeEntered:
        {
            MAIN_DEBUG(("Service Mode!!!\n")) ;
        }
        break;

        case EventUsrUpdateStoredNumber:
            sinkUpdateStoredNumber();
        break;

        case EventUsrDialStoredNumber:
            MAIN_DEBUG(("EventUsrDialStoredNumber\n"));
            sinkDialStoredNumber();
        break;

        case EventUsrDialGivenNumber:
#ifdef ENABLE_BISTO
            MAIN_DEBUG(("EventUsrDialGivenNumber\n"));
            sinkDialSavedGivenNumber();
#endif
        break;

        case EventUsrRestoreDefaults:
            MAIN_DEBUG(("EventUsrRestoreDefaults\n"));
            configManagerRestoreDefaults();

        break;

        case EventSysTone1:
        case EventSysTone2:
            MAIN_DEBUG(("HS: EventTone[%d]\n" , (id - EventSysTone1 + 1) )) ;
        break;

        case EventUsrSelectAudioPromptLanguageMode:
            if(SinkAudioPromptsIsEnabled())
            {
                MAIN_DEBUG(("EventUsrSelectAudioPromptLanguageMode"));
                AudioPromptSelectLanguage();
            }
            else
            {
                lIndicateEvent = FALSE ;
            }
        break;

        /* disabled leds have been re-enabled by means of a button press or a specific event */
        case EventSysResetLEDTimeout:
            MAIN_DEBUG(("EventSysResetLEDTimeout\n"));
            LedManagerIndicateExtendedState ();
            LedManagerSetStateTimeout(FALSE);
        break;
        /* starting paging whilst in connectable state */
        case EventSysStartPagingInConnState:
            MAIN_DEBUG(("EventSysStartPagingInConnState\n"));
            /* set bit to indicate paging status */
            sinkDataSetPagingInProgress(TRUE);
        break;

        /* paging stopped whilst in connectable state */
        case EventSysStopPagingInConnState:
            MAIN_DEBUG(("EventSysStartPagingInConnState\n"));
            /* set bit to indicate paging status */
            sinkDataSetPagingInProgress(FALSE);
        break;

        /* continue the slc connection procedure, will attempt connection
           to next available device */
        case EventSysContinueSlcConnectRequest:
            /* don't continue connecting if in pairing mode */
            if(stateManagerGetState() != deviceConnDiscoverable)
            {
                MAIN_DEBUG(("EventSysContinueSlcConnectRequest\n"));
                /* attempt next connection */
                slcContinueEstablishSLCRequest();
            }
        break;

        /* indication of call waiting when using two AG's in multipoint mode */
        case EventSysMultipointCallWaiting:
            MAIN_DEBUG(("EventSysMultipointCallWaiting\n"));
        break;

        /* kick off a check the role of the device and make changes if appropriate by requesting a role indication */
        case EventSysCheckRole:
            linkPolicyCheckRoles();
        break;

        case EventSysMissedCall:
        {
            if(sinkHfpDataGetMissedCallIndicateTime())
            {
                MessageCancelAll(task , EventSysMissedCall ) ;

                sinkHfpDataSetMissedCallIndicated(sinkHfpDataGetMissedCallIndicated() - 1);
                if(sinkHfpDataGetMissedCallIndicated() != 0)
                {
                    MessageSendLater( &theSink.task , EventSysMissedCall , 0 , D_SEC(sinkHfpDataGetMissedCallIndicateTime())) ;
                }
            }
        }
        break;
        case EventUsrSinkQualificationSPPConnDisconnRequest:
        {
            MAIN_DEBUG(("EventUsrSinkQualificationSPPConnDisconnRequest\n"));
            if(NULL == sppGetSinkData())
            {
                sppUsrConnectRequest();
            }
            else
            {
                sppUsrDisconnectRequest();
            }
            break;
        }
        case EventUsrHfpEnhancedSafetyIndicator:
        {
#ifdef TEST_HF_INDICATORS
            hfIndicatorNotify(hf_enhanced_safety, 0);
#endif /*  TEST_HF_INDICATORS */
        }
        break;

#ifdef ENABLE_PBAP
        case EventUsrPbapDialMch:
        {
            /* pbap dial from missed call history */
            MAIN_DEBUG(("EventUsrPbapDialMch\n"));

            if ( sinkDataIsPowerOffEnabled())
            {
                /* If voice dial is active, cancel the voice dial if the feature bit is set */
                if (sinkHfpDataIsLNRCancelsVoiceDialIfActive()&&
                    sinkHfpDataGetVoiceRecognitionIsActive())
                {
                    MessageSend(&theSink.task , EventUsrInitateVoiceDial , 0 ) ;
                    lIndicateEvent = FALSE ;
                }
                else
                {
                    pbapDialPhoneBook(pbap_mch);
                }
            }
            else
            {
                lIndicateEvent = FALSE ;
            }
        }
        break;

        case EventUsrPbapDialIch:
        {
            /* pbap dial from incoming call history */
            MAIN_DEBUG(("EventUsrPbapDialIch\n"));

            if (sinkDataIsPowerOffEnabled() )
            {
                /* If voice dial is active, cancel the voice dial if the feature bit is set */
                if (sinkHfpDataIsLNRCancelsVoiceDialIfActive() &&
                    sinkHfpDataGetVoiceRecognitionIsActive())
                {
                    MessageSend(&theSink.task , EventUsrInitateVoiceDial , 0 ) ;
                    lIndicateEvent = FALSE ;
                }
                else
                {
                    pbapDialPhoneBook(pbap_ich);
                }
            }
            else
            {
                lIndicateEvent = FALSE ;
            }
        }
        break;

        case EventSysEstablishPbap:
        {
            MAIN_DEBUG(("EventSysEstablishPbap\n"));

            /* Connect to the primary and secondary hfp link devices */
            pbapSetCommand(pbapc_action_idle);

            pbapConnect( hfp_primary_link );
            pbapConnect( hfp_secondary_link );
        }
        break;

        case EventUsrPbapSetPhonebook:
        {
            MAIN_DEBUG(("EventUsrPbapSetPhonebook, active pb is [%d]\n", pbapGetActivePhonebook()));

            pbapSetBrowseEntryIndex(0);
            pbapSetCommand(pbapc_setting_phonebook);

            if(pbapGetActiveLink() == pbapc_invalid_link)
            {
                pbapConnect( hfp_primary_link );
            }
            else
            {
                /* Set the link to active state */
                linkPolicySetLinkinActiveMode(PbapcGetSink(pbapGetActiveLink()));

                PbapcSetPhonebookRequest(pbapGetActiveLink(), pbapGetPhoneRepository(), pbapGetActivePhonebook());
            }

            lIndicateEvent = FALSE ;
        }
        break;

        case EventUsrPbapBrowseEntry:
        {
            MAIN_DEBUG(("EventUsrPbapBrowseEntry\n"));

            if(pbapGetCommand() == pbapc_action_idle)
            {
                /* If Pbap profile does not connected, connect it first */
                if(pbapGetActiveLink() == pbapc_invalid_link)
                {
                    pbapConnect( hfp_primary_link );
                    pbapSetBrowsingStartFlag(1);
                }
                else
                {
                    /* Set the link to active state */
                    linkPolicySetLinkinActiveMode(PbapcGetSink(pbapGetActiveLink()));

                    if(pbapGetBrowsingStartFlag() == 0)
                    {
                        pbapSetBrowsingStartFlag(1);
                        PbapcSetPhonebookRequest(pbapGetActiveLink(), pbapGetPhoneRepository(), pbapGetActivePhonebook());
                    }
                    else
                    {
                        MessageSend ( &theSink.task , PBAPC_APP_PULL_VCARD_ENTRY , 0 ) ;
                    }
                }

                pbapSetCommand(pbapc_browsing_entry);
            }

            lIndicateEvent = FALSE ;
        }
        break;

        case EventUsrPbapBrowseList:
        /* EventUsrPbapBrowseListByName event is added for PTS qualification */
        case EventUsrPbapBrowseListByName:
        {
            MAIN_DEBUG(("EventUsrPbapBrowseList%s\n",(id == EventUsrPbapBrowseListByName) ? "ByName" : "" ));

            if(pbapGetCommand() == pbapc_action_idle)
            {
                if(pbapGetActiveLink() == pbapc_invalid_link)
                {
                    pbapConnect( hfp_primary_link );
                }
                else
                {
                    /* Set the link to active state */
                    linkPolicySetLinkinActiveMode(PbapcGetSink(pbapGetActiveLink()));

                    PbapcSetPhonebookRequest(pbapGetActiveLink(), pbapGetPhoneRepository(), pbapGetActivePhonebook());
                }

                pbapSetCommand(pbapc_browsing_list);

                if (id == EventUsrPbapBrowseListByName)
                {
                    pbapSetSearchAttribute(pbap_search_name);
                    /* for PTS qualification, search string expected is "PTS" */
                    pbapSetSearchValue((const uint8*) "PTS");
                }
                else
                {
                    pbapSetSearchAttribute(pbap_search_number);
                    pbapSetSearchValue(NULL);
                }
            }

            lIndicateEvent = FALSE ;
        }
        break;

        case EventUsrPbapDownloadPhonebook:
        {
            MAIN_DEBUG(("EventUsrPbapDownloadPhonebook\n"));

            if(pbapGetCommand() == pbapc_action_idle)
            {
                if(pbapGetActiveLink() == pbapc_invalid_link)
                {
                    pbapConnect( hfp_primary_link );
                }
                else
                {
                    /* Set the link to active state */
                    linkPolicySetLinkinActiveMode(PbapcGetSink(pbapGetActiveLink()));

                    MessageSend(&theSink.task , PBAPC_APP_PULL_PHONE_BOOK , 0 ) ;
                }

                pbapSetCommand(pbapc_downloading);
            }

            lIndicateEvent = FALSE ;
        }
        break;

        case EventUsrPbapGetPhonebookSize:
        {
            MAIN_DEBUG(("EventUsrPbapGetPhonebookSize"));

            if(pbapGetCommand() == pbapc_action_idle)
            {
                if(pbapGetActiveLink() == pbapc_invalid_link)
                {
                    pbapConnect( hfp_primary_link );
                }
                else
                {
                    /* Set the link to active state */
                    linkPolicySetLinkinActiveMode(PbapcGetSink(pbapGetActiveLink()));

                    MessageSend(&theSink.task , PBAPC_APP_PHONE_BOOK_SIZE , 0 ) ;
                }

                pbapSetCommand(pbapc_phonebooksize);
            }

            lIndicateEvent = FALSE ;
        }
        break;

        case EventUsrPbapSelectPhonebookObject:
        {
            MAIN_DEBUG(("EventUsrPbapSelectPhonebookObject\n"));

            pbapSetBrowseEntryIndex(0);
            pbapSetBrowsingStartFlag(0);

            if(pbapGetCommand() == pbapc_action_idle)
            {
                pbapSetActivePhonebook(pbapGetActivePhonebook() + 1);

                if(pbapGetActivePhonebook() > pbap_cch)
                {
                    pbapSetActivePhonebook(pbap_pb);
                }
            }

            lIndicateEvent = FALSE ;
        }
        break;

        case EventUsrPbapBrowseComplete:
        {
            MAIN_DEBUG(("EventUsrPbapBrowseComplete\n"));

            /* Set the link policy based on the HFP or A2DP state */
            linkPolicyPhonebookAccessComplete(PbapcGetSink(pbapGetActiveLink()));

            pbapSetBrowseEntryIndex(0);
            pbapSetBrowsingStartFlag(0);
            lIndicateEvent = FALSE ;

        }
        break;


#endif

#ifdef WBS_TEST
        /* TEST EVENTS for WBS testing */
        case EventUsrWbsTestSetCodecs:
            if(sinkHfpDataIsRenegotiateSco())
            {
                MAIN_DEBUG(("HS : AT+BAC = cvsd wbs\n")) ;
                sinkHfpDataSetRenegotiateSco(FALSE);
                HfpWbsSetSupportedCodecs((hfp_wbs_codec_mask_cvsd | hfp_wbs_codec_mask_msbc), FALSE);
            }
            else
            {
                MAIN_DEBUG(("HS : AT+BAC = cvsd only\n")) ;
                sinkHfpDataSetRenegotiateSco(TRUE);
                HfpWbsSetSupportedCodecs(hfp_wbs_codec_mask_cvsd , FALSE);
            }

        break;

        case EventUsrWbsTestSetCodecsSendBAC:
            if(sinkHfpDataIsRenegotiateSco())
            {
                MAIN_DEBUG(("HS : AT+BAC = cvsd wbs\n")) ;
                sinkHfpDataSetRenegotiateSco(FALSE);
                HfpWbsSetSupportedCodecs((hfp_wbs_codec_mask_cvsd | hfp_wbs_codec_mask_msbc), TRUE);
            }
           else
           {
               MAIN_DEBUG(("HS : AT+BAC = cvsd only\n")) ;
               sinkHfpDataSetRenegotiateSco(TRUE);
               HfpWbsSetSupportedCodecs(hfp_wbs_codec_mask_cvsd , TRUE);
           }
           break;

         case EventUsrWbsTestOverrideResponse:

           if(sinkHfpDataIsFailAudioNegotiation())
           {
               MAIN_DEBUG(("HS : Fail Neg = off\n")) ;
               sinkHfpDataSetFailAudioNegotiation(FALSE);
           }
           else
           {
               MAIN_DEBUG(("HS : Fail Neg = on\n")) ;
               sinkHfpDataSetFailAudioNegotiation(TRUE);
           }
       break;

#endif

       case EventUsrCreateAudioConnection:
           MAIN_DEBUG(("HS : Create Audio Connection\n")) ;

           CreateAudioConnection();
       break;

#ifdef ENABLE_MAPC
        case EventSysMapcMsgNotification:
            /* Generate a tone or audio prompt */
            MAIN_DEBUG(("HS : EventSysMapcMsgNotification\n")) ;
        break;
        case EventSysMapcMnsSuccess:
            /* Generate a tone to indicate the mns service success */
            MAIN_DEBUG(("HS : EventSysMapcMnsSuccess\n")) ;
        break;
        case EventSysMapcMnsFailed:
            /* Generate a tone to indicate the mns service failed */
            MAIN_DEBUG(("HS : EventSysMapcMnsFailed\n")) ;
        break;
#endif

       case EventUsrIntelligentPowerManagementOn:
           MAIN_DEBUG(("HS : Enable LBIPM\n")) ;
            /* enable LBIPM operation */
           powerManagerEnableLbipm(TRUE);
           /* send plugin current power level */
           AudioSetPower(powerManagerGetLBIPM());
       break;

       case EventUsrIntelligentPowerManagementOff:
           MAIN_DEBUG(("HS : Disable LBIPM\n")) ;
            /* disable LBIPM operation */
            powerManagerEnableLbipm(FALSE);
           /* notify the plugin Low power mode is no longer required */
           AudioSetPower(powerManagerGetLBIPM());
       break;

       case EventUsrIntelligentPowerManagementToggle:
           MAIN_DEBUG(("HS : Toggle LBIPM\n")) ;
           if(powerMangerIsLbipmEnabled())
           {
               MessageSend( &theSink.task , EventUsrIntelligentPowerManagementOff , 0 ) ;
           }
           else
           {
               MessageSend( &theSink.task , EventUsrIntelligentPowerManagementOn , 0 ) ;
           }

       break;

        case EventUsrUsbPlayPause:
        case EventUsrUsbStop:
        case EventUsrUsbFwd:
        case EventUsrUsbBack:
        case EventUsrUsbMute:
        case EventUsrUsbLowPowerMode:
        case EventSysUsbDeadBatteryTimeout:
        case EventSysAllowUSBVolEvents:
            lIndicateEvent = sinkUsbProcessEventUsb(id);
            break;

        case EventUsrAnalogAudioConnected:
        case EventUsrSpdifAudioConnected:
            processEventUsrWiredAudioConnected(id);
        break;
        
        case EventSysAnalogueAudioDisconnectTimeout:
        case EventUsrAnalogAudioDisconnected:
        case EventUsrSpdifAudioDisconnected:
            processEventWiredAudioDisconnected(id);
        break;

        case EventUsrEnterBootMode2:
            MAIN_DEBUG(("Reboot into different bootmode [2]\n")) ;
            BootSetMode(BOOTMODE_CUSTOM) ;
            break ;

        case EventUsrAvrcpPlayPause:
        case EventUsrAvrcpPlay:
        case EventUsrAvrcpPause:
        case EventUsrAvrcpStop:
        case EventUsrAvrcpSkipForward:
        case EventUsrAvrcpSkipBackward:
        case EventUsrAvrcpFastForwardPress:
        case EventUsrAvrcpFastForwardRelease:
        case EventUsrAvrcpRewindPress:
        case EventUsrAvrcpRewindRelease:
        case EventUsrAvrcpToggleActive:
        case EventUsrAvrcpNextGroupPress:
        case EventUsrAvrcpNextGroupRelease:
        case EventUsrAvrcpPreviousGroupPress:
        case EventUsrAvrcpPreviousGroupRelease:
        case EventUsrAvrcpShuffleOff:
        case EventUsrAvrcpShuffleAllTrack:
        case EventUsrAvrcpShuffleGroup:
        case EventUsrAvrcpRepeatOff:
        case EventUsrAvrcpRepeatSingleTrack:
        case EventUsrAvrcpRepeatAllTrack:
        case EventUsrAvrcpRepeatGroup:
        case EventSysSetActiveAvrcpConnection:
        case EventSysResetAvrcpMode:
            MAIN_DEBUG(("AVRCP Event %x\n", id ));
#ifdef ENABLE_BISTO
            if (!sinkVaEventHandler( id ))
                lIndicateEvent = sinkCommonProcessEvent(id, message);
#else
            lIndicateEvent = sinkAvrcpProcessEventAvrcp(id, message);
#endif
            break;

        case EventUsrTwsQualificationVolUp:
            MAIN_DEBUG(( "TWS Qualification Volume Up\n" ));
            handleAvrcpQualificationVolumeUp();
        break;

        case EventUsrTwsQualificationVolDown:
            MAIN_DEBUG(( "TWS Qualification Volume Down\n" ));
            handleAvrcpQualificationVolumeDown();
        break;

        case EventUsrTwsQualificationSetAbsVolume:
            MAIN_DEBUG(( "TWS SetAbsoluteVolume\n" ));
            handleAvrcpQualificationSetAbsoluteVolume();
        break;

        case EventUsrTwsQualificationPlayTrack:
            MAIN_DEBUG(( "TWS Qualification Play Track\n" ));
            handleAvrcpQualificationPlayTrack();
        break;

        case EventUsrTwsQualificationAVRCPConfigureDataSize:
            MAIN_DEBUG(("TWS Qualification AVRCP Configure Data Size\n" ));
            handleAvrcpQualificationConfigureDataSize();
        break;

        case EventUsrQualificationSecondAvctpConnectReq:
            MAIN_DEBUG(("AVCTP:Qualification Send Second Request\n" ));
            sinkSendAvctpSecondConnect();
            break;

#ifdef ENABLE_PEER
        case EventUsrSrcQualificationSendReconfigurationReq:
            MAIN_DEBUG(("TWS: SRC Send Reconfiguration Request\n" ));
            a2dpIssuePeerReconfigureRequest();
        break;
#endif

        case EventUsrSinkQualificationSendReconfigurationReq:
            MAIN_DEBUG(("HS: Sink Send Reconfiguration Request\n" ));
            a2dpIssueSinkReconfigureRequest();
        break;

        case EventUsrSinkQualificationSendSuspendReq:
            MAIN_DEBUG(("HS: Sink Send Suspend Request\n" ));

            if(SinkA2dpIsInitialised())
            {
                if(!getA2dpStatusFlag(QUAL_DISABLE_STREAM_RESUME, INVALID_ID))
                {
                    setA2dpStatusFlag(QUAL_DISABLE_STREAM_RESUME, INVALID_ID, TRUE);
                    a2dpIssueSuspendRequest();
                }
                else
                {
                    /* Reset the stream suspend flag. */
                    setA2dpStatusFlag(QUAL_DISABLE_STREAM_RESUME, INVALID_ID, FALSE);
                }
            }
            break;

       case EventUsrButtonLockingToggle:
            MAIN_DEBUG(("HS : EventUsrButtonLockingToggle (%d)\n", BMGetButtonsLocked()));
            if (BMGetButtonsLocked())
            {
                MessageSend( &theSink.task , EventUsrButtonLockingOff , 0 ) ;
            }
            else
            {
                MessageSend( &theSink.task , EventUsrButtonLockingOn , 0 ) ;
            }
        break;

        case EventUsrButtonLockingOn:
            MAIN_DEBUG(("HS : EventUsrButtonLockingOn\n"));
            BMSetButtonsLocked(TRUE);
        break;

        case EventUsrButtonLockingOff:
            MAIN_DEBUG(("HS : EventUsrButtonLockingOff\n"));
            BMSetButtonsLocked(FALSE);
        break;

        case EventUsrAudioPromptsOff:
            MAIN_DEBUG(("HS : EventUsrAudioPromptsOff"));
            /* disable audio prompts */

            /* Play the disable audio prompts prompt before actually disabling them */
            if(SinkAudioPromptsIsEnabled() == TRUE) /* Check if audio prompts are already enabled */
            {
                sinkAudioIndicationPlayEvent(id);
            }

            SinkAudioPromptsSetEnabled(FALSE);
        break;

        case EventUsrAudioPromptsOn:
            MAIN_DEBUG(("HS : EventUsrAudioPromptsOn"));
            /* enable audio prompts */
            SinkAudioPromptsSetEnabled(TRUE);
        break;
#ifdef ENABLE_SPEECH_RECOGNITION
        case EventSysSpeechRecognitionStart:
        {

            if ( speechRecognitionIsEnabled() )
                speechRecognitionStart() ;
            else
                lIndicateEvent = FALSE;
        }
        break ;
        case EventSysSpeechRecognitionStop:
        {
            if(speechRecognitionIsEnabled() )
                speechRecognitionStop() ;
            else
                lIndicateEvent = FALSE;
        }
        break ;
        /* to tune the Speech Recognition using the UFE generate this event */
        case EventUsrSpeechRecognitionTuningStart:
        {
            /* ensure speech recognition is enabled */
            if ( speechRecognitionIsEnabled() )
            {
                /* ensure not already in tuning mode */
                if(!speechRecognitionTuningIsActive())
                {
                    speechRecognitionSetTuningActive(TRUE);
                    speechRecognitionStart() ;
                }
            }
        }
        break;

        case EventSysSpeechRecognitionTuningYes:
        break;

        case EventSysSpeechRecognitionTuningNo:
        break;
        case EventSysSpeechRecognitionFailed:
        break;
#endif

        case EventUsrTestDefrag:
            MAIN_DEBUG(("HS : EventUsrTestDefrag\n"));
            configManagerFillPs();
        break;

        case EventSysStreamEstablish:
            MAIN_DEBUG(("HS : EventSysStreamEstablish[%u]\n", ((const EVENT_STREAM_ESTABLISH_T *)message)->a2dp_index));
            connectA2dpStream( ((const EVENT_STREAM_ESTABLISH_T *)message)->a2dp_index, 0 );
        break;

        case EventSysA2dpConnected:
            MAIN_DEBUG(("HS : EventSysA2dpConnected\n"));
        break;

        case EventSysA2dpDisconnected:
            MAIN_DEBUG(("HS : EventSysA2dpDisconnected\n"));
        break;

        case EventSysUpdateAttributes:
            deviceManagerDelayedUpdateAttributes((const EVENT_UPDATE_ATTRIBUTES_T*)message);
        break;

        case EventUsrPeerSessionConnDisc:
            MAIN_DEBUG(("HS: PeerSessionConnDisc [%d]\n" , lState )) ;
            /*go into pairing mode*/
            if ( (lState != deviceLimbo) && (deviceManagerNumConnectedPeerDevs() == 0) )
            {
                /* ensure there is only one device connected to allow peer dev to connect */
                if(deviceManagerNumConnectedDevs() < MAX_A2DP_CONNECTIONS)
                {
#ifdef ENABLE_PEER
                    peerPauseIfStreaming();
#endif
                    sinkInquirySetInquirySession(inquiry_session_peer);
                    stateManagerEnterConnDiscoverableState( FALSE );
                }
                /* no free connections, indicate an error condition */
                else
                {
                    /* don't indicate event this time round, disconnect device first
                       and re-generate event */
                    lIndicateEvent = FALSE;
                    disconnectAgResendOrError( EventUsrPeerSessionConnDisc );
                }
            }
            else
            {
                lIndicateEvent = FALSE ;
            }
        break ;

        case ( EventUsrPeerSessionInquire ):
            MAIN_DEBUG(("HS: PeerSessionInquire\n"));
            /* inquire for a peer device if not already connected */
            if ( (lState != deviceLimbo) && (deviceManagerNumConnectedPeerDevs() == 0) )
            {
                /* ensure there is only one device connected to allow peer dev to connect */
                if ( deviceManagerNumConnectedDevs() < MAX_A2DP_CONNECTIONS )
                {
#ifdef ENABLE_PEER
                    peerPauseIfStreaming();
#endif
                    lIndicateEvent = inquiryPair( inquiry_session_peer, FALSE );
                    /* Disable Discoverable mode only during peer inquire */
                    sinkDisableDiscoverable();
                }
                /* no free connections, indicate an error condition */
                else
            	{
                	/* don't indicate event this time round, disconnect device first
                   	   and re-generate event */
                    lIndicateEvent = FALSE;
                    disconnectAgResendOrError( EventUsrPeerSessionInquire );
                }
            }
            /* in limbo mode or already connected */
            else
            {
                lIndicateEvent = FALSE ;
            }
        break;

#ifdef ENABLE_PEER
        case EventUsrPeerSessionPair:
            MAIN_DEBUG(("HS: EventUsrPeerSessionPair [%d]\n" , lState )) ;
            /* go into pairing mode if not in limbo and not already connected */
            if ( (lState != deviceLimbo) && (deviceManagerNumConnectedPeerDevs() == 0) )
            {
                /* ensure there is only one device connected to allow peer dev to connect */
                if(deviceManagerNumConnectedDevs() < MAX_A2DP_CONNECTIONS)
                {
                    uint16 index;
                    uint16 srcIndex;
                    uint16 avrcpIndex;
                    /* check whether the a2dp connection is present and streaming data and that the audio is routed,
                       if so cancel any fast forward/rewind actions and do not inquire for any slave
                       devices since this will disrupt the A2DP audio stream */
                    if(sinkAudioIsAudioRouted() && getA2dpIndexFromSink(sinkAudioGetRoutedAudioSink(), &index)
                        && (A2dpMediaGetState(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index)) == a2dp_stream_streaming)
                        && a2dpGetSourceIndex(&srcIndex) && (srcIndex == index)
                        && sinkAvrcpGetIndexFromBdaddr(getA2dpLinkBdAddr(index), &avrcpIndex, TRUE))
                    {
                        /* cancel any queued ff or rw requests and then pause the streaming*/
                        MessageCancelAll (&theSink.task, EventUsrAvrcpFastForwardPress);
                        MessageCancelAll (&theSink.task, EventUsrAvrcpRewindPress);
                    }
                    /* if not currently receiving an A2DP stream from an AG,
                       attempt to discover a peer device, this will protect the
                       audio quality of the A2DP link */
                    else
                    {
                        lIndicateEvent = inquiryPair( inquiry_session_peer, FALSE );
                    }
                    /* go into discoverable mode */
                    sinkInquirySetInquirySession(inquiry_session_peer);
                    stateManagerEnterConnDiscoverableState( FALSE );
                }
                /* no free connections, indicate an error condition */
                else
                {
                    /* don't indicate event this time round, disconnect device first
                       and re-generate event */
                    lIndicateEvent = FALSE;
                    disconnectAgResendOrError( EventUsrPeerSessionPair );
                }
            }
            /* in limbo or already connected */
            else
            {
                lIndicateEvent = FALSE ;
            }
        break;
#endif /* def ENABLE_PEER */

        case EventSysInquiryDelayedStart:
            {
                INQ_INQUIRY_PAIR_MSG_T *m = (INQ_INQUIRY_PAIR_MSG_T *) message;
                lIndicateEvent = inquiryPair(m->session, m->req_disc);
            }
            break;
            
        case EventUsrPeerSessionEnd:
        {
#ifdef PEER_SCATTERNET_DEBUG   /* Scatternet debugging only */
            uint16 i;
            for_all_a2dp(i)
            {
                if (SinkA2dpIsInitialised())
                {
                    sinkA2dpInvertAgRole(i);
                    MAIN_DEBUG(("HS: invert_ag_role[%u] = %u\n",i, sinkA2dpIsAgRoleToBeInverted(i)));

                    if (getA2dpStatusFlag(CONNECTED, i) && (getA2dpPeerRemoteDevice(i) != remote_device_peer))
                    {
                        linkPolicyUseA2dpSettings(getA2dpLinkDataDeviceId(i),
                                                   getA2dpLinkDataStreamId(i),
                                                   A2dpSignallingGetSink(getA2dpLinkDataDeviceId(i));
                    }
                }
            }
#else   /* Normal operation */
            MAIN_DEBUG(("HS: EventUsrPeerSessionEnd\n"));
            lIndicateEvent = disconnectAllA2dpPeerDevices();
#endif
        }
        break;

        case EventUsrPeerReserveLinkOn:
#ifdef ENABLE_PEER
            peerSetLinkReserved(TRUE);
#endif
        break;

        case EventUsrPeerReserveLinkOff:
#ifdef ENABLE_PEER
            peerSetLinkReserved(FALSE);
#endif
        break;

        case EventUsrSwapA2dpMediaChannel:
            /* attempt to swap media channels, don't indicate event if not successful */
            if(!audioSwapMediaChannel())
                lIndicateEvent = FALSE;
        break;


        case EventUsrBassEnhanceEnableDisableToggle:
        case EventUsrBassEnhanceOn:
        case EventUsrBassEnhanceOff:
        case EventUsr3DEnhancementEnableDisableToggle:
        case EventUsr3DEnhancementOn:
        case EventUsr3DEnhancementOff:
        case EventUsrCompanderOn:
        case EventUsrCompanderOff:
        case EventUsrCompanderOnOffToggle:
        case EventUsrUserEqOnOffToggle:
        case EventUsrUserEqOn:
        case EventUsrUserEqOff:
        case EventSysUpdateUserEq:
        case EventSysUpdateAudioEnhancements:
        case EventSysPeerGeneratedUpdateUserEq:
        case EventSysPeerGeneratedUpdateAudioEnhancements:
        case EventUsrSelectNextUserEQBank:
            lIndicateEvent = sinkAudioProcessEvent(id);
            break;

        /* check whether the Audio Amplifier drive can be turned off after playing
           a tone or voice prompt */
        case EventSysCheckAudioAmpDrive:
            /* cancel any pending messages */
            MessageCancelAll( &theSink.task , EventSysCheckAudioAmpDrive);
            /* when the device is no longer routing audio to the speaker then
               turn off the audio amplifier */
            if((!sinkAudioIsAudioRouted()) && (!sinkAudioIsVoiceRouted()))
            {
                MAIN_DEBUG (( "HS : EventSysCheckAudioAmpDrive turn off amp\n" ));
                PioDrivePio(PIO_AUDIO_ACTIVE, FALSE);
            }
            else
                lIndicateEvent = FALSE;
        break;

        /* external microphone has been connected */
        case EventUsrExternalMicConnected:
            setExternalMic(EXTERNAL_MIC_FITTED);
            /* if routing audio update the mic source for dsp apps that support it */
            if(sinkAudioIsAudioRouted())
                AudioSetMode(AUDIO_MODE_CONNECTED, getAudioPluginModeParams());
        break;

        /* external microphone has been disconnected */
        case EventUsrExternalMicDisconnected:
            setExternalMic(EXTERNAL_MIC_NOT_FITTED);
            /* if routing audio update the mic source for dsp apps that support it */
            if(sinkAudioIsAudioRouted())
                AudioSetMode(AUDIO_MODE_CONNECTED, getAudioPluginModeParams());
       break;

       /* event to enable the simple speech recognition functionality, persistant over power cycle */
       case EventUsrSSROn:
        #ifdef ENABLE_SPEECH_RECOGNITION
                speechRecognitionSetSSREnabled(TRUE);
        #endif
       break;

       /* event to disable the simple speech recognition functionality, persistant over power cycle */
       case EventUsrSSROff:
        #ifdef ENABLE_SPEECH_RECOGNITION
            speechRecognitionSetSSREnabled(FALSE);
        #endif
       break;

       /* NFC tag detected, determine action based on current connection state */
       case EventUsrNFCTagDetected:
            handleUsrNFCTagDetected(task, message);
       break;

       /* check whether the current routed audio is still the correct one and
          change sources if appropriate */
       case EventSysCheckAudioRouting:
            /* check audio routing */
    	    audioRoutingProcessUpdateMsg();
            /* don't indicate event as may be generated by USB prior to configuration
               being loaded */
            lIndicateEvent = FALSE;
       break;



        /* Audio amplifier is to be shut down by PIO for power saving purposes */
       case EventSysAmpPowerDown:
            MAIN_DEBUG (( "HS : EventSysAmpPowerDown\n"));
            stateManagerAmpPowerControl(POWER_DOWN);
       break;

       case EventSysAmpPowerUp:
            MAIN_DEBUG (( "HS : EventSysAmpPowerUp\n"));
            stateManagerAmpPowerControl(POWER_UP);
            break;

       case EventUsrFmRxOn:
       case EventUsrFmRxOff:
       case EventUsrFmRxTuneUp:
       case EventUsrFmRxTuneDown:
       case EventUsrFmRxStore:
       case EventUsrFmRxTuneToStore:
       case EventUsrFmRxErase:
           lIndicateEvent = sinkFmProcessEventUsrFmRx(id);
           break;

       case EventUsrSelectAudioSourceAnalog:
       case EventUsrSelectAudioSourceSpdif:
       case EventUsrSelectAudioSourceI2S:
       case EventUsrSelectAudioSourceUSB:
       case EventUsrSelectAudioSourceA2DP1:
       case EventUsrSelectAudioSourceA2DP2:
       case EventUsrSelectAudioSourceFM:
       case EventUsrSelectAudioSourceNone:
       case EventUsrSelectAudioSourceNext:
       case EventUsrSelectAudioSourceNextRoutable:
           lIndicateEvent = processEventUsrSelectAudioSource(id);
       break;



#ifdef ENABLE_SUBWOOFER
       case EventUsrSubwooferStartInquiry:
            handleEventUsrSubwooferStartInquiry();
       break;

       case EventSysSubwooferCheckPairing:
            handleEventSysSubwooferCheckPairing();
       break;

       case EventSysSubwooferOpenLLMedia:
            /* open a Low Latency media connection */
            handleEventSysSubwooferOpenLLMedia();
       break;

       case EventSysSubwooferOpenStdMedia:
            /* open a standard latency media connection */
            handleEventSysSubwooferOpenStdMedia();
       break;

       case EventUsrSubwooferVolumeUp:
            handleEventUsrSubwooferVolumeUp();
       break;

       case EventUsrSubwooferVolumeDown:
            handleEventUsrSubwooferVolumeDown();
       break;

       case EventSysSubwooferCloseMedia:
            handleEventSysSubwooferCloseMedia();
       break;

       case EventSysSubwooferStartStreaming:
            handleEventSysSubwooferStartStreaming();
       break;

       case EventSysSubwooferSuspendStreaming:
            handleEventSysSubwooferSuspendStreaming();
       break;

       case EventUsrSubwooferDisconnect:
            handleEventUsrSubwooferDisconnect();
       break;

       case EventUsrSubwooferDeletePairing:
            handleEventUsrSubwooferDeletePairing();
       break;

       /* set subwoofer volume level by message to maintain synchronisation with
          audio plugins */
       case EventSysSubwooferSetVolume:
            /* send volume level change to subwoofer */
            updateSwatVolumeNow(((SWAT_VOLUME_CHANGE_MSG_T*)message)->new_volume);
       break;
#endif
#ifdef ENABLE_PARTYMODE
       /* enabling the runtime control of the party mode feature */
       case EventUsrPartyModeOn:
       {
            /* ensure a party mode operating type has been selected in configuration
               before enabling the feature */
            if(sinkPartymodeGetOperatingMode())
            {
               /* turn party mode on */
               sinkPartymodeTurnOn();

                /* if HFP channels are connected, disconnect them, it's 
                * more code efficient to just call to disconnect both 
                * links and let it fail if they're not already connected 
                */

                HfpSlcDisconnectRequest(hfp_primary_link);
                HfpSlcDisconnectRequest(hfp_secondary_link);

                /* Disable the service record to avoid HF re-connect */
                hfpHideServiceRecords();

               /* if more than one device connected, drop the second device */
               if( deviceManagerNumConnectedDevs() > 1 )
               {
                   /* but allow a second one to be connected */
                   sinkPartyModeDisconnectDevice(a2dp_secondary);
               }
               else
               {
                   /* ensure headset is discoverable
                      and connectable once enabled */
                   sinkEnableConnectable();
               }
               /* ensure pairable */
               sinkEnableDiscoverable();
           }
       }
       break;

       /* disabling the runtime control of the party mode feature */
       case EventUsrPartyModeOff:
       {
           /* no need to ensure a party mode operating type has been selected
              in configuration before disabling the feature */
           if(sinkPartymodeIsEnabled())
           {
               /* turn party mode off */
              sinkPartymodeTurnOff();

                /* Shutting down all connections as it's impossible to re-connect
                * after party mode ... because the connection and pairing tables
                * are not accurately maintained after so many connections and
                * pairings while in Party Mode. */

                sinkDisconnectAllSlc();
                disconnectAllA2dpAvrcp(TRUE);

                /* Re-enable the service records for HF re-connect */
                hfpShowServiceRecords();

               /* ensure pairable */
               sinkEnableDiscoverable();
           }
       }
       break;

       /* connected device hasn't played music before timeout, disconnect it to allow
          another device to connect */
       case EventSysPartyModeTimeoutDevice1:
            sinkPartyModeDisconnectDevice(a2dp_primary);
       break;

       /* connected device hasn't played music before timeout, disconnect it to allow
          another device to connect */
       case EventSysPartyModeTimeoutDevice2:
            sinkPartyModeDisconnectDevice(a2dp_secondary);
       break;

       case EventSysPartyModeConnected:
            MAIN_DEBUG(("EventSysPartyModeConnected\n"));
       break;

       case EventSysPartyModeDisconnected:
            MAIN_DEBUG(("EventSysPartyModeDisconnected\n"));
       break;
#endif

#ifdef ENABLE_GAIA
        case EventUsrGaiaDFURequest:
        /*  GAIA DFU requires that audio not be busy, so disallow any tone  */
            lIndicateEvent = FALSE;
            gaiaDfuRequest();
        break;
#endif

        case EventSysRemoteControlCodeReceived:
             /* Display a led pattern*/
        break;

#ifdef ENABLE_IR_REMOTE
        case EventSysIRCodeLearnSuccess:
        {
            /* TODO : Play a tone or something */
        }
        break;
        case EventSysIRCodeLearnFail:
        {
            /* TODO : Play a tone or something */
        }
        break;
        case EventSysIRLearningModeTimeout:
        {
            irStopLearningMode();
        }
        break;
        case EventSysIRLearningModeReminder:
        {
            handleIrLearningModeReminder();
        }
        break;
        case EventUsrStartIRLearningMode:
        {
            irStartLearningMode();
        }
        break;
        case EventUsrStopIRLearningMode:
        {
            irStopLearningMode();
        }
        break;
        case EventUsrClearIRCodes:
        {
            irClearLearntCodes();
        }
        break;
#endif

        case EventUsrMasterDeviceTrimVolumeUp:
        case EventUsrMasterDeviceTrimVolumeDown:
        case EventUsrSlaveDeviceTrimVolumeUp:
        case EventUsrSlaveDeviceTrimVolumeDown:
        case EventUsrChangeAudioRouting:
        case EventUsrDetermineTwsForcedDownmixMode:
            lIndicateEvent = peerProcessEvent(id);
            break;

        case EventSysCheckDefrag:
        case EventSysDefrag:
            configManagerProcessEventSysDefrag(id);
            break;

        case EventSysToneDigit0:
        case EventSysToneDigit1:
        case EventSysToneDigit2:
        case EventSysToneDigit3:
        case EventSysToneDigit4:
        case EventSysToneDigit5:
        case EventSysToneDigit6:
        case EventSysToneDigit7:
        case EventSysToneDigit8:
        case EventSysToneDigit9:
            break;

        /* event to start an LED state indication, called by
           message to reduce maximum stack usage */
        case EventSysLEDIndicateState:
            {
               /* use LEDS to indicate current state which is retrieved inside the function and set beforehand */
               LedManagerIndicateExtendedState() ;
            }
            break;

        case EventUsrBleStartBonding:
            {
                MAIN_DEBUG(("HS : BLE Bondable\n"));

                sinkSportHealthSMHandleBleStartBonding();
                sinkBleBondableEvent();
            }
            break;

        case EventUsrBleDeleteDevice:
            {
                MAIN_DEBUG(( "HS : Delete Marked LE device \n" ));
                sinkBleDeleteMarkedDevices();
            }
            break;

        case EventSysBleConnectionPairingTimeout:
            {
                MAIN_DEBUG(("HS : BLE Connection Pairing Timeout\n"));
                sinkBleConnectionPairingTimeoutEvent();
            }
            break;

        case EventSysBleBondableTimeout:
            {
                MAIN_DEBUG(("HS : BLE Bondable Timeout\n"));
                sinkBleBondableTimeoutEvent();

                sinkSportHealthSMDeviceStateCheck(EventSysBleBondableTimeout);
            }
            break;
         
       case EventSysHeartRateThresholdReached:
            {
                MAIN_DEBUG(( "User heart rate threshold reached\n" ));
            }
            break;
            
       case EventUsrFindMyRemoteImmAlertMild:
            {          
                MAIN_DEBUG(( "Find my remote Imm Alert Mild\n" ));
                sinkGattIasClientSetAlert(gatt_imm_alert_level_mild, sink_gatt_ias_alert_remote);
            }
            break;

        case EventUsrFindMyRemoteImmAlertHigh:
            {
                MAIN_DEBUG(( "Find my remote Imm Alert High\n" ));
                sinkGattIasClientSetAlert(gatt_imm_alert_level_high, sink_gatt_ias_alert_remote);
            }
            break;

       case EventUsrFindMyPhoneImmAlertMild:
            {
                MAIN_DEBUG(( "Find my phone Imm Alert Mild\n" ));
                sinkGattIasClientSetAlert(gatt_imm_alert_level_mild, sink_gatt_ias_alert_phone);
            }
            break;

        case EventUsrFindMyPhoneImmAlertHigh:
            {
                MAIN_DEBUG(( "Find my phone Imm Alert High \n" ));
                sinkGattIasClientSetAlert(gatt_imm_alert_level_high, sink_gatt_ias_alert_phone);
            }
            break;

        /*Alert the phone from remote control using HID. This event needs to be mapped to the HID code for alert*/
        case EventUsrFindMyPhoneRemoteImmAlertHigh:
            {
                MAIN_DEBUG(( "Find my phone Imm Alert High - Triggered from HID Remote\n" ));
                sinkGattImmAlertLocalAlert(gatt_imm_alert_level_high);
                sinkGattIasClientSetAlert(gatt_imm_alert_level_high, sink_gatt_ias_alert_phone);
            }
            break;
            
        case EventUsrImmAlertStop:
        case EventSysImmAlertTimeout:
            {
                MAIN_DEBUG(( "IAS : Stop Alert/Timeout\n" ));

                /*Local Alert*/
                sinkGattServerImmAlertStopAlert();

                /*Remote Alert*/
                if (sinkGattIasClientEnabled())
                {
                    sinkGattIasClientSetAlert(gatt_imm_alert_level_no, sink_gatt_ias_alert_none);
                }
            }
            break;

        case EventSysImmAlertMild:
            {
                MAIN_DEBUG(( "HS : Mild Alert \n" ));
                sinkGattServerImmAlertMild(sinkGattIasServerGetImmediateAlertTimer_s());
            }
            break;

        case EventSysImmAlertHigh:
            {
                MAIN_DEBUG(( "HS : High Alert \n" ));
                sinkGattServerImmAlertHigh(sinkGattIasServerGetImmediateAlertTimer_s());
            }
        break;
        case EventSysImmAlertErrorBeep:
            MAIN_DEBUG(( "HS : Recieved Immediate Alert Error Beep\n" ));
        break;

        case EventSysBleHrSensorInContact:
            {
                MAIN_DEBUG(("HS : HR Sensor in contact, start sending Heart Rate measurements \n"));
                sinkBleHRSensorInContact();
            }
            break;

        case EventSysBleHrSensorNotInContact:
            {
                MAIN_DEBUG(("HS : HR sensor not in contact, Stop sending Heart Rate measurements \n"));
                sinkBleHRSensorNotInContact();
            }
            break;

        case EventSysLlsAlertTimeout:
        case EventUsrLlsAlertStop:
        {
            MAIN_DEBUG(( "HS : Link Loss Stop Alert \n" ));
            sinkGattLinkLossAlertStop();          
        }
        break;
        case EventSysLlsAlertMild:
        {
            MAIN_DEBUG(( "HS : Link Loss Mild Alert \n" ));
            sinkGattLinkLossAlertMild(sinkGattGetLinkLossTimer_s());
        }
        break;
        case EventSysLlsAlertHigh:
        {
            MAIN_DEBUG(( "HS : Link Loss High Alert \n" ));
            sinkGattLinkLossAlertHigh(sinkGattGetLinkLossTimer_s());
        }
        break;

        case EventSysAncsOtherAlert:
            MAIN_DEBUG(( "HS : Recieved ANCS Other Notifications Alert\n" ));
        break;

        case EventSysAncsIncomingCallAlert:
            MAIN_DEBUG(( "HS : Recieved ANCS Incoming Call Alert\n" ));
        break;

        case EventSysAncsMissedCallAlert:
            MAIN_DEBUG(( "HS : Recieved ANCS Missed Call Alert\n" ));
        break;

        case EventSysAncsVoiceMailAlert:
            MAIN_DEBUG(( "HS : Recieved ANCS Voice Mail Alert\n" ));
        break;

        case EventSysAncsSocialAlert:
            MAIN_DEBUG(( "HS : Recieved ANCS Social Alert\n" ));
        break;

        case EventSysAncsScheduleAlert:
            MAIN_DEBUG(( "HS : Recieved ANCS Schedule Alert\n" ));
        break;

        case EventSysAncsEmailAlert:
            MAIN_DEBUG(( "HS : Recieved ANCS Email Alert\n" ));
        break;

        case EventSysAncsNewsAlert:
            MAIN_DEBUG(( "HS : Recieved ANCS News Alert\n" ));
        break;

        case EventSysAncsHealthNFittnessAlert:
            MAIN_DEBUG(( "HS : Recieved ANCS Health N Fittness Alert\n" ));
        break;

        case EventSysAncsBusinessNFinanceAlert:
            MAIN_DEBUG(( "HS : Recieved ANCS Business N Finance Alert\n" ));
        break;

        case EventSysAncsLocationAlert:
            MAIN_DEBUG(( "HS : Recieved ANCS Location Alert\n" ));
        break;

        case EventSysAncsEntertainmentAlert:
            MAIN_DEBUG(( "HS : Recieved ANCS Entertainmen Alert\n" ));
        break;

        case EventUsrBleSimulateHrNotifications:
            MAIN_DEBUG(( "HS : BLE notify simulated Heart rate measurements\n" ));
            MessageSend(&theSink.task, EventSysBleHrSensorInContact, 0);
        break;

        case EventUsrBleHidExtraConfig:
            MAIN_DEBUG(( "HS : BLE HID Extra configuration for qualification\n" ));
            sinkGattHIDExtraConfig();
        break;
        

        case EventSysReboot:
            BootSetMode(BootGetMode());
            break;

        case EventUsrAncOn:
        case EventUsrAncOff:
        case EventUsrAncMode1:
        case EventUsrAncMode2:
        case EventUsrAncMode3:
        case EventUsrAncMode4:
        case EventUsrAncMode5:
        case EventUsrAncMode6:
        case EventUsrAncMode7:
        case EventUsrAncMode8:
        case EventUsrAncMode9:
        case EventUsrAncMode10:
        case EventUsrAncNextMode:
        case EventUsrAncVolumeDown:
        case EventUsrAncVolumeUp:
        case EventUsrAncCycleGain:
        case EventUsrAncToggleOnOff:
        case EventSysAncDisabled:
        case EventSysAncMode1Enabled:
        case EventSysAncMode2Enabled:
        case EventSysAncMode3Enabled:
        case EventSysAncMode4Enabled:
        case EventSysAncMode5Enabled:
        case EventSysAncMode6Enabled:
        case EventSysAncMode7Enabled:
        case EventSysAncMode8Enabled:
        case EventSysAncMode9Enabled:
        case EventSysAncMode10Enabled:
        case EventSysPeerGeneratedAncOn:
        case EventSysPeerGeneratedAncOff:
        case EventSysPeerGeneratedAncMode1:
        case EventSysPeerGeneratedAncMode2:
        case EventSysPeerGeneratedAncMode3:
        case EventSysPeerGeneratedAncMode4:
        case EventSysPeerGeneratedAncMode5:
        case EventSysPeerGeneratedAncMode6:
        case EventSysPeerGeneratedAncMode7:
        case EventSysPeerGeneratedAncMode8:
        case EventSysPeerGeneratedAncMode9:
        case EventSysPeerGeneratedAncMode10:
        case EventUsrEnterAncTuningMode:
            MAIN_DEBUG(("Sys/Usr Anc Event\n"));
            lIndicateEvent = sinkAncProcessEvent(id);
            break;

        case EventUsrStartA2DPStream:
            MAIN_DEBUG(( "A2DP Start streaming media\n" ));
            audioA2dpStartStream();
            break;

        case EventSysPromptsTonesQueueCheck:
            if(!IsAudioBusy())
            {
                MAIN_DEBUG(("Audio Free ,Fetch Event\n" ));
                id = sinkEventQueueFetch();
            }
            break;

        case EventSysVoiceRecognitionRequestTimeout:
            MAIN_DEBUG(("Voice recognition request timeout\n" ));
            break;

        case EventSysVoiceRecognitionDisabled:
            MAIN_DEBUG(("Voice recognition Disabled\n" ));
            break;

        case EventSysVoiceRecognitionActive:
            MAIN_DEBUG(("Voice recognition Active\n" ));
            break;

#ifdef ENABLE_PEER
        case EventUsrTwsQualificationEnablePeerOpen:
            MAIN_DEBUG(( "TWS Qualification Enable Opening of Peer Media Channel\n" ));
            handlePeerQualificationEnablePeerOpen();
            break;
        case EventSysA2DPPeerLinkLossTimeout:
            setA2dpPeerLinkLossReconnect(FALSE);
            MAIN_DEBUG(("EventSysA2DPPeerLinkLossTimeout\n"));
            break;
        case EventSysRemovePeerTempPairing:
            HandlePeerRemoveAuthDevice((const bdaddr*)message);
            break;
        case EventSysA2dpPauseSuspendTimeoutDevice1:
            /* Connected device hasn't suspended its a2dp media channel before timeout,try to force it. */
            a2dpSuspendNonRoutedStream(a2dp_primary);
            break;

        case EventSysA2dpPauseSuspendTimeoutDevice2:
            /* Connected device hasn't suspended its a2dp media channel before timeout, try to force it. */
            a2dpSuspendNonRoutedStream(a2dp_secondary);
            break;

        case EventSysTwsAudioRoutingChanged:
            MAIN_DEBUG(("HS : EventSysTwsAudioRoutingChanged\n"));
            break;
#endif /* def ENABLE_PEER */
            
        case EventUsrSensorUp:
        case EventUsrSensorDown:
            lIndicateEvent = sinkAccessoryReportUserEvent(id);
            break;
            
#ifdef ENABLE_LINKKEY_DISPLAY            
        case EventUsrDisplayConnectedLinkKey:
            MAIN_DEBUG(("EventUsrDisplayConnectedLinkKey\n"));
            if (sinkDataGetDisplayLinkKeys() == FALSE)
            {
            displayShowLinkKeyEnable();
            deviceManagerGetConnectedDeviceLinkKey();
            }
            else
            {
                displayShowLinkKeyDisable();
                displayHideLinkKey();
            }
            break;
#endif            

       /*these events have no required action directly associated with them  */
       /*they are received here so that LED patterns and Tones can be assigned*/
       case EventSysHfpSecureLink:
           MAIN_DEBUG(( "HFP link is Secure\n" ));
           break;
      case EventSysSCOSecureLinkOpen :
            MAIN_DEBUG(("EventSysSCOSecureLinkOpen\n")) ;
            break ;
      case EventSysSCOSecureLinkClose:
            MAIN_DEBUG(("EventSysSCOSecureLinkClose\n")) ;
            break;
      case EventUsrTestUserConfirmationYes:
            MAIN_DEBUG(("EventUsrTestUserConfirmationYes\n")) ;
            test_sinkHandleUserConfirmation(TRUE);
           break;
      case EventUsrTestUserConfirmationNo:
            MAIN_DEBUG(("EventUsrTestUserConfirmationNo\n")) ;
            test_sinkHandleUserConfirmation(FALSE);
           break;
      case EventSysLESecureLink:
            MAIN_DEBUG(("EventSysLESecureLink\n"));
            break;

      case EventSysMultipointConnected:
            MAIN_DEBUG(("EventSysMultipointConnected\n"));
            break;

      case EventSysMultipointDisconnected:
            MAIN_DEBUG(("EventSysMultipointDisconnected\n"));
            break;

      case EventSysCodecAptxInUse:
      case EventSysCodecAptxHDInUse:
      case EventSysCodecAptxLLInUse:
      case EventSysCodecAptxAdaptiveInUse:
            MAIN_DEBUG(("EventSysCodecAptxInUse\n"));
           break;
           
        case EventSysPairingReminder:
            if(stateManagerGetState() == deviceConnDiscoverable)
            {
                sinkCancelAndIfDelaySendLater(EventSysPairingReminder, D_SEC(sinkDataGetPairingReminderInterval()));
            }
            else
            {
                lIndicateEvent = FALSE;
            }
            break;
                 
        case EventSysEQOn:
            break;

        case EventSysEQOff:
            break;

        /* update the status of the current EQ enable status */
        case EventSysEQRefreshStatus:
            /* generate event to drive EQ indicator */
            sinkAudioEQOperatingStateIndication();
            break;
            
        case EventUsrPioClear:
        case EventUsrPioSet:
        case EventUsrPioToggle:
            PioHandleUserEvent(id);
            break;

        case EventUsrVATapToTalk:
            sinkCancelMessage(EventUsrVATapToTalk);
            SinkVaTriggerTaptoTalk();
            break;

        case EventUsrVACancel:
            sinkCancelMessage(EventUsrVACancel);
            SinkVaUsrCancelSession();
            break;

        case EventUsrVAPushToTalkStart:
            sinkCancelMessage(EventUsrVAPushToTalkStart);
            SinkVaTriggerPushToTalkStart();
            break;

		case EventUsrVAPushToTalkStop:
            sinkCancelMessage(EventUsrVAPushToTalkStop);
            SinkVaTriggerPushToTalkStop();
            break;

        case EventUsrVA1:
        case EventUsrVA2:
        case EventUsrVA3:
        case EventUsrVA4:
        case EventUsrVA5:
        case EventUsrVA6:
            lIndicateEvent = sinkVaEventHandler(id);
            break;

        case EventSysPioWriteSessionData:
            PioWriteSessionData();
            break;
            
        case EventSysVAStartVoiceCapture:
            MAIN_DEBUG(("HS: Received Vocie Assistant Start Voice Capture event\n"));
#ifndef ENABLE_BISTO
            /* No need to indicate, as playing of tone is already taken care in sink_va module */
            lIndicateEvent = FALSE;
#endif
            break;
            
        case EventSysVAStopVoiceCapture:
            break;

        case EventSysVASessionError:
            MAIN_DEBUG(("HS: Received Vocie Assistant Session Error\n"));
            break;

        case EventSysUpgradeOptimisation:
            {
                EVENT_UPGRADE_OPTIMISATION_T *m = (EVENT_UPGRADE_OPTIMISATION_T*) message;
                MAIN_DEBUG(("HS: Apply Upgrade Optimisation\n"));
                sinkGaiaProcessUpgradeOptimisation(m->apply);
            }
            break;

#ifdef ENABLE_PEER
        case EventSysPeerStreamTimeout:
            /* Peer didn't response early enough, starting streaming with source now */
            MAIN_DEBUG(("HS : EventSysPeerStreamTimeout: Relay channel NOT available now\n"));
            MAIN_DEBUG(("HS : Send start resp to AV Source\n"));
            A2dpMediaStartResponse(((A2DP_MEDIA_START_IND_T*)message)->device_id,
                                   ((A2DP_MEDIA_START_IND_T*)message)->stream_id, TRUE);

            /* Streaming protection is enabled if it wasn't already the case */
            if (!peerLinkRecoveryWhileStreamingEnabled())
            {
                linklossProtectStreaming(linkloss_stream_protection_on);
            }

            /* To be sure that the amp is not muted when the source is starting to stream */
            AudioSetInputAudioMute(FALSE);
            break ;
#endif /* ENABLE_PEER */

#ifdef ENABLE_ALLOW_USB_DEBUG
        case EventUsrEnableUsbDebug:
            UsbDebugEnableAllow();
            break;
#endif /* ENABLE_ALLOW_USB_DEBUG */
            
        default :
            MAIN_DEBUG_L1(( "HS : UE unhandled!! [%x]\n", id ));
        break ;
    }

    id = peerConvertPeerGeneratedEventToUsrEvent(id);

    if((TRUE ==lResetAutoSwitchOff) && (sinkDataGetAutoSwitchOffTimeout() !=0) && (id != EventUsrPowerOff))
    {
         sinkStartAutoPowerOffTimer();
    }

        /* Inform the event indications that we have received a user event*/
        /* For all events except the end event notification as this will just end up here again calling itself...*/
    if ( lIndicateEvent && (!eventToBeIndicatedBeforeProcessing(id)))
    {
        IndicateEvent(id);
    }

    /* Don't echo EventInvalid to a host because it is a fake event id returned
       by sinkEventQueueFetch() when the queue is empty. 
       It has no external meaning to the host. */
    if (EventInvalid != id)
    {
#ifdef ENABLE_GAIA
        gaiaReportEvent(id);
#endif

        AhiTestReportEvent(id, 0, 0);
    }

#ifdef DEBUG_MALLOC
    printf("MAIN: Event [%x] Available SLOTS:[%d]\n" ,id, VmGetAvailableAllocations() ) ;
#endif

}


/*************************************************************************
NAME
    handleHFPMessage

DESCRIPTION
    handles the messages from the user events

RETURNS

*/
static void handleHFPMessage  ( Task task, MessageId id, Message message )
{
    MAIN_DEBUG(("HFP[%x]\n", id)) ;

    UNUSED(task);

    switch(id)
    {
        /* -- Handsfree Profile Library Messages -- */
    case HFP_INIT_CFM:
        {
            /* Init configuration that is required now */


            InitEarlyUserFeatures();
            MAIN_DEBUG_L1(("HFP_INIT_CFM - enable streaming[%x]\n", sinkA2dpEnabled())) ;

            if(!BA_RECEIVER_MODE_ACTIVE)
                sinkInitConfigureDeviceClass();

            if  ( stateManagerGetState() == deviceLimbo )
            {
                if ( ((const HFP_INIT_CFM_T*)message)->status == hfp_success )
                {
                    sinkAppInit( (const HFP_INIT_CFM_T*)message );
                }
                else
                {
                    Panic();
                }
            }
#ifdef ACTIVITY_MONITORING
            tapUiInit();
            sport_health_app_init();
#endif
        }
    break;

    case HFP_SLC_CONNECT_IND:
        MAIN_DEBUG_L1(("HFP_SLC_CONNECT_IND\n"));
        if (stateManagerGetState() != deviceLimbo)
        {
            sinkHandleSlcConnectInd((const HFP_SLC_CONNECT_IND_T *) message);
        }
    break;

    case HFP_SLC_CONNECT_CFM:
        {
            const HFP_SLC_CONNECT_CFM_T *conncfm = (const HFP_SLC_CONNECT_CFM_T *)message;

            MAIN_DEBUG_L1(("HFP_SLC_CONNECT_CFM [%x]\n", conncfm->status ));
        if (stateManagerGetState() == deviceLimbo)
        {
                if ( conncfm->status == hfp_success )
            {
                /*A connection has been made and we are now  logically off*/
                sinkDisconnectAllSlc();
            }
        }
        else
        {
                sinkHandleSlcConnectCfm(conncfm);
#ifdef ENABLE_PEER
                if(!peerLinkReservedCanDeviceConnect(&conncfm->bd_addr))
            {  /* Another link is reserved for a peer device to connect, disconnect the second AG.*/ 
                    sinkDisconnectSlcFromDevice(&conncfm->bd_addr);
            }
#endif
        }
        }
        break;

    case HFP_SLC_LINK_LOSS_IND:
        MAIN_DEBUG_L1(("HFP_SLC_LINK_LOSS_IND\n"));
        slcHandleLinkLossInd((const HFP_SLC_LINK_LOSS_IND_T*)message);
    break;

    case HFP_SLC_DISCONNECT_IND:
        MAIN_DEBUG_L1(("HFP_SLC_DISCONNECT_IND\n"));
        sinkHandleSlcDisconnectInd((const HFP_SLC_DISCONNECT_IND_T *) message);
    break;
    case HFP_SERVICE_IND:
        MAIN_DEBUG_L1(("HFP_SERVICE_IND [%x]\n" , ((const HFP_SERVICE_IND_T*)message)->service  ));
        indicatorsHandleServiceInd ( (const HFP_SERVICE_IND_T*)message ) ;
    break;
    case HFP_HF_INDICATORS_REPORT_IND:
        MAIN_DEBUG_L1(("HFP_HF_INDICATORS_REPORT_IND  Number of HF Indicators:%x BitMask:[%x]\n" ,
              ((HFP_HF_INDICATORS_REPORT_IND_T*)message)->num_hf_indicators,
              ((HFP_HF_INDICATORS_REPORT_IND_T*)message)->hf_indicators_mask));
    break;
    case HFP_HF_INDICATORS_IND:
        /* To do- App devleopers should take action accordingly based on HF indication*/
        MAIN_DEBUG_L1(("HFP_HF_INDICATORS_IND Assigned Num:[%x] Status:[%x]\n" , ((HFP_HF_INDICATORS_IND_T*)message)->hf_indicator_assigned_num,
              ((HFP_HF_INDICATORS_IND_T*)message)->hf_indicator_status));
    break;
    /* indication of call status information, sent whenever a change in call status
       occurs within the hfp lib */
    case HFP_CALL_STATE_IND:
        /* the Call Handler will perform device state changes and be
           used to determine multipoint functionality */
        /* don't process call indications if in limbo mode */
        if(stateManagerGetState()!= deviceLimbo)
        {
            MAIN_DEBUG(("HFP - Call state %d\n", ((HFP_CALL_STATE_IND_T*)message)->call_state));
            /* If HFP is in call then cancel the VA session. */
            if(((HFP_CALL_STATE_IND_T*)message)->call_state != hfp_call_state_idle)
            {
#ifdef ENABLE_BISTO
                BistoAudioInClose();
#endif
            }
            sinkHandleCallInd((const HFP_CALL_STATE_IND_T*)message);
        }
    break;

    case HFP_RING_IND:
        MAIN_DEBUG_L1(("HFP_RING_IND\n"));
        sinkHandleRingInd((const HFP_RING_IND_T *)message);
    break;
    case HFP_VOICE_TAG_NUMBER_IND:
        MAIN_DEBUG_L1(("HFP_VOICE_TAG_NUMBER_IND\n"));
        sinkWriteStoredNumber((const HFP_VOICE_TAG_NUMBER_IND_T*)message);
    break;
    case HFP_DIAL_LAST_NUMBER_CFM:
        MAIN_DEBUG_L1(("HFP_LAST_NUMBER_REDIAL_CFM\n"));
        handleHFPStatusCFM (((const HFP_DIAL_LAST_NUMBER_CFM_T*)message)->status ) ;
    break;
    case HFP_DIAL_NUMBER_CFM:
        MAIN_DEBUG_L1(("HFP_DIAL_NUMBER_CFM %d %d\n", stateManagerGetState(),
                            ((const HFP_DIAL_NUMBER_CFM_T *) message)->status));
        handleHFPStatusCFM (((const HFP_DIAL_NUMBER_CFM_T*)message)->status ) ;
    break;
    case HFP_DIAL_MEMORY_CFM:
        MAIN_DEBUG_L1(("HFP_DIAL_MEMORY_CFM %d %d\n", stateManagerGetState(),
                            ((const HFP_DIAL_MEMORY_CFM_T *) message)->status));
    break ;
    case HFP_CALL_ANSWER_CFM:
        MAIN_DEBUG_L1(("HFP_ANSWER_CALL_CFM\n"));
    break;
    case HFP_CALL_TERMINATE_CFM:
        MAIN_DEBUG_L1(("HFP_TERMINATE_CALL_CFM %d\n", stateManagerGetState()));
    break;
    case HFP_VOICE_RECOGNITION_IND:
        MAIN_DEBUG_L1(("HS: HFP_VOICE_RECOGNITION_IND_T [%c]\n" ,
                            TRUE_OR_FALSE( ((const HFP_VOICE_RECOGNITION_IND_T* )message)->enable) )) ;


    /*update the state of the voice dialling on the back of the indication*/
        sinkHfpDataSetVoiceRecognitionIsActive(((const HFP_VOICE_RECOGNITION_IND_T* ) message)->enable);
    break;
    case HFP_VOICE_RECOGNITION_ENABLE_CFM:
        MAIN_DEBUG_L1(("HFP_VOICE_RECOGNITION_ENABLE_CFM s[%d] w[%d]i", (((const HFP_VOICE_RECOGNITION_ENABLE_CFM_T *)message)->status ) , sinkHfpDataGetVoiceRecognitionIsActive()));

            /*if the cfm is in error then we did not succeed - toggle */
        if  ( (((const HFP_VOICE_RECOGNITION_ENABLE_CFM_T *)message)->status ) )
            sinkHfpDataSetVoiceRecognitionIsActive(0);

        MAIN_DEBUG_L1(("[%d]\n", sinkHfpDataGetVoiceRecognitionIsActive()));

        handleHFPStatusCFM (((const HFP_VOICE_RECOGNITION_ENABLE_CFM_T *)message)->status ) ;
    break;
    case HFP_CALLER_ID_ENABLE_CFM:
        MAIN_DEBUG_L1(("HFP_CALLER_ID_ENABLE_CFM\n"));
    break;
    case HFP_VOLUME_SYNC_SPEAKER_GAIN_IND:
    {
        const HFP_VOLUME_SYNC_SPEAKER_GAIN_IND_T *ind = (const HFP_VOLUME_SYNC_SPEAKER_GAIN_IND_T *) message;

        MAIN_DEBUG_L1(("HFP_VOLUME_SYNC_SPEAKER_GAIN_IND %d\n", ind->volume_gain));

        VolumeHandleSpeakerGainInd(ind);
    }
    break;
    case HFP_VOLUME_SYNC_MICROPHONE_GAIN_IND:
    {
        const HFP_VOLUME_SYNC_MICROPHONE_GAIN_IND_T *ind = (const HFP_VOLUME_SYNC_MICROPHONE_GAIN_IND_T*)message;

        MAIN_DEBUG_L1(("HFP_VOLUME_SYNC_MICROPHONE_GAIN_IND %d\n", ind->mic_gain));
        if(sinkHfpDataIsEnableSyncMuteMicrophones())
        {
            VolumeSetHfpMicrophoneGainCheckMute(ind->priority, (uint8)ind->mic_gain);
        }
    }

    break;

    case HFP_CALLER_ID_IND:
        {
            const HFP_CALLER_ID_IND_T *ind = (const HFP_CALLER_ID_IND_T *) message;

            /* ensure this is not a HSP profile */
            MAIN_DEBUG_L1(("HFP_CALLER_ID_IND number %s", ind->caller_number));

            /* Show name or number on display */
            if (ind->size_name)
            {
                MAIN_DEBUG_L1((" name %s\n", ind->caller_name));
                displayShowSimpleText((char*) ind->caller_name, SINK_TEXT_TYPE_CALLER_INFO);
            }
            else
            {
                MAIN_DEBUG_L1((" No Name\n"));
                displayShowSimpleText((char*) ind->caller_number, SINK_TEXT_TYPE_CALLER_INFO);
            }

            /* Attempt to play caller name */
            if(!AudioPromptPlayCallerName (ind->size_name, ind->caller_name))
            {
                /* Caller name not present or not supported, try to play number */
                AudioPromptPlayCallerNumber(ind->size_number, ind->caller_number) ;
            }

            if (ind->size_name)
            {
                free(ind->caller_name);
            }
        }

    break;

    case HFP_UNRECOGNISED_AT_CMD_IND:
    {
        sinkHandleUnrecognisedATCmd( (HFP_UNRECOGNISED_AT_CMD_IND_T*)message ) ;
    }
    break ;

    case HFP_HS_BUTTON_PRESS_CFM:
        {
            MAIN_DEBUG_L1(("HFP_HS_BUTTON_PRESS_CFM\n")) ;
        }
    break ;
     /*****************************************************************/

#ifdef THREE_WAY_CALLING
    case HFP_CALL_WAITING_ENABLE_CFM :
            MAIN_DEBUG_L1(("HS3 : HFP_CALL_WAITING_ENABLE_CFM_T [%c]\n",
                            TRUE_OR_FALSE(((const HFP_CALL_WAITING_ENABLE_CFM_T * )message)->status == hfp_success)));
    break ;
    case HFP_CALL_WAITING_IND:
        {
            /* pass the indication to the multipoint handler which will determine if the call waiting tone needs
               to be played, this will depend upon whether the indication has come from the AG with
               the currently routed audio */
            mpHandleCallWaitingInd((HFP_CALL_WAITING_IND_T *)message);
        }
    break;

#endif
    case HFP_SUBSCRIBER_NUMBERS_CFM:
        MAIN_DEBUG_L1(("HS3: HFP_SUBSCRIBER_NUMBERS_CFM [%c]\n" ,
                            TRUE_OR_FALSE(((const HFP_SUBSCRIBER_NUMBERS_CFM_T*)message)->status == hfp_success)));
    break ;
#ifdef DEBUG_MAIN
    case HFP_SUBSCRIBER_NUMBER_IND:
    {
        uint16 i=0;
        const HFP_SUBSCRIBER_NUMBER_IND_T *subs_ind = (const HFP_SUBSCRIBER_NUMBER_IND_T*)message;

        MAIN_DEBUG_L1(("HS3: HFP_SUBSCRIBER_NUMBER_IND [%d]\n" , subs_ind->service ));

        for (i=0;i < subs_ind->size_number ; i++)
        {
            MAIN_DEBUG(("%c", subs_ind->number[i]));
        }
        MAIN_DEBUG(("\n")) ;
    }
    break ;
#endif
    case HFP_CURRENT_CALLS_CFM:
        MAIN_DEBUG_L1(("HS3: HFP_CURRENT_CALLS_CFM [%c]\n",
                        TRUE_OR_FALSE(((const HFP_CURRENT_CALLS_CFM_T*)message)->status == hfp_success)));
    break ;
    case HFP_CURRENT_CALLS_IND:
        MAIN_DEBUG_L1(("HS3: HFP_CURRENT_CALLS_IND id[%d] mult[%d] status[%d]\n" ,
                                        ((const HFP_CURRENT_CALLS_IND_T*)message)->call_idx ,
                                        ((const HFP_CURRENT_CALLS_IND_T*)message)->multiparty  ,
                                        ((const HFP_CURRENT_CALLS_IND_T*)message)->status)) ;
    break;
    case HFP_AUDIO_CONNECT_IND:
        MAIN_DEBUG_L1(("HFP_AUDIO_CONNECT_IND\n")) ;
        audioHandleSyncConnectInd( (const HFP_AUDIO_CONNECT_IND_T *)message ) ;
    break ;
    case HFP_AUDIO_CONNECT_CFM:
        {
            const HFP_AUDIO_CONNECT_CFM_T *conn_cfm = (const HFP_AUDIO_CONNECT_CFM_T *)message;

            MAIN_DEBUG_L1(("HFP_AUDIO_CONNECT_CFM[%x][%x][%s%s%s] r[%d]t[%d]\n", conn_cfm->status ,
                                                          (int)conn_cfm->audio_sink ,
                                                          ((conn_cfm->link_type == sync_link_sco) ? "SCO" : "" )      ,
                                                          ((conn_cfm->link_type == sync_link_esco) ? "eSCO" : "" )    ,
                                                          ((conn_cfm->link_type == sync_link_unknown) ? "unk?" : "" ) ,
                                                          (uint16)conn_cfm->rx_bandwidth ,
                                                          (uint16)conn_cfm->tx_bandwidth)) ;

        /* should the device receive a sco connect cfm in limbo state */
        if (stateManagerGetState() == deviceLimbo)
        {
            /* confirm that it connected successfully before disconnecting it */
                if (conn_cfm->status == hfp_audio_connect_no_hfp_link)
            {
                MAIN_DEBUG_L1(("HFP_AUDIO_CONNECT_CFM in limbo state, disconnect it\n" ));
                    ConnectionSyncDisconnect(conn_cfm->audio_sink, hci_error_oetc_user);
            }
        }
        /* not in limbo state, process sco connect indication */
        else
        {
                audioHandleSyncConnectCfm(conn_cfm);
            }
        }
    break ;
    case HFP_AUDIO_DISCONNECT_IND:
        MAIN_DEBUG_L1(("HFP_AUDIO_DISCONNECT_IND [%x]\n", ((const HFP_AUDIO_DISCONNECT_IND_T *)message)->status)) ;
        audioHandleSyncDisconnectInd ((const HFP_AUDIO_DISCONNECT_IND_T *)message) ;
    break ;
    case HFP_SIGNAL_IND:
        MAIN_DEBUG_L1(("HS: HFP_SIGNAL_IND [%d]\n", ((const HFP_SIGNAL_IND_T* )message)->signal )) ;
    break ;
    case HFP_ROAM_IND:
        MAIN_DEBUG_L1(("HS: HFP_ROAM_IND [%d]\n", ((const HFP_ROAM_IND_T* )message)->roam )) ;
    break;
    case HFP_BATTCHG_IND:
        MAIN_DEBUG_L1(("HS: HFP_BATTCHG_IND [%d]\n", ((const HFP_BATTCHG_IND_T* )message)->battchg )) ;
    break;

/*******************************************************************/

    case HFP_CSR_FEATURES_TEXT_IND:
        csr2csrHandleTxtInd () ;
    break ;

    case HFP_CSR_FEATURES_NEW_SMS_IND:
       csr2csrHandleSmsInd () ;
    break ;

    case HFP_CSR_FEATURES_GET_SMS_CFM:
       csr2csrHandleSmsCfm() ;
    break ;

    case HFP_CSR_FEATURES_BATTERY_LEVEL_REQUEST_IND:
       csr2csrHandleAgBatteryRequestInd() ;
    break ;

/*******************************************************************/

/*******************************************************************/

    /*******************************/

    default :
        MAIN_DEBUG_L1(("HS :  HFP ? [%x]\n",id)) ;
    break ;
    }
}

static void updateEnhancements(uint16 new_enhancements_bypassed)
{
    uint16 old_enhancements_bypassed = getMusicProcessingBypassFlags();
    MAIN_DEBUG(("dsp_ind: store enhancements = %x was %x\n", new_enhancements_bypassed, old_enhancements_bypassed));
    /* Remove the MUSIC_CONFIG_DATA_VALID flag in the stored session data, before comparing with the DSP value. */
    old_enhancements_bypassed &= (~MUSIC_CONFIG_DATA_VALID);

    /* Make sure current setting is no longer set to next and store it */
    if(old_enhancements_bypassed != new_enhancements_bypassed)
    {
        /*  Add the data valid flag, this signifies that the user has altered the 3d or bass boost enhancements,
         *  these values should now be used instead of dsp default values that have been created by the UFE.
         */
        new_enhancements_bypassed |= MUSIC_CONFIG_DATA_VALID;
        MAIN_DEBUG(("dsp_ind: update enhancements = %x\n", new_enhancements_bypassed));
        setMusicProcessingBypassFlags(new_enhancements_bypassed);
    }
}

/* Handle any audio plugin messages */
static void handleAudioPluginMessage( Task task, MessageId id, Message message )
{

    UNUSED(task);

    switch (id)
    {
        case AUDIO_DSP_IND:
        {
            const AUDIO_DSP_IND_T *dspind = (const AUDIO_DSP_IND_T*)message;
            /* Clock mismatch rate, sent from the DSP via the a2dp decoder common plugin? */
            if (dspind->id == KALIMBA_MSG_SOURCE_CLOCK_MISMATCH_RATE)
            {
                handleA2DPStoreClockMismatchRate(dspind->value[0]);
            }
            /* Current EQ bank, sent from the DSP via the a2dp decoder common plugin? */
            else if (dspind->id == A2DP_MUSIC_MSG_CUR_EQ_BANK)
            {
                uint16 eq_bank = dspind->value[0];
                if(sinkMusicProcessingUpdateEqBank(eq_bank))
                {
                    MAIN_DEBUG(("dsp_ind: Update eq bank to %u\n", eq_bank));
#ifdef ENABLE_PEER
                    peerSendAudioEnhancements();
#endif
                }
            }
            /* Current enhancements, sent from the DSP via the a2dp decoder common plugin? */
            else if (dspind->id == A2DP_MUSIC_MSG_ENHANCEMENTS)
            {
                updateEnhancements(dspind->value[1]);
#ifdef ENABLE_PEER
                peerSendAudioEnhancements();
#endif
            }
        }
        break;

        /* indication that the DSP is ready to accept data ensuring no audio samples are disposed of */
        case AUDIO_DSP_READY_FOR_DATA:
            /* ensure dsp is up and running */

#if defined ENABLE_GAIA && defined ENABLE_GAIA_PERSISTENT_USER_EQ_BANK
            handleA2DPUserEqBankUpdate();
#endif

            if(((const AUDIO_DSP_READY_FOR_DATA_T*)message)->dsp_status == DSP_RUNNING)
            {
                MAIN_DEBUG(("HS :  DSP ready for data\n")) ;
#ifdef ENABLE_PEER
                /*Request the connected peer device to send its current user EQ settings across if its a peer source.*/
                peerRequestUserEqSetings();
#endif
            }

#ifdef ENABLE_SUBWOOFER
            /* configure the subwoofer type when the dsp is up and running */
            if(SwatGetMediaType(sinkSwatGetDevId()) == SWAT_MEDIA_STANDARD)
                AudioConfigureSubWoofer(AUDIO_SUB_WOOFER_L2CAP, SwatGetMediaSink(sinkSwatGetDevId()));
            else if(SwatGetMediaType(sinkSwatGetDevId()) == SWAT_MEDIA_LOW_LATENCY)
                AudioConfigureSubWoofer(AUDIO_SUB_WOOFER_ESCO, SwatGetMediaSink(sinkSwatGetDevId()));
#endif
        break;

#ifdef ENABLE_GAIA

        case AUDIO_GET_USER_EQ_PARAMETER_CFM:
        {
            AUDIO_GET_USER_EQ_PARAMETER_CFM_T* get_user_eq_resp_msg = (AUDIO_GET_USER_EQ_PARAMETER_CFM_T *)message;
            GaiaHandleGetUserEqParamResponse(get_user_eq_resp_msg);
        }
        break;

        case AUDIO_GET_USER_EQ_PARAMETERS_CFM:
        {
            AUDIO_GET_USER_EQ_PARAMETERS_CFM_T* get_user_eq_resp_msg = (AUDIO_GET_USER_EQ_PARAMETERS_CFM_T *)message;
            GaiaHandleGetUserEqParamsResponse(get_user_eq_resp_msg);
        }
        break;
#endif    /* ENABLE_GAIA */

        case AUDIO_LATENCY_REPORT:
        {
            const AUDIO_LATENCY_REPORT_T *report = (const AUDIO_LATENCY_REPORT_T *)message;

            handleA2DPLatencyReport(report->estimated, report->latency, report->sink);
        }
        break;

        case AUDIO_REFRESH_VOLUME:
        {
            MAIN_DEBUG(("HS :  AUDIO Refresh volume\n")) ;
            /* Refresh the volume and mute status for the routed audio */
            sinkVolumeUpdateRoutedAudioMainAndAuxVolume();
            VolumeApplySoftMuteStates();
        }
        break;

        case AUDIO_SIGNAL_DETECT_MSG:
        {
            if (stateManagerGetState() != deviceLimbo)
            {
                bool signal_detected = ((AUDIO_SIGNAL_DETECT_MSG_T *) message)->signal_detected;

                MAIN_DEBUG(("HS: AUDIO signal %u\n", signal_detected));
                sinkAudioSetSilence(!signal_detected);
                PioDrivePio(PIO_AUDIO_ACTIVE, signal_detected);
            }
        }
        break;

        case AUDIO_PROMPT_DONE:
            MAIN_DEBUG(("HS : Audio prompt done\n")) ;
            break;

        /* BA Receiver specific Plugin up stream messages */
        case AUDIO_BA_RECEIVER_START_SCAN_VARIANT_IV:
        case AUDIO_BA_RECEIVER_RESET_BD_ADDRESS:
        case AUDIO_BA_RECEIVER_INDICATE_STREAMING_STATE:
        case AUDIO_BA_RECEIVER_AFH_CHANNEL_MAP_CHANGE_PENDING:
        case AUDIO_BA_RECEIVER_RESET_SCM_RECEIVER:
        {
            SinkBroadcastAudioHandlePluginUpStreamMessage(task, id, message);
        }
        break;

        default:
            MAIN_DEBUG(("HS :  AUDIO ? [%x]\n",id)) ;
        break ;
    }
}

#ifdef ENABLE_DISPLAY
/* Handle any display plugin messages */
static void handleDisplayPluginMessage( Task task, MessageId id, Message message )
{
    UNUSED(task);
    switch (id)
    {
    case DISPLAY_PLUGIN_INIT_IND:
        {
            DISPLAY_PLUGIN_INIT_IND_T *m = (DISPLAY_PLUGIN_INIT_IND_T *) message;
            MAIN_DEBUG_L1(("HS :  DISPLAY INIT: %u\n", m->result));
            if (m->result)
            {
                displayIconInit();
                if (powerManagerIsChargerConnected() && (stateManagerGetState() == deviceLimbo) )
                {
                    /* indicate charging if in limbo */
                    displaySetState(TRUE);
                    displayUpdateVolume(0);
                    displayUpdateBatteryLevel(TRUE);
#ifdef ENABLE_LINKKEY_DISPLAY
                    displayHideLinkKey();
#endif
                }
                else if (stateManagerGetState() != deviceLimbo)
                {
                    /* if this init occurs and not in limbo, turn the display on */
                    displaySetState(TRUE);
                    displayShowText(DISPLAYSTR_HELLO, strlen(DISPLAYSTR_HELLO), DISPLAY_TEXT_SCROLL_SCROLL, 1000, 2000, FALSE, 10, SINK_TEXT_TYPE_DEVICE_STATE);
                    displayUpdateVolume((sinkHfpDataGetDefaultVolume() * sinkVolumeGetNumberOfVolumeSteps(audio_output_group_main)) / VOLUME_NUM_VOICE_STEPS);
                    /* update battery display */
                    displayUpdateBatteryLevel(FALSE);
                }
            }
        }
        break;

    default:
        MAIN_DEBUG(("HS :  DISPLAY ? [%x]\n",id)) ;
        break ;
    }
}
#endif /* ENABLE_DISPLAY */


/*******************************************************************************
    AHI message handler
*/
static void ahi_handler(MessageId id, Message message)
{

    switch (id)
    {
    case AHI_APP_MODE_CHANGE_REQ:
        {
            AHI_APP_MODE_CHANGE_REQ_T *msg = (AHI_APP_MODE_CHANGE_REQ_T *)message;

            bool reboot_needed = sinkAhiIsRebootRequired(msg->app_mode);

            if( sinkAhiSetAppMode(msg->app_mode) == sink_ahi_status_succeeded )
            {
                if(reboot_needed)
                {
                    /* TODO */
                    /* Add all code that needs to be run before reboot */
                }
                AhiAppModeChangeCfm(msg->app_mode, reboot_needed);
            }
            else
            {    
                AhiAppModeChangeCfm(sinkAhiGetAppMode(), FALSE);
            }
        }
        break;
    case AHI_APP_TRANSPORT_DISABLE_REQ:
        {
            if (sink_ahi_status_succeeded == sinkAhiSetUsbHidDataLinkState(sink_ahi_usb_hid_datalink_disabled))
                AhiAppTransportDisableCfm(ahi_status_success);
            else
                AhiAppTransportDisableCfm(ahi_status_bad_parameter);
        }
        break;
    default:
        MAIN_DEBUG(("Unhandled AHI msg 0x%x\n", id));
    }
}
#ifdef DEBUG_VM_HATS
/*************************************************************************
NAME
    printVmLogsInTestSystem

DESCRIPTION
    This function prints logs of VM code in conjunction with HATS logs if
    it is enabled.

RETURNS
    void
*/
void printVmLogsInTestSystem (const char *format, ...)
{
  char *print_buffer=NULL;
  va_list arguments;
  va_start (arguments, format);
  print_buffer = PanicUnlessMalloc(PRINT_BUFF_SIZE);
  if(print_buffer)
  {
      vsprintf (print_buffer,format, arguments);
      AhiPrintVmLogs(print_buffer);
      free(print_buffer);
  }
  va_end (arguments);
  return;
}
#endif
/*************************************************************************
NAME
    app_handler

DESCRIPTION
    This is the main message handler for the Sink Application.  All
    messages pass through this handler to the subsequent handlers.

RETURNS

*/
void app_handler(Task task, MessageId id, Message message)
{
/*    MAIN_DEBUG(("MSG [%x][%x][%x]\n", (int)task , (int)id , (int)&message)) ;*/
    {
    uint16 err = LedsInErrorMode();
    if (err)
        LOG_ERROR(("Config Error, code=%u\n", err));
    }

    /* determine the message type based on base and offset */
    if ( ( id >= EVENTS_MESSAGE_BASE ) && ( id < EVENTS_LAST_EVENT ) )
    {
        handleUEMessage(task, id,  message);
    }
    else if ((AHI_MESSAGE_BASE <= id) && (AHI_LAST_APP_MESSAGE > id))
    {
        ahi_handler(id, message);
    }
#ifdef MESSAGE_EXE_FS_VALIDATION_STATUS
    else if (id == MESSAGE_EXE_FS_VALIDATION_STATUS)
    {
        sinkUpgradeMsgHandler(task, id, message);
    }
#endif
    else  if ( (id >= CL_MESSAGE_BASE) && (id < CL_MESSAGE_TOP) )
    {
        handleCLMessage(task, id,  message);
        ahiTestEchoConnectionMessage(id, message);
    }
    else if((id >= LOCAL_DEVICE_MESSAGE_BASE) && (id < LOCAL_DEVICE_MESSAGE_TOP))
    {
        sinkHfpInit();
    }
#if defined(ENABLE_ADK_NFC)
    else if (sinkNfcIsMessageIdNfc(id))
    {
        handleNfcClMessage(task, id,  message);
    }
#endif /* ENABLE_ADK_NFC*/
    else if ( (id >= HFP_MESSAGE_BASE ) && (id < HFP_MESSAGE_TOP) )
    {
        handleHFPMessage(task, id,  message);
        ahiTestEchoHfpMessage(id, message);
    }
    else if ( (id >= POWER_MESSAGE_BASE ) && (id < POWER_MESSAGE_TOP) )
    {
        handlePowerMessage (task, id, message) ;
        sinkUpgradePowerEventHandler();
    }
#ifdef ENABLE_PBAP
    else if ( ((id >= PBAPC_MESSAGE_BASE ) && (id < PBAPC_MESSAGE_TOP)) ||
              ((id >= PBAPC_APP_MSG_BASE ) && (id < PBAPC_APP_MSG_TOP)) )
    {
        handlePbapMessages (task, id, message) ;
    }
#endif
#ifdef ENABLE_MAPC
    else if ( ((id >= MAPC_MESSAGE_BASE )    && (id < MAPC_API_MESSAGE_END)) ||
              ((id >= MAPC_APP_MESSAGE_BASE) && (id < MAPC_APP_MESSAGE_TOP)) )
    {
        handleMapcMessages (task, id, message) ;
    }
#endif
#ifdef ENABLE_AVRCP
    else if ( (id >= AVRCP_INIT_CFM ) && (id < SINK_AVRCP_MESSAGE_TOP) )
    {
        sinkAvrcpHandleMessage (task, id, message) ;
        ahiTestEchoAvrcpMessage(id, message);
    }
#endif

    else if ( (id >= HID_MESSAGE_BASE ) && (id < HID_MESSAGE_TOP) )
    {
        sinkHidHandleMessage (task, id, message) ;
    }

#ifdef CVC_PRODTEST
    else if (id == MESSAGE_FROM_KALIMBA)
    {
        cvcProductionTestKalimbaMessage (task, id, message);
    }
#endif
    else if ( (id >= A2DP_MESSAGE_BASE ) && (id < A2DP_MESSAGE_TOP) )
    {
        handleA2DPMessage(task, id,  message);
        ahiTestEchoA2dpMessage(id, message);
        return;
    }
    else if ( (id >= AUDIO_UPSTREAM_MESSAGE_BASE ) && (id < AUDIO_UPSTREAM_MESSAGE_TOP) )
    {
        handleAudioPluginMessage(task, id,  message);
    }
#ifdef ENABLE_USB
    else if( ((id >= MESSAGE_USB_ENUMERATED) && (id <= MESSAGE_USB_SUSPENDED)) ||
             ((id >= MESSAGE_USB_DECONFIGURED) && (id <= MESSAGE_USB_DETACHED)) ||
             ((id >= USB_DEVICE_CLASS_MSG_BASE) && (id < USB_DEVICE_CLASS_MSG_TOP)) )
    {
        handleUsbMessage(task, id, message);
        return;
    }
#endif /* ENABLE_USB */
#ifdef ENABLE_GAIA
    else if ((id >= GAIA_MESSAGE_BASE) && (id < GAIA_MESSAGE_TOP))
    {
        handleGaiaMessage(task, id, message);
    }
#endif
    else if (id == MESSAGE_DFU_SQIF_STATUS)
    {
        /* tell interested modules the status of a DFU from SQIF operation */
        sinkUpgradeMsgHandler(task, id, message);
    }
#ifdef ENABLE_DISPLAY
    else if ( (id >= DISPLAY_UPSTREAM_MESSAGE_BASE ) && (id < DISPLAY_UPSTREAM_MESSAGE_TOP) )
    {
        handleDisplayPluginMessage(task, id,  message);
    }
#endif   /* ENABLE_DISPLAY */
#ifdef ENABLE_SUBWOOFER
    else if ( (id >= SWAT_MESSAGE_BASE) && (id < SWAT_MESSAGE_TOP) )
    {
        handleSwatMessage(task, id, message);
    }
#endif /* ENABLE_SUBWOOFER */
#ifdef ENABLE_FM
    else if ( (id >= FM_UPSTREAM_MESSAGE_BASE ) && (id < FM_UPSTREAM_MESSAGE_TOP) )
    {
        sinkFmHandleFmPluginMessage(id, message);
    }
#endif /* ENABLE_FM*/
    else if (sinkUpgradeIsUpgradeMsg(id))
    {
        sinkUpgradeMsgHandler(task, id, message);
    }
    else if(SPP_CLIENT_CONNECT_CFM == id)
    {
        SPP_CLIENT_CONNECT_CFM_T *m  = (SPP_CLIENT_CONNECT_CFM_T *) message ;
        MAIN_DEBUG((" SPP_CLIENT_CONNECT_CFM: %u\n", m->status));
        if(m->status  == spp_connect_success)
        {
            sppSetSinkData(m->sink);
        }
    }
    else
    {
        MAIN_DEBUG_L1(("MSGTYPE ? [%x]\n", id)) ;
        return;
    }
}

/* Time critical initialisation */
#ifdef HOSTED_TEST_ENVIRONMENT
void _sink_init(void)
#else
void _init(void)
#endif
{
    config_store_status_t status;

    /* Set the application task */
    theSink.task.handler = app_handler;

    /* set flag to indicate that configuration is being read, use to prevent use of variables
       prior to completion of initialisation */
    sinkDataSetSinkInitialisingStatus(TRUE);

    /* Initialize config store library */
    status = ConfigStoreInit(ConfigDefinitionGetConstData(), 
                             ConfigDefinitionGetConstDataSize(), 
                             SinkUpgradeRunningNewImage());
    if(status != config_store_success)
    {
       DEBUG (("ConfigStoreInit failed [status = %u] \n",status));
       Panic();
    }

    /* Initialise Sink Ahi private data */
    sinkAhiEarlyInit();

    /* Initialise Led Manager private data. Full initialization only if application is not in config mode. */
    LedManagerInit(ahi_app_mode_configuration != sinkAhiGetAppMode());

    /* Init PIO module */
    if(PioInit())
    {
       /* Assert audio amplifier mute if confugured */
        PioDrivePio(PIO_AMP_MUTE, TRUE);
    }
 
    /* Time critical USB setup */
#ifdef ENABLE_USB
    usbTimeCriticalInit();
#endif /* ENABLE_USB */
}

/*************************************************************************
NAME
    sinkConnectionInit

DESCRIPTION
    Initialise the Connection library
*/
static void sinkConnectionInit(void)
{
    uint16 pdl_size = 0;

    pdl_size = sinkDataGetPDLSize();
    MAIN_DEBUG (("PDLSize[%d]\n" , pdl_size));

    /* Initialise the Connection library */
    ConnectionInitEx3(&theSink.task, NULL, pdl_size, SC_CONNECTION_LIB_OPTIONS);
}


/* The Sink Application starts here...*/
#ifdef HOSTED_TEST_ENVIRONMENT
int sink_main(void)
#else
int main(void)
#endif
{
#ifndef HOSTED_TEST_ENVIRONMENT
    /* Initialise the 'pmalloc' pools. */
    sink_pmalloc_init();
#endif

    OsInit();
    VmDeepSleepEnable(1);

    /* Certain chips require a HUB attach to enumerate as a USB device. */
    usbAttachToHub();

    /* Update PS Keys after Upgrade */
    SinkDfuPsUpdate();
    
    /* Initialise Sink Ahi */
    sinkAhiInit(&theSink.task);

    /* Initialise memory required early */
    configManagerInitMemory();

    /* Initialise device state */
    AuthResetConfirmationFlags();

    /*the internal regs must be latched on (smps and LDO)*/
    PioSetPowerPin ( TRUE ) ;

    switch(BootGetMode())
    {
#ifdef CVC_PRODTEST
        case BOOTMODE_CVC_PRODTEST:
            /*run the cvc prod test code and dont start the applicaiton */
            cvcProductionTestEnter() ;
        break ;
#endif
        case BOOTMODE_DFU:
            /*do nothing special for the DFU boot mode,
            This mode expects to have the appropriate host interfface enabled
            Don't start the application */

            /* Initializing only the system components required for flashing the led pattern in the DFU mode*/
            configManagerInit(FALSE);
            LedManagerIndicateEvent(EventUsrEnterDFUMode);
        break ;

        case BOOTMODE_DEFAULT:
        case BOOTMODE_CUSTOM:
        case BOOTMODE_USB_LOW_POWER:
        case BOOTMODE_ALT_FSTAB:
        default:
        {
            /* Initialise non downloadable specific dsp capabilies config*/
            sinkDspCapabilitiesInit();

            /* Initialise downloadable capabilies bundle config container*/
            sinkDownloadableCapsInit();
            
            /* Initialize global data for audio*/
            sinkAudioInit();

            sinkAudioTtpLatencyParamsInit();

            /* Make sure the mute states are correctly set up */
            VolumeSetInitialMuteState();

            if(sinkAhiGetAppMode() == ahi_app_mode_configuration)
            {
                /* Config mode - do not initialise the main sink app. */
                stateManagerEnterConfigMode();
            }
            else
            {
                /* Configure Broadcast Audio mode based on last stored configuration */
                sinkBroadcastAudioConfigure();
                /* Initialise the Connection lib */
                sinkConnectionInit();
            }
        }
        break;
    }

    /* Initialise the Upgrade lib only if BA mode is not Active*/
    sinkUpgradeInit(&theSink.task);

    /* Start protection mechanism for buffer defences */
    watchdogReset();

    /* Start the message scheduler loop */
    MessageLoop();

    /* Never get here...*/
    return 0;
}


/*************************************************************************

NAME
    handleHFPStatusCFM

DESCRIPTION
    Handles a status response from the HFP and sends an error message if one was received

RETURNS

*/
static void handleHFPStatusCFM ( hfp_lib_status pStatus )
{
    if (pStatus != hfp_success )
    {
        MAIN_DEBUG(("HS: HFP CFM Err [%d]\n" , pStatus)) ;
        MessageSend ( &theSink.task , EventSysError , 0 ) ;
#ifdef ENABLE_PBAP
        if(pbapGetCommand() == pbapc_dialling)
        {
            MessageSend ( &theSink.task , EventSysPbapDialFail , 0 ) ;
        }
#endif
    }
    else
    {
         MAIN_DEBUG(("HS: HFP CFM Success [%d]\n" , pStatus)) ;
    }

#ifdef ENABLE_PBAP
    pbapSetCommand(pbapc_action_idle);
#endif
}

/*************************************************************************
NAME
    voiceDialProcessEvent

DESCRIPTION
    Handles a voice dial event

RETURNS TRUE if event is to be indicated, otherwise FALSE

*/
static bool voiceDialProcessEvent(hfp_link_priority hfp_link)
{
    bool indicate_event = TRUE;
    
    if (sinkHfpDataGetVoiceRecognitionIsActive())
    {
        sinkCancelVoiceDial(hfp_link);
        indicate_event = FALSE;
        /* replumb any existing audio connections */       
        audioUpdateAudioRouting();
    }
    else
        sinkInitiateVoiceDial(hfp_link);
    
    return indicate_event;
}

/*************************************************************************
NAME
    IndicateEvent

DESCRIPTION
    Passes the msg Id to the relevant indication informers.

RETURNS None

*/
static void IndicateEvent(MessageId id)
{
    if (id != EventSysLEDEventComplete)
    {
        LedManagerIndicateEvent(id);
    }
    sinkAudioIndicationPlayEvent(id);
    ATCommandPlayEvent(id);
}

/****************************************************************************
NAME
    sinkCommonProcessEvent

DESCRIPTION
    Event handler for certain usr/sys events

RETURNS
    TRUE if the event is to be indicated, otherwise FALSE
*/
#ifdef ENABLE_BISTO
bool sinkCommonProcessEvent(sinkEvents_t event, Message message)
{
    bool indicate_event = FALSE;
    
    switch(event)
    {
        case EventUsrAvrcpPlayPause:
        case EventUsrAvrcpPlay:
        case EventUsrAvrcpPause:
        case EventUsrAvrcpStop:
        case EventUsrAvrcpSkipForward:
        case EventUsrAvrcpSkipBackward:
        case EventUsrAvrcpFastForwardPress:
        case EventUsrAvrcpFastForwardRelease:
        case EventUsrAvrcpRewindPress:
        case EventUsrAvrcpRewindRelease:
        case EventUsrAvrcpToggleActive:
        case EventUsrAvrcpNextGroupPress:
        case EventUsrAvrcpNextGroupRelease:
        case EventUsrAvrcpPreviousGroupPress:
        case EventUsrAvrcpPreviousGroupRelease:
        case EventUsrAvrcpShuffleOff:
        case EventUsrAvrcpShuffleAllTrack:
        case EventUsrAvrcpShuffleGroup:
        case EventUsrAvrcpRepeatOff:
        case EventUsrAvrcpRepeatSingleTrack:
        case EventUsrAvrcpRepeatAllTrack:
        case EventUsrAvrcpRepeatGroup:
        case EventSysSetActiveAvrcpConnection:
        case EventSysResetAvrcpMode:
            indicate_event = sinkAvrcpProcessEventAvrcp(event, message);
            break;
        case EventUsrAnswer:
            sinkAnswerOrRejectCall( TRUE );
            indicate_event = TRUE;
            break ;
        case EventUsrReject:
            sinkAnswerOrRejectCall( FALSE );
            indicate_event = TRUE;
            break ;
        case EventUsrCancelEnd:
            sinkHangUpCall();
            indicate_event = TRUE;
            break ;
        case EventUsrInitateVoiceDial:
            indicate_event = voiceDialProcessEvent(hfp_primary_link);
            break;
        case EventUsrInitateVoiceDial_AG2:
            indicate_event = voiceDialProcessEvent(hfp_secondary_link);
            break;
        default:
            break ;
    }
    return indicate_event;
}
/**  \} */ /* End sink_app group */
#endif

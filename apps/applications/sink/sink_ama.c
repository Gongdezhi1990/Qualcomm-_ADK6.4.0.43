/****************************************************************************
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_ama.c

DESCRIPTION
    For Amazon AVS

*/
/*!
@file   sink_ama.c
@brief  Implementation of the Sink code for Amazon AVS.
*/

#include <vmtypes.h>

#ifdef ENABLE_AMA
#include<ps.h>
#include "sink_ama.h"
#include <audio.h>
#include <audio_config.h>
#include "sink_ble_gap.h"
#include "sink_statemanager.h"
#include "sink_gatt_server_ama.h"
#include <ama.h>
#include <byte_utils.h>
#include "sink_audio_routing.h"
#include "sink_inquiry.h"
#include "sink_callmanager.h"
#include "sink_debug.h"
#include "sink_ama_config_def.h"
#include "sink_configmanager.h"
#include "sink_hfp_data.h"


/* Set the msbc codec bitpool */
#define VA_MSBC_ENCODER_BITPOOL_VALUE 26

/* PS key value for reading AMA Device Serial Number.
     which uses same PSKEY_USB_SERIAL_NUMBER_STRING (PSKHOSTIOUSBB + 7) 
     "USB serial number string" value ( 0x02C3).
*/
#define PSKEY_DEVICE_SERIAL_NUMBER_STRING  0x02C3

#ifdef DEBUG_AMA
#define AMA_INFO(x) DEBUG(x)
#else
#define AMA_INFO(x)
#endif /* DEBUG_AMA */

static void amaMessageHandler(Task task, MessageId id, Message message);

static const TaskData sink_ama_task = {amaMessageHandler};

/* Return the task associated with Voice Assistant */
#define AmaGetTask() ((Task)&sink_ama_task)


typedef struct
{
    ama_transport_t  transport;
    bool             transportUpgrade;
    bool             advertiseAmaEnabled;
}ama_t;

static ama_t sink_ama;


static void getAmaFeatureConfig(va_audio_mgr_feature_config_t *feature_config)
{
    #define MAX_TRIGGER_FILE_PATH_LEN 30

    sink_ama_config_def_t *ama_config_data = NULL;

    if(configManagerGetReadOnlyConfig(SINK_AMA_CONFIG_BLK_ID, (const void **)&ama_config_data))
    {
        char file_path[MAX_TRIGGER_FILE_PATH_LEN];

        if(ama_config_data->enable_voice_trigger)
        {
            feature_config->trigger_type = va_audio_mgr_trigger_both;
            AudioConfigSetVoiceTrigger(TRUE);
        }
        else
        {
            feature_config->trigger_type =va_audio_mgr_trigger_button;
        }

        feature_config->pre_roll_interval = (((uint32)ama_config_data->pre_roll_duration_hi) << 16) | ama_config_data->pre_roll_duration_lo;

        sprintf(file_path, "va/phrase_files/tfd_%d.bin", ama_config_data->phrase_index);

        feature_config->phrase_index = FileFind(FILE_ROOT, file_path, (uint16)strlen(file_path));

        configManagerReleaseConfig(SINK_AMA_CONFIG_BLK_ID);
    }
}

static void getAmaAudioConfig(va_audio_mgr_audio_config_t *audio_config)
{
    audio_config->mic_config.mic_a = sinkAudioGetMic1Params();
    audio_config->mic_config.mic_b = sinkAudioGetMic2Params();
    audio_config->codec_config.codec_type = va_audio_mgr_codec_msbc;
    audio_config->codec_config.bitpool_size = VA_MSBC_ENCODER_BITPOOL_VALUE;
    audio_config->cvc_type = sinkHfpDataGetAudioPlugin();
}

static char* getAmaDeviceSerialNumber(uint16 pskey)
{
    uint16 ps_length = 0;
    char* buffer = NULL;
    uint16 buf_byte_length;

    ps_length = PsFullRetrieve(pskey, NULL, 0);
    if(!ps_length)
    {
        AMA_INFO(("AMA PsFullRetrieve Failed\n"));
        Panic();
    }

    /* Calculate the required buffer byte length from PS KEY word length. */
    buf_byte_length = (2*ps_length + 1);

    buffer = PanicUnlessMalloc(buf_byte_length);

    if(ps_length == PsFullRetrieve(pskey, buffer, ps_length))
        buffer[buf_byte_length -1] = 0;
    else
        AMA_INFO(("AMA PsFullRetrieve Failed\n"));

    return buffer;
}

/****************************************************************************/
static void getAmaDeviceInfo(ama_device_config_t *device_info, CL_DM_LOCAL_NAME_COMPLETE_T *cfm)
{
    uint16 len_words = 0;
    uint16 len_bytes = 0;
    sink_ama_config_def_t *read_data = NULL;

    /* In order to read the AMA device serial number on the device, the product developer
         can choose to use one of the alternative implementations given below:

        1. Using PSKEY_USB_SERIAL_NUMBER_STRING MIB key to store the serial number
            and reading the value from this key using PsFullRetrieve() trap API.

        2. Using one of the customer read-only MIB keys to store the serial number and 
            reading the value from this key using PsFullRetrieve() trap API. 

        3. Creating a new configuration item in sink_ama_module_def.xml and retrieving 
            the value from the sink application configuration using configManagerGetReadOnlyConfig()
            and configManagerReleaseConfig() config manager API functions.

    Below is the example implementation for reading the device serial number from the value in 
    PSKEY_USB_SERIAL_NUMBER_STRING using PsFullRetrieve() trap API. */

    device_info->serial_number = getAmaDeviceSerialNumber(PSKEY_DEVICE_SERIAL_NUMBER_STRING);

    len_words = configManagerGetReadOnlyConfig(SINK_AMA_CONFIG_BLK_ID, (const void **)&read_data);
    if ((len_words) && (cfm->status == success))
    {
        device_info->name = PanicUnlessMalloc(cfm->size_local_name + 1);
        memcpy(device_info->name, cfm->local_name, cfm->size_local_name);
        device_info->name[cfm->size_local_name] = '\0';

        /* Find length of each field strings and unpack. */
        len_bytes = ByteUtilsGetPackedStringLen((const uint16*)read_data->device_type, len_words);
        device_info->device_type = PanicUnlessMalloc(len_bytes + 1);
        ByteUtilsMemCpyUnpackString((uint8 *)device_info->device_type, (const uint16*)read_data->device_type, len_bytes);
        device_info->device_type[len_bytes] = '\0';
        configManagerReleaseConfig(SINK_AMA_CONFIG_BLK_ID);
    }
    else
    {
        AMA_INFO(("AMA device information configuration.failed\n"));
    }
}

static void handleAmaUpgradeTransportInd(void)
{
    AMA_INFO(("AMA_UPGRADE_TRANSPORT_IND\n"));
    sink_ama.transportUpgrade = TRUE;
}

static void handleAmaLinkDropInd(void)
{
    AMA_INFO(("AMA_LINK_DROP_IND\n"));
    AmaStopSpeech(FALSE);
    gapStartAdvertising();
}

static void handleAmaSpeechStateInd(AMA_SPEECH_STATE_IND_T *msg)
{
    AMA_INFO(("AMA_SPEECH_STATE_IND State = "))

    if(msg)
    {
        switch(msg->speech_state)
        {
            case ama_speech_state_idle:
                AMA_INFO(("ama_speech_state_idle\n"))
                break;

            case ama_speech_state_listening:
                AMA_INFO(("ama_speech_state_listening\n"))
                break;

            case ama_speech_state_processing:
                AMA_INFO(("ama_speech_state_processing\n"))
                break;

            case ama_speech_state_speaking:
                AMA_INFO(("ama_speech_state_speaking\n"))
                break;

            default:
                AMA_INFO(("unknown speech state\n"))
                break;
        }
    }
}

static void handleAmaEnableClassicPairingInd(void)
{
    AMA_INFO(("AMA_ENABLE_CLASSIC_PAIRING_IND\n"));
    sinkInquirySetInquirySession(inquiry_session_normal);
    stateManagerEnterConnDiscoverableState( FALSE );
}

static void handleAmaStartAdvertisingAmaInd(void)
{
    AMA_INFO(("AMA_START_ADVERTISING_AMA_IND\n"));
    sink_ama.advertiseAmaEnabled = TRUE;
    sinkBleGapStartReadLocalName(ble_gap_read_name_advertising);
}


static void handleAmaSwitchTransportInd(AMA_SWITCH_TRANSPORT_IND_T *msg)
{
    AMA_INFO(("AMA AMA_SWITCH_TRANSPORT_IND Transport = "));
    if(msg)
    {
        sink_ama.transport = msg->transport;
        sink_ama.transportUpgrade = FALSE;

        switch(msg->transport)
        {
            case ama_transport_ble:
                AMA_INFO(("ama_transport_ble\n"));
                break;

            case ama_transport_rfcomm:
                AMA_INFO(("ama_transport_rfcomm\n"));
                sinkAmaServerConnectionParameterUpdate(FALSE);
                break;

            case ama_transport_iap:
                AMA_INFO(("ama_transport_iap\n"));
                break;

            default:
                AMA_INFO(("UNKNOWN transport\n"));
                break;
        }
    }
}

static void handleAmaStopAvertisingAmaInd(void)
{
    AMA_INFO(("AMA_STOP_ADVERTISING_AMA_IND\n"));
    sink_ama.advertiseAmaEnabled = FALSE;
    sinkBleGapStartReadLocalName(ble_gap_read_name_advertising);
}

static void handleAmaSendVersionId(void)
{
    AMA_INFO(("AMA_LIB_SEND_TRANSPORT_VERSION_ID\n"));
    AmaTransportSendVersion(AMA_VERSION_MAJOR, AMA_VERSION_MINOR);
}

static void handleAmaSendAtCommandInd(AMA_SEND_AT_COMMAND_IND_T *msg)
{
    AMA_INFO(("AMA_SEND_AT_COMMAND_IND Command = "));
    if(msg)
    {
        switch(msg->at_command)
        {
            case ama_at_cmd_ata_ind:
                AMA_INFO(("ama_at_cmd_ata_ind\n"));
                sinkAnswerOrRejectCall(TRUE);
                break;

            case ama_at_cmd_at_plus_chup_ind:
                AMA_INFO(("ama_at_cmd_at_plus_chup_ind\n"));
                sinkAnswerOrRejectCall(FALSE);
                break;

            case ama_at_cmd_at_plus_bldn_ind:
                AMA_INFO(("ama_transport_ble\n"));
                sinkInitiateLNR(hfp_primary_link);
                break;

            case ama_at_cmd_at_plus_chld_eq_0_ind:
                AMA_INFO(("ama_at_cmd_at_plus_chld_eq_0_ind\n"));
                break;

            case ama_at_cmd_at_plus_chld_eq_1_ind:
                AMA_INFO(("ama_at_cmd_at_plus_chld_eq_1_ind\n"));
                break;

            case ama_at_cmd_at_plus_chld_eq_2_ind:
                AMA_INFO(("ama_at_cmd_at_plus_chld_eq_2_ind\n"));
                break;

            case ama_at_cmd_at_plus_chld_eq_3_ind:
                AMA_INFO(("ama_at_cmd_at_plus_chld_eq_3_ind\n"));
                break;

            case ama_at_cmd_atd_ind:
                AMA_INFO(("ama_at_cmd_atd_ind\n"));
                break;

            default:
                AMA_INFO(("UNKNOWN\n"));
                break;
        }
    }
}

static void initAmaLibrary(CL_DM_LOCAL_NAME_COMPLETE_T *cfm)
{
    va_audio_mgr_feature_config_t feature_cfg;
    va_audio_mgr_audio_config_t audio_cfg;
    ama_device_config_t device_info;

    AMA_INFO(("AMA AmaInitialise\n"));

    sink_ama.transport = ama_transport_ble;
    sink_ama.transportUpgrade = FALSE;  /* true when upgrade is in process */
    sink_ama.advertiseAmaEnabled = TRUE;

    getAmaFeatureConfig(&feature_cfg);
    getAmaAudioConfig(&audio_cfg);
    getAmaDeviceInfo(&device_info, cfm);

    AmaInit(AmaGetTask(), &feature_cfg, &audio_cfg, &device_info);
}

static void amaMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);

    AMA_INFO(("SINK_AMA Msg "));

    switch(id)
    {
        case AMA_UPGRADE_TRANSPORT_IND:
            handleAmaUpgradeTransportInd();
            break;

        case AMA_LINK_DROP_IND:
            handleAmaLinkDropInd();
            break;

        case AMA_SPEECH_STATE_IND:
            handleAmaSpeechStateInd((AMA_SPEECH_STATE_IND_T*)message);
            break;

        case AMA_ENABLE_CLASSIC_PAIRING_IND:
            handleAmaEnableClassicPairingInd();
            break;

         case AMA_START_ADVERTISING_AMA_IND:
            handleAmaStartAdvertisingAmaInd();
            break;

        case AMA_SWITCH_TRANSPORT_IND:
            handleAmaSwitchTransportInd((AMA_SWITCH_TRANSPORT_IND_T*)message);
            break;

        case AMA_STOP_ADVERTISING_AMA_IND:
            handleAmaStopAvertisingAmaInd();
            break;

        case AMA_SEND_AT_COMMAND_IND:
            handleAmaSendAtCommandInd((AMA_SEND_AT_COMMAND_IND_T*)message);
            break;

        case AMA_SEND_TRANSPORT_VERSION_ID:
            handleAmaSendVersionId();
            break;

        case CL_DM_LOCAL_NAME_COMPLETE:
            /* We request to read the local device name only as part of the initialisation */
            initAmaLibrary((CL_DM_LOCAL_NAME_COMPLETE_T*)message);

        default:
            AMA_INFO(("Msg unknown %d\n", id));
            break;

    }
}

void SinkAmaInit(void)
{
    /* The AMA library will be initialised once we get the local device name */
    ConnectionReadLocalName(AmaGetTask());
}

bool SinkAmaIsAdvertiseAmaEnabled(void)
{
    return sink_ama.advertiseAmaEnabled;
}

void SinkAmaTriggerTaptoTalk(void)
{
    AMA_INFO(("AMA SinkAmaTriggerTaptoTalk\n"));
	
    if(sinkAudioIsVoiceRouted() )
    {
        AMA_INFO(("AMA StartSpeech Ignore due to Active SCO\n"));
        return;
    }
	
    if(AmaStartSpeech(ama_speech_initiator_tap) == TRUE)
    {
        AMA_INFO(("AMA StartSpeech succcess\n"));
    }
}


void SinkAmaTriggerPushtoTalk(void)
{
    AMA_INFO(("AMA SinkAmaTriggerPushtoTalk\n"));

    if(sinkAudioIsVoiceRouted() )
    {
        AMA_INFO(("AMA StartSpeech Ignore due to Active SCO\n"));
        return;
    }

    if(AmaStartSpeech(ama_speech_initiator_press_and_hold) == TRUE)
    {
        AMA_INFO(("AMA StartSpeech succcess\n"));
    }
}

void SinkAmaTriggerPushtoTalkRelease(void)
{
    AMA_INFO(("AMA SinkAmaTriggerPushtoTalkRelease\n"));
    AmaEndSpeech();
}

void SinkAmaVoiceStop(void)
{
    AmaStopSpeech(TRUE);
}

void SinkAmaHFPCallInd(void)
{
    AmaSuspendSpeech();
}

void SinkAmaResumeSession(void)
{
    AMA_INFO(("AMA SinkAmaResumeSession()  sinkAudioIsVoiceRouted() %d\n", sinkAudioIsVoiceRouted() ));

    if(!sinkAudioIsVoiceRouted())
    {
         AmaResumeSpeech();
    }
}

void SinkAmaResetLink(const bdaddr* bd_addr)
{
    AmaResetRfcommLink(bd_addr);
}

void SinkAmaResetTransportAfterLinkLoss(void)
{
    AmaResetTransportState();
}


#endif /* ENABLE_AMA */


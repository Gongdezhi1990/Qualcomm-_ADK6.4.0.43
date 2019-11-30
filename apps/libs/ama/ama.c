/*****************************************************************************

Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama.c

DESCRIPTION
    Implementation for the common services internal to all AMA library modules

*********************************************************************************/
#include <stdlib.h>
#include "ama.h"
#include <message.h>
#include <vm.h>
#include <voice_assistant_audio_manager.h>
#include <vmal.h>
#include <transport_manager.h>
#include <audio.h>
#include <source.h>
#include "ama_speech_packetiser.h"
#include "ama_transport.h"
#include "ama_debug.h"
#include "ama_rfcomm.h"
#include "ama_speech.h"
#include "ama_state.h"
#include "ama_iap2.h"
#include "ama_send_command.h"
#include "ama_private.h"

static void amaMessageHandler(Task task, MessageId id, Message message);
static void amaHandleWakeWordInd(VA_AUDIO_MGR_WAKEWORD_IND_T *ind);
static void sendMessageMoreData(Task task, Source src, uint32 delay);

static const TaskData ama_task_data = {amaMessageHandler};
static Task app_task = NULL;
static uint32 pre_roll = 0;

/*! The source of captured mic audio */
Source voice_source = NULL;

 /* Return the task associated with Voice Assistant */
#define AmaLibGetTask() ((Task)&ama_task_data)

/* Store the Application's device information configurations. */
ama_device_config_t *ama_device_config = NULL;

/***************************************************************************/
static void amaInitDeviceInformation(const ama_device_config_t *device_config)
{
    /* just single instance */
    if(!ama_device_config)
        ama_device_config = PanicNull(calloc(1, sizeof (ama_device_config_t)));

    /* Store the AMA device information configurations. */
    ama_device_config->name = device_config->name;
    ama_device_config->device_type  = device_config->device_type;
    ama_device_config->serial_number = device_config->serial_number;
}

/***************************************************************************/

static void amaSetSource( Source new_source)
{
    Source old = voice_source;

    /* Removing valid source */
    if (NULL != old) 
    {
        AMA_DEBUG(("AMA: Removing old source: %x\n", old));
        MessageStreamTaskFromSource(old, NULL);
    }

    /* changing source? */
    if (NULL != new_source)
    {
        AMA_DEBUG(("AMA: Adding new source: %x\n", new_source));
        MessageStreamTaskFromSource(new_source, AmaLibGetTask());
        SourceConfigure(new_source, VM_SOURCE_MESSAGES, VM_MESSAGES_SOME);
    }
    voice_source = new_source;
}

/***************************************************************************/
static void handleMsgMoreData(MessageMoreData *message)
{
    if(message)
    {
        if(amaValidStateToSendSpeech())
        {
            if (!amaSendMsbcSourceSpeechData(message->source))
            {
                sendMessageMoreData(AmaLibGetTask(), message->source, 50);
            }
        }
        else
        {
            SourceDrop(message->source, SourceSize(message->source));
        }
    }
}

static void handleTmDisconnectedCfm(TRANSPORT_MGR_LINK_DISCONNECTED_CFM_T *cfm)
{
    switch(cfm->link_cfg.type)
    {
        case transport_mgr_type_rfcomm:
            amaRfcommLinkDisconnectedCfm(cfm);
            amaDisconnect();
            break;

        case transport_mgr_type_accessory:
            amaDisconnect();
            break;

        default:
            break;
    }
}

static void handleTmLinkCreated(TRANSPORT_MGR_LINK_CREATED_CFM_T *cfm)
{
    switch(cfm->link_cfg.type)
    {

        case transport_mgr_type_accessory:
            {
                AmaTransportSwitch(ama_transport_iap);
                amaSendSinkMessage(AMA_SEND_TRANSPORT_VERSION_ID, NULL);
            }
            break;

        case transport_mgr_type_rfcomm:
            amaRfcommLinkCreatedCfm(cfm);
            break;

        default:
            /* Shouldn't get this for another transport */
            Panic();
            break;
    }
}

static void handleTmMoreData(TRANSPORT_MGR_MORE_DATA_T *message)
{
    uint16 len;

    transport_mgr_type_t type = message->type;
    uint16 link_info = message->trans_link_info;

    while((len = TransportMgrGetAvailableDataSize(type, link_info))> 0)
    {
        uint8* s = (uint8*)TransportMgrReadData(type, link_info);

        amaTransportParseData((uint8*)s, len);

        TransportMgrDataConsumed(type, link_info, len);
    }
}

static void handleTmRegisterCfm(TRANSPORT_MGR_REGISTER_CFM_T  *cfm)
{
    AMA_DEBUG(("TRANSPORT_MGR_REGISTER_CFM s=%u t=%u l=%u\n",
           cfm->status,
           cfm->link_cfg.type,
           cfm->link_cfg.trans_info.non_gatt_trans.trans_link_id));
    if (cfm->status)
    {
        switch (cfm->link_cfg.type)
        {
            case transport_mgr_type_rfcomm:
                amaRegisterRfCommSdp(cfm->link_cfg.trans_info.non_gatt_trans.trans_link_id);
                break;

            case transport_mgr_type_accessory:
                break;

            default:
                Panic();
                break;
        }
    }
}

static void amaMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);

    switch(id)
    {
        case MESSAGE_MORE_DATA:
            handleMsgMoreData((MessageMoreData *) message);
            break;

        case VA_AUDIO_MGR_CAPTURE_IND:
            {
                VA_AUDIO_MGR_CAPTURE_IND_T* ind = (VA_AUDIO_MGR_CAPTURE_IND_T*)message;
                AMA_DEBUG(("AMA VA_AUDIO_MGR_CAPTURE_IND\n"));
                AmaInitSetSpeechDataSource(ind->src);
            }
            break;

        case AUDIO_VA_INDICATE_DATA_SOURCE:
            {
                AUDIO_VA_INDICATE_DATA_SOURCE_T* ind = (AUDIO_VA_INDICATE_DATA_SOURCE_T*)message;
                AMA_DEBUG(("AMA VA_INDICATE_DATA_SOURCE\n"));
                AmaInitSetSpeechDataSource(ind->data_src);
            }
            break;

        case TRANSPORT_MGR_REGISTER_CFM:
            AMA_DEBUG(("AMA TRANSPORT_MGR_REGISTER_CFM\n"));
            handleTmRegisterCfm((TRANSPORT_MGR_REGISTER_CFM_T *) message);
            break;

        case TRANSPORT_MGR_LINK_DISCONNECTED_CFM:
            AMA_DEBUG(("AMA TRANSPORT_MGR_LINK_DISCONNECTED_CFM\n"));
            handleTmDisconnectedCfm((TRANSPORT_MGR_LINK_DISCONNECTED_CFM_T *) message);
            break;
            
        case TRANSPORT_MGR_LINK_CREATED_CFM:
            AMA_DEBUG(("AMA TRANSPORT_MGR_LINK_CREATED_CFM\n"));
            handleTmLinkCreated((TRANSPORT_MGR_LINK_CREATED_CFM_T *) message);
            break;
            
        case TRANSPORT_MGR_MORE_DATA:
            AMA_DEBUG(("AMA TRANSPORT_MGR_MORE_DATA\n"));
            handleTmMoreData((TRANSPORT_MGR_MORE_DATA_T *) message);
            break;

        case TRANSPORT_MGR_MORE_SPACE:
            break;

        case VA_AUDIO_MGR_WAKEWORD_IND:
            {
                VA_AUDIO_MGR_WAKEWORD_IND_T *ind = (VA_AUDIO_MGR_WAKEWORD_IND_T*)message;
                AMA_DEBUG(("AMA VA_AUDIO_MGR_WAKEWORD_IND\n"));
                amaHandleWakeWordInd(ind);
            }
            break;

        default:
            AMA_DEBUG(("AMA unh 0x%04X\n", id));
            break;
    }
}

ama_device_config_t * amaGetDeviceConfiguration(void)
{
    return ama_device_config;
}

bool AmaInit(Task application_task,
                 const va_audio_mgr_feature_config_t *feature_config,
                 const va_audio_mgr_audio_config_t *audio_config,
                 const ama_device_config_t *device_config)
{
    AMA_DEBUG(("AMA AmaInit\n"));

    if( (NULL == application_task) || (NULL == feature_config) || (NULL == audio_config) )
        return FALSE;

    app_task = application_task;

    pre_roll = feature_config->pre_roll_interval;

    amaSetSource(NULL);

    AmaTransportInit();

    if(TRUE != AmaRfCommInit(AmaLibGetTask(), NULL) )
        return FALSE;
    
    AmaIap2Init(AmaLibGetTask());
    
    if(TRUE != VaAudioMgrInit(AmaLibGetTask()) )
        return FALSE;

    if(va_audio_mgr_status_success != VaAudioMgrConfigure(feature_config, audio_config) )
        return FALSE;

    amaInitDeviceInformation(device_config);

    amaSpeechMessage(ama_speech_msg_initialise, NULL);

    amaStateInit();

    return TRUE;
}

void amaDisconnect(void)
{
    AMA_DEBUG(("AMA Disconnect\n"));

    amaSpeechMessage(ama_speech_msg_initialise, NULL);

    AmaTransportSwitch(ama_transport_ble);

    AmaTransportResetParseState();
    amaSetSource(NULL);
    amaSendSinkMessage(AMA_START_ADVERTISING_AMA_IND, NULL);

}

static void sendMessageMoreData(Task task, Source src, uint32 delay)
{
    MessageMoreData *message = PanicUnlessMalloc(sizeof(MessageMoreData));
    message->source = src;
    MessageCancelAll(task, MESSAGE_MORE_DATA);
    MessageSendLater(task, MESSAGE_MORE_DATA, message, delay);
}


void AmaInitSetSpeechDataSource(Source src)
{
    uint16 source_size;

    amaSetSource(src);

    source_size = SourceSize(src);

    AMA_DEBUG(("AMA Data already in source %d\n", source_size));

    /* If this already contains data we may have missed the message 
       so repost it*/
    if(source_size)
    {
        sendMessageMoreData(AmaLibGetTask(), src, 0);
    }
}


static void amaHandleWakeWordInd(VA_AUDIO_MGR_WAKEWORD_IND_T *ind)
{
    if(ind)
    {
        ama_speech_msg_start_t message;
        message.initiator = ama_speech_initiator_wakeword;
        message.pre_roll = pre_roll;
        message.start_timestamp = ind->start_timestamp;
        message.end_timestamp = ind->end_timestamp;
        amaSpeechMessage(ama_speech_msg_start, &message);
    }
}

void amaMediaControl( AMA_MEDIA_CONTROL  control)
{
    UNUSED(control);
}


bool AmaParseData(const uint8* stream, uint16 size)
{
    return amaTransportParseData(stream, size);
}


void amaSendSinkMessage(ama_message_type_t id, void* data)
{
    AMA_DEBUG(("AMA Send Sink msg %d\n", id));
    if(app_task)
    {
        MessageSend(app_task, id, data);
    }
}

bool AmaStartSpeech(ama_speech_initiator_t initiator)
{
    ama_speech_msg_start_t message;
    message.initiator = initiator;
    return amaSpeechMessage(ama_speech_msg_start, &message);
}

void AmaEndSpeech(void)
{
    AMA_DEBUG(("AMA AmaEndSpeech \n"));
    amaSpeechMessage(ama_speech_msg_end, NULL);
}

void AmaStopSpeech(bool send_stop)
{
    AMA_DEBUG(("AMA AmaStopSpeech\n"));

    if(send_stop)
    {
        ama_speech_msg_stop_t message;
        message.send_end = TRUE;
        amaSpeechMessage(ama_speech_msg_stop, &message);
    }
    else
    {
        amaSpeechMessage(ama_speech_msg_stop, NULL);
    }
}

void AmaSuspendSpeech(void)
{
    AMA_DEBUG(("AMA AmaSuspendSpeech\n"));
    amaSpeechMessage(ama_speech_msg_suspend, NULL);
}
void AmaResumeSpeech(void)
{
    AMA_DEBUG(("AMA AmaResumeSpeech\n"));
    amaSpeechMessage(ama_speech_msg_resume, NULL);
}

void AmaResetTransportState(void)
{
    AMA_DEBUG(("AMA AmaResetTransportParseState\n"));
    AmaTransportResetParseState();
}

void AmaResetRfcommLink(const bdaddr *bd_addr)
{
    AMA_DEBUG(("AMA AmaDisconnectLink\n"));

    /* This tells the phone app to close the RFCOMM link */
    if(AmaTransportIsBtClassic() && BdaddrIsSame(amaGetRfCommPeerAddress(), bd_addr))
    {
        amaSendCommandResetConnection(0, TRUE);
    }
}


/*!
\copyright  Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_anc_tuning.c
\brief
*/

#include "av_headset.h"
#include "av_headset_kymera.h"
#include "av_headset_log.h"

#include <usb_device_class.h>
#include <usb_hub.h>
#include <audio.h>
#include <audio_anc_tuning.h>
#include <audio_clock.h>
#include <audio_power.h>

/* Unit/Terminal IDs */
#define USB_AUDIO_SPEAKER_IT  0x01
#define USB_AUDIO_SPEAKER_FU  0x02
#define USB_AUDIO_SPEAKER_OT  0x03
#define USB_AUDIO_MIC_IT      0x04
#define USB_AUDIO_MIC_FU      0x05
#define USB_AUDIO_MIC_OT      0x06

#define USB_AUDIO_SAMPLE_RATE                       (48000UL)

#define USB_AUDIO_CHANNELS_MIC                      2
#define USB_AUDIO_CHANNELS_SPEAKER                  2

#if USB_AUDIO_CHANNELS_SPEAKER == 2
#define USB_AUDIO_CHANNEL_CONFIG_SPEAKER            3
#else
#define USB_AUDIO_CHANNEL_CONFIG_SPEAKER            1
#endif

#if USB_AUDIO_CHANNELS_MIC == 2
#define USB_AUDIO_CHANNEL_CONFIG_MIC                3
#else
#define USB_AUDIO_CHANNEL_CONFIG_MIC                1
#endif

#define USB_AUDIO_PACKET_RATE_HZ                    1000
#define USB_AUDIO_SAMPLE_SIZE                       2 /* 2 -> 16bit audio, 3 -> 24bit audio */


/** Round up _value to the nearest _multiple. */
#define ROUNDUP(_value, _multiple) (((_value) + (_multiple) - 1) / (_multiple))

#define MAX_PACKET_SIZE(_sample_rate_hz, _channels)         \
    (ROUNDUP(_sample_rate_hz, USB_AUDIO_PACKET_RATE_HZ) *   \
     USB_AUDIO_SAMPLE_SIZE * (_channels))

#define USB_AUDIO_MAX_PACKET_SIZE                                   \
    MAX_PACKET_SIZE(USB_AUDIO_SAMPLE_RATE, USB_AUDIO_CHANNELS_MIC)


/** USB Audio Class Descriptor: control interfaces */
static const uint8 usb_interface_descriptor_control_mic_and_speaker[] =
{
    /* Class Specific Header */
    0x0A,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x01,           /* bDescriptorSubType = HEADER */
    0x00, 0x01,     /* bcdADC = Audio Device Class v1.00 */
    /* wTotalLength LSB */
    0x0A + 0x0c + (0x08 + USB_AUDIO_CHANNELS_SPEAKER) +
    0x09 + 0x0c + (0x08 + USB_AUDIO_CHANNELS_MIC) + 0x09,
    0x00,           /* wTotalLength MSB */
    0x02,           /* bInCollection = 2 AudioStreaming interfaces */
    0x01,           /* baInterfaceNr(1) - AS#1 id */
    0x02,           /* baInterfaceNr(2) - AS#2 id */

    /* Speaker IT */
    0x0c,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x02,           /* bDescriptorSubType = INPUT_TERMINAL */
    USB_AUDIO_SPEAKER_IT,     /* bTerminalID */
    0x01, 0x01,     /* wTerminalType = USB streaming */
    0x00,           /* bAssocTerminal = none */
    USB_AUDIO_CHANNELS_SPEAKER, /* bNrChannels */
    USB_AUDIO_CHANNEL_CONFIG_SPEAKER & 0xFF,
    /*lint -e{572}
    * we don't care that USB_AUDIO_CHANNEL_CONFIG_SPEAKER_STEREO >> 8 == 0 */
    USB_AUDIO_CHANNEL_CONFIG_SPEAKER >> 8,   /* wChannelConfig */
    0x00,           /* iChannelName = no string */
    0x00,           /* iTerminal = same as USB product string */

    /* Speaker Features */
    0x08 + USB_AUDIO_CHANNELS_SPEAKER,           /*bLength*/
    0x24,           /*bDescriptorType = CS_INTERFACE */
    0x06,           /*bDescriptorSubType = FEATURE_UNIT*/
    USB_AUDIO_SPEAKER_FU,     /*bUnitId*/
    USB_AUDIO_SPEAKER_IT,           /*bSourceId - Speaker IT*/
    0x01,           /*bControlSize = 1 byte per control*/
    0x03,           /*bmaControls[0] = 03 (Master Channel - mute and volume)*/
    0x00,           /*bmaControls[1] = 00 (Logical Channel 1 - nothing)*/
#if USB_AUDIO_CHANNELS_SPEAKER > 1
    0x00,           /*bmaControls[2] = 00 (Logical Channel 2 - nothing)*/
#endif
    0x00,           /*iFeature = same as USB product string*/

    /* Speaker OT */
    0x09,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x03,           /* bDescriptorSubType = OUTPUT_TERMINAL */
    USB_AUDIO_SPEAKER_OT,     /* bTerminalID */
    0x01, 0x03,     /* wTerminalType = Speaker */
    0x00,           /* bAssocTerminal = none */
    USB_AUDIO_SPEAKER_FU,     /* bSourceID - Speaker Features */
    0x00,           /* iTerminal = same as USB product string */

    /* Microphone IT */
    0x0c,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x02,           /* bDescriptorSubType = INPUT_TERMINAL */
    USB_AUDIO_MIC_IT,         /* bTerminalID */
    0x01, 0x02,     /* wTerminalType = Microphone */
    0x00,           /* bAssocTerminal = none */
    USB_AUDIO_CHANNELS_MIC, /* bNrChannels */
    USB_AUDIO_CHANNEL_CONFIG_MIC & 0xFF,
    /*lint -e{572}
     * we don't care that USB_AUDIO_CHANNEL_CONFIG_MIC_STEREO >> 8 == 0 */
    USB_AUDIO_CHANNEL_CONFIG_MIC >> 8,   /* wChannelConfig */
    0x00,           /* iChannelName = no string */
    0x00,           /* iTerminal = same as USB product string */

    /* Microphone Features */
    0x08 + USB_AUDIO_CHANNELS_MIC, /*bLength*/
    0x24,           /*bDescriptorType = CS_INTERFACE */
    0x06,           /*bDescriptorSubType = FEATURE_UNIT*/
    USB_AUDIO_MIC_FU,         /*bUnitId*/
    USB_AUDIO_MIC_IT,         /*bSourceId - Microphone IT*/
    0x01,           /*bControlSize = 1 byte per control*/
    0x02,           /*bmaControls[0] = 02 (Master Channel - volume)*/
    0x00,           /*bmaControls[1] = 00 (Logical Channel 1 - nothing)*/
#if USB_AUDIO_CHANNELS_MIC > 1
    0x00,           /*bmaControls[2] = 00 (Logical Channel 2 - nothing)*/
#endif
    0x00,           /*iFeature = same as USB product string*/

    /* Microphone OT */
    0x09,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x03,           /* bDescriptorSubType = OUTPUT_TERMINAL */
    USB_AUDIO_MIC_OT,         /* bTerminalID */
    0x01, 0x01,     /* wTerminalType = USB streaming */
    0x00,           /* bAssocTerminal = none */
    USB_AUDIO_MIC_FU,         /* bSourceID - Microphone Features */
    0x00            /* iTerminal = same as USB product string */
};

/** USB Audio Class Descriptor: streaming microphone interface */
static const uint8 usb_interface_descriptor_streaming_mic[] =
{
    /* Class Specific AS interface descriptor */
    0x07,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x01,           /* bDescriptorSubType = AS_GENERAL */
    USB_AUDIO_MIC_OT,         /* bTerminalLink = Microphone OT */
    0x00,           /* bDelay */
    0x01, 0x00,     /* wFormatTag = PCM */

    /* Type 1 format type descriptor */
    0x08 + 0x03,    /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x02,           /* bDescriptorSubType = FORMAT_TYPE */
    0x01,           /* bFormatType = FORMAT_TYPE_I */
    USB_AUDIO_CHANNELS_MIC,                         /* bNumberOfChannels */
    USB_AUDIO_SAMPLE_SIZE,                          /* bSubframeSize */
    USB_AUDIO_SAMPLE_SIZE * 8,                      /* bBitsResolution */
    0x01,           /* bSampleFreqType = number discrete sampling freq */
    0xFF & (USB_AUDIO_SAMPLE_RATE),      /* tSampleFreq */
    0xFF & (USB_AUDIO_SAMPLE_RATE >> 8), /* tSampleFreq */
    0xFF & (USB_AUDIO_SAMPLE_RATE >> 16),/* tSampleFreq */

    /* Class specific AS isochronous audio data endpoint descriptor */
    0x07,           /* bLength */
    0x25,           /* bDescriptorType = CS_ENDPOINT */
    0x01,           /* bDescriptorSubType = AS_GENERAL */
    0x00,           /* bmAttributes = none */
    0x02,           /* bLockDelayUnits = Decoded PCM samples */
    0x00, 0x00      /* wLockDelay */
};

/** USB Audio Class Descriptor: streaming speaker interface */
static const uint8 usb_interface_descriptor_streaming_speaker[] =
{
    /* Class Specific AS interface descriptor */
    0x07,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x01,           /* bDescriptorSubType = AS_GENERAL */
    USB_AUDIO_SPEAKER_IT,     /* bTerminalLink = Speaker IT */
    0x00,           /* bDelay */
    0x01, 0x00,     /* wFormatTag = PCM */

    /* Type 1 format type descriptor */
    0x08 + 0x03,    /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x02,           /* bDescriptorSubType = FORMAT_TYPE */
    0x01,           /* bFormatType = FORMAT_TYPE_I */
    USB_AUDIO_CHANNELS_SPEAKER,                  /* bNumberOfChannels */
    USB_AUDIO_SAMPLE_SIZE,                       /* bSubframeSize */
    USB_AUDIO_SAMPLE_SIZE * 8,                   /* bBitsResolution */
    0x01,           /* bSampleFreqType = number discrete sampling freq */
    0xFF & USB_AUDIO_SAMPLE_RATE,     /* tSampleFreq = 48000*/
    0xFF & (USB_AUDIO_SAMPLE_RATE >> 8),
    0xFF & (USB_AUDIO_SAMPLE_RATE >> 16),


    /* Class specific AS isochronous audio data endpoint descriptor */
    0x07,           /* bLength */
    0x25,           /* bDescriptorType = CS_ENDPOINT */
    0x01,           /* bDescriptorSubType = AS_GENERAL */
    0x81,           /* bmAttributes = MaxPacketsOnly and SamplingFrequency control */
    0x02,           /* bLockDelayUnits = Decoded PCM samples */
    0x00, 0x00      /* wLockDelay */
};

static const uint8 audio_endpoint_user_data[] =
{
    0, /* bRefresh */
    0  /* bSyncAddress */
};

/** Microphone streaming isochronous endpoint */
static const EndPointInfo usb_epinfo_streaming_mic[] =
{
    {
        end_point_iso_out,                          /* address */
        end_point_attr_iso,                         /* attributes */
        USB_AUDIO_MAX_PACKET_SIZE,                  /* max packet size */
        1,                                          /* poll_interval */
        audio_endpoint_user_data,                   /* data to be appended */
        sizeof(audio_endpoint_user_data),           /* length of data appended */
    }
};

/** Speaker streaming isochronous endpoint */
static const EndPointInfo usb_epinfo_streaming_speaker[] =
{
    {
        end_point_iso_in,                           /* address */
        end_point_attr_iso,                         /* attributes */
        USB_AUDIO_MAX_PACKET_SIZE,                  /* max packet size */
        1,                                          /* poll_interval */
        audio_endpoint_user_data,                   /* data to be appended */
        sizeof(audio_endpoint_user_data)            /* length of data appended */
    }
};


static usb_device_class_audio_config usb_config_stereo_mic_and_mono_speaker =
{
    {usb_interface_descriptor_control_mic_and_speaker,
    sizeof(usb_interface_descriptor_control_mic_and_speaker),
    NULL},
    {usb_interface_descriptor_streaming_mic,
    sizeof(usb_interface_descriptor_streaming_mic),
    usb_epinfo_streaming_mic},
    {usb_interface_descriptor_streaming_speaker,
    sizeof(usb_interface_descriptor_streaming_speaker),
    usb_epinfo_streaming_speaker}
};


static void appAncTuningHandleMessage(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    switch (id)
    {
        case USB_DEVICE_CLASS_MSG_SPEAKER_SAMPLE_RATE_IND:
            DEBUG_LOG("USB_DEVICE_CLASS_MSG_SPEAKER_SAMPLE_RATE_IND");
            break;

        case USB_DEVICE_CLASS_MSG_MIC_SAMPLE_RATE_IND:
            DEBUG_LOG("USB_DEVICE_CLASS_MSG_MIC_SAMPLE_RATE_IND");
            break;

        case USB_DEVICE_CLASS_MSG_AUDIO_LEVELS_IND:
            DEBUG_LOG("USB_DEVICE_CLASS_MSG_AUDIO_LEVELS_IND");
            break;

        default:
            DEBUG_LOGF("appAncTuningHandleMessage, id %x", id);
            break;
    }
}

void appAncTuningEarlyInit(void)
{
    ancTuningTaskData *theAncTuning = appGetAncTuning();
    theAncTuning->task.handler = appAncTuningHandleMessage;

    if (appConfigAncTuningEnabled())
    {
        usb_device_class_status status;

        status = UsbDeviceClassConfigure(USB_DEVICE_CLASS_CONFIG_AUDIO_INTERFACE_DESCRIPTORS, 0, 0, (const uint8 *)&usb_config_stereo_mic_and_mono_speaker);
        if (status != usb_device_class_status_success)
            Panic();

        status = UsbDeviceClassEnumerate(&theAncTuning->task, USB_DEVICE_CLASS_TYPE_AUDIO_MICROPHONE | USB_DEVICE_CLASS_TYPE_AUDIO_SPEAKER);
        if (status != usb_device_class_status_success)
            Panic();
    }
}


void appAncTuningEnable(bool enable)
{
    ancTuningTaskData *theAncTuning = appGetAncTuning();
    if (enable)
    {
        DEBUG_LOG("appAncTuningEnable, enabling");
        if (appKymeraAncIsEnabled())
        {
            DEBUG_LOG("appAncTuningEnable, disabling ANC");
            appKymeraAncDisable();
        }

        /* Attach USB endpoints to hub */
        UsbHubAttach();
    }
    else
    {
        DEBUG_LOG("appAncTuningEnable, disabling");

        /* Detach USB endpoints from hub */
        UsbHubDetach();

        /* Check if ANC tuning is currently active */
        if (theAncTuning->active)
        {
            /* Clear flag indicating ANC tuning chain is in-active */
            theAncTuning->active = FALSE;

            /* Stop ANC tuning chain */
            appKymeraAncTuningStop();
        }
    }
    theAncTuning->enabled = enable;
}

void appAncTuningEnumerated(void)
{
    ancTuningTaskData *theAncTuning = appGetAncTuning();
    if (theAncTuning->enabled && !theAncTuning->active)
    {
        /* Set flag indicating ANC tuning chain is active */
        theAncTuning->active = TRUE;

        /* Start ANC tuning chain */
        appKymeraAncTuningStart(USB_AUDIO_SAMPLE_RATE);
    }
}

void appAncTuningSuspended(void)
{
    ancTuningTaskData *theAncTuning = appGetAncTuning();
    if (theAncTuning->active)
    {
        /* Clear flag indicating ANC tuning chain is in-active */
        theAncTuning->active = FALSE;

        /* Stop ANC tuning chain */
        appKymeraAncTuningStop();
    }
}


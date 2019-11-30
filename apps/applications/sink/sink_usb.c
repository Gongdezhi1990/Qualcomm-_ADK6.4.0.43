/*
Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    Application level implementation of USB features

NOTES
    - Conditional on ENABLE_USB define
    - Ensure USB host interface and VM control of USB enabled in project/PS
*/

#include <string.h>

#ifdef ENABLE_USB

#include "sink_main_task.h"
#include "sink_debug.h"
#include "sink_powermanager.h"
#include "sink_usb.h"
#include "sink_configmanager.h"
#include "sink_audio.h"
#include "sink_a2dp.h"
#include "sink_avrcp.h"
#include "sink_statemanager.h"
#include "sink_pio.h"
#include "sink_tones.h"
#include "sink_display.h"
#include "sink_volume.h"
#include "ahi_host_usb.h"
#include "sink_ahi.h"
#include "sink_audio_routing.h"
#include "sink_ba.h"
#include "sink_ba_broadcaster.h"
#include "sink_malloc_debug.h"
#include "sink_bootmode.h"
#include "sink_devicemanager.h"

#ifdef ENABLE_SUBWOOFER
#include "sink_swat.h"
#endif

#include "sink_config.h"
#include "sink_development.h"

#include <usb_device_class.h>
#include <power.h>
#include <panic.h>
#include <print.h>
#include <usb.h>
#include <file.h>
#include <stream.h>
#include <source.h>
#include <boot.h>
#include <charger.h>
#include <audio_plugin_voice_variants.h>
#include <audio_plugin_output_variants.h>

#include "sink_usb_config_def.h"
#include "sink_usb_charger_config_def.h"
#if defined ENABLE_CHARGER_V2
#include "sink_usb_proprietary_charger_config_def.h"
#endif

#ifdef ENABLE_USB_HID_UPGRADE
#include <hid_upgrade.h>
#endif

#ifdef ENABLE_USB_HUB_SUPPORT
#include "usb_hub.h"
#endif

#include <ahi.h>
#include <config_store.h>

#ifdef ENABLE_USB_AUDIO

#include <audio.h>
#include <audio_plugin_music_variants.h>

#include "sink_usb_descriptors.h"

/* Reduced delay for audioUpdateAudioRouting to 20ms from 500ms, upon receiving USB
    message type as MESSAGE_USB_ALT_INTERFACE. This is to minimize the delay before USB audio is heard */
#define USB_AUDIO_DISCONNECT_DELAY (20)

#define USB_VOICE_PACKET_RATE_HZ                    1000
#define USB_VOICE_SAMPLE_SIZE                       2 /* 2 -> 16bit audio, 3 -> 24bit audio */
#define USB_VOICE_PACKET_SIZE(channels, sample_rate)          (((sample_rate + USB_VOICE_PACKET_RATE_HZ-1) / USB_VOICE_PACKET_RATE_HZ) * USB_VOICE_SAMPLE_SIZE * channels)

/* Interface Descriptors for mono 8kHz. Do Not Modify. */
static const uint8 interface_descriptor_control_mic_and_speaker_mono[] =
{
    /* Class Specific Header */
    0x0A,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = HEADER */
    0x00, 0x01,   /* bcdADC = Audio Device Class v1.00 */
    0x0A + 0x0c + 0x0b + 0x09 + 0x0c + 0x0b + 0x09, /* wTotalLength LSB */
    0x00,         /* wTotalLength MSB */
    0x02,         /* bInCollection = 2 AudioStreaming interfaces */
    0x01,         /* baInterfaceNr(1) - AS#1 id */
    0x02,         /* baInterfaceNr(2) - AS#2 id */

    /* Microphone IT */
    0x0c,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = INPUT_TERMINAL */
    MIC_IT,       /* bTerminalID */
    0x03, 0x02,   /* wTerminalType = Personal Microphone */
    0x00,         /* bAssocTerminal = none */
    0x01,         /* bNrChannels = 1 */
    0x00, 0x00,   /* wChannelConfig = mono */
    0x00,         /* iChannelName = no string */
    0x00,         /* iTerminal = same as USB product string */

    /* Microphone Features */
    0x0b,           /*bLength*/
    0x24,           /*bDescriptorType = CS_INTERFACE */
    0x06,           /*bDescriptorSubType = FEATURE_UNIT*/
    MIC_FU,         /*bUnitId*/
    MIC_IT,         /*bSourceId - Microphone IT*/
    0x02,           /*bControlSize = 2 bytes per control*/
    0x01, 0x00,     /*bmaControls[0] = 0001 (Mute on Master Channel)*/
    0x00, 0x00,     /*bmaControls[1] = 0000 (No gain control)*/
    0x00,           /*iFeature = same as USB product string*/

    /* Microphone OT */
    0x09,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x03,         /* bDescriptorSubType = OUTPUT_TERMINAL */
    MIC_OT,       /* bTerminalID */
    0x01, 0x01,   /* wTerminalType = USB streaming */
    0x00,         /* bAssocTerminal = none */
    MIC_FU,       /* bSourceID - Microphone Features */
    0x00,         /* iTerminal = same as USB product string */

    /* Speaker IT */
    0x0c,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = INPUT_TERMINAL */
    SPEAKER_IT,   /* bTerminalID */
    0x01, 0x01,   /* wTerminalType = USB streaming */
    0x00,         /* bAssocTerminal = none */
    0x01,         /* bNrChannels = 1 */
    0x00, 0x00,   /* wChannelConfig = mono */
    0x00,         /* iChannelName = no string */
    0x00,         /* iTerminal = same as USB product string */

    /* Speaker Features */
    0x0b,           /*bLength*/
    0x24,           /*bDescriptorType = CS_INTERFACE */
    0x06,           /*bDescriptorSubType = FEATURE_UNIT*/
    SPEAKER_FU,     /*bUnitId*/
    SPEAKER_IT,     /*bSourceId - Speaker IT*/
    0x02,           /*bControlSize = 2 bytes per control*/
    0x01, 0x00,     /*bmaControls[0] = 0001 (Mute on Master Channel)*/
    0x02, 0x00,     /*bmaControls[1] = 0002 (Vol on Left Front)*/
    0x00,           /*iFeature = same as USB product string*/

    /* Speaker OT */
    0x09,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x03,         /* bDescriptorSubType = OUTPUT_TERMINAL */
    SPEAKER_OT,   /* bTerminalID */
    0x01, 0x03,   /* wTerminalType = Speaker */
    0x00,         /* bAssocTerminal = none */
    SPEAKER_FU,   /* bSourceID - Speaker Features*/
    0x00,         /* iTerminal = same as USB product string */
};

static const uint8 interface_descriptor_streaming_mic_mono_nb[] =
{
    /* Class Specific AS interface descriptor */
    0x07,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    MIC_OT,       /* bTerminalLink = Microphone OT */
    0x00,         /* bDelay */
    0x01, 0x00,   /* wFormatTag = PCM */

    /* Type 1 format type descriptor */
    0x08 + 0x03,  /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = FORMAT_TYPE */
    0x01,         /* bFormatType = FORMAT_TYPE_I */
    0x01,         /* bNumberOfChannels */
    0x02,         /* bSubframeSize = 2 bytes */
    0x10,         /* bBitsResolution */
    0x01,         /* bSampleFreqType = 1 discrete sampling freq */
    0xFF & (SAMPLE_RATE_NB),       /* tSampleFreq */
    0xFF & (SAMPLE_RATE_NB >> 8),  /* tSampleFreq */
    0xFF & (SAMPLE_RATE_NB >> 16), /* tSampleFreq */

    /* Class specific AS isochronous audio data endpoint descriptor */
    0x07,         /* bLength */
    0x25,         /* bDescriptorType = CS_ENDPOINT */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    0x00,         /* bmAttributes = none */
    0x02,         /* bLockDelayUnits = Decoded PCM samples */
    0x00, 0x00     /* wLockDelay */
};

static const uint8 interface_descriptor_streaming_speaker_mono_nb[] =
{
    /* Class Specific AS interface descriptor */
    0x07,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    SPEAKER_IT,   /* bTerminalLink = Speaker IT */
    0x00,         /* bDelay */
    0x01, 0x00,   /* wFormatTag = PCM */

    /* Type 1 format type descriptor */
    0x08 + 0x03,/* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = FORMAT_TYPE */
    0x01,         /* bFormatType = FORMAT_TYPE_I */
    0x01,         /* bNumberOfChannels */
    0x02,         /* bSubframeSize = 2 bytes */
    0x10,         /* bBitsResolution */
    0x01,         /* bSampleFreqType = 1 discrete sampling freq */
    0xFF & (SAMPLE_RATE_NB),       /* tSampleFreq */
    0xFF & (SAMPLE_RATE_NB >> 8),  /* tSampleFreq */
    0xFF & (SAMPLE_RATE_NB >> 16), /* tSampleFreq */

    /* Class specific AS isochronous audio data endpoint descriptor */
    0x07,         /* bLength */
    0x25,         /* bDescriptorType = CS_ENDPOINT */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    0x01,         /* bmAttributes = SamplingFrequency control */
    0x02,         /* bLockDelayUnits = Decoded PCM samples */
    0x00, 0x00    /* wLockDelay */
};

static const uint8 audio_endpoint_user_data[] =
{
    0, /* bRefresh */
    0  /* bSyncAddress */
};


/*  Streaming Isochronous Endpoint. Maximum packet size 16 (mono at 8khz) */
static const EndPointInfo epinfo_streaming_speaker_mono_nb[] =
{
    {
        end_point_iso_in, /* address */
        end_point_attr_iso, /* attributes */
        USB_VOICE_PACKET_SIZE(1, SAMPLE_RATE_NB), /* max packet size */
        1, /* poll_interval */
        audio_endpoint_user_data, /* data to be appended */
        sizeof(audio_endpoint_user_data) /* length of data appended */
    }
};


/* Streaming Isochronous Endpoint. Maximum packet size (mono at 8khz) */
static const EndPointInfo epinfo_streaming_mic_mono_nb[] =
{
    {
        end_point_iso_out, /* address */
        end_point_attr_iso, /* attributes */
        USB_VOICE_PACKET_SIZE(1, SAMPLE_RATE_NB), /* max packet size */
        1, /* poll_interval */
        audio_endpoint_user_data, /* data to be appended */
        sizeof(audio_endpoint_user_data), /* length of data appended */
    }
};

static const usb_device_class_audio_config usb_cvc_config =
{
    {interface_descriptor_control_mic_and_speaker_mono,
    sizeof(interface_descriptor_control_mic_and_speaker_mono),
    NULL},
    {interface_descriptor_streaming_mic_mono_nb,
    sizeof(interface_descriptor_streaming_mic_mono_nb),
    epinfo_streaming_mic_mono_nb},
    {interface_descriptor_streaming_speaker_mono_nb,
    sizeof(interface_descriptor_streaming_speaker_mono_nb),
    epinfo_streaming_speaker_mono_nb}
};

static const uint8 interface_descriptor_streaming_mic_mono_wb[] =
{
    /* Class Specific AS interface descriptor */
    0x07,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    MIC_OT,       /* bTerminalLink = Microphone OT */
    0x00,         /* bDelay */
    0x01, 0x00,   /* wFormatTag = PCM */

    /* Type 1 format type descriptor */
    0x08 + 0x03,  /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = FORMAT_TYPE */
    0x01,         /* bFormatType = FORMAT_TYPE_I */
    0x01,         /* bNumberOfChannels */
    0x02,         /* bSubframeSize = 2 bytes */
    0x10,         /* bBitsResolution */
    0x01,         /* bSampleFreqType = 1 discrete sampling freq */
    0xFF & (SAMPLE_RATE_WB),       /* tSampleFreq */
    0xFF & (SAMPLE_RATE_WB >> 8),  /* tSampleFreq */
    0xFF & (SAMPLE_RATE_WB >> 16), /* tSampleFreq */

    /* Class specific AS isochronous audio data endpoint descriptor */
    0x07,         /* bLength */
    0x25,         /* bDescriptorType = CS_ENDPOINT */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    0x00,         /* bmAttributes = none */
    0x02,         /* bLockDelayUnits = Decoded PCM samples */
    0x00, 0x00     /* wLockDelay */
};

static const uint8 interface_descriptor_streaming_speaker_mono_wb[] =
{
    /* Class Specific AS interface descriptor */
    0x07,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    SPEAKER_IT,   /* bTerminalLink = Speaker IT */
    0x00,         /* bDelay */
    0x01, 0x00,   /* wFormatTag = PCM */

    /* Type 1 format type descriptor */
    0x08 + 0x03,/* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = FORMAT_TYPE */
    0x01,         /* bFormatType = FORMAT_TYPE_I */
    0x01,         /* bNumberOfChannels */
    0x02,         /* bSubframeSize = 2 bytes */
    0x10,         /* bBitsResolution */
    0x01,         /* bSampleFreqType = 1 discrete sampling freq */
    0xFF & (SAMPLE_RATE_WB),       /* tSampleFreq */
    0xFF & (SAMPLE_RATE_WB >> 8),  /* tSampleFreq */
    0xFF & (SAMPLE_RATE_WB >> 16), /* tSampleFreq */

    /* Class specific AS isochronous audio data endpoint descriptor */
    0x07,         /* bLength */
    0x25,         /* bDescriptorType = CS_ENDPOINT */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    0x01,         /* bmAttributes = SamplingFrequency control */
    0x02,         /* bLockDelayUnits = Decoded PCM samples */
    0x00, 0x00    /* wLockDelay */
};

/*  Streaming Isochronous Endpoint. Maximum packet size 16 (mono at 8khz) */
static const EndPointInfo epinfo_streaming_speaker_mono_wb[] =
{
    {
        end_point_iso_in, /* address */
        end_point_attr_iso, /* attributes */
        USB_VOICE_PACKET_SIZE(1, SAMPLE_RATE_WB), /* max packet size */
        1, /* poll_interval */
        audio_endpoint_user_data, /* data to be appended */
        sizeof(audio_endpoint_user_data) /* length of data appended */
    }
};


/* Streaming Isochronous Endpoint. Maximum packet size (mono at 16khz) */
static const EndPointInfo epinfo_streaming_mic_mono_wb[] =
{
    {
        end_point_iso_out, /* address */
        end_point_attr_iso, /* attributes */
        USB_VOICE_PACKET_SIZE(1, SAMPLE_RATE_WB), /* max packet size */
        1, /* poll_interval */
        audio_endpoint_user_data, /* data to be appended */
        sizeof(audio_endpoint_user_data), /* length of data appended */
    }
};

static const usb_device_class_audio_config usb_cvc_wb_config =
{
    {interface_descriptor_control_mic_and_speaker_mono,
    sizeof(interface_descriptor_control_mic_and_speaker_mono),
    NULL},
    {interface_descriptor_streaming_mic_mono_wb,
    sizeof(interface_descriptor_streaming_mic_mono_wb),
    epinfo_streaming_mic_mono_wb},
    {interface_descriptor_streaming_speaker_mono_wb,
    sizeof(interface_descriptor_streaming_speaker_mono_wb),
    epinfo_streaming_speaker_mono_wb}
};


static const uint8 interface_descriptor_streaming_mic_mono_uwb[] =
{
    /* Class Specific AS interface descriptor */
    0x07,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    MIC_OT,       /* bTerminalLink = Microphone OT */
    0x00,         /* bDelay */
    0x01, 0x00,   /* wFormatTag = PCM */

    /* Type 1 format type descriptor */
    0x08 + 0x03,  /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = FORMAT_TYPE */
    0x01,         /* bFormatType = FORMAT_TYPE_I */
    0x01,         /* bNumberOfChannels */
    0x02,         /* bSubframeSize = 2 bytes */
    0x10,         /* bBitsResolution */
    0x01,         /* bSampleFreqType = 1 discrete sampling freq */
    0xFF & (SAMPLE_RATE_UWB),       /* tSampleFreq */
    0xFF & (SAMPLE_RATE_UWB >> 8),  /* tSampleFreq */
    0xFF & (SAMPLE_RATE_UWB >> 16), /* tSampleFreq */

    /* Class specific AS isochronous audio data endpoint descriptor */
    0x07,         /* bLength */
    0x25,         /* bDescriptorType = CS_ENDPOINT */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    0x00,         /* bmAttributes = none */
    0x02,         /* bLockDelayUnits = Decoded PCM samples */
    0x00, 0x00     /* wLockDelay */
};

static const uint8 interface_descriptor_streaming_speaker_mono_uwb[] =
{
    /* Class Specific AS interface descriptor */
    0x07,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    SPEAKER_IT,   /* bTerminalLink = Speaker IT */
    0x00,         /* bDelay */
    0x01, 0x00,   /* wFormatTag = PCM */

    /* Type 1 format type descriptor */
    0x08 + 0x03,/* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = FORMAT_TYPE */
    0x01,         /* bFormatType = FORMAT_TYPE_I */
    0x01,         /* bNumberOfChannels */
    0x02,         /* bSubframeSize = 2 bytes */
    0x10,         /* bBitsResolution */
    0x01,         /* bSampleFreqType = 1 discrete sampling freq */
    0xFF & (SAMPLE_RATE_UWB),       /* tSampleFreq */
    0xFF & (SAMPLE_RATE_UWB >> 8),  /* tSampleFreq */
    0xFF & (SAMPLE_RATE_UWB >> 16), /* tSampleFreq */

    /* Class specific AS isochronous audio data endpoint descriptor */
    0x07,         /* bLength */
    0x25,         /* bDescriptorType = CS_ENDPOINT */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    0x81,         /* bmAttributes = MaxPacketsOnly and SamplingFrequency control */
    0x02,         /* bLockDelayUnits = Decoded PCM samples */
    0x00, 0x00    /* wLockDelay */
};

/*  Streaming Isochronous Endpoint. Maximum packet size 16 (mono at 8khz) */
static const EndPointInfo epinfo_streaming_speaker_mono_uwb[] =
{
    {
        end_point_iso_in, /* address */
        end_point_attr_iso, /* attributes */
        USB_VOICE_PACKET_SIZE(1, SAMPLE_RATE_UWB), /* max packet size */
        1, /* poll_interval */
        audio_endpoint_user_data, /* data to be appended */
        sizeof(audio_endpoint_user_data) /* length of data appended */
    }
};


/* Streaming Isochronous Endpoint. Maximum packet size (mono at 16khz) */
static const EndPointInfo epinfo_streaming_mic_mono_uwb[] =
{
    {
        end_point_iso_out, /* address */
        end_point_attr_iso, /* attributes */
        USB_VOICE_PACKET_SIZE(1, SAMPLE_RATE_UWB), /* max packet size */
        1, /* poll_interval */
        audio_endpoint_user_data, /* data to be appended */
        sizeof(audio_endpoint_user_data), /* length of data appended */
    }
};

static const usb_device_class_audio_config usb_cvc_uwb_config =
{
    {interface_descriptor_control_mic_and_speaker_mono,
    sizeof(interface_descriptor_control_mic_and_speaker_mono),
    NULL},
    {interface_descriptor_streaming_mic_mono_uwb,
    sizeof(interface_descriptor_streaming_mic_mono_uwb),
    epinfo_streaming_mic_mono_uwb},
    {interface_descriptor_streaming_speaker_mono_uwb,
    sizeof(interface_descriptor_streaming_speaker_mono_uwb),
    epinfo_streaming_speaker_mono_uwb}
};


static const uint8 interface_descriptor_streaming_mic_mono_swb[] =
{
    /* Class Specific AS interface descriptor */
    0x07,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    MIC_OT,       /* bTerminalLink = Microphone OT */
    0x00,         /* bDelay */
    0x01, 0x00,   /* wFormatTag = PCM */

    /* Type 1 format type descriptor */
    0x08 + 0x03,  /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = FORMAT_TYPE */
    0x01,         /* bFormatType = FORMAT_TYPE_I */
    0x01,         /* bNumberOfChannels */
    0x02,         /* bSubframeSize = 2 bytes */
    0x10,         /* bBitsResolution */
    0x01,         /* bSampleFreqType = 1 discrete sampling freq */
    0xFF & (SAMPLE_RATE_SWB),       /* tSampleFreq */
    0xFF & (SAMPLE_RATE_SWB >> 8),  /* tSampleFreq */
    0xFF & (SAMPLE_RATE_SWB >> 16), /* tSampleFreq */

    /* Class specific AS isochronous audio data endpoint descriptor */
    0x07,         /* bLength */
    0x25,         /* bDescriptorType = CS_ENDPOINT */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    0x00,         /* bmAttributes = none */
    0x02,         /* bLockDelayUnits = Decoded PCM samples */
    0x00, 0x00     /* wLockDelay */
};

static const uint8 interface_descriptor_streaming_speaker_mono_swb[] =
{
    /* Class Specific AS interface descriptor */
    0x07,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    SPEAKER_IT,   /* bTerminalLink = Speaker IT */
    0x00,         /* bDelay */
    0x01, 0x00,   /* wFormatTag = PCM */

    /* Type 1 format type descriptor */
    0x08 + 0x03,/* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = FORMAT_TYPE */
    0x01,         /* bFormatType = FORMAT_TYPE_I */
    0x01,         /* bNumberOfChannels */
    0x02,         /* bSubframeSize = 2 bytes */
    0x10,         /* bBitsResolution */
    0x01,         /* bSampleFreqType = 1 discrete sampling freq */
    0xFF & (SAMPLE_RATE_SWB),       /* tSampleFreq */
    0xFF & (SAMPLE_RATE_SWB >> 8),  /* tSampleFreq */
    0xFF & (SAMPLE_RATE_SWB >> 16), /* tSampleFreq */

    /* Class specific AS isochronous audio data endpoint descriptor */
    0x07,         /* bLength */
    0x25,         /* bDescriptorType = CS_ENDPOINT */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    0x81,         /* bmAttributes = MaxPacketsOnly and SamplingFrequency control */
    0x02,         /* bLockDelayUnits = Decoded PCM samples */
    0x00, 0x00    /* wLockDelay */
};

/*  Streaming Isochronous Endpoint. Maximum packet size 16 (mono at 8khz) */
static const EndPointInfo epinfo_streaming_speaker_mono_swb[] =
{
    {
        end_point_iso_in, /* address */
        end_point_attr_iso, /* attributes */
        USB_VOICE_PACKET_SIZE(1, SAMPLE_RATE_SWB), /* max packet size */
        1, /* poll_interval */
        audio_endpoint_user_data, /* data to be appended */
        sizeof(audio_endpoint_user_data) /* length of data appended */
    }
};


/* Streaming Isochronous Endpoint. Maximum packet size (mono at 16khz) */
static const EndPointInfo epinfo_streaming_mic_mono_swb[] =
{
    {
        end_point_iso_out, /* address */
        end_point_attr_iso, /* attributes */
        USB_VOICE_PACKET_SIZE(1, SAMPLE_RATE_SWB), /* max packet size */
        1, /* poll_interval */
        audio_endpoint_user_data, /* data to be appended */
        sizeof(audio_endpoint_user_data), /* length of data appended */
    }
};

static const usb_device_class_audio_config usb_cvc_swb_config =
{
    {interface_descriptor_control_mic_and_speaker_mono,
    sizeof(interface_descriptor_control_mic_and_speaker_mono),
    NULL},
    {interface_descriptor_streaming_mic_mono_swb,
    sizeof(interface_descriptor_streaming_mic_mono_swb),
    epinfo_streaming_mic_mono_swb},
    {interface_descriptor_streaming_speaker_mono_swb,
    sizeof(interface_descriptor_streaming_speaker_mono_swb),
    epinfo_streaming_speaker_mono_swb}
};


static const usb_device_class_audio_volume_config usb_stereo_audio_volume =
{
    SPEAKER_VOLUME_MIN,
    SPEAKER_VOLUME_MAX,
    SPEAKER_VOLUME_RESOLUTION,
    SPEAKER_VOLUME_DEFAULT,
    MICROPHONE_VOLUME_MIN,
    MICROPHONE_VOLUME_MAX,
    MICROPHONE_VOLUME_RESOLUTION,
    MICROPHONE_VOLUME_DEFAULT
};

/* Don't all APP stereo descriptors in case Broadcast Audio is enabled, as we have a limitation to support only
 * 48KHz sampling frequency */
#if defined(ENABLE_USB_AUDIO_APP_STEREO_DESCRIPTORS) && !defined(ENABLE_BROADCAST_AUDIO)


#define SAMPLE_RATE_48K     ((uint32) 48000)
#define SAMPLE_RATE_44K1    ((uint32) 44100)
#define SAMPLE_RATE_32K     ((uint32) 32000)
#define SAMPLE_RATE_22K05   ((uint32) 22050)
#define SAMPLE_RATE_16K     ((uint32) 16000)
#define SAMPLE_RATE_8K      ((uint32) 8000)

#define NUMBER_MIC_SAMPLE_RATES                     1
#define NUMBER_SPEAKER_SAMPLE_RATES                 6

#define USB_AUDIO_MAX_SAMPLE_RATE_MIC_STEREO        SAMPLE_RATE_48K
#define USB_AUDIO_MAX_SAMPLE_RATE_SPEAKER_STEREO    SAMPLE_RATE_48K

#define USB_AUDIO_CHANNELS_SPEAKER_STEREO           2
#define USB_AUDIO_CHANNELS_MIC_STEREO               2

#if USB_AUDIO_CHANNELS_SPEAKER_STEREO == 2
#define USB_AUDIO_CHANNEL_CONFIG_SPEAKER_STEREO     3
#else
#define USB_AUDIO_CHANNEL_CONFIG_SPEAKER_STEREO     1
#endif

#if USB_AUDIO_CHANNELS_MIC_STEREO == 2
#define USB_AUDIO_CHANNEL_CONFIG_MIC_STEREO         3
#else
#define USB_AUDIO_CHANNEL_CONFIG_MIC_STEREO         1
#endif

#define USB_AUDIO_PACKET_RATE_HZ                    1000
#define USB_AUDIO_SAMPLE_SIZE_STEREO                2 /* 2 -> 16bit audio, 3 -> 24bit audio */

#define USB_AUDIO_MAX_PACKET_SIZE_MIC_STEREO        (((USB_AUDIO_MAX_SAMPLE_RATE_MIC_STEREO+USB_AUDIO_PACKET_RATE_HZ-1)/USB_AUDIO_PACKET_RATE_HZ)*USB_AUDIO_SAMPLE_SIZE_STEREO*USB_AUDIO_CHANNELS_MIC_STEREO)
#define USB_AUDIO_MAX_PACKET_SIZE_SPEAKER_STEREO    (((USB_AUDIO_MAX_SAMPLE_RATE_SPEAKER_STEREO+USB_AUDIO_PACKET_RATE_HZ-1)/USB_AUDIO_PACKET_RATE_HZ)*USB_AUDIO_SAMPLE_SIZE_STEREO*USB_AUDIO_CHANNELS_SPEAKER_STEREO)

/** USB Audio Class Descriptor: control interfaces */
static const uint8 usb_interface_descriptor_control_mic_and_speaker_stereo[] =
{
    /* Class Specific Header */
    0x0A,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x01,           /* bDescriptorSubType = HEADER */
    0x00, 0x01,     /* bcdADC = Audio Device Class v1.00 */
    /* wTotalLength LSB */
    0x0A + 0x0c + (0x08 + USB_AUDIO_CHANNELS_SPEAKER_STEREO) +
        0x09 + 0x0c + (0x08 + USB_AUDIO_CHANNELS_MIC_STEREO) + 0x09,
    0x00,           /* wTotalLength MSB */
    0x02,           /* bInCollection = 2 AudioStreaming interfaces */
    0x01,           /* baInterfaceNr(1) - AS#1 id */
    0x02,           /* baInterfaceNr(2) - AS#2 id */

    /* Speaker IT */
    0x0c,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x02,           /* bDescriptorSubType = INPUT_TERMINAL */
    SPEAKER_IT,     /* bTerminalID */
    0x01, 0x01,     /* wTerminalType = USB streaming */
    0x00,           /* bAssocTerminal = none */
    USB_AUDIO_CHANNELS_SPEAKER_STEREO, /* bNrChannels */
    USB_AUDIO_CHANNEL_CONFIG_SPEAKER_STEREO & 0xFF,
    /*lint -e{572}
    * we don't care that USB_AUDIO_CHANNEL_CONFIG_SPEAKER_STEREO >> 8 == 0 */
    USB_AUDIO_CHANNEL_CONFIG_SPEAKER_STEREO >> 8,   /* wChannelConfig */
    0x00,           /* iChannelName = no string */
    0x00,           /* iTerminal = same as USB product string */

    /* Speaker Features */
    0x08 + USB_AUDIO_CHANNELS_SPEAKER_STEREO,           /*bLength*/
    0x24,           /*bDescriptorType = CS_INTERFACE */
    0x06,           /*bDescriptorSubType = FEATURE_UNIT*/
    SPEAKER_FU,     /*bUnitId*/
    SPEAKER_IT,           /*bSourceId - Speaker IT*/
    0x01,           /*bControlSize = 1 byte per control*/
    0x03,           /*bmaControls[0] = 03 (Master Channel - mute and volume)*/
    0x00,           /*bmaControls[1] = 00 (Logical Channel 1 - nothing)*/
#if USB_AUDIO_CHANNELS_SPEAKER_STEREO > 1
    0x00,           /*bmaControls[2] = 00 (Logical Channel 2 - nothing)*/
#endif
    0x00,           /*iFeature = same as USB product string*/

    /* Speaker OT */
    0x09,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x03,           /* bDescriptorSubType = OUTPUT_TERMINAL */
    SPEAKER_OT,     /* bTerminalID */
    0x01, 0x03,     /* wTerminalType = Speaker */
    0x00,           /* bAssocTerminal = none */
    SPEAKER_FU,     /* bSourceID - Speaker Features */
    0x00,           /* iTerminal = same as USB product string */

    /* Microphone IT */
    0x0c,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x02,           /* bDescriptorSubType = INPUT_TERMINAL */
    MIC_IT,         /* bTerminalID */
    0x01, 0x02,     /* wTerminalType = Microphone */
    0x00,           /* bAssocTerminal = none */
    USB_AUDIO_CHANNELS_MIC_STEREO, /* bNrChannels */
    USB_AUDIO_CHANNEL_CONFIG_MIC_STEREO & 0xFF,
    /*lint -e{572}
     * we don't care that USB_AUDIO_CHANNEL_CONFIG_MIC_STEREO >> 8 == 0 */
    USB_AUDIO_CHANNEL_CONFIG_MIC_STEREO >> 8,   /* wChannelConfig */
    0x00,           /* iChannelName = no string */
    0x00,           /* iTerminal = same as USB product string */

    /* Microphone Features */
    0x08 + USB_AUDIO_CHANNELS_MIC_STEREO, /*bLength*/
    0x24,           /*bDescriptorType = CS_INTERFACE */
    0x06,           /*bDescriptorSubType = FEATURE_UNIT*/
    MIC_FU,         /*bUnitId*/
    MIC_IT,         /*bSourceId - Microphone IT*/
    0x01,           /*bControlSize = 1 byte per control*/
    0x02,           /*bmaControls[0] = 02 (Master Channel - volume)*/
    0x00,           /*bmaControls[1] = 00 (Logical Channel 1 - nothing)*/
#if USB_AUDIO_CHANNELS_MIC_STEREO > 1
    0x00,           /*bmaControls[2] = 00 (Logical Channel 2 - nothing)*/
#endif
    0x00,           /*iFeature = same as USB product string*/

    /* Microphone OT */
    0x09,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x03,           /* bDescriptorSubType = OUTPUT_TERMINAL */
    MIC_OT,         /* bTerminalID */
    0x01, 0x01,     /* wTerminalType = USB streaming */
    0x00,           /* bAssocTerminal = none */
    MIC_FU,         /* bSourceID - Microphone Features */
    0x00            /* iTerminal = same as USB product string */
};

static const uint8 usb_interface_descriptor_control_speaker_stereo[] =
{
    /* Class Specific Header */
    0x09,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = HEADER */
    0x00, 0x01, /* bcdADC = Audio Device Class v1.00 */
    0x09 + 0x0c + (0x08 + USB_AUDIO_CHANNELS_SPEAKER_STEREO) + 0x09, /* wTotalLength LSB */
    0x00,         /* wTotalLength MSB */
    0x01,         /* bInCollection = 1 AudioStreaming interface */
    0x01,         /* baInterfaceNr(1) - AS#1 id */

    /* Speaker IT */
    0x0c,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = INPUT_TERMINAL */
    SPEAKER_IT,   /* bTerminalID */
    0x01, 0x01,   /* wTerminalType = USB streaming */
    0x00,         /* bAssocTerminal = none */
    USB_AUDIO_CHANNELS_SPEAKER_STEREO, /* bNrChannels */
    USB_AUDIO_CHANNEL_CONFIG_SPEAKER_STEREO & 0xFF,
    /*lint -e{572}
    * we don't care that USB_AUDIO_CHANNEL_CONFIG_SPEAKER_STEREO >> 8 == 0 */
    USB_AUDIO_CHANNEL_CONFIG_SPEAKER_STEREO >> 8,   /* wChannelConfig */
    0x00,         /* iChannelName = no string */
    0x00,         /* iTerminal = same as USB product string */

    /* Speaker Features */
    0x08 + USB_AUDIO_CHANNELS_SPEAKER_STEREO,           /*bLength*/
    0x24,           /*bDescriptorType = CS_INTERFACE */
    0x06,           /*bDescriptorSubType = FEATURE_UNIT*/
    SPEAKER_FU,     /*bUnitId*/
    SPEAKER_IT,     /*bSourceId - Speaker IT*/
    0x01,           /*bControlSize = 1 byte per control*/
    0x03,           /*bmaControls[0] = 03 (Master Channel - mute and volume)*/
    0x00,           /*bmaControls[1] = 00 (Logical Channel 1 - nothing)*/
#if USB_AUDIO_CHANNELS_SPEAKER_STEREO > 1
    0x00,           /*bmaControls[2] = 00 (Logical Channel 2 - nothing)*/
#endif
    0x00,           /*iFeature = same as USB product string*/

    /* Speaker OT */
    0x09,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x03,         /* bDescriptorSubType = OUTPUT_TERMINAL */
    SPEAKER_OT,   /* bTerminalID */
    0x01, 0x03,   /* wTerminalType = Speaker */
    0x00,         /* bAssocTerminal = none */
    SPEAKER_FU,   /* bSourceID - Speaker Features*/
    0x00,         /* iTerminal = same as USB product string */
};

static const uint8 usb_interface_descriptor_control_mic_stereo[] =
{
    /* Class Specific Header */
    0x09,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x01,           /* bDescriptorSubType = HEADER */
    0x00, 0x01,     /* bcdADC = Audio Device Class v1.00 */
    /* wTotalLength LSB */
    0x0A + 0x0c + (0x08 + USB_AUDIO_CHANNELS_MIC_STEREO) + 0x09,
    0x00,           /* wTotalLength MSB */
    0x01,           /* bInCollection = 1 AudioStreaming interface */
    0x01,           /* baInterfaceNr(1) - AS#1 id */

    /* Microphone IT */
    0x0c,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x02,           /* bDescriptorSubType = INPUT_TERMINAL */
    MIC_IT,         /* bTerminalID */
    0x01, 0x02,     /* wTerminalType = Microphone */
    0x00,           /* bAssocTerminal = none */
    USB_AUDIO_CHANNELS_MIC_STEREO, /* bNrChannels */
    USB_AUDIO_CHANNEL_CONFIG_MIC_STEREO & 0xFF,
    /*lint -e{572}
     * we don't care that USB_AUDIO_CHANNEL_CONFIG_MIC_STEREO >> 8 == 0 */
    USB_AUDIO_CHANNEL_CONFIG_MIC_STEREO >> 8,   /* wChannelConfig */
    0x00,           /* iChannelName = no string */
    0x00,           /* iTerminal = same as USB product string */

    /* Microphone Features */
    0x08 + USB_AUDIO_CHANNELS_MIC_STEREO, /*bLength*/
    0x24,           /*bDescriptorType = CS_INTERFACE */
    0x06,           /*bDescriptorSubType = FEATURE_UNIT*/
    MIC_FU,         /*bUnitId*/
    MIC_IT,         /*bSourceId - Microphone IT*/
    0x01,           /*bControlSize = 1 byte per control*/
    0x02,           /*bmaControls[0] = 02 (Master Channel - volume)*/
    0x00,           /*bmaControls[1] = 00 (Logical Channel 1 - nothing)*/
#if USB_AUDIO_CHANNELS_MIC_STEREO > 1
    0x00,           /*bmaControls[2] = 00 (Logical Channel 2 - nothing)*/
#endif
    0x00,           /*iFeature = same as USB product string*/

    /* Microphone OT */
    0x09,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x03,           /* bDescriptorSubType = OUTPUT_TERMINAL */
    MIC_OT,         /* bTerminalID */
    0x01, 0x01,     /* wTerminalType = USB streaming */
    0x00,           /* bAssocTerminal = none */
    MIC_FU,         /* bSourceID - Microphone Features */
    0x00            /* iTerminal = same as USB product string */
};


/** USB Audio Class Descriptor: streaming microphone interface */
static const uint8 usb_interface_descriptor_streaming_mic_stereo[] =
{
    /* Class Specific AS interface descriptor */
    0x07,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x01,           /* bDescriptorSubType = AS_GENERAL */
    MIC_OT,         /* bTerminalLink = Microphone OT */
    0x00,           /* bDelay */
    0x01, 0x00,     /* wFormatTag = PCM */

    /* Type 1 format type descriptor */
    0x08 + 0x03*NUMBER_MIC_SAMPLE_RATES,    /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x02,           /* bDescriptorSubType = FORMAT_TYPE */
    0x01,           /* bFormatType = FORMAT_TYPE_I */
    USB_AUDIO_CHANNELS_MIC_STEREO,                  /* bNumberOfChannels */
    USB_AUDIO_SAMPLE_SIZE_STEREO,                   /* bSubframeSize */
    USB_AUDIO_SAMPLE_SIZE_STEREO * 8,               /* bBitsResolution */
    NUMBER_MIC_SAMPLE_RATES,                        /* bSampleFreqType = number discrete sampling freq */
    0xFF & (SAMPLE_RATE_48K),      /* tSampleFreq */
    0xFF & (SAMPLE_RATE_48K >> 8), /* tSampleFreq */
    0xFF & (SAMPLE_RATE_48K >> 16),/* tSampleFreq */

    /* Class specific AS isochronous audio data endpoint descriptor */
    0x07,           /* bLength */
    0x25,           /* bDescriptorType = CS_ENDPOINT */
    0x01,           /* bDescriptorSubType = AS_GENERAL */
    0x00,           /* bmAttributes = none */
    0x02,           /* bLockDelayUnits = Decoded PCM samples */
    0x00, 0x00      /* wLockDelay */
};

/** USB Audio Class Descriptor: streaming speaker interface */
static const uint8 usb_interface_descriptor_streaming_speaker_stereo[] =
{
    /* Class Specific AS interface descriptor */
    0x07,           /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x01,           /* bDescriptorSubType = AS_GENERAL */
    SPEAKER_IT,     /* bTerminalLink = Speaker IT */
    0x00,           /* bDelay */
    0x01, 0x00,     /* wFormatTag = PCM */

    /* Type 1 format type descriptor */
    0x08 + 0x03*NUMBER_SPEAKER_SAMPLE_RATES,  /* bLength */
    0x24,           /* bDescriptorType = CS_INTERFACE */
    0x02,           /* bDescriptorSubType = FORMAT_TYPE */
    0x01,           /* bFormatType = FORMAT_TYPE_I */
    USB_AUDIO_CHANNELS_SPEAKER_STEREO,                  /* bNumberOfChannels */
    USB_AUDIO_SAMPLE_SIZE_STEREO,                       /* bSubframeSize */
    USB_AUDIO_SAMPLE_SIZE_STEREO * 8,                   /* bBitsResolution */
    NUMBER_SPEAKER_SAMPLE_RATES,/* bSampleFreqType = number discrete sampling freq */
    SAMPLE_RATE_48K & 0xff,     /* tSampleFreq = 48000*/
    (SAMPLE_RATE_48K >> 8) & 0xff,
    (SAMPLE_RATE_48K >> 16) & 0xff,
    SAMPLE_RATE_44K1 & 0xff,    /* tSampleFreq = 44100*/
    (SAMPLE_RATE_44K1 >> 8) & 0xff,
    (SAMPLE_RATE_44K1 >> 16) & 0xff,
    SAMPLE_RATE_32K & 0xff,     /* tSampleFreq = 32000 */
    (SAMPLE_RATE_32K >> 8 ) & 0xff,
    (SAMPLE_RATE_32K >> 16) & 0xff,
    SAMPLE_RATE_22K05 & 0xff,   /* tSampleFreq = 22050 */
    (SAMPLE_RATE_22K05 >> 8 ) & 0xff,
    (SAMPLE_RATE_22K05 >> 16) & 0xff,
    SAMPLE_RATE_16K & 0xff,     /* tSampleFreq = 16000 */
    (SAMPLE_RATE_16K >> 8 ) & 0xff,
    (SAMPLE_RATE_16K >> 16) & 0xff,
    SAMPLE_RATE_8K & 0xff,      /* tSampleFreq = 8000 */
    (SAMPLE_RATE_8K >> 8) & 0xff,
    (SAMPLE_RATE_8K >> 16) & 0xff,


    /* Class specific AS isochronous audio data endpoint descriptor */
    0x07,           /* bLength */
    0x25,           /* bDescriptorType = CS_ENDPOINT */
    0x01,           /* bDescriptorSubType = AS_GENERAL */
    0x01,           /* bmAttributes = SamplingFrequency control */
    0x02,           /* bLockDelayUnits = Decoded PCM samples */
    0x00, 0x00      /* wLockDelay */
};

/** Speaker streaming isochronous endpoint */
static const EndPointInfo usb_epinfo_streaming_speaker_stereo[] =
{
    {
        end_point_iso_in,                           /* address */
        end_point_attr_iso,                         /* attributes */
        USB_AUDIO_MAX_PACKET_SIZE_SPEAKER_STEREO,   /* max packet size */
        1,                                          /* poll_interval */
        audio_endpoint_user_data,                   /* data to be appended */
        sizeof(audio_endpoint_user_data)            /* length of data appended */
    }
};

/** Microphone streaming isochronous endpoint */
static const EndPointInfo usb_epinfo_streaming_mic_stereo[] =
{
    {
        end_point_iso_out,                          /* address */
        end_point_attr_iso,                         /* attributes */
        USB_AUDIO_MAX_PACKET_SIZE_MIC_STEREO,       /* max packet size */
        1,                                          /* poll_interval */
        audio_endpoint_user_data,                   /* data to be appended */
        sizeof(audio_endpoint_user_data),           /* length of data appended */
    }
};


static usb_device_class_audio_config usb_stereo_config_mic_and_speaker =
{
    {usb_interface_descriptor_control_mic_and_speaker_stereo,
    sizeof(usb_interface_descriptor_control_mic_and_speaker_stereo),
    NULL},
    {usb_interface_descriptor_streaming_mic_stereo,
    sizeof(usb_interface_descriptor_streaming_mic_stereo),
    usb_epinfo_streaming_mic_stereo},
    {usb_interface_descriptor_streaming_speaker_stereo,
    sizeof(usb_interface_descriptor_streaming_speaker_stereo),
    usb_epinfo_streaming_speaker_stereo}
};

static usb_device_class_audio_config usb_stereo_config_mic =
{
    {usb_interface_descriptor_control_mic_stereo,
    sizeof(usb_interface_descriptor_control_mic_stereo),
    NULL},
    {usb_interface_descriptor_streaming_mic_stereo,
    sizeof(usb_interface_descriptor_streaming_mic_stereo),
    usb_epinfo_streaming_mic_stereo},
    {NULL,
    0,
    NULL}
};

static usb_device_class_audio_config usb_stereo_config_speaker =
{
    {usb_interface_descriptor_control_speaker_stereo,
    sizeof(usb_interface_descriptor_control_speaker_stereo),
    NULL},
    {NULL,
    0,
    NULL},
    {usb_interface_descriptor_streaming_speaker_stereo,
    sizeof(usb_interface_descriptor_streaming_speaker_stereo),
    usb_epinfo_streaming_speaker_stereo}
};
#endif /*ENABLE_USB_AUDIO_APP_STEREO_DESCRIPTORS*/


static const usb_plugin_info usb_plugins[] =
{
/* Don't all APP stereo descriptors in case Broadcast Audio is enabled, as we have a limitation to support only
 * 48KHz sampling frequency */
#if defined(ENABLE_USB_AUDIO_APP_STEREO_DESCRIPTORS) && !defined(ENABLE_BROADCAST_AUDIO)
    {SAMPLE_RATE_STEREO, &usb_stereo_config_mic_and_speaker, &usb_stereo_config_mic, &usb_stereo_config_speaker, usb_plugin_stereo},
#else
    {SAMPLE_RATE_STEREO, NULL, NULL, NULL, usb_plugin_stereo},
#endif /*ENABLE_USB_AUDIO_APP_STEREO_DESCRIPTORS*/    
    {SAMPLE_RATE_NB, &usb_cvc_config, &usb_cvc_config, &usb_cvc_config, usb_plugin_mono_nb},
    {SAMPLE_RATE_WB, &usb_cvc_wb_config, &usb_cvc_wb_config, &usb_cvc_wb_config, usb_plugin_mono_wb},
    {SAMPLE_RATE_UWB, &usb_cvc_uwb_config, &usb_cvc_uwb_config, &usb_cvc_uwb_config, usb_plugin_mono_uwb},
    {SAMPLE_RATE_SWB, &usb_cvc_swb_config, &usb_cvc_swb_config, &usb_cvc_swb_config, usb_plugin_mono_swb}
};


#endif /* ENABLE_USB_AUDIO */

#ifdef DEBUG_USB
    #define USB_DEBUG(x) DEBUG(x)
#else
    #define USB_DEBUG(x)
#endif

typedef struct
{
    FILE_INDEX  index;
    uint32      size;
} usb_file_info;

#define USB_NAME_SIZE 8
#define USB_EXT_SIZE  3

typedef struct
{
    char name[USB_NAME_SIZE];
    char ext[USB_EXT_SIZE];
} usb_file_name;

typedef struct
{
    char  name[USB_NAME_SIZE + USB_EXT_SIZE + 1];
    uint8 size;
} usb_file_name_info;

#define USB_CLASS_ENABLED(x)        ((sinkUsbconfigGetDeviceClassType() & (x)) == (x))
#define USB_CLASS_DISABLE(x)        sinkUsbconfigDisableDeviceClass(x)
#define USB_CLASS_ENABLE(x)         sinkUsbconfigEnableDeviceClass(x)
#define USB_ALL_CLASSES_DISABLED    sinkUsbconfigGetDeviceClassType() == 0

#ifdef HAVE_VBAT_SEL
#define usbDeadBatteryAtBoot()      (PowerBatteryGetStatusAtBoot() == CHARGER_BATTERY_DEAD)
#else
#define usbDeadBatteryAtBoot()      (TRUE) /* Assume dead battery until initial VBAT reading taken */
#endif

#ifdef ENABLE_CHARGER_V2
/* USB Dead Battery Provision is not supported. */
#define usbDeadBatteryProvision()   (sinkUsbIsDeadBattery())
#else
#define usbDeadBatteryProvision()   (sinkUsbIsDeadBattery() && !sinkUsbIsEnumerated() && !sinkUsbIsDeconfigured())
#endif

const char root_name[] = "usb_root";
const char fat_name[]  = "usb_fat";

/* Define some easier to read values... */
#define USB_PLAY_PAUSE  USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_PLAY_PAUSE
#define USB_STOP        USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_STOP
#define USB_FWD         USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_NEXT_TRACK
#define USB_BCK         USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_PREVIOUS_TRACK
#define USB_VOL_UP      USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_VOL_UP
#define USB_VOL_DN      USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_VOL_DOWN
#define USB_MUTE        USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_MUTE

static void sinkUsbConfigInit(void);
static void sinkUsbVolumeEvent(sinkEvents_t event);

void sinkUsbconfigDisableDeviceClass(usb_device_class_type x);
void sinkUsbconfigEnableDeviceClass(usb_device_class_type x);

#if defined(ENABLE_USB_HID_UPGRADE) || defined(ENABLE_AHI_USB_HOST)
static void usbHostHandleMessage(MessageId id, Message message);
#endif

#ifdef ENABLE_USB_AUDIO
static void usbAudioSetVolume(void);
#else
#define usbAudioSetVolume() ((void)(0))
#endif

#ifdef ENABLE_USB_AUDIO
static bool usbMicOrSpeakerEnabled(void);
#else
#define usbMicOrSpeakerEnabled() ((void)(0))
#endif

#ifdef ENABLE_USB_AUDIO
static void usbAudioSetSpeakerSampleRate(uint32 sample_rate);
#else
#define usbAudioSetSpeakerSampleRate(x) ((void)0)
#endif

#ifdef ENABLE_USB_AUDIO
static void usbAudioSetMicSampleRate(uint32 sample_rate);
#else
#define usbAudioSetMicSampleRate(x) ((void)0)
#endif

#ifdef ENABLE_USB_AUDIO
static void usbSendHidEvent(usb_device_class_event event);
#else
#define usbSendHidEvent(x) ((void)(0))
#endif

#define usbPlayPause()  usbSendHidEvent(USB_PLAY_PAUSE)
#define usbStop()       usbSendHidEvent(USB_STOP)
#define usbFwd()        usbSendHidEvent(USB_FWD)
#define usbBck()        usbSendHidEvent(USB_BCK)
#define usbVolUp()      usbSendHidEvent(USB_VOL_UP)
#define usbVolDn()      usbSendHidEvent(USB_VOL_DN)
#define usbMute()       usbSendHidEvent(USB_MUTE)

#ifdef ENABLE_USB_AUDIO
#define sinkUsbVolumeIsAtMinimum()  (USB_RUNDATA.out_l_vol == (int16)(int8)((int16)SPEAKER_VOLUME_MIN >> 8))
#else
#define sinkUsbVolumeIsAtMinimum()  (FALSE)
#endif

#ifdef ENABLE_USB_AUDIO
#define sinkUsbVolumeIsAtMaximum()  (USB_RUNDATA.out_l_vol == (int16)(int8)((int16)SPEAKER_VOLUME_MAX >> 8))
#else
#define sinkUsbVolumeIsAtMaximum()  (FALSE)
#endif

static usb_info USB_RUNDATA;


/*************************************************************************
NAME
    sinkUsbGetSessionData

DESCRIPTION
    Get the USB session data from config store.

RETURNS
    void

**************************************************************************/
void sinkUsbGetSessionData(void)
{
    sink_usb_writeable_config_def_t *write_configdata;

    if (configManagerGetReadOnlyConfig(SINK_USB_WRITEABLE_CONFIG_BLK_ID, (const void **)&write_configdata))
    {
        USB_RUNDATA.volume.main_volume = write_configdata->usb_main_volume;
        USB_RUNDATA.volume.aux_volume = write_configdata->usb_aux_volume;
        configManagerReleaseConfig(SINK_USB_WRITEABLE_CONFIG_BLK_ID);
    }
}

/*************************************************************************
NAME
    sinkUsbSetSessionData

DESCRIPTION
    Set the USB session data from config store.

RETURNS
    void

**************************************************************************/
void sinkUsbSetSessionData(void)
{
    sink_usb_writeable_config_def_t *write_configdata;

    if (configManagerGetWriteableConfig(SINK_USB_WRITEABLE_CONFIG_BLK_ID, (void **)&write_configdata, 0))
    {
        write_configdata->usb_main_volume = USB_RUNDATA.volume.main_volume;
        write_configdata->usb_aux_volume = USB_RUNDATA.volume.aux_volume;
        configManagerUpdateWriteableConfig(SINK_USB_WRITEABLE_CONFIG_BLK_ID);
    }
}
/*************************************************************************
NAME
    sinkUsbSetMainVolume

DESCRIPTION
    Set the usb main volume.

RETURNS
    void

**************************************************************************/
void sinkUsbSetMainVolume(int16 usb_volume)
{
     USB_RUNDATA.volume.main_volume = usb_volume;
}
/*************************************************************************
NAME
    sinkUsbGetUsbVolume

DESCRIPTION
    Get the usb volume.

RETURNS
    void

**************************************************************************/
void sinkUsbGetUsbVolume(volume_info *usb_volume)
{
    *usb_volume = USB_RUNDATA.volume;
}
/*************************************************************************
NAME    
    sinkUsbSetUsbVolume

DESCRIPTION
    Set the usb volume.

RETURNS
    void

**************************************************************************/
void sinkUsbSetUsbVolume(const volume_info usb_volume)
{
     USB_RUNDATA.volume = usb_volume;
}
/*************************************************************************
NAME
    sinkUsbconfigDisableDeviceClass

DESCRIPTION
    Disable one or more USB Device Classes

RETURNS
    void

**************************************************************************/
void sinkUsbconfigDisableDeviceClass(usb_device_class_type x)
{
     USB_RUNDATA.device_class &= ~x;
}
/*************************************************************************
NAME
    sinkUsbconfigEnableDeviceClass

DESCRIPTION
    Enable one or more USB Device Classes

RETURNS
    void

**************************************************************************/
void sinkUsbconfigEnableDeviceClass(usb_device_class_type x)
{
     USB_RUNDATA.device_class |= x;
}
/*************************************************************************
NAME
    usbUpdateConnectorInformation

DESCRIPTION
    Store the voltages on the USB data lines, rounded to nearest 100mV.

RETURNS
    void
**************************************************************************/
#ifdef ENABLE_CHARGER_V2
void usbUpdateConnectorInformation(usb_attached_status attached_status, uint16 dp, uint16 dm, usb_type_c_advertisement cc_status)
{
    USB_RUNDATA.attached_status = attached_status;
    USB_RUNDATA.data_plus       = dp;
    USB_RUNDATA.data_minus      = dm;
#ifdef ENABLE_USB_TYPE_C
    USB_RUNDATA.cc_status       = cc_status;
#else
    UNUSED(cc_status);
#endif
}
#endif
/*************************************************************************
NAME
    sinkUsbconfigIsAudioAlwaysOn

DESCRIPTION
    Get the state of usb audio on.

RETURNS
    bool

**************************************************************************/
bool sinkUsbconfigIsAudioAlwaysOn(void)
{
    bool audio_always_on = FALSE;
    sink_usb_readonly_config_def_t *usb_data;

    if (configManagerGetReadOnlyConfig(SINK_USB_READONLY_CONFIG_BLK_ID, (const void **)&usb_data))
    {
        audio_always_on = usb_data->audio_always_on;
        configManagerReleaseConfig(SINK_USB_READONLY_CONFIG_BLK_ID);
    }

    return audio_always_on;
}
/*************************************************************************
NAME
    sinkUsbconfigGetPauseWhenSwitchingSource

DESCRIPTION
    Get the state of Pause When Switching USB source.

RETURNS
    bool

**************************************************************************/
bool sinkUsbconfigGetPauseWhenSwitchingSource(void)
{
    bool pause_when_switching_source = FALSE;

    sink_usb_readonly_config_def_t *usb_data;

    if (configManagerGetReadOnlyConfig(SINK_USB_READONLY_CONFIG_BLK_ID, (const void **)&usb_data))
    {
        pause_when_switching_source = usb_data->pause_when_switching_source;
        configManagerReleaseConfig(SINK_USB_READONLY_CONFIG_BLK_ID);
    }

    return pause_when_switching_source;
}
/*************************************************************************
NAME
    sinkUsbconfigGetPluginType

DESCRIPTION
    Get USB Plugin type

RETURNS
    usb_plugin_type

**************************************************************************/
usb_plugin_type sinkUsbconfigGetPluginType(void)
{
    usb_plugin_type plugin_type = 0;
    sink_usb_readonly_config_def_t *usb_data;

    if (configManagerGetReadOnlyConfig(SINK_USB_READONLY_CONFIG_BLK_ID, (const void **)&usb_data))
    {
        plugin_type = usb_data->plugin_type;
        configManagerReleaseConfig(SINK_USB_READONLY_CONFIG_BLK_ID);
    }

    return plugin_type;
}
/*************************************************************************
NAME
    sinkUsbconfigGetPluginIndex

DESCRIPTION
    Get USB Plugin index

RETURNS
    The index

**************************************************************************/
uint8 sinkUsbconfigGetPluginIndex(void)
{
    uint8 plugin_index = 0;
    sink_usb_readonly_config_def_t *usb_data;

    if (configManagerGetReadOnlyConfig(SINK_USB_READONLY_CONFIG_BLK_ID, (const void **)&usb_data))
    {
        plugin_index = usb_data->plugin_index;
        configManagerReleaseConfig(SINK_USB_READONLY_CONFIG_BLK_ID);
    }

    return plugin_index;
}

/*************************************************************************
NAME
    sinkUsbGetUsbVolumeIndex

DESCRIPTION
    Get USB Volume index

RETURNS
    The index

**************************************************************************/
uint16 sinkUsbGetUsbVolumeIndex(void)
{
    uint16 volumeIndex;
    volume_info usb_volume;

    sinkUsbGetUsbVolume(&usb_volume);

    volumeIndex = usb_volume.main_volume;

    return volumeIndex;
}

/*************************************************************************
NAME
    sinkUsbconfigGetUsbAttachTimeout

DESCRIPTION
    Get the USB config attach timeout value.

RETURNS
    uint16

**************************************************************************/
uint16 sinkUsbconfigGetUsbAttachTimeout(void)
{
    uint16 timeout = 0;
    sink_usb_readonly_config_def_t *usb_data;

    if (configManagerGetReadOnlyConfig(SINK_USB_READONLY_CONFIG_BLK_ID, (const void **)&usb_data))
    {
        timeout = usb_data->attach_timeout;
        configManagerReleaseConfig(SINK_USB_READONLY_CONFIG_BLK_ID);
    }

    return timeout;
}
/*************************************************************************
NAME
    sinkUsbconfigGetUsbDeconfigTimeout

DESCRIPTION
    Get the USB config deconfig timeout value.

RETURNS
    uint16

**************************************************************************/
uint16 sinkUsbconfigGetUsbDeconfigTimeout(void)
{
    uint16 timeout = 0;

    sink_usb_readonly_config_def_t *usb_data;

    if (configManagerGetReadOnlyConfig(SINK_USB_READONLY_CONFIG_BLK_ID, (const void **)&usb_data))
    {
        timeout = usb_data->deconfigured_timeout;
        configManagerReleaseConfig(SINK_USB_READONLY_CONFIG_BLK_ID);
    }

    return timeout;
}
/*************************************************************************
NAME
    sinkUsbconfigGetDeviceClassType

DESCRIPTION
    Get the usb config device class.

RETURNS
    usb_device_class_type

**************************************************************************/
usb_device_class_type sinkUsbconfigGetDeviceClassType(void)
{
    return USB_RUNDATA.device_class;
}
/*************************************************************************
NAME
    usbSetAudioSuspendState

DESCRIPTION
    Sets the audio suspend state for USB

RETURNS
    None

**************************************************************************/
static void usbSetAudioSuspendState (usb_audio_suspend_state state)
{
    if (state == usb_audio_suspend_none)
    {   /* Returning to the unsuspended state */
        USB_RUNDATA.audio_suspend_state = state;
#ifdef ENABLE_PEER
        peerClaimRelay(TRUE);
#endif
    }
    else
    {   /* Check if we are already suspended before updating state */
        if (sinkUsbAudioIsSuspendedNone())
        {
            USB_RUNDATA.audio_suspend_state = state;
#ifdef ENABLE_PEER
            peerClaimRelay(FALSE);
#endif
        }
    }

    USB_DEBUG(("USB: Setting audio suspend_state = %d\n",USB_RUNDATA.audio_suspend_state));
}


/****************************************************************************
NAME
    usbUpdateChargeCurrent

DESCRIPTION
    Set the charge current based on USB state

RETURNS
    void
*/
void usbUpdateChargeCurrent(void)
{
    /* Don't change anything if battery charging disabled */
    if(USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_BATTERY_CHARGING))
        powerManagerUpdateChargeCurrent();
}


/****************************************************************************
NAME
    usbSetLowPowerMode

DESCRIPTION
    If delay is non zero queue a message to reset into low powered mode. If
    delay is zero do nothing.

RETURNS
    void
*/
static void usbSetLowPowerMode(uint8 delay)
{
    /* Only queue low power mode if not enumerated and attached to normal host/hub */
    if(!sinkUsbIsEnumerated()&& delay && (UsbAttachedStatus() == HOST_OR_HUB))
    {
        USB_DEBUG(("USB: Queue low power in %d sec\n", delay));
        MessageSendLater(&theSink.task, EventUsrUsbLowPowerMode, 0, D_SEC(delay));
    }
}


/****************************************************************************
NAME
    usbSetBootMode

DESCRIPTION
    Set the boot mode to default or low power

RETURNS
    void
*/
void usbSetBootMode(uint8 bootmode)
{
    /* Don't change anything if battery charging disabled */
    if(!USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_BATTERY_CHARGING))
        return;

    if(BootGetMode() != bootmode)
    {
        USB_DEBUG(("USB: Set Mode %d\n", bootmode));
        BootSetMode(bootmode);
    }
}


/****************************************************************************
NAME
    handleUsbMessage

DESCRIPTION
    Handle firmware USB messages

RETURNS
    void
*/
void handleUsbMessage(Task task, MessageId id, Message message)
{
    UNUSED(task);

    USB_DEBUG(("USB: "));
    switch (id)
    {
        case MESSAGE_USB_ATTACHED:
        {
            USB_DEBUG(("MESSAGE_USB_ATTACHED\n"));
            usbUpdateChargeCurrent();
            audioUpdateAudioRouting();
            usbSetLowPowerMode(sinkUsbconfigGetUsbAttachTimeout());
            if(sinkUsbIsDeadBattery())
			{
                MessageSendLater(&theSink.task, EventSysUsbDeadBatteryTimeout, 0, D_MIN(45));
			}
            break;
        }
        case MESSAGE_USB_DETACHED:
        {
            USB_DEBUG(("MESSAGE_USB_DETACHED\n"));
            USB_RUNDATA.enumerated = FALSE;
            USB_RUNDATA.suspended  = FALSE;
            USB_RUNDATA.deconfigured = FALSE;
            usbUpdateChargeCurrent();
            usbSetAudioSuspendState(usb_audio_suspend_remote);
#ifdef ENABLE_PEER
            PEER_UPDATE_REQUIRED_RELAY_STATE("USB DETACHED");
#endif

            audioUpdateAudioRoutingAfterDisconnect();

            MessageCancelAll(&theSink.task, EventUsrUsbLowPowerMode);
            MessageCancelAll(&theSink.task, EventSysUsbDeadBatteryTimeout);
#ifdef ENABLE_PEER
        	{
            	/* The USB has disconnected so notify this to a peer device */
            	sinkAvrcpUpdatePeerSourceDisconnected(USB_AUDIO);
            	USB_RUNDATA.enumerated_spkr = FALSE;
        	}
#endif
            break;
        }
        case MESSAGE_USB_ENUMERATED:
        {
            USB_DEBUG(("MESSAGE_USB_ENUMERATED\n"));
            if(!sinkUsbIsEnumerated())
            {
                USB_RUNDATA.enumerated = TRUE;
                usbUpdateChargeCurrent();
                MessageCancelAll(&theSink.task, EventUsrUsbLowPowerMode);
                MessageCancelAll(&theSink.task, EventSysUsbDeadBatteryTimeout);
            }
            break;
        }
        case MESSAGE_USB_SUSPENDED:
        {
            const MessageUsbSuspended* ind = (const MessageUsbSuspended*)message;
            USB_DEBUG(("MESSAGE_USB_SUSPENDED - %s\n", (ind->has_suspended ? "Suspend" : "Resume")));
            if(ind->has_suspended != sinkUsbIsSuspended())
            {
                USB_RUNDATA.suspended = ind->has_suspended;
                usbUpdateChargeCurrent();
            }

            if (ind->has_suspended)
            {
                usbSetAudioSuspendState(usb_audio_suspend_remote);
#ifdef ENABLE_PEER
                PEER_UPDATE_REQUIRED_RELAY_STATE("USB SUSPENDED");
#endif
            }
            break;
        }
        case MESSAGE_USB_DECONFIGURED:
        {
            USB_DEBUG(("MESSAGE_USB_DECONFIGURED\n"));
            if(sinkUsbIsEnumerated())
            {
                USB_RUNDATA.enumerated = FALSE;
                USB_RUNDATA.deconfigured  = TRUE;
                usbUpdateChargeCurrent();
                usbSetLowPowerMode(sinkUsbconfigGetUsbDeconfigTimeout());
            }
            break;
        }
        case MESSAGE_USB_ALT_INTERFACE:
        {
            uint16 interface_id;
            const MessageUsbAltInterface* ind = (const MessageUsbAltInterface*)message;

            USB_DEBUG(("MESSAGE_USB_ALT_INTERFACE %d %d\n", ind->interface, ind->altsetting));
            UsbDeviceClassGetValue(USB_DEVICE_CLASS_GET_MIC_INTERFACE_ID, &interface_id);
            if(interface_id == ind->interface)
            {
                USB_RUNDATA.mic_active = ind->altsetting ? TRUE : FALSE;
                USB_DEBUG(("USB: Mic ID %d active %d\n", interface_id, sinkUsbIsMicrophoneActive()));
            }
            UsbDeviceClassGetValue(USB_DEVICE_CLASS_GET_SPEAKER_INTERFACE_ID, &interface_id);
            if(interface_id == ind->interface)
            {
                USB_RUNDATA.spkr_active = ind->altsetting ? TRUE : FALSE;
                USB_DEBUG(("USB: Speaker ID %d active %d\n", interface_id, sinkUsbIsSpeakerActive()));

#if defined(ENABLE_PEER) && defined(ENABLE_USB_AUDIO)
				if(USB_RUNDATA.enumerated_spkr == FALSE)
                {
                    /* If USB is used for speaker/headset interface then notify this to a peer device */
                    sinkAvrcpUpdatePeerWiredSourceConnected(USB_AUDIO);
                    USB_RUNDATA.enumerated_spkr = TRUE;
                }
#endif
                
                if (sinkUsbIsSpeakerActive())
                {
                    MessageSend ( &theSink.task , EventSysUsbMediaConnected  , 0 );
                    usbSetAudioSuspendState(usb_audio_suspend_none);
#ifdef ENABLE_PEER
                    PEER_UPDATE_REQUIRED_RELAY_STATE("USB SPEAKER ACTIVE");
#endif
                }
                else
                {
                    MessageSend ( &theSink.task , EventSysUsbMediaDisconnected , 0 ); 
                    usbSetAudioSuspendState(usb_audio_suspend_remote);
#ifdef ENABLE_PEER
                    PEER_UPDATE_REQUIRED_RELAY_STATE("USB SPEAKER INACTIVE");
#endif
                }
            }
#ifdef ENABLE_USB_AUDIO
            /* check for changes in required audio routing */
            USB_DEBUG(("USB: MESSAGE_USB_ALT_INTERFACE checkAudioRouting\n"));
            sinkCancelAndSendLater(EventSysCheckAudioRouting, USB_AUDIO_DISCONNECT_DELAY);
#endif
            break;
        }
        case USB_DEVICE_CLASS_MSG_AUDIO_LEVELS_IND:
        {
            USB_DEBUG(("USB_DEVICE_CLASS_MSG_AUDIO_LEVELS_IND\n"));
            usbAudioSetVolume();
            break;
        }

        /* update of speaker sample rate from usb host */
        case USB_DEVICE_CLASS_MSG_SPEAKER_SAMPLE_RATE_IND:
        {
            USB_DEBUG(("USB_DEVICE_CLASS_MSG_SPEAKER_SAMPLE_RATE_IND: %ld\n",((USB_DEVICE_CLASS_SAMPLE_RATE_T *)message)->sample_rate));
            usbAudioSetSpeakerSampleRate(((USB_DEVICE_CLASS_SAMPLE_RATE_T *)message)->sample_rate);
        }
        break;

        /* update of mic sample rate from usb host */
        case USB_DEVICE_CLASS_MSG_MIC_SAMPLE_RATE_IND:
        {
            USB_DEBUG(("USB_DEVICE_CLASS_MSG_MICR_SAMPLE_RATE_IND: %ld\n",((USB_DEVICE_CLASS_SAMPLE_RATE_T *)message)->sample_rate));
            usbAudioSetMicSampleRate(((USB_DEVICE_CLASS_SAMPLE_RATE_T *)message)->sample_rate);
        }
        break;
#if defined(ENABLE_USB_HID_UPGRADE) || defined(ENABLE_AHI_USB_HOST)
        case USB_DEVICE_CLASS_MSG_REPORT_IND:
        {
            USB_DEBUG(("handleUsbMessage: USB_DEVICE_CLASS_MSG_REPORT_IND\n"));
            usbHostHandleMessage(id, message);
        }
        break;
#endif

        default:
        {
            USB_DEBUG(("Unhandled USB message 0x%x\n", id));
            break;
        }
    }
}

/* AHI uses the DATA_TRANSFER report and nothing else. */
#define HID_REPORTID_DATA_TRANSFER (1)

/****************************************************************************
NAME
    usbHostHandleMessage

DESCRIPTION
    Message handler for messages from the Host.

RETURNS
    void
*/
#if defined(ENABLE_USB_HID_UPGRADE) || defined(ENABLE_AHI_USB_HOST)
static void usbHostHandleMessage(MessageId id, Message message)
{
    USB_DEVICE_CLASS_MSG_REPORT_IND_T* msg = (USB_DEVICE_CLASS_MSG_REPORT_IND_T *) message;

    if(id == USB_DEVICE_CLASS_MSG_REPORT_IND)
    {
        if(msg->report_id == HID_REPORTID_DATA_TRANSFER)
        {
            AhiUsbHostHandleMessage(id, message);
        }
        else
        {
#ifdef ENABLE_USB_HID_UPGRADE
            HidUpgradeSetReportHandler(msg->report_id, msg->size_report, msg->report);
#endif
        }
    }
}
#endif

/****************************************************************************
NAME
    usbFileInfo

DESCRIPTION
    Get file info (index and size) for a given file name.

RETURNS
    void
*/
static void usbFileInfo(const char* name, uint8 size_name, usb_file_info* info)
{
    Source source;
    info->index = FileFind(FILE_ROOT, name, size_name);
    source = StreamFileSource(info->index);
    info->size = SourceSize(source);
    SourceClose(source);
}


/****************************************************************************
NAME
    usbFileName

DESCRIPTION
    Get file name from USB root

RETURNS
    void
*/
static void usbFileName(usb_file_info* root, usb_file_name_info* result)
{
    Source source = StreamFileSource(root->index);
    const usb_file_name* file = (const usb_file_name*)SourceMap(source);

    result->size = 0;

    if(file)
    {
        memmove(result->name, file->name, USB_NAME_SIZE);
        for(result->size = 0; result->size < USB_NAME_SIZE; result->size++)
            if(file->name[result->size] == ' ')
                break;
        *(result->name + result->size) = '.';
        result->size++;
        memmove(result->name + result->size, file->ext, USB_EXT_SIZE);
        result->size += USB_EXT_SIZE;
        SourceClose(source);
    }
#ifdef DEBUG_USB
    {
    uint8 count;
    USB_DEBUG(("USB: File Name "));
    for(count = 0; count < result->size; count++)
        USB_DEBUG(("%c", result->name[count]));
    USB_DEBUG(("\n"));
    }
#endif
}

/****************************************************************************
NAME
    usbTimeCriticalInit

DESCRIPTION
    Initialise USB. This function is time critical and must be called from
    _init. This will fail if either Host Interface is not set to USB or
    VM control of USB is FALSE in PS. It may also fail if Transport in the
    project properties is not set to USB VM.

RETURNS
    void
*/
void usbTimeCriticalInit(void)
{
#ifdef ENABLE_USB_AUDIO
    const usb_plugin_info* plugin;
    const usb_device_class_audio_config* usb_descriptors = NULL;
#endif
    usb_device_class_status status;
    usb_file_info root;
    usb_file_info file;
    usb_file_name_info file_name;

    USB_DEBUG(("USB: Time Critical\n"));

    /* Initialise memory for the USB configuration */
    memset(&USB_RUNDATA, 0, sizeof(usb_info));
    USB_DEBUG(("INIT: Malloc size usb: [%d]\n", sizeof(usb_info)));

    /* Default to not configured or suspended */
    USB_RUNDATA.ready = FALSE;
    USB_RUNDATA.enumerated = FALSE;
    USB_RUNDATA.suspended  = FALSE;
    USB_RUNDATA.vbus_okay  = TRUE;
    USB_RUNDATA.deconfigured = FALSE;
    USB_RUNDATA.audio_suspend_state = usb_audio_suspend_none;
    USB_RUNDATA.attached_status = DETACHED;
#ifdef ENABLE_USB_TYPE_C
    USB_RUNDATA.cc_status = CC_CURRENT_DEFAULT;
#endif

    /* Check if we booted with dead battery */
    usbSetVbatDead(usbDeadBatteryAtBoot());

    /* Get USB configuration */
    sinkUsbConfigInit();

    /* Abort if no device classes supported */
    if(USB_ALL_CLASSES_DISABLED)
    {
        USB_DEBUG(("Aborting USB critical init - no USB device classes enabled\n"));
        return;
    }

    usbFileInfo(root_name, sizeof(root_name)-1, &root);
    usbFileName(&root, &file_name);
    usbFileInfo(file_name.name, file_name.size, &file);

    /* If we can't find the help file don't enumerate mass storage */
    if(file.index == FILE_NONE || root.index == FILE_NONE)
    {
        USB_DEBUG(("USB: Mass Storage device class disabled - No USB configuration files in filesystem!\n"));
        USB_CLASS_DISABLE(USB_DEVICE_CLASS_TYPE_MASS_STORAGE);
    }

#ifdef ENABLE_USB_AUDIO
    plugin = &usb_plugins[sinkUsbconfigGetPluginType()];
    USB_DEBUG(("USB: Audio Plugin %d\n", sinkUsbconfigGetPluginIndex()));
    
    if (USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_AUDIO_MICROPHONE) && USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_AUDIO_SPEAKER))
        usb_descriptors = plugin->usb_descriptors_mic_speaker;
    else if (USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_AUDIO_MICROPHONE))
        usb_descriptors = plugin->usb_descriptors_mic;
    else if (USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_AUDIO_SPEAKER))
        usb_descriptors = plugin->usb_descriptors_speaker;

    status = UsbDeviceClassConfigure(USB_DEVICE_CLASS_CONFIG_AUDIO_INTERFACE_DESCRIPTORS, 0, 0, (const uint8*)usb_descriptors);
    USB_DEBUG(("USB: interface descriptors = %x\n",status));
    /* configure usb audio volume levels/steps */
    status = UsbDeviceClassConfigure(USB_DEVICE_CLASS_CONFIG_AUDIO_VOLUMES, 0, 0, (const uint8*)(&usb_stereo_audio_volume));
    USB_DEBUG(("USB: volume descriptors = %x\n",status));
#else
    /* If audio not supported don't enumerate as mic or speaker */
    USB_DEBUG(("USB: Audio device class disabled - USB audio not enabled in project!\n"));
    USB_CLASS_DISABLE(USB_DEVICE_CLASS_AUDIO);
#endif

    USB_DEBUG(("USB: Endpoint Setup [0x%04X] - ", sinkUsbconfigGetDeviceClassType()));
    /* Attempt to enumerate - abort if failed */
    status = UsbDeviceClassEnumerate(&theSink.task, sinkUsbconfigGetDeviceClassType());

    if(status != usb_device_class_status_success)
    {
        USB_DEBUG(("Error %X\n", status));
        return;
    }

    USB_DEBUG(("Success\n"));

    /* Configure mass storage device */
    if(USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_MASS_STORAGE))
    {
        UsbDeviceClassConfigure(USB_DEVICE_CLASS_CONFIG_MASS_STORAGE_FAT_DATA_AREA, file.index, file.size, 0);
        usbFileInfo(fat_name, sizeof(fat_name)-1, &file);
        UsbDeviceClassConfigure(USB_DEVICE_CLASS_CONFIG_MASS_STORAGE_FAT_TABLE, file.index, file.size, 0);
        UsbDeviceClassConfigure(USB_DEVICE_CLASS_CONFIG_MASS_STORAGE_FAT_ROOT_DIR, root.index, root.size, 0);
    }
}


/****************************************************************************
NAME
    usbInit

DESCRIPTION
    Initialisation done once the main loop is up and running. Determines USB
    attach status etc.

RETURNS
    void
*/
void usbInit(void)
{
#ifdef ENABLE_USB_AUDIO
    A2dpPluginConnectParams *audio_connect_params = getAudioPluginConnectParams();
#endif
    USB_DEBUG(("USB: Init\n"));

    /* Abort if no device classes supported */
    if(USB_ALL_CLASSES_DISABLED)
    {
        USB_DEBUG(("Aborting USB init - no USB device classes enabled\n"));
        return;
    }
    /* If battery charging enabled set the charge current */
    usbUpdateChargeCurrent();
#ifdef ENABLE_USB_AUDIO
    /* Pass NULL USB mic Sink until the plugin handles USB mic */
    audio_connect_params->usb_params = NULL;
#endif
    /* Schedule reset to low power mode if attached */
    usbSetLowPowerMode(sinkUsbconfigGetUsbAttachTimeout());
    /* Check for audio */
    USB_RUNDATA.ready = TRUE;
    audioUpdateAudioRouting();
    stateManagerAmpPowerControl(POWER_UP);
}


/****************************************************************************
NAME
    usbSetVbusLevel

DESCRIPTION
    Set whether VBUS is above or below threshold

RETURNS
    void
*/
void usbSetVbusLevel(voltage_reading vbus)
{
    USB_DEBUG(("USB: VBUS %dmV [%d]\n", vbus.voltage, vbus.level));
    USB_RUNDATA.vbus_okay = vbus.level;
}


/****************************************************************************
NAME
    usbSetDeadBattery

DESCRIPTION
    Set whether VBAT is below the dead battery threshold

RETURNS
    void
*/
void usbSetVbatDead(bool dead)
{
    USB_DEBUG(("USB: VBAT %s\n", dead ? "Dead" : "Okay"));
    USB_RUNDATA.dead_battery = dead;
    if(!dead) MessageCancelAll(&theSink.task, EventSysUsbDeadBatteryTimeout);
}

/****************************************************************************
NAME
    sinkUsbGetBoost

DESCRIPTION
    Get the USB charger boost settings based on USB limits.

RETURNS
    void
*/
power_boost_enable sinkUsbGetBoost(usb_charge_current_setting *usb_limits)
{
    return (usb_limits->boost);
}

/****************************************************************************
NAME
    usbGetProprietaryChargeCurrent

DESCRIPTION
    Get USB charger limits for proprietary chargers

RETURNS
    TRUE if limits were obtained, FALSE if limits not available
*/


#ifdef ENABLE_CHARGER_V2
static bool usbGetProprietaryChargeCurrent(sink_charge_current_t* usb_limit, sink_usb_power_proprietary_readonly_config_def_t *usb_prop_data, int index)
{
    if (USB_RUNDATA.attached_status == NON_COMPLIANT_CHARGER)
    {
        if(usb_limit)
        {
            sink_proprietary_charge_current_t *prop_charge = &(usb_prop_data->proprietary_charge[index]);
            usb_limit->vsel         = prop_charge->vsel;
            usb_limit->charge       = prop_charge->charge;
            usb_limit->current      = prop_charge->current;
            usb_limit->disable_leds = prop_charge->disable_leds;
            usb_limit->power_off    = prop_charge->power_off;
            return TRUE;
        }
    }

    return FALSE;
}
#endif

/****************************************************************************
NAME
    usbGetNonCompliantChargeCurrent

DESCRIPTION
    Get USB charger limits for non-compliant proprietary chargers

RETURNS
    TRUE if limits were obtained, FALSE if limits not available
*/


#ifdef ENABLE_CHARGER_V2

#define in_range(value, target, tolerance) ((value >= target - tolerance) && (value <= target + tolerance))

static bool usbGetNonCompliantChargeCurrent(sink_charge_current_t* usb_limit)
{
    int index;
    sink_proprietary_charge_current_t *prop_charge;
    sink_usb_power_proprietary_readonly_config_def_t *usb_prop_data ;
    bool ret_val = FALSE;
    uint16 size = 0;

    if (USB_RUNDATA.attached_status == NON_COMPLIANT_CHARGER)
    {
        size = configManagerGetReadOnlyConfig(SINK_USB_POWER_PROPRIETARY_READONLY_CONFIG_BLK_ID, (const void **)&usb_prop_data);

        USB_DEBUG(("USB: Device D+/D- %uV %uV\n", USB_RUNDATA.data_plus, USB_RUNDATA.data_minus));

        for (index = 0; index < (uint16)((size * sizeof(uint16)) / sizeof(sink_proprietary_charge_current_t)); ++index)
        {
            prop_charge = &(usb_prop_data->proprietary_charge[index]);
            USB_DEBUG(("USB: Config D+/D- %uV %uV\n", prop_charge->dplus, prop_charge->dminus));
            if (in_range(USB_RUNDATA.data_plus, prop_charge->dplus, prop_charge->tolerance) &&
                in_range(USB_RUNDATA.data_minus, prop_charge->dminus, prop_charge->tolerance))
            {
                ret_val = usbGetProprietaryChargeCurrent(usb_limit, usb_prop_data, index);
                break;
            }
        }

        configManagerReleaseConfig(SINK_USB_POWER_PROPRIETARY_READONLY_CONFIG_BLK_ID);
    }

    return ret_val;
}
#endif

/****************************************************************************
NAME
    isChargerUsbTypeC

DESCRIPTION
    Determines if USB Type-C connector used.

RETURNS
    TRUE if USB Type-C connected, FALSE otherwise
*/
static bool isChargerUsbTypeC(void)
{
#ifdef ENABLE_USB_TYPE_C
    return (USB_RUNDATA.cc_status != CC_CURRENT_DEFAULT);
#else
    return FALSE;
#endif
}

/****************************************************************************
NAME
    usbGetChargeCurrent

DESCRIPTION
    Get USB charger limits

RETURNS
    TRUE if limits were obtained, FALSE if limits not available
*/
bool usbGetChargeCurrent(usb_charge_current_setting* usb_limit)
{
    sink_usb_power_readonly_config_def_t *usb_data ;
    sink_charge_current_t *usb_config = NULL;
    bool ret_val = FALSE;

    /* USB charging not enabled - no limits */
    if(!USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_BATTERY_CHARGING))
        return FALSE;

    PanicZero(configManagerGetReadOnlyConfig(SINK_USB_POWER_READONLY_CONFIG_BLK_ID, (const void **)&usb_data));

    USB_DEBUG(("USB: Status "));

    /* Set charge current */
    if (isChargerUsbTypeC())
    {
#ifdef ENABLE_USB_TYPE_C
        switch (USB_RUNDATA.cc_status)
        {
        case CC_CURRENT_DEFAULT:
        default:
            USB_DEBUG((" (default)"));
            Panic();
            break;

        case CC_CURRENT_1500:
            USB_DEBUG((" (1.5A)"));
            usb_config = &(usb_data->charge[usb_i_dchg_1500mA]);
            break;

        case CC_CURRENT_3000:
            USB_DEBUG((" (3.0A)"));
            usb_config = &(usb_data->charge[usb_i_dchg_3000mA]);
            break;
        }
#endif
        USB_DEBUG(("\n"));
        ret_val = TRUE;
    }
    else
    {
        switch(UsbAttachedStatus())
        {
            case HOST_OR_HUB:
                USB_DEBUG(("Host/Hub "));
                if(sinkUsbIsSuspended())
                {
                    USB_DEBUG(("Suspended (Battery %s)\n", usbDeadBatteryProvision() ? "Dead" : "Okay"));
                    if(usbDeadBatteryProvision())
                    {
                        usb_config = &(usb_data->charge[usb_i_susp_db]);
                    }
                    else
                    {
                        usb_config = &(usb_data->charge[usb_i_susp]);
                    }
                }
                else if(powerManagerIsChargerFullCurrent())
                {
                    USB_DEBUG(("%sEnumerated (Chg Full)\n", sinkUsbIsEnumerated() ? "" : "Not "));
                    if(!sinkUsbIsEnumerated())
                    {
                        usb_config = &(usb_data->charge[usb_i_att]);
                    }
                    else
                    {
                        usb_config = &(usb_data->charge[usb_i_conn]);
                    }
                }
                else
                {
                    USB_DEBUG(("%sEnumerated (Chg Partial)\n", sinkUsbIsEnumerated() ? "" : "Not "));
                    if(!sinkUsbIsEnumerated())
                    {
                        usb_config = &(usb_data->charge[usb_i_att_trickle]);
                    }
                    else
                    {
                        usb_config = &(usb_data->charge[usb_i_conn_trickle]);
                    }
                }
                ret_val = TRUE;
                break;
#ifdef HAVE_FULL_USB_CHARGER_DETECTION
            case DEDICATED_CHARGER:
                USB_DEBUG(("Dedicated Charger Port %s", sinkUsbIsVbusOkay() ? "Unlimited" : " Limited"));
                if(sinkUsbIsVbusOkay())
                {
                    usb_config = &(usb_data->charge[usb_i_dchg]);
                }
                else
                {
                    usb_config = &(usb_data->charge[usb_i_lim]);
                }
                USB_DEBUG(("\n"));
                ret_val = TRUE;
                break;

            case HOST_OR_HUB_CHARGER:
            case CHARGING_PORT:
                USB_DEBUG(("Charger Port%s\n", sinkUsbIsVbusOkay() ? " Unlimited" : " Limited"));
                if(sinkUsbIsVbusOkay())
                {
                    usb_config = &(usb_data->charge[usb_i_chg]);
                }
                else
                {
                    usb_config = &(usb_data->charge[usb_i_lim]);
                }
                ret_val = TRUE;
                break;

#ifdef ENABLE_CHARGER_V2
            case NON_COMPLIANT_CHARGER:
                USB_DEBUG(("Non-compliant charger%s\n", sinkUsbIsVbusOkay() ? " Unlimited" : " Limited"));
                if(sinkUsbIsVbusOkay())
                {
                    if (!usbGetNonCompliantChargeCurrent(usb_config))
                    {
                        usb_config = &(usb_data->charge[usb_i_fchg]);
                    }
                }
                else
                {
                    usb_config = &(usb_data->charge[usb_i_lim]);
                }
                USB_DEBUG(("\n"));
                ret_val = TRUE;
                break;

            case FLOATING_CHARGER:
                USB_DEBUG(("Floating charger%s", sinkUsbIsVbusOkay() ? " Unlimited" : " Limited"));
                if(sinkUsbIsVbusOkay())
                {
                    usb_config = &(usb_data->charge[usb_i_fchg]);
                }
                else
                {
                    usb_config = &(usb_data->charge[usb_i_lim]);
                }
                USB_DEBUG(("\n"));
                ret_val = TRUE;
                break;

            case NON_USB_CHARGER:
                USB_DEBUG(("Non-USB charger\n"));
                break;
#endif
#endif
            case DETACHED:
            default:
                USB_DEBUG(("Detached\n"));
                if(powerManagerIsChargerConnected())
                {
                    usb_config = &(usb_data->charge[usb_i_disc]);
                     ret_val = TRUE;
                }
                break;
        }
    }

    if (ret_val)
    {
        if(usb_limit)
        {
            usb_limit->current = usb_config->current;
            usb_limit->power_off = usb_config->power_off;
            usb_limit->vsel = usb_config->vsel;
            usb_limit->charge = usb_config->charge;
            usb_limit->disable_leds = usb_config->disable_leds;

            USB_DEBUG(("USB limits: current %u, boost %u, charge %u, vsel %s, power_off %u, disable_leds %u\n",
                       usb_limit->current, sinkUsbGetBoost(usb_limit), usb_limit->charge, usb_limit->vsel == 1 ? "VBAT" : "VBYP",
                       usb_limit->power_off, usb_limit->disable_leds));
        }
        else
        {
            USB_DEBUG(("USB: usb_limit is not having a valid value\n"));
        }	
        
    }

    configManagerReleaseConfig(SINK_USB_POWER_READONLY_CONFIG_BLK_ID);

    return ret_val;
}


/****************************************************************************
NAME
    usbIsAttached

DESCRIPTION
    Determine if USB is attached

RETURNS
    TRUE if USB is attached, FALSE otherwise
*/
bool usbIsAttached(void)
{
    /* UsbAttachedStatus() not currently implemented */
    return TRUE;
}

/****************************************************************************
NAME
    usbIsVoiceSupported

DESCRIPTION
    checks for a usb voice support.

RETURNS
    true if usb voice is supported, false if no usb voice or USB music is supported
*/
bool usbIsVoiceSupported(void)
{
    return (sinkUsbconfigGetPluginType() != usb_plugin_stereo);
}

#ifdef ENABLE_USB_AUDIO
/****************************************************************************
NAME
    usbSendHidEvent

DESCRIPTION
    Send HID event over USB

RETURNS
    void
*/
static void usbSendHidEvent(usb_device_class_event event)
{
    USB_DEBUG(("USB: HID Event 0x%X\n", event));
    if(USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_HID_CONSUMER_TRANSPORT_CONTROL))
        UsbDeviceClassSendEvent(event);
}


/****************************************************************************
NAME
    usbGetVolumeLevel

DESCRIPTION
    Get USB audio level

RETURNS
    void
*/
int16 usbGetVolumeLevel(void)
{
    int16 volume = 0;
    usb_device_class_audio_levels *levels = mallocPanic(sizeof (usb_device_class_audio_levels));

    if (levels)
    {
        UsbDeviceClassGetValue(USB_DEVICE_CLASS_GET_VALUE_AUDIO_LEVELS, (uint16 *) levels);

        /* convert to signed 16 */
        levels->out_l_vol = (int16)(int8)(levels->out_l_vol>>8);
        levels->out_r_vol = (int16)(int8)(levels->out_r_vol>>8);

        /* convert to volume steps */
        volume = (sinkVolumeGetNumberOfVolumeSteps(audio_output_group_main) + ((levels->out_l_vol * sinkVolumeGetNumberOfVolumeSteps(audio_output_group_main))/USB_MIN_VOLUME));

        /* limit check */
        if(volume > sinkVolumeGetMaxVolumeStep(audio_output_group_main))
        {
            volume = sinkVolumeGetMaxVolumeStep(audio_output_group_main);
        }
        if(volume < 0)
        {
            volume = 0;
        }

        if (!sinkUsbIsUsbPluginTypeStereo())
        {
            volume = levels->out_l_vol;
        }
        freePanic(levels);
    }
    return volume;
}
/****************************************************************************
NAME
    usbAudioIsAttached

DESCRIPTION
    Determine if USB Audio is attached

RETURNS
    TRUE if USB Audio is attached, FALSE otherwise
*/
bool usbAudioIsAttached(void)
{
    USB_DEBUG(("usbAudioIsAttached: att=%u rdy=%u alw=%u mic=%u spk=%u sus=%u\n",
               usbIsAttached(),
               sinkUsbIsReady(),
               sinkUsbconfigIsAudioAlwaysOn(),
               sinkUsbIsMicrophoneActive(),
               sinkUsbIsSpeakerActive(),
               USB_RUNDATA.audio_suspend_state));

    /* If USB detached or not ready - no audio */
    if (!usbIsAttached() || !USB_RUNDATA.ready)
    {
        return FALSE;
    }

    /* If USB attached and always on - audio */
    if (sinkUsbconfigIsAudioAlwaysOn())
    {
        return TRUE;
    }

    /* If mic and speaker both inactive - no audio */
    if (!sinkUsbIsMicrophoneActive() && !sinkUsbIsSpeakerActive())
    {
        return FALSE;
    }

    /* USB speaker can be held active even though we have suspended */
    if (sinkUsbIsSpeakerActive() && USB_RUNDATA.audio_suspend_state != usb_audio_suspend_none)
    {
        return FALSE;
    }

    /* Mic or speaker active - audio */
    return TRUE;
}

/****************************************************************************
NAME
    usbSpeakerClassIsEnumerated

DESCRIPTION
    Determine if USB speaker class is enumerated

RETURNS
    TRUE if USB speaker class is enumerated, FALSE otherwise
*/
bool usbSpeakerClassIsEnumerated(void)
{
    return USB_RUNDATA.enumerated_spkr;
}

/****************************************************************************
NAME
    usbGetVolume

DESCRIPTION
    Extract USB volume setting from USB lib levels

RETURNS
    Volume to pass to csr_usb_audio_plugin
*/
static int16 usbGetVolume(AUDIO_MODE_T* mode, volume_direction* dir)
{
    int16 result;
    bool mic_muted;
    bool spk_muted = FALSE;

    /* Get vol settings from USB lib */
    usb_device_class_audio_levels levels;
    UsbDeviceClassGetValue(USB_DEVICE_CLASS_GET_VALUE_AUDIO_LEVELS, (uint16*)&levels);

    USB_DEBUG(("USB: RAW Gain L=%d, R=%d\n", levels.out_l_vol, levels.out_r_vol));

    /* Convert to signed 16, from 256 units per 1dB to 1 unit per 1dB (range -45dB to 0dB) */
    levels.out_l_vol = (int16)(int8)(levels.out_l_vol>>8);
    levels.out_r_vol = (int16)(int8)(levels.out_r_vol>>8);

    USB_DEBUG(("USB: Scaled Gain L=%ddB, R=%ddB\n", levels.out_l_vol, levels.out_r_vol));
    USB_DEBUG(("USB: Mute M %X S %X\n", levels.in_mute, levels.out_mute));

    if(sinkUsbconfigGetPluginType() == usb_plugin_stereo)
    {
        /* Pack result */
        result = ((sinkVolumeGetNumberOfVolumeSteps(audio_output_group_main) * (USB_MIN_VOLUME + levels.out_l_vol)) / USB_MIN_VOLUME);
        /* limit check */
        if(result > sinkVolumeGetMaxVolumeStep(audio_output_group_main))
        {
            result = sinkVolumeGetMaxVolumeStep(audio_output_group_main);
        }
        if(result < 0)
        {
            result = 0;
        }

        /* check for mute state */
        if(levels.out_mute)
        {
            /* set to mute */
            result = VOLUME_A2DP_MIN_LEVEL;
        }

        USB_DEBUG(("USB: Gain Level = %d\n", result));
        displayUpdateVolume((VOLUME_NUM_VOICE_STEPS * result)/sinkVolumeGetNumberOfVolumeSteps(audio_output_group_main));
#ifdef ENABLE_SUBWOOFER
        updateSwatVolume(result);
#endif

    }

    else
    {
        /* Use HFP gain mappings */
        /* convert from USB dB to hfp volume steps */
        result = (VOL_NUM_VOL_SETTINGS + ((levels.out_l_vol * VOL_NUM_VOL_SETTINGS)/USB_MIN_VOLUME));
        /* limit check */
        if(result >= VOL_NUM_VOL_SETTINGS)
            result = (VOL_NUM_VOL_SETTINGS - 1);
        if(result < 0)
            result = 0;

        displayUpdateVolume((VOLUME_NUM_VOICE_STEPS * levels.out_l_vol)/sinkVolumeGetNumberOfVolumeSteps(audio_output_group_main));
#ifdef ENABLE_SUBWOOFER
        updateSwatVolume(levels.out_l_vol);
#endif
    }

    /* Mute if muted by host or not supported */
    mic_muted = levels.in_mute  || !USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_AUDIO_MICROPHONE);
    spk_muted = spk_muted || levels.out_mute || !USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_AUDIO_SPEAKER);

    if(mode)
    {
        if(mic_muted && spk_muted)
            *mode = AUDIO_MODE_STANDBY;
        else
            *mode = AUDIO_MODE_CONNECTED;
    }

    VolumeUpdateMuteStatusMicrophone(mic_muted);
    VolumeUpdateMuteStatusAllOutputs(spk_muted);
    /* Compare with previously stored volume level and set dir as appropriate.
       Only compare to out_l_vol as out_r_vol is ignored in setting result.
       */
    if (dir)
    {
        if(sinkUsbGetOutLVol()< levels.out_l_vol)
            *dir = increase_volume;
        else if (sinkUsbGetOutLVol() > levels.out_l_vol)
            *dir = decrease_volume;
        else
            *dir = same_volume;
    }

    /* Store usb audio levels for use later */
    USB_RUNDATA.out_l_vol = levels.out_l_vol;

    return result;
}


/****************************************************************************
NAME
    usbAudioSinkMatch

DESCRIPTION
    Compare sink to the USB audio sink

RETURNS
    TRUE if sink matches USB audio sink, otherwise FALSE
*/
bool usbAudioSinkMatch(Sink sink)
{
    Source usb_source = NULL;
    UsbDeviceClassGetValue(USB_DEVICE_CLASS_GET_VALUE_AUDIO_SOURCE, (uint16*)&usb_source);

    USB_DEBUG(("USB: usbAudioSinkMatch sink %p = %p, mic enabled = %x, spk enabled = %x\n",
               (void*)sink , (void*)StreamSinkFromSource(usb_source),
               USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_AUDIO_MICROPHONE), USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_AUDIO_SPEAKER) ));

    return (usbMicOrSpeakerEnabled() && sink && (sink == StreamSinkFromSource(usb_source)));
}

static plugin_type_t getVoicePluginType(usb_plugin_type usb_type)
{
    plugin_type_t voice_plugin_type = narrow_band;
    switch(usb_type)
    {
        case usb_plugin_mono_nb:
            voice_plugin_type = narrow_band;
            break;
        case usb_plugin_mono_wb:
            voice_plugin_type = wide_band;
            break;
        case usb_plugin_mono_uwb:
            voice_plugin_type = ultra_wide_band;
            break;
        case usb_plugin_mono_swb:
            voice_plugin_type = super_wide_band;
            break;
        default:
            Panic();
            break;
    }
    return voice_plugin_type;
}

/****************************************************************************
NAME
    usbAudioGetPluginInfo

DESCRIPTION
    Get USB plugin info for current config

RETURNS
    TRUE if successful, otherwise FALSE
*/
static const usb_plugin_info* usbAudioGetPluginInfo(Task* task, usb_plugin_type type, uint8 index)
{
    switch(type)
    {
        case usb_plugin_stereo:
            *task = (Task)&csr_usb_decoder_plugin;
            audioControlLowPowerCodecs (FALSE) ;
            break;
        case usb_plugin_mono_nb:
        case usb_plugin_mono_wb:
        case usb_plugin_mono_uwb:
        case usb_plugin_mono_swb:
            *task = AudioPluginVoiceVariantsGetUsbPlugin(getVoicePluginType(type), (plugin_variant_t)index);
            audioControlLowPowerCodecs (TRUE) ;
            break;
        default:
            *task = NULL;
            break;
    }

    return &usb_plugins[type];
}

static bool usbCommonPopulateConnectParameters(audio_connect_parameters *connect_parameters)
{
    Sink sink;
    Source source;
    UsbDeviceClassGetValue(USB_DEVICE_CLASS_GET_VALUE_AUDIO_SOURCE, &source);
    sink = StreamSinkFromSource(source);

    /* Check Audio configured (sink will be NULL if VM USB not enabled) */
    if(usbMicOrSpeakerEnabled() && sink)
    {
        USB_DEBUG(("USB: configured\n"));
        if(usbAudioIsAttached())
        {
            USB_DEBUG(("USB: attached\n"));
            if(sinkAudioGetRoutedAudioSink() != sink)
            {
                uint32 sampleFreq;
                AUDIO_MODE_T mode;
                A2dpPluginConnectParams *audioConnectParams = getAudioPluginConnectParams();

                UsbDeviceClassGetValue(USB_DEVICE_CLASS_GET_VALUE_SPEAKER_SAMPLE_FREQ, &sampleFreq);
                USB_DEBUG(("USB: sample rate %lu\n", sampleFreq));

                /* get current volume level in steps from usb interface,
                 * main group only */
                sinkUsbSetMainVolume(usbGetVolume(&mode, NULL));

                /* Make sure we're using correct parameters for USB */
                audioConnectParams->mode_params = getAudioPluginModeParams();

                /* Allow USB volume changes to generate max and min events */
                MessageSend ( &theSink.task , EventSysAllowUSBVolEvents , 0 );
                
                connect_parameters->audio_sink = sink;
                connect_parameters->sink_type = AUDIO_SINK_USB;
                connect_parameters->volume = TonesGetToneVolumeInDb(audio_output_group_main);
                connect_parameters->rate = sampleFreq;
                connect_parameters->mode = mode;
                connect_parameters->route = AUDIO_ROUTE_INTERNAL;
                connect_parameters->power = powerManagerGetLBIPM();
                connect_parameters->app_task = &theSink.task;

                return TRUE;
            }
        }
    }

    return FALSE;
}

bool usbAudioPopulateConnectParameters(audio_connect_parameters *connect_parameters)
{
    if(usbCommonPopulateConnectParameters(connect_parameters))
    {
        Task plugin;
        AudioPluginFeatures PluginFeatures = sinkAudioGetPluginFeatures();
        peerPopulatePluginConnectData(&PluginFeatures, connect_parameters->rate);
        (void *)usbAudioGetPluginInfo(&plugin, sinkUsbconfigGetPluginType(), sinkUsbconfigGetPluginIndex());

        connect_parameters->audio_plugin = plugin;
        connect_parameters->features = PluginFeatures;
        connect_parameters->params = getAudioPluginConnectParams();
        /* If using BA plugin update the appropriate plugin and volume data for BA */
        if(useBroadcastPlugin())
        {
            connect_parameters->audio_plugin = (Task)&csr_ba_usb_decoder_plugin;
            connect_parameters->volume = sinkBroadcastAudioGetVolume(audio_source_USB);
            ((A2dpPluginConnectParams *)(connect_parameters->params))->ba_output_plugin = AudioPluginOutputGetBroadcastPluginTask();
        }

        return TRUE;
    }

    return FALSE;
}

bool usbVoicePopulateConnectParameters(audio_connect_parameters *connect_parameters)
{
    if(usbCommonPopulateConnectParameters(connect_parameters))
    {
        Task plugin;
        volume_info usb_volume;
        sinkUsbGetUsbVolume(&usb_volume);
        (void *)usbAudioGetPluginInfo(&plugin, sinkUsbconfigGetPluginType(), sinkUsbconfigGetPluginIndex());

        connect_parameters->audio_plugin = plugin;
        connect_parameters->features = sinkAudioGetPluginFeatures();
        connect_parameters->params = sinkHfpDataGetHfpPluginParams();

        AudioSetVolume(sinkVolumeGetVolumeMappingforCVC(usb_volume.main_volume),
                                               (int16)TonesGetToneVolume());

        USB_DEBUG(("USB: Connect voice %p\n",
               (void *) ((hfp_common_plugin_params_t *) connect_parameters->params)->usb_params.usb_sink));

        return TRUE;
    }

    return FALSE;
}

/*************************************************************************
NAME
    usbAudioSuspend

DESCRIPTION
    Issue HID consumer control command to attempt to pause current USB stream

RETURNS
    None

**************************************************************************/
static void usbAudioSuspend (void)
{
    /* If speaker is in use or not marked as suspended then pause */
    if (sinkUsbIsSpeakerActive()|| (sinkUsbAudioIsSuspendedNone()))
    {
        usbSetAudioSuspendState(usb_audio_suspend_local);
        UsbDeviceClassSendEvent(USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_PAUSE);

        /* Ensure audio routing is kicked as we may not always get a notification that streaming has ceased via USB */
        audioUpdateAudioRoutingAfterDisconnect();
    }
}

/*************************************************************************
NAME
    usbAudioResume

DESCRIPTION
    Issue HID consumer control command to attempt to resume current USB stream

RETURNS
    None

**************************************************************************/
void usbAudioResume (void)
{
    /* If speaker is not use or marked as suspended then resume */
    if (!USB_RUNDATA.spkr_active || (USB_RUNDATA.audio_suspend_state != usb_audio_suspend_none))
    {
        usbSetAudioSuspendState(usb_audio_suspend_none);
        UsbDeviceClassSendEvent(USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_PLAY);

        /* Ensure audio routing is kicked as we may not always get a notification that streaming has started via USB */
        sinkCancelAndSendLater(EventSysCheckAudioRouting, 0);
    }
}

/****************************************************************************
NAME
    usbAudioDisconnect

DESCRIPTION
    Disconnect USB audio stream

RETURNS
    void
*/
void usbAudioPostDisconnectConfiguration(void)
{
        USB_DEBUG(("USB: Disconnect %p\n", (void*) sinkAudioGetRoutedAudioSink()));

    /* If speaker is in use then pause */
    if (sinkUsbconfigGetPauseWhenSwitchingSource())
    {
        /* This option was added to address B-164414
         * If the device was paused already this command is redundant, but it will however force the
         * vm to send a resume command when switching back to USB. This will restart a USB audio stream
         * that was previously paused.
         * Also, when using USB audio not all sources will accept the pause command e.g streaming services.
         */
        usbAudioSuspend();
    }

#ifdef ENABLE_PEER
    {
        /*If the USB has disconnected then notify this to the peer device */
        sinkAvrcpUpdatePeerSourceDisconnected(USB_AUDIO);
    }
#endif
}

/****************************************************************************
NAME
    usbVoicePostDisconnectConfiguration

DESCRIPTION
    Exit the current state "deviceActiveCallNoSCO" post disconnection.

RETURNS
    void
*/
void usbVoicePostDisconnectConfiguration(void)
{
    USB_DEBUG(("USB: Disconnect Voice %p\n", (void*) sinkAudioGetRoutedVoiceSink()));

    /* do not enter connectiable state if this disconnection was performed 
       during power off event transition */
    if(stateManagerGetState() != deviceLimbo)
    {
    
        if(deviceManagerNumConnectedDevs() == 0)
        {
            stateManagerEnterConnectableState(TRUE);
        }
        else
        {
            stateManagerEnterConnectedState();
        }
    }
}

/****************************************************************************
NAME
    usbAudioSetVolume

DESCRIPTION
    Set USB audio volume

RETURNS
    void
*/
static void usbAudioSetVolume(void)
{
    if((usbIsCurrentAudioSinkUsb() || usbIsCurrentVoiceSinkUsb()) 
            && usbMicOrSpeakerEnabled())
    {
        AUDIO_MODE_T mode;
        volume_direction direction;
        volume_info usb_volume;

        int16 oldVolume;
        int16 newVolume;        
        
        sinkUsbGetUsbVolume(&usb_volume);

        /* Store old volume to check if we need to change mode */
        oldVolume = usb_volume.main_volume;
        
        /* extract new volume setting from USB lib levels */
        newVolume = usbGetVolume(&mode, &direction);
        if(newVolume != oldVolume)
        {
            usb_volume.main_volume = newVolume;
            /* Update volume structure */
            sinkUsbSetUsbVolume(usb_volume);
        }
        
        if(sinkUsbIsVolEventAllowed())
        {
            if((direction == decrease_volume) && sinkUsbVolumeIsAtMinimum())
            {
                sinkUsbVolumeEvent(EventSysVolumeMin);
            }
            else if((direction == increase_volume) && sinkUsbVolumeIsAtMaximum())
            {
                sinkUsbVolumeEvent(EventSysVolumeMax);
            }
        }

        if(sinkUsbconfigGetPluginType() == usb_plugin_stereo)
        {
            /* Use group volume interface for stereo volume */
            VolumeSetNewMainVolume(&usb_volume, oldVolume);
        }
        else
        {
            /* Use volume interface for CVC volume */
            AudioSetVolume(sinkVolumeGetVolumeMappingforCVC(newVolume) ,
                           (int16)TonesGetToneVolume());
        }
    }
}


/****************************************************************************
NAME
    usbAudioGetMode

DESCRIPTION
    Get the current USB audio mode if USB in use

RETURNS
    void
*/
void usbAudioGetMode(AUDIO_MODE_T* mode)
{
    if(usbIsCurrentAudioSinkUsb() && (sinkUsbconfigGetPluginType() == usb_plugin_stereo))
    {
        (void)usbGetVolume(mode, NULL);
    }
}
/****************************************************************************
NAME
    usbGetAudioSink

DESCRIPTION
    check USB state and return sink if available

RETURNS
   sink if available, otherwise 0
*/
static Sink getUsbSink(void)
{
    Source usb_source = NULL;
    UsbDeviceClassGetValue(USB_DEVICE_CLASS_GET_VALUE_AUDIO_SOURCE, (uint16*)&usb_source);
    return StreamSinkFromSource(usb_source);
}

/****************************************************************************
NAME
    usbGetAudioSink

DESCRIPTION
    check USB state and return sink if available

RETURNS
   sink if available, otherwise 0
*/
Sink usbGetAudioSink(void)
{
    Sink sink = NULL;

    if((sinkUsbconfigIsAudioAlwaysOn()) || ((usbAudioIsAttached()) && (sinkUsbIsSpeakerActive())))
    {
        sink = getUsbSink();
        USB_DEBUG(("USB: usbGetAudioSink sink %p, spk enabled = %x\n", (void*)sink, USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_AUDIO_SPEAKER) ));
    }

    return sink;
}

/****************************************************************************
NAME
    usbGetVoiceSink

DESCRIPTION
    check USB voice state and return sink if available

RETURNS
   sink if available, otherwise 0
*/
Sink usbGetVoiceSink(void)
{
    Sink sink = NULL;

    if((sinkUsbconfigIsAudioAlwaysOn()) || ((usbAudioIsAttached()) && ((sinkUsbIsSpeakerActive())
                                            || (sinkUsbIsMicrophoneActive()))))
    {
        sink = getUsbSink();
        USB_DEBUG(("USB: usbGetVoiceSink sink %p, mic enabled = %x, spk enabled = %x\n", (void*)sink ,
                            USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_AUDIO_MICROPHONE), USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_AUDIO_SPEAKER) ));
    }
    return sink;
}

#ifdef ENABLE_USB_AUDIO


static bool usbMicOrSpeakerEnabled(void)
{
    bool micEnabled = FALSE;
    bool spkEnabled = FALSE;
    /*Test speaker and microphone seperately as both use cases are valid on their own */
    micEnabled = USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_AUDIO_MICROPHONE);
    spkEnabled = USB_CLASS_ENABLED(USB_DEVICE_CLASS_TYPE_AUDIO_SPEAKER);

    return (micEnabled || spkEnabled);
}

static void rescheduleSetUsbSpeakerSampleRate(uint32 sample_rate)
{
    MAKE_USB_DEVICE_CLASS_MESSAGE(USB_DEVICE_CLASS_SAMPLE_RATE);
    message->sample_rate = sample_rate;
    MessageCancelAll (&theSink.task, USB_DEVICE_CLASS_MSG_SPEAKER_SAMPLE_RATE_IND);
    MessageSendLater(&theSink.task, USB_DEVICE_CLASS_MSG_SPEAKER_SAMPLE_RATE_IND, message, 100);
    USB_DEBUG(("USB: Set Spk Sample Rate - DSP Loading - Reschedule\n"));
}

static void configureForNewSampleRateIfRequired(uint32 sample_rate)
{
    if(sample_rate != AudioGetUsbSampleRate())
    {
        USB_DEBUG(("USB: Set Spk Sample Rate - DSP running - *** needs update *** - AudioDisconnect\n"));

        audioDisconnectRoutedAudio();
        sinkAudioSetRoutedAudioSource(audio_source_none);

        USB_DEBUG(("USB: Set Spk Sample Rate - DSP running - *** needs update *** - Switch To USB\n"));

        audioUpdateAudioRouting();
    }
}

static bool usbDecoderHasValidSampleRate(void)
{
    return (AudioGetUsbSampleRate() ? TRUE : FALSE);
}

/****************************************************************************
NAME
    usbAudioSetSpeakerSampleRate

DESCRIPTION
    check if the sample rate has change, whether usb audio is currently streaming
    , if so a restart of the sbc_decoder is necessary to run at the new indicated rate

RETURNS
    none
*/
static void usbAudioSetSpeakerSampleRate(uint32 sample_rate)
{
    if(sinkAudioGetRoutedAudioSource() == audio_source_USB)
    {
        switch(GetCurrentDspStatus())
        {
            case DSP_RUNNING:
                if(usbDecoderHasValidSampleRate())
                {
                    configureForNewSampleRateIfRequired(sample_rate);
                }
                else
                {
                    rescheduleSetUsbSpeakerSampleRate(sample_rate);
                }
                break;
            case DSP_LOADING:
            case DSP_LOADED_IDLE:
                rescheduleSetUsbSpeakerSampleRate(sample_rate);
                break;

            default:
                USB_DEBUG(("USB: Set Spk Sample Rate - DSP NOT running\n"));
                break;

        }
    }
}
#endif

/****************************************************************************
NAME
    usbAudioSetMicSampleRate

DESCRIPTION
    check if the sample rate has change, whether usb audio is currently streaming
    , if so a restart of the sbc_decoder may be necessary to run at the new indicated rate

RETURNS
    none
*/
static void usbAudioSetMicSampleRate(uint32 sample_rate)
{
    UNUSED(sample_rate);
}

bool usbIsCurrentAudioSinkUsb(void)
{
    return usbAudioSinkMatch(sinkAudioGetRoutedAudioSink());
}

bool usbIsCurrentVoiceSinkUsb(void)
{
    return usbAudioSinkMatch(sinkAudioGetRoutedVoiceSink());
}

bool usbIfUsbSinkExists(void)
{
    return usbGetAudioSink() != NULL;
}

#endif /* ENABLE_USB_AUDIO */


static void sinkUsbConfigInit(void)
{

    sink_usb_readonly_config_def_t *usb_data;

    if (configManagerGetReadOnlyConfig(SINK_USB_READONLY_CONFIG_BLK_ID, (const void **)&usb_data))
    {
        USB_RUNDATA.device_class = usb_data->device_class;
        configManagerReleaseConfig(SINK_USB_READONLY_CONFIG_BLK_ID);
    }
#ifdef ENABLE_AHI_USB_HOST
    if( sinkAhiIsUsbHidDataLinkEnabled() )
    {
        USB_RUNDATA.device_class |= USB_DEVICE_CLASS_TYPE_HID_DATALINK_CONTROL;
    }
#endif

#ifdef ENABLE_USB_HID_UPGRADE
    USB_RUNDATA.device_class |= USB_DEVICE_CLASS_TYPE_HID_DATALINK_CONTROL;
#endif
}


static void sinkUsbVolumeEvent(sinkEvents_t event)
{
    MessageSend ( &theSink.task, event, 0 );
    USB_RUNDATA.vol_event_allowed = FALSE;
    MessageSendLater ( &theSink.task, EventSysAllowUSBVolEvents, 0, VOLUME_USB_EVENT_WAIT );
}

void sinkUsbVolumeIncrement(void)
{
    if(sinkUsbIsVolEventAllowed() && sinkUsbVolumeIsAtMaximum())
    {
        /* Send EventSysVolumeMax if volume was increased, we are above the usb max volume and not muted */
        sinkUsbVolumeEvent(EventSysVolumeMax);
    }
    usbVolUp();
}

void sinkUsbVolumeDecrement(void)
{
    if(sinkUsbIsVolEventAllowed() && sinkUsbVolumeIsAtMinimum())
    {
        /* Send EventSysVolumeMin if volume was decreased and we are at the lowest volume level */
        sinkUsbVolumeEvent(EventSysVolumeMin);
    }
    usbVolDn();
}

bool sinkUsbProcessEventUsb(const MessageId EventUsb)
{
    switch(EventUsb)
    {
        case EventUsrUsbPlayPause:
           USB_DEBUG(("HS : EventUsrUsbPlayPause")) ;
           usbPlayPause();
           break;
        case EventUsrUsbStop:
           USB_DEBUG(("HS : EventUsrUsbStop\n")) ;
           usbStop();
           break;
        case EventUsrUsbFwd:
           USB_DEBUG(("HS : EventUsrUsbFwd\n")) ;
           usbFwd();
           break;
        case EventUsrUsbBack:
           USB_DEBUG(("HS : EventUsrUsbBack\n")) ;
           usbBck();
           break;
        case EventUsrUsbMute:
           USB_DEBUG(("HS : EventUsrUsbMute")) ;
           usbMute();
           break;
        case EventUsrUsbLowPowerMode:
            /* USB low power mode */
            usbSetBootMode(BOOTMODE_USB_LOW_POWER);
            break;
        case EventSysUsbDeadBatteryTimeout:
            usbSetVbatDead(FALSE);
            break;
        case EventSysAllowUSBVolEvents:
            USB_RUNDATA.vol_event_allowed = TRUE;
            break;
    }
    return TRUE;
}

#ifdef ENABLE_USB_HUB_SUPPORT
void usbAttachToHub(void)
{
    if (!UsbHubAttach())
    {
        USB_DEBUG(("UsbHubAttach Failed - Check USB interfaces are configured correctly so that USB enumeration will succeed!\n"));
        if (USB_ALL_CLASSES_DISABLED)
        {
            USB_DEBUG(("Error - No USB device classes enabled\n"));
        }
        FATAL_ERROR(("USB Fatal Error!\n"));
    }
}
#endif

/****************************************************************************
NAME
    sinkUsbAudioIsSuspendedLocal

DESCRIPTION
    Determine if USB audio is suspended locally

RETURNS
    TRUE if USB suspend state is Local
*/
bool sinkUsbAudioIsSuspendedLocal(void)
{
    return USB_RUNDATA.audio_suspend_state == usb_audio_suspend_local;
}


/****************************************************************************
NAME
    sinkUsbAudioIsSuspended

DESCRIPTION
    Determine if USB audio is suspended

RETURNS
    TRUE if USB audio is suspended
*/
bool sinkUsbAudioIsSuspended(void)
{
    return USB_RUNDATA.audio_suspend_state != usb_audio_suspend_none;
}


/****************************************************************************
NAME
    sinkUsbIsMicrophoneActive

DESCRIPTION
    Determine if USB microphone is active

RETURNS
    TRUE if the microphone is active
*/
bool sinkUsbIsMicrophoneActive(void)
{
    return USB_RUNDATA.mic_active;
}

/****************************************************************************
NAME
    sinkUsbIsSpeakerActive

DESCRIPTION
    Determine if USB speaker is active

RETURNS
    TRUE if the speaker is active
*/
bool sinkUsbIsSpeakerActive(void)
{
    return USB_RUNDATA.spkr_active;
}

/*************************************************************************
NAME    
    sinkUsbGetHfpPluginParams
    
DESCRIPTION
    Get the HFP Plugin Parameters from the USB Device Class library

RETURNS
    void
    
**************************************************************************/
void sinkUsbGetHfpPluginParams(usb_common_params *usb_params)
{
    UsbDeviceClassGetValue(USB_DEVICE_CLASS_GET_VALUE_AUDIO_SINK, &usb_params->usb_sink);
}

/*************************************************************************
NAME
    sinkUsbVoicePostConnectConfiguration

DESCRIPTION
    Enters to 'deviceActiveCallNoSCO' state post successful connection.

RETURNS
    void

**************************************************************************/
void sinkUsbVoicePostConnectConfiguration(void)
{
    /* Enter deviceActiveCallNoSCO state */
    stateManagerEnterActiveCallState();
}

/*************************************************************************
NAME
    sinkUsbIsEnumerated

DESCRIPTION
    Get the info about  enumerated state of USB.

RETURNS
    bool

**************************************************************************/
bool sinkUsbIsEnumerated(void)
{
    return USB_RUNDATA.enumerated;
}

#endif /* ENABLE_USB */

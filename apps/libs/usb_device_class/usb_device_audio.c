/*******************************************************************************
Copyright (c) 2010 - 2016 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#include <message.h>
#include <panic.h>
#include <sink.h>
#include <source.h>
#include <string.h>
#include <stream.h>
#include <usb.h>
#include <print.h>
#include <vmal.h>

#include "usb_device_class.h"
#include "usb_device_audio.h"
#include "usb_device_class_private.h"

#ifndef USB_DEVICE_CLASS_REMOVE_AUDIO

#ifdef USB_DEVICE_CLASS_BA_48KHz
#define USB_STREAMING_NUM_OF_FREQUENCY (0x01)
#else
/* otherwise we support more sampling frequency */
#define USB_STREAMING_NUM_OF_FREQUENCY (0x06)
#endif


static uint32 current_speaker_sample_freq = SAMPLE_RATE_48K;
static uint32 current_mic_sample_freq = SAMPLE_RATE_48K;

static const usb_device_class_audio_volume_config usb_audio_volume_default =
{
    DEFAULT_SPEAKER_VOLUME_MIN,
    DEFAULT_SPEAKER_VOLUME_MAX,
    DEFAULT_SPEAKER_VOLUME_RESOLUTION,
    DEFAULT_SPEAKER_VOLUME_DEFAULT,
    DEFAULT_MICROPHONE_VOLUME_MIN,
    DEFAULT_MICROPHONE_VOLUME_MAX,
    DEFAULT_MICROPHONE_VOLUME_RESOLUTION,
    DEFAULT_MICROPHONE_VOLUME_DEFAULT
};

static const usb_device_class_audio_volume_config* usb_audio_volume = &usb_audio_volume_default;


static bool usb_device_class_is_audio_volume_default(const usb_device_class_audio_volume_config* vol_config)
{
    if ((vol_config->speaker_min == usb_audio_volume_default.speaker_min) &&
        (vol_config->speaker_max == usb_audio_volume_default.speaker_max) &&
        (vol_config->speaker_res == usb_audio_volume_default.speaker_res) &&
        (vol_config->speaker_default == usb_audio_volume_default.speaker_default) &&
        (vol_config->mic_min == usb_audio_volume_default.mic_min) &&
        (vol_config->mic_max == usb_audio_volume_default.mic_max) &&
        (vol_config->mic_res == usb_audio_volume_default.mic_res) &&
        (vol_config->mic_default == usb_audio_volume_default.mic_default))
    {
        return TRUE;
    }
    
    return FALSE;  
}   


static const UsbCodes usb_codes_ac = {B_INTERFACE_CLASS_AUDIO, /* bInterfaceClass */
                                      B_INTERFACE_SUB_CLASS_AUDIOCONTROL, /* bInterfaceSubClass */
                                      PR_PROTOCOL_UNDEFINED, /* bInterfaceProtocol */
                                      I_INTERFACE_INDEX /* iInterface */
                                      };

static const UsbCodes usb_codes_as = {B_INTERFACE_CLASS_AUDIO, /* bInterfaceClass */
                                      B_INTERFACE_SUB_CLASS_AUDIOSTREAMING, /* bInterfaceSubClass */
                                      PR_PROTOCOL_UNDEFINED, /* bInterfaceProtocol */
                                      I_INTERFACE_INDEX /* iInterface */
                                      };

/*
    USB Audio Descriptors

    Our descriptors declare the dongle as a microphone and speaker
    that support 16 bit mono PCM at 8khz.

    Terminal Architecture
    =====================

    Microphone(4) -> Feature Unit(5) -> USB Output Terminal(6)

    USB Input Terminal(1) -> Feature Unit(2) -> Speaker(3)

    USB Streaming:
    Isochronous, mono, 16 bit, PCM input and output.

    Format of PCM stream:
    | Mono LSB | Mono MSB |
*/
static const uint8 interface_descriptor_control_mic_and_speaker[] =
{
    /* Class Specific Header */
    0x0A,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = HEADER */
    0x00, 0x01,   /* bcdADC = Audio Device Class v1.00 */
    0x0A + 0x0c + 0x0b + 0x09 + 0x0c + 0x0d + 0x09, /* wTotalLength LSB */
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
    0x02,         /* bNrChannels = 2 */
    0x03, 0x00,   /* wChannelConfig = left front and right front */
    0x00,         /* iChannelName = no string */
    0x00,         /* iTerminal = same as USB product string */
    
    /* Speaker Features */
    0x0d,           /*bLength*/
    0x24,           /*bDescriptorType = CS_INTERFACE */
    0x06,           /*bDescriptorSubType = FEATURE_UNIT*/
    SPEAKER_FU,     /*bUnitId*/
    SPEAKER_IT,     /*bSourceId - Speaker IT*/
    0x02,           /*bControlSize = 2 bytes per control*/
    0x01, 0x00,     /*bmaControls[0] = 0001 (Mute on Master Channel)*/
    0x02, 0x00,     /*bmaControls[1] = 0002 (Vol on Left Front)*/
    0x02, 0x00,     /*bmaControls[1] = 0002 (Vol on Right Front)*/
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

/** Default USB control interface descriptors for mic */
static const uint8 interface_descriptor_control_mic[] =
{
    /* Class Specific Header */
    0x09,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = HEADER */
    0x00, 0x01, /* bcdADC = Audio Device Class v1.00 */
    0x09 + 0x0c + 0x0b + 0x09, /* wTotalLength LSB */
    0x00,         /* wTotalLength MSB */
    0x01,         /* bInCollection = 1 AudioStreaming interface */
    0x01,         /* baInterfaceNr(1) - AS#1 id */

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
};

/** Default USB control interface descriptors for speaker */
static const uint8 interface_descriptor_control_speaker[] =
{
    /* Class Specific Header */
    0x09,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = HEADER */
    0x00, 0x01, /* bcdADC = Audio Device Class v1.00 */
    0x09 + 0x0c + 0x0d + 0x09, /* wTotalLength LSB */
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
    0x02,         /* bNrChannels = 2 */
    0x03, 0x00,   /* wChannelConfig = left front and right front */
    0x00,         /* iChannelName = no string */
    0x00,         /* iTerminal = same as USB product string */
    
    /* Speaker Features */
    0x0d,           /*bLength*/
    0x24,           /*bDescriptorType = CS_INTERFACE */
    0x06,           /*bDescriptorSubType = FEATURE_UNIT*/
    SPEAKER_FU,     /*bUnitId*/
    SPEAKER_IT,     /*bSourceId - Speaker IT*/
    0x02,           /*bControlSize = 2 bytes per control*/
    0x01, 0x00,     /*bmaControls[0] = 0001 (Mute on Master Channel)*/
    0x02, 0x00,     /*bmaControls[1] = 0002 (Vol on Left Front)*/
    0x02, 0x00,     /*bmaControls[1] = 0002 (Vol on Right Front)*/
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

/** Default USB streaming interface descriptors for speaker */
static const uint8 interface_descriptor_streaming_speaker[] =
{
    /* Class Specific AS interface descriptor */
    0x07,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    SPEAKER_IT,   /* bTerminalLink = Speaker IT */
    0x00,         /* bDelay */
    0x01, 0x00, /* wFormatTag = PCM */

    /* Type 1 format type descriptor */
    0x08 + 0x03 *USB_STREAMING_NUM_OF_FREQUENCY,  /* bLength 8+((number of sampling frequencies)*3) */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = FORMAT_TYPE */
    0x01,         /* bFormatType = FORMAT_TYPE_I */
    0x02,         /* bNumberOfChannels */
    0x02,         /* bSubframeSize = 2 bytes */
    0x10,         /* bBitsResolution */
    USB_STREAMING_NUM_OF_FREQUENCY,         /* bSampleFreqType = 6 discrete sampling frequencies */
    SAMPLE_RATE_48K & 0xff, /* tSampleFreq = 48000*/
    (SAMPLE_RATE_48K >> 8) & 0xff,
    (SAMPLE_RATE_48K >> 16) & 0xff,
#ifndef USB_DEVICE_CLASS_BA_48KHz
    SAMPLE_RATE_44K1 & 0xff, /* tSampleFreq = 44100*/
    (SAMPLE_RATE_44K1 >> 8) & 0xff,
    (SAMPLE_RATE_44K1 >> 16) & 0xff,
    SAMPLE_RATE_32K & 0xff, /* tSampleFreq = 32000 */
    (SAMPLE_RATE_32K >> 8 ) & 0xff,
    (SAMPLE_RATE_32K >> 16) & 0xff,
    SAMPLE_RATE_22K05 & 0xff, /* tSampleFreq = 22050 */
    (SAMPLE_RATE_22K05 >> 8 ) & 0xff,
    (SAMPLE_RATE_22K05 >> 16) & 0xff,
    SAMPLE_RATE_16K & 0xff, /* tSampleFreq = 16000 */
    (SAMPLE_RATE_16K >> 8 ) & 0xff,
    (SAMPLE_RATE_16K >> 16) & 0xff,
    SAMPLE_RATE_8K & 0xff, /* tSampleFreq = 8000 */
    (SAMPLE_RATE_8K >> 8) & 0xff,
    (SAMPLE_RATE_8K >> 16) & 0xff,
#endif
    

    /* Class specific AS isochronous audio data endpoint descriptor */
    0x07,         /* bLength */
    0x25,         /* bDescriptorType = CS_ENDPOINT */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    0x01,         /* bmAttributes = SamplingFrequency control */
    0x02,         /* bLockDelayUnits = Decoded PCM samples */
    0x00, 0x00     /* wLockDelay */
};

/** Default USB streaming interface descriptors for mic */
static const uint8 interface_descriptor_streaming_mic[] =
{
    /* Class Specific AS interface descriptor */
    0x07,         /* bLength */
    0x24,         /* bDescriptorType = CS_INTERFACE */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    MIC_OT,       /* bTerminalLink = Microphone OT */
    0x00,         /* bDelay */
    0x01, 0x00, /* wFormatTag = PCM */

    /* Type 1 format type descriptor */
    0x08 + 0x03,/* bLength */
    0x24,        /* bDescriptorType = CS_INTERFACE */
    0x02,         /* bDescriptorSubType = FORMAT_TYPE */
    0x01,         /* bFormatType = FORMAT_TYPE_I */
    0x01,         /* bNumberOfChannels */
    0x02,         /* bSubframeSize = 2 bytes */
    0x10,         /* bBitsResolution */
    0x01,         /* bSampleFreqType = 1 discrete sampling freq */
    SAMPLE_RATE & 0xff, /* tSampleFreq */
    (SAMPLE_RATE >> 8) & 0xff,
    (SAMPLE_RATE >> 16) & 0xff,

    /* Class specific AS isochronous audio data endpoint descriptor */
    0x07,         /* bLength */
    0x25,         /* bDescriptorType = CS_ENDPOINT */
    0x01,         /* bDescriptorSubType = AS_GENERAL */
    0x00,         /* bmAttributes = none */
    0x02,         /* bLockDelayUnits = Decoded PCM samples */
    0x00, 0x00     /* wLockDelay */
};

/** Audio endpoint data, provides bRefresh and bSyncAddress */
static const uint8 audio_endpoint_user_data[] =
{
    0, /* bRefresh */
    0  /* bSyncAddress */
};


/*  Streaming Isochronous Endpoint. Maximum packet size 192 (stereo at 48khz) */
static const EndPointInfo epinfo_streaming_speaker[] =
{
    {
        end_point_iso_in, /* address */
        end_point_attr_iso, /* attributes */ /* TODO - B-75533 specify synchronous isochronous endpoints as DSP does rate matching */
        192, /* max packet size */
        1, /* poll_interval */
        audio_endpoint_user_data, /* data to be appended */
        sizeof(audio_endpoint_user_data) /* length of data appended */      
    }
};


/* Streaming Isochronous Endpoint. Maximum packet size 96 (mono at 48khz) */
static const EndPointInfo epinfo_streaming_mic[] =
{
    {
        end_point_iso_out, /* address */
        end_point_attr_iso, /* attributes */ /* TODO - B-75533 specify synchronous isochronous endpoints as DSP does rate matching */
        96, /* max packet size */
        1, /* poll_interval */
        audio_endpoint_user_data, /* data to be appended */
        sizeof(audio_endpoint_user_data), /* length of data appended */
    }
};

/** Default device configuration with interface and endpoint descriptors
 * for both speaker and mic */
static const usb_device_class_audio_config usb_audio_config_mic_and_speaker =
{
    {interface_descriptor_control_mic_and_speaker,
    sizeof(interface_descriptor_control_mic_and_speaker),
    NULL},
    {interface_descriptor_streaming_mic,
    sizeof(interface_descriptor_streaming_mic),
    epinfo_streaming_mic},
    {interface_descriptor_streaming_speaker,
    sizeof(interface_descriptor_streaming_speaker),
    epinfo_streaming_speaker}
};

/** Default device configuration with interface and endpoint descriptors
 * for just mic */
static const usb_device_class_audio_config usb_audio_config_mic =
{
    {interface_descriptor_control_mic,
    sizeof(interface_descriptor_control_mic),
    NULL},
    {interface_descriptor_streaming_mic,
    sizeof(interface_descriptor_streaming_mic),
    epinfo_streaming_mic},
    {NULL, 0, NULL}
};

/** Default device configuration with interface and endpoint descriptors
 * for just speaker */
static const usb_device_class_audio_config usb_audio_config_speaker =
{
    {interface_descriptor_control_speaker,
    sizeof(interface_descriptor_control_speaker),
    NULL},
    {NULL, 0, NULL},
    {interface_descriptor_streaming_speaker,
    sizeof(interface_descriptor_streaming_speaker),
    epinfo_streaming_speaker}
};

/** Stores device configuration with interface and endpoint descriptors */
static const usb_device_class_audio_config* usb_audio_config = NULL;

static bool usbEnumerateAudioMic(usb_device_class_descriptor descriptor);
static bool usbEnumerateAudioSpeaker(usb_device_class_descriptor descriptor);
static bool usbEnumerateAudioControl(usb_device_class_descriptor descriptor);
static void audioControlHandler(Task task, MessageId id, Message message);
static void audioStreamingHandler(Task task, MessageId id, Message message);
static void handleAudioControlClassRequest(Source req);
static void handleAudioStreamingClassRequest(usb_interface_type interface_type, Source req);


static uint8 usbAudioGetRate(usb_device_class_descriptor descriptor, uint32 *supported_rates)
{
    const uint8* ptr = descriptor.descriptor;
    const uint8* end = ptr + descriptor.size_descriptor;
    uint8 i;
    while(ptr < end)
    {
        const usb_format_descriptor* format = (const usb_format_descriptor*) ptr;
        if(format->header.type == B_DESCRIPTOR_TYPE_CS_INTERFACE)
        {
            if(format->header.sub_type == B_DESCRIPTOR_SUB_TYPE_FORMAT)
            {
                if(format->type == B_DESCRIPTOR_FORMAT_TYPE_I)
                {
                    for (i = 0; (i < format->num_freq) && (i < SUPPORTED_FREQUENCIES); i++)
                    {
                        supported_rates[i] = (format->freq[i*3]) | ((uint16)format->freq[(i*3)+1] << 8) | ((uint32)format->freq[(i*3)+2] << 16);
                    }
                    return format->num_freq;
                }
            }
        }       
        ptr += format->header.length;
    }
    return 0;
}


static bool usbEnumerateAudioMic(usb_device_class_descriptor descriptor)
{
    /* Add the microphone Audio Streaming Interface */
    device->usb_interface[usb_interface_audio_streaming_mic] = UsbAddInterface(&usb_codes_as, B_DESCRIPTOR_TYPE_CS_INTERFACE, descriptor.descriptor, descriptor.size_descriptor);
    
    if (device->usb_interface[usb_interface_audio_streaming_mic] == usb_interface_error)
        return FALSE;

    /* Add the microphone endpoint */
    if (UsbAddEndPoints(device->usb_interface[usb_interface_audio_streaming_mic], 1, descriptor.end_point_info) == FALSE)
        return FALSE;
    
    device->usb_task[usb_task_audio_streaming].handler = audioStreamingHandler;
    MessageStreamTaskFromSink(StreamUsbClassSink(device->usb_interface[usb_interface_audio_streaming_mic]), &device->usb_task[usb_task_audio_streaming]);
    
    /* set mic default volume */
    device->audio.in_vol = usb_audio_volume->mic_default;
    device->audio.in_mute = 0;
    
    return TRUE;
}

static bool usbEnumerateAudioSpeaker(usb_device_class_descriptor descriptor)
{
    /* Add the speaker Audio Streaming Interface */
    device->usb_interface[usb_interface_audio_streaming_speaker] = UsbAddInterface(&usb_codes_as, B_DESCRIPTOR_TYPE_CS_INTERFACE, descriptor.descriptor, descriptor.size_descriptor);
    
    if (device->usb_interface[usb_interface_audio_streaming_speaker] == usb_interface_error)
        return FALSE;

    /* Add the speaker endpoint */
    if (UsbAddEndPoints(device->usb_interface[usb_interface_audio_streaming_speaker], 1, descriptor.end_point_info) == FALSE)
        return FALSE;
    
    device->usb_task[usb_task_audio_streaming].handler = audioStreamingHandler;
    MessageStreamTaskFromSink(StreamUsbClassSink(device->usb_interface[usb_interface_audio_streaming_speaker]), &device->usb_task[usb_task_audio_streaming]);
    
    /* set speaker default volume */
    device->audio.out_l_vol = usb_audio_volume->speaker_default;
    device->audio.out_r_vol = device->audio.out_l_vol;
    device->audio.out_mute = 0;
            
    return TRUE;
}


static bool usbEnumerateAudioControl(usb_device_class_descriptor descriptor)
{
    /* Add an Audio Control Interface */
    device->usb_interface[usb_interface_audio_control] = UsbAddInterface(&usb_codes_ac, B_DESCRIPTOR_TYPE_CS_INTERFACE, descriptor.descriptor, descriptor.size_descriptor);
    if (device->usb_interface[usb_interface_audio_control] == usb_interface_error)
        return FALSE;
        
    device->usb_task[usb_task_audio_control].handler = audioControlHandler;
    MessageStreamTaskFromSink(StreamUsbClassSink(device->usb_interface[usb_interface_audio_control]), &device->usb_task[usb_task_audio_control]);
    
    return TRUE;
}


static void audioControlHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    if (id == MESSAGE_MORE_DATA)
    {
        PRINT(("USB: MESSAGE_MORE_DATA audio control\n"));
        handleAudioControlClassRequest(StreamUsbClassSource(device->usb_interface[usb_interface_audio_control]));
    }
}


static void audioStreamingHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    if (id == MESSAGE_MORE_DATA)
    {
        PRINT(("USB: MESSAGE_MORE_DATA audio streaming\n"));
        if (device->usb_classes & USB_DEVICE_CLASS_TYPE_AUDIO_SPEAKER)
        {
            handleAudioStreamingClassRequest(usb_interface_audio_streaming_speaker, StreamUsbClassSource(device->usb_interface[usb_interface_audio_streaming_speaker]));
        }
        if (device->usb_classes & USB_DEVICE_CLASS_TYPE_AUDIO_MICROPHONE)
        {
            handleAudioStreamingClassRequest(usb_interface_audio_streaming_mic, StreamUsbClassSource(device->usb_interface[usb_interface_audio_streaming_mic]));
        }
    }
}


static uint16* audioGetPtr(uint8 control, uint8 code, uint8 unit_id, uint8 channel)
{
    if(control == VOLUME_CONTROL)
    {
        PRINT(("%s VOLUME CODE - 0x%X CHANNEL - 0x%X\n", (unit_id == MIC_FU) ? "MIC" : "SPEAKER", code, channel));
        /* Can get any of CUR, MIN, MAX, RES for vol */
        if(unit_id == SPEAKER_FU)
        {
            if(code == REQ_CUR)
            {
                /* Get pointer to vol for this channel */
                if((channel == CHANNEL_LEFT) || (channel == CHANNEL_MASTER))
                    return (uint16*)&device->audio.out_l_vol;
                else if(channel == CHANNEL_RIGHT)
                    return (uint16*)&device->audio.out_r_vol;
            }
            else if(code == REQ_MIN)
                return (uint16*)&usb_audio_volume->speaker_min;
            else if(code == REQ_MAX)
                return (uint16*)&usb_audio_volume->speaker_max;
            else if(code == REQ_RES)
                return (uint16*)&usb_audio_volume->speaker_res;
        }
        else if(unit_id == MIC_FU)
        {
            if(code == REQ_CUR)
            {
                /* Get pointer to vol for this channel */
                if((channel == CHANNEL_LEFT) || (channel == CHANNEL_MASTER))
                    return (uint16*)&device->audio.in_vol;
            }
            else if(code == REQ_MIN)
                return (uint16*)&usb_audio_volume->mic_min;
            else if(code == REQ_MAX)
                return (uint16*)&usb_audio_volume->mic_max;
            else if(code == REQ_RES)
                return (uint16*)&usb_audio_volume->mic_res;
        }
    }
    else if(control == MUTE_CONTROL && code == REQ_CUR && channel == CHANNEL_MASTER)
    {
        PRINT(("%s MUTE CODE - 0x%X CHANNEL - 0x%X\n", (unit_id == MIC_FU) ? "MIC" : "SPEAKER", code, channel));
        /* Can only get CUR for mute */
        if(unit_id == MIC_FU)
        {
            return (uint16 *) &device->audio.in_mute;
        }
        else if(unit_id == SPEAKER_FU)
        {
            return (uint16 *) &device->audio.out_mute;
        }
    }
    /* Unsupported request */
    return NULL;
}


static uint16 audioGetLevel(Sink sink, uint8 control, uint8 code, uint8 unit_id, uint8 channel)
{
    uint8*  p_snk;
    
    /* Get pointer to requested value */
    uint16* pv =  audioGetPtr(control, code, unit_id, channel);
    
    if(pv)
    {
        PRINT(("VAL - 0x%X Control - %d Code - %d\n", *pv, control, code));
        if(control == MUTE_CONTROL && code == REQ_CUR)
        {
            /* Mute is 1 bytes */
            *claimSink(sink, 1) = (*pv) & 0xFF;
            return 1;
        }
        else if(control == VOLUME_CONTROL)
        {
            /* Volume is 2 bytes */
            p_snk    = claimSink(sink, 2);
            p_snk[0] = (*pv) & 0xFF;
            p_snk[1] = (*pv) >> 8;
            return 2;
        }
    }
    /* Unsupported request */
    return 0;
}


static void audioGainRound(uint8 hi, uint8 lo, int16* res)
{
    /* Convert hi byte to signed int */
    int16 temp = (hi & 0x80) ? (hi - 0xFF) : hi;
    /* Round up if lo byte >= 0.5 (where 1 = 1/256) */
    if(lo >= 0x80) temp ++;
    
    /* Round to nearest 3dB */
    for((*res) = -44; (*res) < 1; (*res)+= 3)
        if(temp <= (*res)) break;
    (*res)--;
}

/** Read the level value from the source, store it in the volume control
 * and notify Application with the USB_DEVICE_CLASS_MSG_AUDIO_LEVELS_IND
 * message. */
static bool audioSetLevel(Source source, uint8 control, uint8 code, uint8 unit_id, uint8 channel)
{
    int16 val = 0;
    const uint8* p_src = SourceMap(source);
    uint16* pv = audioGetPtr(control, code, unit_id, channel);
    
    if(control == MUTE_CONTROL)
    {
        if (SourceSize(source) >= 1)
            val = (int16)p_src[0];
    }
    else
    {
        if (SourceSize(source) >= 2)
        {
            if (device->audio.user_audio_levels)
            {
                /* if user defined levels then just store raw value */
                val = (int16)p_src[0] | (p_src[1] << 8);
            }
            else
            {
                /* round audio level */
                audioGainRound(p_src[1], p_src[0], &val);
            }
        }
    }
    
    /* Only allow setting current levels */
    if(pv && (code == REQ_CUR))
    {
        if((int16)(*pv) != val)
        {
            *pv = val;
            /* Level has changed, notify application */
            MessageCancelFirst(device->app_task, USB_DEVICE_CLASS_MSG_AUDIO_LEVELS_IND);
            MessageSendLater(device->app_task, USB_DEVICE_CLASS_MSG_AUDIO_LEVELS_IND, 0, 10);
        }
        PRINT(("VAL - 0x%X\n", *pv));
        /* Respond with success */
        return TRUE;
    }
    /* Unsupported request */
    return FALSE;
}


static void handleAudioControlClassRequest(Source req)
{
    uint16 packet_size;
    Sink sink = StreamSinkFromSource(req);
    uint8 channel;
    uint8 unit_id;
    uint8 code;
    uint8 control;
    
    /* Check for outstanding Class requests */
    while ((packet_size = SourceBoundary(req)) != 0)
    {
        /*
            Build the response. It must contain the original request,
            so copy from the source header.
        */
        UsbResponse usbresp;
        memcpy(&usbresp.original_request, SourceMapHeader(req), sizeof(UsbRequest));

        /* Reject by default */
        usbresp.data_length = 0;
        usbresp.success = FALSE;
        
        /* Get audio specific request info */
        channel = REQ_CN(usbresp.original_request);
        control = REQ_CS(usbresp.original_request);
        code    = REQ_CODE(usbresp.original_request);
        unit_id = REQ_UNIT(usbresp.original_request);
        
        if(REQ_IS_GET(usbresp.original_request))
        {
            /* Return value to USB host */
            PRINT(("GET\n"));
            usbresp.data_length = audioGetLevel(sink, control, code, unit_id, channel);
            usbresp.success = (usbresp.data_length != 0);
        }
        else
        {
            /* Update value sent by USB host */
            PRINT(("SET\n"));
            usbresp.success = audioSetLevel(req, control, code, unit_id, channel);
        }
        
        PRINT(("SUCCESS 0x%X\n", usbresp.success));
        
        /* Send response */
        if (usbresp.data_length)
        {
            (void)SinkFlushHeader(sink, usbresp.data_length, (void *)&usbresp, sizeof(UsbResponse));
        }
        else
        {
            /* Sink packets can never be zero-length, so flush a dummy byte */
            (void)SinkClaim(sink, 1);
            (void)SinkFlushHeader(sink, 1, (void *)&usbresp, sizeof(UsbResponse));
        }

        /* Discard the original request */
        SourceDrop(req, packet_size);
    }
}


static void handleAudioStreamingClassRequest(usb_interface_type interface_type, Source req)
{
    uint16 packet_size;
    Sink sink = StreamSinkFromSource(req);
    
    /* Check for outstanding Class requests */
    while ((packet_size = SourceBoundary(req)) != 0)
    {
        /*
            Build the response. It must contain the original request,
            so copy from the source header.
        */
        UsbResponse usbresp;
        memcpy(&usbresp.original_request, SourceMapHeader(req), sizeof(UsbRequest));

        /* Set the response fields to default values to make the code below simpler */
        usbresp.success = FALSE;
        usbresp.data_length = 0;

        /* Endpoint only allows SET_/GET_ of sampling frequency */
        if ((REQ_CS(usbresp.original_request) == SAMPLING_FREQ_CONTROL) && (usbresp.original_request.wLength == 3))
        {
            PRINT(("Sampling Freq Request %d\n", usbresp.original_request.wIndex));
            /* Check this is for a valid audio end point */
            if(usbresp.original_request.wIndex == end_point_iso_in || usbresp.original_request.wIndex == end_point_iso_out)
            {
                /* Get the supported rate */
                uint32 supported_rates[SUPPORTED_FREQUENCIES];
                uint8 num_freq;
                if(usbresp.original_request.wIndex == end_point_iso_in)
                {
                    num_freq = usbAudioGetRate(usb_audio_config->streaming_speaker, supported_rates);
                }
                else
                {
                    num_freq = usbAudioGetRate(usb_audio_config->streaming_mic, supported_rates);
                }
                
                if (usbresp.original_request.bRequest == SET_CUR)
                {
                    uint8 i;
                    const uint8* rate = SourceMap(req);
                    uint32 new_rate  = (uint32)rate[0] | ((uint32)rate[1] << 8) | ((uint32)rate[2] << 16);
                    /* Reject bad value */
                    for (i=0; i < num_freq; i++)
                    {
                        if (supported_rates[i] == new_rate)
                        {
                            MAKE_USB_DEVICE_CLASS_MESSAGE(USB_DEVICE_CLASS_SAMPLE_RATE);
                            message->sample_rate = new_rate; 
                            usbresp.success = TRUE;
                            /* store sample rate in static variable */
                            if(interface_type == usb_interface_audio_streaming_speaker)                            
                            {                                        
                                current_speaker_sample_freq = new_rate; 
                                PRINT(("Set Speaker Rate %lu - %X\n", new_rate, usbresp.success));
                                /* notify VM application of a change in sample rate */
                                MessageSend(device->app_task, USB_DEVICE_CLASS_MSG_SPEAKER_SAMPLE_RATE_IND, message);
                            }
                            else
                            {
                                current_mic_sample_freq = new_rate; 
                                PRINT(("Set MicRate %lu - %X\n", new_rate, usbresp.success));
                                /* notify VM application of a change in sample rate */
                                MessageSend(device->app_task, USB_DEVICE_CLASS_MSG_MIC_SAMPLE_RATE_IND, message);
                            }
                            break;
                        }
                    }
                }
                else if (usbresp.original_request.bRequest == GET_CUR)
                {
                    /* Return current value */
                    uint8 *ptr;
                    if ((ptr = claimSink(sink, 3)) != 0)
                    {
                        /* store sample rate in static variable */
                        if(interface_type == usb_interface_audio_streaming_speaker)                            
                        {
                            ptr[0] = (current_speaker_sample_freq & 0xff);
                            ptr[1] = (current_speaker_sample_freq >> 8);
                            ptr[2] = (current_speaker_sample_freq >> 16);
                            PRINT(("Get Speaker Rate %lu - %X\n", current_speaker_sample_freq, usbresp.success));
                        }
                        else
                        {                        
                            ptr[0] = (current_mic_sample_freq & 0xff);
                            ptr[1] = (current_mic_sample_freq >> 8);
                            ptr[2] = (current_mic_sample_freq >> 16);
                            PRINT(("Get Mic Rate %lu - %X\n", current_mic_sample_freq, usbresp.success));
                        }                        
                        usbresp.data_length = 3;
                        usbresp.success = TRUE;
                    }
                }
            }
        }

        /* Send response */
        if (usbresp.data_length)
        {
            (void)SinkFlushHeader(sink, usbresp.data_length, (void *)&usbresp, sizeof(UsbResponse));
        }
        else
        {
            /* Sink packets can never be zero-length, so flush a dummy byte */
            (void)SinkClaim(sink, 1);
            (void)SinkFlushHeader(sink, 1, (void *)&usbresp, sizeof(UsbResponse));
        }

        /* Discard the original request */
        SourceDrop(req, packet_size);
    }
}


bool usbConfigureAudioDescriptors(usb_device_class_config config, const usb_device_class_audio_config* params)
{
    /* Sanity check request */
    if(!params || (config != USB_DEVICE_CLASS_CONFIG_AUDIO_INTERFACE_DESCRIPTORS))
        return FALSE;
    /* Update audio config */
    usb_audio_config = params;
    return TRUE;
}


bool usbConfigureAudioVolume(usb_device_class_config config, const usb_device_class_audio_volume_config* params)
{
   
    /* Sanity check request */
    if(!params || (config != USB_DEVICE_CLASS_CONFIG_AUDIO_VOLUMES))
        return FALSE;
    
    /* Update audio config */
    usb_audio_volume = params;
  
    return TRUE;
}


bool usbEnumerateAudio(uint16 usb_device_class)
{
    if(!usb_audio_config)
    {
        if ((usb_device_class & USB_DEVICE_CLASS_TYPE_AUDIO_MICROPHONE) && (usb_device_class & USB_DEVICE_CLASS_TYPE_AUDIO_SPEAKER))
            usb_audio_config = &usb_audio_config_mic_and_speaker;
        else if (usb_device_class & USB_DEVICE_CLASS_TYPE_AUDIO_MICROPHONE)
            usb_audio_config = &usb_audio_config_mic;
        else if (usb_device_class & USB_DEVICE_CLASS_TYPE_AUDIO_SPEAKER)
            usb_audio_config = &usb_audio_config_speaker;
    }
    
    /* Attempt to register control interface if present */
    if(usb_audio_config->control.descriptor && !usbEnumerateAudioControl(usb_audio_config->control))
        return FALSE;
    /* Attempt to register mic interface if present */
    if(usb_audio_config->streaming_mic.descriptor && !usbEnumerateAudioMic(usb_audio_config->streaming_mic))
        return FALSE;
    /* Attempt to register speaker interface if present */
    if(usb_audio_config->streaming_speaker.descriptor && !usbEnumerateAudioSpeaker(usb_audio_config->streaming_speaker))
        return FALSE;
    
    StreamConfigure(VM_STREAM_USB_ALT_IF_MSG_ENABLED, 1);
    
    if (!usb_device_class_is_audio_volume_default(usb_audio_volume))
    {
        /* store that volumes are user defined */
        device->audio.user_audio_levels = TRUE;
    }
    
    return TRUE;
}

/** Return Sink for the mic */
Sink usbAudioMicSink(void)
{
    return StreamUsbEndPointSink(end_point_iso_out);
}

/** Return Source for the speaker */
Source usbAudioSpeakerSource(void)
{
    return StreamUsbEndPointSource(end_point_iso_in);
}

/** Return current levels */
void usbAudioGetLevels(usb_device_class_audio_levels* levels)
{
    /* Copy global structure into response */
    usb_device_class_audio_levels* audio_levels = levels;
    *audio_levels = device->audio;
    
    if (device->audio.user_audio_levels)
    {
        /* don't do any level conversion if user defined levels */       
    }
    else
    {
        /* Convert from dB to f/w setting */
        audio_levels->out_l_vol = VOL_SETTING(audio_levels->out_l_vol);
        audio_levels->out_r_vol = VOL_SETTING(audio_levels->out_r_vol);
    }
}

/** Return mic and speaker interface IDs */
UsbInterface usbAudioGetInterfaceId(usb_device_class_get_value interface)
{
    if(interface == USB_DEVICE_CLASS_GET_MIC_INTERFACE_ID)
    {
        if(device->usb_classes & USB_DEVICE_CLASS_TYPE_AUDIO_MICROPHONE)
            return device->usb_interface[usb_interface_audio_streaming_mic];
        else
            return usb_interface_error;
    }
    else
    {
        if(device->usb_classes & USB_DEVICE_CLASS_TYPE_AUDIO_SPEAKER)
            return device->usb_interface[usb_interface_audio_streaming_speaker];
        else
            return usb_interface_error;
    }
}

/** Access function to allow VM code to read sample frequency from lib */
uint32 usbAudioGetSpeakerSampleFreq(void)
{
    return current_speaker_sample_freq;
}

/** Access function to allow VM code to read sample frequency from lib */
uint32 usbAudioGetMicSampleFreq(void)
{
    return current_mic_sample_freq;
}

#else /* !USB_DEVICE_CLASS_REMOVE_AUDIO */
#ifndef HYDRACORE
/* ISO C forbids an empty source file */
static const int unused;
#endif /* HYDRACORE */
#endif /* USB_DEVICE_CLASS_REMOVE_AUDIO */

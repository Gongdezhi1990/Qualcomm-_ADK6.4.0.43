/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * USB device class library
 */

#include <panic.h>
#include <sink.h>
#include <stdlib.h>
#include <usb.h>

#include "usb_device_class.h"
#include "usb_device_audio.h"
#include "usb_device_battery_charging.h"
#include "usb_device_mass_storage.h"
#include "usb_device_hid.h"
#include "usb_device_class_private.h"

#include <print.h>
#include <string.h>


/** USB device class state */
device_class_state *device;

/** Helper function for allocating space in Sink */
uint8 *claimSink(Sink sink, uint16 size)
{
    uint8 *dest = SinkMap(sink);
    uint16 claim_result = SinkClaim(sink, size);
    if (claim_result == 0xffff)
    {
        return NULL;
    }
    return (dest + claim_result);
}

/** Configure some aspect of the USB operation */
usb_device_class_status UsbDeviceClassConfigure(usb_device_class_config config,
        uint16 value_16, uint32 value_32, const uint8 *params)
{
#if defined USB_DEVICE_CLASS_REMOVE_MASS_STORAGE && defined USB_DEVICE_CLASS_REMOVE_AUDIO
    UNUSED(config);
    UNUSED(params);
#endif

#ifndef USB_DEVICE_CLASS_REMOVE_MASS_STORAGE
    if (device && (device->usb_classes & USB_DEVICE_CLASS_TYPE_MASS_STORAGE))
    {
        /* Return success if this is a mass storage config request */
        if(usbConfigureMassStorage(config, value_16, value_32, (uint8 *)params))
        {
            return usb_device_class_status_success;
        }
    }
#else
    UNUSED(value_16);
    UNUSED(value_32);
#endif   
#ifndef USB_DEVICE_CLASS_REMOVE_AUDIO
    if (config == USB_DEVICE_CLASS_CONFIG_AUDIO_INTERFACE_DESCRIPTORS)
    {
        /* Return success if this is an audio descriptors config request */
        if(usbConfigureAudioDescriptors(config,
                                  (const usb_device_class_audio_config*)params))
        {
            return usb_device_class_status_success;
        }
    }
    else if (config == USB_DEVICE_CLASS_CONFIG_AUDIO_VOLUMES)
    {        
        /* Return success if this is an audio volume config request */
        if(usbConfigureAudioVolume(config,
                           (const usb_device_class_audio_volume_config*)params))
        {
            return usb_device_class_status_success;
        }
    }
#endif
#ifndef USB_DEVICE_CLASS_REMOVE_HID
    if (config == USB_DEVICE_CLASS_CONFIG_HID_CONSUMER_TRANSPORT_DESCRIPTORS)
    {
        /* Return success if this is an HID config request */
        if (usbConfigureHidConsumerTransport(config,
        (const usb_device_class_hid_consumer_transport_config*)params))
        {
            return usb_device_class_status_success;
        }
    }
    
#endif


    return usb_device_class_status_invalid_param_value;
}

/** Enumerate the device as one or more of the device classes supplied */
usb_device_class_status UsbDeviceClassEnumerate(Task app_task,
                                                uint16 usb_device_class)
{
    usb_device_class_status status =
            usb_device_class_status_invalid_param_value;
    
    device = PanicUnlessNew(device_class_state);
    memset(device, 0, sizeof(device_class_state));
    PRINT(("USB: Allocate memory [0x%x]\n", sizeof(device_class_state)));
    
#ifndef USB_DEVICE_CLASS_REMOVE_AUDIO    
    if ((status != usb_device_class_status_failed_enumeration) &&
        ((usb_device_class & USB_DEVICE_CLASS_TYPE_AUDIO_MICROPHONE) || (usb_device_class & USB_DEVICE_CLASS_TYPE_AUDIO_SPEAKER)))
    {
        status = (usb_device_class_status)(
                (usbEnumerateAudio(usb_device_class) == TRUE) ?
                        usb_device_class_status_success :
                        usb_device_class_status_failed_enumeration);
    }
#endif   
    
#ifndef USB_DEVICE_CLASS_REMOVE_MASS_STORAGE    
    if ((status != usb_device_class_status_failed_enumeration) &&
        (usb_device_class & USB_DEVICE_CLASS_TYPE_MASS_STORAGE))
    {
        status = (usb_device_class_status)(
                (usbEnumerateMassStorage() == TRUE) ?
                        usb_device_class_status_success :
                        usb_device_class_status_failed_enumeration);
    }
#endif    
 
#ifndef USB_DEVICE_CLASS_REMOVE_CHARGING    
    if ((status != usb_device_class_status_failed_enumeration) &&
        (usb_device_class & USB_DEVICE_CLASS_TYPE_BATTERY_CHARGING))
    {
        status = (usb_device_class_status)(
                (usbEnumerateBatteryCharging(app_task) == TRUE) ?
                        usb_device_class_status_success :
                        usb_device_class_status_failed_enumeration);
    }
#endif
    
#ifndef USB_DEVICE_CLASS_REMOVE_HID    
    if ((status != usb_device_class_status_failed_enumeration) &&
        ((usb_device_class & USB_DEVICE_CLASS_TYPE_HID_CONSUMER_TRANSPORT_CONTROL) || 
         (usb_device_class & USB_DEVICE_CLASS_TYPE_HID_KEYBOARD) ||
         (usb_device_class & USB_DEVICE_CLASS_TYPE_HID_DATALINK_CONTROL)))
    {
        status = (usb_device_class_status)(
                (usbEnumerateHid(usb_device_class) == TRUE) ?
                        usb_device_class_status_success :
                        usb_device_class_status_failed_enumeration);
    }
#endif    
    
    if (status == usb_device_class_status_success)
    {
        device->app_task = app_task;
        device->usb_classes = usb_device_class;
        (void)MessageSystemTask(app_task);
    }
    else
    {
        free(device);
    }

    return status;    
}

/** Get the current value of a certain USB feature */
usb_device_class_status UsbDeviceClassGetValue(usb_device_class_get_value id, void *value)
{
#if defined USB_DEVICE_CLASS_REMOVE_AUDIO && defined USB_DEVICE_CLASS_REMOVE_MASS_STORAGE
    UNUSED(value);
#endif

    switch (id)
    {
#ifndef USB_DEVICE_CLASS_REMOVE_AUDIO 
        case USB_DEVICE_CLASS_GET_VALUE_AUDIO_SOURCE:
        {
            * (Source *) value = usbAudioSpeakerSource();
            return usb_device_class_status_success;
        }
        case USB_DEVICE_CLASS_GET_VALUE_AUDIO_SINK:
        {
            * (Sink *) value = usbAudioMicSink();
            return usb_device_class_status_success;
        }
        case USB_DEVICE_CLASS_GET_VALUE_AUDIO_LEVELS:
        {
            usbAudioGetLevels(value);
            return usb_device_class_status_success;
            
        }
        case USB_DEVICE_CLASS_GET_MIC_INTERFACE_ID:
        case USB_DEVICE_CLASS_GET_SPEAKER_INTERFACE_ID:
        {
            * (UsbInterface *) value = usbAudioGetInterfaceId(id);
            return usb_device_class_status_success;
        }
        case USB_DEVICE_CLASS_GET_VALUE_SPEAKER_SAMPLE_FREQ:
        {
            * (uint32 *) value = usbAudioGetSpeakerSampleFreq();
            return usb_device_class_status_success;
        }
        case USB_DEVICE_CLASS_GET_VALUE_MIC_SAMPLE_FREQ:
        {
            * (uint32 *) value = usbAudioGetMicSampleFreq();
            return usb_device_class_status_success;
        }
#endif
#ifndef USB_DEVICE_CLASS_REMOVE_MASS_STORAGE
        case USB_DEVICE_CLASS_GET_VALUE_MASS_STORAGE_SOURCE:
        {
            * (Source *) value = usbMassStorageSource();
            return usb_device_class_status_success;
        }
        case USB_DEVICE_CLASS_GET_VALUE_MASS_STORAGE_SINK:
        {
            * (Sink *) value = usbMassStorageSink();
            return usb_device_class_status_success;
        }
#endif
        default:
        {
            break;
        }
    }
    
    return usb_device_class_status_invalid_param_value;
}

/** Send a predefined event over USB */
usb_device_class_status UsbDeviceClassSendEvent(usb_device_class_event event)
{
#ifndef USB_DEVICE_CLASS_REMOVE_HID    
    if ((event <= USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_MUTE) &&
        (device->usb_classes &
                USB_DEVICE_CLASS_TYPE_HID_CONSUMER_TRANSPORT_CONTROL))
    {
        return usbSendDefaultHidConsumerEvent(event);              
    }
    /*lint -e{685} Lint complains that (event <= ..._KEYBOARD_ALT_END)
     * is always true.*/
    if ((event <= USB_DEVICE_CLASS_EVENT_HID_KEYBOARD_ALT_END) &&
        (device->usb_classes & USB_DEVICE_CLASS_TYPE_HID_KEYBOARD))
    {
        return usbSendDefaultHidKeyboardEvent(event);              
    }
#endif    
    
    return usb_device_class_status_invalid_param_value;
}


/** Send a report over the USB interface */
usb_device_class_status UsbDeviceClassSendReport(
        usb_device_class_type class_type, uint16 report_id,
        uint16 size_report, uint8 *report)
{
#ifndef USB_DEVICE_CLASS_REMOVE_HID
    if ((class_type == USB_DEVICE_CLASS_TYPE_HID_CONSUMER_TRANSPORT_CONTROL) &&
        (device->usb_classes &
                USB_DEVICE_CLASS_TYPE_HID_CONSUMER_TRANSPORT_CONTROL))
    {
        if (usbSendReportHidConsumerTransport(report_id, size_report, report))
            return usb_device_class_status_success;
    }
#endif

    return usb_device_class_status_invalid_param_value;
}

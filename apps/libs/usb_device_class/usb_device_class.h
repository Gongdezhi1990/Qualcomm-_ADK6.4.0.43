/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    usb_device_class.h

DESCRIPTION
    Header file for the USB Device Class library.
*/

/*!
@file    usb_device_class.h
@brief   Header file for the USB Device Class library.

        This file documents the USB Device Class library API. 
        
*/

#ifndef _USB_DEVICE_CLASS_H
#define _USB_DEVICE_CLASS_H


#include <message.h>
#include <library.h>


/*!
    @brief Possible return status of a called API. 
*/
typedef enum
{
    /*! The API call was successful */
    usb_device_class_status_success,
    /*! A parameter passed to the API was invalid */
    usb_device_class_status_invalid_param_value, 
    /*! The enumeration of the USB device was unsuccessful. The Host Interface should be checked that it is set to USB. */
    usb_device_class_status_failed_enumeration, 
    /*! The attempt to connect up the USB ports was unsuccessful */
    usb_device_class_status_failed_port_connection 
} usb_device_class_status;


/*!
    Possible USB device classes that can be handled by the library. These values can be passed into the UsbDeviceClassEnumerate API.
*/
typedef enum
{
    /*! USB audio microphone device class */
    USB_DEVICE_CLASS_TYPE_AUDIO_MICROPHONE = 0x01, 
    /*! USB audio speaker device class */
    USB_DEVICE_CLASS_TYPE_AUDIO_SPEAKER = 0x02, 
    /*! USB battery charging device class */
    USB_DEVICE_CLASS_TYPE_BATTERY_CHARGING = 0x04,
    /*! USB mass storage device class */
    USB_DEVICE_CLASS_TYPE_MASS_STORAGE = 0x08, 
    /*! USB HID consumer transport control device class */
    USB_DEVICE_CLASS_TYPE_HID_CONSUMER_TRANSPORT_CONTROL = 0x10, 
    /*! USB HID keyboard device class */
    USB_DEVICE_CLASS_TYPE_HID_KEYBOARD = 0x20,
    /*! USB HID upgrade control device class */
    USB_DEVICE_CLASS_TYPE_HID_DATALINK_CONTROL = 0x40,

    FORCE_ENUM_TO_MIN_16BIT(usb_device_class_type)
} usb_device_class_type;


/*!
    Possible configuration values that can be passed into the UsbDeviceClassConfigure API.
*/
typedef enum
{
    /*! Configure the FAT table for the mass storage device */
    USB_DEVICE_CLASS_CONFIG_MASS_STORAGE_FAT_TABLE,
    /*! Configure the FAT root directory entries for the mass storage device */
    USB_DEVICE_CLASS_CONFIG_MASS_STORAGE_FAT_ROOT_DIR,
    /*! Configure the FAT data area for the mass storage device */
    USB_DEVICE_CLASS_CONFIG_MASS_STORAGE_FAT_DATA_AREA,
    /*! Configure audio streaming/control interface descriptors (see usb_device_class_audio_config) */
    USB_DEVICE_CLASS_CONFIG_AUDIO_INTERFACE_DESCRIPTORS,
    /*! Configure HID consumer transport interface descriptors (see usb_device_class_hid_consumer_transport_config) */
    USB_DEVICE_CLASS_CONFIG_HID_CONSUMER_TRANSPORT_DESCRIPTORS,
    /*! Configure audio volumes (see usb_device_class_audio_volume_config) */
    USB_DEVICE_CLASS_CONFIG_AUDIO_VOLUMES,
    /*! Configure HID datalink transport interrupt endpoint task. */
    USB_DEVICE_CLASS_CONFIG_HID_DATALINK_INPUT_TASK
} usb_device_class_config;


/*!
    Possible values that can be passed into the UsbDeviceClassGetValue API.
*/
typedef enum
{
    /*! Retrieve the USB Speaker Source */
    USB_DEVICE_CLASS_GET_VALUE_AUDIO_SOURCE,
    /*! Retrieve the USB Microphone Sink */
    USB_DEVICE_CLASS_GET_VALUE_AUDIO_SINK,
    /*! Retrieve the USB Audio Levels */
    USB_DEVICE_CLASS_GET_VALUE_AUDIO_LEVELS,
    /*! Retrieve the USB Mic Interface ID */
    USB_DEVICE_CLASS_GET_MIC_INTERFACE_ID,
    /*! Retrieve the USB Speaker Interface ID */
    USB_DEVICE_CLASS_GET_SPEAKER_INTERFACE_ID,
    /*! Retrieve the USB Mass Storage Source */
    USB_DEVICE_CLASS_GET_VALUE_MASS_STORAGE_SOURCE,
    /*! Retrieve the USB Mass Storage Sink */
    USB_DEVICE_CLASS_GET_VALUE_MASS_STORAGE_SINK,
    /*! Retrieve the USB Speaker Sample Freq */
    USB_DEVICE_CLASS_GET_VALUE_SPEAKER_SAMPLE_FREQ,
    /*! Retrieve the USB Mic Sample Freq */
    USB_DEVICE_CLASS_GET_VALUE_MIC_SAMPLE_FREQ
} usb_device_class_get_value;


/*!
    Possible values that can be returned in the value parameter of the UsbDeviceClassGetValue API. These values will only be returned if USB_DEVICE_CLASS_GET_VALUE_CHARGER_STATUS was
    passed into the API.
*/
typedef enum
{
    /*! The USB charger is detached */
    USB_DEVICE_CLASS_VALUE_CHARGER_STATUS_DETACHED, 
    /*! The USB charger is a host or hub */
    USB_DEVICE_CLASS_VALUE_CHARGER_STATUS_HOST_OR_HUB, 
    /*! The USB charger is a dedicated charger */
    USB_DEVICE_CLASS_VALUE_CHARGER_STATUS_DEDICATED_CHARGER 
} usb_device_class_value_charger_status;


/*!
    Possible values that can be passed into the UsbDeviceClassSendEvent API. Events are only valid if the correct device class has been enumerated.
*/
typedef enum
{
    /*! Send a HID consumer play/pause event over USB */
    USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_PLAY_PAUSE, 
    /*! Send a HID consumer stop event over USB */
    USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_STOP, 
    /*! Send a HID consumer next track event over USB */
    USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_NEXT_TRACK, 
    /*! Send a HID consumer previous track event over USB */
    USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_PREVIOUS_TRACK, 
    /*! Send a HID consumer play event over USB */
    USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_PLAY, 
    /*! Send a HID consumer pause event over USB */
    USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_PAUSE, 
    /*! Send a HID consumer Fast Forward ON event over USB */
    USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_FFWD_ON,
    /*! Send a HID consumer Fast Forward OFF event over USB */
    USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_FFWD_OFF,
    /*! Send a HID consumer Rewind ON event over USB */
    USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_REW_ON,
    /*! Send a HID consumer Rewind OFF event over USB */
    USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_REW_OFF,
    /*! Send a HID consumer Volume Up event over USB */
    USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_VOL_UP,
    /*! Send a HID consumer Volume Down event over USB */
    USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_VOL_DOWN,
    /*! Send a HID consumer Mute event over USB */
    USB_DEVICE_CLASS_EVENT_HID_CONSUMER_TRANSPORT_MUTE,
    /*! Send a HID keyboard ALT+PageUp event over USB */
    USB_DEVICE_CLASS_EVENT_HID_KEYBOARD_ALT_PGUP, 
    /*! Send a HID keyboard ALT+PageDown event over USB */
    USB_DEVICE_CLASS_EVENT_HID_KEYBOARD_ALT_PGDN,
    /*! Send a HID keyboard ALT+End event over USB */
    USB_DEVICE_CLASS_EVENT_HID_KEYBOARD_ALT_END
} usb_device_class_event;


/*!
    Structure to pass when calling UsbDeviceClassGetValue to obtain USB_DEVICE_CLASS_GET_VALUE_AUDIO_LEVELS
*/
typedef struct
{
    /*! Left and right gain settings */
    int16     out_l_vol;
    int16     out_r_vol;
    /*! Input Gain setting */
    int16     in_vol;
    /*! Speaker/mic mute settings (TRUE/FALSE) */
    uint16    out_mute;
    uint16    in_mute;
    /*! Flag to indicate if the audio levels are user supplied */
    bool      user_audio_levels;
} usb_device_class_audio_levels;


/*!
    Structure to hold interface descriptor. Descriptors must be constant, or if allocated may not be freed.
*/
typedef struct
{
    const uint8*        descriptor;
    uint16              size_descriptor;
    const EndPointInfo* end_point_info;
} usb_device_class_descriptor;


/*!
    Structure to pass as params when calling UsbDeviceClassConfigure to set USB_DEVICE_CLASS_CONFIG_AUDIO_INTERFACE_DESCRIPTORS.
    Interface descriptors must be set prior to calling UsbDeviceClassEnumerate if the application wishes to use custom descriptors, 
    otherwise defaults will be used. This structure must contain all descriptors required for the enumerated audio device classes.
    Terminal and unit ID's must match those used by the library, i.e. Microphone(4) -> Feature Unit(5) -> USB Output Terminal(6)
                                                                      USB Input Terminal(1) -> Feature Unit(2) -> Speaker(3)
*/
typedef struct
{
    usb_device_class_descriptor    control;
    usb_device_class_descriptor    streaming_mic;
    usb_device_class_descriptor    streaming_speaker;
} usb_device_class_audio_config;


/*!
    Structure to pass as params when calling UsbDeviceClassConfigure to set USB_DEVICE_CLASS_CONFIG_HID_CONSUMER_TRANSPORT_DESCRIPTORS.
    Interface descriptors must be set prior to calling UsbDeviceClassEnumerate if the application wishes to use custom descriptors, 
    otherwise defaults will be used.
    
*/
typedef struct
{
    usb_device_class_descriptor    interface;
    usb_device_class_descriptor    report;    
} usb_device_class_hid_consumer_transport_config;


/*!
    Structure to pass as params when calling UsbDeviceClassConfigure to set USB_DEVICE_CLASS_CONFIG_HID_CONSUMER_TRANSPORT_DESCRIPTORS.
    Interface descriptors must be set prior to calling UsbDeviceClassEnumerate if the application wishes to use custom descriptors, 
    otherwise defaults will be used.
    
*/
typedef struct
{
    uint16 speaker_min;
    uint16 speaker_max;
    uint16 speaker_res;
    uint16 speaker_default;
    uint16 mic_min;
    uint16 mic_max;
    uint16 mic_res;
    uint16 mic_default;    
} usb_device_class_audio_volume_config;


/*!
    @brief Define the types for the upstream messages sent from the USB device
    class library to the application.
*/

/*
    Do not document this enum.
*/
#ifndef DO_NOT_DOCUMENT

#ifndef MESSAGE_MAKE
/* 
   Normally picked up from message.h, but might not be present pre
   4.1.1h, so fall back to the old behaviour.
*/

/*lint --e{773}
 * we do not want parenthesis around this macro as it is used for
 * code generation */
#define MESSAGE_MAKE(N,T) T *N = PanicUnlessNew(T)

#endif /* MESSAGE_MAKE */

#define MAKE_USB_DEVICE_CLASS_MESSAGE(TYPE) MESSAGE_MAKE(message,TYPE##_T);


typedef struct
{
    uint32 sample_rate;
} USB_DEVICE_CLASS_SAMPLE_RATE_T;

typedef enum
{
    USB_DEVICE_CLASS_MSG_AUDIO_LEVELS_IND = USB_DEVICE_CLASS_MSG_BASE,
    USB_DEVICE_CLASS_MSG_REPORT_IND,
    USB_DEVICE_CLASS_MSG_SPEAKER_SAMPLE_RATE_IND,
    USB_DEVICE_CLASS_MSG_MIC_SAMPLE_RATE_IND,

    /* Library message limit */
    USB_DEVICE_CLASS_MSG_TOP
} usb_device_class_message;

typedef struct
{
    usb_device_class_type class_type;
    uint16 report_id;
    uint16 size_report;
    uint8 report[1];
} USB_DEVICE_CLASS_MSG_REPORT_IND_T;


#endif /*end of DO_NOT_DOCUMENT*/


/*!
    @brief This API can be used to configure some aspect of the USB operation. 
    Could be called before or after UsbDeviceClassEnumerate, depending on the operation.    

    @param config The specific item to configure
    
    @param value_16 The 16 bit configuration value
    
    @param value_32 The 32 bit configuration value
    
    @param params Pointer to configuration data
*/
usb_device_class_status UsbDeviceClassConfigure(usb_device_class_config config, uint16 value_16, uint32 value_32, const uint8 *params);


/*!
    @brief This API is used to enumerate the device as one or more of the device classes supplied. 
    Some combination of classes maybe invalid in which case the function would return failure and no enumeration would be performed. 
    This API should be called in the time critical _init application function.
    
    @param app_task The task to receive any messages returned from the library. 
    For battery charging or mic/speaker enumeration this should be the system Task which receives all MESSAGE_USB_xxx messages.
    For mic/speaker enumeration MESSAGE_USB_ALT_INTERFACE can be used to determine if audio should be routed. The interface ID 
    for mic/speaker can be obtained using UsbDeviceClassGetValue().If an alternate setting of 1 (audio streaming) is selected 
    for these interfaces then audio is present, if an alternate setting of 0 (audio control) is selected then audio is not present. 
    
    @param usb_device_class One or more of the values from usb_device_class_type, indicating which USB device classes to enumerate as.
*/
usb_device_class_status UsbDeviceClassEnumerate(Task app_task, uint16 usb_device_class);


/*!
    @brief This API is used to get the current value of a certain USB feature.

    @param id The ID of the value to retrieve
    @param value Where the returned value should be stored
*/
usb_device_class_status UsbDeviceClassGetValue(usb_device_class_get_value id, void *value);


/*!
    @brief This API is used to send a predefined event over USB.

    @param event The event to send over USB. Events will only be sent if the correct device class has been enumerated.  
*/
usb_device_class_status UsbDeviceClassSendEvent(usb_device_class_event event);


/*!
    @brief This API is used to send a report over the USB interface.

    @param class_type One of the values from usb_device_class_type, indicating which USB device class to send the report for.  
    @param report_id The report identifier for the report data.
    @param size_report The size of the report data which is being sent.
    @param report The report data which is being sent.
*/	
usb_device_class_status UsbDeviceClassSendReport(
        usb_device_class_type class_type, uint16 report_id,
        uint16 size_report, uint8 *report);
		
/*!
    @brief This API is used to send via HID datalink USB interface.

    @param data pointer to the data to be send, this content is copied internally.
    @param size size of the data to de sent.
    @param report_id report ID to use for the message.
*/
usb_device_class_status usb_HidDatalinkSendDataRequest(
        uint8 * data, uint16 size, uint8 report_id);
#endif /* USB_DEVICE_CLASS_H */

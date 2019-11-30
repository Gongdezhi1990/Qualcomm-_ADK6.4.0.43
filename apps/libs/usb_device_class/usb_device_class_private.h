/*******************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/


#ifndef _USB_DEVICE_CLASS_PRIVATE_H
#define _USB_DEVICE_CLASS_PRIVATE_H


#define I_INTERFACE_INDEX 0x00

/** USB interface types */
typedef enum
{
    usb_interface_audio_control,
    usb_interface_audio_streaming_mic,
    usb_interface_audio_streaming_speaker,
    usb_interface_hid_consumer_transport,
    usb_interface_hid_keyboard,
    usb_interface_mass_storage,
    usb_interface_hid_datalink,
    usb_interface_max /* must leave as final entry in enum */       
} usb_interface_type;

#define MAX_USB_INTERFACES usb_interface_max

/** USB task types */
typedef enum
{
    usb_task_audio_control,
    usb_task_audio_streaming,
    usb_task_hid_consumer,
    usb_task_hid_keyboard,
    usb_task_hid_datalink,
    usb_task_mass_storage_default,
    usb_task_mass_storage_bulk,
    usb_task_max /* must leave as final entry in enum */       
} usb_task_type;

#define MAX_USB_TASKS usb_task_max

/** USB device class state */
typedef struct
{
    Task app_task;
    uint16 usb_classes;
    UsbInterface usb_interface[MAX_USB_INTERFACES];
    Sink datalink_class_sink;
    Source datalink_class_source;
    Sink datalink_ep_sink;
    TaskData usb_task[MAX_USB_TASKS];
#ifndef USB_DEVICE_CLASS_REMOVE_AUDIO
    usb_device_class_audio_levels audio;
#endif
} device_class_state;

/** USB device class state */
extern device_class_state *device;

#define SPEAKER_INTF \
    device->usb_interface[usb_interface_audio_streaming_speaker]
#define MIC_INTF \
    device->usb_interface[usb_interface_audio_streaming_mic]
#define CTRL_INTF \
    device->usb_interface[usb_interface_audio_control]
#define HID_CONS_INTF \
    device->usb_interface[usb_interface_hid_consumer_transport]
#define HID_KEYB_INTF \
    device->usb_interface[usb_interface_hid_keyboard]
#define HID_DATA_INTF \
    device->usb_interface[usb_interface_hid_datalink]

#define STREAM_TASK \
    device->usb_task[usb_task_audio_streaming]
#define CTRL_TASK \
    device->usb_task[usb_task_audio_control]
#define HID_CONS_TASK \
    device->usb_task[usb_task_hid_consumer]
#define HID_KEYB_TASK \
    device->usb_task[usb_task_hid_keyboard]
#define HID_DATA_TASK \
    device->usb_task[usb_task_hid_datalink]

/** Helper function for allocating space in Sink */
uint8 *claimSink(Sink sink, uint16 size);


#endif /* _USB_DEVICE_CLASS_PRIVATE_H */

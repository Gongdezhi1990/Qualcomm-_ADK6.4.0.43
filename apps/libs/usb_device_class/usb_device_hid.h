/*******************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#ifndef USB_DEVICE_CLASS_REMOVE_HID

#ifndef _USB_DEVICE_HID_H
#define _USB_DEVICE_HID_H

#define HID_REPORTID_DATA_TRANSFER          1   /* data from host for AHI */
#define HID_REPORTID_RESPONSE               2   /* device response for AHI */
#define HID_REPORTID_COMMAND                3   /* command channel */
#define HID_REPORTID_CONTROL                4   /* control channel dedicated to HID library */
#define HID_REPORTID_UPGRADE_DATA_TRANSFER  5   /* data from host for Upgrade */
#define HID_REPORTID_UPGRADE_RESPONSE       6   /* device response for Upgrade */


#define B_INTERFACE_CLASS_HID 0x03
#define B_INTERFACE_SUB_CLASS_HID_NO_BOOT 0x00
#define B_INTERFACE_PROTOCOL_HID_NO_BOOT 0x00      

#define HID_DESCRIPTOR_LENGTH 9
#define B_DESCRIPTOR_TYPE_HID 0x21
#define B_DESCRIPTOR_TYPE_HID_REPORT 0x22
#define HID_KEYBD_REPORT_DESCRIPTOR_LENGTH 44

#define USB_REPORT_TYPE_INPUT (1 << 8)


bool usbEnumerateHid(uint16 usb_device_class);

bool usbConfigureHidConsumerTransport(usb_device_class_config config, const usb_device_class_hid_consumer_transport_config* params);

bool usbSendRawEventHidKeycode(usb_device_class_type, uint16 size_params, const uint8 *params);

usb_device_class_status usbSendDefaultHidConsumerEvent(usb_device_class_event event);

usb_device_class_status usbSendDefaultHidKeyboardEvent(usb_device_class_event event);

bool usbSendReportHidConsumerTransport(uint16 report_id, uint16 size_report, uint8 *report);

        
#endif /* _USB_DEVICE_HID_H */

#endif /* !USB_DEVICE_CLASS_REMOVE_HID */

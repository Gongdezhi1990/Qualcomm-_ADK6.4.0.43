/*****************************************************************
Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_iap2_remote.h

DESCRIPTION
....Implement MFI Headset remote capability(hid over iAP2)

*/
#ifndef _SINK_IAP2_REMOTE_H_
#define _SINK_IAP2_REMOTE_H_

/* See iAP2 specification, HID Headset Remote */

#define HID_COMPONENT_ID    1


#define HID_VOLUME_UP           0x01
#define HID_VOLUME_DOWN      0x02
#define HID_CENTER                  0x04

#define HID_REPORT_HEADER         10        /* Length of StartHID message not including the report descriptor */
#define HID_LENGTH                        35       /* Length of the report descriptor */
#define HID_REMOTE_REPORT_LEN   1       /* Bytes of payload data in an AccessoryHIDReport message */

#define PSKEY_USB_VENDOR_ID      0x02BE
#define PSKEY_USB_PRODUCT_ID    0x02BF

#define HID_USB_VENDOR_OFF       10
#define HID_USB_PRODUCT_OFF     16

#define IAP2_SIRI_ON_TONE       68 /* G Major Arp up */
#define IAP2_SIRI_OFF_TONE      69  /* G Major Arp down */

typedef enum {
    iap2_remote_ret_status_failure = 0,
    iap2_remote_ret_status_success

}sink_iap2_remote_ret_value;
/*
 * Initialize HID Headset Remote
 */
bool sinkIap2RemoteInit(iap2_link_priority device_id);

void sinkIap2ReportUserEvent(uint16 id, iap2_link_priority device_id);

void sinkIap2RemotePayloadTransmission(const uint8 *data, iap2_link_priority device_id);


#endif

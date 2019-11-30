/*****************************************************************
Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_iap2_remote.c

DESCRIPTION
   Implement MFI Headset remote capability(hid over iAP2)
*/

#include <stdlib.h>
#include <ps.h>
#include <bdaddr.h>
#ifdef ENABLE_IAP2
#include <iap2.h>

#include "sink_accessory.h"
#include "sink_iap2_remote.h"
#include "hfp.h"
#include "sink_a2dp.h"
#include "sink_avrcp.h"

/*
 * See MFI Accessory Interface Specification , HID Headset Remote, Headset Remote Examples,
 * HID Remote Example (telephony) Report descriptor
 */
#include "sink_events.h"


static const uint8 hremote_hid_descriptor[] = {
    0x00, 0x06, 0x00, 0x00,         /* parameter 0 - HID component ID */
            0x00, HID_COMPONENT_ID,
    0x00, 0x06, 0x00, 0x01,         /* parameter 1 - USB vendor ID */
            0, 0,
    0x00, 0x06, 0x00, 0x02,         /* parameter 2 - USB product ID */
            0, 0,
    0x00, HID_LENGTH+4, 0x00, 0x04, /* parameter 4 - HID descriptor */

    /* this is the actual HID report descriptor. See the reference above  to the iAP2 specification */

    0x05, 0x0c, /* USAGE PAGE (Consumer Devices) */
    0x09, 0x01, /* USAGE (Consumer Control) */
    0xA1, 0x01, /* COLLECTION (Application) */
        0x15, 0x00, /* LOGICAL MINIMUM (0) */
        0x25, 0x01, /* LOGICAL MAXIMUM (1) */
        0x75, 0x01, /* REPORT SIZE (1) */
        0x95, 0x02, /* REPORT COUNT (2) */
        0x09, 0xE9, /* USAGE (Volume Up) */
        0x09, 0xEA, /* USAGE (Volume Down) */
        0x81, 0x02, /* INPUT (data, Var, Abs) */
        0x05, 0x0B, /* USAGE PAGE (Telephony) */
        0x95, 0x01, /* REPORT COUNT (1) */
        0x09, 0x21, /* USAGE (Flash) - Center */
        0x81, 0x02, /* INPUT (data, Var, Abs) */
        0x75, 0x05, /* REPORT SIZE (5) */
        0x95, 0x01, /* REPORT COUNT (1) */
        0x81, 0x03, /* INPUT (Cnst, Var, Abs) */
    0xc0        /* END COLLECTION */
} ;


/*! \brief Attempt to send payload to peer device.

Send payload to peer device 
*/
void sinkIap2RemotePayloadTransmission(const uint8 *data, iap2_link_priority device_id)
{
    accessory_info info;
    sinkAccessoryGetDeviceInfo(&info, device_id);

    if (info.device_state == sink_accessory_state_connected && info.hid_over_iap2)
    {
           Iap2ControlMessage(info.link, AccessoryHIDReport, HID_REPORT_HEADER+HID_REMOTE_REPORT_LEN, data, TRUE);
    }
}
/*! \brief Implement functionality of MFI Headphone Remote
 *   Volume Up button
 *   Volume Down button
 *   Center  button (controls answering phone, play/pause)
*/
void sinkIap2ReportUserEvent(uint16 id, iap2_link_priority device_id)
{   

    uint8 hid_center_volume_stats[HID_REPORT_HEADER+HID_REMOTE_REPORT_LEN] = {
                                  0x00, 0x06, 0x00, 0x00,         /* parameter 0 - HID component ID */
                                  0x00, HID_COMPONENT_ID,
                                  0x00, 0x05, 0x00, 0x01,         /* parameter 1 - USB vendor ID */
                                  0
                                  } ;

    uint8 center_status = (1 == sinkAccessoryGetButtonState()) ? HID_CENTER:0;

    switch (id) {
    case EventUsrMainOutVolumeUp:
        hid_center_volume_stats[HID_REPORT_HEADER]= center_status+HID_VOLUME_UP;
        sinkIap2RemotePayloadTransmission(hid_center_volume_stats, device_id);
        hid_center_volume_stats[HID_REPORT_HEADER]= center_status;
        sinkIap2RemotePayloadTransmission(hid_center_volume_stats, device_id);
        break;

    case EventUsrMainOutVolumeDown:
        hid_center_volume_stats[HID_REPORT_HEADER]= center_status+HID_VOLUME_DOWN;
        sinkIap2RemotePayloadTransmission(hid_center_volume_stats, device_id);
        hid_center_volume_stats[HID_REPORT_HEADER]= center_status;
        sinkIap2RemotePayloadTransmission(hid_center_volume_stats, device_id);
        break;

    case EventUsrSensorUp:
        sinkAccessorySetButtonState();
        hid_center_volume_stats[HID_REPORT_HEADER]= HID_CENTER;
        sinkIap2RemotePayloadTransmission(hid_center_volume_stats, device_id);
        break;

    case EventUsrSensorDown:
        sinkAccessoryResetButtonState();
        sinkIap2RemotePayloadTransmission(hid_center_volume_stats, device_id);
        break;
   default:
        return;
    }
    return;
}
/*
 * Initialize HID Headset Remote
 */
bool sinkIap2RemoteInit(iap2_link_priority device_id)
{
    uint8 *buf;
    uint16 usb_vendor_id, usb_product_id;
    accessory_info info;

    sinkAccessoryGetDeviceInfo(&info, device_id);
    buf = PanicUnlessMalloc(sizeof(hremote_hid_descriptor));
    memcpy(buf, hremote_hid_descriptor, sizeof(hremote_hid_descriptor));

    /* copy USB vendor and product information into HID descriptor from PSKeys */
    if (PsFullRetrieve(PSKEY_USB_VENDOR_ID, &usb_vendor_id, PS_SIZE_ADJ(sizeof(usb_vendor_id)))) {
        buf[HID_USB_VENDOR_OFF] = usb_vendor_id >> 8;
        buf[HID_USB_VENDOR_OFF+1] = usb_vendor_id & 0xFF;
    }
    if (PsFullRetrieve(PSKEY_USB_PRODUCT_ID, &usb_product_id, PS_SIZE_ADJ(sizeof(usb_product_id)))) {
        buf[HID_USB_PRODUCT_OFF] = usb_product_id >> 8;
        buf[HID_USB_PRODUCT_OFF+1] = usb_product_id & 0xFF;
    }

    sinkAccessoryResetButtonState();
    if(Iap2ControlMessage(info.link, StartHID, sizeof(hremote_hid_descriptor),
           buf, FALSE) != iap2_status_success)
    {
        /* The application needs to free the buffer in case of any error,
        * The appliation may want to save the content of the buffer before 
        * deallocating if it doesn't queue correctly
        */
        free(buf);
        return iap2_remote_ret_status_failure;
    }

    return iap2_remote_ret_status_success;
}

#endif

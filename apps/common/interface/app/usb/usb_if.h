/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    usb_if.h

CONTAINS
    Definitions for the USB subsystem.

DESCRIPTION
    This file is seen by the stack, and VM applications, and
    contains things that are common between them.

*/

#ifndef __APP_USB_IF_H__
#define __APP_USB_IF_H__

#include <csrtypes.h>

/** Type for indexing USB devices. */
typedef uint8 usb_device_index;

/*! @brief Type reported by trap UsbAttachedStatus(void).
    Is the USB connection currently detached or attached,
    and if attached, what is it attached to.
    The possible values that can be returned depend on whether
    USB charger detection is enabled (see PSKEY_USB_CHARGER_DETECTION_ENABLE).

    If usb charger detection is not enabled then can only report
    DETACHED or HOST_OR_HUB.
    If Resistive Detection is enabled then can report DETACHED,
    HOST_OR_HUB or DEDICATED_CHARGER. (Can not distinguish between
    HOST_OR_HUB and HOST_OR_HUB_CHARGER with Resistive Detection).
    If Voltage Source Detection is enabled then can report
    DETACHED, HOST_OR_HUB, DEDICATED_CHARGER or HOST_OR_HUB_CHARGER.

    Consult the USB battery charging specification available at www.usb.org
    for more details on the difference between a dedicated charger,
    a standard host or hub, and a host or hub charger.
*/
typedef enum
{
    DETACHED,               /*!< Device is detached from the bus. */
    HOST_OR_HUB,            /*!< Device is attached to a standard host or hub (STANDARD DOWNSTREAM PORT). */
    DEDICATED_CHARGER,      /*!< Device is attached to a dedicated charger (DEDICATED CHARGING PORT). */
    HOST_OR_HUB_CHARGER,    /*!< Device is attached to a host or hub charger (CHARGING DOWNSTREAM PORT). */
    CHARGING_PORT,          /*!< temporary state, either dedicated_charger or host_or_hub_charger. */
    NON_COMPLIANT_CHARGER,  /*!< Device is attached to a non-compliant proprietary charger. */
    FLOATING_CHARGER,       /*!< Device is attached to a proprietary charger with floating data lines. */
    NON_USB_CHARGER,        /*!< Device is attached to a non-USB charger. */
    UNKNOWN_STATUS          /*!< Device state is unknown. */
} usb_attached_status;

/** Returns TRUE if attached status indicates connection to a USB host and
 * not a wall charger or a battery pack. */
#define USB_HOST_ATTACHED(attached_status) \
    ((attached_status) == HOST_OR_HUB || \
     (attached_status) == HOST_OR_HUB_CHARGER)


/*! @brief Type reported by trap UsbDeviceState(void).
    The set of possible USB device states described by table 9.1 in the
    USB2.0 specification.
*/
typedef enum {
    usb_device_state_detached,
    usb_device_state_powered,
    usb_device_state_default,
    usb_device_state_address,
    usb_device_state_configured,
    usb_device_state_suspended
} usb_device_state;


/*! UsbInterface type.*/
typedef uint16 UsbInterface;
#define usb_interface_error ((UsbInterface)-1)

/*! UsbAltInterface type.*/
typedef uint16 UsbAltInterface;
#define usb_alt_interface_error ((UsbAltInterface)-1)

#define end_point_int_out   (0x81) /*!< Interrupt ToHost */
#define end_point_bulk_in   (0x02) /*!< Bulk FromHost */
#define end_point_bulk_out  (0x82) /*!< Bulk ToHost */
#define end_point_iso_in    (0x03) /*!< Isochronous FromHost */
#define end_point_iso_out   (0x83) /*!< Isochronous ToHost */
#define end_point_int_out2  (0x85) /*!< Interrupt ToHost */
#define end_point_bulk_in2  (0x06) /*!< Bulk FromHost */
#define end_point_bulk_out2 (0x86) /*!< Bulk ToHost */
#define end_point_iso_in2   (0x07) /*!< Isochronous FromHost */
#define end_point_int_out3  (0x89) /*!< Interrupt ToHost */
#define end_point_bulk_in3  (0x0A) /*!< Bulk FromHost */
#define end_point_bulk_out3 (0x8A) /*!< Bulk ToHost */
#define end_point_int_out4  (0x8D) /*!< Interrupt ToHost */
#define end_point_bulk_in4  (0x0E) /*!< Bulk FromHost */
#define end_point_bulk_out4 (0x8E) /*!< Bulk ToHost */

#define end_point_to_host   (0x80) /*!< Mask specifying endpoint direction ToHost */
#define end_point_from_host (0x00) /*!< Mask specifying endpoint direction FromHost */


typedef enum
{
    end_point_attr_ctl = 0,    /*!< Control.*/
    end_point_attr_iso = 1,    /*!< Isochronous.*/
    end_point_attr_bulk = 2,   /*!< Bulk.*/
    end_point_attr_int = 3,    /*!< Interrupt.*/
    end_point_attr_iso_sync = 13  /*!< Isochronous & Synchronisation Type Synchronous (bits 3:2 = 11) */
} EndPointAttr;

/*! @brief USB Type-C CC current advertisements.
    This is applicable only if USB Type-C is supported. In this case,
    Host can provide default current i.e. 500 mA and 1500 mA. These values are
    decided based on source (Host) advertisment. Refer USB Type-C
    specification for more details.
*/
typedef enum
{
    CC_CURRENT_DEFAULT, /*!< Connected to USB Type-C host with default current -
                             500mA for USB2.0 and 900mA for USB 3.1. */
    CC_CURRENT_1500, /*!< Connected to USB Type-C host with 1500mA source. */
    CC_CURRENT_3000 /*!< Connected to USB Type-C host with 3000mA source. */
} usb_type_c_advertisement;

/*! @brief This structure is used to supply the EndPoint descriptors to the
host.See section 9.6.6 of the USB2.0 specification for more details. The
"extended" field allows additional EndPoint details to be sent to the host as
required in some USB device classes (for example the Audio device class). */
typedef struct
{
    uint8 bEndpointAddress;
    EndPointAttr bmAttributes;
    uint16 wMaxPacketSize;
    uint8 bInterval;
    const uint8 *extended;      /*!< Data to be appended to
                                 * Endpoint Descriptor */
    uint16 extended_length;     /*!< Length of data to be
                                 * appended to Endpoint Descriptor */
} EndPointInfo;


/*! @brief Used in the call UsbAddInterfaceDescriptor to supply interface
descriptors to the host. See section 9.6.5 of the USB2.0 specification. Only
string descriptor index in the range 5 to 255 can be used as 1 to 4 are
reserved. */
typedef struct
{
    uint8 bInterfaceClass;     /*!< Class code.*/
    uint8 bInterfaceSubClass;  /*!< Sub-class code.*/
    uint8 bInterfaceProtocol;  /*!< Protocol code.*/
    uint8 iInterface;          /*!< String descriptor index */
} UsbCodes;


/*! @brief USB device class specific requests from the host are delivered from
the host in this format. They can be read from the VM application using
StreamUsbClassSource, StreamUsbVendorSource and SourceMapHeader.
See section 9.3 of the USB2.0 specification. */
typedef struct
{
    uint16 id;                 /*!< Identifier.*/
    uint8 bmRequestType;      /*!< Request Type.*/
    uint8 bRequest;           /*!< Request.*/
    uint16 wValue;            /*!< Value.*/
    uint16 wIndex;            /*!< Index.*/
    uint16 wLength;           /*!< Length.*/
} UsbRequest;


/*! @brief USB device class specific responses to the host are sent in this
format. They can be sent from the VM application using StreamSinkFromSource
and SinkFlushHeader. */
typedef struct
{
    UsbRequest original_request;     /*!< The original request.*/
    bool success;                    /*!< Success(TRUE) or Failure(FALSE)*/
    uint16 data_length;              /*!< Length of the data.*/
} UsbResponse;


/*! @brief Values to supply when adding an extra configuration. Any member of
ConfigDescriptorInfo can be set to the special value SAME_AS_FIRST_CONFIG,
meaning that the corresponding value for the new configuration should just
be copied over from the first configuration. */
typedef struct
{
    uint16 iConfiguration;
    uint16 bmAttributes;
    uint16 bMaxPower;
} ConfigDescriptorInfo;
#define SAME_AS_FIRST_CONFIG 0xffff  /*!< Can be used within ConfigDescriptorInfo */
#endif /* __APP_USB_IF_H__  */

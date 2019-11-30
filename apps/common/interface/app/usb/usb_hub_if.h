/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*    */

/* Definitions for USB hub trapset. */

#ifndef APP_USB_HUB_IF_H
#define APP_USB_HUB_IF_H

/** USB device configuration parameter ids. */
typedef enum usb_device_cfg_key
{
    /** idVendor in USB device descriptor, redefines USBVendorID MIB. */
    USB_DEVICE_CFG_VENDOR_ID = 0,

    /** idProduct in USB device descriptor, redefines USBProductID MIB. */
    USB_DEVICE_CFG_PRODUCT_ID = 1,

    /** bcdUSB in USB device descriptor, redefines USBVersion MIB. */
    USB_DEVICE_CFG_VERSION = 2,

    /** bDeviceClass, bDeviceSubClass and bDeviceProtocol in USB device
     * descriptor, redefines USBDeviceClassCodes MIB. */
    USB_DEVICE_CFG_DEVICE_CLASS_CODES = 3,

    /** LANGID returned in response to get string descriptor #0 request,
     * redefines USBLangID MIB. */
    USB_DEVICE_CFG_LANGID = 4,

    /** bMaxPower in USB configuration descriptor #0, redefines USBMaxPower MIB.
     * If set to "0", then bmAttributes.Self-powered is set to "1" in
     * USB configuration descriptor #0 regardless of USBSelfPowered MIB
     * in Curator.*/
    USB_DEVICE_CFG_MAX_POWER = 5,

    /** If set to "0", then bmAttributes.Remote Wake is set "1" in
     * USB configuration descriptor #0, redefines USBRemoteWakeDisable MIB. */
    USB_DEVICE_CFG_REMOTE_WAKE_DISABLE = 6,

    /** bcdDevice in USB device descriptor, redefines USBBCDDevice MIB. */
    USB_DEVICE_CFG_BCD_DEVICE = 7,

    /** iManufacturer in USB device descriptor. If set, then
     * USBManufString MIB is ignored and string descriptor with
     * the new index is expected. UsbAddStringDescriptor() trap is used
     * to add descriptor data with the same index. */
    USB_DEVICE_CFG_MANUF_STRING = 8,

    /** iProduct in USB device descriptor. If set, then
     * USBProductString MIB is ignored and string descriptor with
     * the new index is expected. UsbAddStringDescriptor() trap is used
     * to add descriptor data with the same index. */
    USB_DEVICE_CFG_PRODUCT_STRING = 9,

    /** iSerialNumber in USB device descriptor. If set, then
     * USBSerialNumberString MIB is ignored and string descriptor with
     * the new index is expected. UsbAddStringDescriptor() trap is used
     * to add descriptor data with the same index. */
    USB_DEVICE_CFG_SERIAL_NUMBER_STRING = 10,

    /** iConfiguration in USB configuration descriptor #0. If set, then
     * USBConfigString MIB is ignored and string descriptor with the new index
     * is expected. UsbAddStringDescriptor() trap is used
     * to add descriptor data with the same index. */
    USB_DEVICE_CFG_CONFIG_STRING = 11,

    USB_DEVICE_CFG_MAX = 12
} usb_device_cfg_key;

/** Special value for config key meaning take it from the MIB key.
 *
 * This value can be used with UsbHubConfigKey() trap to unset a parameter
 * that was previously set, e.g:
 *
 *     UsbHubConfigKey(USB_DEVICE_CFG_MANUF_STRING, USB_DEVICE_CFG_FROM_MIB);
 *
 * When unset, parameter value comes from the MIB keys. */
#define USB_DEVICE_CFG_FROM_MIB (~0UL)

/**
 * USB device configuration parameters.
 *
 * Any parameter with corresponding bit in the "ValidParams" field set to "0"
 * is ignored. */
typedef struct usb_device_parameters
{
    /** idVendor field in Standard Device Descriptor. */
    uint16 VendorID;
    /** idProduct field in Standard Device Descriptor. */
    uint16 ProductID;
    /** bcdUSB field in Standard Device Descriptor. */
    uint16 Version;
    /** bDeviceClass, bDeviceSubClass and bDeviceProtocol fields in
     * Standard Device Descriptor. */
    uint8 DeviceClassCodes[3];
    /** Supported LANGID for string descriptors */
    uint16 LangID;
    /** bMaxPower in Standard Configuration Descriptor */
    uint16 MaxPower;
    /** Affects bmAttributes.Remote Wakeup in Standard Configuration Descriptor */
    uint16 RemoteWakeDisable;
    /** bcdDevice field in Standard Device Descriptor. */
    uint16 BCDDevice;

    /** Any parameter with corresponding bit set to "0" is ignored.
     * "usb_device_cfg_key" gives bit position for every parameter. */
    uint32 ValidParams;
} usb_device_parameters;

#endif /* APP_USB_HUB_IF_H */

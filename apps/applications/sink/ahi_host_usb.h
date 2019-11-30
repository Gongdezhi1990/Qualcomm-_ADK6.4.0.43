/****************************************************************************
Copyright (c) 2016-2017 Qualcomm Technologies International, Ltd.


FILE NAME
    ahi_host_usb.h

DESCRIPTION
    Implementation of an AHI transport using the USB HID device class transport.

*/
/*!
@file   ahi_host_usb.h
@brief  Implementation of an AHI transport using the USB HID device class transport.

        This is a VM application module that acts as the 'glue' between
        The USB HID device class transport and the AHI transport API.
*/
#ifndef AHI_USB_HOST_HID_H_
#define AHI_USB_HOST_HID_H_


#if defined ENABLE_USB && defined ENABLE_AHI_USB_HOST
void AhiUSBHostInit(void);
#else
#define AhiUSBHostInit() ((void)0)
#endif /* ENABLE_AHI_USB_HOST */

#ifdef ENABLE_AHI_USB_HOST
void AhiUsbHostHandleMessage(MessageId id, Message message);
#else
#define AhiUsbHostHandleMessage(id, message) ((void)0)
#endif /* ENABLE_AHI_USB_HOST */

#endif /* AHI_USB_HOST_HID_H_ */

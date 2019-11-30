/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    ahi_host_usb.c

DESCRIPTION
    Implementation of an AHI transport using the USB HID device class transport.

*/
/*!
@file   ahi_host_usb.c
@brief  Implementation of an AHI transport using the USB HID device class transport.

        This is a VM application module that acts as the 'glue' between
        The USB HID device class transport and the AHI transport API.
*/

#include "sink_debug.h"
#include <message.h>
#include <panic.h>
#include <print.h>
#include <sink.h>
#include <source.h>
#include <stdlib.h>
#include <stream.h>
#include <string.h>

#include "ahi.h"
#include "byte_utils.h"
#include "usb_device_class.h"
#include "sink_usb.h"
#include "ahi_host_usb.h"


#ifdef DEBUG_AHI
    #define AHI_DEBUG(x) DEBUG(x)
#else
    #define AHI_DEBUG(x)
#endif

/* if we don't have USB enabled, force disable AHI_USB_HOST */
#ifndef ENABLE_USB
#ifdef ENABLE_AHI_USB_HOST
#undef ENABLE_AHI_USB_HOST
#endif
#endif

#ifdef ENABLE_AHI_USB_HOST


/* AHI transport task for data going to Host */
static void ahiTransportHandleMessage(Task task, MessageId id, Message message);
static const TaskData ahiTask = { ahiTransportHandleMessage };

static void handleTransportSendData(Task task, Message message);

/* Message delay (ms) when retrying a data send. */
#define AHI_USB_RETRY_DELAY 5

/* Report ID for the AHI response. */
#define HID_REPORTID_RESPONSE 2

/******************************************************************************
    Private functions 
*/

/******************************************************************************
NAME
    handleTransportSendData

DESCRIPTION
    Send data contained in a AHI_TRANSPORT_SEND_DATA message to the host.

RETURNS
    n/a
*/
static void handleTransportSendData(Task task, Message message)
{
#ifdef HYDRACORE    
    UNUSED(task);
#endif    
    if(!sinkUsbIsEnumerated())
    {
        AHI_DEBUG(("AHI: USB disconnected\n"));
        AhiTransportSendDataAck();
        return;
    }
        {
        AHI_TRANSPORT_SEND_DATA_T *msg = (AHI_TRANSPORT_SEND_DATA_T *)message;
        uint8 *data;
        AHI_DEBUG(("AHI: handleTransportSendData payload 0x%p size %u\n", msg->payload, msg->size));

        /* Un-pack the bytes in the payload before passing them to
           usb_HidDatalinkSendDataRequest. */
        data = PanicUnlessMalloc(msg->size);
        ByteUtilsMemCpyToStream(data, msg->payload, msg->size);

        if (!usb_HidDatalinkSendDataRequest(data, msg->size, HID_REPORTID_RESPONSE) != usb_device_class_status_success)
        {
            AhiTransportSendDataAck();
        }
        else
        {
            AHI_TRANSPORT_SEND_DATA_T *repost;
            AHI_DEBUG(("AHI:   Failed to send %u bytes - retrying", msg->size));
            /* Retry by sending the same message back to ourselves. */
            repost = PanicUnlessMalloc(sizeof(AHI_TRANSPORT_SEND_DATA_T) + msg->size);
            repost->size = msg->size;
            memmove(repost->payload, msg->payload, msg->size);
            MessageSendLater(task, AHI_TRANSPORT_SEND_DATA, repost, AHI_USB_RETRY_DELAY);
        }
        free(data);
        }
}

/******************************************************************************
NAME
    ahiTransportHandleMessage

DESCRIPTION
    Message handler for messages from the AHI library.

    Usually the message will be a request to send data to the Host.

RETURNS
    n/a
*/
static void ahiTransportHandleMessage(Task task, MessageId id, Message message)
{
    AHI_DEBUG(("AHI: ahiTransportHandleMessage id 0x%x\n", id));

    switch (id)
    {
    case AHI_TRANSPORT_SEND_DATA:
        handleTransportSendData(task, message);
        break;
    default:
        AHI_DEBUG(("AHI: Unexpected ahi msg id 0x%x\n", id));
        break;
    }
}

/******************************************************************************
NAME
    AhiUsbHostHandleMessage

DESCRIPTION
    Message handler for messages from the Host.

RETURNS
    n/a
*/
void AhiUsbHostHandleMessage(MessageId id, Message message)
{
    USB_DEVICE_CLASS_MSG_REPORT_IND_T* msg = (USB_DEVICE_CLASS_MSG_REPORT_IND_T *) message;

    AHI_DEBUG(("AHI: AhiUsbHostHandleMessage id 0x%x\n", id));

    if(id == USB_DEVICE_CLASS_MSG_REPORT_IND)
    {
        AhiTransportProcessData((TaskData*)&ahiTask, (uint8*)msg->report);
    }

}

/******************************************************************************
    Public functions 
*/

/******************************************************************************
NAME
    AhiUSBHostInit

DESCRIPTION
    Initialise the USB Host AHI transport.

RETURNS
    n/a
*/
void AhiUSBHostInit(void)
{
    AHI_DEBUG(("AHI: AhiUSBHostInit\n"));
}

#endif /* ENABLE_AHI_USB_HOST */

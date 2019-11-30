/*******************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.
 

FILE NAME
    hid_upgrade.c

DESCRIPTION
    Implementation for the USB HID Upgrade Transport.
*******************************************************************************/

#ifndef USB_DEVICE_CLASS_REMOVE_HID

#include <upgrade.h>
#include <usb_device_class.h>
#include <panic.h>

#include "hid_upgrade.h"

TaskData hid_upgrade_transport_task;

#define HID_REPORTID_UPGRADE_DATA_TRANSFER (5)
#define HID_REPORTID_UPGRADE_RESPONSE      (6)
#define HID_REPORTID_COMMAND               (3)

/* Only connect and disconnect commands are supported. */
#define HID_CMD_CONNECTION_REQ   (0x02)
#define HID_CMD_DISCONNECT_REQ   (0x07)

/* Upgrade Library Message Handling. */
static void HidUpgradeConnectCfmHandler(UPGRADE_TRANSPORT_CONNECT_CFM_T *message);
static void HidUpgradeDataIndHandler(UPGRADE_TRANSPORT_DATA_IND_T *message);

static void HidUpgradeMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    switch(id)
    {
    /* Response from call to UpgradeTransportConnectRequest() */
    case UPGRADE_TRANSPORT_CONNECT_CFM:
        HidUpgradeConnectCfmHandler((UPGRADE_TRANSPORT_CONNECT_CFM_T *)message);
        break;

    /* Request from upgrade library to send a data packet to the host */
    case UPGRADE_TRANSPORT_DATA_IND: /* aka UPGRADE_PROCESS_DATA_IND */
        HidUpgradeDataIndHandler((UPGRADE_TRANSPORT_DATA_IND_T *)message);
        break;

    default:
        /* Unhandled */
        break;
    }
}

static void HidUpgradeConnectCfmHandler(UPGRADE_TRANSPORT_CONNECT_CFM_T *message)
{
    usb_HidDatalinkSendDataRequest((uint8*)&message->status,
                                   sizeof(message->status),
                                   HID_REPORTID_UPGRADE_RESPONSE);
}

static void HidUpgradeDataIndHandler(UPGRADE_TRANSPORT_DATA_IND_T *message)
{
    usb_HidDatalinkSendDataRequest(message->data,
                                   message->size_data,
                                   HID_REPORTID_UPGRADE_RESPONSE);
}

/* SET_REPORT handling functions */
static void HidUpgradeCommandHandler(uint16 data_in_size, uint8 *data_in);
static void HidUpgradeDataRequestHandler(uint16 data_in_size, uint8 *data_in);

void HidUpgradeSetReportHandler(uint16 report_id, uint16 data_in_size, uint8 *data_in)
{
    PanicNull(data_in);
    PanicZero(data_in_size);

    switch(report_id)
    {
    case HID_REPORTID_COMMAND:
        HidUpgradeCommandHandler(data_in_size, data_in);
        break;
    case HID_REPORTID_UPGRADE_DATA_TRANSFER:
        HidUpgradeDataRequestHandler(data_in_size, data_in);
        break;
    default:
        /* Unhandled. */
        break;
    }
}

static void HidUpgradeConnect(void);
static void HidUpgradeDisconnect(void);

static void HidUpgradeCommandHandler(uint16 data_in_size, uint8 *data_in)
{
    UNUSED(data_in_size);

    switch(data_in[0])
    {
    case HID_CMD_CONNECTION_REQ:
        HidUpgradeConnect();
        break;
    case HID_CMD_DISCONNECT_REQ:
        HidUpgradeDisconnect();
        break;
    default:
        /* Unhandled. */
        break;
    }
}

static void HidUpgradeConnect(void)
{
    hid_upgrade_transport_task.handler = HidUpgradeMessageHandler;
    /* Connect transport task and don't request UPGRADE_TRANSPORT_DATA_CFM
     * messages, but request several bocks */
    UpgradeTransportConnectRequest(&hid_upgrade_transport_task, FALSE, TRUE);
}

static void HidUpgradeDisconnect(void)
{
    UpgradeTransportDisconnectRequest();
}

static void HidUpgradeDataRequestHandler(uint16 data_in_size, uint8 *data_in)
{
    UpgradeProcessDataRequest(data_in_size, data_in);
}

#endif /* !USB_DEVICE_CLASS_REMOVE_HID */

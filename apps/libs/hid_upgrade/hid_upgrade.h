/*******************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.
 

FILE NAME
    hid_upgrade.h

DESCRIPTION
    Interface for the USB HID Upgrade Transport.
*******************************************************************************/

#ifndef USB_DEVICE_CLASS_REMOVE_HID

#ifndef HID_UPGRADE_H_
#define HID_UPGRADE_H_

#include <csrtypes.h>

/**
 * @file hid_upgrade.h
 * @brief Header file for HID USB Upgrade.
 *
 * @page hid_upgrade USB HID report handling for the upgrade transport.
 *
 * @section Incoming (from host) reports
 * @subsection Set Reports
 *
 * The following set reports are handled by this transport:
 *  - HID_REPORTID_UPGRADE_DATA_TRANSFER (5)
 *  - HID_REPORTID_COMMAND               (3)
 *
 * The HID_REPORTID_CONTROL (4) report is not handled as the functionality it
 * provides is not required or is duplicated by HID_REPORTID_COMMAND.
 *
 * HID_REPORTID_DATA_TRANSFER
 * The data and data size provided in this report is passed directly to
 * UpgradeProcessDataRequest.
 *
 * HID_REPORTID_COMMAND
 * The associated data contains a 2 octet 'command'. The supported values for
 * this command are:
 *  - HID_CMD_CONNECTION_REQ (2) - Calls UpgradeTransportConnectRequest
 *  - HID_CMD_DISCONNECT_REQ (7) - Calls UpgradeTransportDisconnectRequest
 *
 * Other commands are not supported.
 *
 * @subsection Get Reports
 *
 * No get reports are handled by this transport.
 *
 * @section Outgoing (to host) messages
 *
 * After connection, data may be sent to the host from this transport via calls
 * to usb_HidDatalinkSendDataRequest(). These will have the report ID
 * HID_REPORTID_UPGRADE_RESPONSE (6). The data size is passed along with the
 * report and will be a maximum length of USB_RESPONSE_MAX_SIZE (64) octets.
 *
 * UPGRADE_TRANSPORT_DATA_IND will send a response report containing a data
 * packet.
 *
 * UPGRADE_TRANSPORT_DATA_CFM will send a response report with 0 size and
 * NULL data.
 *
 * UPGRADE_TRANSPORT_CONNECT_CFM will send a response report containing the
 * upgrade_status_t status.
 *
 * UPGRADE_TRANSPORT_DISCONNECT_CFM does not send any response.
 */

/**
 * @brief Handles a USB HID Set Report.
 *
 * Generally called by an application's message handling task in response to
 * the USB_DEVICE_CLASS_MSG_REPORT_IND message for the
 * USB_DEVICE_CLASS_TYPE_HID_DATALINK_CONTROL class.
 *
 * @param [in] The message's report id.
 * @param [in] The message's report size.
 * @param [in] A pointer to the message's report data.
 */
void HidUpgradeSetReportHandler(uint16 report_id,
                                uint16 data_in_size,
                                uint8 *data_in);

#endif /* HID_UPGRADE_H_ */

#endif /* !USB_DEVICE_CLASS_REMOVE_HID */

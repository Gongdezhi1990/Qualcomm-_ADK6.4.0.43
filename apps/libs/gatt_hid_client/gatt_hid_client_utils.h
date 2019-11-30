/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_HID_CLIENT_UTILS_H_
#define GATT_HID_CLIENT_UTILS_H_

/* Included for Mock generation */
#include "gatt_hid_client_private.h"

/***************************************************************************
NAME
    hid_client_convert_status

DESCRIPTION
   Utility function to convert gatt status to HID client Lib status code
*/
gatt_hid_client_status hid_client_convert_status(gatt_status_t gatt_status);

/***************************************************************************
NAME
    hid_client_set_notifications_for_report_id

DESCRIPTION
   Set notifications for the report_id. 

NOTE
   If notifications are not enabled for the report_id, the HID Client library
   will ignore them and not pass them to the application so this API must
   be called when the application requests notifications for a particular
   report_id.
*/
void hid_client_set_notifications_for_report_id(GHIDC_T *const hid_client, uint16 report_id, bool enable);

/***************************************************************************
NAME
    hid_client_get_notifications_for_report_id

DESCRIPTION
   Utility function to find out whether notifications have been enabled for the
   report_id

RETURNS
    TRUE if notifications are enabled for report_id
    FALSE if notifications are disabled or report_id wasn't found
*/
bool hid_client_get_notifications_for_report_id(GHIDC_T *const hid_client, uint16 report_id);

/***************************************************************************
NAME
    hid_client_get_report_handle_and_type_from_id

DESCRIPTION
   Utility function to get report handle and report type from report ID.
   return TRUE for success and FALSE for failure
*/
bool  hid_client_get_report_handle_and_type_from_id(GHIDC_T *const hid_client,
          uint16 report_id,
          uint16 *report_handle,
          uint16 *type);

/***************************************************************************
NAME
    hid_client_get_report_id_from_handle

DESCRIPTION
   Utility function to get report id fom report handle.
   return TRUE for success and FALSE for failure
*/
bool hid_client_get_report_id_from_handle(GHIDC_T *const hid_client,
     uint16 report_handle,
     uint16 *report_id);

/***************************************************************************
NAME
    hid_client_validate_notification_handle

DESCRIPTION
   Utility function to validate notification handle , if the notifitcaion handle is valid
   hid handle then it returns a valid report_it
*/
bool hid_client_validate_notification_handle(GHIDC_T *const hid_client,
     uint16 handle,
     uint16 *report_id);

#endif /* GATT_HID_CLIENT_UTILS_H_ */



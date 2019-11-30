/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_HID_CLIENT_WRITE_H_
#define GATT_HID_CLIENT_WRITE_H_

/* Included for Mock generation */
#include "gatt_hid_client_private.h"

/***************************************************************************
NAME
    hid_client_write_set_protocol

DESCRIPTION
    Initiate setting protocol
*/
void hid_client_write_set_protocol(GHIDC_T *const hid_client,
            const HID_CLIENT_INTERNAL_SET_PROTOCOL_T* msg);

/***************************************************************************
NAME
    hid_client_write_set_control_point

DESCRIPTION
    Initiate setting control point
*/
void hid_client_write_set_control_point(GHIDC_T *const hid_client,
            const HID_CLIENT_INTERNAL_SET_CTRL_POINT_T* ctrl);

/***************************************************************************
NAME
    hid_client_write_set_report_request

DESCRIPTION
    Initiate set report
*/
void hid_client_write_set_report_request(GHIDC_T *const hid_client,
            const HID_CLIENT_INTERNAL_MSG_SET_REPORT_T* msg);

/***************************************************************************
NAME
    hid_client_write_boot_report_request

DESCRIPTION
    Initiate write to a boot report
*/
void hid_client_write_boot_report_request(GHIDC_T *const hid_client,
            const HID_CLIENT_INTERNAL_MSG_WRITE_BOOT_REPORT_T* msg);


/***************************************************************************
NAME
    handle_hid_client_write_without_response_cfm

DESCRIPTION
    Handle all write without response confirmation mesages
*/
void handle_hid_client_write_without_response_cfm(GHIDC_T *const hid_client,
                const GATT_MANAGER_WRITE_WITHOUT_RESPONSE_CFM_T* write_cfm);

/***************************************************************************
NAME
    handle_hid_client_write_char_value_cfm

DESCRIPTION
    Handle all write characteristic value confirmation mesages
*/
void handle_hid_client_write_char_value_cfm(GHIDC_T *const  hid_client,
                const GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T*write_val_cfm);


#endif /* GATT_HID_CLIENT_WRITE_H_ */


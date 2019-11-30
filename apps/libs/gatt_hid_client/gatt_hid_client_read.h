/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_HID_CLIENT_READ_H_
#define GATT_HID_CLIENT_READ_H_

/* Included for Mock generation */
#include "gatt_hid_client_private.h"

/***************************************************************************
NAME
    hid_client_read_get_protocol

DESCRIPTION
    Initiate a read for Get Protocol handle 
*/
void hid_client_read_get_protocol(GHIDC_T *const hid_client);

/***************************************************************************
NAME
    hid_client_read_hidinfo

DESCRIPTION
    Initiate a read for Hid Informtaion  
*/
void hid_client_read_hidinfo(GHIDC_T *const hid_client);

/***************************************************************************
NAME
    hid_client_read_external_reference

DESCRIPTION
    Initiate a read for external reference  
*/
void  hid_client_read_external_reference(GHIDC_T *const hid_client);

/***************************************************************************
NAME
    hid_client_read_ccd_handle

DESCRIPTION
    Initiate a read for requested CCD handle
*/
void hid_client_read_ccd_handle(GHIDC_T *const hid_client, 
        const HID_CLIENT_INTERNAL_MSG_READ_CCD_T* msg);

/***************************************************************************
NAME
    hid_client_read_report_id_map

DESCRIPTION
    Interface for reading report ID map  
*/
 void hid_client_read_report_id_map(GHIDC_T *const hid_client);

/***************************************************************************
NAME
    hid_client_read_get_report

DESCRIPTION
    Initiate Get report
*/
void  hid_client_read_get_report(GHIDC_T *const hid_client, 
        const HID_CLIENT_INTERNAL_MSG_GET_REPORT_T* msg);

/***************************************************************************
NAME
    hid_client_read_get_report

DESCRIPTION
    Initiate report map read
*/
void hid_client_read_report_map(GHIDC_T *const hid_client);

/***************************************************************************
NAME
    hid_client_read_boot_report

DESCRIPTION
    Initiate read boot report
*/
 void hid_client_read_boot_report(GHIDC_T *const hid_client,
                    const HID_CLIENT_INTERNAL_MSG_READ_BOOT_REPORT_T * msg);
 
/***************************************************************************
NAME
    handle_hid_client_read_char_value_cfm

DESCRIPTION
    Handle all read responses for char values  
*/
void handle_hid_client_read_char_value_cfm(GHIDC_T *const hid_client, 
                    const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T* read_cfm);


/***************************************************************************
NAME
    handle_hid_client_read_long_char_value_cfm

DESCRIPTION
    Handle all read responses for long char values  
*/
void  handle_hid_client_read_long_char_value_cfm(GHIDC_T *const hid_client,
                    const GATT_MANAGER_READ_LONG_CHARACTERISTIC_VALUE_CFM_T* read_lng_cfm);

/***************************************************************************
NAME
    handle_hid_client_read_using_uuid_cfm

DESCRIPTION
    Handle all read responses for UUID read   
*/
void handle_hid_client_read_using_uuid_cfm(GHIDC_T *const hid_client,
                    const GATT_MANAGER_READ_USING_CHARACTERISTIC_UUID_CFM_T* read_uuid_cfm);

#endif /* GATT_HID_CLIENT_READ_H_ */


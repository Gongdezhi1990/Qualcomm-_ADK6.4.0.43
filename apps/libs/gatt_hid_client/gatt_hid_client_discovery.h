/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_HID_CLIENT_DISCOVERY_H
#define GATT_HID_CLIENT_DISCOVERY_H

/* Included for Mock generation */
#include "gatt_hid_client_private.h"

/***************************************************************************
NAME
    hid_client_init_discovery_instance

DESCRIPTION
    Interface to update the discovery instance handles before initiating Discovery .
*/
bool  hid_client_init_discovery_instance(const GATT_HID_CLIENT_CONFIG_PARAMS_T *const cfg,
      uint16 end_handle);

/***************************************************************************
NAME
    hid_client_start_discovery

DESCRIPTION
    Starts the discovery of all handles at HID service Side .
*/
void  hid_client_start_discovery(GHIDC_T *const hid_client);


/***************************************************************************
NAME
    hid_client_complete_discovery

DESCRIPTION
    handle discovery complete internal message .
*/
void hid_client_complete_discovery(GHIDC_T *const hid_client, uint16 cid);


/***************************************************************************
NAME
    hid_client_discovery_in_progress

DESCRIPTION
    Returns the curent status of hid discovery  .
*/
bool hid_client_discovery_in_progress(void);

/***************************************************************************
NAME
    hid_client_cleanup_discovery_instance

DESCRIPTION
    Interface to clean up the discovery instance 
*/
void hid_client_cleanup_discovery_instance(void);

/***************************************************************************
NAME
    handle_hid_client_discover_all_char_cfm

DESCRIPTION
    Handle characteristic handle discovery 
*/
void handle_hid_client_discover_all_char_cfm(GHIDC_T *const hid_client,
       const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T *const cfm);

/***************************************************************************
NAME
    handle_hid_client_discover_all_char_descriptors_cfm

DESCRIPTION
    Handle characteristic descriptor discovery 
*/
void handle_hid_client_discover_all_char_descriptors_cfm(GHIDC_T *const hid_client,
       const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *const cfm);


/***************************************************************************
NAME
    handle_hid_client_discover_read_char_value_cfm

DESCRIPTION
    Handle read characteristic value 
*/
void handle_hid_client_discover_read_char_value_cfm(GHIDC_T *const hid_client,
             const GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T *const cfm );

#endif /* GATT_HID_CLIENT_DISCOVERY_H */


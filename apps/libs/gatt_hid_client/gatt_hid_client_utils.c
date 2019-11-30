/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>
#include <stdio.h>

#include "gatt_hid_client_private.h"

#include "gatt_hid_client_utils.h"


/***************************************************************************/
gatt_hid_client_status hid_client_convert_status(gatt_status_t gatt_status)
{
    gatt_hid_client_status hid_client_status;
    /* Convert GATT Lib status to HID lib status */
    switch(gatt_status)
    {   
        case gatt_status_success:
        {
            hid_client_status = gatt_hid_client_status_success;
        }
        break;
        
        case gatt_status_insufficient_authentication:
        {
            hid_client_status = gatt_hid_client_status_insufficient_authentication;
        }
        break;

        case gatt_status_invalid_cid:
        {
            hid_client_status = gatt_hid_client_status_no_connection;
        }
        break;

        default:
        {
            /* All other cases, return status failed */
            hid_client_status = gatt_hid_client_status_failed;
        }
        break;
    }
    return hid_client_status;
}

/***************************************************************************/
void hid_client_set_notifications_for_report_id(GHIDC_T *const hid_client, uint16 report_id, bool enable)
{
    uint16 count = 0;
    /* Get Report handle form Report ID */
    for(count = 0;count < hid_client->num_report_id;count++)
    {
        /* Find the report ID map instance */
        if(hid_client->report_id_map[count].report_id == report_id)
        {
            hid_client->report_id_map[count].notify = enable;
            return;
        }
    }
}

/***************************************************************************/
bool hid_client_get_notifications_for_report_id(GHIDC_T *const hid_client, uint16 report_id)
{
    uint16 count = 0;
    /* Get Report handle form Report ID */
    for(count = 0;count < hid_client->num_report_id;count++)
    {
        /* Find the report ID map instance */
        if(hid_client->report_id_map[count].report_id == report_id)
        {
            return hid_client->report_id_map[count].notify;
        }
    }
    return FALSE;
}

/***************************************************************************/
bool  hid_client_get_report_handle_and_type_from_id(GHIDC_T *const hid_client,
                                                    uint16 report_id,
                                                    uint16 *report_handle,
                                                    uint16 *type)
{
    gatt_hid_client_report_id_map_t *report_id_map = NULL;
    uint16 count = 0;

    /* Reset return param*/
    *report_handle = 0;
    *type = 0;

    /* Get Report handle form Report ID */
    for(count = 0;count < hid_client->num_report_id;count++)
    {
        /* Find out the report ID map instance */
        report_id_map = &hid_client->report_id_map[count];
        if(report_id_map->report_id == report_id)
        {
            /* report informtaion has been found, fill in data and break */
            *report_handle = report_id_map->report_handle;
            *type = report_id_map->type;
            return TRUE;
        }
    }
    return FALSE;
}

/***************************************************************************/
bool hid_client_get_report_id_from_handle(GHIDC_T *const hid_client,
                                          uint16 report_handle,
                                          uint16 *report_id)
{
    uint16 count = 0;
    gatt_hid_client_report_id_map_t *report_id_map = NULL;

    /* Reset return param*/
    *report_id = 0;

    /* Go through all available report ID maps */
    for(count = 0;count< hid_client->num_report_id;count++)
    {
        /* Find out the report ID map instance */
        report_id_map = &hid_client->report_id_map[count];
        GATT_HID_CLIENT_DEBUG_INFO(("GHIDC:Func:hid_client_get_report_id_from_handle(),reportId= %x\n",
                                                        report_id_map->report_id));
        if(report_id_map->report_handle == report_handle)
        {
             /* report informtaion has been found, fill in data and break */
            *report_id  = report_id_map->report_id;
             return TRUE;
        }
    }
    return FALSE;
}

/***************************************************************************/
bool hid_client_validate_notification_handle(GHIDC_T *const hid_client,
                                             uint16 handle,
                                             uint16 *report_id)
{
    uint16 count = 0;
    gatt_hid_client_report_id_map_t *report_id_map = NULL;

    /* Reset return param*/
    *report_id = 0;

    /* Go through all available report ID maps */
    for(count = 0;count< hid_client->num_report_id;count++)
    {
        /* Find out the report ID map instance */
        report_id_map = &hid_client->report_id_map[count];
        if((report_id_map->type == gatt_hid_input_report) && (report_id_map->report_handle == handle))
        {
            /* report informtaion has been found, fill in data and break */
            *report_id  = report_id_map->report_id;
            return TRUE;
        }
    }
    return FALSE;
}


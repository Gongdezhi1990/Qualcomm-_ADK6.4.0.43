/* Copyright (c) 2015 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_HID_PRIVATE_H
#define GATT_HID_PRIVATE_H

#include <csrtypes.h>
#include <stdlib.h>
#include <message.h>
#include <panic.h>

#include <gatt.h>
#include <gatt_manager.h>

#include "gatt_hid_client.h"
#include "gatt_hid_client_debug.h"
#include "gatt_client.h"

/* GATT Invalid Handle */
#define GATT_HID_INVALID_HANDLE      (0)
/* Invalid HID handle */
#define INVALID_HID_HANDLE (0x0)
/* Invalid UUID */
#define GATT_HID_INVALID_UUID (0x0000)

/* Macros for creating messages */
#define MAKE_HID_CLIENT_MESSAGE(TYPE) TYPE##_T *message = (TYPE##_T *) PanicNull(calloc(1,sizeof(TYPE##_T)))
#define MAKE_HID_CLIENT_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicNull(calloc(1,sizeof(TYPE##_T) + ((LEN) - 1) * sizeof(uint8)))

/*Check Discovery is in progress*/
#define DISCOVERY_IN_PROGRESS(hid_client) (hid_client->discovery_in_progress)
/* Verify the handles are already found out and exists */
#define IS_HANDLE_AVAILABLE(handle)     (handle != INVALID_HID_HANDLE)
/*Set the pending FLAG so that only one request has been processed at a TIME */
#define SET_PENDING_FLAG(type,pending_request) (pending_request = type)
/* Clear pending FLAG as new requests can be processed */
#define CLEAR_PENDING_FLAG(pending_request) (pending_request = hid_client_pending_none)
/* Check GATT status is success */
#define CHECK_GATT_SUCCESS(status)     ((status == gatt_status_success) || (status == gatt_status_success_more))
/* Check Init Input prams are valid */
#define INPUT_PARAM_NULL(app_task,hid_client,init_params) (app_task == NULL) || (hid_client == NULL ) ||(init_params == NULL)
/* Check config parama is valid */
#define CONFIG_PARAM_INVALID(cfg) ((cfg!= NULL) && (cfg->max_num_report == 0 ||cfg->max_num_char_handles == 0 || cfg->max_num_reportmode_ccd == 0))
/* Check the descriptor mandatory or not */
#define CHECK_DESCRIPTOR_NOT_MANDATORY(uuid) (uuid == GATT_HID_EXT_REPORT_REFERENCE_UUID) 

/* HID service UUID's as defined in blluetooth sig */
#define GATT_HID_SERVICE_UUID                       (0x1812)
#define GATT_HID_INFORMATION_UUID                   (0x2A4A)
#define GATT_HID_REPORT_MAP_UUID                    (0x2A4B)
#define GATT_HID_CONTROL_POINT_UUID                 (0x2A4C)
#define GATT_HID_REPORT_UUID                        (0x2A4D)
#define GATT_HID_PROTOCOL_MODE_UUID                 (0x2A4E)
#define GATT_HID_BOOT_KB_INPUT_UUID                 (0x2A22)
#define GATT_HID_BOOT_MOUSE_INPUT_UUID              (0x2A33)
#define GATT_HID_BOOT_OUTPUT_UUID                   (0x2A32)
#define GATT_HID_REPORT_REFERENCE_UUID              (0x2908)
#define GATT_HID_EXT_REPORT_REFERENCE_UUID          (0x2907)
#define GATT_HID_CHARACTERISTIC_CONFIGURATION_UUID  (0x2902)


/* Macro to fill in high byte and low byte */
#define GATT_HID_CLIENT_CONVERT_UINT16_TO_UINT8(offset, input, output) \
        {\
            output[offset++] = (uint8) (input >> 8);\
            output[offset++] = (uint8) (input & 0x00ff);\
        }


/* Enumerations for HID client Internal Messages */
typedef enum
{
    HID_CLIENT_INTERNAL_MSG_BASE = 0,
    HID_CLIENT_INTERNAL_MSG_DISCOVER, /* Internal Message for HID client connect request */
    HID_CLIENT_INTERNAL_MSG_DISCOVER_COMPLETE, /*Internal Message for HID client connect discovery Completed*/
    HID_CLIENT_INTERNAL_MSG_READ_INFO, /* Internal message for reading HID Information */
    HID_CLIENT_INTERNAL_MSG_GET_PROTOCOL, /* Internal message for reading HID Protocol */
    HID_CLIENT_INTERNAL_MSG_READ_EXT_REFERENCE,/* Internal message for reading HID external reference */
    HID_CLIENT_INTERNAL_MSG_GET_REPORT_ID_MAP, /* Internal message for reading HID report ID map */
    HID_CLIENT_INTERNAL_MSG_GET_REPORT, /* Internal message for reading HID Report */
    HID_CLIENT_INTERNAL_MSG_READ_REPORT_MAP, /* Internal message for reading HID report map */
    HID_CLIENT_INTERNAL_SET_PROTOCOL, /* Internal message for setting HID protocol mode */
    HID_CLIENT_INTERNAL_SET_CTRL_POINT, /* Internal message for setting HID control point */
    HID_CLIENT_INTERNAL_HANDLE_NOTIFICATION_REQ, /* Internal message for setting HID notification */
    HID_CLIENT_INTERNAL_SET_NOTIFICATION_CCDHANDLE_REQ,/* Internal Message for setting HID notification for a ccd handle request*/
    HID_CLIENT_INTERNAL_MSG_SET_REPORT, /* Internal message for setting HID report */
    HID_CLIENT_INTERNAL_MSG_WRITE_BOOT_REPORT, /* Internal message for writing to  HID boot report handles */
    HID_CLIENT_INTERNAL_MSG_READ_BOOT_REPORT,/* Internal message for reading  HID boot report handles */
    HID_CLIENT_INTERNAL_MSG_READ_CCD, /* Internal message for reading  ccd handles */
    HID_CLIENT_INTERNAL_MSG_TOP
}hid_client_internal_msg_type;

/* Enumerations for HID client message which is pending and yet to process completely  */
typedef enum
{
    hid_client_pending_none = 0,
    hid_client_read_pending_protocol,
    hid_client_read_pending_info,
    hid_client_read_pending_ext_reference,
    hid_client_read_pending_get_report,
    hid_client_read_pending_read_report_map,
    hid_client_read_pending_boot_report,
    hid_client_write_pending_set_protocol,
    hid_client_write_pending_set_control,
    hid_client_write_pending_boot_notification,
    hid_client_write_pending_report_notification,
    hid_client_write_pending_ccdhandle_notification,
    hid_client_write_pending_set_report,
    hid_client_write_pending_boot_report,
    hid_client_read_pending_ccdhandle
}hid_client_pending_read_type;

/* Internal Message Structure to start HID service characteristic handle discovery */
typedef struct 
{
    uint16 cid;
    uint16 start_handle;
    uint16 end_handle;
}HID_CLIENT_INTERNAL_MSG_DISCOVER_T;

/* Internal Message Structure to indicate HID service discovery complete */
typedef struct 
{
    uint16 cid;
}HID_CLIENT_INTERNAL_MSG_DISCOVER_COMPLETE_T;

/* Internal Message Structure to Initiate get report request */
typedef struct
{
    uint16 report_handle;
}HID_CLIENT_INTERNAL_MSG_GET_REPORT_T;

/* Internal Message Structure to Initiate registering notification */
typedef struct
{
    uint16  mode;
    bool    enable;
}HID_CLIENT_INTERNAL_HANDLE_NOTIFICATION_REQ_T;

/* Internal Message Structure to Initiate registering notification for a handle*/
typedef struct
{
    uint16  handle;
    bool    enable;
}HID_CLIENT_INTERNAL_SET_NOTIFICATION_CCDHANDLE_REQ_T;

/* Internal Message Structure to Initiate set control point */
typedef struct
{
    uint8 ctrl_type;
}HID_CLIENT_INTERNAL_SET_CTRL_POINT_T;

/* Internal Message Structure to set protocol */
typedef struct
{
    uint8 protocol_type;
}HID_CLIENT_INTERNAL_SET_PROTOCOL_T;

/* Internal Message Structure to Initiate set report */
typedef struct
{
    uint16 handle;
    uint16 size_data;
    uint16 type;
    uint8 data[1];
}HID_CLIENT_INTERNAL_MSG_SET_REPORT_T;

/* Internal Message Structure to Initiate set boot report */
typedef struct
{
    uint16 handle;
    uint16 size_data;
    uint16 type;
    uint8 data[1];
}HID_CLIENT_INTERNAL_MSG_WRITE_BOOT_REPORT_T;

/* Internal Message Structure to Initiate read boot report */
typedef struct
{
    uint16 report_type;
}HID_CLIENT_INTERNAL_MSG_READ_BOOT_REPORT_T;

/* Internal Message Structure to Initiate read of CCD handle */
typedef struct
{
    uint16 ccd_handle;
}HID_CLIENT_INTERNAL_MSG_READ_CCD_T;

/* HID instance structure used for Handling Discovery, only one instance of this structure can be supported */
typedef struct
{
    unsigned num_report_ids:5; /* A counter to keep Number of report ID's read from HIDServer */
    unsigned num_reports_available:5; /* A counter to keep track of report mapping handles read from HIDServer */ 
    unsigned read_report_id_count:5; /*A counter to keep track of report ID values read from HID server  */
    unsigned is_boot_handle:1; /* Flag to store the handle is boot mode handle or report mode handle */
    unsigned read_report_handle_count:5; /*Counter to keep track of report handles */
    unsigned max_report:5; /* Maximum reports supported */
    unsigned max_reportmode_ccd:5; /* Maximum report mode CCD's  */
    unsigned boot_mode_supported:1; /* Is boot mode support is configured */
    unsigned max_bootmode_ccd:5; /* Maximum boot mode CCD's */
    unsigned max_char_handles:5; /* Maximum Characteristic handles supported */
    unsigned char_uuid_count:5; /* Characteristic UUID counts to keep in track how many are availabel */
    unsigned _SPARE_:1; 
    uint16 char_descriptor_mask; /* Characteristic descriptor mask used to read the descriptors */
    uint16 end_handle; /* End handle of the service */
    uint16 characteristic_handle; /* Store the characteristic report handle for a client characteristic descriptor */
    uint16 char_array[1]; /* descriptor array used to store declaration handles till discovery is completed */
}gatt_hid_current_discovery_instance_t;


#endif /* GATT_HID_PRIVATE_H */


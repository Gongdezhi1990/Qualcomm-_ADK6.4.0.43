/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*    */
#ifndef _USB_DEVICE_CLASS_TRANSPORT_PRIVATE_H
#define _USB_DEVICE_CLASS_TRANSPORT_PRIVATE_H

#define USB_RESPONSE_MAX_SIZE       64
#define USB_DATA_MAX_SIZE           64

typedef struct {
    uint8 report_id;
    uint8 size;
    uint8 data[USB_DATA_MAX_SIZE];
} hid_data_msg_t;

typedef struct {
    uint8 report_id;
    uint8 size;     
    uint8 response[USB_RESPONSE_MAX_SIZE];
} hid_response_msg_t;

typedef struct {
    uint8 report_id;
    uint8 size;        
    uint8 command[2];
} hid_command_msg_t;

#define REPORT_TRANSFER_SIZE    ((sizeof(hid_data_msg_t)/sizeof(uint8))-1)
#define REPORT_RESPONSE_SIZE    ((sizeof(hid_response_msg_t)/sizeof(uint8))-1)
#define REPORT_COMMAND_SIZE     ((sizeof(hid_command_msg_t)/sizeof(uint8))-1)

#define USB_UPGRADE_RESPONSE_SIZE      12

typedef struct {
    uint8 report_id;
    uint8 size;
    uint8 response[USB_UPGRADE_RESPONSE_SIZE];
} hid_upgrade_response_msg_t;

#define REPORT_UPGRADE_RESPONSE_SIZE    ((sizeof(hid_upgrade_response_msg_t)/sizeof(uint8))-1)

/* commands received from command report ID */

#define HID_CMD_CONNECTION_REQ              0x02
#define HID_CMD_GET_STATUS_REQ              0x03
#define HID_CMD_STATUS_REQ                  0x04
#define HID_CMD_RESPONSE_REQ                0x05
#define HID_CMD_ABORT_REQ                   0x06
#define HID_CMD_DISCONNECT_REQ              0x07
#define HID_CMD_RESET_REQ                   0xFF

#define USB_HID_CSR_SIGNATURE               0x0C57


#endif /* _USB_DEVICE_CLASS_TRANSPORT_PRIVATE_H */



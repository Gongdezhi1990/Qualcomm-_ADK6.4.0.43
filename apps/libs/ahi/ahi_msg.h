/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    ahi_msg.h

DESCRIPTION
    AHI internal message types. The types in this header are intended
    to be used only within the AHI library.

    When an AHI data packet is received from the host it gets converted
    to one of the types defined below. However if the the data packet
    has no payload (header only) then only the opcode is needed and
    there will be no type defined for it below.

    Note: The memory layout of an AHI data packet and an internal
    message is different on BlueCore devices:
      AHI data packet (ahi_protocol.h):
        In "stream" format, i.e. one byte per word.
      AHI internal message (ahi_msg.h):
        In "packed" format, i.e. two bytes per word.

*/

#ifndef AHI_MSG_H_
#define AHI_MSG_H_

#include <csrtypes.h>
#include <source.h>


/******************************************************************************
    Define the internal AHI message Ids
*/

#define AHI_INTERNAL_MSG_BASE 0x3000

#define AHI_INTERNAL_CONFIG_GET_REQ_COMPLETE 0x3001
#define AHI_INTERNAL_APP_MODE_SET_REBOOT 0x3002
#define AHI_INTERNAL_DELAYED_SEND_DATA 0x3003

/* Map AHI opcodes to an internal msg id */
#define AHI_INTERNAL_MSG_OPCODE_BASE 0x3800

#define AHI_INTERNAL_MSG_ID_FROM_OPCODE(opcode) (opcode + AHI_INTERNAL_MSG_OPCODE_BASE)
#define AHI_OPCODE_FROM_INTERNAL_MSG_ID(id) (id - AHI_INTERNAL_MSG_OPCODE_BASE)

#define AHI_INTERNAL_MSG_OPCODE_LAST 0x3900

#define IS_AHI_OPCODE(id) (AHI_INTERNAL_MSG_OPCODE_BASE <= id && AHI_INTERNAL_MSG_OPCODE_LAST > id)


#define AHI_INTERNAL_MSG_LAST 0x3FFF


/******************************************************************************
    Message data definitions for messages that have a payload
*/

typedef struct
{
    Task transport_task;
} AHI_CONNECT_REQ_T;

typedef struct
{
    Task transport_task;
} AHI_DISCONNECT_REQ_T;

typedef struct
{
    uint16 status;
    uint16 major;
    uint16 minor;
} AHI_VERSION_CFM_T;

typedef struct
{
    uint16 config_id;
} AHI_CONFIG_GET_REQ_T;

typedef struct
{
    uint16 config_id;
    uint16 status;
    uint16 size;
    uint16 offset;
    const void *data;
} AHI_CONFIG_GET_CFM_T;

typedef struct
{
    uint8 flags;
    uint16 config_id;
    uint16 size;
    uint16 data_size;
    uint8 data[1];
} AHI_CONFIG_SET_REQ_T;

typedef struct
{
    uint16 config_id;
    uint16 status;
} AHI_CONFIG_SET_CFM_T;

typedef struct
{
    uint16 status;
    uint32 size;
} AHI_CONFIG_FILE_GET_CFM_T;

typedef struct
{
    Source source;
} AHI_CONFIG_FILE_GET_DATA_IND_T;

typedef struct
{
    uint16 status;
    ahi_application_mode_t current_mode;
} AHI_MODE_GET_CFM_T;

typedef struct
{
    ahi_application_mode_t app_mode;
} AHI_MODE_SET_REQ_T;

typedef struct
{
    uint16 status;
    ahi_application_mode_t app_mode;
} AHI_MODE_SET_CFM_T;

typedef struct
{
    uint16 event;
} AHI_EVENT_INJECT_IND_T;

typedef struct
{
    uint16 id;
    const uint8 *payload;
    uint16 payload_size;
} AHI_EVENT_REPORT_IND_T;

typedef struct
{
    uint16 machine_id;
    uint16 state;
} AHI_STATE_MACHINE_STATE_REPORT_IND_T;

typedef struct
{
    uint16 mtu;
} AHI_REPORT_MTU_IND_T;

typedef struct
{
    uint16 ci;
} AHI_REPORT_CONN_INT_IND_T;

typedef struct
{
    uint16 status;
    uint16 type;
    const uint16 *signature;
} AHI_CONFIG_FILE_SIGNATURE_CFM_T;

typedef struct
{
    uint16 status;
} AHI_DISABLE_TRANSPORT_CFM_T;

typedef struct
{
    uint16 status;
    uint32 product_id;
} AHI_PRODUCT_ID_GET_CFM_T;

typedef struct
{
    uint16 status;
    uint16 app_build_id;
} AHI_APP_BUILD_ID_GET_CFM_T;


/*
   When ahi_host_if has sent all the data in a config block to the Host it
   sends this message to ahi_task to tell it to release the config block
   data that was being read.
*/
typedef struct
{
    uint16 config_id;
} AHI_INTERNAL_CONFIG_GET_REQ_COMPLETE_T;

/*
    When a AHI_TRANSPORT_SEND_DATA_T can't be sent to the host because the
    outgoing queue is full, this message may be used to post it back to
    ahi_task so that it can be sent later.
*/
typedef struct
{
    Task transport_task;
    AHI_TRANSPORT_SEND_DATA_T send_data_msg;
} AHI_INTERNAL_DELAYED_SEND_DATA_T;

#endif /* AHI_MSG_H_ */

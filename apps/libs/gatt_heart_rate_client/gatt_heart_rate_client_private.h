/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_HEART_RATE_CLIENT_PRIVATE_H
#define GATT_HEART_RATE_CLIENT_PRIVATE_H

#include <csrtypes.h>
#include <message.h>
#include <panic.h>

#include "gatt.h"
#include "gatt_manager.h"

#include "gatt_heart_rate_client.h"
#include "gatt_heart_rate_client_debug.h"
#include "gatt_client.h"

/* Defines for heart rate client library */

/* Invalid heart rate measurement handle*/
#define INVALID_HEART_RATE_HANDLE (0xffff)

/* HRP notification value */
#define HEART_RATE_NOTIFICATION_VALUE   (0x01)

 /* Macros for creating messages */
#define MAKE_HEART_RATE_CLIENT_MESSAGE(TYPE) TYPE##_T *message = (TYPE##_T *)PanicNull(calloc(1,sizeof(TYPE##_T)))

/*To be used for WORD order values*/
#define MAKE_HEART_RATE_CLIENT_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicNull(calloc(1,sizeof(TYPE##_T) + LEN))

#define NOTIFY_LEN_WITHOUT_RRINTERVAL 2 /* flag + HR measurement in uint8 value and energy expended field not present */

/* Macro to check control point support */
#define HR_CHECK_CONTROL_POINT_SUPPORT(heart_rate_client) \
    (heart_rate_client->hr_control_point_handle!=INVALID_HEART_RATE_HANDLE)?(TRUE):(FALSE)

/* Macro to fill in high byte and low byte */
#define HR_FILL_HIGH_AND_LOW_BYTE(input_offset, input, output) \
        {\
            uint8 low_byte=0;\
            low_byte = (uint8)input[++input_offset];\
            output = (uint16) (input[++input_offset]<<8)|(low_byte);\
        }

/* Enum For LIB internal messages */
typedef enum
{
    HEART_RATE_CLIENT_INTERNAL_MSG_BASE = 0,
    HEART_RATE_CLIENT_INTERNAL_MSG_DISCOVER,                 /* For a discover Request */
    HEART_RATE_CLIENT_INTERNAL_MSG_NOTIFICATION_REQ,  /* For enable/disable notification */
    HEART_RATE_CLIENT_INTERNAL_MSG_RESET_EE_REQ,        /*  For reset energy expended field */
    HEART_RATE_CLIENT_INTERNAL_MSG_TOP                        /* Top of message */
}heart_rate_client_internal_msg_t;

/* Enumerations for HID client message which is pending and yet to process completely  */
typedef enum
{
    heart_rate_client_pending_none = 0,
    heart_rate_client_init_pending,
    heart_rate_client_write_pending_notification,
    heart_rate_client_write_pending_reset_ee
}heart_rate_client_pending_read_type;

/*Set the pending FLAG so that only one request is processed at a TIME */
#define SET_PENDING_FLAG(type, pending_request) (pending_request = type)
/* Clear pending FLAG as new requests can be processed */
#define CLEAR_PENDING_FLAG(pending_request) (pending_request = heart_rate_client_pending_none)

/* Internal message structure for discover request  */
typedef struct
{
    uint16 cid;                 /* Connection Identifier for remote device */                                                             
    uint16 start_handle;  /* Start handle of the service */
    uint16 end_handle;   /* End handle of the service */
} HEART_RATE_CLIENT_INTERNAL_MSG_DISCOVER_T;

/* Internal Message Structure to Initiate registering notification */
typedef struct
{
    bool enable; /* Enable/Disable notification */   
}HEART_RATE_CLIENT_INTERNAL_MSG_NOTIFICATION_REQ_T;

#endif /* GATT_HEART_RATE_CLIENT_PRIVATE_H */


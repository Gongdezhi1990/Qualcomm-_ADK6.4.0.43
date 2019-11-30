
/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_IMM_ALERT_CLIENT_PRIVATE_H
#define GATT_IMM_ALERT_CLIENT_PRIVATE_H

#include <csrtypes.h>
#include <message.h>
#include <panic.h>

#include "gatt_imm_alert_client.h"
#include "gatt_client.h"


#define MAKE_IMM_ALERT_CLIENT_MESSAGE(TYPE) TYPE##_T *message = (TYPE##_T*)PanicNull(calloc(1,sizeof(TYPE##_T)))


/* Defines for immediate alert client library */

/* Set alert level Value length*/
#define GATT_IMM_ALERT_VALUE_LEN    (0x0001)
/* Invalid immediate alert handle*/
#define INVALID_IMM_ALERT_HANDLE    (0xffff)
/* Immediate Alert level characteristics  UUID as defined in bluetooth.org */
#define IMM_ALERT_LEVEL_CHAR_UUID   (0x2A06)

/* Enumeration for confirmation message types */
typedef enum
{
    IMM_ALERT_CHAR_HANDLE_CFM,    /* characteristics  handle */
    IMM_ALERT_WRITE_REQUEST_CFM   /* Write Alert Level */
}imm_alert_cfm_mesage_type;

/* Enum For LIB internal messages */
typedef enum
{
    IMM_ALERT_CLIENT_INTERNAL_MSG_BASE = 0,
    IMM_ALERT_CLIENT_INTERNAL_MSG_CONNECT,      /* For a connect Request */
    IMM_ALERT_CLIENT_INTERNAL_MSG_SET_ALERT,    /* For setting Alert Level */
    IMM_ALERT_CLIENT_INTERNAL_MSG_TOP               /* Top of message */
}imm_alert_client_internal_msg_t;

/* Internal message structure for connect request  */
typedef struct
{
    uint16 cid;                 /* Connection Identifier for remote device */                                                             
    uint16 start_handle;  /* Start handle of the service */
    uint16 end_handle;    /* End handle of the service */
} IMM_ALERT_CLIENT_INTERNAL_MSG_CONNECT_T;

/* Internal message structure for setting alert level */
typedef struct
{
    gatt_imm_alert_set_level alert_level; /*Alert level that need to be set */
} IMM_ALERT_CLIENT_INTERNAL_MSG_SET_ALERT_T;

#endif /* GATT_IMM_ALERT_CLIENT_PRIVATE_H */


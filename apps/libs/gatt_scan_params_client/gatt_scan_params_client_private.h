
/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_SCAN_PARAMS_CLIENT_PRIVATE_H
#define GATT_SCAN_PARAMS_CLIENT_PRIVATE_H

#include <csrtypes.h>
#include <message.h>
#include <panic.h>

#include <gatt_manager.h>

#include "gatt_scan_params_client_debug.h"

/* Macros for creating messages */
#define MAKE_SCAN_PARAMS_CLIENT_MESSAGE(TYPE) TYPE##_T *message = (TYPE##_T*)PanicNull(calloc(1,sizeof(TYPE##_T)))

/* Defines for Scan Parameters client library */

/* Invalid Scan Interval Window handle*/
#define INVALID_SCAN_INTERVAL_WINDOW_HANDLE (0xffff)

/* Invalid Scan Refresh handle*/
#define INVALID_SCAN_REFRESH_HANDLE (0xffff)

/* Scan interval window characteristics  UUID as defined in bluetooth.org */
#define SCAN_INTERVAL_WINDOW_CHAR_UUID   (0x2A4F)

/* Scan refresh characteristics  UUID as defined in bluetooth.org */
#define SCAN_REFRESH_CHAR_UUID   (0x2A31)

/* Enable scan refresh notification */
#define ENABLE_SCAN_REFRESH_NOTIFICATION 0x01

/* Scan refresh required */
#define SCAN_REFRESH_REQUIRED   0

/* Check Init Input prams are valid */
#define INPUT_PARAM_NULL(app_task,scan_params_client,init_params) (app_task == NULL) || (scan_params_client == NULL ) ||(init_params == NULL)

/* Enum For LIB internal messages */
typedef enum
{
    SCAN_PARAMS_CLIENT_INTERNAL_MSG_BASE = 0,
    SCAN_PARAMS_CLIENT_INTERNAL_MSG_SET_SCAN,      /* For a updating scan parameters */
    SCAN_PARAMS_CLIENT_INTERNAL_MSG_TOP            /* Top of message */
}scan_params_client_internal_msg_t;

#endif /* GATT_SCAN_PARAMS_CLIENT_PRIVATE_H */


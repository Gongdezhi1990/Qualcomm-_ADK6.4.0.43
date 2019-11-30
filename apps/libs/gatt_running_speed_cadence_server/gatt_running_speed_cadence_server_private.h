/* Copyright (c) 2017 Qualcomm Technologies International, Ltd.

*/

#ifndef GATT_RSCS_SERVER_PRIVATE_H
#define GATT_RSCS_SERVER_PRIVATE_H

#include <csrtypes.h>
#include <message.h>
#include <panic.h>

#include <gatt.h>
#include <gatt_manager.h>

#include "gatt_running_speed_cadence_server.h"
#include "gatt_running_speed_cadence_server_db.h"
#include "gatt_running_speed_cadence_server_debug.h"

/* Check Init Input prams are valid */
#define INPUT_PARAM_NULL(app_task,rscs) (app_task == NULL) || (rscs == NULL )


/* Macros for creating messages */
#define MAKE_RSCS_SERVER_MESSAGE(TYPE) TYPE##_T* message = (TYPE##_T *)PanicNull(calloc(1,sizeof(TYPE##_T)))

#endif /* GATT_RSCS_SERVER_PRIVATE_H */


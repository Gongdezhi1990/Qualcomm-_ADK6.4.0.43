/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_HR_SERVER_PRIVATE_H
#define GATT_HR_SERVER_PRIVATE_H

#include <csrtypes.h>
#include <message.h>
#include <panic.h>

#include <gatt.h>
#include <gatt_manager.h>

#include "gatt_heart_rate_server.h"
#include "gatt_heart_rate_server_db.h"
#include "gatt_heart_rate_server_debug.h"

/* Check Init Input prams are valid */
#define INPUT_PARAM_NULL(app_task,hr_Sensor) (app_task == NULL) || (hr_Sensor == NULL )


/* Macros for creating messages */
#define MAKE_HR_SERVER_MESSAGE(TYPE) TYPE##_T* message = (TYPE##_T *)PanicNull(calloc(1,sizeof(TYPE##_T)))

#endif /* GATT_HR_SERVER_PRIVATE_H */


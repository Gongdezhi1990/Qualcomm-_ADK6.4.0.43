/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_BATTERY_SERVER_PRIVATE_H_
#define GATT_BATTERY_SERVER_PRIVATE_H_

#include <csrtypes.h>
#include <message.h>
#include <panic.h>
#include <stdlib.h>

#include <gatt_manager.h>

#include "gatt_battery_server.h"
#include "gatt_battery_server_debug.h"

/* Macros for creating messages */
#define MAKE_BATTERY_MESSAGE(TYPE) TYPE##_T *message = (TYPE##_T*)PanicNull(calloc(1,sizeof(TYPE##_T)))

#endif

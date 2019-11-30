/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/* Part of 6.2 */

#ifndef GATT_AMA_SERVER_PRIVATE_H_
#define GATT_AMA_SERVER_PRIVATE_H_

#include <csrtypes.h>
#include <message.h>
#include <panic.h>
#include <stdlib.h>

#include <gatt_manager.h>

#include "gatt_ama_server.h"
#include "gatt_ama_server_debug.h"

/* Macros for creating messages */
#define MAKE_GATT_AMA_SERVER_MESSAGE(TYPE) TYPE##_T *message = (TYPE##_T*)PanicNull(calloc(1,sizeof(TYPE##_T)))

#endif

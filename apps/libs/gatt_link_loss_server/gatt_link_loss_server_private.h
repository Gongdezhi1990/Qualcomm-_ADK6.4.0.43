/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_LINK_LOSS_SERVER_PRIVATE_H
#define GATT_LINK_LOSS_SERVER_PRIVATE_H

#include <csrtypes.h>
#include <message.h>
#include <panic.h>
#include <stdlib.h>

#include <gatt.h>

#include "gatt_link_loss_server.h"
#include "gatt_link_loss_server_debug.h"
#include "gatt_link_loss_server_db.h"

/* Check Init Input prams are valid */
#define INPUT_PARAM_NULL(app_task, link_loss_server) (app_task == NULL) || (link_loss_server == NULL )

/* Macros for creating messages */
#define MAKE_LINK_LOSS_MESSAGE(TYPE) TYPE##_T *message = (TYPE##_T*)PanicNull(calloc(1,sizeof(TYPE##_T)))

#endif /* GATT_LINK_LOSS_SERVER_PRIVATE_H */


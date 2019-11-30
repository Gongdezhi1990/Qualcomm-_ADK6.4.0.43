/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_IMM_ALERT_PRIVATE_H
#define GATT_IMM_ALERT_PRIVATE_H

#include <csrtypes.h>
#include <message.h>
#include <panic.h>
#include <stdlib.h>

#include "gatt_imm_alert_server.h"
#include "gatt_imm_alert_server_db.h"
#include "gatt_imm_alert_server_debug.h"

/* Check Init Input prams are valid */
#define INPUT_PARAM_NULL(app_task,imm_alert_server) (app_task == NULL) || (imm_alert_server == NULL )

/* Macros for creating messages */
#define MAKE_IMM_ALERT_MESSAGE(TYPE) MESSAGE_MAKE(message,TYPE##_T)

#endif /* GATT_IMM_ALERT_PRIVATE_H */


/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */
/* 
    FILE NAME
    gatt_transmit_power_server_private.h

DESCRIPTION
    Header file for the Transmit Power Server Service data structure.
*/

/*!
@file   gatt_transmit_power_server_private.h
@brief  Header file for the Transmit Power Server data structure.

        This file documents the basic data structure of Transmit Power Server Service.
*/

#ifndef GATT_TRANSMIT_POWER_SERVER_PRIVATE_H
#define GATT_TRANSMIT_POWER_SERVER_PRIVATE_H

#include <csrtypes.h>
#include <message.h>
#include <panic.h>
#include <stdlib.h>

#include "gatt_transmit_power_server.h"
#include "gatt_transmit_power_server_db.h"
#include "gatt_transmit_power_server_debug.h"


/* Check Init Input prams are valid */
#define INPUT_PARAM_NULL(app_task, tps_server) (app_task == NULL) || (tps_server == NULL )

/* Macros for creating messages */
#define MAKE_TRANSMIT_POWER_MESSAGE(TYPE) MESSAGE_MAKE(message,TYPE##_T)

#endif /* GATT_TRANSMIT_POWER_SERVER_PRIVATE_H */

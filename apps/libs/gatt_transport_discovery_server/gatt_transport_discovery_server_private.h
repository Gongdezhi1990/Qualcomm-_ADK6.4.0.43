/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_TDS_SERVER_PRIVATE_H
#define GATT_TDS_SERVER_PRIVATE_H

#include <csrtypes.h>
#include <message.h>
#include <panic.h>

#include <gatt.h>
#include <gatt_manager.h>

#include "gatt_transport_discovery_server.h"
#include "gatt_transport_discovery_server_db.h"
#include "gatt_transport_discovery_server_debug.h"

/* Macros for creating messages */
#define MAKE_TDS_SERVER_MESSAGE(TYPE) TYPE##_T* message = (TYPE##_T *)PanicNull(calloc(1,sizeof(TYPE##_T)))

#define MAKE_TDS_SERVER_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicNull(calloc(1,sizeof(TYPE##_T) + LEN))

#endif /* GATT_TDS_SERVER_PRIVATE_H */


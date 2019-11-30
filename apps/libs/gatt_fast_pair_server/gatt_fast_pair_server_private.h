/* Copyright (c) 2018 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_FAST_PAIR_SERVER_PRIVATE_H
#define GATT_FAST_PAIR_SERVER_PRIVATE_H

#include <csrtypes.h>
#include <message.h>
#include <panic.h>
#include <stdlib.h>
#include <string.h>

#include <gatt.h>
#include <gatt_manager.h>

#include "gatt_fast_pair_server.h"
#include "gatt_fast_pair_server_db.h"
#include "gatt_fast_pair_server_debug.h"

#define MAKE_FPS_MESSAGE(TYPE) \
    TYPE##_T* message = (TYPE##_T *)PanicNull(calloc(1, sizeof(TYPE##_T)))

#define MAKE_FPS_MESSAGE_WITH_LEN(TYPE, LEN) \
    TYPE##_T *message = (TYPE##_T *)PanicNull( \
            calloc(1, sizeof(TYPE##_T) + (sizeof(uint8) * LEN) - 1) \
            )

#endif /* GATT_FAST_PAIR_SERVER_PRIVATE_H */

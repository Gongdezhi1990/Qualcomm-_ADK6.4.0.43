/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_CLIENT_PRIVATE_H_
#define GATT_CLIENT_PRIVATE_H_

#include "gatt_client.h"

#include <csrtypes.h>
#include <message.h>
#include <panic.h>
#include <stdlib.h>


/* Macros for creating messages */
#define MAKE_GATT_CLIENT_MESSAGE(TYPE) MESSAGE_MAKE(message,TYPE##_T);


#endif

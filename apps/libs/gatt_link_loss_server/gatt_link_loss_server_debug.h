/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

/*
FILE NAME
    gatt_link_loss_server_debug.h

DESCRIPTION
    Header file for the GATT Link Loss library debug functionality.
*/
#ifndef GATT_LINK_LOSS_SERVER_DEBUG_H_
#define GATT_LINK_LOSS_SERVER_DEBUG_H_

/* Macro used to generate debug version of this library */
#ifdef GATT_LINK_LOSS_DEBUG_LIB

#ifndef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT_ENABLED
#endif

#include <panic.h>
#include <print.h>
#include <stdio.h>

#define GATT_LINK_LOSS_DEBUG_INFO(x) {PRINT(("%s:%d - ", __FILE__, __LINE__)); PRINT(x);}
#define GATT_LINK_LOSS_DEBUG_PANIC(x) {GATT_LINK_LOSS_DEBUG_INFO(x); Panic();}
#define GATT_LINK_LOSS_SERVER_PANIC(x) {GATT_LINK_LOSS_DEBUG_INFO(x); Panic();}

#else /* GATT_LINK_LOSS_DEBUG_LIB */


#define GATT_LINK_LOSS_DEBUG_INFO(x)
#define GATT_LINK_LOSS_DEBUG_PANIC(x)
#define GATT_LINK_LOSS_SERVER_PANIC(x) { Panic();}
#endif /* GATT_LINK_LOSS_DEBUG_LIB */

#endif /* GATT_LINK_LOSS_COMMON_H_ */


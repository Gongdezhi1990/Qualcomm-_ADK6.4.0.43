/* Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd. */
/*  */

/*
FILE NAME
    gatt_broadcast_client_debug.h

DESCRIPTION
    Header file for the GATT BA client library debug functionality.
*/
#ifndef GATT_BROADCAST_CLIENT_DEBUG_H
#define GATT_BROADCAST_CLIENT_DEBUG_H

/* Macro used to generate debug version of this library */
#ifdef GATT_BROADCAST_CLIENT_DEBUG_LIB

#ifndef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT_ENABLED
#endif

#include <panic.h>
#include <print.h>
#include <stdio.h>

#define GATT_BROADCAST_CLIENT_DEBUG_INFO(x) {PRINT(("%s:%d - ", __FILE__, __LINE__)); PRINT(x);}
#define GATT_BROADCAST_CLIENT_DEBUG_PANIC(x) {GATT_BROADCAST_CLIENT_DEBUG_INFO(x); Panic();}


#else /* GATT_BROADCAST_CLIENT_DEBUG_LIB */


#define GATT_BROADCAST_CLIENT_DEBUG_INFO(x)
#define GATT_BROADCAST_CLIENT_DEBUG_PANIC(x) {Panic();}

#endif /* GATT_BROADCAST_CLIENT_DEBUG_LIB */


#endif /* GATT_BROADCAST_CLIENT_DEBUG_H */


/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

/*
FILE NAME
    gatt_heart_rate_client.h

DESCRIPTION
    Header file for the GATT Heart rate client library debug functionality.
*/
#ifndef GATT_HEART_RATE_CLIENT_DEBUG_H
#define GATT_HEART_RATE_CLIENT_DEBUG_H

/* Macro used to generate debug version of this library */
#ifdef GATT_HEART_RATE_CLIENT_DEBUG_LIB

#ifndef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT_ENABLED
#endif

#include <panic.h>
#include <print.h>
#include <stdio.h>

#define GATT_HEART_RATE_CLIENT_DEBUG_INFO(x) {PRINT(("%s:%d - ", __FILE__, __LINE__)); PRINT(x);}
#define GATT_HEART_RATE_CLIENT_DEBUG_PANIC(x) {GATT_HEART_RATE_CLIENT_DEBUG_INFO(x); Panic();}


#else /* GATT_HEART_RATE_CLIENT_DEBUG_LIB */


#define GATT_HEART_RATE_CLIENT_DEBUG_INFO(x)
#define GATT_HEART_RATE_CLIENT_DEBUG_PANIC(x) {Panic();}

#endif /* GATT_HEART_RATE_CLIENT_DEBUG_LIB */


#endif /* GATT_HEART_RATE_CLIENT_DEBUG_H */


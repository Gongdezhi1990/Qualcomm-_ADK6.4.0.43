/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

/*
FILE NAME
    gatt_scan_params_client_debug.h

DESCRIPTION
    Header file for the GATT Scan Parameters client library debug functionality.
*/
#ifndef GATT_SCAN_PARAMS_CLIENT_DEBUG_H_
#define GATT_SCAN_PARAMS_CLIENT_DEBUG_H_

#include <panic.h>

/* Macro used to generate debug version of this library */
#ifdef GATT_SCAN_PARAMS_CLIENT_DEBUG_LIB

#ifndef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT_ENABLED
#endif

#include <print.h>
#include <stdio.h>

#define GATT_SCAN_PARAMS_CLIENT_DEBUG_INFO(x) {PRINT(("%s:%d - ", __FILE__, __LINE__)); PRINT(x);}
#define GATT_SCAN_PARAMS_CLIENT_DEBUG_PANIC(x) {GATT_SCAN_PARAMS_CLIENT_DEBUG_INFO(x); Panic();}
#define GATT_SCAN_PARAMS_CLIENT_PANIC(MSG) {GATT_SCAN_PARAMS_CLIENT_DEBUG_INFO(MSG); Panic();}

#else /* GATT_SCAN_PARAMS_CLIENT_DEBUG_LIB */

#define GATT_SCAN_PARAMS_CLIENT_DEBUG_INFO(x)
#define GATT_SCAN_PARAMS_CLIENT_DEBUG_PANIC(x)
#define GATT_SCAN_PARAMS_CLIENT_PANIC(MSG) {Panic();}

#endif /* GATT_SCAN_PARAMS_CLIENT_DEBUG_LIB */


#endif /* GATT_SCAN_PARAMS_CLIENT_DEBUG_H_ */

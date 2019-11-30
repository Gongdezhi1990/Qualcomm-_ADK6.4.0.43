/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. 

 */

/*
FILE NAME
    gatt_logging_server_debug.h

DESCRIPTION
    Header file for the GATT logging service library debug functionality.
*/
#ifndef GATT_LOGGING_SERVER_DEBUG_H_
#define GATT_LOGGING_SERVER_DEBUG_H_

#include <panic.h>

/* Macro used to generate debug version of this library */
#ifdef GATT_LOGGING_SERVER_DEBUG_LIB

#ifndef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT_ENABLED
#endif

#include <print.h>
#include <stdio.h>

#define GATT_LOGGING_SERVER_DEBUG_INFO(x) {PRINT(("%s:%d - ", __FILE__, __LINE__)); PRINT(x);}
#define GATT_LOGGING_SERVER_DEBUG_PANIC(x) {GATT_RSC_SERVER_DEBUG_INFO(x); Panic();}
#define GATT_LOGGING_SERVER_PANIC(MSG) {GATT_RSC_SERVER_DEBUG_INFO(MSG); Panic();}

#else /* GATT_RSC_SERVER_DEBUG_LIB */

#define GATT_LOGGING_SERVER_DEBUG_INFO(x)
#define GATT_LOGGING_SERVER_DEBUG_PANIC(x)
#define GATT_LOGGING_SERVER_PANIC(MSG) {Panic();}

#endif /* GATT_LOGGING_SERVER_DEBUG_LIB */


#endif /* GATT_LOGGING_SERVER_DEBUG_H_ */


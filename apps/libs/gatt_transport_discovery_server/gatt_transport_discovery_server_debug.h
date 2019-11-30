/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*  */

/*
FILE NAME
    gatt_transport_discovery_server_debug.h

DESCRIPTION
    Header file for the GATT Transport Discovery Server library debug functionality.
*/
#ifndef GATT_TDS_SERVER_DEBUG_H_
#define GATT_TDS_SERVER_DEBUG_H_

#include <panic.h>

/* Macro used to generate debug version of this library */
#ifdef GATT_TDS_SERVER_DEBUG_LIB

#ifndef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT_ENABLED
#endif

#include <print.h>
#include <stdio.h>

#define GATT_TDS_SERVER_DEBUG_INFO(x) {PRINT(("%s:%d - ", __FILE__, __LINE__)); PRINT(x);}
#define GATT_TDS_SERVER_DEBUG_PANIC(x) {GATT_TDS_SERVER_DEBUG_INFO(x); Panic();}
#define GATT_TDS_SERVER_PANIC(MSG) {GATT_TDS_SERVER_DEBUG_INFO(MSG); Panic();}

#else /* GATT_TDS_SERVER_DEBUG_LIB */

#define GATT_TDS_SERVER_DEBUG_INFO(x)
#define GATT_TDS_SERVER_DEBUG_PANIC(x)
#define GATT_TDS_SERVER_PANIC(MSG) {Panic();}

#endif /* GATT_TDS_SERVER_DEBUG_LIB */


#endif /* GATT_TDS_SERVER_DEBUG_H_ */


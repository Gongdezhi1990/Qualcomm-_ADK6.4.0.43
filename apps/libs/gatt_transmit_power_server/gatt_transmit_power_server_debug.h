/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

/*
FILE NAME
    gatt_transmit_power_server_debug.h

DESCRIPTION
    Header file for the GATT Transmit Power library debug functionality.
*/
#ifndef GATT_TRANSMIT_POWER_SERVER_DEBUG_H_
#define GATT_TRANSMIT_POWER_SERVER_DEBUG_H_

/* Macro used to generate debug version of this library */
#ifdef GATT_TRANSMIT_POWER_SERVER_DEBUG_LIB

#ifndef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT_ENABLED
#endif

#include <panic.h>
#include <print.h>
#include <stdio.h>

#define GATT_TRANSMIT_POWER_SERVER_DEBUG_INFO(x) {PRINT(("%s:%d - ", __FILE__, __LINE__)); PRINT(x);}
#define GATT_TRANSMIT_POWER_SERVER_DEBUG_PANIC(x) {GATT_TRANSMIT_POWER_SERVER_DEBUG_INFO(x); Panic();}
#define GATT_TRANSMIT_POWER_SERVER_PANIC(MSG) {GATT_TRANSMIT_POWER_SERVER_DEBUG_INFO(MSG); Panic();}

#else /* GATT_TRANSMIT_POWER_SERVER_DEBUG_LIB */

#define GATT_TRANSMIT_POWER_SERVER_DEBUG_INFO(x)
#define GATT_TRANSMIT_POWER_SERVER_DEBUG_PANIC(x)
#define GATT_TRANSMIT_POWER_SERVER_PANIC(MSG) {Panic();}

#endif /* GATT_TRANSMIT_POWER_SERVER_DEBUG_LIB */

#endif /* GATT_TRANSMIT_POWER_SERVER_DEBUG_H_ */

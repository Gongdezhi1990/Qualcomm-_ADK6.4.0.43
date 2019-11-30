/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

/*
FILE NAME
    gatt_hid_client_debug.h

DESCRIPTION
    Header file for the GATT HID Client library debug functionality.
*/
#ifndef GATT_HID_CLIENT_DEBUG_H_
#define GATT_HID_CLIENT_DEBUG_H_

/* Macro used to generate debug version of this library */
#ifdef GATT_HID_CLIENT_DEBUG_LIB

#ifndef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT_ENABLED
#endif

#include <panic.h>
#include <print.h>
#include <stdio.h>

#define GATT_HID_CLIENT_DEBUG_INFO(x) {PRINT(("%s:%d - ", __FILE__, __LINE__)); PRINT(x);}
#define GATT_HID_CLIENT_DEBUG_PANIC(x) {GATT_HID_CLIENT_DEBUG_INFO(x); Panic();}

#else /* GATT_HID_CLIENT_DEBUG_LIB */


#define GATT_HID_CLIENT_DEBUG_INFO(x)
#define GATT_HID_CLIENT_DEBUG_PANIC(x)

#endif /* GATT_HID_CLIENT_DEBUG_LIB */

#endif /* GATT_HID_CLIENT_DEBUG_H_ */


/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */


#ifndef GATT_BATTERY_CLIENT_DEBUG_H_
#define GATT_BATTERY_CLIENT_DEBUG_H_

#include <panic.h>

/* Macro used to generate debug version of this library */
#ifdef GATT_BATTERY_CLIENT_DEBUG_LIB


#ifndef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT_ENABLED
#endif

#include <print.h>
#include <stdio.h>

#define GATT_BATTERY_CLIENT_DEBUG_INFO(x)  {PRINT(("%s:%d - ", __FILE__, __LINE__)); PRINT(x);}
#define GATT_BATTERY_CLIENT_DEBUG_PANIC(x) {GATT_BATTERY_CLIENT_DEBUG_INFO(x); Panic();}
#define GATT_BATTERY_CLIENT_PANIC(x)       {GATT_BATTERY_CLIENT_DEBUG_INFO(x); Panic();}


#else /* GATT_BATTERY_CLIENT_DEBUG_LIB */


#define GATT_BATTERY_CLIENT_DEBUG_INFO(x)
#define GATT_BATTERY_CLIENT_DEBUG_PANIC(x)
#define GATT_BATTERY_CLIENT_PANIC(x)        {Panic();}

#endif /* GATT_BATTERY_CLIENT_DEBUG_LIB */


#endif /* GATT_BATTERY_CLIENT_DEBUG_H_ */

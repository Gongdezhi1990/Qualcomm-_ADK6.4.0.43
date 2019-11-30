/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/* Part of 6.2 */

#ifndef GATT_AMA_SERVER_DEBUG_H_
#define GATT_AMA_SERVER_DEBUG_H_

/* Macro used to generate debug version of this library */
#ifdef GATT_AMA_SERVER_DEBUG_LIB


#ifndef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT_ENABLED
#endif

#include <panic.h>
#include <print.h>
#include <stdio.h>



#define GATT_AMA_SERVER_DEBUG_INFO(x) {PRINT(("%d:", __LINE__)); PRINT(x);}
#define GATT_AMA_SERVER_DEBUG_PANIC(x) {GATT_AMA_SERVER_DEBUG_INFO(x); Panic();}
#define GATT_AMA_SERVER_PANIC(x) {GATT_AMA_SERVER_DEBUG_INFO(x); Panic();}


#else /* GATT_AMA_DEBUG_LIB */


#define GATT_AMA_SERVER_DEBUG_INFO(x)
#define GATT_AMA_SERVER_DEBUG_PANIC(x)
#define GATT_AMA_SERVER_PANIC(x) {Panic();}

#endif /* GATT_AMA_SERVER_DEBUG_LIB */


#endif /* GATT_AMA_SERVER_DEBUG_H_ */

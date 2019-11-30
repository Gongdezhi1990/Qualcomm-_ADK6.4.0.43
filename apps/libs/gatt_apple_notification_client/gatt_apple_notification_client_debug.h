/* Copyright (c) 2014 - 2019 Qualcomm Technologies International, Ltd. */

#ifndef GATT_APPLE_NOTIFICATION_CLIENT_DEBUG_H_
#define GATT_APPLE_NOTIFICATION_CLIENT_DEBUG_H_

#ifdef GATT_APPLE_NOTIFICATION_DEBUG_LIB
#define DEBUG_PANIC(x) {PRINT(x); Panic();}
#else
#define DEBUG_PANIC(x) {PRINT(x);}
#endif

#define PANIC(x) {PRINT(x); Panic();}

#endif

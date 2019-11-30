/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.

*/

#ifndef GATTMANAGER_MSG_INTERNAL_H_
#define GATTMANAGER_MSG_INTERNAL_H_

#include <csrtypes.h>
#include <message.h>
#include <panic.h>
#include "gatt_manager.h"

/* Macro used to generate debug version of this library */
#ifdef GATT_MANAGER_DEBUG_LIB

#ifndef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT_ENABLED
#endif

#include <print.h>
#include <stdio.h>

#define GATT_MANAGER_DEBUG_INFO(MSG) {PRINT(("%s:%d - ", __FILE__, __LINE__)); PRINT(MSG);}
#define GATT_MANAGER_DEBUG_PANIC(MSG) {GATT_MANAGER_DEBUG_INFO(MSG); Panic();}
#define GATT_MANAGER_PANIC(MSG) {GATT_MANAGER_DEBUG_INFO(MSG); Panic();}
#define GATT_MANAGER_PANIC_NULL(PTR, MSG) {if(NULL == PTR) GATT_MANAGER_DEBUG_INFO(MSG); PanicNull((void*)PTR);}
#define GATT_MANAGER_PANIC_NOT_NULL(PTR, MSG) {if(NULL != PTR) GATT_MANAGER_DEBUG_INFO(MSG); PanicNotNull((void*)PTR);}
#else

#define GATT_MANAGER_DEBUG_INFO(MSG)
#define GATT_MANAGER_DEBUG_PANIC(MSG)
#define GATT_MANAGER_PANIC(MSG) {Panic();}
#define GATT_MANAGER_PANIC_NULL(PTR, MSG) {PanicNull((void*)PTR);}
#define GATT_MANAGER_PANIC_NOT_NULL(PTR, MSG) {PanicNotNull((void*)PTR);}
#endif

#define GATT_MANAGER_INTERNAL_MSG_BASE 0x0000

#define MAKE_GATT_MANAGER_MESSAGE(TYPE) \
            TYPE##_T * const message = (TYPE##_T *)PanicNull(calloc(1,sizeof(TYPE##_T)))

#define MAKE_GATT_MANAGER_MESSAGE_WITH_LEN(TYPE, LEN) \
            TYPE##_T *message = (TYPE##_T *) PanicNull(calloc(1,sizeof(TYPE##_T) + (LEN)))

typedef enum __gatt_manager_internal_msg
{
    GATT_MANAGER_INTERNAL_MSG_CANCEL_ADVERTISING = GATT_MANAGER_INTERNAL_MSG_BASE,
    GATT_MANAGER_INTERNAL_MSG_WAIT_FOR_REMOTE_CLIENT,
    GATT_MANAGER_INTERNAL_MSG_CONNECT_TO_REMOTE_SERVER,
    GATT_MANAGER_INTERNAL_MSG_TOP

} gatt_manager_internal_msg_t;

typedef struct PACK_STRUCT
{
    Task task;
    bool taddr_not_null;
    typed_bdaddr taddr;
    gatt_connection_type conn_type;

} GATT_MANAGER_INTERNAL_MSG_WAIT_FOR_REMOTE_CLIENT_T;

typedef struct PACK_STRUCT
{
    Task task;
    typed_bdaddr taddr;
    gatt_connection_type conn_type;
    bool conn_timeout;

} GATT_MANAGER_INTERNAL_MSG_CONNECT_TO_REMOTE_SERVER_T;

#endif

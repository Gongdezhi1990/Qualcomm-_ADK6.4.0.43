/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    broadcast_cmd_debug.h

DESCRIPTION
    A utility file to enable debugging of the library
*/


/*!
@file    broadcast_cmd_debug.h
@brief   Write a short description about what the module does and how it
         should be used.

         This file provides methods to enable library debugging.
*/


#ifndef BROADCAST_CMD_DEBUG_
#define BROADCAST_CMD_DEBUG_

/* Macro used to generate debug version of this library */
#ifdef BROADCAST_CMD_DEBUG_LIB

#include <panic.h>
#include <print.h>
#include <stdio.h>

#define BROADCAST_CMD_DEBUG_INFO(x) printf x

/* Utility function to get the role as string for readability */
const char* BroadcastcmdGetRoleAsString(uint16 role);

#else /* BROADCAST_CMD_DEBUG_LIB */

#define BROADCAST_CMD_DEBUG_INFO(x)
/* if not a debug utility then just have a dummy function to avoid compilation error */
void BroadcastcmdGetRoleAsString(void);

#endif /* BROADCAST_CMD_DEBUG_LIB */

#endif /* BROADCAST_CMD_DEBUG_ */

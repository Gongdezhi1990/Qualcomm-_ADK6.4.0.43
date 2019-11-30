/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    broadcast_cmd_debug.c

DESCRIPTION
    A utility file to enable debugging of the library
*/


/*!
@file    broadcast_cmd_debug.c
@brief   This file provides methods to enable library debugging.
*/


#include "broadcast_cmd_debug.h"

#ifdef BROADCAST_CMD_DEBUG_LIB

const char * const bcmd_role[3] = {
    "UNKNOWN",
    "BROADCASTER",
    "RECEIVER",
};

/*!
@brief   Untility function to provide the string equivalent of the role
*/
const char* BroadcastcmdGetRoleAsString(uint16 role)
{
    return bcmd_role[role];
}

#else

/*!
@brief   if not it debug, then dummy function
*/
void BroadcastcmdGetRoleAsString(void)
{
    /* Dummy */
}


#endif /* BROADCAST_CMD_DEBUG_LIB */


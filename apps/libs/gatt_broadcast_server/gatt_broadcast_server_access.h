/* Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd. */
/*  */

/*
FILE NAME
    gatt_broadcast_server_access.h

DESCRIPTION
    Interface for access to Broadcast GATT service characteristics.
*/

#ifndef GATT_BROADCAST_SERVER_ACCESS_H_
#define GATT_BROADCAST_SERVER_ACCESS_H_


#include <gatt_manager.h>

#include "gatt_broadcast_server.h"

/***************************************************************************
NAME
    broadcast_server_handle_access_ind

DESCRIPTION
    Deals with access of BA Service characteristics.
*/
void broadcast_server_handle_access_ind(GBSS* broadcast_server,
                                               const GATT_MANAGER_SERVER_ACCESS_IND_T* ind);

#endif


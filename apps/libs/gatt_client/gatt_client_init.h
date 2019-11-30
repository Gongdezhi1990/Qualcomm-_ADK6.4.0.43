/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#ifndef GATT_CLIENT_INIT_H_
#define GATT_CLIENT_INIT_H_


#include "gatt_client_private.h"


/***************************************************************************
NAME
    gattClientSendInitCfm
    
DESCRIPTION
    Send a GATT_CLIENT_INIT_CFM message to the registered client task with 
    the supplied status code.
*/
void gattClientSendInitCfm(GGATTC *gatt_client, uint16 service_changed_handle, gatt_client_status_t status);



#endif

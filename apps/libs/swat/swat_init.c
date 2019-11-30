/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_init.c

DESCRIPTION
    This file contains the initialisation code for the SWAT profile library.
*/


/****************************************************************************
    Header files
*/
#include "swat.h"
#include "swat_private.h"
#include "swat_l2cap_handler.h"
#include "swat_command_handler.h"
#include "swat_profile_handler.h"
#include "swat_service_record.h"
#include "swat_api.h"

/* Libs */
#include <connection.h>

/* Firmware */
#include <panic.h>
#include <stdlib.h>



/* The one, and only, SWAT instance */
swatTaskData *swat = 0;  /* Pointer located in global space */


/****************************************************************************/
void SwatInit(Task clientTask, uint16 max_remote_devs, swat_role role, bool auto_handle, const uint8 *service_record, uint16 size_service_record, const sync_config_params * esco_config)
{
    /* Attempt to initialise the task data */
    if ( !swat )
    {
        swat = PanicUnlessNew(swatTaskData);
        memset( swat, 0, sizeof(swatTaskData) );
        
        SWAT_DEBUG(("[SWAT] Sizeof(swatTaskData) = %u\n", sizeof(swatTaskData)));
        
        /* Initialise the swat profile data */
        swat->l2cap_task.handler = swatL2capHandler;
        swat->profile_task.handler = swatProfileHandler;
        swat->command_task.handler = swatCommandHandler;
        swat->clientTask = clientTask;
        swat->auto_handle = auto_handle;
        swat->role = role;
        swat->max_remote_devs = max_remote_devs;

        /* Allocate memory to store remote device data depending on number of allowed devices */
        swat->remote_devs = (remoteDevice *)PanicNull(malloc(max_remote_devs * sizeof(remoteDevice)));
        memset(swat->remote_devs, 0, (max_remote_devs * sizeof(remoteDevice)));
        
        /* Use service record supplied by client */
        if (service_record)
        {
            ConnectionRegisterServiceRecord(&swat->l2cap_task, (sizeof(uint8) * size_service_record), service_record);
        }
        /* Use library default service record */
        else
        {
            ConnectionRegisterServiceRecord(&swat->l2cap_task, sizeof(swat_service_record), swat_service_record);
        }
        
        /* Use eSCO config supplied by client */
        if (esco_config)
        {
            swat->esco_config = esco_config;
        }
        else
        {
            swat->esco_config = NULL;
        }
    }
    else
    {
        swatSendInitCfmToClient(swat_init_failed);
    }
}

/* Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd. */
/*  */

#ifdef ENABLE_BROADCAST_GATT_SERVICE

#include "gatt_broadcast_client.h"
#include "gatt_broadcast_client_debug.h"
#include "gatt_broadcast_client_msg_handler.h"

#include <panic.h>
#include <gatt.h>
#include <gatt_manager.h>
#include <stdlib.h>
#include <gatt_client.h>

/*! @brief Initialisation function for the Broadcast client.
 */
gatt_broadcast_client_status GattBroadcastClientInit(Task app_task,
                                                     GBSC* const broadcast_client,
                                                     const GATT_BROADCAST_CLIENT_INIT_PARAMS_T* params)
{
    gatt_broadcast_client_status retval = gatt_broadcast_client_status_invalid_parameter;
    gatt_manager_client_registration_params_t reg_params;
        /* validate the input parameters */
    if((app_task == NULL) || (broadcast_client == NULL) || CLIENT_INIT_PARAM_INVALID(params))
    {
        GATT_BROADCAST_CLIENT_DEBUG_PANIC(("GBSC: Invalid Initialisation parameters"));
    }

    /* setup our broadcast client data structure */
    memset(broadcast_client, 0, sizeof(*broadcast_client));
    broadcast_client->lib_task.handler = broadcast_client_msg_handler;
    broadcast_client->app_task = app_task;

    /* setup the GATT manager client parameter structure */
    reg_params.cid = params->cid;
    reg_params.client_task = &broadcast_client->lib_task;
    reg_params.start_handle = params->start_handle;
    reg_params.end_handle = params->end_handle;

    /* register the client with the GATT manager */
    if (GattManagerRegisterClient(&reg_params) == gatt_manager_status_success)
    {
        MessageSend(&broadcast_client->lib_task, BROADCAST_CLIENT_INTERNAL_START_FINDING_CHAR_VALUES,
                                NULL);
        retval = gatt_broadcast_client_status_success;
    }
    else
    {
        retval = gatt_broadcast_client_status_registration_failed;
    }

    BROADCAST_CLIENT_DEBUG(("BL:GATTMAN:CLIENT: Init %s\n", 
                retval == gatt_broadcast_client_status_success ?  "success" : "failed"));

    return retval;
}

/*! @brief Destruction function for the Broadcast client. */
gatt_broadcast_client_status GattBroadcastClientDestroy(GBSC* const broadcast_client)
{
    gatt_broadcast_client_status retval = gatt_broadcast_client_status_invalid_parameter;

    if(NULL == broadcast_client)
    {
        GATT_BROADCAST_CLIENT_DEBUG_PANIC(("GBSC: Null instance"));
    }

    if (GattManagerUnregisterClient(&broadcast_client->lib_task) == gatt_manager_status_success)
    {
        retval = gatt_broadcast_client_status_success;
    }
    else
    {
        retval = gatt_broadcast_client_status_destruction_failed;
    }

    MessageFlushTask(&broadcast_client->lib_task);

    /* free any stream service record data that may have been allocated
       it may also not exist yet, if we're in the middle of an association. */
    if (broadcast_client->assoc_data.stream_service_records)
    {
        free(broadcast_client->assoc_data.stream_service_records);
    }

    BROADCAST_CLIENT_DEBUG(("BL:GATTMAN:CLIENT: Destroy %s\n",
                retval == gatt_broadcast_client_status_success ?  "success" : "failed"));

    return retval;
}

#else

typedef int iamnotempty;

#endif /* ENABLE_BROADCAST */

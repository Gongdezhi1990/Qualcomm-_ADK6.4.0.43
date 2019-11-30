/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */
/*!
  @file broadcast_util.c
  @brief Implementation of utility functions used by the broadcast library.

  Currently only support the single instance of the library.

  This will need expanding when this library fully supports multiple
  instances. See reference to "SINGLE".
 */

#include "broadcast_private.h"
#include "broadcast_msg_client.h"

#include <stdlib.h>
#include <message.h>

/*! @brief Find the BROADCAST instance for a given library task. */
BROADCAST* broadcast_identify_instance(Task task)
{
    BROADCAST* instance = lib_core->lib_instances_head;

    while (instance)
    {
        if (task == &instance->lib_task)
            return instance;
        instance = instance->next_instance;
    }

    return NULL;
}

/*! @brief Validate that a BROADCAST library instance reference is valid */
bool broadcast_is_valid_instance(BROADCAST* broadcast)
{
    BROADCAST* instance = lib_core->lib_instances_head;

    while (instance)
    {
        if (broadcast == instance)
            return TRUE;

        instance = instance->next_instance;
    }

    return FALSE;
}

/*! @brief Free the memory associated with a broadcast library instance.
     - unlink the instance from the lib_core linked list
     - cancel any pending messages on the instance message queue
     - free any associated memory and the instance structure itself
     - inform the client task, i.e. the application
 * */
void broadcast_destroy_lib(BROADCAST* broadcast)
{
    Task client_task = broadcast->client_task;
    BROADCAST* old_instance = NULL;

    /* SINGLE
     * Take advantage of only a single instance to simplify list
     * deletion. If we support multiple concurrent instances in
     * future this will need updating to walk the list. */
    old_instance = lib_core->lib_instances_head;
    lib_core->lib_instances_head = NULL;
    /* SINGLE */

    MessageFlushTask(&old_instance->lib_task);

    /* ensure dynamic state in a receiver instance is cleaned up */
    if (IS_RECEIVER_ROLE(broadcast))
    {
        /* and free any stream service records we may have from past
         * associations */
        if (broadcast->config.receiver.assoc_data.stream_service_records)
        {
            free(broadcast->config.receiver.assoc_data.stream_service_records);
        }
    }
    /* can now free the instance itself */
    free(old_instance);

    ConnectionCsbRegisterTask(NULL);

    broadcast_msg_client_destroy_cfm(client_task, broadcast_success);
}


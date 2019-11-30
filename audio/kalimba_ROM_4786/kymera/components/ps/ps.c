/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file ps.c
 * \ingroup ps
 *
 */

/****************************************************************************
Include Files
*/
#include "ps.h"
#include "ps_msg.h"
#include "sched_oxygen/sched_oxygen.h"
#include "pmalloc/pl_malloc.h"
#include "string.h"

#ifdef INSTALL_PS_FOR_ACCMD
#define TASK_ID PS_SAR_TASK_QUEUE_ID
#else
#define TASK_ID PS_MSG_TASK_QUEUE_ID
#endif

/****************************************************************************
Public Function Definitions
*/


/****************************************************************************
 *
 * ps_entry_read
 *
 */
void ps_entry_read(void* instance_data, PS_KEY_TYPE key, PERSISTENCE_RANK rank, PS_READ_CALLBACK cback)
{
    tRoutingInfo routing;
    PS_MSG_DATA* ps_msg;

    /* Build the routing info, destination is not used in this case. */
    routing.src_id = (unsigned)(uintptr_t)instance_data;
    routing.dest_id = 0;

    /* Construct message to PS handler */
    ps_msg = xpnew(PS_MSG_DATA);

    if(ps_msg != NULL)
    {
        /* there is no PS data in this message */
        ps_msg->data_length = 0;
        ps_msg->callback = (void*)cback;
        ps_msg->key = key;
        ps_msg->rank = rank;

        /* drop message on the platform-specific PS handler queue and hope for the best */
        put_message_with_routing(TASK_ID, PS_READ_REQ, (void*)ps_msg, &routing);
    }
    else
    {
        /* callback with error */
        ((PS_READ_CALLBACK)cback)(instance_data, key, rank, 0, NULL, STATUS_CMD_FAILED, 0);
    }
}

/****************************************************************************
 *
 * ps_entry_write
 *
 */
void ps_entry_write(void* instance_data, PS_KEY_TYPE key, PERSISTENCE_RANK rank, uint16 length,
                     uint16* data, PS_WRITE_CALLBACK cback)
{
    tRoutingInfo routing;
    PS_MSG_DATA* ps_msg;

    /* Build the routing info, destination is not used in this case. */
    routing.src_id = (unsigned)(uintptr_t)instance_data;
    routing.dest_id = 0;

    /* Construct message to PS handler, decoupling data from the caller */
    ps_msg = xpmalloc(sizeof(PS_MSG_DATA) + length*sizeof(uint16));

    if(ps_msg != NULL)
    {
        ps_msg->data_length = length;
        ps_msg->callback = (void*)cback;
        ps_msg->key = key;
        ps_msg->rank = rank;

        memcpy(ps_msg->data, data, length*sizeof(uint16));

        /* drop message on the platform-specific PS handler queue and hope for the best */
        put_message_with_routing(TASK_ID, PS_WRITE_REQ, (void*)ps_msg, &routing);
    }
    else
    {
        /* callback with error */
        ((PS_WRITE_CALLBACK)cback)(instance_data, key, rank, STATUS_CMD_FAILED, 0);
    }
}

/****************************************************************************
 *
 * ps_entry_delete
 *
 */
void ps_entry_delete(void* instance_data, PS_KEY_TYPE key, PS_ENTRY_DELETE_CALLBACK cback)
{
    tRoutingInfo routing;
    PS_MSG_DATA* ps_msg;

    /* Build the routing info, destination is not used in this case. */
    routing.src_id = (unsigned)(uintptr_t)instance_data;
    routing.dest_id = 0;

    /* Construct message to PS handler */
    ps_msg = xpnew(PS_MSG_DATA);

    if(ps_msg != NULL)
    {
        /* there is no PS data in this message */
        ps_msg->data_length = 0;
        ps_msg->callback = (void*)cback;
        ps_msg->key = key;
        ps_msg->rank = PERSIST_ANY;

        /* drop message on the platform-specific PS handler queue */
        put_message_with_routing(TASK_ID, PS_ENTRY_DELETE_REQ, (void*)ps_msg, &routing);
    }
    else
    {
        /* callback with error */
        ((PS_ENTRY_DELETE_CALLBACK)cback)(instance_data, key, STATUS_CMD_FAILED, 0);
    }
}

/****************************************************************************
 *
 * ps_delete
 *
 */
void ps_delete(void* instance_data, PERSISTENCE_RANK rank, PS_DELETE_CALLBACK cback)
{
    tRoutingInfo routing;
    PS_MSG_DATA* ps_msg;

    /* Build the routing info, destination is not used in this case. */
    routing.src_id = (unsigned)(uintptr_t)instance_data;
    routing.dest_id = 0;

    /* Construct message to PS handler */
    ps_msg = xpnew(PS_MSG_DATA);

    if(ps_msg != NULL)
    {
        /* there is no PS data in this message */
        ps_msg->data_length = 0;
        ps_msg->callback = (void*)cback;
        ps_msg->key = 0;
        ps_msg->rank = rank;

        /* drop message on the platform-specific PS handler queue */
        put_message_with_routing(TASK_ID, PS_DELETE_REQ, (void*)ps_msg, &routing);
    }
    else
    {
        /* callback with error */
        ((PS_DELETE_CALLBACK)cback)(instance_data, rank, STATUS_CMD_FAILED, 0);
    }
}

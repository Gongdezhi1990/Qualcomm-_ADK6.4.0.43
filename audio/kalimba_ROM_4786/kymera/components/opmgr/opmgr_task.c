/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  opmgr_task.c
 * \ingroup opmgr
 *
 * Operator Manager task file. <br>
 * This file contains the operator manager task state and tracks which requests
 * a response is due from.<br>
 */

/****************************************************************************
Include Files
*/
#include "opmgr_private.h"

/****************************************************************************
Private type definitions
*/
typedef struct CURRENT_TASK
{
    /** The id of the client that made the request */
    unsigned client_id;
    /** The id of the operator the request was made of */
    unsigned op_id;
    /** Pointer to data required for deciphering the response of this request */
    void *data;
    /** Pointer to the next in progress operator message being serviced */
    struct CURRENT_TASK *next;
} OPMGR_CURRENT_TASK;

/****************************************************************************
Private variable definitions
*/
static OPMGR_CURRENT_TASK *first_active_req = NULL;

/****************************************************************************
Public function definitions
*/

bool opmgr_store_in_progress_task(unsigned client_id, unsigned op_id, void *data)
{
    OPMGR_CURRENT_TASK **element = &first_active_req;
    OPMGR_CURRENT_TASK *new_task = xpnew(OPMGR_CURRENT_TASK);

    if (NULL == new_task)
    {
        return FALSE;
    }

    new_task->client_id = client_id;
    new_task->op_id = op_id;
    new_task->data = data;
    new_task->next = NULL;

    /* Put the new task on the end of the list of active requests. */
    while (*element != NULL)
    {
        element = &((*element)->next);
    }
    *element = new_task;

    return TRUE;
}

void *opmgr_retrieve_in_progress_task(unsigned client_id, unsigned op_id)
{
    OPMGR_CURRENT_TASK **element = &first_active_req;

    /* Find the task and remove it from the list if we find it. If we find it we
     * return the callback that was provided with this request. If it can't be
     * found we return a NULL pointer and the caller decides what this means. */
    while (*element != NULL)
    {
        if ((client_id == (*element)->client_id) && (op_id == (*element)->op_id))
        {
            OPMGR_CURRENT_TASK *this_task;
            void *data;
            this_task = *element;

            /* This should always be called from the background so no need to
             * block interrupts. */
            *element = this_task->next;
            data = this_task->data;
            pfree(this_task);

            return data;
        }
        element = &((*element)->next);
    }
    return NULL;
}


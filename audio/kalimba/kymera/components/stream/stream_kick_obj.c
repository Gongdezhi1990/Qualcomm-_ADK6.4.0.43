/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_kick_obj.c
 * \ingroup stream
 *
 * This file contains functions for the kick object. <br>
 *
 */

/****************************************************************************
Include Files
*/

#include "stream_private.h"
#include "stream_kick_obj.h"
#include "platform/profiler_c.h"

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Macro Declarations
*/

/****************************************************************************
Private Variable Definitions
*/
static KICK_OBJECT *kick_object_list = NULL;

/****************************************************************************
Private Function Declarations
*/

/*
 * kick_obj_create
 */
KICK_OBJECT *kick_obj_create(ENDPOINT *sched_ep, ENDPOINT *head_ep)
{
    KICK_OBJECT *ko;

    ko = pnew(KICK_OBJECT);
    ko->sched_ep = sched_ep;
    ko->kick_ep = head_ep;
    ko->next = kick_object_list;
    kick_object_list = ko;
    return ko;
}

/*
 * kick_obj_destroy
 */
void kick_obj_destroy(KICK_OBJECT *ko)
{
    /* There isn't much complexity associated with a kick object so all we need
     * to do is free the memory that was allocated and update the kick_object_list
     */
    KICK_OBJECT **element = &kick_object_list;
    for ( ; (*element) != NULL; element = &((*element)->next))
    {
        if (*element == ko)
        {
            *element = ko->next;
            pdelete(ko);
            return;
        }
    }
    /* TODO should this panic here? */
}


/*
 * kick_obj_kick
 */
void kick_obj_kick(void *kick_object)
{
    KICK_OBJECT *ko = kick_object;
    /* There are two stages, first do any work needed to reschedule the chain,
     * then kick off the audio processing for the chain.
     *
     * NOTE: This function occurs at interrupt.
     */
    ko->sched_ep->functions->sched_kick(ko->sched_ep, ko);

    if (ko->kick_ep->deferred.kick_is_deferred && is_current_context_interrupt())
    {
        ko->kick_ep->deferred.interrupt_handled_time = hal_get_time();
        ko->kick_ep->deferred.kick_dir = STREAM_KICK_INTERNAL;
        raise_bg_int(ko->kick_ep->deferred.bg_task);
    }
    else
    {
        ko->kick_ep->functions->kick(ko->kick_ep, STREAM_KICK_INTERNAL);
    }
}


/*
 * kick_obj_from_sched_endpoint
 */
KICK_OBJECT *kick_obj_from_sched_endpoint(ENDPOINT *ep)
{
    KICK_OBJECT *ko;
       
    for(ko = kick_object_list; ko != NULL; ko = ko->next)
    {
        if(ko->sched_ep == ep)
        {
            break;
        }
    }

    return ko;
}

/*
 * kick_set_sched_ep
 */
void kick_set_sched_ep(KICK_OBJECT *ko, ENDPOINT *ep)
{
    ko->sched_ep = ep;
}

/*
 * kick_get_sched_ep
 */
ENDPOINT *kick_get_sched_ep(KICK_OBJECT *ko)
{
    return ko->sched_ep;
}

/*
 * kick_set_kick_ep
 */
void kick_set_kick_ep(KICK_OBJECT *ko, ENDPOINT *ep)
{
    ko->kick_ep = ep;
}

/*
 * kick_get_kick_ep
 */
ENDPOINT *kick_get_kick_ep(KICK_OBJECT *ko)
{
    return ko->kick_ep;
}


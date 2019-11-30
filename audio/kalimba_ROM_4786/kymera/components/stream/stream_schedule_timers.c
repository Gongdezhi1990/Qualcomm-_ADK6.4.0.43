/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_schedule_timers.c
 * \ingroup stream
 *
 * stream timer scheduler. <br>
 * This file contains stream functions for setting up transform
 * timers. <br>
 *
 * \section sec1 Contains:
 *  <br>
 */

/****************************************************************************
Include Files
*/

#include "stream_private.h"
#include "platform/pl_trace.h"

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

/****************************************************************************
Functions
*/

/****************************************************************************
Private Function Definitions
*/

/****************************************************************************
Public Function Definitions
*/

/****************************************************************************
 *
 * stream_enable_endpoint
 *
 */
void stream_enable_endpoint(ENDPOINT *ep)
{
    ENDPOINT *connected = ep->connected_to;
    ENDPOINT *head_of_sync_ep;
    /* Check that the transform exists, it is a valid case for an operator
       to signal a start to an unconnected operator terminal */
    if (connected != NULL)
    {
        KICK_OBJECT *ko;
        connected->is_enabled = TRUE;
        /* Previously, this was specially handled for audio & file with knowledge
         * that they could be synchronised and should only be started when all
         * synchronised endpoints are connected and only the endpoint scheduling
         * the kick needs to be started. But the endpoints should take care of
         * that. As long as a (group of) endpoint(s) have a kick object, all the
         * endpoints in the group should be started.
         */
        head_of_sync_ep = stream_get_head_of_sync(connected);
        ko = kick_obj_from_sched_endpoint(head_of_sync_ep);
        if (ko != NULL)
        {
            connected->functions->start(connected, ko);
        }
    }

}

/****************************************************************************
 *
 * stream_disable_endpoint
 *
 */
void stream_disable_endpoint(ENDPOINT *ep)
{
    ENDPOINT *connected = ep->connected_to;
    /* Check that the transform exists, it is a valid case for an operator
       to signal a stop to an unconnected operator */
    if (connected != NULL)
    {
        connected->functions->stop(connected);
        connected->is_enabled = FALSE;
    }
}

/*
 * set_timing_information_for_real_source
 */
void set_timing_information_for_real_source(ENDPOINT *source_ep)
{
    ENDPOINT_TIMING_INFORMATION src_time, sink_time;
    KICK_OBJECT *ko;
    unsigned kick_period;
    unsigned int final_block_size;
    ENDPOINT *sched_ep = source_ep; /* assume the head for starters */
    ENDPOINT *old_sched_ep;
    bool restart_ep = FALSE;

    /* Software patchpoint just in case */
    patch_fn_shared(stream_schedule_timers);

    PL_PRINT_P1(TR_STREAM, "set_timing_information_for_real_source: ep 0x%x \n",
                                    stream_external_id_from_endpoint(source_ep));

    /* Make sure the endpoint looks sensible */
    if (!source_ep || !source_ep->is_real)
    {
        panic(PANIC_AUDIO_INVALID_ENDPOINT_IN_SCHEDULE_CONTEXT);
    }

    /* I case of an audio endpoint timing information is only needed for the head of sync. */
    if (IS_AUDIO_ENDPOINT(source_ep) && !IS_ENDPOINT_HEAD_OF_SYNC(source_ep))
    {
        return;
    }

    /* Stage 1: find out the source kick period */
    source_ep->functions->get_timing_info(source_ep, &src_time);
    ko = kick_obj_from_sched_endpoint(source_ep);
    if (ko == NULL)
    {
        /* Start by assuming that this endpoint is both responsible for
         * scheduling and the thing that gets kicked.
         */
        ko = kick_obj_create(source_ep, source_ep);
    }

    sink_time.block_size = 0;

    if (source_ep->connected_to)
    {
        source_ep->connected_to->functions->get_timing_info(source_ep->connected_to, &sink_time);
    }

    /* If the real source endpoint has no inherent block size, try the connected sink */
    if (src_time.block_size == 1)
    {
        if (sink_time.block_size > 1)
        {
            final_block_size = sink_time.block_size;
        }
        else
        {
            if (0 == src_time.period)
            {
                /* Neither end has an inherent block size or useful period, so
                 * make one up. We choose something largish so that if the
                 * data rate is high interrupts aren't firing to often. */
                final_block_size = 60;
            }
            else
            {
                /* A block size based on the default kick period. */
                final_block_size = (unsigned)STREAM_KICK_PERIOD_FROM_USECS(stream_if_get_system_kick_period())/src_time.period;
            }
        }
    }
    else
    {
        final_block_size = src_time.block_size;
    }

    /* Get the kick period from the endpoint, if it has one */
    if (src_time.period != 0)
    {
        kick_period = src_time.period;
    }
    else
    {
        /* For now this is not a valid case so we will set it up with some defaults */
        kick_period = (unsigned)STREAM_KICK_PERIOD_FROM_USECS(stream_if_get_system_kick_period());
    }

    /* Before changing anything stop the chain if it is running. Remembering
     * that it was running so it can be restarted once the calculations are
     * complete. */

    old_sched_ep = kick_get_sched_ep(ko);
    if (old_sched_ep->functions->stop(old_sched_ep))
    {
        restart_ep = TRUE;
    }

    if ((source_ep->connected_to) && (src_time.has_deadline))
    {
        /* Tell the operator that it should ignore any other kicks */
        source_ep->connected_to->functions->configure(source_ep->connected_to,
                                                EP_KICKED_FROM_ENDPOINT, TRUE);
    }


    /* Another software patchpoint, just in case */
    patch_fn_shared(stream_schedule_timers);

    /* Store the timing settings in the scheduling endpoint, if it cares */
    sched_ep->functions->configure(sched_ep, EP_BLOCK_SIZE, final_block_size);
    sched_ep->functions->configure(sched_ep, EP_KICK_PERIOD, kick_period);

    /* Stage 4: If we were running then we want to redo the timers as something in the chain might have just changed */
    if (restart_ep)
    {
        sched_ep->functions->start(sched_ep, ko);
    }
}


/*
 * set_timing_information_for_real_sink
 */
void set_timing_information_for_real_sink(ENDPOINT *sink_ep)
{
    ENDPOINT_TIMING_INFORMATION src_time, sink_time;
    KICK_OBJECT *ko;
    unsigned kick_period;
    unsigned int final_block_size;
    ENDPOINT *sched_ep = sink_ep; /* assume the head for starters */
    ENDPOINT *old_sched_ep;
    bool restart_ep = FALSE;

    /* Software patchpoint just in case */
    patch_fn_shared(stream_schedule_timers);

    PL_PRINT_P1(TR_STREAM, "set_timing_information_for_real_sink: ep 0x%x \n",
                                    stream_external_id_from_endpoint(sink_ep));

    /* Make sure the endpoint looks sensible */
    if (!sink_ep || !sink_ep->is_real)
    {
        panic(PANIC_AUDIO_INVALID_ENDPOINT_IN_SCHEDULE_CONTEXT);
    }

    /* I case of an audio endpoint timing information is only needed for the head of sync. */
    if (IS_AUDIO_ENDPOINT(sink_ep) && !IS_ENDPOINT_HEAD_OF_SYNC(sink_ep))
    {
        return;
    }

    /* Stage 1: find out the sink kick period */
    sink_ep->functions->get_timing_info(sink_ep, &sink_time);
    ko = kick_obj_from_sched_endpoint(sink_ep);

    if (ko == NULL)
    {
        /* Start by assuming that this endpoint is both responsible for
         * scheduling and the thing that gets kicked.
         */
        ko = kick_obj_create(sink_ep, sink_ep);
    }

    src_time.block_size = 0;

    if (sink_ep->connected_to)
    {
        sink_ep->connected_to->functions->get_timing_info(sink_ep->connected_to, &src_time);
    }

    /* If the real sink endpoint has no inherent block size, try the connected source */
    if (sink_time.block_size == 1)
    {
        if (src_time.block_size > 1)
        {
            final_block_size = src_time.block_size;
        }
        else
        {
            if (0 == sink_time.period)
            {
                /* Neither end has an inherent block size or useful period, so
                 * make one up. We choose something largish so that if the
                 * data rate is high interrupts aren't firing to often. */
                final_block_size = 60;
            }
            else
            {
                /* A block size based on the default kick period. */
                final_block_size = (unsigned)STREAM_KICK_PERIOD_FROM_USECS(stream_if_get_system_kick_period())/sink_time.period;
            }
        }
    }
    else
    {
        final_block_size = sink_time.block_size;
    }

    /* Get the kick period from the endpoint, if it has one */
    if (sink_time.period != 0)
    {
        kick_period = sink_time.period;
    }
    else
    {
        /* For now this is not a valid case so we will set it up with some defaults */
        kick_period = (unsigned)STREAM_KICK_PERIOD_FROM_USECS(stream_if_get_system_kick_period());
    }

    /* Before changing anything stop the chain if it is running. Remembering
     * that it was running so it can be restarted once the calculations are
     * complete. */

    old_sched_ep = kick_get_sched_ep(ko);
    if (old_sched_ep->functions->stop(old_sched_ep))
    {
        restart_ep = TRUE;
    }

    if ((sink_ep->connected_to) && (sink_time.has_deadline))
    {
        /* For hard-deadline sinks, what we kick is the upstream operator */
        kick_set_kick_ep(ko, sink_ep->connected_to);
        /* Store the processing time so we know how far ahead to kick */
        sched_ep->functions->configure(sched_ep, EP_PROC_TIME,
            opmgr_get_processing_time(stream_external_id_from_endpoint(sink_ep->connected_to), final_block_size));
        /* And tell the operator that it should ignore any other kicks */
        sink_ep->connected_to->functions->configure(sink_ep->connected_to, EP_KICKED_FROM_ENDPOINT, TRUE);
    }

    /* Another software patchpoint, just in case */
    patch_fn_shared(stream_schedule_timers);

    /* Store the timing settings and block size in the scheduling endpoint, if it cares */
    sched_ep->functions->configure(sched_ep, EP_BLOCK_SIZE, final_block_size);
    sched_ep->functions->configure(sched_ep, EP_KICK_PERIOD, kick_period);

    /* Stage 4: If we were running then we want to redo the timers as something in the chain might have just changed */
    if (restart_ep)
    {
        sched_ep->functions->start(sched_ep, ko);
    }
}


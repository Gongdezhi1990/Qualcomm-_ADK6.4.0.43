/****************************************************************************
 * Copyright (c) 2012 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream.c
 * \ingroup stream
 *
 * stream main file. <br>
 * This file contains interfaces to streams for unit tests <br>
 *
 * \section sec1 Contains:
 * stream_ <br>
 */

/****************************************************************************
Include Files
*/

#include "stream_private.h"

/* This file is only used for unit test builds to allow visibility into the
   stream subsystem */
#ifdef UNIT_TEST_BUILD

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
Private Function Declarations
*/


/****************************************************************************
Public Function Definitions
*/


/****************************************************************************
 *
 * get_timing_information_from_transform
 *
 */

/* This is an internal only function and doesn't go into the final app so it
*  shoudn't matter that this is a bit yucky!!! This is written this way so not to
*  touch any of the rest of the stream system so to insulate the stream subsystem
*  from the unit tests.
*/
bool get_timing_information_from_transform(unsigned tid, unsigned *period, unsigned *hard_endpoint, unsigned *proc_time)
{
    ENDPOINT *sched;
    KICK_OBJECT *ko;
    TRANSFORM *transform = stream_transform_from_external_id(tid);

    if (!transform)
    {
        return FALSE;
    }

    ko = kick_obj_from_sched_endpoint(transform->source);
    sched = kick_get_sched_ep(ko);

    /* to find where the timing information is stored, lets look at the endpoint types */
    switch (sched->stream_endpoint_type)
    {
        case endpoint_audio:
#ifdef CHIP_BASE_BC7
            *period = STREAM_KICK_PERIOD_TO_USECS(sched->state.audio.kick_period);
#else
            *period = 0;
#endif
            *proc_time = 0;
            *hard_endpoint = 0;
            return TRUE;
        case endpoint_operator:
        default:
            /* these don't have timing information something has gone rather wrong! */
            PL_PRINT_P1(TR_PL_TEST_TRACE,"The endpoint %x responsible for scheduling doesn't possess timing information!\n", sched->id);
    }
    PL_PRINT_P0(TR_PL_TEST_TRACE,"an as of yet unsupported endpoint has been used, get_timing_information_from_transform in stream_unit_test.c needs to be updated\n\n");
    return FALSE;
}


/****************************************************************************
Private Function Definitions
*/

#endif /*UNIT_TEST_BUILD*/


/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_measure_metadata.c
 * \ingroup rate_lib
 *
 */

#include "rate_private.h"

#ifdef INSTALL_METADATA
/****************************************************************************
 * Public Function Implementations
 */

/** Check whether the difference between the new and last timestamp
 * points to a rate outside the accepted range
 */
bool rate_measure_metadata_check_last_rate(const RATE_MEASURE_METADATA* rmm, unsigned samples_before, TIME ts)
{
    unsigned num_samples;
    TIME elapsed_usec;

    /* Wrapping subtraction, order of events is known */
    elapsed_usec = ts - rmm->core.last_timestamp;

    /* From the number of samples, get the limits for elapsed time */
    num_samples = rmm->core.last_num_samples + samples_before;

    return rate_measure_check_rate(rmm->core.sample_period, num_samples, elapsed_usec);
}

/** Update measurement context: Check the passed list
 * of tags for discontinuities. If there are none and the first
 * tag has a timestamp (TTP or ToA), record it as "last".
 *
 * When the primary stream calls this, there may be more than one
 * rate measurement context in a linked list (one for each rate
 * adjusting sink group.) Some operations are then duplicated.
 * I.e. this does not scale optimally to multiple adjusting sink
 * groups. That is not an important case (and amount of cycles)
 * right now.
 */
void rate_measure_metadata_record_tags(RATE_MEASURE_METADATA* rmm, unsigned num_samples, unsigned octets_before, unsigned octets_after, const metadata_tag* tags)
{
    patch_fn_shared(rate_lib);

    if (((octets_before % OCTETS_PER_SAMPLE) != 0)
        || ((octets_after % OCTETS_PER_SAMPLE) != 0))
    {
        while (rmm != NULL)
        {
            /* Very misaligned -- don't attempt to track */
            rate_measure_stop(&rmm->core);
            rmm->remaining_octets = 0;
            rmm = rmm->next;
        }
    }
    else if (tags == NULL)
    {
        /* Check transport -- if there is a gap, stop tracking */
        unsigned octets = num_samples * OCTETS_PER_SAMPLE;
        while (rmm != NULL)
        {
            if (octets <= rmm->remaining_octets)
            {
                rmm->remaining_octets -= octets;
            }
            else
            {
                RATE_MSG3("rate_measure_metadata_record_tags %p no tag ns %d remaining_o %d gap -> tracking invalid",
                          rmm, num_samples, rmm->remaining_octets);
                rate_measure_stop(&rmm->core);
                rmm->remaining_octets = 0;
            }
            rmm = rmm->next;
        }
    }
    else
    {
        RATE_TIMESTAMP_TYPE tt = rate_metadata_get_timestamp_type(tags);
        unsigned samples_before = octets_before / OCTETS_PER_SAMPLE;

        while (rmm != NULL)
        {
            if (rmm->remaining_octets != octets_before)
            {
                /* The start is misaligned, try to restart */
                RATE_MSG3("rate_measure_metadata_record_tags %p tag remaining_o %d b4_o %d misaligned -> tracking invalid",
                          rmm, rmm->remaining_octets, octets_before);
                rate_measure_stop(&rmm->core);
            }

            if (! rate_measure_valid(&rmm->core))
            {
                if (tt != RATE_TIMESTAMP_TYPE_NONE)
                {
                    /* Start */
                    rmm->base_type = tt;
                    rate_measure_start(&rmm->core, num_samples - samples_before, tags->timestamp);
                    RATE_MSG4("rate_measure_metadata_record_tags %p start type %d base_ns %d base_t %d",
                              rmm, tt, rmm->core.base_num_samples, rmm->core.base_timestamp);
                }
            }
            else
            {
                if ((tt == rmm->base_type)
                    && rate_measure_metadata_check_last_rate(rmm, samples_before, tags->timestamp))
                {
                    /* Continue */
                    rate_measure_continue(&rmm->core, num_samples, tags->timestamp, num_samples - samples_before);
                }
                else
                {
                    RATE_MSG1("rate_measure_metadata_record_tags %p discontinuous", rmm);
                    rate_measure_stop(&rmm->core);
                }
            }

            const metadata_tag* look = tags;
            while (look->next != NULL)
            {
                look = look->next;

                if (rate_measure_valid(&rmm->core)
                    && (rate_metadata_get_timestamp_type(look) != rmm->base_type))
                {
                    RATE_MSG2("rate_measure_metadata_record_tags %p tag flags 0x%02x type change", rmm, look->flags);
                    rate_measure_stop(&rmm->core);
                }
            }

            /* look now points to the last received tag */
            rmm->remaining_octets = look->length - octets_after;

            rmm = rmm->next;
        }
    }
}

#endif /* INSTALL_METADATA */

/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_measure_metadata.h
 * \ingroup rate_lib
 *
 */

#ifndef RATE_RATE_MEASURE_METADATA_H
#define RATE_RATE_MEASURE_METADATA_H

#include "rate_types.h"
#include "rate_measure.h"
#include "buffer/cbuffer_c.h"

#ifdef INSTALL_METADATA

/****************************************************************************
 * Public Type Definitions
 */

/** Extension of RATE_MEASURE to keep track of metadata tag types
 * and transport. Also allow for a list of these contexts to be updated
 * in a single call to rate_measure_metadata_record_tags
 */
typedef struct rate_measure_metadata
{
    RATE_MEASURE                    core;

    /** Type of timestamp being followed */
    RATE_TIMESTAMP_TYPE             base_type;

    /** Octets overhanging from last tag of previous buffer */
    unsigned                        remaining_octets;

    /** Linked list of measurement contexts for the primary rate */
    struct rate_measure_metadata    *next;
} RATE_MEASURE_METADATA;

/****************************************************************************
 * Public Function Declarations
 */

/**
 * Check that values in a new tag and the last recorded tag imply a rate
 * which is within reasonable distance of the nominal rate.
 * \param rmm Context for rate measurement with metadata
 * \param samples_before Number of samples from the start of the current buffer
 *                       to the tag with the new timestamp
 * \param ts Timestamp of the new tag
 */
extern bool rate_measure_metadata_check_last_rate(
        const RATE_MEASURE_METADATA* rmm, unsigned samples_before, TIME ts);

/**
 * Process a list of metadata tags.
 * If there are transport inconsistencies (gaps/overlaps), out-of-range
 * values or the tag type doesn't match the type being tracked, stop
 * tracking.
 * Either start or continue tracking using the first tag of the list.
 * \param rmm Context for rate measurement with metadata
 * \param num_samples Number of samples spanned by the tags
 * \param octets_before Offset of the first tag from the start of
 *                      the current buffer
 * \param octets_after  Offset of the last tag from the end of the
 *                      current buffer
 * \param tags List of metadata tags (can be NULL)
 */
extern void rate_measure_metadata_record_tags(
        RATE_MEASURE_METADATA* rmm, unsigned num_samples, unsigned octets_before,
        unsigned octets_after, const metadata_tag* tags);

#endif /* INSTALL_METADATA */

#endif /* RATE_RATE_MEASURE_METADATA_H */

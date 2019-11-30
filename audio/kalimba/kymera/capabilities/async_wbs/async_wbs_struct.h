/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup wbs
 * \file  async_wbs_struct.h
 * \ingroup capabilities
 *
 * Wide Band Speech type header. <br>
 *
 */

#ifndef ASYNC_WBS_TYPES_H
#define ASYNC_WBS_TYPES_H
/*****************************************************************************
Include Files
*/
#include "wbs_struct.h"

typedef struct ASYNC_WBS_ENC_SPECIFIC_DATA
{
    /** The input buffer with metadata to transport from */
    tCbuffer *metadata_ip_buffer;

    /** The input sample rate */
    unsigned sample_rate;

    /** Timestamp from the last metadata tag processed */
    TIME last_tag_timestamp;

    /** Sample period adjustment value from the last metadata tag */
    unsigned last_tag_spa;

    /** Samples read since the last metadata tag */
    unsigned last_tag_samples;

    /* Error offset ID from last tag */
    unsigned last_tag_err_offset_id;

    /** Encode / Decode algorithmic delay, in samples */
    unsigned delay_samples;

    /** Number of frames that can fit in bluetooth packet */
    unsigned frames_per_packet;
} ASYNC_WBS_ENC_SPECIFIC_DATA;

/** capability-specific extra operator data for ASYNC WBS ENC
 *  extends WBS_ENC_OP_DATA */
typedef struct ASYNC_WBS_ENC_OP_DATA
{
    /** WBS common structure */
    WBS_ENC_OP_DATA wbs;

    /** ASYNC_WBS specific data */
    ASYNC_WBS_ENC_SPECIFIC_DATA aswbs;
} ASYNC_WBS_ENC_OP_DATA;

typedef enum ASYNC_WBS_DEC_INIT_PHASE
{
    AWBSD_NOT_INITIALIZED,
    AWBSD_IGNORE_FIRST_KICK,
    AWBSD_RUN,
}ASYNC_WBS_DEC_INIT_PHASE;

/** capability-specific extra operator data for ASYNC WBS DEC
 *  extends WBS_DEC_OP_DATA */
typedef struct ASYNC_WBS_DEC_OP_DATA
{
    /** WBS common structure */
    WBS_DEC_OP_DATA wbs;

    /** ASYNC_WBS specific data */
    ASYNC_WBS_DEC_INIT_PHASE init_phase;
} ASYNC_WBS_DEC_OP_DATA;


#endif /* ASYNC_WBS_TYPES_H */

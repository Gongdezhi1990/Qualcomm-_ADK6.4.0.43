/****************************************************************************
//  Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 
****************************************************************************/

/*!
    @file erasure_code_input_stats.h
    @brief Erasure code input statistics.
*/

#ifndef EC_INPUT_STATS_H
#define EC_INPUT_STATS_H

#include <erasure_code_common.h>

typedef struct _stream_id_count
{
    unsigned int stream_id;
    unsigned int count;
} stream_id_count_t;

typedef struct ec_input_stats
{
    /* Stores the pairs of packets that are decoded */
    unsigned int decoded_pairs[EC_2_5_PAIR_COMBINATIONS];
    /* Stores the number of EC decode failures */
    unsigned int decode_failures;
    /* Stores the stream IDs received */
    stream_id_count_t stream_id_counts[EC_STREAM_ID_COUNT_TABLE_SIZE];
} ec_input_stats_t;

#endif

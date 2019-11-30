/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_endpoint_a2dp.h
 * \ingroup stream
 */
#ifndef STREAM_ENDPOINT_A2DP_H
#define STREAM_ENDPOINT_A2DP_H
#include "buffer.h"
#include "stream.h"
#include "stream_endpoint.h"

/** The minimum buffer size that this endpoint needs to be connected to */
#ifdef TIMED_PLAYBACK_MODE
/* No need for the jitter buffer when the timed playback mode is used. In this case,
 * the latency buffer is located immediately after the capability which timestamps the
 * stream (RTP decode and TTP pass are the current capabilities able to do such a thing). */
#define A2DP_BUFFER_MIN_SIZE (256)
#else
#define A2DP_BUFFER_MIN_SIZE (4000)
#endif

/** Usable octets in a native word size sample */
#define A2DP_USABLE_OCTETS_PER_SAMPLE (2)

/** The Amount of encoded data to buffer before starting to play. This is set at
 *  75% of the local buffer.
 */
#define BUFFERING_TARGET ((A2DP_BUFFER_MIN_SIZE * 3)/4)
/** The Minimum buffer level at which play continues, rather than switching back
 * to the buffering state. */
#define MINIMUM_AMOUNT (255)
/** Magnitude of the deadband to accept around the BUFFERING_TARGET. This is
 * sized to fit expected atu sizes of around 800 octets */
#define TOLERANCE_BAND (400)
/** Fixed point precision to use in the buffer level average calculations. This
 * value defines the amount of fixed point resolution. */
#define RM_A2DP_FIX_POINT_SHIFT (8)
/** The number of samples to integrate the buffer level over is 2**RM_A2DP_AVG_SHIFT.
 *  The fixed point value is integrated over "**RM_A2DP_AVG_SHIFT
 */
#define RM_A2DP_AVG_SHIFT (8)
/** The Cumulative shift achieved by applying both the fix point and averaging
 *  shifts to a value.
 */
#define RM_A2DP_NET_SHIFT ((RM_A2DP_FIX_POINT_SHIFT) + (RM_A2DP_AVG_SHIFT))

/* Rate matching constant
 * (1/(TARGET_WINDOW_MAX_LVL - TARGET_WINDOW_MIN_LVL) * (1 << RM_AVG_SHIFT))
 */

#ifdef TODO_CRESCENDO_A2DP_RM_TUNING
/** B-203812. Current A2DP RM buffering thresholds
 *  may break APTX. Have a TODO until we tune these
 *  thresholds.
 */
#define RM_CONSTANT  (0.0000048828125)
#define TARGET_WINDOW_MAX_LVL  ((BUFFERING_TARGET) + (TOLERANCE_BAND))
#define TARGET_WINDOW_MIN_LVL  ((BUFFERING_TARGET) - (TOLERANCE_BAND))
#else /* TODO_CRESCENDO_A2DP_RM_TUNING */
#define RM_CONSTANT  (0.00000138)
#define TARGET_WINDOW_MAX_LVL  ((BUFFERING_TARGET) + (TOLERANCE_BAND))
#define TARGET_WINDOW_MIN_LVL  ((MINIMUM_AMOUNT) + (TOLERANCE_BAND))
#endif /* TODO_CRESCENDO_A2DP_RM_TUNING */

#define TOTAL_BUFF_SIZE(ep) (cbuffer_get_size_in_words(ep->state.a2dp.source_buf) + cbuffer_get_size_in_words(ep->state.a2dp.sink_buf))
#ifdef A2DP_USE_OCTET_BUFFER_INTERFACE
#define TOTAL_BUFF_DATA(ep) (cbuffer_calc_amount_data_in_words(ep->state.a2dp.source_buf->buffer) + cbuffer_calc_amount_data_in_words(ep->state.a2dp.sink_buf->buffer))
#else
#define TOTAL_BUFF_DATA(ep) (cbuffer_calc_amount_data_in_words(ep->state.a2dp.source_buf) + cbuffer_calc_amount_data_in_words(ep->state.a2dp.sink_buf))
#endif

bool a2dp_configure(ENDPOINT *endpoint, unsigned key, uint32 value);
bool a2dp_buffer_details(ENDPOINT *endpoint, BUFFER_DETAILS *details);
bool a2dp_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr,  ENDPOINT *ep_to_kick, bool* start_on_connect);
bool a2dp_disconnect(ENDPOINT *endpoint);
bool a2dp_get_config(ENDPOINT *endpoint, unsigned key, ENDPOINT_GET_CONFIG_RESULT* result);

/**
 * \brief perform part of a2dp start that is common across all platforms
 * \param ep a2dp endpoint
 * \return TRUE if the endpoint is started as a result of this call,
 *  FALSE if the endpoint was started already, prior to this call
 */
bool a2dp_start_common(ENDPOINT *ep);

/**
 * \brief perform part of a2dp source endpoint kick that is common across
 *  all platforms. Implements the playing, buffering state machine and
 *  updates average buffer level, propagates kicks if needed.
 *  \param ep a2dp endpoint
 */
void a2dp_source_kick_common(ENDPOINT *ep);

/**
 * \brief perform part of a2dp get_timing that is common across all platforms
 * \param ep a2dp endpoint
 * \param time_info endpoint timing information
 */
void a2dp_get_timing_common(ENDPOINT *ep, ENDPOINT_TIMING_INFORMATION *time_info);

#endif /* STREAM_ENDPOINT_A2DP_H */


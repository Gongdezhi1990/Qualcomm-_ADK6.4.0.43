/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup Audio Audio endpoint
 * \ingroup endpoints
 * \file stream_endpoint_audio.h
 *
 */

#ifndef _STREAM_ENDPOINT_AUDIO_H_
#define _STREAM_ENDPOINT_AUDIO_H_

/****************************************************************************
Include Files
*/
#ifdef CHIP_BASE_BC7
#include "bluecore_port.h"
#endif

#include "sections.h"

/****************************************************************************
Private Constant declarations
*/
#if defined (CHIP_BASE_HYDRA) /* Hydra constants */
/** Amount of sink processing time that all the cbops can take without the headroom
 *  getting used up in chains between start and first kick.
 *  The time spent from start to first kick on any
 *  channel has to be less than the time represented by the headroom samples
 *  placed at start in the sink output buffer - otherwise by the time we get to
 *  first kick on some channel(s), that buffer wraps. Some empirical values are
 *  unavoidable, these take into account the measurements and also the
 *  algorithmic needs.
 */
#define CBOP_PROCESSING_TIME_ALLOWANCE_IN_USECS                850
#define RM_AVG_SHIFT                               7
#define RM_AVG_RESOLUTION                          20

/** On Hydra, accumulation of rm_diff needs to happen always.
 *  delta_samples being non-zero is irrelevant.
 */
#define DELTA_SAMPLES_NOT_COMPUTED                 (unsigned int)(-1)
/** Hydra audio buffer size */
#define AUDIO_BUFFER_SIZE                          128
/** A minimum amount of samples to present source endpoint cbops with */
#define CBOP_MIN_HEADROOM_SAMPLES                  17

#elif defined (CHIP_BASE_BC7) /* BC7 constants */

#define CBOP_PROCESSING_TIME_ALLOWANCE_IN_USECS                750
/** The bit resolution for fixed point arithemtic of the hardware rate. */
#define RM_FIX_POINT_SHIFT                                      20
/** The minimum number of kicks that must occur before we are prepared to
 * calculate the hardware rate.
 */
#define RM_MIN_KICKS_BEFORE_ADJUST                              50
/** The size of an audio buffer on the XAP side of a port */
#define AUDIO_BUFFER_SIZE 128
/** A minimum amount of samples to present source endpoint cbops with */
#define CBOP_MIN_HEADROOM_SAMPLES                              10


#elif defined (CHIP_BASE_A7DA_KAS) /* A7DA_KAS constants */
/** Todo: measure constant definitions for this a7da_kas, using hydra for the time being */
#define CBOP_PROCESSING_TIME_ALLOWANCE_IN_USECS                850
#define RM_AVG_SHIFT                               7
#define RM_FIX_POINT_SHIFT                         (20 - RM_AVG_SHIFT)
/** On Hydra, accumulation of rm_diff needs to happen always.
 *  delta_samples being non-zero is irrelevant.
 */
#define DELTA_SAMPLES_NOT_COMPUTED                 (unsigned int)(-1)
/** Hydra audio buffer size */
#define AUDIO_BUFFER_SIZE                          128
/** A minimum amount of samples to present source endpoint cbops with */
#define CBOP_MIN_HEADROOM_SAMPLES                  17

#elif defined (CHIP_BASE_NAPIER)
/** Todo: measure constant definitions for napier, using default for the time being */
#define CBOP_PROCESSING_TIME_ALLOWANCE_IN_USECS                850
#define RM_AVG_SHIFT                               7
#define RM_AVG_RESOLUTION                          20

/** On Hydra, accumulation of rm_diff needs to happen always.
 *  delta_samples being non-zero is irrelevant.
 */
#define DELTA_SAMPLES_NOT_COMPUTED                 (unsigned int)(-1)
/** Hydra audio buffer size */
#define AUDIO_BUFFER_SIZE                          128
/** A minimum amount of samples to present source endpoint cbops with */
#define CBOP_MIN_HEADROOM_SAMPLES                  17
#else /* Unknown */

#error "Unknown constant definitions for this platform"

#endif /* Platform constants End */

/** The sink audio endpoint buffer size when time to play is enabled. */
#define TTP_SINK_AUDIO_BUFFER_SIZE                 (TIMED_PLAYBACK_REFRAME_PERIOD * 2)


/** Max number of sync'ed endpoints, to be updated when more needed */
#define NR_MAX_SYNCED_EPS                        (CBOPS_MAX_NR_CHANNELS)

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Public Macro Declarations for accessing audio endpoint "base class" info.
*/

/* Masks for the audio endpoint key fields.
 * The key for audio encodes the hardware, instance and channel into 24
 * bits. Bits 3:0 are the channel, bits 7:4 the instance, bits 15:8 the
 * hardware type and bits 16:23 are always zero.
 */
#define AUDIO_EP_DEVICE_MASK                      0x00FF
#define AUDIO_EP_DEVICE_SHIFT                     8
#define AUDIO_EP_INSTANCE_MASK                    0x000F
#define AUDIO_EP_INSTANCE_SHIFT                   4
#define AUDIO_EP_CHANNEL_MASK                     0x000F
#define AUDIO_EP_CHANNEL_SHIFT                    0

/* Get device ID from endpoint key. There can be other such macros to extract the other parts,
 * as & when needed.
 */
#define GET_DEVICE_FROM_AUDIO_EP_KEY(key)  (((key) >> AUDIO_EP_DEVICE_SHIFT) & AUDIO_EP_DEVICE_MASK)
#define GET_INSTANCE_FROM_AUDIO_EP_KEY(key)  (((key) >> AUDIO_EP_INSTANCE_SHIFT) & AUDIO_EP_INSTANCE_MASK)
#define GET_CHANNEL_FROM_AUDIO_EP_KEY(key)  (((key) >> AUDIO_EP_CHANNEL_SHIFT) & AUDIO_EP_CHANNEL_MASK)

/** The number of samples headroom to allow in the buffer to compensate for any
 * rate missmatch variation in buffer levels before the RM system compensates */
#define AUDIO_RM_HEADROOM_AMOUNT    2

/****************************************************************************
Public Function Declarations
*/
#ifdef INSTALL_UNINTERRUPTABLE_ANC
/**
 * \brief get_anc_instance_id function
 *
 *
 * \param endpoint pointer to the endpoint
 */
ACCMD_ANC_INSTANCE get_anc_instance_id(ENDPOINT *ep);

/**
 * \brief set_anc_instance_id function
 *
 *
 * \param endpoint pointer to the endpoint
 * \param ANC instance ID to associate with the endpoint
 */
void set_anc_instance_id(ENDPOINT *ep, ACCMD_ANC_INSTANCE instance_id);

/**
 * \brief get_anc_input_path_id function
 *
 *
 * \param endpoint pointer to the endpoint
 */
ACCMD_ANC_PATH get_anc_input_path_id(ENDPOINT *ep);

/**
 * \brief set_anc_input_path_id function
 *
 *
 * \param endpoint pointer to the endpoint
 * \param ANC input ID to associate with the endpoint
 */
void set_anc_input_path_id(ENDPOINT *ep, ACCMD_ANC_PATH path_id);

#ifdef INSTALL_ANC_STICKY_ENDPOINTS
/**
 * \brief get_anc_close_pending function
 *
 *
 * \param endpoint pointer to the endpoint
 */
bool get_anc_close_pending(ENDPOINT *ep);

/**
 * \brief set_anc_close_pending function
 *
 *
 * \param endpoint pointer to the endpoint
 */
void set_anc_close_pending(ENDPOINT *ep, bool close_pending);
#endif /* INSTALL_ANC_STICKY_ENDPOINTS */

#endif /* INSTALL_UNINTERRUPTABLE_ANC */

/****************************************************************************
Private Function Declarations
*/
#if defined (CHIP_BASE_HYDRA) /* Hydra functions */
/**
 * \brief get the Hydra block size tied to the kick period in samples
 * \param audio audio state of the end point that received a kick
 */
INLINE_SECTION static inline unsigned get_block_size(endpoint_audio_state *audio)
{
    return audio->monitor_threshold;
}


/**
 * \brief get the Hydra sample rate at the end point
 * \param endpoint pointer to the endpoint that received a kick
 */
INLINE_SECTION static inline uint32 get_sample_rate(ENDPOINT *endpoint)
{
    return audio_vsm_get_sample_rate_from_sid(stream_external_id_from_endpoint(endpoint));
}

#elif defined (CHIP_BASE_BC7) /* BC7 functions */

/**
 * \brief get the BC7 block size tied to the kick period in samples
 * \param audio audio state of the end point that received a kick
 */
INLINE_SECTION static inline unsigned get_block_size(endpoint_audio_state *audio)
{
    return audio->block_size;
}


/**
 * \brief get the BC7 sample rate at the end point
 * \param endpoint pointer to the endpoint that received a kick
 */
INLINE_SECTION static inline uint32 get_sample_rate(ENDPOINT *endpoint)
{
    return endpoint->state.audio.sample_rate;
}

#elif defined (CHIP_BASE_A7DA_KAS) /* A7DA_KAS functions */
/**
 * \brief get the A7DA_KAS block size tied to the kick period in samples
 * \param audio audio state of the end point that received a kick
 */
INLINE_SECTION static inline unsigned get_block_size(endpoint_audio_state *audio)
{
    return audio->block_size;
}

/**
 * \brief get the A7DA_KAS maximum number of source samples that can be processed
 *        by the endpoint
 * \param audio audio state of the end point that received a kick
 */
INLINE_SECTION static inline unsigned get_max_data(endpoint_audio_state *audio)
{
    return audio->source_buf->size;
}

/**
 * \brief get the A7DA_KAS sample rate at the end point
 * \param endpoint pointer to the endpoint that received a kick
 */
INLINE_SECTION static inline uint32 get_sample_rate(ENDPOINT *endpoint)
{
    return endpoint->state.audio.port->sample_rate;
}

#elif defined (CHIP_BASE_NAPIER) /* NAPIER functions */
/**
 * \brief get the napier block size tied to the kick period in samples
 * \param audio audio state of the end point that received a kick
 */
INLINE_SECTION static inline unsigned get_block_size(endpoint_audio_state *audio)
{
    /* TODO_NAPIER: should this look like hydra? */
    return audio->monitor_threshold;
}

/**
 * \brief get the napier maximum number of source samples that can be processed
 *        by the endpoint
 * \param audio audio state of the end point that received a kick
 */
INLINE_SECTION static inline unsigned get_max_data(endpoint_audio_state *audio)
{
    /* TODO_NAPIER: should this look like hydra? */
    // return audio->source_buf->size;
    return 0;
}

/**
 * \brief get the napier sample rate at the end point
 * \param endpoint pointer to the endpoint that received a kick
 */
INLINE_SECTION static inline uint32 get_sample_rate(ENDPOINT *endpoint)
{
    /* TODO_NAPIER: should this look like hydra? */
    // return endpoint->state.audio.port->sample_rate;
    return 0;
}
#else /* Unknown */

#error "Undefined functions for this platform"

#endif /* Platform functions end */

/**
 * \brief audio_kick function
 *
 *
 * \param endpoint pointer to the endpoint that received a kick
 * \param kick_dir direction of the kick
 */
extern void audio_kick(ENDPOINT *endpoint, ENDPOINT_KICK_DIRECTION kick_dir);

/**
 * \brief Obtains rate measurement data
 * \param endpoint pointer to the endpoint that received a kick
 * \return delta samples from the last kick (OR) an indication
           that delta samples from the last kick wasn't computed
 */
extern unsigned get_rm_data(ENDPOINT *endpoint);

/**
 * \brief Accumulates rate measurement data
 * \param endpoint pointer to the endpoint that received a kick
 * \param num_cbops_read Count of samples consumed by endpoint's cbops
 * \param num_cbops_written Count of samples produced by endpoint's cbops
 *
 * \return delta samples from the last kick (OR) an indication
           that delta samples from the last kick wasn't computed
 */
extern void process_rm_data(ENDPOINT *endpoint,
                            unsigned num_cbops_read, unsigned num_cbops_written);

/**
 * \brief Set cbops values for an endpoint.
 * \param ep    pointer to an endpoint that owns a cbops chain
 * \param vals  pointer to structure of cbop values used by cbops mgr in chain creation
 */
extern void set_endpoint_cbops_param_vals(ENDPOINT *ep, CBOP_VALS *vals);

/**
 * \brief Obtain the audio buffer length based on the sample rate of the endpoint.
 *      The buffer must be big enough to hold samples arrived in two kick periods.
 *
 * \param sample_rate of the endpoint.
 * \param dir direction of the endpoint
 * \param get_hw_size TRUE to round up to a power of two for MMU buffers
 *
 * \return length of the required buffer
 */
unsigned int get_audio_buffer_length(uint32 sample_rate, ENDPOINT_DIRECTION dir, bool get_hw_size);

#define GET_CHANNEL_FROM_AUDIO_EP_KEY(key)        (((key) >> AUDIO_EP_CHANNEL_SHIFT) & AUDIO_EP_CHANNEL_MASK)

#ifdef INSTALL_AUDIO_SOURCE_TOA_METADATA_GENERATE
#if defined(INSTALL_TTP) && defined(INSTALL_METADATA)

/* include metadata generation */
#define AUDIO_SOURCE_GENERATE_METADATA

/* minimum tag length (in words)
 * Normally one tag should be generated for every new chunk of samples,
 * however to avoid generating tags too frequently, we make sure that
 * a tag covers a minimum number of samples. This limitation is a bit
 * relaxed for smaller buffer sizes, in a way full buffer size
 * can covey 4 min-length tags if needed, i.e:
 *
 * min_tag_len: min(AUDIO_SOURCE_TOA_MIN_TAG_LEN, buffer_size/4)
 *
 */
#define AUDIO_SOURCE_TOA_MIN_TAG_LEN (30)

/**
 * audio_generate_metadata
 * \breif generates metadata for audio source endpoints
 * \param endpoint pointer to audio endpoint structure
 * \param new_words_written amount of new words written into buffer
 */
extern void audio_generate_metadata(ENDPOINT *endpoint, unsigned new_words_written,
                                    TIME last_sample_time);

#endif /* defined(INSTALL_TTP) && defined(INSTALL_METADATA) */
#endif /* INSTALL_AUDIO_SOURCE_TOA_METADATA_GENERATE */
#endif /* !_STREAM_ENDPOINT_AUDIO_H_ */

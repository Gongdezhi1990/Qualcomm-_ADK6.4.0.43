/**
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
 * 
 * \file  ttp.h
 *
 * \ingroup ttp
 * 
 * Public header file for the time-to-play (TTP) module.
 */

#ifndef TTP_H
#define TTP_H

#include "types.h"

typedef struct ttp_context ttp_context;

/** 
 * Status structure
 * This returns all the data a typical TTP client needs 
 */
typedef struct 
{
    TIME            ttp;                /**< Calculated time to play */
    int             sp_adjustment;      /**< Current sample period adjustment */
    unsigned        err_offset_id;      /**< ID for error offset */
    bool            stream_restart;     /**< TRUE if the stream has restarted */
}
ttp_status;

/** 
 * Parameters structure
 * Various configurable values affecting the TTP generation
 */
typedef struct 
{
    unsigned        nominal_sample_rate;
    unsigned        filter_gain;
    unsigned        err_scale;
    TIME_INTERVAL   startup_period;
}
ttp_params;

typedef enum 
{
    TTP_TYPE_NONE,
    TTP_TYPE_PCM,
    TTP_TYPE_A2DP
} ttp_source_type;

#define INFO_ID_INVALID 0

/**
 * \brief  Initialise time to play.
 *
 * \return  pointer to a new (empty) ttp_context structure.
 * 
 * Typically called at operator create.
 * Initialises the time to play functionality for a new instance, and returns a pointer
 * to a ttp_context structure. This should be stored by the caller for subsequent use,
 * and eventually freed by calling ttp_free.
 */
extern ttp_context *ttp_init(void);

/**
 * \brief  Reset time-to-play.
 *
 * \param  context   pointer to active TTP context structure.
 *
 * Resets TTP state without changing parameters.
 * Typically used for streams that can stop and restart.
 */
extern void ttp_reset(ttp_context *context);

/**
 * \brief  Populate TTP parameters
 *
 * \param  source type   enum describing the nature of the source data.
 *
 * Populates a ttp_params structure with some appropriate default values 
 * for the given source type.
 */
extern void ttp_get_default_params(ttp_params *params, ttp_source_type source_type);

/**
 * \brief  Extract target latency from operator message
 *
 * \param  message_data   pointer to message payload
 *
 * \return latency value extracted from payload
 *
 * Helper function to extract the target latency value from 
 * a received OPMSG_COMMON_SET_TTP_LATENCY message
 */
extern TIME_INTERVAL ttp_get_msg_latency(void *message_data);

/**
 * \brief  Extract latency limits from operator message
 *
 * \param  message_data   pointer to message payload
 *
 * \param  min_latency    pointer to store low limit value
 *
 * \param  max_latency    pointer to store high limit value
 *
 * Helper function to extract the latency limits from 
 * a received OPMSG_COMMON_SET_LATENCY_LIMITS message
 */
extern void ttp_get_msg_latency_limits(void *message_data, TIME_INTERVAL *min_latency, TIME_INTERVAL *max_latency);

/**
 * \brief  Extract TTP parameters from operator message
 *
 * \param  params   pointer to parameter struct to populate
 * 
 * \param  message_data   pointer to message payload
 *
 * Helper function to extract the parameter values from 
 * a received OPMSG_COMMON_SET_TTP_PARAMS message
 */
extern void ttp_get_msg_params(ttp_params *params, void *message_data);

/**
 * \brief  Configure TTP target latency
 *
 * \param  context   pointer to active TTP context structure.
 *
 * \param  target_latency   Required target latency
 *
 * Configures target latency.
 * Typically called at operator start, when all the required information is available.
 */
extern void ttp_configure_latency(ttp_context *context, TIME_INTERVAL target_latency);

/**
 * \brief  Configure TTP latency limits
 *
 * \param  context   pointer to active TTP context structure.
 *
 * \param  min_latency   Minimum latency value to trigger a reset
 *
 * \param  max_latency   Maximum latency value to trigger a reset
 *
 * Configures latency limits. 
 * A max_latency value of zero means there is no upper limit
 * (other than that implied by available buffering)
 */
extern void ttp_configure_latency_limits(ttp_context *context, TIME_INTERVAL min_latency, TIME_INTERVAL max_latency);


/**
 * \brief  Configure TTP context parameters
 *
 * \param  context   pointer to active TTP context structure.
 *
 * \param  params   pointer to parameter data
 *
 * Configures sample period estimation parameters. 
 * Typically called at operator start, when all the required information is available.
 * Note that the sample rate might not be known at this point. 
 * If the nominal_sample_rate value in the supplied structure is zero, the existing
 * context value will be unchanged.
 */
extern void ttp_configure_params(ttp_context *context, const ttp_params *params);

/**
 * \brief  Configure TTP sample rate
 *
 * \param  context   pointer to active TTP context structure.
 *
 * \param  sample_rate   nominal sample rate, in Hz
 *
 * Configures nominal sample rate for TTP calculations.
 * Typically called at operator start, when all the required information is available.
 */
extern void ttp_configure_rate(ttp_context *context, unsigned sample_rate);

/**
 * \brief  Update TTP context and return new time-to-play
 *
 * \param  context   pointer to active TTP context structure
 *
 * \param  time   local microsecond time when some data was received
 *
 * \param  samples   number of samples in the data 
 *
 * \param status  pointer to status structure for return data
 *
 * This should be called each time the TTP-generating operator receives some data.
 * It updates internal state based on the given time and number of samples,
 * and provides the calculated time to play for the first sample. 
 */
extern void ttp_update_ttp(ttp_context *context, TIME time, unsigned samples, ttp_status *status);

/**
 * \brief  Clock TTP context based on amount of data and return new time-to-play
 *
 * \param  context   pointer to active TTP context structure
 *
 * \param  samples   number of samples in the data
 *
 * \param status  pointer to status structure for return data
 *
 * This may be called when there is no fixed relationship between the
 * data and the processing time. E.g. by an operator which attempts to fill
 * a gap in a TTP stream.
 */
extern void ttp_continue_ttp(ttp_context *context, unsigned samples, ttp_status *status);

/**
 * \brief  Get current sample-period adjustment factor
 *
 * \param  context   pointer to active TTP context structure
 *
 * \return calculated SP adjustment value (fractional difference from nominal)
 *
 * This information is also available in the status structure populated
 * by ttp_update_ttp. A separate access function is provided in case clients 
 * need the adjustment independently from calculating a new TTP value.
 */
extern int ttp_get_sp_adjustment(ttp_context *context);


/**
 * \brief Function calculates the timestamp for a new tag based on a previous tag.
 *
 *      new timestamp [us] =
 *          last timestamp + ((samples to new * 1 000 000)/ sample rate [samples/sec]) *(1 + sample period adjustment)
 *
 * \param samples Samples available to consume.
 * \param sample_rate The rate in which the samples are consumed in samples/sec.
 *
 * \return New timestamp.
 */
unsigned ttp_get_next_timestamp(unsigned last_timestamp, unsigned nr_of_samples,
        unsigned sample_rate, int sp_adjust);

/**
 * \brief  Free TTP context
 *
 * \param  context   pointer to context structure to be freed.
 *
 * Frees the previously-allocated context structure, typically called at operator destroy.
 * This is probably just a thin wrapper round pfree/pdelete,
 * but the caller shouldn't assume that to be the case.
 * Freeing a NULL context pointer is safely ignored
 */
extern void ttp_free(ttp_context *context);

/* Functions from ttp_info.c */

/**
 * \brief  Create a new info entry
 *
 * \param  data   pointer to data item to pass in metadata
 * 
 * \return identifier for data item, or zero if the entry creation fails.
 *
 */
extern unsigned ttp_info_create(void *data);

/**
 * \brief  Get a pointer from an info entry
 *
 * \param  ID  info entry ID
 * 
 * \return pointer to data item, or NULL if invalid
 *
 */
extern void *ttp_info_get(unsigned id);

/**
 * \brief  Destroy an info entry
 *
 * \param  ID  info entry ID
 * 
 * \return TRUE if destroyed, or FALSE if invalid
 *
 */
extern bool ttp_info_destroy(unsigned id);


#endif /* TTP__H */


/**
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
 * 
 * \defgroup ttp Time to play generator
 * \file  ttp.c
 *
 * \ingroup ttp
 *
 * Time-to-play (TTP) implementation
 *
 */

/****************************************************************************
Include Files
*/

#include "ttp_private.h"

/****************************************************************************
Private Type Declarations
*/

typedef enum 
{
    TTP_STATE_INIT,
    TTP_STATE_STARTUP,
    TTP_STATE_RUNNING,
    TTP_STATE_CONTINUING
} ttp_state;


struct ttp_context
{
    TIME ttp;
    TIME_INTERVAL target_latency;
    TIME_INTERVAL countdown;
    TIME_INTERVAL error_offset;
    TIME_INTERVAL old_latency;
    TIME expiry_time;
    uint48 filtered_error;
    unsigned adj_info_id;
    unsigned delta;
    unsigned delta_remainder;
    int sp_adjustment;
#ifdef TTP_GEN_DEBUG
    TIME prev_toa;
#endif
    ttp_state state;
    ttp_params params;
    TIME_INTERVAL min_latency;
    TIME_INTERVAL max_latency;
    TIME prev_source_time;
    int ttp_frac;
};



/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Macro Declarations
*/

#if defined (__GNUC__) && !defined (K32)
/* For unit tests on 24-bit platforms, the platform word size isn't the compiler's native word */
#define LSW_MASK 0xffffff
#else
/* For everything else it is */
#define LSW_MASK UINT_MAX
#endif

#define MSW(x) ((x) >> DAWTH)
#define LSW(x) ((x) & LSW_MASK)

/* Parameter defaults */
#define DEFAULT_STARTUP_PERIOD (200 * MILLISECOND)
#define DEFAULT_FILT_GAIN (FRACTIONAL(0.997))
#define DEFAULT_ERR_SCALE (FRACTIONAL(-0.00000001))

/* Minimum difference between countdown period and target latency */
#define STARTUP_MARGIN (20 * MILLISECOND)

/* Minimum startup countdown period */
#define MIN_COUNTDOWN (10 * MILLISECOND)

/* Some constants lifted from Broadcast Audio code */
#define LATENCY_STEP_LIMIT -2000
#define LATENCY_LEAK_FACTOR (FRACTIONAL(0.999))

/* Fractional-part scaling for source time based TTP generation
 * Pick a power of 2 to make the calculations more efficient, exact value is not critical
 */
#define DELTA_FRAC_SCALING 1024

#ifdef TTP_GEN_DEBUG
/* Report ToA steps larger than this with a debug message */
#define TOA_DEBUG_LIMIT (50 * MILLISECOND)
#endif

/****************************************************************************
Public Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/

/****************************************************************************
Private Function Definitions
*/

/**
 * frac_mul_long
 *
 * \brief  Multiply a double-word by a fractional value
 *
 */
static int48 frac_mul_long(int48 m, int f)
{
    return ((MSW(m)*f) << 1) + ((LSW(m)*f) >> (DAWTH-1));
}

/**
 * calc_filtered_error
 *
 * \brief  Filter the raw latency error and store the result
 *
 */
static void calc_filtered_error(ttp_context *context, TIME_INTERVAL raw_error)
{
    context->filtered_error = frac_mul_long(context->filtered_error, context->params.filter_gain) + 
        frac_mul_long((int48)raw_error << DAWTH, FRACTIONAL(1.0)-context->params.filter_gain);
}

/**
 * get_msg_fractional
 *
 * \brief  Convert message parameters from 2 16-bit words to fractional
 *
 * Note message is always 32-bit, so the LS 8 bits are lost on 24-bit platforms
 *
 */
static int get_msg_fractional(uint16 msw, uint16 lsw)
{
#ifdef K32
    return (int)(lsw | (msw << 16));
#else
    return (int)((lsw >> 8) | (msw << 8));
#endif
}

/****************************************************************************
Public Function Definitions
*/

/**
 * ttp_init 
 *
 * \brief  Initialise time to play.
 *
 */
ttp_context *ttp_init(void)
{
    ttp_context *context = xzpnew(ttp_context);

    patch_fn_shared(ttp_gen);

    if (context != NULL)
    {
        /* TODO what if this fails ? */

        /* 
         * Store the location of the error offset
         * so the playback control can adjust the initial timestamps
         */
        context->adj_info_id = ttp_info_create(&context->error_offset);
    }
    
    return context;
}

/**
 * ttp_reset 
 *
 * \brief  Reset time to play.
 *
 */
void ttp_reset(ttp_context *context)
{
    patch_fn_shared(ttp_gen);

    /* Go back to the init state.
     * This will mean most values get reset automatically.
     */
    context->state = TTP_STATE_INIT;

    /* Reset some working data */
    context->filtered_error = 0;
    context->delta_remainder = 0;
    context->sp_adjustment = 0;
    context->error_offset = 0;
    context->ttp_frac = 0;
    context->prev_source_time = 0;
}


/**
 * ttp_get_default_params
 *
 * \brief  Populate TTP parameters
 *
 */
void ttp_get_default_params(ttp_params *params, ttp_source_type source_type)
{
    patch_fn_shared(ttp_gen);

    /* Populate the supplied parameter struct with some defaults based on the source type */
    params->startup_period = DEFAULT_STARTUP_PERIOD;
    params->filter_gain = DEFAULT_FILT_GAIN;
    params->err_scale = DEFAULT_ERR_SCALE;
    params->nominal_sample_rate = stream_if_get_system_sampling_rate();
}

/**
 * ttp_get_msg_latency
 *
 * \brief  Extract target latency from operator message
 *
 */
TIME_INTERVAL ttp_get_msg_latency(void *message_data)
{
    patch_fn_shared(ttp_gen);

    PL_ASSERT(OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_TTP_LATENCY, MESSAGE_ID) == OPMSG_COMMON_SET_TTP_LATENCY);

    /* decode the message data and return the requested latency value */
    TIME_INTERVAL msg_latency = (TIME_INTERVAL)OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_TTP_LATENCY, LATENCY_LS) +
                               ((TIME_INTERVAL)OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_TTP_LATENCY, LATENCY_MS) << 16);
    return msg_latency;
}

/**
 * ttp_get_msg_latency_limits
 *
 * \brief  Extract latency limits from operator message
 *
 */
void ttp_get_msg_latency_limits(void *message_data, TIME_INTERVAL *min_latency, TIME_INTERVAL *max_latency)
{
    patch_fn_shared(ttp_gen);

    PL_ASSERT(OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_LATENCY_LIMITS, MESSAGE_ID) == OPMSG_COMMON_SET_LATENCY_LIMITS);

    /* decode the message data and populate the latency limits */
    *min_latency = (TIME_INTERVAL)OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_LATENCY_LIMITS, MIN_LATENCY_LS) +
                  ((TIME_INTERVAL)OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_LATENCY_LIMITS, MIN_LATENCY_MS) << 16);
    *max_latency = (TIME_INTERVAL)OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_LATENCY_LIMITS, MAX_LATENCY_LS) +
                  ((TIME_INTERVAL)OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_LATENCY_LIMITS, MAX_LATENCY_MS) << 16);
}

/**
 * ttp_get_msg_params
 *
 * \brief  Extract TTP parameters from operator message
 *
 */
void ttp_get_msg_params(ttp_params *params, void *message_data)
{
    patch_fn_shared(ttp_gen);

    PL_ASSERT(OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_TTP_PARAMS, MESSAGE_ID) == OPMSG_COMMON_SET_TTP_PARAMS);

    /* decode the message data */
    params->filter_gain = get_msg_fractional(OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_TTP_PARAMS, FILTER_GAIN_MS),
                                             OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_TTP_PARAMS, FILTER_GAIN_LS));
    params->err_scale = get_msg_fractional(OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_TTP_PARAMS, ERROR_SCALE_MS),
                                           OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_TTP_PARAMS, ERROR_SCALE_LS));
    params->startup_period = (TIME_INTERVAL)(OPMSG_FIELD_GET(message_data, OPMSG_COMMON_MSG_SET_TTP_PARAMS, STARTUP_TIME) * MILLISECOND);
}

/**
 * ttp_configure_latency
 *
 * \brief  Configure TTP target latency
 *
 */
void ttp_configure_latency(ttp_context *context, TIME_INTERVAL target_latency)
{
    patch_fn_shared(ttp_gen);

    /* Just copy the supplied value into the context structure
     * If this needs to change while running, the estimation should be reset
     * For now that's not supported 
     */
    TTP_WARN_MSG1("TTP target latency = %d", target_latency);

    context->target_latency = target_latency;
}

/**
 * ttp_configure_latency_limits
 *
 * \brief  Configure TTP latency limits
 *
 */
void ttp_configure_latency_limits(ttp_context *context, TIME_INTERVAL min_latency, TIME_INTERVAL max_latency)
{
    patch_fn_shared(ttp_gen);

    TTP_WARN_MSG2("TTP latency min = %d max = %d", min_latency, max_latency);

    context->min_latency = min_latency;
    context->max_latency = max_latency;
}

/**
 * ttp_configure_params
 *
 * \brief  Configure TTP estimation parameters
 *
 */
void ttp_configure_params(ttp_context *context, const ttp_params *params)
{
    ttp_params temp_params = *params;
    
    patch_fn_shared(ttp_gen);

    /* If the supplied rate is zero, use the previous one instead */
    if (temp_params.nominal_sample_rate == 0)
    {
        temp_params.nominal_sample_rate = context->params.nominal_sample_rate;
    }

    context->params = temp_params;
}


/**
 * ttp_configure_rate
 *
 * \brief  Configure TTP nominal sample rate
 *
 */
void ttp_configure_rate(ttp_context *context, unsigned sample_rate)
{
    patch_fn_shared(ttp_gen);

    /* Just copy the supplied value into the context structure
     * If this needs to change while running, the estimation should be reset
     * For now that's not supported 
     */
    context->params.nominal_sample_rate = sample_rate;
}

void ttp_update_ttp_from_source_time(ttp_context *context, TIME toa, TIME source_time, ttp_status *status)
{
    TIME_INTERVAL raw_error = 0;
    TIME_INTERVAL time_delta = 0;
    int delta_frac;

    patch_fn_shared(ttp_gen);

    status->stream_restart = FALSE;

    if (context->state == TTP_STATE_CONTINUING)
    {
        /* Switch from continuing back to generating real timestamps.
         * Recognize not being called from ttp_continue_ttp by non-zero time.
         * This may rarely delay switching away from CONTINUING for one block.
         */
        if (toa != 0)
        {
            context->state = TTP_STATE_INIT;
            status->stream_restart = TRUE;
        }
    }
    else if (context->state != TTP_STATE_INIT)
    {
        /* First check the expiry time */
        /* If the new data arrival time is later than the TTP 
         * of the last thing that was processed, it must have been played already
         */
        if (time_gt(toa, context->expiry_time))
        {
            TTP_WARN_MSG1("TTP Gen stall, time = %d", toa);
            ttp_reset(context);
            status->stream_restart = TRUE;
        }

        time_delta = source_time - context->prev_source_time;
        delta_frac = frac_mult(time_delta*DELTA_FRAC_SCALING, context->sp_adjustment);
        time_delta += delta_frac / DELTA_FRAC_SCALING;
        if (delta_frac > 0)
        {
            context->ttp_frac += delta_frac % DELTA_FRAC_SCALING;
            if (context->ttp_frac >= DELTA_FRAC_SCALING)
            {
                context->ttp_frac -= DELTA_FRAC_SCALING;
                time_delta++;
            }
        }
        else
        {
            context->ttp_frac -= (-delta_frac) % DELTA_FRAC_SCALING;
            if (context->ttp_frac <= -DELTA_FRAC_SCALING)
            {
                context->ttp_frac += DELTA_FRAC_SCALING;
                time_delta--;
            }
        }
    }

#ifdef TTP_GEN_DEBUG
    if ((context->prev_toa != 0) && (time_sub(toa, context->prev_toa) > TOA_DEBUG_LIMIT))
    {
        TTP_WARN_MSG2("TTP Gen ToA = %d step = %d", toa, time_sub(toa, context->prev_toa));
    }
    context->prev_toa = toa;
#endif
    
    if (context->state == TTP_STATE_INIT)
    {
        /* Sample rate must have been set by this point 
         *
         * Note use of assert here - although the sample rate will 
         * typically be set by opmsg, it's the caller's responsibility 
         * to make sure this doesn't get called unless that's happened 
         * (e.g. by failing to start the operator, 
         * or by setting some default parameters).
         */
        PL_ASSERT(context->params.nominal_sample_rate >= 8000);
        PL_ASSERT(context->params.nominal_sample_rate <= 192000);

        /* First call with this context, so just set the initial TTP */
        context->ttp = time_add(toa, context->target_latency);

        /* Move to startup state and set the countdown period */
        context->state = TTP_STATE_STARTUP;
        context->countdown = context->params.startup_period;
        context->old_latency = context->target_latency;
        if ((context->countdown == DEFAULT_STARTUP_PERIOD) && (context->countdown >= context->target_latency - STARTUP_MARGIN))
        {
            context->countdown = context->target_latency - STARTUP_MARGIN;
        }
        if (context->countdown < MIN_COUNTDOWN)
        {
            context->countdown = 0;
        }
    }
    else if (context->state == TTP_STATE_CONTINUING)
    {
        /* TTP for this block is based on the calculated duration for the previous one.
         * No further updates are done.
         */
        context->ttp = time_add(context->ttp, time_delta);
    }
    else
    {
        TIME_INTERVAL raw_latency;
        /* TTP for this block is based on the calculated duration for the previous one */
        context->ttp = time_add(context->ttp, time_delta);
        raw_latency = time_sub(context->ttp, toa);

        if ((context->state == TTP_STATE_RUNNING) &&
            (raw_latency - context->old_latency < LATENCY_STEP_LIMIT))
        {
            context->old_latency = frac_mult((int)(context->old_latency), LATENCY_LEAK_FACTOR);
        }
        else
        {
            context->old_latency = raw_latency;
        }
        raw_error = context->old_latency - context->target_latency;
        if ((context->max_latency > 0) && (raw_latency > context->max_latency))
        {
            TTP_WARN_MSG2("TTP Max latency exceeded max = %d raw = %d", context->max_latency, raw_latency);
            ttp_reset(context);
        }
        if ((context->min_latency > 0) && (raw_latency < context->min_latency))
        {
            TTP_WARN_MSG2("TTP Min latency exceeded min = %d raw = %d", context->min_latency, raw_latency);
            ttp_reset(context);
        }

    }

    /* Check countdown period */
    if (context->state == TTP_STATE_STARTUP)
    {
        context->countdown -= time_delta;
        if (context->countdown <= 0)
        {
            /* Initial countdown is done, so go to the normal running state */
            context->state = TTP_STATE_RUNNING;
            /* Correct the accumulated error */
            context->error_offset = raw_error;
            context->old_latency = context->target_latency;
            context->ttp = time_sub(context->ttp, raw_error);
            raw_error = 0;
        }
    }

    /* The time when the current block of samples will finish playing */
    context->expiry_time = time_add(context->ttp, time_delta);

    if (context->state == TTP_STATE_RUNNING)
    {
        calc_filtered_error(context, raw_error);
        /* 
         * Convert the filtered error to SP adjustment
         * We assume the scaling factor is small enough that the resulting value fits in a single word
         */
        context->sp_adjustment = (int)(frac_mul_long(context->filtered_error, context->params.err_scale));

        /* Limit SP adjustment to +/- 0.5% */
        if (context->sp_adjustment > FRACTIONAL(0.005)) 
        {
            context->sp_adjustment = FRACTIONAL(0.005);
        }
        if (context->sp_adjustment < -FRACTIONAL(0.005)) 
        {
            context->sp_adjustment = -FRACTIONAL(0.005);
        }
    }

    /* Populate the status structure provided by the caller */
    status->ttp = context->ttp;
    status->sp_adjustment = context->sp_adjustment;

    context->prev_source_time = source_time;

    if (context->state == TTP_STATE_STARTUP)
    {
        status->err_offset_id = context->adj_info_id;
    }
    else
    {
        status->err_offset_id = INFO_ID_INVALID;
    }
    TTP_DBG_MSG5("TTP Generator 0x%08x: time = %4d, time to play = %4d, sra = 0x%08x, context delta = %4d",
                         (uintptr_t)context,       toa,       context->ttp, context->sp_adjustment, context->delta);
}




/**
 * ttp_update_ttp
 *
 * \brief  Update TTP context and return new time-to-play
 *
 */
void ttp_update_ttp(ttp_context *context, TIME time, unsigned samples, ttp_status *status)
{
    TIME_INTERVAL raw_error = 0;
    uint32 period_adj;

    patch_fn_shared(ttp_gen);

    status->stream_restart = FALSE;

    if (context->state == TTP_STATE_CONTINUING)
    {
        /* Switch from continuing back to generating real timestamps.
         * Recognize not being called from ttp_continue_ttp by non-zero time.
         * This may rarely delay switching away from CONTINUING for one block.
         */
        if (time != 0)
        {
            context->state = TTP_STATE_INIT;
            status->stream_restart = TRUE;
        }
    }
    else if (context->state != TTP_STATE_INIT)
    {
        /* First check the expiry time */
        /* If the new data arrival time is later than the TTP 
         * of the last thing that was processed, it must have been played already
         */
        if (time_gt(time, context->expiry_time))
        {
            TTP_WARN_MSG1("TTP Gen stall, time = %d", time);
            ttp_reset(context);
            status->stream_restart = TRUE;
        }
    }

#ifdef TTP_GEN_DEBUG
    if ((context->prev_toa != 0) && (time_sub(time, context->prev_toa) > TOA_DEBUG_LIMIT))
    {
        TTP_WARN_MSG2("TTP Gen ToA = %d step = %d", time, time_sub(time, context->prev_toa));
    }
    context->prev_toa = time;
#endif
    
    if (context->state == TTP_STATE_INIT)
    {
        /* Sample rate must have been set by this point 
         *
         * Note use of assert here - although the sample rate will 
         * typically be set by opmsg, it's the caller's responsibility 
         * to make sure this doesn't get called unless that's happened 
         * (e.g. by failing to start the operator, 
         * or by setting some default parameters).
         */
        PL_ASSERT(context->params.nominal_sample_rate >= 8000);
        PL_ASSERT(context->params.nominal_sample_rate <= 192000);

        /* First call with this context, so just set the initial TTP */
        context->ttp = time_add(time, context->target_latency);

        /* Move to startup state and set the countdown period */
        context->state = TTP_STATE_STARTUP;
        context->countdown = context->params.startup_period;
        context->old_latency = context->target_latency;
        if ((context->countdown == DEFAULT_STARTUP_PERIOD) && (context->countdown >= context->target_latency - STARTUP_MARGIN))
        {
            context->countdown = context->target_latency - STARTUP_MARGIN;
        }
        if (context->countdown < MIN_COUNTDOWN)
        {
            context->countdown = 0;
        }
    }
    else if (context->state == TTP_STATE_CONTINUING)
    {
        /* TTP for this block is based on the calculated duration for the previous one.
         * No further updates are done.
         */
        context->ttp = time_add(context->ttp, (TIME_INTERVAL)context->delta);
    }
    else
    {
        TIME_INTERVAL raw_latency;
        /* TTP for this block is based on the calculated duration for the previous one */
        context->ttp = time_add(context->ttp, (TIME_INTERVAL)context->delta);
        raw_latency = time_sub(context->ttp, time);

        if ((context->state == TTP_STATE_RUNNING) &&
            (raw_latency - context->old_latency < LATENCY_STEP_LIMIT))
        {
            context->old_latency = frac_mult((int)(context->old_latency), LATENCY_LEAK_FACTOR);
        }
        else
        {
            context->old_latency = raw_latency;
        }
        raw_error = context->old_latency - context->target_latency;
        if ((context->max_latency > 0) && (raw_latency > context->max_latency))
        {
            TTP_WARN_MSG2("TTP Max latency exceeded max = %d raw = %d", context->max_latency, raw_latency);
            ttp_reset(context);
        }
        if ((context->min_latency > 0) && (raw_latency < context->min_latency))
        {
            TTP_WARN_MSG2("TTP Min latency exceeded min = %d raw = %d", context->min_latency, raw_latency);
            ttp_reset(context);
        }

    }

    /* Check countdown period */
    if (context->state == TTP_STATE_STARTUP)
    {
        context->countdown -= context->delta;
        if (context->countdown <= 0)
        {
            /* Initial countdown is done, so go to the normal running state */
            context->state = TTP_STATE_RUNNING;
            /* Correct the accumulated error */
            context->error_offset = raw_error;
            context->old_latency = context->target_latency;
            context->ttp = time_sub(context->ttp, raw_error);
            raw_error = 0;
        }
    }

    /* Calculate the duration of this block of samples, including a rolling remainder */

    period_adj = 1000000ul*samples + frac_mul_long(1000000ul*samples, context->sp_adjustment);

    context->delta = (unsigned)((period_adj + context->delta_remainder) / context->params.nominal_sample_rate);
    context->delta_remainder += (unsigned)(period_adj - (context->delta * context->params.nominal_sample_rate));

    /* The time when the current block of samples will finish playing */
    context->expiry_time = time_add(context->ttp, context->delta);

    if (context->state == TTP_STATE_RUNNING)
    {
        calc_filtered_error(context, raw_error);
        /* 
         * Convert the filtered error to SP adjustment
         * We assume the scaling factor is small enough that the resulting value fits in a single word
         */
        context->sp_adjustment = (int)(frac_mul_long(context->filtered_error, context->params.err_scale));

        /* Limit SP adjustment to +/- 0.5% */
        if (context->sp_adjustment > FRACTIONAL(0.005)) 
        {
            context->sp_adjustment = FRACTIONAL(0.005);
        }
        if (context->sp_adjustment < -FRACTIONAL(0.005)) 
        {
            context->sp_adjustment = -FRACTIONAL(0.005);
        }
    }

    /* Populate the status structure provided by the caller */
    status->ttp = context->ttp;
    status->sp_adjustment = context->sp_adjustment;

    if (context->state == TTP_STATE_STARTUP)
    {
        status->err_offset_id = context->adj_info_id;
    }
    else
    {
        status->err_offset_id = INFO_ID_INVALID;
    }
    TTP_DBG_MSG5("TTP Generator 0x%08x: time = %4d, time to play = %4d, sra = 0x%08x, context delta = %4d",
                         (uintptr_t)context,       time,       context->ttp, context->sp_adjustment, context->delta);
}

/**
 * ttp_continue_ttp
 *
 * \brief  Data-driven contiguous update to TTP context
 *
 */
void ttp_continue_ttp(ttp_context *context, unsigned samples, ttp_status *status)
{
    TIME now;

    if (context->state == TTP_STATE_INIT)
    {
        /*
         * ttp_continue_ttp should not be called right after
         * ttp_reset()/ttp_init(), but if it is, that must
         * be handled somehow. Set up normally.
         * Subsequent updates will be data driven.
         */
        now = hal_get_time();
    }
    else
    {
        context->state = TTP_STATE_CONTINUING;
        now = 0;
    }

    ttp_update_ttp(context, now, samples, status);
}


/**
 * ttp_get_sp_adjustment
 * 
 * \brief  Get current sample-period adjustment factor
 */
int ttp_get_sp_adjustment(ttp_context *context)
{
    return context->sp_adjustment;
}

/*
 * ttp_get_next_timestamp
 * 
 * \brief Function calculates the timestamp for a new tag based on a previous tag.
 */
unsigned ttp_get_next_timestamp(unsigned last_timestamp, unsigned nr_of_samples,
        unsigned sample_rate, int sp_adjust)
{
    unsigned advance = (unsigned)(((uint48)nr_of_samples * 1000000)/sample_rate);

    advance = advance + frac_mult(advance, sp_adjust);

    TTP_DBG_MSG5("TTP Reframe: last_timestamp = 0x%08x, tag nr_of_samples = 0x%08x, sp_adjust+1.0  = 0x%08x, advance = 0x%08x , new_time = 0x%08x",
                              last_timestamp, nr_of_samples, sp_adjust, advance, time_add(last_timestamp, advance));

    return time_add(last_timestamp, advance);
}

/**
 * ttp_free
 *
 * \brief  Free TTP context
 *
 */
void ttp_free(ttp_context *context)
{
    if ((context != NULL) && (context->adj_info_id != INFO_ID_INVALID))
    {
        ttp_info_destroy(context->adj_info_id);
    }
    pdelete(context);
}

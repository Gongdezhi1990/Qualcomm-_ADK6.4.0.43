/**
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
 *
 * \file  timed_playback.c
 *
 * \ingroup ttp
 *
 * Timed playback implementation
 *
 */

/****************************************************************************
Include Files
*/

#include "timed_playback.h"
#include "ttp_private.h"

/****************************************************************************
Private Type Declarations
*/
/**
 * Enumeration used to present the status of the metadata tag.
 */
typedef enum
{
    /**
     * TTP time is invalid.
     */
    TAG_INVALID = 0,
    /**
     * TTP time late.
     */
    TAG_LATE = 1,
    /**
     * TTP time on time.
     */
    TAG_ON_TIME = 2,
    /**
     * TTP time is early.
     */
    TAG_EARLY = 3

} tag_timestamp_status;

/**
 * Structure hold the useful information from the metadata tag.
 */
typedef struct current_metadata_tag_struct
{
    /** Size of the fragment (in octets) */
    unsigned length;

    /** Size of the fragment (in octets) */
    unsigned index;

    /** TTP playback time (in us) */
    unsigned playback_time;

    /** The tag sample period adjustment.  */
    int sp_adjust;

    /** The error of the timestamp (in us)*/
    int error;

    /** TTP timestamp (in us) */
    unsigned timestamp;

    /** The status of the tag.*/
    tag_timestamp_status status;

    /** True if the tag is void one. False, otherwise */
    bool is_void;
}current_metadata_tag ;



struct TIMED_PLAYBACK_STRUCT
{
    /**
     * Nr of channels in the cbops chain.
     */
    unsigned nr_of_channels;

    /**
     * Nr of used channels in the cbops chain.
     */
    unsigned used_channels;

    /**
     * The input buffers of the timed playback module.
     */
    tCbuffer *in_buffers[CBOPS_MAX_NR_CHANNELS];

    /**
     * The output buffers of the timed playback module.
     */
    tCbuffer *out_buffers[CBOPS_MAX_NR_CHANNELS];

    /**
     * TRUE if HW warp is active.
     */
    bool do_hw_warp;

    /**
     * Reference for rate adjust function
     */
    void *rm_data;

    /**
     * Function to call to adjust output rate
     */
    void (*rate_adjust)(void *data, int32 adjust_val);

    /**
     * Module used for timestamp reframing.
     */
    REFRAME tag_reframe;

    /**
     * Cbops used to fine adjust the error during the timed playback.
     */
    cbops_op* rate_adjustment;

    /**
     * DC remove cbops is kept to remove the dc offset.
     */
    cbops_op* dc_remove_op;

    /**
     * Pointer to cbops_manager that encapsulates all the cbops operators.
     */
    struct cbops_mgr *cbops_manager;

    /** Internal settings of the PID controller .*/
    pid_controller_settings pid_params;

    /** The PID controller internal state. */
    pid_controller_state pid_state;

    /* Information stored about the last metadata tag which includes the calculated
     * error for the tag. */
    current_metadata_tag current_tag;

    /**
     * The sample rate in which the timed playback works.
     */
    unsigned sample_rate;

    /**
     * The period in which the timed playback run is called.
     */
    TIME_INTERVAL period;

    /**
     * The error threshold which will trigger a late or early case.
     */
    unsigned error_limit;

    /**
     * The extra delay introduced by the endpoint.
     */
    unsigned endpoint_delay;

    /**
     * True if at least one timestamped tag was read from the input buffer enabling the
     * timed playback, false otherwise.
     */
    bool start;

    /**
     * Counts the tags with increasingly late timestamp. Used to detect unachievable latency.
     */
    unsigned later_tag_count;

    /**
     * Callback structure for sending an unachievable latency unsolicited message
     */
    ttp_unachv_lat_callback_struct cback;
};


#ifdef TTP_PLOTTER
/**
 * Variable used to share the error with the plotter tool.
 */
static int timed_palyback_error = 0;

/**
 * Variable used to share the calculated sra with the plotter tool.
 */
static int timed_palyback_sra = 0;

/**
 * Variable used to share the playback time with the plotter tool.
 */
static int timed_palyback_pb_time = 0;

/**
 * Variable used to share the error offset with the plotter tool.
 */
static int timed_palyback_err_offset = 0;

/**
 * Variable used to share the sample rate adjustment with the plotter tool.
 */
static int timed_palyback_sp_adjust = 0;

#endif



/****************************************************************************
Private Constant Declarations
*/
/* Enble disable buffer debug.*/
/*#define TTP_BUFFER_DEBUG*/

#define WARP_SHIFT_AMOUNT   8
#define WARP_P_TERM_MAX     (30000 << (WARP_SHIFT_AMOUNT))
#define WARP_I_ERROR_MAX    0x400000

/** Tight error limit in us*/
#define WARP_TIGHT_LIMIT    50

/** Loose error limit in us*/
#define WARP_LOOSE_LIMIT    2000

/** Error shift */
#define ERROR_SHIFT_AMOUNT  8

/**
 * Compiler switch to tell the timed playback module to use cbuffer functions (where
 * cbops operations are used). This is useful for testing.*/
/*#define DBG_USE_CBUFFER*/

/**
 * Compiler switch to tell the timed playback that for void tag silence insertion is used
 * rather than a copy. This is useful for testing.
 */
/* #define USE_SILENCE_INSERTION */

/** Silence samples per kick.  */
#define SILENCE_SAMPLES_PER_RUN 2000

/** The maximum samples an sra can generate in one run */
#define SRA_SAMPLES 5

/** If more than 5 late tags with increasing errors are read than generate an error.  */
#define UNACHIEVABLE_LATENCY_LIMIT 5

/** The error margin for the timed playback error in us. */
#define PERIOD_ERROR 500

/****************************************************************************
Private Macro Declarations
*/

/** Subtract samples */
#define SUBTRACT_SAMPLES(X,Y) ( (X)>(Y)? ((X)-(Y)): 0)

/** Limits A between [L,U] */
#define CLAMP(A, L, U)  MIN(MAX((A), (L)),(U))

/****************************************************************************
Public Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/
static bool timed_playback_tag_on_time(TIMED_PLAYBACK* timed_pb, TIME_INTERVAL error);
static void timed_playback_discard_samples(TIMED_PLAYBACK* timed_pb, unsigned samples_to_discard);
static void timed_playback_insert_silence(TIMED_PLAYBACK* timed_pb, unsigned silence_samples);
static void ttp_reset_cbops_chain(TIMED_PLAYBACK* timed_pb);

/****************************************************************************
Private Function Definitions
*/





/**
* \brief Calculates the available output space (considering all channels).
*
* \param timed_pb - Pointer to the timed_pb playback instance
*
* \return Returns the minimum available space calculated in all the output channels.
*/
static unsigned ttp_output_buffers_space(TIMED_PLAYBACK* timed_pb)
{
    unsigned retval = UINT_MAX;
    unsigned current;
    unsigned channel;
    for (channel = 0;channel < timed_pb->used_channels; channel++)
    {
        current = cbuffer_calc_amount_space_in_words(timed_pb->out_buffers[channel]);
        if (current < retval)
        {
            retval = current;
        }
    }
    return retval;
}

/**
* \brief Calculates the available samples in the output buffers.
*
* \param timed_pb - Pointer to the timed_pb playback instance
*
* \return Returns the minimum available data calculated in all the output channels.
*/
static unsigned ttp_output_buffers_data(TIMED_PLAYBACK* timed_pb)
{
    unsigned retval = UINT_MAX;
    unsigned current;
    unsigned channel;
    for (channel = 0;channel < timed_pb->used_channels; channel++)
    {
        current = cbuffer_calc_amount_data_in_words(timed_pb->out_buffers[channel]);
        if (current < retval)
        {
            retval = current;
        }
    }
    return retval;
}

/**
* \brief Calculates the available data in the input buffers.
*
* \param timed_pb - Pointer to the timed_pb playback instance
*
* \return Returns the minimum available space calculated in all the output channels.
*/
static unsigned ttp_input_buffers_data(TIMED_PLAYBACK* timed_pb)
{
    unsigned retval = UINT_MAX;
    unsigned current;
    unsigned channel;
    for (channel = 0;channel < timed_pb->used_channels; channel++)
    {
        current = cbuffer_calc_amount_data_in_words(timed_pb->in_buffers[channel]);
        if (current < retval)
        {
            retval = current;
        }
    }

    /* Check the available octets in the metadata because it can be different if
     * the timed playback module (which is running in interrupt level) interrupts an
     * operator between the cbuffer updated and the metadata update. */
    current = buff_metadata_available_octets(timed_pb->in_buffers[0]) / OCTETS_PER_SAMPLE;
    if (current < retval)
    {
        retval = current;
    }

    return retval;
}

#if defined(DBG_USE_CBUFFER) || !defined(USE_SILENCE_INSERTION)
/**
* \brief Function copy samples in all channels.
*
* \param timed_pb - Pointer to the timed_pb playback instance
* \param silence_samples - samples to copy.
*/
static void ttp_buffer_copy(TIMED_PLAYBACK* timed_pb, unsigned samples_to_copy)
{
    unsigned channel;
    for (channel = 0;channel < timed_pb->used_channels; channel++)
    {
        cbuffer_copy(timed_pb->out_buffers[channel], timed_pb->in_buffers[channel], samples_to_copy);
    }

    /* The output buffer of the cbops chain has been modified. Reset the chain. */
    ttp_reset_cbops_chain(timed_pb);
}
#endif

#ifdef USE_SILENCE_INSERTION
/**
* \brief Function discards samples in all channels.
*
* \param timed_pb - Pointer to the timed_pb playback instance
* \param samples_to_discard - samples to discard.
*/
static void ttp_buffer_discard(TIMED_PLAYBACK* timed_pb, unsigned samples_to_discard)
{
    unsigned channel;
    for (channel = 0;channel < timed_pb->used_channels; channel++)
    {
        cbuffer_discard_data(timed_pb->in_buffers[channel], samples_to_discard);
    }

    /* The output buffer of the cbops chain has been modified. Reset the chain. */
    ttp_reset_cbops_chain(timed_pb);
}
#endif

/**
 * \brief Calculated the offset (in us) of the tag
 *
 * \param timed_pb Pointer to the timed_pb playback instance
 * \param tag_timestamp timestamp of the latest tag.
 *
 * \return Returns the delta time error of the tag timestamp.
 */
static TIME_INTERVAL get_timestamp_error(TIMED_PLAYBACK *timed_pb, unsigned tag_timestamp)
{
    TIME current_time, last_sample_play_time;
    TIME_INTERVAL buffer_empty_period;
    unsigned samples_out_buff;

    /* Check the tag status */
    current_time = hal_get_time();
    samples_out_buff = ttp_output_buffers_data(timed_pb);

    /* Calculate the time when the last sample in the buffer will be played. */
    buffer_empty_period = convert_samples_to_time(samples_out_buff, timed_pb->sample_rate);
    last_sample_play_time = time_add(current_time, buffer_empty_period + timed_pb->endpoint_delay);

    /* Calculate the error, which is the difference between the metadata
     * timestamp and the time when the data will be played. */
    return time_sub(tag_timestamp, last_sample_play_time);
}

/**
 * \brief Gets the status of the current error.
 *
 * \param error - The tag calculated error.
 * \param error_limit Error limit.
 *
 * \return Returns the status of the error.
 */
static tag_timestamp_status get_timestamp_status(TIME_INTERVAL error, int error_limit)
{
    /* Check the error state. */
    if ((error + error_limit) < 0)
    {
        return TAG_LATE;
    }
    else if (error > error_limit)
    {
        return TAG_EARLY;
    }
    else
    {
        return TAG_ON_TIME;
    }
}


/**
 * \brief Checks if the input buffer read index is in sync with the metadata read offset.
 *        Function panics if the two is out of sync.
 *
 * \param timed_pb Pointer to the timed_pb playback instance
 */
#ifdef TTP_BUFFER_DEBUG
static void check_input_buff_read_index_validity(TIMED_PLAYBACK *timed_pb)
{
    static unsigned int read_index = 0;
    static unsigned meta_index = 0;
    tCbuffer *metadata_buff = timed_pb->in_buffers[0];

    read_index = (uintptr_t)metadata_buff->read_ptr - (uintptr_t)metadata_buff->base_addr;

    /*Convert the adresses to words. */
    read_index = read_index >> LOG2_ADDR_PER_WORD;

    /* Convert the words to usable octets. */
    read_index = read_index * OCTETS_PER_SAMPLE;

    meta_index = buff_metadata_get_read_offset(metadata_buff);

    PL_ASSERT(meta_index == read_index);
}
#else
#define check_input_buff_read_index_validity(x)                 ((void)0)
#endif /* TTP_BUFFER_DEBUG */


/**
 * \brief Resets the cbops chain. The cbops manager stores local variables about the
 *    input/output buffers. Whenever one those buffers change the chain needs resetting.
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 */
static void ttp_reset_cbops_chain(TIMED_PLAYBACK* timed_pb)
{
    patch_fn_shared(timed_playback);
    cbops_sra_reset(timed_pb->rate_adjustment, timed_pb->nr_of_channels);
    /* TODO Also reset DC remove cbops */

    cbops_mgr_buffer_reinit(timed_pb->cbops_manager);
}


/**
 * \brief Function makes a rate adjustment.
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 * \param silence_samples - Samples to rate adjust.
 */
static void  timed_playback_do_rate_adjust(TIMED_PLAYBACK* timed_pb, unsigned samples_to_play)
{
#ifdef TTP_BUFFER_DEBUG
    unsigned amount_of_space_before = ttp_output_buffers_space(timed_pb);
    unsigned amount_of_space_after;
#endif

#ifdef DBG_USE_CBUFFER
    ttp_buffer_copy(timed_pb, samples_to_play );
#else
    patch_fn_shared(timed_playback);

    if (timed_pb->do_hw_warp)
    {
        timed_pb->rate_adjust(timed_pb->rm_data, timed_pb->pid_state.warp);
    }
    else
    {
        /* Make sure the sra is not in passthrough mode  */
        cbops_mgr_rateadjust_passthrough_mode(timed_pb->cbops_manager, FALSE);

        /* Set the SRA */
        cbops_sra_set_rate_adjust(timed_pb->rate_adjustment,timed_pb->nr_of_channels, timed_pb->pid_state.warp);
    }
    /* ... and finally, run the cbops chain. */
    cbops_mgr_process_data(timed_pb->cbops_manager, samples_to_play);
#endif

#ifdef TTP_BUFFER_DEBUG
    amount_of_space_after = ttp_output_buffers_space(timed_pb);
    TTP_DBG_MSG5("TTP Playback sra 0x%08x: samples to copy = %4d, with sra = 0x%08x, space before = %4d, space after = %4d",
            (uintptr_t)timed_pb, samples_to_play, timed_pb->pid_state.warp,
            amount_of_space_before,amount_of_space_after);
#endif

    /* Update the metadata. */
    reframe_consume(&timed_pb->tag_reframe, samples_to_play * OCTETS_PER_SAMPLE);
}

/**
 * \brie Calculates the samples to the next tag. Should only be called when there is an
 *    available tag in the input buffer (reframe_tag_available returns true).
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 */
static unsigned samples_to_next_tag(TIMED_PLAYBACK *timed_pb)
{
    tCbuffer* metadata_buff = timed_pb->in_buffers[0];
    unsigned remaining_octets;
    unsigned buffer_size = buff_metadata_get_buffer_size(metadata_buff);

    /* Calculate the octets advanced in the buffer since the last tag.*/
    remaining_octets =  reframe_tag_index(&timed_pb->tag_reframe) - buff_metadata_get_read_offset(metadata_buff);
    if (remaining_octets >= buffer_size)
    {
        remaining_octets = remaining_octets + buffer_size;
    }
    return remaining_octets / OCTETS_PER_SAMPLE;
}

/**
 * \brie Continue to consume the previous on time tag with the already calculated sra.
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 *
 * \return True, if there is more data to process. False otherwise.
 */
static bool continue_to_consume_on_time_tag(TIMED_PLAYBACK *timed_pb)
{

    /* The SRA is recalculated at the beginning of each tag. This function consumes
     * (continues to consume) the last tag and applies the already calculated SRA on it.
     * First the function calculates how much samples are remaining from the last tag,
     * validates it against the output space and than do sra on it*/
    int samples_to_play;
    unsigned space_out_sample;
    /* Remaining octets from the previous metadata tag.*/
    unsigned available_tag_samples;

    patch_fn_shared(timed_playback);

    unsigned in_data = ttp_input_buffers_data(timed_pb);

    if (!reframe_tag_available(&timed_pb->tag_reframe))
    {
        /* No tag in the input buffer so all the available data should belong to the last tag. */
        available_tag_samples = in_data;
        if (available_tag_samples == 0)
        {
            return FALSE;
        }
    }
    else
    {
        available_tag_samples = samples_to_next_tag(timed_pb);
        available_tag_samples = MIN(available_tag_samples, in_data);
        if (available_tag_samples == 0)
        {
            /* We are at the beginning of the tag. Make sure that only one tag is consumed
             * by setting the samples to tag to the the tag length or the available data,
             * whichever is smaller. */
            available_tag_samples = MIN((reframe_tag_length(&timed_pb->tag_reframe) / OCTETS_PER_SAMPLE), in_data);
        }
    }

    space_out_sample = ttp_output_buffers_space(timed_pb);
    /* Leave some space in case if the sra kicks in. */
    space_out_sample = SUBTRACT_SAMPLES(space_out_sample, SRA_SAMPLES);
    if (space_out_sample == 0)
    {
        return FALSE;
    }

    /* Convert the octets to samples. */
    samples_to_play = MIN(available_tag_samples, space_out_sample);

    if (samples_to_play == 0)
    {
        TTP_WARN_MSG("Timed playback: continue_to_consume_on_time_tag: samples_to_play==0");
        return FALSE;
    }

    /* samples_to_play is bigger than 0 because none of the three factors are 0. */
    if(timed_pb->current_tag.is_void)
    {
#ifdef USE_SILENCE_INSERTION
        timed_playback_insert_silence(timed_pb, samples_to_play);
        ttp_buffer_discard(timed_pb, samples_to_play);
#else
        ttp_buffer_copy(timed_pb, samples_to_play );
#endif
        /* Update the metadata. */
        reframe_consume(&timed_pb->tag_reframe, samples_to_play * OCTETS_PER_SAMPLE);
    }
    else
    {
        /* samples_to_play is bigger than 0 because none of the three factors are 0. */
        timed_playback_do_rate_adjust(timed_pb, samples_to_play);
    }

    /* Check if we can rerun. */
    if (samples_to_play < space_out_sample)
    {
        /* there is place for the following tag. */
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

/**
 * \brie Continue to consume the previous late tag with the current given error.
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 * \param error - The current error of the tag.
 *
 * \return True, if there is more data to process. False otherwise.
 */
static bool continue_to_consume_late_tag(TIMED_PLAYBACK *timed_pb, TIME_INTERVAL error)
{

    /*  This function continues to consume the last late tag. */
    int samples_to_drop;
    /* Remaining octets from the previous metadata tag.*/
    unsigned samples_to_tag, input_data;

    patch_fn_shared(timed_playback);

    /* Convert the error into samples. */
    samples_to_drop = convert_time_to_samples(-error,
            timed_pb->sample_rate);

    if (!reframe_tag_available(&timed_pb->tag_reframe))
    {
        /* Calculate the available data in the input buffer. */
        input_data = ttp_input_buffers_data(timed_pb);
        if (input_data == 0)
        {
            return FALSE;
        }
        if (samples_to_drop >= input_data)
        {
            /* Min = samples_to_tag. Remove the complete tag and run again. */
            timed_playback_discard_samples(timed_pb, input_data);

            /* Advance the time to play and the length for the tag.*/
            timed_pb->current_tag.playback_time = ttp_get_next_timestamp(timed_pb->current_tag.playback_time,
                    input_data, timed_pb->sample_rate, timed_pb->current_tag.sp_adjust);

            return FALSE;
        }
    }
    else
    {
        /* Calculate how many sample can we consume until the next tag is read. */
        samples_to_tag = samples_to_next_tag(timed_pb);
        if (samples_to_tag == 0)
        {
            TTP_WARN_MSG("Timed playback: We are at the beginning of the tag when continuing consuming a tag. This shouldn't happen.");
            return FALSE;
        }
        if (samples_to_drop >= samples_to_tag)
        {
            /* Min = samples_to_tag. Remove the complete tag and run again. */
            timed_playback_discard_samples(timed_pb, samples_to_tag);

            /* Advance the time to play and the length for the tag.*/
            timed_pb->current_tag.playback_time = ttp_get_next_timestamp(timed_pb->current_tag.playback_time,
                    samples_to_tag, timed_pb->sample_rate, timed_pb->current_tag.sp_adjust);

            return TRUE;
        }
    }

    /* Min = samples_to_drop. Drop part of the tag. */
    timed_playback_discard_samples(timed_pb, samples_to_drop);

    /* The rest of the tag is on time with no error. */
    timed_pb->current_tag.status = TAG_ON_TIME;
    return timed_playback_tag_on_time(timed_pb, 0);

}


/**
 * \brie Continue to consume the previous tag.
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 *
 * \return True, if there is more data to process. False otherwise.
 */
static bool continue_to_consume_tag(TIMED_PLAYBACK *timed_pb)
{
    patch_fn_shared(timed_playback);


    if(timed_pb->current_tag.is_void)
    {
        /* Void tag is always on time. */
        return continue_to_consume_on_time_tag(timed_pb);
    }

    if (timed_pb->current_tag.status == TAG_LATE)
    {
        /* Recalculate the error. */
        TIME_INTERVAL error;
        tag_timestamp_status status;

        /* Check the previous tag error. */
        error = get_timestamp_error(timed_pb, timed_pb->current_tag.playback_time);

        /* Get the tag status. */
        status = get_timestamp_status(error, timed_pb->error_limit);

        /* Early packets shouldn't be consumed until they are on time. More importantly
         * switching form late to early should never happen*/
        PL_ASSERT(status != TAG_EARLY);

        DBG_PRINT_LATE_ERROR_AND_STATUS(timed_pb, error, status);

        if (status == TAG_LATE)
        {
            return continue_to_consume_late_tag(timed_pb, error);
        }

        /* else, continue to consume the on time tag below. */
        timed_pb->current_tag.status = TAG_ON_TIME;
    }

    return continue_to_consume_on_time_tag(timed_pb);
}




/**
 * \brief Function discards samples in all channels.
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 * \param samples_to_discard - Samples to discard.
 */
static void  timed_playback_discard_samples(TIMED_PLAYBACK* timed_pb, unsigned samples_to_discard)
{
#ifdef TTP_BUFFER_DEBUG
    unsigned amount_of_data_before = ttp_input_buffers_data(timed_pb);
    unsigned amount_of_data_after;
#endif

    unsigned channel;

    patch_fn_shared(timed_playback);

    TTP_WARN_MSG4("TTP sample discard, playback time = %d samples = %d spa = %d error = %d",
        timed_pb->current_tag.playback_time, samples_to_discard, timed_pb->current_tag.sp_adjust, timed_pb->current_tag.error);

    for (channel = 0;channel < timed_pb->used_channels; channel++)
    {
        cbuffer_discard_data(timed_pb->in_buffers[channel], samples_to_discard);
    }

    /* The input buffer of the cbops chain has been modified. Reset the chain. */
    ttp_reset_cbops_chain(timed_pb);

#ifdef TTP_BUFFER_DEBUG
    amount_of_data_after = ttp_input_buffers_data(timed_pb);
    TTP_DBG_MSG4("TTP Playback drop 0x%08x: samples dropped = %4d, data before = %4d, data after = %4d",
            (uintptr_t)timed_pb, samples_to_discard,
            amount_of_data_before, amount_of_data_after);
    PL_ASSERT(amount_of_data_before > amount_of_data_after);
#endif

    /* Update the metadata. */
    reframe_consume(&timed_pb->tag_reframe, samples_to_discard * OCTETS_PER_SAMPLE);
}

/**
 * \brief Function handles when the metadata tag timestamp is later than expected.
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 * \param error - The calculated error of the metadata timestamp.
 * \param tag - The metadata tag read from the beginning of the buffer.
 *
 * \return True, if there is more data to process. False otherwise.
 */
static bool timed_playback_tag_late(TIMED_PLAYBACK* timed_pb, TIME_INTERVAL error)
{
    unsigned samples_to_drop;
    unsigned tag_samples = reframe_tag_length(&timed_pb->tag_reframe) / OCTETS_PER_SAMPLE;
    unsigned input_data;
    unsigned minimum;


    patch_fn_shared(timed_playback);

    /* Reset the PID controller, the rate adjustment and set the error limit lower now
     * that samples will be discarded.*/
    timed_pb->error_limit = WARP_TIGHT_LIMIT;
    timed_reset_pid_controller(&timed_pb->pid_state);
    ttp_reset_cbops_chain(timed_pb);

    /* Calculate the available data in the input buffer. */
    input_data = ttp_input_buffers_data(timed_pb);

    /* Exit early if no data. */
    if(input_data == 0)
    {
        return FALSE;
    }

    /* Check that the error is negative. */
    PL_ASSERT(error < 0);

    /* Convert the error into samples. */
    samples_to_drop = convert_time_to_samples(ABS(error),
            timed_pb->sample_rate);

    minimum = MIN(tag_samples, MIN(samples_to_drop, input_data));

    timed_playback_discard_samples(timed_pb, minimum);

    if (minimum == tag_samples)
    {
        /* The complete tag was removed. If there is more data run again. */
        if (input_data > minimum)
        {
            return TRUE;
        }
    }
    else if (minimum == samples_to_drop)
    {
        /* Only part of the tag was dropped. The rest of the tag is on time with no error. */
        if (tag_samples > minimum)
        {
            timed_pb->current_tag.status = TAG_ON_TIME;
            return timed_playback_tag_on_time(timed_pb, 0);
        }
    }
    else /* (minimum == input_data) */
    {
        /* Only part of the tag was dropped. The rest will be dropped later when the data
         * arrives. */
        if (tag_samples > minimum)
        {
            /* Advance the time to play of the tag for future error calculation. */
            timed_pb->current_tag.playback_time = ttp_get_next_timestamp(timed_pb->current_tag.playback_time,
                    minimum, timed_pb->sample_rate, timed_pb->current_tag.sp_adjust);
        }
    }

    return FALSE;
}

/**
 * \brief Function inserts silence in all channels.
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 * \param silence_samples - Silence samples to insert.
 */
static void  timed_playback_insert_silence(TIMED_PLAYBACK* timed_pb, unsigned silence_samples)
{
#ifdef TTP_BUFFER_DEBUG
    unsigned amount_of_space_before = ttp_output_buffers_space(timed_pb);
    unsigned amount_of_space_after;
#endif

    unsigned channel;

    patch_fn_shared(timed_playback);
    for (channel = 0;channel < timed_pb->used_channels; channel++)
    {
        cbuffer_block_fill(timed_pb->out_buffers[channel], silence_samples, 0);
    }

    /* The output buffer of the cbops chain has been modified. Reset the chain. */
    ttp_reset_cbops_chain(timed_pb);

#ifdef TTP_BUFFER_DEBUG
    amount_of_space_after = ttp_output_buffers_space(timed_pb);
    TTP_DBG_MSG4("TTP Playback silence 0x%08x:silence samples inserted = %4d, space before = %4d, space after = %4d",
            (uintptr_t)timed_pb, silence_samples,
            amount_of_space_before, amount_of_space_after);
    PL_ASSERT(amount_of_space_before > amount_of_space_after);
#endif
}


/**
 * \brief Function handles when the metadata tag timestamp is earlier than expected.
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 * \param error - The calculated error of the metadata timestamp.
 * \param tag - The metadata tag read from the beginning of the buffer.
 *
 * \return True, if there is more data to process. False otherwise.
 */
static bool timed_playback_tag_early( TIMED_PLAYBACK* timed_pb, TIME_INTERVAL error)
{
    int silence_samples;
    unsigned space_out_sample;

    patch_fn_shared(timed_playback);

    /* Reset the PID controller, the rate adjustment and set the error limit lower now
     * that silence will be inserted.*/
    timed_pb->error_limit = WARP_TIGHT_LIMIT;
    timed_reset_pid_controller(&timed_pb->pid_state);
    timed_pb->later_tag_count = 0;

    /* Calculate the space in the output buffer. */
    space_out_sample = ttp_output_buffers_space(timed_pb);

    /* insert silence if frame timed_pb is too far in future
     convert microseconds into samples */
    silence_samples = convert_time_to_samples(error, timed_pb->sample_rate);
    silence_samples = MIN(space_out_sample, silence_samples);

    if (silence_samples > 0)
    {
        /* Insert silence samples. */
        timed_playback_insert_silence(timed_pb, silence_samples);
    }

    /* Try to copy next frame if the output is not full */
    return (SUBTRACT_SAMPLES(space_out_sample, SRA_SAMPLES) > silence_samples);
}

/**
 * Function avoids buffer wrap by inserting silences when there is not enough data until
 * the next run. Note: silence insertions should only happen when the timestamps is
 * unachievable or during the startup.
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 */
static void avoid_buffer_wrap(TIMED_PLAYBACK* timed_pb)
{
    int silence_samples;
    TIME_INTERVAL output_buff_time;
    unsigned output_buf_samples, space_out_sample;

    /* Check if there is enough data in the input buffers until the next run. */
    output_buf_samples = ttp_output_buffers_data(timed_pb);
    output_buff_time = convert_samples_to_time(output_buf_samples,
            timed_pb->sample_rate);

    /* insert silence if frame timed_pb is too far in future
     convert microseconds into samples */

    if (output_buff_time < timed_pb->period + PERIOD_ERROR)
    {
        silence_samples = convert_time_to_samples(
                time_add(time_sub(timed_pb->period, output_buff_time),
                        PERIOD_ERROR), timed_pb->sample_rate);
    }
    else
    {
        return;
    }

    /* Calculate the space in the output buffer. */
    space_out_sample = ttp_output_buffers_space(timed_pb);
    silence_samples = MIN(space_out_sample, silence_samples);

    TTP_DBG_MSG4("TTP silence insertion, playback time = %d  samples = %d spa = %d error = %d",
        timed_pb->current_tag.playback_time, silence_samples, timed_pb->current_tag.sp_adjust, timed_pb->current_tag.error);

    if (silence_samples > 0)
    {
        /* Insert silence samples. */
        timed_playback_insert_silence(timed_pb, silence_samples);
    }

}


/**
 * \brief Handle the on time case.
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 * \param error - The calculated error of the metadata timestamp.
 *
 * \return True, if there is more data to process. False otherwise.
 */
static bool timed_playback_tag_on_time(TIMED_PLAYBACK* timed_pb, TIME_INTERVAL error)
{
    patch_fn_shared(timed_playback);
    /* warp over a larger time-to-play error now we're copying audio frames */
    timed_pb->error_limit = WARP_LOOSE_LIMIT;
    timed_run_pid_controller(&timed_pb->pid_state, &timed_pb->pid_params, error);
    timed_pb->later_tag_count = 0;
    return continue_to_consume_on_time_tag(timed_pb);
}

/**
 * \brief Saves the last metadata tag from the input buffer. This is important because
 *      the PID controller only runs when a metadata is at the front of the buffer.
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 * \param tag - The metadata tag read from the input buffer.
 * \param error - The calculated error of the metadata timestamp.
 */
static void timed_playback_save_tag(TIMED_PLAYBACK* timed_pb, TIME_INTERVAL error,
        tag_timestamp_status status)
{
    /* Save the tag for the future reference. */
    timed_pb->current_tag.length = reframe_tag_length(&timed_pb->tag_reframe);
    timed_pb->current_tag.index = reframe_tag_index(&timed_pb->tag_reframe);
    timed_pb->current_tag.timestamp = reframe_tag_timestamp(&timed_pb->tag_reframe);
    timed_pb->current_tag.playback_time = reframe_tag_playback_time(&timed_pb->tag_reframe);
    timed_pb->current_tag.sp_adjust = reframe_sp_adjust(&timed_pb->tag_reframe);
    timed_pb->current_tag.error = error;
    timed_pb->current_tag.status = status;

    /* Share the playback time with the plotter. */
    TTP_PLOTTER_MACRO(timed_palyback_pb_time = timed_pb->current_tag.playback_time);
    /* playback time = timestamp - error offset. =>
     * error offset  = timestamp - playback time*/
    TTP_PLOTTER_MACRO(timed_palyback_err_offset = timed_pb->current_tag.timestamp - timed_pb->current_tag.playback_time);
    /* Share the timed_palyback_sp_adjust  with the plotter. */
    TTP_PLOTTER_MACRO(timed_palyback_sp_adjust = reframe_sp_adjust(&timed_pb->tag_reframe));

    TTP_DBG_MSG5("TTP Playback 0x%08x: tag_index = 0x%08x, tag_length = 0x%08x, tag_timestamp = 0x%08x, playback_time = 0x%08x ",
            (uintptr_t)timed_pb, timed_pb->current_tag.index, timed_pb->current_tag.length, timed_pb->current_tag.timestamp, timed_pb->current_tag.playback_time);
}


/****************************************************************************
Public Function Definitions
*/

/*
 * Convert Samples to TIME
 */
TIME_INTERVAL convert_samples_to_time(unsigned samples, unsigned sample_rate)
{
    return (TIME_INTERVAL)(((uint64)samples * 1000000)/sample_rate) ;
}


/*
 * Convert TIME to Samples
 */
 unsigned convert_time_to_samples(TIME_INTERVAL time, unsigned sample_rate)
{
    return (unsigned) (((uint48)time * sample_rate) / 1000000);
}


/*
 * timed_playback_create
 */
TIMED_PLAYBACK* timed_playback_create(void )
{
    return xzpnew(TIMED_PLAYBACK);
}

/*
 * timed_playback_destroy
 */
void timed_playback_destroy(TIMED_PLAYBACK* timed_playback)
{
    pdelete(timed_playback);
}

/*
 * timed_playback_init
 */
bool timed_playback_init(TIMED_PLAYBACK *timed_pb, cbops_mgr *cbops_manager, unsigned sample_rate,
                             TIME_INTERVAL period, unachievable_latency_callback cback, unsigned con_id, unsigned ep_id, unsigned delay)
{
    cbops_graph *head;
    unsigned channel_count;

    patch_fn_shared(timed_playback);

    /* Sanity check.*/
    PL_ASSERT(timed_pb != NULL);
    PL_ASSERT(cbops_manager != NULL);

    /* Save the cbops manager */
    timed_pb->cbops_manager = cbops_manager;

    timed_pb->endpoint_delay = delay;

    /* Check if all the necessary (discard, rate adjust and underrun) cbops operators
     * are present.
     * TODO: Handle when rate adjust is replaced by a hardware wrap. */
    head = cbops_manager->graph;

    PL_ASSERT(head != NULL);

    /* Check if we have at least one channel and if the number of the outputs are the same.
     * Note: This returns total inputs in the cbops graph, used as well unused */
    timed_pb->nr_of_channels = cbops_get_num_inputs(head);
    PL_ASSERT(timed_pb->nr_of_channels >= 1);
    PL_ASSERT(timed_pb->nr_of_channels == cbops_get_num_outputs(head));

    cbops_mgr_get_buffer_info(cbops_manager, timed_pb->nr_of_channels, timed_pb->in_buffers, timed_pb->out_buffers);

    /* Check the number of channels and make sure they have the same metadata*/
    channel_count = 0;
    while((timed_pb->in_buffers[channel_count] != NULL) && (timed_pb->out_buffers[channel_count] != NULL))
    {
        if (channel_count >0)
        {
            PL_ASSERT(timed_pb->in_buffers[channel_count]->metadata == timed_pb->in_buffers[channel_count]->metadata);
        }
        channel_count++;
    }
    timed_pb->used_channels = channel_count;

    /* Before modifying the cbops set the reframe module to reframe the timstaps in
     * every 512 samples */
    reframe_init(&timed_pb->tag_reframe, timed_pb->in_buffers[0], TIMED_PLAYBACK_REFRAME_PERIOD, sample_rate);

    /* Remove the underrun and the discard cbops from the chain.
     * TODO This logic need changing if timed playback will be used by other operators */
    if (!cbops_mgr_remove(timed_pb->cbops_manager, CBOPS_DISCARD | CBOPS_UNDERRUN,
            timed_pb->nr_of_channels, timed_pb->in_buffers, timed_pb->out_buffers, TRUE))
    {
        return FALSE;
    }

    /* Check if the discard and underrun was successfully removed. */
    PL_ASSERT(find_cbops_op(head, cbops_discard_table) == NULL);
    PL_ASSERT(find_cbops_op(head, cbops_underrun_comp_table) == NULL);

    if ((timed_pb->rate_adjustment = find_cbops_op(head, cbops_rate_adjust_table)) == NULL)
    {
        return FALSE;
    }

    if ((timed_pb->dc_remove_op = find_cbops_op(head, cbops_dc_remove_table)) == NULL)
    {
        return FALSE;
    }

    /* Set the default initial parameters for the  pid controller. For 24 bit machines
     * the fractional values needs shifting to the right to fit in the word. This operation
     * involves losing some precision. */
    timed_set_pid_controller_default_settings(&timed_pb->pid_params);

    /* Save the sample rate and the period of the timed playback module.*/
    timed_pb->sample_rate = sample_rate;
    timed_pb->period = period;

    /* Set the error limit to tight. */
    timed_pb->error_limit = WARP_TIGHT_LIMIT;

    /* Set the startup flag and the late tag count. */
    timed_pb->start = TRUE;
    timed_pb->later_tag_count = 0;

    /* initialise unachievable latency callback structure for unsolicited message */
    timed_pb->cback.callback = cback;
    timed_pb->cback.con_id = con_id;
    timed_pb->cback.ep_id = ep_id;

    return TRUE;
}

/*
 * timed_playback_enable_hw_warp
 */
void timed_playback_enable_hw_warp(TIMED_PLAYBACK *timed_pb, void *data, void (*rate_adjust)(void *data, int32 adjust_val))
{
    PL_ASSERT(timed_pb != NULL);
    timed_pb->do_hw_warp = TRUE;
    timed_pb->rm_data = data;
    timed_pb->rate_adjust = rate_adjust;
    /* Make sure the SRA is in passthrough mode  */
    cbops_mgr_rateadjust_passthrough_mode(timed_pb->cbops_manager, TRUE);
}

/*
 * timed_playback_set_delay
 */
void timed_playback_set_delay(TIMED_PLAYBACK *timed_pb, unsigned delay)
{
    PL_ASSERT(timed_pb != NULL);
    timed_pb->endpoint_delay = delay;
}


/*
 * timed_playback_timed_frames_target
 */
void timed_playback_run(TIMED_PLAYBACK *timed_pb)
{
    bool rerun = FALSE;
    tCbuffer *metadata_buff;
    REFRAME *tag_reframe = &timed_pb->tag_reframe;

    patch_fn_shared(timed_playback);

#ifdef TTP_DEBUG
    DYN_PROFILER_START("timed_playback_run",(uintptr_t)timed_pb);
#endif

    /* Sanity check. */
    PL_ASSERT(timed_pb != NULL);

    /* All the input buffers have the same metadata. */
    metadata_buff = timed_pb->in_buffers[0];

    do{
        /* Let the reframe library search for void tags. */
        reframe_check_tags(tag_reframe);

        /* Get the next from the input buffer using the reframe module. */
        if (!reframe_tag_available(tag_reframe))
        {
            /* Check if we are still in startup */
            if (timed_pb->start)
            {
                /* Silence will be inserted after the do..while() loop in avoid_buffer_wrap. */
                rerun = FALSE;
            }
            else
            {
                /* no metadata, but we had at least one before so continue to consume the previous tag. */
                continue_to_consume_tag(timed_pb);
                rerun = FALSE;
            }
        }
        else
        {

            if (reframe_tag_index(tag_reframe) != buff_metadata_get_read_offset(metadata_buff))
            {
                /* no metada at the beginning, continue consume the rest of the previous tag. */
                rerun = continue_to_consume_tag(timed_pb);
            }
            else if (!reframe_tag_is_void(tag_reframe))
            {
                /* When the metadata tag is at the beginning of the buffer the error
                 * will be recalculated. */
                TIME_INTERVAL error;
                tag_timestamp_status status;
                unsigned tag_timestamp = reframe_tag_playback_time(tag_reframe);

                if (timed_pb->start)
                {
                    TTP_WARN_MSG1("TTP playback start, timestamp = %d", tag_timestamp);
                }

                /* exit from startup mode*/
                timed_pb->start = FALSE;
                timed_pb->current_tag.is_void = FALSE;

                /* Calculate the tag error */
                error = get_timestamp_error(timed_pb, tag_timestamp);

                /* Share the error with the plotter. */
                TTP_PLOTTER_MACRO(timed_palyback_error = error);

                /* Get the tag status. */
                status = get_timestamp_status(error, timed_pb->error_limit);

                /* If the packet is late (the error in this case is negative) and
                 * the error increased (because the error is negative a smaller check
                 * is used) increment the late tag count. Continuous late packets and
                 * increasing error is a sign of unachievable latency. */
                if ((status == TAG_LATE) && (timed_pb->current_tag.error > error))
                {
                    timed_pb->later_tag_count++;
                    /* It is enough to generate the fault once. */
                    if (timed_pb->later_tag_count == UNACHIEVABLE_LATENCY_LIMIT)
                    {
                        fault_diatribe(FAULT_UNACHIEVABLE_LATENCY, error);
                        /* also callback for sending an unsolicited message */
                        timed_pb->cback.callback(timed_pb->cback.con_id, timed_pb->cback.ep_id, -error);
                    }
                }

                /* Save the tag for future reference. */
                timed_playback_save_tag(timed_pb, error, status);

                switch (status)
                {
                    case TAG_LATE:
                        rerun = timed_playback_tag_late(timed_pb, error);
                        break;
                    case TAG_EARLY:
                        rerun = timed_playback_tag_early(timed_pb, error);
                        break;
                    case TAG_ON_TIME:
                        rerun = timed_playback_tag_on_time(timed_pb, error);
                        break;
                    default:
                        PL_ASSERT(FALSE);
                    break;
                }

                DBG_PRINT_ERROR_AND_STATUS(timed_pb, error,timed_pb->pid_state.warp, status);

            }
            else
            {
                /* exit from startup mode*/
                timed_pb->start = FALSE;
                timed_pb->current_tag.is_void = TRUE;
                /* It is a void timestamp. just copy the data without updating any
                 * internal state. */
                rerun = continue_to_consume_tag(timed_pb);

            }

        }

        /* Check if the metadata and the buffer is still in sync. */
        check_input_buff_read_index_validity(timed_pb);
    }while (rerun);

    /* Insert silence if there is not enough data until the next run. */
    avoid_buffer_wrap(timed_pb);

#ifdef TTP_DEBUG
    DYN_PROFILER_STOP("timed_playback_run",(uintptr_t)timed_pb);
#endif
}

/**
 * \brief Runs the PID controller for the given error.
 *
 * \param pid_state Pointer to the pid_controller_state instance
 * \param pid_params Pointer to the pid_controller_settings instance
 * \param error Time difference between the timestamp time and the expected playback time.
 */
void timed_run_pid_controller(pid_controller_state *pid_state, const pid_controller_settings *pid_params, TIME_INTERVAL error)
{
    int tmp, error_us, avg_error, min_error, max_error;

    patch_fn_shared(timed_playback);
    /* calculate middle of min and max error */
    error_us = error << ERROR_SHIFT_AMOUNT;
    avg_error = (pid_state->min_error + pid_state->max_error) / 2;

    /* decay min error */
    min_error = pid_state->min_error - avg_error;
    min_error = frac_mult(min_error, pid_params->error_decay);
    pid_state->min_error = min_error + avg_error;

    /* decay max error */
    max_error = pid_state->max_error - avg_error;
    max_error = frac_mult(max_error, pid_params->error_decay);
    pid_state->max_error = max_error + avg_error;

    /* if error is less than min_error, move min_error towards current error.  error_grow
     * determines how fast min_error moves */
    if (error_us < pid_state->min_error)
    {
        int min_diff = pid_state->min_error - error_us;
        pid_state->min_error -= frac_mult(min_diff, pid_params->error_grow);
    }

    /* if error is greater than max_error, move max_error towards current error.  error_grow
     * determines how fast max_error moves */
    if (error_us > pid_state->max_error)
    {
        int max_diff = error_us - pid_state->max_error;
        pid_state->max_error += frac_mult(max_diff, pid_params->error_grow);
    }

    /* calculate mid-point between min and max */
    pid_state->avg_error = (pid_state->min_error + pid_state->max_error) / 2;

    /* calculate p_term and clamp within limits */
    tmp = frac_mult(pid_state->avg_error, pid_params->p_factor);
    pid_state->warp_p_term = CLAMP(tmp, -WARP_P_TERM_MAX, WARP_P_TERM_MAX);

    /* calculate i_term and clamp within limits */
    tmp = error;
    tmp = tmp + pid_state->error_i_sum;
    tmp = CLAMP(tmp, -WARP_I_ERROR_MAX, WARP_I_ERROR_MAX);
    pid_state->error_i_sum = tmp;
    pid_state->warp_i_term = frac_mult(tmp, pid_params->i_factor);

    /* sum p_term & i_term and scale to word-sized fractional */
    pid_state->warp = pid_params->warp_scale * (pid_state->warp_p_term + pid_state->warp_i_term);

    /* Share the information with the python plotter tool. */
    TTP_PLOTTER_MACRO(timed_palyback_sra = pid_state->warp);
}

/**
 * \brief Resets the PID controller internal state.
 * \param pid_params Pointer to the pid_controller_settings instance
 */
void timed_reset_pid_controller(pid_controller_state *pid_state)
{
    /* Reset the PID controller state */
    pid_state->warp = 0;
    pid_state->warp_p_term = 0;
    pid_state->warp_i_term = 0;
    pid_state->error_i_sum = 0;

    /* set min_error to maximum value so that is gets updated immediately */
    pid_state->min_error = 32767 << ERROR_SHIFT_AMOUNT;
    /* set max_error to minimum value so that is gets updated immediately */
    pid_state->max_error = -32767 << ERROR_SHIFT_AMOUNT;
}

/**
 * \brief set pid contoller default settings
 * \param pid_params Pointer to the pid_controller_settings instance
 */
void timed_set_pid_controller_default_settings(pid_controller_settings *pid_params)
{
    /* Set the default initial parameters for the  pid controller. For 24 bit machines
     * the fractional values needs shifting to the right to fit in the word. This operation
     * involves losing some precision. */
    pid_params->p_factor    = (int)(mibgetreqi32(TIMEDPLAYBACKPFACTOR) >> (32 - DAWTH));
    pid_params->i_factor    = (int)(mibgetreqi32(TIMEDPLAYBACKIFACTOR) >> (32 - DAWTH));
    pid_params->error_decay = (int)(mibgetreqi32(TIMEDPLAYBACKERRORDECLINE) >> (32 - DAWTH));
    pid_params->error_grow  = (int)(mibgetreqi32(TIMEDPLAYBACKERRORGROW) >> (32 - DAWTH));
    pid_params->warp_scale  = mibgetreqi32(TIMEDPLAYBACKWARPSCALE);
}

/**
 * \brief set pid controller proportional gain
 * \param pid_params Pointer to the pid_controller_settings instance
 * \p_factor proportional gain value
 */
void timed_set_pid_controller_pfactor(pid_controller_settings *pid_params, int p_factor)
{
    pid_params->p_factor = p_factor;
}

/**
 * \brief set pid controller integral gain
 * \param pid_params Pointer to the pid_controller_settings instance
 * \i_factor integral gain value
 */
void timed_set_pid_controller_ifactor(pid_controller_settings *pid_params, int i_factor)
{
    pid_params->i_factor = i_factor;
}


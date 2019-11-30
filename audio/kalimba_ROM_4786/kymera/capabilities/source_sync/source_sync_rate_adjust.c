/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  source_sync_rate_adjust.c
 * \ingroup  capabilities
 *
 *  src_sync operator
 *
 */
#include "source_sync_defs.h"
#include "patch/patch.h"

/****************************************************************************
Private Macro Definitions
*/

/* Definitions supporting latency adjustment.
 * There is a constant component which derives from the
 * group delay of the upsampled FIR underlying the polyphase
 * filter design in the SRA, and a variable component from
 * the current phase. Only the constant part is currently
 * calculated and adjusted.
 * Taking into account the phase part would require accessing
 * fields of the SRA state which are currently private.
 */

/* Fractional bits in RATE_ADJUST_DELAY in samples.
 * For the group delay time of a symmetric FIR, a resolution of
 * half sample periods should be enough. */
#define SRC_SYNC_RATE_ADJUST_DELAY_RESOLUTION    (1)

/* Make a fractional representation of the floating point number X
 * in type T with F fractional bits.
 * (This is defined locally because FRACTIONAL does not round,
 * and FRACTIONAL_TO_QFORMAT cannot be used to obtain representations
 * of values larger than 1.0.) */
#define MKF(X,F,T) ( (T) ( ( (X) * (1<<F) )+0.5 ) )

/* Make a fractional constant for the rate adjust delay */
#define MKF_RATE_ADJUST_DELAY(X,T) MKF(X,SRC_SYNC_RATE_ADJUST_DELAY_RESOLUTION,T)

/* Rate adjust delay in samples.
 * TODO this is currently fixed at 6.5 sample periods, matching that the
 * SRA coefficients are fixed to use the built-in "normal" quality coefficients
 * with length = 12. That may eventually become configurable to use the higher
 * quality coefficient set with length = 36.
 */
#define SRC_SYNC_RATE_ADJUST_DELAY_SAMPLES (6.5)

/* Rate adjust delay in Qm.1 samples */
#define SRC_SYNC_RATE_ADJUST_DELAY_F \
        MKF_RATE_ADJUST_DELAY(SRC_SYNC_RATE_ADJUST_DELAY_SAMPLES,unsigned)

/** Sanity limit on rate deviation */
#define SRC_SYNC_MAX_RATE_DEVIATION_FL  (0.05)
#define SRC_SYNC_MAX_RATE_DEVIATION_Q   FRACTIONAL(SRC_SYNC_MAX_RATE_DEVIATION_FL)

/** Proportion of output space to allow SRA to process at its input */
#define SRC_SYNC_MAX_SRA_SPACE_FL       (1.0 - SRC_SYNC_MAX_RATE_DEVIATION_FL)
#define SRC_SYNC_MAX_SRA_SPACE_Q        FRACTIONAL(SRC_SYNC_MAX_SRA_SPACE_FL)

/****************************************************************************
Private Constant Definitions
*/

enum
{
    /* Length of intermediate buffer in multiples of SS_PERIOD;
     * i.e. with a default SS_PERIOD=0.001, 3 corresponds to 3ms.
     * A typical SS_PERIOD is 0.5 x kick period, so the intermediate
     * buffer will be 1.5 x kick period.
     */
    SRC_SYNC_RM_BUFFER_SIZE_SS_PERIOD_MULT = 3
};

/****************************************************************************
Private Type Definitions
*/

/* This was forward declared in source_sync_defs.h */
struct src_sync_rm_state_struct
{
    /**
     * Pointer to cbops_manager that encapsulates the optional cbops
     * information for rate adjustment
     */
    struct cbops_mgr*               rm_cbops;

    /** Number of channels */
    unsigned                        num_channels;

    /** Intermediate buffers, output for rate adjustment cbops,
     * input for flow control calculations and transfer
     */
    tCbuffer**                      adjusted_buffers;

    /** The fields below mimic an endpoint */

    /* From ENDPOINT_LATENCY_CTRL_INFO */
    /**
     * Information for real endpoint latency control. This only exists for real
     * endpoints, operator endpoints are assumed not needing this now nor in future.
     */
    struct
    {
        /** Number of inserted sink silence samples that are not yet compensated for */
        unsigned silence_samples;

        /** Perceived input data block */
        unsigned data_block;

    } latency_ctrl_info;

    /* From struct endpoint_audio_state (including comments) */
    /** This is a signed value representing the difference between the rate data
     * is being produced and consumed. This value is normalised.
     */
    unsigned int                    rm_adjust_amount;

    /** Amount of compensation to perform in the sink endpoint cbops copy to
     * maintain the output buffer level on account of any rate mismatch. */
    int                             rm_diff;

    /** Data started flag (across all channels) */
    /* \note Clear during start, used by cbops */
    bool                            sync_started;

    /** Replaces monitor_threshold */
    unsigned                        block_size;

#ifdef INSTALL_METADATA
    /** Most recent TTP tag which carried an error context, or NULL */
    metadata_tag*                   ttp_template_tag;

    /** TTP is valid at start/end of transfer */
    bool                            valid_ttp_reconstruct;

    /** Last sample period correction */
    int                             last_sp_adjust;

    /** Estimated timestamp value for the start of the input block */
    TIME                            ts_start_of_input_block;

    /** TTP-based rate measurement for feedback (this sink group) */
    RATE_MEASURE_METADATA           rm_measure_feedback;

    /** Comparison state */
    RATE_COMPARE                    rm_cmp;

    /** SRA delay in usec */
    unsigned                        rm_delay_usec;
#endif /* INSTALL_METADATA  */
};

/****************************************************************************
Local Function Declarations
*/

static void src_sync_set_cbops_vals(SRC_SYNC_RM_STATE* rm_state, CBOP_VALS *vals);
static bool src_sync_is_sink_group_rm_initialised(
        SRC_SYNC_OP_DATA *op_extra_data, unsigned channel_num,
        SRC_SYNC_SINK_GROUP** p_sink_grp);

/****************************************************************************
Function Definitions
*/

/* Set cbops values */
static void src_sync_set_cbops_vals(SRC_SYNC_RM_STATE* rm_state, CBOP_VALS *vals)
{
    /* From stream_audio_hydra.c: set_endpoint_cbops_param_vals,
     * stream_audio_bluecore.c: audio_connect,
     * stream_common_a7da_kas.c: a7da_audio_common_create_cbops */

    vals->data_block_size_ptr = &(rm_state->latency_ctrl_info.data_block);

    /* The silence counter is to become the single sync group counter (when synced) */
    vals->total_inserts_ptr = &(rm_state->latency_ctrl_info.silence_samples);

    /* The rm_diff is to become the single sync group rm_diff (when synced) */
    vals->rm_diff_ptr = &(rm_state->rm_diff);

    vals->rate_adjustment_amount = &(rm_state->rm_adjust_amount);

    /* Not using insertion on this cbops chain */
    vals->insertion_vals_ptr = NULL;

    /* Delta samples is not in use on Hydra - ptr to it must be set to NULL */
    vals->delta_samples_ptr = NULL;

    /* Endpoint block size (that equates to the endpoint kick period's data amount).
     * This "arrives" later on, and is owned, possibly updated, by endpoint only.
     */
    vals->block_size_ptr = &(rm_state->block_size);

    vals->rm_headroom = SRC_SYNC_RM_HEADROOM_AMOUNT;

    vals->shift_amount = 0;

    vals->sync_started_ptr = &(rm_state->sync_started);
}

static bool src_sync_is_sink_group_rm_initialised(
        SRC_SYNC_OP_DATA *op_extra_data, unsigned channel_num,
        SRC_SYNC_SINK_GROUP** p_sink_grp)
{
    SRC_SYNC_SINK_GROUP* sink_grp =
            src_sync_find_sink_group(op_extra_data, channel_num);
    PL_ASSERT(sink_grp != NULL);

    if (p_sink_grp != NULL)
    {
        *p_sink_grp = sink_grp;
    }

    if ((op_extra_data->sink_rm_initialised_mask & (1<<channel_num)) != 0)
    {
        PL_ASSERT(sink_grp->rate_adjust_enable
                  && (sink_grp->rm_state != NULL));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bool src_sync_is_sink_rm_enabled(
        const SRC_SYNC_OP_DATA *op_extra_data, unsigned channel_num)
{
    return (op_extra_data->sink_rm_enabled_mask & (1<<channel_num)) != 0;
}

#ifdef INSTALL_METADATA
void src_sync_ra_set_rate(SRC_SYNC_OP_DATA *op_extra_data, SRC_SYNC_SINK_GROUP* sink_grp, unsigned rate)
{
    patch_fn_shared(src_sync_rate_adjust);

    if ((sink_grp != NULL) && (sink_grp->rm_state != NULL))
    {
        rate_measure_set_nominal_rate(
                &sink_grp->rm_state->rm_measure_feedback.core, rate);
        rate_compare_set_fb_sample_rate(&sink_grp->rm_state->rm_cmp, rate);
        src_sync_ra_update_delay(op_extra_data, sink_grp);
    }
}

void src_sync_ra_set_primary_rate(SRC_SYNC_OP_DATA *op_extra_data, unsigned rate)
{
    SRC_SYNC_SINK_GROUP* sink_grp;

    patch_fn_shared(src_sync_rate_adjust);

    for ( sink_grp = op_extra_data->sink_groups;
          sink_grp != NULL;
          sink_grp = next_sink_group(sink_grp))
    {
        if (sink_grp->rm_state != NULL)
        {
            rate_measure_set_nominal_rate(
                    &(op_extra_data->rm_measure_primary.core), rate);
            rate_compare_set_ref_sample_rate(&sink_grp->rm_state->rm_cmp, rate);
        }
    }
}

void src_sync_ra_update_delay(SRC_SYNC_OP_DATA *op_extra_data, SRC_SYNC_SINK_GROUP* sink_grp)
{
    /* rate_samples_to_usec has a result range of up to 2^16-1 usec,
     * so with the 1 fractional bit in rm_delay_samples, the range for
     * the delay is up to ~32ms.
     */
    sink_grp->rm_state->rm_delay_usec =
            rate_samples_frac_to_usec( SRC_SYNC_RATE_ADJUST_DELAY_F,
                                       SRC_SYNC_RATE_ADJUST_DELAY_RESOLUTION,
                                       sink_grp->rm_state
                                           ->rm_measure_feedback.core.sample_period);

    SOSY_MSG3( SRC_SYNC_TRACE_RATE_MATCH, "rm sink_g%d fs %d delay_us %d",
               sink_grp->common.idx,
               sink_grp->rm_state->rm_measure_feedback.core.sample_rate_div25 * 25,
               sink_grp->rm_state->rm_delay_usec);
}

#endif /* INSTALL_METADATA */

/** If none of the "primary clock" terminals
 * remain connected, reset the cached sp_adjust value
 */
void src_sync_check_primary_clock_connected(SRC_SYNC_OP_DATA *op_extra_data)
{
    patch_fn_shared(src_sync_rate_adjust);

    if ((op_extra_data->sources_connected == 0) &&
        ( ( op_extra_data->sinks_connected
            & ~op_extra_data->sink_rm_enabled_mask) == 0))
    {
        op_extra_data->primary_sp_adjust = 0;
    }
}

/** Turn rate adjustment on / off.
 * Do not suspend processing before calling this function.
 */
bool src_sync_rm_enact(
        SRC_SYNC_OP_DATA *op_extra_data, bool is_sink, unsigned terminal_num,
        bool enable)
{
    bool success;

    patch_fn_shared(src_sync_rate_adjust);

    if (is_sink)
    {
        SRC_SYNC_SINK_GROUP* sink_grp = NULL;

        if (src_sync_is_sink_group_rm_initialised(op_extra_data, terminal_num,
                                                  &sink_grp))
        {
            bool passthrough = !enable;

            /* Don't process in the middle of changing this */
            src_sync_suspend_processing(op_extra_data);
            success = cbops_mgr_rateadjust_passthrough_mode(
                            sink_grp->rm_state->rm_cbops, passthrough);
            src_sync_resume_processing(op_extra_data);
        }
        else
        {
            success = FALSE;
        }
        sink_grp->rm_enact = enable;
    }
    else
    {
        SOSY_MSG4( SRC_SYNC_TRACE_ALWAYS, "0x%04x rate monitor (src_%d) %d -> %d",
                    op_extra_data->id, terminal_num,
                    op_extra_data->have_primary_monitored_rate, enable);
        if (op_extra_data->have_primary_monitored_rate && !enable)
        {
            op_extra_data->primary_sp_adjust = 0;
        }
        op_extra_data->have_primary_monitored_rate = enable;
        success = TRUE;
    }
    return success;
}

bool src_sync_rm_adjust(
        SRC_SYNC_OP_DATA *op_extra_data, bool is_sink, unsigned terminal_num,
        uint32 value)
{
    SRC_SYNC_SINK_GROUP* sink_grp = NULL;

    patch_fn_shared(src_sync_rate_adjust);

    if (! is_sink)
    {
        /* This flag reflects whether the ratematch manager is "enacting"
         * on a source endpoint which is first channel of a source group.
         */
        if (op_extra_data->have_primary_monitored_rate)
        {
            /* Convert from frequency to time, approx. */
            int adjust = - (int)value;

            if (pl_abs(adjust) > SRC_SYNC_MAX_RATE_DEVIATION_Q)
            {
                unsigned print_adjust;
                char sign_adjust = src_sync_sign_and_magnitude(adjust, &print_adjust);
                unsigned percent = (print_adjust + FRACTIONAL(0.005)) / FRACTIONAL(0.01);

                L2_DBG_MSG5( "src_sync 0x%04x rm pri src_%d mon 0x%08x ~ 1 %c%d %% out of range, ignored",
                           op_extra_data->id, terminal_num, value, sign_adjust, percent);
            }
            else
            {
                op_extra_data->primary_sp_adjust = adjust;
#ifdef SOSY_VERBOSE
                unsigned print_adjust;
                char sign_adjust = src_sync_sign_and_magnitude(adjust, &print_adjust);
                SOSY_MSG4( SRC_SYNC_TRACE_RATE_MATCH, "rm pri src_%d mon 0x%08x 1 %c%d * 2^-31",
                           terminal_num, value, sign_adjust, print_adjust);
#endif /* SOSY_VERBOSE */
            }
        }
        return TRUE;
    }
    /* Terminal is a sink */
    else if (src_sync_is_sink_group_rm_initialised(op_extra_data, terminal_num,
                                                   &sink_grp))
    {
        SRC_SYNC_RM_STATE* rm_state = sink_grp->rm_state;
#ifdef INSTALL_METADATA
        /*
         * Check if there is a valid timestamp based measurement at both
         * reference and feedback, and if yes, use them.
         * Check available interval for both measurements before
         * taking the measurement, as the latter resets a measurement's
         * starting point
         */
        int deviation = 0;
        bool valid_deviation = FALSE;
        TIME now = hal_get_time();

        if (rate_measure_available(
                &rm_state->rm_measure_feedback.core,
                &rate_measurement_validity_default, now))
        {
            if (rate_measure_available(
                    &op_extra_data->rm_measure_primary.core,
                    &rate_measurement_validity_default, now))
            {
                /* Compare timestamp-based measurements */
                RATE_MEASUREMENT ref, fb;

                rate_measure_take_measurement(&op_extra_data->rm_measure_primary.core, &ref,
                                              &rate_measurement_validity_default, now);
                rate_measure_take_measurement(&rm_state->rm_measure_feedback.core, &fb,
                                              &rate_measurement_validity_default, now);

                valid_deviation = rate_compare(&rm_state->rm_cmp, &ref, &fb, &deviation);
#ifdef SOSY_VERBOSE
                if (! valid_deviation)
                {
                    SOSY_MSG1(SRC_SYNC_TRACE_RATE_MATCH, "rm sink_g%d deviation(2) range error",
                              sink_grp->common.idx);
                }
                else if (deviation < 0)
                {
                    SOSY_MSG2(SRC_SYNC_TRACE_RATE_MATCH, "rm sink_g%d deviation(2) -%d",
                              sink_grp->common.idx, -deviation);
                }
                else
                {
                    SOSY_MSG2(SRC_SYNC_TRACE_RATE_MATCH, "rm sink_g%d deviation(2) %d",
                              sink_grp->common.idx, deviation);
                }
#endif /* SOSY_VERBOSE */
            }
            else
            {
                /* Feedback measurement vs nominal reference rate */
                RATE_MEASUREMENT fb;
                rate_measure_take_measurement(&rm_state->rm_measure_feedback.core, &fb,
                                              &rate_measurement_validity_default, now);

                valid_deviation = rate_deviation(rm_state->rm_cmp.fb_sample_rate_div25,
                                                 &fb, &deviation);

                if (valid_deviation)
                {
                    /* Ignore primary_sp_adjust if:
                     * - this sink group is receiving and adjusting TTP
                     * - this TTP is routed to an output group
                     * (Assuming that this TTP will control the sink rate).
                     */
                    int reference;
                    if ((rm_state->rm_measure_feedback.base_type == RATE_TIMESTAMP_TYPE_TTP)
                        && (sink_grp->metadata_dest->common.metadata_enabled != 0)
                        && (sink_grp->metadata_dest->common.metadata_buffer != NULL)
                        && (sink_grp->metadata_dest->metadata_dest.provide_ttp))
                    {
                        reference = 0;
                    }
                    else
                    {
                        reference = op_extra_data->primary_sp_adjust;
                    }

#ifdef SOSY_VERBOSE
                    unsigned mag_deviation;
                    char sign_deviation = src_sync_sign_and_magnitude(deviation, &mag_deviation);
                    SOSY_MSG3(SRC_SYNC_TRACE_RATE_MATCH, "rm sink_g%d deviation(1) %c%d",
                              sink_grp->common.idx, sign_deviation, mag_deviation);
                    if (reference != 0)
                    {
                        unsigned mag_reference;
                        char sign_reference = src_sync_sign_and_magnitude(reference, &mag_reference);
                        SOSY_MSG3(SRC_SYNC_TRACE_RATE_MATCH, "rm sink_g%d deviation(1) reference %c%d",
                                  sink_grp->common.idx, sign_reference, mag_reference);
                    }
#endif /* SOSY_VERBOSE */

                    deviation = deviation - reference;
                }
                else
                {
                    SOSY_MSG1(SRC_SYNC_TRACE_RATE_MATCH, "rm sink_g%d deviation(1) range error",
                              sink_grp->common.idx);
                }
            }
        }

        if (valid_deviation)
        {
            int rm_adjust = (int)rm_state->rm_adjust_amount;
            rm_adjust = rm_adjust + frac_mult(FRACTIONAL(0.25),deviation);
            rm_state->rm_adjust_amount = (unsigned)rm_adjust;

#ifdef SOSY_VERBOSE
            unsigned mag_deviation;
            char sign_deviation = src_sync_sign_and_magnitude(deviation, &mag_deviation);
            unsigned mag_adjust;
            char sign_adjust = src_sync_sign_and_magnitude(rm_adjust, &mag_adjust);
            SOSY_MSG4( SRC_SYNC_TRACE_RATE_MATCH, "rm dev %c%d adj %c%d (x2^-31)",
                       sign_deviation, mag_deviation, sign_adjust, mag_adjust);
#endif /* SOSY_VERBOSE */
        }
        else
#endif /* INSTALL_METADATA */
        {
            /*
             * The rm_adjust_amount field is declared unsigned, based on the
             * pointer to it in CBOPS_VALS.rate_adjustment_amount. In cbops this
             * is probably meant in the sense of "machine word", without regard
             * to whether the value will be used as signed or unsigned.
             * Size-changing casts should be signed.
             */
            int s_value = (int)(int32)value;

            /* Assume that this assignment is atomic. Other code
             * updating the adjustment field does so too.
             */
            rm_state->rm_adjust_amount = (unsigned)s_value;

#ifdef SOSY_VERBOSE
            /* Logging doesn't reliably show the sign */
            unsigned mag_value;
            char sign_value = src_sync_sign_and_magnitude(s_value, &mag_value);
            SOSY_MSG3( SRC_SYNC_TRACE_RATE_MATCH, "rm_adjust sink_%d %c%d * 2^-31",
                       terminal_num, sign_value, mag_value);
#endif /* SOSY_VERBOSE */
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/**
 * If the sink is in a group for which rate adjustment is enabled and
 * initialized, return the intermediate buffer; otherwise return
 * the connection buffer.
 *
 * This should only be called from data processing, after transitions
 * and after group/entry links have been refreshed.
 */
tCbuffer* src_sync_get_input_buffer(
        SRC_SYNC_SINK_GROUP* sink_grp, SRC_SYNC_SINK_ENTRY* sink_ptr)
{
    PL_ASSERT(sink_grp != NULL);
    SRC_SYNC_RM_STATE* rm_state = sink_grp->rm_state;

    patch_fn_shared(src_sync_rate_adjust);

    if (sink_grp->rm_state != NULL)
    {
        SOSY_DEV_ASSERT( sink_grp->rate_adjust_enable );
        SOSY_DEV_ASSERT( sink_ptr->common.idx_in_group < rm_state->num_channels );
        SOSY_DEV_ASSERT( rm_state->adjusted_buffers != NULL );

        return rm_state->adjusted_buffers[sink_ptr->common.idx_in_group];
    }
    else
    {
        return sink_ptr->common.buffer;
    }
}

#ifdef INSTALL_METADATA
/**
 * If rate adjustment is enabled and initialized for a sink group,
 * and metadata is enabled and connected, return the intermediate
 * metadata buffer (always the first channel); otherwise return
 * the connection's metadata buffer.
 *
 * This should only be called from data processing.
 */
tCbuffer* src_sync_get_input_metadata_buffer(SRC_SYNC_SINK_GROUP* sink_grp)
{
    patch_fn_shared(src_sync_rate_adjust);

    if ( sink_grp->rate_adjust_enable
         && sink_grp->common.metadata_enabled
         && (sink_grp->rm_state != NULL)
         && (sink_grp->common.metadata_buffer != NULL) )
    {
        return sink_grp->rm_state->adjusted_buffers[0];
    }
    else
    {
        return sink_grp->common.metadata_buffer;
    }
}
#endif /* INSTALL_METADATA */

/**
 * When all sink terminals in a group have been connected,
 * set up multi-channel cbops for rate adjustment.
 * Processing should be suspended when this is called.
 */
bool src_sync_rm_init( SRC_SYNC_OP_DATA *op_extra_data,
                       SRC_SYNC_SINK_GROUP* sink_grp)
{
    unsigned num_channels;
    unsigned i, n;
    SRC_SYNC_RM_STATE* rm_state = NULL;
    tCbuffer** buffers = NULL;
    struct cbops_mgr* cbops = NULL;
    /** Simple diagnostic */
    unsigned step = 0;

    patch_fn_shared(src_sync_rate_adjust);

    /** Temporary array holding the channels' connection buffers
     * for passing to cbops_mgr_connect */
    tCbuffer** conn_buffers = NULL;

    SOSY_DEV_ASSERT(sink_grp->rate_adjust_enable);

    num_channels = pl_one_bit_count(sink_grp->common.channel_mask);
    if (num_channels == 0)
    {
        SOSY_MSG1( SRC_SYNC_TRACE_RATE_MATCH,
                   "rm_init sink_g%d no channels",
                   sink_grp->common.idx);
        return FALSE;
    }

    if (sink_grp->rm_state != NULL)
    {
        SOSY_MSG1( SRC_SYNC_TRACE_RATE_MATCH,
                   "rm_init sink_g%d rm_state exists, clean first",
                   sink_grp->common.idx);
        src_sync_rm_fini(op_extra_data, sink_grp);
    }

    SOSY_MSG2( SRC_SYNC_TRACE_RATE_MATCH,
               "rm_init sink_g%d channels %d",
               sink_grp->common.idx, num_channels);

    /* Collect connection buffers */
    conn_buffers = xzpnewn(SRC_SYNC_CAP_MAX_CHANNELS, tCbuffer*);
    if (conn_buffers == NULL)
    {
        L2_DBG_MSG2("src_sync 0x%04x WARNING rm_init sink_g%d failed (alloc)",
                    op_extra_data->id, sink_grp->common.idx);
        return FALSE;
    }

    /* At this point, the linked lists of terminals in groups have not been
     * updated (this is called from connect, before refresh).
     */
    n = 0;
    for (i = 0; i < SRC_SYNC_CAP_MAX_CHANNELS; ++ i)
    {
        SRC_SYNC_SINK_ENTRY* sink_ptr = op_extra_data->sinks[i];

        if ( (sink_ptr != NULL)
             && (sink_group_from_entry(sink_ptr) == sink_grp))
        {
            SOSY_DEV_ASSERT(n < num_channels);
            SOSY_DEV_ASSERT(sink_ptr->common.buffer != NULL);
            SOSY_DEV_ASSERT(sink_ptr->common.idx_in_group == n);

            conn_buffers[n] = sink_ptr->common.buffer;

            SOSY_MSG3( SRC_SYNC_TRACE_RATE_MATCH,
                       "rm_init sink_g%d sink_%d is ch_%d",
                       sink_grp->common.idx, sink_ptr->common.idx, n);

            n += 1;
        }
    }
    PL_ASSERT(n == num_channels);

    rm_state = xzpnew(SRC_SYNC_RM_STATE);
    buffers = xzpnewn(num_channels, tCbuffer*);
    if ((rm_state != NULL) && (buffers != NULL))
    {
        step += 1;

#ifdef INSTALL_METADATA
        rate_measure_set_nominal_rate(&rm_state->rm_measure_feedback.core,
                             sink_grp->common.sample_rate);
#endif /* INSTALL_METADATA */

        unsigned buffer_size_t = SRC_SYNC_RM_BUFFER_SIZE_SS_PERIOD_MULT
                                 * op_extra_data->cur_params.OFFSET_SS_PERIOD;
        unsigned buffer_size_w = frac_mult(buffer_size_t, sink_grp->common.sample_rate);

        for (i = 0; i < num_channels; ++ i)
        {
            buffers[i] = cbuffer_create_with_malloc(buffer_size_w, BUF_DESC_SW_BUFFER);
            if (buffers[i] == NULL)
            {
                break;
            }
        }

        if (buffers[num_channels-1] != NULL)
        {
            step += 1;

#ifdef INSTALL_METADATA
            bool enable_metadata_success = TRUE;

            if ( sink_grp->common.metadata_enabled
                 && (sink_grp->common.metadata_buffer != NULL) )
            {
                buff_metadata_enable(buffers[0]);
                buff_metadata_set_usable_octets(
                        buffers[0],
                        get_octets_per_word(AUDIO_DATA_FORMAT_FIXP));

                for (i = 1; i < num_channels; ++ i )
                {
                    if (! buff_metadata_connect(buffers[i],
                                                buffers[0], buffers[0]))
                    {
                        enable_metadata_success = FALSE;
                        break;
                    }
                }
            }
            if (enable_metadata_success)
            {
                step += 1;
#endif /* INSTALL_METADATA */

                cbops = cbops_mgr_create(SINK, CBOPS_RATEADJUST);
                if (cbops != NULL)
                {
                    CBOP_VALS vals;

                    step += 1;

                    src_sync_set_cbops_vals(rm_state, &vals);

                    /* Make up a processing quantity: Use the smallest processing
                     * length, and "default" sampling rate
                     */
                    rm_state->block_size =
                            rate_time_to_samples(
                                    op_extra_data->cur_params.OFFSET_SS_PERIOD,
                                    sink_grp->common.sample_rate);

                    if (cbops_mgr_connect( cbops, num_channels,
                                           conn_buffers, buffers, &vals ))
                    {
                        step += 1;

                        if (cbops_mgr_rateadjust_passthrough_mode(
                                cbops, !sink_grp->rm_enact))
                        {
                            rm_state->rm_cbops = cbops;
                            rm_state->num_channels = num_channels;
                            rm_state->adjusted_buffers = buffers;
#ifdef INSTALL_METADATA
                            sink_grp->metadata_input_buffer = buffers[0];
#endif /* INSTALL_METADATA  */
                            sink_grp->rm_state = rm_state;
#ifdef INSTALL_METADATA
                            src_sync_ra_update_delay(op_extra_data, sink_grp);
#endif /* INSTALL_METADATA  */
                            op_extra_data->sink_rm_initialised_mask |=
                                    sink_grp->common.channel_mask;

                            pfree(conn_buffers);

                            return TRUE;
                        }
                    }
                }
#ifdef INSTALL_METADATA
            }
#endif /* INSTALL_METADATA */
        }
    }

    /* If we arrive here, something failed. Clean up what had been allocated */
    L2_DBG_MSG3("src_sync 0x%04x WARNING rm_init sink_g%d failed after %d steps",
                op_extra_data->id, sink_grp->common.idx, step);

    if (cbops != NULL)
    {
        cbops_mgr_destroy(cbops);
    }
    pfree(rm_state);
    if (buffers != NULL)
    {
        for (i = 0; i < num_channels; ++ i)
        {
            if (buffers[i] != NULL)
            {
                cbuffer_destroy(buffers[i]);
            }
        }
    }
    pfree(buffers);
    pfree(conn_buffers);

    return FALSE;
}

/**
 * When any sink terminal in a group has been disconnected,
 * close the cbops for rate adjustment.
 * Processing should be suspended when this is called.
 */
bool src_sync_rm_fini( SRC_SYNC_OP_DATA *op_extra_data,
                       SRC_SYNC_SINK_GROUP* sink_grp)
{
    SRC_SYNC_RM_STATE* rm_state = sink_grp->rm_state;

    patch_fn_shared(src_sync_rate_adjust);

    op_extra_data->sink_rm_initialised_mask &=
            (~ sink_grp->common.channel_mask);

    if (rm_state == NULL)
    {
        SOSY_MSG1( SRC_SYNC_TRACE_RATE_MATCH,
                   "rm_fini sink_g%d not initialized",
                   sink_grp->common.idx);
        return FALSE;
    }

    sink_grp->rm_state = NULL;
#ifdef INSTALL_METADATA
    sink_grp->metadata_input_buffer = sink_grp->common.metadata_buffer;
#endif /* INSTALL_METADATA */

    if (rm_state->rm_cbops != NULL)
    {
        cbops_mgr_disconnect(rm_state->rm_cbops);
        cbops_mgr_destroy(rm_state->rm_cbops);
    }
    if (rm_state->adjusted_buffers != NULL)
    {
        unsigned i;
        for (i = 0; i < rm_state->num_channels; ++ i)
        {
            if (rm_state->adjusted_buffers[i] != NULL)
            {
                cbuffer_destroy(rm_state->adjusted_buffers[i]);
            }
        }
        pfree(rm_state->adjusted_buffers);
    }
#ifdef INSTALL_METADATA
    buff_metadata_delete_tag(rm_state->ttp_template_tag, TRUE);
#endif /* INSTALL_METADATA  */
    pfree(rm_state);

    return TRUE;
}

void src_sync_rm_process( SRC_SYNC_OP_DATA *op_extra_data, unsigned *back_kick )
{
    SRC_SYNC_SINK_GROUP* sink_grp;

    patch_fn_shared(src_sync_rate_adjust);

    for ( sink_grp = op_extra_data->sink_groups;
          sink_grp != NULL;
          sink_grp = next_sink_group(sink_grp) )
    {
        SRC_SYNC_RM_STATE* rm_state = sink_grp->rm_state;

        if (! sink_grp->rate_adjust_enable || (rm_state == NULL))
        {
            continue;
        }

        /* In many cases, cbops are run using
         * cbops_mgr_process_data(endpoint->cbops, CBOPS_MAX_COPY_SIZE - 1);
         * The actual amount processed could be very small, including the case
         * where SRA consumes one sample but doesn't write any.
         * To avoid difficulty writing matching metadata, check in advance
         * whether a reasonable amount can be processed. As in the main
         * source sync flow control, the threshold for this is based on the
         * MIN_PERIOD parameter, typically 1 ms.
         */

        unsigned skip_if_less;
        unsigned max_words = MAXINT;
        unsigned max_space_words;
        unsigned max_avail_words;
#ifdef INSTALL_METADATA
        tCbuffer* first_input_buffer = NULL;
        unsigned input_rd_idx_before = 0;
        unsigned input_rd_idx_after, input_buffer_size, input_read_w;
        unsigned read_oct_before, read_oct_after;
        metadata_tag* read_tags;
        tCbuffer* first_output_buffer;
        unsigned output_wr_idx_before = 0;
        unsigned output_wr_idx_after, output_buffer_size, output_produced_w;
        const bool process_metadata =
                sink_grp->common.metadata_enabled &&
                (sink_grp->common.metadata_buffer != NULL);
#endif /* INSTALL_METADATA */

        SOSY_DEV_ASSERT(sink_grp->common.terminals != NULL);

        if (sink_grp->stall_state == SRC_SYNC_SINK_PENDING)
        {
            /* Don't observe a minimum amount to process */
            skip_if_less = 1;
        }
        else
        {
            skip_if_less = rate_time_to_samples(
                               op_extra_data->cur_params.OFFSET_SS_PERIOD,
                               sink_grp->common.sample_rate );
        }

        max_space_words = MAXINT;
        unsigned i;
        for (i = 0; i < rm_state->num_channels; ++ i)
        {
            unsigned space_avail;
            space_avail = cbuffer_calc_amount_space_in_words(
                              rm_state->adjusted_buffers[i]);
            max_space_words = pl_min(max_space_words, space_avail);
            if (max_space_words == 0)
            {
                break;
            }
        }
#ifdef INSTALL_METADATA
        first_output_buffer = rm_state->adjusted_buffers[0];

        if (process_metadata)
        {
            /* Limit by space/data in metadata buffers as well */
            unsigned words;
            unsigned octets;
            octets = buff_metadata_available_space(first_output_buffer);
            words = octets / OCTETS_PER_SAMPLE;
            max_space_words = pl_min(max_space_words, words);
        }
#endif /* INSTALL_METADATA */

        /* Early check for minimum amount worth working on */
        if (max_space_words < skip_if_less)
        {
            /* Next sink group */
            continue;
        }

        max_avail_words = MAXINT;

        SRC_SYNC_SINK_ENTRY* sink_ptr;
        for ( sink_ptr = sink_entries_from_group(sink_grp);
              sink_ptr != NULL;
              sink_ptr = next_sink_entry(sink_ptr) )
        {
            unsigned input_avail;

            SOSY_DEV_ASSERT(sink_ptr->common.buffer != NULL);

            input_avail = cbuffer_calc_amount_data_in_words(sink_ptr->common.buffer);

            max_avail_words = pl_min(max_avail_words, input_avail);
            if (max_avail_words == 0)
            {
                break;
            }
        }
#ifdef INSTALL_METADATA
        if (process_metadata)
        {
            /* Limit by space/data in metadata buffers as well */
            unsigned words;
            unsigned octets;
            octets = buff_metadata_available_octets(sink_grp->common.metadata_buffer);
            words = octets / OCTETS_PER_SAMPLE;
            max_avail_words = pl_min(max_avail_words, words);
        }
#endif /* INSTALL_METADATA */

        /* If limited by available data, request more */
        if (max_avail_words < max_space_words)
        {
            *back_kick |= sink_grp->common.channel_mask;
        }

        /* Early check for minimum amount worth working on */
        if (max_avail_words < skip_if_less)
        {
            /* Next sink group */
            continue;
        }

#ifdef INSTALL_METADATA
        if (process_metadata)
        {
            /* Measure how many words were consumed, by the movement of
             * the read pointer of the first channel's input buffer.
             * Likewise get the amount produced using the write pointer
             * of an output buffer.
             */
            first_input_buffer = sink_grp->common.terminals->buffer;
            input_rd_idx_before = cbuffer_get_read_offset(first_input_buffer);
            output_wr_idx_before = cbuffer_get_write_offset(first_output_buffer);
        }
#endif /* INSTALL_METADATA */

        /* cbops SRA doesn't seem to limit the output to the given number
         * of samples. I.e. must leave margin for rate adjustment to write more
         * than requested.
         */
        unsigned use_space = frac_mult(SRC_SYNC_MAX_SRA_SPACE_Q,
                                       max_space_words);
        if (use_space > 1)
        {
            use_space -= 1;
        }

        max_words = pl_min(use_space, max_avail_words);
        SOSY_MSG4( SRC_SYNC_TRACE_RM_TRANSFER,
                   "rm sink_g%d avail_w %d space_w %d max_w %d",
                   sink_grp->common.idx, max_avail_words, max_space_words,
                   max_words);

        cbops_mgr_process_data(rm_state->rm_cbops, max_words);

#ifdef INSTALL_METADATA
        if (process_metadata)
        {
            metadata_tag* eof_tag = NULL;

            input_rd_idx_after = cbuffer_get_read_offset(first_input_buffer);
            input_buffer_size = cbuffer_get_size_in_words(first_input_buffer);
            /* unsigned wrapping calculation */
            input_read_w = input_rd_idx_after - input_rd_idx_before;
            if (input_read_w >= input_buffer_size)
            {
                input_read_w += input_buffer_size;
            }

            output_wr_idx_after = cbuffer_get_write_offset(first_output_buffer);
            output_buffer_size = cbuffer_get_size_in_words(first_output_buffer);
            /* unsigned wrapping calculation */
            output_produced_w = output_wr_idx_after - output_wr_idx_before;
            if (output_produced_w >= output_buffer_size)
            {
                output_produced_w += output_buffer_size;
            }

            patch_fn_shared(src_sync_rate_adjust);

            if (! sink_grp->rm_enact)
            {
                SOSY_MSG3( SRC_SYNC_TRACE_RM_TRANSFER,
                           "rm sink_g%d md !en max_w %d fwd_w %d",
                           sink_grp->common.idx, max_words, input_read_w);

                PL_ASSERT( (input_read_w == output_produced_w)
                           && (output_produced_w <= max_space_words ) );

                metadata_strict_transport( sink_grp->common.metadata_buffer,
                                           first_output_buffer,
                                           OCTETS_PER_SAMPLE * input_read_w);
            }
            else
            {
                TIME ts_start_of_next_block = 99999999; /* canary */

                SOSY_MSG4( SRC_SYNC_TRACE_RM_TRANSFER,
                           "rm sink_g%d md en max_w %d in_w %d out_w %d",
                           sink_grp->common.idx, max_words, input_read_w,
                           output_produced_w);

                PL_ASSERT( (input_read_w <= max_avail_words)
                           && (output_produced_w <= max_space_words) );

                if (input_read_w > 0)
                {
                    read_tags = buff_metadata_remove(sink_grp->common.metadata_buffer,
                                                     OCTETS_PER_SAMPLE * input_read_w,
                                                     &read_oct_before, &read_oct_after);

                    SOSY_MSG4( SRC_SYNC_TRACE_RM_TRANSFER,
                               "rm sink_g%d rcv %d tags before_oct %d after_oct %d",
                               sink_grp->common.idx,
                               src_sync_metadata_count_tags(read_tags),
                               read_oct_before, read_oct_after);

                    if (read_tags == NULL)
                    {
                        if (rm_state->valid_ttp_reconstruct)
                        {
                            ts_start_of_next_block = src_sync_get_next_timestamp(
                                    rm_state->ts_start_of_input_block,
                                    input_read_w,
                                    sink_grp->common.inv_sample_rate,
                                    rm_state->last_sp_adjust);

                            SOSY_MSG4( SRC_SYNC_TRACE_RM_TRANSFER,
                                       "rm sink_g%d (cont w %d) ts_start %d -> %d",
                                       sink_grp->common.idx, input_read_w,
                                       rm_state->ts_start_of_input_block,
                                       ts_start_of_next_block);
                        }
                    }
                    else
                    {
                        metadata_tag* tag = read_tags;
                        do
                        {
                            metadata_tag* consume_tag = tag;
                            tag = tag->next;
                            consume_tag->next = NULL;


                            if (IS_TIMESTAMPED_TAG(consume_tag) || IS_TIME_OF_ARRIVAL_TAG(consume_tag))
                            {
                                SOSY_MSG4( SRC_SYNC_TRACE_RM_TRANSFER,
                                           "rm sink_g%d rcv len %d t %d sp %d",
                                           sink_grp->common.idx, consume_tag->length,
                                           consume_tag->timestamp, consume_tag->sp_adjust);

                                rm_state->last_sp_adjust = consume_tag->sp_adjust;
                            }

                            /* Maintain reconstructed TTP */
                            if (IS_TIMESTAMPED_TAG(consume_tag))
                            {
                                if (! rm_state->valid_ttp_reconstruct)
                                {
                                    rm_state->valid_ttp_reconstruct = TRUE;
                                    SOSY_MSG3( SRC_SYNC_TRACE_RATE_MATCH,
                                               "rm sink_g%d tag_f 0x%02x valid_ttp 0->1 fs_hz %d",
                                               sink_grp->common.idx, consume_tag->flags,
                                               sink_grp->common.sample_rate);
                                }

                                /* From the first tag, extrapolate back to the start of the buffer.
                                 * If the first tag is not a TTP, there is an estimate from
                                 * the previous run.
                                 */
                                if (consume_tag == read_tags)
                                {
                                    unsigned words_back = read_oct_before/OCTETS_PER_SAMPLE;
                                    rm_state->ts_start_of_input_block =
                                            src_sync_get_prev_timestamp(
                                                    consume_tag, words_back,
                                                    sink_grp->common.inv_sample_rate);

                                    SOSY_MSG4( SRC_SYNC_TRACE_RM_TRANSFER,
                                               "rm sink_g%d (tag t %d) back_w %d ts_start %d",
                                               sink_grp->common.idx, consume_tag->timestamp,
                                               words_back, rm_state->ts_start_of_input_block);
                                }
                            }
                            else if (IS_VOID_TTP_TAG(consume_tag))
                            {
                                /* ignore */
                            }
                            else if (rm_state->valid_ttp_reconstruct)
                            {
                                SOSY_MSG2( SRC_SYNC_TRACE_RATE_MATCH,
                                           "rm sink_g%d tag_f 0x%02x valid_ttp 1->0",
                                           sink_grp->common.idx, consume_tag->flags);

                                rm_state->valid_ttp_reconstruct = FALSE;
                            }

                            if (tag == NULL)
                            {
                                /* If the last tag is a TTP, use it to get the next
                                 * block start timestamp; otherwise extrapolate
                                 * from the start of this block
                                 */
                                if (IS_TIMESTAMPED_TAG(consume_tag))
                                {
                                    unsigned fwd_words = read_oct_after / OCTETS_PER_SAMPLE;

                                    ts_start_of_next_block =
                                            src_sync_get_next_timestamp(
                                                    consume_tag->timestamp,
                                                    fwd_words,
                                                    sink_grp->common.inv_sample_rate,
                                                    rm_state->last_sp_adjust);

                                    SOSY_MSG4( SRC_SYNC_TRACE_RM_TRANSFER,
                                               "rm sink_g%d (tag t %d) fwd w %d ts_next_t %d",
                                               sink_grp->common.idx,
                                               consume_tag->timestamp, fwd_words,
                                               ts_start_of_next_block);
                                }
                                else
                                {
                                    ts_start_of_next_block = src_sync_get_next_timestamp(
                                            rm_state->ts_start_of_input_block,
                                            input_read_w,
                                            sink_grp->common.inv_sample_rate,
                                            rm_state->last_sp_adjust);

                                    SOSY_MSG4( SRC_SYNC_TRACE_RM_TRANSFER,
                                               "rm sink_g%d ts_start %d w %d ts_next_t %d",
                                               sink_grp->common.idx,
                                               rm_state->ts_start_of_input_block,
                                               input_read_w, ts_start_of_next_block);
                                }
                            }

                            /* Keep received tags for either of two reasons:
                             * If it is a TTP tag with META_PRIV_KEY_TTP_OFFSET,
                             * keep it as a template for output TTP tags; if it
                             * is an EOF, reuse it right away. (If it is both
                             * -- though that seems illegal -- EOF wins)
                             */
                            if (METADATA_STREAM_END(consume_tag))
                            {
                                /* Strange but there might be more than one
                                 * EOF tag received within one processing run.
                                 * Keep them all. Force length to zero so they
                                 * can be appended without affecting the
                                 * transport pointers.
                                 */
                                consume_tag->length = 0;
                                consume_tag->flags &= METADATA_STREAM_END_MASK;
                                consume_tag->next = eof_tag;
                                eof_tag = consume_tag;
                                consume_tag = NULL;
                            }
                            else if ( IS_TIME_OF_ARRIVAL_TAG(consume_tag))
                            {
                                /* Make sure not to forward a previously
                                 * cached offset context
                                 */
                                if (rm_state->ttp_template_tag != NULL)
                                {
                                    buff_metadata_delete_tag(
                                            rm_state->ttp_template_tag, FALSE);
                                    rm_state->ttp_template_tag = NULL;
                                }
                            }
                            else if (IS_TIMESTAMPED_TAG(consume_tag))
                            {
                                /* Keep the most recently received timestamped tag,
                                 * in order to forward its error_offset_ptr, if any
                                 */
                                if (rm_state->ttp_template_tag != NULL)
                                {
                                    buff_metadata_delete_tag(
                                            rm_state->ttp_template_tag, FALSE);
                                }
                                consume_tag->flags &= METADATA_TIMESTAMP_MASK;
                                rm_state->ttp_template_tag = consume_tag;
                                consume_tag = NULL;
                            }

                            buff_metadata_delete_tag(consume_tag, FALSE);

                        } while (tag != NULL);
                    }
                }

                if (output_produced_w > 0)
                {
                    unsigned octets = output_produced_w * OCTETS_PER_SAMPLE;
                    metadata_tag* forward_tag;

                    if ( rm_state->valid_ttp_reconstruct
                         && (rm_state->ttp_template_tag != NULL))
                    {
                        forward_tag = buff_metadata_copy_tag(rm_state->ttp_template_tag);
                    }
                    else
                    {
                        forward_tag = buff_metadata_new_tag();
                    }

                    if (forward_tag != NULL)
                    {
                        forward_tag->length = octets;
                        if (rm_state->valid_ttp_reconstruct)
                        {
                            METADATA_TIMESTAMP_SET(
                                    forward_tag,
                                    rm_state->ts_start_of_input_block
                                    - rm_state->rm_delay_usec,
                                    METADATA_TIMESTAMP_LOCAL);
                            forward_tag->sp_adjust = op_extra_data->primary_sp_adjust;

                            SOSY_MSG3( SRC_SYNC_TRACE_RM_TRANSFER,
                                       "rm sink_g%d wr t %d len_w %d",
                                       sink_grp->common.idx, rm_state->ts_start_of_input_block,
                                       output_produced_w);
                        }
                        else
                        {
                            METADATA_VOID_TTP_SET(forward_tag);
                        }

                        forward_tag->next = eof_tag;
                    }

                    /* Record the timestamp/sample for the feedback path
                     * of the TTP/TOA based rate adjustment
                     */
                    rate_measure_metadata_record_tags(
                            &rm_state->rm_measure_feedback, output_produced_w,
                            0, octets, forward_tag);

                    buff_metadata_append(first_output_buffer, forward_tag, 0, octets);
                }
                else if (eof_tag != NULL)
                {
                    /* Exceptional case, if cbops did not produce output,
                     * don't try to hang on to an EOF tag
                     */
                    buff_metadata_tag_list_delete(eof_tag);
                }

                rm_state->ts_start_of_input_block = ts_start_of_next_block;
            }
        }
#endif /* INSTALL_METADATA */
    }
}

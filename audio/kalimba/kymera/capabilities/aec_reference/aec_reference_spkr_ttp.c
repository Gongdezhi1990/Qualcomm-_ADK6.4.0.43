/**
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 * \file  aec_reference_spkr_ttp.c
 * \ingroup  capabilities
 *
 *  AEC Reference
 *
 */

/****************************************************************************
Include Files
*/
#include "aec_reference_cap_c.h"
#ifdef AEC_REFERENCE_SPKR_TTP
#include "platform/pl_intrinsics.h"
#include "platform/pl_fractional.h"
#include "aec_reference_config.h"
#include "op_msg_helpers.h"
#include "patch/patch.h"
#include "pl_assert.h"

/****************************************************************************
Private Constant Definitions
*/

/* Void tags turn off rate adjustment only
 * if we see them for at least 50ms, this is to avoid
 * frequent turning on and off the rate adjustment when
 * broken audio is being received.
 */
#define SPKR_TTP_VOID_TAG_COOL_OFF_TIME_MS 50

/* TTP error Threshold to switch from real audio to discarding/silence insertion */
#define SPKR_TTP_MAX_LATENCY_HIGH_THRESHOLD_US 2000

/* TTP error Threshold to switch from discarding/silence insertion to real audio */
#define SPKR_TTP_MAX_LATENCY_LOW_THRESHOLD_US  250

/* maximum amount of jump in error we expect between runs,
 * we don't update ttp if error has a sudden big jump
 */
#define SPKR_TTP_MAX_ERROR_JUMP_US 50

/* error is calculated each run, update is done every 10ms
 * based on average error
 */
#define SPKR_TTP_ERROR_AVERAGING_PERIOD_MS 10

/* Enables some debug log in this file */
#define SPKR_TTP_DEBUGx

/****************************************************************************
Private Function Declarations
*/
unsigned aec_reference_spkr_ttp_discard_samples(AEC_REFERENCE_OP_DATA  *op_extra_data,
                                                       unsigned amount_to_discard);
static bool aec_reference_spkr_ttp_get_error(AEC_REFERENCE_OP_DATA   *op_extra_data,
                                             TIME_INTERVAL *error);
static bool aec_reference_spkr_ttp_setup_timed_playback(AEC_REFERENCE_OP_DATA *op_extra_data);
static void aec_reference_spkr_ttp_reset(AEC_REFERENCE_OP_DATA *op_extra_data);
static void aec_reference_spkr_ttp_error_control(AEC_REFERENCE_OP_DATA *op_extra_data,
                                                 TIME_INTERVAL error);
static void aec_reference_spkr_ttp_adjust_rate(AEC_REFERENCE_OP_DATA *op_extra_data, int warp);
/****************************************************************************
public Function Definitions
*/

/**
 * aec_reference_spkr_ttp_init
 * \brief initialises the speaker timed playback
 * \param op_extra_data Pointer to the AEC reference operator specific data.
 */
void aec_reference_spkr_ttp_init(AEC_REFERENCE_OP_DATA  *op_extra_data)
{
    patch_fn_shared(aec_reference);

    /* initialisations */
}

/**
 * aec_reference_spkr_ttp_terminate
 * \brief terminates the speaker timed playback
 * \param op_extra_data Pointer to the AEC reference operator specific data.
 */
void aec_reference_spkr_ttp_terminate(AEC_REFERENCE_OP_DATA  *op_extra_data)
{
    patch_fn_shared(aec_reference);

    op_extra_data->spkr_timed_playback_mode = FALSE;
    op_extra_data->spkr_last_timestamp_valid = 0;
    op_extra_data->spkr_void_tag_counter = 0;
}

/**
 * aec_reference_spkr_ttp_run
 * \brief manages timed playback for speaker path
 * \param op_extra_data Pointer to the AEC reference operator specific data.
 * \param error current TTP error in speaker path
 * \param max_to_process Pointer max amount of data available to process by speaker cbops
 */
void aec_reference_spkr_ttp_run(AEC_REFERENCE_OP_DATA  *op_extra_data, unsigned *max_to_process)
{
    TIME_INTERVAL error = 0;
    bool error_valid;
    unsigned amount_can_process = *max_to_process;
    /* only used for debug log */
    TIME current_time = hal_get_time();

    patch_fn_shared(aec_reference_run);

    /* calculate current TTP error */
    error_valid = aec_reference_spkr_ttp_get_error(op_extra_data, &error);

    /* see if we have received void tags while in TTP mode */
    if(op_extra_data->spkr_void_tag_observed)
    {
        /* increment a counter, so this shows the time that we have
         * continuously seen void tag
         */
        op_extra_data->spkr_void_tag_counter++;

        if(op_extra_data->spkr_void_tag_counter == SPKR_TTP_VOID_TAG_COOL_OFF_TIME_MS)
        {
            /* if we have seen void tag continuously for significant amount of time,
             * then we turn off TTP rate adjustment for power saving.             *
             */

            /* error isn't valid anymore, only seeing a timestamp tag will
             * make it valid again
             */
            op_extra_data->spkr_last_timestamp_valid = FALSE;
            error_valid = FALSE;
        }
    }
    else
    {
        op_extra_data->spkr_void_tag_counter = 0;
    }

    if(!error_valid)
    {
        /* if we don't have a valid error, then no reliable decision can be made */
        return;
    }

#ifdef SPKR_TTP_DEBUG
    L2_DBG_MSG2("AEC REFERENCE SPEAKER: time=%d, ttp error=%d", current_time, error);
#endif

    /* See if the input is too late */
    if(error < -op_extra_data->spkr_error_threshold &&
       amount_can_process > 0)
    {
        /* Calculate how many samples we are late,
         * we discard late samples, as much as available
         */
        unsigned samples_to_trash = convert_time_to_samples((unsigned)(-error), op_extra_data->input_rate);

        /* limit to amount available */
        samples_to_trash = MIN(amount_can_process, samples_to_trash);

        if(samples_to_trash > 0)
        {
            /* We have something to trash */
            unsigned samples_trashed = aec_reference_spkr_ttp_discard_samples(op_extra_data,samples_to_trash);
            L3_DBG_MSG3("AEC REFERENCE SPEAKER TTP - discard samples=%d, time =%d , error=%d",
                        samples_trashed, current_time, error);

            /* some samples discarded, update error */
            error += convert_samples_to_time(samples_trashed, op_extra_data->input_rate);

            /* update amount left to process */
            amount_can_process -= samples_trashed;

            /* ttp play back needs resetting */
            aec_reference_spkr_ttp_reset(op_extra_data);
        }
    }

    if (pl_abs_i32(error) < op_extra_data->spkr_error_threshold)
    {
        /* This is normal situation, we have on-time input
         * and we can control ttp error
         */
        aec_reference_spkr_ttp_error_control(op_extra_data, error);

        /* real audio is played, go to higher threshold */
        op_extra_data->spkr_error_threshold = SPKR_TTP_MAX_LATENCY_HIGH_THRESHOLD_US;
    }
    else
    {
        /* Input is early, so we cannot allow samples from input buffers to be played now
         * instead we need to insert silence until time to play has reached. Silence insertion
         * is managed by cbops, we only tell cbops not to use samples from input buffer.
         */
        unsigned samples_early = convert_time_to_samples((unsigned)(error), op_extra_data->input_rate);
        if(samples_early >= op_extra_data->spkr_in_threshold)
        {
            /* It's early more than the amount cbops is expected to copy,
             * so allow nothing to copy.
             */
            amount_can_process = 0;
        }
        else
        {
            /* It's still early but less than the expected amount, we allow cbops to consume
             * expected amount less the amount early, silence insertion will be used for the amount early.
             * next time we expect to switch to normal TTP playback.
             */
            amount_can_process = MIN(op_extra_data->spkr_in_threshold - samples_early, amount_can_process);
        }

        /* We are not playing real audio, so keep the threshold low */
        op_extra_data->spkr_error_threshold = SPKR_TTP_MAX_LATENCY_LOW_THRESHOLD_US;

        L3_DBG_MSG3("AEC REFERENCE SPEAKER TTP: possible silence insertion: time=%d, samples=%d, max_proc=%d",
                    current_time, op_extra_data->spkr_in_threshold-amount_can_process, amount_can_process);

    }

    /* update amount to process */
    *max_to_process = amount_can_process;
    return;
}

/**
 * aec_reference_spkr_ttp_update_last_timestamp
 * \brief updates time stamp for speaker input buffer
 *        Call this function after consuming inputs, e.g. after
 *        cbops process, this will enable to have fresh updated time
 *        stamp associated with last consumed sample.
 * \param op_extra_data pointer to AEC_REFERENCE_OP_DATA data
 * \param amount_read amount read from input buffer
 */
void aec_reference_spkr_ttp_update_last_timestamp(AEC_REFERENCE_OP_DATA   *op_extra_data, unsigned amount_read)
{
    if(op_extra_data->spkr_last_timestamp_valid)
    {
        /* work out time stamp for next input block for the just read block,
         * TODO_AEC_REFERENCE_TTP: for further accuracy we could consider the tag sp_adjust as well */
        TIME_INTERVAL time_passed = convert_samples_to_time(amount_read, op_extra_data->input_rate);
        op_extra_data->spkr_last_timestamp = time_add(op_extra_data->spkr_last_timestamp, time_passed);
    }
}

#ifndef AEC_REFERENCE_SPKR_TTP_DISCARD_SAMPLES_USE_ROM
/****************************************************************************
Private Function Definitions
*/
/**
 * aec_reference_spkr_ttp_discard_samples
 * \brief discard some samples from speaker graph input
 *
 * \param op_extra_data Pointer to the AEC reference operator specific data.
 * \param amount_to_discard number of samples to discard
 *
 * \return number of samples discarded
 > */
unsigned aec_reference_spkr_ttp_discard_samples(AEC_REFERENCE_OP_DATA  *op_extra_data, unsigned amount_to_discard)
{
    unsigned idx;

    patch_fn_shared(aec_reference);

    for(idx=0; idx < MAX_NUMBER_SPEAKERS; idx++)
    {
        tCbuffer *this_buf = op_extra_data->input_stream[SpeakerInputTerminalByIndex(idx)];
        if(NULL !=  this_buf)
        {
            unsigned amount_data = cbuffer_calc_amount_data_in_words(this_buf);
            amount_to_discard = MIN(amount_to_discard, amount_data);
        }
    }

    /* don't continue if nothing to discard */
    if(amount_to_discard == 0)
    {
        return 0;
    }

    /* advance the read point of all speaker graph input buffers */
    for(idx=0; idx < MAX_NUMBER_SPEAKERS; idx++)
    {
        tCbuffer *this_buf = op_extra_data->input_stream[SpeakerInputTerminalByIndex(idx)];
        if(NULL !=  this_buf)
        {
            cbuffer_advance_read_ptr(this_buf, amount_to_discard);
        }
    }

    /* any change of buffer pointers outside cbops will need
     * refreshing buffers by cbops
     */
    cbops_reshresh_buffers(op_extra_data->spkr_graph);
    return amount_to_discard;

}
#endif /* #ifndef AEC_REFERENCE_SPKR_TTP_DISCARD_SAMPLES_USE_ROM */
/**
 * aec_reference_spkr_ttp_get_error
 * \brief get the current TTP error for speaker input
 *  This function normally should be called at the start of speaker
 *  graph task.
 * \param op_extra_data pointer to AEC_REFERENCE_OP_DATA data
 * \param error pointer for TTP error to be populated by this function.
 *        It will be the difference between desired playback time(ttp)
 *        and (estimated) actual playback time. So the error will be
 *        positive if ttp is later than actual playback time.
 * return whether the error is valid
 */
static bool aec_reference_spkr_ttp_get_error(AEC_REFERENCE_OP_DATA *op_extra_data, TIME_INTERVAL *error)
{
    /* see if we have valid metadata buffer */
    tCbuffer *met_buf = op_extra_data->spkr_input_metadata_buffer;
    patch_fn_shared(aec_reference_run);

    if (met_buf!= NULL && buff_has_metadata(met_buf))
    {
        /* see if input has a timestamped tag */
        unsigned b4idx = 0;
        metadata_tag *mtag = buff_metadata_peek_ex(met_buf, &b4idx);

        if(mtag != NULL)
        {
            if(IS_TIMESTAMPED_TAG(mtag))
            {
                unsigned *err_offset_id;
                unsigned out_length;

                /* go back to first input sample */
                TIME_INTERVAL time_back = convert_samples_to_time(b4idx / OCTETS_PER_SAMPLE, op_extra_data->input_rate);

                /* set the time stamp,
                 * TODO_AEC_REFERENCE_TTP: for further accuracy we could consider the tag sp_adjust as well */
                op_extra_data->spkr_last_timestamp = time_sub(mtag->timestamp, time_back);

                /* See if tag's timestamp has an offset*/
                if (buff_metadata_find_private_data(mtag, META_PRIV_KEY_TTP_OFFSET, &out_length,
                                                    (void **)&err_offset_id))
                {
                    int *err_offset_ptr = ttp_info_get(*err_offset_id);
                    if (err_offset_ptr != NULL)
                    {
                        /* subtract the offset */
                        op_extra_data->spkr_last_timestamp =
                            time_sub(op_extra_data->spkr_last_timestamp, *err_offset_ptr);
                    }
                }

                /* this will stay valid */
                op_extra_data->spkr_last_timestamp_valid = TRUE;

                /* switch to timed playback mode if not already,
                 * this is irreversible
                 */
                if(!op_extra_data->spkr_timed_playback_mode)
                {
                    /* For the first time we have seen a timestamp,
                     * This means the metadata buffer will supply timestamp tags
                     * that are required for timed playback. Now is the time to
                     * switch to timed playback mode for speaker graph.
                     */
                    op_extra_data->spkr_timed_playback_mode = aec_reference_spkr_ttp_setup_timed_playback(op_extra_data);
                    if(op_extra_data->spkr_timed_playback_mode)
                    {
                        /* tell the endpoint that speaker is performing rate adjustment, normally
                         * this is done when enacting config is received from framework, however
                         * in case we haven't received (can happen if input is coming from a2dp
                         * source) we do it here.
                         */
                        set_override_ep_ratematch_enacting(op_extra_data->spkr_endpoint, TRUE);

                        /* tell the overridden endpoint that we are in timed playback mode,
                         * this is to prevent extra accumulation of HW warps in endpoint so it
                         * won't mess with PID controller. (only nedded for hw rate adjust)
                         */
                        set_override_ep_set_hw_warp_apply_mode(op_extra_data->spkr_endpoint, TRUE);

                        L2_DBG_MSG1("AEC REFERENCE: Speaker graph switched to timed play back mode: %d",
                                    op_extra_data->spkr_timed_playback_mode);
                    }
                    else
                    {
                        /* can't do anything except to retry later */
                    }
                }

#ifdef SPKR_TTP_DEBUG
                if(op_extra_data->spkr_void_tag_observed)
                {
                    L2_DBG_MSG2("AEC REFERENCE, stopped seeing VOID tags, time=%d, %d",
                                hal_get_time(), op_extra_data->spkr_void_tag_counter);
                }
#endif
                /* void tag not seen */
                op_extra_data->spkr_void_tag_observed = FALSE;

            } /* IS_TIMESTAMPED_TAG(mtag) */
            else if(IS_VOID_TTP_TAG(mtag) && op_extra_data->spkr_timed_playback_mode)
            {
#ifdef SPKR_TTP_DEBUG
                if(!op_extra_data->spkr_void_tag_observed)
                {
                    L2_DBG_MSG1("AEC REFERENCE, started seeing VOID tags, time=%d", hal_get_time());
                }
#endif
                op_extra_data->spkr_void_tag_observed = TRUE;
            }
        } /* if(mtag != NULL) */

        if(op_extra_data->spkr_last_timestamp_valid)
        {
            /* We know the time stamp for first sample in input buffer
             * Now work out how much delay is ahead of that sample.
             *
             * See how many samples are in output (MMU) buffer and
             * convert that to time. There could be up to 1 sample jitter.
             *
             * TODO_AEC_REFERENCE_TTP: since the task for speaker graph is timer based, the time
             * of sampling isn't aligned with the output consuming time, so it
             * could have up to one sample period random jitter.
             */
            TIME cur_time = hal_get_time();
            unsigned amount_in_output = cbuffer_calc_amount_data_in_words(op_extra_data->output_stream[AEC_REF_SPKR_TERMINAL1]);
            TIME_INTERVAL offset = convert_samples_to_time(amount_in_output, op_extra_data->spkr_rate);

            /* if we have SW rate adjustment in the graph */
            if(op_extra_data->spkr_sw_rateadj_op != NULL)
            {
                /* adjust for the phase difference between first input and first output sample */
                offset -= (int)frac_mult(SECOND, (int)cbops_sra_get_phase(op_extra_data->spkr_sw_rateadj_op)) / (int) op_extra_data->spkr_rate;
            }

            /* Add extra delay that might occur in the path
             * (adjustment for any resampler, HW rate matching,
             *  cbops algorithmic delays and any external delay)
             * TODO_AEC_REFERENCE_TTP: spkr_extra_delay isn't set yet
             */
            offset += op_extra_data->spkr_extra_delay;

            /* compute error */
            *error = time_sub(op_extra_data->spkr_last_timestamp, cur_time) - offset;
        }

        return op_extra_data->spkr_last_timestamp_valid;
    } /* buff_has_metadata  */

    /* always return invalid error if we don't have metadata buffer */
    return FALSE;
}

/**
 * aec_reference_spkr_ttp_setup_timed_playback
 * \brief does necessary changes for speaker graph to work in timed
 *  playback mode.
 *
 * \param op_extra_data pointer to AEC_REFERENCE_OP_DATA data
 * return TRUE if setting up was successful else FALSE.
 */
static bool aec_reference_spkr_ttp_setup_timed_playback(AEC_REFERENCE_OP_DATA *op_extra_data)
{
    patch_fn_shared(aec_reference);

    /* initialise pid controller parameters */
    timed_set_pid_controller_default_settings(&op_extra_data->spkr_pid_params);



    /* reset speaker ttp control */
    aec_reference_spkr_ttp_reset(op_extra_data);

    /* use ttp SW rate adjust if speaker is capable */
    if(op_extra_data->spkr_rate_ability == RATEMATCHING_SUPPORT_HW)
    {
        op_extra_data->spkr_ttp_adjust_type = RATEMATCHING_SUPPORT_HW;

        /* we want to do HW rate matching in timed playback mode */
        if(op_extra_data->spkr_sw_rateadj_op != NULL)
        {
            /* if we have SW rate adjust, then set it to pass-through mode */
            cbops_rateadjust_passthrough_mode(op_extra_data->spkr_sw_rateadj_op, TRUE);
        }

        /* fresh start from 0 warp value */
        set_override_ep_ratematch_adjustment(op_extra_data->spkr_endpoint, 0);
    }
#ifdef INSTALL_DELEGATE_RATE_ADJUST_SUPPORT
    else if(0 != op_extra_data->spkr_ext_rate_adjust_op)
    {
        /* we will do software TTP using a standalone rate adjust operator */
        op_extra_data->spkr_ttp_adjust_type = RATEMATCHING_SUPPORT_SW;
        op_extra_data->spkr_rate_adjustment = 0;

        /* reset the current rate */
        stream_delegate_rate_adjust_set_current_rate(op_extra_data->spkr_ext_rate_adjust_op, 0);

        /* not in passthrough mode */
        stream_delegate_rate_adjust_set_passthrough_mode(op_extra_data->spkr_ext_rate_adjust_op, FALSE);

        /* we shouldn't have created built-in rate adjust operator */
        PL_ASSERT(op_extra_data->spkr_sw_rateadj_op == NULL);
    }
#endif /* INSTALL_DELEGATE_RATE_ADJUST_SUPPORT */
    else if(op_extra_data->spkr_sw_rateadj_op != NULL)
    {
        /* We have a rate adjust operator, so we can do software TTP */
        op_extra_data->spkr_ttp_adjust_type = RATEMATCHING_SUPPORT_SW;
        op_extra_data->spkr_rate_adjustment=0;
        cbops_sra_set_rate_adjust(op_extra_data->spkr_sw_rateadj_op, op_extra_data->num_spkr_channels, 0);
		/* force coming out of pass-through mode */
        cbops_rateadjust_passthrough_mode(op_extra_data->spkr_sw_rateadj_op, FALSE);
    }
    else
    {
        /* For doing TTP playback either it should support HW rate adjustment
         * or have a SW rate adjust operator in the graph.
         */
        return FALSE;
    }

	/* even if we aren't told, speaker graph is enacting */
	op_extra_data->spkr_rate_enactment = op_extra_data->spkr_rate_ability;

    return TRUE;
}

/**
 * aec_reference_spkr_ttp_reset
 * \brief reset timed playback mode internal state
 * \param op_extra_data pointer to AEC_REFERENCE_OP_DATA data
 * \param error difference between expected ttp and now
 */
static void aec_reference_spkr_ttp_reset(AEC_REFERENCE_OP_DATA *op_extra_data)
{
    patch_fn_shared(aec_reference);

    op_extra_data->spkr_ttp_error_acc = 0;
    op_extra_data->spkr_ttp_error_acc_cnt = 0;
    op_extra_data->spkr_last_ttp_error = 0;
    op_extra_data->spkr_error_threshold = SPKR_TTP_MAX_LATENCY_LOW_THRESHOLD_US;

    /* pid controller will be started */
    timed_reset_pid_controller(&op_extra_data->spkr_pid_state);
}

/**
 * aec_reference_spkr_ttp_error_control
 * \brief running PID controller to minimise the ttp error
 * \param pointer to AEC_REFERENCE_OP_DATA data
 * \param error difference between expected and estimated time to play
 */
static void aec_reference_spkr_ttp_error_control(AEC_REFERENCE_OP_DATA *op_extra_data, TIME_INTERVAL error)
{
    int error_diff;
    patch_fn_shared(aec_reference);

    /* ignore the error if we have a big jump in error */
    error_diff = error - op_extra_data->spkr_last_ttp_error;
    op_extra_data->spkr_last_ttp_error = error;
    if (pl_abs_i32(error_diff) > SPKR_TTP_MAX_ERROR_JUMP_US)
    {
        return;
    }

    /* accumulate error for averaging */
    op_extra_data->spkr_ttp_error_acc_cnt++;
    op_extra_data->spkr_ttp_error_acc += error;
    if(op_extra_data->spkr_ttp_error_acc_cnt == SPKR_TTP_ERROR_AVERAGING_PERIOD_MS)
    {
        /* calculate average error */
        int error_avg = frac_mult(op_extra_data->spkr_ttp_error_acc,
                                  FRACTIONAL(1.0/SPKR_TTP_ERROR_AVERAGING_PERIOD_MS));

        /* run controller to minimise the error */
        timed_run_pid_controller(&op_extra_data->spkr_pid_state, &op_extra_data->spkr_pid_params, error_avg);

#ifdef SPKR_TTP_DEBUG
        L2_DBG_MSG3("AEC_REFERENCE TTP error control, time=%d, error=%d, warp=%-8d",
                    hal_get_time(),
                    error_avg,
                    op_extra_data->spkr_pid_state.warp);
#endif
        /* apply the last calculated warp rate */
        aec_reference_spkr_ttp_adjust_rate(op_extra_data, op_extra_data->spkr_pid_state.warp);         /* reset the averaging accumulator */

        op_extra_data->spkr_ttp_error_acc_cnt = 0;
        op_extra_data->spkr_ttp_error_acc = 0;
    }
}

/**
 * aec_reference_spkr_ttp_adjust_rate
 * \brief applies warp rate to enacting sra
 * \param pointer to AEC_REFERENCE_OP_DATA data
 * \param warp warp value
 */
static void aec_reference_spkr_ttp_adjust_rate(AEC_REFERENCE_OP_DATA *op_extra_data, int warp)
{
    patch_fn_shared(aec_reference);

    /* expect to come here only in timed playback mode */
    PL_ASSERT(op_extra_data->spkr_timed_playback_mode);

    if(op_extra_data->spkr_ttp_adjust_type == RATEMATCHING_SUPPORT_HW)
    {
        /* set the HW warp value by sending that to real audio ep
         * NOTE TODO_AEC_REFERENCE_TTP: HW warp hasn't been tested */
        set_override_ep_ratematch_adjustment(op_extra_data->spkr_endpoint, warp);
    }
    else if(op_extra_data->spkr_ttp_adjust_type == RATEMATCHING_SUPPORT_SW)
    {
#ifdef INSTALL_DELEGATE_RATE_ADJUST_SUPPORT
        if(0 != op_extra_data->spkr_ext_rate_adjust_op)
        {
            /* rate adjust using external rate adjust operator */
            stream_delegate_rate_adjust_set_current_rate(op_extra_data->spkr_ext_rate_adjust_op, warp);
        }
        else
#endif /* INSTALL_DELEGATE_RATE_ADJUST_SUPPORT */
        {
            /* set the SW rate adjust warp value */
            cbops_sra_set_rate_adjust(op_extra_data->spkr_sw_rateadj_op, op_extra_data->num_spkr_channels, warp);
        }
    }
}
#endif /* AEC_REFERENCE_SPKR_TTP */

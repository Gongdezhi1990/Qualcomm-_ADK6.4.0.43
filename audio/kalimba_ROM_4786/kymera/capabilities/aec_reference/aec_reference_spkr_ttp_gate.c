/**
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 * \file  aec_reference_spkr_ttp_gate.c
 * \ingroup  capabilities
 *
 *  AEC Reference
 *
 */

/****************************************************************************
Include Files
*/
#include "aec_reference_cap_c.h"
#ifdef AEC_REFERENCE_SPKR_TTP_GATE
#include "platform/pl_intrinsics.h"
#include "platform/pl_fractional.h"
#include "aec_reference_config.h"
#include "op_msg_helpers.h"
#include "patch/patch.h"
#include "pl_assert.h"

/****************************************************************************
Private Constant Definitions
*/
/* Maximum number of ISRs the gate can be closed,
 * the gate normally will open much earlier, however
 * if error doesn't become stable by this time the
 * gate will open anyway.
 */
#define SPKR_TTP_GATE_MAX_CLOSED_TIME 500

/* acceptable error threshold to open the gate */
#define SPKR_TTP_GATE_ERROR_THRESHOLD_US 125

/* default/max/min values for initial delay
 * in the error check process. All in milliseconds
 * or more accurately in number of ISRs.
 */
#define SPKR_TTP_GATE_DEFAULT_DELAY_TIME_MS 100
#define SPKR_TTP_GATE_MIN_DELAY_TIME_MS 50
#define SPKR_TTP_GATE_MAX_DELAY_TIME_MS 300

/* used for higher precision in error averaging */
#define SPKR_TTP_POST_GATE_ERROR_SHIFT 8

/* max expected error for post gate, this will only
 * be used to limit the amount of warp fix,
 * in microseconds.
 */
#define SPKR_TTP_POST_GATE_MAX_ERROR_US 1000

/* coefficient for error tracking filter
 * An arbitrary value but provided sufficient
 * tracking performance with 1ms ISRs.
 */
#define SPKR_TTP_POST_GATE_AVG_COEFF FRACTIONAL(0.01)

/* post gate lower error threshold, action will stop
 * if error goes below this value, in microseconds.
 */
#define SPKR_TTP_POST_GATE_LOW_THRESHOLD_US  250

/* post gate higher error threshold, post gate action
 * will start if error goes above this value,
 * in microseconds.
 */
#define SPKR_TTP_POST_GATE_HIGH_THRESHOLD_US 500

/* maximum warp fix for latency drift in ppm,
 * This is to compensate for the drift when doing
 * open loop rate matching. The assumption is the
 * drift is less than 10ppm.
 */
#define SPKR_TTP_POST_GATE_MAX_WARP_FIX_PPM 10

/* convert ppm to a multiplication factor,
 * so this will turn 1000us error to 10PPM warp fix */
#define SPKR_TTP_POST_GATE_KP FRACTIONAL (                              \
        (SPKR_TTP_POST_GATE_MAX_WARP_FIX_PPM<<(DAWTH-1-SPKR_TTP_POST_GATE_ERROR_SHIFT)) \
        * 1e-6 /SPKR_TTP_POST_GATE_MAX_ERROR_US)

/* speaker ttp gate states */
enum
{
    /* initialising the process */
    SPKR_TTP_GATE_INIT = 0,

    /* Delaying the process
     */
    SPKR_TTP_GATE_DELAY_STAGE = 1,

    /* error is checked, positive errors will cause
     * running cbops with no input, negative errors will
     * cause discarding some inputs. Goes to
     * SPKR_TTP_GATE_DONE once error is
     * within certain limits
     */
    SPKR_TTP_GATE_ERROR_CHECK = 2,

    /* All done, the gate is open and no error check
     * will be done
     */
    SPKR_TTP_GATE_DONE = 3
};

/****************************************************************************
Private Function Declarations
*/
static unsigned aec_reference_spkr_ttp_gate_discard_samples(AEC_REFERENCE_OP_DATA  *op_extra_data, unsigned amount_to_discard);
static void aec_reference_spkr_post_ttp_gate_drift_control(AEC_REFERENCE_OP_DATA  *op_extra_data, TIME_INTERVAL error);

/****************************************************************************
public Function Definitions
*/
/**
 * aec_reference_check_speaker_ttp_gate
 * \brief checks whether speaker ttp gate should be open.
 *
 * \param op_extra_data Pointer to the AEC reference operator specific data.
 * \param error current ttp error for speaker graph
 * \param max_to_process pointer to maximum amount for cbops to process (input and output)
 */
void aec_reference_check_speaker_ttp_gate(AEC_REFERENCE_OP_DATA  *op_extra_data, TIME_INTERVAL error, unsigned *max_to_process)
{

    patch_fn_shared(aec_reference);
    /* return if the feature isn't enabled */
    if(!op_extra_data->spkr_ttp_gate_enable)
    {
        return;
    }

    /* see if gate is already open */
    if(op_extra_data->spkr_ttp_gate_state == SPKR_TTP_GATE_DONE)
    {
        /* see if post gate error limit is enabled */
        if(op_extra_data->spkr_post_ttp_gate_drift_control)
        {
            /* latency drift control after opening the gate */
            aec_reference_spkr_post_ttp_gate_drift_control(op_extra_data, error);
        }
        return;
    }

    /* if not open within maximum duration, then open
     * the gate an exit
     */
    if(op_extra_data->spkr_ttp_gate_counter >=SPKR_TTP_GATE_MAX_CLOSED_TIME)
    {
        L2_DBG_MSG1("AEC REFERENCE SPEAKER GATE: gate opened without adjusting error, current error=%d", error);
        op_extra_data->spkr_ttp_gate_state = SPKR_TTP_GATE_DONE;
        return;
    }

    switch(op_extra_data->spkr_ttp_gate_state)
    {
        case SPKR_TTP_GATE_INIT:
            L3_DBG_MSG("AEC REFERENCE SPEAKER GATE: process started");
            /* Any initialisations that are needed */
            op_extra_data->spkr_ttp_gate_state = SPKR_TTP_GATE_DELAY_STAGE;
            /* FALLTHROUGH */

        case SPKR_TTP_GATE_DELAY_STAGE:
            if(error < 0)
            {
                /* discard some data */
                unsigned amount_discarded;
                int amount_to_discard = frac_mult(-error, frac_div(op_extra_data->input_rate, SECOND));
                amount_to_discard = MIN(amount_to_discard, *max_to_process);
                amount_discarded = aec_reference_spkr_ttp_gate_discard_samples(op_extra_data, amount_to_discard);
                L4_DBG_MSG2("AEC REFERENCE SPEAKER GATE, discarded=%d error=%d", amount_discarded, error);
            }
            if(op_extra_data->spkr_ttp_gate_counter >= op_extra_data->spkr_ttp_gate_delay_period)
            {
                /* move to error check state */
                op_extra_data->spkr_ttp_gate_state = SPKR_TTP_GATE_ERROR_CHECK;
                L3_DBG_MSG1("AEC REFERENCE SPEAKER GATE, delay period ends  error=%d", error);
            }
            *max_to_process = 0;
            break;

        case SPKR_TTP_GATE_ERROR_CHECK:

            if(error > SPKR_TTP_GATE_ERROR_THRESHOLD_US)
            {
                /* cbops to process no input, buffer will fill up until ttp error becomes near 0 */
                *max_to_process = 0;
                L3_DBG_MSG1("AEC REFERENCE SPEAKER GATE, error is too high: %d", error);
            }
            else if(error < -SPKR_TTP_GATE_ERROR_THRESHOLD_US)
            {
                /* discard some data to move ttp error towards 0 */
                unsigned amount_discarded;
                int amount_to_discard = frac_mult(-error, frac_div(op_extra_data->input_rate, SECOND));
                amount_to_discard = MIN(amount_to_discard, *max_to_process);
                amount_discarded = aec_reference_spkr_ttp_gate_discard_samples(op_extra_data, amount_to_discard);
                L3_DBG_MSG2("AEC REFERENCE SPEAKER GATE, error is too low: error=%d, discarded=%d", error, amount_discarded);
                *max_to_process -= amount_discarded;
            }
            else
            {
                /* open the gate */
                op_extra_data->spkr_ttp_gate_state = SPKR_TTP_GATE_DONE;
                L2_DBG_MSG2("AEC REFERENCE SPEAKER GATE: -- GATE OPENED --  error=%d, period closed=%dms",
                            error, op_extra_data->spkr_ttp_gate_counter);
            }
            break;

        default:
            break;
    }

    /* increment the times that the gate has been closed */
    op_extra_data->spkr_ttp_gate_counter++;
}

/**
 * aec_reference_spkr_ttp_gate_init
 * \brief initialises the speaker ttp gate
 * \param op_extra_data Pointer to the AEC reference operator specific data.
 */
void aec_reference_spkr_ttp_gate_init(AEC_REFERENCE_OP_DATA  *op_extra_data)
{
    patch_fn_shared(aec_reference);

    /* initialisations */
    op_extra_data->spkr_ttp_gate_state = SPKR_TTP_GATE_INIT;
    op_extra_data->spkr_ttp_gate_counter = 0;
    op_extra_data->spkr_post_ttp_gate_average_error = 0;
    op_extra_data->spkr_post_ttp_gate_active = FALSE;
}

/**
 * aec_reference_opmsg_enable_spkr_ttp_gate
 * \brief message handler for enabling speaker ttp gate
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the start request message
 * \param resp_length pointer to location to write the response message length
 * \param response_data Location to write a pointer to the response message
 *
 * \return TRUE if successful else FALSE
 */
bool aec_reference_opmsg_enable_spkr_ttp_gate(OPERATOR_DATA *op_data, void *message_data,
                                              unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    AEC_REFERENCE_OP_DATA   *op_extra_data = (AEC_REFERENCE_OP_DATA*)op_data->extra_op_data;
    int idx;

    patch_fn_shared(aec_reference);

    /* only allow this when the op isn't running */
    if(OP_RUNNING == op_data->state)
    {
        return FALSE;
    }

    /* also we cannot change the config while any speaker input is connected */
    for(idx=0; idx < MAX_NUMBER_SPEAKERS; idx++)
    {
        if(NULL !=  op_extra_data->input_stream[SpeakerInputTerminalByIndex(idx)])
        {
            return FALSE;
        }
    }

    /* set the enable for speaker TTP gate */
    op_extra_data->spkr_ttp_gate_enable  = (OPMSG_FIELD_GET(message_data, OPMSG_AEC_ENABLE_SPKR_INPUT_GATE, ENABLE) != 0);

    /* read other settings only if gate is enabled */
    if(op_extra_data->spkr_ttp_gate_enable)
    {
        /* get the initial delay period */
        unsigned delay_period = OPMSG_FIELD_GET(message_data, OPMSG_AEC_ENABLE_SPKR_INPUT_GATE, INITIAL_DELAY);
        if(delay_period == 0)
        {
            /* interpret 0 as default */
            delay_period = SPKR_TTP_GATE_DEFAULT_DELAY_TIME_MS;
        }
        /* apply upper and lower limit */
        delay_period = MAX(delay_period, SPKR_TTP_GATE_MIN_DELAY_TIME_MS);
        delay_period = MIN(delay_period, SPKR_TTP_GATE_MAX_DELAY_TIME_MS);
        op_extra_data->spkr_ttp_gate_delay_period = delay_period;

        /* post gate drift control must be requested */
        op_extra_data->spkr_post_ttp_gate_drift_control =
            (OPMSG_FIELD_GET(message_data, OPMSG_AEC_ENABLE_SPKR_INPUT_GATE, POST_GATE_DRIFT_CONTROL) != 0);

        /* set post gate convergence gain */
        op_extra_data->spkr_post_ttp_gate_kp = SPKR_TTP_POST_GATE_KP;

        L2_DBG_MSG2("AEC REFERENCE speaker ttp gate enabled, delay_period=%dms, post_gate_drift_control=%d",
                    op_extra_data->spkr_ttp_gate_delay_period, op_extra_data->spkr_post_ttp_gate_drift_control);
    }
    else
    {
        /* gate isn't enabled, make sure post gate drift control is also off */
        op_extra_data->spkr_post_ttp_gate_drift_control = FALSE;
        L2_DBG_MSG("AEC REFERENCE speaker ttp gate disabled");
    }

    return TRUE;
}

/**
 * aec_reference_spkr_post_ttp_gate_fixing_value
 * \brief calculates the amount of extra warp value needed
 * \param op_extra_data Pointer to the AEC reference operator specific data.
 *
 * \return amount of extra warp needed
 */
int aec_reference_spkr_post_ttp_gate_fixing_value(AEC_REFERENCE_OP_DATA  *op_extra_data)
{
    patch_fn_shared(aec_reference);

    if(op_extra_data->spkr_post_ttp_gate_active)
    {
        /* turn average error to a warp amount */
        int fix_value = frac_mult(op_extra_data->spkr_post_ttp_gate_average_error, op_extra_data->spkr_post_ttp_gate_kp);

        /* limit fix value, we only want to compensate for small rate drifts in
         * open loop rate matching
         */
        fix_value = MIN(fix_value, FRACTIONAL(SPKR_TTP_POST_GATE_MAX_WARP_FIX_PPM*1e-6));
        fix_value = MAX(fix_value, -FRACTIONAL(SPKR_TTP_POST_GATE_MAX_WARP_FIX_PPM*1e-6));

        return fix_value;
    }
    else
    {
        /* no extra warp is needed, error is in good shape */
        return 0;
    }
}

/****************************************************************************
Private Function Definitions
*/
/**
 * aec_reference_spkr_ttp_gate_discard_samples
 * \brief discard some samples from speaker graph input
 *
 * \param op_extra_data Pointer to the AEC reference operator specific data.
 * \param amount_to_discard number of samples to discard
 *
 * \return number of samples discarded
 */
static unsigned aec_reference_spkr_ttp_gate_discard_samples(AEC_REFERENCE_OP_DATA  *op_extra_data, unsigned amount_to_discard)
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

/**
 * aec_reference_spkr_post_ttp_gate_drift_control
 * \brief controls the drift in latency after gate opens
 *
 * \param op_extra_data Pointer to the AEC reference operator specific data.
 * \param error current estimation of ttp error
 */
static void aec_reference_spkr_post_ttp_gate_drift_control(AEC_REFERENCE_OP_DATA  *op_extra_data, TIME_INTERVAL error)
{
    patch_fn_shared(aec_reference_run);

    /* smoothing estimated error */
    op_extra_data->spkr_post_ttp_gate_average_error +=
        frac_mult((error<<SPKR_TTP_POST_GATE_ERROR_SHIFT) - op_extra_data->spkr_post_ttp_gate_average_error, SPKR_TTP_POST_GATE_AVG_COEFF);

    /* This isn't a tight error control loop, we want to avoid intervention until
     * it is really required, so we only supervise the error using a hysteresis
     * process. We start intervention if error has reached a high threshold, but
     * then we continue the intervention until the error reaches a low enough level.
     * The intention isn't to provide an error near 0, but an error in acceptable
     * range and it will be allowed to vary within that range. With this we make sure
     * error doesn't drift in long run and we also minimise having impact on the main
     * open loop rate matching mechanism.
     */
    if(op_extra_data->spkr_post_ttp_gate_active)
    {
        /* drift limitation is active, go inactive if error is low enough */
        if(pl_abs_i32(op_extra_data->spkr_post_ttp_gate_average_error) <
           (SPKR_TTP_POST_GATE_LOW_THRESHOLD_US<<SPKR_TTP_POST_GATE_ERROR_SHIFT))
        {
            /* no action needed, error is low enough */
            op_extra_data->spkr_post_ttp_gate_active = FALSE;
        }
    }
    else
    {
        /* it's inactive, go active if error is high enough */
        if(pl_abs_i32(op_extra_data->spkr_post_ttp_gate_average_error) >
           (SPKR_TTP_POST_GATE_HIGH_THRESHOLD_US<<SPKR_TTP_POST_GATE_ERROR_SHIFT))
        {
            op_extra_data->spkr_post_ttp_gate_active = TRUE;
        }
    }
}
#endif /* AEC_REFERENCE_SPKR_TTP_GATE */

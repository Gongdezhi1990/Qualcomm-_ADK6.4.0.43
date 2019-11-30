// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// $Change$  $DateTime$
// *****************************************************************************

// ASM functions for processing volume
// The function(s) obey the C compiler calling convention (see documentation, CS-124812-UG)

#include "portability_macros.h"
#include "vol_ctrl_gen_asm.h"
#include "volume_control_cap_asm_defs.h"

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif

// *****************************************************************************
// MODULE:
//    $_vol_ctrl_update_aux_state
//    void vol_ctrl_update_aux_state(vol_ctrl_data_t *op_extra_data,unsigned num_auxs,vol_time_constants_t *lpvcs);
//
// INPUTS:
//      r0 = Pointer to capability data object
//      r1 = Bit mask of AUX channels to check
//      r2 = Pointer to period's time constants
// OUTPUTS:
//      none
// DESCRIPTION:
//    Update state of each AUX channel.  Function is C compatible.
//    Amount of data in each aux channel, or 0 if not connected,
//    has been put in aux_channel[i].advance_buffer.
//
// *****************************************************************************

// Frame Pointer Referesnces
.CONST $vol_ctrl.axfp.aux_ptr       1*ADDR_PER_WORD;  // r0
.CONST $vol_ctrl.axfp.aparm_ptr     2*ADDR_PER_WORD;  // r1
.CONST $vol_ctrl.axfp.num_words     3*ADDR_PER_WORD;  // r2
.CONST $vol_ctrl.axfp.period        4*ADDR_PER_WORD;  // r3

.MODULE $M.vol_ctrl_update_aux_state;
    .CODESEGMENT PM;

$_vol_ctrl_update_aux_state:
    pushm <r5,r7>;

#if defined(PATCH_LIBS)
    LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($vol_ctrl_update_aux_state.PATCH_ID_0, r7)
#endif
    // Check if any AUX to monitor
    r7 = r1;
    if Z jump update_aux_state_abort;

    r5 = r0;

    /* Setup Parameters Pointer */
    r1 = r0 + ($volume_control_cap.vol_ctrl_data_struct.PARAMETERS_FIELD +
               $volume_control_cap._tag_VOL_CTRL_PARAMETERS_struct.OFFSET_AUX1_SCALE_FIELD);
    /* Setup Aux Pointer */
    r0 = r5 + $volume_control_cap.vol_ctrl_data_struct.AUX_CHANNEL_FIELD;
    /* setup period */
    r3 = M[r2 + $volume_control_cap.vol_time_constants_struct.PERIOD_FIELD];
    /* Get number of words to process */
    r2 = M[r2 + $volume_control_cap.vol_time_constants_struct.NUM_WORDS_FIELD];

    pushm <FP(=SP),r0,r1,r2,r3,r4,r6,r8,rLink>;

    r8 = NULL;
    r6 = 1;

    // r2: unsigned num_words
    // r3: unsigned time_prd
vol_ctrl_update_aux_state_next:

    // r0: vol_ctrl_aux_channel_t *aux
    // r1: vol_ctrl_aux_params_t *aux_params

    // Get aux->state
    r4 = M[r0 + $volume_control_cap.vol_ctrl_aux_channel_struct.STATE_FIELD];

    // amount = aux_ptr->advance_buffer
    //        = aux_ptr->buffer ? cbuffer_calc_amount_data_in_words(aux_ptr->buffer) : 0
    r0 = M[r0 + $volume_control_cap.vol_ctrl_aux_channel_struct.ADVANCE_BUFFER_FIELD];

    // Is Aux Data Present in sufficient quantity to mix?
    r1 = M[FP + $vol_ctrl.axfp.num_words];
    NULL = r1 - r0;
    if GT jump update_aux_state_insufficient_data;

    update_aux_state_sufficient_data:
        r3 = M[FP + $vol_ctrl.axfp.aux_ptr];
        // aux_ptr->advance_buffer = volume_tc.num_words;
        M[r3 + $volume_control_cap.vol_ctrl_aux_channel_struct.ADVANCE_BUFFER_FIELD]=r1;

        NULL = r4 - $volume_control_cap.AUX_STATE_IN_AUX;
        if Z jump update_aux_state_done;
        /* Not in IN_AUX state */
        NULL = r4 - $volume_control_cap.AUX_STATE_START_AUX;
        if Z jump update_aux_state_in_AUX_START;
            /* Not in START_AUX state, enter START_AUX state
               Start count down.  Don't use data until in IN_AUX state */
            r1 = $volume_control_cap.AUX_STATE_START_AUX;
            M[r3 + $volume_control_cap.vol_ctrl_aux_channel_struct.STATE_FIELD] = r1;
            M[r3 + $volume_control_cap.vol_ctrl_aux_channel_struct.TRANSITION_FIELD] = NULL;
           // jump update_aux_do_not_use_data;
        update_aux_state_in_AUX_START:
            /* Continue count down */
            r0 = M[FP + $vol_ctrl.axfp.period];
            r1 = M[FP + $vol_ctrl.axfp.aparm_ptr];
            r1 = M[r1 + $volume_control_cap.vol_ctrl_aux_params_struct.ATK_TC_FIELD];
            rMAC = r0 * r1;
            rMAC = rMAC ASHIFT (DAWTH-1) (56bit);
            r0 = M[r3 + $volume_control_cap.vol_ctrl_aux_channel_struct.TRANSITION_FIELD];
            r2 = 1.0;
            rMAC = rMAC + r0;
            if V rMAC = r2;
            M[r3 + $volume_control_cap.vol_ctrl_aux_channel_struct.TRANSITION_FIELD] = rMAC;
            NULL = rMAC - r2;
            if NZ jump update_aux_do_not_use_data;
            /* Entering IN_AUX state */
            r1 = $volume_control_cap.AUX_STATE_IN_AUX;
            M[r3 + $volume_control_cap.vol_ctrl_aux_channel_struct.STATE_FIELD] = r1;
            jump update_aux_state_done;

     update_aux_state_insufficient_data:
        r3 = M[FP + $vol_ctrl.axfp.aux_ptr];
        // aux_ptr->advance_buffer = amount;

        NULL = r4 - $volume_control_cap.AUX_STATE_NO_AUX;
        if Z jump update_aux_do_not_use_data;
        /* Not in NO_AUX state */
        NULL = r4 - $volume_control_cap.AUX_STATE_END_AUX;
        if Z jump update_aux_state_in_AUX_END;
            /* Not in END_AUX state, enter END_AUX state
               Start count down.  Don't purge data until in NO_AUX state */
            r1 = $volume_control_cap.AUX_STATE_END_AUX;
            M[r3 + $volume_control_cap.vol_ctrl_aux_channel_struct.STATE_FIELD] = r1;
            r1 = 1.0;
            M[r3 + $volume_control_cap.vol_ctrl_aux_channel_struct.TRANSITION_FIELD] = r1;
           // jump update_aux_do_not_use_data;
       update_aux_state_in_AUX_END:
            /* Continue count down */
            r0 = M[FP + $vol_ctrl.axfp.period];
            r1 = M[FP + $vol_ctrl.axfp.aparm_ptr];
            r1 = M[r1 + $volume_control_cap.vol_ctrl_aux_params_struct.DEC_TC_FIELD];
            rMAC = r0 * r1;
            rMAC = rMAC ASHIFT (DAWTH-1) (56bit);
            r2 = M[r3 + $volume_control_cap.vol_ctrl_aux_channel_struct.TRANSITION_FIELD];
            rMAC = r2 - rMAC;
            if NEG rMAC=NULL;
            M[r3 + $volume_control_cap.vol_ctrl_aux_channel_struct.TRANSITION_FIELD] = rMAC;
            if GT jump update_aux_do_not_use_data;
            /* Entering NO_AUX state */
            r1 = $volume_control_cap.AUX_STATE_NO_AUX;
            M[r3 + $volume_control_cap.vol_ctrl_aux_channel_struct.STATE_FIELD] = r1;
            M[r3 + $volume_control_cap.vol_ctrl_aux_channel_struct.TRANSITION_FIELD] = NULL;
            jump update_aux_state_done;

update_aux_do_not_use_data:
    M[r3 + $volume_control_cap.vol_ctrl_aux_channel_struct.ADVANCE_BUFFER_FIELD] = NULL;
update_aux_state_done:

    // Next aux channel
    r0 = M[FP + $vol_ctrl.axfp.aux_ptr];

    /* Is NO_AUX */
    NULL = M[r0 + $volume_control_cap.vol_ctrl_aux_channel_struct.STATE_FIELD];
    if NZ r8 = r8 OR r6;
    r0 = r0 + ($volume_control_cap.vol_ctrl_aux_channel_struct.STRUC_SIZE*ADDR_PER_WORD);
    M[FP + $vol_ctrl.axfp.aux_ptr]=r0;

    r1 = M[FP + $vol_ctrl.axfp.aparm_ptr];
    r1 = r1 + ($volume_control_cap.vol_ctrl_aux_params_struct.STRUC_SIZE*ADDR_PER_WORD);
    M[FP + $vol_ctrl.axfp.aparm_ptr]=r1;

    r6 = r6 LSHIFT 1;
    r7 = r7 LSHIFT -1;
    if NZ jump vol_ctrl_update_aux_state_next;

    /* Save overall state */
    M[r5 + $volume_control_cap.vol_ctrl_data_struct.AUX_ACTIVE_FIELD] = r8;

    popm <FP,r0,r1,r2,r3,r4,r6,r8,rLink>;

update_aux_state_abort:
    popm <r5,r7>;
    rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_vol_ctrl_compute_time_constants
//    void vol_ctrl_compute_time_constants(unsigned sample_rate,unsigned vol_tc,vol_time_constants_t *lpvcs);
//
// INPUTS:
//      r0 = sample rate of channels
//      r1 = Time constant parameter
//      r2 = Pointer to period's time constants
// OUTPUTS:
//      none
// DESCRIPTION:
//    Compute Time Consants used for volume.  Function is C compatible
//
// ------------------------------------------------------------------
// calculate coefficients, we have received n=r0 sample to process
// There are two coefficients to compute, volume ramp up/down coeff,
// which only is used if master volume or a trim volume has changed, and
// limiter attack/release coeff which only is used if limiter is triggered
//
// both of these coefficient calculated once per run, this is because
// we don't want to update limiter/volume gains per sample. Since the
// amount to process is changeable the coefficients are so too.
//
// for volume update maximum volume change is limited like the following:
//     max_volume_change =  vol_max_change_coeff * current_volume + offset_volume_change
//     where vol_max_change_coeff = (10^(5*n/fs) - 1 ~= 11.5*(n/fs) where n is
//     the amount of input sample to process this time, this gives around up to
//     1dB increase of volume per 10ms.
//
// For limiter, normally it has to have a look ahead buffer so it can attack
// above threshold inputs smoothly, here we don't have look ahead buffers for
// input channels (we cant afford it) and we want to support variable amount
// of samples to process each run. The attack and release coeffs for limiter
// is used when the the peak of all inputs go above or below threshold.
//    current_gain += (target_gain - current_gain)*attack_coeff/release_coeff
//    the attack and release coeffs are also dependent on n/fs, we want to have
//    10ms attack time and 50ms release time.
//    release_coeff ~= 1 - exp((log(0.01)/0.05/)*n/fs) = 1 - 2^(-132n/fs)
//    attack_coeff   = 1 - exp((log(0.01)/0.01/)*n/fs) = 1 - 2^(-664n/fs)
// *****************************************************************************
.MODULE $M.vol_ctrl_update_timeConstants;
    .CODESEGMENT PM;

    .CONST POS_ONE_Q16_N   (1<<(DAWTH-16));  // 1.0 in Q16.8 (arch4: Q16.16)

$_vol_ctrl_compute_time_constants:
    pushm <r4,r5,r6,r7,rlink>;
// r0:  unsigned sample_rate
// r1:  unsigned vol_tc  (r-1)
// r2:  vol_time_constants_t *lpvcs

    r3 = r1;
    r1 = M[r2+$volume_control_cap.vol_time_constants_struct.NUM_WORDS_FIELD];

    // compute period in 100th of msec  (samples/sample_rate)*100,000
    rMAC = r1 * 3125 (int);
    rMAC = rMAC LSHIFT (5-DAWTH) (56bit);
    Div  = rMAC / r0;
    r4 = DivResult;
    M[r2 + $volume_control_cap.vol_time_constants_struct.PERIOD_FIELD] = r4;

    r4 = r4 ASHIFT (DAWTH-14);
    // compute volume update coeff: 11.5*n/fs (rougly 0.1dB/ms)
    r5 = r4 * 0.942071589546666;      // update coef in Q1.23 (arch4: Q1.31)
    // positive VOL_TC will increase the rate.
    // modified_coef ~= coef*r*(1+(r-1)*coef) (r = ramp factor)
    r1 = r3 + POS_ONE_Q16_N;
    if NEG r1 = 0;          // safegaurd
    rMAC = r1 * r5;
    r1 = rMAC ASHIFT 15;    // coef*r                in Q1.23 (arch4: Q1.31)
    rMAC = POS_ONE_Q16_N;
    rMAC = rMAC + r5 * r3;  // (1+(r-1)*coef)        in Q16.8 (arch4: Q16.16)
    rMAC = rMAC ASHIFT 8 (56bit);
    rMAC = rMAC * r1;       // coef*r*(1+(r-1)*coef) in Q8.16 (arch4: Q8.24)
    r1 = rMAC ASHIFT 7;     // convert modified_coef to Q1.23 (arch4: Q1.31)
    M[r2 + $volume_control_cap.vol_time_constants_struct.VOL_TC_FIELD] = r1;


   // calculate release coeff(reaches 99% in 50ms)
   r0 = r4 * 0.6553593446;
   r0 = r0 * (log(0.01)/0.05/log(2.0)/128.0/100.0)(frac);
   call $math.pow2_table;
   M[r2 + $volume_control_cap.vol_time_constants_struct.SAT_TC_FIELD] = r0;

   // attack coeff, 5 times faster
   r1 = r0 * r0 (frac);
   r1 = r1 * r1 (frac);
   r1 = r0 * r1 (frac);
   M[r2 + $volume_control_cap.vol_time_constants_struct.SAT_TCP5_FIELD] = r1;

   popm <r4,r5,r6,r7,rlink>;
   rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_vol_ctrl_update_channel
//    void vol_ctrl_update_channel(vol_ctrl_data_t *op_extra_data, vol_ctrl_channel_t *chan_ptr,vol_ctrl_gains_t *volptr,vol_time_constants_t *lpvcs)
//
// INPUTS:
//      r0 = Pointer to capability data object
//      r1 = Pointer to channel state info
//      r2 = Pointer to volume state info
//      r3 = Pointer to period's time constants
// OUTPUTS:
//      none
// DESCRIPTION:
//    Update channel.
//      Select Aux and compute mixing ratios.
//      Compute saturation prevention adjustment
//      Function is C compatible
//
// *****************************************************************************

// Some channel constants
.CONST $volume_and_limit.VOLUME_RAMP_OFFSET_CONST (0.0001/16.0);   // offset used for ramping up/down the volume
.CONST $volume_and_limit.OneQ5                    (1<<(DAWTH-5));  // 1.0 in Q5.XX
.CONST $volume_and_limit.MIN_POSITIVE_VOLUME      $volume_and_limit.OneQ5;  // 0dB volume
.CONST $music_example.LIMIT_THRESHOLD_LINEAR      ((10.0**(-1.0/20.0))/16.0);  // Threshold = -1dB


// Frame Pointer References
.CONST $vol_ctrl.cufp.op_data_ptr   1*ADDR_PER_WORD;
.CONST $vol_ctrl.cufp.chan_ptr      2*ADDR_PER_WORD;
.CONST $vol_ctrl.cufp.vol_ptr       3*ADDR_PER_WORD;
.CONST $vol_ctrl.cufp.tc_ptr        4*ADDR_PER_WORD;

.CONST $vol_ctrl.cuup.op_data_ptr   1*ADDR_PER_WORD;
.CONST $vol_ctrl.cuup.chan_ptr      2*ADDR_PER_WORD;
.CONST $vol_ctrl.cuup.tc_ptr        3*ADDR_PER_WORD;
.CONST $vol_ctrl.cuup.dummy         4*ADDR_PER_WORD;

#if (DAWTH==24)
#define MINUS90dBinQ5   0xFFEAE8
#elif (DAWTH==32)
#define MINUS90dBinQ5   0xFFFFEAE8
#else
#error "DAWTH not supported!"
#endif

.MODULE $M.vol_ctrl_update_channel;
    .MAXIM;
    .CODESEGMENT VOL_CTRL_UPD_CHAN_PM;

$_vol_ctrl_update_channel:
    pushm <r4,r5,r6,r7,r8,r9>;
    pushm <I0,I1,I4,I5,M3,L0,L4,L5>;
    pushm <B0,B4,B5>;
    pushm <FP(=SP),r0,r1,r2,r3,rLink>;

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($vol_ctrl_update_channel.PATCH_ID_0, r7)
#endif
    /* Number of channels */
    r7 = M[r0 + $volume_control_cap.vol_ctrl_data_struct.NUM_CHANNELS_FIELD];
    if LE jump vol_ctrl_apply_volume_abort;

    // Enable saturate on add/sub
    r4 = M[$ARITHMETIC_MODE];
    push r4;
    r4 = r4 OR $ADDSUB_SATURATE_ON_OVERFLOW_MASK;
    M[$ARITHMETIC_MODE] = r4;


    /* Update Channel for AUX stream priority.   */
    /* op_extra_data->aux_in_use  = 0; */
    M[r0 + $volume_control_cap.vol_ctrl_data_struct.AUX_IN_USE_FIELD] = NULL;


    /*
        for(i=0;i<op_extra_data->num_channels;i++)
        {
            vol_ctrl_update_channel(op_extra_data,&op_extra_data->channels[i],op_extra_data->lpvols,&volume_tc);
        }
    */
vol_ctrl_update_channel_next:
    push r7;

// r0: vol_ctrl_data_t      *op_extra_data
// r1: vol_ctrl_channel_t   *chan_ptr
// r2: vol_ctrl_gains_t     *volptr
// r3: vol_time_constants_t *lpvcs


    r4 = 1.0;
    M[r1 + $volume_control_cap.vol_ctrl_channel_struct.LIMITER_GAIN_LINEAR_FIELD]=r4;
    // chan_idx
    r4 = M[r1 + $volume_control_cap.vol_ctrl_channel_struct.CHAN_IDX_FIELD];
    // target_vol (r5) = volptr->master_gain;
    r5 = M[r2 + $volume_control_cap.vol_ctrl_gains_struct.MASTER_GAIN_FIELD];
    // channel_trim (r6) = volptr->channel_trims[chan_ptr->chan_idx];
    r6 = r4 * ADDR_PER_WORD (int);
    r6 = r6 + $volume_control_cap.vol_ctrl_gains_struct.CHANNEL_TRIMS_FIELD;
    r6 = M[r2 + r6];
    // chan_params (r7) = (vol_ctrl_chan_params_t*)&op_extra_data->parameters.OFFSET_CHAN1_AUX_ROUTE;
    // aux_routing (r9) = chan_params[chan_ptr->chan_idx].aux_routing;
    r8 = r0 + ($volume_control_cap.vol_ctrl_data_struct.PARAMETERS_FIELD + $volume_control_cap._tag_VOL_CTRL_PARAMETERS_struct.OFFSET_CHAN1_AUX_ROUTE_FIELD);
    r7 = r4 * ($volume_control_cap.vol_ctrl_chan_params_struct.STRUC_SIZE * ADDR_PER_WORD) (int);
    r7 = r7 + r8;
    r9 = M[r7 + $volume_control_cap.vol_ctrl_chan_params_struct.AUX_ROUTING_FIELD];
    // Pointer (I0) to Channel Aux prim_scale factors
    I0 = r7 + $volume_control_cap.vol_ctrl_chan_params_struct.PRIM_SCALE_FIELD;

    // Conditionally add in NDVC adjustment
    NULL = r9 AND $M.VOL_CTRL.CONSTANT.CHAN_NDVC_ENABLE_BIT;
    if Z jump vol_ctrl_update_channel_no_ndvc;
        r8 = M[r0 + $volume_control_cap.vol_ctrl_data_struct.SHARED_VOLUME_PTR_FIELD];
        r8 = M[r8 + $volume_control_cap._shared_volume_struct.NDVC_NOISE_LEVEL_FIELD];
	    /* NDVC is in 3dB steps */
        r8 = r8 * 180 (int);
        r6 = r6 + r8;
    vol_ctrl_update_channel_no_ndvc:

   /* Bypass Aux */
   NULL = M[r0 + $volume_control_cap.vol_ctrl_data_struct.AUX_ACTIVE_FIELD];
   if Z r9 = NULL;

    // channel_trim -= op_extra_data->post_gain;
    r8 = M[r0 + $volume_control_cap.vol_ctrl_data_struct.POST_GAIN_FIELD];
    r6 = r6 - r8;
    // target_vol = dB60toLinearQ5(target_vol+channel_trim);
    r0 = r6 + r5;
    call $_dB60toLinearQ5;

    // Update Channel Gain (r0 is target)
    r1 = M[FP + $vol_ctrl.cufp.chan_ptr];
    r2 = M[r1 + $volume_control_cap.vol_ctrl_channel_struct.CHANNEL_GAIN_FIELD];

    r3 = M[FP + $vol_ctrl.cufp.tc_ptr];
    r3 = M[r3 + $volume_control_cap.vol_time_constants_struct.VOL_TC_FIELD];

    r8 = r0 - r2;
    if Z jump vol_ctrl_update_channel_no_vol_change;
        r8 = ABS r8;
        r3 = r2 * r3 (frac);
        r3 = r3 + $volume_and_limit.VOLUME_RAMP_OFFSET_CONST;
        r3 = MIN r8;
        Null = r0 - r2;
        if NEG r3 = -r3;
        r2 = r2 + r3;
        M[r1 + $volume_control_cap.vol_ctrl_channel_struct.CHANNEL_GAIN_FIELD] = r2;
    vol_ctrl_update_channel_no_vol_change:

     r0 = M[FP + $vol_ctrl.cufp.op_data_ptr];
     r0 = r0 + $volume_control_cap.vol_ctrl_data_struct.AUX_CHANNEL_FIELD;

// r9=routing, r1=chan_ptr, I0=prim_scale, r0=aux_chan_ptr, r6=chan_trim, r4=chan_idx

    /* Check each aux in channel priority */
    M[r1 + $volume_control_cap.vol_ctrl_channel_struct.AUX_MIX_GAIN_FIELD]  = NULL;
    M[r1 + $volume_control_cap.vol_ctrl_channel_struct.AUX_BUFFER_FIELD]    = NULL;

    // while(aux_routing&VOL_CTRL_CONSTANT_AUX_PRIORITY_VALID_BIT)
vol_ctrl_update_channel_aux_prio_lp:
    NULL = r9 AND $M.VOL_CTRL.CONSTANT.AUX_PRIORITY_VALID_BIT;
    if NZ jump vol_ctrl_update_channel_aux_prio_valid;
        // There is no AUX
        r2 = $volume_and_limit.OneQ5;
        M[r1 + $volume_control_cap.vol_ctrl_channel_struct.PRIM_MIX_GAIN_FIELD]      = r2;
        jump vol_ctrl_update_channel_aux_prio_done;
vol_ctrl_update_channel_aux_prio_valid:
        // aux_idx =  aux_routing&VOL_CTRL_CONSTANT_AUX_PRIORITY_CHANNEL_MASK;
        r5 = r9 AND $M.VOL_CTRL.CONSTANT.AUX_PRIORITY_CHANNEL_MASK;
        // op_extra_data->aux_channel[aux_idx].state
        r8 = r5 * ($volume_control_cap.vol_ctrl_aux_channel_struct.STRUC_SIZE * ADDR_PER_WORD) (int);
        r8 = r8 + r0,   r3=M[I0,M1];    // r3 = prim_scale
        r7 = M[r8 + $volume_control_cap.vol_ctrl_aux_channel_struct.STATE_FIELD];
        if NZ jump vol_ctrl_update_channel_aux_prio_found;
            // NO_AUX, Try Next entry
            r9 = r9 LSHIFT -$M.VOL_CTRL.CONSTANT.AUX_PRIORITY_NUM_BITS;
            jump vol_ctrl_update_channel_aux_prio_lp;
vol_ctrl_update_channel_aux_prio_found:
        NULL = r7 - $volume_control_cap.AUX_STATE_IN_AUX;
        if Z jump vol_ctrl_update_channel_aux_prio_in_aux;
    	    r2 = M[r1 + $volume_control_cap.vol_ctrl_channel_struct.PRIM_MIX_GAIN_FIELD];

            NULL = r7 - $volume_control_cap.AUX_STATE_END_AUX;
            if NZ jump vol_ctrl_update_channel_aux_prio_transition;
                // Leaving AUX mix,  see if a lower priority aux is pending
                r3 = M[r1 + $volume_control_cap.vol_ctrl_channel_struct.PRIM_MIX_GAIN_FIELD];
                vol_ctrl_update_channel_aux_prio_prev_lp:
                    r9 = r9 LSHIFT -$M.VOL_CTRL.CONSTANT.AUX_PRIORITY_NUM_BITS;
                    NULL = r9 AND $M.VOL_CTRL.CONSTANT.AUX_PRIORITY_VALID_BIT;
                    if Z jump vol_ctrl_update_channel_aux_prio_prev_none;
                        r5 = r9 AND $M.VOL_CTRL.CONSTANT.AUX_PRIORITY_CHANNEL_MASK;
                        r5 = r5 * ($volume_control_cap.vol_ctrl_aux_channel_struct.STRUC_SIZE * ADDR_PER_WORD) (int);
                        r5 = r5 + r0,   r2=M[I0,M1];    // r3 = prim_scale
                        r7 = M[r5 + $volume_control_cap.vol_ctrl_aux_channel_struct.STATE_FIELD];
                        if Z jump vol_ctrl_update_channel_aux_prio_prev_lp;
                        jump vol_ctrl_update_channel_aux_prio_transition;
                vol_ctrl_update_channel_aux_prio_prev_none:
                r2 = $volume_and_limit.OneQ5;

            vol_ctrl_update_channel_aux_prio_transition:
            // transition = op_extra_data->aux_channel[aux_idx].transition;
            r7 = M[r8 + $volume_control_cap.vol_ctrl_aux_channel_struct.TRANSITION_FIELD];
            // chan_ptr->prim_mix_gain = (MAXINT-transition)*Q5.xx + chan_params->prim_scale[chan_ptr->chan_idx]*transition;
            rMAC = r2;              // r2=1.0  Q5.xx
            rmAC = rMAC - r7*r2;
            rMAC = rMAC + r7*r3;
            M[r1 + $volume_control_cap.vol_ctrl_channel_struct.PRIM_MIX_GAIN_FIELD] = rMAC;
            jump vol_ctrl_update_channel_aux_prio_done;
        vol_ctrl_update_channel_aux_prio_in_aux:
            // Transition is complete, in aux
            M[r1 + $volume_control_cap.vol_ctrl_channel_struct.PRIM_MIX_GAIN_FIELD] = r3;

            // Check if aux mix not muted
            NULL = r9 AND $M.VOL_CTRL.CONSTANT.AUX_PRIORITY_MUTE_BIT;
            if NZ jump vol_ctrl_update_channel_aux_prio_done;
            // chan_ptr->aux_buffer       = op_extra_data->aux_channel[aux_idx].buffer;
            r7 = M[r8 + $volume_control_cap.vol_ctrl_aux_channel_struct.BUFFER_FIELD];
            M[r1 + $volume_control_cap.vol_ctrl_channel_struct.AUX_BUFFER_FIELD] = r7;

            // r5=aux_idx, r1=chan_ptr, r6=chan_trim, r4 = chan_idx
            r0 = M[FP+$vol_ctrl.cufp.op_data_ptr];

            // aux_in_use |= (1<<aux_idx);
            r2 = 1 LSHIFT r5;
            r3 = M[r0 + $volume_control_cap.vol_ctrl_data_struct.AUX_IN_USE_FIELD];
            r3 = r3 OR r2;
            M[r0 + $volume_control_cap.vol_ctrl_data_struct.AUX_IN_USE_FIELD]=r3;

            // aux_params = (vol_ctrl_aux_params_t*)&op_extra_data->parameters.OFFSET_AUX1_SCALE;
            // aux_volume = aux_params[aux_idx].aux_scale;
            r0 = M[FP+$vol_ctrl.cufp.op_data_ptr];
            r0 = r0 + ($volume_control_cap.vol_ctrl_data_struct.PARAMETERS_FIELD+$volume_control_cap._tag_VOL_CTRL_PARAMETERS_struct.OFFSET_AUX1_SCALE_FIELD);
            r2 = r5 * ($volume_control_cap.vol_ctrl_aux_params_struct.STRUC_SIZE * ADDR_PER_WORD) (int);
            r0 = M[r0 + r2];
            // aux_volume += op_extra_data->lpvols->auxiliary_gain[aux_idx];
            r3 = M[FP + $vol_ctrl.cufp.vol_ptr];
            r2 = r5 * ADDR_PER_WORD (int);
            r3 = r3 + r2;
            r3 = M[r3 + $volume_control_cap.vol_ctrl_gains_struct.AUXILIARY_GAIN_FIELD];
            r0 = r0 + r3;
            // chan_ptr->aux_mix_gain  = dB60toLinearQ5(aux_volume+channel_trim);
            r0 = r0 + r6;
            call $_dB60toLinearQ5;
            r1 = M[FP + $vol_ctrl.cufp.chan_ptr];
            M[r1 + $volume_control_cap.vol_ctrl_channel_struct.AUX_MIX_GAIN_FIELD]  = r0;

vol_ctrl_update_channel_aux_prio_done:

    r0 = M[FP + $vol_ctrl.cufp.op_data_ptr];

    // r4=chan_idx, r0=op_data_ptr

    // limiter_attn = 0;
    r5 = NULL;

    r1 = M[r0 + ($volume_control_cap.vol_ctrl_data_struct.PARAMETERS_FIELD+$volume_control_cap._tag_VOL_CTRL_PARAMETERS_struct.OFFSET_CONFIG_FIELD)];
    NULL = r1 AND $M.VOL_CTRL.CONFIG.SATURATEENA;
    if Z jump vol_ctrl_update_channel_sat_done;
    r7 = M[FP + $vol_ctrl.cufp.chan_ptr];
    r8 = M[r7 + $volume_control_cap.vol_ctrl_channel_struct.CHANNEL_GAIN_FIELD];
    NULL = r8 - $volume_and_limit.MIN_POSITIVE_VOLUME;
    if LE jump vol_ctrl_update_channel_sat_done;

        // Get Data Buffer
        r6 = r4 * ADDR_PER_WORD (int);
        r6 = r6 + $volume_control_cap.vol_ctrl_data_struct.INPUT_BUFFER_FIELD;
        r0 = M[r0 + r6];
        call $cbuffer.get_read_address_and_size_and_start_address;
        I0 = r0;
        L0 = r1;
        push r2;
        pop B0;

        // Compute Peak
        r3 = M[FP + $vol_ctrl.cufp.tc_ptr];
        r10 = M[r3+$volume_control_cap.vol_time_constants_struct.NUM_WORDS_FIELD];
        r2 = M[r3+$volume_control_cap.vol_time_constants_struct.SAT_TCP5_FIELD];
        r3 = M[r7 + $volume_control_cap.vol_ctrl_channel_struct.LAST_PEAK_FIELD];
        r10 = r10 - 1;
        // Use history for peak but decay it based on num samples
        r3 = r3 * r2 (frac), r2=M[I0,M1];
        do vol_ctrl_update_channel_peak_lp;
            r2 = ABS r2;
            r3 = MAX r2, r2=M[I0,M1];
        vol_ctrl_update_channel_peak_lp:
        r2 = ABS r2;
        r3 = MAX r2;
        M[r7 + $volume_control_cap.vol_ctrl_channel_struct.LAST_PEAK_FIELD]=r3;

        // max_abs = MAX(ABS(channel data)) * chan_ptr->channel_gain;
        rMAC = r3 * r8;

        r0 = M[FP + $vol_ctrl.cufp.op_data_ptr];
        r1 = M[r0 + ($volume_control_cap.vol_ctrl_data_struct.PARAMETERS_FIELD + $volume_control_cap._tag_VOL_CTRL_PARAMETERS_struct.OFFSET_LIMIT_THRESHOLD_LINEAR_FIELD)];
        NULL = rMAC - r1;
        if NEG jump vol_ctrl_update_channel_sat_done;

        r7 = M[r0 + ($volume_control_cap.vol_ctrl_data_struct.PARAMETERS_FIELD + $volume_control_cap._tag_VOL_CTRL_PARAMETERS_struct.OFFSET_LIMIT_ADAPTATION_RATIO_FIELD)];
        r9 = M[r0 + ($volume_control_cap.vol_ctrl_data_struct.PARAMETERS_FIELD + $volume_control_cap._tag_VOL_CTRL_PARAMETERS_struct.OFFSET_LIMIT_THRESHOLD_LOG_FIELD)];

        // limiter_attn = (LIMIT_THRESHOLD - log2(max_abs)) * LIMIT_RATIO;
        call $math.log2_table;
        /* Input to log was Q5.xx,  need to adjust to Q1.xx */
        r0 = r0 + (4<<(DAWTH-8));
        r9 = r9 - r0;
        r5 = r9 * r7 (frac);
        if POS r5=NULL;
vol_ctrl_update_channel_sat_done:

    // Time constants are scaled based on num samples
    r1 = M[FP+$vol_ctrl.cufp.chan_ptr];
    r2 = M[r1 + $volume_control_cap.vol_ctrl_channel_struct.LIMIT_GAIN_LOG2_FIELD];
    r3 = M[FP + $vol_ctrl.cufp.tc_ptr];
    r4 = M[r3 + $volume_control_cap.vol_time_constants_struct.SAT_TC_FIELD];
    r3 = M[r3 + $volume_control_cap.vol_time_constants_struct.SAT_TCP5_FIELD];
    NULL = r5 - r2;
    if POS r3 = r4;

    // chan_ptr->limit_gain_log2 = (1.0 - tc)*limiter_attn + tc*chan_ptr->limit_gain_log2;
    // chan_ptr->limit_gain_log2 = MIN(chan_ptr->limit_gain_log2,0);
    rMAC = r5;
    rMAC = rMAC - r3*r5;
    rMAC = rMAC + r3*r2;
    if POS rMAC = NULL;

    r3 = r5 - rMAC;
    r3 = ABS r3;
    NULL = r3 - 0.00001;
    if LE rMAC = r5;

    M[r1 + $volume_control_cap.vol_ctrl_channel_struct.LIMIT_GAIN_LOG2_FIELD]=rMAC;

    // r1 = chan_ptr

    L0 = NULL;
    push NULL;
    pop B0;

    /* channel count */
    pop r7;

    /* advance channel */
    r1 = r1 + ($volume_control_cap.vol_ctrl_channel_struct.STRUC_SIZE*ADDR_PER_WORD);
    M[FP + $vol_ctrl.cufp.chan_ptr] = r1;

    /* restore data */
    r0 = M[FP + $vol_ctrl.cufp.op_data_ptr];
    r2 = M[FP + $vol_ctrl.cufp.vol_ptr];

    /* Another channel ? */
    r7 = r7 - 1;
    if GT jump vol_ctrl_update_channel_next;


// *****************************************************************************
// MODULE:
//    $_vol_ctrl_update_saturation
//    void vol_ctrl_update_saturation(vol_ctrl_data_t *op_extra_data)
//
// INPUTS:
//      r0 = Pointer to capability data object
// OUTPUTS:
//      none
// DESCRIPTION:
//    Complete saturation by conditionally syncing channels
//    Function is C compatible
//
// *****************************************************************************
$_vol_ctrl_update_saturation:
    /* Saturation protection may be per channel or across all channels */
    r1 = M[r0 + ($volume_control_cap.vol_ctrl_data_struct.PARAMETERS_FIELD + $volume_control_cap._tag_VOL_CTRL_PARAMETERS_struct.OFFSET_CONFIG_FIELD)];
    NULL = r1 AND $M.VOL_CTRL.CONFIG.SATURATEENA;
    if Z jump vol_ctrl_update_saturation_done;

        r2 = M[r0 + $volume_control_cap.vol_ctrl_data_struct.CHANNELS_FIELD];
        r10 = M[r0 + $volume_control_cap.vol_ctrl_data_struct.NUM_CHANNELS_FIELD];
        M3 = ($volume_control_cap.vol_ctrl_channel_struct.STRUC_SIZE * ADDR_PER_WORD);
        I0 = r2 + $volume_control_cap.vol_ctrl_channel_struct.LIMIT_GAIN_LOG2_FIELD;
        I1 = r2 + $volume_control_cap.vol_ctrl_channel_struct.LIMITER_GAIN_LINEAR_FIELD;

        NULL = r1 AND $M.VOL_CTRL.CONFIG.SATURATESYNCENA;
        if Z jump vol_ctrl_update_saturation_not_sync;
            // Min log gain of channels
            push r10,    r1 = M[I0,M3];
            r10 = r10 - 1;
            r0  = r1;
            do vol_ctrl_update_saturation_min_lp;
                r0 = MIN r1, r1 = M[I0,M3];
            vol_ctrl_update_saturation_min_lp:
            r0 = MIN r1;
            // Convert to linear
            call $math.pow2_table;
            // Apply to all channels
            pop r10;
            // Nops ensures pipeline for setting r10
            nop;
            do vol_ctrl_update_saturation_apply_lp;
                M[I1,M3]=r0;
            vol_ctrl_update_saturation_apply_lp:
            jump vol_ctrl_update_saturation_done;

        vol_ctrl_update_saturation_not_sync:
            // Convert to linear
            do vol_ctrl_update_saturation_conv_lp;
                r0 = M[I0,M3];
                call $math.pow2_table;
                M[I1,M3]=r0;
            vol_ctrl_update_saturation_conv_lp:

vol_ctrl_update_saturation_done:

// *****************************************************************************
// MODULE:
//    $_vol_ctrl_apply_volume
//    void vol_ctrl_apply_volume(vol_ctrl_data_t *op_extra_data, vol_ctrl_channel_t *chan_ptr, vol_time_constants_t *lpvcs);
//
// INPUTS:
//      r0 = Pointer to capability data object
//      r1 = Pointer to channel state info
//      r2 = Pointer to period's time constants
// OUTPUTS:
//      none
// DESCRIPTION:
//    Apply volume, aux mixing, and boost/hard clip
//    Function is C compatible
//
// *****************************************************************************

   /* Apply volume to channels.  Also, Aux Mix and hard clip */
   /*
      for(i=0;i<op_extra_data->num_channels;i++)
      {
           vol_ctrl_apply_volume(op_extra_data,&op_extra_data->channels[i],&volume_tc);
       }
   */
$_vol_ctrl_apply_volume:

    r0 = M[FP + $vol_ctrl.cufp.op_data_ptr];

    /* Number of channels */
    r7 = M[r0 + $volume_control_cap.vol_ctrl_data_struct.NUM_CHANNELS_FIELD];

    // Reset chan_ptr
    r1 = M[r0 + $volume_control_cap.vol_ctrl_data_struct.CHANNELS_FIELD];
    M[FP + $vol_ctrl.cuup.chan_ptr]=r1;

    // Move time constants */
    r2 = M[FP + $vol_ctrl.cufp.tc_ptr];
    M[FP + $vol_ctrl.cuup.tc_ptr] = r2;

    // r0: vol_ctrl_data_t *op_extra_data
    // r1: vol_ctrl_channel_t *chan_ptr
    // r2: vol_time_constants_t *lpvcs

vol_ctrl_apply_volume_next:
    push r7;
#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($vol_ctrl_apply_volume.PATCH_ID_0, r7)
#endif

    // r0: vol_ctrl_data_t *op_extra_data
    // r1: vol_ctrl_channel_t *chan_ptr
    // r2: vol_time_constants_t *lpvcs

    // r3: routing
    r7 = r0 + ($volume_control_cap.vol_ctrl_data_struct.PARAMETERS_FIELD + $volume_control_cap._tag_VOL_CTRL_PARAMETERS_struct.OFFSET_CHAN1_AUX_ROUTE_FIELD);
    r6 = M[r1 + $volume_control_cap.vol_ctrl_channel_struct.CHAN_IDX_FIELD];
    r6 = r6  * ($volume_control_cap.vol_ctrl_chan_params_struct.STRUC_SIZE * ADDR_PER_WORD) (int);
    r3 = M[r7+r6];

    // Boost
    r8 = $volume_and_limit.OneQ5;
    r5  = M[r0 + ($volume_control_cap.vol_ctrl_data_struct.PARAMETERS_FIELD + $volume_control_cap._tag_VOL_CTRL_PARAMETERS_struct.OFFSET_BOOST_FIELD)];
    NULL = r3 AND $M.VOL_CTRL.CONSTANT.CHAN_BOOST_CLIP_ENABLE_BIT;
    if Z r5 = r8;

    // Gains
    r9  = M[r1 + $volume_control_cap.vol_ctrl_channel_struct.CHANNEL_GAIN_FIELD];        // Q5.xx
    r10 = M[r2 + $volume_control_cap.vol_time_constants_struct.NUM_WORDS_FIELD];
    r7  = M[r1 + $volume_control_cap.vol_ctrl_channel_struct.PRIM_MIX_GAIN_FIELD];       // Q5.xx
    r6  = M[r1 + $volume_control_cap.vol_ctrl_channel_struct.LIMITER_GAIN_LINEAR_FIELD]; // Q.xx

    // calculate init and final volume for this frame
    rMAC = r9 * r6;
    rMAC = rMAC * r7;
    rMAC = rMAC ASHIFT 4 (56bit);
    rMAC = rMAC * r5;
    rMAC = rMAC ASHIFT 4 (56bit);  // Boost

    // Complute the volume step size
    r4  = M[r1 + $volume_control_cap.vol_ctrl_channel_struct.LAST_VOLUME_FIELD];         // Q5.xx
    r2 = rMAC - r4;
    rMAC = r2 ASHIFT 0 (LO);
    Div = rMAC / r10;

    // Aux Mix
    r8  = M[r1 + $volume_control_cap.vol_ctrl_channel_struct.AUX_MIX_GAIN_FIELD];        // Q5.xx
    rMAC = r8 * r5;
    r8 = rMAC ASHIFT 4;  // Boost

    /* Adjust Clip Point
       (might as well do it even if we don't use it, divide takes time anyway) */
    r5 = M[r0 + ($volume_control_cap.vol_ctrl_data_struct.PARAMETERS_FIELD + $volume_control_cap._tag_VOL_CTRL_PARAMETERS_struct.OFFSET_CLIP_POINT_FIELD)];
    r2 = M[r0 + ($volume_control_cap.vol_ctrl_data_struct.PARAMETERS_FIELD + $volume_control_cap._tag_VOL_CTRL_PARAMETERS_struct.OFFSET_BOOST_CLIP_POINT_FIELD)];
    // Multiply the Clip Point (r5) with the Inverse Post Gain (r7)
    r7 = M[r0 + $volume_control_cap.vol_ctrl_data_struct.SHARED_VOLUME_PTR_FIELD];
    r7 = M[r7 + $volume_control_cap._shared_volume_struct.INV_POST_GAIN_FIELD];
    // The inverse DAC gain is Q5.xx, so we need to shift r7 to get Q1.xx
    rMAC = r5 * r7;
    r7 = rMAC ASHIFT 4;
    // limit the clipper threshold so DAC wrap bug doesn't occur
    Null = r7 - r2;
    if POS r7 = r2;

    // Step for main volume
    r5 = DivResult;

    // Setup Buffers
    pushm <r3,r10>;

    r1 = M[r1 + $volume_control_cap.vol_ctrl_channel_struct.CHAN_IDX_FIELD];
    r1 = r1 * ADDR_PER_WORD (int);
    r0 = r0 + r1;
    push r0;
    r0 = M[r0];
    call $cbuffer.get_read_address_and_size_and_start_address;
    I0 = r0;
    push r2;
    pop  B0;
    L0 = r1;

    pop r0;
    r0 = M[r0 + $volume_control_cap.vol_ctrl_data_struct.OUTPUT_BUFFER_FIELD];
    call $cbuffer.get_write_address_and_size_and_start_address;
    I5 = r0;
    push r2;
    pop B5;
    L5 = r1;

    r1 = M[FP + $vol_ctrl.cuup.chan_ptr];
    r0 = M[r1 + $volume_control_cap.vol_ctrl_channel_struct.AUX_BUFFER_FIELD];
    if NZ jump vol_ctrl_apply_volume_get_aux;
        r0 = FP + $vol_ctrl.cuup.dummy;
        r1 = ADDR_PER_WORD;
        r2 = r0;
        r8 = NULL;
        jump vol_ctrl_apply_volume_get_aux_ok;
    vol_ctrl_apply_volume_get_aux:
        call $cbuffer.get_read_address_and_size_and_start_address;
  vol_ctrl_apply_volume_get_aux_ok:
    // Set Aux pointer
    I4 = r0;
    push r2;
    pop B4;
    L4 = r1;

    popm <r3,r10>;
    // r7=clip, r4=main gain, r8=aux gain, r3=routing, r10 = amount

    /* Check Mute */
    r0 = M[FP + $vol_ctrl.cuup.op_data_ptr];
    r6 = M[r0 + $volume_control_cap.vol_ctrl_data_struct.CUR_MUTE_GAIN_FIELD];
    r9 = M[r0 + $volume_control_cap.vol_ctrl_data_struct.MUTE_INCREMENT_FIELD];
    if NZ jump vol_ctrl_apply_volume_mute;

    NULL = r3 AND $M.VOL_CTRL.CONSTANT.CHAN_BOOST_CLIP_ENABLE_BIT;
    if NZ jump vol_ctrl_apply_volume_clip;
        do vol_ctrl_apply_volume_lp;
            r4 = r4 + r5,   r2 = M[I0,MK1];
            rMAC = r2 * r4, r2 = M[I4,MK1];
            rMAC = rMAC + r2 * r8;
            rMAC = rMAC ASHIFT 4 (56bit);
            M[I5,MK1] = rMAC;
        vol_ctrl_apply_volume_lp:
        jump vol_ctrl_apply_volume_done;

    vol_ctrl_apply_volume_clip:
        do vol_ctrl_apply_volume_clip_lp;
            r4 = r4 + r5,   r2 = M[I0,MK1];
            rMAC = r2 * r4, r2 = M[I4,MK1];
            rMAC = rMAC + r2 * r8;
            rMAC = rMAC ASHIFT 4 (56bit);
            // Hard Clip
            r1 = rMAC ASHIFT (8 + 2 * DAWTH);
            r2 = ABS rMAC;
            r2 = MIN r7;
            r2 = r2 * r1 (frac);
            M[I5,MK1] = r2;
        vol_ctrl_apply_volume_clip_lp:
        jump vol_ctrl_apply_volume_done;

vol_ctrl_apply_volume_mute:
        NULL = r3 AND $M.VOL_CTRL.CONSTANT.CHAN_BOOST_CLIP_ENABLE_BIT;
        if NZ jump vol_ctrl_apply_volume_clip_mute;

        do vol_ctrl_apply_volume_mute_lp;
            r4 = r4 + r5,   r2 = M[I0,MK1];
            rMAC = r2 * r4, r2 = M[I4,MK1];
            rMAC = rMAC + r2 * r8;
            rMAC = rMAC ASHIFT 4 (56bit);
            // Mute
            r6 = r6 + r9;
            if NEG r6=NULL;
            rMAC = rMAC * r6 (frac);
            // Save result
            M[I5,MK1] = rMAC;
        vol_ctrl_apply_volume_mute_lp:
        jump vol_ctrl_apply_volume_done;

vol_ctrl_apply_volume_clip_mute:
        do vol_ctrl_apply_volume_clip_mute_lp;
            r4 = r4 + r5,   r2 = M[I0,MK1];
            rMAC = r2 * r4, r2 = M[I4,MK1];
            rMAC = rMAC + r2 * r8;
            rMAC = rMAC ASHIFT 4 (56bit);
            // Hard Clip
            r1 = rMAC ASHIFT (8 + 2 * DAWTH);
            r2 = ABS rMAC;
            r2 = MIN r7;
            r2 = r2 * r1 (frac);
            // Mute
            r6 = r6 + r9;
            if NEG r6=NULL;
            r2 = r2 * r6 (frac);
            // Save result
            M[I5,MK1] = r2;
        vol_ctrl_apply_volume_clip_mute_lp:

vol_ctrl_apply_volume_done:

    // Save last gain
    r1 = M[FP +$vol_ctrl.cuup.chan_ptr];
    M[r1 + $volume_control_cap.vol_ctrl_channel_struct.LAST_VOLUME_FIELD] = r4;

    // Update Buffer
    r1 = M[r1 + $volume_control_cap.vol_ctrl_channel_struct.CHAN_IDX_FIELD];
    r1 = r1 * ADDR_PER_WORD(int);
    r0 = M[FP + $vol_ctrl.cuup.op_data_ptr];
    r0 = r0 + r1;
    push r0;
    r0 = M[r0];
    r1 = I0;
    call $cbuffer.set_read_address;
    pop r0;
    r0 = M[r0 + $volume_control_cap.vol_ctrl_data_struct.OUTPUT_BUFFER_FIELD];
    r1 = I5;
    call $cbuffer.set_write_address;

    /* Next Channel */
    pop r7;

    /* Restore parameters */
    r0 = M[FP + $vol_ctrl.cuup.op_data_ptr];
    r1 = M[FP + $vol_ctrl.cuup.chan_ptr];
    r2 = M[FP + $vol_ctrl.cuup.tc_ptr];

    /* advance chan_ptr */
    r1 = r1 + ($volume_control_cap.vol_ctrl_channel_struct.STRUC_SIZE*ADDR_PER_WORD);
    M[FP + $vol_ctrl.cuup.chan_ptr]=r1;

    /* Another channel */
    r7 = r7 - 1;
    if GT jump vol_ctrl_apply_volume_next;

    // Save current mute
    M[r0 + $volume_control_cap.vol_ctrl_data_struct.CUR_MUTE_GAIN_FIELD]=r6;
    NULL = r6 - 1.0;
    if Z r9=NULL;
    M[r0 + $volume_control_cap.vol_ctrl_data_struct.MUTE_INCREMENT_FIELD]=r9;

    // Restore arithmetic mode */
    pop r4;
    M[$ARITHMETIC_MODE]=r4;

vol_ctrl_apply_volume_abort:

    popm <FP,r0,r1,r2,r3,rlink>;
    popm <B0,B4,B5>;
    popm <I0,I1,I4,I5,M3,L0,L4,L5>;
    popm <r4,r5,r6,r7,r8,r9>;

    rts;
.ENDMODULE;



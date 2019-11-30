// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    rate_adjustment_and_shift operator
//
// DESCRIPTION:
//    This operator replaces the warp_and_shift operator, it resamples the
//    incoming data to a proper rate to compensate the rate mismatch between source
//    and sink clocks.
//
//
// *****************************************************************************

#include "cbops.h"
#include "profiler_c_asm_defs.h"

// Operator function table
.MODULE $M.cbops.rate_adjustment_and_shift;
   .DATASEGMENT DM;


   // ** function vector **
   .VAR $cbops.rate_adjustment_and_shift[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.function_vector.NO_FUNCTION,               // reset vector
      &$cbops.rate_adjustment_and_shift.amount_to_use,   // amount to use function
      &$cbops.rate_adjustment_and_shift.main;            // main function

.ENDMODULE;

// Expose the function table to C
.set $_cbops_rate_adjust_table,     $cbops.rate_adjustment_and_shift

// *******************************************************************************
// MODULE:
//    $cbops.rate_adjustment_and_shift.amount_to_use
//
// DESCRIPTION:
//    Amount to use function for the rate_adjustment_and_shift operator.  The number of
//    input samples to process is calculated based on the amount of space in
//    the output buffer and the current warp factor that is being applied.
//
// INPUTS:
//    - r4 = buffer table
//    - r8 = pointer to operator structure (see below main function for details)
//
// OUTPUTS:
//    - r5 = number of input samples to use
//
// TRASHED REGISTERS:
//    r0, r4, r5
//
// *******************************************************************************
.MODULE $M.cbops.rate_adjustment_and_shift.amount_to_use;
   .CODESEGMENT CBOPS_RATE_ADJUSTMENT_AND_SHIFT_AMOUNT_TO_USE_PM;
   .DATASEGMENT DM;

   // ** amount to use function **
$cbops.rate_adjustment_and_shift.amount_to_use:
   push rlink;
#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.rate_adjustment_and_shift.amount_to_use.PATCH_ID_0, r9)
#endif
   // Compute num channels in address
   r9 = M[r8 + $cbops.param_hdr.NR_INPUT_CHANNELS_FIELD];
   Words2Addr(r9);

   // Get space at output 
   r0 = r9 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
   r0 = M[r8 + r0];     // Output index (first channel)
   call $cbops.get_amount_ptr;
   r7 = M[r0];

   // Get data at input
   r0 = M[r8 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD];     // input index (first channel)
   call $cbops.get_amount_ptr;
   // pointer to input amount (r0)
   r5 = M[r0];

   // compute maxium input to consume (r4)
   r1 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];
   r1 = M[r1 + $cbops.rate_adjustment_and_shift.SRA_CURRENT_RATE_FIELD];
   r4 = r1 * r7 (frac);
   r4 = r7 - r4;

   // because of rounding errors, to be safe we knock off 2
   r4 = r4 - 2;
   if NEG r4 = 0;


   // if the amount of data avaiable > calculate amount -> set amount available to calculate amount
   r5 = MIN r4;
  
   // update amount to consume
   M[r0] = r5;
   pop rlink;
   rts;

.ENDMODULE;
// *******************************************************************************
// MODULE:
//    $cbops.rate_adjustment_and_shift.main
//
// DESCRIPTION:
//    Main processing function for the rate_adjustment_and_shift operator.
//
// INPUTS:
//    - r4 = pointer to buffer table
//    - r8 = pointer to operator structure:
//
// Operator param structure:
//
//    - header:
//          nr inputs
//          nr outputs (equal in this case)
//          <nr inputs> indexes for input channels (some may be marked as unused)
//          <nr outputs> indexes for output channels (some may be marked as unused)
//
//    - Block of channel-independent parameters:
//          $cbops.rate_adjustment_and_shift.SHIFT_AMOUNT_FIELD
//             -amount of shift required after resampling
//          $cbops.rate_adjustment_and_shift.FILTER_COEFFS_FIELD
//             - coefficient used to resample
//          $cbops.rate_adjustment_and_shift.SRA_TARGET_RATE_ADDR_FIELD
//             - address which stores the target rate value,
//                output_rate = (1 + target_rate) * input_rate, target rate is expected to be
//                between -0.005 and 0.005
//          $cbops.rate_adjustment_and_shift.SRA_CURRENT_RATE_FIELD
//             - internal state, init to zero
//          $cbops.rate_adjustment_and_shift.RF
//             - internal state, init to zero
//          $cbops.rate_adjustment_and_shift.PREV_SHORT_SAMPLES_FIELD
//             - internal state, init to zero
//
//    - Blocks of channel-specific params, each containing:
//          $cbops.rate_adjustment_and_shift.CHAN_HIST_BUF_FIELD
//          $cbops.rate_adjustment_and_shift.CHAN_HIST_BUF_START_FIELD  (if base reg. mode)
//             - history buffer for resampling input
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    assume everything
//
// *******************************************************************************
.MODULE $M.cbops.rate_adjustment_and_shift.main;
   .CODESEGMENT CBOPS_RATE_ADJUSTMENT_AND_SHIFT_MAIN_PM;
   .DATASEGMENT DM;

   // ** main routine **
   $cbops.rate_adjustment_and_shift.main:

   // offsets for the local variables (after [FP+1] that contains rLink):
   .CONST $cbops.sra_temp_var.FP_OFFSET               2*ADDR_PER_WORD;  // FP, rLink
   .CONST $cbops.sra_temp_var.BUFFER_TABLE            $cbops.sra_temp_var.FP_OFFSET;
   .CONST $cbops.sra_temp_var.AMOUNT_DATA             $cbops.sra_temp_var.BUFFER_TABLE + ADDR_PER_WORD;
   .CONST $cbops.sra_temp_var.RF_TEMP                 $cbops.sra_temp_var.AMOUNT_DATA + ADDR_PER_WORD;

  .CONST $cbops.sra_temp_var.NUM_TEMP_VARS            3*ADDR_PER_WORD;

#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.rate_adjustment_and_shift.main.PATCH_ID_0, r9)
#endif

   // make stack frame for above local vars in conjunction with saving FP & rLink
   pushm <FP(=SP), rLink>, SP = SP + $cbops.sra_temp_var.NUM_TEMP_VARS;


#ifdef ENABLE_PROFILER_MACROS
  .VAR/DM1 $cbops.profile_rate_adjustment_and_shift[$profiler_c.profiler_struct.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
   r0 = &$cbops.profile_rate_adjustment_and_shift;
   call $profiler.start;
#endif
   
   call $cbops.get_transfer_and_update_multi_channel;
   // r9 = num channels in addresses
   r10 = r0;
   if LE jump jp_done;

   // save buffer table pointer and amount of data
   M[FP + $cbops.sra_temp_var.BUFFER_TABLE] = r4;
   M[FP + $cbops.sra_temp_var.AMOUNT_DATA]  = r10;
   // r4 is pointer to independent data in the cbop param struct
   r4 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];

   // get filter coeffs size
   r0 = $cbops.rate_adjustment_and_shift.SRA_COEFFS_SIZE;
   r1 = M[r4 + $cbops.rate_adjustment_and_shift.FILTER_COEFFS_SIZE_FIELD];
   if Z r1 = r0;
   Words2Addr(r1);     // coeffs_size (arch4: in addrs)
   L0 = r1;
   L1 = r1;

   // If the cbop is in passthrough mode take the simple code path
   r0 = M[r4 + $cbops.rate_adjustment_and_shift.PASSTHROUGH_MODE_FIELD];
   if NZ jump passthrough_mode;

   // get the coeffs addr
   r0 = M[r4 + $cbops.rate_adjustment_and_shift.FILTER_COEFFS_FIELD];

   // I3 is now pointer to the end of coefficient table
   r1 = r1 * ($cbops.rate_adjustment_and_shift.SRA_UPRATE)(int);
   r1 = r1 LSHIFT -1;  // sra_uprate*coeffs_size/2 (arch4: in addrs)
   I3 = r0 + r1;

   // slowly move towards the target rate
   r5 = M[r4 + $cbops.rate_adjustment_and_shift.SRA_CURRENT_RATE_FIELD];
   // calculate current rate, slowly move towards the target rate
   r0 = M[r4 + $cbops.rate_adjustment_and_shift.SRA_TARGET_RATE_ADDR_FIELD];
   // Check if the current rate calculation is bypassed
   if Z jump current_rate_calculation_done;
   r3 = M[r0];

   // logarithmic then linear movement
   r1 = $sra.MOVING_STEP;
   r0 = r5 - r3;
   if NEG r0 = -r0;
   r2 = r0 * 0.001(frac);
   Null = r0 - 0.0015;
   if NEG r2 = r1;
   r1 = r2 - (30*$sra.MOVING_STEP);
   if POS r2 = r2 - r1;
   r1 = r5 - r3;
   r0 = r1 - r2;
   if POS r1 = r1 - r0;
   r0 = r1 + r2;
   if NEG r1 = r1 - r0;
   // update the current rate
   r5 = r5 - r1;
   // store the updated current rate for all channels
   M[r4 + $cbops.rate_adjustment_and_shift.SRA_CURRENT_RATE_FIELD] = r5;
   current_rate_calculation_done:

   // r7 is pointer to channel specific data in the cbop param struct
   r7 = r4 + ($cbops.rate_adjustment_and_shift.COMMON_PARAM_STRUC_SIZE * ADDR_PER_WORD);
   
   // r3=channel counter
   r3 = Null;
   
#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.rate_adjustment_and_shift.main.PATCH_ID_1, r1)
#endif

   // -------------------- start of channel-by-channel processing -------------------------
 process_channel:
   r4 = M[FP + $cbops.sra_temp_var.BUFFER_TABLE];

   // get index of currently to be used input channel
   r5 = r3 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD; 

   // Setup Input Buffer
   r0 = M[r8 + r5];     // input index
   call $cbops.get_buffer_address_and_length;
   I4 = r0;
   if Z jump next_channel;
   L4 = r1;
   push r2;
   pop B4;
   
   // Setup Output Buffer
   r0 = r5 + r9;
   r0 = M[r8 + r0];     // output index
   call $cbops.get_buffer_address_and_length;
   I5 = r0;
   if Z jump next_channel;
   L5 = r1;
   push r2;
   pop B5;

   // get the history buffer pointer for this channel (and its base address, if not riding a dinosaur)
   r0 = M[r7 + $cbops.rate_adjustment_and_shift.CHAN_HIST_BUF_FIELD];
   I0 = r0;
   r0 = M[r7 + $cbops.rate_adjustment_and_shift.CHAN_HIST_BUF_START_FIELD];
   push r0;
   B0 = M[SP - 1*ADDR_PER_WORD];
   pop B1;

   // Save variables trashhed by function call
   pushm <r3,r7,r8,r9>;
   // r8 = Pointer to channel indpentent data and M3=1
   // L0=L1=history buffer length set for all channels
   // I3 is pointer to coeffifcients set for all channels
   // I5,L5,B5 = channel output buffer
   // I4,L4,B4 = channel input buffer
   r8 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];	// Channel independent data pointer
    M3 = MK1;
   //  r10: number of input samples
   r10 = M[FP + $cbops.sra_temp_var.AMOUNT_DATA];
   call $sra_resample_core;
   M[FP + $cbops.sra_temp_var.RF_TEMP] = r4;

   // Restore registsers
   popm <r3,r7,r8,r9>;

   //  r0: total number of sample required for last iteration of the loop
   //  r10:  (available number of sample for last iteration of the loop - r0)
   //  r4 : new value of sra_c.sra_params_struct.RF_FIELD
   //  I7: total number of output samples generated
   //  I0: updated hist buffer address

   // advance hister buffer pointer by available number of sample for last iteration of the loop
   Words2Addr(r10);

#ifdef CBOPS_DEBUG
        // this is potentially dangerous, if we return from sra_resample with a large-ish r10
        // (r10 more than 3x size of cbuffer), under normal circumstances r10=0 or -1
        // we might end up overwriting a location outside of history cbuffer
        Null = r10 - L0;
        if POS call $error;     // unexpected exit from sra_resample doloop
#endif
   M1 = r10;
   r0 = M[I0, M1];
   r1 = I0;
   M[r7 + $cbops.rate_adjustment_and_shift.CHAN_HIST_BUF_FIELD] = r1;

 next_channel:
   // Advance channel specific data pointer (r7)
   r7 = r7 + ($cbops.rate_adjustment_and_shift.CHAN_STRUC_SIZE * ADDR_PER_WORD);
   // update channel counter
   r3 = r3 + 1*ADDR_PER_WORD;
   // do we resample any more channels? (both numbers compared here are in ADDR_PER_WORDs)
   Null = r3 - r9;
   if LT jump process_channel;

   // ------------------- end of channel processing loop -----------------------

   // r4 was output from sra_resample, update RF with it
   // ### it used to do this after both channels were resampled - did that really work if/when they used
   // also right channel? There are some very fishy things in original code, including history buffer stuff.
   r2 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];
   r4 = M[FP + $cbops.sra_temp_var.RF_TEMP];
   M[r2 + $cbops.rate_adjustment_and_shift.RF] = r4;

   r0 = -M1;
   M[r2 + $cbops.rate_adjustment_and_shift.PREV_SHORT_SAMPLES_FIELD] = r0;

   L5 = 0;
   L4 = 0;
   L0 = 0;
   L1 = 0;
   push NULL;
   B0 = M[SP - 1*ADDR_PER_WORD];
   B1 = M[SP - 1*ADDR_PER_WORD];
   B4 = M[SP - 1*ADDR_PER_WORD];
   pop B5;

   // Set number of output samples produced - get back the framework object pointer.
   // Strictly speaking, this is updating based on last channel's run of sra_resample;
   // now having been driven by same parameters in terms of SRA itself, one sincerely hopes
   // it does the very same thing so this should be fine.
   r7 = I7;
   r4 = M[FP + $cbops.sra_temp_var.BUFFER_TABLE];

   r0 = r9 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD;
   r0 = M[r8 + r0];     // Output index (first channel)
   call $cbops.get_amount_ptr;
   M[r0]=r7;

jp_done:

   #ifdef ENABLE_PROFILER_MACROS
       // stop profiling if enabled
       r0 = &$cbops.profile_rate_adjustment_and_shift;
       call $profiler.stop;
   #endif

   // kill off temp var area and close the ceremony
   SP = SP - ($cbops.sra_temp_var.NUM_TEMP_VARS), popm <FP, rLink>;
   rts;


// *****************************************************************************
// NAME:
//    passthrough_mode:
//
// DESCRIPTION:
//
//  private funtion to pass data whilst maintaining the history buffer contents
//  of each channel. This function attempts to improve throughput by only copying
//  samples into/out of the history buffer that persist between runs.
//
// INPUTS:
//  r1, L0: History buffer size
//  r4 = channel independent data pointer
//  r8: pointer to operator structure's start of the channel-independent params
//  r9: Number of cbop channels
//  r10: number of input samples
//
// OUTPUTS:
// None:
//
// TRASHED REGISTERS:
//    r0-r7,r9, rMAC, rMACB I0,I1, I4, I5, M0
// *****************************************************************************
passthrough_mode:

#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.rate_adjustment_and_shift.passthrough_mode.PATCH_ID_0, r3)
#endif

   // r1 is in address units here, need it in samples
   Addr2Words(r1);
   // number of samples to push through history buffer in r1
   rMAC = r10 - r1; // stash samples to process in rMAC for each channel
   if NEG r1 = r10; // if r10 > history buffer length then limit history samples

   // r7 is pointer to channel specific data in the cbop param struct
   r7 = r4 + ($cbops.rate_adjustment_and_shift.COMMON_PARAM_STRUC_SIZE * ADDR_PER_WORD);
   
   // Channel counter (r3)
   r3 = Null;
   r6 = r10;
   M0 = MK1;

   // -------------------- start of channel-by-channel processing -------------------------
 passthrough_channel:
   
   // get index of currently to be used input channel
   r4 = M[FP + $cbops.sra_temp_var.BUFFER_TABLE];
   
   // Setup Input Buffer
   r5 = r3 + $cbops.param_hdr.CHANNEL_INDEX_START_FIELD; 
   r0 = M[r8 + r5];     // input index



   call $cbops.get_buffer_address_and_length;
   I4 = r0;
   if Z jump next_passthrough_channel;
   L4 = r1;
   push r2;
   pop B4;

   // Setup Output Buffer
   r0 = r5 + r9;
   r0 = M[r8 + r0];     // output index
   call $cbops.get_buffer_address_and_length;
   I1 = r0;
   if Z jump next_passthrough_channel;


   L1 = r1;
   push r2;
   pop B1;


   // r4 is pointer to independent data in the cbop param struct
   r4 = M[r8 + $cbops.param_hdr.OPERATOR_DATA_PTR_FIELD];

   // get the history buffer pointer for this channel (and its base address, if not riding a dinosaur)
   r0 = M[r7 + $cbops.rate_adjustment_and_shift.CHAN_HIST_BUF_FIELD];

   I0 = r0;
   I5 = r0;
   r0 = M[r7 + $cbops.rate_adjustment_and_shift.CHAN_HIST_BUF_START_FIELD];
   push r0;
   B0 = M[SP - 1*ADDR_PER_WORD];
   pop B5;
   // History buffer length was initialised in L0 before we jumped here.
   L5 = L0;


   r10 = r6;
   // r10 is in samples here, need to convert to addresses to compare with L0
   // and then change it back again afterwards
   Words2Addr(r10);
   Null = r10 - L0;
   if POS jump more_samples_than_history;
   Addr2Words(r10);

   rMACB = M[r4 + $cbops.rate_adjustment_and_shift.SHIFT_AMOUNT_FIELD];
   if POS jump pre_shift;
   // copy the samples via the history buffer shifting them on the way out
   r0 = M[I5, M0];
   do delay_line_post_shift_loop;
      r0 = r0 ASHIFT rMACB;
      M[I1, M0] = r0, r4 = M[I4, M0];
      M[I0, M0] = r4, r0 = M[I5, M0];
   delay_line_post_shift_loop:

	       // Update history Buffer
	   	   r0 = I0;
	       M[r7 + $cbops.rate_adjustment_and_shift.CHAN_HIST_BUF_FIELD] = r0;
	       jump next_passthrough_channel;

   pre_shift:
   // copy the samples via the history buffer shifting them on the way in
   r0 = M[I5, M0];
   do delay_line_pre_shift_loop;
      M[I1, M0] = r0, r4 = M[I4, M0];
      r4 = r4 ASHIFT rMACB;
      M[I0, M0] = r4, r0 = M[I5, M0];
   delay_line_pre_shift_loop:

           // Update history Buffer
	   	   r0 = I0;

	       M[r7 + $cbops.rate_adjustment_and_shift.CHAN_HIST_BUF_FIELD] = r0;
   jump next_passthrough_channel;

more_samples_than_history:
   // In this case lets not copy every sample through the history buffer it
   // just adds an extra copy for each sample. So instead do:
   // 1. Read history buffer contents into output buffer
   // 2. Copy samples to process - hist buf length sameple from input buffer directly to output
   // 3. Copy last samples from input buffer to fill history buffer.
   // read the shift amount.   If positive use as a pre-shift (input)
   // otherwise it is a post shift.
   
   r1 = L0;
   Addr2Words(r1); // Samples in history
   r10 = r1;
   
   rMACB = M[r4 + $cbops.rate_adjustment_and_shift.SHIFT_AMOUNT_FIELD];
   if Z jump bypass_noshift;
   if POS rMACB=NULL;

   // read history buffer to output

   r0 = M[I5, M0];
   do empty_hist_loop;
      r0 = r0 ASHIFT rMACB;
      M[I1, M0] = r0, r0 = M[I5, M0];
   empty_hist_loop:



   // copy the samples skipping the history buffer. So here it doesn't matter if
   // it's a pre/post shift just apply it.   
   r10 = r6 - r1; // Run loop for NbSamples - History Samples
   rMACB = M[r4 + $cbops.rate_adjustment_and_shift.SHIFT_AMOUNT_FIELD];
   r0 = M[I4, M0];
   do straight_copy_loop;
      r0 = r0 ASHIFT rMACB;
      M[I1, M0] = r0, r0 = M[I4, M0];
   straight_copy_loop:
   
    // fill the history buffer
   r10 = r1;

   // setup any pre/shift   
   rMACB = M[r4 + $cbops.rate_adjustment_and_shift.SHIFT_AMOUNT_FIELD];
   if NEG rMACB=NULL;

   do fill_hist_loop;
      r0 = r0 ASHIFT rMACB;
      M[I0, M0] = r0, r0 = M[I4, M0];
   fill_hist_loop:

// I0 hasn't moved so we don't need to update the history buffer pointer   
   jump next_passthrough_channel;
   
   bypass_noshift:
   // read history buffer to output
   r0 = M[I5, M0];
   do empty_hist_noshift;
      M[I1, M0] = r0, r0 = M[I5, M0];
   empty_hist_noshift:

   r10 = r6 - r1; // Run loop for NbSamples - History Samples
   r0 = M[I4, M0];
   do straight_copy_noshift;
      M[I1, M0] = r0, r0 = M[I4, M0];
   straight_copy_noshift:
   
   // fill the history buffer
   r10 = r1;
   do fill_hist_noshift;
      M[I0, M0] = r0, r0 = M[I4, M0];
   fill_hist_noshift:

   next_passthrough_channel:
   // Advance channel specific data pointer (r7)
   r7 = r7 + ($cbops.rate_adjustment_and_shift.CHAN_STRUC_SIZE * ADDR_PER_WORD);
   // update channel count
   r3 = r3 + 1*ADDR_PER_WORD;

   Null = r3 - r9;
   if LT jump passthrough_channel;

   // ------------------- end of channel processing loop -----------------------

   L5 = 0;
   L4 = 0;
   L0 = 0;
   L1 = 0;
   push NULL;
   B0 = M[SP - 1*ADDR_PER_WORD];
   B1 = M[SP - 1*ADDR_PER_WORD];
   B4 = M[SP - 1*ADDR_PER_WORD];
   pop B5;



   // kill off temp var area and close the ceremony
   SP = SP - ($cbops.sra_temp_var.NUM_TEMP_VARS), popm <FP, rLink>;
   rts;

.ENDMODULE;

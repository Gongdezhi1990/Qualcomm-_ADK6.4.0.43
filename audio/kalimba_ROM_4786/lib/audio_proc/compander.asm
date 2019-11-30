// *****************************************************************************
// Copyright (c) 2015 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "core_library.h"
#include "compander.h"

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif


// *****************************************************************************
// MODULE:
//    $audio_proc.cmpd.initialize
//
// DESCRIPTION:
//    Initialize function for the compander module
//
// INPUTS:
//    - r8 = pointer to the compander current_channel(left/right) data_object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - Assume everything
//
// NOTES:
// *****************************************************************************
.MODULE $M.audio_proc.cmpd.initialize;
   .CODESEGMENT   PM;

$audio_proc.cmpd.initialize:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.COMPANDER_ASM.CMPD.INITIALIZE.PATCH_ID_0,r1)     // compander_patchers1
//#endif



   // ***********************************
   // load the pointer to parameters
   // ***********************************
   r9 = M[r8 + $audio_proc.compander.PARAM_PTR_FIELD];

   // ******************
   // calculate 1/fs
   // ******************
   r0 = M[r8 + $audio_proc.compander.SAMPLE_RATE_FIELD];
   r2 = 0;
   call $audio_proc.cmpd.recip_calc;

   // *****************************
   // exp(-1/(LevelAttackTime*fs))
   // *****************************
   r4 = r5;
   r1 = 0;
   r0 = M[r9 + $audio_proc.compander.parameter.LEVEL_ATTACK_TC];
   if NZ call $audio_proc.cmpd.tc_exp_calc;
   M[r8 + $audio_proc.compander.LVL_ALPHA_ATK] = r1;

   // *****************************
   // exp(-1/(LevelReleaseTime*fs))
   // *****************************
   r1 = 0;
   r0 = M[r9 + $audio_proc.compander.parameter.LEVEL_RELEASE_TC];
   if NZ call $audio_proc.cmpd.tc_exp_calc;
   M[r8 + $audio_proc.compander.LVL_ALPHA_RLS] = r1;

   // *****************************
   // exp(-1/(LevelAverageTime*fs))
   // *****************************
   r1 = 0;
   r0 = M[r9 + $audio_proc.compander.parameter.LEVEL_AVERAGE_TC];
   if NZ call $audio_proc.cmpd.tc_exp_calc;
   M[r8 + $audio_proc.compander.LVL_ALPHA_AVG] = r1;

   // ******************************************************
   //   CALCULATE SLOPE AND INTERCEPT FOR ALL SECTIONS
   // ******************************************************
   pushm <r4,r8,r9>;

   // ********************
   //  calculate slope
   // ********************
   r10 = M[r9 + $audio_proc.compander.parameter.NUM_SECTIONS];
   r10 = r10 - 1;

   // ********************************************
   // set slope for the last section (1.0)
   // slope is in q.19(arch3)/q.27(arch4) format
   // ********************************************
   r0 = r8 + $audio_proc.compander.SLOPE1;
   r1 = Qfmt_(1.0,5);
   Words2Addr(r10);
   M[r0 + r10] = r1;
   Addr2Words(r10);

   // ************************************************
   // calculate slope for all remaining sections
   // slope is in q.19(arch3)/q.27(arch4) format
   // ************************************************
   I0 = r9 + $audio_proc.compander.parameter.GAIN_RATIO_SECTION1;
   I4 = r0;
   r7 = 9;                                 // (4 + 5) bit shifts
   r2 = 1;
   M1 = 1 * ADDR_PER_WORD;
   cmpd_ratio_inv_loop:
      r0 = M[I0,M1] , r1 = r7;
      call $audio_proc.cmpd.recip_calc;
      M[I4,M1] = r5 , r10 = r10 - r2;
   if NZ jump cmpd_ratio_inv_loop;

   // **********************
   //  calculate intercept
   // **********************
   r10 = M[r9 + $audio_proc.compander.parameter.NUM_SECTIONS];
   r10 = r10 - 1;

   // ************************************
   // set intercept for the last section
   // ************************************
   r0 = r8 + $audio_proc.compander.INTERCEPT1;
   Words2Addr(r10);
   M[r0 + r10] = Null;
   Addr2Words(r10);

   // **************************************************************************
   // load input/output pointers with offsets starting from the last section
   // **************************************************************************
   r10 = r10 - 1;
   Words2Addr(r10);
   I7 = r8 + $audio_proc.compander.INTERCEPT1;
   I7 = I7 + r10;
   I3 = r8 + $audio_proc.compander.SLOPE1;
   I3 = I3 + r10;
   I6 = r9 + $audio_proc.compander.parameter.GAIN_THRESHOLD_SECTION1;
   I6 = I6 + r10;
   Addr2Words(r10);

   // *******************************************************
   // calculate intercept for the (last-1) section
   // incpt(last) = Threshold(last-1) + (1.0 - slope(last-1))
   // *******************************************************
   M0 = 0 * ADDR_PER_WORD;
   M1 = -1 * ADDR_PER_WORD;
   r3 = M[I3,M0] , r1 = M[I6,M1]; // slope in q.19(arch3)/q.27(arch4) , threshold in q.19(arch3)/q.27(arch4)
   r2 = Qfmt_(1.0,5) - r3;        // (1.0 - slope) in q.19(arch3)/q.27(arch4)
   r4 = r1 * r2(frac);            // intercept in q.15(arch3)/q.23(arch4)

   // **************************************************************
   // calculate intercept for the remaining sections
   // incpt(i) = Threshold(i)*[slope(i+1) - slope(i)] + incpt(i+1)
   // **************************************************************
   r3 = M[I3,M1] , M[I7,M0] = r4;     // preload slope(i+1) , write intercept of last section
   do intercept_calc_loop;
      r1 = M[I6,M1] , r5 = M[I3,M0];  // load threshold in q.19(arch3)/q.27(arch4) , load slope(i)
      r3 = r3 - r5 , r4 = M[I7,M1];   // slope(i+1) - slope(i) in q.19(arch3)/q.27(arch4) , load incpt(i+1) in q.15(arch3)/q.23(arch4)
      r4 = r4 + r1 * r3;              // incpt(i) = incpt(i+1) + threshold(i)*r3
      M[I7,M0] = r4 , r3 = M[I3,M1];  // write incpt(i) in q.15(arch3)/q.23(arch4) , load slope(i+1)
   intercept_calc_loop:

   popm <r4,r8,r9>;

   // *********************************************************************
   // set "gain_update_rate"
   // gain_update_rate = 0.5ms*sample_rate(block_based)/ 1(sample_based)
   // *********************************************************************
   r0 = M[r8 + $audio_proc.compander.SAMPLE_RATE_FIELD];
   r1 = 0.0005;
   r0 = r0 * r1(frac);
   r2 = 1;
   Null = M[r9 + $audio_proc.compander.parameter.GAIN_UPDATE_FLAG];
   if Z r0 = r2;
   M[r8 + $audio_proc.compander.GAIN_UPDATE_RATE] = r0;

   // **************************************
   // set inverse of "gain_update_rate"
   // **************************************
   r2 = 0;
   call $audio_proc.cmpd.recip_calc;
   M[r8 + $audio_proc.compander.GAIN_UPDATE_RATE_INV] = r5;

   // ********************************************************
   // GainAlphaAtk = exp(-1/(AttackTime*fs/GainUpdateRate))
   // GainAlphaAtk = exp(-GainUpdateRate/(AttackTime*fs))
   // ********************************************************
   r1 = 0;
   r0 = M[r8 + $audio_proc.compander.GAIN_UPDATE_RATE];
   r4 = r4 * r0(int);                                           // GainUpdateRate/Fs
   r0 = M[r9 + $audio_proc.compander.parameter.GAIN_ATTACK_TC];
   if NZ call $audio_proc.cmpd.tc_exp_calc;
   M[r8 + $audio_proc.compander.GAIN_ALPHA_ATK] = r1;

   // ********************************************************
   // GainAlphaRls = exp(-1/(ReleaseTime*fs/GainUpdateRate))
   // GainAlphaRls = exp(-GainUpdateRate/(ReleaseTime*fs))
   // ********************************************************
   r1 = 0;
   r0 = M[r9 + $audio_proc.compander.parameter.GAIN_RELEASE_TC];
   if NZ call $audio_proc.cmpd.tc_exp_calc;
   M[r8 + $audio_proc.compander.GAIN_ALPHA_RLS] = r1;

   // ****************************************************
   // initialize the lookahead history buffer pointer
   // ****************************************************
   r0 = M[r8 + $audio_proc.compander.START_LOOKAHEAD_HIST];
   M[r8 + $audio_proc.compander.PTR_LOOKAHEAD_HIST] = r0;

   // ********************************************************************
   // calculate "lookahead_samples"
   // set "lookahead_samples" to 0 if lookahead buffer allocation failed
   // ********************************************************************
   r0 = M[r8 + $audio_proc.compander.SAMPLE_RATE_FIELD];
   r1 = M[r9 + $audio_proc.compander.parameter.LOOKAHEAD_TIME];
   r0 = r0 * r1;
   Null = M[r8 + $audio_proc.compander.START_LOOKAHEAD_HIST];
   if Z r0 = 0;
   M[r8 + $audio_proc.compander.LOOKAHEAD_SAMPLES] = r0;

   // ***********************************************************
   // Initialize "framebuffer_flag" for the current data_object
   // ***********************************************************
   M[r8 + $audio_proc.compander.FRAMEBUFFER_FLAG] = Null;

   // ***********************************************************
   // set "gain_interp_flag" = 0 for sample_based processing
   // set "gain_interp_flag" = 0/1 for block_based processing
   // ***********************************************************
   r1 = M[r9 + $audio_proc.compander.parameter.GAIN_INTERP_FLAG];
   r0 = M[r9 + $audio_proc.compander.parameter.GAIN_UPDATE_FLAG];
   if Z r1 = 0;
   M[r9 + $audio_proc.compander.parameter.GAIN_INTERP_FLAG] = r1;

   // *******************************
   // set inverse of "num_channels"
   // *******************************
   r0 = M[r8 + $audio_proc.compander.NUM_CHANNELS];
   r2 = 0;
   call $audio_proc.cmpd.recip_calc;
   M[r8 + $audio_proc.compander.NUM_CHANNELS_INV] = r5;

   // ****************************************************************************
   // initialize "GAIN_SMOOTH_HIST_LINEAR" value to 1.0[q.19(arch3)/q.27(arch4)]
   // ****************************************************************************
   r0 = Qfmt_(1.0,5);
   M[r8 + $audio_proc.compander.GAIN_SMOOTH_HIST_LINEAR] = r0;

   // ***********************************
   // set level detect function pointer
   // ***********************************
   r1 = $audio_proc.cmpd.level_detect_peak;
   r2 = $audio_proc.cmpd.level_detect_rms;
   r0 = M[r9 + $audio_proc.compander.parameter.LEVEL_ESTIMATION_FLAG];
   if Z r1 = r2;
   M[r8 + $audio_proc.compander.LEVEL_DETECT_FUNC_PTR] = r1;

cmpd_init_done:
   jump $pop_rLink_and_rts;

.ENDMODULE;


// ***********************************************************************************
// MODULE:
//    $audio_proc.cmpd.tc_exp_calc
//
// DESCRIPTION:
//    Calculate exp(-1/(TC*fs)) for the input TC and fs
//
// INPUTS:
//    - r0 = TC (range = 0.01 msec to 5000 msec) in q.20(arch3)/q.28(arch4) format
//    - r4 = 1/fs
//
// OUTPUTS:
//    - r1 = exp(-1/(TC*fs)) in q.23(arch3)/q.31(arch4)
//
// TRASHED REGISTERS:
//    - r0,r2,r3,r5,r6,r7,I7
//
// NOTES:
// ***********************************************************************************
.MODULE $M.audio_proc.cmpd.tc_exp_calc;
   .CODESEGMENT   PM;

$audio_proc.cmpd.tc_exp_calc:

   push rLink;


   r1 = 21;                    // (3 + 18) bit shifts
   r2 = 1;
   call $audio_proc.cmpd.recip_calc;

   // ***********
   // 1/(TC*fs)
   // ***********
   r3 = r4 ASHIFT 10;
   r5 = r5 ASHIFT 1;
   r1 = r3 * r5(frac);

   // ***************
   // exp(-1/(TC*fs))
   // ***************
   r1 = -r1;
   r0 = r1 * Qfmt_(1.44269504,2);         // q.16(arch3)/q.24(arch4)
   call $math.pow2_taylor;
   r1 = r0;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// **************************************************************************************
// MODULE:
//    $audio_proc.cmpd.recip_calc
//
// DESCRIPTION:
//    Calculate reciprocal of the input value
//
// INPUTS:
//    - r0 = input value
//    - r1 = number of bit-shifts (valid only for fractional input)
//    - r2 = flag indicating integer/fractional(0/1) input
//
// OUTPUTS:
//    - r5 = fixed-point format output(in appropriate q.format for fractional input)
//
// TRASHED REGISTERS:
//    - r0,r1,r6,rMAC
//
// NOTES:
// **************************************************************************************
.MODULE $M.audio_proc.cmpd.recip_calc;
   .CODESEGMENT   PM;

$audio_proc.cmpd.recip_calc:

   push rLink;


   r5 = $kal_float_lib.q_to_float;
   r6 = $kal_float_lib.int_to_float;
   Null = r2;
   if Z r5 = r6;
   call r5;
   call $kal_float_lib.recip;
   r1 = r1 LSHIFT -1;          // shift mantissa so fits twos complement arithmetic
   r0 = r0 and 0x0000ff;       // expect sign to be zero, but mask off just in case
   r0 = r0 - 127;              // remove offset from exponent
   r5 = r1 ASHIFT r0;          // fixed-point equivalent

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.compander.process
//
// DESCRIPTION:
//    This function does the compander processing on the inputs. It has 2 entry points.
//    $audio_proc.compander.frame_process  : Entry point for frame_sync API
//    $audio_proc.compander.stream_process : Entry point for Kymera API
//
// INPUTS:
//    r8 = pointer to first channel compander data_object
//
// OUTPUTS:
//    None
//
// TRASHED REGISTERS:
//    Everything except r8
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.compander.process;
   .CODESEGMENT   PM;

$audio_proc.compander.frame_process:


   // ************************************************************************************************
   // Sets "samples_to_process" and "input_framebuffer_flag" value for all compander data_objects
   // Falls through to the $audio_proc.compander.stream_process() function
   // ************************************************************************************************

   // *******************************************************************
   // set "framebuffer_flag" and "samples_to_process" for all channels
   // *******************************************************************
   r0 = M[r8 + $audio_proc.compander.DATA_OBJECTS_PTR];
   I0 = r0;
   r1 = 1;
   M1 = 1 * ADDR_PER_WORD;
   r10 = M[r8 + $audio_proc.compander.NUM_CHANNELS];                   // number of channels
   do cmpd_frame_proc_init_loop;
      r3 = M[I0,M1];                                                   // load current channel data_object
      r4 = M[r3 + $audio_proc.compander.CHANNEL_INPUT_PTR];            // load input framebuffer pointer for current channel
      r0 = M[r4 + $frmbuffer.FRAME_SIZE_FIELD];                        // load "samples_to_process" for current channel
      M[r3 + $audio_proc.compander.SAMPLES_TO_PROCESS] = r0;           // set "samples_to_process" for current channel
      M[r3 + $audio_proc.compander.FRAMEBUFFER_FLAG] = r1;             // set "framebuffer_flag" for current channel
   cmpd_frame_proc_init_loop:

$audio_proc.compander.stream_process:

   push rLink;

   // ****************************************************************************
   // initialize input_read_addr/output_write_addr for all data_objects(channels)
   // ****************************************************************************
   call $audio_proc.cmpd.input_read_output_write_init;

   // *****************************************************
   // load "samples_to_process" and "gain_update_rate"
   // ****************************************************
   r3 = M[r8 + $audio_proc.compander.SAMPLES_TO_PROCESS];
   r4 = M[r8 + $audio_proc.compander.GAIN_UPDATE_RATE];

   // *******************************************************
   // process all channels in blocks of "gain_update_rate"
   // *******************************************************
   cmpd_frame_proc_loop:
      Null = r3 - r4;
      if LT jump reset_samples_to_process;
      pushm <r3,r4>;
      call $audio_proc.cmpd.process_channels;
      popm <r3,r4>;
      r3 = r3 - r4;
      if Z jump cmpd_process_done;
   jump cmpd_frame_proc_loop;

reset_samples_to_process:
   r0 = M[r8 + $audio_proc.compander.SAMPLES_TO_PROCESS];
   r0 = r0 - r3;
   M[r8 + $audio_proc.compander.SAMPLES_TO_PROCESS] = r0;

cmpd_process_done:
   jump $pop_rLink_and_rts;

.ENDMODULE;


// ********************************************************************************************************
// MODULE:
//    $audio_proc.cmpd.process_channels
//
// DESCRIPTION:
//    This function calls a series of functions operating on the input channels
//
// INPUTS:
//    r8 = pointer to first channel data_object
//
// OUTPUTS:
//    None
//
// TRASHED REGISTERS:
//    Everything except r8
//
// NOTES
//
// ********************************************************************************************************
.MODULE $M.audio_proc.cmpd.process_channels;
   .CODESEGMENT   PM;

$audio_proc.cmpd.process_channels:

   push rLink;


   // ***************************************************
   // call level_detect() function channel by channel
   // ***************************************************
   r9 = M[r8 + $audio_proc.compander.NUM_CHANNELS];           // number of channels
   r1 = M[r8 + $audio_proc.compander.DATA_OBJECTS_PTR];
   I2 = r1;
   M1 = 1 * ADDR_PER_WORD;
   M3 = 1;
   r7 = M[r8 + $audio_proc.compander.NUM_CHANNELS_INV];       // load 1/num_channels
   r4 = 0;                                                    // initialize running max/running mean(rms)across channels
   r5 = M[r8 + $audio_proc.compander.LEVEL_DETECT_FUNC_PTR];  // load level detect function pointer
   r3 = M[I2,M1];                                             // load first channel data_object
   level_detect_call_loop:
      call r5;                                                // call level detect peak/rms function
      r9 = r9 - M3 , r3 = M[I2,M1];                           // decrement channel_counter , load next channel data_object
   if NZ jump level_detect_call_loop;

   // *************************************************
   // calculate log2(max)/log2(mean(rms))
   // store output in the first channel data_object
   // *************************************************
   rMAC = r4;
   call $math.log2_taylor;                                    // output is in r0 in q.16(arch3)/q.24(arch4) format
   M[r8 + $audio_proc.compander.LEVEL_DETECT_CHN_MAX_OR_MEAN_LOG2] = r0;

   // *******************************
   // call computegain() function
   // *******************************
   call $audio_proc.cmpd.computegain;

   // *******************************
   // call smoothGainB() function
   // *******************************
   call $audio_proc.cmpd.smoothGainB;

   // *******************************
   // call smoothGainS() function
   // *******************************
   call $audio_proc.cmpd.smoothGainS;

   // **************************************************
   // advance the read/write pointers for all channels
   // **************************************************
   r10 = M[r8 + $audio_proc.compander.NUM_CHANNELS];       // number of channels
   r1 = M[r8 + $audio_proc.compander.DATA_OBJECTS_PTR];
   I2 = r1;
   do cmpd_advance_ptrs_loop;
      r3 = M[I2,M1];                                       // load current channel data_object
      call $audio_proc.cmpd.advance_pointers;
   cmpd_advance_ptrs_loop:

   jump $pop_rLink_and_rts;

.ENDMODULE;


// ********************************************************************************************************
// MODULE:
//    $audio_proc.cmpd.level_detect_peak
//
// DESCRIPTION:
//    Calculate max(input_level)for current channel and update running max(across channels)
//    Update last sample hist value for the current channel
//
// INPUTS:
//    r3 = current channel data_object
//    M1 = preset modify register
//
// OUTPUTS:
//    r4 = running max across channels
//
// TRASHED REGISTERS:
//    r0,r1,r2,r6,r7,r10,rMAC,rMACB,I0,L0,B0
//
// NOTES
//
// ********************************************************************************************************
.MODULE $M.audio_proc.cmpd.level_detect_peak;
   .CODESEGMENT   PM;

$audio_proc.cmpd.level_detect_peak:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.COMPANDER_ASM.CMPD.LEVEL_DETECT_PEAK.PATCH_ID_0,r1)     // compander_patchers
//#endif

   // **********************
   // load the block_size
   // **********************
   r10 = M[r3 + $audio_proc.compander.GAIN_UPDATE_RATE];

   // ***************************************************************
   // extract the <size,base_address> for input cbuffer/frmbuffer
   // <size,base_address> are in <L0,B0> respectively
   // ***************************************************************
   r0 = M[r3 + $audio_proc.compander.CHANNEL_INPUT_PTR];
   r1 = M[r3 + $audio_proc.compander.FRAMEBUFFER_FLAG];
   call $audio_proc.cmpd.get_input_buffer_details;
   r0 = M[r3 + $audio_proc.compander.INPUT_READ_ADDR];
   I0 = r0;

   // *****************************
   // load last sample history
   // load time constants
   // *****************************
   rMAC = M[r3 + $audio_proc.compander.LEVEL_DETECT_LAST_SAMPLE_HIST];  // last smoothed sample from previous frame
   r1 = M[r3 + $audio_proc.compander.LVL_ALPHA_ATK];                    // alphaA
   r6 = M[r3 + $audio_proc.compander.LVL_ALPHA_RLS];                    // alphaR
   rMACB = 0;                                                           // initialize max value
   do level_detect_peak_loop;
      r7 = r6 , r0 = M[I0,M1];                                          // set r7 to alpha_rls , load input
      Null = r0 - rMAC;                                                 // x(sample,ch) > y(sample,ch)
      if GT r7 = r1;                                                    // if x(sample,ch) > y(sample,ch) then r7 = alpha_atk
      r0 = ABS r0;                                                      // abs(x(sample,ch))
      r2 = -1.0 + r7;                                                   // -(beta)
      rMAC = r7 * rMAC;                                                 // alpha*y(sample,ch)
      rMAC = rMAC - r2 * r0;                                            // y(sample+1) = alpha*y(sample,ch) + beta*abs(x(sample,ch))
      r2 = rMAC - rMACB;                                                // check y(sample+1) against previous max
      if GT rMACB = rMAC;                                               // update max if necessary
   level_detect_peak_loop:

   // *****************************************************
   // update last sample history for the next block
   // update running max across channels
   // *****************************************************
   M[r3 + $audio_proc.compander.LEVEL_DETECT_LAST_SAMPLE_HIST] = rMAC;
   Null = rMACB - r4;
   if GT r4 = rMACB;

   // ***************************************
   // clear all base and length registers
   // ***************************************
#ifdef BASE_REGISTER_MODE
   push Null;
   pop B0;
#endif
   L0 = 0;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// ********************************************************************************************************
// MODULE:
//    $audio_proc.cmpd.level_detect_rms
//
// DESCRIPTION:
//    Calculate rms(input_level)for current channel and update running mean(rms)across channels
//    Update last sample hist value for the current channel
//
// INPUTS:
//    r3 = current channel data_object
//    M1 = preset modify register
//    r7 = 1/num_channels
//
// OUTPUTS:
//    r4 = running mean(rms) across channels
//
// TRASHED REGISTERS:
//    r0,r1,r2,r10,rMAC,I0,L0,B0
//
// NOTES
//
// ********************************************************************************************************
.MODULE $M.audio_proc.cmpd.level_detect_rms;
   .CODESEGMENT   PM;

$audio_proc.cmpd.level_detect_rms:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.COMPANDER_ASM.CMPD.LEVEL_DETECT_RMS.PATCH_ID_0,r1)     // compander_patchers
//#endif

   // *********************
   // load the block_size
   // *********************
   r10 = M[r3 + $audio_proc.compander.GAIN_UPDATE_RATE];

   // ***************************************************************
   // extract the <size,base_address> for input cbuffer/frmbuffer
   // <size,base_address> are in <L0,B0> respectively
   // ***************************************************************
   r0 = M[r3 + $audio_proc.compander.CHANNEL_INPUT_PTR];
   r1 = M[r3 + $audio_proc.compander.FRAMEBUFFER_FLAG];
   call $audio_proc.cmpd.get_input_buffer_details;
   r0 = M[r3 + $audio_proc.compander.INPUT_READ_ADDR];
   I0 = r0;

   // *****************************
   // load last sample history
   // load time constants
   // *****************************
   rMAC = M[r3 + $audio_proc.compander.LEVEL_DETECT_LAST_SAMPLE_HIST];  // last smoothed sample from previous frame
   r1 = M[r3 + $audio_proc.compander.LVL_ALPHA_AVG];                    // alpha
   r2 = 1.0 - r1;                                                       // beta
   do level_detect_rms_loop;
      rMAC = r1 * rMAC , r0 = M[I0,M1];                                 // alpha*y(sample,ch) , load input
      r0 = ABS r0;                                                      // abs(x(sample,ch))
      rMAC = rMAC + r2 * r0;                                            // alpha*y(sample,ch) + beta*abs(x(sample,ch))
   level_detect_rms_loop:

   // ***********************************************
   // update last sample history for the next block
   // update running mean(rms) across channels
   // ***********************************************
   M[r3 + $audio_proc.compander.LEVEL_DETECT_LAST_SAMPLE_HIST] = rMAC;
   r4 = r4 + rMAC * r7;

   // ***************************************
   // clear all base and length registers
   // ***************************************
#ifdef BASE_REGISTER_MODE
   push Null;
   pop B0;
#endif
   L0 = 0;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// ********************************************************************************************************
// MODULE:
//    $audio_proc.cmpd.computegain
//
// DESCRIPTION:
//    Compute gain at the block level
//
// INPUTS:
//    r8 = pointer to first channel data_object
//    M1 = preset modify register
//
// OUTPUTS:
//    rMAC = G_log2
//
// TRASHED REGISTERS:
//    r0-r7,r9,r10,I0,I1,M2
//
// NOTES
//
// ********************************************************************************************************
.MODULE $M.audio_proc.cmpd.computegain;
   .CODESEGMENT   PM;

$audio_proc.cmpd.computegain:

   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.COMPANDER_ASM.CMPD.COMPUTEGAIN.PATCH_ID_0,r1)     // compander_patchers
#endif

   // ***************************************************
   // load "Level_log2" value(arch3:q.16,arch4:q.24)
   // ***************************************************
   r3 = M[r8 + $audio_proc.compander.LEVEL_DETECT_CHN_MAX_OR_MEAN_LOG2];

   // ****************************
   // load input parameters
   // ****************************
   r7 = M[r8 + $audio_proc.compander.PARAM_PTR_FIELD];
   r10 = M[r7 + $audio_proc.compander.parameter.NUM_SECTIONS];
   r5 = r10;                               // initialize threshold_index
   r10 = r10 - 1;
   r6 = r10;                               // initialize kw_index'
   r1 = r10 - 1;
   Words2Addr(r1);
   I0 = r7 + $audio_proc.compander.parameter.GAIN_THRESHOLD_SECTION1;
   I0 = I0 + r1;                           // set threshold input pointer
   I1 = r7 + $audio_proc.compander.parameter.GAIN_KNEEWIDTH_SECTION1;
   M2 = -1 * ADDR_PER_WORD;
   I1 = I1 + r1 , r0 = M[I0,M2];           // set kneewidth input pointer , preload threshold in q.19(arch3)/q.27(arch4)
   r1 = -3;
   do computegain_loop;
      r0 = r0 ASHIFT r1 , r2 = M[I1,M2];   // threshold in q.16(arch3)/q.24(arch4) , load kneewidth(arch3:q.23,arch4:q.31)
      r2 = r2 ASHIFT -7;                   // kneewidth in q.16(arch3)/q.24(arch4)
      Null = r0 - r3;                      // Threshold(j) <= xG
      if LE r5 = r10;                      // if(Threshold(j) <= xG) update threshold_index
      r4 = r3 - r0;                        // xG-Threshold(j)
      r4 = ABS r4;                         // abs(xG-Threshold(j))
      r4 = r4 * 2(int);                    // 2*abs(xG-Threshold(j)
      Null = r4 - r2;                      // 2*abs(xG-Threshold(j)) <= KneeWidth(j)
      if LE r6 = r10;                      // if(2*abs(xG-Threshold(j)) <= KneeWidth(j)) update kw_index'
      r0 = M[I0,M2];                       // load next threshold(not done in ||lel to retain r10 for previous instruction)
   computegain_loop:

   // ************************************************
   // find kw_index = min(threshold_index,kw_index')
   // ************************************************
   r6 = MIN r5;

   // *********************************************************
   // check if input falls in the soft-knee part of section k
   // shift input to full_scale
   // *********************************************************
   r9 = SIGNDET r3;
   r4 = r3 ASHIFT r9;
   r10 = r6 - 1;
   Words2Addr(r10);
   r0 = r7 + $audio_proc.compander.parameter.GAIN_THRESHOLD_SECTION1;
   r0 = M[r0 + r10];
   r0 = r0 ASHIFT -3;                      // threshold in q.16(arch3)/q.24(arch4)
   r1 = r7 + $audio_proc.compander.parameter.GAIN_KNEEWIDTH_SECTION1;
   r1 = M[r1 + r10];
   r1 = r1 ASHIFT -7;                      // kneewidth in q.16(arch3)/q.24(arch4)
   r2 = r3 - r0;                           // xG-Threshold(s)
   r2 = ABS r2;                            // abs(xG-Threshold(s))
   r2 = r2 * 2(int);                       // 2*abs(xG-Threshold(s)
   Null = r2 - r1;                         // 2*abs(xG-Threshold(s)) <= KneeWidth(s)
   if GT jump linear_part;

soft_knee_part:
   r6 = r6 - 1;
   r6 = r6 * 3(int);                       // offset into the coeffs
   Words2Addr(r6);
   I0 = r7 + $audio_proc.compander.parameter.SOFT_KNEE_1_COEFF_A;
   I0 = I0 + r6;
   rMAC = r4 * r4 , r0 = M[I0,M1];
   rMAC = rMAC * r0;
   r1 = -2 * r9(int);
   r0 = r1 + 19;
   rMAC = rMAC ASHIFT r0 , r1 = M[I0,M1];
   r1 = r1 * r4;
   r0 = 12 - r9;
   r1 = r1 ASHIFT r0;
   rMAC = rMAC + r1 , r2 = M[I0,M1];
   r2 = r2 ASHIFT 5;
   rMAC = rMAC + r2;                       // yG = polyval(p, xG) (arch3:q.15, arch4:q.23)
   jump compute_gain;

linear_part:
   r5 = r5 - 1;
   Words2Addr(r5);
   r0 = r8 + $audio_proc.compander.SLOPE1;
   r1 = M[r0 + r5];                        // (arch3:q.19,arch4:q.27)
   r0 = r8 + $audio_proc.compander.INTERCEPT1;
   r2 = M[r0 + r5];                        // (arch3:q.15,arch4:q.23)
   rMAC = r1 * r4(frac);
   r5 = 3 - r9;                            // bit-shifts for q.15(arch3)/q.23(arch4)
   rMAC = rMAC ASHIFT r5;                  // arch3:q.15, arch4:q.23
   rMAC = rMAC + r2;                       // yG = polyval(p, xG) (arch3:q.15, arch4:q.23)

compute_gain:
   r0 = M[r7 + $audio_proc.compander.parameter.MAKEUP_GAIN];
   r0 = r0 ASHIFT -6;
   rMAC = rMAC ASHIFT 1;                   // yG (arch3:q.16, arch4:q.24)
   rMAC = rMAC - r3;                       // yG - xG
   rMAC = rMAC + r0;                       // G_log2 = (yG - xG + MakeUpGain) (arch3:q.16, arch4:q.24)

   jump $pop_rLink_and_rts;

.ENDMODULE;



// ********************************************************************************************************
// MODULE:
//    $audio_proc.cmpd.smoothGainB
//
// DESCRIPTION:
//    Gain smoothing at the block level
//
// INPUTS:
//    r8   = pointer to first channel data_object
//    rMAC = Gt(G_log2) (arch3:q.16, arch4:q.24)
//    M1   = preset modify register
//
// OUTPUTS:
//    None
//
// TRASHED REGISTERS:
//    r0-r4
//
// NOTES
//
// ********************************************************************************************************
.MODULE $M.audio_proc.cmpd.smoothGainB;
   .CODESEGMENT   PM;

$audio_proc.cmpd.smoothGainB:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.COMPANDER_ASM.CMPD.SMOOTHGAINB.PATCH_ID_0,r1)     // compander_patchers
//#endif

   r1 = M[r8 + $audio_proc.compander.GAIN_ALPHA_ATK];
   r2 = M[r8 + $audio_proc.compander.GAIN_ALPHA_RLS];
   r4 = M[r8 + $audio_proc.compander.GAIN_SMOOTH_HIST];    // load Gs_old
   Null = rMAC - r4;                                       // Gt - Gs
   if LT r2 = r1;
   r3 = 1.0 - r2;
   r0 = r2 * r4;
   r0 = r0 + r3 * rMAC;                                    // Gs(arch3:q.16, arch4:q.24) = Gain_param.GainAlpha*Gs + Gain_param.GainBeta*Gt
   M[r8 + $audio_proc.compander.GAIN_SMOOTH_HIST] = r0;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// ********************************************************************************************************
// MODULE:
//    $audio_proc.cmpd.smoothGainS
//
// DESCRIPTION:
//    Gain smoothing at the sample level
//
// INPUTS:
//    r8 = pointer to first channel data_object
//    M1 = preset modify register
//
// OUTPUTS:
//    None
//
// TRASHED REGISTERS:
//    r0-r7,r9,r10,rMAC,I0,I1,I3,I4,I5,I7,B0,B1,B4,L0,L1,L4,M0,M2,M3
//
// NOTES
//
// ********************************************************************************************************
.MODULE $M.audio_proc.cmpd.smoothGainS;
   .CODESEGMENT   PM;

$audio_proc.cmpd.smoothGainS:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.COMPANDER_ASM.CMPD.SMOOTHGAINS.PATCH_ID_0,r1)     // compander_patchers
//#endif


   // ************************************
   // load Gs_old(in linear domain)
   // ************************************
   r3 = M[r8 + $audio_proc.compander.GAIN_SMOOTH_HIST_LINEAR];

   // ************************************
   // convert Gs to linear domain
   // ************************************
   call $audio_proc.cmpd.calc_pow2_input;

   // ********************************
   // check "gain_interp_flag" value
   // ********************************
   r0 = M[r8 + $audio_proc.compander.PARAM_PTR_FIELD];
   r0 = M[r0 + $audio_proc.compander.parameter.GAIN_INTERP_FLAG];
   if Z jump gain_final_apply;

gain_interp_on:
   // *************************************************
   // calculate Gdelta = (Gs - Gs_old)/GainUpdateRate
   // *************************************************
   r5 = M[r8 + $audio_proc.compander.GAIN_UPDATE_RATE_INV];
   r4 = r4 - r3;
   r4 = r4 * r5(frac);

gain_final_apply:
   r1 = M[r8 + $audio_proc.compander.DATA_OBJECTS_PTR];
   I3 = r1;                                                    // pointer to all data_objects
   r6 = M[r8 + $audio_proc.compander.NUM_CHANNELS];            // number of channels
   r5 = M[I3,M1];                                              // preload channel1 data_object
   M3 = 1;
   gain_apply_call_loop:
      call $audio_proc.cmpd.final_gain_apply;                  // call gain_apply function for the current input channel , load next channel data_object
      r6 = r6 - M3 , r5 = M[I3,M1] ;
   if NZ jump gain_apply_call_loop;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// ********************************************************************************************************
// MODULE:
//    $audio_proc.cmpd.calc_pow2_input
//
// DESCRIPTION:
//    Convert the "gain_smooth_hist" value from log2() to linear domain
//    Update the data_object with the output
//
// INPUTS:
//    r8 = pointer to first channel data_object
//
// OUTPUTS:
//    None
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r4,r6,r7,I7
//
// NOTES
//
// ********************************************************************************************************
.MODULE $M.audio_proc.cmpd.calc_pow2_input;
   .CODESEGMENT   PM;

$audio_proc.cmpd.calc_pow2_input:

   push rLink;
//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.COMPANDER_ASM.CMPD.CALC_POW2_INPUT.PATCH_ID_0,r1)     // compander_patchers
//#endif



   r4 = Qfmt_(1.0,5);
   r0 = M[r8 + $audio_proc.compander.GAIN_SMOOTH_HIST];
   if Z jump cmpd_pow2_calc_done;
   if NEG jump cmpd_pow2_call;

gain_smooth_pos:
   r2 = r0 ASHIFT (8-DAWTH);   // integer
   r1 = r2 ASHIFT (DAWTH - 8); // integer in q.16(arch3)/q.24(arch4)
   r4 = r0 - r1;               // fractional value in q.16(arch3)/q.24(arch4)
   r1 = r2 + (DAWTH-4);        // integer+1 q.19(arch3)/q.27(arch4)
   r0 = r4 - Qfmt_(1.0,8);     // negative fractional part
   r4 = 1 ASHIFT r1;           // pow2(integer_part) in q.19(arch3)/q.27(arch4)

cmpd_pow2_call:
   call $math.pow2_taylor;
   r4 = r4 * r0;

cmpd_pow2_calc_done:
   M[r8 + $audio_proc.compander.GAIN_SMOOTH_HIST_LINEAR] = r4;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// ********************************************************************************************************
// MODULE:
//    $audio_proc.cmpd.final_gain_apply
//
// DESCRIPTION:
//    Apply the final compander gain to the input
//
// INPUTS:
//    r5   = pointer to current channel data_object
//    r4   = Gs(gain_interp = off) / Gdelta(gain_interp = on)
//    r3   = Gs_old
//    M1   = preset modify register
//
// OUTPUTS:
//    None
//
// TRASHED REGISTERS:
//    r0-r2,r7,r9,r10,I0,I1,I4,B0,B1,B4,L0,L1,L4,M0,M2
//
// NOTES
//
// ********************************************************************************************************
.MODULE $M.audio_proc.cmpd.final_gain_apply;
   .CODESEGMENT   PM;

$audio_proc.cmpd.final_gain_apply:

   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.COMPANDER_ASM.CMPD.FINAL_GAIN_APPLY.PATCH_ID_0,r1)     // compander_patchers
#endif


   // *********************************
   // load "lookahead_samples" value
   // *********************************
   r7 = M[r5 + $audio_proc.compander.LOOKAHEAD_SAMPLES];

   // *******************************************************************
   // extract the <size,base_address> for output cbuffer/frmbuffer
   // <size,base_address> are in <L1,B1> respectively
   // *******************************************************************
   r0 = M[r5 + $audio_proc.compander.CHANNEL_OUTPUT_PTR];
   r1 = M[r5 + $audio_proc.compander.FRAMEBUFFER_FLAG];
   call $audio_proc.cmpd.get_output_buffer_details;
   push B0;
   pop B1;
   L1 = L0;
   r0 = M[r5 + $audio_proc.compander.OUTPUT_WRITE_ADDR];
   I1 = r0;

   // *******************************************************************
   // extract the <size,base_address> for input cbuffer/frmbuffer
   // <size,base_address> are in <L0,B0> respectively
   // *******************************************************************
   r0 = M[r5 + $audio_proc.compander.CHANNEL_INPUT_PTR];
   r1 = M[r5 + $audio_proc.compander.FRAMEBUFFER_FLAG];
   call $audio_proc.cmpd.get_input_buffer_details;
   r0 = M[r5 + $audio_proc.compander.INPUT_READ_ADDR];
   I0 = r0;

   // ***************************************
   // load input lookahead history buffer
   // ***************************************
   r0 = M[r5 + $audio_proc.compander.START_LOOKAHEAD_HIST];
   push r0;
   pop B4;
   r0 = M[r5 + $audio_proc.compander.PTR_LOOKAHEAD_HIST];
   I4 = r0;
   r0 = r7 + 1;
   r0 = r0 * ADDR_PER_WORD(int);
   L4 = r0;

   // *********************************************************
   // read first sample from input frame and store in history
   // set modify registers
   // *********************************************************
   M0 = 0 * ADDR_PER_WORD;
   M2 = -1 * ADDR_PER_WORD;
   r0 = M[I0,M1];                 // read x(1)
   M[I4,M2] = r0;                 // store x(1) , I4 points to end of history buffer

   // ****************************
   // load "gain_update" rate
   // ****************************
   r10 = M[r5 + $audio_proc.compander.GAIN_UPDATE_RATE];

   // ****************************
   // check "gain_interp" flag
   // ****************************
   r0 = M[r5 + $audio_proc.compander.PARAM_PTR_FIELD];
   r0 = M[r0 + $audio_proc.compander.parameter.GAIN_INTERP_FLAG];
   if Z jump gain_interp_off_apply;

gain_interp_on_apply:
   r1 = 1;                                   // loop counter
   r9 = 1;                                   // increment value
   do gain_interp_on_apply_loop;
      r2 = r4 * r1(int) , rMAC = M[I0,M1];   // Gdelta*counter , load next input
      r2 = r2 + r3 , r0 = M[I4,M0];          // Gn = Gs_old + Gdelta*counter , load current_input from history
      r0 = r0 * r2(frac) , M[I4,M2] = rMAC;  // input*Gn , write input to history
      r0 = r0 ASHIFT 4;                      // arch3:q.19 -> q.23, arch4:q.27 -> q.31
      r1 = r1 + r9 , M[I1,M1] = r0;          // increment counter , write output
   gain_interp_on_apply_loop:
   jump gain_apply_done;

gain_interp_off_apply:
   do gain_interp_off_apply_loop;
      rMAC = M[I0,M1] , r0 = M[I4,M0];       // load next input , load current_input from history
      r1 = r0 * r4(frac);                    // input*Gn(Gs)
      r1 = r1 ASHIFT 4;                      // arch3:q.19 -> q.23, arch4:q.27 -> q.31
      M[I1,M1] = r1 , M[I4,M2] = rMAC;       // write output , write input to history
   gain_interp_off_apply_loop:

gain_apply_done:
   // **********************************************************
   // dummy read to set history buffer pointer for next frame
   // update history buffer pointer
   // **********************************************************
   r0 = M[I4,M1];
   r0 = I4;
   M[r5 + $audio_proc.compander.PTR_LOOKAHEAD_HIST] = r0;

   // **********************************
   // clear base and length registers
   // **********************************
#ifdef BASE_REGISTER_MODE
   push Null;
   pop B0;
   push Null;
   pop B1;
#endif
   push Null;
   pop B4;
   L0 = 0;
   L1 = 0;
   L4 = 0;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************************************
// MODULE:
//    $audio_proc.cmpd.get_input_buffer_details
//
// DESCRIPTION:
//   Extracts the <read_addr,size,base_addr> for the input cbuffer/framebuffer for the current channel
//
// INPUTS:
//    r0 = input cbuffer/framebuffer pointer
//    r1 = cbuffer/framebuffer flag(0/1)
//
// OUTPUTS:
//    - <r0,L0,B0> = <read_address,size,base_address> for the input cbuffer/framebuffer
//
// TRASHED REGISTERS:
//    - r1,r2
//
// NOTES
//
// *****************************************************************************************************
.MODULE $M.audio_proc.cmpd.get_input_buffer_details;
   .CODESEGMENT   PM;

$audio_proc.cmpd.get_input_buffer_details:
   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($audio_proc.COMPANDER_ASM.CMPD.GET_INPUT_BUFFER_DETAILS.PATCH_ID_0,r2)     // compander_patchers
//#endif



   Null = r1;
   if NZ jump cmpd_input_frame_buffer_details;
   // ************************************************************
   // get read pointer, size and start address of input cbuffer
   // ************************************************************
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_read_address_and_size_and_start_address;
   push r2;
   pop B0;
#else
    call $cbuffer.get_read_address_and_size;
#endif
   L0 = r1;
   jump cmpd_input_buffer_details_done;

cmpd_input_frame_buffer_details:
   // ****************************************************************
   // get read pointer, size and start address of input framebuffer
   // ****************************************************************
#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B0;
#else
   call $frmbuffer.get_buffer;
#endif
   L0 = r1;

cmpd_input_buffer_details_done:
   jump $pop_rLink_and_rts;

.ENDMODULE;


// ********************************************************************************************************
// MODULE:
//    $audio_proc.cmpd.get_output_buffer_details
//
// DESCRIPTION:
//    Extracts the <write_addr,size,base_addr> for the output cbuffer
//
// INPUTS:
//    r0 = output cbuffer pointer
//    r1 = cbuffer/framebuffer flag(0/1)
//
// OUTPUTS:
//    - <r0,L0,B0> = <write_address,size,base_address> for the output cbuffer/framebuffer
//
// TRASHED REGISTERS:
//    - r1,r2
//
// NOTES
//
// ********************************************************************************************************
.MODULE $M.audio_proc.cmpd.get_output_buffer_details;
   .CODESEGMENT   PM;

$audio_proc.cmpd.get_output_buffer_details:
   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($audio_proc.COMPANDER_ASM.CMPD.GET_OUTPUT_BUFFER_DETAILS.PATCH_ID_0,r2)     // compander_patchers
//#endif


   Null = r1;
   if NZ jump cmpd_output_frame_buffer_details;
   // ************************************************************
   // get write pointer, size and start address of output cbuffer
   // ************************************************************
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_write_address_and_size_and_start_address;
   push r2;
   pop B0;
#else
    call $cbuffer.get_write_address_and_size;
#endif
   L0 = r1;
   jump cmpd_output_buffer_details_done;

cmpd_output_frame_buffer_details:
   // ****************************************************************
   // get write pointer, size and start address of output framebuffer
   // ****************************************************************
#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B0;
#else
   call $frmbuffer.get_buffer;
#endif
   L0 = r1;

cmpd_output_buffer_details_done:
   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.cmpd.advance_pointers
//
// DESCRIPTION:
//    Advances the read/write pointers of the input/output respectively
//
// INPUTS:
//    r3 = pointer to the current channel data_object
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r4,I0,L0,B0,M0
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.cmpd.advance_pointers;
   .CODESEGMENT   PM;

$audio_proc.cmpd.advance_pointers:
   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.COMPANDER_ASM.CMPD.ADVANCE_POINTERS.PATCH_ID_0,r1)     // compander_patchers
//#endif


   // **********************
   // load the block_size
   // **********************
   r4 = M[r3 + $audio_proc.compander.GAIN_UPDATE_RATE];
   Words2Addr(r4);
   M0 = r4;

   // ***********************************************************
   // extract the <length,base_address> for the input buffer
   // advance the input read address by block_size
   // ***********************************************************
   r0 = M[r3 +  $audio_proc.compander.CHANNEL_INPUT_PTR];
   r1 = M[r3 + $audio_proc.compander.FRAMEBUFFER_FLAG];
   call $audio_proc.cmpd.get_input_buffer_details;
   r0 = M[r3 + $audio_proc.compander.INPUT_READ_ADDR];
   I0 = r0;
   r0 = M[I0,M0];
   r0 = I0;
   M[r3 + $audio_proc.compander.INPUT_READ_ADDR] = r0;

   // ***********************************************************
   // extract the <length,base_address> for the output buffer
   // advance the output write address by block_size
   // ***********************************************************
   r0 = M[r3 +  $audio_proc.compander.CHANNEL_OUTPUT_PTR];
   r1 = M[r3 + $audio_proc.compander.FRAMEBUFFER_FLAG];
   call $audio_proc.cmpd.get_output_buffer_details;
   r0 = M[r3 + $audio_proc.compander.OUTPUT_WRITE_ADDR];
   I0 = r0;
   r0 = M[I0,M0];
   r0 = I0;
   M[r3 + $audio_proc.compander.OUTPUT_WRITE_ADDR] = r0;

   // **********************************
   // clear base and length registers
   // **********************************
#ifdef BASE_REGISTER_MODE
   push Null;
   pop B0;
#endif
   L0 = 0;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// **************************************************************************************
// MODULE:
//    $audio_proc.cmpd.input_read_output_write_init
//
// DESCRIPTION:
//    Initialize the input read_address and output write_address for all the channels
//
// INPUTS:
//    r8   = pointer to the first channel data_object
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r10,I1,L0,B0
//
// NOTES
//
// **************************************************************************************
.MODULE $M.audio_proc.cmpd.input_read_output_write_init;
   .CODESEGMENT   PM;

$audio_proc.cmpd.input_read_output_write_init:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.COMPANDER_ASM.CMPD.INPUT_READ_OUTPUT_WRITE_INIT.PATCH_ID_0,r1)     // compander_patchers
//#endif


   r1 = M[r8 + $audio_proc.compander.DATA_OBJECTS_PTR];
   I1 = r1;                                                    // pointer to all data_objects
   r10 = M[r8 + $audio_proc.compander.NUM_CHANNELS];           // number of channels
   M1 = 1 * ADDR_PER_WORD;
   do input_read_output_write_init_loop;
      r3 = M[I1,M1];                                           // load current channel data_object
      r0 = M[r3 + $audio_proc.compander.CHANNEL_INPUT_PTR];    // load input cbuffer/frmbuffer pointer for current data_object
      r1 = M[r3 + $audio_proc.compander.FRAMEBUFFER_FLAG];     // load "framebuffer_flag"
      call $audio_proc.cmpd.get_input_buffer_details;
      M[r3 + $audio_proc.compander.INPUT_READ_ADDR] = r0;      // set input read_address
      r0 = M[r3 + $audio_proc.compander.CHANNEL_OUTPUT_PTR];   // load output cbuffer/frmbuffer pointer for current data_object
      r1 = M[r3 + $audio_proc.compander.FRAMEBUFFER_FLAG];     // load "framebuffer_flag"
      call $audio_proc.cmpd.get_output_buffer_details;
      M[r3 + $audio_proc.compander.OUTPUT_WRITE_ADDR] = r0;    // set output write_address
   input_read_output_write_init_loop:

   // **********************************
   // clear base and length registers
   // **********************************
#ifdef BASE_REGISTER_MODE
   push Null;
   pop B0;
#endif
   L0 = 0;

   jump $pop_rLink_and_rts;

.ENDMODULE;

// *****************************************************************************
// Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "core_library.h"
#include "vse.h"

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif


// *****************************************************************************
// MODULE:
//    $audio_proc.vse.initialize
//
// DESCRIPTION:
//    Initialize function for the VSE module
//
// INPUTS:
//    - r8 = pointer to the vse current_channel(left/right) data_object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r3,r4,r5,I1,I4
//
// NOTES:
// *****************************************************************************
.MODULE $M.audio_proc.vse.initialize;
   .CODESEGMENT   PM;

$audio_proc.vse.initialize:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.INITIALIZE.PATCH_ID_0,r1)     // VSE_patchers1
//#endif



   // ***************************************************
   // set the pointers to various filter coefficients
   // ***************************************************
   r0 = r8 + $audio_proc.vse.FILTER_COEFF_FIELD;
   M[r8 + $audio_proc.vse.IPSI_COEFF_PTR_FIELD] = r0;    // size 3 (total 3)
   r0 = r0 + ($BIN_SYNTH_FILTER_COEFF_SIZE * ADDR_PER_WORD);
   M[r8 + $audio_proc.vse.CONTRA_COEFF_PTR_FIELD] = r0;  // size 3 (total 6)
   r0 = r0 + ($BIN_SYNTH_FILTER_COEFF_SIZE * ADDR_PER_WORD);
   M[r8 + $audio_proc.vse.XTC_COEFF_PTR_FIELD] = r0;     // size 3 (total 9)
   r0 = r0 + ($BIN_SYNTH_FILTER_COEFF_SIZE * ADDR_PER_WORD);
   M[r8 + $audio_proc.vse.DCB_COEFF_PTR_FIELD] = r0;     // size 3 (total 12)
   r0 = r0 + ($BIN_SYNTH_FILTER_COEFF_SIZE * ADDR_PER_WORD);
   M[r8 + $audio_proc.vse.ITF_COEFF_PTR_FIELD] = r0;     // size 2 (total 14)
   r0 = r0 + ($ITF_COEFF_FILTER_SIZE * ADDR_PER_WORD);
   M[r8 + $audio_proc.vse.LSF_COEFF_PTR_FIELD] = r0;     // size 5 (total 19)
   r0 = r0 + ($LSF_PEAK_COEFF_FILTER_SIZE * ADDR_PER_WORD);
   M[r8 + $audio_proc.vse.PEAK_COEFF_PTR_FIELD] = r0;    // size 5 (total 24)

   // *******************************************************************************************
   // [Hvirt_ipsi_param, Hvirt_ipsi_buffer] = compute_virtual_filters(theta_virtual/2, fs, 0);
   // [Hvirt_contra_param, Hvirt_contra_buffer] = compute_virtual_filters(theta_virtual/2, fs, 1);
   // [Hxtc_eq_param, Hxtc_eq_buffer] = compute_xtc_filters(theta_speaker/2, fs, 1);
   // *******************************************************************************************
   r0 = M[r8 + $audio_proc.vse.FS];
   call $audio_proc.vse.sample_rate_index;       // r1 holds the index
   Words2Addr(r1);                               // arch4: rate index in addr

   r3 = M[r8 + $audio_proc.vse.PARAM_PTR_FIELD];

   r0 = M[r3 + $audio_proc.vse.parameter.VIRTUAL_ANGLE];
   call $audio_proc.vse.virtual_angle_index;     // r2 holds the index
   Words2Addr(r2);                               // arch4: angle index in addr

   r0 = M[r3 + $audio_proc.vse.parameter.SPEAKER_SPACING];
   call $audio_proc.vse.speaker_spacing_index;   // r5 holds the index
   Words2Addr(r5);                               // arch4: spacing index in addr

   r3 = $all_params;
   r3 = M[r3 + r1];         // index into the appropriate sample rate parameters

   // *******************************************************
   // copy the coefficient values to RAM for all filters
   // *******************************************************
   r4 = M[r3 + Null];       // pointer to ipsi_coefficients
   r4 = M[r4 + r2];         // ipsi_coefficients for theta_virtual value
   r0 = M[r8 + $audio_proc.vse.IPSI_COEFF_PTR_FIELD];
   r10 = 3;
   call copytoram;

   r4 = M[r3 + 1*ADDR_PER_WORD];  // pointer to contra_coefficients
   r4 = M[r4 + r2];               // contra_coefficients for theta_virtual value
   r0 = M[r8 + $audio_proc.vse.CONTRA_COEFF_PTR_FIELD];
   r10 = 3;
   call copytoram;

   r4 = M[r3 + 2*ADDR_PER_WORD];  // pointer to theta_speaker_coefficients
   r4 = M[r4 + r5];               // theta_speaker coefficients for speaker_spacing value
   r0 = M[r8 + $audio_proc.vse.XTC_COEFF_PTR_FIELD];
   r10 = 3;
   call copytoram;

   r4 = M[r3 + 4*ADDR_PER_WORD];  // pointer to dcb coeffs
   r0 = M[r8 + $audio_proc.vse.DCB_COEFF_PTR_FIELD];
   r10 = 3;
   call copytoram;

   r4 = M[r3 + 5*ADDR_PER_WORD];  // pointer to itf coeffs
   r0 = M[r8 + $audio_proc.vse.ITF_COEFF_PTR_FIELD];
   r10 = 2;
   call copytoram;

   r4 = M[r3 + 6*ADDR_PER_WORD];  // pointer to lsf coeffs
   r0 = M[r8 + $audio_proc.vse.LSF_COEFF_PTR_FIELD];
   r10 = 5;
   call copytoram;

   r4 = M[r3 + 7*ADDR_PER_WORD];  // pointer to peak coeffs
   r0 = M[r8 + $audio_proc.vse.PEAK_COEFF_PTR_FIELD];
   r10 = 5;
   call copytoram;

   // ***********************
   // set the delay value
   // ***********************
   r4 = M[r3 + 3*ADDR_PER_WORD];  // pointer to delay values
   r4 = M[r4 + r2];               // delay value for theta_virtual value
   M[r8 + $audio_proc.vse.DELAY_FIELD]= r4;

   // *****************************************
   // initialize the history buffer pointers
   // *****************************************
   r0 = r8 + $audio_proc.vse.START_HISTORY_BUF_Hc;
   M[r8 + $audio_proc.vse.PTR_HISTORY_BUF_Hc] = r0;
   r0 = r8 + $audio_proc.vse.START_HISTORY_BUF_PEAK;
   M[r8 + $audio_proc.vse.PTR_HISTORY_BUF_PEAK] = r0;
   r0 = r8 + $audio_proc.vse.START_HISTORY_BUF_LSF;
   M[r8 + $audio_proc.vse.PTR_HISTORY_BUF_LSF] = r0;

   // *****************************************
   // call the limiter initialize function
   // *****************************************
   call $audio_proc.vse.limiter.initialize;

   // *****************************************************************
   // Initialize "framebuffer_flag" for the current data_object
   // *****************************************************************
   M[r8 + $audio_proc.vse.FRAMEBUFFER_FLAG] = Null;

   jump $pop_rLink_and_rts;

  // *************************************************************************
  // Description: Subroutine to copy coefficient tables from DMCONST to RAM
  // r0  = RAM ADDRESS
  // r4  = DMCONST ADDRESS
  // r10 = number of coefficients to copy
  // *************************************************************************
copytoram:
   I4 = r0;
   I1 = r4;
   r0 = M[I1,MK1];
   do copy_vsecoeffs_to_ram;
      r0 = M[I1,MK1] , M[I4,MK1] = r0;
   copy_vsecoeffs_to_ram:

   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.vse.sample_rate_index
//
// DESCRIPTION:
//   Returns the array index of the coefficients buffer
//   for the input sample rate
//
// INPUTS:
//    - r0 = sampling rate
//
// OUTPUTS:
//    - r1 = index into the coefficients buffer
//
// TRASHED REGISTERS:
//    - None
//
// NOTES:
// *****************************************************************************
.MODULE $M.audio_proc.vse.sample_rate_index;
   .CODESEGMENT   PM;

$audio_proc.vse.sample_rate_index:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.SAMPLE_RATE_INDEX.PATCH_ID_0,r1)     // VSE_patchers1
//#endif



sample_rate_32k_check:
   Null = r0 - 32000;
   if NZ jump sample_rate_44k_check;
   r1 = 0;
   jump sample_rate_set;

sample_rate_44k_check:
   Null = r0 - 44100;
   if NZ jump sample_rate_48k_check;
   r1 = 1;
   jump sample_rate_set;

sample_rate_48k_check:
   Null = r0 - 48000;
   if NZ jump sample_rate_88k_check;
   r1 = 2;
   jump sample_rate_set;

sample_rate_88k_check:
   Null = r0 - 88200;
   if NZ jump sample_rate_96k_check;
   r1 = 3;
   jump sample_rate_set;

sample_rate_96k_check:
   Null = r0 - 96000;
   if NZ call $error;
   r1 = 4;

sample_rate_set:

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.vse.virtual_angle_index
//
// DESCRIPTION:
//   Returns the array index of the coefficients buffer
//   for the input virtual angle
//
// INPUTS:
//    - r0 = virtual angle
//
// OUTPUTS:
//    - r2 = index into the buffer
//
// TRASHED REGISTERS:
//    - None
//
// NOTES:
// *****************************************************************************
.MODULE $M.audio_proc.vse.virtual_angle_index;
   .CODESEGMENT   PM;

$audio_proc.vse.virtual_angle_index:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.VIRTUAL_ANGLE_INDEX.PATCH_ID_0,r2)     // VSE_patchers1
//#endif


virtual_angle_60_check:
   Null = r0 - 60;
   if NZ jump virtual_angle_120_check;
   r2 = 0;
   jump virtual_angle_set;

virtual_angle_120_check:
   Null = r0 - 120;
   if NZ jump virtual_angle_180_check;
   r2 = 1;
   jump virtual_angle_set;

virtual_angle_180_check:
   r2 = 2;

virtual_angle_set:

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.vse.speaker_spacing_index
//
// DESCRIPTION:
//   Returns the array index of the coefficients buffer
//   for the speaker spacing
//
// INPUTS:
//    - r0 = speaker spacing
//
// OUTPUTS:
//    - r5 = index into the buffer
//
// TRASHED REGISTERS:
//    -None
//
// NOTES:
// *****************************************************************************
.MODULE $M.audio_proc.vse.speaker_spacing_index;
   .CODESEGMENT   PM;

$audio_proc.vse.speaker_spacing_index:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.SPEAKER_SPACING_INDEX.PATCH_ID_0,r5)     // VSE_patchers1
//#endif


   Null = r0 - $VSE_SPEAKER_SPACING_5CM;
   if NZ jump spkr_space_10cm_check;
   r5 = 0;
   jump spkr_spacing_set;

spkr_space_10cm_check:
   Null = r0 - $VSE_SPEAKER_SPACING_10CM;
   if NZ jump spkr_space_12cm_check;
   r5 = 1;
   jump spkr_spacing_set;

spkr_space_12cm_check:
   Null = r0 - $VSE_SPEAKER_SPACING_15CM;
   if NZ jump spkr_space_20cm_check;
   r5 = 2;
   jump spkr_spacing_set;

spkr_space_20cm_check:
   r5 = 3;

spkr_spacing_set:

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.vse.limiter.initialize
//
// DESCRIPTION:
//   Initialize function for the limiter
//
// INPUTS:
//    - r8 = pointer to vse data object
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    - r1,r2,r3,r4
//
// NOTES:
// *****************************************************************************
.MODULE $M.audio_proc.vse.limiter.initialize;
   .CODESEGMENT   PM;

$audio_proc.vse.limiter.initialize:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.LIMITER.INITIALIZE.PATCH_ID_0,r1)     // VSE_patchers1
//#endif


   r0 = M[r8 + $audio_proc.vse.FS];

rate_44k_check:
   Null = r0 - 44100;
   if NZ jump rate_non_44k;
   r1 = $LIMIT_ATTACK_TC_44K;
   r2 = $LIMIT_DECAY_TC_44K;
   r3 = $LINEAR_ATTACK_TC_44K;
   r4 = $LINEAR_DECAY_TC_44K;
   jump rate_set;

rate_non_44k:
   r1 = $LIMIT_ATTACK_TC_NON_44K;
   r2 = $LIMIT_DECAY_TC_NON_44K;
   r3 = $LINEAR_ATTACK_TC_NON_44K;
   r4 = $LINEAR_DECAY_TC_NON_44K;

rate_set:
   M[r8 + $audio_proc.vse.LIMIT_ATTACK_TC]  =  r1;
   M[r8 + $audio_proc.vse.LIMIT_DECAY_TC]   =  r2;
   M[r8 + $audio_proc.vse.LINEAR_ATTACK_TC] =  r3;
   M[r8 + $audio_proc.vse.LINEAR_DECAY_TC]  =  r4;

   r1 = $LIMIT_THRESHOLD;
   M[r8 + $audio_proc.vse.LIMIT_THRESHOLD] = r1;

   r1 = $MAKEUPGAIN;
   M[r8 + $audio_proc.vse.MAKEUPGAIN] = r1;

   r1 = 0;
   M[r8 + $audio_proc.vse.LIMITER_GAIN_LOG] = r1;

   jump $pop_rLink_and_rts;

.ENDMODULE;

// **************************************************************************************
// MODULE:
//    $audio_proc.vse.process
//
// DESCRIPTION:
//   This function does the 3D processing on the input channels
//   $audio_proc.vse.frame_process  : Entry point for frame_sync API
//   $audio_proc.vse.stream_process : Entry point for Kymera API
//
// INPUTS:
//    - r7 = pointer to vse left_data_object
//    - r8 = pointer to vse right_data_object
//
// OUTPUTS:
//    -None
//
// TRASHED REGISTERS:
//    - Everything except r7,r8
//
// NOTES:
// **************************************************************************************
.MODULE $M.audio_proc.vse.process;
   .CODESEGMENT   PM;

$audio_proc.vse.frame_process:

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.PROCESS.PATCH_ID_0,r3)     // VSE_patchers1
#endif


   // *******************************************************************************************************
   // Sets "samples_to_process" and "input_framebuffer_flag" value for the vse left/right data objects.
   // Falls through to the $audio_proc.vse.stream_process() function
   // *******************************************************************************************************

set_buffer_left_data_object:
   // *******************************************************
   // Set "samples_to_process" for the left_data_object
   // *******************************************************
   r0 = M[r7 + $audio_proc.vse.INPUT_ADDR_FIELD];
   r3 = M[r0 + $frmbuffer.FRAME_SIZE_FIELD];
   M[r7 + $audio_proc.vse.SAMPLES_TO_PROCESS] = r3;

   // *******************************************************
   // Set "framebuffer_flag" for the left_data_object
   // *******************************************************
   r0 = 1;
   M[r7 + $audio_proc.vse.FRAMEBUFFER_FLAG] = r0;

set_buffer_right_data_object:
   // *******************************************************
   // Set "samples_to_process" for the right_data_object
   // *******************************************************
   r0 = M[r8 + $audio_proc.vse.INPUT_ADDR_FIELD];
   r3 = M[r0 + $frmbuffer.FRAME_SIZE_FIELD];
   M[r8 + $audio_proc.vse.SAMPLES_TO_PROCESS] = r3;

   // *******************************************************
   // Set "framebuffer_flag" for the right_data_object
   // *******************************************************
   r0 = 1;
   M[r8 + $audio_proc.vse.FRAMEBUFFER_FLAG] = r0;


$audio_proc.vse.stream_process:

   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.PROCESS.PATCH_ID_1,r3)     // VSE_patchers1
#endif


   // *********************************************************************
   // initialize input_read_addr/output_write_addr for the left_data_object
   // *********************************************************************
   push r8;
   r8 = r7;
   call $audio_proc.vse.get_input_buffer_details;
   M[r7 + $audio_proc.vse.INPUT_READ_ADDR] = r3;
   call $audio_proc.vse.get_output_buffer_details;
   M[r7 + $audio_proc.vse.OUTPUT_WRITE_ADDR] = r3;
   pop r8;

   // *********************************************************************
   // initialize input_read_addr/output_write_addr for the right_data_object
   // *********************************************************************
   call $audio_proc.vse.get_input_buffer_details;
   M[r8 + $audio_proc.vse.INPUT_READ_ADDR] = r3;
   call $audio_proc.vse.get_output_buffer_details;
   M[r8 + $audio_proc.vse.OUTPUT_WRITE_ADDR] = r3;

   // *****************************************************
   // load "samples_to_process" and "vse_frame_size"
   // ****************************************************
   r3 = M[r7 + $audio_proc.vse.SAMPLES_TO_PROCESS];
   r4 = $VSE_FRAME_SIZE;

vse_frame_proc_loop:
   Null = r3 - r4;
   if LE r4 = r3;
   pushm <r3,r4,r8>;
   call $audio_proc.vse.process_channels;
   popm <r3,r4,r8>;
   r3 = r3 - r4;
   if Z jump vse_process_done;
   jump vse_frame_proc_loop;

vse_process_done:
   jump $pop_rLink_and_rts;


.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.vse.process_channels
//
// DESCRIPTION:
//   This function calls a series of filters operating on the input channels
//
// INPUTS:
//    - r7 = pointer to vse left_data_object
//    - r8 = pointer to vse right_data_object
//    - r4 = current vse_block_size
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - Everything except r7
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.vse.process_channels;
   .CODESEGMENT   PM;

$audio_proc.vse.process_channels:

   push rLink;

   // ***************************************************************
   // set "vse_cur_block_size" for the left and right channels
   // **************************************************************
   M[r7 + $audio_proc.vse.VSE_CUR_BLOCK_SIZE] = r4;
   M[r8 + $audio_proc.vse.VSE_CUR_BLOCK_SIZE] = r4;

   // ************************************************
   // load the input_read_addr for the right_channel
   // set the size/base_address for the buffer
   // ************************************************
   r0 = M[r8 + $audio_proc.vse.INPUT_READ_ADDR];
   I5 = r0;
   call $audio_proc.vse.get_input_buffer_details;
#ifdef BASE_REGISTER_MODE
   push B0;
   pop B5;
#endif
   L5 = L0;

   // ************************************************
   // load the input_read_addr for the left_channel
   // set the size/base_address for the buffer
   // ************************************************
   r0 = M[r7 + $audio_proc.vse.INPUT_READ_ADDR];
   I0 = r0;
   push r8;
   r8 = r7;
   call $audio_proc.vse.get_input_buffer_details;
   pop r8;

   // *****************************************************
   // Bypass "Binaural_Synthesis" for binaural recording
   // *****************************************************
   r0 = M[r7 + $audio_proc.vse.PARAM_PTR_FIELD];
   r1 = M[r0 + $audio_proc.vse.parameter.BINAURAL_FLAG];
   if NZ jump binaural_scaling;


   // ***********************************************
   //         BINAURAL SYNTHESIS
   // ***********************************************

   // ******************************************
   // Binaural synthesis for the left channel
   // ******************************************
   call $audio_proc.vse.binaural_synthesis_filters;

   // ******************************************
   // Binaural synthesis for the right channel
   // ******************************************
   I0 = I5;
#ifdef BASE_REGISTER_MODE
   push B5;
   pop B0;
#endif
   push r7;
   r7 = r8;
   call $audio_proc.vse.binaural_synthesis_filters;
   pop r7;

   // *****************************************************************************************************
   // Obuf_BL = Obuf_Lipsi + Obuf_Rcontra;
   // Obuf_BL = Obuf_BL/2;
   // Obuf_BR = Obuf_Ripsi + Obuf_Lcontra;
   // Obuf_BR = Obuf_BR/2;
   // Note : Obuf_BL & Obuf_BR are scaled down by a factor of 4 in DSP to prevent saturation issues
   // *****************************************************************************************************
   r0 = M[r7 + $audio_proc.vse.out_ipsi];
   I2 = r0;                                             // pointer to left_ipsi buffer
   r0 = M[r7 + $audio_proc.vse.out_contra];
   I6 = r0;                                             // pointer to left_contra buffer
   r0 = M[r8 + $audio_proc.vse.out_ipsi];
   I7 = r0;                                             // pointer to right_ipsi buffer
   r0 = M[r8 + $audio_proc.vse.out_contra];
   I4 = r0;                                             // pointer to right_contra buffer
   r10 = M[r8 + $audio_proc.vse.VSE_CUR_BLOCK_SIZE];
   r0 = -1.0;
   do binaural_synth_output;
      rMAC = M[I2,0] , r1 = M[I4,MK1];
      rMAC = rMAC - r0*  r1 , r3 = M[I6,MK1];
      rMAC = rMAC * 0.25;
      M[I2,MK1] = rMAC , rMAC = M[I7,0] ;
      rMAC = rMAC - r0 * r3  ;
      rMAC = rMAC * 0.25;
      M[I7,MK1] = rMAC ;
   binaural_synth_output:
   jump binaural_done;

binaural_scaling:
   r0 = M[r7 + $audio_proc.vse.out_ipsi];
   I1 = r0;                                             // pointer to left_ipsi buffer
   r0 = M[r8 + $audio_proc.vse.out_ipsi];
   I4 = r0;                                             // pointer to right_ipsi buffer
   r10 = M[r7 + $audio_proc.vse.VSE_CUR_BLOCK_SIZE];
   do binaural_scaling_loop;
      r0 = M[I0,MK1] , r1 = M[I5,MK1];
      r0 = r0 * 0.25;
      r1 = r1 * 0.25;
      M[I1,MK1] = r0 , M[I4,MK1] = r1;
   binaural_scaling_loop:

binaural_done:
   // **************************************************************
   // clear the <B0,B5> base registers under "BASE_REGISTER_MODE"
   // clear <L0,L5> length registers
   // **************************************************************
#ifdef BASE_REGISTER_MODE
   push Null;
   pop B0;
   push Null;
   pop B5;
#endif
   L0 = 0;
   L5 = 0;

   // ****************************************************************
   // Cross-talk cancellation and DCB filtering for the left channel
   // ****************************************************************
   call $audio_proc.vse.xtc_dcb_filters;

   // ****************************************************************
   // Cross-talk cancellation and DCB filtering for the right channel
   // ****************************************************************
   push r7;
   r7 = r8;
   call $audio_proc.vse.xtc_dcb_filters;
   pop r7;

   // **************************************************************************
   // ITF filter processing (done for both L & R channels)
   // Obuf_xtcL(t) = Obuf_xtc_dcbL(t) - ITF_bufR(1,2); % yL(n) = xL(n) - eR(n)
   // **************************************************************************
   call $audio_proc.vse.ITF_process;

   // *********************************************
   // Peak and LSF filtering for the left channel
   // *********************************************
   push r8;
   call $audio_proc.vse.peak_lsf_filters;
   pop r8;

   // *********************************************
   // Peak and LSF filtering for the right channel
   // *********************************************
   pushm <r7,r8>;
   r7 = r8;
   call $audio_proc.vse.peak_lsf_filters;
   popm <r7,r8>;

   // *****************************************
   // scale the values back up before limiter
   // *****************************************
   r0 = M[r7 + $audio_proc.vse.out_ipsi];
   I0 = r0;
   r0 = M[r8 + $audio_proc.vse.out_ipsi];
   I4 = r0;
   r4 = 1;
   M0 = 0;
   M1 = MK1;
   r10 = M[r7 + $audio_proc.vse.VSE_CUR_BLOCK_SIZE];
   r0 = M[I0,M0];                                  // preload left_ch
   do scale_up_values;
      r0 = r0 ASHIFT r4 , r1 = M[I4,M0];           // shift left_ch  , load right_ch
      r1 = r1 ASHIFT r4 , M[I0,M1] = r0;           // shift right_ch , write left_ch
      r0 = M[I0,M0] , M[I4,M1] = r1;               // load left_ch , write right_ch
   scale_up_values:

   // *********************************************
   // Call the limiter function
   // *********************************************
   call $audio_proc.vse.limiter.process;

   // ************************************************
   // load the output_write_addr for the right_channel
   // set the size/base_address for the buffer
   // ************************************************
   r0 = M[r8 + $audio_proc.vse.OUTPUT_WRITE_ADDR];
   I5 = r0;
   call $audio_proc.vse.get_output_buffer_details;
#ifdef BASE_REGISTER_MODE
   push B0;
   pop B5;
#endif
   L5 = L0;

   // ************************************************
   // load the output_write_addr for the left_channel
   // set the size/base_address for the buffer
   // ************************************************
   push r8;
   r8 = r7;
   r0 = M[r8 + $audio_proc.vse.OUTPUT_WRITE_ADDR];
   I0 = r0;
   call $audio_proc.vse.get_output_buffer_details;
   pop r8;

   // **************************
   // Apply the limiter gain
   // **************************
   r0 = M[r7 + $audio_proc.vse.out_ipsi];
   I1 = r0;
   r0 = M[r8 + $audio_proc.vse.out_ipsi];
   I4 = r0;
   r10 = M[r7 + $audio_proc.vse.VSE_CUR_BLOCK_SIZE];
   r3 = M[r7 + $audio_proc.vse.LIMITER_GAIN];
   r4 = M[r7 + $audio_proc.vse.MAKEUPGAIN];
   r3 = r3 * r4(frac) , r0 = M[I1,MK1];                 // calculate Gain , preload left channel
   do apply_limiter_gain_loop;
      r0 = r0 * r3(frac) , r1 = M[I4,MK1];
      r1 = r1 * r3(frac) , M[I0,MK1] = r0;
      M[I5,MK1] = r1 , r0 = M[I1,MK1];
   apply_limiter_gain_loop:

   // ************************************************************************
   // advance the input/output cbuffer pointers for the left/right channels
   // ************************************************************************
   call $audio_proc.vse.advance_pointers;
   r8 = r7;
   call $audio_proc.vse.advance_pointers;

   // ********************************************************************
   // clear the input/output base/length registers used in the vse module
   // ********************************************************************
#ifdef BASE_REGISTER_MODE
   push Null;
   pop B0;
   push Null;
   pop B5;
#endif

   L0 = 0;
   L5 = 0;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// ******************************************************************************************************
// MODULE:
//    $audio_proc.vse.xtc_dcb_filters
//
// DESCRIPTION:
//  This function calls the cross-talk cancellation and dcb filters for the current channel(left/right)
//
// INPUTS:
//    - r7 = pointer to vse_data_object(left/right)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r3,r4,r6,r9,r10,rMAC,M1,M2,I0,I1,I2,I3
//
// NOTES
//
// ******************************************************************************************************

.MODULE $M.audio_proc.vse.xtc_dcb_filters;
   .CODESEGMENT   PM;

$audio_proc.vse.xtc_dcb_filters:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.XTC_DCB_FILTERS.PATCH_ID_0,r1)     // VSE_patchers1
//#endif


   // ****************************************************************************************************************
   // Cross-talk cancellation for the current channel
   // [Obuf_xtc_eq, Hxtc_eq_buffer(:,1)] = filter(Hxtc_eq_param.B, Hxtc_eq_param.A, Obuf_B, Hxtc_eq_buffer(:,1));
   // ****************************************************************************************************************
   r0 = M[r7 + $audio_proc.vse.XTC_COEFF_PTR_FIELD];
   I3 = r0;
   I2 = r7 + $audio_proc.vse.START_HISTORY_BUF_EQ;
   r0 = M[r7 + $audio_proc.vse.out_ipsi];
   I0 = r0;
   I1 = I0;                                          // in-place processing
   call $audio_proc.vse.filter_process;              // cross-talk cancel ipsieq filter

   // ************************************************************************************************************************
   // DCB filter for the current channel
   // [Obuf_xtc_dcb, Hxtc_dcb_buffer(:,1)] = filter(Hxtc_dcb_param.B, Hxtc_dcb_param.A, Obuf_xtc_eq, Hxtc_dcb_buffer(:,1));
   // ************************************************************************************************************************
   r0 = M[r7 + $audio_proc.vse.DCB_COEFF_PTR_FIELD];
   I3 = r0;
   I2 = r7 + $audio_proc.vse.START_HISTORY_BUF_DCB;
   r0 = M[r7 + $audio_proc.vse.out_ipsi];
   I0 = r0;
   I1 = I0;                                          // in-place processing
   call $audio_proc.vse.filter_process;              // DCB filter

   jump $pop_rLink_and_rts;


.ENDMODULE;


// ******************************************************************************************************
// MODULE:
//    $audio_proc.vse.peak_lsf_filters
//
// DESCRIPTION:
//  This function calls the peak and lsf filters for the current channel(left/right)
//
// INPUTS:
//    - r7 = pointer to vse_data_object(left/right)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - Everything except r7
//
// NOTES
//
// ******************************************************************************************************

.MODULE $M.audio_proc.vse.peak_lsf_filters;
   .CODESEGMENT   PM;

$audio_proc.vse.peak_lsf_filters:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.PEAK_LSF_FILTERS.PATCH_ID_0,r5)     // VSE_patchers1
//#endif


   // ************************************************************************************************************************
   // Peak filter for the current channel
   // [Obuf_xtc_peak, Hxtc_peak_buffer(:,1)] = filter(Hxtc_peak_param.B, Hxtc_peak_param.A, Obuf_xtc, Hxtc_peak_buffer(:,1));
   // ************************************************************************************************************************
   r0 = M[r7 + $audio_proc.vse.out_ipsi];
   I0 = r0;
   I5 = r0;
   r0 = M[r7 + $audio_proc.vse.PEAK_COEFF_PTR_FIELD];
   I3 = r0;
   r5 = 0;
   call $audio_proc.vse.filter2_process;            // Peak filter

   // ************************************************************************************************************************
   // LSF filter for the current channel
   // [Obuf_xtc_lsf, Hxtc_lsf_buffer(:,1)] = filter(Hxtc_lsf_param.B, Hxtc_lsf_param.A, Obuf_xtc_peak, Hxtc_lsf_buffer(:,1));
   // ************************************************************************************************************************
   r0 = M[r7 + $audio_proc.vse.out_ipsi];
   I0 = r0;
   I5 = r0;
   r0 = M[r7 + $audio_proc.vse.LSF_COEFF_PTR_FIELD];
   I3 = r0;
   r5 = 1;
   call $audio_proc.vse.filter2_process;            // LSF filter


   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************************************************************************
// MODULE:
//    $audio_proc.vse.binaural_synthesis_filters
//
// DESCRIPTION:
//  This function performs binaural synthesis by calling the ipsilateral and contralateral filters for the current channel(left/right)
//
// INPUTS:
//    - r7 = pointer to vse_data_object(left/right)
//    - I0 = pointer to input buffer for the current channel(left/right)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r3,r4,r5,r6,r9,r10,rMAC,M0,M1,M2,I1,I2,I3,L0
//
// NOTES
//
// *****************************************************************************************************************************************

.MODULE $M.audio_proc.vse.binaural_synthesis_filters;
   .CODESEGMENT   PM;

$audio_proc.vse.binaural_synthesis_filters:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.BINAURAL_SYNTHESIS_FILTERS.PATCH_ID_0,r1)     // VSE_patchers1
//#endif


   // *************************************************************************************************************************
   // [Obuf_ipsi, Hvirt_ipsi_buffer(:,1)] = filter(Hvirt_ipsi_param.B, Hvirt_ipsi_param.A, Ibuf_B, Hvirt_ipsi_buffer(:,1));
   // *************************************************************************************************************************
   r0 = M[r7 + $audio_proc.vse.IPSI_COEFF_PTR_FIELD];
   I3 = r0;                                             // pointer to coeff buffer
   r0 = M[r7 + $audio_proc.vse.out_ipsi];
   I1 = r0;                                             // pointer to ipsi(output) buffer
   I2 = r7 + $audio_proc.vse.START_HISTORY_BUF_Hi;      // pointer to history buffer
   push I0;
   call $audio_proc.vse.filter_process;                 // ipsilateral filter for current channel
   pop I0;

   // ************************************************************************************************************************************
   // [Obuf_contra, Hvirt_contra_buffer(:,1)] = filter(Hvirt_contra_param.B, Hvirt_contra_param.A, Ibuf_B, Hvirt_contra_buffer(:,1));
   // ************************************************************************************************************************************
   r0 = M[r7 + $audio_proc.vse.out_contra];
   I2 = r0;                                             // pointer to contra(output) buffer
   r0 = M[r7 + $audio_proc.vse.CONTRA_COEFF_PTR_FIELD];
   I3 = r0;                                             // pointer to coeff buffer
   call $audio_proc.vse.hc_process;                     // contralateral filter for current channel

   jump $pop_rLink_and_rts;

.ENDMODULE;


// **************************************************************************************************
// MODULE:
//    $audio_proc.vse.filter_process
//
// DESCRIPTION:
//  This function implements a filter commonly used by :
//  DCB(Hdcb) / Cross-talk Ipsilateral Equalization filter(Heq) / Binaural Synthesis Ipsilateral filter(Hi)
//  The difference equation for the filter is : y(n) = b0 * x(n) + b1 * x(n-1) - a1 * y(n-1)
//
// INPUTS:
//    - r7 = pointer to vse_data_object(left/right)
//    - I0 = Input buffer
//    - I1 = Output buffer
//    - I3 = Coefficient ptr
//    - I2 = History ptr
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r3,r4,r6,r9,r10,rMAC,M1,M2
//
// NOTES
//
// **************************************************************************************************

.MODULE $M.audio_proc.vse.filter_process;
   .CODESEGMENT   PM;

$audio_proc.vse.filter_process:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.FILTER_PROCESS.PATCH_ID_0,r1)     // VSE_patchers1
//#endif


   M1 = 1*ADDR_PER_WORD;
   M2 = -1*ADDR_PER_WORD;
   // ******************************************************
   // load the filter coefficient and loop counter
   // ******************************************************
   r2 = M[I3,M1];                // load b0
   r1 = M[I3,M1];                // load b1
   r6 = r1 , r3 = M[I3,M1];      // set r6 to b1 , load a1
   r10 = M[r7 + $audio_proc.vse.VSE_CUR_BLOCK_SIZE];
   r9 = DAWTH;

   // ***************************
   // load the history values
   // ***************************
   r0 = M[I2,M1];                                     // r0 = y_l'
   r1 = M[I2,M1];                                     // r1 = y_h'
   r4 = M[I2,0];                                      // r4 = x_in'

   do filter_loop;
      rMAC = r3 * r0 (SU);                            // y = r3 * y_l'
      rMAC = rMAC ASHIFT -DAWTH (56bit) ;             // shift the computed value to the lower 24 bits
      rMAC = rMAC + r4 * r6;                          // y = r6 * x_in' - r3 * y_l' ,r4 = x_in
      rMAC = rMAC + r3 * r1 , r4 = M[I0,M1];          // y = r6 * x_in' - r3 * (y_l' +  y_h')
      rMAC = rMAC + r4 * r2;                          // y = r2 * x_in+ r6 * x_in' - r3 * (y_l' +  y_h')
      r1 = rMAC ASHIFT Null;                          // y_h_out
      r0 = rMAC LSHIFT r9 , M[I1,M1] = r1;            // y_l_out , write heq_out(y_h_out)
   filter_loop:

   M[I2,M2] = r4;                                     // store x_in'
   M[I2,M2] = r1;                                     // store y_h'
   M[I2,0]  = r0;                                     // store y_l'

   jump $pop_rLink_and_rts;

.ENDMODULE;


// ***********************************************************************************************
// MODULE:
//    $audio_proc.vse.ITF_process
//
// DESCRIPTION:
//  This function implements Cross Talk inter-aural filter(low pass)
//  The difference equation for the filters is as follows :  hitf_out = y = b0*x(n) - a1*y(n-1)
//
//  INPUTS:
//    - r7 = pointer to vse left_data object
//    - r8 = pointer to vse right_data object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r3,r4,r5,r6,r9,r10,M1,rMAC,I0,I1,I3,I4,I5
//
// NOTES
//
// ***********************************************************************************************

.MODULE $M.audio_proc.vse.ITF_process;
   .CODESEGMENT   PM;

$audio_proc.vse.ITF_process:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.ITF_PROCESS.PATCH_ID_0,r1)     // VSE_patchers1
//#endif



   // ******************************************************
   // load the input/output buffers for the left channel
   // output_buf = input_buf (in-place processing)
   // ******************************************************
   r0 = M[r7 + $audio_proc.vse.out_ipsi];
   I0 = r0;
   I1 = I0;

   // ******************************************************
   // load the input/output buffers for the right channel
   // output_buf = input_buf (in-place processing)
   // ******************************************************
   r0 = M[r8 + $audio_proc.vse.out_ipsi];
   I5 = r0;
   I4 = r0;

   // *****************************************
   // load the coefficients buffer pointer
   // *****************************************
   r0 = M[r7 + $audio_proc.vse.ITF_COEFF_PTR_FIELD];
   I3 = r0;

   // ******************************************************
   // load the Hi filter coefficient and loop counter
   // ******************************************************
   r2 = M[I3,MK1];                                        // load b0
   r6 = r2;                                               // set r6 to b0
   r3 = M[I3,-MK1];                                       // load a1
   r9 = r3;                                               // set r9 to a1
   r10 = M[r7 + $audio_proc.vse.VSE_CUR_BLOCK_SIZE];
   M1 = 1*ADDR_PER_WORD;
   r0 = M[r7 + $audio_proc.vse.HiftOUT_L_1];              // r0 = y_l' left ITF
   r1 = M[r7 + $audio_proc.vse.HiftOUT_H_1];              // r1 = y_h' left ITF
   r2 = M[r8 + $audio_proc.vse.HiftOUT_L_1];              // r0 = y_l' right ITF
   r3 = M[r8 + $audio_proc.vse.HiftOUT_H_1];              // r1 = y_h' right ITF
   push r7;
   r4 = M[I0,M1] , r5 = M[I5,M1];                         // r4 = left_dcb_input, r5 = right_dcb_input
   r7 = -1.0;
   do hitf_loop;
      rMAC = r5;
      rMAC = rMAC + r7 * r1;
      r5 = rMAC;                                          // right_dcb_input - itf_left
      rMAC = r4;
      rMAC = rMAC + r7 * r3 , M[I4,M1] = r5;
      r4 = rMAC;                                          // left_dcb_input  - itf_right
      rMAC = r9 * r0 (SU) , M[I1,M1] = r4;                // y = a1 * y_l'
      rMAC = rMAC ASHIFT -DAWTH (56bit);                  // shift the computed value to the lower 24 bits
      rMAC = rMAC + r4 * r6;                              // y = b0 * xin + a1 * y_l'
      rMAC = rMAC + r9 * r1 , r4 = M[I0,M1];              // y = b0 * xin  + a1 * (y_l' + y_h')
      r1 = rMAC ASHIFT Null;                              // y_h_out(left)
      r0 = rMAC LSHIFT DAWTH;                             // y_l_out(left)
      rMAC = r9 * r2 (SU);                                // y = a1 * y_l' (r3 to be stored as -ve)
      rMAC = rMAC ASHIFT -DAWTH (56bit);                  // shift the computed value to the lower 24 bits
      rMAC = rMAC + r5 * r6;                              // y = b0 * xin + a1 * y_l'
      rMAC = rMAC + r9 * r3 , r5 = M[I5,M1];              // y = b0 * xin  + a1 * (y_l' + y_h')
      r3 = rMAC ASHIFT Null;                              // y_h_out(right)
      r2 = rMAC LSHIFT DAWTH;                             // y_l_out(right)
   hitf_loop:
   pop r7;
   M[r7 + $audio_proc.vse.HiftOUT_L_1]  = r0;             // store (y_l_out)
   M[r7 + $audio_proc.vse.HiftOUT_H_1]  = r1;             // store (y_h_out)
   M[r8 + $audio_proc.vse.HiftOUT_L_1]  = r2;             // store (y_l_out)
   M[r8 + $audio_proc.vse.HiftOUT_H_1]  = r3;             // store (y_h_out)

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.vse.hc_process
//
// DESCRIPTION:
//  This function implements Binaural synthesis contralateral filter
//  The difference equation for the filters is as follows :
//  hi_out = y = b0*x(n-d) + b1*x(n-d-1) - a1*y(n-1) // d is delay

//
//  INPUTS:
//    - r7 = pointer to vse data object
//    - I0 = pointer to input buffer
//    - I2 = pointer to output buffer
//    - I3 = coefficient pointer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r3,r4,r5,r6,r9,M0,M1,M2
//
// NOTES
//
// *****************************************************************************

.MODULE $M.audio_proc.vse.hc_process;
   .CODESEGMENT   PM;

$audio_proc.vse.hc_process:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.HC_PROCESS.PATCH_ID_0,r1)     // VSE_patchers1
//#endif


   // *************************************************
   // Setup I1/B1/L1 for history buffer access
   // *************************************************
   r0 = r7 + $audio_proc.vse.START_HISTORY_BUF_Hc;
   push r0;
   pop B1;
   r0 = M[r7 + $audio_proc.vse.PTR_HISTORY_BUF_Hc];
   I1 = r0;
   r0 = M[r7 + $audio_proc.vse.DELAY_FIELD];
   Words2Addr(r0);                  // arch4: delay in addr
   L1 = r0 + 1*ADDR_PER_WORD;

   // ******************************************************
   // load the Hc filter coefficient and loop counter
   // ******************************************************
   r2 = M[I3,MK1];                  // load b0
   r1 = M[I3,MK1];                  // load b1
   r6 = r1;                         // set r6 to b1
   r3 = M[I3,MK1];                  // load a1
   r10 = M[r7 + $audio_proc.vse.VSE_CUR_BLOCK_SIZE];

   // *******************
   // setup the loop
   // *******************
   M1 = 1*ADDR_PER_WORD;
   M2 = 2*ADDR_PER_WORD;
   r9 = DAWTH;
   r4 = M[I0, M1] , M0 = 0;                                   // r4 = x_in
   r0 = M[r7 + $audio_proc.vse.HcOUT_L_1];                    // r0 = y_l'
   r1 = M[r7 + $audio_proc.vse.HcOUT_H_1];                    // r1 = y_h'
   r5 = M[I1,0];                                              // r5 = x_in(n-d-1) where d = delay , I1 = &x_in( n-d-1)
   do hc_loop;
      rMAC = r3 * r0 (SU), M[I1,-MK1] = r4;                   // y = r3 * y_l' , write x_in , I1 = &x_in(n-d)
      rMAC = rMAC ASHIFT -DAWTH (56bit);                      // shift the computed value to the lower 24 bits ,
      rMAC = rMAC + r5 * r6 , r4 = M[I1,M0];                  // y = r3 * y_l'+ x_in(n-d-1) * b1 , r4 = x(n-d) I1 = &x_in( n-d)
      rMAC = rMAC + r4 * r2 , r5 = M[I1,M0] ;                 // y = r3 * y_l'+ x_in(n-d-1) * b1  +  x_in(n-d) * b2
      rMAC = rMAC + r3 * r1 , r4 = M[I0,M1];                  // y = r3 * (y_l' + y_h')+ x_in(n-d-1) * b1  +  x_in(n-d) * b2 , r4 = Next sample
      r1 = rMAC ASHIFT Null;                                  // y_h_out
      r0 = rMAC LSHIFT r9 , M[I2,M1] = r1;                    // y_l_out , write hp1_out_h(y_h_out) , I1 = &hp1_out_l
   hc_loop:

   M[r7 + $audio_proc.vse.HcOUT_L_1] = r0;
   M[r7 + $audio_proc.vse.HcOUT_H_1] = r1;
   r0 = I1;                                                   // I1 = &x_in( n-d-1)
   M[r7 + $audio_proc.vse.PTR_HISTORY_BUF_Hc] = r0;           // start at history buf base for next frame

   push Null;
   pop B1;
   L1 = 0;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.vse.filter2_process
//
// DESCRIPTION:
//  This function implements the high-pass filter hp2
//  The difference equation for the filter is as follows :
//   y = b0* x + b1 *x' + b2 x" - a1 *y' -a2 *y"
//
// INPUTS:
//    - r7 = pointer to VSE data object
//    - I0 = input buffer
//    - I5 = output buffer
//    - I3 = coefficient ptr
//    - r5 = Peaking filter/Low-Shelf cut filter (0/1)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - Everything except r7
//
// NOTES
//
// *****************************************************************************
.MODULE $M.audio_proc.vse.filter2_process;
   .CODESEGMENT   PM;

$audio_proc.vse.filter2_process:

   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.FILTER2_PROCESS.PATCH_ID_0,r1)     // VSE_patchers1
#endif



   // ********************************
   // Setup  Peak / LSF configuration
   // ********************************
   Null = r5;
   if NZ jump LSF_filter_setup;
   // *************************************************
   // Setup I1/B1/L1 for history buffer access
   // *************************************************
PEAK_filter_setup:
   r0 = r7 + $audio_proc.vse.START_HISTORY_BUF_PEAK;
   push r0;
   pop B1;
   r0 = M[r7 + $audio_proc.vse.PTR_HISTORY_BUF_PEAK];
   I1 = r0;
   L1 = $audio_proc.vse.PEAK_HIST_BUF_SIZE - $audio_proc.vse.START_HISTORY_BUF_PEAK;
   jump filter_setup_done;

LSF_filter_setup:
   r0 = r7 + $audio_proc.vse.START_HISTORY_BUF_LSF;
   push r0;
   pop B1;
   r0 = M[r7 + $audio_proc.vse.PTR_HISTORY_BUF_LSF];
   I1 = r0;
   L1 = $audio_proc.vse.LSF_HIST_BUF_SIZE - $audio_proc.vse.START_HISTORY_BUF_LSF;

filter_setup_done:
   // *************************************************
   // load history and set the loop counter
   // *************************************************
   r10 = M[r7 + $audio_proc.vse.VSE_CUR_BLOCK_SIZE];
   M1 = 1*ADDR_PER_WORD;
   M2 = 2*ADDR_PER_WORD;
   M3 = -2*ADDR_PER_WORD;
   M0 = 3*ADDR_PER_WORD;

   r2 = M[I3,MK1];  // load b0
   r3 = M[I3,MK1];  // load b1
   r1 = M[I3,MK1];  // load b2
   r8 = r1;         // set r8 to b2
   r1 = M[I3,MK1];  // load a1
   r6 = r1;         // set r6 to a1
   r1 = M[I3,MK1];  // load a2
   r9 = r1;         // set r9 to a2
   r4 = M[I0,MK1];  // r4 = x
   r0 = M[I1,MK1];  // dummy read
   r0 = M[I1,MK1];  // r0 = yL', I1 = &yL''
   push r5;
   r5 = DAWTH;
   do filter2_loop;
      rMAC =  r6 * r0 (SU) , r0 = M[I1, M1];           // y = a1*yL', r0 = yL'', I1 = &x
      rMAC = rMAC + r9 * r0 (SU), M[I1, M1] = r4;      // y = a1*yL' + a2 *yL'', write x , I1 = &x'
      rMAC = rMAC ASHIFT -DAWTH (56bit);               // shift the computed values to the lower 24 bits ,
      rMAC = rMAC + r2 * r4, r0 = M[I1, M1];           // y = a1*yL' + a2 *yL''+ b0* x , r0 = x' , I1 = &x''
      rMAC = rMAC + r3 * r0, r0 = M[I1, M2];           // y = a1*yL' + a2 *yL''+ b0* x + b1 *x' , r0 = x'' , I1 = &yh'
      rMAC = rMAC + r8 * r0, r0 = M[I1, M1];           // y = a1*yL' + a2 *yL''+ b0* x + b1 *x' + b2 *x'' r0 = yh' , I1 = &yh''
      rMAC = rMAC + r6 * r0, r0 = M[I1, M3];           // y = a1*yL' + a2 *(yL'' + yH'')+ b0* x + b1 *x' + b2 *x'' r0 = yh'' , I1 = &yh
      rMAC = rMAC + r9 * r0, r4 = M[I0,M1];            // y = a1*(yL' + yH') + a2 *(yL'' + yH'')+ b0* x + b1 *x' + b2 *x'' ,r0 = yh'', r4 = next sample
      rMAC = rMAC ASHIFT 1 (56bit);
      r1 = rMAC ASHIFT Null;                           // y_h_out
      r0 = rMAC LSHIFT r5 , M[I1,M0] = r1;             // y_l_out , write y_h'
      M[I1,M1] = r0, M[I5,M1] = rMAC;                  // write yL' , write y_h_out
   filter2_loop:

   r0 = M[I1,M3];                                      // dummy read , I1 = &yL
   r0 = I1;
   pop r5;
   if NZ jump LSF_filter_setup2;
   M[r7 + $audio_proc.vse.PTR_HISTORY_BUF_PEAK] = r0;  // start at history buf base for peaking filter
   jump filter_done;

LSF_filter_setup2:
   M[r7 + $audio_proc.vse.PTR_HISTORY_BUF_LSF] = r0;  // start at history buf base for LSF filter

filter_done:
   push Null;
   pop B1;
   L1 = 0;

   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $audio_proc.vse.limiter.process
//
// DESCRIPTION:
// This function implements the limiter process function
//
//  INPUTS:
//    - r7 = pointer to vse left_data_object
//    - r8 = pointer to vse right_data_object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0,r1,r2,r3,r4,r5,r6,r10,rMAC,I1,I4
//
// NOTES
//
// *****************************************************************************

.MODULE $M.audio_proc.vse.limiter.process;
   .CODESEGMENT   PM;

$audio_proc.vse.limiter.process:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.LIMITER.PROCESS.PATCH_ID_0,r1)     // VSE_patchers1
//#endif


   // ****************************************************************
   // Extract the limiter-input buffers for the left/right channels
   // ****************************************************************
   r0 = M[r7 + $audio_proc.vse.out_ipsi];
   I1 = r0;
   r0 = M[r8 + $audio_proc.vse.out_ipsi];
   I2 = r0;

   // ***********************************************
   // Limiter_Peak = max(abs(input(:)));
   // ***********************************************
   M1 = MK1;
   r10 = M[r7 + $audio_proc.vse.VSE_CUR_BLOCK_SIZE];
   r0 = M[I1,M1];                   // pre-load the first left channel input sample
   r3 = 0;                          // initialize running max
   do find_max_loop;
      r0 = ABS r0, r1 = M[I2,M1];   // abs(ch1_input) , load ch2_input
      r1 = ABS r1;                  // abs(ch2_input)
      r0 = MAX r1;                  // max(abs(ch1_input),abs(ch2_input))
      r3 = MAX r0 , r0 = M[I1,M1];  // running max , load ch1_input
   find_max_loop:

   // ***********************************************
   // InputLvl = log2(max(Limiter_Peak,pow2(-24)));
   // ***********************************************
   rMAC = r3;
   call $math.log2_taylor;         // output is in r0 in q8.xx

   // ***********************************************
   // if (InputLvl>LimitThreshold)
   // Limiter_OutputLvl_log = LimitThreshold + (InputLvl-LimitThreshold)/LimitRatio;
   // Inst_Gain_log = Limiter_OutputLvl_log-InputLvl;
   // Attack_tc = LimitAttack_tc;
   // Decay_tc = LimitDecay_tc;
   // ***********************************************
   rMAC = r0;                    // set rMAC to InputLvl(q8.xx)
   r2 = $LIMIT_THRESHOLD;        // q8.xx
   r3 = -1.0;
   rMAC = rMAC + r3 * r2;        // (InputLvl-LimitThreshold) q8.xx
   if NEG jump limiter_label1;
   rMAC = rMAC * 0.1;            // (InputLvl-LimitThreshold)/LimitRatio
   rMAC = rMAC - r3 * r2;        // Limiter_OutputLvl_log = LimitThreshold + (InputLvl-LimitThreshold)/LimitRatio
   rMAC = rMAC + r3 * r0;        // Inst_Gain_log = Limiter_OutputLvl_log-InputLvl;  // q8.xx
   r3 = M[r7 + $audio_proc.vse.LIMIT_ATTACK_TC];
   r4 = M[r7 + $audio_proc.vse.LIMIT_DECAY_TC];
   jump limiter_values_set;

limiter_label1:
   rMAC = 0;
   r3 = M[r7 + $audio_proc.vse.LINEAR_ATTACK_TC];
   r4 = M[r7 + $audio_proc.vse.LINEAR_DECAY_TC];

limiter_values_set:
   r5 = M[r7 + $audio_proc.vse.LIMITER_GAIN_LOG];
   Null = r5 - rMAC;
   if NEG jump limiter_label2;
   // ***********************************************************************************
   // Limiter_Gain_log = Inst_Gain_log*Attack_tc + Limiter_Gain_log*(1-Attack_tc);
   // ***********************************************************************************
   rMACB = 1.0;
   r0 = -1.0;
   rMACB = rMACB + r0 * r3;    // (1-Attack_tc)
   rMAC = rMAC * r3;           // Inst_Gain_log*Attack_tc
   rMAC = rMAC + r5 * rMACB;   // Limiter_Gain_log = Inst_Gain_log*Attack_tc + Limiter_Gain_log*(1-Attack_tc)  //q8.xx
   jump limiter_gain_calc_done;

limiter_label2:
   rMACB = 1.0;
   r0 = -1.0;
   rMACB = rMACB + r0 * r4;    // (1-Decay_tc)
   rMAC = rMAC * r4;           // Inst_Gain_log*Decay_tc
   rMAC = rMAC + r5 * rMACB;   // Limiter_Gain_log = Inst_Gain_log*Decay_tc + Limiter_Gain_log*(1-Decay_tc)    //q8.xx

limiter_gain_calc_done:
   // ***********************************************************************************
   // Limiter_Gain = pow2(Limiter_Gain_log);
   // ***********************************************************************************
   r0 = 1.0;
   M[r7 + $audio_proc.vse.LIMITER_GAIN_LOG] = rMAC;
   if Z jump limiter_gain_set;
   r0 = rMAC;               // q8.xx
   push r7;
   call $math.pow2_taylor;  // r0(q1.xx) holds the output
   pop r7;

limiter_gain_set:
   M[r7 + $audio_proc.vse.LIMITER_GAIN] = r0;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $audio_proc.vse.advance_pointers
//
// DESCRIPTION:
//   Advances the read/write pointers of the input/output respectively
//
// INPUTS:
//   r8 = pointer to the current channel data_object
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
.MODULE $M.audio_proc.vse.advance_pointers;
   .CODESEGMENT   PM;


$audio_proc.vse.advance_pointers:
   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.ADVANCE_POINTERS.PATCH_ID_0,r4)     // VSE_patchers1
//#endif


   // ****************************************************************
   // load the "cur_block_size"
   // ****************************************************************
   r4 = M[r8 + $audio_proc.vse.VSE_CUR_BLOCK_SIZE];
   Words2Addr(r4);
   M0 = r4;

   // ***********************************************************
   // extract the <length,base_address> for the input buffer
   // advance the input read address by cur_block_size
   // ***********************************************************
   call $audio_proc.vse.get_input_buffer_details;
   r0 = M[r8 + $audio_proc.vse.INPUT_READ_ADDR];
   I0 = r0;
   r0 = M[I0,M0];
   r0 = I0;
   M[r8 + $audio_proc.vse.INPUT_READ_ADDR] = r0;

   // ***********************************************************
   // extract the <length,base_address> for the output buffer
   // advance the output write address by cur_block_size
   // ***********************************************************
   call $audio_proc.vse.get_output_buffer_details;
   r0 = M[r8 + $audio_proc.vse.OUTPUT_WRITE_ADDR];
   I0 = r0;
   r0 = M[I0,M0];
   r0 = I0;
   M[r8 + $audio_proc.vse.OUTPUT_WRITE_ADDR] = r0;

   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************************************
// MODULE:
//    $audio_proc.vse.get_input_buffer_details
//
// DESCRIPTION:
//   Extracts the <read_addr,size,base_addr> for the input cbuffer/framebuffer for the current channel
//
// INPUTS:
//   r8 = pointer to the current channel data_object
//
// OUTPUTS:
//    - <r3,L0,B0> = <read_address,size,base_address> for the input cbuffer/framebuffer
//
// TRASHED REGISTERS:
//    - r0,r1,r2
//
// NOTES
//
// *****************************************************************************************************
.MODULE $M.audio_proc.vse.get_input_buffer_details;
   .CODESEGMENT   PM;

$audio_proc.vse.get_input_buffer_details:
   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.GET_INPUT_BUFFER_DETAILS.PATCH_ID_0,r1)     // VSE_patchers1
//#endif


   r0 = M[r8 + $audio_proc.vse.INPUT_ADDR_FIELD];
   Null = M[r8 + $audio_proc.vse.FRAMEBUFFER_FLAG];
   if NZ jump input_frame_buffer_details;

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
   r3 = r0;
   L0 = r1;
   jump input_buffer_details_done;

input_frame_buffer_details:
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
   r3 = r0;
   L0 = r1;

input_buffer_details_done:
   jump $pop_rLink_and_rts;

.ENDMODULE;


// ********************************************************************************************************
// MODULE:
//    $audio_proc.vse.get_output_buffer_details
//
// DESCRIPTION:
//   Extracts the <write_addr,size,base_addr> for the output cbuffer/framebuffer for the current channel
//
// INPUTS:
//   r8 = pointer to the current channel data_object
//
// OUTPUTS:
//    - <r3,L0,B0> = <write_address,size,base_address> for the output cbuffer/framebuffer
//
// TRASHED REGISTERS:
//    - r0,r1,r2
//
// NOTES
//
// ********************************************************************************************************
.MODULE $M.audio_proc.vse.get_output_cbuffer_details;
   .CODESEGMENT   PM;

$audio_proc.vse.get_output_buffer_details:
   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.VSE_ASM.VSE.GET_OUTPUT_BUFFER_DETAILS.PATCH_ID_0,r1)     // VSE_patchers1
//#endif


   r0 = M[r8 + $audio_proc.vse.OUTPUT_ADDR_FIELD];
   Null = M[r8 + $audio_proc.vse.FRAMEBUFFER_FLAG];
   if NZ jump output_frame_buffer_details;

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
   r3 = r0;
   L0 = r1;
   jump output_buffer_details_done;

output_frame_buffer_details:
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
   r3 = r0;
   L0 = r1;

output_buffer_details_done:
   jump $pop_rLink_and_rts;

.ENDMODULE;

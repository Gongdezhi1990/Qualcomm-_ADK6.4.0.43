/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#include "core_library.h"
#include "xover.h"
#include "peq.h"

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif


// *****************************************************************************
// MODULE:
//    $audio_proc.xover.initialize
//
// DESCRIPTION:
//    Initialize function for the xover module
//
// INPUTS:
//    - r8 = pointer to current channel xover_data_object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - Assume everything
//
// NOTES:
// *****************************************************************************
.MODULE $M.audio_proc.xover.initialize;
   .CODESEGMENT   PM;

$audio_proc.xover.initialize:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.XOVER_ASM.XOVER.INITIALIZE.PATCH_ID_0,r1)     // xover_patchers1
//#endif

   // *******************************************************************************
   // Check if 2-band-xover is bypassed and if not call the xover initialize function
   // *******************************************************************************
   r0 = M[r8 + $audio_proc.xover_2band.filter.BYPASS];
   if NZ jump init_crossover_done;

   // ****************************************************
   // ****************************************************
   //          PEQ COEFFICIENTS GENERATION
   // ****************************************************
   // ****************************************************

   // **************************************************************
   // load pointer to xover parameters
   // call the BW_LR/APC initialize based on the input filter_type
   // **************************************************************
   r7 = M[r8 + $audio_proc.xover_2band.PARAM_PTR_FIELD];
   r5 = M[r7 + $audio_proc.xover_2band.parameter.LP_TYPE];
   Null = r5 - $audio_proc.xover_2band.filter.APC;
   if Z jump apc_filter_init;

   // **************************************************************
   // call the coeffs initialize function for the low_pass filter
   // **************************************************************
   r6 = 1;
   call $audio_proc.xover.bw_lr_filter.initialize;

   // **************************************************************
   // call the coeffs initialize function for the high_pass filter
   // **************************************************************
   r5 = M[r7 + $audio_proc.xover_2band.parameter.HP_TYPE];
   r6 = 2;
   call $audio_proc.xover.bw_lr_filter.initialize;
   jump select_peq_core;

apc_filter_init:
   call $audio_proc.xover.apc.initialize;

   // ****************************************************
   // ****************************************************
   //          PEQ FILTER INITIALIZE
   // ****************************************************
   // ****************************************************

select_peq_core:
   // *******************************************
   // select the peq_core filter to be called
   // *******************************************
   r3 = M[r7 + $audio_proc.xover_2band.parameter.XOVER_CORE_TYPE];

sh_initialize:
   Null = r3 - $audio_proc.xover_2band.filter_prec.SH;
   if NZ jump hq_initialize;
   r4 = $audio_proc.sh_peq.initialize;
   jump init_peq_filter;

hq_initialize:
   Null = r3 - $audio_proc.xover_2band.filter_prec.HQ;
   if NZ jump dh_initialize;
   r4 = $audio_proc.hq_peq.initialize;
   jump init_peq_filter;

dh_initialize:
   r4 = $audio_proc.dh_peq.initialize;

init_peq_filter:
   // ********************************************
   // initialize the peq low_freq data_object
   // ********************************************
   r7 = M[r8 + $audio_proc.xover_2band.PEQ_OBJECT_PTR_LOW_FREQ];
   push r8;
   call r4;
   pop r8;

   // ********************************************
   // initialize the peq high_freq data_object
   // ********************************************
   r7 = M[r8 + $audio_proc.xover_2band.PEQ_OBJECT_PTR_HIGH_FREQ];
   push r8;
   call r4;
   pop r8;

   // ***********************************
   // load pointer to xover parameters
   // ***********************************
   r7 = M[r8 + $audio_proc.xover_2band.PARAM_PTR_FIELD];


   // ****************************************
   // load pointer to "headroom_bits" tables
   // ****************************************
   r5 = $headroom_bits_all_values;

   // ***********************************************
   // set "headroom_bits" for the low_pass filter
   // ***********************************************
   r0 = M[r7 + $audio_proc.xover_2band.parameter.LP_ORDER];
   r1 = M[r7 + $audio_proc.xover_2band.parameter.LP_TYPE];
   Null = r1 - $audio_proc.xover_2band.filter.APC;
   if Z jump headroom_bits_high_pass;
   r0 = r0 - 1;        // calculate index in the headroom_bits table
   Words2Addr(r0);     // arch4: headroom _bits index in addr
   r1 = r1 - 1;        // index for picking bw/lr filter
   Words2Addr(r1);     // arch4: bw/lr index in addr
   r3 = M[r5 + r1];    // index into the bw or lr table
   r4 = M[r3 + r0];    // extract the "headroom_bits" value for the "low_pass" filter
   r3 = M[r8 + $audio_proc.xover_2band.PEQ_OBJECT_PTR_LOW_FREQ];
   M[r3	+ $audio_proc.peq.HEADROOM_BITS] = r4;


headroom_bits_high_pass:
   // ***********************************************
   // set "headroom_bits" for the high_pass filter
   // ***********************************************
   r0 = M[r7 + $audio_proc.xover_2band.parameter.HP_ORDER];
   r1 = M[r7 + $audio_proc.xover_2band.parameter.HP_TYPE];
   Null = r1 - $audio_proc.xover_2band.filter.APC;
   if Z jump init_crossover_done;
   r0 = r0 - 1;        // calculate index in the headroom_bits table
   Words2Addr(r0);     // arch4: headroom _bits index in addr
   r1 = r1 - 1;        // index for picking bw/lr table
   Words2Addr(r1);     // arch4: bw/lr index in addr
   r3 = M[r5 + r1];    // index into the bw or lr table
   r4 = M[r3 + r0];    // extract the "headroom_bits" value for the high_pass filter
   r3 = M[r8 + $audio_proc.xover_2band.PEQ_OBJECT_PTR_HIGH_FREQ];
   M[r3	+ $audio_proc.peq.HEADROOM_BITS] = r4;

init_crossover_done:
   jump $pop_rLink_and_rts;

.ENDMODULE;


// ***************************************************************************************
// MODULE:
//    $audio_proc.xover.bw_lr_filter.initialize
//
// DESCRIPTION:
//   Generates the coeffs for BW/LR filter by calling the "peq_cap_gen_coeffs" function
//
// INPUTS:
//    - r5 = flag indicating butterworth/linkwitz_riley filter (1/2)
//    - r6 = flag indicating low_pass_filter/high_pass_filter (1/2)
//    - r7 = pointer to xover parameters
//    - r8 = pointer to current channel xover_data_object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - Everything except <r7,r8>
//
// NOTES:
// ***************************************************************************************
.MODULE $M.audio_proc.xover.bw_lr_filter.initialize;
   .CODESEGMENT   PM;

$audio_proc.xover.bw_lr_filter.initialize:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.XOVER_ASM.XOVER.BW_LR_FILTER.INITIALIZE.PATCH_ID_0,r1)     // compander_patchers1
//#endif



   pushm <r7,r8>;
   r3 = r7;
   Null = r6 - 2;
   if Z jump high_pass_filter_coeff_gen;

   // *********************************************************************
   //                   LOW-PASS FILTER COEFFICIENT GENERATION
   // *********************************************************************

low_pass_filter_coeff_gen:
   // ********************************************************************************
   // set the peq_cap_params values based on the xover_params for the low_pass filter
   // ********************************************************************************
   r0 = M[r3 + $audio_proc.xover_2band.parameter.LP_ORDER];
   r1 = M[r3 + $audio_proc.xover_2band.parameter.LP_FC];
   r2 = M[r8 + $audio_proc.xover_2band.PEQ_CAP_PARAMS_LOW_FREQ];
   call $audio_proc.xover.set_peq_params;

   // ************************************************************************
   // call the coefficient generation function for the low-pass filter
   // ************************************************************************
   r6 = M[r8 + $audio_proc.xover_2band.SAMPLE_RATE_FIELD];                // sample rate
   r7 = M[r8 + $audio_proc.xover_2band.PEQ_OBJECT_PTR_LOW_FREQ];
   r7 = M[r7 + $audio_proc.peq.PARAM_PTR_FIELD];                          // pointer to coeff_params
   r8 = M[r8 + $audio_proc.xover_2band.PEQ_CAP_PARAMS_LOW_FREQ];
   r8 = r8 + $M.PEQ.PARAMETERS.OFFSET_NUM_BANDS;                          // pointer to peq_cap_params with num_bands offset
   pushm <r3,r7>;
   call $peq_cap_gen_coeffs;
   popm <r3,r7>;

   // ***************************************************************
   // invert <b2,b1,b0> for stage1 if "INV_BAND1(low_freq)" is set
   // ***************************************************************
   r0 = M[r3 + $audio_proc.xover_2band.parameter.XOVER_CONFIG];
   r1 = r0 AND $M.XOVER.CONFIG.INV_BAND1;
   if NZ jump invert_stage1_b_coeffs;
   if Z jump filter_coeff_gen_done;


   // *********************************************************************
   //                   HIGH-PASS FILTER COEFFICIENT GENERATION
   // *********************************************************************

high_pass_filter_coeff_gen:
   // ********************************************************************************
   // set the peq_cap_params values based on the xover_params for the high_pass filter
   // ********************************************************************************
   r0 = M[r3 + $audio_proc.xover_2band.parameter.HP_ORDER];
   r1 = M[r3 + $audio_proc.xover_2band.parameter.HP_FC];
   r2 = M[r8 + $audio_proc.xover_2band.PEQ_CAP_PARAMS_HIGH_FREQ];
   call $audio_proc.xover.set_peq_params;

   // ************************************************************************
   // call the coefficient generation function for the high-pass filter
   // ************************************************************************
   r6 = M[r8 + $audio_proc.xover_2band.SAMPLE_RATE_FIELD];                // sample rate
   r7 = M[r8 + $audio_proc.xover_2band.PEQ_OBJECT_PTR_HIGH_FREQ];
   r7 = M[r7 + $audio_proc.peq.PARAM_PTR_FIELD];                          // pointer to coeff_params
   r8 = M[r8 + $audio_proc.xover_2band.PEQ_CAP_PARAMS_HIGH_FREQ];
   r8 = r8 + $M.PEQ.PARAMETERS.OFFSET_NUM_BANDS;                          // pointer to peq_cap_params with num_bands offset
   pushm <r3,r7>;
   call $peq_cap_gen_coeffs;
   popm <r3,r7>;

   // ***************************************************************
   // invert <b2,b1,b0> for stage1 if "INV_BAND2(high_freq)" is set
   // ***************************************************************
   r0 = M[r3 + $audio_proc.xover_2band.parameter.XOVER_CONFIG];
   r1 = r0 AND $M.XOVER.CONFIG.INV_BAND2;
   if Z jump filter_coeff_gen_done;

invert_stage1_b_coeffs:
   r0 = M[r7 + $audio_proc.xover_2band.PEQ_STAGE1_COEFF_OFFSET];
   r0 = -r0;
   M[r7 + $audio_proc.xover_2band.PEQ_STAGE1_COEFF_OFFSET] = r0;
   r0 = M[r7 + ($audio_proc.xover_2band.PEQ_STAGE1_COEFF_OFFSET + 1*ADDR_PER_WORD)];
   r0 = -r0;
   M[r7 + ($audio_proc.xover_2band.PEQ_STAGE1_COEFF_OFFSET + 1*ADDR_PER_WORD)] = r0;
   r0 = M[r7 + ($audio_proc.xover_2band.PEQ_STAGE1_COEFF_OFFSET + 2*ADDR_PER_WORD)];
   r0 = -r0;
   M[r7 + ($audio_proc.xover_2band.PEQ_STAGE1_COEFF_OFFSET + 2*ADDR_PER_WORD)] = r0;

filter_coeff_gen_done:
   popm <r7,r8>;
   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $audio_proc.xover.set_peq_params
//
// DESCRIPTION:
//    Set the peq_params buffer based on the input
//
// INPUTS:
//    - r0 = filter order
//    - r1 = filter_cutoff frequency
//    - r2 = pointer to peq_params
//    - r5 = flag indicating butterworth/linkwitz_riley filter (1/2)
//    - r6 = flag indicating low_pass_filter/high_pass filter (1/2)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0,r4,r5,r6,r9,r10,I0,I4,I5,M0,M1,M2
//
// NOTES:
// *****************************************************************************
.MODULE $M.audio_proc.xover.set_peq_params;
   .CODESEGMENT   PM;

$audio_proc.xover.set_peq_params:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($audio_proc.XOVER_ASM.XOVER.SET_PEQ_PARAMS.PATCH_ID_0,r4)     // xover_patchers1
//#endif


   // ***********************************
   // choose butterworth/linkwitz_riley
   // ***********************************
   r9 = $butterworth_filter_all_params;
   r4 = $linkwitz_riley_filter_all_params;
   Null = r5 - 2;
   if NZ jump index_set;

linkwitz_riley:
   r9 = r4;
   Null = r0 AND 1;                                   // check if input filter order is odd
   if NZ r0 = r0 + 1;                                 // make filter order even if input order is odd
   r0 = r0 ASHIFT -1;

index_set:
   r0 = r0 - 1;                                       // index into the params table
   Words2Addr(r0);                                    // arch4: index in addr
   r9 = M[r9 + r0];                                   // pointer to all params for the input filter order

   // *****************************
   // write the number of stages
   // *****************************
   r4 = M[r9 + Null];                                 // pointer to number of stages
   r10 = M[r4 + Null];                                // number of stages
   M[r2 + $M.PEQ.PARAMETERS.OFFSET_NUM_BANDS] = r10;  // write num_stages to peq_params

   // *******************************
   // set the input/output pointers
   // *******************************
   Words2Addr(r6);                                    // arch4: lp/hp flag in addr
   r4 = M[r9 + r6];
   I0 = r4;                                           // pointer to filter_types(input)
   r5 = M[r9 + 3*ADDR_PER_WORD];
   I4 = r5;                                           // pointer to Q_values    (input)
   I1 = r2 + $M.PEQ.PARAMETERS.OFFSET_STAGE1_TYPE;    // pointer to stage1_type (output)
   I5 = r2 + $M.PEQ.PARAMETERS.OFFSET_STAGE1_GAIN;    // pointer to stage1_gain (output)

   // ********************************
   // loop constants (r10 set above)
   // ********************************
   r4 = 0;                                            // gain for each stage (0 dB)
   M0 = 3*ADDR_PER_WORD;
   M1 = 1*ADDR_PER_WORD;
   M2 = 2*ADDR_PER_WORD;
   do set_peq_params_loop;
      r0 = M[I0,MK1] , r5 = M[I4,MK1];                // load type , Q
      M[I1,M1] = r0 , M[I5,M1] = r4;                  // write type , gain
      M[I1,M0] = r1 , M[I5,M0] = r5;                  // write FC , write Q
   set_peq_params_loop:

   // **************************
   // write the master gain
   // **************************
   M[r2 + $M.PEQ.PARAMETERS.OFFSET_MASTER_GAIN] = 0;  // master gain(0 dB)

   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $audio_proc.xover.apc.initialize
//
// DESCRIPTION:
//   Generates the filter coefficients for the APC filter type
//
// INPUTS:
//    - r7 = pointer to xover parameters
//    - r8 = pointer to current channel xover_data_object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - Everything except r7,r8
//
// NOTES:
// *****************************************************************************
.MODULE $M.audio_proc.xover.apc.initialize;
   .CODESEGMENT   PM;

$audio_proc.xover.apc.initialize:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.XOVER_ASM.XOVER.APC.INITIALIZE.PATCH_ID_0,r2)     // XOVER_patchers1
//#endif

   pushm <r7,r8>;

   r4 = M[r7 + $audio_proc.xover_2band.parameter.LP_FC];           // cutoff_frequency
   r6 = M[r8 + $audio_proc.xover_2band.SAMPLE_RATE_FIELD];         // sample rate

   r2 = M[r8 + $audio_proc.xover_2band.PEQ_CAP_PARAMS_LOW_FREQ];
   r7 = M[r8 + $audio_proc.xover_2band.PEQ_OBJECT_PTR_LOW_FREQ];
   r3 = 3;                                                         // low_pass filter type
   push r8;
   call $audio_proc.xover.apc_filter.coeff_gen;
   pop r8;

   r2 = M[r8 + $audio_proc.xover_2band.PEQ_CAP_PARAMS_HIGH_FREQ];
   r7 = M[r8 + $audio_proc.xover_2band.PEQ_OBJECT_PTR_HIGH_FREQ];
   r3 = 9;                                                         // high_pass filter type
   call $audio_proc.xover.apc_filter.coeff_gen;

   popm <r7,r8>;
   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $audio_proc.xover.apc_filter.coeff_gen
//
// DESCRIPTION:
//    Set the peq_params buffer based on the input and call the
//    coefficients generate function for the APC filter
//
// INPUTS:
//    - r2 = pointer to peq_cap_params buffer
//    - r3 = filter_type value
//    - r4 = filter cut_off frequency
//    - r6 = sample rate
//    - r7 = pointer to peq_data_object(low/high)
//    - r8 = pointer to current channel xover_data_object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - Everything except r6
//
// NOTES:
// *****************************************************************************
.MODULE $M.audio_proc.xover.apc_filter.coeff_gen;
   .CODESEGMENT   PM;

$audio_proc.xover.apc_filter.coeff_gen:

   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.XOVER_ASM.XOVER.APC_FILTER.COEFF_GEN.PATCH_ID_0,r1)     // XOVER_patchers1
//#endif


   // ***************************
   // constants to be written
   // ***************************
   r0 = 1;                                                 // num_stages
   r1 = 0;                                                 // master_gain and stage_gain (0 dB)
   r5 = Qfmt_(1.000000, 8);                                // Q value (1.0 in Q8.xx)

   // ***********************************************
   // write the values to peq_cap_params buffer
   // ***********************************************
   M[r2 + $M.PEQ.PARAMETERS.OFFSET_NUM_BANDS]   = r0;
   M[r2 + $M.PEQ.PARAMETERS.OFFSET_MASTER_GAIN] = r1;
   M[r2 + $M.PEQ.PARAMETERS.OFFSET_STAGE1_TYPE] = r3;
   M[r2 + $M.PEQ.PARAMETERS.OFFSET_STAGE1_FC]   = r4;
   M[r2 + $M.PEQ.PARAMETERS.OFFSET_STAGE1_GAIN] = r1;
   M[r2 + $M.PEQ.PARAMETERS.OFFSET_STAGE1_Q]    = r5;

   // *********************************************
   // call the coefficient generation function
   // *********************************************
   r7 = M[r7 + $audio_proc.peq.PARAM_PTR_FIELD];                          // pointer to coeff_params
   r8 = r2 + $M.PEQ.PARAMETERS.OFFSET_NUM_BANDS;                          // pointer to peq_cap_params with num_bands offset
   call $peq_cap_gen_coeffs;

   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************************
// MODULE:
//    $audio_proc.xover_2band.process
//
// DESCRIPTION:
//   Separates the input into low and high frequency bands. It has 2 entry points.
//   $audio_proc.xover_2band.frame_process  : Entry point for frame_sync API
//   $audio_proc.xover_2band.stream_process : Entry point for Kymera API
//
// INPUTS:
//    - r8 = pointer to current channel xover_data_object
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - Assume everything
//
// NOTES:
// *****************************************************************************************
.MODULE $M.audio_proc.xover_2band.process;
   .CODESEGMENT   PM;

$audio_proc.xover_2band.frame_process:

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.XOVER_ASM.XOVER_2BAND.PROCESS.PATCH_ID_0,r1)     // XOVER_patchers1
//#endif


   // *************************************************************************************************************************************
   // Extracts the cbuffer pointers from the framebuffers and writes them to the input/output addresses of current_channel data object.
   // Falls through to the $audio_proc.xover_2band.stream_process() function
   // *************************************************************************************************************************************

   // ******************************************
   // Set input_cbuffer from input frame buffer
   // ******************************************
   r0 = M[r8 + $audio_proc.xover_2band.INPUT_FRAME_ADDR_FIELD];
   r0 = M[r0 + $frmbuffer.CBUFFER_PTR_FIELD];
   M[r8 + $audio_proc.xover_2band.INPUT_ADDR_FIELD] = r0;

   // *******************************************************************************************
   // Set the low_freq and high_freq output cbuffers from the corresponding output frame buffers
   // *******************************************************************************************
   r0 = M[r8 + $audio_proc.xover_2band.OUTPUT_LOW_FRAME_ADDR_FIELD];
   r0 = M[r0 + $frmbuffer.CBUFFER_PTR_FIELD];
   M[r8 + $audio_proc.xover_2band.OUTPUT_ADDR_FIELD_LOW] = r0;

   r0 = M[r8 + $audio_proc.xover_2band.OUTPUT_HIGH_FRAME_ADDR_FIELD];
   r0 = M[r0 + $frmbuffer.CBUFFER_PTR_FIELD];
   M[r8 + $audio_proc.xover_2band.OUTPUT_ADDR_FIELD_HIGH] = r0;

   // ************************************
   // Set "samples_to_process" from input
   // ************************************
   r0 = M[r8 + $frmbuffer.FRAME_SIZE_FIELD];
   M[r8 + $audio_proc.xover_2band.SAMPLES_TO_PROCESS] = r0;


$audio_proc.xover_2band.stream_process:

   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.XOVER_ASM.XOVER_2BAND.PROCESS.PATCH_ID_1,r1)     // XOVER_patchers1
#endif


   // ****************************************
   // select the peq_core filter to be called
   // ****************************************
   r7 = M[r8 + $audio_proc.xover_2band.PARAM_PTR_FIELD];
   r1 = M[r7 + $audio_proc.xover_2band.parameter.XOVER_CORE_TYPE];

sh_process:
   Null = r1 - $audio_proc.xover_2band.filter_prec.SH;
   if NZ jump hq_process;
   r3 = $audio_proc.sh_peq.process_op;
   jump xover_processing;
hq_process:
   Null = r1 - $audio_proc.xover_2band.filter_prec.HQ;
   if NZ jump dh_process;
   r3 = $audio_proc.hq_peq.process_op;
   jump xover_processing;
dh_process:
   r3 = $audio_proc.dh_peq.process_op;



xover_processing:
   // ****************************************************************
   // ****************************************************************
   //                       LOW FREQUENCY OUTPUT
   // ****************************************************************
   // ****************************************************************

   // ************************************************************
   // get read pointer, size and start address of input cbuffer
   // ************************************************************
   r0 = M[r8 + $audio_proc.xover_2band.INPUT_ADDR_FIELD];
   call $audio_proc.xover_2band.get_input_cbuffer_details;

   // **********************************************************************
   // get write pointer, size and start address of low_freq output cbuffer
   // **********************************************************************
   r0 = M[r8 + $audio_proc.xover_2band.OUTPUT_ADDR_FIELD_LOW];
   call $audio_proc.xover_2band.get_output_cbuffer_details;

   // ****************************************************************
   // call the peq process function for low_frequency component
   // ****************************************************************
   r4 = M[r8 + $audio_proc.xover_2band.SAMPLES_TO_PROCESS];
   r7 = M[r8 + $audio_proc.xover_2band.PEQ_OBJECT_PTR_LOW_FREQ];
   pushm <r3,r8>;
   call r3;
   popm <r3,r8>;


   // ****************************************************************
   // ****************************************************************
   //                     HIGH FREQUENCY OUTPUT
   // ****************************************************************
   // ****************************************************************

   // ************************************************************
   // get read pointer, size and start address of input cbuffer
   // ************************************************************
   r0 = M[r8 + $audio_proc.xover_2band.INPUT_ADDR_FIELD];
   call $audio_proc.xover_2band.get_input_cbuffer_details;

   // **********************************************************************
   // get write pointer, size and start address of high_freq output cbuffer
   // **********************************************************************
   r0 = M[r8 + $audio_proc.xover_2band.OUTPUT_ADDR_FIELD_HIGH];
   call $audio_proc.xover_2band.get_output_cbuffer_details;

   // ****************************************************************
   // call the peq process function for high_frequency component
   // ****************************************************************
   r4 = M[r8 + $audio_proc.xover_2band.SAMPLES_TO_PROCESS];
   r7 = M[r8 + $audio_proc.xover_2band.PEQ_OBJECT_PTR_HIGH_FREQ];
   push r8;
   call r3;
   pop  r8;

   // *********************************
   // check if the filter_type is APC
   // *********************************
   r7 = M[r8 + $audio_proc.xover_2band.PARAM_PTR_FIELD];
   r0 = M[r7 + $audio_proc.xover_2band.parameter.LP_TYPE];
   Null = r0 - $audio_proc.xover_2band.filter.APC;
   if NZ jump xover_process_done;

   // **********************************************************************
   // get write pointer, size and start address of low_freq output cbuffer
   // **********************************************************************
   r0 = M[r8 + $audio_proc.xover_2band.OUTPUT_ADDR_FIELD_LOW];
   call $audio_proc.xover_2band.get_output_cbuffer_details;
#ifdef BASE_REGISTER_MODE
   push B0;
   pop B1;
#endif
   I1 = I0;
   L1 = L0;

   // **********************************************************************
   // get write pointer, size and start address of high_freq output cbuffer
   // **********************************************************************
   r0 = M[r8 + $audio_proc.xover_2band.OUTPUT_ADDR_FIELD_HIGH];
   call $audio_proc.xover_2band.get_output_cbuffer_details;

   // *******************************************************
   // set the band multipliers based on the the user input
   // *******************************************************
   r5 = 1.0;
   r6 = -1.0;
   r0 = M[r7 + $audio_proc.xover_2band.parameter.XOVER_CONFIG];
   r1 = r0 AND $M.XOVER.CONFIG.INV_BAND1;
   if NZ r5 = r6;
   r1 = r0 AND $M.XOVER.CONFIG.INV_BAND2;
   if Z r6 = r5;

   // *********************************************
   // generate the low_freq and high_freq outputs
   // *********************************************
   r10 = M[r8 + $audio_proc.xover_2band.SAMPLES_TO_PROCESS];
   r5 = r5 * 0.5(frac);
   r6 = r6 * 0.5(frac);
   M1 = 1*ADDR_PER_WORD;
   r0 = M[I0,0] , M0 = 0;
   do apc_output_loop;
      r2 = r0 * r6(frac) , r1 = M[I1,M0];
      r2 = r2 + r1 * r6;
      r3 = r1 * r5(frac) , M[I0,M1] = r2;
      r3 = r3 - r0 * r5 , r0 = M[I0,M0];
      M[I1,M1] = r3;
   apc_output_loop:

xover_process_done:
   // **************************************************************
   // clear the base registers <B0,B1> under "BASE_REGISTER_MODE"
   // clear <L0,L1> length registers
   // **************************************************************
#ifdef BASE_REGISTER_MODE
   push Null;
   pop B0;
   push Null;
   pop B1;
#endif
   L0 = 0;
   L1 = 0;

   jump $pop_rLink_and_rts;

.ENDMODULE;



// *****************************************************************************************
// MODULE:
//    $audio_proc.xover_2band.get_input_cbuffer_details
//
// DESCRIPTION:
//   Extracts the <read_addr,size,base_addr> for the input cbuffer for the current channel
//
// INPUTS:
//   r0 = pointer to the input cbuffer
//
// OUTPUTS:
//    - <I4,L4,B4> = <read_address,size,base_address> for the input cbuffer
//
// TRASHED REGISTERS:
//    - r0,r1,r2
//
// NOTES
//
// *****************************************************************************************
.MODULE $M.audio_proc.xover_2band.get_input_cbuffer_details;
   .CODESEGMENT   PM;

$audio_proc.xover_2band.get_input_cbuffer_details:
   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($audio_proc.XOVER_ASM.XOVER_2BAND.GET_INPUT_CBUFFER_DETAILS.PATCH_ID_0,r1)     // XOVER_patchers1
//#endif


#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_read_address_and_size_and_start_address;
   push r2;
   pop B4;
#else
    call $cbuffer.get_read_address_and_size;
#endif
   I4 = r0;
   L4 = r1;

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************************
// MODULE:
//    $audio_proc.xover_2band.get_output_cbuffer_details
//
// DESCRIPTION:
//   Extracts the <write_addr,size,base_addr> for the output cbuffer for the current channel
//
// INPUTS:
//   r0 = pointer to the output cbuffer
//
// OUTPUTS:
//    - <I0,L0,B0> = <write_address,size,base_address> for the output cbuffer
//
// TRASHED REGISTERS:
//    - r0,r1,r2
//
// NOTES
//
// *****************************************************************************************
.MODULE $M.audio_proc.xover_2band.get_output_cbuffer_details;
   .CODESEGMENT   PM;

$audio_proc.xover_2band.get_output_cbuffer_details:
   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($audio_proc.XOVER_ASM.XOVER_2BAND.GET_OUTPUT_CBUFFER_DETAILS.PATCH_ID_0,r1)     // XOVER_patchers1
//#endif


#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_write_address_and_size_and_start_address;
   push r2;
   pop B0;
#else
    call $cbuffer.get_write_address_and_size;
#endif
   I0 = r0;
   L0 = r1;

   jump $pop_rLink_and_rts;

.ENDMODULE;

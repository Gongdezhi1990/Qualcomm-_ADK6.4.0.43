/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#include "sra.h"
#include "stack.h"
#include "portability_macros.h"
#include "sra_c_asm_defs.h"

#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif

.MODULE $M.audio_proc.sra_resample;
   .CODESEGMENT PM;
   .DATASEGMENT DM;
// *****************************************************************************
// NAME:
//    $sra_resample_core
//
// DESCRIPTION:
//
//  private function to resample the input
//
// INPUTS:
//  r8: pointer to operator structure's start of the channel-independent params
//  r10: number of input samples
//  I4, L4: input buffer address and size
//  I5, L5: output buffer and size
//  I0, L0, L1: hist buffer and size (L0 = L1)
//  I3: pointer to the coeffs
//  M3: MK1
//
// OUTPUTS:
//  r0: total number of sample required for last iteration of the loop
//  r10:  (available number of sample for last iteration of the loop - r0)
//  r4 : new value of sra_c.sra_params_struct.RF_FIELD
//  I7: total number of output samples generated
//  I0: updated hist buffer address
//
// TRASHED REGISTERS:
//    r0-r7,r9, I7, rMAC, I0-I2, I6, M0-M2
// NOTE:
// this function has not been optimized yet
// *****************************************************************************
   $sra_resample_core:

   /* need two temp vars, use stack;
    */
   .CONST  $sra.STACK_VAR_SPACE              2*ADDR_PER_WORD;
   .CONST  $sra.TEMPVAR_SHIFT_VALUE_FIELD   -2*ADDR_PER_WORD;
   .CONST  $sra.TEMPVAR_SCRATCH_FIELD       -1*ADDR_PER_WORD;
   // NB. optimisation - those two must be consecutive since they are accessed via inc/dec of I2

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.SRA_ASM.CORE.PATCH_ID_0, r7)
#endif

   // Reserve locations "below" SP for temp vars
   SP = SP + $sra.STACK_VAR_SPACE;
   // read the shift amount.   If positive use as a pre-shift (input)
   // otherwise it is a post shift.  Saved value is output shift
   // r7 is input shift
   r0 = M[r8 + $sra_c.sra_params_struct.SHIFT_AMOUNT_FIELD];
   if POS r0=NULL;
   r7 = M[r8 + $sra_c.sra_params_struct.SHIFT_AMOUNT_FIELD];
   if NEG r7=NULL;
   // save it in a temp variable
   M[SP + $sra.TEMPVAR_SHIFT_VALUE_FIELD] = r0;
   // I2 pointing to scratch, optimised to access the shift_val as well
   r0 = SP + $sra.TEMPVAR_SCRATCH_FIELD;
   I2 = r0;

   // set up registers
   r5 = M[r8 + $sra_c.sra_params_struct.SRA_CURRENT_RATE_FIELD];

   r4 = M[r8 + $sra_c.sra_params_struct.RF_FIELD];
   r6 = $sra.SRA_UPRATE;
   M2 = -($sra.SRA_UPRATE * ADDR_PER_WORD);
   M0 = $sra.SRA_UPRATE * ADDR_PER_WORD;
   rMAC = r4*r6;
   I7 = 0;
   r9 = $FALSE; // Initialise to no extra load has occurred
   r0 = M[r8 + $sra_c.sra_params_struct.PREV_SHORT_SAMPLES_FIELD];
   if Z jump start_gen;
      r1 = M[I4, MK1];
      r1 = r1 ASHIFT r7;
      r10 = r10 - 1, M[I0, M3] = r1;

   start_gen:
   do sra_gen_loop;
      // input is upsampled by rate=r6, find the best points to iterpolate
      r3 = rMAC0;
      r3 = r3 LSHIFT -1; // r3 = interpolation ratio
      r2 = rMAC1;        // r2 = index of first point
      Words2Addr(r2);    // first_index (arch4: in addrs)

      // FIR filtering starts from right side of coeffs
      I6 = I3 + r2;
      r0 = (-$sra.SRA_UPRATE-1)*ADDR_PER_WORD - r2;
      M1 = r0 - r2;  // -2*first_index - sra_uprate - 1 (arch4: in addrs)
      I1 = I0;
      rMAC = 0, r0 = M[I1,M3], r1 = M[I6,M2];              //1st coeff & sample
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M2]; //2nd coeff & sample
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M2]; //3rd coeff & sample
      #ifndef SRA_NORMAL_QUALITY_COEFFS
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M2]; //4th coeff & sample
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M2]; //5th coeff & sample
     #ifdef SRA_VERY_HIGH_QUALITY_COEFFS
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M2]; //6th coeff & sample
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M2]; //7th coeff & sample
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M2]; //8th coeff & sample
     #endif
     #endif
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M1]; //9th coeff & sample

     #ifndef SRA_NORMAL_QUALITY_COEFFS
     #ifdef SRA_VERY_HIGH_QUALITY_COEFFS
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //10th coeff & sample
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //11th coeff & sample
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //12th coeff & sample
     #endif
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //13th coeff & sample
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //14th coeff & sample
     #endif
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //15th coeff & sample
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //16th coeff & sample
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //17th coeff & sample
      rMAC = rMAC - r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //18th coeff & sample
      rMAC = rMAC - r0 * r1;

      //calculate second point
      M1 = M1 - 2*ADDR_PER_WORD;
      r2 = r2 + M3, M[I2,  0] = rMAC;
      //if first point index is (r6-1) then to calculate next point and extra load is required
      if NEG jump no_extra_load;
         // load an extra point
         M1 = M0 - ADDR_PER_WORD;
         /* Mark that this sample has been read to avoid re-reading port later.
         also shift the input */
         NULL = r9- $TRUE;
         // If this is already loaded then we can just move I1 to point at the
         // next value in the history buffer.
         if NZ jump not_loaded;
         r2 = M2, r1 = M[I1, M3];
         jump no_extra_load;
         not_loaded:
         r9 = $TRUE;
         r1 = M[I4, MK1];
         r1 = r1 ASHIFT r7;
         r2 = M2, M[I1, M3] = r1;
      no_extra_load:
      // calculate second point
      I6 = I3 + r2;
      I7 = I7 + 1;
      r0 = M[I1,M3], r1 = M[I6,M2];                        //1st coeff & sample
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M2]; //2nd coeff & sample
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M2]; //3rd coeff & sample
      #ifndef SRA_NORMAL_QUALITY_COEFFS
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M2]; //4th coeff & sample
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M2]; //5th coeff & sample
      #ifdef SRA_VERY_HIGH_QUALITY_COEFFS
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M2]; //6th coeff & sample
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M2]; //7th coeff & sample
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M2]; //8th coeff & sample
      #endif
      #endif
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M1]; //9th coeff & sample
      #ifndef SRA_NORMAL_QUALITY_COEFFS
      #ifdef SRA_VERY_HIGH_QUALITY_COEFFS
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //10th coeff & sample
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //11th coeff & sample
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //12th coeff & sample
      #endif
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //13th coeff & sample
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //14th coeff & sample
      #endif
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //15th coeff & sample
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //16th coeff & sample
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //17th coeff & sample
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I6,M0]; //18th coeff & sample
      rMAC = rMAC + r0 * r1, r1 = M[I2, -MK1];

      // linear interpolation between two adjacent points
      rMAC = rMAC * r3;
      rMAC = rMAC - r1 * 1.0;
      r2 = 0;
      r1 = rMAC - r2, r0 = M[I2,M3];

      //shift
      r1 = r1 ASHIFT r0;

      /* If r9 == $TRUE then the next input sample is already in the hist buffer
      so don't read it again */

      // load new sample to hist buf
      r4 = r4 - r5, M[I5, M3] = r1;
      if NEG jump no_extra_copy;
         r4 = r4 + (-1.0);
         Null = r5;
         if NEG jump is_neg_rate;
         is_pos_rate:
            // don't read from input buffer as want to use same sample again
            r3 = M[I0, -MK1];
            r3 = M[I0, 0];
            r10 = r10 + 1;
            // Don't modify r9 here, if it's TRUE then we don't need to read
            // again the next time around, if it's FALSE then it's business
            // as usual
            jump end_of_loop;
         is_neg_rate:
            // skip along a sample, but put both into the history buffer
            NULL = r9 - $TRUE;
            if NZ jump neg_not_already_read;
            // The next value was already read previously so just get that from history buffer
            r9 = $FALSE;
            r3 = M[I0, 0];
            jump neg_re_read_complete;
            neg_not_already_read:
            r3 = M[I4, MK1];
            r3 = r3 ASHIFT r7;
            neg_re_read_complete:
            M[I0,M3] = r3, r3 = M[I4,M3];
            r10 = r10 - 1;
            if LE jump end_rts;
            r3 = r3 ASHIFT r7;
            jump end_of_loop;
      no_extra_copy:
      NULL = r9 - $TRUE;
      if NZ jump no_extra_copy_not_already_read;
      // The next value was already read previously so just increment I0
      r9 = $FALSE;
      r3 = M[I0,0];
      jump end_of_loop;
      no_extra_copy_not_already_read:
      r3 = M[I4, MK1];
      r3 = r3 ASHIFT r7;
      end_of_loop:
      rMAC = r4 * r6, M[I0,M3] = r3;
   sra_gen_loop:
   SP = SP - $sra.STACK_VAR_SPACE; // tidy-up stack
   rts;

   end_rts:
   r3 = r3 ASHIFT r7;
   r10 = r10 - 1, M[I0, M3] = r3;
   SP = SP - $sra.STACK_VAR_SPACE; // tidy-up stack
   rts;


// *****************************************************************************
// NAME:
//    $_sra_resample
//
// DESCRIPTION:
//
//  public funtion to resample the input
//
// INPUTS:
//  r0: source buffer array
//  r1: sink buffer array
//  r2: number of channels (array length of r0 & r1)
//  r3: samples to process
//  M[SP -1]: sra_params structure
//
//
// OUTPUTS:
//  None
//
// TRASHED REGISTERS:
//    obeys C calling convention
// *****************************************************************************
$_sra_resample:
   PUSH_ALL_C;

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($audio_proc.SRA_ASM.RESAMPLE.PATCH_ID_0, r8)
#endif

   r8 = M[SP - (PUSH_ALL_C_SP_INC + ADDR_PER_WORD)]; // store the sra_params in r8 for this function and its callees
   r10 = r3;    // samples to process in r10

   pushm <r0, r1, r2>;

   // update target rate
//   r0 = M[r8 + $sra_c.sra_params_struct.SRA_TARGET_RATE_ADDR_FIELD];
//   r0 = M[r0];
//   M[r8 + $sra_c.sra_params_struct.SRA_CURRENT_RATE_FIELD] = r0;

   // slowly move towards the target rate
   r5 = M[r8 + $sra_c.sra_params_struct.SRA_CURRENT_RATE_FIELD];

   // calculate current rate, slowly move towards the target rate
   r0 = M[r8 + $sra_c.sra_params_struct.SRA_TARGET_RATE_ADDR_FIELD];
   r3 = M[r0];

   // logarithmic then linear movement
   r1 = $sra.MOVING_STEP;
   r0 = r5 - r3;
   if NEG r0 = -r0;
   r4 = r0 * 0.001(frac);
   Null = r0 - 0.0015;
   if NEG r4 = r1;
   r1 = r4 - (30*$sra.MOVING_STEP);
   if POS r4 = r4 - r1;
   r1 = r5 - r3;
   r0 = r1 - r4;
   if POS r1 = r1 - r0;
   r0 = r1 + r4;
   if NEG r1 = r1 - r0;
   // update the current rate
   r5 = r5 - r1;

   // store the updated current rate for all channels
   M[r8 + $sra_c.sra_params_struct.SRA_CURRENT_RATE_FIELD] = r5;


   r1 = M[r8 + $sra_c.sra_params_struct.FILTER_COEFFS_SIZE_FIELD];
   Words2Addr(r1);     // coeffs_size (arch4: in addrs)
   L0 = r1;
   L1 = r1;

   // get the coeffs addr
   r0 = M[r8 + $sra_c.sra_params_struct.FILTER_COEFFS_FIELD];

   // I3 is now pointer to the end of coefficient table
   r1 = r1 * ($sra.SRA_UPRATE)(int);
   r1 = r1 LSHIFT -1;  // sra_uprate*coeffs_size/2 (arch4: in addrs)
   I3 = r0 + r1;
   push I3; // Store this address for use by later channels

   // channel count. It's more efficient to decrement as we can test for negative cheaply
   r5 = r2 - 1;

// loop through each channel
per_channel_loop:
   I3 = M[SP - ADDR_PER_WORD];
   rMAC = $sra_c.sra_hist_params_struct.STRUC_SIZE * ADDR_PER_WORD * r5 (int);
   rMAC = rMAC + r8;
   r0 = M[rMAC + $sra_c.sra_n_channel_struct.HIST_BUFFERS_FIELD + $sra_c.sra_hist_params_struct.HIST_BUF_FIELD];
   I0 = r0;
   r0 = M[rMAC + $sra_c.sra_n_channel_struct.HIST_BUFFERS_FIELD + $sra_c.sra_hist_params_struct.HIST_BUF_START_FIELD];
   push r0;
   B0 = M[SP - 1*ADDR_PER_WORD];
   pop B1;

   push r5; // Push the channel index on the stack
   Words2Addr(r5);
   // Setup input buffer indexes
   r0 = M[SP - (5 * ADDR_PER_WORD)];
   r0 = M[r0 + r5];
   call $cbuffer.get_read_address_and_size_and_start_address;
   push r2;
   pop B4;
   I4 = r0;
   L4 = r1;

   // Setup output buffer indexes
   r0 = M[SP - (4 * ADDR_PER_WORD)];
   r0 = M[r0 + r5];
   call $cbuffer.get_write_address_and_size_and_start_address;
   push r2;
   pop B5;
   I5 = r0;
   L5 = r1;

   M3 = MK1;
   // Call $sra_resample_core preserving amount to process
   push r10;
   call $sra_resample_core;


   // adjust hist buffer index
   Words2Addr(r10);
   M1 = r10;
   r0 = M[I0, M1];
   pop r10;

   r5 = M[SP - 1*ADDR_PER_WORD];   // get channel index
   rMAC = $sra_c.sra_hist_params_struct.STRUC_SIZE * ADDR_PER_WORD * r5 (int);
   rMAC = rMAC + r8;
   r0 = I0;
   M[rMAC + $sra_c.sra_n_channel_struct.HIST_BUFFERS_FIELD + $sra_c.sra_hist_params_struct.HIST_BUF_FIELD] = r0;

   Words2Addr(r5);
   // Update output buffer write address
   r0 = M[SP -(4 * ADDR_PER_WORD)];
   r0 = M[r0 + r5];
   r1 = I7;
   call $cbuffer.advance_write_ptr;

   pop r5;
   r5 = r5 - 1;
   if POS jump per_channel_loop;

   // Processed all channels so update channel independent parameters
   M[r8 + $sra_c.sra_params_struct.RF_FIELD] = r4;
   r0 = -M1;
   M[r8 + $sra_c.sra_params_struct.PREV_SHORT_SAMPLES_FIELD] = r0;

   pop I3;
   popm <r0, r1, r2>;

   POP_ALL_C;
   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $audio_proc.sra_coeffs
// DESCRIPTION:
//    sra filter coefficients which are suitable for almost all applications.
//
//    for fs=48000Hz:
//       0-16khz -> flat with maximum +-0.1dB variation
//       18Khz   -> -0.5dB
//       20kHz   -> -2dB loss
//       MIPS ~= 5.5 for stereo rate matching
//    Coeff size = 126 Words
// *****************************************************************************
.MODULE $M.audio_proc.sra_coeffs;
   .DATASEGMENT DM;

   #ifdef SRA_NORMAL_QUALITY_COEFFS
#ifdef KYMERA
      .VAR/DMCONST $sra_coeffs_normal[] =
#else
      .VAR/DM2 $sra_coeffs[] =
#endif
       0.883632587818095 , 0.877877667726512 , 0.866443871047688 , 0.849481980288737 , 0.827214933843787 , 0.799934015475623 , 0.767993891817363,
       0.731806595890197 , 0.691834575082403 , 0.648582939922374 , 0.602591064925419 , 0.554423704489808 , 0.504661795025301 , 0.453893119060371,
       0.402703007918735 , 0.351665256691948 , 0.301333418756658 , 0.252232637167451 , 0.204852157150132 , 0.159638647946650 , 0.116990443804719,
       0.077252793397754 , 0.040714184881428 , 0.007603790649646, -0.021909947828970, -0.047719598401042, -0.069777906321170, -0.088095900668149,
      -0.102740008349253, -0.113828263666210, -0.121525717807619, -0.126039166381943, -0.127611323955546, -0.126514582325473, -0.123044493825721,
      -0.117513122303832, -0.110242402553105, -0.101557644059644, -0.091781307107955, -0.081227168833244, -0.070194984019594, -0.058965730676323,
      -0.047797514074957, -0.036922185420447, -0.026542713104928, -0.016831325999948, -0.007928429929338 , 0.000057719239355 , 0.007050619136037,
       0.013004449615505 , 0.017902388812804 , 0.021754390263427 , 0.024594504206424 , 0.026477833193915 , 0.027477216662542 , 0.027679741147062,
       0.027183172363990 , 0.026092402555440 , 0.024516001409899 , 0.022562951769962 , 0.020339642446051 , 0.017947180067616 , 0.015479070338026,
       0.013019306656501 , 0.010640891182893 , 0.008404800404566 , 0.006359394468313 , 0.004540257298090 , 0.002970443141156 , 0.001661094949487,
       0.000612391150156, -0.000185229915856, -0.000749624153787, -0.001105334463641, -0.001281883918868, -0.001312031206822, -0.001230048348753,
      -0.001070077206441, -0.000864616226647, -0.000643182464165, -0.000431186383555, -0.000249048519276, -0.000111578043379, -0.000027623929975;
   #endif

   #ifdef SRA_HIGH_QUALITY_COEFFS
#ifdef KYMERA
      .VAR/DMCONST $sra_coeffs_high[] =
#else
      .VAR/DM2 $sra_coeffs[] =
#endif
       0.933850970048108,  0.927376126544060,  0.914511820984558,  0.895427410856051,  0.870373445063747,  0.839677551978420,  0.803739076645469,
       0.763022566728372,  0.718050227824640,  0.669393487428124,  0.617663822648689,  0.563503019530097,  0.507573041187022,  0.450545687841049,
       0.393092234076503,  0.335873227239258,  0.279528625921107,  0.224668449038947,  0.171864094333613,  0.121640470445817,  0.074469069404650,
       0.030762086768647, -0.009132324784476, -0.044933605455838, -0.076431102685057, -0.103485134790299, -0.126026812859670, -0.144056653332668,
      -0.157642036120485, -0.166913584022311, -0.172060558195040, -0.173325381166429, -0.170997413040753, -0.165406117884919, -0.156913765619775,
      -0.145907819960789, -0.132793165008258, -0.117984322001971, -0.101897803618222, -0.084944746151719, -0.067523950204076, -0.050015448360978,
      -0.032774704095341, -0.016127530136622, -0.000365797180057,  0.014256014521220,  0.027523387016881,  0.039263445316049,  0.049345750907038,
       0.057682230252107,  0.064226277363341,  0.068971085170613,  0.071947274308839,  0.073219899925373,  0.072884926935664,  0.071065271685768,
       0.067906513119490,  0.063572379257748,  0.058240115096121,  0.052095835985762,  0.045329966306239,  0.038132856936780,  0.030690666897158,
       0.023181584808950,  0.015772454799298,  0.008615859431263,  0.001847699510423, -0.004414702494274, -0.010073961848333, -0.015053453122385,
      -0.019297605668487, -0.022771696300397, -0.025461172986623, -0.027370552611928, -0.028521943674943, -0.028953251036063, -0.028716124422588,
      -0.027873715292209, -0.026498307849666, -0.024668889544640, -0.022468724331287, -0.019982988457056, -0.017296523718799, -0.014491757153211,
      -0.011646829214545, -0.008833964849679, -0.006118113734301, -0.003555877513804, -0.001194733426773,  0.000927444601456,  0.002782574214505,
       0.004352276382166,  0.005627531401830,  0.006608105592530,  0.007301779836839,  0.007723414750961,  0.007893889755449,  0.007838954659420,
       0.007588032572743,  0.007173012067035,  0.006627064588909,  0.005983520285022,  0.005274831747135,  0.004531650863817,  0.003782039124568,
       0.003050826521994,  0.002359128802950,  0.001724027394360,  0.001158411033405,  0.000670973115047,  0.000266354169840, -0.000054585177961,
      -0.000294378846640, -0.000458478309612, -0.000554706700794, -0.000592667859623, -0.000583134482703, -0.000537439076928, -0.000466890185527,
      -0.000382234446548, -0.000293182524230, -0.000208013925388, -0.000133272286110, -0.000073559008735, -0.000031429271243, -0.000007390548200;
   #endif

   #ifdef SRA_VERY_HIGH_QUALITY_COEFFS
#ifdef KYMERA
      .VAR/DMCONST $sra_coeffs_very_high[] =
#else
      .VAR/DM2 $sra_coeffs[] =
#endif
       0.958502046350840,  0.951841359216985,  0.938579214227939,  0.918787305923865,  0.892711389471900,  0.860688528176726,  0.823131354050241,
       0.780521555636404,  0.733402296359882,  0.682369696634284,  0.628063529040696,  0.571157289147113,  0.512347814741953,  0.452344633218247,
       0.391859220451825,  0.331594354715267,  0.272233745975172,  0.214432114415272,  0.158805882358303,  0.105924631132790,  0.056303459110970,
       0.010396359445896, -0.031409283681339, -0.068797002833383, -0.101524304174855, -0.129424441776768, -0.152406901317971, -0.170456608567859,
      -0.183631900861520, -0.192061321567005, -0.195939317887654, -0.195520940867106, -0.191115662837525, -0.183080441486648, -0.171812170982038,
      -0.157739669003361, -0.141315353976600, -0.123006769219455, -0.103288110097750, -0.082631906721566, -0.061501008298427, -0.040341006185406,
      -0.019573221169285,  0.000411633174211,  0.019259014723857,  0.036655265385405,  0.052331410908948,  0.066066005817761,  0.077687000635420,
       0.087072627895856,  0.094151322312472,  0.098900708599705,  0.101345707438297,  0.101555825639179,  0.099641710410962,  0.095751059534399,
       0.090063989000700,  0.082787967134582,  0.074152429302773,  0.064403189961515,  0.053796769030191,  0.042594747450684,  0.031058262408929,
       0.019442746206395,  0.007993004365098, -0.003061281544176, -0.013509552389226, -0.023163810002079, -0.031861448009013, -0.039467441327072,
      -0.045875875027766, -0.051010807542423, -0.054826477095066, -0.057306873515822, -0.058464709950258, -0.058339840203745, -0.056997177341886,
      -0.054524177539725, -0.051027959902573, -0.046632137977762, -0.041473441887542, -0.035698211426914, -0.029458840114100, -0.022910248120973,
      -0.016206458346696, -0.009497344762780, -0.002925615712596,  0.003375912722091,  0.009286706666890,  0.014700530287642,  0.019527059757520,
       0.023693077524703,  0.027143238984777,  0.029840413397098,  0.031765610207081,  0.032917510653180,  0.033311632474606,  0.032979162543355,
       0.031965498197535,  0.030328542856197,  0.028136805082014,  0.025467352590443,  0.022403673775599,  0.019033499156223,  0.015446633789874,
       0.011732849235445,  0.007979880162095,  0.004271566325579,  0.000686175496161, -0.002705062825725, -0.005839189271666, -0.008662494904009,
      -0.011131239430648, -0.013212103403336, -0.014882376867913, -0.016129893004826, -0.016952720906257, -0.017358636667146, -0.017364396324170,
      -0.016994837782146, -0.016281841664944, -0.015263182983189, -0.013981306610656, -0.012482059813768, -0.010813414512435, -0.009024210613309,
      -0.007162949713038, -0.005276665798861, -0.003409896368991, -0.001603773757527,  0.000104747513370,  0.001683515704844,  0.003105651778825,
       0.004349861142487,  0.005400588238418,  0.006248018917125,  0.006887939136027,  0.007321461747539,  0.007554635902489,  0.007597955852406,
       0.007465787648817,  0.007175733388624,  0.006747953236518,  0.006204465477757,  0.005568444341473,  0.004863534322971,  0.004113198271981,
       0.003340114661353,  0.002565637273998,  0.001809328117996,  0.001088571777352,  0.000418276707533, -0.000189333731512, -0.000724840371113,
      -0.001181659855259, -0.001555993522051, -0.001846700367440, -0.002055102703721, -0.002184734580441, -0.002241044128660, -0.002231061705449,
      -0.002163046049627, -0.002046120618849, -0.001889911879328, -0.001704200589432, -0.001498596092555, -0.001282242355528, -0.001063563004883,
      -0.000850050977297, -0.000648106667764, -0.000462926685837, -0.000298443572236, -0.000157315138515, -0.000040960520766,  0.000050361371011,
       0.000117436535303,  0.000161950198502,  0.000186318000491,  0.000193501730957,  0.000186817482321,  0.000169743995230,  0.000145738622633,
       0.000118067747501,  0.000089657683453,  0.000062971098974,  0.000039912871929,  0.000021768042532,  0.000009173233384,  0.000002121589138;
    #endif

.ENDMODULE;

#ifdef KYMERA
// Alias the coeffs tables so that C can see them.
#ifdef SRA_NORMAL_QUALITY_COEFFS
.set $_sra_coeffs_normal,      $sra_coeffs_normal
#endif
#ifdef SRA_HIGH_QUALITY_COEFFS
.set $_sra_coeffs_high,        $sra_coeffs_high
#endif
#ifdef SRA_VERY_HIGH_QUALITY_COEFFS
.set $_sra_coeffs_very_high,   $sra_coeffs_very_high
#endif
#endif // KYMERA



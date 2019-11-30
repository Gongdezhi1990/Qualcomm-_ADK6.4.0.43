// *****************************************************************************
// Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// $****************************************************************************
// NAME:
//    Audio Processing Library PEQ Module (version 2.0.0)
//
// DESCRIPTION:
//    Parametric Equalizer based on multi-stage biquad filter
//
// MODULES:
//    $audio_proc.peq.initialize
//    $audio_proc.peq.process
// *****************************************************************************

#ifndef AUDIO_PEQ_INCLUDED
#define AUDIO_PEQ_INCLUDED

#include "core_library.h"
#include "peq.h"
#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif


// *****************************************************************************
// MODULE: "efficient" single precision feedback PEQ
//    $audio_proc.sh_peq.initialize
//
// DESCRIPTION:
//    Based on the number of stages specified, set the size of PEQ delay buffer
//    and filter coefficient buffer, and clear the PEQ delay buffer.
//
// INPUTS:
//    - r7 = pointer to peq data object, with the below field being set
//       - $audio_proc.peq.NUM_STAGES_FIELD
//       - $audio_proc.peq.DELAYLINE_ADDR_FIELD
//       - $audio_proc.peq.DELAYLINE_START_ADDR_FIELD
//
// OUTPUTS:
//    - none
//
//    - the following fields of data object pointed by r7 being altered:
//       - $audio_proc.peq.DELAYLINE_SIZE_FIELD
//       - $audio_proc.peq.COEFS_SIZE_FIELD
//
//    - delay buffer pointed by $audio_proc.peq.DELAYLINE_START_ADDR_FIELD is zeroed
//
// TRASHED REGISTERS:
//    r0, r1, r10, I0
//
// NOTES:
// *****************************************************************************

.MODULE $M.audio_proc.sh_peq.initialize;
   .CODESEGMENT   AUDIO_PROC_PEQ_INITIALIZE_PM;
   .DATASEGMENT   DM;

$audio_proc.sh_peq.initialize:

   // Pointer to PEQ parameters
   r8 = M[r7 + $audio_proc.peq.PARAM_PTR_FIELD];
   // number of stages (lower LSBs)
   r0 = M[r8 + $audio_proc.peq.parameter.NUM_STAGES_FIELD];
   r0 = r0 AND $audio_proc.peq.const.NUM_STAGES_MASK;
   M[r7 + $audio_proc.peq.NUM_STAGES_FIELD] = r0;

   // size of delay buffer = (num_stage+1)*2
   r1 = r0 + 1;
   r10 = r1 ASHIFT (1 + LOG2_ADDR_PER_WORD);
   M[r7 + $audio_proc.peq.DELAYLINE_SIZE_FIELD] = r10;

   // Delay buffer appended to end of peq data structure
   r1 = r7 + $audio_proc.peq.STRUC_SIZE * ADDR_PER_WORD;
   M[r7 + $audio_proc.peq.DELAYLINE_ADDR_FIELD]=r1;
   M[r7 + $audio_proc.peq.DELAYLINE_START_ADDR_FIELD]=r1;

   // size of coef buffer = (num_stage) * 6
   r10 = r0 * (6 * ADDR_PER_WORD) (int);
   M[r7 + $audio_proc.peq.COEFS_SIZE_FIELD] = r10;
   // Set Pointer to coefficients in parameters
   r0 = r8 + $audio_proc.peq.parameter.STAGES_SCALES;
   M[r7 + $audio_proc.peq.COEFS_ADDR_FIELD] = r0;

   // set the number of headroom bits
   r0 = $audio_proc.peq.SH_PEQ_HEADROOM_SHIFTS;
   M[r7 + $audio_proc.peq.HEADROOM_BITS] = r0;

   rts;

.ENDMODULE;



.MODULE $M.audio_proc.sh_peq.zero_delay_data;
   .CODESEGMENT   AUDIO_PROC_PEQ_INITIALIZE_PM;
   .DATASEGMENT   DM;

$audio_proc.sh_peq.zero_delay_data:

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($audio_proc.PEQ_ASM.SH_PEQ.ZERO_DELAY_DATA.PATCH_ID_0) // af05_CVC_patches1
//#endif

    push r10;

    // size of delay buffer = (max_num_stages+1)*2
    r10 = M[r7 + $audio_proc.peq.MAX_STAGES_FIELD];
    r10 = r10 + 1;
    r10 = r10 ASHIFT 1;
   // Delay buffer appended to end of peq data structure
    r1 = r7 + $audio_proc.peq.STRUC_SIZE*ADDR_PER_WORD;
    // Initialize delay buffer to zero
    I0 = r1;
    r1 = Null;
    do init_dly_ln_loop;
        M[I0, MK1] = r1;
    init_dly_ln_loop:

    pop r10;
    rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $audio_proc.sh_peq.process
//
// DESCRIPTION:
//    Implementation of multi-stage cascaded biquad filters:
//
//       y(n) = (  b0/a0 * x(n) + b1/a0 * x(n-1) + b2/a0 * x(n-2)
//               - a1/a0 * y(n-1) - a2/a0 * y(n-2) ) << scalefactor
//
//    Overall output level is adjusted by specified gain factor.
//
// INPUTS:
//    - r7 = pointer to delay data object, all fields should be initialized
//    - *r4 = sample count (*only for CBOPS entry point)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    rMAC, r0-r6, r10, M0-M1, I0-I5, L0, L1, L4, L5, LOOP
//
// NOTES:
//    The coefs buffers and delay line buffer MUST be circular.
//
//    Upon completion of the process, the pointers of input and output buffer
//    are updated. In case the PEQ function needs to be called multiple times
//    within a loop (Ex. 8 samples at a time within a 64 sample block), this
//    saves the framework re-assigning the pointers before calling the PEQ
//    function again. Even if it does, it wouldnt harm anything.

//
// *****************************************************************************

.MODULE $M.audio_proc.sh_peq.process;
   .CODESEGMENT   AUDIO_PROC_PEQ_PROCESS_PM;
   .DATASEGMENT   DM;

$audio_proc.sh_peq.process:
   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.PEQ_ASM.SH_PEQ.PROCESS.PATCH_ID_0,r1) // af05_CVC_patches1
//#endif

   // Get input buffer
   r0 = M[r7 +  $audio_proc.peq.INPUT_ADDR_FIELD];
#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B4;
#else
   call $frmbuffer.get_buffer;
#endif
   I4 = r0;
   L4 = r1;

   // Set frame size from input
   r4 = r3;
   // Update frame size of output from input
   r0 = M[r7 +  $audio_proc.peq.OUTPUT_ADDR_FIELD];
   call $frmbuffer.set_frame_size;

   // Get output buffer
#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B0;
#else
   call $frmbuffer.get_buffer;
#endif
   I0 = r0;
   L0 = r1;

   pop rLink;

$audio_proc.sh_peq.process_op:
   push r8;
   push r9;
   M1 = MK1;
   I3 = r7+$audio_proc.peq.PARAM_PTR_FIELD;
   M0 = -MK1;

   r10 = Null,      r0 = M[I3,M1];      // I2 = &PARAM_PTR_FIELD[1]
   I2 = r0 + M1,    r0 = M[I3,M1];      // I5,B5 = DELAYLINE_ADDR_FIELD,DELAYLINE_START_ADDR_FIELD
   // I5 = ptr to delay line,
   push r0;
   pop  B5, r0 = M[I3,M1];
   I5 = r0, r0 = M[I3,M1];              // I1,B1 = COEFS_ADDR_FIELD
   // I1 = ptr to coefs buffer,
   push r0;
   pop  B1;
   I1 = r0, r1 = M[I3,M1];              // M3 = NUM_STAGES_FIELD
   M3 = r1, r1 = M[I3,M1];              // L5 = DELAYLINE_SIZE_FIELD

   L5 = r1, r1 = M[I3,M1];              // L1 = COEFS_SIZE_FIELD
   L1 = r1, r1 = M[I2,M1];              // r1 = GAIN_EXPONENT_FIELD
   r0 = M[I2,M1];                       // r8 = GAIN_MANTISA_FIELD
   r8 = r0;
   // Set Gain. Add head room
   r9 = M[r7 + $audio_proc.peq.HEADROOM_BITS];     // load the number of headroom bits
   r6 = r1 - r9;
   r1 = Null;
   Words2Addr(r4);
      // I0/L0/B0 = y                              M0 = -1
      // I1/L1/B1 = coeffs b2,b1,b0,a2,a1,...      M1 = 1
      // I3       = param_ptr                      r4 = sample_ct
      // I4/L4/B4 = x                              r8/r6 = mantissa/exp
      // I5/L5/B5 = history
peq_block_loop:
      r10 = r10 + M3, r0 = M[I4,M1];
      // apply mantisa,Exp to front end gain
      rMAC = r0 * r8, r3 = M[I5,M1];
      // apply exponent, r2=b2, r3=x_N-2
      rMACB = rMAC ASHIFT r6 (56bit), r2 = M[I1,M1];
      do biquad_loop;
         // 1) acc =  b2*x_N-2, r2=b1, r5=x_N-1
         rMAC = r2*r3, r2 = M[I1,MK1], r5 = M[I5,MK1];
         // 2) acc += b1*x_N-1, r2=b0, r3=y_N-2
         rMAC = rMAC + r2*r5, r2 = M[I1,MK1], r3 = M[I5,MK1];
         // 3) acc += b0*x, r2=a2, r5=y_N-1
         rMAC = rMAC + r2*rMACB, r2 = M[I1,MK1], r5 = M[I5,-MK1];
         // 4) acc -= a2*Y_N-2, r0=x, r2=a1
         rMAC = rMAC - r2*r3, r0=r1+rMACB, r2 = M[I1,M1];
         // 5) acc -= a1*Y_N-1, r2=scale, x=r0
         rMAC = rMAC - r2*r5, r2 = M[I1, MK1], M[I5, MK1] = r0;
         // 6) accB = acc << scale, r2=b2
         rMACB = rMAC ASHIFT r2 (56bit), r2 = M[I1, M1];
      biquad_loop:

      // dummy MAC instruction,  r0=Y(last stage), increment I5, backup I1
      rMAC = rMAC+r1*r1, r0=r1+rMACB, r2 = M[I1,M0], r5 = M[I5,M1];
      // Restore Head room -can be eliminated if headroom=0(replace rMAC w/r0 in store below)
      rMAC = rMACB ASHIFT r9(56bit);
      // Decrement the block counter,write back o/p sample, store new y(n-1)
      r4 = r4 - M1,  M[I0,M1] = rMAC, M[I5,M1] = r0;
   if NZ jump peq_block_loop;

   // save I5
   r0 = I5;
   M[r7 + $audio_proc.peq.DELAYLINE_ADDR_FIELD] = r0;

   // clear L-regs
   L0 = Null;
   L4 = Null;
   L1 = Null;
   L5 = Null;
   // clear base registers
   push Null;
#ifdef BASE_REGISTER_MODE
   B4 = M[SP-MK1];
   B0 = M[SP-MK1];
#endif
   B1 = M[SP-MK1];
   pop B5;
   pop r9;
   pop r8;
   rts;

.ENDMODULE;



// "legacy" single precision feedback PEQ modules, using non-interleaved coefficient object format
// for compatibilty only.... All new PEQ designs should use the above sh_peq.xxx modules
// (or the dh_peq/hq_peq modules)

// *****************************************************************************
// MODULE:
//    $audio_proc.peq.initialize
//
// DESCRIPTION:
//    Based on the number of stages specified, set the size of PEQ delay buffer
//    and filter coefficient buffer, and clear the PEQ delay buffer.
//
// INPUTS:
//    - r7 = pointer to peq data object, with the below field being set
//       - $audio_proc.peq.NUM_STAGES_FIELD
//       - $audio_proc.peq.DELAYLINE_ADDR_FIELD
//       - $audio_proc.peq.DELAYLINE_START_ADDR_FIELD
//
// OUTPUTS:
//    - none
//
//    - the following fields of data object pointed by r7 being altered:
//       - $audio_proc.peq.DELAYLINE_SIZE_FIELD
//       - $audio_proc.peq.COEFS_SIZE_FIELD
//
//    - delay buffer pointed by $audio_proc.peq.DELAYLINE_START_ADDR_FIELD is zeroed
//
// TRASHED REGISTERS:
//    r0, r1, r10, I0
//
// NOTES:
// *****************************************************************************


.MODULE $M.audio_proc.peq.initialize;
   .CODESEGMENT   AUDIO_PROC_PEQ_INITIALIZE_PM;
   .DATASEGMENT   DM;

$audio_proc.peq.initialize:
//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.PEQ_ASM.PEQ.INITIALIZE.PATCH_ID_0,r10) // af05_CVC_patches1
//#endif

   // Pointer to PEQ parameters
   r8 = M[r7 + $audio_proc.peq.PARAM_PTR_FIELD];
   // number of stages (lower LSBs)
   r0 = M[r8 + $audio_proc.peq.parameter.NUM_STAGES_FIELD];
   r0 = r0 AND $audio_proc.peq.const.NUM_STAGES_MASK;
   M[r7 + $audio_proc.peq.NUM_STAGES_FIELD] = r0;

   // size of delay buffer = (num_stage+1)*2
   r1 = r0 + 1;
   r10 = r1 ASHIFT (1 + LOG2_ADDR_PER_WORD);
   M[r7 + $audio_proc.peq.DELAYLINE_SIZE_FIELD] = r10;

   // Delay buffer appended to end of peq data structure
   r1 = r7 + $audio_proc.peq.STRUC_SIZE * ADDR_PER_WORD;
   M[r7 + $audio_proc.peq.DELAYLINE_ADDR_FIELD]=r1;
   M[r7 + $audio_proc.peq.DELAYLINE_START_ADDR_FIELD]=r1;

   // size of coef buffer = (num_stage) * 5
   r10 = r0 * (5 * ADDR_PER_WORD) (int);
   M[r7 + $audio_proc.peq.COEFS_SIZE_FIELD] = r10;
   // Set Pointer to coefficients in parameters
   r0 = r8 + $audio_proc.peq.parameter.STAGES_SCALES;
   M[r7 + $audio_proc.peq.COEFS_ADDR_FIELD] = r0;

   // set the number of headroom bits
   r0 = $audio_proc.peq.LEGACY_PEQ_HEADROOM_SHIFTS;
   M[r7 + $audio_proc.peq.HEADROOM_BITS] = r0;

   rts;

.ENDMODULE;



.MODULE $M.audio_proc.peq.zero_delay_data;
   .CODESEGMENT   AUDIO_PROC_PEQ_INITIALIZE_PM;
   .DATASEGMENT   DM;

$audio_proc.peq.zero_delay_data:

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($audio_proc.PEQ_ASM.PEQ.ZERO_DELAY_DATA.PATCH_ID_0) // af05_CVC_patches1
//#endif

    push r10;

    // size of delay buffer = (max_num_stages+1)*2
    r10 = M[r7 + $audio_proc.peq.MAX_STAGES_FIELD];
    r10 = r10 + 1;
    r10 = r10 ASHIFT 1;
   // Delay buffer appended to end of peq data structure
    r1 = r7 + $audio_proc.peq.STRUC_SIZE*ADDR_PER_WORD;
    // Initialize delay buffer to zero
    I0 = r1;
    r1 = Null;
    do init_dly_ln_loop;
        M[I0, MK1] = r1;
    init_dly_ln_loop:

    pop r10;
    rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $audio_proc.peq.process
//
// DESCRIPTION:
//    Implementation of multi-stage cascaded biquad filters:
//
//       y(n) = (  b0/a0 * x(n) + b1/a0 * x(n-1) + b2/a0 * x(n-2)
//               - a1/a0 * y(n-1) - a2/a0 * y(n-2) ) << scalefactor
//
//    Overall output level is adjusted by specified gain factor.
//
// INPUTS:
//    - r7 = pointer to delay data object, all fields should be initialized
//    - *r4 = sample count (*only for CBOPS entry point)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    rMAC, r0-r6, r10, M0-M2, I0-I5, L0, L1, L4, L5, LOOP
//
// NOTES:
//    The coefs buffers and delay line buffer MUST be circular.
//
//    Upon completion of the process, the pointers of input and output buffer
//    are updated. In case the PEQ function needs to be called multiple times
//    within a loop (Ex. 8 samples at a time within a 64 sample block), this
//    saves the framework re-assigning the pointers before calling the PEQ
//    function again. Even if it does, it wouldnt harm anything.

//
// *****************************************************************************

.MODULE $M.audio_proc.peq.process;
   .CODESEGMENT   AUDIO_PROC_PEQ_PROCESS_PM;
   .DATASEGMENT   DM;

$audio_proc.peq.process:

   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.PEQ_ASM.PEQ.PROCESS.PATCH_ID_0,r1) // af05_CVC_patches1
#endif
   // Get input buffer
   r0 = M[r7 +  $audio_proc.peq.INPUT_ADDR_FIELD];
#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B4;
#else
   call $frmbuffer.get_buffer;
#endif
   I4 = r0;
   L4 = r1;

   // Set frame size from input
   r4 = r3;
   // Update frame size of output from input
   r0 = M[r7 +  $audio_proc.peq.OUTPUT_ADDR_FIELD];
   call $frmbuffer.set_frame_size;

   // Get output buffer
#ifdef BASE_REGISTER_MODE
   call $frmbuffer.get_buffer_with_start_address;
   push r2;
   pop  B0;
#else
   call $frmbuffer.get_buffer;
#endif
   I0 = r0;
   L0 = r1;

   pop rLink;

$audio_proc.peq.process_op:
   push r8;
   push r9;
//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.PEQ_ASM.PEQ.PROCESS.PATCH_ID_1,r10) // af05_CVC_patches1
//#endif
   M1 = MK1;
   I3 = r7+$audio_proc.peq.PARAM_PTR_FIELD;
   M0 = -MK1;

   r10 = Null,      r0 = M[I3,M1];      // I2 = &PARAM_PTR_FIELD[1]
   I2 = r0 + M1,    r0 = M[I3,M1];      // I5,B5 = DELAYLINE_ADDR_FIELD,DELAYLINE_START_ADDR_FIELD
   // I5 = ptr to delay line,
   push r0;
   pop  B5, r0 = M[I3,M1];
   I5 = r0, r0 = M[I3,M1];              // I1,B1 = COEFS_ADDR_FIELD
   // I1 = ptr to coefs buffer,
   push r0;
   pop  B1;
   I1 = r0, r1 = M[I3,M1];              // M2 = NUM_STAGES_FIELD
   M3 = r1;
   Words2Addr(r1);                      // M2 *= MK1 (on arch4)
   M2 = r1, r1 = M[I3,M1];              // L5 = DELAYLINE_SIZE_FIELD
   L5 = r1, r1 = M[I3,M1];              // L1 = COEFS_SIZE_FIELD
   L1 = r1, r1 = M[I2,M1];              // r1 = GAIN_EXPONENT_FIELD
   r0 = M[I2,M1];                       // r8 = GAIN_MANTISA_FIELD
   r8 = r0;
   // Set Gain. Add head room
   r9 = M[r7 + $audio_proc.peq.HEADROOM_BITS];     // load the number of headroom bits
   r6 = r1 - r9;
   r1 = Null;
   // Set Ptr to scale factors
   r2 = M[r7 + $audio_proc.peq.MAX_STAGES_FIELD];
   r2 = r2 * (5*ADDR_PER_WORD) (int);
   I2 = I2 + r2;                        // Jump over coefficients
   I2 = I2 + M2;                        // end of scale factors (reset to beginnng below)
   // needed for bug in index feed forward, M2 = -num stages
   M2 = Null - M2;
   Words2Addr(r4);
      // I0/L0/B0 = y                              M0 = -1
      // I1/L1/B1 = coeffs b2,b1,b0,a2,a1,...      M1 = 1
      // I2       = scalefactor                    M2 = -num stages
      // I3       = param_ptr                      r4 = sample_ct
      // I4/L4/B4 = x                              r8/r6 = mantissa/exp
      // I5/L5/B5 = history
peq_block_loop:
      r10 = r10 + M3, r0 = M[I4,M1];
      // apply mantisa,Exp to front end gain, backup scalefactor pointer
      rMAC = r0 * r8, r0 = M[I2,M2], r3 = M[I5,M1];
      // apply exponent, r2=b2, r3=x_N-2
      rMACB = rMAC ASHIFT r6 (56bit), r2 = M[I1,M1];
      do biquad_loop;
         // 1) acc =  b2*x_N-2, r2=b1, r5=x_N-1
         rMAC = r2*r3, r2 = M[I1,MK1], r5 = M[I5,MK1];
         // 2) acc += b1*x_N-1, r2=b0, r3=y_N-2
         rMAC = rMAC + r2*r5, r2 = M[I1,MK1], r3 = M[I5,MK1];
         // 3) acc += b0*x, r2=a2, r5=y_N-1
         rMAC = rMAC + r2*rMACB, r2 = M[I1,MK1], r5 = M[I5,-MK1];
         // 4) acc -= a2*Y_N-2, r0=x, r2=a1
         rMAC = rMAC - r2*r3, r0=r1+rMACB, r2 = M[I1,M1];
         // 5) acc -= a1*Y_N-1, r2=scale, x=r0
         rMAC = rMAC - r2*r5, r2 = M[I2, MK1], M[I5, MK1] = r0;
         // 6) accB = acc << scale, r2=b2
         rMACB = rMAC ASHIFT r2 (56bit), r2 = M[I1, M1];
      biquad_loop:

      // dummy MAC instruction,  r0=Y(last stage), increment I5, backup I1
      rMAC = rMAC+r1*r1, r0=r1+rMACB, r2 = M[I1,M0], r5 = M[I5,M1];
      // Restore Head room -can be eliminated if headroom=0(replace rMAC w/r0 in store below)
      rMAC = rMACB ASHIFT r9(56bit);
      // Decrement the block counter,write back o/p sample, store new y(n-1)
      r4 = r4 - M1,  M[I0,M1] = rMAC, M[I5,M1] = r0;
   if NZ jump peq_block_loop;

   // save I5
   r0 = I5;
   M[r7 + $audio_proc.peq.DELAYLINE_ADDR_FIELD] = r0;

   // clear L-regs
   L0 = Null;
   L4 = Null;
   L1 = Null;
   L5 = Null;
   // clear base registers
   push Null;
#ifdef BASE_REGISTER_MODE
   B4 = M[SP-MK1];
   B0 = M[SP-MK1];
#endif
   B1 = M[SP-MK1];
   pop B5;
   pop r9;
   pop r8;
   rts;

.ENDMODULE;




#endif // AUDIO_PEQ_INCLUDED

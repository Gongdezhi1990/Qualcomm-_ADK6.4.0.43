// *****************************************************************************
// Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
// Part of ADK 3.5
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
//    $audio_proc.hq_peq.initialize
//    $audio_proc.hq_peq.process
// *****************************************************************************

#ifndef AUDIO_HQ_PEQ_INCLUDED
#define AUDIO_HQ_PEQ_INCLUDED

#include "core_library.h"
#include "peq.h"
#ifdef PATCH_LIBS
   #include "patch_library.h"
#endif


// *****************************************************************************
// MODULE:
//    $audio_proc.hq_peq.initialize
//
// DESCRIPTION:
//    Based on the number of stages specified, set the size of PEQ delay buffer
//    and filter coefficient buffer, and clear the PEQ delay buffer.
//
// INPUTS:
//    - r7 = pointer to peq data object, with the below field being set
//       - $audio_proc.peq.NUM_STAGES_FIELD
//       - $audio_proc.peq.DELAYLINE_ADDR_FIELD
//
// OUTPUTS:
//    - none
//
//    - the following fields of data object pointed by r7 being altered:
//       - $audio_proc.peq.DELAYLINE_SIZE_FIELD
//       - $audio_proc.peq.COEFS_SIZE_FIELD
//
//    - delay buffer pointed by $audio_proc.peq.DELAYLINE_ADDR_FIELD is zeroed
//
// TRASHED REGISTERS:
//    r0, r1, r10, I0
//
// NOTES:
// *****************************************************************************


.MODULE $M.audio_proc.hq_peq.initialize;
   .CODESEGMENT   AUDIO_PROC_PEQ_INITIALIZE_PM;
   .DATASEGMENT   DM;

$audio_proc.hq_peq.initialize:

   // Pointer to PEQ parameters
   r8 = M[r7 + $audio_proc.peq.PARAM_PTR_FIELD];
   // number of stages (lower LSBs)
   r0 = M[r8 + $audio_proc.peq.parameter.NUM_STAGES_FIELD];
   r0 = r0 AND $audio_proc.peq.const.NUM_STAGES_MASK;
   M[r7 + $audio_proc.peq.NUM_STAGES_FIELD] = r0;

   // size of delay buffer = 2+(num_stage*3)
   r10 = r0 * 3 (int);
   r10 = r10 + 2;
   r10 = r10 * ADDR_PER_WORD (int);
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
   r0 = $audio_proc.peq.HQ_PEQ_HEADROOM_SHIFTS;
   M[r7 + $audio_proc.peq.HEADROOM_BITS] = r0;
   
   rts;

.ENDMODULE;



.MODULE $M.audio_proc.hq_peq.zero_delay_data;
   .CODESEGMENT   AUDIO_PROC_PEQ_INITIALIZE_PM;
   .DATASEGMENT   DM;

$audio_proc.hq_peq.zero_delay_data:

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($audio_proc.HQ_PEQ_ASM.HQ_PEQ.ZERO_DELAY_DATA.PATCH_ID_0) // af05_CVC_patches1
//#endif


    push r10;

    r10 = M[r7 + $audio_proc.peq.MAX_STAGES_FIELD];
    // size of delay buffer = 2+(num_stage*3)
    r10 = r10 * 3 (int);
    r10 = r10 + 2;
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
//    $audio_proc.hq_peq.process
//
// DESCRIPTION:
//    Implementation of multi-stage cascaded biquad filters:
//
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
// *****************************************************************************

.MODULE $M.audio_proc.hq_peq.process;
   .CODESEGMENT   AUDIO_PROC_PEQ_PROCESS_PM;
   .DATASEGMENT   DM;

$audio_proc.hq_peq.process:
   push rLink;

//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.HQ_PEQ_ASM.HQ_PEQ.PROCESS.PATCH_ID_0,r1) // af05_CVC_patches1, push rLink forward for the sake of consistency
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

$audio_proc.hq_peq.process_op:
// INPUTS:
//      I4,L4,B4  - Input Buffer
//      I0,L0,B0  - Output Buffer
//      r4        - Number of samples
//      r7        - PEQ data structure

   M1 = MK1;
   I3 = r7+$audio_proc.peq.PARAM_PTR_FIELD;
   M0 = -MK1;

   r10 = Null,      r0 = M[I3,M1];      // I2 = &PARAM_PTR_FIELD[1]
   I2 = r0 + M1,    r0 = M[I3,M1];      // I5,B5 = DELAYLINE_ADDR_FIELD
   // I5 = ptr to delay line,

   push r0;
   pop  B5, r0 = M[I3,M1];

   I5 = r0, r0 = M[I3,M1];              // I1,B1 = COEFS_ADDR_FIELD
   // I1 = ptr to coefs buffer,

   push r0;
   pop  B1;

   I1 = r0, r1 = M[I3,M1];              // M2 = NUM_STAGES_FIELD
   M2 = r1, r1 = M[I3,M1];              // L5 = DELAYLINE_SIZE_FIELD
   L5 = r1, r1 = M[I3,M1];              // L1 = COEFS_SIZE_FIELD
   L1 = r1, r1 = M[I2,M1];              // r1 = GAIN_EXPONENT_FIELD
   r5 = M[I2,M1];                       // r5 = GAIN_MANTISA_FIELD
   // Set Gain. Add headroom
   r9 = M[r7 + $audio_proc.peq.HEADROOM_BITS];     // load the number of headroom bits
   r6 = r1 - r9;
   // needed for bug in index feed forward, M2 = -num stages
   M2 = Null - M2;
   Words2Addr(r4); 
   // this loop excutes for each sample in the block
hq_peq_block_loop:
        // get new input sample
        push r4;
        // number of Biquad stages used, get new input sample
        r10 = r10 - M2, r0 = M[I4,M1];
        // Apply mantisa,Exp to front end gain
        rMAC = r0 * r5;
        r0 = rMAC ASHIFT r6;

        m3 = 3*ADDR_PER_WORD;
       
        r2 = m[i5,+MK1];
        r1 = m[i5,+MK1];
        do hq_biquad_loop;
            rmac = 0,                   r3 = m[i5,-MK1],      r4 = m[i1,+MK1];
            rmac0 = r3;
            rmac = rmac + r2 * r4,      m[i5,-MK1] = r0,      r4 = m[i1,+MK1];
            rmac = rmac + r1 * r4,      m[i5,m3] = r1,      r4 = m[i1,m1];  // +3   +1
            rmac = rmac + r0 * r4,      r2 = m[i5,+MK1],      r4 = m[i1,+MK1];
            rmac = rmac - r2 * r4,      r1 = m[i5,-MK1],      r4 = m[i1,+MK1];
            rmac = rmac - r1 * r4,      r2 = m[i5,-MK1],      r4 = m[i1,+MK1];
            r3 = rmac0;
            rmac0 = null;
            r0 = rmac ashift r4;
            m[i5,m3] = r3;
        hq_biquad_loop:
        r3 = m[i5,-MK1];      // dummy read
        r3 = m[i5,-MK1];      // dummy read
        m[i5,+MK1] = r1;
        m[i5,+MK1] = r0;
        
        // Restore headroom
        r0 = r0 ASHIFT r9;
        pop r4;
        // Decrement the block counter,write back o/p sample
        r4 = r4 - M1,  M[I0,M1] = r0;

   if NZ jump hq_peq_block_loop;

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

   rts;

.ENDMODULE;

#endif // AUDIO_HQ_PEQ_INCLUDED

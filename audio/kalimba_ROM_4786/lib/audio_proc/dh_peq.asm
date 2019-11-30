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
// MODULE:
//    $audio_proc.dh_peq.initialize
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


.MODULE $M.audio_proc.dh_peq.initialize;
   .CODESEGMENT   AUDIO_PROC_PEQ_INITIALIZE_PM;
   .DATASEGMENT   DM;

$audio_proc.dh_peq.initialize:

   // Pointer to PEQ parameters
   r8 = M[r7 + $audio_proc.peq.PARAM_PTR_FIELD];
   // number of stages (lower LSBs)
   r0 = M[r8 + $audio_proc.peq.parameter.NUM_STAGES_FIELD];
   r0 = r0 AND $audio_proc.peq.const.NUM_STAGES_MASK;
   M[r7 + $audio_proc.peq.NUM_STAGES_FIELD] = r0;

   // size of delay buffer = (num_stage+1)*2
   r1 = r0 + 1;
   r10 = r1 ASHIFT (2 + LOG2_ADDR_PER_WORD);
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
   r0 = $audio_proc.peq.DH_PEQ_HEADROOM_SHIFTS;
   M[r7 + $audio_proc.peq.HEADROOM_BITS] = r0;

   rts;

.ENDMODULE;
   


.MODULE $M.audio_proc.dh_peq.zero_delay_data;
   .CODESEGMENT   AUDIO_PROC_PEQ_INITIALIZE_PM;
   .DATASEGMENT   DM;

$audio_proc.dh_peq.zero_delay_data:

//#if defined(PATCH_LIBS)
//   LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT($audio_proc.DH_PEQ_ASM.DH_PEQ.ZERO_DELAY_DATA.PATCH_ID_0) // dh_peq_patches1
//#endif


    push r10;

    // size of delay buffer = (max_num_stages+1)*2
    r10 = M[r7 + $audio_proc.peq.MAX_STAGES_FIELD];
    r10 = r10 + 1;
    r10 = r10 ASHIFT 2;
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
//    $audio_proc.dh_peq.process
//
// DESCRIPTION:
//    Implementation of multi-stage cascaded biquad filters, using double-precision
//    arithmetic on x and y history values:
//
//    y(n) = 
// [ ( b0*x_L(n) + b1*x_L(n-1) + b2*x_L(n-2) - a1*y_L(n-1) - a2*y_L(n-2) ) >> 24
// + ( b0*x_H(n) + b1*x_H(n-1) + b2*x_H(n-2) - a1*y_H(n-1) - a2*y_H(n-2) ) ] << scalefactor
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
//    rMAC, r0-r6, r10, M0-M2, I0-I6, L0, L1, L4, L5, LOOP
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
//    Notes for dh_peq:
//       The dh_peq variant interprets each x and y value in the hisotory buffer as
//       a double precision value split into a low and high word. 
//       The difference equation (listed above) uses both the low and high
//       history components in the output calculation, reducing quanitztion error in the 
//       output (at the expense of more cycles than the standard peq process function)
//
//    Cycles: ~50 + #samples * [ 14 + (16*#stages) ], 5stages@48k = ~4.5 MIPS
//       
//
// *****************************************************************************


.MODULE $M.audio_proc.dh_peq.process;
   .CODESEGMENT   AUDIO_PROC_PEQ_PROCESS_PM;
   .DATASEGMENT   DM;

$audio_proc.dh_peq.process:

   push rLink;
//#if defined(PATCH_LIBS)
//   LIBS_SLOW_SW_ROM_PATCH_POINT($audio_proc.DH_PEQ_ASM.DH_PEQ.PROCESS.PATCH_ID_0,r1) // dh_peq_patches1, push rLink forward for the sake of consistency
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

$audio_proc.dh_peq.process_op:
   push r8;
   push r9;
   Words2Addr(r4);
   I6 = r4;
   M0 = -4*MK1;
   M1 = 1*MK1;
   M2 = -3*MK1;

   I3 = r7+$audio_proc.peq.PARAM_PTR_FIELD;

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
   r8 = r0, r2 = M[I1, M1];
   // Set Gain. Add head room
   r9 = M[r7 + $audio_proc.peq.HEADROOM_BITS];     // load the number of headroom bits
   r6 = r1 - r9;
   push r7;
   // I0/L0/B0 = y                              M0 = -4
   // I1/L1/B1 = coeffs b2,b1,b0,a2,a1,...      M1 = 1
   // I3       = param_ptr                      M2 = -3
   // I4/L4/B4 = x                              I6 = sample_ct     
   // I5/L5/B5 = history                        r8/r6 = mantissa/exp  
peq_block_loop:       
      // I5 = &x_N-2_L
      //  r10 = sample_ct, r0 = xN, r1 = xN-2_L
      r10 = r10 + M3, rMAC = M[I4,M1];
      // apply mantisa,Exp to front end gain, r3 = xN-2_H
      rMAC = rMAC * r8, r1 = M[I5,M1];
      rMAC = rMAC ASHIFT r6 (56bit), r3 = M[I5,M1];
      // r4/r7 = x
      r4 = rMAC ASHIFT Null;
      r7 = rMAC LSHIFT DAWTH;
      // r5=yN-1_L, r2=b2, r7/r4 = x_L/x_H, r1/r3 = x_N-2_L/x_N-2_H, I5=&x_N-1_L
      do biquad_loop;
         // acc = b2*x_N-2_L, r2=b1, r1=x_N-1_L                 
         rMAC = r2*r1(SU), r2=M[I1,MK1],  r5 = M[I5,MK2];
         // acc = b1*x_N-1_L, r2=b0
         rMAC = rMAC + r2*r5(SU), r2 = M[I1,MK1];
         // acc += b0*x_L, r2=a2, r1=y_N-2_L
         rMAC = rMAC + r2*r7(SU), r2 = M[I1,MK1], r1 = M[I5,MK2];
         // acc -= a2*y_N-2_L, r2=a1, r5=y_N-1_L
         rMAC = rMAC - r2*r1(SU), r2 = M[I1,M0], r5 = M[I5,M2];
         // acc -= a1*y_N-1_L, r2=b2, r5=x_N-1_H
         rMAC = rMAC - r2*r5(SU), r2 = M[I1,MK1], r5 = M[I5, MK2];
         // acc = acc >> DAWTH
         rMAC = rMAC ASHIFT -DAWTH (56bit);
         r0 = r7;
         // acc +=  b2*x_N-2_H, r2=b1, r3=y_N-2_H                 
         rMAC = rMAC + r2*r3, r2 = M[I1,MK1], r3 = M[I5,-MK1];
         // acc +=  b1*x_N-1_H, r2=b0, x_L = r0
         rMAC = rMAC + r2*r5, r2 = M[I1,MK1], M[I5, MK1] = r0;
         // acc +=  b0*x_H, r2=a2,  x_H=r4
         rMAC = rMAC + r2*r4, r2 = M[I1,MK1], M[I5, MK2] = r4;
         // acc -=  a2*Y_N-2_H, r2=a1,  r5=y_N-1_H
         rMAC = rMAC - r2*r3, r2 = M[I1,MK1], r5 = M[I5,-MK1];
         // acc -=  a1*Y_N-1_H, r2=scale
         rMAC = rMAC - r2*r5, r2 = M[I1,MK1];
         // accB =  acc << scale, r2=b2 (next stage)
         rMAC = rMAC ASHIFT r2 (56bit), r2 = M[I1, M1];
         // get y_H
         r4 = rMAC ASHIFT Null;
         // get y_L
         r7 = rMAC LSHIFT DAWTH;
      biquad_loop:
      // dummmy read to advance I5 to &y_N_L  
      r0 = M[I5, MK2]; 
      // get y_H/y_L and store
      r0 = rMAC ASHIFT Null;
      r1 = rMAC LSHIFT DAWTH;
      // Restore Head room -can be eliminated if headroom=0(replace rMAC w/r0 in store below)
      rMAC = rMAC ASHIFT r9 (56bit), M[I5,MK1] = r1;
      // Decrement the block counter,write back o/p sample, store new y(n-1)
      I6 = I6 - M1,  M[I0,M1] = rMAC, M[I5,M1] = r0;
   if NZ jump peq_block_loop;
   pop r7;
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

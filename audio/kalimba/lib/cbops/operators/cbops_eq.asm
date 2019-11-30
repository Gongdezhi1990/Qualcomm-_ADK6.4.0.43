// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    EQ Operator
//
// DESCRIPTION:
//
//   This routine implements a cascaded structure of NUM_STAGES
//   biquad filters.
//
//
//  Each filter uses the Direct Form I implementation described by the
//  following difference equation.
//
//    y(n) = ( b0/a0 * x(n) + b1/a0 * x(n-1) + b2/a0 * x(n-2)
//                       - a1/a0 * y(n-1) - a2/a0 * y(n-2) ) << scalefactor
//
//  This routine processes a block of samples defined by BLOCK_SIZE.
//  In the operator format, the BLOCK_SIZE field is ignored and r10 is used
//  the BLOCK_SIZE. Remember that r10 is an input to the operator.
//
//  Please note that the coefficient order is b0,b1,b2,a2,a1.
//
// CPU USAGE:
//
//        cycles = BLOCK_SIZE * ( 12 + (7 * NUM_STAGES) )
//
//
// *****************************************************************************
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.eq;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.eq[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.eq.reset,              // reset function
      $cbops.function_vector.NO_FUNCTION,   // amount to use function
      &$cbops.eq.main;               // main function

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.eq.reset
//
// DESCRIPTION:
//    Reset routine for the EQ operator, see $cbops.eq.main
//
// INPUTS:
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    none
//
// *****************************************************************************
.MODULE $M.cbops.eq.reset;
   .CODESEGMENT CBOPS_EQ_RESET_PM;
   .DATASEGMENT DM;

   // ** reset routine **
   $cbops.eq.reset:

	r0  = M[r8 + $cbops.eq.NUM_STAGES_FIELD];	//number of stages

	// size of delay buffer = (num_stage+1)*2
	r1 = r0 + 1;
	r1 = r1 ASHIFT 1;
	M[r8 + $cbops.eq.DELAY_BUF_SIZE] = r1;

	// size of coef buffer = (num_stage) * 5
	r1 = r0 ASHIFT 2;
	r1 = r1 + r0;
	M[r8 + $cbops.eq.COEFF_BUF_SIZE] = r1;

	r1  = M[r8 + $cbops.eq.PTR_DELAY_LINE_FIELD];
	I0  = r1;
	r10 = M[r8 + $cbops.eq.DELAY_BUF_SIZE];
	r0  = 0;   // to zero the delay buffer
	do init_dly_ln_loop;
		M[I0,1] = r0;
  	init_dly_ln_loop:

   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.eq.main
//
// DESCRIPTION:
//    Operator that performs EQ on the input data
//
// INPUTS:
//    - r6 = pointer to the list of input and output buffer pointers
//    - r7 = pointer to the list of buffer lengths
//    - r8 = pointer to operator structure
//    - r10 = the number of samples to process
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    rMAC, r0-4, r10, I0, L0, I4, L4, DoLoop
//
// *****************************************************************************
.MODULE $M.cbops.eq.main;
   .CODESEGMENT CBOPS_EQ_MAIN_PM;
   .DATASEGMENT DM;

   $cbops.eq.main:

   // start profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_eq[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      push rLink;
      r0 = &$cbops.profile_eq;
      call $profiler.start;
   #endif

   // get the offset to the read buffer to use
   r0 = M[r8 + $cbops.eq.INPUT_START_INDEX_FIELD];

   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I4
   I4 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L4
   L4 = r1;

   // get the offset to the write buffer to use
   r0 = M[r8 + $cbops.eq.OUTPUT_START_INDEX_FIELD];

   // get the output buffer write address
   r1 = M[r6 + r0];
   // store the value in I0
   I0 = r1;
   // get the output buffer length
   r1 = M[r7 + r0];
   // store the value in L0
   L0 = r1;

    M1 = 1;
    M0 = -1;
    I3 = r8 + $cbops.eq.PTR_DELAY_LINE_FIELD;
    r0 = M[I3,M1];
    I5 = r0,        r0 = M[I3,M1];                              // I5 = ptr to delay line,      Read PTR_COEFS_BUFF_FIELD
    I1 = r0,        r1 = M[I3,M1];                              // I1 = ptr to coefs buffer,    Read NUM_STAGES_FIELD
    M2 = r1,        r1 = M[I3,M1];                              // M2 = num stages,             Read DELAY_BUF_SIZE
    L5 = r1,        r1 = M[I3,M1];                              // L5 = delay buffer size,      Read COEFF_BUF_SIZE
    L1 = r1,        r4 = M[I3,M1];                              // L1 = coeff buffer size,      Read BLOCK_SIZE_FIELD
    r1 = M[I3,M1];                                              // Read PTR_SCALE_BUFF_FIELD
    I7 = r1,        r1 = M[I3,M1];                              // I7 = scale buffer,           Read INPUT_GAIN_EXPONENT_PTR

    r6 = M[r1];                                                 // INPUT_GAIN_EXPONENT
    r6 = r6 + M0,   r5 = M[I3,M1];                              // Add 2-bit head room          Read INPUT_GAIN_MANTISA_PTR
    r6 = r6 + M0;
    r5 = M[r5];                                                 // INPUT_GAIN_MANTISA

    r4 = r10;                                                   // not going to use the BLOCK SIZE field
eq_block_loop:                                                  // this loop excutes for each sample in the block
    r0 = M[I4,M1];                                              // get new input sample
    // Apply mantisa,Exp to front end gain
    rMAC = r0 * r5;
    r0   = rMAC ASHIFT r6;

    I2 = I7;
    r10 = M2;        // number of Biquad stages used
    do biquad_loop;
        r1 = M[I5,M1], r2 = M[I1,M1];                           // get x(n-2), get coef b2
        rMAC = r1 * r2, r1 = M[I5,M0], r2 = M[I1,M1];           // b2*x(n-2), get x(n-1), get coef b1
        rMAC = rMAC + r1 * r2, M[I5,M1] = r1, r2 = M[I1,M1];    // +b1*x(n-1), store new x(n-2), get coef b0
        rMAC = rMAC + r0 * r2, M[I5,M1] = r0;                   // +b0*x(n),store new x(n-1)
        r1 = M[I5,M1],  r2 = M[I1,M1];                          // get y(n-2), get coef a2
        rMAC = rMAC - r1 * r2, r1 = M[I5,M0], r2 = M[I1,M1];    // -a2*y(n-2), get y(n-1), get coef a1
        rMAC = rMAC - r1 * r2, r3 = M[I2,M1];                   // -a1*y(n-1),get the scalefactor
        r0 = rMAC ASHIFT r3;                                    // get y(n)
    biquad_loop:
    M[I5,M1] = r1;                                              // store new y(n-2)
    M[I5,M1] = r0;                                              // store new y(n-1)

    r0 = r0 ASHIFT 2;                                           // Restore Head room
    r4 = r4 - M1, M[I0,M1] = r0;                                // Decrement the block counter,write back o/p sample
    if NZ jump eq_block_loop;

    // Clear L registers
    L0 = Null;
    L4 = Null;
    L1 = Null;
    L5 = Null;

   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.eq;
      call $profiler.stop;
      jump $pop_rLink_and_rts;
   #else
      rts;
   #endif

.ENDMODULE;

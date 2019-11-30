// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef IIR_INCLUDED
#define IIR_INCLUDED

// *****************************************************************************
// MODULE:
//    $math.iir
//
// DESCRIPTION:
//    Library subroutine for a bi-quad IIR filter
//
//    Equation of each section:
//
//    w(n) = -a1*w(n-1) - a2*w(n-2) + x(n);
//    y(n) = b0*w(n) + b1*w(n-1) + b2*w(n-2);
//
// INPUTS:
//    I0  = pointer to the input data
//    L0  = lenght of input buffer (it will be circular)
//    I1  = pointer to the output data
//    L1  = lenght of output buffer (it will be circular)
//    I2  = pointer to the delay line w1(n-1), w1(n-2), w2(n-1), w2(n-2)
//    I4  = pointer to the coefficients a11,a21,b01,b11,b21,
//                                      a12,a22,b02,b12,b22, ......
//    r6  = forward coeffs shift
//    r7  = backward coeffs shift
//    r8  = no of samples to apply the filter to
//    r10 = number of biquad sections
//
// OUTPUTS:
//    The filtered data will be in the buffer pointed to by I1
//
// TRASHED REGISTERS:
//    r0-r5, r8 r10, DoLoop, I0, I2, I4, M0-M2
//
// INDICATIONS TO MAKE THE FILTER WORK:
//    If any of the forward/backward coefficients is larger than +/-1
//    all the forward/backward coefficients in all the biquads will be
//    scaled so that the largest will be within the +/-1 range.
//    This scaling of coefficients should be compensated for in the filter:
//    - r6 - shift required for the forward coeffs
//    - r7 - shift required for the backward coeffs
//    The global gain of the filter (from Matlab tf2sos function) will be
//    embedded in the forward coefficients
//
// *****************************************************************************
.MODULE $M.math.iir;
   .CODESEGMENT MATH_IIR_PM;
   .DATASEGMENT DM;

   $math.iir:

   M0 = I4;
   M1 = I2;
   M2 = r10;
   M3 = 1;

   // get x(n)
   r0 = M[I0,1];
   // get w(n-1)
   // and get a1
   r1 = M[I2,1], r2 = M[I4,1];
iir_sample_process:

      do biquad_loop;

         // load w(n-2) in r3
         // and load a2 in r4
         rMAC = r1 * r2, r3 = M[I2,0], r4 = M[I4,1];

         // save w(n-2)
         rMAC = rMAC + r3*r4, M[I2,-1] = r1;

         r5 = rMAC ASHIFT r7;

         // dummy get of w(n-1)
         // and load b0 into r2
         r0 = r0 - r5, r1 = M[I2,0], r2 = M[I4,1];

         // save w(n-1)
         // and load b1 into r4
         rMAC = r0 * r2, M[I2,2] = r0, r4 = M[I4,1];

         // load b2 into r2
         rMAC = rMAC + r1 * r4, r2 = M[I4,1];

         // get w(n-1)
         // and load a1 into r2
         rMAC = rMAC + r3 * r2, r1 = M[I2,1], r2 = M[I4,1];

         r0 = rMAC ASHIFT r6;

      biquad_loop:

      I2  = M1, M[I1,M3] = r0;
      // get x(n)
      I4  = M0, r0  = M[I0,M3];
      // get w(n-1)
      r10 = M2, r1  = M[I2,M3];
      // get a1
      r8  = r8 - M3, r2 = M[I4,M3];

   if GT jump iir_sample_process;

   rts;

.ENDMODULE;

#endif // IIR_INCLUDED

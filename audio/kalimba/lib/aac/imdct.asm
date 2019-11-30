// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"
//.warning "aac fft should load twiddle table dynamically"

// *****************************************************************************
// MODULE:
//    $aacdec.imdct
//
// DESCRIPTION:
//    1024 / 512 / 128 sample IMDCT routine
//
// INPUTS:
//    - r6 = pointer to imdct structure:
//         $aacdec.imdct.NUM_POINTS_FIELD
//            - number of input data points (1024 / 512 / 128)
//         $aacdec.imdct.INPUT_ADDR_FIELD
//            - pointer to input data (circular)
//         $aacdec.imdct.INPUT_ADDR_BR_FIELD
//            - bit reversed pointer to input data (circular)
//         $aacdec.imdct.OUTPUT_ADDR_FIELD
//            - pointer to output data (circular)
//         $aacdec.imdct.OUTPUT_ADDR_BR_FIELD
//            - bit reversed pointer to output data (circular)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - everything including $aacdec.tmp but L0 & L1
//
// *****************************************************************************
.MODULE $M.aacdec.imdct;
   .CODESEGMENT AACDEC_IMDCT_PM;
   .DATASEGMENT DM;

   $aacdec.imdct:

   // push rLink onto stack
   push rLink;

   r8 = M[r6 + $aacdec.imdct.NUM_POINTS_FIELD];
   r1 = M[r9 + $aac.mem.sin_const_iptr];
   I2 = r1;//&sin_const;
#ifdef AACDEC_ELD_ADDITIONS
   r0 = M[r9 + $aac.mem.ELD_sin_const_ptr];
   I3 = r0;
   Null = r8 - 512;
   if EQ I2 = I3;
#endif // AACDEC_ELD_ADDITIONS

   // set up the modify registers
   M0 = MK1;
   M1 = MK2;
   M2 = -MK2;

   // check how may data points we are using
   Null = r8 - 1024;
   if POS I2 = I2 + M1;

#ifdef AACDEC_ELD_ADDITIONS
   Null = r8 - 512;
   if EQ I2 = I2 + M1;
#endif // AACDEC_ELD_ADDITIONS

   // need to copy the odd values into the output buffer
   r10 = r8 LSHIFT -1;
   r10 = r10 - 1;

   // set a pointer to the start of the copy, and the target
   // and two buffers as output pointers for below
   r1 = M[r6 + $aacdec.imdct.INPUT_ADDR_FIELD];
   I0 = r1;                         // input
   I4 = r1;                         // input
#ifdef BASE_REGISTER_MODE
   push r1;
   pop B0;
   push r1;
   pop B4;
#endif
   Words2Addr(r8);
   I0 = I0 + r8,
    r2 = M[I2,M0];                  // cfreq
   I0 = I0 - M0,
    r3 = M[I2,M0];                  // sfreq

   r1 = M[r6 + $aacdec.imdct.OUTPUT_ADDR_FIELD];
   I5 = r1;                         // output
   I6 = r1;                         // output
   I5 = I5 + r8,
    r0 = M[I0,M2];

   do pre_copy_loop;
      r0 = M[I0,M2],
       M[I5,M0] = r0;
   pre_copy_loop:

   M[I5,M0] = r0;

   // set up two registers to work through the input in opposite directions
   I0 = I4;                         // input
   I5 = I6;                         // output
   I5 = I5 + r8,
    r4 = M[I2,M0];                  // c

   // to make the additions easier below set c= -c & s= -s
   r4 = -r4,                        // r4 = -c
    r5 = M[I2,M0];                  // s
   r5 = -r5,                        // r5 = -s
    r0 = M[I0, M1];                 // -tempr

   // use M3 as a loop counter
   M3 = 3;
   M2 = 0;

   // tmp used to store c
   r1 = r9 + $aac.mem.TMP;
   I1 = r1;
   Addr2Words(r8);


   outer_pre_process_loop:

      r10 = r8 LSHIFT -3;              // r10 = N/8
      I2 = I2 + MK2;
      do pre_process_loop;

         // process the data
         rMAC = r0 * r4,                  // rMAC = (-tempr) * (-c)
          r1 = M[I5, M0];                 // tempi

         rMAC = rMAC + r1 * r5;           // rMAC = temp*c + tempi*(-s)

         rMAC = r0 * r5,                  // rMAC = (-tempr)*(-s)
          M[I4, M0] = rMAC;

         rMAC = rMAC - r1 * r4;           // rMAC = tempr*s - tempi*(-c)

         // update the multipliers: "c" and "s"
         rMAC = r4 * r2,                  // (-c) * cfreq
          M[I6, M0] = rMAC;

         rMAC = rMAC - r5 * r3,           // (-c)'= (-c) * cfreq - (-s) * sfreq
          r0 = M[I0, M1];

         rMAC = r4 * r3,                  // (-c_old) * sfreq
          M[I1,M2] = rMAC;
         rMAC = rMAC + r5 * r2;           // (-s)' = (-c_old)*sfreq + (-s)*cfreq

         r5 = rMAC,                       // r5 = (-s)'
          r4 = M[I1,M2];
      pre_process_loop:

      // load the constant points mid way to improve accuracy
      r4 = M[I2,M0];

      r4 = -r4,
       r5 = M[I2,M0];

      r5 = -r5;
      M3 = M3 - 1;
   if POS jump outer_pre_process_loop;

   // set up data in fft_structure
   // I7 is what the ifft actually uses
   I7 = r6;
   r2 = r8 LSHIFT -1;
   M[r6 + $fft.NUM_POINTS_FIELD] = r2;

   // -- call the ifft --
   // set up for a scaled 64point IFFT (gain of 64)
   r8 = 1.0;               // (64*0.5^6)^(1/6)
   // set up for a scaled 512point IFFT (gain of 64)
   r7 = 0.79370052598410;  // (64*0.5^9)^(1/9)
   Null = r2 - 512;
   if Z r8 = r7;
#ifdef AACDEC_ELD_ADDITIONS
   // set up for a scaled 256point IFFT (gain of 64)
   r7 = 0.840896415253715;  // (64*0.5^8)^(1/8)
   Null = r2 - 256;
   if Z r8 = r7;
#endif // AACDEC_ELD_ADDITIONS
   push r9;
   call $math.scaleable_ifft;
   pop r9;


   // re-set up the number of points
   r1 = I7;
   r10 = M[r1 + $aacdec.imdct.NUM_POINTS_FIELD]; // r10 = N/2
   r8 = r10 LSHIFT 1;
   M[r1 + $aacdec.imdct.NUM_POINTS_FIELD] = r8;

   // set up the shift registers
   M0 = MK1;
   M1 = MK2;
   M2 = -MK2;

   // copy the data out of the temporary store after the IFFT
   r2 = M[r1 + $aacdec.imdct.INPUT_ADDR_FIELD];
   Words2Addr(r10);
   I0 = r2 + r10;                      // I0 points to the second half
#ifdef BASE_REGISTER_MODE
   push r2;
#endif

   r2 = M[r1 + $aacdec.imdct.OUTPUT_ADDR_FIELD];
   I4 = r2;
   I7 = r2;

#ifdef AACDEC_ELD_ADDITIONS
   // for AAC ELD we write the output buffer from an offset == half the size of IMDCT
   r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r0 - $aacdec.ER_AAC_ELD;
   if EQ I7 = I7 + r10;
#endif //AACDEC_ELD_ADDITIONS
   Addr2Words(r10);

   r10 = r10 - 1;                     // r10 = N/2 - 1
   r0 = M[I4, M0];                    // do one read and write outside the loop

   do copy_loop;
      r0 = M[I4, M0],
       M[I0, M0] = r0;
   copy_loop:

   // calculate some bit reverse constants
   r6 = SIGNDET r8,
    M[I0, M0] = r0;                   // perform the last memory write
   r6 = r6 + (2-LOG2_ADDR_PER_WORD);  // would use +1, but splitting data in half
   r7 = 1;                            // r7 used as loop counter, set for below
   r6 = r7 LSHIFT r6;
   M3 = r6;                           // bit reverse shift register
   r6 = r6 LSHIFT -1;                 // shift operator for I1 initialisation

   // post process the data
   r2 = M[r9 + $aac.mem.sin_const_iptr];
   I2 = r2;//&sin_const;
#ifdef AACDEC_ELD_ADDITIONS
   r0 = M[r9 + $aac.mem.ELD_sin_const_ptr];
   I3 = r0;
   Null = r8 - 512;
   if EQ I2 = I3;
#endif // AACDEC_ELD_ADDITIONS

   Null = r8 - 1024;
   if POS I2 = I2 + M1;

#ifdef AACDEC_ELD_ADDITIONS
   Null = r8 - 512;
   if EQ I2 = I2 + M1;
#endif // AACDEC_ELD_ADDITIONS
#ifdef BASE_REGISTER_MODE
   pop B1;
   push B1;
   pop B0;
#endif
   r2 = M[r1 + $aacdec.imdct.INPUT_ADDR_BR_FIELD];
   I0 = r2;                         // real ifft component


   r0 = M[r1 + $aacdec.imdct.INPUT_ADDR_FIELD];
   r2 = r8 LSHIFT -1;
   Words2Addr(r2);
   r0 = r0 + r2;
   call $math.address_bitreverse;
   I1 = r1;
   r2 = M[I2, M0];                 // cfreq



   r10 = r8 LSHIFT -1;              // r10 = N/2

   // set up pointers to output buffers
   r3 = M[I2, M0];                  // sfreq
   I6 = I7 - M0,
    r4 = M[I2, M0];                 // c
   Words2Addr(r8);
   I6 = I6 + r8,
    r5 = M[I2, M0];                 // s
   Addr2Words(r8);

   // store the constant locations in r6
   r6 = I2 + MK2;

   // data is returned bit reversed, so enable bit reverse addressing on AG1
   rFlags = rFlags OR $BR_FLAG;

   // load bit reversed tmp c location
   r0 = r9 + $aac.mem.TMP;
   call $math.address_bitreverse;
   I2 = r1;

   // I2 = BITREVERSE(&$aacdec.tmp);
   M0 = 0,
    r0 = M[I0, M3];                 // tempr

   // use r7 as outer loop counter, set above
   post_process_loop1:

      r10 = r8 LSHIFT -3;             // r10 = N/8

      do inner_post_process_loop1;

         rMAC = r0 * r4,                  // rMAC = tempr * c
          r1 = M[I1, M3];                 // tempi

         rMAC = rMAC - r1 * r5;           // rMAC = tempr*c - tempi*s


         rMAC = r1 * r4,                  // rMAC = tempi * c
          M[I6, M2] = rMAC;               // I6 = tr

         rMAC = rMAC + r0 * r5;           // rMAC = tempi*c + tempr*s

         // update the multipliers: "c" and "s"
         rMAC = r4 * r2,                  // c * cfreq
          M[I7, M1] = rMAC;

         rMAC = rMAC - r5 * r3,           // c' = c * cfreq - s * sfreq
          r0 = M[I0, M3];

         rMAC = r4 * r3,                  // c_old * sfreq
          M[I2,M0] = rMAC;

         rMAC = rMAC + r5 * r2;           // s' = c_old*sfreq + s*cfreq

         r5 = rMAC,                       // r5 = s'
          r4 = M[I2,M0];

      inner_post_process_loop1:

      // load more accurate data for c and s
      r4 = M[r6];
      r5 = M[r6 + ADDR_PER_WORD];
      r6 = r6 + (4*ADDR_PER_WORD);
      r7 = r7 - 1;
   if POS jump post_process_loop1;

   // use r7 as loop counter but check for Zero condition to loop

   post_process_loop2:

      r10 = r8 LSHIFT -3;              // r10 = N/8

      do inner_post_process_loop2;

         rMAC = r0 * r5,                  // rMAC = tempr*s
          r1 = M[I1, M3];                 // tempi

         rMAC = rMAC + r1 * r4;           // rMAC = tempr*s + tempi * c

         rMAC = r0 * r4,                  // rMAC = tempr * c
          M[I7, M1] = rMAC;

         rMAC = rMAC - r1 * r5;           // rMAC = tempr*c - tempi*s

         // Update the multipliers: "c" and "s"
         rMAC = r4 * r2,                  // c * cfreq
          M[I6, M2] = rMAC;

#ifdef AAC_AVOID_DUMMY_UNMAPPED_AND_MISSALIGNED_READS
         // Avoid read from unmapped memory so that kalsim doesn't warn and pause the processor
         Null = r10 - 1;
         if NZ jump ok;
         Null = r7 + 1;
         if Z jump ok;
           rMAC = rMAC - r5 * r3;
           jump skip_br_read;
         ok:
#endif
         rMAC = rMAC - r5 * r3,           // c' = c * cfreq - s * sfreq
          r0 = M[I0, M3];

         skip_br_read:

         rMAC = r4 * r3,                  // c_old * sfreq
          M[I2,M0] = rMAC;

         rMAC = rMAC + r5 * r2;           // s' = c_old*sfreq + s*cfreq

         r5 = rMAC,                       // r5 = s'
          r4 = M[I2,M0];

      inner_post_process_loop2:

      // load more accurate data for c and s
      r4 = M[r6];
      r5 = M[r6 + ADDR_PER_WORD];
      r6 = r6 + (4*ADDR_PER_WORD);
      r7 = r7 + 1;
      // to save instruction check zero condition, as r7 will come through as -1
      // the first time, so add one each time.
   if Z jump post_process_loop2;

   // disable bit reversed addressing on AG1
   rFlags = rFlags AND $NOT_BR_FLAG;

   L4 = 0;
   L5 = 0;
#ifdef BASE_REGISTER_MODE
   push Null;
   pop  B1;
   push Null;
   pop  B0;
   push Null;
   pop  B4;

#endif

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

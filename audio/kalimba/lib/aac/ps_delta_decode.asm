// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

#ifdef AACDEC_PS_ADDITIONS

// *****************************************************************************
// MODULE:
//    $aacdec.ps_delta_decode
//
// DESCRIPTION:
//    -
//
// INPUTS:
//    - r3 PS_NR_{IID||ICC}_PAR
//    - r5 PS_{IID||ICC}_CODING_DIRECTION[envelope]
//    - r6 maximum_index_limit
//    - r7 minimum_index_limit
//    - M0 stride
//    - I1 pointer to PS_{IID||ICC}_INDEX[envelope-1]
//    - I5 pointer to PS_{IID||ICC}_INDEX[envelope]
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0, r1, r10, M1
//
// *****************************************************************************
.MODULE $M.aacdec.ps_delta_decode;
   .CODESEGMENT AACDEC_PS_DELTA_DECODE_PM;
   .DATASEGMENT DM;

   $aacdec.ps_delta_decode:

   // r10 = PS_NR_{IID||ICC}_PAR - 1
   r10 = r3 - 1;

   // if(PS_{IID||ICC}_CODING_DIRECTION[envelope] == 1)
   //    delta decode in time with respect to the corresponding bins in the previous envelope
   Null = r5;
   if Z jump ps_envelope_frequency_delta_coded;
      // stride = PS_{IID||ICC}_STRIDE
      r0 = M0;
      Words2Addr(r0);
      M1 = r0;
      // PS_{IID||ICC}_INDEX[envelope][0] += PS_{IID||ICC}_INDEX[envelope-1][0]
      r0 = M[I5, 0];
      r1 = M[I1,M1]; // j += stride (by use of M1)
      r0 = r0 + r1;
      M[I5, MK1] = r0;
      jump end_if_ps_envelope_time_or_freq_coded;
   // else
   //    delta decode in frequency with respect to previous bin in the current envelope
   ps_envelope_frequency_delta_coded:
      I1 = I5;  // j = 0
      I5 = I5 + ADDR_PER_WORD;
      // stride = 1
      M1 = MK1;
   end_if_ps_envelope_time_or_freq_coded:

   // for i=1:PS_NR_{IID||ICC}_PAR-1,
   do ps_delta_decode_loop;
      // r0 = PS_{IID||ICC}_INDEX[envelope][i]
      r0 = M[I5,0];
      // r1 = PS_{IID||ICC)_INDEX[{prev||current}_envelope][j]
      r1 = M[I1,M1];  // j += stride (by use of M1)
      r0 = r0 + r1;
      // r0 is contained within the range [-minimum_index_limit,+maximum_index_limit]
      Null = r0 - r7;
      if LT r0 = r7;
      Null = r0 - r6;
      if GT r0 = r6;
      M[I5, MK1] = r0;
   ps_delta_decode_loop:
   rts;

.ENDMODULE;

#endif

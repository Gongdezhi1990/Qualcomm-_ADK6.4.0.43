// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.pulse_decode
//
// DESCRIPTION:
//    Decode the pulse data
//
// INPUTS:
//    - r3 = pulse_data_ptr
//    - r4 = ICS_ptr
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3, r10, I1, I4
//
// *****************************************************************************
.MODULE $M.aacdec.pulse_decode;
   .CODESEGMENT AACDEC_PULSE_DECODE_PM;
   .DATASEGMENT DM;

   $aacdec.pulse_decode:

   // check the range of pulse_start_sfb
   // if (pulse_start_sfb > ics.num_swb)
   //    error
   r0 = M[r4 + $aacdec.ics.NUM_SWB_FIELD];
   r1 = M[r3 + $aacdec.pulse.PULSE_START_SFB_FIELD];
   Null = r0 - r1;
   if NEG jump possible_corruption;


   // k = ics[ch].swb_offset[ics[ch].pulse_start_sfb];
   // for j=0:ics[ch].number_pulse,
   //    k = k + ics[ch].pulse_offset[j];
   //
   //    % restore coefficents
   //    if (x_quant[ch,k] > 0)
   //       x_quant[ch,k] += ics[ch].pulse_amp[j];
   //    else
   //       x_quant[ch,k] -= ics[ch].pulse_amp[j];
   //    end
   // end

   r0 = M[r4 + $aacdec.ics.SWB_OFFSET_PTR_FIELD];
   Words2Addr(r1);
   r1 = M[r0 + r1];

   r0 = M[r9 + $aac.mem.CURRENT_SPEC_PTR];
   Words2Addr(r1);
   r1 = r1 + r0;

   r10 = M[r3 + $aacdec.pulse.NUMBER_PULSE_FIELD];
   r10 = r10 + 1;

   I4 = r3 + $aacdec.pulse.PULSE_OFFSET_FIELD;
   I1 = r3 + $aacdec.pulse.PULSE_AMP_FIELD;

   do pulse_decode_loop;

      r3 = M[I4, MK1],     // read pulse_offset(j)
       r2 = M[I1, MK1];    // read pulse_amp(j)

      Words2Addr(r3);
      r1 = r1 + r3;     // k = k + pulse_offset(j)

      r0 = M[r1] + 0;   // read x_quant(k)
      if LE r2 = -r2;
      M[r1] = r0 + r2;  // write updated x_quant(k)

   pulse_decode_loop:

   rts;

   possible_corruption:
      // set the possible_corruption flag and hope to get back in sync next time
      #ifdef AACDEC_CALL_ERROR_ON_POSSIBLE_CORRUPTION
         call $error;
      #else
         r0 = 1;
         M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION] = r0;
         rts;
      #endif

.ENDMODULE;

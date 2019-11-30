// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.pulse_data
//
// DESCRIPTION:
//    Get the pulse data
//
// INPUTS:
//    - r4 = current ics pointer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3, r5, r10, I1, I2
//
// *****************************************************************************
.MODULE $M.aacdec.pulse_data;
   .CODESEGMENT AACDEC_PULSE_DATA_PM;
   .DATASEGMENT DM;

   $aacdec.pulse_data:

   // push rLink onto stack
   push rLink;

   // allocate temp memory for pulse data
   // - it's only needed up the inverse quantization stage
   r0 = $aacdec.pulse.STRUC_SIZE;
   call $aacdec.tmp_mem_pool_allocate;
   if NEG jump $aacdec.possible_corruption;
   r5 = r1;

   // store pulse_data_ptr
   M[r4 + $aacdec.ics.PULSE_DATA_PTR_FIELD] = r5;


   // number_pulse = getbits(2);
   call $aacdec.get2bits;
   M[r5 + $aacdec.pulse.NUMBER_PULSE_FIELD] = r1;
   r10 = r1 + 1;

   // pulse_start_sfb = getbits(6);
   call $aacdec.get6bits;
   M[r5 + $aacdec.pulse.PULSE_START_SFB_FIELD] = r1;


   // for i=0:number_pulse
   // {
      // pulse_offset(i) = getbits(5);
      // pulse_amp(i) = getbits(4);
   // }
   I1 = r5 + $aacdec.pulse.PULSE_OFFSET_FIELD;
   I2 = r5 + $aacdec.pulse.PULSE_AMP_FIELD;

   do number_pulse_loop;
      call $aacdec.get5bits;
      M[I1, MK1] = r1;
      call $aacdec.get4bits;
      M[I2, MK1] = r1;
   number_pulse_loop:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

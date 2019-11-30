// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.payload_mux
//
// DESCRIPTION:
//    Read the payload mux
//
// INPUTS:
//    - I0 = buffer pointer to read words from
//
// OUTPUTS:
//    - I0 = buffer pointer to read words from (updated)
//
// TRASHED REGISTERS:
//    - assume everything including $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.payload_mux;
   .CODESEGMENT AACDEC_PAYLOAD_MUX_PM;
   .DATASEGMENT DM;

   $aacdec.payload_mux:

   // push rLink onto stack
   push rLink;

   // prevbitpos = filebitpos();
   // raw_data_block;
   // extrabits = latm.MuxSlotLengthBytes*8 - (filebitpos - prevbitpos);
   // for i=1:extrabits,
   //    getbits(1);
   // end

   r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
   M[r9 + $aac.mem.latm.PREVBITPOS] = r0;

   // -- Decode the raw data block --
   PROFILER_START(&$aacdec.profile_raw_data_block)
#ifdef AACDEC_ELD_ADDITIONS
   r2 = $aacdec.raw_data_block;
   r3 = $aacdec.eld_raw_data_block;
   r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r0 - $aacdec.ER_AAC_ELD;
   if EQ r2 = r3;
   // r0 and r1 are not required for $aacdec.raw_data_block but they do no harm
   r1 = $aacdec.BYTE_ALIGN_OFF;
   r0 = M[r9 + $aac.mem.CHANNEL_CONFIGURATION_FIELD];
   call r2;
#else
   call $aacdec.raw_data_block;
#endif // AACDEC_ELD_ADDITIONS
   Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
   if NZ jump $aacdec.possible_corruption;
   PROFILER_STOP(&$aacdec.profile_raw_data_block)

   r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
   r1 = M[r9 + $aac.mem.latm.PREVBITPOS];
   r0 = r0 -  r1;

   r1 = M[r9 + $aac.mem.latm.MUX_SLOT_LENGTH_BYTES ];
   r1 = r1 * 8 (int);
   r10 = r1 - r0;
   if NEG jump $aacdec.possible_corruption;

#ifdef AACDEC_ELD_ADDITIONS
   // If there is a large amount of padding in the current frame
   // start by discarding whole octets and then the remaining bits.
   // There is no danger in applying this to non-ELD streams.
   r4 = r10;
   r10 = r10 ASHIFT -3;
   r1 = r10 * 8(int);
   r4 = r4 - r1;

   do skip_octet_loop;
      call $aacdec.get1byte;
   skip_octet_loop:

   r10 = r4;
   if NEG jump $aacdec.possible_corruption;
#endif // AACDEC_ELD_ADDITIONS

   do loop;
      call $aacdec.get1bit;
   loop:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;


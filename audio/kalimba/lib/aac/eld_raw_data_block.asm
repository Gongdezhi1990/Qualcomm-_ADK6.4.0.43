// *****************************************************************************
// Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

#ifdef AACDEC_ELD_ADDITIONS

// *****************************************************************************
// MODULE:
//    $aacdec.eld_raw_data_block
//
// DESCRIPTION:
//    Get a raw data block ELD
//
// INPUTS:
//    - r0 = channel configuration
//    - I0 = buffer pointer to read words from
//
// OUTPUTS:
//    - I0 = buffer pointer to read words from (updated)
//
// TRASHED REGISTERS:
//    - assume everything including $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.eld_aw_data_block;
   .CODESEGMENT AACDEC_ER_RAW_DATA_BLOCK_ELD_PM;
   .DATASEGMENT DM;

   $aacdec.eld_raw_data_block:

   // push rLink onto stack
   push rLink;

   // save byte alignment switch
   M[r9 + $aac.mem.TMP + (2*ADDR_PER_WORD)] = r1;

   // zero the SCE and CPE counters
   M[r9 + $aac.mem.NUM_SCEs] = Null;
   M[r9 + $aac.mem.NUM_CPEs] = Null;


   r1= 16;
   r2 = 15;
   Null = M[r9 + $aac.mem.ELD_frame_length_flag];
   if NZ r1 = r2;

   M[r9 + $aac.mem.ELD_sbr_numTimeSlots]=r1;
   M[r9 + $aac.mem.SBR_in_synth_loops] = r1;

   M[r9 + $aac.mem.SBR_numTimeSlotsRate] = r1;
   M[r9 + $aac.mem.SBR_numTimeSlotsRate_adjusted] = r1;



   Null = r0 - 1;
   if LT jump $aacdec.possible_corruption;
   if GT jump ch_conf_2;
      call $aacdec.eld_decode_sce;
      jump get_ld_sbr_block;
   ch_conf_2:
   Null = r0 - 2;
   if GT jump $aacdec.possible_corruption;
      call $aacdec.eld_decode_cpe;

   get_ld_sbr_block:
      r0 = M[r9 + $aac.mem.CHANNEL_CONFIGURATION_FIELD];
      Null = M[r9 + $aac.mem.ELD_sbr_present];
      if NZ call $aacdec.eld_sbr_block;

   // restore byte alignment switch
   // - for .mp4 file format the byte alignment is done here
   // - for .latm file format the byte alignment is done by $aacdec.payload_mux
   r1 = M[r9 + $aac.mem.TMP + (2*ADDR_PER_WORD)];
   Null = r1 - $aacdec.BYTE_ALIGN_ON;
   if EQ call $aacdec.byte_align;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif //AACDEC_ELD_ADDITIONS

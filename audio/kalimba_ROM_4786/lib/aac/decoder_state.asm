// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.store_boundary_snapshot
//
// DESCRIPTION:
//    - Store the changing state of the decoder.
//
// INPUTS:
//    - I0 = pointer to state storing buffer
//
// OUTPUTS:
//    - I0 = Updated
//    - r0 = Offset from the beginning of the frame (in words)
//
// TRASHED REGISTERS:
//    - r1, r2 , r3
//
// *****************************************************************************
.MODULE $M.aacdec.store_boundary_snapshot;
   .CODESEGMENT AACDEC_STORE_BOUNDARY_SNAPSHOT_PM;
   .DATASEGMENT DM;

   $aacdec.store_boundary_snapshot:
   $aacdec.suspend_decoder:

   r1 = 1;
   r0 = M[r9 + $aac.mem.READ_FRAME_FUNCTION]; // store as offset into read_frame_func_table (1 bit as just using first two entries)
   r2 = M[r9 + $aac.mem.READ_FRAME_FUNC_TABLE + 0];
   r0 = r0 - r2;
   if NZ r0 = r1;
   r1 = r0;

   r0 = M[r9 + $aac.mem.GET_BITPOS]; // 5 bits
   r0 = r0 AND 0x1F;
   r0 = r0 LSHIFT 1;
   r1 = r1 OR r0;

   r0 = M[r9 + $aac.mem.SF_INDEX_FIELD]; // 4 bits
   r0 = r0 AND 0xF;
   r0 = r0 LSHIFT 6;
   r1 = r1 OR r0;

   r0 = M[r9 + $aac.mem.CHANNEL_CONFIGURATION_FIELD]; // 4 bits
   r0 = r0 AND 0xF;
   r0 = r0 LSHIFT 10;
   r1 = r1 OR r0;

   r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD]; // 5 bits
   r0 = r0 AND 0x1F;
   r0 = r0 LSHIFT 14;
   r1 = r1 OR r0;

   r0 = M[r9 + $aac.mem.MP4_HEADER_PARSED]; // 1 bit
   r0 = r0 AND 0x1;
   r0 = r0 LSHIFT 19;
   r1 = r1 OR r0;

   r0 = M[r9 + $aac.mem.MP4_HEADER_PARSED]; // 1 bit
   Null = r0 AND 0x6;
   if NZ jump not_first_snapshot;

      r0 = 3;
      r2 = 1;
      Null = M[r9 + $aac.mem.MDAT_SIZE + (0*ADDR_PER_WORD)];
      if NZ jump mdat_size_done;
      r0 = r0 - r2;
      Null = M[r9 + $aac.mem.MDAT_SIZE + (1*ADDR_PER_WORD)];
      if NZ jump mdat_size_done;
      r0 = r0 - r2;
      Null = M[r9 + $aac.mem.MDAT_SIZE + (2*ADDR_PER_WORD)];
      if NZ jump mdat_size_done;
      r0 = r0 - r2;
      mdat_size_done:
      r0 = r0 LSHIFT 20;
      r1 = r1 OR r0;                  // 2 bit
      r2 = r0 LSHIFT -19;
      r0 = M[r9 + $aac.mem.MP4_HEADER_PARSED];
      r0 = r0 OR r2;
      M[r9 + $aac.mem.MP4_HEADER_PARSED] = r0;
      jump cont_saving_first_word;

   not_first_snapshot:
      r0 = M[r9 + $aac.mem.MP4_HEADER_PARSED];
      r0 = r0 AND 0x6;
      r0 = r0 LSHIFT 19;
      r1 = r1 OR r0;
   cont_saving_first_word:
   M[I0, MK1] = r1;

   // frame offset
   r0 = 0;

   r1 = M[r9 + $aac.mem.MP4_HEADER_PARSED];
   Null = r1 AND 0x1;
   if Z rts;
   r1 = r1 LSHIFT -1;
   r2 = r1 AND 0x3;
   if Z rts;
      r1 = M[r9 + $aac.mem.MDAT_SIZE + (2*ADDR_PER_WORD)];
      M[I0, MK1] = r1;
   r2 = r2 - 1;
   if Z rts;
      r1 = M[r9 + $aac.mem.MDAT_SIZE + (1*ADDR_PER_WORD)];
      M[I0, MK1] = r1;
   r2 = r2 - 1;
   if Z rts;
      r1 = M[r9 + $aac.mem.MDAT_SIZE + (0*ADDR_PER_WORD)];
      M[I0, MK1] = r1;
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $aacdec.restore_boundary_snapshot
//
// DESCRIPTION:
//    - restore the state of the decoder at the beginning of the most recent
//      packet
//
// INPUTS:
//    - I0 = pointer to state storing buffer
//
// OUTPUTS:
//    - I0 = Updtated
//
// TRASHED REGISTERS:
//    - r0, r1, r2
//
// *****************************************************************************
.MODULE $M.aacdec.restore_boundary_snapshot;
   .CODESEGMENT AACDEC_RESTORE_BOUNDARY_SNAPSHOT_PM;
   .DATASEGMENT DM;

   $aacdec.restore_boundary_snapshot:
   $aacdec.resume_decoder:

   r0 = M[I0, MK1];

   r1 = r0 AND 0x1;
   r2 = r9 + $aac.mem.READ_FRAME_FUNC_TABLE;
   r1 = M[r2 + r1];
   M[r9 + $aac.mem.READ_FRAME_FUNCTION] = r1; // 1 bits
   r0 = r0 LSHIFT -1;

   r1 = r0 AND 0x1F;
   M[r9 + $aac.mem.GET_BITPOS] = r1; // 5 bits
   r0 = r0 LSHIFT -5;

   r1 = r0 AND 0xF;
   M[r9 + $aac.mem.SF_INDEX_FIELD] = r1; // 4 bits
   r0 = r0 LSHIFT -4;

   r1 = r0 AND 0xF;
   M[r9 + $aac.mem.CHANNEL_CONFIGURATION_FIELD] = r1; // 4 bits
   r0 = r0 LSHIFT -4;

   r1 = r0 AND 0x1F;
   M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD] = r1; // 5 bits
   r0 = r0 LSHIFT -5;

   r1 = r0 AND 0x1;
   M[r9 + $aac.mem.MP4_HEADER_PARSED] = r1; // 1 bit

   r1 = M[r9 + $aac.mem.MP4_HEADER_PARSED];
   Null = r1 AND 1;
   if Z rts;

   r1 = r0 LSHIFT -1;

   r2 = r1 AND 0x3;
   if Z rts;
      r1 = M[I0, MK1];
      M[r9 + $aac.mem.MDAT_SIZE + (2*ADDR_PER_WORD)] = r1;
   r2 = r2 - 1;
   if Z rts;
      r1 = M[I0, MK1];
      M[r9 + $aac.mem.MDAT_SIZE + (1*ADDR_PER_WORD)] = r1;
   r2 = r2 - 1;
   if Z rts;
      r1 = M[I0, MK1];
      M[r9 + $aac.mem.MDAT_SIZE + (0*ADDR_PER_WORD)] = r1;
   rts;

.ENDMODULE;

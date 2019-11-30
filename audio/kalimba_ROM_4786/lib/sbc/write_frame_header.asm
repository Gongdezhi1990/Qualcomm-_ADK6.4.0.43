// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************
#ifndef SBC_WBS_ONLY
#ifndef SBCENC_WRITE_FRAME_HEADER_INCLUDED
#define SBCENC_WRITE_FRAME_HEADER_INCLUDED

#include "stack.h"

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $sbcenc.write_frame_header
//
// DESCRIPTION:
//    Write Frame Header
//
// INPUTS:
//    - I0 = buffer pointer to write words to
//
// OUTPUTS:
//    - I0 = buffer pointer to write words to (updated)
//
// TRASHED REGISTERS:
//    r0-r4, r6, r10, DoLoop, I1
//
// *****************************************************************************
//
//  CRC check is done on the header (except syncword and crc_check) and all the
//  scalefactors.
//
//     @verbatim
//     frame_header()
//     (
//        $syncword                                            8 BsMsbf
//        $sampling_freq                                       2 UiMsbf
//        $blocks                                              2 UiMsbf
//        $channel_mode                                        2 UiMsbf
//        $allocation_method                                   1 UiMsbf
//        $subbands                                            1 UiMsbf
//        $bitpool                                             8 UiMsbf
//        $crc_check                                           8 UiMsbf
//        If (channel_mode==JOINT_STEREO)
//        (
//           for (sb=0;sb<nrof_subbands-1;sb++)
//           (
//              $join[sb]                                      1 UiMsbf
//           )
//           $RFA                                              1 UiMsbf
//        )
//     )
//     @endverbatim
//
// *****************************************************************************
.MODULE $M.sbcenc.write_frame_header;
   .CODESEGMENT SBCENC_WRITE_FRAME_HEADER_PM;
   .DATASEGMENT DM;

   $sbcenc.write_frame_header:

   // push rLink onto stack
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($sbcenc.WRITE_FRAME_HEADER_ASM.WRITE_FRAME_HEADER.PATCH_ID_0, r1)
#endif
   
   

   // init crc_checksum = 0x0f
   r0 = 0x0f;

   M[r9 + $sbc.mem.CRC_CHECKSUM_FIELD] = r0;



   // write syncword
   r0 = 8;
   r1 = $sbc.SYNC_WORD;
   call $sbcenc.putbits;


   // write sampling frequency
   r0 = 2;

   r1 = M[r9 + $sbc.mem.SAMPLING_FREQ_FIELD];

   call $sbcenc.putbits;
   call $sbc.crc_calc;


   // write nrof_blocks
   r0 = 2;

   r1 = M[r9 + $sbc.mem.NROF_BLOCKS_FIELD];

   r1 = r1 ASHIFT -2;
   r1 = r1 - 1;
   call $sbcenc.putbits;
   call $sbc.crc_calc;


   // write channel_mode
   r0 = 2;

   r1 = M[r9 + $sbc.mem.CHANNEL_MODE_FIELD];

   call $sbcenc.putbits;
   call $sbc.crc_calc;


   // write allocation_method
   r0 = 1;

   r1 = M[r9 + $sbc.mem.ALLOCATION_METHOD_FIELD];

   call $sbcenc.putbits;
   call $sbc.crc_calc;


   // write nrof_subbands
   r0 = 1;

   r1 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];

   r1 = r1 ASHIFT -2;
   r1 = r1 - 1;
   call $sbcenc.putbits;
   call $sbc.crc_calc;

   // write bitpool
   r0 = 8;

   r1 = M[r9 + $sbc.mem.BITPOOL_FIELD];

   call $sbcenc.putbits;
   call $sbc.crc_calc;



   // calculate rest of crc

   r0 = M[r9 + $sbc.mem.CHANNEL_MODE_FIELD];

   Null = r0 - $sbc.JOINT_STEREO;
   // if joint stereo then do crc on join vector
   if NZ jump crc_notjoint;
   // and also on RFA bit which is 0
   r6 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];
   I1 = r9 + $sbc.mem.JOIN_FIELD;
   r0 = 1;
   jointcrc_loop:
      r1 = M[I1, MK1];
      call $sbc.crc_calc;
      r6 = r6 - 1;
   if NZ jump jointcrc_loop;
   crc_notjoint:

   // do crc on scalefactors
   r0 = M[r9 + $sbc.mem.NROF_CHANNELS_FIELD];
   r1 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];


   r6 = r0 * r1 (int);

   r0 = M[r9 + $sbc.mem.SCALE_FACTOR_FIELD];
   I1 = r0;

   r0 = 4;
   scalefactor_loop:
      r1 = M[I1, MK1];
      call $sbc.crc_calc;
      r6 = r6 - 1;
   if NZ jump scalefactor_loop;


   // write crc check word
   r0 = 8;

   r1 = M[r9 + $sbc.mem.CRC_CHECKSUM_FIELD];

   // select just lower 8 bits
   r1 = r1 AND 0xff;
   call $sbcenc.putbits;


   // write join data and RFA bit if required

   r0 = M[r9 + $sbc.mem.CHANNEL_MODE_FIELD];

   Null = r0 - $sbc.JOINT_STEREO;
   if NZ jump not_joint;

   I1 = r9 + $sbc.mem.JOIN_FIELD;
   r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];

   r10 = r10 - 1;
   r0 = 1;
   r1 = M[I1, MK1];
   do joint_loop;
      call $sbcenc.putbits;
      r1 = M[I1, MK1];
   joint_loop:
   call $sbcenc.putbits;

   not_joint:
   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
#endif

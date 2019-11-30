// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.filterbank
//
// DESCRIPTION:
//    Filterbank (imdct, windowing and overlap-add)
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - assume all including $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.filterbank;
   .CODESEGMENT AACDEC_FILTERBANK_PM;
   .DATASEGMENT DM;

   $aacdec.filterbank:

   // push rLink onto stack
   push rLink;

   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.FILTERBANK_ASM.FILTERBANK.FILTERBANK.PATCH_ID_0, r4)
#endif
   
   // -- IMDCT --
   PROFILER_START(&$aacdec.profile_imdct)

   // set up r4 as an ics pointer
   r4 = M[r9 + $aac.mem.CURRENT_ICS_PTR];

   // set up input and output addresses and bit-reversed addresses
   // for the imdct routine
   r0 = M[r9 + $aac.mem.BUF_LEFT_PTR];
   call $math.address_bitreverse;
   r2 = r1;

   r0 = M[r9 + $aac.mem.BUF_RIGHT_PTR];
   call $math.address_bitreverse;


   r0 = r2;
   r2 = M[r9 + $aac.mem.CURRENT_SPEC_PTR];
   M[r9 + $aac.mem.IMDCT_INFO + $aacdec.imdct.INPUT_ADDR_FIELD] = r2;
   r5 = M[r9 + $aac.mem.BUF_LEFT_PTR];
   Null = r2 - r5;
   if NZ r0 = r1;
   M[r9 + $aac.mem.IMDCT_INFO + $aacdec.imdct.INPUT_ADDR_BR_FIELD] = r0;

   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r9 + $aac.mem.IMDCT_INFO + $aacdec.imdct.OUTPUT_ADDR_FIELD] = r0;


   call $math.address_bitreverse;
   r0 = r1;
   M[r9 + $aac.mem.IMDCT_INFO + $aacdec.imdct.OUTPUT_ADDR_BR_FIELD] = r0;


   r0 = M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD];
   Null = r0 - $aacdec.EIGHT_SHORT_SEQUENCE;
   if Z jump short_sequence;
   long_sequence:
#ifdef AACDEC_ELD_ADDITIONS
      r3 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
      Null = r3 - $aacdec.ER_AAC_ELD;
      if NE jump stream_is_not_eld;

      r1 = $aacdec.FRAME_SIZE_480;
      r0 = $aacdec.FRAME_SIZE_512;
      Null = M[r9 + $aac.mem.ELD_frame_length_flag];
      if NZ r0 = r1;

      M[r9 + $aac.mem.IMDCT_INFO + $aacdec.imdct.NUM_POINTS_FIELD] = r0;

      // pre-IMDCT spectral buffer processing (LDFB2MDCT)
      r10 = r0 LSHIFT -2;
      r1 = r0 - 1;
      Words2Addr(r1);
      M0 = 0;
      M1 = MK2;
      M2 = -MK2;
      r2 = M[r9 + $aac.mem.CURRENT_SPEC_PTR];
      I0 = r2;
      I1 = I0 + MK1;
      I4 = r2 + r1;
      I5 = I4 - MK1;
      do swap_and_negate;
         r2 = M[I1,M0];
         r0 = M[I5,M0];
         M[I5,M2] = r2;
         r2 = -r0, r5 = M[I0,M0];
         r3 = M[I4,M0];
         r1 = -r5, M[I1,M1] = r2;
         M[I0,M1] = r3;
         M[I4,M2] = r1;
      swap_and_negate:

      // call imdct
      r0 = &$aacdec.imdct;
      r1 = &$aacdec.eld_imdct480;
      r6 = r9 + $aac.mem.IMDCT_INFO;
      Null = M[r9 + $aac.mem.ELD_frame_length_flag];
      if NZ r0 = r1;
      call r0;

      // The loop below does three main tasks:
      // 1. post-IMDCT data symmetries
      // IMDCT output data starts at offset 1/2*IMDCT_size and ends at offset 3/2*IMDCT_size
      //  in the output buffer. 'Unfold' IMDCT data around two symmetry axes:
      // - anti-symmetry around offset 1/2*IMDCT_size
      // - symmetry around offset 3/2*IMDCT_size
      // This operation doubles the data size to 2*IMDCT_size.
      // 2. Change the sign of all the odd-index data (LDFB2MDCT in reference decoder)
      // 3. Duplicate data with sign change (LDFB2MDCT in reference decoder). This
      // operation again doubles the data size to 4*IMDCT_size.
      //
      // Here is an example for frame size 512 which follows the changes
      // of IMDCT output buffer after each operation (x - means no valid data)
      //    |=======|=============|=============|=============|=============|
      //    |Offset |IMDCT_buffer |After task 1 |After task 2 |After task 3 |
      //    |=======|=============|=============|=============|=============|
      //    |0      |x            |-d255        |-d255        |-d255        |
      //    |1      |x            |-d254        | d254        |d254         |
      //    |...    |...          |...          |...          |...          |
      //    |254    |x            |-d1          |-d1          |-d1          |
      //    |255    |x            |-d0          |d0           |d0           |
      //    |256    |d0           |d0           |d0           |d0           |
      //    |257    |d1           |d1           |-d1          |-d1          |
      //    |...    |...          |...          |...          |...          |
      //    |766    |d510         |d510         |d510         |d510         |
      //    |767    |d511         |d511         |-d511        |-d511        |
      //    |768    |x            |d511         |d511         |d511         |
      //    |769    |x            |d510         |-d510        |-d510        |
      //    |...    |...          |...          |...          |...          |
      //    |1022   |x            |d257         |d257         |d257         |
      //    |1023   |x            |d256         |-d256        |-d256        |
      //    |1024   |x            |x            |x            |d255         |
      //    |1025   |x            |x            |x            |-d254        |
      //    |...    |...          |...          |...          |...          |
      //    |1278   |x            |x            |x            |d1           |
      //    |1279   |x            |x            |x            |-d0          |
      //    |1280   |x            |x            |x            |-d0          |
      //    |1281   |x            |x            |x            |d1           |
      //    |...    |...          |...          |...          |...          |
      //    |1790   |x            |x            |x            |-d510        |
      //    |1791   |x            |x            |x            |d511         |
      //    |1792   |x            |x            |x            |-d511        |
      //    |1793   |x            |x            |x            |d510         |
      //    |...    |...          |...          |...          |...          |
      //    |2044   |x            |x            |x            |-d259        |
      //    |2045   |x            |x            |x            |d258         |
      //    |2046   |x            |x            |x            |-d257        |
      //    |2047   |x            |x            |x            |d256         |
      //    |=======|=============|=============|=============|=============|
      //
      r10 = M[r9 + $aac.mem.IMDCT_INFO + $aacdec.imdct.NUM_POINTS_FIELD];
      r1 = M[r9 + $aac.mem.IMDCT_INFO + $aacdec.imdct.OUTPUT_ADDR_FIELD];
      r2 = r10 LSHIFT -1;
      r3 = r10 LSHIFT 1;
      Words2Addr(r2);
      Words2Addr(r3);
      Words2Addr(r10);

      M0 = 0;
      M1 = MK1;
      M2 = -MK1;
      I5 = r1 + r2;     // 1/2*IMDCT_size..IMDCT_size-1
      I4 = I5 - MK1;    // 0..1/2*IMDCT_size-1
      I7 = I5 + r3;     // 5/2*IMDCT_size..3*IMDCT_size-1
      I6 = I4 + r3;     // 2*IMDCT_size..5/2*IMDCT_size-1
      I0 = I4 + r10;    // IMDCT_size..3/2*IMDCT_size-1
      I1 = I0 + MK1;    // 3/2*IMDCT_size..2*IMDCT_size-1
      I2 = I0 + r3;     // 3*IMDCT_size..7/2*IMDCT_size-1
      I3 = I1 + r3;     // 7/2*IMDCT_size..4*IMDCT_size-1
      // loop counter is set to IMDCT_size/4
      Addr2Words(r2);
      r10 = r2 LSHIFT -1;
      do data_symmetry;
         // starts on 1/2*IMDCT_size border and processes even index data
         r0 = M[I5,M1];
         M[I4,M2] = r0;
         r2 = -r0, r1 = M[I0,M0];
         M[I6,M2] = r2;
         M[I7,M1] = r2;

         // starts on 3/2*IMDCT_size border and processes odd index data
         M[I1,M1] = r1;
         r2 = -r1, M[I2,M2] = r1;
         M[I3,M1] = r2;
         M[I0,M2] = r2;

         // starts on 1/2*IMDCT_size border and processes odd index data
         r0 = M[I5,M0];
         M[I7,M1] = r0;
         M[I6,M2] = r0;
         r2 = -r0;
         M[I4,M2] = r2;
         M[I5,M1] = r2;

         // starts on 3/2*IMDCT_size border and processes even index data
         r1 = M[I0,M2];
         r2 = -r1, M[I3,M1] = r1;
         M[I1,M1] = r2;
         M[I2,M2] = r2;
      data_symmetry:
      jump imdcts_done;
      stream_is_not_eld:
#endif // AACDEC_ELD_ADDITIONS

      r0 = 1024;
      M[r9 + $aac.mem.IMDCT_INFO + $aacdec.imdct.NUM_POINTS_FIELD] = r0;

      // call imdct
      r6 = r9 + $aac.mem.IMDCT_INFO;

      call $aacdec.imdct;

      jump imdcts_done;

   short_sequence:
      r0 = 128;

      M[r9 + $aac.mem.IMDCT_INFO + $aacdec.imdct.NUM_POINTS_FIELD] = r0;

      short_imdct_loop:
         // call imdct
         r6 = r9 + $aac.mem.IMDCT_INFO;

         call $aacdec.imdct;

         // move on to the next window

         r0 = M[r9 + $aac.mem.IMDCT_INFO + $aacdec.imdct.INPUT_ADDR_FIELD];
         r0 = r0 + (128*ADDR_PER_WORD);

         M[r9 + $aac.mem.IMDCT_INFO + $aacdec.imdct.INPUT_ADDR_FIELD] = r0;
         call $math.address_bitreverse;

         M[r9 + $aac.mem.IMDCT_INFO + $aacdec.imdct.INPUT_ADDR_BR_FIELD] = r1;
         r0 = M[r9 + $aac.mem.IMDCT_INFO + $aacdec.imdct.OUTPUT_ADDR_FIELD];
         r0 = r0 + (128*ADDR_PER_WORD);
         r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r1 = r1 + (1024*ADDR_PER_WORD);
         Null = r0 - r1;
         if Z jump imdcts_done;
         M[r9 + $aac.mem.IMDCT_INFO + $aacdec.imdct.OUTPUT_ADDR_FIELD] = r0;

         call $math.address_bitreverse;
         M[r9 + $aac.mem.IMDCT_INFO + $aacdec.imdct.OUTPUT_ADDR_BR_FIELD] = r1;


      jump short_imdct_loop;


   imdcts_done:
   PROFILER_STOP(&$aacdec.profile_imdct)

   // do the windowing of short/long sequences as needed
   PROFILER_START(&$aacdec.profile_windowing)
   M0 = 0;
#ifdef AACDEC_ELD_ADDITIONS
   r0 = $aacdec.windowing;
   r1 = $aacdec.eld_windowing;
   r2 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r2 - $aacdec.ER_AAC_ELD;
   if EQ r0 = r1;
   call r0;
#else
   call $aacdec.windowing;
#endif //AACDEC_ELD_ADDITIONS
   PROFILER_STOP(&$aacdec.profile_windowing)

   // do the overlap-add of short/long sequences as needed
   PROFILER_START(&$aacdec.profile_overlap_add)
#ifdef AACDEC_ELD_ADDITIONS
   r0 = $aacdec.overlap_add;
   r1 = $aacdec.eld_overlap_add;
   r2 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r2 - $aacdec.ER_AAC_ELD;
   if EQ r0 = r1;
   call r0;
#else
   call $aacdec.overlap_add;
#endif //AACDEC_ELD_ADDITIONS
   PROFILER_STOP(&$aacdec.profile_overlap_add)

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

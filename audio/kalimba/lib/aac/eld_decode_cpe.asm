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
//    $aacdec.eld_decode_cpe
//
// DESCRIPTION:
//    Get channel pair element information (ELD version)
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - assume everything
//
// *****************************************************************************
.MODULE $M.aacdec.eld_decode_cpe;
   .CODESEGMENT AACDEC_DECODE_CPE_ELD_PM;
   .DATASEGMENT DM;

   $aacdec.eld_decode_cpe:

   // push rLink onto stack
   push rLink;

   // make sure that sample_rate is known
   // (ie. corrupt frames might get us here with out it being set)
   Null = M[r9 + $aac.mem.SF_INDEX_FIELD];
   if NEG jump $aacdec.possible_corruption;

   // make sure we haven't had too many CPEs
   r0 = M[r9 + $aac.mem.NUM_CPEs];
   r0 = r0 + 1;
   M[r9 + $aac.mem.NUM_CPEs] = r0;
   Null = r0 - $aacdec.MAX_NUM_CPES;
   if GT jump $aacdec.possible_corruption;

   r0 = 1;
   M[r9 + $aac.mem.COMMON_WINDOW] = r0;

   // set initial ics and spec pointers to the left channel
   r4 = r9 + $aac.mem.ICS_LEFT;
   M[r9 + $aac.mem.CURRENT_ICS_PTR] = r4;
   r0 = M[r9 + $aac.mem.BUF_LEFT_PTR];
   M[r9 + $aac.mem.CURRENT_SPEC_PTR] = r0;
   M[r9 + $aac.mem.CURRENT_CHANNEL] = Null;

   // max_sfb = getbits(6);
   call $aacdec.get6bits;
   M[r4 + $aacdec.ics.MAX_SFB_FIELD] = r1;

   call $aacdec.calc_sfb_and_wingroup;
   Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
   if NZ jump possible_corruption;

   // ms_mask_present = getbits(2);
   call $aacdec.get2bits;
   M[r4 + $aacdec.ics.MS_MASK_PRESENT_FIELD] = r1;

   // this is extracted from the reference decoder huffdec2 -> getmask
   // if (ms_mask_preset == 1)
   //    read the mask bits from bit stream
   // (ms_mask_preset == 2)
   //    don't read the mask from bitstream and instead force
   //    the mask ON across the whole spectrum.
   Null = r1;
   if Z jump ms_mask_not_one_or_two;
   Null = r1 - 2;
   if GT jump ms_mask_not_one_or_two;

     // allocate max_sfb words for the ms_used data
     // use frame memory
     r0 = M[r4 + $aacdec.ics.MAX_SFB_FIELD];
     call $aacdec.frame_mem_pool_allocate;
     if NEG jump $aacdec.possible_corruption;
     M[r4 + $aacdec.ics.MS_USED_PTR_FIELD] = r1;

     // r5 will be used as indication to:
     // - force the masks ON and not read them from bit stream, if 1.
     // - read/set masks from bitstream, if 0.
     r5 = 1;
     r0 = M[r4 + $aacdec.ics.MS_MASK_PRESENT_FIELD];
     Null = r0 - 1;
     if EQ r5 = 0;

     // set the initial bitmask = 1
     r7 = 1;

     // set I1 = start of ms_used array
     r1 = M[r4 + $aacdec.ics.MS_USED_PTR_FIELD];
     I1 = r1;

     // for sfb = 0:max_sfb-1,
     r6 = M[r4 + $aacdec.ics.MAX_SFB_FIELD];
     if Z jump max_sfb_loop_end;
     max_sfb_loop:

        r0 = 1;
        r1 = r5;
        // if (ms_mask_present == 1) ms_used(0,sfb) = getbits(1);
        // else if (ms_mask_present == 2) ms_used(0,sfb) = 1;
        // two registers are prepared for the case that getbits is not called
        // r0 = 1; | number of bits to read
        // r1 = 1; | get1bit 'returned value' to force mask ON
        if Z call $aacdec.get1bit;

        // read current ms_used word
        r2 = M[I1,0];
        // clear the current bit
        r3 = r2 AND r7;
        r2 = r2 - r3;
        // if getbits was 1 then add the bit to the current word
        Null = r1;
        if NZ r2 = r2 + r7;

        // move on to the next sfb
        r6 = r6 - r0,
         M[I1, MK1] = r2;          // and store back ms_used word
     if NZ jump max_sfb_loop;
     max_sfb_loop_end:

   ms_mask_not_one_or_two:

   // copy across data from the left ics structure to the right
   // not entirely sure if we need to copy all data up to ics.NUM_SEC_FIELD
   // but it won't do anything wrogn if we do.
   r10 = ($aacdec.ics.NUM_SEC_FIELD/ADDR_PER_WORD);
   I1 = r9 + $aac.mem.ICS_LEFT;
   I2 = r9 + $aac.mem.ICS_RIGHT;
   do ics_copy;
      r0 = M[I1, MK1];
      M[I2, MK1] = r0;
   ics_copy:

   not_common_window:

   // -- reset the list of unpacked huffman tables --
   call $aacdec.huffman_reset_unpacked_list;

   call $aacdec.eld_individual_channel_stream;
   Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
   if NZ jump $aacdec.possible_corruption;

   // set ics and spec pointers to the right channel
   r0 = r9 + $aac.mem.ICS_RIGHT;
   M[r9 + $aac.mem.CURRENT_ICS_PTR] = r0;
   r0 = M[r9 + $aac.mem.BUF_RIGHT_PTR];
   M[r9 + $aac.mem.CURRENT_SPEC_PTR] = r0;
   r0 = 1;
   M[r9 + $aac.mem.CURRENT_CHANNEL] = r0;

   call $aacdec.eld_individual_channel_stream;

   // Flag that ics_info has been called successfully
   r0 = 1;
   M[r9 + $aac.mem.ICS_INFO_DONE_FIELD] = r0;

   possible_corruption:
   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif //AACDEC_ELD_ADDITIONS

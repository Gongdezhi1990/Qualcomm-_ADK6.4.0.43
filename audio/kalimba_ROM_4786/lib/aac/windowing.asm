// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.windowing
//
// DESCRIPTION:
//    Windowing
//
// INPUTS:
//    - M0  = 0 when need to update previous_window_shape (filterbank),
//         != 0 when don't want to update (ltp)
//                = -2 when called from self due to nonsense window_sequence order
//                = -1 when called from filterbank_analysis_ltp first time
//                =  1 when called from ltp_decode or from self due to nonsense window_sequence order
//                =  2 when called from filterbank_analysis_ltp second time
//    - I5  = start address of output buffer if M0 != 0
//    - r4  = buffer to second half window (if not overlap_add ie when M0=2)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - assume all
//
// *****************************************************************************
.MODULE $M.aacdec.windowing;
   .CODESEGMENT AACDEC_WINDOWING_PM;
   .DATASEGMENT DM;

   $aacdec.windowing:

   // push rLink onto stack
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.WINDOWING_ASM.AACDEC_WINDOWING.WINDOWING.PATCH_ID_0, r2)
#endif

   // set r6 and tmp+9 to window shape (current if M0==2, previous otherwise)
   r2 = M[r9 + $aac.mem.CURRENT_CHANNEL];
   Words2Addr(r2);
   r2 = r2 + r9;
   r6 = M[r2 + $aac.mem.PREVIOUS_WINDOW_SHAPE];
   r0 = M[r9 + $aac.mem.CURRENT_ICS_PTR];
   r0 = M[r0 + $aacdec.ics.WINDOW_SHAPE_FIELD];
   Null = M0 - 2;
   if Z r6 = r0;
   M[r9 + $aac.mem.TMP + (9*ADDR_PER_WORD)] = r6;


   // store M0 for later
   r0 = M0;
   M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)] = r0;

   // choose buffers depending on channel
   r5 = M[r9 + $aac.mem.codec_struc];
   Null = M[r9 + $aac.mem.CURRENT_CHANNEL];
   if NZ jump right_chan;
   left_chan:
      r0 = M[r9 + $aac.mem.OVERLAP_ADD_LEFT_PTR];
      I3 = r0;//&$aacdec.overlap_add_left;
      r0 = M[r5 + $codec.DECODER_OUT_LEFT_BUFFER_FIELD];
      // if no buffer connected just exit
      // (eg. only playing 1 channel of a stereo stream)
      if Z jump $pop_rLink_and_rts;
      jump chan_select_done;
   right_chan:
      r0 = M[r9 +  $aac.mem.OVERLAP_ADD_RIGHT_PTR];
      I3 = r0;//&$aacdec.overlap_add_right;
      r0 = M[r5 + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD];
      // if no buffer connected just exit
      // (eg. only playing 1 channel of a stereo stream)
      if Z jump $pop_rLink_and_rts;
   chan_select_done:

   // set up basic modify registers
   M2 = -MK1;
   M3 = +MK1;

   // set up I5 (output) if M0 = 0
   // otherwise set up dummy input and, if M0==2 set I3 to another location
   Null = M0;
   if NZ jump dummy_input;
      M[r9 + $aac.mem.TMP + (2*ADDR_PER_WORD)] = r0;

#ifdef BASE_REGISTER_MODE
      call $cbuffer.get_write_address_and_size_and_start_address;
      push r2;
      pop  B5;
      push B5;
      pop B4;
#else
      call $cbuffer.get_write_address_and_size;
#endif
      I5 = r0;
      L5 = r1;
      L4 = r1;
      jump i3_and_i5_set;
   dummy_input:
      M[r9 + $aac.mem.TMP + (4*ADDR_PER_WORD)] = Null;
      Null = M0 - 2;
      if Z I3 = r4;
      Null = M0;
      if POS M2 = 0;
      if POS jump i3_and_i5_set;
      M3 = 0;
      I3 = r9 + $aac.mem.TMP + (4*ADDR_PER_WORD);
   i3_and_i5_set:

   // set tmp+5 to -1, 0 or 1 depending on M0
   r0 = 0;
   r1 = 1;
   r2 = -1;
   Null = M0 + 1;
   if Z r0 = r2;
   Null = M0 - 2;
   if Z r0 = r1;
   M[r9 + $aac.mem.TMP + (5*ADDR_PER_WORD)] = r0;

   // set tmp+6 to beginning of tmp_mem_pool if tmp+5==0 or middle otherwise
   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r2 = r1 + (512*ADDR_PER_WORD);
   Null = r0;
   if NZ r1 = r2;
   M[r9 + $aac.mem.TMP + (6*ADDR_PER_WORD)] = r1;

   // branch depending on window sequence
   // normal branch if M0!=2 otherwise have to branch on what last sequence was
   r4 = M[r9 + $aac.mem.CURRENT_ICS_PTR];
   r0 = M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD];
   r1 = M[r9 + $aac.mem.CURRENT_CHANNEL];
   Words2Addr(r1);
   r1 = r1 + r9;
   r1 = M[$aac.mem.PREVIOUS_WINDOW_SEQUENCE + r1];
   Null = M0 - 2;
   if NZ jump normal_choice;
      Null = r0 - $aacdec.LONG_START_SEQUENCE;
      if Z jump long_start_sequence_processing;
      jump only_long_sequence;

   normal_choice:
      Null = M[r4 + $aacdec.ics.PREV_WINDOW_SEQ_EQ_LONG_START_FIELD];
      if NZ jump long_start_sequence_processing;
      Null = r0 - $aacdec.ONLY_LONG_SEQUENCE;
      if Z jump only_long_sequence;
      Null = r0 - $aacdec.LONG_STOP_SEQUENCE;
      if Z jump long_stop_sequence;
      Null = r0 - $aacdec.EIGHT_SHORT_SEQUENCE;
      if Z jump eight_short_sequence;

   // -- LONG_START_SEQUENCE --
   long_start_sequence:
      Null = M0;
      if NZ jump only_long_sequence;
      r2 = 1;
      M[r4 + $aacdec.ics.PREV_WINDOW_SEQ_EQ_LONG_START_FIELD] = r2;
      // this frame should be processed by only_long_sequence, the next
      // frame should be processed by long_start_sequence

   // -- ONLY_LONG_SEQUENCE --
   only_long_sequence:
      Null = M0;
      if NZ jump only_long_sequence_ok;
         Null = r1 - $aacdec.ONLY_LONG_SEQUENCE;
         if Z jump only_long_sequence_ok;
         Null = r1 - $aacdec.LONG_STOP_SEQUENCE;
         if Z jump only_long_sequence_ok;
         M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD] = r1;
         M0 = 1;
         call $aacdec.windowing;
         r1 = $aacdec.ONLY_LONG_SEQUENCE;
         r4 = M[r9 + $aac.mem.CURRENT_ICS_PTR];
         M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD] = r1;
         M0 = -(1024*ADDR_PER_WORD);
         r0 = M[I5,M0];
         M0 = -2;
         call $aacdec.windowing;
         M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)] = Null;
         jump finished_windowing;
      only_long_sequence_ok:

      // set up r8 & r9
      r4 = M[r9 + $aac.mem.CURRENT_SPEC_BLKSIGNDET_PTR];

      r1 = 1;
      r8 = 2;
      Null = M[r4 + (1*ADDR_PER_WORD)];
      if NZ r8 = r1;
      r7 = 1;

      r10 = 1024;

      // write full output
      M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)] = Null;

      // set I2 to input buffer or dummy input
      r0 = M[r9 + $aac.mem.TMP + (6*ADDR_PER_WORD)];
      r0 = r0 + (511*ADDR_PER_WORD);
      I2 = r9 + $aac.mem.TMP + (4*ADDR_PER_WORD);
      Null = M2;
      if NZ I2 = r0;
      call window;

      jump finished_windowing;

   // -- long_start_sequence_processing --
   long_start_sequence_processing:
      Null = M0;
      if NZ jump long_start_sequence_ok;
         Null = r0 - $aacdec.EIGHT_SHORT_SEQUENCE;
         if Z jump long_start_sequence_reset;
         M[r9 + $aac.mem.TMP + (11*ADDR_PER_WORD)] = r0;
         M0 = 1;
         call $aacdec.windowing;
         r4 = M[r9 + $aac.mem.CURRENT_ICS_PTR];
         M[r4 + $aacdec.ics.PREV_WINDOW_SEQ_EQ_LONG_START_FIELD] = Null;
         r1 = M[r9 + $aac.mem.TMP + (11*ADDR_PER_WORD)];
         M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD] = r1;
         M0 = -(1024*ADDR_PER_WORD);
         r0 = M[I5,M0];
         M0 = -2;
         call $aacdec.windowing;
         M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)] = Null;
         jump finished_windowing;
      long_start_sequence_reset:
         // if M0==0 reset variable to not jump into here next time
         r1 = r4 + $aacdec.ics.PREV_WINDOW_SEQ_EQ_LONG_START_FIELD;
         Null = M0;
         if Z M[r1] = Null;
      long_start_sequence_ok:


      // copy first buffer (or dummy) and scale
      r10 = 448;
      r7 = $aacdec.AUDIO_OUT_SCALE_AMOUNT;

      r0 = $aacdec.AUDIO_OUT_SCALE_AMOUNT/2;
      Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
      if NZ r7 = r0;
      r0 = M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)];
      Null = r0 + 2;
      if NZ jump normal_copy;
         do copy_start_overlap;
            r0 = M[I3,M3];
            r0 = r0 * r7 (int) (sat),
             r1 = M[I5,0];
            r0 = r0 + r1;
            M[I5, MK1] = r0;
         copy_start_overlap:
         jump finished_copy;

      normal_copy:
         do copy_start;
            r0 = M[I3,M3];
            r0 = r0 * r7 (int) (sat);
            M[I5, MK1] = r0;
         copy_start:

      finished_copy:

      // set I2 to input buffer or dummy input
      r0 = M[r9 + $aac.mem.TMP + (6*ADDR_PER_WORD)];
      r0 = r0 + (63*ADDR_PER_WORD);
      I2 = r9 + $aac.mem.TMP + (4*ADDR_PER_WORD);
      Null = M2;
      if NZ I2 = r0;

      // write full output
      M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)] = Null;

      r10 = 128;
      // set up r8 from r9, include scale up by 2 if not done in tns
      r4 = M[r9 + $aac.mem.CURRENT_SPEC_BLKSIGNDET_PTR];
      r8 = 2;
      r7 = 1;
      Null = M[r4 + (1*ADDR_PER_WORD)];
      if NZ r8 = r7;

      call window;

      // set I3 to same buffer as I2 and amend M3 as appropriate
      I3 = I2 - (127*ADDR_PER_WORD);
      M3 = -M2;
      if Z I3 = I2;

      // now I3 is same buffer as I2, r9 must equal r8
      r7 = r8;

      // set window shape to current window shape
      r6 = M[r9 + $aac.mem.CURRENT_ICS_PTR];
      r6 = M[r6 + $aacdec.ics.WINDOW_SHAPE_FIELD];
      M[r9 + $aac.mem.TMP + (9*ADDR_PER_WORD)] = r6;
      I4 = I5;
      M0 = (384*ADDR_PER_WORD);
      r0 = M[I4,M0];
      I7 = I4;
      short_end_loop:
         // reset r10 and r6
         r10 = 128;
         r6 = M[r9 + $aac.mem.TMP + (9*ADDR_PER_WORD)];

         call window;

         Null = I5 - I7;
      if NZ jump short_end_loop;

      // write only first half of output
      r1 = 1;
      M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)] = r1;

      // reset r10 and r6
      r10 = 128;
      r6 = M[r9 + $aac.mem.TMP + (9*ADDR_PER_WORD)];

      call window;

      jump finished_windowing;

   // -- LONG_STOP_SEQUENCE --
   long_stop_sequence:
      Null = M0;
      if NZ jump long_stop_sequence_ok;
         Null = r1 - $aacdec.EIGHT_SHORT_SEQUENCE;
         if Z jump long_stop_sequence_ok;
         r2 = $aacdec.ONLY_LONG_SEQUENCE;
         Null = r1 - $aacdec.LONG_STOP_SEQUENCE;
         if Z r1 = r2;
         M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD] = r1;
         M0 = 1;
         call $aacdec.windowing;
         r1 = $aacdec.LONG_STOP_SEQUENCE;
         r4 = M[r9 + $aac.mem.CURRENT_ICS_PTR];
         M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD] = r1;
         M0 = (-1024*ADDR_PER_WORD);
         r0 = M[I5,M0];
         M0 = -2;
         call $aacdec.windowing;
         M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)] = Null;
         jump finished_windowing;
      long_stop_sequence_ok:
      // set I2 to point to same buffer as I3
      // store M2 in tmp+10 and amend M2 based on M3
      I2 = I3 + (127*ADDR_PER_WORD);
      Null = M3;
      if Z I2 = I3;
      r0 = M2;
      M[r9 + $aac.mem.TMP + (10*ADDR_PER_WORD)] = r0;
      M2 = -M3;

      // write only second half of output
      r1 = -1;
      M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)] = r1;

      // set up r8 and r9
      r7 = 1;
      r8 = r7;

      M0 = (-64*ADDR_PER_WORD);
      r0 = M[I5,M0];
      I4 = I5;
      M0 = (512*ADDR_PER_WORD);
      r0 = M[I4,M0];
      I7 = I4;
      short_start_loop:
         // reset r10
         r10 = 128;

         call window;
         // reset r6
         r6 = M[r9 + $aac.mem.TMP + (9*ADDR_PER_WORD)];
        // write full output
         M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)] = Null;

         Null = I5 - I7;
      if NZ jump short_start_loop;

      // restore I2, M2
      r0 = M[r9 + $aac.mem.TMP + (6*ADDR_PER_WORD)];
      r0 = r0 + (63*ADDR_PER_WORD);
      I2 = r9 + $aac.mem.TMP + (4*ADDR_PER_WORD);
      r1 = M[r9 + $aac.mem.TMP + (10*ADDR_PER_WORD)];
      M2 = r1;
      if NZ I2 = r0;

      // reset r10
      r10 = 128;

      // update r8 to include scale up by 2 if not done in tns
      r4 = M[r9 + $aac.mem.CURRENT_SPEC_BLKSIGNDET_PTR];
      r8 = 2;
      Null = M[r4 + (1*ADDR_PER_WORD)];
      if NZ r8 = r7;

      call window;

      // shift I2 along buffer (unless using dummy)
      r0 = I2 - (127*ADDR_PER_WORD);
      M0 = -M2;
      if NZ I2 = r0;

      M1 = (1*ADDR_PER_WORD);
      r10 = 448;

      r0 = r8 * ($aacdec.AUDIO_OUT_SCALE_AMOUNT/2) (int);
      r8 = r8 * $aacdec.AUDIO_OUT_SCALE_AMOUNT (int);
      Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
      if NZ r8 = r0;

      // set r8 to 1 if M0==-1or2
      r0 = 1;
      Null = M[r9 + $aac.mem.TMP + (5*ADDR_PER_WORD)];
      if NZ r8 = r0;

      r0 = M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)];
      Null = r0 + 2;
      if Z jump overlap_and_scale_long_stop;
         // copy end of second buffer and scale
         do copy_end;
            r0 = M[I2,M0];
            r0 = r0 * r8 (int) (sat);
            M[I5,M1] = r0;
         copy_end:

         jump finished_windowing;

      overlap_and_scale_long_stop:
         // copy end of second buffer, scale and overlap
         do copy_end_overlap;
            r0 = M[I2,M0];
            r0 = r0 * r8 (int) (sat),
             r1 = M[I5,0];
            r0 = r0 + r1;
            M[I5,M1] = r0;
         copy_end_overlap:

         jump finished_windowing;

   // -- EIGHT_SHORT_SEQUENCE --
   eight_short_sequence:
      Null = M0;
      if NZ jump eight_short_sequence_ok;
         Null = r1 - $aacdec.EIGHT_SHORT_SEQUENCE;
         if Z jump eight_short_sequence_ok;
         Null = r1 - $aacdec.LONG_START_SEQUENCE;
         if Z jump eight_short_sequence_ok;
         r2 = $aacdec.ONLY_LONG_SEQUENCE;
         Null = r1 - $aacdec.LONG_STOP_SEQUENCE;
         if Z r1 = r2;
         M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD] = r1;
         M0 = 1;
         call $aacdec.windowing;
         r1 = $aacdec.EIGHT_SHORT_SEQUENCE;
         r4 = M[r9 + $aac.mem.CURRENT_ICS_PTR];
         M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD] = r1;
         M0 = (-1024*ADDR_PER_WORD);
         r0 = M[I5,M0];
         M0 = -2;
         call $aacdec.windowing;
         M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)] = Null;
         jump finished_windowing;
      eight_short_sequence_ok:

      // set I2 to point to same buffer as I3
      // store M2 in tmp+10 and amend M2 based on M3
      I2 = I3 + (127*ADDR_PER_WORD);
      Null = M3;
      if Z I2 = I3;
      r0 = M2;
      M[r9 + $aac.mem.TMP + (10*ADDR_PER_WORD)] = r0;
      M2 = -M3;

      // write only second half of output
      r1 = -1;
      M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)] = r1;

      // set first time change stuff counter thing
      M[r9 + $aac.mem.TMP + (7*ADDR_PER_WORD)] = Null;

      // set up r8 and r9
      r7 = 1;
      r8 = r7;

      M0 = (-64*ADDR_PER_WORD);
      r0 = M[I5,M0];
      M0 = (512*ADDR_PER_WORD);
      I4 = I5;
      r0 = M[I4,M0];
      I0 = I4;
      r0 = M[I4,M0];
      I7 = I4;
      short_loop:
         // reset r10
         r10 = 128;

         call window;

         // reset r6
         r6 = M[r9 + $aac.mem.TMP + (9*ADDR_PER_WORD)];
         // write full output
         M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)] = Null;

         Null = I5 - I0;
         if NZ jump no_change;
            // select first or second change
             r0 = M[r9 + $aac.mem.TMP + (7*ADDR_PER_WORD)];
            if NZ jump i3_change;
               r0 = r0 + 1;
               M[r9 + $aac.mem.TMP + (7*ADDR_PER_WORD)] = r0;

               // restore I2, M2
               r0 = M[r9 + $aac.mem.TMP + (6*ADDR_PER_WORD)];
               r0 = r0 + (63*ADDR_PER_WORD);
               I2 = r9 + $aac.mem.TMP + (4*ADDR_PER_WORD);
               r1 = M[r9 + $aac.mem.TMP + (10*ADDR_PER_WORD)];
               M2 = r1;
               if NZ I2 = r0;

               // update criterion for jumping into change loop
               L0 = L5;
#ifdef BASE_REGISTER_MODE
               push B5;  pop B0;
#endif

               M0 = (128*ADDR_PER_WORD);
               r0 = M[I0,M0];
               L0 = 0;
#ifdef BASE_REGISTER_MODE
               push Null;  pop B0;
#endif

               // update r8 to include scale up by 2 if not done in tns
               r4 = M[r9 + $aac.mem.CURRENT_SPEC_BLKSIGNDET_PTR];
               r8 = r7 * 2 (int);
               Null = M[r4 + (1*ADDR_PER_WORD)];
               if NZ r8 = r7;

               jump no_change;
            i3_change:
               // set I3 to same buffer as I2 and amend M3 as appropriate
               I3 = I2 - (127*ADDR_PER_WORD);
               M3 = -M2;
               if Z I3 = I2;
               // now I3 is same buffer as I2, r9 must equal r8
               r7 = r8;
               // set window shape to current window shape
               r6 = M[r9 + $aac.mem.CURRENT_ICS_PTR];
               r6 = M[r6 + $aacdec.ics.WINDOW_SHAPE_FIELD];
               M[r9 + $aac.mem.TMP + (9*ADDR_PER_WORD)] = r6;
         no_change:

         Null = I5 - I7;
      if NZ jump short_loop;

      // write only first half of output
      r1 = 1;
      M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)] = r1;

      // reset r10
      r10 = 128;

      call window;

   finished_windowing:

   // if M0!=0 exit
   Null = M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)];
   if Z jump dont_exit_now;
      jump $pop_rLink_and_rts;
   dont_exit_now:

   // if its a mono stream and we have stereo buffers connected then copy
   // the left channel's output to the right channel
   Null = M[r9 + $aac.mem.CONVERT_MONO_TO_STEREO_FIELD];
   if Z jump dont_copy_to_right_channel;

      // move I5 pointer back 1024 (to start of the frame) before we do the copy
      M0 = (-1024*ADDR_PER_WORD);
      r0 = M[I5,M0];

      // set I1/L1 to point to the right audio output buffer
      r5 = M[r9 + $aac.mem.codec_struc];
      r0 = M[r5 + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD];
#ifdef BASE_REGISTER_MODE
      call $cbuffer.get_write_address_and_size_and_start_address;
      push r2;
      pop B1;
#else
      call $cbuffer.get_write_address_and_size;
#endif
      I1 = r0;
      L1 = r1;

      // do the copy
      r10 = 1024;
      do copy_to_right_loop;
         r0 = M[I5, MK1];
         M[I1, MK1] = r0;
      copy_to_right_loop:

#ifdef AACDEC_SBR_ADDITIONS
      Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
      if NZ jump dont_set_write_ptr_mono_to_stereo;
#endif
         // store updated cbuffer pointer for the right channel
         r0 = M[r5 + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD];
         r1 = I1;
         call $cbuffer.set_write_address;
      dont_set_write_ptr_mono_to_stereo:
      L1 = 0;
#ifdef BASE_REGISTER_MODE
      push Null;  pop B1;
#endif
   dont_copy_to_right_channel:


#ifdef AACDEC_SBR_ADDITIONS
   Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
   if NZ jump dont_set_write_ptr;
#endif
      // store updated cbuffer pointer
      r0 = M[r9 + $aac.mem.TMP + (2*ADDR_PER_WORD)];
      r1 = I5;
      call $cbuffer.set_write_address;
   dont_set_write_ptr:
   L5 = 0;
   L4 = 0;
#ifdef BASE_REGISTER_MODE
   push Null;  pop B5;
   push Null;  pop B4;
#endif

   // store updated previous_window_shape and previous_window_sequence
   r4 = M[r9 + $aac.mem.CURRENT_ICS_PTR];
   r8 = M[r4 + $aacdec.ics.WINDOW_SHAPE_FIELD];
   r2 = M[r9 + $aac.mem.CURRENT_CHANNEL];
   Words2Addr(r2);
   r2 = r2 + r9;
   M[r2 + $aac.mem.PREVIOUS_WINDOW_SHAPE] = r8;
   r8 = M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD];
   M[r2 + $aac.mem.PREVIOUS_WINDOW_SEQUENCE] = r8;

   push Null;  pop B1;
   push Null;  pop B4;
   push Null;  pop B5;

   // pop rLink from stack
   jump $pop_rLink_and_rts;



   // DESCRIPTION: Subroutine to do sin or kaiser windowing and overlap add
   //
   // INPUTS:
   //    r6  - window type SIN_WINDOW (0) or KAISER_WINDOW (1)
   //    r8  - gain for I2 data
   //    r7  - gain for I3 data r9 replaced by r7
   //    r10 - number of samples 1024/128
   //    I2  - ptr to end of data to be second half windowed
   //    I3  - ptr to start of data to be first half windowed
   //    I5  - ptr to start of output buffer
   //    M2  = -1 if I2 points to data
   //        =  0 if I2 points to dummy data
   //    M3  = +1 if I3 points to data
   //        =  0 if I3 points to dummy data
   //    $aacdec.tmp + 3  = -1 - only second half of output written
   //                     =  0 - all of output written
   //                     = +1 - only first half of output written
   //    L4  - set to length of output buffer
   //    L5  - set to length of output buffer
   //
   // OUTPUTS:
   //    r10 = 0;
   //    I5  = I5 + r10
   //    I2  = I2 + 128 (unless pointing to dummy variable, in which case I2 = I2)
   //    I3  = I3 + 128 (unless pointing to dummy variable, in which case I3 = I3)
   //
   // TRASHED:
   //    r0-r7, r10, I1, I4, I6, M0, M1
window:

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.WINDOWING_ASM.AACDEC_WINDOWING.WINDOW.PATCH_ID_0, r5)
#endif

   r0 = r10 - 1;
   Words2Addr(r0);
   M0 = r0;
   I4 = I5;
   r0 = M[I4,M0];
   M0 = +MK1;
   M1 = -MK1;
   Null = r6;
   if NZ jump window_kaiser;

   window_sin:
   // select either 2048 or 256 length coefs
   r0 = M[r9 + $aac.mem.SIN2048_COEFS_FIELD];
   I1 = r0;
   r0 = M[r9 + $aac.mem.SIN256_COEFS_FIELD];
   I6 = r0;
   Null = r10 - 1024;
   if NZ I1 = I6;

   // r6 = r10 = 512 or 64
   r10 = r10 ASHIFT -1;
   r6 = r10;
   Words2Addr(r6);

   // load the init_vector and rotation_matrix into registers
   I6 = I1 + (1*ADDR_PER_WORD);
   r0 = M[I1, MK2],   // init_vector[0]
    r3 = M[I6, MK2];  // init_vector[1]
   r4 = M[I1, MK2],   // rotation_matrix[0][0] =  rotation_matrix[1][1]
    r5 = M[I6, MK2];  // rotation_matrix[1][0] = -rotation_matrix[0][1]

   // if input data isn't mirrored jump to other routine
   Null = M[r9 + $aac.mem.TMP + (5*ADDR_PER_WORD)];
   if NZ jump non_mirrored;

   // now r8old = r9 LSHIFT r8new
   r2 = SIGNDET r8;
   r8 = SIGNDET r7;
   r8 = r8 - r2;
   M[r9 + $aac.mem.TMP + (12*ADDR_PER_WORD)] = r7;

   r2 = r7 * ($aacdec.AUDIO_OUT_SCALE_AMOUNT/2) (int);
   r7 = r7 * $aacdec.AUDIO_OUT_SCALE_AMOUNT (int);
   Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
   if NZ r7 = r2;

   // if only writing to half of audio buffer set up one
   // pointer to point to a dummy location and set the
   // corresponding modify register to 0
   Null = M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)];
   if Z jump ok_sin;
      if NEG jump second_half_sin;
         I4 = r9 + $aac.mem.TMP;
         L4 = 0;
#ifdef BASE_REGISTER_MODE
         push Null;  pop B4;
#endif
         M1 = 0;
         r6 = 0;
      jump ok_sin;
      second_half_sin:
         I5 = r9 + $aac.mem.TMP;
         L5 = 0;
#ifdef BASE_REGISTER_MODE
         push Null;  pop B5;
#endif
         M0 = 0;
   ok_sin:
   r2 = M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)];
   Null = r2 + 2;
   if Z jump two_passes_loop;

   do sin_loop;
      r2 = r0,
       r0 = M[I2,M2];         // read value A
      r0 = r0 LSHIFT r8,
       r1 = M[I3,M3];         // read value B
      rMAC = r1 * r3;
      rMAC = rMAC + r0 * r2;
      rMAC = rMAC * r7 (int) (sat);
      rMAC = r1 * r2,
       M[I4,M1] = rMAC;       // write walue A'
      rMAC = rMAC - r0 * r3;
      rMAC = rMAC * r7 (int) (sat);
      rMAC = r4 * r2,         // update window
       M[I5,M0] = rMAC;       // write walue B'
      rMAC = rMAC - r5 * r3;  // update window
      r0 = rMAC;
      rMAC = r5 * r2;         // update window
      rMAC = rMAC + r4 * r3;  // update window
      r3 = rMAC;
   sin_loop:
   // if I5 pointing at dummy need to use I4
   r0 = M[I4, MK1];
   Null = M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)];
   if NEG jump use_I4_sin;
      L4 = L5;
#ifdef BASE_REGISTER_MODE
      push B5;  pop B4;
#endif
      jump use_I4_sin_done;
   use_I4_sin:
      I5 = I4;
   use_I4_sin_done:
   L5 = L4;
#ifdef BASE_REGISTER_MODE
   push B4;  pop B5;
#endif

   M0 = r6;
   Null = M1;
   if Z M0 = 0;
   r0 = M[I5,M0];
   r1 = M2;
   Addr2Words(r1);
   r6 = r6 + (128*ADDR_PER_WORD);
   r0 = r1 * r6 (int);
   I2 = I2 - r0;
   r1 = M3;
   Addr2Words(r1);
   r6 = r6 - (256*ADDR_PER_WORD);
   r0 = r1 * r6 (int);
   I3 = I3 - r0;

   // restore r8, r9
   r7 = M[r9 + $aac.mem.TMP + (12*ADDR_PER_WORD)];
   r8 = r7 LSHIFT r8;
   rts;

   two_passes_loop:
   push r6;
   do second_pass_sin_loop;
      r2 = r0,
       r0 = M[I2,M2];         // read value A
      r0 = r0 LSHIFT r8,
       r1 = M[I3,M3];         // read value B
      rMAC = r1 * r3;
      rMAC = rMAC + r0 * r2;
      rMAC = rMAC * r7 (int) (sat);
      r6 = I4;
      rMAC = rMAC + M[r6];
      rMAC = r1 * r2,
       M[I4,M1] = rMAC;       // write walue A'
      rMAC = rMAC - r0 * r3;
      rMAC = rMAC * r7 (int) (sat);
      r6 = I5;
      rMAC = rMAC + M[r6];
      rMAC = r4 * r2,         // update window
       M[I5,M0] = rMAC;       // write walue B'
      rMAC = rMAC - r5 * r3;  // update window
      r0 = rMAC;
      rMAC = r5 * r2;         // update window
      rMAC = rMAC + r4 * r3;  // update window
      r3 = rMAC;
   second_pass_sin_loop:
   pop r6;
   jump sin_loop;


   non_mirrored:
   // rMAC = 255 or 31
   rMAC = r10 ASHIFT 1;
   rMAC = rMAC - 1;
   Words2Addr(rMAC);
   Null = M[r9 + $aac.mem.TMP + (5*ADDR_PER_WORD)];
   if NEG jump first_half_type;
      // swap r3 and r0 and invert r5
      r2 = r0;
      r0 = r3;
      r3 = r2;
      r5 = -r5;
      // store I2,M2 then use I3,M3 to set up I2,M2
      r1 = I2;
      r2 = M2;
      I2 = I3 + rMAC;
      Null = M3;
      if Z I2 = I3;
      M2 = -M3;
      jump start_the_loop;
   first_half_type:
      // store I3,M3 then use I2,M2 to set up I3,M3
      r1 = I3;
      r2 = M3;
      I3 = I2 - rMAC;
      Null = M2;
      if Z I3 = I2;
      M3 = -M2;
   start_the_loop:
   M[r9 + $aac.mem.TMP + (7*ADDR_PER_WORD)] = r1;
   M[r9 + $aac.mem.TMP + (8*ADDR_PER_WORD)] = r2;


   // if only writing to half of audio buffer set up one
   // pointer to point to a dummy location and set the
   // corresponding modify register to 0
   Null = M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)];
   if Z jump non_mirrored_ok_sin;
      if NEG jump non_mirrored_second_half_sin;
         I4 = r9 + $aac.mem.TMP;
         L4 = 0;
#ifdef BASE_REGISTER_MODE
         push Null;  pop B4;
#endif
         M1 = 0;
         r6 = 0;
      jump non_mirrored_ok_sin;
      non_mirrored_second_half_sin:
         I5 = r9 + $aac.mem.TMP;
         L5 = 0;
#ifdef BASE_REGISTER_MODE
         push Null;  pop B5;
#endif
         M0 = 0;
   non_mirrored_ok_sin:


   do non_mirrored_sin_loop;
      r2 = r0,
       r0 = M[I2,M2];         // read value A
      r1 = M[I3,M3];          // read value B
      rMAC = r0 * r2;
      rMAC = r1 * r3,
       M[I4,M1] = rMAC;       // write walue A'
      rMAC = r4 * r2,         // update window
       M[I5,M0] = rMAC;       // write walue B'
      rMAC = rMAC - r5 * r3;  // update window
      r0 = rMAC;
      rMAC = r5 * r2;         // update window
      rMAC = rMAC + r4 * r3;  // update window
      r3 = rMAC;
   non_mirrored_sin_loop:

   // if I5 pointing at dummy need to use I4
   r0 = M[I4, MK1];
   Null = M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)];
   if NEG I5 = I4;

   // set I5 to correct point
   M0 = r6;
   r0 = M[I5,M0];
   r0 = M[r9 + $aac.mem.TMP + (7*ADDR_PER_WORD)];
   r1 = M[r9 + $aac.mem.TMP + (8*ADDR_PER_WORD)];
   Null = M[r9 + $aac.mem.TMP + (5*ADDR_PER_WORD)];
   if NEG jump first_half_type_post;
      // restore I2,M2
      I2 = r0;
      M2 = r1;
      r1 = M3;
      Addr2Words(r1);
      // set I3, to correct point
      r0 = r1 * r6 (int);
      I3 = I3 - r0;
      jump shift_i2_i3;
   first_half_type_post:
      // restore I3,M3
      I3 = r0;
      M3 = r1;
      r1 = M2;
      Addr2Words(r1);
      // set I2 to correct point
      r0 = r1 * r6 (int);
      I2 = I2 - r0;
      jump shift_i2_i3;


   window_kaiser:
   // select either 2048 or 256 length coefs
   r5 = M[r9 + $aac.mem.KAISER2048_COEFS_FIELD];
   I1 = r5;//&$aacdec.kaiser2048_coefs;
   r5 = M[r9 + $aac.mem.KAISER256_COEFS_FIELD];
   I6 = r5;//&$aacdec.kaiser256_coefs;
   Null = r10 - 1024;
   if NZ I1 = I6;
   I6 = I1 + (18*ADDR_PER_WORD);
   // set r7 = 2^15 (if r10 = 1024)
   // set r7 = 2^18 (if r10 = 128)
   r5 = SIGNDET r10;
   r5 = 8 ASHIFT r5;

   // if input data isn't mirrored jump to other routine
   Null = M[r9 + $aac.mem.TMP + (5*ADDR_PER_WORD)];
   if NZ jump non_mirrored_kaiser;

   // if only writing to half of audio buffer set up one
   // pointer to point to a dummy location and set the
   // corresponding modify register to 0
   Null = M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)];
   if Z jump ok_kai_1;
      if NEG jump second_half_kai_1;
         I4 = r9 + $aac.mem.TMP;
         L4 = 0;
#ifdef BASE_REGISTER_MODE
         push Null;  pop B4;
#endif
         M1 = 0;
      jump ok_kai_1;
      second_half_kai_1:
         I5 = r9 + $aac.mem.TMP;
         L5 = 0;
#ifdef BASE_REGISTER_MODE
         push Null;  pop B5;
#endif
         M0 = 0;
   ok_kai_1:

   r8 = -r8;
   r2 = M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)];
   Null = r2 + 2;
   if NZ jump kaiser_outer_loop;
      r4 = $aacdec.AUDIO_OUT_SCALE_AMOUNT;
      r0 = $aacdec.AUDIO_OUT_SCALE_AMOUNT/2;
      Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
      if NZ r4 = r0;
      M[r9 + $aac.mem.TMP + (12*ADDR_PER_WORD)] = r7;
      M[r9 + $aac.mem.TMP + (13*ADDR_PER_WORD)] = r8;
      r8 = r8 * r4 (int);
      r7 = r7 * r4 (int);
      r4 = 1;
      jump kaiser_outer_loop_2;

   kaiser_outer_loop:
      r1 = M[I1, MK1];  // coef for x^0
      r2 = M[I1, MK1];  // coef for x^1
      r3 = M[I1, MK1];  // coef for x^2
      r4 = M[I1, MK1];  // coef for x^3
      r0 = M[I1, MK1];  // coef for x^4
      rMACB = r0;
      r0 = M[I1, MK1];  // number of values generated with these coefs
      r10 = r0;

      r6 = 0;

      // window loop
      do kaiser_inner_loop;
         rMAC = r1;
         rMAC = rMAC + r6 * r2;
         r0 = r6 * r6 (frac);  // r6 = x^2;
         rMAC = rMAC + r0 * r3;
         r0 = r0 * r6 (frac);  // r6 = x^3;
         rMAC = rMAC + r0 * r4;
         r0 = r0 * r6 (frac);  // r6 = x^4;
         rMAC = rMAC + r0 * rMACB,
          r0 = M[I3,M3];
         r0 = rMAC * r0 (frac);
         r0 = r0 * r7 (int) (sat);
         r6 = r6 + r5,  // increment x;
          M[I4,M1] = r0,
          r0 = M[I2,M2];
         r0 = rMAC * r0 (frac);
         r0 = r0 * r8 (int) (sat);
         M[I5,M0] = r0;
      kaiser_inner_loop:
      Null = I1 - I6;
   if NZ jump kaiser_outer_loop;

   end_kaiser_outer_loop_1:
   // work along I2 and I3 in other direction
   r0 = I2 + MK1;
   M2 = -M2;
   if NZ I2 = r0;
   r0 = I3 - MK1;
   M3 = -M3;
   if NZ I3 = r0;
   I6 = I1 + (18*ADDR_PER_WORD);

   r8 = -r8;

   // if only writing to half of audio buffer set up one
   // pointer to point to a dummy location and set the
   // corresponding modify register to 0
   Null = M[r9 + $aac.mem.TMP + (3*ADDR_PER_WORD)];
   if Z jump ok_kai_2;
      if NEG jump second_half_kai_2;
         r0 = M[I5, -MK1];
         I4 = I5;
         L4 = L5;
#ifdef BASE_REGISTER_MODE
         push B5;  pop B4;
#endif
         M1 = -M0;
         I5 = r9 + $aac.mem.TMP;
         L5 = 0;
#ifdef BASE_REGISTER_MODE
         push Null;  pop B5;
#endif
         M0 = 0;
      jump ok_kai_2;
      second_half_kai_2:
         I5 = I4;
         L5 = L4;
#ifdef BASE_REGISTER_MODE
         push B4;  pop B5;
#endif
         r0 = M[I5, MK1];
         M0 = -M1;
         I4 = r9 + $aac.mem.TMP;
         L4 = 0;
#ifdef BASE_REGISTER_MODE
         push Null;  pop B4;
#endif
         M1 = 0;
   ok_kai_2:

   r4 = $aacdec.AUDIO_OUT_SCALE_AMOUNT;
   r0 = $aacdec.AUDIO_OUT_SCALE_AMOUNT/2;
   Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
   if NZ r4 = r0;
   r0 = 1;
   r2 = M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)];
   Null = r2 + 2;
   if Z r4 = r0;

   kaiser_outer_loop_2:
      r1 = M[I1, MK1];  // coef for x^0
      r2 = M[I1, MK1];  // coef for x^1
      r3 = M[I1, MK1];  // coef for x^2
      r0 = M[I1, MK2];  // coef for x^3 (re-read in inside loop)
                     // coef for x^4 (read in inside loop)
      r0 = M[I1, -MK1];  // number of values generated with these coefs
      r10 = r0;

      r0 = M[I1, -MK1];  // dummy read

      // window, scale and overlap add loop
      do kaiser_inner_loop_2;
         rMAC = r1;
         push r1;
         rMAC = rMAC + r6 * r2;
         r0 = r6 * r6 (frac);  // r6 = x^2;
         rMAC = rMAC + r0 * r3,
          r1 = M[I1, MK1];
         r0 = r0 * r6 (frac);  // r6 = x^3;
         rMAC = rMAC + r0 * r1,
          r1 = M[I1, MK1];
         r0 = r0 * r6 (frac);  // r6 = x^4;
         rMAC = rMAC + r0 * r1,
          r0 = M[I3,M3];
         r0 = r0 * rMAC (frac),
          r1 = M[I1, -MK1];  // dummy read
         r0 = r0 * r7 (int) (sat),
          r1 = M[I4,0];
         r0 = r0 + r1;
         r0 = r0 * r4 (int) (sat);
         r6 = r6 + r5,  // increment x;
          M[I4,M1] = r0,
          r0 = M[I2,M2];
         r0 = rMAC * r0 (frac);
         r0 = r0 * r8 (int) (sat),
          r1 = M[I5,0];
         r0 = r0 + r1,
          r1 = M[I1, -MK1];  // dummy read
         r0 = r0 * r4 (int) (sat);
         M[I5,M0] = r0;
         pop r1;
      kaiser_inner_loop_2:

      I1 = I1 + (3*ADDR_PER_WORD);
      r6 = 0;
      Null = I1 - I6;
   if NZ jump kaiser_outer_loop_2;
   r2 = M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)];
   Null = r2 + 2;
   if Z jump repeat_kaiser;
   // if I5 pointing at dummy need to use I4
   M0 = (65*ADDR_PER_WORD);
   r0 = r9 + $aac.mem.TMP + (3*ADDR_PER_WORD);
   Null = -M[r0];
   if NEG jump use_I4_kaiser;
      M0 = 0;
      L4 = L5;
#ifdef BASE_REGISTER_MODE
      push B5;  pop B4;
#endif
      jump use_I4_kaiser_done;
   use_I4_kaiser:
      I5 = I4;

   use_I4_kaiser_done:
   L5 = L4;
#ifdef BASE_REGISTER_MODE
   push B4;  pop B5;
#endif

   r0 = M[I5,M0];
   // reset I2,I3 to original values
   r0 = I2 + (127*ADDR_PER_WORD);
   M2 = -M2;
   if NZ I2 = r0;
   r0 = I3 + (129*ADDR_PER_WORD);
   M3 = -M3;
   if NZ I3 = r0;
   r2 = M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)];
   Null = r2 + 3;
   if NZ rts;
   r7 = M[r9 + $aac.mem.TMP + (12*ADDR_PER_WORD)];
   r8 = M[r9 + $aac.mem.TMP + (13*ADDR_PER_WORD)];
   rts;


   repeat_kaiser:
   r2 = -3;
   M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)] = r2;
   r6 = I1 - (1*ADDR_PER_WORD);
   r6 = M[r6];
   r6 = r5 * r6 (int);
   jump end_kaiser_outer_loop_1;


   non_mirrored_kaiser:
      r8 = r10 - 1;
      Words2Addr(r8);
      r7 = I6;
      I6 = I6 + (18*ADDR_PER_WORD);
      Null = M[r9 + $aac.mem.TMP + (5*ADDR_PER_WORD)];
      if POS jump second_half_windowing;
         // store I3,M3 then set up I3,M3 based on I2,M2
         r1 = I3;
         M[r9 + $aac.mem.TMP + (7*ADDR_PER_WORD)] = r1;
         r1 = M3;
         M[r9 + $aac.mem.TMP + (8*ADDR_PER_WORD)] = r1;
         I3 = I2 - r8;
         M3 = -M2;
         jump non_mirrored_kaiser_outer_loop;
      second_half_windowing:
         // work along buffers in other direction (for 2nd half windowing)
         r1 = M0;
         M0 = r8;
         r2 = M[I5, M0];
         M0 = -r1;
         I3 = I3 + r8;
         M3 = -M3;

      non_mirrored_kaiser_outer_loop:
      // don't reset r6 if half way through section
      Null = I1 - r7;
      if NZ r6 = 0;

      r1 = M[I1, MK1];  // coef for x^0
      M1 = r1;
      r2 = M[I1, MK1];  // coef for x^1
      r3 = M[I1, MK1];  // coef for x^2
      r4 = M[I1, MK1];  // coef for x^3
      r1 = M[I1, MK1];  // coef for x^4
      r0 = M[I1, MK1];  // number of values generated with these coefs
      r10 = r0;

      do non_mirrored_kaiser_inner_loop;
         rMAC = M1;
         rMAC = rMAC + r6 * r2;
         r0 = r6 * r6 (frac);  // r6 = x^2;
         rMAC = rMAC + r0 * r3;
         r0 = r0 * r6 (frac);  // r6 = x^3;
         rMAC = rMAC + r0 * r4;
         r0 = r0 * r6 (frac);  // r6 = x^4;
         rMAC = rMAC + r0 * r1;
         r6 = r6 + r5,  // increment x;
          r0 = M[I3,M3];
         r0 = rMAC * r0 (frac);
         M[I5,M0] = r0;
      non_mirrored_kaiser_inner_loop:

      Null = I1 - I6;
   if NZ jump non_mirrored_kaiser_outer_loop;

   // reset 'I's and 'M's
   Null = M[r9 + $aac.mem.TMP + (5*ADDR_PER_WORD)];
   if POS jump restore_second_half_windowing;
   r1 = M[r9 + $aac.mem.TMP + (7*ADDR_PER_WORD)];
   I3 = r1;
   r1 = M[r9 + $aac.mem.TMP + (8*ADDR_PER_WORD)];
   M3 = r1;
   jump shift_i2_i3;

   restore_second_half_windowing:
   M0 = r8 + (1*ADDR_PER_WORD);
   r0 = M[I5, M0];
   I3 = I3 + (1*ADDR_PER_WORD);
   M3 = -M3;


   shift_i2_i3:
      r0 = I2 + (128*ADDR_PER_WORD);
      Null = M2;
      if NZ I2 = r0;
      r0 = I3 + (128*ADDR_PER_WORD);
      Null = M3;
      if NZ I3 = r0;
      rts;

.ENDMODULE;

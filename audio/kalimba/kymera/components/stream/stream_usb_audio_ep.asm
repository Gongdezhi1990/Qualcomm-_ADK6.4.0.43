#ifndef STREAM_USB_AUDIO_EP_ASM_INCLUDED
#define STREAM_USB_AUDIO_EP_ASM_INCLUDED
#include "stack.h"
#include "patch/patch_asm_macros.h"

/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_udb_audio_ep.asm
 * \ingroup stream
 *
 * stream usb audio type file. <br>
 *
 * This file contains some asm functions used by usb audio endpoint,
 * for speed reason these are done in assembly.
 */

#ifdef KAL_ARCH4
// *****************************************************************************
// MODULE:
//    $_stream_usb_audio_read_audio_subframes
//
// DESCRIPTION:
//    reads and unpack audio subframes from packed usb audio raw buffer
//
// INPUTS:
//
//    - r0 =  pointer to local cbuffer for writing audio subframes into
//    - r1 =  pointer to source cbuffer, this is normally a local mmu buffer
//    - r2 =  size of subframes in bits, can be 16, 24 or 32
//    - r3 =  maximum amount of data to read from source buffer in OCTETs.
//
// OUTPUTS:
//    - r0 = number of audio subframes written into the sink buffer
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//   - None of the buffers can be remote, this function doesn't check it.
//
// *****************************************************************************
.MODULE $M.stream_usb_audio_read_audio_subframes;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;

$_stream_usb_audio_read_audio_subframes:

   /* save the registers that are used */
   pushm <FP(=SP), r0, r1, r2, r3, rLink>;
   pushm <I0, I4, L0, L4>;
   pushm <r4,r5,r6,r7,r8>;
   pushm <B0,B4>;
   push M0;

   // patch point
   LIBS_SLOW_SW_ROM_PATCH_POINT($usb_audio_ep.STREAM_USB_AUDIO_EP_ASM.READ_AUDIO_SUBFRAMES.PATCH_ID_0, r7)

   // get the amount of data in the source buffer
   r0 = M[FP + 2*ADDR_PER_WORD];
   call $cbuffer.calc_amount_data_in_addrs;
   // limit the amount to read to
   // maximum octets
   r5 = M[FP + 4*ADDR_PER_WORD];
   r5 = MIN r0;
   if Z jump all_done;

   // read the amount space available in sink buffer
   r0 = M[FP + 1*ADDR_PER_WORD];
   call $cbuffer.calc_amount_space_in_words;
   r8 = r0;

   // see if we have space to read the whole frame
   r2 = M[FP + 3*ADDR_PER_WORD];   // word length in bits
   r2 = r2 LSHIFT -3;              // word length in octets
   r2 = r2 * r8 (int);             // max octets can be handled
   Null = r2 - r5;
   if POS jump process_full_packet;
discard_packet:

      /* If we cannot read a full packet,  we just discard it. */
      r0 = M[FP + 2*ADDR_PER_WORD]; /* input buffer */
      r1 = r5;                      /* amount data to discard */
      call $stream_usb_audio_discard_octets_from_source_buffer;
      r5 = 0; /* no subframe written */
      jump all_done;
 process_full_packet:

   // ---- 16-bit audio handling ---
   // open sink buffer for writing
   r0 = M[FP + 1*ADDR_PER_WORD];
   call $cbuffer.get_write_address_and_size_and_start_address;
   I4 = r0;
   L4 = r1;
   push r2;
   pop B4;

   // open source buffer for reading
   r0 = M[FP + 2*ADDR_PER_WORD];
   call $cbuffer.get_read_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;

   // for reading/writing 1 full word from/to buffers
   M0 = ADDR_PER_WORD;

   // get the word length
   r2 = M[FP + 3*ADDR_PER_WORD];

   // see if it's 24-bit usb-audio
   Null = r2 - 24;
   if Z jump handle_24bit_subframe;

   // see if it's 16-bit usb-audio
   Null = r2 - 16;
   if Z jump handle_16bit_subframe;

   // see if it's 32-bit usb-audio
   Null = r2 - 32;
   if Z jump handle_32bit_subframe;

handle_8bit_subframe:
   // no other word length is supported
   L0 = 0;
   L4 = 0;
   jump discard_packet;

handle_32bit_subframe:
   // ---- 32-bit audio handling ---
   // use cbuffer copy, expect it is word aligned

#ifdef DEBUG_ON
   /* 32-bit usb-audio is expected to
    * be always full-word aligned
    */
   r0 = I0;
   Null = r0 AND 0x3;
   if NZ call $error;
#endif

   r0 = M[FP + 1*ADDR_PER_WORD];       /* sink buffer */
   r1 = M[FP + 2*ADDR_PER_WORD];       /* source buffer */
   r2 = r5 LSHIFT -LOG2_ADDR_PER_WORD; /* number of words to read */
   r2 = MIN r8;                        /* max words we can copy */
   call $_cbuffer_copy;
   r5 = r0;
   jump all_done;

handle_16bit_subframe:
   // I0/L0/B0: source buffer read address (must be half word aligned)
   // I4/L4/B4: sink buffer write address (must be word aligned)
   // r5: amount to read from source buffer in octets
   // r8 = amount of space in the output buffer in samples
   //  M0 = 4 (ADDR_PER_WORD)
   call $stream_usb_audio_read_16bit_raw_data;
   jump update_buffers;

handle_24bit_subframe:

   // ---- 24-bit audio handling ----
   // I0/L0/B0: source buffer read address
   // I4/L4/B4: sink buffer write address (must be word aligned)
   // r5: amount to read from source buffer in octets
   // r8 = amount of space in the output buffer in samples
   // M0 = 4 (ADDR_PER_WORD)
   call $stream_usb_audio_read_24bit_raw_data;
update_buffers:
      // I4: updated write pointer for sink buffer
      // I0: update read pointer for source buffer

      /* close destination buffer */
      r0 = M[FP + 1*ADDR_PER_WORD];
      r1 = I4;
      call $cbuffer.set_write_address;

      /* close source buffer */
      r0 = M[FP + 2*ADDR_PER_WORD];
      r1 = I0;
      call $cbuffer.set_read_address;

   all_done:
   // r5 holds the number of subframes read,
   // here we save it onto stack which will be
   // popped into r0 at the end
   M[FP + 1*ADDR_PER_WORD] = r5;

   // restore the registers from stack
   pop M0;
   popM <B0,B4>;
   popm <r4,r5,r6,r7,r8>;
   popm <I0, I4, L0, L4>;
   popm <FP, r0, r1, r2, r3, rLink>;

   // r0 will have the number of subframes received
   rts;
.ENDMODULE;
// *****************************************************************************
// MODULE:
//    $_stream_usb_audio_write_audio_subframes
//
// DESCRIPTION:
//    packs and writes audio subframes into usb audio raw buffer
//
// INPUTS:
//    - r0 = destination buffer, this is normally a local mmu buffer
//    - r1 = source cbuffer containing audio subframes
//    - r2 = size of subframes in bits, can be 16, 24 or 32
//    - r3 = maximum amount of data to write to sink buffer in OCTETs
//
// OUTPUTS:
//    - r0 = number of subframes written into sink buffer
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//   - None of the buffers can be remote, this function doesn't check it.
// *****************************************************************************
.MODULE $M.stream_usb_audio_write_audio_subframes;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;

$_stream_usb_audio_write_audio_subframes:

   /* save the registers that are used */
   pushm <FP(=SP), r0, r1, r2, r3, rLink>;
   pushm <I0, I4, L0, L4>;
   pushm <r4,r5,r6,r7,r8>;
   pushm <B0,B4>;
   push M0;

   // patch point
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($usb_audio_ep.STREAM_USB_AUDIO_EP_ASM.WRITE_AUDIO_SUBFRAMES.PATCH_ID_0, r7)

   // for reading/writing from/to buffers
   M0 = ADDR_PER_WORD;

   // get the amount of octets we can write
   call $cbuffer.calc_amount_space_in_addrs;
   // limit to maximum octets to write
   r8 = M[FP + 4*ADDR_PER_WORD];
   r8 = MIN r0;
   if Z jump all_done;

   r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
   call $cbuffer.calc_amount_data_in_words;
   r5 = r0;

   // r8 space available in addrs
   // r5 data available in words

   // open sink buffer for writing
   r0 = M[FP + 1*ADDR_PER_WORD];
   call $cbuffer.get_write_address_and_size_and_start_address;
   I4 = r0;
   L4 = r1;
   push r2;
   pop B4;

   // open source buffer for reading
   r0 = M[FP + 2*ADDR_PER_WORD];                // cbuffer_src
   call $cbuffer.get_read_address_and_size_and_start_address;
   I0 = r0;
   L0 = r1;
   push r2;
   pop B0;

   // get the word length
   r2 = M[FP + 3*ADDR_PER_WORD];

   // see if it's 16-bit usb-audio
   Null = r2 - 16;
   if Z jump handle_16bit_subframe;

   // see if it is 24-bit usb-audio
   Null = r2 - 24;
   if Z jump handle_24bit_subframe;

   // see if it is 24-bit usb-audio
   Null = r2 - 32;
   if Z jump handle_32bit_subframe;

   // we don't support other word length,
   // do nothing
   // TODO: we could discard
   jump all_done;

handle_32bit_subframe:
   // ---- 32-bit audio handling ----
   // use cbuffer copy
#ifdef DEBUG_ON
   /* 32-bit usb-audio is expected to
    * be always full-word aligned
    */
   r0 = I4;
   Null = r0 AND 0x3;
   if NZ call $error;
#endif
   r0 = M[FP + 1*ADDR_PER_WORD];       /* sink buffer */
   r1 = M[FP + 2*ADDR_PER_WORD];       /* source buffer */
   r2 = r8 LSHIFT -LOG2_ADDR_PER_WORD; /* number of words to read */
   r2 = MIN r5;                        /* max words we can copy */
   call $_cbuffer_copy;
   r5 = r0;
   jump all_done;

handle_16bit_subframe:
   // write 16-bit per subframe
   call $stream_usb_audio_write_16bit_raw_data;
   jump update_buffers;

handle_24bit_subframe:

   // write 24-bit per subframe
   call $stream_usb_audio_write_24bit_raw_data;

update_buffers:
      r0 = M[FP + 1*ADDR_PER_WORD];
      r1 = I4;
      call $cbuffer.set_write_address;
      r0 = M[FP + 2*ADDR_PER_WORD];
      r1 = I0;
      call $cbuffer.set_read_address;

   all_done:
   // Restore index & length registers
   M[FP + 1*ADDR_PER_WORD] = r5;
   pop M0;
   popm <B0, B4>;
   popm <r4,r5,r6,r7,r8>;
   popm <I0, I4, L0, L4>;
   popm <FP, r0, r1, r2, r3, rLink>;
   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $stream_usb_audio_read_16bit_raw_data
//
// DESCRIPTION:
//    read 24bit usb audio raw data into audio subframe buffer
//
// INPUTS:
//    - r5 = amount of data in input buffer, in octets
//    - r8 = amount of space in the output buffer in samples
//    - I0/L0/B0: input buffer
//    - I4/L4/B4: output buffer
//    - M0 = 4 (ADDR_PER_WORD)
//
// OUTPUTS:
//    - r5 number of subframes written
//    - I0: updated
//    - I4: updated
//
// TRASHED REGISTERS:
//    r0-r8, rMAC
//
// NOTES:
//   - This ISN'T a c callable function
//   - This isn't a usb_audio specific function, might be moved to a more generic place
//
// *****************************************************************************
.MODULE $M.stream_usb_audio_read_16bit_raw_data;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;

$stream_usb_audio_read_16bit_raw_data:

   // patch point
   LIBS_SLOW_SW_ROM_PATCH_POINT($usb_audio_ep.STREAM_USB_AUDIO_EP_ASM.READ_16BIT_RAW_DATA, r7)

   // r8 space available in words
   // r5 data available in octets
   // r5 <- r5/2
   r5 = r5 LSHIFT -1;

   // r8 = total number of words we will write
   r8 = MIN r5;
   r5 = r8;
   if Z rts;  // this shouldn't happen

   r0 = I0;
   r0 = r0 AND 0x3;
#ifdef DEBUG_ON
   /* 16-bit usb-audio is expected to
    * be always half-word aligned
    */
   Null = r0 AND 1;
   if NZ call $error;
#endif
   I0 = I0 - r0;
   NULL = r0;
   if Z jump is_word_aligned_now;
   r0 = M[I0, M0];              // r0 = 1122xxxx
   r0 = r0 AND 0xFFFF0000;      // r0 = 11220000
   r8 = r8 - 1;                 // one more word written
   if Z rts;
   // --- at this point reading point is word aligned ---

is_word_aligned_now:
   // in the loop below, we write
   // 2 subframes per iteration
   r10 = r8 LSHIFT -1;
   if Z jump loop_done;

   // setup some registers for the reading loop
   r6 = 16;
   r7 = 0xFFFF0000;
   r10 = r10 - 1;
   r0 = M[I0, M0];
   do read_16bit_words_loop;
      //  source buffer: 0x33441122
      //  subframes to extract:    0x11220000 0x33440000
      r1 = r0 LSHIFT r6;               // r6 = 11220000
      r0 = r0 AND r7, M[I4, M0] = r1;  // r0 = 33440000
      M[I4, M0] = r0, r0 = M[I0, M0];
   read_16bit_words_loop:
   r1 = r0 LSHIFT r6;
   r0 = r0 AND r7, M[I4, M0] = r1;
   M[I4, M0] = r0;

   loop_done:
   // see if a single subframe
   // needs to be read
   Null = r8 AND 0x1;
   if Z rts;
   r0 = M[I0, 0];          // r0 = xxxx1122
   r0 = r0 LSHIFT 16;      // r0 = 11220000
   M[I4, M0] = r0;         // write new subframe
   I0 = I0 + 2;            // move to B2 offset
   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $stream_usb_audio_read_24bit_raw_data
//
// DESCRIPTION:
//    read 24bit usb audio raw data into audio subframe buffer
//
// INPUTS:
//    - r5 = amount of data in input buffer, in octets
//    - r8 = amount of space in the output buffer in samples
//    - I0/L0/B0: input buffer
//    - I4/L4/B4: output buffer
//    - M0 = 4 (ADDR_PER_WORD)
//
// OUTPUTS:
//    - r5 number of subframe written
//    - I0: updated
//    - I4: updated
//
// TRASHED REGISTERS:
//    r0-r8, rMAC
//
// NOTES:
//   - This ISN'T a c callable function
//   - This isn't a usb_audio specific function, might be moved to a more generic place
//
// *****************************************************************************
.MODULE $M.stream_usb_audio_read_24bit_raw_data;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;

$stream_usb_audio_read_24bit_raw_data:

   // patch point
   LIBS_SLOW_SW_ROM_PATCH_POINT($usb_audio_ep.STREAM_USB_AUDIO_EP_ASM.READ_24BIT_RAW_DATA, r7)

   // r8 space available in words
   // r5 data available in octets

   // work out maximum subframes can be read
   // r5 <- r5/3
   r5 = r5 - 1;
   r5 = r5 * (1.0/3.0) (frac);

   // r8 = total number of words we will write
   r8 = MIN r5;
   r5 = r8;
   if Z rts;  // this shouldn't happen

   r0 = I0;
   r0 = r0 AND 0x3;
   I0 = I0 - r0;

   // see if it's already word aligned
   NULL = r0;
   if Z jump is_word_aligned_now;

   // half word aligned?
   Null = r0 - 2;
   if Z jump check_rem_2;

   // or 1 octet from word boundary
   if NEG jump check_rem_1;
check_rem_3:

   // starts from octet 3
   r0 = M[I0, M0];              // r0 = 33xxxxxx
   r1 = M[I0, 0];               // r1 = xxxx1122
   r0 = r0 LSHIFT -24;          // r0 = 00000033
   r1 = r1 LSHIFT 8;            // r1 = xx112200
   r1 = r1 OR r0;               // r1 = xx112233
   r1 = r1 LSHIFT 8;            // r1 = 11223300
   M[I4, M0] = r1;              // write new subframe
   r4 = I0 + 2;                 // move to B2 offset
   r8 = r8 - 1;                 // one more word written
   if Z jump read_24bit_complete;
check_rem_2:

   // starts from octet 2 (half word aligned)
   r0 = M[I0, M0];              // r0 = 2233xxxx
   r1 = M[I0, 0];               // r1 = xxxxxx11
   r1 = r1 LSHIFT 24;           // r1 = 11000000
   r0 = r0 LSHIFT -16;          // r0 = 00002233
   r0 = r0 LSHIFT 8;            // r0 = 00223300
   r1 = r1 OR r0;               // r1 = 11223300
   M[I4, M0] = r1;              // write new subframe
   r4 = I0 + 1;                 // move to B1 offset
   r8 = r8 - 1;                 // one more word written
   if Z jump read_24bit_complete;
check_rem_1:

   // start from octet 1
   r8 = r8 - 1, r1 = M[I0, M0]; // r1 = 112233xx
   r1 = r1 AND 0xFFFFFF00;      // r1 = 11223300
   M[I4, M0] = r1;              // write new subframe

   // --- at this point reading point is word aligned ---

is_word_aligned_now:
   // r8: number of subframes to read
   // I0: read address
   // I4: write address

   // in the loop below, we write
   // 4 subframes per iteration
   r10 = r8 LSHIFT -2;

   // remaining subframe at the end
   // of loop
   r8 = r8 AND 0x3;

   // setup some registers for the reading loop
   r4 = -24;
   r6 = -8;

   // r7 = amount to shift to
   r7 = 8;

   // TODO: unwinding loop can save at least 1 instruction per iteration
   do read_24bit_words_loop;
      //  source buffer: 0x66112233 0x88994455 0xaabbcc77
      //  subframes to extract:    0x112233 0x445566 0x778899 0xaabbcc
      r1 = M[I0,M0];                                 // r1 = 66112233
      r0 = r1 LSHIFT r7, r2 = M[I0,M0];              // r0 = 33221100, r2 = 88994455
      rMAC = r2, r3 = M[I0,M0];                      // rMAC = xx 88994455 00000000, r3 = aabbcc77
      rMAC0 = r1;                                    // rMAC = xx 88994455 66112233
      rMAC2 = r3 LSHIFT 0;                           // rMAC = 77 88994455 66112233
      rMAC = rMAC LSHIFT r4 (56bit), M[I4, M0] = r0; // rMAC = 00 00007788 99445566
      r0 = rMAC0;                                    // r0 = 99445566
      r0 = r0 LSHIFT r7;                             // r0 = 44556600
      rMAC = rMAC LSHIFT r4 (56bit), M[I4, M0] = r0; // rMAC = 00 00000000 00778899
      r0 = rMAC0;                                    // r0 = 00778899
      r0 = r0 LSHIFT r7;                             // r0 = 77889900
      r3 = r3 LSHIFT r6, M[I4, M0] = r0;             // r3 = 00aabbcc
      r3 = r3 LSHIFT r7;                             // r3 = aabbcc00
      M[I4, M0] = r3;
   read_24bit_words_loop:

   r4 = I0;
   Null = r8;
   if Z jump read_24bit_complete;
   r4 = I0 + 3;            // next move to B3
   r0 = M[I0, 0];          // r0 = xx112233
   r0 = r0 LSHIFT 8;       // r0 = 11223300
   M[I4, M0] = r0;         // write new subframe
   r8 = r8 - 1;            // one more subframe done
   if Z jump read_24bit_complete;

   r0 = M[I0, M0];         // r0 = 33xxxxxx
   r4 = I0 + 2;            // next move to B2
   r1 = M[I0, 0];          // r1 = xxxx1122
   r0 = r0 LSHIFT -24;     // r0 = 00000033
   r1 = r1 LSHIFT 8;       // r1 = xx112200
   r1 = r1 OR r0;          // r1 = xx112233
   r1 = r1 LSHIFT 8;       // r1 = 11223300
   M[I4, M0] = r1;         // write new subframe
   r8 = r8 - 1;            // one more subframe done
   if Z jump read_24bit_complete;

   r0 = M[I0, M0];         // r0 = 2233xxxx
   r1 = M[I0, 0];          // r1 = xxxxxx11
   r4 = I0 + 1;            // next move to B1
   r1 = r1 LSHIFT 24;      // r1 = 11000000
   r0 = r0 LSHIFT -16;     // r0 = 00002233
   r0 = r0 LSHIFT 8;       // r0 = 00223300
   r1 = r1 OR r0;          // r1 = 11223300
   M[I4, M0] = r1;         // write new subframe
read_24bit_complete:
   I0 = r4;
   rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $stream_usb_audio_write_16bit_raw_data
//
// DESCRIPTION:
//    write audio subframes into 16bit usb audio raw data buffer
//
// INPUTS:
//    - r8 = amount of space in output buffer, in octets
//    - r5 = amount of data in the input buffer in samples
//    - I0/L0/B0: input buffer
//    - I4/L4/B4: output buffer
//    - M0 = 4 (ADDR_PER_WORD)
//
// OUTPUTS:
//    - r5 number of subframes written
//    - I0: updated
//    - I4: updated
//
// TRASHED REGISTERS:
//    r0-r8, rMAC
//
// NOTES:
//   - This ISN'T a c callable function
//   - This isn't a usb_audio specific function, might be moved to a more generic place
//
// *****************************************************************************
.MODULE $M.stream_usb_audio_write_16bit_raw_data;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;

$stream_usb_audio_write_16bit_raw_data:

   // patch point
   LIBS_SLOW_SW_ROM_PATCH_POINT($usb_audio_ep.STREAM_USB_AUDIO_EP_ASM.WRITE_16BIT_RAW_DATA, r6)

   // 24-bit audio, each 3 input words will generate
   // 4 audio subframe.

   // r8 space available in addrs
   // r5 data available in words

   // r8 <- r8/2 : max subframes that can be written
   r8 = r8 LSHIFT -1 ;

   r5 = MIN r8;
   r10 = r5;
   if Z rts;

   r0 = I4;
   r0 = r0 AND 0x3;
   if Z jump is_word_aligned_now;
check_rem_1:
   r0 = M[I0, M0];       // r0 = 1122xxxx
   r0 = r0 LSHIFT -16;   // r0 = 00112233
   MH[I4] = r1;          // write: 1122----
   I4 = I4 - 2;          // I4 = start of the word
   r1 = M[I4, M0];       // go to next word
   r10 = r10 - 1;        // one more word written
   if Z rts;
is_word_aligned_now:

   // r10 <- r10/2
   r8 = r10 AND 0x1;
   r10 = r10 LSHIFT -1;
   if Z jump loop_done;

   // setup some registers for the reading loop
   r4 = 0xFFFF0000;
   r6 = -16;
   do write_words_16_loop;
      //  subframes:     0x1122xxxx 0x3344xxxx
      //  output buffer: 0x33441122
      r0 = M[I0, M0];
      r0 = r0 LSHIFT r6, r1 = M[I0, M0];
      r1 = r1 AND r4;
      r0 = r0 OR r1;
      M[I4, M0] = r0;
   write_words_16_loop:

loop_done:
   Null = r8;
   if Z jump write_16bit_complete;
      r0 = M[I0, M0];      // r0 = 1122xxxx
      r0 = r0 LSHIFT -16;   // r0 = 00001122
      MH[I4] = r0;         // write: ----1122
      I4 = I4 + 2;         // move I4 to B2
write_16bit_complete:

   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $stream_usb_audio_write_24bit_raw_data
//
// DESCRIPTION:
//    write audio subframes into 24bit usb audio raw data buffer
//
// INPUTS:
//    - r8 = amount of space in output buffer, in octets
//    - r5 = amount of data in the input buffer in samples
//    - I0/L0/B0: input buffer
//    - I4/L4/B4: output buffer
//    - M0 = 4 (ADDR_PER_WORD)
//
// OUTPUTS:
//    - r5 number of subframes written
//    - I0: updated
//    - I4: updated
//
// TRASHED REGISTERS:
//    r0-r8, rMAC
//
// NOTES:
//   - This ISN'T a c callable function
//   - This isn't a usb_audio specific function, might be moved to a more generic place
//
// *****************************************************************************
.MODULE $M.stream_usb_audio_write_24bit_raw_data;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;

$stream_usb_audio_write_24bit_raw_data:

   // patch point
   LIBS_SLOW_SW_ROM_PATCH_POINT($usb_audio_ep.STREAM_USB_AUDIO_EP_ASM.WRITE_24BIT_RAW_DATA, r6)

   // 24-bit audio, each 3 input words will generate
   // 4 audio subframe.

   // r8 space available in addrs
   // r10 data available in words

   // r8 <- r8/3 : max subframes that can be written
   r8 = r8 - 1;
   r8 = r8 * (1.0/3.0)(frac);

   r5 = MIN r8;
   r10 = r5;
   if Z rts;

   r0 = I4;
   r0 = r0 AND 0x3;
   if Z jump is_word_aligned_now;
   Null = r0 - 2;
   if Z jump check_rem_2;
   if NEG jump check_rem_1;
check_rem_3:
   r0 = M[I0, M0];       // r0 = 112233xx
   r0 = r0 LSHIFT -8;    // r0 = 00112233
   MB[I4] = r1;          // write: 33------
   I4 = I4 - 3;          // I4 = start of the word
   r1 = M[I4, M0];       // go to next word
   r0 = r0 LSHIFT -8;    // r0 = 00001122
   MH[I4] = r0;          // write: ----1122
   I4 = I4 + 2;          // move I4 to B2
   r10 = r10 - 1;        // one more word written
   if Z rts;
check_rem_2:
   r0 = M[I0, M0];       // r0 = 112233xx
   r0 = r0 LSHIFT -8;    // r0 = 00112233
   MH[I4] = r0;          // write: 2233----
   r0 = r0 LSHIFT -16;   // r0 = 00000011
   I4 = I4 - 2;          // I4 = start of the word
   r1 = M[I4, M0];       // go to next word
   MB[I4] = r0;          // write: ------11
   I4 = I4 + 1;          // move I4 to B1
   r10 = r10 - 1;        // one more word written
   if Z rts;
check_rem_1:
   r0 = M[I0, M0];       // r0 = 112233xx
   r0 = r0 LSHIFT -8;    // r0 = 00112233
   MB[I4] = r0;          // write: ----33--
   I4 = I4 + 1;          // move I4 to B2
   r0 = r0 LSHIFT -8;    // r0 = 00001122
   MH[I4] = r0;          // write: 1122----
   I4 = I4 - 2;          // I4 = start of the word
   r1 = M[I4, M0];       // go to next word
   r10 = r10 - 1;        // one more word written
   if Z rts;

is_word_aligned_now:

   // r10 <- r10/4
   r8 = r10 AND 0x3;
   r10 = r10 LSHIFT -2;
   if Z jump loop_done;

   // setup some registers for the reading loop
   r4 = -16;
   r6 = -8;
   do write_words_24_loop;
      //  subframes:     0x112233xx 0x445566xx 0x778899xx 0xaabbccxx
      //  output buffer: 0x66112233 0x88994455 0xaabbcc77
      r1 = M[I0, M0];                                // r1 = 112233xx
      rMAC0 = r1 LSHIFT 0;                           // rMAC = xx xxxxxxxx 112233xx
      r2 = M[I0, M0];                                // r2 = 445566xx
      rMAC12 = r2 LSHIFT -8;                         // rMAC = 00 00445566 112233xx
      rMAC = rMAC LSHIFT r6 (56bit), r3 = M[I0, M0]; // rMAC = 00  00004455  66112233, r3 = 0x778899xx
      r0 = rMAC0;                                    // r0 = w0
      rMAC = rMAC LSHIFT r4(56bit), M[I4, M0] = r0;  // rMAC = 00  00000000  44556611
      rMAC12 = r3 LSHIFT -8;                         // rMAC = 00  00778899  44556611
      rMAC = rMAC LSHIFT r4(56bit), r2 = M[I0, M0];  // rMAC = 00  00000077  88994455
      r0 = rMAC0;                                    // r0 = 88994455
      r2 = r2 AND 0xFFFFFF00;                        // r2 = aabbcc00
      r3 = rMAC1;                                    // r3 = 00000077
      r2 = r2 OR r3, M[I4, M0] = r0;                 // r2 = aabbcc77
      M[I4, M0] = r2;
   write_words_24_loop:

loop_done:
   Null = r8;
   if Z jump write_24bit_complete;
      r0 = M[I0, M0];      // r0 = 112233xx
      r0 = r0 LSHIFT -8;   // r0 = 00112233
      MH[I4] = r0;         // write: ----2233
      r0 = r0 LSHIFT -16;  // r0 = 00000011
      I4 = I4 + 2;         // move I4 to B2
      MB[I4] = r0;         // write: --11----
      I4 = I4 + 1;         // move I4 to B3
      r8 = r8 - 1;         // one more word written
   if Z jump write_24bit_complete;
      r0 = M[I0, M0];      // r0 = 112233xx
      r0 = r0 LSHIFT -8;   // r0 = 00112233
      MB[I4] = r0;         // write 33------
      I4 = I4 - 3;         // I4 = start of the word
      r1 = M[I4, M0];      // move to next word
      r0 = r0 LSHIFT -8;   // r0 = 00001122
      MH[I4] = r0;         // write: ----1122
      I4 = I4 + 2;         // move to B2
      r8 = r8 - 1;         // one more word written
   if Z jump write_24bit_complete;
      r0 = M[I0, M0];      // r0 = 112233xx
      r0 = r0 LSHIFT -8;   // r1 = 00112233
      MH[I4] = r0;         // write 2233----
      I4 = I4 - 2;         // I4 = start of the word
      r1 = M[I4, M0];      // move to next word
      r0 = r0 LSHIFT -16;  // r0 = 00000011
      MB[I4] = r0;         // write: ------11
      I4 = I4 + 1;         // move to B1
write_24bit_complete:

   rts;

.ENDMODULE;
// *****************************************************************************
// MODULE:
//    $stream_usb_audio_discard_octets_from_source_buffer
//
// DESCRIPTION:
//    utility function to discard source
//
// INPUTS:
//    - r0 = source buffer
//    - r1 = number of octets
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//
// NOTES:
//   - C calling convention respected.
//   - This isn't a usb_audio specific function
//
// *****************************************************************************
.MODULE $M.stream_usb_audio_discard_octets_from_source_buffer;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;

$stream_usb_audio_discard_octets_from_source_buffer:
$_stream_usb_audio_discard_octets_from_source_buffer:

   pushm <FP(=SP), r0, r1, rLink>;
   pushm <I0, L0>;
   push B0;
   push M0;

   // patch point
   LIBS_SLOW_SW_ROM_PATCH_POINT($usb_audio_ep.STREAM_USB_AUDIO_EP_ASM.DISCARD_OCTETS_FROM_SOURCE_BUFFER, r1)

   r0 = M[FP + 1*ADDR_PER_WORD];
   call $cbuffer.get_read_address_and_size_and_start_address;

   L0 = r1;
   I0 = r0;
   push r2;
   pop B0;
   r1 = r0 AND 0x3;                // read address is byte aligned
   I0 = I0 - r1;                   // move back to the start of word
   r0 = M[FP + 2*ADDR_PER_WORD];   // get the number of octets to discard
   r0 = r0 + r1;                   // add number of octets we moved back
   r1 = r1 AND 0x3;                //
   M0 = r0 - r1;                   // M0 will be full words
   r0 = M[I0, M0];                 // move words
   r1 = I0 + r1;                   // move remaining octets
   r0 = M[FP +1*ADDR_PER_WORD];    //

   call $cbuffer.set_read_address; // set the read address
   pop M0;
   pop B0;
   popm <I0, L0>;
   popm <FP, r0, r1, rLink>;
   rts;

.ENDMODULE;
// *****************************************************************************
// MODULE:
//    $_stream_usb_audio_write_silence_packet
//
// DESCRIPTION:
//    writes silence into output buffer
//
// INPUTS:
//    - r0 = destination buffer, this is normally a local mmu buffer
//    - r1 = packet size in octets
//
// OUTPUTS:
//    - r0 = number of octets written into output buffer
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
// *****************************************************************************
.MODULE $M.stream_usb_audio_write_silence_packet;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;

$_stream_usb_audio_write_silence_packet:

   /* save the registers that are used */
   pushm <FP(=SP), r0, r1, r4, r6, r7, r8, rLink>;
   pushm <I4, L4>;
   pushm <B4>;

   // patch point
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($usb_audio_ep.STREAM_USB_AUDIO_EP_ASM.WRITE_SILENCE_PACKET.PATCH_ID_0, r7)

   // get the amount of octets we can write
   call $cbuffer.calc_amount_space_in_addrs;

   // don't write anything if there not enough
   // space for full packet
   r6 = 0;
   // get the packet length in octets, it's in stack frame offset 2
   r8 = M[FP + 2*ADDR_PER_WORD];
   NULL = r0 - r8;
   if NEG jump all_done;

   // full silence packet will be written
   r6 = r8;
   r4 = 0;

   // get the output buffer, it's in stack frame offset 1
   r0 = M[FP + 1*ADDR_PER_WORD];
   call $cbuffer.get_write_address_and_size_and_start_address;
   I4 = r0;
   L4 = r1;
   push r2;
   pop B4;

   // see if it is word aligned
   r0 = r0 AND (ADDR_PER_WORD -1);
   if Z jump is_word_aligned_now;


   // store address of next word in r1
   r1 = I4 - r0;
   r1 = r1 + ADDR_PER_WORD;

   // Not word aligned, write 1,2 or
   // 3 octets to make it word aligned
   r10 = ADDR_PER_WORD - r0;
   r10 = MIN r8;
   r8 = r8 - r10;
   do pre_write_loop;
      MB[I4] = r4;
      I4 = I4 + 1;
   pre_write_loop:

   // make sure we consider circular buffering
   // if just last octet of words written
   Null = I4 - r1;
   if NZ jump pre_write_done;
      // move back to beginning of the word
      // then go to next word "circularly"
      I4 = r1 - ADDR_PER_WORD;
      r1 = M[I4, ADDR_PER_WORD];
   pre_write_done:

   // all requested octets written?
   Null = r8;
   if Z jump update_buffer_address;
is_word_aligned_now:

   // now is word aligned and we can write full words
   // see how many full words are needed
   r10 = r8 LSHIFT -LOG2_ADDR_PER_WORD;
   r7 = r10 LSHIFT LOG2_ADDR_PER_WORD;
   do write_word_loop;
      M[I4, ADDR_PER_WORD] = r4;
   write_word_loop:

   // could be up to 3 octets remaining
   r10 = r8 - r7;
   if Z jump update_buffer_address;

   // write the remaining octets
   do post_write_loop;
      MB[I4] = r4;
      I4 = I4 + 1;
   post_write_loop:

update_buffer_address:

   /* close destination buffer (buufer address is in stack frame) */
   r0 = M[FP + 1*ADDR_PER_WORD];
   r1 = I4;
   call $cbuffer.set_write_address;

   all_done:
   // r6 is the number of octets written, i.e. the
   // return value, store it in stack, it will be
   // popped to r0 when restoring the stack
   M[FP + 1*ADDR_PER_WORD] = r6;

   // restore registers
   popm <B4>;
   popm <I4, L4>;
   popm <FP, r0, r1, r4, r6, r7, r8, rLink>;
   rts;
.ENDMODULE;

#endif // #ifdef KAL_ARCH4

#endif

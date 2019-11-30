/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \File spdif_frame_decode.asm
 *
 * This file contains some asm functions used by spdif decode operator
 * for speed reason these are done in assembly.
 * TODO:This file will be platform independent, however it hasn't been tested
 * on a 32-bit platform yet.
 *
 * \section sec1 Contains:
 * spdif_frame_decode <br>
 * spdif_copy_channels <br>
 * spdif_soft_mute_output_buffers <br>
 */
#ifndef SPDIF_FRAME_DECODE_ASM
#define SPDIF_FRAME_DECODE_ASM
#include "stack.h"

#if defined(PATCH_LIBS)
   #include "patch_library.h"
#endif

#ifdef CHIP_GORDON
    /* in GORDON we might have one sample shift */
    #define   SPDIF_STREAM_SHIFT_CHECK
#endif

   .CONST $SPDIF_DECODE_MAX_OUTPUT_CHANNELS 8;

   // defining data types, not all supported
   .CONST $spdif.DATA_TYPE_MUTED                           0x00;
   .CONST $spdif.DATA_TYPE_UNKNOWN                         0x01;
   .CONST $spdif.DATA_TYPE_PCM                             0x02;
   .CONST $spdif.CODED_DATA_TYPE_NULL                      0x00;
   .CONST $spdif.CODED_DATA_TYPE_AC3                       0x01;
   .CONST $spdif.CODED_DATA_TYPE_PAUSE                     0x03;
   .CONST $spdif.CODED_DATA_TYPE_MPEG1_LAYER1              0x04;
   .CONST $spdif.CODED_DATA_TYPE_MPEG1_LAYER23             0x05;
   .CONST $spdif.CODED_DATA_TYPE_MPEG2_EXTENTION           0x06;
   .CONST $spdif.CODED_DATA_TYPE_MPEG2_AAC                 0x07;
   .CONST $spdif.CODED_DATA_TYPE_MPEG2_LAYER1_LSF          0x08;
   .CONST $spdif.CODED_DATA_TYPE_MPEG2_LAYER2_LSF          0x09;
   .CONST $spdif.CODED_DATA_TYPE_MPEG2_LAYER3_LSF          0x0A;
   .CONST $spdif.CODED_DATA_TYPE_DTS1                      0x0B;
   .CONST $spdif.CODED_DATA_TYPE_DTS2                      0x0C;
   .CONST $spdif.CODED_DATA_TYPE_DTS3                      0x0D;
   .CONST $spdif.CODED_DATA_TYPE_ATRAC                     0x0E;
   .CONST $spdif.CODED_DATA_TYPE_ATRAC3                    0x0F;
   .CONST $spdif.CODED_DATA_TYPE_ATRACX                    0x10;
   .CONST $spdif.CODED_DATA_TYPE_DTSHD                     0x11;
   .CONST $spdif.CODED_DATA_TYPE_WMAPRO                    0x12;
   .CONST $spdif.CODED_DATA_TYPE_MPEG2_AAC_LSF             0x13;
   .CONST $spdif.CODED_DATA_TYPE_EAC3                      0x15;
   .CONST $spdif.CODED_DATA_TYPE_TRUEHD                    0x16;

   /* DATA TYPE includes all pcm and coded data types */
   #define $spdif.DATA_TYPE(coded_data_type)               (coded_data_type+3)

   // macro to enable data type
   #define $spdif.CODED_DATA_TYPE_SUPPORT(x)               (1<<(x))

   // audio frame length for some IEC-61937 data types
   .CONST $spdif.AC3_AUDIO_FRAME_LENGTH                    1536;
   .CONST $spdif.MPEG1_LAYER23_AUDIO_FRAME_LENGTH          1152;
   .CONST $spdif.DTS1_AUDIO_FRAME_LENGTH                   512;
   .CONST $spdif.DTS2_AUDIO_FRAME_LENGTH                   1024;
   .CONST $spdif.DTS3_AUDIO_FRAME_LENGTH                   2048;
   .CONST $spdif.MPEG2_AAC_FRAME_LENGTH                    1024;

   // IEC_61937 preambles
   .CONST $spdif.IEC_61937_Pz                              0x000000;
   .CONST $spdif.IEC_61937_Pa                              (0xf872<<(DAWTH-16));
   .CONST $spdif.IEC_61937_Pb                              (0x4e1f<<(DAWTH-16));

   // some empirical constants
   .CONST $spdif.MAX_COPY_PROCESS_SAMPLES                  2048; // maximum number of samples to precess by copy sub-lib each run
   .CONST $spdif.MAX_UNSYNC_SAMPLES                        10;   // maximum number of sample to stay un-synced before muting output
   .CONST $spdif.PCM_SWITCH_FADE_IN_SAMPLES                256;  // number of samples to (linearly) fade in over when starting pcm mode
   .CONST $spdif.INPUT_BUFFER_MIN_SPACE                    144;  // minimum space that shall be available in the buffer
   .CONST $spdif.MIN_PCM_ACTIVITY_SAMPLES                  2048; // minimum number of samples before unmuting pcm stream

   // define alignment constants
   // Mis-alignment can happen in KAL_ARCH3
   .CONST $spdif.STREAM_ALIGNMENT_NORMAL                   0; // left and right are aligned
   .CONST $spdif.STREAM_ALIGNMENT_SHIFTED                  1; // right is lagging by one word

   .CONST $spdif_op.VALID_FIELD                            0*ADDR_PER_WORD;
   .CONST $spdif_op.OP_SAMPLE_RATE_FIELD                   1*ADDR_PER_WORD;
   .CONST $spdif_op.INPUT_BUFFER_FIELD                     2*ADDR_PER_WORD;
   .CONST $spdif_op.INPUT_BUFFER_B_FIELD                   3*ADDR_PER_WORD;
   .CONST $spdif_op.NR_OF_OUTPUT_CHANNELS_FIELD            4*ADDR_PER_WORD;
   .CONST $spdif_op.OUTPUT_BUFFER_PCM_LEFT_FIELD           5*ADDR_PER_WORD;
   .CONST $spdif_op.OUTPUT_BUFFER_PCM_RIGHT_FIELD          6*ADDR_PER_WORD;
   .CONST $spdif_op.EXTRA_OUTPUT_BUFFERS_FIELD             7*ADDR_PER_WORD;
   .CONST $spdif_op.OUTPUT_BUFFER_CODED_FIELD              13*ADDR_PER_WORD;
   .CONST $spdif_op.SUPPORTED_DATA_TYPES_BITS1_FIELD       14*ADDR_PER_WORD; // bitmap of supported data types
   .CONST $spdif_op.SUPPORTED_DATA_TYPES_BITS2_FIELD       15*ADDR_PER_WORD; // bitmap of supported data types
   .CONST $spdif_op.USER_DATA_TYPES_PERIOD_FIELD           16*ADDR_PER_WORD; // bitmap of supported data types
   .CONST $spdif_op.RESET_NEEDED_FIELD                     17*ADDR_PER_WORD; // flag showing whether reset is needed
   .CONST $spdif_op.OUTPUT_DATA_TYPE_FIELD                 18*ADDR_PER_WORD; // data type of output stream
   .CONST $spdif_op.CHSTS_DATA_MODE_FIELD                  19*ADDR_PER_WORD; // channels status data or audio flag
   .CONST $spdif_op.EP_DATA_TYPE_FIELD                     20*ADDR_PER_WORD; // data type detected from input stream
   .CONST $spdif_op.INVALID_BURST_COUNT_FIELD              21*ADDR_PER_WORD; // counts number of invalid burst received
   .CONST $spdif_op.NULL_BURST_COUNT_FIELD                 22*ADDR_PER_WORD; // counts number of null burst received
   .CONST $spdif_op.PAUSE_BURST_COUNT_FIELD                23*ADDR_PER_WORD; // counts number of pause burst received
   .CONST $spdif_op.UNSUPPORTED_BURST_COUNT_FIELD          24*ADDR_PER_WORD; // counts number of pause burst received
   .CONST $spdif_op.READING_STATE_FIELD                    25*ADDR_PER_WORD; // buffer reading state
   .CONST $spdif_op.PCM_COUNTER_FIELD                      26*ADDR_PER_WORD; // init phase pcm sample counter
   .CONST $spdif_op.STREAM_ALIGNMENT_FIELD                 27*ADDR_PER_WORD; // whether right stream is aligned or shifted (for coded data)
   .CONST $spdif_op.PAY_LOAD_LENGTH_FIELD                  28*ADDR_PER_WORD; // payload length for the coded frame
   .CONST $spdif_op.PAY_LOAD_LEFT_FIELD                    29*ADDR_PER_WORD; // coded payload length left to load
   .CONST $spdif_op.UNSYNC_COUNTER_FIELD                   30*ADDR_PER_WORD; // counting number of consecyutive words looking for sync
   .CONST $spdif_op.STUFFING_DATA_LENGTH_FIELD             31*ADDR_PER_WORD; // length of stuffing data (for coded data only)
   .CONST $spdif_op.CODED_AUDIO_FRAME_SIZE_FIELD           32*ADDR_PER_WORD; // spdif audio frame size
   .CONST $spdif_op.PCM_FADE_IN_INDEX_FIELD                33*ADDR_PER_WORD; // fading index used when starting PCM playback
   .CONST $spdif_op.NEW_DATA_TYPE_FIELD                    34*ADDR_PER_WORD; // codec type detected from stream
   .CONST $spdif_op.NEW_CODED_AUDIO_FRAME_SIZE_FIELD       35*ADDR_PER_WORD; // audio frame size detected from stream
   .CONST $spdif_op.HIST_FIELD                             36*ADDR_PER_WORD; // last 4 words are stored
   .CONST $spdif_op.HIST_POINTER_FIELD                     40*ADDR_PER_WORD;
   .CONST $spdif_op.STRUC_SIZE                             44*ADDR_PER_WORD;

   /* Maximum process length, used for invalid/unsupported
    * data types only*/
   #define $spdif_op.MAX_IEC61937_FRAME_PROCESS_LEN       0x2000
#include "stack.h"
// *****************************************************************************
// MODULE:
//    $spdif_op.open_buffers:
//
// DESCRIPTION:
//   open input and output buffers
//
// INPUTS:
//   r8 = spdif input structure
//
// OUTPUTS:
//   I4/L4 = coded buffer or left pcm write address for pcm mode
//   I5/L5 = write pcm write address for pcm mode
//   I0/L0 = read address for spdif input buffer
//   I1/L1 = for interleaved input -> same as I0, with one word circular shift
//           for two channel input -> read address for spdif second input buffer
//   M0 = 1*ADDR_PER_WORD for 2 channel input
//        2*ADDR_PER_WORD for interleaved input
//   M1 = ADDR_PER_WORD
//   r7 = amount available to read (in words, for interleave mode will be in two-words )
//   r6 = amount can be written (in words)
// TRASHED REGISTERS:
//   assume everything
// *****************************************************************************
.MODULE $M.spdif_op.open_buffers;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;
   $spdif_op.open_buffers:

   // push rLink onto stack
   push rLink;

   // default to two-channel config
   M0 = ADDR_PER_WORD;
   M1 = M0;

   // get the input buffer info buffer
   r0 = M[r8 + $spdif_op.INPUT_BUFFER_FIELD];
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_read_address_and_size_and_start_address;
   push r2;
   pop  B0;
#else
   call $cbuffer.get_read_address_and_size;
#endif
   I0 = r0;
   L0 = r1;
   r0 = M[r8 + $spdif_op.INPUT_BUFFER_FIELD];
   call $cbuffer.calc_amount_data_in_words;
   r7 = r0;

   // see if the second input is available
   // this is the only way we test whether
   // the input is interleaved or two-channel
   r0 = M[r8 + $spdif_op.INPUT_BUFFER_B_FIELD];
   if Z jump is_interleaved;
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_read_address_and_size_and_start_address;
   push r2;
   pop  B1;
#else
   call $cbuffer.get_read_address_and_size;
#endif
   I1 = r0;
   L1 = r1;
   r0 = M[r8 + $spdif_op.INPUT_BUFFER_B_FIELD];
   call $cbuffer.calc_amount_data_in_words;
   r7 = MIN r0;
   jump input_buffer_done;

   is_interleaved:
   // input is interleaved
   r7 = r7 LSHIFT -1;
   I1 = I0;
   L1 = L0;
#ifdef BASE_REGISTER_MODE
   push B0;
   pop B1;
#endif
   M0 = M0 + M0, r0 = M[I1, M1];
   input_buffer_done:

   // see if in pcm mode
   r0 = M[r8 + $spdif_op.OUTPUT_DATA_TYPE_FIELD];
   Null = r0 - $spdif.DATA_TYPE_PCM;
   if LE jump set_pcm_buffers;

   // or if the stream isn't valid
   Null = M[r8 + $spdif_op.VALID_FIELD];
   if Z jump set_pcm_buffers;

   // in coded mode
   r0 = M[r8 + $spdif_op.OUTPUT_BUFFER_CODED_FIELD];
   call $cbuffer.calc_amount_space_in_words;
   r6 = r0 LSHIFT -1;

   // open the coded buffer
   r0 = M[r8 + $spdif_op.OUTPUT_BUFFER_CODED_FIELD];
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_write_address_and_size_and_start_address;
   push r2;
   pop  B4;
#else
   call $cbuffer.get_write_address_and_size;
#endif
   I4 = r0;
   L4 = r1;
   jump end;
set_pcm_buffers:
   // in pcm mode
   // open  left pcm buffers
   r0 = M[r8 + $spdif_op.OUTPUT_BUFFER_PCM_LEFT_FIELD];
   call $cbuffer.calc_amount_space_in_words;
   r6 = r0;
   r0 = M[r8 + $spdif_op.OUTPUT_BUFFER_PCM_LEFT_FIELD];
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_write_address_and_size_and_start_address;
   push r2;
   pop  B4;
#else
   call $cbuffer.get_write_address_and_size;
#endif
   I4 = r0;
   L4 = r1;

   // open right pcm buffers
   r0 = M[r8 + $spdif_op.OUTPUT_BUFFER_PCM_RIGHT_FIELD];
   call $cbuffer.calc_amount_space_in_words;
   r6 = MIN r0;
   r0 = M[r8 + $spdif_op.OUTPUT_BUFFER_PCM_RIGHT_FIELD];
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_write_address_and_size_and_start_address;
   push r2;
   pop  B5;
#else
   call $cbuffer.get_write_address_and_size;
#endif
   I5 = r0;
   L5 = r1;

   /* In PCM mode, extra pcm buffers should also
    * have enough space for silence insertion
    */
   I6 = r8 + $spdif_op.EXTRA_OUTPUT_BUFFERS_FIELD;
   r10 = M[r8 + $spdif_op.NR_OF_OUTPUT_CHANNELS_FIELD];
   r10 = r10 - 2;
   if NEG r10 = 0;
   do find_min_space_loop;
      r0 = M[I6, M1];
      Null = r0;
      if Z jump end;
      call $cbuffer.calc_amount_space_in_words;
      r6 = MIN r0;
   find_min_space_loop:
end:

   // convert the words to addrs
   r6 = r6 LSHIFT LOG2_ADDR_PER_WORD;
   r7 = r7 LSHIFT LOG2_ADDR_PER_WORD;

   // pop rLink from stack
   jump $pop_rLink_and_rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $spdif_op.close_buffers:
//
// DESCRIPTION:
//   closes input and output buffers, this might happen when switching buffers
//   or when decoding process ends
//
// INPUTS:
//   r8 = spdif input structure
//   r6 = amount written
//   M0 = 1*ADDR_PER_WORD for 2 channel input
//        2*ADDR_PER_WORD for interleaved input
//   M1 = ADDR_PER_WORD
//   I4/L4 = coded buffer or left pcm write address for pcm mode
//   I5/L5 = write pcm write address for pcm mode
//   I0/L0 = read address for spdif input buffer
//   I1/L1 = for interleaved input -> same as I0, with one word circular shift
//           for two channel input -> read address for spdif second input buffer
//
// OUTPUTS:
//   None
//
// TRASHED REGISTERS:
//
// *****************************************************************************
.MODULE $M.spdif_op.close_buffers;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;

   $spdif_op.close_buffers:

   // push rLink onto stack
   push rLink;

   // close input buffer
   r1 = I0;
   r0 = M[r8 + $spdif_op.INPUT_BUFFER_FIELD];
   call $cbuffer.set_read_address;
   L0 = 0;
   L1 = 0;

   r0 = M[r8 + $spdif_op.INPUT_BUFFER_B_FIELD];
   if Z jump second_input_closed;

   r1 = I1;
   call $cbuffer.set_read_address;
   second_input_closed:

   // if L5 != 0 it means pcm buffers are open
   Null = L5;
   if NZ jump close_pcm_buffers;

   // if only L4 != 0, it means coded buffer is open
   Null = L4;
   if Z jump $pop_rLink_and_rts;

   // close coded buffer
   r0 = M[r8 + $spdif_op.OUTPUT_BUFFER_CODED_FIELD];
   r1 = I4;
   call $cbuffer.set_write_address;
   L4 = 0;
   jump $pop_rLink_and_rts;

   close_pcm_buffers:

   // close left pcm buffer
   r0 = M[r8 + $spdif_op.OUTPUT_BUFFER_PCM_LEFT_FIELD];
   r1 = I4;
   call $cbuffer.set_write_address;
   L4 = 0;

   // close right pcm buffer
   r0 = M[r8 + $spdif_op.OUTPUT_BUFFER_PCM_RIGHT_FIELD];
   r1 = I5;
   call $cbuffer.set_write_address;
   L5 = 0;

   // see if any sample written
   Null = r6;
   if Z jump end;

   /* insert silence to othe pcm buffers
    */
   I6 = r8 + $spdif_op.EXTRA_OUTPUT_BUFFERS_FIELD;
   r5 = M[r8 + $spdif_op.NR_OF_OUTPUT_CHANNELS_FIELD];
   r5 = r5 - 2;
   insert_silence_loop:
      r5 = r5 - 1;
      if NEG jump end;
      r4 = M[I6, M1];
      r0 = r4;
      if Z jump end;
      #ifdef BASE_REGISTER_MODE
          call $cbuffer.get_write_address_and_size_and_start_address;
          push r2;
          pop  B5;
      #else
         call $cbuffer.get_write_address_and_size;
      #endif
      I5 = r0;
      L5 = r1;
      r10 = r6 LSHIFT -LOG2_ADDR_PER_WORD;
      r0 = 0;
      do insert_loop;
         M[I5, M1] = r0;
      insert_loop:
      r0 = r4;
      r1 = I5;
      call $cbuffer.set_write_address;
      L5 = 0;
   jump insert_silence_loop;
end:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
// *****************************************************************************
// MODULE:
//    $spdif.set_output_data_type
//
// DESCRIPTION:
//    set the codec type, if codec type changes it forces exit,
//    smooth transition when switching from one codec type to another type
//    is managed by the capability
//
// INPUTS:
//   r8 = frame copy structure
//   r3 = new codec type to set
//
// OUTPUTS:
//   -r7 might get reset to force an early exit
//
// TRASHED REGISTERS:
//    r4
// *****************************************************************************
.MODULE $M.spdif_op.set_output_data_type;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;

$spdif_op.set_output_data_type:

   // get current codec type
   r4 = M[r8 + $spdif_op.OUTPUT_DATA_TYPE_FIELD];
   M[r8 + $spdif_op.OUTPUT_DATA_TYPE_FIELD] = r3;
   Null = r4 - r3;
   if Z rts;
   Null = r4 - $spdif.DATA_TYPE_PCM;
   if GT r7 = 0;
   Null = r3 - $spdif.DATA_TYPE_PCM;
   if GT r7 = 0;
   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $spdif_op.reset
//
// DESCRIPTION:
//    Initialisations needed for processing data received from spdif ports
//
// INPUTS:
//    r8: spdif frame copy structure
//
// OUTPUTS:
//    None
//
// TRASHED REGISTERS:
//    r0
//
// *****************************************************************************
.MODULE $M.spdif_op.reset;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;

   $spdif_op.reset:

   // init reading state (looking for sync info)
   r0 = $M.spdif_frame_decode.looking_for_sync_info;
   M[r8 + $spdif_op.READING_STATE_FIELD] = r0;

   // set the counter that waits for a good number of samples
   // before unmuting pcm
   M[r8 + $spdif_op.PCM_COUNTER_FIELD] = 0;

   // reset frame unsync counter
   M[r8 + $spdif_op.UNSYNC_COUNTER_FIELD] = 0;

   // reset done, clear the flag
   M[r8 + $spdif_op.RESET_NEEDED_FIELD] = Null;

   // unknown input data type
   r0 = $spdif.DATA_TYPE_UNKNOWN;
   M[r8 + $spdif_op.EP_DATA_TYPE_FIELD] = r0;

   // output is muted
   r0 = $spdif.DATA_TYPE_MUTED;
   M[r8 + $spdif_op.OUTPUT_DATA_TYPE_FIELD] = r0;

   // clear hist buffer
   M[r8 + $spdif_op.HIST_POINTER_FIELD + (0*ADDR_PER_WORD)] = Null;
   M[r8 + $spdif_op.HIST_POINTER_FIELD + (1*ADDR_PER_WORD)] = Null;
   M[r8 + $spdif_op.HIST_POINTER_FIELD + (2*ADDR_PER_WORD)] = Null;
   M[r8 + $spdif_op.HIST_POINTER_FIELD + (3*ADDR_PER_WORD)] = Null;

   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_spdif_frame_decode
//
// DESCRIPTION:
//    processes spdif input stream and extract PCM/coded audio
//
// INPUTS:
//    - r0 = pointer to spdif structure
// OUTPUTS:
//    - r0 whether any output generated
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.spdif_frame_decode;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;
$_spdif_frame_decode:

   // Preserve rLink and the registers C doesn't view as Scratch
   push rLink;
   PUSH_ALL_C

   // save the structure in r8
   r8 = r0;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($spdif_cap.SPDIF_FRAME_DECODE_ASM.SPDIF_FRAME_DECODE.PATCH_ID_0, r2)
#endif
   
   
   // see if reset is needed
   NULL = M[r8 + $spdif_op.RESET_NEEDED_FIELD];
   if NZ call $spdif_op.reset;

   // open input and output buffers, they will remain open until
   // end of processing
   call $spdif_op.open_buffers;

   // amount space available
   push r6;

   // rMACB = jump address
   // M1 = ADDR_PER_WORD
   // M2 = -ADDR_PER_WORD
   // M0 = 1 or 2 (interleaved) * ADDR_PER_WORD
   // I0, I1 = input
   // I4, I5 = outputs
   // r7 = amount data available
   // r6 = amount space avaliable
   // I2 = PCM counter

   Null = M[r8 + $spdif_op.VALID_FIELD];
   if NZ jump process_input;
   // input is invalid, we write silence as much as we can
   r10 = r6;
   r10 = MIN r7;
   r6 = r6 - r10;
   r7 = r7 - r10;
   r10 = r10 ASHIFT -LOG2_ADDR_PER_WORD;
   r0 = 0;
   do silence_invalid_loop;
      M[I4, M1] = r0, r1 = M[I0, M0];
      M[I5, M1] = r0, r1 = M[I1, M0];
   silence_invalid_loop:
   jump end;

   process_input:
   /* get the processing function */
   r1 = looking_for_sync_info;
   rMACB = M[r8 + $spdif_op.READING_STATE_FIELD];
   if Z rMACB = r1;

   /* prevent going to PCM mode if channel status shaows
    * data, the other way isn't enforced, i.e if it shows
    * PCM we still search for coded audio
    * TODO: allow configuring this feature, so we can also
    * fully rely or fully ignore this bit.
    */
   NULL = M[r8 + $spdif_op.CHSTS_DATA_MODE_FIELD];
   if Z jump chsts_data_checked;
   r1 = handle_pcm_mute;
   Null = rMACB - handle_pcm;
   if Z rMACB = r1;
   chsts_data_checked:

   M2 = -M1;
   I3 = r8 + $spdif_op.HIST_POINTER_FIELD;
   I7 = I3 + 2*ADDR_PER_WORD;
   r0 = M[r8 + $spdif_op.PCM_COUNTER_FIELD];
   I2 = r0;
   r0 = M[r8 + $spdif_op.UNSYNC_COUNTER_FIELD];
   I6 = r0;

   procecssing_loop_start_point:

      // exit process loop if ran out of either input data or output space
      r0 = r6*r7(int)(sat);
      if Z jump end_process;
      // r0 = L[n-2], r3 = R[n-2]
      // r2 = L[n-1], r3 = R[n-1]
      // r4 = L[n], r5 = R[n]
      // rMAC = OR(r0-r5)
      // r0 = OR(r0, r1)
      r7 = r7 - M1,       r2 = M[I7, M1], r4 = M[I0, M0];
      rMAC = r2 OR r4,    r5 = M[I1, M0];
      rMAC = rMAC OR r5, r0 = M[I3, M1], r3 = M[I7, M2];
      rMAC = rMAC OR r3,  r1 = M[I3, M2], M[I7, M1] = r4;
      r0 = r0 OR r1,      M[I3, M1] = r2, M[I7, M2] = r5;
      rMAC = rMAC OR r0,  M[I3, M2] = r3;
      jump rMACB;
   end_process:
   M[r8 + $spdif_op.READING_STATE_FIELD] = rMACB;
   r0 = I2;
   M[r8 + $spdif_op.PCM_COUNTER_FIELD] = r0;
   r0 = I6;
   M[r8 + $spdif_op.UNSYNC_COUNTER_FIELD] = r0;

   end:
   // close output buffers
   pop r0;
   r6 = r0 - r6;
   call $spdif_op.close_buffers;

   r0 = r6 + r7;
   // Restore the original state
   POP_ALL_C
   pop rLink;
   rts;

// *****************************************************************************
// MODULE:
//    $spdif_op.decode.handle_pcm
//
// DESCRIPTION:
// handling pcm mode:
// pcm mute:
//     writing silence into output buffers, input buffer is continuously being checked
//     for existing of IEC_61937 preambles, if seen it might switch to codec mode. if
//     activity is seen in either ports for a minimum predefined time it will switch to
//     unmute pcm mode.
//
// pcm_unmute:
//     writing pcm data into output buffers, input buffer is continuously being checked
//     for existing of IEC_61937 preambles, if seen it might switch to codec mode. it
//     will never switch back to pcm mute mode
//
// pcm_fade_in:
//     same as pcm_unmute, lasts only for a limited number of pcm samples and during that
//     time a fading in factor is applied
//
// INPUTS:
//   r8 = spdif input structure
//
// OUTPUTS:
//   None
//
// TRASHED REGISTERS:
//    assume everything
//
// NOTE:
//    PCM states are run per sample, so code size has been compromised in
//    favour of cpu cycles
// *****************************************************************************
#define CHECK_PREAMPLE_IN_PCM_MODE \
   Null = rMAC;         \
   if Z jump reset_pcm_counter;\
   Null = r2 - $spdif.IEC_61937_Pa;\
   if Z jump IEC_61937_Pa_Px_found

   jump_to_pcm_handler:
   r0 = rMACB + handle_pcm_write;
   r0 = r0 - handle_pcm;
   jump r0;

   handle_pcm_mute:
   // read new L/R samples and also two older samples
   // r4 = L[n],   r5 = R[n]
   // r2 = L[n-1], r3 = R[n-1]
   CHECK_PREAMPLE_IN_PCM_MODE;

// ****************************************************
// handle pcm mute state
// ****************************************************
   r0 = 0;
   // if channel status shows data then ignore pcm detect,
   // the other way will not be enforced, i.e. if channel
   // status shows pcm, we still check for data
   r1 = M[r8 + $spdif_op.CHSTS_DATA_MODE_FIELD];
   if NZ jump reset_pcm_counter;

   // We need to see a minimum activity time in L/R
   // before unmuting PCM, we cannot have a look ahead
   // buffer due to memory needed, so we might loose around
   // 50ms of data at the beginning of playing
   I2 = I2 + 1;
   Null = I2 - $spdif.MIN_PCM_ACTIVITY_SAMPLES;
   if NEG jump write_pcm_silence;

      // now input type is pcm
      r3 = $spdif.DATA_TYPE_PCM;
      M[r8 + $spdif_op.EP_DATA_TYPE_FIELD] = r3;

      // start playing pcm samples,
      r3 = $spdif.DATA_TYPE_PCM;
      call $spdif_op.set_output_data_type;

      // first a fade-in period
      rMACB = handle_pcm_fade_in;
      M[r8 + $spdif_op.PCM_FADE_IN_INDEX_FIELD] = 0;
   reset_pcm_counter:
   I2 = Null;
   write_pcm_silence:
   M[I4,M1] = r0;
   r6 = r6 - M1, M[I5,M1] = r0;
   jump procecssing_loop_start_point;

// ****************************************************
// handle pcm fade in state
// ****************************************************
handle_pcm_fade_in:
   CHECK_PREAMPLE_IN_PCM_MODE;
   // apply fade in factor for $spdif_op.PCM_FADE_IN_SAMPLES sample
   r0 = M[r8 + $spdif_op.PCM_FADE_IN_INDEX_FIELD];
   r1 = r0 * (1.0/($spdif.PCM_SWITCH_FADE_IN_SAMPLES*ADDR_PER_WORD))(int)(sat);
   r2 = r2 * r1 (frac);
   r3 = r3 * r1 (frac), M[I4,M1] = r2;
   r0 = r0 + M1, M[I5,M1] = r3;
   r6 = r6 - M1;
   M[r8 + $spdif_op.PCM_FADE_IN_INDEX_FIELD] = r0;
   Null = r0 - ($spdif.PCM_SWITCH_FADE_IN_SAMPLES*ADDR_PER_WORD);
   if NEG jump procecssing_loop_start_point;
   // fade in time ended, now switch to full pcm
   rMACB = handle_pcm;
  jump procecssing_loop_start_point;

// ****************************************************
// handle pcm unmute state
// ****************************************************
handle_pcm:
   CHECK_PREAMPLE_IN_PCM_MODE;
   handle_pcm_write:
   M[I4,M1] = r2;
   r6 = r6 - M1, M[I5,M1] = r3;
   jump procecssing_loop_start_point;

// *****************************************************************************
// MODULE:
//    $spdif_op.decode.looking_for_sync_info
//
// DESCRIPTION:
//   State handler for $spdif.state.LOOKING_FOR_SYNC_INFO spdif state, decoder
//   continuously is looking for IEC_61937 preambles, once found it will further
//   process them to see whether coded data are coming from spdif input ports.
//
// INPUTS:
//   r8 = spdif input structure
//
// OUTPUTS:
//   None
//
// TRASHED REGISTERS:
//    assume everything
// *****************************************************************************
// *****************************************************************************
// MODULE:
//    $spdif_op.decode.looking_for_sync_info
//
// DESCRIPTION:
//   State handler for $spdif.READING_STATE_LOOKING_FOR_SYNC_INFO spdif state, decoder
//   continuously is looking for IEC_61937 preambles, once found it will further
//   process them to see whether coded data are coming from spdif input ports.
//
// INPUTS:
//   r8 = spdif input structure
//
// OUTPUTS:
//   None
//
// TRASHED REGISTERS:
//    assume everything
// *****************************************************************************
looking_for_sync_info:

   // read new L/R samples and also two older samples
   // r4 = L[n],   r5 = R[n]
   // r2 = L[n-1], r3 = R[n-1]
   // r0 = L[n-2]|R[n-2]
   NULL = r0;
   if NZ jump check_switch_to_pcm_time;

   // check for the Pa preamble
   Null = r2 - $spdif.IEC_61937_Pa;
   if NZ jump check_switch_to_pcm_time;

   IEC_61937_Pa_Px_found:
   IEC_61937_Pa_found:
   Null = r3 - $spdif.IEC_61937_Pb;
#ifndef SPDIF_STREAM_SHIFT_CHECK
   if Z jump IEC_61937_Pa_Pb_found;
#else
   // L = Pa, R = Pz, see if Pb is seen
   // in next R sample, if not no pre-amble check.
   Null = r5 - $spdif.IEC_61937_Pb;
   if Z jump IEC_61937_Pb_Pa_found;
#endif

// ---------------------------------------------
// new input words processed, and no valid and
// supported data type found, now continue in
// the same state
// -----------------------------------------------
looking_for_sync_info_done:
   // if in pcm mode, stay in pcm mode
   Null = rMACB - looking_for_sync_info;
   if NZ jump jump_to_pcm_handler;

   // see if time to switch to pcm(mute)
   check_switch_to_pcm_time:
   // increment unsync counter
   I6 = I6 + 1;

   // switch to PCM mode if unsynced for more than
   // certain number of samples
   Null = I6 - $spdif.MAX_UNSYNC_SAMPLES;
   if NEG jump pcm_switch_done;
         // unknown input mute output
         r3 = $spdif.DATA_TYPE_UNKNOWN;
         M[r8 + $spdif_op.EP_DATA_TYPE_FIELD] = r3;

         r3 = $spdif.DATA_TYPE_MUTED;
         call $spdif_op.set_output_data_type;

         I6 = 0;
         rMACB = handle_pcm_mute;
         I2 = 0;
   pcm_switch_done:
   jump procecssing_loop_start_point;

// --------------------------------------------
// invalid_pa_found, if no supported data type is
// seen nor is it detected as PCM input then the
// output will be muted.
// --------------------------------------------
invalid_pa_found:
   // star looking for a fresh new burst

   r3 = $spdif.DATA_TYPE_MUTED;
   call $spdif_op.set_output_data_type;

   I6 = $spdif.MAX_UNSYNC_SAMPLES;
   I2 = 0;
   rMACB = handle_pcm_mute;
   r0 = 0;
   M[I4, M1] = r0;
   r6 = r6 - M1, M[I5, M1] = r0;
   jump procecssing_loop_start_point;

   IEC_61937_Pa_Pb_found:

   //r2 = Pa, r3 = Pb, r4 = Pc, r5 = Pd
   rMAC = $spdif.STREAM_ALIGNMENT_NORMAL;

   // next state: loading coded data
   rMACB = loading_coded_data_aligned;

   check_supporting_data_type:
   // calculate burst pay load length in words
   r1 = r5 + (15<<(DAWTH-16));
   r1 = r1 LSHIFT (12 - DAWTH);
   M[r8 + $spdif_op.PAY_LOAD_LENGTH_FIELD] = r1;
   M[r8 + $spdif_op.PAY_LOAD_LEFT_FIELD] = r1;

// -----------------------------------------------------------------
// A IEC-61937 header has been found, now decide what to do
// with the frame. Coded data are extracted only for some data types,
// otherwise they will be discarded and flagged as 'unsupported'. To
// extract coded data the data type must be in supported list.
// ------------------------------------------------------------------

   // see if the burst is valid
   I2 = Null;
   r4 = r4 LSHIFT (16-DAWTH);
   r1 = r4 AND 0x80;
   if NZ jump invalid_burst_found;

   // extract the data type
   r1 = r4 AND 0x1F;
   r3 = r1 + 3;
   M[r8 + $spdif_op.EP_DATA_TYPE_FIELD] = r3;

   // see if it's in supported group1
   r0 = M[r8 + $spdif_op.SUPPORTED_DATA_TYPES_BITS1_FIELD];
   r0 = r0 OR ($spdif.CODED_DATA_TYPE_SUPPORT($spdif.CODED_DATA_TYPE_NULL) | $spdif.CODED_DATA_TYPE_SUPPORT($spdif.CODED_DATA_TYPE_PAUSE));
   r2 = r1 - 16;
   if NEG jump check_supported_groups;

   // see if it's in supported group2
   r0 = M[r8 + $spdif_op.SUPPORTED_DATA_TYPES_BITS2_FIELD];
   check_supported_groups:
   r2 = 1 LSHIFT r2;
   r2 = r2 AND r0;
   if Z jump unsupported_burst_found;

   // we use 0x1F for user-specefic data type
   Null = r1 - 0x1F;
   if Z jump user_specefic_burst_found;

check_supported_types:
   // jump to specefic handler for each data type
   Null = r1 - $spdif.CODED_DATA_TYPE_NULL;
   if Z jump null_burst_found;

   // check AC3 data type
   Null = r1 - $spdif.CODED_DATA_TYPE_AC3;
   if Z jump ac3_burst_found;

   // check MPEG1_LAYER23 data type
   Null = r1 - $spdif.CODED_DATA_TYPE_PAUSE;
   if Z jump pause_burst_found;

   // check MPEG1_LAYER23 data type
   Null = r1 - $spdif.CODED_DATA_TYPE_MPEG1_LAYER23;
   if Z jump mpeg1_layer23_burst_found;

   // check for MPEG2_AAC data type
   Null = r1 - $spdif.CODED_DATA_TYPE_MPEG2_AAC;
   if Z jump mpeg2_aac_burst_found;

   // nothing else is supported
   jump unsupported_burst_found;

user_specefic_burst_found:
    r0 = M[r8 + $spdif_op.USER_DATA_TYPES_PERIOD_FIELD];
    if LE jump unsupported_burst_found;
    r1 = $spdif.DATA_TYPE(0x1F);
    jump supported_coded_burst;

// ------------------------------
// AC-3 data type
// ------------------------------
ac3_burst_found:
    r0 = $spdif.AC3_AUDIO_FRAME_LENGTH;
    r1 = $spdif.DATA_TYPE($spdif.CODED_DATA_TYPE_AC3);
    jump supported_coded_burst;

// ------------------------------
// MPEG1 Layer 2 and 3 data type
// ------------------------------
mpeg1_layer23_burst_found:
    r0 = $spdif.MPEG1_LAYER23_AUDIO_FRAME_LENGTH;
    r1 = $spdif.DATA_TYPE($spdif.CODED_DATA_TYPE_MPEG1_LAYER23);
    jump supported_coded_burst;

// ------------------------------
// MPEG2 AAC data type
// ------------------------------
mpeg2_aac_burst_found:
    r0 = $spdif.MPEG2_AAC_FRAME_LENGTH;
    r1 = $spdif.DATA_TYPE($spdif.CODED_DATA_TYPE_MPEG2_AAC);
    jump supported_coded_burst;

// -----------------------------------------
// common point for all supported  data types
// ----------------------------------------
supported_coded_burst:

    // r0 = number of audio samples
    // r1 = codec type
   M[r8 + $spdif_op.NEW_CODED_AUDIO_FRAME_SIZE_FIELD] = r0;
   M[r8 + $spdif_op.NEW_DATA_TYPE_FIELD] = r1;

   I6 = 0;
#ifndef SPDIF_STREAM_SHIFT_CHECK
   jump switch_to_loading_coded_data;
#else
   M[r8 + $spdif_op.STREAM_ALIGNMENT_FIELD] = rMAC;
   if Z jump switch_to_loading_coded_data;
   supported_unaligned:
   rMACB = handle_IEC_61937_Pa_Pz;
   jump procecssing_loop_start_point;
#endif /* #ifndef SPDIF_STREAM_SHIFT_CHECK */

// --------------------------------------------
// invalid burst
// --------------------------------------------
invalid_burst_found:
   // invalid burst detected, increment the counter
   // this is just for user info
   r1 = M[r8 + $spdif_op.INVALID_BURST_COUNT_FIELD];
   r1 = r1 + 1;
   M[r8 + $spdif_op.INVALID_BURST_COUNT_FIELD] = r1;
   jump invalid_pa_found;

// --------------------------------------------
// Null burst
// --------------------------------------------
null_burst_found:
   // null burst detected, increment the counter
   // this is just for user info
   r1 = M[r8 + $spdif_op.NULL_BURST_COUNT_FIELD];
   r1 = r1 + 1;
   M[r8 + $spdif_op.NULL_BURST_COUNT_FIELD] = r1;
jump invalid_pa_found;

// --------------------------------------------
// Pause burst
// --------------------------------------------
pause_burst_found:
   // pause buttes detected, treat it like invalid
   // burst, the pause period will be implemented
   // using muting pcm
   r1 = M[r8 + $spdif_op.PAUSE_BURST_COUNT_FIELD];
   r1 = r1 + 1;
   M[r8 + $spdif_op.PAUSE_BURST_COUNT_FIELD] = r1;
jump invalid_pa_found;
// --------------------------------------------
// common point for all unsupported data types
// -------------------------------------------

reserved_burst_found:
unsupported_burst_found:
   // unsupported codec type detected
   r1 = M[r8 + $spdif_op.UNSUPPORTED_BURST_COUNT_FIELD];
   r1 = r1 + 1;
   M[r8 + $spdif_op.UNSUPPORTED_BURST_COUNT_FIELD] = r1;
   // star looking for a fresh new burst
   I6 = $spdif.MAX_UNSYNC_SAMPLES;
   I2 = 0;

   r3 = $spdif.DATA_TYPE_MUTED;
   call $spdif_op.set_output_data_type;

   r0 = 0;
   M[I4, M1] = r0;
   r6 = r6 - M1, M[I5, M1] = r0;

   r0 = M[r8 + $spdif_op.PAY_LOAD_LEFT_FIELD];
   r1 = $spdif_op.MAX_IEC61937_FRAME_PROCESS_LEN;
   r0 = MIN r1;
   r0 = r0 - 4;
   if NEG r0 = 0;
   rMACB = mute_unsupported_data;
   M[r8 + $spdif_op.PAY_LOAD_LEFT_FIELD] = r0;
   if NZ jump procecssing_loop_start_point;
   rMACB = looking_for_sync_info;
   jump procecssing_loop_start_point;

// -----------------------------------------
// Supported data type detected, start
// loading coded data from input stream
// -----------------------------------------
switch_to_loading_coded_data:

   // update audio frame size
   r0 = M[r8 + $spdif_op.NEW_CODED_AUDIO_FRAME_SIZE_FIELD];
   M[r8 + $spdif_op.CODED_AUDIO_FRAME_SIZE_FIELD] = r0;

   // set the codec type
   r3 = M[r8 + $spdif_op.EP_DATA_TYPE_FIELD];
   call $spdif_op.set_output_data_type;

   // compute the length of stuffing data
   r2 = M[r8 + $spdif_op.PAY_LOAD_LENGTH_FIELD];
   r3 = M[r8 + $spdif_op.CODED_AUDIO_FRAME_SIZE_FIELD];
   r2 = r2 LSHIFT -1;
   r2 = r3 - r2;
   r2 = r2 - 4;
   if NEG r2 = 0;
   M[r8 + $spdif_op.STUFFING_DATA_LENGTH_FIELD] = r2;
   jump procecssing_loop_start_point;
#ifdef SPDIF_STREAM_SHIFT_CHECK
  IEC_61937_Pb_Pa_found:
   rMACB = handle_IEC_61937_Pa_Pz;
   jump procecssing_loop_start_point;
// *****************************************************************************
// MODULE:
//    $spdif_op.decode.handle_IEC_61937_Pa_Pz
//
// DESCRIPTION:
//   short lived state handler to read coded data payload when stream is
//   no aligned
//
// INPUTS:
//   r8 = spdif input structure
//
// OUTPUTS:
//   None
//
// TRASHED REGISTERS:
//    assume everything
// *****************************************************************************
handle_IEC_61937_Pa_Pz:
   rMAC = $spdif.STREAM_ALIGNMENT_SHIFTED;
   r4 = r2;
   // next state: loading coded data
   rMACB = loading_coded_data_unaligned;
   jump check_supporting_data_type;

loading_coded_data_unaligned:
   rMAC   = read_stuffing_data;
   call   setup_reading_coded_stream;
   r0 = (16-DAWTH);
   r2 = r2 LSHIFT r0;
   r5 = r5 LSHIFT r0, M[I4, M1] = r2;
   do load_loop_unaligned;
       r4 = r4 LSHIFT r0, r5 = M[I1, M0], M[I4, M1] = r5;
       r5 = r5 LSHIFT r0, r4 = M[I0, M0], M[I4, M1] = r4;
   load_loop_unaligned:
   M[I7, 0] = r4;
   r1 = r1 AND 1;
   if NZ jump procecssing_loop_start_point;
   M[I4, M1] = r5;
   jump procecssing_loop_start_point;

#endif /* #ifdef SPDIF_STREAM_SHIFT_CHECK */
// *****************************************************************************
// MODULE:
//    $spdif_op.decode.loading_coded_data
//
// DESCRIPTION:
//   in this state decoder loads the coded data from input buffer into the codec
//   buffer.
//
//
// INPUTS:
//   r8 = spdif input structure
//
// OUTPUTS:
//   None
//
// TRASHED REGISTERS:
//    assume everything
// *****************************************************************************
setup_reading_coded_stream:
   r0 = M[r8 + $spdif_op.PAY_LOAD_LEFT_FIELD];
   r1 = r7 + ADDR_PER_WORD;
   r1 = MIN r6;
   r1 = r1 LSHIFT (1-LOG2_ADDR_PER_WORD);
   r1 = MIN r0;
   r0 = r0 - r1;
   M[r8 + $spdif_op.PAY_LOAD_LEFT_FIELD] = r0;
   if Z rMACB = rMAC;
   r10 = r1 - 1;
   r10 = r10 ASHIFT -1;
   r0 = r10 LSHIFT LOG2_ADDR_PER_WORD;
   r6 = r6 - r0;
   r7 = r7 - r0;
   r6 = r6 - ADDR_PER_WORD;
   rts;

loading_coded_data_aligned:
   rMAC = read_stuffing_data;
   call setup_reading_coded_stream;
   r0 = (16-DAWTH);
   r4 = r4 LSHIFT r0;
   do load_loop_aligned;
       r5 = r5 LSHIFT r0, r4 = M[I0, M0], M[I4, M1] = r4;
       r4 = r4 LSHIFT r0, r5 = M[I1, M0], M[I4, M1] = r5;
   load_loop_aligned:
   r5 = r5 LSHIFT r0, M[I4, M1] = r4;
   r1 = r1 AND 1;
   if NZ jump procecssing_loop_start_point;
   M[I4, M1] = r5;
   jump procecssing_loop_start_point;

// *****************************************************************************
// MODULE:
//    $spdif_op.decode.read_stuffing_data
//
// DESCRIPTION:
//   loading and discarding the stuffing data
//   buffer.
//
// INPUTS:
//   r8 = input structure
//
// OUTPUTS:
//   None
//
// TRASHED REGISTERS:
//    assume everything
// *****************************************************************************
read_stuffing_data:
   r2 = M[r8 + $spdif_op.STUFFING_DATA_LENGTH_FIELD];
   r2 = r2 - 1;
   r10 = r7 LSHIFT -LOG2_ADDR_PER_WORD;
   r10 = MIN r2;
   r2 = r2 - r10;
   M[r8 + $spdif_op.STUFFING_DATA_LENGTH_FIELD] = r2;
   if GT jump continue_skipping_stuffing_data;
      r0 = 0;
      M[I7, M1] = r0, M[I3, M1] = r0;
      M[I7, M2] = r0, M[I3, M2] = r0;
      // if no stuffing data left, look for new sync info
      rMACB = looking_for_sync_info;
   continue_skipping_stuffing_data:
   // skip stuffing data
   r10 = r10 LSHIFT LOG2_ADDR_PER_WORD;
   r7 = r7 - r10;
   M3 = r10;
   NULL = M0 - (ADDR_PER_WORD*2);
   if Z M3 = M3 + M3;
   r0 = M[I0, M3];
   r1 = M[I1, M3];
   jump procecssing_loop_start_point;

// *****************************************************************************
// MODULE:
//    $spdif_op.decode.read_stuffing_data
//
// DESCRIPTION:
//   loading and discarding the stuffing data
//   buffer.
//
// INPUTS:
//   r8 = input structure
//
// OUTPUTS:
//   None
//
// TRASHED REGISTERS:
//    assume everything
// *****************************************************************************
mute_unsupported_data:
   // calculate the amount of stuffing data will be read this
   // time and the amount will be left
   // M2 = LEFT
   // M3 = 2
   rMAC = handle_pcm_mute;
   call setup_reading_coded_stream;
   r0 = 0;
   do mute_unsupported_loop;
       r4 = M[I0, M0], M[I4, M1] = r0;
       r5 = M[I1, M0], M[I5, M1] = r0;
   mute_unsupported_loop:
   M[I4, M1] = r0;
   M[I5, M1] = r0;
   jump procecssing_loop_start_point;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_spdif_copy_channels
//
// DESCRIPTION:
//    copies a set of input channels to a set of output channels
//
// INPUTS:
//    - r0 = array of input channels
//    - r1 = array of output channels
//    - r2 = max number of channels
//    - r3 = max samples to copy
// OUTPUTS:
//    - r0: amount copied
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.spdif_copy_channels;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;
$spdif_copy_channels:
$_spdif_copy_channels:

   // Preserve rLink and the registers C doesn't view as Scratch
   push rLink;
   PUSH_ALL_C

   // r8: input channels
   // r7: output channels
   // r6: number of channels
   // r3: max number of samples to copy
   r8 = r0;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($spdif_cap.SPDIF_FRAME_DECODE_ASM.SPDIF_COPY_CHANNELS.PATCH_ID_0, r6)
#endif
   
   r7 = r1;
   r6 = r2;
   M3 = 0;
   M0 = ADDR_PER_WORD;
   // find min data
   I6 = r8;
   r10 = r6;
   do find_amount_data_loop;
       r0 = M[I6, M0];
       Null = r0;
       if Z jump find_amount_data_loop_end;
       call $cbuffer.calc_amount_data_in_words;
       r3 = MIN r0;
   find_amount_data_loop:
   find_amount_data_loop_end:

   // find min data
   I6 = r7;
   r10 = r6;
   do find_amount_space_loop;
       r0 = M[I6, M0];
       Null = r0;
       if Z jump find_amount_space_loop_end;
       call $cbuffer.calc_amount_space_in_words;
       r3 = MIN r0;
   find_amount_space_loop:
   find_amount_space_loop_end:

   M3 = r3;
   if Z jump copy_loop_end;

   // copy
   I6 = r7; // outputs
   I2 = r8; // inputs
   copy_loop:
       Null = r6;
       if Z jump copy_loop_end;
       // get next channels
       r4 = M[I2, M0], r5 = M[I6, M0];
       r1 = r5 OR r4;
       if Z jump copy_loop_end;
       // at least one of inputs or
       // outputs is valid
       r10 = r3;
       r0 = r4;
       if Z jump input_addr_read;
       // input channel is valid,
       // get the read info
       #ifdef BASE_REGISTER_MODE
          call $cbuffer.get_read_address_and_size_and_start_address;
          push r2;
          pop  B0;
      #else
         call $cbuffer.get_read_address_and_size;
      #endif
      I0 = r0;
      L0 = r1;
      input_addr_read:
      r0 = r5;
      if Z jump input_only;
      // output channel is valid,
      // get the write info
      #ifdef BASE_REGISTER_MODE
          call $cbuffer.get_write_address_and_size_and_start_address;
          push r2;
          pop  B5;
      #else
         call $cbuffer.get_write_address_and_size;
      #endif
      I5 = r0;
      L5 = r1;
      Null = r4;
      if Z jump output_only;
      copy_channels:
      // both channels are valid
      // copy input to output
      r10 = r10 - 1;
      r0 = M[I0, M0];
      do copy_ch_loop;
         r0 = M[I0, M0], M[I5,M0] = r0;
      copy_ch_loop:
      M[I5, M0] = r0;

      r0 = r4;
      r1 = I0;
      call $cbuffer.set_read_address;
      L0 = 0;

      r0 = r5;
      r1 = I5;
      call $cbuffer.set_write_address;
      L5 = 0;
      jump continue_next_channel;

      input_only:
      // only input channel is valid,
      // discard the samples
      r0 = r10 ASHIFT LOG2_ADDR_PER_WORD;
      M1 = r0;
      r0 = M[I0, M1];
      r0 = r4;
      r1 = I0;
      call $cbuffer.set_read_address;
      L0 = 0;
      jump continue_next_channel;

      output_only:
      // only output channel is valid
      // insert silence
      r0 = 0;
      do insert_loop;
         M[I5,M0] = r0;
      insert_loop:
      r0 = r5;
      r1 = I5;
      call $cbuffer.set_write_address;
      L5 = 0;

   continue_next_channel:
   r6 = r6 - 1;
   jump copy_loop;
   copy_loop_end:

   r0 = M3;
   // Restore the original state
   POP_ALL_C
   pop rLink;
   rts;
.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $spdif_frame_decode.soft_mute_output_buffers
//
// DESCRIPTION:
//    soft mutes output buffers
//
// INPUTS:
//    - r0 = array of buffers to soft mute
//    - r1 = number of buffers
//    - r2 = max samples to fade, the rest will be muted
//    - r3 = offset, fading starts from this sample
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.spdif_frame_decode_soft_mute_output_buffers;
   .CODESEGMENT PM_FLASH;
   .DATASEGMENT DM;
$_spdif_soft_mute_output_buffers:
$spdif_frame_decode.soft_mute_output_buffers:

   // Preserve rLink and the registers C doesn't view as Scratch
   push rLink;
   PUSH_ALL_C

   // Store input params
   // r8: buffers
   // r7: number of buffers
   // r6: max samples to fade
   r8 = r0;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($spdif_cap.SPDIF_FRAME_DECODE_ASM.SPDIF_FRAME_DECODE.SOFT_MUTE_OUTPUT_BUFFERS.PATCH_ID_0, r6)
#endif
  
   
   r7 = r1;
   r6 = r2;
   M0 = ADDR_PER_WORD;
   call $block_interrupts;
   // find min data to process
   I6 = r8;
   r10 = r7;
   r4 = r2 + r3;
   r5 = 1.0; // just a big number
   do find_amount_data_loop;
       r0 = M[I6, 0];
       Null = r0;
       if Z jump find_amount_data_loop_end;
       call $cbuffer.calc_amount_data_in_words;
       r4 = MIN r0;
       r0 = M[I6, M0];
       call $_cbuffer_get_size_in_words;
       r5 = MIN r0;
   find_amount_data_loop:
   find_amount_data_loop_end:

    // amount to fade
    r6 = MIN r4;
    if Z jump start_loop;//process_loop_end;

    // 1.0/amount to fade
    // this is for linear soft mute
    rMAC = 1;
    r0 = r6 ASHIFT 1;
    Div = rMAC / r0;
    r0 = DivResult;
    I3 = r0;

    start_loop:
    // amount to offset
    r4 = r4 - r6;
    I6 = r8;
    process_loop:
       // get next buffer
       r3 = M[I6, M0];
       r0 = r3;
       if Z jump process_loop_end;
       #ifdef BASE_REGISTER_MODE
          call $cbuffer.get_read_address_and_size_and_start_address;
          push r2;
          pop  B0;
      #else
         call $cbuffer.get_read_address_and_size;
      #endif
      I0 = r0;
      L0 = r1;
      I2 = r5 - 1;
      // |---Pass section -------|---- Fade out section ----|---silence section |
      //     L=r4                         Len=r6                Len=I2-(r4+r6)

      // pass section
      r0 = r4 LSHIFT LOG2_ADDR_PER_WORD;
      M1 = r0;
      r0 = M[I0, M1];

      // fade out section
      r10 = r6;
      r2 = 1.0;
      do fade_loop;
         r2 = r2 - I3, r0 = M[I0, 0];
         r0 = r0 * r2 (frac);
         M[I0, M0] = r0;
      fade_loop:

      // silence section
      r10 = I2 - r6;
      r10 = r10 - r4;
      r0 = 0;
      do silence_loop;
         M[I0, M0] = r0;
      silence_loop:
      r0 = r3;
      r1 = I0;
      call $cbuffer.set_write_address;

      L0 = 0;

    // next buffer
    r7 = r7 - 1;
    if NZ jump process_loop;
process_loop_end:

   call $unblock_interrupts;

   r0 = r6;
   // Restore the original state
   POP_ALL_C
   pop rLink;
   rts;
.ENDMODULE;

#endif //#ifndef SPDIF_FRAME_DECODE_ASM

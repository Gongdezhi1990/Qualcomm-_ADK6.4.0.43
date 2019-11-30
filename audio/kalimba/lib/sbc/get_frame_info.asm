// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************
#ifndef SBC_WBS_ONLY
#ifndef SBC_GET_FRAME_INFO_INCLUDED
#define SBC_GET_FRAME_INFO_INCLUDED

#include "codec_library.h"


// *****************************************************************************
// MODULE:
//    $sbcdec.get_frame_info
//
// DESCRIPTION:
//    Read information about an sbc frame by parsing the cbuffer
//
// INPUTS:
//    - r5 = pointer to a $codec.DECODER_STRUC structure
//
// OUTPUTS:
//    - r0 - frame size in encoded bytes (0 if not enough data)
//    - r1 - pointer to 1st byte of frame
//    - r2 - word aligned (0), not word aligned (1)
//    - r3 - frame size in audio samples
//    - r4 - sample rate in Hz
//
// TRASHED REGISTERS:
//    assume everything
//
// NOTES:
//    The cbuffer's read pointer is not moved, the cbuffer's data is only
// looked at.
//
// *****************************************************************************
.MODULE $M.sbcdec.get_frame_info;
   .CODESEGMENT SBCDEC_GET_FRAME_INFO_PM;
   .DATASEGMENT DM;

   .VAR sampling_freq_hz[4] = 16000, 32000, 44100, 48000;
   .VAR temp_samp_freq = -1;

   $sbcdec.get_frame_info:

   // push rLink onto stack
   push rLink;

.VAR $sbcdec.relay_get_bitpos = 16;


   r0 = M[r9 + $sbc.mem.GET_BITPOS_FIELD];

.VAR tmp;      // TODO ez01 remove this or implement it differently
M[tmp] = r0;

r0 = M[$sbcdec.relay_get_bitpos];

   M[r9 + $sbc.mem.GET_BITPOS_FIELD] = r0;

   // -- Setup SBC input stream buffer info --
   // set I0 to point to cbuffer for sbc input stream
// dh 9/1/07   r5 = M[$sbcdec.codec_struc];
   r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
#ifdef BASE_REGISTER_MODE
   call $cbuffer.get_read_address_and_size_and_start_address;
   push r2;
   pop B0;
#else
   call $cbuffer.get_read_address_and_size;
#endif
   I0 = r0;   L0 = r1;


   // -- Store number of bytes of data available in the SBC stream --
   r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
   call $cbuffer.calc_amount_data_in_words;
   r0 = r0 + r0;
   // adjust by the number of bits we've currently read

   r1 = M[r9 + $sbc.mem.GET_BITPOS_FIELD];

   r1 = r1 ASHIFT -3;
   r0 = r0 + r1;
   r0 = r0 - 2;
   if NEG r0 = 0;
retry:

   M[r9 + $sbc.mem.NUM_BYTES_AVAILABLE_FIELD] = r0;

   Null = r0 - $sbc.MIN_SBC_FRAME_SIZE_IN_BYTES;
   if POS jump no_buffer_underflow;
      buffer_underflow:
      L0 = 0;
      r0 = 0;
      jump exit;
   no_buffer_underflow:

   // -- Read in header --
   PROFILER_START(&$sbcdec.profile_read_frame_header)
   call $sbcdec.read_frame_header;
   PROFILER_STOP(&$sbcdec.profile_read_frame_header)

   // if the sampling frequency has changed mark this corrupt

   r0 = M[r9 + $sbc.mem.SAMPLING_FREQ_FIELD];

   r1 = M[temp_samp_freq];
   if NEG r1 = r0;
   M[temp_samp_freq] = r0;
   Null = r0 - r1;
   if NZ jump corrupt;


   // if corruption in frame then deal with it cleanly

   Null = M[r9 + $sbc.mem.FRAME_CORRUPT_FIELD];

   if Z jump dont_retry;
      corrupt:
      r0 = I0 - I2;
      if NEG r0 = r0 + L0;

      r1 = M[r9 + $sbc.mem.NUM_BYTES_AVAILABLE_FIELD];

      r0 = r1 - r0;
      if NEG r0 = 0;

      r1 = -1;
      M[temp_samp_freq] = r1;

      jump retry;

   dont_retry:

   // if buffer underflow will occur then exit here

   // TODO this does not seem to be set any where
   Null = M[r9 + $sbc.mem.FRAME_UNDERFLOW_FIELD];

   if NZ jump buffer_underflow;

   call $sbc.calc_frame_length;

//adjust I0 back to beginning of frame
I0 = I2;
M0 = ADDR_PER_WORD;
Null = r6;
if NZ M0 = 0;
r1 = M[I0,M0];
r1 = I0;



r2 = 1;
Null = r6 - 8;
if NZ r2 = 0;

L0 = 0;

   r3 = M[r9 + $sbc.mem.NROF_BLOCKS_FIELD];
   r4 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];
   r3 = r3 * r4 (int);
   r4 = M[r9 + $sbc.mem.SAMPLING_FREQ_FIELD];

   Words2Addr (r4);
   r4 = M[r4 + sampling_freq_hz];


   exit:

Null = r0 AND 1;
if Z jump not_odd;
   r6 = 8;
   r5 = M[$sbcdec.relay_get_bitpos];
   Null = r5 - 8;
   if Z r6 = 0;
   M[$sbcdec.relay_get_bitpos] = r6;
not_odd:



r5 = M[tmp];

   M[r9 + $sbc.mem.GET_BITPOS_FIELD] = r5;


   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
#endif

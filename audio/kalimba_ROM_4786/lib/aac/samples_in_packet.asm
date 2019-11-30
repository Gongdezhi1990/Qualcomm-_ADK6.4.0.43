// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "cbuffer_asm.h"
#include "stack.h"
#include "portability_macros.h"
#include "pmalloc/pl_malloc_preference.h"

#define CORE_AAC_FRAME_LENGTH           1024
#define CORE_AAC_ELD_STD_FRAME_LENGTH    512
#define CORE_AAC_ELD_SHORT_FRAME_LENGTH  480

// *****************************************************************************
// MODULE:
//    $aacdec.samples_in_packet_lc
//
// DESCRIPTION:
//    Calculate how many audio samples are in an encoded AAC packet
//
// INPUTS:
//    - r0 = pointer to a $aac.dummy_decode structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - assume everything
//
// NOTES:
//    This module is used as a utility function by other libraries/capabilities
//    to determine the number of audio samples contained in a packet.
//
//    The utility:
//    - supports only AAC-LC transported over LATM.
//    - receives a pointer to a 'main' decoder that exists in the graph. 
//    The coupling between the 'main' decoder and the utility is very loose - 
//    the utility initialises a few fields from the 'main' decoder's structure.
//    This coupling can be completely removed in the future by keeping a mini
//    AAC decoder structure permanently allocated & initialised in the caller.
//    - doesn't touch the input cbuffer passed by the caller; it operates on a 
//    clone cbuffer allocated internally.
//    -  parses the stream up to AudioMuxElement -> PayloadLengthInfo and skips
//    the rest of the payload.
//    - returns valid == FALSE in frame_dec_struc only if a frame is detected as
//    corrupted. It is recommended to the caller to DISCARD it instead of passing 
//    it on to the 'main' decoder.
//   
//
// API:
// *****************************************************************************
//    The user communicates with the utility via two structures - dummy_decoder
//    and frame_dec_struc. The user ALLOCATES both structures and populates
//    the fields:
//      - codec_struc
//      - input_buffer
//      - payload_size
//      - frame_dec_struc (RTP_FRAME_DECODE_DATA with all the fields on 0)
//      - bit_position
//
//    and will find the 'number of samples' calculated in the structure pointed
//    to by frame_dec_struc. The user will know how many octets of the AAC
//    stream have been consumed from the frame_length field of frame_dec_struc;
//    bit_position will be updated by the utility.
//
//    The user is responsible for FREEING UP the dummy_decoder and the
//    frame_dec_struc structures.
//
// *****************************************************************************
//
//    typedef struct
//    {
//        void            *codec_struc;
//        void            *input_buffer;
//        unsigned int    payload_size;
//        void            *frame_dec_struc;
//        unsigned int    bit_position;
//        void            *cbuff_clone;
//        unsigned int    payload_left;
//    } dummy_decoder;
//
// Where:
// -----------------------------------------------------------------------------
// FIELD NAME                |DIR     | Details
// -----------------------------------------------------------------------------
// codec_struc               | I      | Pointer to the main decoder's structure
//                           |        | of type DECODER structure(see codec_c.h)
// -----------------------------------------------------------------------------
// in_cbuffer                | I      | Pointer to a tCbuffer containing the
//                           |        | encoded AAC data (may be different from
//                           |        | the in_buffer contained in codec_struc)
// -----------------------------------------------------------------------------
// payload_size              | I      | Amount of encoded data in input_buffer
//                           |        | the dummy decoder will look at [octets].
// -----------------------------------------------------------------------------
// bit_position              | I      | It is used to initialise
//                           |        | $aac.mem.GET_BITPOS and should refer to
//                           |        | the stream passed in the input_buffer.
// -----------------------------------------------------------------------------
// frame_dec_struc           | O      | Pointer to structure of type
//                           |        | RTP_FRAME_DECODE_DATA (see
//                           |        | rtp_decode_struct.h)containing:
//                           |        |  - valid [bool]
//                           |        |  - frame_length [octets] (if known)
//                           |        |  - frame_samples
//                           |        |  - nr_of_frames (in the payload)
// -----------------------------------------------------------------------------
// cbuff_clone               | Status | The caller can ignore these fields.
// payload_left              |        | They are used internally by the utility
//                           |        | and have been added to this structure
//                           |        | for convenience.
// -----------------------------------------------------------------------------
//
// *****************************************************************************

.MODULE $M.aacdec.samples_in_packet_lc;
   .CODESEGMENT AACDEC_SAMPLES_IN_PACKET_LC_PM;
   .DATASEGMENT DM;

   .VAR io_struc_ptr_lc = 0;

   $_aacdec_samples_in_packet_lc:
   $aacdec.samples_in_packet_lc:

   // Save I/O structure pointer for later use and check its validity
   M[io_struc_ptr_lc] = r0;
   if Z rts;

   PUSH_ALL_C
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.SAMPLES_IN_PACKET_LC_ASM.SAMPLES_IN_PACKET.SAMPLES_IN_PACKET.PATCH_ID_0, r1)
#endif

   r6 = M[io_struc_ptr_lc];
   r0 = M[r6 + $aac.dummy_decode.IN_CBUFFER];
#ifdef KYMERA
   call $cbuffer.calc_amount_data_in_words;
#else
   call $cbuffer.calc_amount_data;
#endif
   r0 = r0 + r0;
   r1 = M[r6 + $aac.dummy_decode.PAYLOAD_SIZE];
   // If payload size is zero there is nothing to decode -> exit
   if Z jump exit4;
   Null = r0 - r1;
   // If the input buffer doesn't hold at least PAYLOAD_SIZE octets -> exit
   if NEG jump exit4;

   // Allocate memory for cloning input buffer -> early exit if unsuccessful
   r0 = $cbuffer.STRUC_SIZE * ADDR_PER_WORD;
   r1 = MALLOC_PREFERENCE_NONE;
   call $_xzppmalloc;
   M[r6 + $aac.dummy_decode.CBUFF_CLONE] = r0;
   if Z jump exit4;

   // Allocate memory for the decoder's structure -> early exit if unsuccessful
   r0 = $aac.mem.STRUC_SIZE * ADDR_PER_WORD;
   r1 = MALLOC_PREFERENCE_NONE;
   call $_xzppmalloc;
   r9 = r0;
   if Z jump exit3;

   // Clone input buffer
   r10 = $cbuffer.STRUC_SIZE;
   r0 = M[r6 + $aac.dummy_decode.IN_CBUFFER];
   r1 = M[r6 + $aac.dummy_decode.CBUFF_CLONE];
   I0 = r0;
   I4 = r1;
   do clone_in_buff;
      r4 = M[I0, MK1];
      M[I4, MK1] = r4;
   clone_in_buff:

   // Initialise a few fields: $aac.mem.BITMASK_LOOKUP_FIELD,
   // $aac.mem.SF_INDEX_FIELD and $aac.mem.SAMPLE_RATE_TAGS_FIELD
   // needed to start parsing audio_mux_element
   r5 = M[r6 + $aac.dummy_decode.CODEC_STRUC];
   r4 = M[r5 + $codec.DECODER_DATA_OBJECT_FIELD];
   r0 = M[r4 + $aac.mem.BITMASK_LOOKUP_FIELD];
   r1 = M[r4 + $aac.mem.SAMPLE_RATE_TAGS_FIELD];
   M[r9 + $aac.mem.BITMASK_LOOKUP_FIELD] = r0;
   M[r9 + $aac.mem.SAMPLE_RATE_TAGS_FIELD] = r1;
   // Make sure TMP_MEM_POOL points to a safe place 
   // (in an unused area of the decoder's structure).
   r0 = r9 + $aac.mem.TMP;
   M[r9 + $aac.mem.TMP_MEM_POOL_END_PTR] = r0;
   M[r9 + $aac.mem.TMP_MEM_POOL_PTR] = r0;

   // A few more initialisations before the frame loop
   r0 = M[r6 + $aac.dummy_decode.GET_BITPOS];
   r1 = M[r6 + $aac.dummy_decode.PAYLOAD_SIZE];
   M[r9 + $aac.mem.GET_BITPOS] = r0;
   M[r6 + $aac.dummy_decode.PAYLOAD_LEFT] = r1;

   // Set validation to TRUE and frame_length to 0
   r1 = 1;
   r5 = M[r6 + $aac.dummy_decode.FRAME_DEC_STRUC];
   M[r5 + $aac.frame_decode_data.valid] = r1;
   M[r5 + $aac.frame_decode_data.frame_length] = 0;

   reattempt_decode:
      // Setup aac input stream buffer info
      // Make I0 to point to the cloned input cbuffer
      r0 = M[r6 + $aac.dummy_decode.CBUFF_CLONE];
      #ifdef BASE_REGISTER_MODE
         call $cbuffer.get_read_address_and_size_and_start_address;
         push r2;
         pop B0;
      #else
         call $cbuffer.get_read_address_and_size;
      #endif
      I0 = r0;
      L0 = r1;

      // Determine how much we've got left in the payload in bits/bytes
      // and reset the bit counter for the current raw data block;
      r0 = M[r6 + $aac.dummy_decode.PAYLOAD_LEFT];
      M[r9 + $aac.mem.READ_BIT_COUNT] = Null;
      r1 = r0 ASHIFT 3;
      M[r9 + $aac.mem.FRAME_NUM_BITS_AVAIL] = r1;
      M[r9 + $aac.mem.NUM_BYTES_AVAILABLE] = r0;

      // Do we have enough data to start decoding a frame?
      r4 = $aacdec.MIN_LATM_FRAME_SIZE_IN_BYTES;
      Null = r0 - r4;
      if NEG jump exit2;

      // Parse latm frame up to payload_length_info.
      // This is the top of audio_mux_element() with muxConfigPresent = 1
      call $aacdec.get1bit;
      if Z call $aacdec.stream_mux_config;
      // Corruption check 1: POSSIBLE_FRAME_CORRUPTION raised by $aacdec.stream_mux_config
      Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
      if NZ jump frame_corrupted;

      call $aacdec.payload_length_info;
      M[r9 + $aac.mem.latm.MUX_SLOT_LENGTH_BYTES] = r4;

      // Corruption check 2: MUX_SLOT_LENGTH_BYTES too large
      Null = r4 - ($aacdec.MAX_AAC_FRAME_SIZE_IN_BYTES*2);
      if POS jump frame_corrupted;

      // Corruption check 3: if amount read too large fall through to corrupt frame
      r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
      Null = r0 -  ($aacdec.MAX_AAC_FRAME_SIZE_IN_BYTES*8*2);
      if NEG jump skip_frame;

      // The decoding of current frame failed
   frame_corrupted:
      // Set validity to FALSE and exit
      r2 = M[io_struc_ptr_lc];
      r4 = M[r2 + $aac.dummy_decode.FRAME_DEC_STRUC];
      M[r4 + $aac.frame_decode_data.valid] = 0;
      jump exit2;

   skip_frame:
      // We are not going to parse the whole raw data block - we try to 
      // skip the frame; first check if we have enough data available.
      r5 = M[r9 + $aac.mem.latm.MUX_SLOT_LENGTH_BYTES];
      r4 = r5 LSHIFT 3;
      r1 = M[r9 + $aac.mem.READ_BIT_COUNT];
      r1 = r1 + r4;
      r0 = M[r9 + $aac.mem.FRAME_NUM_BITS_AVAIL];
      Null = r0 - r1;
      // If not enough data exit immediately.
      if NEG jump exit2;

      // byte_alignment - should be byte aligned anyway
      call $aacdec.byte_align;

      // Skip the rest of the frame by a combination of:
      // * moving the encoded stream read pointer ahead by full words & manually update READ_BIT_COUNT,
      // * use $aacdec.get1byte if MUX_SLOT_LENGTH_BYTES is odd ($aacdec.get1byte takes care of READ_BIT_COUNT).
      // We update READ_BIT_COUNT to simulate a full parsing of the encoded stream.
      // Convert octets to words (2 octets per word)
      r1 = r5 LSHIFT -1;
      // Calculate number of bits per word
      r2 = r1 LSHIFT 4;
      Words2Addr(r1);
      M0 = r1;
      r1 = M[I0, M0];
      r1 = M[r9 + $aac.mem.READ_BIT_COUNT];
      r1 = r1 + r2;
      M[r9 + $aac.mem.READ_BIT_COUNT] = r1;
      Null = r5 AND 1;
      if NZ call $aacdec.get1byte;

   successful_decode:
      // Increment frame counter and set decoded frame size
      r6 = M[io_struc_ptr_lc];
      r4 = CORE_AAC_FRAME_LENGTH;
      r3 = M[r6 + $aac.dummy_decode.FRAME_DEC_STRUC];
      r0 = M[r3 + $aac.frame_decode_data.nr_of_frames];
      r0 = r0 + 1;
      M[r3 + $aac.frame_decode_data.nr_of_frames] = r0;
      M[r3 + $aac.frame_decode_data.frame_samples] = r4;

   update_payload_left:
      // Update payload left and exit.
      // We don't attempt any reruns as we want the caller to timestamp each
      // frame (see B-236217 - dg 10705717 and B-243436 for more details).
      
      r1 = M[r9 + $aac.mem.READ_BIT_COUNT];
      r2 = M[r6 + $aac.dummy_decode.PAYLOAD_LEFT];
      r0 = r1 ASHIFT -3;
      // Update the frame_length == the amount of data consumed from
      // the input stream (this may cover more than one frame!)
      r4 = M[r3 + $aac.frame_decode_data.frame_length];
      r4 = r4 + r0;
      M[r3 + $aac.frame_decode_data.frame_length] = r4;

      r2 = r2 - r0;
      M[r6 + $aac.dummy_decode.PAYLOAD_LEFT] = r2;

   exit2:
      // Free cloned decoder structure
      r0 = r9;
      call $_pfree;
   exit3:
      // Free cloned input cbuffer structure
      r6 = M[io_struc_ptr_lc];
      r0 = M[r6 + $aac.dummy_decode.CBUFF_CLONE];
      call $_pfree;
   exit4:

   POP_ALL_C
   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $aacdec.samples_in_packet
//
// DESCRIPTION:
//    Calculate how many audio samples are in an encoded payload
//
// INPUTS:
//    - r0 = pointer to a $aac.dummy_decode structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - assume everything including $aacdec.tmp
//
// NOTES:
//    This module is used as a utility function by other libraries/capabilities
//    to determine the number of audio samples contained in a payload.
//
//    The utility piggybacks the decoding tasks on a decoder already
//    configured, the 'main' decoder. In order to not alter the behaviour
//    of the main decoder the utility clones two items:
//      - the main decoder's structure
//      - the input buffer passed in the $aac.dummy_decode structure
//    The utility doesn't produce any audio output; it just consumes the input
//    stream counting how many raw_data_blocks have been decoded and infers
//    the audio frame length from information obtained from the main decoder
//    (SBR on, ELD on, etc.).
//
// API:
// *****************************************************************************
//    The user communicates with the utility via two structures - dummy_decoder
//    and frame_dec_struc. The user ALLOCATES both structures and populates
//    the fields:
//      - codec_struc
//      - input_buffer
//      - payload_size
//      - frame_dec_struc (RTP_FRAME_DECODE_DATA with all the fields on 0)
//      - bit_position
//
//    and will find the 'number of samples' calculated in the structure pointed
//    to by frame_dec_struc. The user will know how many octets of the AAC
//    stream have been consumed from the frame_length field of frame_dec_struc;
//    bit_position will be updated by the utility.
//
//    The user is responsible for FREEING UP the dummy_decoder and the
//    frame_dec_struc structures.
//
// *****************************************************************************
//
//    typedef struct
//    {
//        void            *codec_struc;
//        void            *input_buffer;
//        unsigned int    payload_size;
//        void            *frame_dec_struc;
//        unsigned int    bit_position;
//        void            *cbuff_clone;
//        unsigned int    payload_left;
//    } dummy_decoder;
//
// Where:
// -----------------------------------------------------------------------------
// FIELD NAME                |DIR     | Details
// -----------------------------------------------------------------------------
// codec_struc               | I      | Pointer to the main decoder's structure
//                           |        | of type DECODER structure(see codec_c.h)
// -----------------------------------------------------------------------------
// in_cbuffer                | I      | Pointer to a tCbuffer containing the
//                           |        | encoded AAC data (may be different from
//                           |        | the in_buffer contained in codec_struc)
// -----------------------------------------------------------------------------
// payload_size              | I      | Amount of encoded data in input_buffer
//                           |        | the dummy decoder will look at [octets].
// -----------------------------------------------------------------------------
// frame_dec_struc           | O      | Pointer to structure of type
//                           |        | RTP_FRAME_DECODE_DATA (see
//                           |        | rtp_decode_struct.h)containing:
//                           |        |  - valid [bool]
//                           |        |  - frame_length [octets] (if known)
//                           |        |  - frame_samples
//                           |        |  - nr_of_frames (in the payload)
// -----------------------------------------------------------------------------
// bit_position              | I/O    | As input, it will be used to initialise
//                           |        | $aac.mem.GET_BITPOS and should refer to
//                           |        | the stream passed in the input_buffer.
//                           |        | As output, it will take the updated
//                           |        | value of $aac.mem.GET_BITPOS at the end
//                           |        | of dummy decode.
// -----------------------------------------------------------------------------
// cbuff_clone               | Status | The caller can ignore these fields.
// payload_left              |        | They are used internally by the utility
//                           |        | and have been added to this structure
//                           |        | for convenience.
// -----------------------------------------------------------------------------
//
// *****************************************************************************

.MODULE $M.aacdec.samples_in_packet;
   .CODESEGMENT AACDEC_SAMPLES_IN_PACKET_PM;
   .DATASEGMENT DM;

   .VAR io_struc_ptr = 0;

   $_aacdec_samples_in_packet:
   $aacdec.samples_in_packet:

   // Save I/O structure pointer for later use and check its validity
   M[io_struc_ptr] = r0;
   if Z rts;

   PUSH_ALL_C
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.SAMPLES_IN_PACKET_ASM.SAMPLES_IN_PACKET.SAMPLES_IN_PACKET.PATCH_ID_0, r1)
#endif

   r6 = M[io_struc_ptr];
   r0 = M[r6 + $aac.dummy_decode.IN_CBUFFER];
#ifdef KYMERA
   call $cbuffer.calc_amount_data_in_words;
#else
   call $cbuffer.calc_amount_data;
#endif
   r0 = r0 + r0;
   r1 = M[r6 + $aac.dummy_decode.PAYLOAD_SIZE];
   // If payload size is zero there is nothing to decode -> exit
   if Z jump exit4;
   Null = r0 - r1;
   // If the input buffer doesn't hold at least PAYLOAD_SIZE octets -> exit
   if NEG jump exit4;

   // Allocate memory for cloning input buffer -> early exit if unsuccessful
   r0 = $cbuffer.STRUC_SIZE * ADDR_PER_WORD;
   r1 = MALLOC_PREFERENCE_NONE;
   call $_xzppmalloc;
   M[r6 + $aac.dummy_decode.CBUFF_CLONE] = r0;
   if Z jump exit4;

   // Allocate memory for cloning the decoder's structure -> early exit
   // if unsuccessful
   r0 = $aac.mem.STRUC_SIZE * ADDR_PER_WORD;
   r1 = MALLOC_PREFERENCE_NONE;
   call $_xppmalloc;
   r9 = r0;
   if Z jump exit3;

   // Clone input buffer
   r10 = $cbuffer.STRUC_SIZE;
   r0 = M[r6 + $aac.dummy_decode.IN_CBUFFER];
   r1 = M[r6 + $aac.dummy_decode.CBUFF_CLONE];
   I0 = r0;
   I4 = r1;
   do clone_in_buff;
      r4 = M[I0, MK1];
      M[I4, MK1] = r4;
   clone_in_buff:

   // Clone the decoder's structure
   r5 = M[r6 + $aac.dummy_decode.CODEC_STRUC];
   r10 = $aac.mem.STRUC_SIZE;
   r4 = M[r5 + $codec.DECODER_DATA_OBJECT_FIELD];
   I0 = r4;
   I4 = r9;
   do clone_structure;
      r4 = M[I0, MK1];
      M[I4, MK1] = r4;
   clone_structure:

   // Save $codec.DECODER_STRUC pointer
   M[r9 + $aac.mem.codec_struc] = r5;

   // Allocate private mem_pools
   r0 = $aacdec.TMP_MEM_POOL_LENGTH * ADDR_PER_WORD;
   r1 = MALLOC_PREFERENCE_DM1;
   call $_xppmalloc;
   Null = r0;
   if Z jump exit31;
   M[r9 + $aac.mem.TMP_MEM_POOL_END_PTR] = r0;
   M[r9 + $aac.mem.TMP_MEM_POOL_PTR] = r0;

   r0 = $aacdec.FRAME_MEM_POOL_LENGTH * ADDR_PER_WORD;
   r1 = MALLOC_PREFERENCE_DM2;
   call $_xppmalloc;
   Null = r0;
   if Z jump exit32;
   M[r9 + $aac.mem.FRAME_MEM_POOL_END_PTR] = r0;
   M[r9 + $aac.mem.FRAME_MEM_POOL_PTR] = r0;

   // A few initialisations before the frame loop
   r0 = M[r6 + $aac.dummy_decode.GET_BITPOS];
   r1 = M[r6 + $aac.dummy_decode.PAYLOAD_SIZE];
   M[r9 + $aac.mem.GET_BITPOS] = r0;
   M[r6 + $aac.dummy_decode.PAYLOAD_LEFT] = r1;

   // Set validation to TRUE and frame_length to 0
   r1 = 1;
   r5 = M[r6 + $aac.dummy_decode.FRAME_DEC_STRUC];
   M[r5 + $aac.frame_decode_data.valid] = r1;
   M[r5 + $aac.frame_decode_data.frame_length] = 0;


   reattempt_decode:

      // Clear the flag indicating if $aacdec.ics_info completed successfully
      M[r9 + $aac.mem.ICS_INFO_DONE_FIELD] = 0;

      // Setup aac input stream buffer info
      // Make I0 to point to the cloned input cbuffer
      r0 = M[r6 + $aac.dummy_decode.CBUFF_CLONE];
      #ifdef BASE_REGISTER_MODE
         call $cbuffer.get_read_address_and_size_and_start_address;
         push r2;
         pop B0;
      #else
         call $cbuffer.get_read_address_and_size;
      #endif
      I0 = r0;
      L0 = r1;

      // Determine how much we've got left in the payload in bits and bytes
      // and reset the bit counter for the current raw data block;
      // r6 is already loaded with M[io_struc_ptr]
      r0 = M[r6 + $aac.dummy_decode.PAYLOAD_LEFT];

      // Adjust by the number of bits we've currently read
      r1 = M[r9 + $aac.mem.GET_BITPOS];
      r1 = r1 ASHIFT -3;
      r0 = r0 + r1;
      r0 = r0 - 2;
      r1 = M[r9 + $aac.mem.WRITE_BYTEPOS];
      r0 = r0 - r1;
      if NEG r0 = 0;
      M[r9 + $aac.mem.READ_BIT_COUNT] = Null;
      r1 = r0 ASHIFT 3;
      M[r9 + $aac.mem.FRAME_NUM_BITS_AVAIL] = r1;
      M[r9 + $aac.mem.NUM_BYTES_AVAILABLE] = r0;

      r4 = $aacdec.MIN_AAC_FRAME_SIZE_IN_BYTES;
      r3 = M[r9 + $aac.mem.READ_FRAME_FUNCTION];

      // Check minimum size for MP4 bitstreams
      #ifdef AACDEC_MP4_FILE_TYPE_SUPPORTED
         r2 = $aacdec.MIN_MP4_HEADER_SIZE_IN_BYTES;
         r1 = $aacdec.MIN_MP4_FRAME_SIZE_IN_BYTES;
         r5 = M[r9 + $aac.mem.MP4_HEADER_PARSED];
         Null = r5 AND 0x1;
         if NZ r2 = r1;
         r6 = M[r9 + $aac.mem.READ_FRAME_FUNC_TABLE + (0*ADDR_PER_WORD)];
         Null = r3 - r6;//M[$aacdec.read_frame_func_table + 0];
         if Z r4 = r2;
      #endif

      // Check minimum size for ADTS bitstreams
      #ifdef AACDEC_ADTS_FILE_TYPE_SUPPORTED
         r2 = $aacdec.MIN_ADTS_FRAME_SIZE_IN_BYTES;
         r6 = M[r9 + $aac.mem.READ_FRAME_FUNC_TABLE + (1*ADDR_PER_WORD)];
         Null = r3 - r6;
         if Z r4 = r2;
      #endif

      // Check minimum size for LATM bitstream
      #ifdef AACDEC_LATM_FILE_TYPE_SUPPORTED
         r2 = $aacdec.MIN_LATM_FRAME_SIZE_IN_BYTES;
         r6 = M[r9 + $aac.mem.READ_FRAME_FUNC_TABLE + (2*ADDR_PER_WORD)];
         Null = r3 - r6;
         if Z r4 = r2;
      #endif

      // Check NUM_BYTES_AVAILABLE against minimum size for current bitstream
      Null = r0 - r4;
      if POS jump no_buffer_underflow;
         buffer_underflow:
         // Free all of the mem pools
         call $aacdec.tmp_mem_pool_free_all;
         call $aacdec.frame_mem_pool_free_all;
         // Store updated read pointer for aac input stream
         r6 = M[io_struc_ptr];
         r0 = M[r6 + $aac.dummy_decode.CBUFF_CLONE];
         r1 = I0;
         call $cbuffer.set_read_address;
         L0 = 0;
         jump exit1;
      no_buffer_underflow:

      // Read adts, mp4, or latm frame
      r0 = M[r9 + $aac.mem.READ_FRAME_FUNCTION];
      call r0;

      // If corruption in file then deal with it cleanly
      Null = M[r9 + $aac.mem.FRAME_CORRUPT];
      if NZ jump frame_corrupted;

      // If buffer underflow will occur then exit here
      Null = M[r9 + $aac.mem.FRAME_UNDERFLOW];
      if NZ jump buffer_underflow;

      // Check that ics_info has been called successfully
      // (otherwise force a frame corrupt)
      Null = M[r9 + $aac.mem.ICS_INFO_DONE_FIELD];
      if NZ jump decode_successful;

      // The decoding of current frame failed
      frame_corrupted:

         // Free all of the mem pools
         call $aacdec.tmp_mem_pool_free_all;
         call $aacdec.frame_mem_pool_free_all;

         // get byte aligned for the next decode
         r0 = M[r9 + $aac.mem.GET_BITPOS];
         r0 = r0 AND 7;
         call $aacdec.getbits;

         // Set validity to FALSE
         r2 = M[io_struc_ptr];
         r4 = M[r2 + $aac.dummy_decode.FRAME_DEC_STRUC];
         M[r4 + $aac.frame_decode_data.valid] = 0;
         // Store updated read pointer for aac input stream
         r6 = M[io_struc_ptr];
         r5 = M[r9 + $aac.mem.codec_struc];
         r1 = I0;
         r0 = M[r6 + $aac.dummy_decode.CBUFF_CLONE];
         call $cbuffer.set_read_address;
         L0 = 0;
         r3 = M[r6 + $aac.dummy_decode.FRAME_DEC_STRUC];
         jump update_payload_left;
      decode_successful:

      // Store updated read pointer for aac input stream
      r6 = M[io_struc_ptr];
      r1 = I0;
      r0 = M[r6 + $aac.dummy_decode.CBUFF_CLONE];
      call $cbuffer.set_read_address;
      L0 = 0;

      // Free all frame memory
      call $aacdec.frame_mem_pool_free_all;
      // Free all tmp memory again
      call $aacdec.tmp_mem_pool_free_all;

      // Select the correct frame length
      r4 = CORE_AAC_FRAME_LENGTH;

      #ifdef AACDEC_ELD_ADDITIONS
         r1 = CORE_AAC_ELD_STD_FRAME_LENGTH;
         r2 = CORE_AAC_ELD_SHORT_FRAME_LENGTH;
         r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
         Null = r0 - $aacdec.ER_AAC_ELD;
         if EQ r4 = r1;
         Null = M[r9 + $aac.mem.ELD_frame_length_flag];
         if NZ r4 = r2;
      #endif

      // Increment frame counter
      r6 = M[io_struc_ptr];
      r3 = M[r6 + $aac.dummy_decode.FRAME_DEC_STRUC];
      r0 = M[r3 + $aac.frame_decode_data.nr_of_frames];
      r0 = r0 + 1;
      M[r3 + $aac.frame_decode_data.nr_of_frames] = r0;
      M[r3 + $aac.frame_decode_data.frame_samples] = r4;

      update_payload_left:
      // Update payload left and re-attempt a decode
      // (the most reliable counter for the amount of data consumed
      // while reading a raw data block is the READ_BIT_COUNT)
      r1 = M[r9 + $aac.mem.READ_BIT_COUNT];
      r2 = M[r6 + $aac.dummy_decode.PAYLOAD_LEFT];
      r0 = r1 ASHIFT -3;
      // Update the frame_length == the amount of data consumed from
      // the input stream (this may cover more than one frame!)
      r4 = M[r3 + $aac.frame_decode_data.frame_length];
      r4 = r4 + r0;
      M[r3 + $aac.frame_decode_data.frame_length] = r4;

      r2 = r2 - r0;
      M[r6 + $aac.dummy_decode.PAYLOAD_LEFT] = r2;
      if GT jump reattempt_decode;

   exit1:
      r6 = M[io_struc_ptr];
      r1 = M[r9 + $aac.mem.GET_BITPOS];
      M[r6 + $aac.dummy_decode.GET_BITPOS] = r1;
      r3 = M[r6 + $aac.dummy_decode.FRAME_DEC_STRUC];
      r4 = M[r3 + $aac.frame_decode_data.frame_samples];
      // Update $codec.DECODER_STRUC
      #ifdef AACDEC_SBR_ADDITIONS
         #ifdef AACDEC_SBR_HALF_SYNTHESIS
            // TODO for ELD support -> make sure this field is set
            Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
            if Z jump sbr_not_present;
               r4 = r4 * 2 (int);
            sbr_not_present:
         #endif
      #endif
      M[r3 + $aac.frame_decode_data.frame_samples] = r4;


    // Free private mem_pools
    r0 = M[r9 + $aac.mem.FRAME_MEM_POOL_PTR];
      call $_pfree;

   exit32:
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      call $_pfree;
   exit31:
      // Free cloned decoder structure
      r0 = r9;
      call $_pfree;
   exit3:
      // Free cloned input cbuffer structure
      r0 = M[r6 + $aac.dummy_decode.CBUFF_CLONE];
      call $_pfree;
   exit4:

   POP_ALL_C
   rts;

.ENDMODULE;

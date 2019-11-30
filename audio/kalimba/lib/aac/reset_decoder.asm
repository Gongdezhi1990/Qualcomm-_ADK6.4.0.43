// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.reset_decoder
//
// DESCRIPTION:
//    This library contains functions to decode AAC and AAC+SBR. This function
//    resets the decoder.
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0, r1, r10, I0, I4
//
// *****************************************************************************
.MODULE $M.aacdec.reset_decoder;
   .CODESEGMENT AACDEC_RESET_DECODER_PM;
   .DATASEGMENT DM;

   $aacdec.reset_decoder:
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aac.RESET_DECODER_ASM.AACDEC.RESET_DECODER.PATCH_ID_0, r10)
#endif
   
   
   // reset tmp_mem_pool_end
   r0 =  M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r9 + $aac.mem.TMP_MEM_POOL_END_PTR] = r0;

   // clear the overlap-add buffers
   r0 = M[r9 +  $aac.mem.OVERLAP_ADD_LEFT_PTR];
   I0 = r0;
   r0 = M[r9 +  $aac.mem.OVERLAP_ADD_RIGHT_PTR];
   I4 = r0;
   r10 = $aacdec.OVERLAP_ADD_LENGTH;
   r0 = 0;
   do zero_overlap_add;
      M[I0, MK1] = r0,
       M[I4, MK1] = r0;
   zero_overlap_add:

   //clear the ics structures
   I0 = r9 + $aac.mem.ICS_LEFT;
   I4 = r9 + $aac.mem.ICS_RIGHT;
   r10 = $aacdec.ics.STRUC_SIZE;
   do zero_ics_struc;
      M[I0, MK1] = r0,
       M[I4, MK1] = r0;
   zero_ics_struc:

   // clear internal buffers
   r10 = $aacdec.BUF_PTR_LENGTH;
   r1 = M[r9 + $aac.mem.BUF_LEFT_PTR];
   I0 = r1;
   r1 = M[r9 + $aac.mem.BUF_RIGHT_PTR];
   I4 = r1;
   do zero_buffer;
      M[I0, MK1] = r0,
       M[I4, MK1] = r0;
   zero_buffer:


   // clear previous window shape
   r0 = r9 + $aac.mem.PREVIOUS_WINDOW_SHAPE;
   M[r0] = Null;
   M[r0 + ADDR_PER_WORD] = Null;


   // set sampling rate index to -1 to imply start of new file
   r0 = -1;
   M[r9 + $aac.mem.SF_INDEX_FIELD]  = r0;


   // initial bit reading variables for the start of new file
   M[r9 + $aac.mem.READ_BIT_COUNT] = Null;
   r0 = 16;
   M[r9 + $aac.mem.GET_BITPOS] = r0;

   // reset mp4 variables
   M[r9 + $aac.mem.MP4_DECODING_STARTED] = Null;
   M[r9 + $aac.mem.MP4_HEADER_PARSED] = Null;
   M[r9 + $aac.mem.MP4_SEQUENCE_FLAGS_INITIALISED] = Null;
   M[r9 + $aac.mem.MP4_MOOV_ATOM_SIZE_MS] = Null;
   M[r9 + $aac.mem.MP4_MOOV_ATOM_SIZE_LS] = Null;
   M[r9 + $aac.mem.MP4_DISCARD_AMOUNT_MS] = Null;
   M[r9 + $aac.mem.MP4_DISCARD_AMOUNT_LS] = Null;
   M[r9 + $aac.mem.MP4_IN_MOOV] = Null;
   M[r9 + $aac.mem.MP4_IN_DISCARD_ATOM_DATA] = Null;
   r3 = r9 + $aac.mem.MDAT_SIZE;
   M[r3 + (2*ADDR_PER_WORD)] = Null;
   M[r3 + ADDR_PER_WORD] = Null;
   M[r3] = Null;
   r3 = r9 + $aac.mem.SAMPLE_COUNT;
   M[r3] = Null;
   M[r3 + ADDR_PER_WORD] = Null;
   M[r9 + $aac.mem.MDAT_PROCESSED] = Null;
   r3 = r9 + $aac.mem.MDAT_OFFSET;
   M[r3 + ADDR_PER_WORD] = Null;
   M[r3] = Null;
   r3 = r9 + $aac.mem.MP4_FILE_OFFSET;
   M[r3 + ADDR_PER_WORD] = Null;
   M[r3] = Null;
   r3 = r9 + $aac.mem.STSZ_OFFSET;
   M[r3 + ADDR_PER_WORD] = Null;
   M[r3] = Null;
   r3 = r9 + $aac.mem.STSS_OFFSET;
   M[r3 + ADDR_PER_WORD] = Null;
   M[r3] = Null;

   M[r9 + $aac.mem.MP4_FRAME_COUNT] = Null;

   M[r9 + $aac.mem.MP4_FF_REW_STATUS] = Null;
   r3 = r9 + $aac.mem.FF_REW_SKIP_AMOUNT;
   M[r3] = Null;
   M[r3 + ADDR_PER_WORD] = Null;

   // clear spec_blksigndet
   M[r9 + $aac.mem.LEFT_SPEC_BLKSIGNDET] = Null;
   M[r9 + $aac.mem.LEFT_SPEC_BLKSIGNDET + ADDR_PER_WORD] = Null;
   M[r9 + $aac.mem.RIGHT_SPEC_BLKSIGNDET ] = Null;
   M[r9 + $aac.mem.RIGHT_SPEC_BLKSIGNDET + ADDR_PER_WORD] = Null;


#ifdef AACDEC_SBR_ADDITIONS
   // set in_synth to zero to indicate not half way through synthesis filterbank
   M[r9 + $aac.mem.SBR_in_synth] = Null;
   r0 = 32;
   M[r9 + $aac.mem.SBR_in_synth_loops] = r0;

   r0 = $aacdec.SBR_numTimeSlotsRate;
   M[r9 + $aac.mem.SBR_numTimeSlotsRate] = r0;
   r0 = $aacdec.SBR_numTimeSlotsRate + 6;
   M[r9 + $aac.mem.SBR_numTimeSlotsRate_adjusted] = r0;

   r0 = r9 + $aac.mem.SBR_v_left_cbuffer_struc;
   M[r9 + $aac.mem.SBR_v_cbuffer_struc_address + (0*ADDR_PER_WORD)] = r0;
   r0 = r9 + $aac.mem.SBR_v_right_cbuffer_struc;
   M[r9 + $aac.mem.SBR_v_cbuffer_struc_address + (1*ADDR_PER_WORD)] = r0;

   // clear X_sbr buffers
   r0 = 0;
   r10 = $aacdec.X_SBR_LEFTRIGHT_2ENV_SIZE*2;
   r1 = M[r9 + $aac.mem.SBR_X_2env_imag_ptr];
   I0 = r1;
   r1 = M[r9 + $aac.mem.SBR_X_2env_real_ptr];
   I4 = r1;
   do zero_X_sbr_2env_loop;
      M[I0, MK1] = r0,
       M[I4, MK1] = r0;
   zero_X_sbr_2env_loop:

   r10 = $aacdec.X_SBR_LEFTRIGHT_SIZE;
   r1 = M[r9 + $aac.mem.SBR_X_curr_imag_ptr];
   I0 = r1;
   r1 = M[r9 + $aac.mem.SBR_X_curr_real_ptr];
   I4 = r1;
   do zero_X_sbr_curr_loop;
      M[I0, MK1] = r0,
       M[I4, MK1] = r0;
   zero_X_sbr_curr_loop:

   // clear/initialise x_input buffers
   r10 = $aacdec.X_INPUT_BUFFER_LENGTH;
   r1 = M[r9 + $aac.mem.SBR_x_input_buffer_left_ptr];
   I0 = r1;
   M[r9 + $aac.mem.SBR_x_input_buffer_write_pointers      + (0*ADDR_PER_WORD)] = r1;
   M[r9 + $aac.mem.SBR_x_input_buffer_write_base_pointers + (0*ADDR_PER_WORD)] = r1;
   r1 = M[r9 + $aac.mem.SBR_x_input_buffer_right_ptr];
   I4 = r1;
   M[r9 + $aac.mem.SBR_x_input_buffer_write_pointers      + (1*ADDR_PER_WORD)] = r1;
   M[r9 + $aac.mem.SBR_x_input_buffer_write_base_pointers + (1*ADDR_PER_WORD)] = r1;
   do zero_x_input_buffer_loop;
      M[I0, MK1] = r0,
       M[I4, MK1] = r0;
   zero_x_input_buffer_loop:


   // clear/initialise v_buffer buffers
   r10 = $aacdec.SBR_N*10;
   r1 = M[r9 + $aac.mem.SBR_v_buffer_left_ptr];
   I0 = r1;
   r2 = M[r9 + $aac.mem.SBR_v_buffer_right_ptr];
   I4 = r2;
   do zero_v_buffer_loop;
      M[I0, MK1] = r0,
       M[I4, MK1] = r0;
   zero_v_buffer_loop:

   r2 = M[r9 + $aac.mem.SBR_v_buffer_left_ptr];
   r1 = r2 + (127*ADDR_PER_WORD);
   M[r9 + $aac.mem.SBR_v_left_cbuffer_struc + $cbuffer.READ_ADDR_FIELD] = r1;
   M[r9 + $aac.mem.SBR_v_left_cbuffer_struc + $cbuffer.WRITE_ADDR_FIELD] = r1;
#ifdef BASE_REGISTER_MODE
   M[r9 + $aac.mem.SBR_v_left_cbuffer_struc + $cbuffer.START_ADDR_FIELD] = r2;
#endif
   r1 = $aacdec.SBR_N*10*ADDR_PER_WORD;
   M[r9 + $aac.mem.SBR_v_left_cbuffer_struc + $cbuffer.SIZE_FIELD] = r1;


   r2 = M[r9 + $aac.mem.SBR_v_buffer_right_ptr];
   r1 = r2 + (127*ADDR_PER_WORD);
   M[r9 + $aac.mem.SBR_v_right_cbuffer_struc + $cbuffer.READ_ADDR_FIELD] = r1;
   M[r9 + $aac.mem.SBR_v_right_cbuffer_struc + $cbuffer.WRITE_ADDR_FIELD] = r1;
#ifdef BASE_REGISTER_MODE
   M[r9 + $aac.mem.SBR_v_right_cbuffer_struc + $cbuffer.START_ADDR_FIELD] = r2;
#endif
   r1 = $aacdec.SBR_N*10*ADDR_PER_WORD;
   M[r9 + $aac.mem.SBR_v_right_cbuffer_struc + $cbuffer.SIZE_FIELD] = r1;


   // clear/initialise X_sbr_other buffers
   r10 = $aacdec.X_SBR_LEFTRIGHT_SIZE;
   r1 = M[r9 + $aac.mem.SBR_X_sbr_other_real_ptr];
   I0 = r1;
   r1 = M[r9 + $aac.mem.SBR_X_sbr_other_imag_ptr];
   I4 = r1;
   do zero_X_sbr_other_loop;
      M[I0, MK1] = r0,
       M[I4, MK1] = r0;
   zero_X_sbr_other_loop:


   // clear/initialise sbr_info
   r10 = $aacdec.SBR_SIZE;
   r1 = M[r9 + $aac.mem.SBR_info_ptr];
   I0 = r1;
   do zero_sbr_info_loop;
      M[I0, MK1] = r0;
   zero_sbr_info_loop:


   M[r1 + $aacdec.SBR_num_crc_bits] = Null;
   M[r1 + $aacdec.SBR_header_count] = Null;
   M[r1 + $aacdec.SBR_bs_stop_freq] = Null;
   M[r1 + $aacdec.SBR_bs_stop_freq_prev] = Null;
   M[r1 + $aacdec.SBR_bs_freq_scale_prev] = Null;
   M[r1 + $aacdec.SBR_bs_alter_scale_prev] = Null;
   M[r1 + $aacdec.SBR_bs_xover_band] = Null;
   M[r1 + $aacdec.SBR_bs_xover_band_prev] = Null;
   M[r1 + $aacdec.SBR_bs_noise_bands_prev] = Null;
   M[r1 + $aacdec.SBR_k0] = Null;
   M[r1 + $aacdec.SBR_k2] = Null;
   M[r1 + $aacdec.SBR_kx] = Null;

   r0 = -1;
   M[r1 + $aacdec.SBR_bs_start_freq_prev] = r0;

   r0 = 1;
   M[r1 + $aacdec.SBR_bs_interpol_freq] = r0;
   M[r1 + $aacdec.SBR_bs_smoothing_mode] = r0;
   M[r1 + $aacdec.SBR_reset] = r0;
   M[r1 + $aacdec.SBR_bs_alter_scale] = r0;
   M[r1 + $aacdec.SBR_bs_amp_res] = r0;

   r0 = 2;
   M[r1 + $aacdec.SBR_bs_noise_bands] = r0;
   M[r1 + $aacdec.SBR_bs_limiter_bands] = r0;
   M[r1 + $aacdec.SBR_bs_limiter_gains] = r0;
   M[r1 + $aacdec.SBR_bs_freq_scale] = r0;

   r0 = 5;
   M[r1 + $aacdec.SBR_bs_start_freq] = r0;

   M[r9 + $aac.mem.SBR_PRESENT_FIELD] = Null;
#endif //AACDEC_SBR_ADDITIONS



#ifdef AACDEC_PS_ADDITIONS
   r10 = $aacdec.PS_INFO_SIZE;
   r0 = M[r9 + $aac.mem.PS_info_ptr];
   I0 = r0;
   r0 = 0;
   do zero_ps_info;
      M[I0, MK1] = r0;
   zero_ps_info:

   M[r9 + $aac.mem.PS_present] = Null;
#endif //AACDEC_PS_ADDITIONS



#ifdef AACDEC_ELD_ADDITIONS
   r0 = -1;
   M[r9 + $aac.mem.ELD_frame_length_flag] = r0;
#endif // AACDEC_ELD_ADDITIONS



   r0 = 0xDEAD00;
   M[r9 + $aac.mem.PNS_RAND_NUM] = r0;
   // discard half word awaiting to be written into the buffer
   M[r9 + $aac.mem.WRITE_BYTEPOS] = Null;

   I0 = r9 + $aac.mem.SYNTATIC_ELEMENT_FUNC_TABLE;
   r0 = &$aacdec.decode_sce;
   M[I0, MK1] =  r0;
   r0 = &$aacdec.decode_cpe;
   M[I0, MK1] =  r0;
   r0 = -1;
   M[I0, MK1] =  r0;
   M[I0, MK1] =  r0;
   r0 = &$aacdec.discard_dse;
   M[I0, MK1] = r0;
   r0 = &$aacdec.program_element_config;
   M[I0, MK1] =  r0;
   r0 = &$aacdec.decode_fil;
   M[I0, MK1] = r0;
   r0 = 0;
   M[I0, MK1] = r0;

    // set links to supported file types
   I0 = r9 + $aac.mem.READ_FRAME_FUNC_TABLE;

#ifdef AACDEC_MP4_FILE_TYPE_SUPPORTED
      r0 =  &$aacdec.mp4_read_frame;
#else
      r0 = &$error;
#endif
   M[I0, MK1] = r0;
#ifdef AACDEC_ADTS_FILE_TYPE_SUPPORTED
      r0 = &$aacdec.adts_read_frame;
#else
      r0 = &$error;
#endif
   M[I0, MK1] = r0;
#ifdef AACDEC_LATM_FILE_TYPE_SUPPORTED
      r0 = &$aacdec.latm_read_frame;
#else
      r0 = &$error;
#endif
   M[I0, MK1] = r0;



   // list of huffman tables in flash
   I0 = r9 + $aac.mem.huffman_packed_list;
   r0 = &$aacdec.huff_packed_bookscl;
   M[I0, MK1] = r0;

   r0 = &$aacdec.huff_packed_book1;
   M[I0, MK1]= r0;
   r0 = &$aacdec.huff_packed_book2;
   M[I0, MK1]= r0;
   r0 = &$aacdec.huff_packed_book3;
   M[I0, MK1]= r0;
   r0 = &$aacdec.huff_packed_book4;
   M[I0, MK1]= r0;
   r0 = &$aacdec.huff_packed_book5;
   M[I0, MK1]= r0;
   r0 = &$aacdec.huff_packed_book6;
   M[I0, MK1]= r0;
   r0 = &$aacdec.huff_packed_book7;
   M[I0, MK1]= r0;
   r0 = &$aacdec.huff_packed_book8;
   M[I0, MK1]= r0;
   r0 = &$aacdec.huff_packed_book9;
   M[I0, MK1]= r0;
   r0 = &$aacdec.huff_packed_book10;
   M[I0, MK1]= r0;
   r0 = &$aacdec.huff_packed_book11;
   M[I0, MK1]= r0;


   // sizes/2 of the huffman tables once unpacked
   I0 = r9 + $aac.mem.huffman_table_sizes_div_2;
   r0 = $aac.HUFFMAN_bookscl_SIZE / 2;
   M[I0, MK1]= r0;
   r0 = $aac.HUFFMAN_book1_SIZE / 2;
   M[I0, MK1]= r0;
   r0 = $aac.HUFFMAN_book2_SIZE / 2;
   M[I0, MK1]= r0;
   r0 = $aac.HUFFMAN_book3_SIZE / 2;
   M[I0, MK1]= r0;
   r0 = $aac.HUFFMAN_book4_SIZE / 2;
   M[I0, MK1]= r0;
   r0 = $aac.HUFFMAN_book5_SIZE / 2;
   M[I0, MK1]= r0;
   r0 = $aac.HUFFMAN_book6_SIZE / 2;
   M[I0, MK1]= r0;
   r0 = $aac.HUFFMAN_book7_SIZE / 2;
   M[I0, MK1]= r0;
   r0 = $aac.HUFFMAN_book8_SIZE / 2;
   M[I0, MK1]= r0;
   r0 = $aac.HUFFMAN_book9_SIZE / 2;
   M[I0, MK1]= r0;
   r0 = $aac.HUFFMAN_book10_SIZE / 2;
   M[I0, MK1]= r0;
   r0 = $aac.HUFFMAN_book11_SIZE / 2;
   M[I0, MK1]= r0;
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $_aac_decode_reset
//
// DESCRIPTION:
//    C - callable version of $aacdec.reset_decoder.
//
// INPUTS:
//    - r0 = pointer to decoder structure
//      allocated
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1, r3, r10, DoLoop, I0, I4
//
// *****************************************************************************
.MODULE $M.aacdec.reset_decoder;
   .CODESEGMENT AACDEC_RESET_DECODER_PM;
   .DATASEGMENT DM;

   $_aac_decode_lib_reset:
   // Save the registers C expects us to preserve that get trashed
   pushm <r9, rLink>;
   push I0;

   r9 = r0;
   call $aacdec.reset_decoder;

   // restore preserved registers
   pop I0;
   popm <r9, rLink>;
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $_aac_decode_free_decoder_twiddle
//
// DESCRIPTION:
//    C - callable version of $aacdec.free_decoder_twiddle.
//
// INPUTS:
//    - None
//
// OUTPUTS:
//    - None
//
// TRASHED REGISTERS:
//    r0, DoLoop, I1?
//
// *****************************************************************************
.MODULE $M.aacdec.free_decoder_twiddle;
   .CODESEGMENT AACDEC_RESET_DECODER_PM;
   .DATASEGMENT DM;

   $_aac_decode_free_decoder_twiddle:
   // Save the registers C expects us to preserve that get trashed
   pushm <r5, r9, rLink>;
   push I0;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aac.RESET_DECODER_ASM.FREE_DECODER_TWIDDLE.AAC_DECODE_FREE_DECODER_TWIDDLE.PATCH_ID_0, r5)
#endif
   
   
   r0 = $aac.FFT_TWIDDLE_SIZE;
   call $math.fft_twiddle.release;

   // restore preserved registers
   pop I0;
   popm <r5, r9, rLink>;

   rts;

.ENDMODULE;

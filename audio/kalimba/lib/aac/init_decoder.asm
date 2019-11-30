// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.init_decoder
//
// DESCRIPTION:
//    This library contains functions to decode AAC and AAC+SBR. This function
//    initialises the decoder.
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3
//
// *****************************************************************************
.MODULE $M.aacdec.init_decoder;
   .CODESEGMENT AACDEC_INIT_DECODER_PM;
   .DATASEGMENT DM;

   $aacdec.init_decoder:

   // push rLink onto stack
   push rLink;

   // -- reset decoder variables --
   call $aacdec.reset_decoder;

   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.INIT_DECODER_ASM.INIT_DECODER.INIT_DECODER.PATCH_ID_0, r1)
#endif
   
   
   call $aacdec.init_tables;


   // initialise profiling and macros if enabled
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $aacdec.profile_frame_decode[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      .VAR/DM1 $aacdec.profile_read_frame[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      .VAR/DM1 $aacdec.profile_raw_data_block[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      .VAR/DM1 $aacdec.profile_read_scalefactors[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      .VAR/DM1 $aacdec.profile_read_spectral_data[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      .VAR/DM1 $aacdec.profile_apply_scalefactors_and_dequantize[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      .VAR/DM1 $aacdec.profile_reorder_spec[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      .VAR/DM1 $aacdec.profile_ms_decode[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      .VAR/DM1 $aacdec.profile_pns_decode[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      .VAR/DM1 $aacdec.profile_is_decode[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      .VAR/DM1 $aacdec.profile_ltp_decode[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      .VAR/DM1 $aacdec.profile_tns_encdec[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      .VAR/DM1 $aacdec.profile_imdct[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      .VAR/DM1 $aacdec.profile_windowing[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      .VAR/DM1 $aacdec.profile_overlap_add[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;

      #ifdef AACDEC_SBR_ADDITIONS
         .VAR/DM1 $aacdec.profile_sbr_synthesis_filterbank[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_analysis_filterbank[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_hf_generation[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_hf_generation_internal[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_calc_chirp_factors[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_patch_construction[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_auto_correlation_opt[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_prediction_coeff[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_limiter_frequency_table[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_limiter_frequency_table_internal[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;

         .VAR/DM1 $aacdec.profile_sbr_synthesis_filterbank_combined[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_wrap_last_thfgen_envelopes[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_estimate_current_envelope[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_estimate_current_envelope_internal[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;

         .VAR/DM1 $aacdec.profile_sbr_calculate_gain[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_calculate_gain_loop1[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_calculate_gain_loop2[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_calculate_gain_loop3[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;

         .VAR/DM1 $aacdec.profile_sbr_grid[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_dtdf[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_invf[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_envelope[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_noise[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_envelope_noise_dequantisation[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_envelope_noise_dequantisation_coupling_mode[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_extract_envelope_data[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_extract_noise_floor_data[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;

         .VAR/DM1 $aacdec.profile_sbr_hf_assembly[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_hf_assembly_loop_1[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_hf_assembly_loop_2[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_hf_assembly_loop_3[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_hf_assembly_loop_4[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;

         .VAR/DM1 $aacdec.profile_sbr_calc_tables[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_channel_pair_element[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_sbr_extension_data[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;

         .VAR/DM1 $aacdec.profile_sbr_construct_x_matrix[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      #endif

      #ifdef AACDEC_PS_ADDITIONS
         .VAR/DM1 $aacdec.profile_ps_data_decode[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_ps_delta_decode[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_ps_data[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_ps_huff_dec[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_ps_extension[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;

         .VAR/DM1 $aacdec.profile_ps_hybrid_analysis[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_ps_hybrid_analysis_type_a_fir[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_ps_hybrid_analysis_type_b_fir[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;

         .VAR/DM1 $aacdec.profile_ps_transient_detection[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_ps_decorrelation[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_ps_stereo_processing[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_ps_hybrid_synthesis[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;

         .VAR/DM1 $aacdec.profile_ps_decorr_transient_detection[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
         .VAR/DM1 $aacdec.profile_ps_decorr_all_pass_filter[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      #endif
   #endif



   ///////////////////////////////////////
   //handle selecting of AAC file format//
   ///////////////////////////////////////

   // set up message handler for file type message
   //.VAR set_mp4_file_type_message_struc[$message.STRUC_SIZE];
#ifndef KYMERA
   r1 = r9 + $aac.mem.set_mp4_file_type_message_struc;
   r2 = $MESSAGE_AACDEC_SET_FILE_TYPE;
   r3 = &$aacdec.set_mp4_file_type_handler;
   call $message.register_handler;
   jump set_up_handler;


   // message handler for file type message
   $aacdec.set_mp4_file_type_handler:
      // INPUTS:
      //    - r1 = file type, 0 means mp4, 1 adts, 2 latm
      // OUTPUTS:
      //    - none
      r2 = r9 + $aac.mem.READ_FRAME_FUNC_TABLE;
      r1 = M[r2  + r1];

      //M[&$aacdec.read_frame_function] = r1;
      M[r9 +$aac.mem.READ_FRAME_FUNCTION ] = r1;
      rts;


   set_up_handler:

#else
      r1 = M[r9 +$aac.mem.READ_FRAME_FUNCTION];
      Words2Addr(r1);
      r2 = r9 + $aac.mem.READ_FRAME_FUNC_TABLE ;
      r1 = M[r2  + r1];

      M[r9 +$aac.mem.READ_FRAME_FUNCTION ] = r1;

      NULL = M[r9 + $aac.mem.FFT_TABLE_INIT_DONE_FIELD];
      if NZ jump dont_init_fft_tables_aac;
      r0 = $aac.FFT_TWIDDLE_SIZE;
      call $math.fft_twiddle.alloc;
      r0 = 1;
      M[r9 + $aac.mem.FFT_TABLE_INIT_DONE_FIELD] = r0;
dont_init_fft_tables_aac:
 #endif


   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_aac_decode_init
//
// DESCRIPTION:
//    C - callable version of $aacdec.init_decoder.
//
// INPUTS:
//    - r0 = pointer to decoder structure
//      allocated
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-r3, r10, DoLoop
//
// *****************************************************************************
.MODULE $M.aacdec.init_decoder;
   .CODESEGMENT AACDEC_INIT_DECODER_PM;
   .DATASEGMENT DM;

   $_aac_decode_lib_init:
   // Save the registers C expects us to preserve that get trashed
   pushm <r5, r9, rLink>;
   push I0;

   r5 = r0;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aac.INIT_DECODER_ASM.INIT_DECODER.AAC_DECODE_LIB_INIT.PATCH_ID_0, r9)
#endif
   
   // -- Load memory structure pointer
   // This pointer should have been initialised externally
   r9 = M[r5 + $codec.DECODER_DATA_OBJECT_FIELD];

   call $aacdec.init_decoder;

   // restore preserved registers
   pop I0;
   popm <r5, r9, rLink>;

   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $aacdec.deinit_decoder
//
// DESCRIPTION:
//    remove (unregister) message AAC decoder message handlers
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3
//
// *****************************************************************************
.MODULE $M.aacdec.deinit_decoder;
   .CODESEGMENT AACDEC_DEINIT_DECODER_PM;
   .DATASEGMENT DM;

   $aacdec.deinit_decoder:

   // push rLink onto stack
   push rLink;

   // remove handler
   r3 = $MESSAGE_AACDEC_SET_FILE_TYPE;
   call $message.unregister_handler;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

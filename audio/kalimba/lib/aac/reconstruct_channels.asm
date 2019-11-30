// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.reconstruct_channels
//
// DESCRIPTION:
//    Reconstruct a channel pair
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - assume everything including $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.reconstruct_channels;
   .CODESEGMENT AACDEC_RECONSTRUCT_CHANNELS_PM;
   .DATASEGMENT DM;

   $aacdec.reconstruct_channels:

   // push rLink onto stack
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.RECONSTRUCT_CHANNELS_ASM.RECONSTRUCT_CHANNELS.RECONSTRUCT_CHANNELS.PATCH_ID_0, r5)
#endif
   
   

   #ifdef AACDEC_SBR_ADDITIONS
      #ifdef AACDEC_SBR_HALF_SYNTHESIS
         Null = M[r9 + $aac.mem.SBR_in_synth];
         if Z jump not_in_synth;
         r8 = 32;
         M[r9 + $aac.mem.SBR_in_synth_loops] = r8;
         r8 = 16;
         Null = M[r9 + $aac.mem.NUM_CPEs];
         if Z jump jump_to_synth_single_channel;
         jump jump_to_synth_channel_pair;
         not_in_synth:
      #endif
   #endif

   // -- Optionally flag to convert from mono to stereo --
   r5 = M[r9 + $aac.mem.codec_struc];
   r0 = 1;
   Null = M[r5 + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD];
   if Z r0 = 0;
   Null = M[r9 + $aac.mem.NUM_SCEs];
   if Z r0 = 0;
   #ifdef AACDEC_PS_ADDITIONS
      Null = M[r9 + $aac.mem.PS_present];
      if NZ r0 = 0;
   #endif
   M[r9 + $aac.mem.CONVERT_MONO_TO_STEREO_FIELD] = r0;


   // -- apply_scalefactors_and_dequantize: --
   call select_left_channel;
   PROFILER_START(&$aacdec.profile_apply_scalefactors_and_dequantize)
   call $aacdec.apply_scalefactors_and_dequantize;
   PROFILER_STOP(&$aacdec.profile_apply_scalefactors_and_dequantize)
   PROFILER_START(&$aacdec.profile_reorder_spec)
   r0 = M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD];
   Null = r0 - $aacdec.EIGHT_SHORT_SEQUENCE;
   if Z call $aacdec.reorder_spec;
   Null = M[r9 + $aac.mem.FRAME_CORRUPT];
   if NZ jump frame_corrupt;
   PROFILER_STOP(&$aacdec.profile_reorder_spec)

   Null = M[r9 + $aac.mem.NUM_CPEs];
   if Z jump skip_right;
   call select_right_channel;
   PROFILER_START(&$aacdec.profile_apply_scalefactors_and_dequantize)
   call $aacdec.apply_scalefactors_and_dequantize;
   PROFILER_STOP(&$aacdec.profile_apply_scalefactors_and_dequantize)
   PROFILER_START(&$aacdec.profile_reorder_spec)
   r0 = M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD];
   Null = r0 - $aacdec.EIGHT_SHORT_SEQUENCE;
   if Z call $aacdec.reorder_spec;
   Null = M[r9 + $aac.mem.FRAME_CORRUPT];
   if NZ jump frame_corrupt;
   PROFILER_STOP(&$aacdec.profile_reorder_spec)

   // -- MS decode --
   PROFILER_START(&$aacdec.profile_ms_decode)
   Null = M[r9 + $aac.mem.ICS_LEFT + $aacdec.ics.MS_MASK_PRESENT_FIELD];
   if NZ call $aacdec.ms_decode;
   PROFILER_STOP(&$aacdec.profile_ms_decode)

   // -- PNS decode --
   PROFILER_START(&$aacdec.profile_pns_decode)
   call select_left_channel;
#ifndef AACDEC_ELD_ADDITIONS
   r0 = pns_channel_pair_mode;
#else
   r0 = pns_done;
   r1 = pns_channel_pair_mode;
   r2 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r2 - $aacdec.ER_AAC_ELD;
   if NE r0 = r1;
#endif

   Null = M[r4 + $aacdec.ics.MS_MASK_PRESENT_FIELD];
   if NZ jump r0;
      // not channel_pair mode
      M3 = 0;
      call $aacdec.pns_decode;
      call select_right_channel;
      call $aacdec.pns_decode;
      jump pns_done;

   pns_channel_pair_mode:
      // channel_pair mode
      M3 = 1;
      call $aacdec.pns_decode;
   pns_done:
   PROFILER_STOP(&$aacdec.profile_pns_decode)


   // -- IS decode --
   PROFILER_START(&$aacdec.profile_is_decode)
   call $aacdec.is_decode;
   PROFILER_STOP(&$aacdec.profile_is_decode)
   jump free_all;

   skip_right:
   PROFILER_START(&$aacdec.profile_pns_decode)
   M3 = 0;
   call $aacdec.pns_decode;
   PROFILER_STOP(&$aacdec.profile_pns_decode)

   free_all:
   // free all tmp memory
   call $aacdec.tmp_mem_pool_free_all;

   #ifdef AAC_LOWRAM
      // Allocate 512 words from the end of tmp_mem_pool to load fft
      // twiddle factors
      // This is done in this way because tmp_mem_pool
      // is used from the beginning of it in imdct/filterbank
      r0 = 512;
      r1 =  M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r1 = r1  + $aacdec.TMP_MEM_POOL_LENGTH;
      r1 = r1 - 512;
      r2 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r2 = r2 + $aacdec.TMP_MEM_POOL_LENGTH;
      r2 = r2 - 256;
      call $math.load_fft_twiddle_factors;
   #endif

   // -- LTP decode --
   call select_left_channel;
   PROFILER_START(&$aacdec.profile_ltp_decode)
   // reset $aacdec.ics.TNS_ENCODE_DONE_FIELD for left channel
   M[r4 + $aacdec.ics.TNS_ENCODE_DONE_FIELD] = Null;
   Null = M[r4 + $aacdec.ics.PREDICTOR_DATA_PRESENT_FIELD];
   // ltp_decode if needed
   #ifdef AACDEC_SBR_ADDITIONS
      if Z jump no_ltp_left;
         Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
         #ifndef AACDEC_CALL_ERROR_ON_POSSIBLE_CORRUPTION
            if NZ call $error;         // Currently LTP & SBR together not supported
         #endif
         if Z call $aacdec.ltp_decode;
      no_ltp_left:
   #else
      if NZ call $aacdec.ltp_decode;
   #endif
   Null = M[r9 + $aac.mem.FRAME_CORRUPT];
   if NZ jump frame_corrupt;
   PROFILER_STOP(&$aacdec.profile_ltp_decode)
   PROFILER_START(&$aacdec.profile_tns_encdec)
   // select tns decode mode
   M2 = 0;
   Null = M[r4 + $aacdec.ics.TNS_DATA_PTR_FIELD];
   if NZ call $aacdec.tns_encdec;
   Null = M[r9 + $aac.mem.FRAME_CORRUPT];
   if NZ jump frame_corrupt;
   PROFILER_STOP(&$aacdec.profile_tns_encdec)
   call $aacdec.filterbank;

   Null = M[r9 + $aac.mem.NUM_CPEs];
   if Z jump plain_decoder_done;
   call select_right_channel;
   PROFILER_START(&$aacdec.profile_ltp_decode)
   // reset $aacdec.ics.TNS_ENCODE_DONE_FIELD for right channel
   M[r4 + $aacdec.ics.TNS_ENCODE_DONE_FIELD] = Null;
   Null = M[r4 + $aacdec.ics.PREDICTOR_DATA_PRESENT_FIELD];
   #ifdef AACDEC_SBR_ADDITIONS
      if Z jump no_ltp_right;
         Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
         #ifndef AACDEC_CALL_ERROR_ON_POSSIBLE_CORRUPTION
            if NZ call $error;         // Currently LTP & SBR together not supported
         #endif
         if Z call $aacdec.ltp_decode;
      no_ltp_right:
   #else
      if NZ call $aacdec.ltp_decode;
   #endif
   Null = M[r9 + $aac.mem.FRAME_CORRUPT];
   if NZ jump frame_corrupt;
   PROFILER_STOP(&$aacdec.profile_ltp_decode)
   PROFILER_START(&$aacdec.profile_tns_encdec)
   // select tns decode mode
   M2 = 0;
   Null = M[r4 + $aacdec.ics.TNS_DATA_PTR_FIELD];
   if NZ call $aacdec.tns_encdec;
   Null = M[r9 + $aac.mem.FRAME_CORRUPT];
   if NZ jump frame_corrupt;
   PROFILER_STOP(&$aacdec.profile_tns_encdec)
   call $aacdec.filterbank;
   plain_decoder_done:

#ifdef AACDEC_SBR_ADDITIONS
   //-----LEFT CHANNEL----------
   // free all frame memory
   call $aacdec.frame_mem_pool_free_all;
   // free all tmp memory again
   call $aacdec.tmp_mem_pool_free_all;

   // -- SBR decode --
   Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
   if Z jump sbr_not_present_left;
      call select_left_channel;

      PROFILER_START(&$aacdec.profile_sbr_analysis_filterbank)
      call $aacdec.sbr_analysis_filterbank;
      PROFILER_STOP(&$aacdec.profile_sbr_analysis_filterbank)
      Null = M[r9 + $aac.mem.FRAME_CORRUPT];
      if NZ jump frame_corrupt;
      // if(SBR_HF_reconstruction)
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      Null = M[r0 + $aacdec.SBR_HF_reconstruction];
      if Z jump no_hf_reconstruction_left;
         //PROFILER_START(&$aacdec.profile_sbr_hf_generation)
         call $aacdec.sbr_hf_generation;
         //PROFILER_STOP(&$aacdec.profile_sbr_hf_generation)
         Null = M[r9 + $aac.mem.FRAME_CORRUPT];
         if NZ jump frame_corrupt;
         r5 = 0;  // ch0
         call $aacdec.sbr_hf_adjustment;
         Null = M[r9 + $aac.mem.FRAME_CORRUPT];
         if NZ jump frame_corrupt;
      no_hf_reconstruction_left:
      //PROFILER_START(&$aacdec.profile_sbr_construct_x_matrix)
      call $aacdec.sbr_construct_x_matrix;
      //PROFILER_STOP(&$aacdec.profile_sbr_construct_x_matrix)

      // Parametric Stereo
      #ifdef AACDEC_PS_ADDITIONS
         Null = M[r9 + $aac.mem.PS_present];
         if Z jump end_if_parametric_stereo_present;
            PROFILER_START(&$aacdec.profile_ps_data_decode)
            call $aacdec.ps_data_decode;
            PROFILER_STOP(&$aacdec.profile_ps_data_decode)

            PROFILER_START(&$aacdec.profile_ps_hybrid_analysis)
            call $aacdec.ps_hybrid_analysis;
            PROFILER_STOP(&$aacdec.profile_ps_hybrid_analysis)

            M0 = $aacdec.X_SBR_WIDTH*ADDR_PER_WORD;
            r0 = 0;

            // update contents of ps_hybrid_analysis history buffer
            // for use in next frame

            // for p=0:(PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS-1)

            update_analysis_fir_history_buffer_subband_loop:
               // ps_time_history[p][n=0:11] = X_SBR[ch=0][k=p][l=(SBR_tHFAdj+26):(SBR_tHFAdj+37)]

               // I0 <- real(X_SBR[ch=0][k=p][l=SBR_tHFAdj+26])
               r1 = M[r9 + $aac.mem.SBR_X_curr_real_ptr];
               Words2Addr(r0);
               r1 = r1 + r0;
               I0 = r1 + ((28-$aacdec.SBR_tHFAdj)*$aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
               // I4 <- imag(X_SBR[ch=0][k=p][l=SBR_tHFAdj+26])
               r1 = M[r9 + $aac.mem.SBR_X_curr_imag_ptr];
               r1 = r1 + r0;
               I4 = r1 + ((28-$aacdec.SBR_tHFAdj)*$aacdec.X_SBR_WIDTH*ADDR_PER_WORD);

               r10 = $aacdec.PS_HYBRID_ANALYSIS_FIR_FILTER_LENGTH - 1;

               r1 = r0 * ($aacdec.PS_HYBRID_ANALYSIS_FIR_FILTER_LENGTH - 1) (int);
               I1 = $aac.mem.PS_time_history_real + r9;
               I1 = I1 + r1;  // I1 <- real(ps_time_history[p][n=0])
               I5 = $aac.mem.PS_time_history_imag + r9;
               I5 = I5 + r1;  // I5 <- imag(ps_time_history[p][n=0])

               do update_analysis_fir_history_buffer_time_sample_loop;
                  r1 = M[I0,M0],
                   r2 = M[I4,M0];

                  M[I1, MK1] = r1,
                   M[I5, MK1] = r2;
               update_analysis_fir_history_buffer_time_sample_loop:
               Addr2Words(r0);

               r0 = r0 + 1;
               Null = r0 - $aacdec.PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS;
            if LT jump update_analysis_fir_history_buffer_subband_loop;

            //PROFILER_START(&$aacdec.profile_ps_transient_detection)
            call $aacdec.ps_transient_detection;
            //PROFILER_STOP(&$aacdec.profile_ps_transient_detection)

            PROFILER_START(&$aacdec.profile_ps_decorrelation)
            call $aacdec.ps_decorrelate;
            PROFILER_STOP(&$aacdec.profile_ps_decorrelation)

            PROFILER_START(&$aacdec.profile_ps_stereo_processing)
            call $aacdec.ps_stereo_mixing;
            PROFILER_STOP(&$aacdec.profile_ps_stereo_processing)

            // complete left chanel hybrid synthesis
            r4 = $aacdec.PS_NUM_SAMPLES_PER_FRAME;
            M2 = 0;  // set flag to start synthesis at beginning of SBR frame
            r5 = 0;  // ch = 0
            PROFILER_START(&$aacdec.profile_ps_hybrid_synthesis)
            call $aacdec.ps_hybrid_synthesis;
            PROFILER_STOP(&$aacdec.profile_ps_hybrid_synthesis)

         end_if_parametric_stereo_present:
      #endif
      // store main synthesis loop number in r8
      #ifdef AACDEC_SBR_HALF_SYNTHESIS
         r8 = 16;
         M[r9 + $aac.mem.SBR_in_synth_loops] = r8;
      #endif
      r8 = 0;


      jump_to_synth_single_channel:
      r5 = M[r9 + $aac.mem.codec_struc];
      r0 = M[r5 + $codec.DECODER_OUT_LEFT_BUFFER_FIELD];
#ifdef BASE_REGISTER_MODE
      call $cbuffer.get_write_address_and_size_and_start_address;
      M[r9 + $aac.mem.TMP + (11*ADDR_PER_WORD)] = r2;
#else
      call $cbuffer.get_write_address_and_size;
#endif
      M[r9 + $aac.mem.TMP + (5*ADDR_PER_WORD)] = r0;
      M[r9 + $aac.mem.TMP + (6*ADDR_PER_WORD)] = r1;


      r7 = $aacdec.SBR_LEFT_CH;
      PROFILER_START(&$aacdec.profile_sbr_synthesis_filterbank_combined)
      call $aacdec.sbr_synthesis_filterbank_combined;
      PROFILER_STOP(&$aacdec.profile_sbr_synthesis_filterbank_combined)


      // write back current output buffer location
      r5 = M[r9 + $aac.mem.codec_struc];
      r0 = M[r5 + $codec.DECODER_OUT_LEFT_BUFFER_FIELD];
      r1 = I5;
      call $cbuffer.set_write_address;
      Null = M[r9 + $aac.mem.NUM_CPEs];
      if Z jump single_channel;

         #ifdef AACDEC_SBR_HALF_SYNTHESIS
            // store main synthesis loop number in r8
            r0 = 32;
            M[r9 + $aac.mem.SBR_in_synth_loops] = r0;
            r0 = M[r9 + $aac.mem.SBR_synth_temp_ptr];
            M[r9 + $aac.mem.TMP + (5*ADDR_PER_WORD)] = r0;
            M[r9 + $aac.mem.TMP + (6*ADDR_PER_WORD)] = Null;
            PROFILER_START(&$aacdec.profile_sbr_synthesis_filterbank_combined)
            call $aacdec.sbr_synthesis_filterbank_combined;
            PROFILER_STOP(&$aacdec.profile_sbr_synthesis_filterbank_combined)
         #endif
         // deallocate temp buffer for X_sbr_shared if finished synthesis
         r0 = $aacdec.X_SBR_SHARED_SIZE;
         call $aacdec.tmp_mem_pool_free;
         jump channel_pair;

      single_channel:
         #ifdef AACDEC_SBR_HALF_SYNTHESIS
            #ifdef AACDEC_PS_ADDITIONS
               Null = M[r9 + $aac.mem.PS_present];
               if NZ jump synthesis_parametric_stereo_right_channel;
            #endif
            r1 = M[r9 + $aac.mem.SBR_in_synth];
            r1 = 1 - r1;
            M[r9 + $aac.mem.SBR_in_synth] = r1;
            if NZ jump $pop_rLink_and_rts;
            synthesis_parametric_stereo_right_channel:
         #endif
         call $aacdec.tmp_mem_pool_free_all;

      channel_pair:
      r5 = 0;  // ch0
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      Null = M[r0 + $aacdec.SBR_HF_reconstruction];
      if NZ call $aacdec.sbr_save_prev_data;

      //PROFILER_START(&$aacdec.profile_sbr_wrap_last_thfgen_envelopes)
      call $aacdec.sbr_wrap_last_thfgen_envelopes;
      //PROFILER_STOP(&$aacdec.profile_sbr_wrap_last_thfgen_envelopes)


      // synthesise right channel if Parametric Stereo is present in this frame
      #ifdef AACDEC_PS_ADDITIONS
         Null = M[r9 + $aac.mem.PS_present];
         if Z jump end_if_parametric_stereo_processing_done;

            // copy envelopes of X_SBR[ch=1][][] into sbr_x_real[] and sbr_x_imag[] ready for synthesis

            // memory organisation of X_SBR[ch=1][][] is as follows

            // X_sbr_other_real = real(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+0:SBR_tHFAdj+5])
            // fmp_remains = real(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+6:SBR_tHFAdj+PS_NUM_SAMPLES_PER_FRAME-1])

            // X_sbr_other_imag = imag(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+0:SBR_tHFAdj+5])
            // overlap_add_right = imag(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+6:SBR_tHFAdj+14])
            // x_input_buffer_right = imag(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+15:SBR_tHFAdj+19])
            // fmp_remains[((PS_NUM_SAMPLES_PER_FRAME-6)*64):(26*64 + 12*64 - 1)] = ...
                                                   // imag(X_SBR[ch=1][k=0:63][l=SBR_tHFAdj+20:SBR_tHFAdj+PS_NUM_SAMPLES_PER_FRAME-1])

            call $aacdec.sbr_swap_channels;

            #ifdef AACDEC_SBR_HALF_SYNTHESIS
               Null = M[r9 + $aac.mem.SBR_in_synth];
               if NZ jump synthesise_second_half_right_channel;

                  // copy real(D_k[n=0:15][k=0:63]) into synthesis buffers
                  r10 = $aacdec.X_SBR_WIDTH * 10;
                  r0 = M[r9 + $aac.mem.SBR_X_curr_real_ptr];
                  I0 = r0 + (($aacdec.SBR_tHFGen-$aacdec.SBR_tHFAdj)*$aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
                  r0 = M[r9 + $aac.mem.PS_fmp_remains_ptr];
                  I1 = r0;

                  do parametric_stereo_right_copy_loop_one;
                     r0 = M[I1, MK1];
                     M[I0, MK1] = r0;
                  parametric_stereo_right_copy_loop_one:

                  // copy imag(D_k[n=0:15][k=0:63]) into synthesis buffers
                  r10 = $aacdec.X_SBR_WIDTH * 9;
                  r0 = M[r9 + $aac.mem.SBR_X_curr_imag_ptr];
                  I0 = r0 + (($aacdec.SBR_tHFGen-$aacdec.SBR_tHFAdj)*$aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
                  r0 = M[r9 + $aac.mem.OVERLAP_ADD_RIGHT_PTR];
                  I1 = r0;

                  do parametric_stereo_right_copy_loop_two;
                     r0 = M[I1, MK1];
                     M[I0, MK1] = r0;
                  parametric_stereo_right_copy_loop_two:

                  r10 = $aacdec.X_SBR_WIDTH * 1;
                  r0 = M[r9 + $aac.mem.SBR_x_input_buffer_right_ptr];
                  I1 = r0;

                  do parametric_stereo_right_copy_loop_debug;
                     r0 = M[I1, MK1];
                     M[I0, MK1] = r0;
                  parametric_stereo_right_copy_loop_debug:

                  // 1st half of right chanel hybrid synthesis
                  r4 = ($aacdec.PS_NUM_SAMPLES_PER_FRAME / 2);
                  M2 = 0;  // set flag to start synthesis at beginning of SBR frame
                  r5 = 1;  // ch = 1
                  PROFILER_START(&$aacdec.profile_ps_hybrid_synthesis)
                  call $aacdec.ps_hybrid_synthesis;
                  PROFILER_STOP(&$aacdec.profile_ps_hybrid_synthesis)

                  jump end_if_first_or_second_half_right_synthesis;
               synthesise_second_half_right_channel:

                  // copy real(D_k[n=16:31][k=0:63]) into synthesis buffers
                  r10 = $aacdec.X_SBR_WIDTH * ($aacdec.PS_NUM_SAMPLES_PER_FRAME / 2);
                  r0 = M[r9 + $aac.mem.SBR_X_curr_real_ptr];
                  I0 = r0 + (($aacdec.SBR_tHFGen-$aacdec.SBR_tHFAdj + 10)*$aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
                  r0 = M[r9 + $aac.mem.PS_fmp_remains_ptr];
                  I1 = r0 + ($aacdec.X_SBR_WIDTH * 10*ADDR_PER_WORD);

                  do parametric_stereo_right_copy_loop_three;
                     r0 = M[I1, MK1];
                     M[I0, MK1] = r0;
                  parametric_stereo_right_copy_loop_three:

                  // copy imag(D_k[n=16:31][k=0:63]) into synthesis buffers
                  r10 = $aacdec.X_SBR_WIDTH * 4;
                  r0 = M[r9 + $aac.mem.SBR_X_curr_imag_ptr];
                  I0 = r0 + (($aacdec.SBR_tHFGen-$aacdec.SBR_tHFAdj + 10)*$aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
                  r0 = M[r9 + $aac.mem.SBR_x_input_buffer_right_ptr];
                  I1 = r0 + ($aacdec.X_SBR_WIDTH * ADDR_PER_WORD);

                  do parametric_stereo_right_copy_loop_four;
                     r0 = M[I1, MK1];
                     M[I0, MK1] = r0;
                  parametric_stereo_right_copy_loop_four:

                  r10 = $aacdec.X_SBR_WIDTH * (($aacdec.PS_NUM_SAMPLES_PER_FRAME / 2) - 4);
                  r0 = M[r9 + $aac.mem.PS_fmp_remains_ptr];
                  I1 = r0 + ($aacdec.X_SBR_WIDTH * 26 * ADDR_PER_WORD);

                  do parametric_stereo_right_copy_loop_five;
                     r0 = M[I1, MK1];
                     M[I0, MK1] = r0;
                  parametric_stereo_right_copy_loop_five:

                  // 2nd half of right chanel hybrid synthesis
                  r4 = ($aacdec.PS_NUM_SAMPLES_PER_FRAME / 2);
                  M2 = 1;  // set flag to start synthesis at mid point of SBR frame
                  r5 = 1;  // ch = 1
                  PROFILER_START(&$aacdec.profile_ps_hybrid_synthesis)
                  call $aacdec.ps_hybrid_synthesis;
                  PROFILER_STOP(&$aacdec.profile_ps_hybrid_synthesis)

               end_if_first_or_second_half_right_synthesis:

               // store main synthesis loop number in r8
               r8 = M[r9 + $aac.mem.SBR_in_synth_loops];
               r8 = r8 - 16;
            #else

               // copy real(D_k[n=0:31][k=0:63]) into synthesis buffers


               r10 = $aacdec.X_SBR_WIDTH * ($aacdec.PS_NUM_SAMPLES_PER_FRAME - ($aacdec.SBR_tHFGen-$aacdec.SBR_tHFAdj));
#ifdef AACDEC_ELD_ADDITIONS
               r1 = $aacdec.X_SBR_WIDTH * ($aacdec.PS_NUM_SAMPLES_PER_FRAME - ($aacdec.SBR_tHFGen_eld-$aacdec.SBR_tHFAdj));
               r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
               Null = r0 - $aacdec.ER_AAC_ELD;
               if Z r10 = r1;
#endif

               r0 = M[r9 + $aac.mem.SBR_X_curr_real_ptr];

               I0 = r0 + (($aacdec.SBR_tHFGen-$aacdec.SBR_tHFAdj)*$aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
#ifdef AACDEC_ELD_ADDITIONS
               I1 = r0 + (($aacdec.SBR_tHFGen_eld-$aacdec.SBR_tHFAdj)*$aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
               r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
               Null = r0 - $aacdec.ER_AAC_ELD;
               if Z I0 = I1;
#endif

               r0 = M[r9 + $aac.mem.PS_fmp_remains_ptr];
               I1 = r0;

               do parametric_stereo_right_copy_loop_one;
                  r0 = M[I1, MK1];
                  M[I0, MK1] = r0;
               parametric_stereo_right_copy_loop_one:

               // copy imag(D_k[n=0:31][k=0:63]) into synthesis buffers
               r10 = $aacdec.X_SBR_WIDTH * 9;
               r0 = M[r9 + $aac.mem.SBR_X_curr_imag_ptr];

               I0 = r0 + (($aacdec.SBR_tHFGen-$aacdec.SBR_tHFAdj)*$aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
#ifdef AACDEC_ELD_ADDITIONS
               I1 = r0 +  (($aacdec.SBR_tHFGen_eld-$aacdec.SBR_tHFAdj)*$aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
               r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
               Null = r0 - $aacdec.ER_AAC_ELD;
               if Z I0 = I1;
#endif

               r0 = M[r9 + $aac.mem.OVERLAP_ADD_RIGHT_PTR];
               I1 = r0;

               do parametric_stereo_right_copy_loop_two;
                  r0 = M[I1, MK1];
                  M[I0, MK1] = r0;
               parametric_stereo_right_copy_loop_two:

               r10 = $aacdec.X_SBR_WIDTH * 5;
               r0 = M[r9 + $aac.mem.SBR_x_input_buffer_right_ptr];
               I1 = r0;

               do parametric_stereo_right_copy_loop_three;
                  r0 = M[I1, MK1];
                  M[I0, MK1] = r0;
               parametric_stereo_right_copy_loop_three:

               r10 = $aacdec.X_SBR_WIDTH * (($aacdec.PS_NUM_SAMPLES_PER_FRAME - ($aacdec.SBR_tHFGen-$aacdec.SBR_tHFAdj)) - 9 - 5);
#ifdef AACDEC_ELD_ADDITIONS
               r1 = $aacdec.X_SBR_WIDTH * (($aacdec.PS_NUM_SAMPLES_PER_FRAME - ($aacdec.SBR_tHFGen_eld-$aacdec.SBR_tHFAdj)) - 9 - 5);
               r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
               Null = r0 - $aacdec.ER_AAC_ELD;
               if Z r10 = r1;
#endif

               r0 = M[r9 + $aac.mem.PS_fmp_remains_ptr];
               I1 = r0 + ($aacdec.X_SBR_WIDTH * 26 * ADDR_PER_WORD);

               do parametric_stereo_right_copy_loop_four;
                  r0 = M[I1, MK1];
                  M[I0, MK1] = r0;
               parametric_stereo_right_copy_loop_four:


               // complete right chanel hybrid synthesis
               r4 = $aacdec.PS_NUM_SAMPLES_PER_FRAME;
               M2 = 0;  // set flag to start synthesis at beginning of SBR frame
               r5 = 1;  // ch = 1
               PROFILER_START(&$aacdec.profile_ps_hybrid_synthesis)
               call $aacdec.ps_hybrid_synthesis;
               PROFILER_STOP(&$aacdec.profile_ps_hybrid_synthesis)

               r8 = 0;

            #endif

            r5 = M[r9 + $aac.mem.codec_struc];
            r0 = M[r5 + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD];
#ifdef BASE_REGISTER_MODE
            call $cbuffer.get_write_address_and_size_and_start_address;
            M[r9 + $aac.mem.TMP + (11*ADDR_PER_WORD)] = r2;
#else
            call $cbuffer.get_write_address_and_size;
#endif
            M[r9 + $aac.mem.TMP + (5*ADDR_PER_WORD)] = r0;
            M[r9 + $aac.mem.TMP + (6*ADDR_PER_WORD)] = r1;
            r7 = $aacdec.SBR_RIGHT_CH;
            PROFILER_START(&$aacdec.profile_sbr_synthesis_filterbank)
            call $aacdec.sbr_synthesis_filterbank_combined;
            PROFILER_STOP(&$aacdec.profile_sbr_synthesis_filterbank)
            // write back current output buffer location
            r5 = M[r9 + $aac.mem.codec_struc];
            r0 = M[r5 + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD];
            r1 = I5;
            call $cbuffer.set_write_address;
            call $aacdec.sbr_swap_channels;
            #ifdef AACDEC_SBR_HALF_SYNTHESIS
               r1 = M[r9 + $aac.mem.SBR_in_synth];
               r1 = 1 - r1;
               M[r9 + $aac.mem.SBR_in_synth] = r1;
               if NZ jump $pop_rLink_and_rts;
            #endif
         end_if_parametric_stereo_processing_done:
      #endif
   sbr_not_present_left:
   Null = M[r9 + $aac.mem.NUM_CPEs];
   if Z jump sbr_not_present_right;


   //-----RIGHT CHANNEL----------
   // free all tmp memory again
   call $aacdec.tmp_mem_pool_free_all;

   // -- SBR decode --
   Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
   if Z jump sbr_not_present_right;

      call $aacdec.sbr_swap_channels;
      call select_right_channel;

      PROFILER_START(&$aacdec.profile_sbr_analysis_filterbank)
      call $aacdec.sbr_analysis_filterbank;
      PROFILER_STOP(&$aacdec.profile_sbr_analysis_filterbank)
      Null = M[r9 + $aac.mem.FRAME_CORRUPT];
      if NZ jump frame_corrupt;
      // if(SBR_HF_reconstruction)
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      Null = M[r0 + $aacdec.SBR_HF_reconstruction];
      if Z jump no_hf_reconstruction_right;
         //PROFILER_START(&$aacdec.profile_sbr_hf_generation)
         call $aacdec.sbr_hf_generation;
         //PROFILER_STOP(&$aacdec.profile_sbr_hf_generation)
         Null = M[r9 + $aac.mem.FRAME_CORRUPT];
         if NZ jump frame_corrupt;
         r5 = 1;  // ch1
         call $aacdec.sbr_hf_adjustment;
         Null = M[r9 + $aac.mem.FRAME_CORRUPT];
         if NZ jump frame_corrupt;
         r0 = M[r9 + $aac.mem.SBR_info_ptr];
         r0 = M[r0 + $aacdec.SBR_Nlow];
         r0 = r0 + 5;
         r0 = r0 * 4 (int);
         call $aacdec.frame_mem_pool_free;
      no_hf_reconstruction_right:
      //PROFILER_START(&$aacdec.profile_sbr_construct_x_matrix)
      call $aacdec.sbr_construct_x_matrix;
      //PROFILER_STOP(&$aacdec.profile_sbr_construct_x_matrix)
      // store main synthesis loop number in r8
      #ifdef AACDEC_SBR_HALF_SYNTHESIS
         r8 = 16;
         M[r9 + $aac.mem.SBR_in_synth_loops] = r8;
      #endif
      r8 = 0;
      jump_to_synth_channel_pair:
      r5 = M[r9 + $aac.mem.codec_struc];
      r0 = M[r5 + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD];
#ifdef BASE_REGISTER_MODE
      call $cbuffer.get_write_address_and_size_and_start_address;
      M[r9 + $aac.mem.TMP + (11*ADDR_PER_WORD)] = r2;
#else
      call $cbuffer.get_write_address_and_size;
#endif
      M[r9 + $aac.mem.TMP + (5*ADDR_PER_WORD)] = r0;
      M[r9 + $aac.mem.TMP + (6*ADDR_PER_WORD)] = r1;
      r7 = $aacdec.SBR_RIGHT_CH;
      PROFILER_START(&$aacdec.profile_sbr_synthesis_filterbank_combined)
      call $aacdec.sbr_synthesis_filterbank_combined;
      PROFILER_STOP(&$aacdec.profile_sbr_synthesis_filterbank_combined)
      // write back current output buffer location
      r5 = M[r9 + $aac.mem.codec_struc];
      r0 = M[r5 + $codec.DECODER_OUT_RIGHT_BUFFER_FIELD];
      r1 = I5;
      call $cbuffer.set_write_address;
      #ifdef AACDEC_SBR_HALF_SYNTHESIS
         // set in_synth flag to indicate whether half way through synthesis filterbank or not
         r1 = M[r9 + $aac.mem.SBR_in_synth];
         r1 = 1 - r1;
         M[r9 + $aac.mem.SBR_in_synth] = r1;
         if NZ jump $pop_rLink_and_rts;
         r5 = M[r9 + $aac.mem.codec_struc];
         r0 = M[r5 + $codec.DECODER_OUT_LEFT_BUFFER_FIELD];
#ifdef BASE_REGISTER_MODE
         call $cbuffer.get_read_address_and_size_and_start_address;
         push r2;
         pop B1;
#else
         call $cbuffer.get_read_address_and_size;
#endif
         I1 = r0;
         L1 = r1;
         r10 = 511;
         r0 = M[r9 + $aac.mem.SBR_synth_temp_ptr];
         I4 = r0;
         r0 = M[I4, MK1];
         do copy_audio_loop;
            M[I1, MK1] = r0,
             r1 = M[I4, MK1];
            M[I1, MK1] = r1,
             r0 = M[I4, MK1];
         copy_audio_loop:
         M[I1, MK1] = r0,
          r1 = M[I4, MK1];
         M[I1, MK1] = r1;
         r0 = M[r5 + $codec.DECODER_OUT_LEFT_BUFFER_FIELD];
         r1 = I1;
         call $cbuffer.set_write_address;
         L1 = 0;
      #endif
      // deallocate temp buffer for X_sbr_shared if finished synthesis
      r0 = $aacdec.X_SBR_SHARED_SIZE;
      call $aacdec.tmp_mem_pool_free;
      r5 = 1;  // ch1
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      Null = M[r0 + $aacdec.SBR_HF_reconstruction];
      if Z jump dont_save_prev_data_right;
         r4 = M[r9 + $aac.mem.SBR_info_ptr];
         call $aacdec.sbr_save_prev_data;
      dont_save_prev_data_right:
      //PROFILER_START(&$aacdec.profile_sbr_wrap_last_thfgen_envelopes)
      call $aacdec.sbr_wrap_last_thfgen_envelopes;
      //PROFILER_STOP(&$aacdec.profile_sbr_wrap_last_thfgen_envelopes)
      call $aacdec.sbr_swap_channels;
   sbr_not_present_right:

#endif


   frame_corrupt:
   // free all frame memory
   call $aacdec.frame_mem_pool_free_all;
   // free all tmp memory again
   call $aacdec.tmp_mem_pool_free_all;

   // pop rLink from stack
   jump $pop_rLink_and_rts;


   // little sub functions to save code space
   select_left_channel:
      r4 = r9 + $aac.mem.ICS_LEFT;
      M[r9 + $aac.mem.CURRENT_ICS_PTR] = r4;
      r0 =  M[r9 + $aac.mem.BUF_LEFT_PTR];
      M[r9 + $aac.mem.CURRENT_SPEC_PTR] = r0;
      r0 = r9 + $aac.mem.LEFT_SPEC_BLKSIGNDET;
      M[r9 + $aac.mem.CURRENT_SPEC_BLKSIGNDET_PTR] = r0;
      M[r9 + $aac.mem.CURRENT_CHANNEL] = Null;
   rts;

   select_right_channel:
      r4 = r9 + $aac.mem.ICS_RIGHT;
      M[r9 + $aac.mem.CURRENT_ICS_PTR] = r4;
      r0 = M[r9 + $aac.mem.BUF_RIGHT_PTR];
      M[r9 + $aac.mem.CURRENT_SPEC_PTR] = r0;
      r0 = r9 + $aac.mem.RIGHT_SPEC_BLKSIGNDET;
      M[r9 + $aac.mem.CURRENT_SPEC_BLKSIGNDET_PTR] = r0;
      r0 = 1;
      M[r9 + $aac.mem.CURRENT_CHANNEL] = r0;
   rts;

.ENDMODULE;

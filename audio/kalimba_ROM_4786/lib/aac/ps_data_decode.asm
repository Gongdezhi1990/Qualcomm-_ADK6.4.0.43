// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

#ifdef AACDEC_PS_ADDITIONS

// *****************************************************************************
// MODULE:
//    $aacdec.ps_data_decode
//
// DESCRIPTION:
//    -
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - toupdate
//
// *****************************************************************************
.MODULE $M.aacdec.ps_data_decode;
   .CODESEGMENT AACDEC_PS_DATA_DECODE_PM;
   .DATASEGMENT DM;

   $aacdec.ps_data_decode:

   // push rLink onto stack
   push rLink;

   r8 = M[r9 + $aac.mem.PS_info_ptr];
   Null = M[r8 + $aacdec.PS_NUM_ENV];
   if Z jump ps_num_env_eq_zero;
      M1 = $aacdec.PS_IID_NUM_QUANT_STEPS_FINE_RES;
      M3 = 2;
      // delta decode IID data then ICC data for all envelopes
      M2 = 1;
      iid_icc_delta_decode_outer_loop:
         Null = M2;
         // if(first iteration) initialise IID decoding
         if Z jump initialise_icc_delta_decode;
            // if(PS_IID_MODE < 3) coarse resolution IID delta decoding for all envelopes
            r6 = $aacdec.PS_IID_NUM_QUANT_STEPS_COARSE_RES;
            r5 = M[r8 + $aacdec.PS_IID_MODE];
            // else fine resolution IID delta decoding for all envelopes
            Null = r5 - 3;
            if GE r6 = M1;
            r7 = -r6;
            r3 = M[r8 + $aacdec.PS_NR_IID_PAR];
            r4 = M[r8 + $aacdec.PS_ENABLE_IID];
            // I4 <- PS_IID_INDEX[0][0]
            I4 = r8 + $aacdec.PS_IID_INDEX;
            // I1 <- PS_IID_INDEX_PREV : last envelope of the previous frame
            I1 = r9 + $aac.mem.PS_iid_index_prev;
            jump delta_decode_initialised;
         // elsif(second iteration) initialise ICC decoding
         initialise_icc_delta_decode:
            r6 = 7;  // maximum index limit
            r7 = 0;  // minimum index limit
            r3 = M[r8 + $aacdec.PS_NR_ICC_PAR];
            r4 = M[r8 + $aacdec.PS_ENABLE_ICC];
            r5 = M[r8 + $aacdec.PS_ICC_MODE];
            // I4 <- PS_ICC_INDEX[0][0]
            I4 = r8 + $aacdec.PS_ICC_INDEX;
            // I1 <- PS_ICC_INDEX_PREV : last envelope of the previous frame
            I1 = r9 + $aac.mem.PS_icc_index_prev;
         delta_decode_initialised:

         // M0 = PS_{IID||ICC}_STRIDE = 1
         // if((PS_{IID||ICC}_MODE == 0) || (PS_{IID||ICC}_MODE == 3))
         //    PS_{IID||ICC}_STRIDE = 2
         M0 = 1;
         Null = r5;
         if Z M0 = M3;
         Null = r5 - 3;
         if Z M0 = M3;

         // for envelope=0:PS_NUM_ENV-1,
         r2 = 0;  // r2 = envelope
         iid_icc_enabled_delta_decode_outer_loop:
            // r5 = PS_{IID||ICC}_CODING_DIRECTION[envelope]
            Words2Addr(r2);
            Null = M2;
            if Z jump read_icc_coding_direction;
               r5 = r8 + r2;
               r5 = M[r5 + $aacdec.PS_IID_CODING_DIRECTION];
               jump end_if_iid_or_icc_coding_direction;
            read_icc_coding_direction:
               r5 = r8 + r2;
               r5 = M[r5 + $aacdec.PS_ICC_CODING_DIRECTION];
            end_if_iid_or_icc_coding_direction:
            Addr2Words(r2);

            // I5 <- PS_{IID||ICC}_INDEX[envelope][]
            I5 = I4;

            // if(PS_ENABLE_{IID||ICC}) delta decode this envelope
            Null = r4;
            if Z jump ps_iid_or_icc_disabled;
               PROFILER_START(&$aacdec.profile_ps_delta_decode)
               call $aacdec.ps_delta_decode;
               PROFILER_STOP(&$aacdec.profile_ps_delta_decode)
               jump end_if_ps_iid_or_icc_enabled;
            // else clear PS_{IID||ICC}_INDEX[envelope][0:PS_NR_{IID||ICC}_PAR-1]
            ps_iid_or_icc_disabled:
               r10 = r3;
               r0 = 0;
               do iid_or_icc_disabled_clear_envelope_loop;
                  M[I5, MK1] = r0;
               iid_or_icc_disabled_clear_envelope_loop:
            end_if_ps_iid_or_icc_enabled:

            // if(PS_{IID||ICC}_STRIDE == 2) perform interpolation to fill in empty parameter band values
            Null = M0 - 2;
            if NZ jump end_if_disabled_iid_or_icc_stride_eq_two;
               I5 = I4 + ($aacdec.PS_MIN_NUM_PARAMETERS*2 - 1)*ADDR_PER_WORD;
               I1 = I4 + ($aacdec.PS_MIN_NUM_PARAMETERS - 1)*ADDR_PER_WORD;
               r10 = $aacdec.PS_MIN_NUM_PARAMETERS - 1;

               // PS_{IID||ICC}_INDEX[envelope][(PS_MIN_NUM_PARAMETERS-1):-1:1] = ...
                     // PS_{IID||ICC}_INDEX[envelope][floor(((PS_MIN_NUM_PARAMETERS-1):-1:1)/2)]
               r0 = M[I1,0];
               do iid_or_icc_disabled_downsample_freq_by_two;
                  M[I5, -MK1] = r0;
                  r0 = M[I1, -MK1];
                  M[I5, -MK1] = r0;
                  r0 = M[I1,0];
               iid_or_icc_disabled_downsample_freq_by_two:
               M[I5,0] = r0;
            end_if_disabled_iid_or_icc_stride_eq_two:

            // I1 <- PS_{IID||ICC}_INDEX[envelope][0]
            I1 = I4;
            // I4 <- PS_{IID||ICC}_INDEX[envelope+1][0]
            I4 = I4 + $aacdec.PS_MAX_NUM_PARAMETERS*ADDR_PER_WORD;

         r2 = r2 + 1;
         r0 = M[r8 + $aacdec.PS_NUM_ENV];
         Null = r2 - r0;
         if LT jump iid_icc_enabled_delta_decode_outer_loop;
      M2 = M2 - 1;
      if POS jump iid_icc_delta_decode_outer_loop;

      jump end_if_ps_num_env_eq_zero;

   // error case (PS_NUM_ENV == 0)
   ps_num_env_eq_zero:
      // force the number of envelopes to 1
      r0 = 1;
      M[r8 + $aacdec.PS_NUM_ENV] = r0;

      // delta decode IID data then ICC data for the single envelope
      r2 = 1;
      iid_or_icc_ps_num_env_eq_zero_outer_loop:
         if Z jump initialise_icc_first_envelope_loop;
            // I5 <- PS_IID_INDEX[0][0]
            I5 = r8 + $aacdec.PS_IID_INDEX;
            // I1 <- PS_IID_INDEX_PREV : last envelope of the previous frame
            I1 = r9 + $aac.mem.PS_iid_index_prev;
            r4 = M[r8 + $aacdec.PS_ENABLE_IID];
            jump end_if_init_iid_or_icc_first_envelope_loop;
         initialise_icc_first_envelope_loop:
            // I5 <- PS_ICC_INDEX[0][0]
            I5 = r8 + $aacdec.PS_ICC_INDEX;
            // I1 <- PS_ICC_INDEX_PREV : last envelope of the previous frame
            I1 = r9 + $aac.mem.PS_icc_index_prev;
            r4 = M[r8 + $aacdec.PS_ENABLE_ICC];
         end_if_init_iid_or_icc_first_envelope_loop:

         // if(PS_ENABLE_{IID||ICC})
                  // copy inter-channel parameter data from last envelope of previous frame into first envelope
                  // of current frame
         if Z jump enable_iid_or_icc_eq_zero_num_env_eq_zero;
            r10 = ($aacdec.PS_MAX_NUM_PARAMETERS - 1);
            r0 = M[I1, MK1];
            do copy_icc_index_prev_into_icc_index_loop;
               M[I5, MK1] = r0;
               r0 = M[I1, MK1];
            copy_icc_index_prev_into_icc_index_loop:
            M[I5, MK1] = r0;
            jump end_if_iid_or_icc_enable_num_env_eq_zero;
         enable_iid_or_icc_eq_zero_num_env_eq_zero:
            // clear PS_{IID||ICC}_INDEX[0][0:PS_MAX_NUM_PARAMETERS-1]
            r10 = ($aacdec.PS_MAX_NUM_PARAMETERS);
            r0 = 0;
            do clear_iid_or_icc_index_num_env_eq_zero;
               M[I5, MK1] = r0;
            clear_iid_or_icc_index_num_env_eq_zero:
         end_if_iid_or_icc_enable_num_env_eq_zero:
      r2 = r2 - 1;
      if POS jump iid_or_icc_ps_num_env_eq_zero_outer_loop;
   end_if_ps_num_env_eq_zero:


   // copy the last envelope of the current frame into PS_{IID||ICC}_INDEX_PREV[]
   // for use in the next frame
   I4 = r9 + $aac.mem.PS_iid_index_prev;
   I5 = r9 + $aac.mem.PS_icc_index_prev;

   r0 = M[r8 + $aacdec.PS_NUM_ENV];
   r0 = r0 * ($aacdec.PS_MAX_NUM_PARAMETERS*ADDR_PER_WORD) (int);
   // I1 <- PS_IID_INDEX[envelope=PS_NUM_ENV-1][0]
   r0 = r0 + r8;
   I1 = ($aacdec.PS_IID_INDEX - $aacdec.PS_MAX_NUM_PARAMETERS*ADDR_PER_WORD) + r0;
   // I2 <- PS_ICC_INDEX[envelope=PS_NUM_ENV-1][0]
   I2 = ($aacdec.PS_ICC_INDEX - $aacdec.PS_MAX_NUM_PARAMETERS*ADDR_PER_WORD) + r0;

   // for p=0:PS_MAX_NUM_PARAMETERS-1,
   r10 = ($aacdec.PS_MAX_NUM_PARAMETERS);
   r0 = M[I1, MK1];
   do save_last_envelope_loop;
      M[I4, MK1] = r0,
       r1 = M[I2, MK1];
      M[I5, MK1] = r1,
       r0 = M[I1, MK1];
   save_last_envelope_loop:

   // assign time-envelope borders
   r0 = M[r8 + $aacdec.PS_NUM_ENV];

   // PS_BORDER_POSITION[0] = 0
   M[r8 + $aacdec.PS_BORDER_POSITION] = Null;

   M0 = 16;
   // if(PS_FRAME_CLASS == 0)
   Null = M[r8 + $aacdec.PS_FRAME_CLASS];
   if NZ jump ps_frame_class_not_zero;
      // FIX_BORDERS : envelope borders are uniformly spaced across the frame
      // with either 4 or 2 envelope in the FIX_BORDERS frame class
      r1 = 8;
      Null = r0 - 2;
      if Z r1 = M0;
      r10 = r0;
      r2 = $aacdec.PS_NUM_SAMPLES_PER_FRAME;
      // for envelope=1:PS_NUM_ENV
      do uniformly_spaced_env_borders_loop;
         r0 = r10 * ADDR_PER_WORD (int);
         r0 = r0 + r8;
         M[$aacdec.PS_BORDER_POSITION + r0] = r2;
         r2 = r2 - r1;
      uniformly_spaced_env_borders_loop:
      jump end_if_ps_frame_class_eq_zero;
   ps_frame_class_not_zero:
      // VAR_BORDERS : envelope borders positions are non-uniformly spaced across the frame
      // r1 = PS_BORDER_POSITION[PS_NUM_ENV]
      Words2Addr(r0);
      r1 = r8 + r0;
      r1 = M[$aacdec.PS_BORDER_POSITION + r1];
      Addr2Words(r0);
      // if(PS_BORDER_POSITIONS[PS_NUM_ENV] < 32)
            // append an extra envelope to the end of the frame
      Null = r1 - $aacdec.PS_NUM_SAMPLES_PER_FRAME;
      if POS jump end_if_need_to_add_extra_envelope;
         // use the parametric data of the last envelope to fill in the
         // samples remaining upto to the end of the frame

         // PS_NUM_ENV += 1
         r0 = r0 + 1;
         M[$aacdec.PS_NUM_ENV] = r0;
         r1 = $aacdec.PS_NUM_SAMPLES_PER_FRAME;
         Words2Addr(r0);
         r2 = r8 + r0;
         M[$aacdec.PS_BORDER_POSITION + r2] = r1;
         Addr2Words(r0);

         r10 = $aacdec.PS_MAX_NUM_PARAMETERS;
         r1 = r0 * ($aacdec.PS_MAX_NUM_PARAMETERS*ADDR_PER_WORD) (int);
         r1 = r1 + r8;
         I4 = ($aacdec.PS_IID_INDEX) + r1;
         I5 = ($aacdec.PS_ICC_INDEX) + r1;
         I1 = ($aacdec.PS_IID_INDEX - $aacdec.PS_MAX_NUM_PARAMETERS*ADDR_PER_WORD) + r1;
         I2 = ($aacdec.PS_ICC_INDEX - $aacdec.PS_MAX_NUM_PARAMETERS*ADDR_PER_WORD) + r1;

         // copy the inter-channel IID and ICC parameter data of the last existing
         // envelope into the newly created last envelope
         // for bin=0:33
         do extrapolate_last_envelope_parameters_loop;
            r1 = M[I1, MK1];
            M[I4, MK1] = r1,
             r2 = M[I2, MK1];
            M[I5, MK1] = r2;
         extrapolate_last_envelope_parameters_loop:
      end_if_need_to_add_extra_envelope:

      // for envelope=1:PS_NUM_ENV-1,
      r10 = r0 - 1;
      // I1 <- PS_BORDER_POSITION[envelope=1]
      I1 = r8 + ($aacdec.PS_BORDER_POSITION + 1*ADDR_PER_WORD);

      do upper_limit_ps_border_position_loop;
         // upper_threshold = PS_NUM_SAMPLES_PER_FRAME - (PS_NUM_ENV - envelope)
         r1 = r10 - $aacdec.PS_NUM_SAMPLES_PER_FRAME;
         r2 = M[I1,0];
         // PS_BORDER_POSITION[envelope] = min(PS_BORDER_POSITION[envelope], upper_threshold)
         Null = r2 + r1;
         if GT r2 = -r1;
         M[I1, MK1] = r2;
      upper_limit_ps_border_position_loop:

      // for envelope=1:PS_NUM_ENV-1,
      r10 = r0 - 1;
      // I1 <- PS_BORDER_POSITION[envelope=0]
      I1 = r8 + $aacdec.PS_BORDER_POSITION;
      r2 = 1;

      do lower_limit_ps_border_position_loop;
         // lower_threshold = PS_BORDER_POSITION[envelope-1]+1
         r1 = M[I1, MK1];
         r1 = r1 + r2,
          r3 = M[I1,0];
         // PS_BORDER_POSITION[envelope] = max(PS_BORDER_POSITION[envelope], lower_threshold)
         Null = r1 - r3;
         if GT r3 = r1;
         M[I1,0] = r3;
      lower_limit_ps_border_position_loop:
   end_if_ps_frame_class_eq_zero:

   r0 = M[r8 + $aacdec.PS_IID_MODE];
   M0 = r0;
   r0 = M[r8 + $aacdec.PS_ICC_MODE];
   M1 = r0;

   // for envelope=0:PS_NUM_ENV-1,
   r10 = M[r8 + $aacdec.PS_NUM_ENV];
   // r4 = envelope
   r4 = 0;
   do map_34_parameters_to_20_loop;
      // if(PS_IID_MODE == 2)
      Null = M0 - 2;
      if NZ jump end_if_map_34_iid_parameters_to_20;
         r0 = r8 + $aacdec.PS_IID_INDEX;
         call $aacdec.ps_map_34_parameters_to_20;
      end_if_map_34_iid_parameters_to_20:
      // if(PS_ICC_MODE == 2)
      Null = M1 - 2;
      if NZ jump end_if_map_34_icc_parameters_to_20;
         r0 = r8 + $aacdec.PS_ICC_INDEX;
         call $aacdec.ps_map_34_parameters_to_20;
      end_if_map_34_icc_parameters_to_20:

      r4 = r4 + 1;
   map_34_parameters_to_20_loop:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

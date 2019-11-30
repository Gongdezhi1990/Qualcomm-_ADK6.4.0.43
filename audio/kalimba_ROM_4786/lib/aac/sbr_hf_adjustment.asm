// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

#ifdef AACDEC_SBR_ADDITIONS

// *****************************************************************************
// MODULE:
//    $aacdec.sbr_hf_adjustment
//
// DESCRIPTION:
//    Adjusts the new high frequencies that have been added by hf_generation
//
// INPUTS:
//    - r5 current channel (0/1)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r8, r10, rMAC, I0-I7, L0, L4, L5, M0-M3, Div
//    - $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_hf_adjustment;
   .CODESEGMENT AACDEC_SBR_HF_ADJUSTMENT_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_hf_adjustment:

   // push rLink onto stack
   push rLink;
   
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.SBR_HF_ADJUSTMENT_ASM.SBR_HF_ADJUSTMENT.SBR_HF_ADJUSTMENT.PATCH_ID_0, r1)
#endif   

   Words2Addr(r5);
   
   r1 = -1;
   // if(SBR_bs_frame_class[ch] == SBR_FIXFIX)
   //    SBR_l_A[ch] = -1
   r0 = r5 + r9;
   r0 = M[$aac.mem.SBR_bs_frame_class + r0];
   Null = r0 - $aacdec.SBR_FIXFIX;
   if Z jump sine_gen_starting_env_assign;



#ifdef AACDEC_ELD_ADDITIONS
   r2 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r2 - $aacdec.ER_AAC_ELD;
   if NZ jump non_eld_version;

      // if(SBR_bs_frame_class[ch] == SBR_LDTRAN)
      r0 = r5 + r9;
      r0 = M[$aac.mem.SBR_bs_transient_position + r0];
      r1 = r0 * (4*ADDR_PER_WORD) (int);
      r1 = r1 + (3*ADDR_PER_WORD);            // [bs_transient_position][transientldx]  //transientldx position =  4 *bs_transient_position + 3 from table ld_envelopetable512

      r2 = &$aacdec.ld_envelopetable480;
      r0 = &$aacdec.ld_envelopetable512;
      Null = M[r9 + $aac.mem.ELD_frame_length_flag];
      if NZ r0 = r2;

      r0 = r0 + r1;
      call $mem.ext_window_access_as_ram;
      r1 = M[r0];

      jump sine_gen_starting_env_assign;

   non_eld_version:
#endif


      // elsif(SBR_bs_frame_class[ch] == SBR_VARFIX)
      Null = r0 - $aacdec.SBR_VARFIX;
      if NZ jump not_frame_class_varfix;
         // if(SBR_bs_pointer[ch] > 1)
         //    SBR_l_A[ch] = -1
         // else
         //    SBR_l_A[ch] = SBR_bs_pointer[ch] - 1
         r0 = r5 + r9;
         r0 = M[$aac.mem.SBR_bs_pointer + r0];
         Null = r0 - 1;
         if GT jump sine_gen_starting_env_assign;
            r1 = r0 - 1;
            jump sine_gen_starting_env_assign;

      // else
      not_frame_class_varfix:
         // if(SBR_bs_pointer[ch] == 0)
         //    SBR_l_A[ch] = -1
         // else
         //    SBR_l_A[ch] = SBR_bs_num_env[ch] + 1 - SBR_bs_pointer[ch]
         r0 = r5 + r9;
         Null = M[$aac.mem.SBR_bs_pointer + r0];
         if Z jump sine_gen_starting_env_assign;
            r0 = r5 + r9;
            r0 = M[$aac.mem.SBR_bs_num_env + r0];
            r1 = r5 + r9;
            r1 = M[$aac.mem.SBR_bs_pointer + r1];
            r0 = r0 + 1;
            r1 = r0 - r1;

   sine_gen_starting_env_assign:
   // SBR_l_A[ch] = 'value set above in r1'
   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r0 = r0 + r5;
   M[r0 + $aacdec.SBR_l_A] = r1;

   Addr2Words(r5);

   PROFILER_START(&$aacdec.profile_sbr_estimate_current_envelope)
   call $aacdec.sbr_estimate_current_envelope;
   PROFILER_STOP(&$aacdec.profile_sbr_estimate_current_envelope)
   Null = M[r9 + $aac.mem.FRAME_CORRUPT];
   if NZ jump frame_corrupt;

   PROFILER_START(&$aacdec.profile_sbr_calculate_gain)
   call $aacdec.sbr_calculate_gain;
   PROFILER_STOP(&$aacdec.profile_sbr_calculate_gain)
   Null = M[r9 + $aac.mem.FRAME_CORRUPT];
   if NZ jump frame_corrupt;

   PROFILER_START(&$aacdec.profile_sbr_hf_assembly)
   call $aacdec.sbr_hf_assembly;
   PROFILER_STOP(&$aacdec.profile_sbr_hf_assembly)

   frame_corrupt:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

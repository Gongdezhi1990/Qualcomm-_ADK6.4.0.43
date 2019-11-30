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
//    $aacdec.sbr_envelope_time_border_vector
//
// DESCRIPTION:
//    Calculate time borders for each segment
//
// INPUTS:
//    - r5 current channel (0/1)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3, r6, r7, r10
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_envelope_time_border_vector;
   .CODESEGMENT AACDEC_SBR_ENVELOPE_TIME_BORDER_VECTOR_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_envelope_time_border_vector:

   Words2Addr(r5); // In all code below r5 is a channel address offset

   r2 = $aacdec.SBR_RATE;
#ifdef AACDEC_ELD_ADDITIONS
   r1 = $aacdec.SBR_RATE_eld;
   r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r0 - $aacdec.ER_AAC_ELD;
   if Z r2 = r1;
#endif


   // SBR_t_E[ch][0] = SBR_RATE * SBR_abs_bord_lead[ch]
   // r6 = SBR_abs_bord_lead[ch]
   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r0 = r0 + r5;
   r6 = M[r0 + $aacdec.SBR_abs_bord_lead];
   r1 = r6 * r2 (int);
   r0 = r5 * 6 (int);
   r3 = r0 + r9;
   M[$aac.mem.SBR_t_E + r3] = r1;

   // SBR_t_E[ch][SBR_bs_num_env[ch]] = SBR_RATE * SBR_abs_bord_trail[ch]
   // r3 = SBR_bs_num_env[ch]
   r3 = r5 + r9;
   r3 = M[$aac.mem.SBR_bs_num_env + r3];
   // r7 = SBR_abs_bord_trail[ch]
   r7 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r7 = r7 + r5;
   r7 = M[r7 + $aacdec.SBR_abs_bord_trail];
   r1 = r7 * r2 (int);
   Words2Addr(r3);
   r0 = r0 + r3;
   r0 = r0 + r9;
   M[$aac.mem.SBR_t_E + r0] = r1;
   Addr2Words(r3);


   // switch(SBR_bs_frame_class[ch])
   r0 = r5 + r9;
   r0 = M[$aac.mem.SBR_bs_frame_class + r0];

   // case SBR_FIXFIX
   Null = r0 - $aacdec.SBR_FIXFIX;
   if NZ jump not_case_fixfix;
      // switch(SBR_bs_num_env[ch]
      // case 4
      r1 = $aacdec.SBR_RATE * ($aacdec.SBR_numTimeSlots / 4);
#ifdef AACDEC_ELD_ADDITIONS
      r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
      Null = r0 - $aacdec.ER_AAC_ELD;
      if NZ jump case4_no_eld;
         r6 = M[r9 + $aac.mem.ELD_sbr_numTimeSlots];
         r1 = r6 * 0.25;///to be rounded up in a similar way as the ISO reference code for odd value of SBR_numTimeSlots
         r1 = $aacdec.SBR_RATE_eld * r1(int);
      case4_no_eld:
#endif

      r0 = r5 * 6 (int);
      Null = r3 - 4;
      if NZ jump fixfix_not_case_4;

         r0 = r0 + r9;
         M[$aac.mem.SBR_t_E + (1*ADDR_PER_WORD) + r0] = r1;
         r0 = r0 + (1*ADDR_PER_WORD);
         r2 = r1 * 2 (int);
         M[$aac.mem.SBR_t_E + (1*ADDR_PER_WORD) + r0] = r2;
         r0 = r0 + (1*ADDR_PER_WORD);
         r2 = r1 * 3(int);
         M[$aac.mem.SBR_t_E + (1*ADDR_PER_WORD) + r0] = r2;
         jump exit;

      fixfix_not_case_4:
      // case 2
      Null = r3 - 2;
      if NZ jump exit;

         r1 = $aacdec.SBR_RATE * ($aacdec.SBR_numTimeSlots / 2);
#ifdef AACDEC_ELD_ADDITIONS
         r3 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
         Null = r3 - $aacdec.ER_AAC_ELD;
         if NZ jump not_case4_no_eld;
            r1 = r6 * 0.5 (frac);
            r1 = $aacdec.SBR_RATE_eld * r1 (int);
         not_case4_no_eld:
#endif

         r0 = r0 + r9;
         M[$aac.mem.SBR_t_E + (1*ADDR_PER_WORD) + r0] = r1;
         jump exit;

   // case SBR_FIXVAR
   not_case_fixfix:

#ifdef AACDEC_ELD_ADDITIONS
   r2 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r2 - $aacdec.ER_AAC_ELD;
   if NZ jump non_eld_version;

      // *** ELD MODE ***
      // ****************

      r0 = r5 * 6 (int);
      // tE(0) = 0;
      r6 = 0;
      r0 = r0 + r9;
      M[$aac.mem.SBR_t_E + r0] = r6;

      // load L_E(bs_num_env)
      r3 = r5 + r9;
      r3 = M[$aac.mem.SBR_bs_num_env + r3];
      // tE(L_E) = numberTimeSlots;
      r0 = r5 * 6 (int);
      Words2Addr(r3);
      r0 = r0 + r3;
      Addr2Words(r3);
      r6 = M[r9 + $aac.mem.ELD_sbr_numTimeSlots];
      r0 = r0 + r9;
      M[$aac.mem.SBR_t_E + r0] = r6;

      // load bs_transient_position
      r1 = r5 + r9;
      r1 = M[$aac.mem.SBR_bs_transient_position + r1];
      r1 = r1 * (4*ADDR_PER_WORD) (int);    // calculate the row

      // load the envelope table and set the pointer to the correct row
      r2 = &$aacdec.ld_envelopetable480;
      r0 = &$aacdec.ld_envelopetable512;
      Null = M[r9 + $aac.mem.ELD_frame_length_flag];
      if NZ r0 = r2;

      r0 = r0 + r1;
      call $mem.ext_window_access_as_ram;
      I2 = r0 + MK1;                              // indexing into the correct [row][column]

      // tE(l) = LD EnvelopeTable[bs_transient_position][l+1] for 0 < l < L_E
      r10 = r3 - 1;
      I1 = r9 + $aac.mem.SBR_t_E;
      r0 = r5 * 6 (int);
      r0 = r0 + (1*ADDR_PER_WORD);
      I1 = I1 + r0;
      do set_te_loop;
         r3 = M[I2, MK1];   // LD EnvelopeTable[bs_transient_position][l+1]
         M[I1, MK1] = r3;   // tE(l) = LD EnvelopeTable[bs_transient_position][l+1]
      set_te_loop:

      jump exit;
#endif

   non_eld_version:
      // *** NON ELD MODE ***
      // ********************

      Null = r0 - $aacdec.SBR_FIXVAR;
      if NZ jump not_case_fixvar;
         // if(SBR_bs_num_env[ch] > 1)
         r10 = r3 - 1;
         if LE jump exit;
            // for l=0:SBR_bs_num_env[ch]-2,

            r0 = r5 * 5 (int);
            r2 = r5 * 6 (int);
            Words2Addr(r10);
            r2 = r2 + r10;
            Addr2Words(r10);
            r2 = r2 + r9;

            do fixvar_bs_rel_border_loop;
               // r1 = SBR_bs_rel_bord[ch][l]
               r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
               r1 = r1 + r0;
               r1 = M[r1 + $aacdec.SBR_bs_rel_bord];
               // SBR_bs_rel_bord[ch][l] - SBR_abs_bord_trail[ch]
               Null = r1 - r7;
               if GT jump exit;

               // border -= SBR_bs_rel_bord[ch][l]
               r7 = r7 - r1;
               // SBR_t_E[ch][i] = SBR_RATE * border
               r1 = r7 * $aacdec.SBR_RATE (int);
               M[$aac.mem.SBR_t_E + r2] = r1;
               // i -= 1
               r2 = r2 - MK1;
               r0 = r0 + MK1;
            fixvar_bs_rel_border_loop:
            jump exit;

      // case SBR_VARFIX
      not_case_fixvar:
      Null = r0 - $aacdec.SBR_VARFIX;
      if NZ jump not_case_varfix;
         // if(SBR_bs_num_env[ch] > 1)
         r10 = r3 - 1;
         if LE jump exit;
            // for l=0:SBR_bs_num_env[ch]-2,

            r0 = r5 * 5 (int);
            r2 = r5 * 6 (int);
            r2 = r2 + r9;

            do varfix_bs_rel_border_loop;
               // r1 = SBR_bs_rel_bord[ch][l]
               r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
               r1 = r1 + r0;
               r1 = M[r1 + $aacdec.SBR_bs_rel_bord];
               // border += SBR_bs_rel_bord[ch][l]
               r6 = r6 + r1;
               r1 = r6 * $aacdec.SBR_RATE (int);
               Null = r1 - ($aacdec.SBR_numTimeSlotsRate + $aacdec.SBR_tHFGen - $aacdec.SBR_tHFAdj);
               if GT jump exit;

               // SBR_t_E[ch][l+1] = SBR_RATE * border;
               M[$aac.mem.SBR_t_E + (1*ADDR_PER_WORD) + r2] = r1;
               r2 = r2 + MK1;
               r0 = r0 + MK1;
            varfix_bs_rel_border_loop:
            jump exit;

      // case SBR_VARVAR
      not_case_varfix:
      Null = r0 - $aacdec.SBR_VARVAR;
      if NZ jump exit;

         // if(SBR_bs_num_rel_0[ch]==1)
         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r0 = r0 + r5;
         r10 = M[r0 + $aacdec.SBR_bs_num_rel_0];

         r0 = r5 * 5 (int);
         r2 = r5 * 6 (int);
         r2 = r2 + r9;

         // for l=0:SBR_bs_num_rel_0[ch]-1,
         do varvar_bs_num_rel_0_loop;
            r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
            r1 = r1 + r0;
            r1 = M[r1 + $aacdec.SBR_bs_rel_bord_0];
            // border += SBR_bs_rel_bord_0[ch][l]
            r6 = r6 + r1;

            // if((SBR_RATE*border+SBR_tHFAdj) > (SBR_numTimeSlotsRate+SBR_tHFGen))
            r1 = r6 * $aacdec.SBR_RATE (int);
            Null = r1 - ($aacdec.SBR_numTimeSlotsRate + $aacdec.SBR_tHFGen - $aacdec.SBR_tHFAdj);
            if GT jump do_rel_1;

            // SBR_t_E[ch][l+1] = SBR_RATE * border
            M[$aac.mem.SBR_t_E + (1*ADDR_PER_WORD) + r2] = r1;
            r2 = r2 + MK1;
            r0 = r0 + MK1;
         varvar_bs_num_rel_0_loop:


         do_rel_1:
         // if(SBR_bs_num_rel_1[ch]==1)
         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r0 = r0 + r5;
         r10 = M[r0 + $aacdec.SBR_bs_num_rel_1];

         r0 = r5 * 5 (int);
         r2 = r5 * 6 (int);
         // r3 = SBR_bs_num_env[ch]
         Words2Addr(r3);
         r2 = r2 + r3;
         r2 = r2 + r9;

         // for l=0:SBR_bs_num_rel_1[ch]
         do varvar_bs_num_rel_1_loop;
            // if(border < SBR_bs_rel_bord_1[ch][l]
            r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
            r1 = r1 + r0;
            r1 = M[r1 + $aacdec.SBR_bs_rel_bord_1];
            Null = r7 - r1;
            if LT jump exit;

            // border -= SBR_bs_rel_bord_1[ch][l]
            r7 = r7 - r1;
            // SBR_t_E[ch][i] = SBR_RATE * border
            r1 = r7 * $aacdec.SBR_RATE (int);
            M[$aac.mem.SBR_t_E - (1*ADDR_PER_WORD) + r2] = r1;
            // i -= 1
            r2 = r2 - MK1;
            r0 = r0 + MK1;
         varvar_bs_num_rel_1_loop:

   exit:
   Addr2Words(r5); // In all code above r5 was a channel address offset (convert back)
   rts;

.ENDMODULE;

#endif  //AACDEC_SBR_ADDITIONS

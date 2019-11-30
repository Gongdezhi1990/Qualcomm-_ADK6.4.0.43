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
//    $aacdec.sbr_extract_noise_floor_data
//
// DESCRIPTION:
//    Calculate noise floor information
//
// INPUTS:
//    - r5 current channel (0/1)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0, r1, r3, r4, r6, r7, r10, I1, I2, I4
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_extract_noise_floor_data;
   .CODESEGMENT AACDEC_SBR_EXTRACT_NOISE_FLOOR_DATA_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_extract_noise_floor_data:

   // push rLink onto stack
   push rLink;

   Words2Addr(r5);

   // delta = 1;
   r6 = 1;
   Null = r5;
   if Z jump delta_assigned;
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      r0 = M[r0 + $aacdec.SBR_bs_coupling]; // delta = 2
      r6 = r6 + r0;
   delta_assigned:


   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r0 = r0 + r5;
   r7 = M[r0 + $aacdec.SBR_bs_num_noise];
   Words2Addr(r7);
   r0 = M[r0 + $aacdec.SBR_Q_envelope_base_ptr];
   I2 = r0;
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r4 = M[r0 + $aacdec.SBR_Nq];


   r3 = 0;
   // for l=0:SBR_bs_num_noise[ch]
   outer_loop:

      // if(SBR_bs_df_noise[ch][l] == 0)
      r0 = r5 * 2 (int);
      r0 = r0 + r3;
      r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r1 = r1 + r0;
      Null = M[r1 + $aacdec.SBR_bs_df_noise];
      if NZ jump not_delta_frequency_noise_coding;
         r0 = r3 * 5 (int);
         I1 = I2 + r0;
         I4 = I1;

         // SBR_Q_envelope[ch][0][l] = delta * SBR_Q_envelope[ch][0][l]
         r0 = M[I4, 0];
         r0 = r0 * r6 (int);
         M[I1, MK1] = r0;

         r10 = r4 - 1;

         do q_envelope_delta_freq_coding_loop;
            r0 = M[I1, 0],
             r1 = M[I4, MK1];

            r0 = r0 * r6 (int);
            r1 = r1 + r0;

            M[I1, MK1] = r1;
         q_envelope_delta_freq_coding_loop:

         jump eval_loop_index;
      // else
      not_delta_frequency_noise_coding:

      // if(l == 0)
      Null = r3;
      if NZ jump not_first_envelope;
         // for k=0:SBR_Nq-1,
         r10 = r4;

         r0 = r5 * 5 (int);
         r1 = M[r9 + $aac.mem.SBR_info_ptr];
         r1 = r1 + r0;
         I4 = r1 + $aacdec.SBR_Q_prev;

         I1 = I2;

         do q_env_d_time_coding_first_env_loop;
            r0 = M[I1, 0],
             r1 = M[I4, MK1];

            r0 = r0 * r6 (int);
            r1 = r1 + r0;

            M[I1, MK1] = r1;
         q_env_d_time_coding_first_env_loop:

         jump eval_loop_index;
      not_first_envelope:
         // for k=0:SBR_Nq-1,
         r10 = r4;

         r0 = r3 * 5 (int);
         I1 = I2 + r0;
         I4 = I1 - (5*ADDR_PER_WORD);

         do q_env_d_time_coding_not_first_env_loop;
            r0 = M[I1, 0],
             r1 = M[I4, MK1];

            r0 = r0 * r6 (int);
            r1 = r1 + r0;

            M[I1, MK1] = r1;
         q_env_d_time_coding_not_first_env_loop:

   eval_loop_index:
   r3 = r3 + (1*ADDR_PER_WORD);
   Null = r7 - r3;
   if GT jump outer_loop;



   // SBR_Q_prev[ch][0:4] = SBR_Q_envelope[ch][0:4][SBR_bs_num_noise[ch]-1]
   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r0 = r0 + r5;
   r0 = M[r0 + $aacdec.SBR_Q_envelope_base_ptr];
   r7 = r7 - (1*ADDR_PER_WORD);
   r1 = r7 * $aacdec.SBR_Nq_max (int);
   I4 = r0 + r1;

   r0 = r5 * $aacdec.SBR_Nq_max (int);
   r1 = M[r9 + $aac.mem.SBR_info_ptr];
   r1 = r1 + r0;
   I1 = r1 + $aacdec.SBR_Q_prev;

   r10 = ($aacdec.SBR_Nq_max-1);

   r0 = M[I4, MK1];

   do q_prev_loop;
      M[I1, MK1] = r0,
       r0 = M[I4, MK1];
   q_prev_loop:

   M[I1, MK1] = r0;

   Addr2Words(r5);

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

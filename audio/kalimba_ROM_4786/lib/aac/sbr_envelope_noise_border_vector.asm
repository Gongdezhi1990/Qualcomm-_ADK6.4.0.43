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
//    $aacdec.sbr_envelope_noise_border_vector
//
// DESCRIPTION:
//    Calculate noise borders for each segment
//
// INPUTS:
//    - r5 current channel (0/1)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_envelope_noise_border_vector;
   .CODESEGMENT AACDEC_SBR_ENVELOPE_NOISE_BORDER_VECTOR_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_envelope_noise_border_vector:

   // push rLink onto stack
   push rLink;

   Words2Addr(r5);
   // SBR_t_Q[ch][0] = SBR_t_E[ch][0]
   r0 = r5 * 6 (int);
   r2 = r0 + r9;
   r2 = M[$aac.mem.SBR_t_E + r2];

   r1 = r5 * 3 (int);
   r1 = r1 + r9;
   M[$aac.mem.SBR_t_Q + r1] = r2;

   r2 = r5 + r9;
   r2 = M[$aac.mem.SBR_bs_num_env + r2];
   Addr2Words(r5);

   Null = r2 - 1;
   if NZ jump bs_num_env_ne_1;
      // SBR_t_Q[ch][1] = SBR_t_E[ch][1]
      r2 = r0 + r9;
      r2 = M[$aac.mem.SBR_t_E + (1*ADDR_PER_WORD) + r2];
      M[$aac.mem.SBR_t_Q + (1*ADDR_PER_WORD) + r1] = r2;

      // SBR_t_Q[ch][2] = 0
      M[$aac.mem.SBR_t_Q + (2*ADDR_PER_WORD) + r1] = Null;

      // pop rLink from stack
      jump $pop_rLink_and_rts;

   bs_num_env_ne_1:
   Null = r2 - 1;
   if LE jump $pop_rLink_and_rts;
      call $aacdec.sbr_middle_border;
      // SBR_t_Q[ch][1] = SBR_t_E[ch][SBR_middle_border_index[ch]]
      Words2Addr(r3);
      r3 = r3 + r0;
      r3 = r3 + r9;
      r3 = M[$aac.mem.SBR_t_E + r3];
      M[$aac.mem.SBR_t_Q + (1*ADDR_PER_WORD) + r1] = r3;
      // SBR_t_Q[ch][2] = SBR_t_E[ch][SBR_bs_num_env[ch]]
      Words2Addr(r2);
      r2 = r2 + r0;
      r2 = r2 + r9;
      r2 = M[$aac.mem.SBR_t_E + r2];
      M[$aac.mem.SBR_t_Q + (2*ADDR_PER_WORD) + r1] = r2;

      // pop rLink from stack
      jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

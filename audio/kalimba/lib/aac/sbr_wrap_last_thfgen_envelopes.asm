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
//    $aacdec.sbr_wrap_last_thfgen_envelopes
//
// DESCRIPTION:
//    Wraps the last tHFGen envelopes in X_sbr to the front of X_sbr for the
//    next frame. The first tHFAdj of these envelopes have only the lower half
//    of the frequencies wrapped.
//
// INPUTS:
//    - r5 channel
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0, r1, r10, I1, I2, I4, I5
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_wrap_last_thfgen_envelopes;
   .CODESEGMENT AACDEC_SBR_WRAP_LAST_THFGEN_ENVELOPES_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_wrap_last_thfgen_envelopes:

   // push rLink onto stack
   push rLink;

#ifdef AACDEC_ELD_ADDITIONS
   r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r0 - $aacdec.ER_AAC_ELD;
   if Z jump eld_version;
#endif

      // *** NON ELD MODE ***
      // ********************

      r0 = $aacdec.X_SBR_WIDTH * $aacdec.SBR_numTimeSlotsRate;
      Words2Addr(r0);
      // I2 <- real(X_sbr[ch][0][SBR_numTimeSlotsRate])
      r1 = M[r9 + $aac.mem.SBR_X_2env_real_ptr];
      I2 = r1 + r0;

      // I5 <- imag(X_sbr[ch][0][SBR_numTimeSlotsRate])
      r1 = M[r9 + $aac.mem.SBR_X_2env_imag_ptr];
      I5 = r1 + r0;

      //
      // X_sbr[ch][0:31][0] = X_sbr[ch][0:31][SBR_numTimeSlotsRate]
      //
      r10 = $aacdec.X_SBR_LEFTRIGHT_2ENV_SIZE/2;
      r0 = M[r9 + $aac.mem.SBR_X_2env_real_ptr];
      I1 = r0;

      r0 = M[r9 + $aac.mem.SBR_X_2env_imag_ptr];
      I4 = r0;

      do x_sbr_wrap_last_thf_gen_env_loop2a;
         r0 = M[I2, MK1],
          r1 = M[I5, MK1];
         M[I1, MK1] = r0,
          M[I4, MK1] = r1;
      x_sbr_wrap_last_thf_gen_env_loop2a:

      //
      // X_sbr[ch][0:31][1] = X_sbr[ch][0:31][SBR_numTimeSlotsRate+1]
      //
      r10 = $aacdec.X_SBR_LEFTRIGHT_2ENV_SIZE/2;
      I1 = I1 + ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD/2);
      I2 = I2 + ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD/2);
      I4 = I4 + ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD/2);
      I5 = I5 + ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD/2);


      do x_sbr_wrap_last_thf_gen_env_loop2b;
         r0 = M[I2, MK1],
          r1 = M[I5, MK1];
         M[I1, MK1] = r0,
          M[I4, MK1] = r1;
      x_sbr_wrap_last_thf_gen_env_loop2b:

      //
      // X_sbr[ch][0:63][SBR_tHFAdj:SBR_tHFGen-1] = X_sbr[ch][0:63][SBR_numTimeSlotsRate+SBR_tHFAdj:39]
      //
      r10 = $aacdec.X_SBR_LEFTRIGHT_SIZE;
      I1 = I1 + ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD/2);
      I2 = I2 + ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD/2);
      I4 = I4 + ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD/2);
      I5 = I5 + ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD/2);
      do x_sbr_wrap_last_thf_gen_env_loop;
         r0 = M[I2, MK1],
          r1 = M[I5, MK1];
         M[I1, MK1] = r0,
          M[I4, MK1] = r1;
      x_sbr_wrap_last_thf_gen_env_loop:

      // pop rLink from stack
      jump $pop_rLink_and_rts;



#ifdef AACDEC_ELD_ADDITIONS
   eld_version:
      // *** ELD MODE ***
      // ****************

      r1 = M[r9 + $aac.mem.SBR_numTimeSlotsRate];
      r0 = $aacdec.X_SBR_WIDTH;
      r0 = r0 * r1 (int);
      Words2Addr(r0);
      // I2 <- real(X_sbr[ch][0][SBR_numTimeSlotsRate])
      r1 = M[r9 + $aac.mem.SBR_X_2env_real_ptr];
      I2 = r1 + r0;

      // I5 <- imag(X_sbr[ch][0][SBR_numTimeSlotsRate])
      r1 = M[r9 + $aac.mem.SBR_X_2env_imag_ptr];
      I5 = r1 + r0;

      //
      // X_sbr[ch][0:31][0] = X_sbr[ch][0:31][SBR_numTimeSlotsRate]
      //
      r10 = $aacdec.X_SBR_LEFTRIGHT_2ENV_SIZE;
      r10 = r10 * 2 (int);
      r0 = M[r9 + $aac.mem.SBR_X_2env_real_ptr];
      I1 = r0;

      r0 = M[r9 + $aac.mem.SBR_X_2env_imag_ptr];
      I4 = r0;

      do x_sbr_wrap_last_thf_gen_env_loop2a_eld;
         r0 = M[I2, MK1],
          r1 = M[I5, MK1];
         M[I1, MK1] = r0,
          M[I4, MK1] = r1;
      x_sbr_wrap_last_thf_gen_env_loop2a_eld:

      // pop rLink from stack
      jump $pop_rLink_and_rts;

#endif //AACDEC_ELD_ADDITIONS

.ENDMODULE;

#endif

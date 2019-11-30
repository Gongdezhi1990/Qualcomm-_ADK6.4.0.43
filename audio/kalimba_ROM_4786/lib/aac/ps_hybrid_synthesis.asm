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
//    $aacdec.ps_hybrid_synthesis
//
// DESCRIPTION:
//    -
//
// INPUTS:
//    - r4 number of time samples to synthesise {16¦¦32}
//    - r5 ch
//    - M2 flag = 0 when doing 1st half of synthesis (samples [0:15]); = 1 when 2nd half (samples [16:31])
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - toupdate
//
// *****************************************************************************
.MODULE $M.aacdec.ps_hybrid_synthesis;
   .CODESEGMENT AACDEC_PS_HYBRID_SYNTHESIS_PM;
   .DATASEGMENT DM;

   $aacdec.ps_hybrid_synthesis:

   Words2Addr(r5);
   M0 = $aacdec.X_SBR_WIDTH*ADDR_PER_WORD;

   // I0 -> real(X_sbr[ch][k=0][l=SBR_tHFAdj]) = real({S¦D}_k[n=0][k=0])
   r0 = M[r9 + $aac.mem.SBR_X_curr_real_ptr];
   I0 = r0;
   // I4 -> imag(X_sbr[ch][k=0][l=SBR_tHFAdj]) = imag({S¦D}_k[n=0][k=0])
   r0 = M[r9 + $aac.mem.SBR_X_curr_imag_ptr];
   I4 = r0;

   Null = M2;
   if Z jump end_if_second_half_of_frame;
      // I0 -> real(X_sbr[ch][k=0][l=16+SBR_tHFAdj]) = real({S¦D}_k[n=16][k=0])
      r0 = M[r9 + $aac.mem.SBR_X_curr_real_ptr];
      I0 = r0 + (($aacdec.PS_NUM_SAMPLES_PER_FRAME/2) * $aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
      // I4 -> imag(X_sbr[ch][k=0][l=16+SBR_tHFAdj]) = imag({S¦D}_k[n=16][k=0])
      r0 = M[r9 + $aac.mem.SBR_X_curr_imag_ptr];
      I4 = r0 + (($aacdec.PS_NUM_SAMPLES_PER_FRAME/2) * $aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
   end_if_second_half_of_frame:


   // for p=0:PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS-1,
   r8 = 0;
   ps_hybrid_synthesis_qmf_subband_loop:

      // I3 -> real(X_sbr[ch][k=p][l=SBR_tHFAdj+{0¦¦16}]) = real({S¦D}_k[n={0¦¦16}][k=p])
      I3 = I0;
      // I7 -> imag(X_sbr[ch][k=p][l=SBR_tHFAdj+{0¦¦16}]) = imag({S¦D}_k[n={0¦¦16}][k=p])
      I7 = I4;

      // for n=0:{16¦¦32}-1,
      r10 = r4;

      r0 = 0;

      do ps_hybrid_synthesis_clear_hybrid_qmf_band_loop;
         M[I3,M0] = r0,
          M[I7,M0] = r0;
      ps_hybrid_synthesis_clear_hybrid_qmf_band_loop:


      // for q=0:PS_NUM_SUB_SUBBANDS_PER_HYBRID_QMF_SUBBAND[p]-1,
      r0 = M[r9 + $aac.mem.PS_num_sub_subbands_per_hybrid_qmf_subband_ptr];
      r7 = M[r0 + r8];
      r6 = 0;

      ps_hybrid_synthesis_sub_subband_loop:

         r1 = M[r9 + $aac.mem.PS_hybrid_qmf_sub_subband_offset_ptr];
         r1 = M[r1 + r8];
         r0 = r9 + $aac.mem.PS_X_hybrid_real_address;
         r0 = M[r0 + r5];
         r2 = r9 + $aac.mem.PS_X_hybrid_imag_address;
         r2 = M[r2 + r5];
         r1 = r1 + r6;
         r1 = r1 * ($aacdec.PS_NUM_SAMPLES_PER_FRAME*ADDR_PER_WORD) (int);
         // I3 -> real(ps_X_hybrid[ch][k=hybrid_sub_subband][l={0¦¦16}]) = real({S¦D}_k[n={0¦¦16}][k=hybrid_sub_subband])
         // I7 -> imag(ps_X_hybrid[ch][k=hybrid_sub_subband][l={0¦¦16}]) = imag({S¦D}_k[n={0¦¦16}][k=hybrid_sub_subband])
         I3 = r0 + r1;
         I7 = r2 + r1;
         Null = M2;
         if Z jump end_if_add_second_half_of_subband_signal;
            I3 = I3 + ($aacdec.PS_NUM_SAMPLES_PER_FRAME * ADDR_PER_WORD / 2);
            I7 = I7 + ($aacdec.PS_NUM_SAMPLES_PER_FRAME * ADDR_PER_WORD / 2);
         end_if_add_second_half_of_subband_signal:

         // for n=0:{0¦¦16}-1,
         r10 = r4;

         // I2 -> real(X_sbr[ch][k=p][l=SBR_tHFAdj+{0¦¦16}]) = real({S¦D}_k[n={0¦¦16}][k=p])
         I2 = I0;
         // I6 -> imag(X_sbr[ch][k=p][l=SBR_tHFAdj+{0¦¦16}]) = imag({S¦D}_k[n={0¦¦16}][k=p])
         I6 = I4;

         do ps_hybrid_synthesis_time_sample_loop_1;

            r0 = M[I2,0],  // r0 = real(X_sbr[ch][k=p][l=SBR_tFHAdj+n]) = real({S¦D}_k[n][k=p])
             r1 = M[I6,0]; // r1 = imag(X_sbr[ch][k=p][l=SBR_tHFAdj+n]) = imag({S¦D}_k[n][k=p])

            r2 = M[I3, MK1];  // r2 = real(ps_X_hybrid[ch][k=hybrid_sub_subband][l=n]) = real({S¦D}_k[n][k=hybrid_sub_subband])

            r0 = r0 + r2,
             r3 = M[I7, MK1]; // r3 = imag(ps_X_hybrid[ch][k=hybrid_sub_subband][l=n]) = imag({S¦D}_k[n][k=hybrid_sub_subband])

            r1 = r1 + r3;

            M[I2,M0] = r0, // real(X_sbr[ch][k=p][l=SBR_tFHAdj+n]) += real(ps_X_hybrid[ch][k=hybrid_sub_subband][l=n])
             M[I6,M0] = r1; // imag(X_sbr[ch][k=p][l=SBR_tFHAdj+n]) += imag(ps_X_hybrid[ch][k=hybrid_sub_subband][l=n])

         ps_hybrid_synthesis_time_sample_loop_1:

         r6 = r6 + 1;
         Null = r6 - r7;
      if LT jump ps_hybrid_synthesis_sub_subband_loop;

      I0 = I0 + MK1;
      I4 = I4 + MK1;

      r8 = r8 + ADDR_PER_WORD;
      Null = r8 - ($aacdec.PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS*ADDR_PER_WORD);
   if LT jump ps_hybrid_synthesis_qmf_subband_loop;

   Addr2Words(r5);
   rts;

.ENDMODULE;

#endif

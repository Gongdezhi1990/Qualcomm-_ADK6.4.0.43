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
//    $aacdec.ps_hybrid_analysis
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
.MODULE $M.aacdec.ps_hybrid_analysis;
   .CODESEGMENT AACDEC_PS_HYBRID_ANALYSIS_PM;
   .DATASEGMENT DM;

   $aacdec.ps_hybrid_analysis:

   // push rLink onto stack
   push rLink;

   M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_ANALYSIS_TEMP_R8] = r8;

   // ps_hybrid_sub_subband_index_offset = 0
   M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_SUB_SUBBAND_INDEX_OFFSET] = Null;

   // for p=0:PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS
   r0 = 0;

   hybrid_analysis_qmf_subband_loop:

      M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_QMF_SUBBAND] = r0;

      Null = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_QMF_SUBBAND];
      if NZ jump ps_type_b_fir_filter;
         // type A filter (complex 8-channel FIR)

         PROFILER_START(&$aacdec.profile_ps_hybrid_analysis_type_a_fir)
         call $aacdec.ps_hybrid_type_a_fir_filter;
         PROFILER_STOP(&$aacdec.profile_ps_hybrid_analysis_type_a_fir)

         r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_SUB_SUBBAND_INDEX_OFFSET];
         r0 = r0 + ($aacdec.PS_HYBRID_TYPE_A_NUM_SUB_SUBBANDS - 2);
         M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_SUB_SUBBAND_INDEX_OFFSET] = r0;

         jump qmf_subband_filtered;
      ps_type_b_fir_filter:
         // type B filter (real 2-channel FIR)

         PROFILER_START(&$aacdec.profile_ps_hybrid_analysis_type_b_fir)
         call $aacdec.ps_hybrid_type_b_fir_filter;
         PROFILER_STOP(&$aacdec.profile_ps_hybrid_analysis_type_b_fir)

         r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_SUB_SUBBAND_INDEX_OFFSET];
         r0 = r0 + $aacdec.PS_HYBRID_TYPE_B_NUM_SUB_SUBBANDS;
         M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_SUB_SUBBAND_INDEX_OFFSET] = r0;

      qmf_subband_filtered:

      r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_QMF_SUBBAND];
      r0 = r0 + 1;
      Null = r0 - $aacdec.PS_NUM_HYBRID_QMF_BANDS_WHEN_20_PAR_BANDS;
   if LT jump hybrid_analysis_qmf_subband_loop;


   r8 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_ANALYSIS_TEMP_R8];

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

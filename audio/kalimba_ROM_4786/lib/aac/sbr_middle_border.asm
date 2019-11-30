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
//    $aacdec.sbr_middle_border
//
// DESCRIPTION:
//    Calculate the 'middleBorder'
//
// INPUTS:
//    - r2 SBR_bs_num_env[ch]
//    - r5 current channel (0/1)
//
// OUTPUTS:
//    - r3 SBR_middle_border_index[ch]
//
// TRASHED REGISTERS:
//    - none
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_middle_border;
   .CODESEGMENT AACDEC_SBR_MIDDLE_BORDER_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_middle_border:

   // switch(SBR_bs_frame_class[ch])
   Words2Addr(r5);
   r3 = r5 + r9;
   r3 = M[$aac.mem.SBR_bs_frame_class + r3];
   Addr2Words(r5);

   // case SBR_FIXFIX
   Null = r3 - $aacdec.SBR_FIXFIX;
   if NZ jump not_case_fixfix;
      r3 = r2 LSHIFT -1;
      rts;
   // case SBR_VARFIX
   not_case_fixfix:


#ifdef AACDEC_ELD_ADDITIONS
   r4 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r4 - $aacdec.ER_AAC_ELD;
   if NZ jump non_eld_version;

      // case SBR_LDTRAN
      Null = r3 - $aacdec.SBR_LDTRAN;
      if NZ rts;
         r3 = 1;
         rts;

   non_eld_version:
#endif

      Null = r3 - $aacdec.SBR_VARFIX;
      if NZ jump not_case_varfix;
         Words2Addr(r5);
         r3 = r5 + r9;
         r3 = M[$aac.mem.SBR_bs_pointer + r3];
         Addr2Words(r5);
         Null = r3;
         // if(SBR_bs_pointer[ch]==0)
         if NZ jump bs_pointer_ne_zero;
            r3 = 1;
            rts;
         // if(SBR_bs_pointer[ch]==1)
         bs_pointer_ne_zero:
         Null = r3 - 1;
         if NZ jump bs_pointer_ne_one;
            r3 = r2 - 1;
            rts;
         bs_pointer_ne_one:
            r3 = r3 - 1;
            rts;
      // case SBR_FIXVAR | SBR_VARFIX
      not_case_varfix:
      // if(SBR_bs_pointer[ch] > 1)
      Words2Addr(r5);
      r3 = r5 + r9;
      r3 = M[$aac.mem.SBR_bs_pointer + r3];
      Addr2Words(r5);
      Null = r3 - 1;
      if LE jump bs_pointer_le_one;
         r3 = r2 - r3;
         r3 = r3 + 1;
         rts;
      bs_pointer_le_one:
      r3 = r2 - 1;
      rts;

.ENDMODULE;

#endif

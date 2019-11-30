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
//    $aacdec.sbr_read_qdiv_tables
//
// DESCRIPTION:
//
//
// INPUTS:
//    - r3 - column number
//    - r4 - row number
//    - r8 - 9
//    - M3 - address of page mapped onto dm from flash
//
// OUTPUTS:
//    - r0 = value from table
//    - r1 = 1.0 - r0
//
// TRASHED REGISTERS:
//    - none
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_read_qdiv_tables;
   .CODESEGMENT AACDEC_SBR_READ_QDIV_TABLES_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_read_qdiv_tables:

   r0 = r4 * ($aacdec.SBR_Q_DIV_TABLES_ROW_LENGTH*ADDR_PER_WORD) (int);
   r0 = r0 + M3;
   Words2Addr(r3);
   r0 = M[r0 + r3];
#ifdef KAL_ARCH4
   r0 = r0 LSHIFT 16;
   r0 = r0 ASHIFT -8;
#endif
   if NEG r0 = r0 LSHIFT r8;
   r1 = 1.0 - r0;
   Addr2Words(r3);
   rts;

.ENDMODULE;

#endif

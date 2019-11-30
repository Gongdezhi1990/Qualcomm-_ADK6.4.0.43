// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.tmp_mem_pool_allocate
//
// DESCRIPTION:
//    Temporary memory pool allocate
//
// INPUTS:
//    - r0 = number of words to allocate
//
// OUTPUTS:
//    - r0 = set to -ve on error otherwise unaffected
//    - r1 = pointer to memory to use
//
// TRASHED REGISTERS:
//    r2
//
// *****************************************************************************
.MODULE $M.aacdec.tmp_mem_pool_allocate;
   .CODESEGMENT AACDEC_TMP_MEM_POOL_ALLOCATE_PM;
   .DATASEGMENT DM;

   $aacdec.tmp_mem_pool_allocate:

   Null = r0;
   if NEG rts;

   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_END_PTR];
   Words2Addr(r0);
   r2 = r1 + r0;
   Addr2Words(r0);
   M[r9 + $aac.mem.TMP_MEM_POOL_END_PTR] = r2;

  // r2 = r1 + r0;
   push r3;
   r3 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r3 = r3 + ($aacdec.TMP_MEM_POOL_LENGTH*ADDR_PER_WORD);
   r2 = r2 - r3;
   #ifdef AACDEC_CALL_ERROR_ON_MALLOC_FAIL
      if GT call $error;
   #else
      if LE jump ok;
         r0 = -1;
      ok:
   #endif
   pop r3;

   Null = r0;
   rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $aacdec.tmp_mem_pool_free
//
// DESCRIPTION:
//    Temporary memory pool free
//
// INPUTS:
//    - r0 = number of words to free
//
// OUTPUTS:
//    - r0 = set to -ve on error otherwise unaffected
//
// TRASHED REGISTERS:
//    r1
//
// *****************************************************************************
.MODULE $M.aacdec.tmp_mem_pool_free;
   .CODESEGMENT AACDEC_TMP_MEM_POOL_FREE_PM;
   .DATASEGMENT DM;

   $aacdec.tmp_mem_pool_free:

   Null = r0;
   if NEG rts;

   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_END_PTR];
   Words2Addr(r0);
   r2 = r1 - r0;
   Addr2Words(r0);
   M[r9 + $aac.mem.TMP_MEM_POOL_END_PTR] = r2;

   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r1 = r2 - r1;
   #ifdef AACDEC_CALL_ERROR_ON_MALLOC_FAIL
      if NEG call $error;
   #else
      if POS jump ok;
         r0 = -1;
      ok:
   #endif

   Null = r0;
   rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $aacdec.tmp_mem_pool_free_all
//
// DESCRIPTION:
//    Temporary memory pool free all
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0
//
// *****************************************************************************
.MODULE $M.aacdec.tmp_mem_pool_free_all;
   .CODESEGMENT AACDEC_TMP_MEM_POOL_FREE_ALL_PM;
   .DATASEGMENT DM;

   $aacdec.tmp_mem_pool_free_all:

   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r9 + $aac.mem.TMP_MEM_POOL_END_PTR] = r0;
   rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $aacdec.frame_mem_pool_allocate
//
// DESCRIPTION:
//    Frame memory pool allocate
//
// INPUTS:
//    - r0 = number of words to allocate
//
// OUTPUTS:
//    - r0 = set to -ve on error otherwise unaffected
//    - r1 = pointer to memory to use
//
// TRASHED REGISTERS:
//    r2
//
// *****************************************************************************
.MODULE $M.aacdec.frame_mem_pool_allocate;
   .CODESEGMENT AACDEC_FRAME_MEM_POOL_ALLOCATE_PM;
   .DATASEGMENT DM;

   $aacdec.frame_mem_pool_allocate:

   Null = r0;
   if NEG rts;

   r1 = M[r9 + $aac.mem.FRAME_MEM_POOL_END_PTR];
   Words2Addr(r0);
   r2 = r1 + r0;
   Addr2Words(r0);
   M[r9 + $aac.mem.FRAME_MEM_POOL_END_PTR] = r2;

   push r3;
   r3 = M[r9 + $aac.mem.FRAME_MEM_POOL_PTR];
   r3 = r3 + ($aacdec.FRAME_MEM_POOL_LENGTH*ADDR_PER_WORD);
   r2 = r2 - r3;
   #ifdef AACDEC_CALL_ERROR_ON_MALLOC_FAIL
      if GT call $error;
   #else
      if LE jump ok;
         r0 = -1;
      ok:
   #endif
   pop r3;

   Null = r0;
   rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $aacdec.frame_mem_pool_free
//
// DESCRIPTION:
//    Frame memory pool free
//
// INPUTS:
//    - r0 = number of words to free
//
// OUTPUTS:
//    - r0 = set to -ve on error otherwise unaffected
//
// TRASHED REGISTERS:
//    r1
//
// *****************************************************************************
.MODULE $M.aacdec.frame_mem_pool_free;
   .CODESEGMENT AACDEC_FRAME_MEM_POOL_FREE_PM;
   .DATASEGMENT DM;

   $aacdec.frame_mem_pool_free:

   Null = r0;
   if NEG rts;

   push r0;
   r1 = M[r9 + $aac.mem.FRAME_MEM_POOL_END_PTR];
   Words2Addr(r0);
   r1 = r1 - r0;
   M[r9 + $aac.mem.FRAME_MEM_POOL_END_PTR] = r1;

   //r1 = r1 - r0;
   r0 = M[r9 + $aac.mem.FRAME_MEM_POOL_PTR];
   r1 = r1 - r0;
   pop r0;
   Null = r1;
   #ifdef AACDEC_CALL_ERROR_ON_MALLOC_FAIL
      if NEG call $error;
   #else
      if POS jump ok;
         r0 = -1;
      ok:
   #endif
   Null = r0;
   rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $aacdec.frame_mem_pool_free_all
//
// DESCRIPTION:
//    Frame memory pool free all
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0
//
// *****************************************************************************
.MODULE $M.aacdec.frame_mem_pool_free_all;
   .CODESEGMENT AACDEC_FRAME_MEM_POOL_FREE_ALL_PM;
   .DATASEGMENT DM;

   $aacdec.frame_mem_pool_free_all:

   r0 = M[r9 + $aac.mem.FRAME_MEM_POOL_PTR];
   M[r9 + $aac.mem.FRAME_MEM_POOL_END_PTR] = r0;
   M[r9 + $aac.mem.AMOUNT_UNPACKED] = Null;
   rts;

.ENDMODULE;

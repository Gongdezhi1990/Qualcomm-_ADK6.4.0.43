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
//    $aacdec.sbr_limiter_frequency_table
//
// DESCRIPTION:
//    Calculate fTableLim which contains the frequency borders used by the limiter
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r8, r10, rMAC
//    - I0-I6
//    - M0-M3
//    - first element in $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_limiter_frequency_table;
   .CODESEGMENT AACDEC_SBR_LIMITER_FREQUENCY_TABLE_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_limiter_frequency_table:

   // push rLink onto stack
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.SBR_LIMITER_FREQUENCY_TABLE_ASM.SBR_LIMITER_FREQUENCY_TABLE.SBR_LIMITER_FREQUENCY_TABLE.PATCH_ID_0, r1)
#endif
   
   // allocate temporary buffers (Ftable_lim deallocated in later function)
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = M[r0 + $aacdec.SBR_Nlow];
   Words2Addr(r0);
   I6 = r0 + (4*ADDR_PER_WORD);
   Addr2Words(r0);
   r0 = r0 + 5;
   r0 = r0 * 4 (int);
   call $aacdec.frame_mem_pool_allocate;
   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r0 + $aacdec.SBR_F_table_lim_base_ptr] = r1;
   I3 = r1;

   r0 = $aacdec.SBR_LIM_TABLE_SIZE;
   call $aacdec.frame_mem_pool_allocate;
   M1 = r1;

   r0 = $aacdec.SBR_PATCH_BORDERS_SIZE;
   call $aacdec.frame_mem_pool_allocate;
   if NEG jump $aacdec.corruption;
   M2 = r1;

   // sbr.Ftable_lim(1, 1) = sbr.F_TableLow(1) - sbr.Kx;
   // sbr.Ftable_lim(1, 2) = sbr.F_TableLow(sbr.Nlow+1) - sbr.Kx;
   // sbr.N_L(1) = 1;
   r2 = M[r9 + $aac.mem.SBR_info_ptr];
   r1 = M[r2 + $aacdec.SBR_kx];
   M0 = r1;
   r3 = M[r2 + $aacdec.SBR_F_table_low];
   r3 = r3 - r1;

   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   I5 = r0 + $aacdec.SBR_N_L;
   rMAC = 1;

   r4 = M[r2 + $aacdec.SBR_Nlow];
   Words2Addr(r4);
   r2 = r2 + r4;
   r0 = M[r2 + $aacdec.SBR_F_table_low];
   Addr2Words(r4);
   r0 = r0 - r1,
    M[I3, MK1] = r3,
    M[I5, MK1] = rMAC;

   M3 = I6;
   r1 = 0,
    M[I3, M3] = r0;

   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r0 = M[r0 + $aacdec.SBR_num_patches];
   M3 = r0;

   main_loop:

      // store loop number
      M[r9 + $aac.mem.TMP] = r1;

      // patchBorders(1:64) = 0;
      // patchBorders(1) = sbr.Kx;
      //
      // for k=1:sbr.noPatches,
      //    patchBorders(k+1) = patchBorders(k) + sbr.patchNoSubbands(k);
      // end;

      I0 = M1;
      I1 = M2;

      r10 = M3;
      r8 = $aacdec.SBR_PATCH_BORDERS_SIZE - r10;
      r0 = M0;
      M[I1, 0] = r0;
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      I4 = r0 + $aacdec.SBR_patch_num_subbands;
      do patch_borders_loop;
         r0 = M[I1, MK1],
          r1 = M[I4, MK1];
         r0 = r0 + r1;
         M[I1, 0] = r0;
      patch_borders_loop:

      r10 = r8;
      r0 = 0;
      I1 = I1 + MK1;
      do pb_zero_loop;
         M[I1, MK1] = r0;
      pb_zero_loop:


      // limTable(1:100) = 0;
      // for k=0:sbr.Nlow,
      //    limTable(k+1) = sbr.F_TableLow(k+1);
      // end;
      //
      // for k=1:sbr.noPatches-1,
      //    limTable(k + sbr.Nlow + 1) = patchBorders(k+1);
      // end;

      r10 = r4 + 1;
      r6 = ($aacdec.SBR_LIM_TABLE_SIZE + 1) - r10;
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      I4 = r0 + $aacdec.SBR_F_table_low;
      do lim_table_loop_1;
         r0 = M[I4, MK1];
         M[I0, MK1] = r0;
      lim_table_loop_1:

      r10 = M3;
      r8 = r10 + r4;
      I4 = I1 - ($aacdec.SBR_PATCH_BORDERS_SIZE*ADDR_PER_WORD);

      r6 = r6 - r10,
       r0 = M[I4, MK1];

      r10 = r10 - 1;
      do lim_table_loop_2;
         M[I0, MK1] = r0,
          r0 = M[I4, MK1];
      lim_table_loop_2:

      r10 = r6;
      r0 = 0;
      do lt_zero_loop;
         M[I0, MK1] = r0;
      lt_zero_loop:



      // nrLim = sbr.noPatches + sbr.Nlow - 1;
      // limTable(1:100) = sbr_bubble_sort(limTable, (sbr.noPatches + sbr.Nlow) );
      // k = 0;
      //
      // if (nrLim < 0)
      //    return;
      // end;

      r0 = r8;
      r1 = M1;
      call $aacdec.sbr_bubble_sort;

      r5 = r1;                                           // r5 = &lim_table[k]     k = 0
      r8 = r8 - 1;                                       // r8 = nrLim
      if NEG jump escape;



      /*///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        [inc_k]        <-------------------------------------------------------------------------------
                           k = k + 1                                                                                  |
                              |                                                                                       |
                             \|/                                                                                      |
                        [top]                                                                                         |
      --------------->  if k > nrLim                                                                                  |
      |                       |                                                                                       |
      |         |-----------T---F--------------------------------|                                                    |
      |    N_L(s+1) = nrLim                                  [small_k]                                                |
      |    for k = 0:nrLim  [writeback_loop]                 if LT(k+1) = LT(k)                                       |
      |       Ftable_lim(s+1, k+1) = LT(k+1) - Kx                       |                                             |
      |    end                                                          |                                             |
      |         |                              |----------------------T---F-----------------------------|             |
      |         |                              |                         if log2(LT(k+1)) - log2(LT(k)) < 0.49/Q(s)   |
      |         |                              |                                         OR     LT(k+1) = 0           |
      |        \|/                             |                                         OR       LT(k) = 0           |
      |    [escape]                            |                                    Q = [1.2, 2, 3]     |             |
      |        OUT                             |                                                        |             |
      |                                        |                                                        |             |
      |                                        |                     |--------------------------------T---F---------->|
      |                                        |           [search_pb]                                               /|\
      |                                        |              if LT(k+1) is in                                        |
      |                                        |                patchBorders                                          |
      |                                        |                     |           [found_inpb]                         |
      |                                        |<------------------F---T---------------------------|                  |
      |                                        |                                           [search_pb]                |
      |                                        |                                              if LT(k) is in          |
      |                                        |                                               patchBorders           |
      |                                        |                                                   |                  |
      |                                        |<------------------------------------------------F---T----------------|
      |                                        |                                                         [found_inpb]
      |                                       \|/
      |                              [rem_kth]
      |                                 if come from LT(k) in patchBorders
      |                                    n = k
      |                                    length = noPatches + Nlow
      |                                 else
      |                                    n = k + 1
      |                                    length = nrLim
      |                                 end
      |                                 LT(n) = F_TableLow(Nlow+1)
      |                                 Bubblesort(LT, length)
      |                                 nrLim = nrLim - 1
      |                                       |
      |----------------------------------------

      labels in square brackets
      LT = limTable
      *////////////////////////////////////////////////////////////////////////////////////////////////////////////////


      inc_k:
         r5 = r5 + (1*ADDR_PER_WORD);                           // r5 = &lim_table[k]     k = k+1;

      top:
         r0 = M1 - r5;
         Addr2Words(r0);
         Null = r0 + r8;
         if GE jump small_k;
            r1 = r8;
            M[I5, MK1] = r1;
            r10 = r8 + 1;
            I1 = M1;
            do writeback_loop;
               r0 = M[I1, MK1];
               r0 = r0 - M0;
               M[I3, MK1] = r0;
            writeback_loop:
            Words2Addr(r8);
            r0 = r8 - I6;
            I3 = I3 - r0;
            jump escape;

         small_k:
            r2 = M[r5];

            r3 = M[r5 + (-1*ADDR_PER_WORD)];

            r6 = r5;
            Null = r2 - r3;
            if Z jump rem_kth;
               r6 = r3 * ADDR_PER_WORD (int);
               if Z jump search_pb_start;
               r7 = r2 * ADDR_PER_WORD (int);
               if Z jump search_pb_start;
               r0 = r6 + (&$aacdec.sbr_log_base2_table - (1*ADDR_PER_WORD));
               call $aacdec.sbr_read_one_word_from_flash;
               r10 = r0 LSHIFT (DAWTH-16);

               r0 = r7 + (&$aacdec.sbr_log_base2_table - (1*ADDR_PER_WORD));
               call $aacdec.sbr_read_one_word_from_flash;
               r0 = r0 LSHIFT (DAWTH-16);

               r0 = r0 - r10;
               r1 = M[r9 + $aac.mem.TMP];
               Words2Addr(r1);
               r2 = M[r9 + $aac.mem.SBR_limiter_bands_compare_ptr];
               r1 = M[r2 + r1];
               Null = r0 - r1;
               if GE jump inc_k;

               search_pb_start:
               r1 = -1;
               search_pb_repeat:

               r10 = M3 + 1;
               I0 = M2;
               Addr2Words(r7);
               do search_pb;
                  r0 = M[I0, MK1];
                  Null = r7 - r0;
                  if LE jump found_inpb;    // jump out if found r7 in patch_borders or gone past
               search_pb:                   // point where would be stored
               r6 = r5 - (1*ADDR_PER_WORD);
               Null = r1;
               if NEG r6 = r5;

            rem_kth:
               Words2Addr(r4);
               r0 = M[r9 + $aac.mem.SBR_info_ptr];
               r0 = r0 + r4;
               r2 = M[r0 + $aacdec.SBR_F_table_low];
               Addr2Words(r4);
               M[r6] = r2;
               r0 = M3 + r4;
               Null = r1;
               if NEG r0 = r8;
               r1 = M1;
               call $aacdec.sbr_bubble_sort;
               r8 = r8 - 1;
               jump top;

                  found_inpb:
                     if NZ jump search_pb;   // if not found jump back
                     r7 = r6;
                     r1 = r1 + 1;
                     if Z jump search_pb_repeat;
                     jump inc_k;


   escape:
   // retrieve loop number. loop 3 times
   r1 = M[r9 + $aac.mem.TMP];
   r1 = r1 + 1;
   Null = r1 - 3;
   if LT jump main_loop;


   // deallocate temporary buffers
   r0 = $aacdec.SBR_PATCH_BORDERS_SIZE + $aacdec.SBR_LIM_TABLE_SIZE;
   call $aacdec.frame_mem_pool_free;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************
#if defined(PATCH_LIBS)
   #include "patch_library.h"
#endif
// *****************************************************************************
// MODULE:
//    $math.median_filter_process
//
// DESCRIPTION:
//    median filter
//
// INPUTS:
//    - I0 = history buffer
//    - I1 = index buffer
//    - r8 = buffer length ( = filter length)
//    - r0 = new sample
//
// OUTPUTS:
//    - r0 = median-filtered value
//
// TRASHED REGISTERS:
//    r1-r4, I0, I1, I4, I5, M0, r10, DoLoop
//
// CPU USAGE:
//    TODO
//
// NOTES:
//    1) TODO
// *****************************************************************************
.MODULE $M.math.median_filter_process;
   .CODESEGMENT MATH_MEDIAN_FILTER_PROCESS_PM;
   .DATASEGMENT DM;

   $math.median_filter_process:
   
#if defined(PATCH_LIBS)
   // patch point trashes r1
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($math.MEDIAN_FILTER_ASM.MEDIAN_FILTER_PROCESS.PATCH_ID_0, r1)
#endif

   // find old sample and increase index
   r10 = r8;
   r4 = r8;
   r1 = M[I1, 0];
   M0 = 1;
   do loop_1;
      // find the old sample and increment indices
      r1 = r1 + M0,       r3 = M[I0, M0];
      Null = r1 - r8;
      if EQ r2 = r10,     M[I1, M0] = r1;

      // find new samples location
      r3 = r3 - r0,       r1 = M[I1, 0];
      if GE r4 = r10;
   loop_1:
   // r2 = (r8 - index_of_the_oldest_sample)
   // r4 = (r8 - 1 - index_of_the_new_sample)
   r4 = r4 + 1;
   r4 = r8 - r4;
   r2 = r8 - r2;
   r1 = I0 - r8;
   I4 = I1 - r8;

   Null = r2 - r4;
   if LT jump is_less_than;
      // otherwise it is greater_than
      if NE r4 = r4 + M0;
      r10 = r2 - r4;

      jump start_copying;
   is_less_than:
      M0 = -1;
      r10 = r4 - r2;

   start_copying:
   I0 = r1 + r4;
   I1 = I4 + r4;
   r10 = r10 + 1;
   r2 = 0;
   I4 = I0;
   I5 = I1;
   do loop_2;
      M[I0, M0] = r0,       r0 = M[I4, M0];
      M[I1, M0] = r2,       r2 = M[I5, M0];
   loop_2:

   sorted:
   r2 = r8 ASHIFT -1;
   r2 = r2 + r1;
   r0 = M[r2];
   Null = r8 AND 0x1;
   if NZ rts;
   r2 = r2 - 1;
   r1 = r0 + M[r2];
   if V jump overflow_happened;
   r0 = r1 ASHIFT -1;

   rts;
   overflow_happened:
      r0 = r0 ASHIFT -1;
      r1 = M[r2];
      r1 = r1 ASHIFT -1;
      r0 = r0 + r1;
      rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $math.median_filter_initialise
//
// DESCRIPTION:
//    median filter
//
// INPUTS:
//    - I0 = history buffer
//    - I4 = index buffer
//    - r0 = initial value
//    - r10 = buffer length ( = filter length)
//
// OUTPUTS:
//    - TODO
//
// TRASHED REGISTERS:
//    r1-r2, I0, I4, r10, doLoop
//
// CPU USAGE:
//    TODO
//
// NOTES:
//    1) TODO
// *****************************************************************************
.MODULE $M.math.median_filter_initialise;
   .CODESEGMENT MATH_MEDIAN_FILTER_INITIALISE_PM;
   .DATASEGMENT DM;

   $math.median_filter_initialise:
   
#if defined(PATCH_LIBS)
   // patch point trashes r1
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($math.MEDIAN_FILTER_ASM.MEDIAN_FILTER_INITIALIZE.PATCH_ID_0, r1)
#endif

   // populate the buffers
   r1 = 0;
   r2 = 1;
   do init_loop;
      r1 = r1 + r2, M[I0, 1] = r0, M[I4, 1] = r1;
   init_loop:

   rts;

.ENDMODULE;

/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
// Previn-compatible patch macros for Hydra audio subsystem
// These patch macros are also available in generic assembler code
//
// Currently these all use the "fast" direct table lookup

#ifndef PATCH_ASM_MACROS_HEADER_INCLUDED
#define PATCH_ASM_MACROS_HEADER_INCLUDED

#include "patch_ids.h"

#ifdef INCLUDE_PATCHES
#define FAST_SW_ROM_PATCH_POINT(patch_id, reg, table_fast)  \
   reg = M[$patch.table_fast + (patch_id * ADDR_PER_WORD)];                   \
   if NZ jump reg;                                          \
   patch_id##.##PATCH_LABEL:
#else
#define FAST_SW_ROM_PATCH_POINT(patch_id, reg, table_fast)
#endif /* INCLUDE_PATCHES */

// Slow ROM patch
#ifdef INCLUDE_PATCHES
#define PUSH_R0_SLOW_SW_ROM_PATCH_POINT(patch_id, reg, table_slow, num_fast_patch)  \
   push r0;                                                                         \
   r0 = patch_id;                                                                   \
   reg = M[$patch.table_slow + ((patch_id / DAWTH) * ADDR_PER_WORD)];                                    \
   Null = reg AND (1 << (patch_id % DAWTH));                                           \
   if NZ jump $patch.id_table_search;                                               \
   patch_id##.##PATCH_LABEL:                                                        \
   pop r0;
#else
#define PUSH_R0_SLOW_SW_ROM_PATCH_POINT(patch_id, reg, table_slow, num_fast_patch)
#endif /* INCLUDE_PATCHES */

// Slow ROM patch (Library version)
#ifdef INCLUDE_PATCHES
#define LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT(patch_id, reg)  \
   push r0;                                                  \
   r0 = patch_id;                                            \
   reg = M[$patch.table_slow + ((patch_id / DAWTH) * ADDR_PER_WORD)];             \
   Null = reg AND (1 << (patch_id % DAWTH));                    \
   if NZ jump $patch.id_table_search;                        \
   patch_id##.##PATCH_LABEL:                                 \
   pop r0;
#else
#define LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT(patch_id, reg)
#endif /* INCLUDE_PATCHES */

// Slow ROM patch
#ifdef INCLUDE_PATCHES
// reg is the register that can be trashed - also trashes r0
#define SLOW_SW_ROM_PATCH_POINT(patch_id, reg, table_slow, num_fast_patch)  \
   r0 = patch_id;                                           \
   reg = M[$patch.table_slow + ((patch_id / DAWTH) * ADDR_PER_WORD)];                            \
   Null = reg AND (1 << (patch_id % DAWTH));                                   \
   if NZ jump $patch.id_table_search;                                       \
   patch_id##.##PATCH_LABEL:
#else
#define SLOW_SW_ROM_PATCH_POINT(patch_id, reg, table_slow, num_fast_patch)
#endif /* INCLUDE_PATCHES */

// Slow ROM patch (Library version)
#ifdef INCLUDE_PATCHES
#define LIBS_SLOW_SW_ROM_PATCH_POINT(patch_id, reg)  \
   r0 = patch_id;                                    \
   reg = M[$patch.table_slow + ((patch_id / DAWTH) * ADDR_PER_WORD)];     \
   Null = reg AND (1 << (patch_id % DAWTH));            \
   if NZ jump $patch.id_table_search;                \
   patch_id##.##PATCH_LABEL:
#else
#define LIBS_SLOW_SW_ROM_PATCH_POINT(patch_id, reg)
#endif /* INCLUDE_PATCHES */

// Very Slow ROM patch
#ifdef INCLUDE_PATCHES
// NOTE: VERY_SLOW_SW_ROM_PATCH_POINT should be used only when rLink has been pushed
#define PUSH_R0_VERY_SLOW_SW_ROM_PATCH_POINT(patch_id) \
   push r0;                                            \
   r0 = patch_id;                                      \
   call $patch.id_table_search;                        \
   pop r0;
#else
#define PUSH_R0_VERY_SLOW_SW_ROM_PATCH_POINT(patch_id)
#endif /* INCLUDE_PATCHES */

// Very Slow ROM patch
#ifdef INCLUDE_PATCHES
// trashes r0
// NOTE: VERY_SLOW_SW_ROM_PATCH_POINT should be used only when rLink has been pushed
#define VERY_SLOW_SW_ROM_PATCH_POINT(patch_id) \
   r0 = patch_id;                              \
   call $patch.id_table_search;
#else
#define VERY_SLOW_SW_ROM_PATCH_POINT(patch_id)
#endif /* INCLUDE_PATCHES */

// Slow ROM patch, preserves registers
#ifdef INCLUDE_PATCHES
#define PUSH_REGS_SLOW_SW_ROM_PATCH_POINT(patch_id, table_slow, num_fast_patch) \
   pushm <r0, r1>;                                                              \
   r0 = patch_id;                                                               \
   r1 = M[$patch.table_slow + (patch_id / DAWTH) * ADDR_PER_WORD];                                 \
   Null = r1 AND (1 << (patch_id % DAWTH));                                        \
   if NZ jump $patch.id_table_search;                                           \
   patch_id##.##PATCH_LABEL:                                                    \
   popm <r0, r1>;
#else
#define PUSH_REGS_SLOW_SW_ROM_PATCH_POINT(patch_id, table_slow, num_fast_patch)
#endif /* INCLUDE_PATCHES */

// Slow ROM patch, preserves registers (Library version)
#ifdef INCLUDE_PATCHES
#define LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT(patch_id) \
   pushm <r0, r1>;                                       \
   r0 = patch_id;                                        \
   r1 = M[$patch.table_slow + (patch_id / DAWTH) * ADDR_PER_WORD];          \
   Null = r1 AND (1 << (patch_id % DAWTH));                 \
   if NZ jump $patch.id_table_search;                    \
   patch_id##.##PATCH_LABEL:                             \
   popm <r0, r1>;
#else
#define LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT(patch_id)
#endif /* INCLUDE_PATCHES */

#endif  // PATCH_ASM_MACROS_HEADER_INCLUDED

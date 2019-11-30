/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
// Previn-compatible patch macros for Hydra audio subsystem
// These patch macros are also available in generic assembler code
//
// Currently these all use the "fast" direct table lookup

// ### not needed for dummy macros:    #include "patch_ids.h"

#define FAST_SW_ROM_PATCH_POINT(patch_id, reg, table_fast)

// Slow ROM patch
#define PUSH_R0_SLOW_SW_ROM_PATCH_POINT(patch_id, reg, table_slow, num_fast_patch)

// Slow ROM patch (Library version)
#define LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT(patch_id, reg)  

// Slow ROM patch
// reg is the register that can be trashed - also trashes r0
#define SLOW_SW_ROM_PATCH_POINT(patch_id, reg, table_slow, num_fast_patch)

// Slow ROM patch (Library version)
#define LIBS_SLOW_SW_ROM_PATCH_POINT(patch_id, reg)

// Very Slow ROM patch
// NOTE: VERY_SLOW_SW_ROM_PATCH_POINT should be used only when rLink has been pushed
#define PUSH_R0_VERY_SLOW_SW_ROM_PATCH_POINT(patch_id)

// Very Slow ROM patch
// trashes r0
// NOTE: VERY_SLOW_SW_ROM_PATCH_POINT should be used only when rLink has been pushed
#define VERY_SLOW_SW_ROM_PATCH_POINT(patch_id)

// Slow ROM patch, preserves registers
#define PUSH_REGS_SLOW_SW_ROM_PATCH_POINT(patch_id, table_slow, num_fast_patch)

// Slow ROM patch, preserves registers (Library version)
#define LIBS_PUSH_REGS_SLOW_SW_ROM_PATCH_POINT(patch_id)

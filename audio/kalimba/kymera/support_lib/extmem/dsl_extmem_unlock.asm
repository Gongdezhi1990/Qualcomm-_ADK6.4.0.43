/**************************************************************************/
/* Copyright (c) 2018-2019 Qualcomm Technologies International, Ltd.      */
/**************************************************************************/

/* These public C functions, declared in extmem.h, are resolved
 * to absolute addresses exported from the patch build.
 */

#include "subsys3_patch0_fw000012B2_map_public.h"

#ifdef DISABLE_PATCH_BUILD_ID_CHECK
.const $_extmem_unlock  PATCH_ENTRY_EXTMEM_UNLOCK;

#else /* DISABLE_PATCH_BUILD_ID_CHECK */

.MODULE $M.download_support_lib.extmem_unlock;
.CODESEGMENT PM;
.MINIM;

$_extmem_unlock:
#ifdef PATCH_ENTRY_EXTMEM_UNLOCK
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_EXTMEM_UNLOCK;
#endif
L_pb_mismatch:
    r0 = 0;
    rts;

.ENDMODULE;

#endif /* DISABLE_PATCH_BUILD_ID_CHECK */


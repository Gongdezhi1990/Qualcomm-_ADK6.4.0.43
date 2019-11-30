/**************************************************************************/
/* Copyright (c) 2018-2018 Qualcomm Technologies International, Ltd.      */
/**************************************************************************/

/* These public C functions, declared in cbops_c.h, are resolved
 * to absolute addresses exported from the patch build.
 */

#include "subsys3_patch0_fw000012B2_map_public.h"

#ifdef DISABLE_PATCH_BUILD_ID_CHECK

.const $_create_mute_op     PATCH_ENTRY_CREATE_MUTE_OP;

#else /* DISABLE_PATCH_BUILD_ID_CHECK */

.MODULE $M.download_support_lib.create_mute_op;
.CODESEGMENT PM;
.MINIM;

$_create_mute_op:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_CREATE_MUTE_OP;

L_pb_mismatch:
    /* Stub: return false; */
    r0 = 0;
    rts;

.ENDMODULE;

#endif /* DISABLE_PATCH_BUILD_ID_CHECK */


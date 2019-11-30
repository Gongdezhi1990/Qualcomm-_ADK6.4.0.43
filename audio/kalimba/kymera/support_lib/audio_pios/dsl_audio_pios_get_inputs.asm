/**************************************************************************/
/* Copyright (c) 2018-2019 Qualcomm Technologies International, Ltd.      */
/**************************************************************************/

/* These public C functions, declared in audio_pios.h, are resolved
 * to absolute addresses exported from the patch build.
 */

#include "subsys3_patch0_fw000012B2_map_public.h"

#ifdef DISABLE_PATCH_BUILD_ID_CHECK
.const $_audio_pios_get_inputs     PATCH_ENTRY_AUDIO_PIOS_GET_INPUTS;

#else /* DISABLE_PATCH_BUILD_ID_CHECK */

.MODULE $M.download_support_lib.audio_pios_get_inputs;
.CODESEGMENT PM;
.MINIM;

$_audio_pios_get_inputs:
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_AUDIO_PIOS_GET_INPUTS;

L_pb_mismatch:
    r0 = 0;
    rts;

.ENDMODULE;

#endif /* DISABLE_PATCH_BUILD_ID_CHECK */


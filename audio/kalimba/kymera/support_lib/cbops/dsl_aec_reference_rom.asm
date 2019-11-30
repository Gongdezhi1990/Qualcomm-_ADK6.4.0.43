/**************************************************************************/
/* Copyright (c) 2018 Qualcomm Technologies International, Ltd.      */
/**************************************************************************/

/* These public C functions, declares that aec reference isn't
 * from rom, so cbops patches will be aware of that.
 */

#include "subsys3_patch0_fw000012B2_map_public.h"
// *****************************************************************************
// MODULE:
//   void set_aec_reference_not_running_from_rom(void)
//
// DESCRIPTION:
//   Make the cbops patch functions aware that aec_reference isn't running from rom
//
// INPUTS:
//   None
//
// OUTPUTS:
//    None
//
// TRASHED REGISTERS:
//    C callable
//
// *****************************************************************************
.MODULE $M.download_support_lib.set_aec_reference_not_running_from_rom;
.CODESEGMENT PM;
.MINIM;
$_set_aec_reference_not_running_from_rom:
#ifndef DISABLE_PATCH_BUILD_ID_CHECK
    // panic if not using the right patch
    r0 = 0;
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if NZ call $error;
#endif // DISABLE_PATCH_BUILD_ID_CHECK
#ifdef PATCH_ENTRY_SET_AEC_REFERENCE_NOT_RUNNING_FROM_ROM
    // if we have patch entry for setting the downloadable flag
    // then go to that patch.
    jump PATCH_ENTRY_SET_AEC_REFERENCE_NOT_RUNNING_FROM_ROM;
#else
   // otherwise quietly return
   rts;
#endif // PATCH_ENTRY_SET_AEC_REFERENCE_NOT_RUNNING_FROM_ROM
.ENDMODULE;

/**************************************************************************/
/* Copyright (c) 2019 Qualcomm Technologies International, Ltd.      */
/**************************************************************************/
/* These public C functions, declared in cbops_c.h, are resolved
 * to absolute addresses exported from the patch build.
 */
#ifdef ENABLE_SW_DAC_DC_OFFSET_COMPENSATION
#include "subsys3_patch0_fw000012B2_map_public.h"
.MODULE $M.download_support_lib.setup_cbops_dc_offset_compensation;
.CODESEGMENT PM;
   .MINIM;
#ifdef PATCH_ENTRY_SETUP_CBOPS_DC_OFFSET_COMPENSATION
#ifdef DISABLE_PATCH_BUILD_ID_CHECK
   .const $_setup_cbops_dc_offset_compensation PATCH_ENTRY_SETUP_CBOPS_DC_OFFSET_COMPENSATION;
#else /* PATCH_ENTRY_SETUP_CBOPS_DC_OFFSET_COMPENSATION */
$_setup_cbops_dc_offset_compensation:
   /* Check patch version */
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump PATCH_ENTRY_SETUP_CBOPS_DC_OFFSET_COMPENSATION;
#endif /* DISABLE_PATCH_BUILD_ID_CHECK */
#else /* PATCH_ENTRY_SETUP_CBOPS_DC_OFFSET_COMPENSATION */
   /* patch doesn't support dc offset compensation,
   * return NULL if setup API is called, so we don't panic.
   */
   $_setup_cbops_dc_offset_compensation:
   $_cbops_configure_dc_offset_op:
   $_cbops_dc_offset_zero_all:
#endif
return_null:
   /* if not matching then just return NULL,
    * so no dc offset op created/added
    */
    r0 = 0;
    rts;
.ENDMODULE;
#endif /* ENABLE_SW_DAC_DC_OFFSET_COMPENSATION */

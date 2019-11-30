/**************************************************************************/
/* Copyright (c) 2018-2018 Qualcomm Technologies International, Ltd.      */
/**************************************************************************/

/* These public C functions, declared in opmgr_op_client_interface.h,
 * are resolved to absolute addresses exported from the patch build.
 */

#include "subsys3_patch0_fw000012B2_map_public.h"

#ifdef DISABLE_PATCH_BUILD_ID_CHECK
.const $_opmgr_op_client_send_message          PATCH_ENTRY_OPMGR_CLIENT_SEND_MESSAGE;
#else /* DISABLE_PATCH_BUILD_ID_CHECK */

.MODULE $M.opmgr_op_client_interface.opmgr_op_client_send_message;
.CODESEGMENT PM;
.MINIM;

$_opmgr_op_client_send_message:
#ifdef PATCH_ENTRY_OPMGR_CLIENT_SEND_MESSAGE
    rMAC = M[$_patched_fw_version];
    Null = rMAC - PATCH_BUILD_ID;
    if EQ jump  PATCH_ENTRY_OPMGR_CLIENT_SEND_MESSAGE; 
#endif
L_pb_mismatch:
    /* Stub: return false; */
    r0 = 0;
    rts;

.ENDMODULE;

#endif /* DISABLE_PATCH_BUILD_ID_CHECK */


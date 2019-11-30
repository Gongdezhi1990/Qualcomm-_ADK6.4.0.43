/**************************************************************************/
/* Copyright (c) 2018-2018 Qualcomm Technologies International, Ltd.      */
/**************************************************************************/

/* These public C functions, declared in cbops_c.h, are resolved
 * to absolute addresses exported from the patch build.
 */

#include "subsys3_patch0_fw000012B2_map_public.h"

/* Patch build ID check is not necessary since
 * create_mute_op would already have checked */

.const $_cbops_mute_reset   PATCH_ENTRY_CBOPS_MUTE_RESET;


/*****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
 ****************************************************************************/
/**
 * \file  patch_private.h
 * \ingroup patch
 *
 * Private header file for the patch component of the audio subsystem
 */

#ifndef __PATCH_PRIVATE_H__
#define __PATCH_PRIVATE_H__

/****************************************************************************
Include Files
*/

#include "patch.h"
#include "pmalloc/pl_malloc.h"
#include "hydra_types.h"
#include "hydra_macros.h"
#include "hydra_cbuff.h"
#include "fileserv/fileserv.h"
#include "fault/fault.h"
#include "id/id.h"
#include "filename_prim.h"
#include "isp_prim.h"
#include "hal_patch.h"
#include "hal_dm_sections.h"

/* 
 * pm_code_limit is defined in crt0.asm.
 * It gets set to the first free PM RAM address 
 * we could use for patches
 */
extern unsigned pm_code_limit;

#endif /* __PATCH_PRIVATE_H__ */

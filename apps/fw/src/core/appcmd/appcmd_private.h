/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Curator CUCMD private definitions.
*/
#ifndef APPCMD_PRIVATE_H
#define APPCMD_PRIVATE_H

#include "hydra/hydra_types.h"
#include "hydra/hydra_macros.h"

#include "appcmd/appcmd.h"

#include "hydra_log/hydra_log.h"
#include "int/int.h"
#include "io/io_map.h"
#ifdef MIB_MODULE_PRESENT
#include "mib/mib.h"
#endif
#include "sched/sched.h"
#include "buffer/buffer.h"
#include "pmalloc/pmalloc.h"
#include "gen/k32/test_tunnel_prim.h"
#include "ipc/ipc.h"
#ifdef SIFLASH_MODULE_PRESENT
#include "siflash/siflash.h"
#endif

/*****************************************************************************
 * Declarations
 ****************************************************************************/

/**
 * Assembly function that pushes appcmd_params[1] values from appcmd_params[2:]
 * into the appropriate registers and calls appcmd_params[0]
 */
void appcmd_call_function(void);

/*****************************************************************************
 * Dependencies.
 ****************************************************************************/

#endif /* APPCMD_PRIVATE_H */

/*
 * Copyright (c) 2016 - 2019 Qualcomm Technologies International, Ltd 
 * 
 */

/**
 * \file
 * Internal header for initialisation code
 */

#ifndef INIT_PRIVATE_H
#define INIT_PRIVATE_H

#include "init/init.h"
#include "excep/excep.h"
#include "hydra_log/hydra_log.h"
#include "pioint/pioint.h"
#include "id/id.h"
#include "appcmd/appcmd.h"
#include "fault/fault.h"
#include "pmalloc/pmalloc.h"
#include "itime/itime.h"
#include "ipc/ipc.h"
#include "memprot/memprot.h"
#if CHIP_HAS_CLEAN_SHALLOW_SLEEP_CLOCK_STOP_THAT_DEFAULTS_OFF
#define IO_DEFS_MODULE_K32_MISC
#include "hal/hal_macros.h"
#endif
#include "cache/cache.h"
#include "portability/portability.h"

/* Remove me! */
#include <operator.h>
/* Remove me! */
#include "assert.h"

#endif /* INIT_PRIVATE_H */

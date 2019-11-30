/**
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
 * 
 * \file  ttp_private.h
 *
 * \ingroup ttp
 * 
 * Private header file for the time-to-play (TTP) module.
 */

#ifndef TTP_PRIVATE_H
#define TTP_PRIVATE_H

#include "ttp.h"
#include "pmalloc/pl_malloc.h"
#include "pl_timers/pl_timers.h"
#include "platform/pl_fractional.h"
#include "platform/pl_assert.h"
#include "opmgr/opmgr_for_ops.h"
#include "opmsg_prim.h"
#include "stream/stream.h"
#include "fault/fault.h"

/* TTP playback module*/
#include "hal/hal_time.h"
#include "panic/panic.h"
#include "buffer/buffer_metadata.h"
#include "hal/hal_time.h"
#include "cbops_mgr/cbops_flags.h"
#include "profiler_c.h"
#include "patch/patch.h"
#include "timestamp_reframe.h"
#include "hydra_modules/mib/mib.h"
#include "audio_log/audio_log.h"

/**
 * Macro used to enable/disable the static debug logging.
 */
#ifndef TTP_PLOTTER
#define TTP_PLOTTER_MACRO(x) ((void)0)
#else
#define TTP_PLOTTER_MACRO(x) (x)
#endif

/**
 * Print the tag error and status. Because debuglog is not good at displaying negative
 * numbers the macro takes care of that too.
 */
#define DBG_PRINT_ERROR_AND_STATUS(TP, ERROR, WARP, STATUS) \
do\
{\
    if (error < 0){\
        TTP_DBG_MSG4("TTP Playback 0x%08x: error = -%8d, warp = 0x%08xFRAC, status (LATE 1 ON_TIME 2 EARLY 3)= %d", (uintptr_t)(TP), -(ERROR), WARP, STATUS);\
    }else{\
        TTP_DBG_MSG4("TTP Playback 0x%08x: error =  %8d, warp = 0x%08xFRAC, status (LATE 1 ON_TIME 2 EARLY 3)= %d", (uintptr_t)(TP), ERROR, WARP, STATUS);\
    }\
} while(0)

#define DBG_PRINT_LATE_ERROR_AND_STATUS(TP, ERROR, STATUS) \
do\
{\
    if (error < 0){\
        TTP_DBG_MSG3("TTP Playback 0x%08x: LATE error = -%8d, status (LATE 1 ON_TIME 2 EARLY 3)= %d", (uintptr_t)(TP), -(ERROR), STATUS);\
    }else{\
        TTP_DBG_MSG3("TTP Playback 0x%08x: LATE error =  %8d, status (LATE 1 ON_TIME 2 EARLY 3)= %d", (uintptr_t)(TP), ERROR, STATUS);\
    }\
} while(0)


#define TTP_WARN_MSG(x)                 L2_DBG_MSG(x)
#define TTP_WARN_MSG1(x, a)             L2_DBG_MSG1(x, a)
#define TTP_WARN_MSG2(x, a, b)          L2_DBG_MSG2(x, a, b)
#define TTP_WARN_MSG3(x, a, b, c)       L2_DBG_MSG3(x, a, b, c)
#define TTP_WARN_MSG4(x, a, b, c, d)    L2_DBG_MSG4(x, a, b, c, d)
#define TTP_WARN_MSG5(x, a, b, c, d, e) L2_DBG_MSG5(x, a, b, c, d, e)

/**
 * TTP debug messages.
 */
#ifdef TTP_DEBUG_VERBOSE
#define TTP_DBG_MSG(x)                 L2_DBG_MSG(x)
#define TTP_DBG_MSG1(x, a)             L2_DBG_MSG1(x, a)
#define TTP_DBG_MSG2(x, a, b)          L2_DBG_MSG2(x, a, b)
#define TTP_DBG_MSG3(x, a, b, c)       L2_DBG_MSG3(x, a, b, c)
#define TTP_DBG_MSG4(x, a, b, c, d)    L2_DBG_MSG4(x, a, b, c, d)
#define TTP_DBG_MSG5(x, a, b, c, d, e) L2_DBG_MSG5(x, a, b, c, d, e)

#else  /* TTP_DEBUG_VERBOSE */

#define TTP_DBG_MSG(x)                 L4_DBG_MSG(x)
#define TTP_DBG_MSG1(x, a)             L4_DBG_MSG1(x, a)
#define TTP_DBG_MSG2(x, a, b)          L4_DBG_MSG2(x, a, b)
#define TTP_DBG_MSG3(x, a, b, c)       L4_DBG_MSG3(x, a, b, c)
#define TTP_DBG_MSG4(x, a, b, c, d)    L4_DBG_MSG4(x, a, b, c, d)
#define TTP_DBG_MSG5(x, a, b, c, d, e) L4_DBG_MSG5(x, a, b, c, d, e)

#endif /* TTP_DEBUG */

#endif /* TTP_PRIVATE_H */


/****************************************************************************
 * Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  fault_private.h
 * \ingroup fault
 *
 * Internal header for groaning quietly
 *
 */

#ifndef FAULT_PRIVATE_H
#define FAULT_PRIVATE_H

#include "fault/fault.h"
#include "hal/hal.h"
#include "sched/bg_int.h"
#include "preserved/preserved.h"
#ifdef INSTALL_HYDRA
#ifndef FAULT_LEAN_AND_MEAN
#include "subreport/subreport.h"
#endif
#endif
#include "int/int.h"
#include "hydra/hydra_macros.h"
#include "panic/panic.h"
#include "itime_kal/itime_kal.h"
#ifdef INSTALL_FAULT_TEST
#include "appcmd/appcmd.h"
#endif
#include "audio_log/audio_log.h"

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
#include "kip_mgr/kip_mgr.h"
#include "adaptor/adaptor.h"
#include "kip_msg_prim.h"
#include "pmalloc/pl_malloc.h"
#endif

#endif /* FAULT_PRIVATE_H */

/****************************************************************************
 * Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file panic_private.h
 * \ingroup panic
 * Internal header for dying noisily
 */

#ifndef PANIC_PRIVATE_H
#define PANIC_PRIVATE_H

#include "panic/panic.h"
#include "fault/fault.h"
#include "preserved/preserved.h"
#ifdef OS_OXYGOS
#include "patch/patch.h"
#endif /* OS_OXYGOS */
#ifdef INSTALL_HYDRA
#ifndef SUBSYSTEM_AUDIO
#include "hal/halint.h"
#include "hal/halauxio.h"
#include "hal/haltime.h"
#else
#include "int/int.h"
#endif
#include "subreport/subreport.h"
#include "reset/reset.h"
#include "pio_debug/pio_debug.h"
#include "hydra_log/hydra_log.h"
#if defined(DESKTOP_TEST_BUILD) || defined(PRODUCTION_BUILD)
#include "crt/crt.h"
#endif
#else /* INSTALL_HYDRA */
#include "audio_log/audio_log.h"
#include "int/int.h"
#endif /* INSTALL_HYDRA */

#ifdef CHIP_BASE_A7DA_KAS
#include "kascmd/kascmd.h"
#endif

#ifdef CHIP_BASE_NAPIER
#include "comms.h"
#endif

#endif /* PANIC_PRIVATE_H */

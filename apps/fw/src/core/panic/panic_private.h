/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Internal header for dying noisily
 */

#ifndef PANIC_PRIVATE_H
#define PANIC_PRIVATE_H


#include "panic/panic.h"
#include "hal/haltime.h"
#include "fault/fault.h"
#include "int/int.h"
#ifdef PRESERVED_MODULE_PRESENT
#ifndef OS_OXYGOS
#include "preserved/preserved.h"
#else /* OS_OXYGOS */
#include "platform/core_library_preserve_block.h"
#endif /* OS_OXYGOS */
#endif
#include "patch.h"
#ifdef SUBREPORT_MODULE_PRESENT
#include "subreport/subreport.h"
#endif
#include "timed_event/rtime.h"
#ifdef RESET_MODULE_PRESENT
#include "reset/reset.h"
#endif
#ifdef PIO_DEBUG_MODULE_PRESENT
#include "pio_debug/pio_debug.h"
#endif
#include "hydra_log/hydra_log.h"
#if defined(PRODUCTION_BUILD)
#include "crt/crt.h"
#endif
#ifdef IPC_MODULE_PRESENT
#include "ipc/ipc.h"
#endif

/**
 * Reports a panic from P0 to the Curator or from P1 to P0.
 * \param id Panic ID.
 * \param t Timestamp.
 * \param arg Panic argument.
 * \param cpu Identifies the cpu this panic is coming from.
 */
void panic_report(panicid id, uint32 t, DIATRIBE_TYPE arg, uint16 cpu);


#endif /* PANIC_PRIVATE_H */

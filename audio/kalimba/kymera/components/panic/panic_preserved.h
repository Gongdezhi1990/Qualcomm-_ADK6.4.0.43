/****************************************************************************
 * Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/

/**
 * \file panic_preserved.h
 * \ingroup panic
 * Preserved memory used for dying noisily
 *
 */

#ifndef PANIC_PRESERVED_H
#define PANIC_PRESERVED_H

#include "panic/panic.h"
#include "hydra/hydra_types.h"
#include "timed_event/timed_event.h"

/**
 * Static preserved data for recording panic status
 */
typedef struct panic_preserved
{
     /** Why we panicked last time */
    panicid last_id;

    /** Argument passed to last panic */
    DIATRIBE_TYPE last_arg;

    /** Time of last panic */
    TIME last_time;
} panic_preserved;
#define PANIC_PRESERVED_DATA(m) PRESERVED_DATA(m, (panic_preserved, panic))


#endif /* PANIC_PRESERVED_H */

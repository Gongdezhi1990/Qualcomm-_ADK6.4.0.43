/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file panicids.h
 * \ingroup panic
 *
 * Reason codes for dying noisily
 *
 */
/**
 * \section panicids_introduction INTRODUCTION
 *
 * This file is audio specific and includes audio subsystem panic codes in
 * addition to the common hydra panicid codes.
 *
 * Sometimes things go wrong, and when they do, it is often impossible
 * (or undesirable) for the software to continue running.  When this
 * happens the panic() function should be called with one of the reason
 * codes from this file.
 *
 * Depending on the platform and software configuration, the reason code
 * may be reported to the host, or it may just be stored in a (preserve)
 * memory location.
 *
 */

#ifndef PANICIDS_H
#define PANICIDS_H

/* Reasons to be cheerless */
typedef enum panicid
{
#include "hydra_panicids.h"
#include "audio_panicids.h"

    /**
     * Do not use this special value - it indicates "not a panic"
     * (placed at the end of the list to prevent some compilers complaining
     * about a superfluous final comma within the enum declaration) */
    PANIC_NONE = 0x00
} panicid;

#endif /* PANICIDS_H */

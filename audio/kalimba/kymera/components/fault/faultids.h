/****************************************************************************
 * Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file faultids.h  -  reason codes for moaning
 * \ingroup fault
 *
 *  Sometimes things go wrong, and when they do it is often reasonable
 *  for the software to continue running.  When this happens the fault_diatribe()
 *  function should be called with one of the reason codes from this file.
 *
 *  Depending on the platform and software configuration, the reason code
 *  may be reported to the host, or it may just be stored in a memory
 *  location somewhere.
 *
*/

#ifndef FAULTIDS_H
#define FAULTIDS_H

/* Reasons to be cheerless. */
typedef enum faultid
{
#include "hydra_faultids.h"
#include "audio_faultids.h"

#ifdef FAULTIDS_EXTRA
    FAULTIDS_EXTRA
#endif

    /*
     * Do not use this special value - it indicates "not a fault".  (It
     * has been placed at the end of the list to prevent some compilers
     * complaining about a superfluous final comma within the enum
     * declaration.)
     */
    FAULT_NONE = 0x0000
} faultid;

#endif /* FAULTIDS_H */

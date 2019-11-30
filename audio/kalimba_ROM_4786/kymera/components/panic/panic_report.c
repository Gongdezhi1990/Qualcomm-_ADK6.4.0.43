/****************************************************************************
 * Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file panic_report.c
 * \ingroup panics
 *
 * Report the last panic ID to the host
 */

#include "panic/panic_private.h"

/**
 * Report the last panic ID to the host
 */
void panic_report(void)
{
    volatile preserved_struct *this_preserved = (volatile preserved_struct *)&preserved;
#ifdef AUDIO_SECOND_CORE
    if( hal_get_reg_processor_id() != 0 )
    {
        this_preserved = (volatile preserved_struct *)&preserved1;
    }
#endif//AUDIO_SECOND_CORE
    fault_diatribe(FAULT_HYDRA_PANIC, (uint16) this_preserved.panic.last_id);
}

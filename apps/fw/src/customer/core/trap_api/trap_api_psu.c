/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Implementation for some PSU traps from the PSU trapset.
 */

#include "trap_api/trap_api_private.h"
#include "pio/pio.h"

#define LOG_PREFIX "PSU TRAP: "


bool PsuGetVregEn(void)
{
    bool state;

#if defined(DESKTOP_TEST_BUILD)
    state = FALSE;
#else
    state = pio_get_level(SYS_CTRL_IN_PAD_IDX);
#endif
    L4_DBG_MSG1(LOG_PREFIX "polling VREG EN, state %d", state);

    return state;
}

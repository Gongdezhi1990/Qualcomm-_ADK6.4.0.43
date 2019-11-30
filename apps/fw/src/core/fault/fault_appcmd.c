/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Implementation for fault appcmd interface.
 */

#ifdef APPCMD_MODULE_PRESENT

#include "fault/fault_private.h"
#include "fault/fault_appcmd.h"
#include "hal/haltime.h"


volatile TIME fault_test_last_groan;

#define IMMEDIATE_PUBLISHING_DELAY ((INTERVAL)500)
#define DELAYED_PUBLISHING_DELAY \
                 time_add(PUBLISHING_DELAY, IMMEDIATE_PUBLISHING_DELAY)

static bool fault_test_check_publishing_delay(TIME deadline)
{
    TIME extended_deadline = time_add(deadline, (TIME)100U*MILLISECOND);
    INTERVAL late_by;

    /* Busy wait for a generous version of the publishing delay */
    /*lint -e722 */
    while (time_eq(fault_test_last_groan, 0) &&
            time_lt(hal_get_time(), extended_deadline));

    /* The interrupt really ought to have fired by now, but we don't want to
     * get confused if it hasn't and it fires in the middle of this block */
    block_interrupts();
    if (time_eq(fault_test_last_groan, 0))
    {
        L0_DBG_MSG1("It is now 0x%x and no groan has been seen",
                    hal_get_time());
        L0_DBG_MSG1("(just to check, fault_test_last_groan = 0x%x)",
                fault_test_last_groan);
        unblock_interrupts();
        L0_DBG_MSG("Raised fault not published!");
        return FALSE;
    }

    /* Was the fault published on time? */
    late_by = time_sub(fault_test_last_groan, deadline);
    unblock_interrupts();

    if (late_by > 0)
    {
        L0_DBG_MSG1("Raised fault published late: over deadline by %ld us",
                    late_by);
        return FALSE;
    }
    return TRUE;
}

static bool fault_test_raise_a_fault(faultid id, bool first_time)
{
    TIME raised_at;
    uint16f i;

    raised_at = hal_get_time();
    fault_test_last_groan = (TIME)0;
    L0_DBG_MSG1("Raised fault at time 0x%x", raised_at);

    fault_diatribe(id, 0);

    for (i = 0; i < NFI; i++)
    {
        if (faultdb[i].f == id)
        {
            break;
        }
    }
    if (i == NFI)
    {
        /* Fault didn't appear in database */
        L0_DBG_MSG("Raised fault didn't appear!");
        return FALSE;
    }

    L0_DBG_MSG("Raised fault appeared");

    if (!fault_test_check_publishing_delay(time_add(raised_at,
                                   first_time ? IMMEDIATE_PUBLISHING_DELAY :
                                                DELAYED_PUBLISHING_DELAY)))
    {
        return FALSE;
    }
    L0_DBG_MSG1("Raised fault published after %ld us",
                time_sub(fault_test_last_groan, raised_at));
    return TRUE;
}

APPCMD_RESPONSE fault_test(APPCMD_TEST_ID command,
                           uint32 * params,
                           uint32 * result)
{
    UNUSED(command);
    UNUSED(params);

    /* Raise some faults.  Check that they appear in the database 'n' stuff */


    if (!fault_test_raise_a_fault(FAULT_HYDRA_PANIC, TRUE))
    {
        *result = 0;
        return APPCMD_RESPONSE_SUCCESS;
    }

    if (!fault_test_raise_a_fault(FAULT_HYDRA_PANIC, FALSE))
    {
        *result = 0;
        return APPCMD_RESPONSE_SUCCESS;
    }

    if (!fault_test_raise_a_fault(FAULT_HYDRA_DIVIDE_BY_ZERO, TRUE))
    {
        *result = 0;
        return APPCMD_RESPONSE_SUCCESS;
    }

    *result = 1;
    return APPCMD_RESPONSE_SUCCESS;
}


#endif /* APPCMD_MODULE_PRESENT */

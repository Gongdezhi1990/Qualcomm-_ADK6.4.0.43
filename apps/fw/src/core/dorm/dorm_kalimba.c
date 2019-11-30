/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

/****************************************************************************
Include Files
*/
#include "dorm/dorm_private.h"
#include "int/int.h"
#include "pl_timers/pl_timers.h"
#include "sched_oxygen/sched_oxygen.h"
#include "assert.h"
#include "ipc/ipc.h"
#include "mmu/mmu.h"
#include "hal/hal_cross_cpu_registers.h"

PRESERVE_ENUM_FOR_DEBUGGING(dorm_state)

/****************************************************************************
Private Types
*/
enum best_case_sleep {
    best_case_no_sleep,
    best_case_shallow_sleep,
    best_case_deep_sleep
};

/****************************************************************************
Private Function Prototypes
*/
static enum best_case_sleep sleep_depth_to_consider(void);
static bool is_worth_deep_sleeping(TIME latest, bool deadline);
static void safe_enable_deep_sleep(TIME earliest, TIME latest, bool deadline);

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private Function Definitions
*/

/****************************************************************************
Public Function Definitions
*/

/* Configure the way we sleep */
void init_dorm(void) 
{
}

/* dorm_sleep */

/** Main entry point for when the processor background is idle. */
void dorm_sleep_sched(void)
{
    switch(sleep_depth_to_consider())
    {
    case best_case_no_sleep:
        return;
    case best_case_deep_sleep:
    {
        /* Deep sleep is a possibility so we need to check further. */
        TIME earliest, latest;
        bool deadline;

        deadline = sched_get_sleep_deadline(&earliest, &latest);

        if (is_worth_deep_sleeping(latest, deadline))
        {
            safe_enable_deep_sleep(earliest, latest, deadline);
            return;
        }

        /* No go for deep sleep, we'll have to shallow sleep. */
        break;
    }
    default:
        /* It must be shallow sleep in this case. */
        break;
    }
    enter_shallow_sleep();

}

void dorm_shallow_sleep(TIME latest)
{
    uint16 kip_flags = dorm_get_combined_kip_flags();

    UNUSED(latest);

    if ((kip_flags & DORM_STATE_NO_SHALLOW) == 0)
    {
        enter_shallow_sleep();
    }
}


/**
 * \brief Decides which sleep level apps should consider.
 *
 * \note The point of this function is to allow us to short-circuit expensive
 *       calculations. Hence this should include only really cheap tests. Test
 *       should be ordered to have the test with the average least cost first.
 *       If this function says that deep sleep is worth considering the you
 *       still need to call is_worth_deep_sleeping() which can do the
 *       expensive tests. If you want both then call should_deep_sleep().
 *
 * \return best_case_no_sleep if we're not allowed to sleep at all,
 *         best_case_shallow_sleep if we're definitely not allowed to deep
 *         sleep or best_case_deep_sleep if deep sleep is a possibility.
 */
static enum best_case_sleep sleep_depth_to_consider(void)
{
    uint16 kip_flags = dorm_get_combined_kip_flags();

    if ((kip_flags & DORM_STATE_NO_SHALLOW) != 0)
    {
        return best_case_no_sleep;
    }

    if ((kip_flags & DORM_STATE_NO_DEEP) != 0)
    {
        return best_case_shallow_sleep;
    }

    return best_case_deep_sleep;
}

/**
 * \brief Given that we could deep sleep, is it worth it?
 *
 * \note This completes the tests for deep sleep started in
 *       sleep_depth_to_consider(). You must call that function before
 *       this one and proceed to this one only if that function returns
 *       best_case_deep_sleep. If you want both in one go then call
 *       should_deep_sleep();
 *
 * \return TRUE if we should proceed with deep sleeping, FALSE if it's not
 *         worth it.
 */
static bool is_worth_deep_sleeping(TIME latest, bool deadline)
{
    TIME earliest_deep_sleep = time_add(get_time(), DEEP_SLEEP_MIN_TIME);
    if (deadline && time_gt(earliest_deep_sleep, latest))
    {
        L4_DBG_MSG("Not enough time to deep sleep");
        return FALSE;
    }

    return TRUE;
}

/**
 * \brief P1 sleep procedure
 * P1 delegates P0 for any deep sleep communication to curator
 * \param earliest the earliest time P1'd like to be woken. Will be
 * ignored if deadline parameter is set to FALSE.
 * \param latest the deadline by which P1 must be woken. Will be
 * ignored if deadline parameter is set to FALSE.
 * \param deadline TRUE if deep sleep has been requested with deadline,
 * FALSE otherwise.
 */
static void safe_enable_deep_sleep(TIME earliest, TIME latest, bool deadline)
{
    /* P1 sets the deep sleep registry and let P0 drive the deep sleep through curator. */
    hal_set_reg_proc_deep_sleep_en(1);

    /* Use special values if deadline flag is not set */
    if (!deadline)
    {
        earliest = (TIME) DORM_EARLIEST_WAKEUP_TIME_NO_DEADLINE;
        latest =  (TIME) DORM_LATEST_WAKEUP_TIME_NO_DEADLINE;
    }

    ipc_send_p1_deep_sleep_msg(TRUE, earliest, latest);

    enter_shallow_sleep();

    /* Reset deep sleep registry and inform P0 no more deep sleep is required. */
    hal_set_reg_proc_deep_sleep_en(0);
    ipc_send_p1_deep_sleep_msg(FALSE, 0L, 0L);
}

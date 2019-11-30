/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//**
 * \file pl_interrupt.c
 * \ingroup platform
 *
 * C implementation of platform interrupt operations for desktop test builds
 *
 ****************************************************************************/

/* This is only needed for host-based unit tests */
#ifdef DESKTOP_TEST_BUILD

/****************************************************************************
Include Files
*/
#include "platform/pl_interrupt.h"
#include "pl_timers/pl_timers_private.h"
#include "types.h"

/****************************************************************************
Private Macro Declarations
*/

/****************************************************************************
Private Type Declarations
*/
#if defined(CHIP_GORDON)
/* This is an enum on Amber, Rick (and Crescendo?), but a raw uint24 on Gordon */
typedef uint24 int_sources_list;
#endif /* CHIP_GORDON */


/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Variable Definitions
*/
volatile unsigned  INT_UNBLOCK = 1;
#ifdef CHIP_GORDON
volatile int_sources_list  INT_SOURCE = 0;
#else /* CHIP_GORDON */
volatile uint32 INT_SOURCE = 0;
#endif /* CHIP_GORDON */

int PlInterruptActive = 0;


/****************************************************************************
Private Function Prototypes
*/

/****************************************************************************
Private Function Definitions
*/


/****************************************************************************
Public Function Definitions
*/

/**
 * \brief Disable all interrupts
 *
 * \note Dummy function included in non-kalimba builds
 */
void interrupt_block( void )
{
    /* Blank function just return */
}

/**
 * \brief Enable all interrupts
 *
 * \note Dummy function included in non-kalimba builds
 */
void interrupt_unblock( void )
{
    /* Blank function just return */
}

void test_run_timers(void)
{
    /* If there are any timers waiting to expire, wind on TIMER_TIME until we
     * hit them. */
    TIME next_timer_expiry;
    if( timers_get_next_event_time(&next_timer_expiry) )
    {
        PlRunTimers(next_timer_expiry);
    }
}

/**
 * \brief Function to make sure that shallow sleep enable is done correctly
 *
 * \note Simple function included in non-kalimba builds
 */
void safe_enable_shallow_sleep(void)
{
    test_run_timers();
}

#endif /* DESKTOP_TEST_BUILD */

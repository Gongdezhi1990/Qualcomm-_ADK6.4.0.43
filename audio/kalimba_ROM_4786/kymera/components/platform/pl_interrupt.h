/**************************************************************************** 
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//** 
 * \defgroup platform Kalimba Platform
 *
 * \file pl_interrupt.h
 * \ingroup platform
 *
 * Header file for kalimba interrupt functions
 *
 ****************************************************************************/

#if !defined(PL_INTERRUPT_H)
#define PL_INTERRUPT_H

/* PUBLIC FUNCTION PROTOTYPES ***********************************************/

extern void interrupt_initialise(void);

extern void interrupt_block(void);

extern void interrupt_unblock(void);

extern void interrupt_register(int int_source, int int_priority, void (*IntFunction) (void));

extern void safe_enable_shallow_sleep(void);

#ifdef DESKTOP_TEST_BUILD
extern void test_run_timers(void);
#endif

#endif /* PL_INTERRUPT_H */



/**************************************************************************** 
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//** 
 * \file pl_assert.h
 * \ingroup platform
 *
 * Definition of assert macros
 *
 ****************************************************************************/

#if !defined(PL_ASSERT_H)
#define PL_ASSERT_H

/****************************************************************************
Include Files
*/

#include "platform/pl_trace.h"
#include "assert.h"
#ifdef DESKTOP_TEST_BUILD
#include <stdio.h>
#endif

/****************************************************************************
Public Macro Declarations
*/

/**
 * Use our own assert function that doesn't take any
 * parameters to save code space 
 */

#ifdef DESKTOP_TEST_BUILD
#define PL_ASSERT(cond) (!(cond) ? printf("Warning : Assertion failed in %s : Line %d\n", __FILE__, __LINE__) : (void) 0)
#else
/* To compile out this assert define NDEBUG */
#define PL_ASSERT(cond) (assert(cond))
#endif

#define COMPILE_TIME_ASSERT(expr, msg) struct compile_time_assert_ ## msg { \
    int compile_time_assert_ ## msg [1 - (!(expr))*2]; \
}

/****************************************************************************
Public Type Declarations
*/

/****************************************************************************
Global Variable Definitions
*/

/****************************************************************************
Public Function Prototypes
*/

extern void exit( int);

extern void abort( void);

#endif /* TEMPLATE_H */

/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/**
 * \file pbc.h
 * Programming by contract (PBC) support macros.
 *
 * If you #include this in individual source files you can
 * override -D NDEBUG just before the #include to have localised
 * control over PBC.
 *
 * Example:-
 * \verbatim

    ** Control PBC in this file **
    #if defined(NDEBUG) && defined(ENABLE_PBC_IN_MY_MODULE)
    #undef    NDEBUG
    #endif

    #include "include/pbc.h"

\endverbatim
 *
 */
#ifndef PBC_H
#define PBC_H

#include "assert.h"

#define assert_precondition(c)  assert(c)
#define assert_postcondition(c) assert(c)
#define assert_invariant(c)     assert(c)
#define assert_not_reached()    /*lint --e{774} */ assert(FALSE)

#endif /* !PBC_H */

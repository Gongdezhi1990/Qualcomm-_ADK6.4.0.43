/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_test.h
 * \ingroup rate_lib
 *
 * This file contains declarations which are only exported from the module
 * to its unit tests, and are otherwise private
 */

#ifndef RATE_RATE_TEST_H
#define RATE_RATE_TEST_H

#if defined(UNIT_TEST_BUILD)  ||defined(DESKTOP_TEST_BUILD)

#include "rate_types.h"

/****************************************************************************
 * Function Declarations
 */

/* From rate_compare.c */
#if RATE_COMPARE_ALG==3
bool rate_compare_calc_offset(TIME t_ref, TIME t_fb,
                              const RATE_MEASUREMENT* fb,
                              int* c_int_p, int* c_frac_p);
#endif /* RATE_CONTROL_ALG==3 */

#endif /* A test build */

#endif /* RATE_RATE_TEST_H */

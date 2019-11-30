/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  source_sync_test.c
 * \ingroup  capabilities
 *
 *  src_sync operator
 *
 * This file packages and exports some internal functions to a test harness.
 * In particular the inline functions for time unit conversions would not
 * be easily accessible in an unit test build otherwise.
 * There are no references to these functions in regular code, so the linker
 * should discard them.
 */
#include "types.h"

/****************************************************************************
Declarations of functions shared with tester
*/
extern unsigned src_sync_test_usec_to_sec_frac(unsigned usec);
extern unsigned src_sync_test_samples_to_time(unsigned n, unsigned ts);
extern unsigned src_sync_test_samples_to_usec(unsigned num_samples, unsigned inv_sample_rate);


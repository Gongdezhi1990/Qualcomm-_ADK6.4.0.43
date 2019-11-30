/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#ifndef CACHE_H_
#define CACHE_H_
#include "hydra/hydra_types.h"


#ifdef ENABLE_CACHE_TEST_LARGE_CODE
/**
 * Test function to run lots of huge code to check that the cache copes
 */
void cache_test(void);

/**
 * Constant value used in test to check dm fetches through cache
 */
extern const uint32 cache_const_mask[20];
#endif /* ENABLE_CACHE_TEST_LARGE_CODE */

#endif /* CACHE_H_ */

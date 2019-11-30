/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#ifndef PATCH_H_
#define PATCH_H_

/**
 * \file
 * Dummy patch header for quietly removing software patch points from Apps
 * builds of common code.
 */

#define patch_fn(x) (void)0
#define patch_fn_shared(x) (void)0

#endif /* PATCH_H_ */

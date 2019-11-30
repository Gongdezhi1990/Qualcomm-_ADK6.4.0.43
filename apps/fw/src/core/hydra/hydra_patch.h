/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#ifndef HYDRA_PATCH_H
#define HYDRA_PATCH_H

#if defined(CHIP_TEST_BUILD) || (defined(DESKTOP_TEST_BUILD) && !defined(HYDRA_SW_PATCH_DISABLE))
/* Mechanism is disabled in unit testing */
#define HYDRA_SW_PATCH_DISABLE
#endif

#ifdef HYDRA_SW_PATCH_DISABLE


/** Disable all patch points with a return value and an argument */
#define HYDRA_SW_PATCH_POINT_RET_ARG(x,y) ((void)y,0U)
/** Disable all patch points with a return value and no argument */
#define HYDRA_SW_PATCH_POINT_RET(x)     (0U)
/** Disable all patch points with no return value and an argument. */
#define HYDRA_SW_PATCH_POINT_ARG(x,y)   ((void)y)
/** Disable all patch points with no return value or argument. */
#define HYDRA_SW_PATCH_POINT(x)         ((void)(0U))

/** All patch points are disabled */
#define HYDRA_SW_PATCH_POINT_ACTIVE(x)  (0U)

/** All patch points with alternatives use the simple variant */
#define HYDRA_SW_PATCH_SIMPLE (1)

/*@}*/

#else

/*
 * Redirect to the appropriate file.
 */
#include "hydra_sw_patch/hydra_sw_patch.h"

#endif

#endif /* HYDRA_PATCH_H */

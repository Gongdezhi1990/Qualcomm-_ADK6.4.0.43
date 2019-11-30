/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
 ***************************************************************************/
#ifndef HAL_DM_SECTIONS_H
#define HAL_DM_SECTIONS_H

#if !defined(__GNUC__)

/* The sections are valid on all KAL_ARCH4 platforms that use kld. */
#ifdef KAL_ARCH4

#define DM_SHARED       _Pragma("datasection DM_SHARED")
#define DM_SHARED_ZI    _Pragma("datasection DM_SHARED_ZI")
#define DM_P0_RW        _Pragma("datasection DM1_P0_RW")
#define DM_P0_RW_ZI     _Pragma("datasection DM1_P0_RW_ZI")
#define DM_P1_RW        _Pragma("datasection DM2_P1_RW")
#define DM_P1_RW_ZI     _Pragma("datasection DM2_P1_RW_ZI")
#define DM1_DEBUG       DM_P1_RW_ZI
#define DM2_DEBUG       DM_P0_RW_ZI

#else /* KAL_ARCH4 */

#define DM1_ALLOC   _Pragma("datasection DM1_ZI")
#define DM2_ALLOC   _Pragma("datasection DM2_ZI")
#define DM_SHARED
#define DM_SHARED_ZI
#define DM_P0_RW
#define DM_P0_RW_ZI
#define DM_P1_RW
#define DM_P1_RW_ZI
#define DM1_DEBUG           DM1_ALLOC
#define DM2_DEBUG           DM2_ALLOC

#endif /* KAL_ARCH4 */

#else /* __GNUC__ */

#define DM_SHARED
#define DM_SHARED_ZI
#define DM_P0_RW
#define DM_P0_RW_ZI
#define DM_P1_RW
#define DM_P1_RW_ZI
#define DM1_DEBUG
#define DM2_DEBUG

#endif /* __GNUC__ */

#endif /* HAL_DM_SECTIONS_H */

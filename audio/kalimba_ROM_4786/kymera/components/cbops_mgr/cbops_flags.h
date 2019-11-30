/* *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// 
// ****************************************************************************/
/*
 * \file cbops_flags.h
 * \ingroup cbops_mgr
 *
 * Shared definitions for the cbops that can be instantiated for the purposes
 * of audio format conversion, dc-remove etc.
 *
 ****************************************************************************/

#if !defined(CBOPS_FLAGS_H)
#define CBOPS_FLAGS_H

/**
 * These flags are used to indicate which cbops should be added to the cbops chain.
 */
#define CBOPS_COPY_ONLY                    0
#define CBOPS_DC_REMOVE                    (1 << 0)
#define CBOPS_U_LAW                        (1 << 1)
#define CBOPS_A_LAW                        (1 << 2)
#define CBOPS_RATEADJUST                   (1 << 3)
#define CBOPS_SHIFT                        (1 << 4)
#define CBOPS_MIX                          (1 << 5) /* two_to_one_chan_copy */
#define CBOPS_SPLIT                        (1 << 6) /* one_to_two_chan_copy */
#define CBOPS_UPSAMPLE                     (1 << 7) /* DON'T USE upsample by 2 operator TODO_CONSIDER_DELETE development only !!!A */
#define CBOPS_DISCARD                      (1 << 8)
#define CBOPS_UNDERRUN                     (1 << 9)
#define CBOPS_LAST_FLAG                    CBOPS_UPSAMPLE /* Need to be updated if a new flag is added! */

#endif /* CBOPS_FLAGS_H */


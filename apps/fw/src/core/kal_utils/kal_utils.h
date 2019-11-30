/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
Copyright (c) 2008 - 2016 Qualcomm Technologies International, Ltd.
  
*/
#ifndef KAL_UTILS_H
#define KAL_UTILS_H

/****************************************************************************
Include Files
*/
#include "hydra/hydra_types.h"

/****************************************************************************
Public Macro Declarations
*/

/**
 * Macro to find the highest set bit.
 *
 * \note
 * Assumes x is an unsigned "natural word", and is greater than 0
 * Also due to how signdet works Mask cannot have its MSB set.
 *
 * \par
 * CALLER MUST ENSURE THESE RESTRICTIONS ARE MET
 */
#define MAX_BIT_POS_31(x) ((UINT_BIT - 2) - PL_SIGNDET(x))

/**
 * Macro to find the highest set bit.
 *
 * Unlike MAX_BIT_POS_31 this macro works if the MSB is set.
 *
 * Assumes x is an unsigned "natural word", and is greater than 0.
 */
#define MAX_BIT_POS(x) ((uint8)(((x) & (1u << (UINT_BIT-1))) ? \
                           (UINT_BIT-1) : MAX_BIT_POS_31(x)))

/****************************************************************************
Public Type Declarations
*/

/****************************************************************************
Global Variable Definitions
*/

/**
 * Maps to SIGNDET instruction on Kalimba - finds the number of redundant sign
 * bits. Currently we cant map to a single instruction - instead call a function
 * in pl_intrinsics.asm that just does this
 */
#define PL_SIGNDET(m) pl_sign_detect_asm((int)m)

/****************************************************************************
Public Function Prototypes
*/

extern unsigned int pl_sign_detect_asm(int input);

/**
 * @brief The first potential trigger point for instruction trace hardware.
 *
 * This function does nothing except provide an address for triggering starting
 * or stopping the instruction trace hardware.
 *
 * This function should be present for instruction tracing on Apps p0 and p1.
 *
 * @warning Renaming this function should be avoided as pylib depends on it for
 * the start/stop tracing feature. Other subsystems, e.g. Audio, may also be
 * using this name.
 */
extern void tracepoint_trigger_1(void);

/**
 * @brief The second potential trigger point for instruction trace hardware.
 *
 * This function does nothing except provide an address for triggering starting
 * or stopping the instruction trace hardware.
 *
 * This function should be present for instruction tracing on Apps p0 and p1.
 *
 * @warning Renaming this function should be avoided as pylib depends on it for
 * the start/stop tracing feature. Other subsystems, e.g. Audio, may also be
 * using this name.
 */
extern void tracepoint_trigger_2(void);

#endif   /* KAL_UTILS_H */

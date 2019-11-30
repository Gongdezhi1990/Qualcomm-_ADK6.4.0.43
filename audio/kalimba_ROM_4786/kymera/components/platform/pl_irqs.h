/**************************************************************************** 
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//** 
 * \file pl_irqs.h
 * \ingroup platform
 *
 * Definitions for interrupt handler code
 *
 *
 ****************************************************************************/

#if !defined(PL_INT_H)
#define PL_INT_H

/****************************************************************************
Include Files
*/

/****************************************************************************
Public Macro Declarations
*/

/****************************************************************************
Public Type Declarations
*/

/**
 * Enum for interrupt priorities 
 */
typedef enum
{
    PL_IRQ_LOW_PRIORITY = 1,
    PL_IRQ_MED_PRIORITY,
    PL_IRQ_HIGH_PRIORITY,
    PL_IRQ_NUM_PRIORITIES = PL_IRQ_HIGH_PRIORITY
}tPlIrqPriorities;

/****************************************************************************
Global Variable Definitions
*/

/****************************************************************************
Public Function Prototypes
*/

#endif /* PL_INT_H */

/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2009 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//** 
 * \file pl_trace_masks.h
 * \ingroup platform
 *
 * Defintitions of trace masks and bitfields for all modules using trace
 *
 *
 * NOTES : Trace masks used within various modules, and default values for these masks
 *
 *         To add a new trace mask, add to enum for the module.
 *         Stick to the naming convention:
 *
 *         TR_*MOD_ID*_*TRACE_NAME*
 *
 *         Trace masks  must have incrementing indices up to 23. If more than
 *         24 are needed, a second bitfield variable must be added and the
 *         PL_CHECK_MASK macro in the local *mod_id*_trace.h file of the module
 *         must be adapted.
 *
 *         Defined globally so applications can set the trace level in the
 *         components it uses.
 *
 *
 ****************************************************************************/

#if !defined(PL_TRACE_MASKS_H)
#define PL_TRACE_MASKS_H

/****************************************************************************
Include Files
*/

#include "types.h"

/****************************************************************************
Public Macro Declarations
*/

/****************************************************************************
Public Type Declarations
*/

/**
 * Trace masks. 
 * TR_PL_XXX masks are for trace in platform code only.
 * For other modules, add one or more new masks to the list.
 */
typedef enum tPlTraceMasksTag
{
    TR_PL_PUT_MESSAGE = 0,
    TR_PL_GET_MESSAGE,
    TR_PL_MALLOC, /**< Mallocs */
    /**
     * Traces that a malloc was made that would have fitted into a given pool,
     * but this pool has no free blocks. Malloc may succeed but use larger block
     */
    TR_PL_MALLOC_POOL_EMPTY, 
    TR_PL_MALLOC_FAIL,  /**< Malloc fails */
    TR_PL_FREE, /**< Frees */
    TR_PL_TEST_TRACE,  /**< Generic mask for unit test code */
    /**
     * Traces scheduler control flow
     */
    TR_PL_SCHED_TASK_CREATE,
    TR_PL_SCHED_TASK_RUN,
    TR_PL_SCHED_TASK_DELETE,
    /**
     * Component-specific mask values start here
     */
    TR_CBUFFER,
    TR_STREAM,
    TR_OPMGR,
    TR_DUALCORE,

    /**
     * Note if > 24 must modify PL_CHECK_MASK macro and add 2nd BF variable 
     */
    TR_PL_NUM_MASKS 
}tPlTraceMasks;

#define TR_PL_TRACE_MASK_DEFAULT  ((1 << TR_PL_MALLOC_FAIL) |  \
                                   (1 << TR_PL_MALLOC_POOL_EMPTY))


/****************************************************************************
Global Variable Definitions
*/

/** Bitfield variable declarations for each module using trace */
extern unsigned int PlTraceMaskBitField;

/* Declaration of bitfield used to turn on/off various TE trace masks.
 * Not used if trace is not compiled in for this module */

/****************************************************************************
Public Function Prototypes
*/

#endif /* PL_TRACE_MASKS_H */

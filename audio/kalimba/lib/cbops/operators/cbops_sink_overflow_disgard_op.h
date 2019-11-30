/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file sink_overflow_disgard.h
 * \ingroup cbops
 *
 */

#ifndef _CBOPS_SINK_OVRFLW_DROP_OP_H_
#define _CBOPS_SINK_OVRFLW_DROP_OP_H_

#include "types.h"

/****************************************************************************
Public Type Declarations
*/

/** Structure of the multi-channel sink overflow cbops specific data */
typedef struct sink_overflow_disgard
{
   unsigned min_space;
   unsigned num_drops;
} cbops_sink_overflow_disgard_op;

/****************************************************************************
Public Variable Definitions
*/
/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_sink_overflow_disgard_table[];

#endif /* _CBOPS_UNDERRUN_COMP_OP_H_ */

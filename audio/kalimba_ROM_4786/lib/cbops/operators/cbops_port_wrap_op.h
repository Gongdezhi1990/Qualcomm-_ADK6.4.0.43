/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_port_wrap_op.h
 *
 * \ingroup cbops
 *
 * Monitor 1 or 2 output buffers.  The 2nd buffer may be NULL (inactive).
 * Limit amount of data in output to between max_advance and 2*max_advance.
 * Also, ensure that at least max_advance is available in the output each
 * iteration.   If not, insert zeros to keep output fed.
 *
 * Note:  The insertion will only occur if insufficient data was produced
 *        by other operators in the cbops graph.
 */


#ifndef CBOPS_PORT_WRAP_H
#define CBOPS_PORT_WRAP_H
/****************************************************************************
Include Files
*/
#include "buffer/cbuffer_c.h"

/****************************************************************************
Public Type Declarations
*/

typedef struct port_wrap_op{
   /* Channel-independent parameters */
    unsigned    buffer_adj;     /**< Adjustment to account for abiguity in reading amount of data in buffer  */
    unsigned    max_advance;    /**< Threshold for latency control */
    unsigned    wrap_count;     /**< Total number of insertions.  Status - NOTE: it used to count insertions
                                     across both channels, now keeping same logic for up to N channels.
                                     Otherwise this is to be made into channel-specific parameter.*/
}cbops_port_wrap_op;

/****************************************************************************
Public Variable Definitions
*/

/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_port_wrap_table[];


#endif /* CBOPS_PORT_WRAP_H */



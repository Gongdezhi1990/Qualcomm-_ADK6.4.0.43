/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_rate_monitor_op.h
 *
 * \ingroup cbops
 *
 * Monitor rate for application for HW and SW rate adjustment
 *  SW rate adjustment differs from HW rate adjustment in that the
 *  rate into this operator is fixed while HW adjusment changes.   
 *  Therefore, for SW rate adjustment extra averaging
 *  can be applied to stablize the rate adjustment.
 */

#ifndef CBOPS_RATE_MONITOR_H
#define CBOPS_RATE_MONITOR_H

#if 0
#include "buffer/cbuffer_c.h"
#endif

/****************************************************************************
Public Type Declarations
*/

typedef struct rate_monitor_op{
    unsigned    period_per_second;  /**< number of iterations per second (1.0/period) */
    unsigned    collect_periods;    /**< number of periods to collect data between updates */
    unsigned    expected_acum;      /**< expected accumulation */
    unsigned    alpha_limit;        /**< Limit to evaluate validity of data collection */
    unsigned    average_io_rate;    /**< averaging constant */

    unsigned    monitor_mode;       /**< HW/SW mode)  */

    unsigned    accumulator;        /**< internal use */
    unsigned    period_counter;     /**< internal use */
    unsigned    stall;              /**< internal use */
    unsigned    last_accumulator;   /**< internal use */
    unsigned    current_alpha_index;    /**< internal use */
    unsigned    measured_rate;          /**< computed rate (est/accum)     */
    unsigned    measurement_complete; /* Flag indicating H/W measurement complete */
}cbops_rate_monitor_op;


/****************************************************************************
Public Variable Definitions
*/

/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_rate_monitor_table[];


#endif /* CBOPS_RATE_MONITOR_H */




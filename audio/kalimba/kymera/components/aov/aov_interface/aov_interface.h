/****************************************************************************
 * Copyright (c) 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  aov_interface.h
 * \ingroup  aov
 *
 * AOV interface to Operator Clients. <br>
 * This file contains the AOV APIs that can be used by an operator client. <br>
 */

#ifndef AOV_INTERFACE_H
#define AOV_INTERFACE_H

/****************************************************************************
Include Files
*/

#include "types.h"
#include "opmgr/opmgr_for_ops.h"
/****************************************************************************
Public Type Declarations
*/
/* Note: This type must be kept in line with its counterpart
   on the clock manager. */
typedef enum
{
    AOV_IF_CPU_CLK_EXT_LP_CLOCK = 1,
    AOV_IF_CPU_CLK_VERY_LP_CLOCK = 2,
    AOV_IF_CPU_CLK_LP_CLOCK = 3,
    AOV_IF_CPU_CLK_VERY_SLOW_CLOCK = 4,
    AOV_IF_CPU_CLK_SLOW_CLOCK = 5,
    AOV_IF_CPU_CLK_BASE_CLOCK = 6,
    AOV_IF_CPU_CLK_TURBO = 7,
    AOV_IF_CPU_CLK_TURBO_PLUS = 8
} AOV_IF_CPU_CLK;

/****************************************************************************
Public Constant Declarations
*/

/****************************************************************************
Public Macro Declarations
*/

/****************************************************************************
Public Variable Declarations
*/

/****************************************************************************
Public Function Definitions
To be used for the development of an operator client.
*/


/**
 * \brief Send a request to the AOV (if present) to change the cpu clock to a
 *        custom value.
 *
 * \param op_data OPERATOR_DATA of the requesting operator client
 *
 * \param freq Requested clock frequency value.
 *
 * \return TRUE if the message has been successfully sent
 */
bool aov_request_custom_clock(OPERATOR_DATA *op_data, AOV_IF_CPU_CLK cpu_clk);

/**
 * \brief Send a request to the AOV (if present) to return to the default
 *        frequency value.
 *
 * \param op_data OPERATOR_DATA of the requesting operator client
 *
 * \return TRUE if the message has been successfully sent
 */
bool aov_request_default_clock(OPERATOR_DATA *op_data);

/**
 * \brief Send a request to the AOV (if present) to return to forward a trigger
 * notification message.
 *
 * \param op_data OPERATOR_DATA of the requesting operator client.
 *
 * \param msg_len Length of the message contained in the trigger event
 *
 * \param msg Message to send in the trigger event
 *
 * \return TRUE if the message has been successfully sent
 */
bool aov_request_notify_trigger(OPERATOR_DATA *op_data,
                                unsigned msg_len, void *msg);

#endif /* OPMGR_OPERATOR_CLIENT_FRAMEWORK_H */


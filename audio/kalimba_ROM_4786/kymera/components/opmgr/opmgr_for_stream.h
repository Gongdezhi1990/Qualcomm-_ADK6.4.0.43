/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup opmgr Operator Manager
 * \file  opmgr_for_stream.h
 *
 * Operator Manager header file. <br>
 * This file contains opmgr functions and types that are only exposed to stream. <br>
 *
 */

#ifndef OPMGR_FOR_STREAM_H
#define OPMGR_FOR_STREAM_H

/****************************************************************************
Include Files
*/
#include "sched_oxygen/sched_oxygen.h"

/****************************************************************************
Public Function Declarations
*/
/**
 * \brief Get the task_id of the underlying operator of an operator endpoint.
 *
 * \note This is provided to facilitate kicking the operator without performing
 * any slow lookups.
 *
 * \return The task id of the underlying operator.
 */
extern BGINT_TASK opmgr_get_op_task_from_epid(unsigned opidep);

/**
 * \brief This function is used at connect, to cache information about the thing
 * on the other side of the connection if it wants to be kicked. If the other
 * side of the connection doesn't wish to receive kicks then it is expected that
 * this function is not called.
 *
 * \param endpoint_id The ID of the endpoint associated with the operator
 * terminal which is being connected and is required to propagate kicks.
 *
 * \return TRUE the table was successfully updated. FALSE the table update failed
 * likely reason is insufficient RAM.
 */
extern bool opmgr_kick_prop_table_add(unsigned endpoint_id, unsigned ep_id_to_kick);

/**
 * \brief Removes a connection from an operator's kick propagation table
 * when it no longer needs to be kicked.
 *
 * \param endpoint_id The ID of the endpoint to remove from the table.
 */
extern void opmgr_kick_prop_table_remove(unsigned endpoint_id);

#ifdef UNIT_TEST_BUILD
/**
 * Check if endpoint 1 kicks endpoint 2. If the result is different than the input
 *  parameter kicks the function panic.
 *
 * \param ep_id_1 The ID of the endpoint 1.
 * \param ep_id_2 The ID of the endpoint 2.
 * \param kicks boolean value which shows what do we expect.
 */
extern void check_propagation_table(unsigned ep_id_1, unsigned ep_id_2, bool kicks);
#endif

#endif /* OPMGR_FOR_STREAM_H */


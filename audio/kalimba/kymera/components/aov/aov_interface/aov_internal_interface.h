/****************************************************************************
 * Copyright (c) 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  aov_internal_interface.h
 * \ingroup  aov
 *
 * AOV interface APIs used by the AOV task. <br>
 */

#ifndef AOV_INTERNAL_INTERFACE_H
#define AOV_INTERNAL_INTERFACE_H
/****************************************************************************
Include Files
*/

#include "types.h"
#include "utils/utils_fsm.h"
#include "sched_oxygen/sched_oxygen.h"
/****************************************************************************
Public Type Declarations
*/

/* Contains all the information the AOV interface needs to forward the
 * requests to the AOV Client. */
typedef struct AOV_CLIENT_INTERFACE
{
    /* AOV Client queue where we need to send the events to.  */
    qid queue_id;

    /* External operator id of the only operator that can send requests to the
     * AOV Client. */
    unsigned allowed_ext_op_id;

    /* Event to send for the custom clock request. */
    const utils_fsmevent_type *use_custom_clock;

    /* Event to send for the default clock request. */
    const utils_fsmevent_type *use_default_clock;

    /* Event to send for the trigger notification request. */
    const utils_fsmevent_type *op_trigger_positive;
}AOV_CLIENT_INTERFACE;

/* Encapsulates additional data to the event, so that the state machine in the
 * AOV Client can receive all needed information. */
typedef struct
{
    utils_fsmevent base;
    void * body;
} aovsm_event;

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
Public Function Declarations
*/
/**
 * \brief Registers AOV client's presence to the interface.
 *
 * \param aov_interface Struct containing all the information about the AOV
 *                      client needed by the interface.
 */
void aov_client_present(AOV_CLIENT_INTERFACE *aov_client);

#endif /* AOV_INTERNAL_INTERFACE_H */

/*************************************************************************
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 * All Rights Reserved.
 * Qualcomm Technologies International, Ltd. Confidential and Proprietary.
*************************************************************************/
#ifndef AOV_H
#define AOV_H

#ifdef INSTALL_AOV
#include "subserv/subserv.h"

/** AOV operator type */
typedef enum
{
    AOV_OP_VAD = 0, /** Voice activity detector */
    AOV_OP_VTD,     /** Voice Trigger phrase detector */
    AOV_OP_VCI,     /** Voice Command interpretor */
    AOV_OP_MAX,
    AOV_OP_INVALID

} AOV_OP;

#define AOVSM_ID (0x1E)

/*
 * \brief Initialise the AOV module
 */
void aov_init(void);

/** External event senders */

/*
 * \brief Configure the AOV module's trigger parameters
 * \param wake_ext_op_id external operator id of the wake on operator
 * \param aov_op AOV operator type
 */
void aov_send_configure_trigger(unsigned wake_ext_op_id, AOV_OP aov_op);

/*
 * \brief Request the AOV module to enter low power mode
 * \param stag global service tag of the accmd service that will be
 *        be torn down post a low power mode transition
 */
void aov_send_lp_activate(STAG stag);

/*
 * \brief Request the AOV module to enter active mode
 * \param stag global service tag of the accmd service whose client
          must own the AOV graph
 */
void aov_send_lp_deactivate(STAG stag);

/*
 * \brief Stop the AOV module
 */
void aov_send_stop(void);

#endif // INSTALL_AOV

#endif // AOV_H

/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    broadcast_cmd_scm_trasport.c

DESCRIPTION
      This file holds hardware specific SCM transport interface definations.
*/

#include <hydra_macros.h>

#include "broadcast_cmd.h"

/****************************************************************************
 * Hardware Specific SCM transport interface functions
 ****************************************************************************/

/*!
@brief Interface for hardware specific module to register its task as SCM transport task.
*/
void BroadcastCmdScmTransportRegisterReq(Task transport_task)
{
    UNUSED(transport_task);    
}

/*!
@brief Interface for hardware specific module to de-register its task as SCM transport task.
*/
void BroadcastCmdScmTransportUnRegisterReq(void)
{
    /* Dummy function */
}

/*!
@brief Interface for hardware specific module to inform message send comfirmation
*/
void BroadcastCmdScmSeqmentSendCfm(unsigned header, unsigned num_transmissions)
{
    UNUSED(header); 
    UNUSED(num_transmissions);
}

 /*!
@brief Interface for hardware specific module to inform message indication
*/
void BroadcastCmdScmSeqmentInd(unsigned header, const uint8 *data)
{
    UNUSED(header); 
	UNUSED(data);
}

/*!
@brief Interface for hardware specific module to inform message Seqment Expired
*/
void BroadcastCmdScmSeqmentExpiredInd(unsigned header)
{
    UNUSED(header);
}




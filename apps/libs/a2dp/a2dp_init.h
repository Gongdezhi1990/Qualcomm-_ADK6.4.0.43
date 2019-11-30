/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_init.h

DESCRIPTION


*/

#ifndef A2DP_INIT_H_
#define A2DP_INIT_H_

#include "a2dp.h"
#include "a2dp_private.h"


/***************************************************************************
NAME
    a2dpSendInitCfmToClient

DESCRIPTION
    Send an A2DP_INIT_CFM message to the client task idicating the outcome
    of the library initialisation request.
*/
void a2dpSendInitCfmToClient(a2dp_status_code status);


/***************************************************************************
NAME
    a2dpInitTask

DESCRIPTION
    Initialise the a2dp task data structure with the supplied parameters.
*/
void a2dpInitTask(Task clientTask);


/***************************************************************************
NAME
    a2dpInitialiseRemoteDevice

DESCRIPTION
    Initialise a remote_device structure by setting .device_id member
    to given value and zeroing the rest.
*/
void a2dpInitialiseRemoteDevice(remote_device *device, uint8 device_id);


#endif /* A2DP_INIT_H_ */

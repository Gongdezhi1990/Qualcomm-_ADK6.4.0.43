/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_state_manager.h

DESCRIPTION
    Contains functions to manage SWAT connection states
*/

#ifndef _SWAT_STATE_MANAGER_H_
#define _SWAT_STATE_MANAGER_H_

#include "swat.h"
#include "swat_private.h"


/****************************************************************************
FUNCTION: 
    swatSetMediaState

PARAMETERS:
    device - the remote device to set the media channel state for
    media_type - the media channel to set the state of
    new_state - new state for the media channel

DESCRIPTION:
    Function to set the state of the media channel for *device

NOTES:
    This function will set the watchdog timer if the new state requires it
*/
void swatSetMediaState(remoteDevice *device, swatMediaType media_type, swatMediaState new_state);


/****************************************************************************
FUNCTION: 
    swatSetSignallingState

PARAMETERS:
    device - the remote device to set the signalling channel state for
    new_state - new state for the signalling channel

DESCRIPTION:
    Function to set the state of the signalling channel for *device
*/
void swatSetSignallingState(remoteDevice *device, swatSignallingState new_state);


#endif /* _SWAT_STATE_MANAGER_H_ */

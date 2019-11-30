/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_audio_manager.h

DESCRIPTION
    Contains functions to manage audio connections to remote devices
*/

#ifndef _SWAT_AUDIO_MANAGER_H_
#define _SWAT_AUDIO_MANAGER_H_

#include "swat.h"
#include "swat_private.h"


/****************************************************************************
NAME
    swatRegisterEsco

DESCRIPTION
    Function to register the eSCO with the connection library
*/
void swatRegisterEsco(remoteDevice *device);


/****************************************************************************
NAME
    swatRequestEsco

PARAMETERS
    *device - The remote device to request eSCO connection

DESCRIPTION
    Function to request an eSCO connection to the remote device
*/
void swatRequestEsco(remoteDevice *device);


/****************************************************************************
NAME
    swatHandleIncomingScoRequest

PARAMETERS
    *ind - pointer to the SCO connect indication message

DESCRIPTION
    Function to handle when the remote device requests to open a SCO
    (low latency) media channel.
*/
void swatHandleIncomingScoRequest(CL_DM_SYNC_CONNECT_IND_T * ind);


/****************************************************************************
NAME
    swatHandleScoConnectCfm

PARAMETERS
    *cfm - pointer to the SCO connect confirm message

DESCRIPTION
    Function to handle when a SCO connection has completed (or failed)
*/
void swatHandleScoConnectCfm(CL_DM_SYNC_CONNECT_CFM_T * cfm);


/****************************************************************************
NAME
    swatRequestEscoDisconnect

PARAMETERS
    device - The device which to disconnect the eSCO

RETURNS
    TRUE when channel is not in idle state; FALSE otherwise

DESCRIPTION
    Function to request disconnection of the eSCO media channel
*/
bool swatRequestEscoDisconnect(remoteDevice *device);


/****************************************************************************
NAME
    swatHandleIncomingScoDisconnect

PARAMETERS
    device - The device which to disconnect the eSCO

DESCRIPTION
    Function to request disconnection of the eSCO media channel
*/
void swatHandleIncomingScoDisconnect(Sink sink, hci_status status);


/****************************************************************************
NAME
    swatHandleUnresponsiveMediaLL

PARAMETERS
    device_id - The device ID for the device whose low latency media channel 
    has become unresponsive

DESCRIPTION
    Function to handle when a low latency SWAT media channel has become
    unresponsive
*/
void swatHandleUnresponsiveMediaLL(uint16 device_id);

#endif /* _SWAT_AUDIO_MANAGER_H_ */

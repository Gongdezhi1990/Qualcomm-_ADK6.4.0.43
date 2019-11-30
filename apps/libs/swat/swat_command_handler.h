/****************************************************************************
Copyright (c) 2013 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_command_handler.c

DESCRIPTION
    Contains functions related to sending / recieving SWAT commands
*/


#ifndef _SWAT_COMMAND_HANDLER_H_
#define _SWAT_COMMAND_HANDLER_H_


/****************************************************************************
    Header files
*/
#include "swat.h"
#include "swat_private.h"




/*****************************************************************************
FUNCTION:
    swatCommandHandler

DESCRIPTION:
    Handler for incoming SWAT commands
*/
void swatCommandHandler(Task task, MessageId id, Message message);




/****************************************************************************
    SWAT_MEDIA_OPEN
****************************************************************************/


/*****************************************************************************
FUNCTION:
    swatSendMediaOpenRequest

PARAMETERS:
    device_id - Identifier for the device to send the request to
    media_type - The media type to request

DESCRIPTION:
    Function to send SWAT_OPEN_CMD to the remote device
*/
void swatSendMediaOpenRequest(uint16 device_id, swatMediaType media_type);


/****************************************************************************
NAME
    swatHandleOpenMediaCmdFromRemoteDevice

PARAMETERS
    *device Pointer to the device requesting a media connection

DESCRIPTION
    Function to handle when a remote device sends a signalling request to
    open a media channel
*/
void swatHandleOpenMediaCmdFromRemoteDevice(remoteDevice * device, swatMediaType media_type);


/****************************************************************************
NAME
    swatHandleMediaOpenResponseFromClient

PARAMETERS
    device - The device which to send the response to
    media_type - The media channel type associated with the OPEN_REQ
    accept - Whether to accept or reject the OPEN_REQ from the remote device

DESCRIPTION
    Allows the Client task to respond to a SWAT_MEDIA_OPEN_IND message
*/
void swatHandleMediaOpenResponseFromClient(remoteDevice * device, swatMediaType media_type, bool accept);


/*****************************************************************************
FUNCTION:
    swatSendMediaOpenResponse

PARAMETERS:
    device - Device to send the response to
    response - The response code to send to the device
    media_type - The media channel type this response is related to

DESCRIPTION:
    Function to send SWAT_OPEN_RSP to the remote device
*/
void swatSendMediaOpenResponse(remoteDevice * device, swatMediaResponse response, swatMediaType media_type);


/****************************************************************************
NAME
    swatHandleOpenMediaRspFromRemoteDevice

PARAMETERS
    *device Pointer to the device requesting a media connection
    media_rsp The response code sent by the remote device
    media_type The type of media channel the response relates to

DESCRIPTION
    Function to handle when a remote device sends a signalling response to
    an open a media signalling request sent from this device
*/
void swatHandleOpenMediaRspFromRemoteDevice(remoteDevice * device, swatMediaResponse media_rsp, swatMediaType media_type);




/****************************************************************************
    SWAT_MEDIA_CLOSE
****************************************************************************/


/****************************************************************************
NAME
    swatSendMediaCloseReq

PARAMETERS
    device - The identifier for the remote device to send a MEDIA_CLOSE_REQ
    media_type - The media channel type associated with the CLOSE_REQ

DESCRIPTION
    Send a MEDIA_CLOSE_REQ to the remote device *device_id*
*/
void swatSendMediaCloseReq(uint16 device_id, swatMediaType media_type);


/****************************************************************************
NAME
    swatHandleCloseRequestFromRemoteDevice

PARAMETERS
    device - The remote device that sent a MEDIA_CLOSE_REQ
    media_type - The media channel type associated with the CLOSE_REQ

DESCRIPTION
    Handle when a remote device sends a MEDIA_CLOSE_REQ message
*/
void swatHandleCloseRequestFromRemoteDevice(remoteDevice * device, swatMediaType media_type);



/*****************************************************************************
FUNCTION:
    swatSendMediaCloseResponse

PARAMETERS:
    device - Device to send the response to
    response - The response code to send to the device
    media_type - The media channel type this response is related to

DESCRIPTION:
    Function to send SWAT_CLOSE_RSP to the remote device
*/
void swatSendMediaCloseResponse(remoteDevice * device, swatMediaResponse response, swatMediaType media_type);


/****************************************************************************
NAME
    swatHandleCloseResponseFromRemoteDevice

PARAMETERS
    device - The remote device that sent a MEDIA_CLOSE_RES
    media_rsp - The SWAT response code (accept / reject) that was sent by the remote device
    media_type - The media channel type associated with the CLOSE_REQ

DESCRIPTION
    Handle when a remote device sends a MEDIA_CLOSE_RES message
*/
void swatHandleCloseResponseFromRemoteDevice(remoteDevice * device, swatMediaResponse media_rsp, swatMediaType media_type);




/****************************************************************************
    SWAT_MEDIA_START
****************************************************************************/


/****************************************************************************
NAME
    swatSendMediaStartReq

PARAMETERS
    device_id - Identifier for the device to send start reqest
    media_type - The media channel type associated with the START_REQ

DESCRIPTION
    Send a START_REQ command to device *device_id*
*/
void swatSendMediaStartReq(uint16 device_id, swatMediaType media_type);


/****************************************************************************
NAME
    swatHandleStartRequestFromRemoteDevice

PARAMETERS
    device - the remote device that sent a START_REQ
    media_type - the media channel type associated with the START_REQ

DESCRIPTION
    Handle an incoming START_REQ sent from the remote device *device*
*/
void swatHandleStartRequestFromRemoteDevice(remoteDevice * device, swatMediaType media_type);


/****************************************************************************
NAME
    swatHandleMediaStartResponseFromClient

PARAMETERS
    device - The device which to send the response to
    media_type - The media channel type associated with the START_REQ
    accept - Whether to accept or reject the START_REQ from the remote device

DESCRIPTION
    Allows the Client task to respond to a SWAT_MEDIA_START_IND message
*/
void swatHandleMediaStartResponseFromClient(remoteDevice * device, swatMediaType media_type, bool accept);


/*****************************************************************************
FUNCTION:
    swatSendMediaStartResponse

PARAMETERS:
    device - Device to send the response to
    response - The response code to send to the device
    media_type - The media channel type this response is related to

DESCRIPTION:
    Function to send SWAT_START_RSP to the remote device
*/
void swatSendMediaStartResponse(remoteDevice * device, swatMediaResponse response, swatMediaType media_type);


/****************************************************************************
NAME
    swatHandleStartResponseFromRemoteDevice

PARAMETERS
    device - The remote device that sent a START_RES
    media_rsp - The SWAT response code (accept / reject) that was sent by the remote device
    media_type - The media channel type associated with the START_RES

DESCRIPTION
    Handle an incoming START_RES sent from the remote device
*/
void swatHandleStartResponseFromRemoteDevice(remoteDevice * device, swatMediaResponse media_rsp, swatMediaType media_type);




/****************************************************************************
    SWAT_MEDIA_SUSPEND
****************************************************************************/


/****************************************************************************
NAME
    swatSendMediaSuspendReq

PARAMETERS
    device_id = The remote device to send a SUSPEND_REQ to
    media_type - The media channel type associated with the SUSPEND_REQ

DESCRIPTION
    Send a SUSPEND_REQ to the specified remote device
*/
void swatSendMediaSuspendReq(uint16 device_id, swatMediaType media_type);


/****************************************************************************
NAME
    swatHandleSuspendRequestFromRemoteDevice

PARAMETERS
    device - The remote device that sent a SUSPEND_REQ
    media_type - The media channel type associated with the SUSPEND_REQ

DESCRIPTION
    Handle an incoming SUSPEND_REQ sent from the remote device
*/
void swatHandleSuspendRequestFromRemoteDevice(remoteDevice * device, swatMediaType media_type);



/*****************************************************************************
FUNCTION:
    swatSendMediaSuspendResponse

PARAMETERS:
    device - Device to send the response to
    response - The response code to send to the device
    media_type - The media channel type this response is related to

DESCRIPTION:
    Function to send SWAT_SUSPEND_RSP to the remote device
*/
void swatSendMediaSuspendResponse(remoteDevice * device, swatMediaResponse response, swatMediaType media_type);


/****************************************************************************
NAME
    swatHandleSuspendResponseFromRemoteDevice

PARAMETERS
    device - The remote device that sent a  SUSPEND_RES
    media_rsp - The SWAT response code (accept / reject) that was sent by the remote device
    media_type - The media channel type associated with the SUSPEND_RES

DESCRIPTION
    Handle an incoming SUSPEND_RES sent from the remote device
*/
void swatHandleSuspendResponseFromRemoteDevice(remoteDevice * device, swatMediaResponse media_rsp, swatMediaType media_type);




/****************************************************************************
    SWAT_SET_VOLUME
****************************************************************************/


/****************************************************************************
NAME
    swatSendSetVolumeReq

PARAMETERS
    device - The remote device to send SET_VOLUME request to
    volume - The absolute volume level to set
    sub_trim - The absolute sub trim level to set

DESCRIPTION
    Send a SET_VOLUME_REQ to the remote device *device_id*
*/
void swatSendSetVolumeReq(uint16 device_id, uint8 volume, uint8 sub_trim);


/****************************************************************************
NAME
    swatHandleVolumeRequestFromRemoteDevice

PARAMETERS
    device - The remote device to send SET_VOLUME request to
    volume - The absolute volume level to set
    sub_trim - The absolute sub_trim level to set

DESCRIPTION
    Handle an incoming SET_VOLUME_REQ from the remote device *device_id*
*/
void swatHandleVolumeRequestFromRemoteDevice(remoteDevice * device, uint8 volume, uint8 sub_trim);


/****************************************************************************
NAME
    swatHandleVolumeResponseFromRemoteDevice

PARAMETERS
    device - The remote device to send SET_VOLUME request to
    volume - The absolute volume level to set
    subtrim - The absolute sub_trim level to set

DESCRIPTION
    Handle an incoming SET_VOLUME_RES from the remote device *device_id*
*/
void swatHandleVolumeResponseFromRemoteDevice(remoteDevice * device, uint8 volume, uint8 sub_trim);




/****************************************************************************
    SWAT_SAMPLE_RATE
****************************************************************************/


/****************************************************************************
NAME
    swatSendSampleRateCommand

PARAMETERS
    device_id - The ID for the remote device to send sample rate to
    rate - The sample rate in use

DESCRIPTION
    Function to send SWAT_SEND_SAMPLE_RATE_CMD to the remote device
*/
void swatSendSampleRateCommand(uint16 device_id, uint16 rate);


/****************************************************************************
NAME
    swatHandleSampleRateCommandFromRemoteDevice

PARAMETERS
    device - The remote device that sent the command
    rate - The sample rate that was sent by the remote device

DESCRIPTION
    Function to handle when a remote device sent a SWAT_SAMPLE_RATE_CMD message
*/
void swatHandleSampleRateCommandFromRemoteDevice(remoteDevice * device, uint16 rate);



/****************************************************************************
NAME
    swatHandleSampleRateResponseFromClient

PARAMETERS
    device - The device which to send the response to
    rate - The sample rate that the client has set (should be the same as the
           sample rate that was sent in the SWAT_SAMPLE_RATE_CMD message

DESCRIPTION
    Allows the Client task to respond to a SWAT_SAMPLE_RATE_IND message
*/
void swatHandleSampleRateResponseFromClient(remoteDevice * device, uint16 rate);



/****************************************************************************
NAME
    swatSendSampleRateResponse

PARAMETERS
    device - The device to send the response to
    response - Should always accept sample rate updates
    rate - The sample rate in use

RETURNS
    bool - TRUE if response was sent, FALSE otherwise
    
DESCRIPTION
    Function to send the SWAT_SAMPLE_RATE_RSP packet to the remote device 
*/
void swatSendSampleRateResponse(remoteDevice * device, swatMediaResponse response, uint16 rate);



/****************************************************************************
NAME
    swatHandleSampleRateResponseFromRemoteDevice

PARAMETERS
    device - The remote device that sent the response
    media_rsp = The SWAT response code (accept / reject) tat was sent by the remote device
    rate - The sample rate that was understood by the remote device

DESCRIPTION
    Function to handle when a remote device has responded to a SWAT_SAMPLE_RATE_CMD message
*/
void swatHandleSampleRateResponseFromRemoteDevice(remoteDevice * device, swatMediaResponse media_rsp, uint16 rate);




/****************************************************************************
    SWAT_GENERAL_REJECT
****************************************************************************/


/*****************************************************************************
FUNCTION:
    swatSendGeneralReject

PARAMETERS:
    device - Device to send the reject to
    cmd_id - The SWAT command ID that was rejected (invalid SWAT command)

DESCRIPTION:
    Function to send the GENERAL_REJECT message to remote device
*/
void swatSendGeneralReject(remoteDevice * device, swatCommandId cmd_id);


/*****************************************************************************
FUNCTION:
    swatSendGeneralReject

PARAMETERS:
    device_id - Identifier for the remote device
    cmd_id - The SWAT command ID that was rejected by the remote device (invalid SWAT command)

DESCRIPTION:
    Function to handle when we got a SWAT_GENERAL_REJECT from the remote device
*/
void swatHandleGeneralRejectFromRemoteDevice(uint16 device_id, uint16 cmd_id);

/*****************************************************************************
FUNCTION:
    swatSendGetVersionNoReq

PARAMETERS:
    device_id - Identifier for the remote device

DESCRIPTION:
    Function to get the version number from the subwoofer device
*/
void swatSendGetVersionNoReq(uint16 device_id);

/*****************************************************************************
FUNCTION:
    swatHandleVersionNoRequestFromRemoteDevice

PARAMETERS:
    device_id - Identifier for the remote device

DESCRIPTION:
    Function to handle the get version number request on the Subwoofer
*/
void swatHandleVersionNoRequestFromRemoteDevice(remoteDevice *device);

/*****************************************************************************
FUNCTION:
    swatHandleVersionNoResponseFromRemoteDevice

PARAMETERS:
    device_id - Identifier for the remote device
    major - Subwoofer Major version number
    minor - Subwoofer Minor version number
    
DESCRIPTION:
    Function to handle the version number response from the subwoofer
*/
void swatHandleVersionNoResponseFromRemoteDevice(remoteDevice *device, uint16 major, uint16 minor);


#endif /* _SWAT_COMMAND_HANDLER_H_ */

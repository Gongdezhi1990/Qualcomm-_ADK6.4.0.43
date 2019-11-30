/****************************************************************************
Copyright (c) 2013 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_api.h
    
DESCRIPTION
*/

#ifndef SWAT_API_H_
#define SWAT_API_H_

/* Library includes */
#include "swat.h"

/* Firmware includes */
#include <sink.h>


/*****************************************************************************
FUNCTION:
    swatSendInitCfmToClient

DESCRIPTION:
    Function to send SWAT_INIT_CFM message to the client task
*/
void swatSendInitCfmToClient(swat_status_code status);


/*****************************************************************************
FUNCTION:
    swatSendSignallingConnectIndToClient

DESCRIPTION:
    Function to send SWAT_SIGNALLING_CONNECT_IND message to the client task
*/
void swatSendSignallingConnectIndToClient(uint16 device_id, uint16 connection_id, uint8 identifier, bdaddr bd_addr);


/*****************************************************************************
FUNCTION:
    swatSendSignallingConnectCfmToClient

DESCRIPTION:
    Function to send SWAT_SIGNALLING_CONNECT_CFM message to the client task
*/
void swatSendSignallingConnectCfmToClient(swat_status_code status, uint16 device_id, Sink sink);


/*****************************************************************************
FUNCTION:
    swatSendSignallingDisconnectIndToClient

DESCRIPTION:
    Function to send SWAT_SIGNALLING_DISCONNECT_IND message to the client task
*/
void swatSendSignallingDisconnectIndToClient(swat_status_code status, uint16 device_id);


/*****************************************************************************
FUNCTION:
    swatSendSignallingDisconnectCfmToClient

DESCRIPTION:
    Function to send SWAT_SIGNALLING_DISCONNECT_CFM message to the client task
*/
void swatSendSignallingDisconnectCfmToClient(swat_status_code status, uint16 device_id);


/*****************************************************************************
FUNCTION:
    swatSendMediaOpenIndToClient

DESCRIPTION:
    Function to send SWAT_MEDIA_OPEN_IND message to the client task
*/
void swatSendMediaOpenIndToClient(uint16 device_id, swatMediaType media_type);


/*****************************************************************************
FUNCTION:
    swatSendMediaOpenCfmToClient

DESCRIPTION:
    Function to send SWAT_MEDIA_OPEN_CFM message to the client task
*/
void swatSendMediaOpenCfmToClient(swat_status_code status, uint16 id, swatMediaType type, Sink sink);


/*****************************************************************************
FUNCTION:
    swatSendMediaCloseIndToClient

DESCRIPTION:
    Function to send SWAT_MEDIA_CLOSE_IND message to the client task
*/
void swatSendMediaCloseIndToClient(uint16 id, swatMediaType type);


/*****************************************************************************
FUNCTION:
    swatSendMediaCloseCfmToClient

DESCRIPTION:
    Function to send SWAT_MEDIA_CLOSE_CFM message to the client task
*/
void swatSendMediaCloseCfmToClient(swat_status_code status, uint16 id, swatMediaType type);


/*****************************************************************************
FUNCTION:
    swatSendMediaStartIndToClient

DESCRIPTION:
    Function to send SWAT_MEDIA_START_IND message to the client task
*/
void swatSendMediaStartIndToClient(uint16 id, swatMediaType type);


/*****************************************************************************
FUNCTION:
    swatSendMediaStartCfmToClient

DESCRIPTION:
    Function to send SWAT_MEDIA_START_CFM message to the client task
*/
void swatSendMediaStartCfmToClient(swat_status_code status, uint16 id, swatMediaType type);


/*****************************************************************************
FUNCTION:
    swatSendMediaSuspendIndToClient

DESCRIPTION:
    Function to send SWAT_MEDIA_SUSPEND_IND message to the client task
*/
void swatSendMediaSuspendIndToClient(uint16 id, swatMediaType type);


/*****************************************************************************
FUNCTION:
    swatSendMediaSuspendCfmToClient

DESCRIPTION:
    Function to send SWAT_MEDIA_SUSPEND_CFM message to the client task
*/
void swatSendMediaSuspendCfmToClient(swat_status_code status, uint16 id, swatMediaType type);


/*****************************************************************************
FUNCTION:
    swatSendVolumeIndToClient

DESCRIPTION:
    Function to send SWAT_SET_VOLUME_IND message to the client task
*/
void swatSendVolumeIndToClient(uint16 id, uint8 volume, uint8 sub_trim);


/*****************************************************************************
FUNCTION:
    swatSendVolumeCfmToClient

DESCRIPTION:
    Function to send SWAT_SET_VOLUME_CFM message to the client task
*/
void swatSendVolumeCfmToClient(swat_status_code status, uint16 id, uint8 volume, uint8 sub_trim);


/*****************************************************************************
FUNCTION:
    swatSendSampleRateIndToClient

DESCRIPTION:
    Function to send SWAT_SAMPLE_RATE_IND message to the client task
*/
void swatSendSampleRateIndToClient(uint16 id, uint16 rate);


/*****************************************************************************
FUNCTION:
    swatSendSampleRateIndToClient

DESCRIPTION:
    Function to send SWAT_SAMPLE_RATE_CFM message to the client task
*/
void swatSendSampleRateCfmToClient(swat_status_code status, uint16 id, uint16 rate);

/*****************************************************************************
FUNCTION:
    swatSendVersionNoIndToClient

DESCRIPTION:
    Function to send sub version no message to the client task
*/
void swatSendVersionNoIndToClient(uint16 id);

/*****************************************************************************
FUNCTION:
    swatSendVersionNoCfmToClient

DESCRIPTION:
    Function to send sub version no message to the client task
*/
void swatSendVersionNoCfmToClient(swat_status_code status, uint16 id, uint16 major, uint16 minor);


#endif

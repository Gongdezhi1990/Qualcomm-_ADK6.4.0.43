/****************************************************************************
Copyright (c) 2013 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_api.c

DESCRIPTION
    Contains all SWAT functions accessible to the client task.
    Contains all SWAT functions used by SWAT to send messages to the client
    task.
*/


/****************************************************************************
    Header files
*/
#include "swat.h"
#include "swat_private.h"
#include "swat_l2cap_handler.h"
#include "swat_command_handler.h"
#include "swat_api.h"

/* Firmware includes */
#include <message.h>
#include <stdlib.h>
#include <bdaddr.h>
#include <sink.h>


/****************************************************************************
    
    API Functions used by the client task to make use of SWAT functionality
    
****************************************************************************/


/****************************************************************************/
swatMediaState SwatGetMediaState(uint16 device_id)
{
    /* obtain pointer to requested sub device */
    remoteDevice * device = &swat->remote_devs[device_id];
    /* return current media state */
    if((device->media_state >= swat_media_closed) && (device->media_state <= swat_media_closing))
    {
        return device->media_state;       
    }
    
    return swat_media_invalid;
}


/****************************************************************************/
swatMediaState SwatGetMediaLLState(uint16 device_id)
{
    /* obtain pointer to requested sub device */
    remoteDevice * device = &swat->remote_devs[device_id];
    
    /* return current media state */
    if((device->media_ll_state >= swat_media_closed) && (device->media_ll_state <= swat_media_closing))
    {
        return device->media_ll_state;
    }
    
    return swat_media_invalid;
}


/****************************************************************************/
Sink SwatGetMediaSink(uint16 device_id)
{
    /* obtain pointer to requested sub device */
    remoteDevice * device = &swat->remote_devs[device_id];

    /* Return the sink of the active media channel or NULL if no active media channel */
    if (device->transport_state == swat_transport_connected)
    {
        return device->media_sink;
    }
    else if (device->transport_ll_state == swat_transport_connected)
    {
        return device->media_ll_sink;
    }
    else
    {
        return NULL;
    }
}


/****************************************************************************/
swatMediaType SwatGetMediaType(uint16 device_id)
{
    /* obtain pointer to requested sub device */
    remoteDevice * device = &swat->remote_devs[device_id];

    /* Return the active media channel */
    if (device->transport_state == swat_transport_connected)
    {
        return SWAT_MEDIA_STANDARD;
    }
    else if (device->transport_ll_state == swat_transport_connected)
    {
        return SWAT_MEDIA_LOW_LATENCY;
    }
    else
    {
        return SWAT_MEDIA_NONE;
    }
}


/****************************************************************************/
Sink SwatGetSignallingSink(uint16 device_id)
{
    if(swat)
    {
        /* obtain pointer to requested sub device */
        remoteDevice * device = &swat->remote_devs[device_id];
        
        return device->signalling_sink;    
    }
    else
        return NULL;
}


/****************************************************************************/
bdaddr * SwatGetBdaddr(uint16 device_id)
{
    if(swat)
    {
        /* obtain pointer to requested sub device */
        remoteDevice * device = &swat->remote_devs[device_id];
    
        return &device->bd_addr;    
    }
    else
        return NULL;
}






/****************************************************************************/
bool SwatSignallingConnectRequest(bdaddr *addr)
{
    if ((swat == NULL) || (addr == NULL) || BdaddrIsZero(addr))
    {
        return FALSE;
    }
    else
    {
        MAKE_SWAT_MESSAGE(SWAT_INTERNAL_SIGNALLING_CONNECT_REQ);
        message->addr = *addr;
        MessageSend(&swat->profile_task, SWAT_INTERNAL_SIGNALLING_CONNECT_REQ, message);
        return TRUE;
    }
}


/****************************************************************************/
bool SwatSignallingConnectResponse(uint16 device_id, uint16 connection_id, uint8 identifier, bool accept)
{
    if (swat == NULL)
    {
        return FALSE;
    }
    else
    {
        swatL2capSignallingConnectResponse(device_id, connection_id, identifier, accept);
        return TRUE;
    }
}


/****************************************************************************/
bool SwatSignallingDisconnectRequest(uint16 device_id)
{
    if (swat == NULL)
    {
        return FALSE;
    }
    else
    {
        MAKE_SWAT_MESSAGE(SWAT_INTERNAL_SIGNALLING_DISCONNECT_REQ);
        message->device_id = device_id;
        MessageSend(&swat->profile_task, SWAT_INTERNAL_SIGNALLING_DISCONNECT_REQ, message);
        return TRUE;
    }
}






/****************************************************************************/
bool SwatMediaOpenRequest(uint16 device_id, swatMediaType media_type)
{
    if (swat == NULL)
    {
        return FALSE;
    }
    else
    {
        MAKE_SWAT_MESSAGE(SWAT_INTERNAL_MEDIA_OPEN_REQ);
        message->device_id = device_id;
        message->media_type = media_type;
        MessageSend(&swat->profile_task, SWAT_INTERNAL_MEDIA_OPEN_REQ, message);
        return TRUE;
    }
}


/****************************************************************************/
bool SwatMediaOpenResponse(uint16 device_id, swatMediaType media_type, bool accept)
{
    if (swat == NULL)
    {
        return FALSE;
    }
    else
    {
        swatHandleMediaOpenResponseFromClient(&swat->remote_devs[device_id], media_type, accept);
        return TRUE;
    }
}






/****************************************************************************/
bool SwatMediaCloseRequest(uint16 device_id, swatMediaType media_type)
{
    if (swat == NULL)
    {
        return FALSE;
    }
    else
    {
        MAKE_SWAT_MESSAGE(SWAT_INTERNAL_MEDIA_CLOSE_REQ);
        message->device_id = device_id;
        message->media_type = media_type;
        MessageSend(&swat->profile_task, SWAT_INTERNAL_MEDIA_CLOSE_REQ, message);
        return TRUE;
    }
}






/****************************************************************************/
bool SwatMediaStartRequest(uint16 device_id, swatMediaType media_type)
{
    if (swat == NULL)
    {
        return FALSE;
    }
    else
    {
        MAKE_SWAT_MESSAGE(SWAT_INTERNAL_MEDIA_START_REQ);
        message->device_id = device_id;
        message->media_type = media_type;
        MessageSend(&swat->profile_task, SWAT_INTERNAL_MEDIA_START_REQ, message);
        return TRUE;
    }
}


/****************************************************************************/
bool SwatMediaStartResponse(uint16 device_id, swatMediaType media_type, bool accept)
{
    if (swat == NULL)
    {
        return FALSE;
    }
    else
    {
        swatHandleMediaStartResponseFromClient(&swat->remote_devs[device_id], media_type, accept);
        return TRUE;
    }
}






/****************************************************************************/
bool SwatMediaSuspendRequest(uint16 device_id, swatMediaType media_type)
{
    if (swat == NULL)
    {
        return FALSE;
    }
    else
    {
        MAKE_SWAT_MESSAGE(SWAT_INTERNAL_MEDIA_SUSPEND_REQ);
        message->device_id = device_id;
        message->media_type = media_type;
        MessageSend(&swat->profile_task, SWAT_INTERNAL_MEDIA_SUSPEND_REQ, message);
        return TRUE;
    }
}





/****************************************************************************/
bool SwatSetVolume(uint16 device_id, uint8 volume, uint8 sub_trim)
{
    if (swat == NULL)
    {
        return FALSE;
    }
    else
    {
        MAKE_SWAT_MESSAGE(SWAT_INTERNAL_SET_VOLUME_REQ);
        message->device_id = device_id;
        message->volume = volume;
        message->sub_trim = sub_trim;
        MessageSend(&swat->profile_task, SWAT_INTERNAL_SET_VOLUME_REQ, message);
        return TRUE;
    }
}


/****************************************************************************/
bool SwatSendSampleRateCommand(uint16 device_id, uint16 rate)
{
    if ( (swat == NULL) || (swat->role != swat_role_source) )
    {
        return FALSE;
    }
    else
    {
        swatSendSampleRateCommand(device_id, rate);
        return TRUE;
    }
}


/****************************************************************************/
bool SwatSampleRateResponse(uint16 device_id, uint16 rate)
{
    if (swat == NULL)
    {
        return FALSE;
    }
    else
    {
        swatHandleSampleRateResponseFromClient(&swat->remote_devs[device_id], rate);
        return TRUE;
    }
}

/****************************************************************************/
/* function to get Subwoofer version number */
/****************************************************************************/
bool SwatSendGetVersionNoCommand(uint16 device_id)
{
    if ( (swat == NULL) || (swat->role != swat_role_source) )
    {
        return FALSE;
    }
    else
    {
        swatSendGetVersionNoReq(device_id);
        return TRUE;
    }
}



/****************************************************************************
    
    API Functions used by the SWAT library to send messages to client task
    
****************************************************************************/




/****************************************************************************/
void swatSendInitCfmToClient(swat_status_code status)
{
    MAKE_SWAT_MESSAGE(SWAT_INIT_CFM);
    message->status = status;
    MessageSend(swat->clientTask, SWAT_INIT_CFM, message);

    /* If the initialisation failed, free the allocated task */
    if (status != swat_success)
    {
        free(swat);
        swat = 0;
    }
}




/****************************************************************************/
void swatSendSignallingConnectIndToClient(uint16 device_id, uint16 connection_id, uint8 identifier, bdaddr bd_addr)
{
    MAKE_SWAT_MESSAGE(SWAT_SIGNALLING_CONNECT_IND);
    message->device_id = device_id;
    message->connection_id = connection_id;
    message->identifier = identifier;
    message->bd_addr = bd_addr;
    
    MessageSend(swat->clientTask, SWAT_SIGNALLING_CONNECT_IND, message);
}


/****************************************************************************/
void swatSendSignallingConnectCfmToClient(swat_status_code status, uint16 device_id, Sink sink)
{
    MAKE_SWAT_MESSAGE(SWAT_SIGNALLING_CONNECT_CFM);
    message->status = status;
    message->device_id = device_id;
    message->sink = sink;

    MessageSend(swat->clientTask, SWAT_SIGNALLING_CONNECT_CFM, message);
}




/****************************************************************************/
void swatSendSignallingDisconnectIndToClient(swat_status_code status, uint16 device_id)
{
    MAKE_SWAT_MESSAGE(SWAT_SIGNALLING_DISCONNECT_IND);
    message->device_id = device_id;
    MessageSend(swat->clientTask, SWAT_SIGNALLING_DISCONNECT_IND, message);
}


/****************************************************************************/
void swatSendSignallingDisconnectCfmToClient(swat_status_code status, uint16 device_id)
{
    MAKE_SWAT_MESSAGE(SWAT_SIGNALLING_DISCONNECT_CFM);
    message->status = status;
    message->device_id = device_id;
    MessageSend(swat->clientTask, SWAT_SIGNALLING_DISCONNECT_CFM, message);
}




/****************************************************************************/
void swatSendMediaOpenIndToClient(uint16 device_id, swatMediaType media_type)
{
    MAKE_SWAT_MESSAGE(SWAT_MEDIA_OPEN_IND);
    message->device_id = device_id;
    message->media_type = media_type;
    MessageSend(swat->clientTask, SWAT_MEDIA_OPEN_IND, message);
}


/****************************************************************************/
void swatSendMediaOpenCfmToClient(swat_status_code status, uint16 id, swatMediaType type, Sink sink)
{
    MAKE_SWAT_MESSAGE(SWAT_MEDIA_OPEN_CFM);
    message->status = status;
    message->device_id = id;
    message->media_type = type;
    message->audio_sink = sink;
    MessageSend(swat->clientTask, SWAT_MEDIA_OPEN_CFM, message);
}




/****************************************************************************/
void swatSendMediaCloseIndToClient(uint16 id, swatMediaType type)
{
    MAKE_SWAT_MESSAGE(SWAT_MEDIA_CLOSE_IND);
    message->device_id = id;
    message->media_type = type;
    MessageSend(swat->clientTask, SWAT_MEDIA_CLOSE_IND, message);
}


/****************************************************************************/
void swatSendMediaCloseCfmToClient(swat_status_code status, uint16 id, swatMediaType type)
{
    MAKE_SWAT_MESSAGE(SWAT_MEDIA_CLOSE_CFM);
    message->status = status;
    message->device_id = id;
    message->media_type = type;
    MessageSend(swat->clientTask, SWAT_MEDIA_CLOSE_CFM, message);
}




/****************************************************************************/
void swatSendMediaStartIndToClient(uint16 id, swatMediaType type)
{
    MAKE_SWAT_MESSAGE(SWAT_MEDIA_START_IND);
    message->device_id = id;
    message->media_type = type;
    MessageSend(swat->clientTask, SWAT_MEDIA_START_IND, message);
}


/****************************************************************************/
void swatSendMediaStartCfmToClient(swat_status_code status, uint16 id, swatMediaType type)
{
    MAKE_SWAT_MESSAGE(SWAT_MEDIA_START_CFM);
    message->status = status;
    message->device_id = id;
    message->media_type = type;
    MessageSend(swat->clientTask, SWAT_MEDIA_START_CFM, message);
}




/****************************************************************************/
void swatSendMediaSuspendIndToClient(uint16 id, swatMediaType type)
{
    MAKE_SWAT_MESSAGE(SWAT_MEDIA_SUSPEND_IND);
    message->device_id = id;
    message->media_type = type;
    MessageSend(swat->clientTask, SWAT_MEDIA_SUSPEND_IND, message);
}


/****************************************************************************/
void swatSendMediaSuspendCfmToClient(swat_status_code status, uint16 id, swatMediaType type)
{
    MAKE_SWAT_MESSAGE(SWAT_MEDIA_SUSPEND_CFM);
    message->status = status;
    message->device_id = id;
    message->media_type = type;
    MessageSend(swat->clientTask, SWAT_MEDIA_SUSPEND_CFM, message);
}




/****************************************************************************/
void swatSendVolumeIndToClient(uint16 id, uint8 volume, uint8 sub_trim)
{
    MAKE_SWAT_MESSAGE(SWAT_SET_VOLUME_IND);
    message->device_id = id;
    message->volume = volume;
    message->sub_trim = sub_trim;
    MessageSend(swat->clientTask, SWAT_SET_VOLUME_IND, message);
}


/****************************************************************************/
void swatSendVolumeCfmToClient(swat_status_code status, uint16 id, uint8 volume, uint8 sub_trim)
{
    MAKE_SWAT_MESSAGE(SWAT_SET_VOLUME_CFM);
    message->status = status;
    message->device_id = id;
    message->volume = volume;
    message->sub_trim = sub_trim;
    MessageSend(swat->clientTask, SWAT_SET_VOLUME_CFM, message);
}




/****************************************************************************/
void swatSendSampleRateIndToClient(uint16 id, uint16 rate)
{
    MAKE_SWAT_MESSAGE(SWAT_SAMPLE_RATE_IND);
    message->device_id = id;
    message->sample_rate = rate;
    MessageSend(swat->clientTask, SWAT_SAMPLE_RATE_IND, message);
}


/****************************************************************************/
void swatSendSampleRateCfmToClient(swat_status_code status, uint16 id, uint16 rate)
{
    MAKE_SWAT_MESSAGE(SWAT_SAMPLE_RATE_CFM);
    message->device_id = id;
    message->status = status;
    message->sample_rate = rate;
    MessageSend(swat->clientTask, SWAT_SAMPLE_RATE_CFM, message);
}

/****************************************************************************/
void swatSendVersionNoIndToClient(uint16 id)
{
    /* request version number from Subwoofer */
    MAKE_SWAT_MESSAGE(SWAT_GET_VERSION_IND);
    message->device_id = id;
    MessageSend(swat->clientTask, SWAT_GET_VERSION_IND, message);
}

/****************************************************************************/
void swatSendVersionNoCfmToClient(swat_status_code status, uint16 id, uint16 major, uint16 minor)
{
    /* version number repsonse from the Subwoofer */
    MAKE_SWAT_MESSAGE(SWAT_GET_VERSION_CFM);
    message->device_id = id;
    message->status = status;
    message->major = major;
    message->minor = minor;
    MessageSend(swat->clientTask, SWAT_GET_VERSION_CFM, message);
}

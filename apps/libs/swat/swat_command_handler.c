/****************************************************************************
Copyright (c) 2013 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_command_handler.c

DESCRIPTION
    Contains functions related to sending / recieving SWAT commands
*/


/****************************************************************************
    Header files
*/
#include "swat.h"
#include "swat_private.h"
#include "swat_api.h"
#include "swat_state_manager.h"
#include "swat_audio_manager.h"
#include "swat_packet_handler.h"
#include "swat_l2cap_handler.h"
#include "swat_command_handler.h"
#include "swat_device_manager.h"

/* External lib includes */
#include <connection.h>
#include <bdaddr.h>

/* Firmware includes */
#include <sink.h>
#include <source.h>
#include <stream.h>
#include <message.h>




/*****************************************************************************
FUNCTION:
    checkMediaType

PARAMETERS:
    media_type - The media type to check is valid
    
DESCRIPTION:
    Helper function to validate that *media_type* is understood:
        Returns TRUE if media_type is valid
        Returns FALSE if media_type is not valid
*/
static bool checkMediaType(swatMediaType media_type)
{
    if ((media_type == SWAT_MEDIA_STANDARD) || (media_type == SWAT_MEDIA_LOW_LATENCY))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}




/*****************************************************************************/
void swatCommandHandler(Task task, MessageId id, Message message)
{   
    switch(id)
    {
        /* open media channel request/response */
        case SWAT_COMMAND_OPEN:
        {
            uint8 * ptr = &((SWAT_COMMAND_OPEN_T *)message)->data[0];
            remoteDevice* device = ((SWAT_COMMAND_OPEN_T *)message)->device;
            
            if (ptr[PKT_SIGNAL_TYPE] == SWAT_CMD)
            {
                /* block any more signalling commands until current process has completed */
                device->signalling_block = DATA_BLOCK;
                
                SWAT_DEBUG(("[SWAT] SWAT_OPEN_CMD Actioned\n"));
                swatHandleOpenMediaCmdFromRemoteDevice(device, (swatMediaType)ptr[PKT_PAYLOAD]);
            }
            else if (ptr[PKT_SIGNAL_TYPE] == SWAT_RSP)
            {
                SWAT_DEBUG(("[SWAT] SWAT_OPEN_RSP Actioned\n"));
                swatHandleOpenMediaRspFromRemoteDevice(device, (swatMediaResponse)ptr[PKT_PAYLOAD], (swatMediaType)ptr[PKT_PAYLOAD+1]);
            }
            else
            {
                SWAT_DEBUG(("[SWAT] Recieved SWAT_OPEN command from device ID[%x] Badly Formatted packet\n", device->id));
                swatSendGeneralReject(device, ptr[PKT_SIGNAL_ID]);
            }
        }
        break;

        /* close media channel request/repsonse */
        case SWAT_COMMAND_CLOSE:
        {
            uint8 * ptr = &((SWAT_COMMAND_CLOSE_T *)message)->data[0];
            remoteDevice* device = ((SWAT_COMMAND_CLOSE_T *)message)->device;
            
            if (ptr[PKT_SIGNAL_TYPE] == SWAT_CMD)
            {
                /* block any more signalling commands until current process has completed */
                device->signalling_block = DATA_BLOCK;
                
                SWAT_DEBUG(("[SWAT] SWAT_CLOSE_CMD Actioned\n"));
                swatHandleCloseRequestFromRemoteDevice(device, (swatMediaType)ptr[PKT_PAYLOAD]);
            }
            else if (ptr[PKT_SIGNAL_TYPE] == SWAT_RSP)
            {
                SWAT_DEBUG(("[SWAT] SWAT_CLOSE_RSP Actioned\n"));
                swatHandleCloseResponseFromRemoteDevice(device, (swatMediaResponse)ptr[PKT_PAYLOAD], (swatMediaType)ptr[PKT_PAYLOAD+1]);
            }
            else
            {
                SWAT_DEBUG(("[SWAT] Recieved SWAT_CLOSE command from device ID[%x] Badly formatted packet\n", device->id));
                swatSendGeneralReject(device, ptr[PKT_SIGNAL_ID]);
            }
        }
        break;
        
        /* start media streaming request/response */
        case SWAT_COMMAND_START:
        {
            uint8 * ptr = &((SWAT_COMMAND_START_T *)message)->data[0];
            remoteDevice* device = ((SWAT_COMMAND_START_T *)message)->device;
            
            if (ptr[PKT_SIGNAL_TYPE] == SWAT_CMD)
            {
                /* block any more signalling commands until current process has completed */
                device->signalling_block = DATA_BLOCK;
                
                SWAT_DEBUG(("[SWAT] SWAT_START_CMD Actioned\n"));
                swatHandleStartRequestFromRemoteDevice(device, (swatMediaType)ptr[PKT_PAYLOAD]);
            }
            else if (ptr[PKT_SIGNAL_TYPE] == SWAT_RSP)
            {
                SWAT_DEBUG(("[SWAT] SWAT_START_RSP Actioned\n"));
                swatHandleStartResponseFromRemoteDevice(device, (swatMediaResponse)ptr[PKT_PAYLOAD], (swatMediaType)ptr[PKT_PAYLOAD+1]);
            }
            else
            {
                SWAT_DEBUG(("[SWAT] Recieved SWAT_START command from device ID[%x] Badly formatted packet\n", device->id));
                swatSendGeneralReject(device, ptr[PKT_SIGNAL_ID]);
            }
        }            
        break;
        
        /* suspend the media streaming request/response */
        case SWAT_COMMAND_SUSPEND:
        {
            uint8 * ptr = &((SWAT_COMMAND_SUSPEND_T *)message)->data[0];
            remoteDevice* device = ((SWAT_COMMAND_SUSPEND_T *)message)->device;
            
            if (ptr[PKT_SIGNAL_TYPE] == SWAT_CMD)
            {
                /* block any more signalling commands until current process has completed */
                device->signalling_block = DATA_BLOCK;
                
                SWAT_DEBUG(("[SWAT] SWAT_SUSPEND_CMD Actioned\n"));
                swatHandleSuspendRequestFromRemoteDevice(device, (swatMediaType)ptr[PKT_PAYLOAD]);
            }
            else if (ptr[PKT_SIGNAL_TYPE] == SWAT_RSP)
            {
                SWAT_DEBUG(("[SWAT] SWAT_SUSPEND_RSP Actioned\n"));
                swatHandleSuspendResponseFromRemoteDevice(device, (swatMediaResponse)ptr[PKT_PAYLOAD], (swatMediaType)ptr[PKT_PAYLOAD+1]);
            }
            else
            {
                SWAT_DEBUG(("[SWAT] Recieved SWAT_SUSPEND command from device ID[%x] Badly formatted packet\n", device->id));
                swatSendGeneralReject(device, ptr[PKT_SIGNAL_ID]);
            }
        } 
        break;
        
        /* set volume request/response */
        case SWAT_COMMAND_SET_VOLUME:
        {
            uint8 * ptr = &((SWAT_COMMAND_SET_VOLUME_T *)message)->data[0];
            remoteDevice* device = ((SWAT_COMMAND_SET_VOLUME_T *)message)->device;
            
            if (ptr[PKT_SIGNAL_TYPE] == SWAT_CMD)
            {
                /* block any more signalling commands until current process has completed */
                device->signalling_block = DATA_BLOCK;
                
                SWAT_DEBUG(("[SWAT] SWAT_VOLUME_CMD Actioned\n"));
                swatHandleVolumeRequestFromRemoteDevice(device, ptr[PKT_PAYLOAD], ptr[PKT_PAYLOAD+1]);
            }
            else if (ptr[PKT_SIGNAL_TYPE] == SWAT_RSP)
            {
                SWAT_DEBUG(("[SWAT] SWAT_VOLUME_RSP Actioned\n"));
                swatHandleVolumeResponseFromRemoteDevice(device, ptr[PKT_PAYLOAD], ptr[PKT_PAYLOAD+1]);
            }
            else
            {
                SWAT_DEBUG(("[SWAT] Recieved SWAT_SET_VOLUME command from device ID[%x] Badly formatted packet\n", device->id));
                swatSendGeneralReject(device, ptr[PKT_SIGNAL_ID]);
            }    
        }
        break;
        
        /* Set sample rate request/response */
        case SWAT_COMMAND_SAMPLE_RATE:
        {
            uint16 sample_rate = 0;
            uint8 * ptr = &((SWAT_COMMAND_SAMPLE_RATE_T *)message)->data[0];
            remoteDevice* device = ((SWAT_COMMAND_SAMPLE_RATE_T *)message)->device;
            
            if (ptr[PKT_SIGNAL_TYPE] == SWAT_CMD)
            {
                /* block any more signalling commands until current process has completed */
                device->signalling_block = DATA_BLOCK;
                
                SWAT_DEBUG(("[SWAT] SWAT_SAMPLE_RATE_CMD Actioned\n"));
                /* Sample rate is 1x(uint16) stuffed into 2x(uint8), so unpack */
                sample_rate = (ptr[PKT_PAYLOAD+1] << 8) | ptr[PKT_PAYLOAD];
                swatHandleSampleRateCommandFromRemoteDevice(device, sample_rate);
            }
            else if (ptr[PKT_SIGNAL_TYPE] == SWAT_RSP)
            {
                SWAT_DEBUG(("[SWAT] SWAT_SAMPLE_RATE_RSP Actioned\n"));
                /* Sample rate is 1x(uint16) stuffed into 2x(uint8), so unpack */
                sample_rate = (ptr[PKT_PAYLOAD+2] << 8) | ptr[PKT_PAYLOAD+1];
                swatHandleSampleRateResponseFromRemoteDevice(device, ptr[PKT_PAYLOAD], sample_rate);
            }
            else
            {
                SWAT_DEBUG(("[SWAT] Recieved SWAT_SAMPLE_RATE command from device ID[%x] Badly formatted packet\n", device->id));
                swatSendGeneralReject(device, ptr[PKT_SIGNAL_ID]);
            }
        }
        break;
        
        case SWAT_COMMAND_GET_VERSION:
        {
            uint8 * ptr = &((SWAT_COMMAND_GET_VERSION_T *)message)->data[0];
            remoteDevice* device = ((SWAT_COMMAND_GET_VERSION_T *)message)->device;
            
            if (ptr[PKT_SIGNAL_TYPE] == SWAT_CMD)
            {
                /* block any more signalling commands until current process has completed */
                device->signalling_block = DATA_BLOCK;
                
                SWAT_DEBUG(("[SWAT] SWAT_VOLUME_CMD Actioned\n"));
                swatHandleVersionNoRequestFromRemoteDevice(device);
            }
            else if (ptr[PKT_SIGNAL_TYPE] == SWAT_RSP)
            {
                SWAT_DEBUG(("[SWAT] SWAT_VOLUME_RSP Actioned\n"));
                swatHandleVersionNoResponseFromRemoteDevice(device, ptr[PKT_PAYLOAD], ptr[PKT_PAYLOAD+1]);
            }
            else
            {
                SWAT_DEBUG(("[SWAT] Recieved SWAT_SET_VOLUME command from device ID[%x] Badly formatted packet\n", device->id));
                swatSendGeneralReject(device, ptr[PKT_SIGNAL_ID]);
            }                      
        }                
        break;                
                
        /* general reject of packet due to being interpretted incorrectly */
        case SWAT_COMMAND_GENERAL_REJECT:
        {
            uint8 * ptr = &((SWAT_COMMAND_GENERAL_REJECT_T *)message)->data[0];
            swatHandleGeneralRejectFromRemoteDevice(((SWAT_COMMAND_GENERAL_REJECT_T *)message)->device->id, ptr[PKT_PAYLOAD]);
        }
        break;
        
        /* Recieved an unrecognised SWAT command; Reject this command */
        case SWAT_COMMAND_DEFAULT:
        {
            uint8 * ptr = &((SWAT_COMMAND_DEFAULT_T *)message)->data[0];
            swatSendGeneralReject(((SWAT_COMMAND_DEFAULT_T *)message)->device, ptr[PKT_SIGNAL_ID]);
        }      
        break;
    }
}        




/****************************************************************************/
void swatSendMediaOpenRequest(uint16 device_id, swatMediaType media_type)
{
    remoteDevice * device = &swat->remote_devs[device_id];
    
    /* Build the SWAT_OPEN_REQ packet */
    uint8 packet[4] = {SWAT_OPEN, 1, SWAT_CMD, 0};
    packet[PKT_PAYLOAD] = media_type;
    
    /* Check the device supplied is valid */
    if (BdaddrIsZero(&device->bd_addr))
    {
        swatSendMediaOpenCfmToClient(swat_bad_data, 0, 0, 0);
        return;
    }
    
    /* Check there is a connected signalling channel with the device */
    if (device->signalling_state != swat_signalling_connected)
    {
        swatSendMediaOpenCfmToClient(swat_no_signalling_connection, 0, 0, 0);
        return;
    }
    
    /* Is requested media channel in correct state to issue an OPEN_REQ command? */
    if ( ((media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state) != swat_media_closed)
    {
        SWAT_DEBUG(("[SWAT] Device ID[%x] : Media channel TYPE[%x] in wrong STATE[%x] to send an open request\n", device_id, media_type, (media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state));
        /* only indicate to VM app if link is currently closing or suspending */
        if(((media_type == SWAT_MEDIA_STANDARD)&&((device->media_state == swat_media_suspending)||(device->media_state == swat_media_closing)))||
           ((media_type == SWAT_MEDIA_LOW_LATENCY)&&((device->media_ll_state == swat_media_suspending)||(device->media_ll_state == swat_media_closing))))
        {
            swatSendMediaOpenCfmToClient(swat_wrong_state, 0, 0, 0);
        }
        return;
    }  
        
    /* Send MEDIA_OPEN_CMD packet to the device */
    if (swatSendData(device, packet, sizeof(packet)))
    {
        SWAT_DEBUG(("[SWAT] Sent MEDIA_OPEN_CMD to device [%x] for media type [%x]\n", device->id, media_type));        
        swatSetMediaState(device, media_type, swat_media_opening);
    }
    else
    {
        SWAT_DEBUG(("[SWAT] Error sending MEDIA_OPEN_CMD\n"));
    }
}


/*****************************************************************************/
void swatHandleOpenMediaCmdFromRemoteDevice(remoteDevice * device, swatMediaType media_type)
{
    if (device)
    {
        /* Ensure the media_type being requested is valid */
        if ((checkMediaType(media_type)) != TRUE)
        {
            swatSendMediaOpenResponse(device, SWAT_REJECT_MEDIA, media_type);
            
            /* Command has been handled; can now release command block to process next queued command (if one exists) */
            SWAT_DEBUG(("[SWAT] SWAT_OPEN_CMD Handled\n"));
            device->signalling_block = 0;
            
            return;
        }
        else
        {
            /* Check the requested media channel is in the correct state to accept an OPEN_CMD */
            if ( ((media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state) != swat_media_closed)
            {
                swatSendMediaOpenResponse(device, SWAT_REJECT_MEDIA, media_type);
                swatSendMediaOpenCfmToClient(swat_wrong_state, device->id, media_type, 0);
                
                /* Command has been handled; can now release command block to process next queued command (if one exists) */
                SWAT_DEBUG(("[SWAT] SWAT_OPEN_CMD Handled\n"));
                device->signalling_block = 0;
            }
            /* Should the SWAT library auto accept the request? */
            else if (swat->auto_handle)
            {
                /* Auto accept the OPEN request from the remote device */
                swatSetMediaState(device, media_type, swat_media_opening);
                swatSendMediaOpenResponse(device, SWAT_ACCEPT_MEDIA, media_type);
            }
            else
            {
                /* Send indication to client task so they can accept / reject the request */
                swatSetMediaState(device, media_type, swat_media_opening);
                swatSendMediaOpenIndToClient(device->id, media_type);
                
                /* Wait for the application task to handle the IND */
                return;
            }
        }
    }
    else
    {
        /* Should never get here */
        SWAT_DEBUG(("[SWAT] LIBRARY ERROR : Device is null\n"));
    }
}


/*****************************************************************************/
void swatHandleMediaOpenResponseFromClient(remoteDevice * device, swatMediaType media_type, bool accept)
{
    /* Check application has supplied a valid device */
    if (BdaddrIsZero((const bdaddr *) &device->bd_addr))
    {
        swatSendMediaOpenCfmToClient(swat_bad_data, 0, 0, 0);
    }
    else
    {
        /* Send the response as supplied by client task and update SWAT library accordingly */
        if (accept)
        {
            /* Application accepted the OPEN command */
            swatSendMediaOpenResponse(device, SWAT_ACCEPT_MEDIA, media_type);
        }
        else
        {
            /* Application rejected the OPEN command */
            swatSendMediaOpenResponse(device, SWAT_REJECT_MEDIA, media_type);
            swatSetMediaState(device, media_type, swat_media_closed);
            swatSendMediaOpenCfmToClient(swat_rejected_by_client, device->id, media_type, 0);
            
            /* Command has been handled; can now release command block to process next queued command (if one exists) */
            SWAT_DEBUG(("[SWAT] SWAT_OPEN_CMD Handled\n"));
            device->signalling_block = 0;
        }
    }
}


/*****************************************************************************/
void swatSendMediaOpenResponse(remoteDevice * device, swatMediaResponse response, swatMediaType media_type)
{
    /* Build the MEDIA_OPEN_RSP packet */
    uint8 packet[5] = {SWAT_OPEN, 2, SWAT_RSP, 0, 0};
    packet[PKT_PAYLOAD]   = response;
    packet[PKT_PAYLOAD+1] = media_type;
    
    if (swatSendData(device, packet, sizeof(packet)))
    {
        SWAT_DEBUG(("[SWAT] Sent MEDIA_OPEN_RSP media_type[%x] response[%x]\n", packet[PKT_PAYLOAD+1], packet[PKT_PAYLOAD]));
    }
    else
    {
        SWAT_DEBUG(("[SWAT] Error sending MEDIA_OPEN_RSP\n"));
    }
    
    /* Wait for the media channel to open before releasing the signalling block as this command is not yet complete */
}


/*****************************************************************************/
void swatHandleOpenMediaRspFromRemoteDevice(remoteDevice * device, swatMediaResponse media_rsp, swatMediaType media_type)
{
    if (device)
    {
        if (media_rsp == SWAT_ACCEPT_MEDIA)
        {
            SWAT_DEBUG(("[SWAT] Device ID[%x] accepted request for media channel TYPE[%x]\n", device->id, media_type));
            
            /* Now connect the requested media channel */
            if (media_type == SWAT_MEDIA_STANDARD)
            {
                swatL2capMediaConnectReq(device);
            }
            else if (media_type == SWAT_MEDIA_LOW_LATENCY)
            {
                swatRequestEsco(device);
            }
            else
            {
                SWAT_DEBUG(("[SWAT] Library error : Invalid media type [%x]\n", media_type));
            }
        }
        else
        {
            SWAT_DEBUG(("[SWAT] Device ID[%x] rejected request for media channel TYPE[%x]\n", device->id, media_type));
        }
    }
    else
    {
        /* Should never get here */
        SWAT_DEBUG(("[SWAT] LIBRARY ERROR : Device is null\n"));
    }
}






/*****************************************************************************/
void swatSendMediaCloseReq(uint16 device_id, swatMediaType media_type)
{
    remoteDevice * device = &swat->remote_devs[device_id];
    
    /* Build the close request packet */
    uint8 packet[4] = {SWAT_CLOSE, 1, SWAT_CMD};
    packet[PKT_PAYLOAD] = media_type;
    
    /* Ensure the media channel is in correct state to send a CLOSE_REQ */
    if ( ( ((media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state) != swat_media_open ) &&
         ( ((media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state) != swat_media_streaming ) )
    {
        SWAT_DEBUG(("[SWAT] Media channel TYPE[%x] in wrong STATE[%x] to send CLOSE_REQ\n", media_type, (media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state));
        swatSendMediaCloseCfmToClient(swat_wrong_state, device->id, media_type);
        return;
    }
    
    /* Send the packet */
    if (swatSendData(device, packet, sizeof(packet)))
    {
        swatSetMediaState(device, media_type, swat_media_closing);
        
        /* For eSCO disonnections, need to set the flag to know when the local device initiated the disconnection */
        if (media_type == SWAT_MEDIA_LOW_LATENCY)
        {
            device->initiated_esco_disconnect = 1;
        }
    }
    else
    {
        SWAT_DEBUG(("[SWAT] Error sending MEDIA_SUSPEND_CMD\n"));
    }
}


/*****************************************************************************/
void swatHandleCloseRequestFromRemoteDevice(remoteDevice * device, swatMediaType media_type)
{
    if (device)
    {
        /* Check the requested media channel is in the correct state to accept a CLOSE_CMD */
        if ( ( ((media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state) != swat_media_streaming ) &&
             ( ((media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state) != swat_media_open) )
        {
            /* Silently reject this request, Application does not need to be informed of this bad request */
            swatSendMediaCloseResponse(device, SWAT_REJECT_MEDIA, media_type);
            
            SWAT_DEBUG(("[SWAT] SWAT_CLOSE_CMD Handled\n"));
            device->signalling_block = 0;
        }
        else
        {
            /* SWAT Specification states the remote device cannot reject a CLOSE request; respond & update SWAT state to CLOSING and wait for remote device to close the media channel */
            swatSetMediaState(device, media_type, swat_media_closing);
            swatSendMediaCloseResponse(device, SWAT_ACCEPT_MEDIA, media_type);
        }
    }
    else
    {
        /* Should never get here */
        SWAT_DEBUG(("[SWAT] LIBRARY ERROR : Device is null\n"));
    }
}


/*****************************************************************************/
void swatSendMediaCloseResponse(remoteDevice * device, swatMediaResponse response, swatMediaType media_type)
{
    /* Build the MEDIA_CLOSE_RSP packet */
    uint8 packet[5] = {SWAT_CLOSE, 2, SWAT_RSP, 0, 0};
    packet[PKT_PAYLOAD]   = response;
    packet[PKT_PAYLOAD+1] = media_type;
    
    if (swatSendData(device, packet, sizeof(packet)))
    {
        SWAT_DEBUG(("[SWAT] Sent MEDIA_CLOSE_RSP\n"));
    }
    else
    {
        SWAT_DEBUG(("[SWAT] Error sending MEDIA_CLOSE_RSP\n"));
    }
    
    /* If the CLOSE_REQ was rejected, can action the next signalling command (if one exists) */
    if (response == SWAT_REJECT_MEDIA)
    {
        SWAT_DEBUG(("[SWAT] SWAT_CLOSE_CMD Handled\n"));
        device->signalling_block = 0;
    }
}


/*****************************************************************************/
void swatHandleCloseResponseFromRemoteDevice(remoteDevice * device, swatMediaResponse media_rsp, swatMediaType media_type)
{
    if (device)
    {
        /* Check we are expecting to recieve a CLOSE_RSP from the remote device */
        if ( ((media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state) != swat_media_closing )
        {
            SWAT_DEBUG(("[SWAT] Device ID[%x] sent CLOSE_RSP for media channel TYPE[%x] in wrong STATE[%x]\n", device->id, media_type, (media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state));
        }
        
        /* Check that media_type is valid */
        if (!checkMediaType(media_type))
        {
            SWAT_DEBUG(("[SWAT] Device ID[%x] sent CLOSE_RSP for invalid media channel TYPE[%x]\n", device->id, media_type));
            return;
        }
        
        /* Was the CLOSE_REQ accepted or rejected by the remote device? */
        if (media_rsp == SWAT_ACCEPT_MEDIA)
        {
            SWAT_DEBUG(("[SWAT] Device ID[%x] accepted CLOSE request for media channel TYPE[%x]\n", device->id, media_type));
            
            /* Close down media channel before sending MEDIA_CLOSE_CFM and updating the media channel's state */
            if (media_type == SWAT_MEDIA_STANDARD)
            {
                swatL2capMediaCloseReq(device);
            }
            else
            {
                swatRequestEscoDisconnect(device);
            }
        }
        else
        {
            /* Remote device is not allowed to reject the close request, inform the client task */
            swatSetMediaState(device, media_type, swat_media_open);
            swatSendMediaCloseCfmToClient(swat_rejected_by_remote_device, device->id, media_type);
        }
    }
    else
    {
        /* Should never get here */
        SWAT_DEBUG(("[SWAT] LIBRARY ERROR : Device is null\n"));
    }
}





/****************************************************************************/
void swatSendMediaStartReq(uint16 device_id, swatMediaType media_type)
{
    remoteDevice * device = &swat->remote_devs[device_id];
    
    /* Build the SWAT_START_REQ packet */
    uint8 packet[4] = {SWAT_START, 1, SWAT_CMD, 0};
    packet[PKT_PAYLOAD] = media_type;
    
    /* Ensure the media channel is in the right state to send a START request */
    if ( ((media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state) != swat_media_open)
    {   
        SWAT_DEBUG(("[SWAT] Media channel TYPE[%x] in wrong STATE[%x] to send START_REQ\n", media_type, (media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state));
        swatSendMediaStartCfmToClient(swat_wrong_state, device->id, media_type);
        return;
    }
    
    /* Send the packet */
    if (swatSendData(device, packet, sizeof(packet)))
    {
        SWAT_DEBUG(("[SWAT] Sent SWAT_START_REQ\n"));
        swatSetMediaState(device, media_type, swat_media_starting);
    }
    else
    {
        SWAT_DEBUG(("[SWAT] Error sending MEDIA_START_CMD\n"));
    }
}


/*****************************************************************************/
void swatHandleStartRequestFromRemoteDevice(remoteDevice * device, swatMediaType media_type)
{
    SWAT_DEBUG(("[SWAT] Recieved START from device ID[%x] for media channel TYPE[%x]\n", device->id, media_type));
    
    if (device)
    {
        /* Check the requested media channel is in the correct state to accept a START_CMD */
        if ( ((media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state) != swat_media_open)
        {
            swatSendMediaStartResponse(device, SWAT_REJECT_MEDIA, media_type);
            swatSendMediaStartCfmToClient(swat_wrong_state, device->id, media_type);
        }
        else if (swat->auto_handle)
        {
            /* Auto accept the START request from the remote device */
            swatSetMediaState(device, media_type, swat_media_streaming);
            swatSendMediaStartResponse(device, SWAT_ACCEPT_MEDIA, media_type);
            swatSendMediaStartCfmToClient(swat_success, device->id, media_type);
        }
        else
        {
            /* Send indication to client task so they can accept / reject the request */
            swatSetMediaState(device, media_type, swat_media_starting);
            swatSendMediaStartIndToClient(device->id, media_type);
        }
    }
    else
    {
        /* Should never get here */
        SWAT_DEBUG(("[SWAT] LIBRARY ERROR : Device is null\n"));
    }
}


/*****************************************************************************/
void swatHandleMediaStartResponseFromClient(remoteDevice * device, swatMediaType media_type, bool accept)
{
    if (BdaddrIsZero((const bdaddr *) &device->bd_addr))
    {
        /* Client task has supplied bad device ID data */
        swatSetMediaState(device, media_type, swat_media_open);
        swatSendMediaStartCfmToClient(swat_bad_data, 0, 0);
        
        /* Auto reject the media open request so the remote device does not end up in a bad state */
        swatSendMediaStartResponse(device, SWAT_REJECT_MEDIA, media_type);
    }
    else
    {
        /* Send the response as supplied by client task */
        swatSendMediaStartResponse(device, (accept) ? SWAT_ACCEPT_MEDIA : SWAT_REJECT_MEDIA, media_type);
        if (accept)
        {
            /* Application accepted the START command */
            swatSetMediaState(device, media_type, swat_media_streaming);
            swatSendMediaStartCfmToClient(swat_success, device->id, media_type);
        }
        else
        {
            /* Application rejected the START command */
            swatSetMediaState(device, media_type, swat_media_open);
            swatSendMediaStartCfmToClient(swat_success, device->id, media_type);
        }
    }
}


/*****************************************************************************/
void swatSendMediaStartResponse(remoteDevice * device, swatMediaResponse response, swatMediaType media_type)
{
    /* Build the MEDIA_START_RSP packet */
    uint8 packet[5] = {SWAT_START, 2, SWAT_RSP, 0, 0};
    packet[PKT_PAYLOAD]   = response;
    packet[PKT_PAYLOAD+1] = media_type;
    
    if (swatSendData(device, packet, sizeof(packet)))
    {
        SWAT_DEBUG(("[SWAT] Sent MEDIA_START_RSP for media TYPE[%x] RESPONSE[%x]\n", media_type, response));
    }
    else
    {
        SWAT_DEBUG(("[SWAT] Error sending MEDIA_START_RSP\n"));
    }
    
    /* Command has been handled; can now release command block to process next queued command (if one exists) */
    SWAT_DEBUG(("[SWAT] SWAT_START_CMD Handled\n"));
    device->signalling_block = 0;
}


/*****************************************************************************/
void swatHandleStartResponseFromRemoteDevice(remoteDevice * device, swatMediaResponse media_rsp, swatMediaType media_type)
{
    if (device)
    {
        /* Check we are expecting to recieve a START_RSP from the remote device */
        if ( ((media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state) != swat_media_starting)
        {
            SWAT_DEBUG(("[SWAT] Device ID[%x] sent START_RSP for media channel TYPE[%x] in wrong STATE[%x]\n", device->id, media_type, (media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state));
        }
        
        /* Check that media_type is valid */
        if (!checkMediaType(media_type))
        {
            SWAT_DEBUG(("[SWAT] Device ID[%x] sent START_RSP for invalid media channel TYPE[%x]\n", device->id, media_type));
            swatSendMediaStartCfmToClient(swat_bad_data, device->id, media_type);
        }
        
        /* Was the START_REQ accepted or rejected by the remote device? */
        if (media_rsp == SWAT_ACCEPT_MEDIA)
        {
            SWAT_DEBUG(("[SWAT] Device ID[%x] accepted START request for media channel TYPE[%x]\n", device->id, media_type));
            swatSetMediaState(device, media_type, swat_media_streaming);
            swatSendMediaStartCfmToClient(swat_success, device->id, media_type);
        }
        else
        {
            /* Remote device rejected START_REQ; return to open state and inform the client task */
            swatSetMediaState(device, media_type, swat_media_open);
            swatSendMediaStartCfmToClient(swat_rejected_by_remote_device, device->id, media_type);
        }
    }
    else
    {
        /* Should never get here */
        SWAT_DEBUG(("[SWAT] LIBRARY ERROR : Device is null\n"));
    }
}






/****************************************************************************/
void swatSendMediaSuspendReq(uint16 device_id, swatMediaType media_type)
{
    remoteDevice * device = &swat->remote_devs[device_id];
    
    /* Build the SWAT_SUSPEND_REQ packet */
    uint8 packet[4] = {SWAT_SUSPEND, 1, SWAT_CMD, 0};
    packet[PKT_PAYLOAD] = media_type;
    
    /* Ensure the media channel is in correct state to send a suspend request */
    if ( ((media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state) != swat_media_streaming)
    {
        SWAT_DEBUG(("[SWAT] Media channel TYPE[%x] in wrong STATE[%x] to send SUSPEND_REQ\n", media_type, (media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state));
        swatSendMediaSuspendCfmToClient(swat_wrong_state, device->id, media_type);
        return;
    }
    
    /* Send the packet */
    if (swatSendData(device, packet, sizeof(packet)))
    {
        SWAT_DEBUG(("[SWAT] Sent MEDIA_SUSPEND_REQ\n"));
        swatSetMediaState(device, media_type, swat_media_suspending);
    }
    else
    {
        SWAT_DEBUG(("[SWAT] Error sending MEDIA_SUSPEND_CMD\n"));
    }
}


/*****************************************************************************/
void swatHandleSuspendRequestFromRemoteDevice(remoteDevice * device, swatMediaType media_type)
{
    if (device)
    {
        /* Check the requested media channel is in the correct state to accept a SUSPEND_CMD */
        if ( ((media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state) != swat_media_streaming)
        {
            /* Reject suspend request as SWAT is in wrong state */
            swatSendMediaSuspendResponse(device, SWAT_REJECT_MEDIA, media_type);
        }
        else
        {
            /* SWAT specification states the SUSPEND request can never be rejected; respond & return to OPEN state */
            swatSetMediaState(device, media_type, swat_media_open);
            swatSendMediaSuspendResponse(device, SWAT_ACCEPT_MEDIA, media_type);
            
            /* Inform client task the remote device has suspended media streaming */
            swatSendMediaSuspendIndToClient(device->id, media_type);
        }
    }
    else
    {
        /* Should never get here */
        SWAT_DEBUG(("[SWAT] LIBRARY ERROR : Device is null\n"));
    }
}


/*****************************************************************************/
void swatSendMediaSuspendResponse(remoteDevice * device, swatMediaResponse response, swatMediaType media_type)
{
    /* Build the MEDIA_SUSPEND_RSP packet */
    uint8 packet[5] = {SWAT_SUSPEND, 2, SWAT_RSP, 0, 0};
    packet[PKT_PAYLOAD]   = response;
    packet[PKT_PAYLOAD+1] = media_type;
    
    if (swatSendData(device, packet, sizeof(packet)))
    {
        SWAT_DEBUG(("[SWAT] Sent MEDIA_SUSPEND_RSP\n"));
    }
    else
    {
        SWAT_DEBUG(("[SWAT] Error sending MEDIA_SUSPEND_RSP\n"));
    }
    
    /* Command has been handled; can now release command block to process next queued command (if one exists) */
    SWAT_DEBUG(("[SWAT] SWAT_SUSPEND_CMD Handled\n"));
    device->signalling_block = 0;
}


/*****************************************************************************/
void swatHandleSuspendResponseFromRemoteDevice(remoteDevice * device, swatMediaResponse media_rsp, swatMediaType media_type)
{
    if (device)
    {
        /* Check we are expecting to recieve a SUSPEND_RSP from the remote device */
        if ( ((media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state) != swat_media_suspending)
        {
            SWAT_DEBUG(("[SWAT] Device ID[%x] sent SUSPEND_RSP for media channel TYPE[%x] in wrong STATE[%x]\n", device->id, media_type, (media_type == SWAT_MEDIA_STANDARD) ? device->media_state : device->media_ll_state));
            return;
        }
        
        /* Check that media_type is valid */
        if (!checkMediaType(media_type))
        {
            SWAT_DEBUG(("[SWAT] Device ID[%x] sent SUSPEND_RSP for invalid media channel TYPE[%x]\n", device->id, media_type));
            swatSendMediaSuspendCfmToClient(swat_bad_data, device->id, media_type);
            return;
        }
        
        /* Was the SUSPEND_REQ accepted or rejected by the remote device? */
        if (media_rsp == SWAT_ACCEPT_MEDIA)
        {
            SWAT_DEBUG(("[SWAT] Device ID[%x] accepted SUSPEND request for media channel TYPE[%x]\n", device->id, media_type));
            swatSetMediaState(device, media_type, swat_media_open);
            swatSendMediaSuspendCfmToClient(swat_success, device->id, media_type);
        }
        else
        {
            /* Remote device rejected SUSPEND_REQ; return to streaming state and inform the client task */
            swatSetMediaState(device, media_type, swat_media_streaming);
            swatSendMediaSuspendCfmToClient(swat_rejected_by_remote_device, device->id, media_type);
        }
    }
    else
    {
        /* Should never get here */
        SWAT_DEBUG(("[SWAT] LIBRARY ERROR : Device is null\n"));
    }
}






/****************************************************************************/
void swatSendSetVolumeReq(uint16 device_id, uint8 volume, uint8 sub_trim)
{
    remoteDevice * device = &swat->remote_devs[device_id];
    
    if (device)
    {
        /* Build the SWAT_SET_VOLUME_REQ packet */
        uint8 packet[5] = {SWAT_SET_VOLUME, 2, SWAT_CMD, 0, 0};
        packet[PKT_PAYLOAD]   = volume;
        packet[PKT_PAYLOAD+1] = sub_trim;
        
        /* Send the packet */
        if (swatSendData(device, packet, sizeof(packet)))
        {
            SWAT_DEBUG(("[SWAT] Sent SWAT_SET_VOLUME_REQ VOLUME[%x]\n", volume));
        }
        else
        {
            SWAT_DEBUG(("[SWAT] Error sending SET_VOLUME_REQ\n"));
        }
    }
    else
    {
        SWAT_DEBUG(("[SWAT] Device is NULL\n"));
    }
}


/****************************************************************************/
void swatHandleVolumeRequestFromRemoteDevice(remoteDevice *device, uint8 volume, uint8 sub_trim)
{
    /* Build the SET_VOLUME_RSP packet */
    uint8 packet[5] = {SWAT_SET_VOLUME, 2, SWAT_RSP, 0, 0};
    packet[PKT_PAYLOAD]   = volume;
    packet[PKT_PAYLOAD+1] = sub_trim;
    
    if (swatSendData(device, packet, sizeof(packet)))
    {
        SWAT_DEBUG(("[SWAT] Sent SET_VOLUME_RSP\n"));
        
        /* Update volume for remote device and inform client task of the new volume */
        device->volume = volume;
        device->sub_trim = sub_trim;
        swatSendVolumeIndToClient(device->id, volume, sub_trim);
    }
    else
    {
        SWAT_DEBUG(("[SWAT] Error sending SET_VOLUME_RSP\n"));
    }
    
    /* Command has been handled; can now release command block to process next queued command (if one exists) */
    SWAT_DEBUG(("[SWAT] SWAT_VOLUME_CMD Handled\n"));
    device->signalling_block = 0;
}


/****************************************************************************/
void swatHandleVolumeResponseFromRemoteDevice(remoteDevice *device, uint8 volume, uint8 sub_trim)
{
    SWAT_DEBUG(("[SWAT] Volume sync'd with remote device ID[%x] VOLUME[%x]\n", device->id, volume));
    swatSendVolumeCfmToClient(swat_success, device->id, volume, sub_trim);
}

/*****************************************************************************/
void swatSendSampleRateCommand(uint16 device_id, uint16 rate)
{
    remoteDevice * device = &swat->remote_devs[device_id];
    
    /* Build the SWAT_SAMPLE_RATE_REQ packet */
    uint8 packet[5] = {SWAT_SAMPLE_RATE, 2, SWAT_CMD};
    /* Stuff sample rate (uint16) into 2x (uint8) for sending over the air */
    packet[PKT_PAYLOAD]   = rate & 0xFF;
    packet[PKT_PAYLOAD+1] = (rate >> 8) & 0xFF;
    
    /* Check the device supplied is valid */
    if (BdaddrIsZero(&device->bd_addr))
    {
        swatSendSampleRateCfmToClient(swat_bad_data, device_id, 0);
        return;
    }
    
    /* Check there is a connected signalling channel with the device */
    if (device->signalling_state != swat_signalling_connected)
    {
        swatSendSampleRateCfmToClient(swat_no_signalling_connection, device_id, 0);
        return;
    }
    
    /* Send SWAT_SAMPLE_RATE packet to the device */
    if (swatSendData(device, packet, sizeof(packet)))
    {
        SWAT_DEBUG(("[SWAT] Sent MEDIA_SEND_SAMPLE_RATE_CMD to device [%x] rate [%u]\n", device->id, rate));
        
        /* Update the sample rate being used for the device */
        device->sample_rate = rate;
    }
    else
    {
        swatSendSampleRateCfmToClient(swat_bad_data, device_id, 0);
        SWAT_DEBUG(("[SWAT] Error sending MEDIA_OPEN_CMD\n"));
    }
}


/*****************************************************************************/
void swatHandleSampleRateCommandFromRemoteDevice(remoteDevice * device, uint16 rate)
{
    if (device)
    {
        SWAT_DEBUG(("[SWAT] Device ID[%x] sent SAMPLE_RATE_CMD [%u]\n", device->id, rate));
        
        if (swat->auto_handle)
        {
            /* Update the sample rate being used for the remote device */
            device->sample_rate = rate;
            
            /* Inform application the remote device sent a SAMPLE_RATE message */
            swatSendSampleRateCfmToClient(swat_success, device->id, device->sample_rate);
            
            /* Respond to the remote device that sample rate has been acknowledged */
            swatSendSampleRateResponse(device, SWAT_ACCEPT_MEDIA, rate);
        }
        else
        {
            /* Send IND to client and wait for the client to respond before actioning next SWAT command (if one exists) */
            swatSendSampleRateIndToClient(device->id, rate);
        }
    }
    else
    {
        /* Should never get here */
        SWAT_DEBUG(("[SWAT] LIBRARY ERROR : Device is null\n"));
        Panic();
    }
}


/*****************************************************************************/
void swatHandleSampleRateResponseFromClient(remoteDevice * device, uint16 rate)
{
    if (BdaddrIsZero((const bdaddr *) &device->bd_addr))
    {
        SWAT_DEBUG(("[SWAT] SAMPLE_RATE_RSP bad data\n"));
        
        /* Client task has supplied bad device ID data */
        swatSendSampleRateCfmToClient(swat_bad_data, 0, 0);
    }
    else
    {
        /* Update the sample rate being used for the remote device */
        device->sample_rate = rate;
        
        /* Send the response as supplied by client task */
        swatSendSampleRateResponse(device, SWAT_ACCEPT_MEDIA, rate);
        swatSendSampleRateCfmToClient(swat_success, device->id, rate);
    }
}


/*****************************************************************************/
void swatSendSampleRateResponse(remoteDevice * device, swatMediaResponse response, uint16 rate)
{
    /* Build the SAMPLE_RATE_RSP packet */
    uint8 packet[6] = {SWAT_SAMPLE_RATE, 3, SWAT_RSP};
    packet[PKT_PAYLOAD]   = response;
    /* Stuff sample rate (uint16) into 2x (uint8) for sending over the air */
    packet[PKT_PAYLOAD+1]   = rate & 0xFF;
    packet[PKT_PAYLOAD+2] = (rate >> 8) & 0xFF;
    
    if (swatSendData(device, packet, sizeof(packet)))
    {
        SWAT_DEBUG(("[SWAT] Sent SAMPLE_RATE_RSP rate[%u] response[%x]\n", rate, packet[PKT_PAYLOAD]));
    }
    else
    {
        SWAT_DEBUG(("[SWAT] Error sending SAMPLE_RATE_RSP\n"));
    }
    
    /* Command has been handled; can now release command block to process next queued command (if one exists) */
    SWAT_DEBUG(("[SWAT] SWAT_SAMPLE_RATE_CMD Handled\n"));
    device->signalling_block = 0;
}


/*****************************************************************************/
void swatHandleSampleRateResponseFromRemoteDevice(remoteDevice * device, swatMediaResponse media_rsp, uint16 rate)
{
    SWAT_DEBUG(("[SWAT] Device ID[%x] responded to sample rate [%u]\n", device->id, rate));
    
    /* Double check the rate is what we sent */
    if (rate == device->sample_rate)
    {
        /* Rates match so send CFM message to the client task */
        swatSendSampleRateCfmToClient(swat_success, device->id, rate);
    }
    else
    {
        /* Rate did not match so send again; remote device MUST know the correct sample rate being used */
        swatSendSampleRateCommand(device->id, device->sample_rate);
    }
}






/****************************************************************************/
void swatSendGeneralReject(remoteDevice * device, swatCommandId cmd_id)
{
    /* Build the SWAT_GENERAL_REJECT packet */
    uint8 packet[4] = {SWAT_GENERAL_REJECT, 1, SWAT_CMD, 0};
    packet[PKT_PAYLOAD] = cmd_id;
    
    /* Send GENERAL_REJECT packet to the device */
    if (swatSendData(device, packet, sizeof(packet)))
    {
        SWAT_DEBUG(("[SWAT] Sent GENERAL_REJECT to device ID[%x] for unrecognised CMD[%x]\n", device->id, cmd_id));
    }
    else
    {
        SWAT_DEBUG(("[SWAT] Error sending GENERAL_REJECT to device ID[%x]\n", device->id));
    }
    
    /* release command block */
    SWAT_DEBUG(("[SWAT] SWAT_GENERAL_REJECT_CMD Handled\n"));
    device->signalling_block = 0;
}


/****************************************************************************/
void swatHandleGeneralRejectFromRemoteDevice(uint16 device_id, uint16 cmd_id)
{
    /* Check that the command is not one that we think should be valid */
    switch (cmd_id)
    {
        case SWAT_OPEN:
        case SWAT_CLOSE:
        case SWAT_START:
        case SWAT_SUSPEND:
        case SWAT_SET_VOLUME:
        case SWAT_SAMPLE_RATE:
        case SWAT_GENERAL_REJECT:
        {
            SWAT_DEBUG(("[SWAT] Remote device ID[%x] Does not handle valid SWAT command ID[%x]\n", device_id, cmd_id));
            /* It is possible the command we sent got corrupted over the air */
            /* It is also possible the remote device does not handle the SWAT command we sent */
        }
        break;
        default:
        {
            /* Should never get here as we only send commands we understand */
            SWAT_DEBUG(("[SWAT] LIBRARY ERROR : Sent invalid SWAT command ID[%x] That was rejected by the remote device ID[%x]\n", cmd_id, device_id));
        }
        break;
    }
}


/****************************************************************************/
/* Soundbar function to get the version number from the Subwoofer device    */
/****************************************************************************/
void swatSendGetVersionNoReq(uint16 device_id)
{
    remoteDevice * device = &swat->remote_devs[device_id];
    
    if (device)
    {
        /* Build the SWAT_SET_VOLUME_REQ packet */
        uint8 packet[3] = {SWAT_GET_VERSION, 0, SWAT_CMD};

        /* set to legacy (ROM) subwoofer version before querying version as
           ROM will not respond with a version number */
        device->version_major = SUBWOOFER_MAJOR_VERSION_LEGACY;
        device->version_minor = SUBWOOFER_MINOR_VERSION;
        
        /* Send the packet */
        if (swatSendData(device, packet, sizeof(packet)))
        {
            SWAT_DEBUG(("[SWAT] Sent SWAT_GET_VERSION\n"));
        }
        else
        {
            SWAT_DEBUG(("[SWAT] Error sending SWAT_GET_VERSION\n"));
        }
    }
    else
    {
        SWAT_DEBUG(("[SWAT] Device is NULL\n"));
    }
}


/****************************************************************************/
void swatHandleVersionNoRequestFromRemoteDevice(remoteDevice *device)
{
    /* Build the SWAT_GET_VERSION packet */
    uint8 packet[5] = {SWAT_GET_VERSION, 2, SWAT_RSP, 0, 0};
    packet[PKT_PAYLOAD]   = SUBWOOFER_MAJOR_VERSION;
    packet[PKT_PAYLOAD+1] = SUBWOOFER_MINOR_VERSION;
    
    if (swatSendData(device, packet, sizeof(packet)))
    {
        SWAT_DEBUG(("[SWAT] Sent SWAT_GET_VERSION RSP \n"));
        
        swatSendVersionNoIndToClient(device->id);
    }
    else
    {
        SWAT_DEBUG(("[SWAT] Error sending SWAT_GET_VERSION_RSP \n"));
    }
    
    /* Command has been handled; can now release command block to process next queued command (if one exists) */
    SWAT_DEBUG(("[SWAT] SWAT_GET_VERSION Handled\n"));
    device->signalling_block = 0;
}


/****************************************************************************/
void swatHandleVersionNoResponseFromRemoteDevice(remoteDevice *device, uint16 major, uint16 minor)
{
    SWAT_DEBUG(("[SWAT] Version No Major[%x] Minor[%x]\n", major, minor));

    /* update version number, ESCO packet configuration varies by version number */
    device->version_major = major;
    device->version_minor = minor;
    /* send notification of the version number to the application */
    swatSendVersionNoCfmToClient(swat_success, device->id, major, minor);
}


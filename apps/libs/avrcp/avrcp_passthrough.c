/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_passthrough.c

DESCRIPTION
    This file defines all API functions related to PASSTHROUGH. This includes
    - PASSTHROUGH
    - Basic Group Navigation  


NOTES

*/


/****************************************************************************
    Header files
*/
#include <panic.h>
#include <source.h>
#include "avrcp_signal_passthrough.h"
#include "avrcp_init.h"

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/****************************************************************************
* NAME    
* sendGroupRequest    
*
* DESCRIPTION
* Send a Group request to the TG. 
*******************************************************************************/
static void sendGroupRequest(AVRCP *avrcp, uint8 vendor_id, bool state)
{
    Source src;    
    const uint16 length = 5;
    uint8 *data = (uint8 *) PanicUnlessMalloc(length);

    /* Send a group request as part of the Passthrough command. */
    data[0] = (AVRCP_BT_COMPANY_ID >> 16) & 0xff;
    data[1] = (AVRCP_BT_COMPANY_ID >> 8) & 0xff;
    data[2] = AVRCP_BT_COMPANY_ID & 0xff;
    data[3] = 0x00;
    data[4] = vendor_id;

    src = avrcpSourceFromData(avrcp, data, length);

    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_PASSTHROUGH_REQ);
        message->subunit_type = subunit_panel;
        message->subunit_id = 0;
        message->state = state;
        message->opid = opid_vendor_unique;
        message->operation_data = src;
        message->operation_data_length = length;
        if (vendor_id)
        {
            message->pending = avrcp_previous_group;
        }
        else
        {
            message->pending = avrcp_next_group;
        }

        MessageSendConditionally(&avrcp->task, 
                                AVRCP_INTERNAL_PASSTHROUGH_REQ, 
                                message, 
                                &avrcp->pending);
    }
}

/****************************************************************************
*NAME    
*    AvrcpPassthroughRequest
*
*DESCRIPTION
*    This function is called to request that a Pass Through control command
*    is sent to the target on the the connection identified by the sink.
*    
*    The Passthrough command is used to convey the proper user operation to 
*    the target(transparently to the user).
*    
*    
*    identifier           - Used to identify the matching AV/C Command 
*    subunit_type
*    subunit_id          - AV/C protocol - Used to form the targets address
*    opid                - Identifies the button pressed
*    state               - Indicates the user action of pressing or releasing
*                          the button identified by opid.  Active low.
*    operation_data     - Required for the Vendor Unique operation.  For other 
*    data_length          operations both fields should be zero
*
*                     
*    
*                     -----------------------------------------------
*                    | MSB |      |   |     |    |     |    |    LSB |
*                    |-----------------------------------------------|
*    opcode          |              PASSTHROUGH (0x7C)               |
*                     -----------------------------------------------
*    operand(0)      |state|            operation_id                 |
*                      -----------------------------------------------
*    operand(1)      |          operation data field length          |
*                     -----------------------------------------------
*    operand(2)      |     operation data(operation id dependant)    |
*        :            |                                              |
*                     -----------------------------------------------
*
*MESSAGE RETURNED
*    AVRCP_PASSTHROUGH_CFM 
********************************************************************************/
void AvrcpPassthroughRequest(  AVRCP               *avrcp, 
                               avc_subunit_type    subunit_type, 
                               avc_subunit_id      subunit_id, 
                               bool                state, 
                               avc_operation_id    opid, 
                               uint16              operation_data_length, 
                               Source              operation_data )
{
    
#ifdef AVRCP_DEBUG_LIB    
    if (subunit_type > 0x1F)
    {
        AVRCP_DEBUG(("Out of range subunit type  0x%x\n", subunit_type));
    }
    if (subunit_id > 0x07)
    {
        AVRCP_DEBUG(("Out of range subunit id  0x%x\n", subunit_id));
    }
    if (opid > 0x7F)
    {
        AVRCP_DEBUG(("Out of range op id  0x%x\n", opid));
    }
#endif
    
    if (isAvrcpConnected(avrcp))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_PASSTHROUGH_REQ);
        message->subunit_type = subunit_type;
        message->subunit_id = subunit_id;
        message->state = state;
        message->opid = opid;
        message->pending = avrcp_passthrough;

        if (opid == opid_vendor_unique)
        {
            message->operation_data = operation_data;
            message->operation_data_length = operation_data_length;
        }
        else
        {
            if (SourceSize(operation_data))
                SourceEmpty(operation_data);
            message->operation_data = 0;
            message->operation_data_length = 0;
        }

        MessageSendConditionally(&avrcp->task, 
                                AVRCP_INTERNAL_PASSTHROUGH_REQ, 
                                message, 
                                &avrcp->pending);
    }
    else
    {
        /* Device is not connected */
        avrcpSendPassthroughCfmToClient(avrcp, avrcp_device_not_connected);
    }
}

/**************************************************************************
* NAME
*  AvrcpPassthroughRequestCancelAll
* 
* DESCRIPTION
*  This function is used to Cancel all queued Passthrough requests.
*
**************************************************************************/
void AvrcpPassthroughRequestCancelAll(AVRCP *avrcp)
{
    MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_PASSTHROUGH_REQ);
}

/**************************************************************************
* NAME
*  AvrcpNextGroupRequest
* 
* DESCRIPTION
*  This function is used to move to the first song in the next group.
*
**************************************************************************/
void AvrcpNextGroupRequest(AVRCP *avrcp, bool state)
{

    if(!isAvrcpMetadataEnabled(avrcpGetDeviceTask()))
    {
        /* Must have Metadata supported at this end to be able to send 
           this command.  
           Maybe should look at remote supported features also. 
        */
        avrcpSendCommonMetadataCfm(avrcp, 
                                   avrcp_unsupported,
                                   AVRCP_NEXT_GROUP_CFM);
        return;
    }

    if(avrcp->dataFreeTask.sent_data)
    {
        /* Previous data was not flushed */
        avrcpSendCommonMetadataCfm(avrcp, 
                                   avrcp_busy,
                                   AVRCP_NEXT_GROUP_CFM);
    }
    else if(isAvrcpConnected(avrcp))
    {
        /* Send the data if none is already being sent. */
        sendGroupRequest(avrcp, 0, state);
    }
    else
    {
        /* Device Not connected */
        avrcpSendCommonMetadataCfm(avrcp, 
                                   avrcp_device_not_connected,
                                   AVRCP_NEXT_GROUP_CFM);
    }
}

/**************************************************************************
* NAME
*  AvrcpPreviousGroupRequest
* 
* DESCRIPTION
*  This function is used to move to the first song in the previous group.
*
**************************************************************************/
void AvrcpPreviousGroupRequest(AVRCP *avrcp, bool state)
{
    if(!isAvrcpMetadataEnabled(avrcpGetDeviceTask()))
    {
        /* Must have Metadata supported at this end to be able to 
           send this command.  
           Maybe should look at remote supported features also. 
        */
        avrcpSendCommonMetadataCfm(avrcp, avrcp_unsupported, 
                                   AVRCP_PREVIOUS_GROUP_CFM);
        return;
    }

    if(avrcp->dataFreeTask.sent_data)
    {
        /* Previous data was not flushed */
        avrcpSendCommonMetadataCfm(avrcp, 
                                   avrcp_busy,
                                   AVRCP_PREVIOUS_GROUP_CFM);
    }
    else if(isAvrcpConnected(avrcp))
    {
        /* Send the data if none is already being sent. */
        sendGroupRequest(avrcp, 1, state);
    }
    else
    {
        /* Device Not connected */
        avrcpSendCommonMetadataCfm(avrcp, 
                                   avrcp_device_not_connected,
                                   AVRCP_PREVIOUS_GROUP_CFM); 
    }
}

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
/**************************************************************************
* NAME
*  AvrcpPassthroughResponse
* 
* DESCRIPTION
*  API to send a Pass Through Response to CT. 
**************************************************************************/
void AvrcpPassthroughResponse(AVRCP *avrcp, avrcp_response_type response)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_PASSTHROUGH_RES);

#ifdef AVRCP_DEBUG_LIB    
    if (response > avctp_response_interim)
    {
        AVRCP_DEBUG(("Out of range response  0x%x\n", response));
    }
#endif

    message->response = response;
    MessageSend(&avrcp->task, AVRCP_INTERNAL_PASSTHROUGH_RES, message);
}


/**************************************************************************
* NAME
*  AvrcpNextGroupResponse
* 
* DESCRIPTION
*  API to send Next Group response to the CT.
**************************************************************************/
void AvrcpNextGroupResponse(AVRCP *avrcp, avrcp_response_type response)
{
    /* Send the response only if the command arrived to start with. */
    if (avrcp->bitfields.block_received_data == avrcp_next_group)
    {
        sendGroupResponse(avrcp, response);
    }
    else
    {
        AVRCP_INFO(("AvrcpNextGroupResponse in wrong state\n"));
    }
}

/**************************************************************************
* NAME
*  AvrcpPreviousGroupResponse
* 
* DESCRIPTION
*  API to send Previous Group response to the CT.
**************************************************************************/
void AvrcpPreviousGroupResponse(AVRCP *avrcp, avrcp_response_type response)
{
    /* Send the response only if the command arrived to start with. */
    if (avrcp->bitfields.block_received_data == avrcp_previous_group)
    {
        sendGroupResponse(avrcp, response);
    }
    else
    {
        AVRCP_INFO(("AvrcpPreviousGroupResponse in wrong state\n"));
    }
}
#endif /* !AVRCP_CT_ONLY_LIB*/





/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_profile_handler.c        

DESCRIPTION
   This file implements the profile state machine and the task handler function.

NOTES

*/


/****************************************************************************
    Header files
*/
#include <stdlib.h>
#include <connection.h>
#include <panic.h>
#include <source.h>
#include "avrcp_caps_handler.h"
#include "avrcp_connect_handler.h"
#include "avrcp_continuation_handler.h"
#include "avrcp_element_attributes_handler.h"
#include "avrcp_player_app_settings_handler.h"
#include "avrcp_l2cap_handler.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_notification_handler.h"
#include "avrcp_signal_passthrough.h"
#include "avrcp_signal_unit_info.h"
#include "avrcp_signal_vendor.h"
#include "avrcp_absolute_volume.h"
#include "avrcp_profile_handler.h"
#include "avrcp_browsing_handler.h"

#ifdef HYDRACORE
PRESERVE_TYPE_FOR_DEBUGGING(AVRCP_INTERNAL_T)
#endif

/****************************************************************************
 *NAME    
 *    avrcpHandleReadyMessages    
 *
 *DESCRIPTION
 *  Profile state machine to handle Messages in the Ready state. 
 *
 *PARAMETERS
 *  avrcp       - AVRCP Instance
 *  MessageId   - Message ID.
 *  message     - Message.
 ****************************************************************************/
static bool avrcpHandleReadyMessages(AVRCP *avrcp, 
                                     MessageId id, 
                                     Message   message)
{
    switch(id)
    {
        case AVRCP_INTERNAL_SDP_SEARCH_REQ:
            avrcpSdpSearchRequest(avrcp,
                    &((const AVRCP_INTERNAL_SDP_SEARCH_REQ_T*)message)->bd_addr,
                    avrcp_sdp_search_and_connect_pending);

            break;
        case AVRCP_INTERNAL_CONNECT_REQ:
            avrcpHandleInternalConnectReq(avrcp, 
                                   (const AVRCP_INTERNAL_CONNECT_REQ_T *) message);
            break;

        default:
            return FALSE;  
    }

    return TRUE;
}

/****************************************************************************
 *NAME    
 *    avrcpHandleConnectingMessages    
 *
 *DESCRIPTION
 *  Profile state machine to handle Messages in the Connecting state. 
 *
 *PARAMETERS
 *  avrcp       - AVRCP Instance
 *  MessageId   - Message ID.
 *  message     - Message.
 ****************************************************************************/
static bool avrcpHandleConnectingMessages(AVRCP *avrcp, 
                                         MessageId id, 
                                         Message   message)
{
    switch(id)
    {
       case AVRCP_INTERNAL_CONNECT_RES:
             avrcpHandleInternalL2capConnectRes(avrcp, 
                             (const AVRCP_INTERNAL_CONNECT_RES_T *) message);
             break;

       case  CL_L2CAP_CONNECT_CFM:
            avrcpHandleL2capConnectCfm(avrcp, 
                       (const CL_L2CAP_CONNECT_CFM_T *) message);
            break;

       case CL_L2CAP_DISCONNECT_CFM:
            avrcpHandleL2capDisconnectCfm(avrcp,
                    (const CL_L2CAP_DISCONNECT_CFM_T *) message);
            break;
            
        case CL_L2CAP_DISCONNECT_IND:
            avrcpHandleL2capDisconnectInd(avrcp, 
                          (const CL_L2CAP_DISCONNECT_IND_T *) message);
              break;

        default:
            return FALSE;  
    }

    return TRUE;
}

/****************************************************************************
 *NAME    
 *    avrcpHandleConnectedMessages    
 *
 *DESCRIPTION
 *  Profile state machine to handle Messages in the Connected state. 
 *
 *PARAMETERS
 *  avrcp       - AVRCP Instance
 *  MessageId   - Message ID.
 *  message     - Message.
 ****************************************************************************/
static bool avrcpHandleConnectedMessages(AVRCP *avrcp, 
                                         MessageId id, 
                                         Message   message)
{
    switch(id)
    {

        case AVRCP_INTERNAL_DISCONNECT_REQ:
            avrcpHandleInternalDisconnectReq(avrcp);
            break;

        case AVRCP_INTERNAL_GET_FEATURES:
            avrcpGetSupportedFeatures(avrcp);
            break;

        case AVRCP_INTERNAL_GET_EXTENSIONS:
            avrcpGetProfileVersion(avrcp);
            break;

    #ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
        case AVRCP_INTERNAL_PASSTHROUGH_REQ:
            avrcpHandleInternalPassThroughReq(avrcp, 
                           (const AVRCP_INTERNAL_PASSTHROUGH_REQ_T *) message);
            break;

        case AVRCP_INTERNAL_UNITINFO_REQ:
            avrcpHandleInternalUnitInfoReq(avrcp);
            break;

        case AVRCP_INTERNAL_SUBUNITINFO_REQ:
            avrcpHandleInternalSubUnitInfoReq(avrcp, 
                            (const AVRCP_INTERNAL_SUBUNITINFO_REQ_T *) message);
            break;

        case AVRCP_INTERNAL_VENDORDEPENDENT_REQ:
            avrcpHandleInternalVendorDependentReq(avrcp, 
                            (const AVRCP_INTERNAL_VENDORDEPENDENT_REQ_T *) message);
            break;

    #endif /* !AVRCP_TG_ONLY_LIB */

    #ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

        case AVRCP_INTERNAL_PASSTHROUGH_RES:
            avrcpHandleInternalPassThroughRes(avrcp, 
                            (const AVRCP_INTERNAL_PASSTHROUGH_RES_T *) message);
            break;


        case AVRCP_INTERNAL_UNITINFO_RES:
            avrcpHandleInternalUnitInfoRes( avrcp, 
                            (const AVRCP_INTERNAL_UNITINFO_RES_T *) message);
            break;


        case AVRCP_INTERNAL_SUBUNITINFO_RES:
            avrcpHandleInternalSubUnitInfoRes(avrcp, 
                            (const AVRCP_INTERNAL_SUBUNITINFO_RES_T *) message);
            break;


        case AVRCP_INTERNAL_VENDORDEPENDENT_RES:
            avrcpHandleInternalVendorDependentRes(avrcp, 
                            (const AVRCP_INTERNAL_VENDORDEPENDENT_RES_T *) message);
            break;

       case AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT:
            avrcpHandleInternalSendResponseTimeout(avrcp,
                          (const AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT_T *) message);
            break;

        case AVRCP_INTERNAL_GET_CAPS_RES:
            avrcpHandleInternalGetCapsResponse(avrcp, 
                               (AVRCP_INTERNAL_GET_CAPS_RES_T *) message);
            break;

        case AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES:
            avrcpHandleInternalListAppAttributesResponse(avrcp, 
                           (AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES_T *) message);
            break;

        case AVRCP_INTERNAL_LIST_APP_VALUE_RES:
            avrcpHandleInternalListAppValuesResponse(avrcp, 
                            (AVRCP_INTERNAL_LIST_APP_VALUE_RES_T *) message);
            break;

        case AVRCP_INTERNAL_GET_APP_VALUE_RES:
            avrcpHandleInternalGetAppValueResponse(avrcp, 
                            (AVRCP_INTERNAL_GET_APP_VALUE_RES_T *) message);
            break;

        case AVRCP_INTERNAL_SET_APP_VALUE_RES:
            avrcpHandleInternalSetAppValueResponse(avrcp, 
                             (AVRCP_INTERNAL_SET_APP_VALUE_RES_T *) message);
            break;

        case AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES:
            avrcpHandleInternalGetAppAttributeTextResponse(avrcp, 
                     (AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES_T *) message);
            break;

        case AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES:
            avrcpHandleInternalGetAppValueTextResponse(avrcp, 
                     (AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES_T *) message);
            break;

        case AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES:
            avrcpHandleInternalGetElementAttributesResponse(avrcp, 
                       (AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES_T *) message);
            break;

        case AVRCP_INTERNAL_GET_PLAY_STATUS_RES:
            avrcpHandleInternalGetPlayStatusResponse(avrcp, 
                      (AVRCP_INTERNAL_GET_PLAY_STATUS_RES_T *) message);
            break;

        case AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES:
            avrcpHandleInternalEventPlaybackStatusChangedResponse(avrcp, 
                 (const AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES_T*)message);
            break;

        case AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES:
            avrcpHandleInternalEventTrackChangedResponse(avrcp, 
                       (const AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES_T *) message);
            break;

        case AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES:
            avrcpHandleInternalEventPlaybackPosChangedResponse(avrcp, 
                     (const AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES_T*)message);
            break;

        case AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES:
            avrcpHandleInternalEventBattStatusChangedResponse(avrcp, 
                     (const AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES_T*) message);
            break;

        case AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES:
            avrcpHandleInternalEventSystemStatusChangedResponse(avrcp, 
                  (const AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES_T*) message);
            break;

       case AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES:
           avrcpHandleInternalEventPlayerAppSettingChangedResponse(avrcp, 
              (const AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES_T*) message);
            break;

       case AVRCP_INTERNAL_REJECT_METADATA_RES:
           avrcpHandleInternalRejectMetadataResponse(avrcp, 
                    ((const AVRCP_INTERNAL_REJECT_METADATA_RES_T*)message)->response,
                    ((const AVRCP_INTERNAL_REJECT_METADATA_RES_T*)message)->pdu_id);
           break;   

       case AVRCP_INTERNAL_ABORT_CONTINUING_RES:
           avrcpHandleInternalAbortContinuingResponse(avrcp, 
                        (const AVRCP_INTERNAL_ABORT_CONTINUING_RES_T *) message);
           break;

       case AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET:
           avrcpHandleNextContinuationPacket(avrcp, 
                     (const AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET_T *) message);
           break;

        case AVRCP_INTERNAL_GROUP_RES:
           avrcpHandleInternalGroupResponse(avrcp, 
                     (const AVRCP_INTERNAL_GROUP_RES_T *) message);
           break;   

        case AVRCP_INTERNAL_COMMON_METADATA_CONTROL_RES:
            avrcpHandleCommonMetadataControlResponse(avrcp, 
                     (const AVRCP_INTERNAL_COMMON_METADATA_CONTROL_RES_T*) message);
            break;

        case AVRCP_INTERNAL_SET_ABSOLUTE_VOL_RES:
            avrcpHandleInternalAbsoluteVolumeRsp(avrcp,
                     (const AVRCP_INTERNAL_SET_ABSOLUTE_VOL_RES_T*) message);
            break; 
   
        case AVRCP_INTERNAL_EVENT_VOLUME_CHANGED_RES:
            avrcpHandleInternalAbsoluteVolumeEvent(avrcp,
                     (const AVRCP_INTERNAL_EVENT_VOLUME_CHANGED_RES_T*) message);
            break; 

        case AVRCP_INTERNAL_EVENT_ADDRESSED_PLAYER_CHANGED_RES:
            avrcpHandleInternalAddressPlayerChangedEvent(avrcp,
                     (const AVRCP_INTERNAL_EVENT_ADDRESSED_PLAYER_CHANGED_RES_T*)message);
            break;

        case AVRCP_INTERNAL_EVENT_UIDS_CHANGED_RES:
            avrcpHandleInternalEventUidsChangedEvent(avrcp,
                (const AVRCP_INTERNAL_EVENT_UIDS_CHANGED_RES_T*)message);
            break;

        case AVRCP_INTERNAL_EVENT_COMMON_RES:
            avrcpHandleInternalCommonEventResponse(avrcp,
                (const AVRCP_INTERNAL_EVENT_COMMON_RES_T*) message);
            break;
    #endif /* !AVRCP_CT_ONLY_LIB*/

       case AVRCP_INTERNAL_MESSAGE_MORE_DATA:
       case MESSAGE_MORE_DATA: /* Fall Through */
            avrcpHandleReceivedData(avrcp);        
            break;
   
        case CL_L2CAP_DISCONNECT_IND:
            avrcpHandleL2capDisconnectInd(avrcp, 
                          (const CL_L2CAP_DISCONNECT_IND_T *) message);
              break;

        default:
            return FALSE;  
    }

    return TRUE;
}


/****************************************************************************
 *NAME
 *    avrcpHandleDisconnectingMessages
 *
 *DESCRIPTION
 *  Profile state machine to handle Messages in the Disconnecting state.
 *
 *PARAMETERS
 *  avrcp       - AVRCP Instance
 *  MessageId   - Message ID.
 *  message     - Message.
 ****************************************************************************/
static bool avrcpHandleDisconnectingMessages(AVRCP *avrcp,
                                             MessageId id,
                                             Message   message)
{
    switch(id)
    {
       case CL_L2CAP_DISCONNECT_CFM:
            avrcpHandleL2capDisconnectCfm(avrcp,
                    (const CL_L2CAP_DISCONNECT_CFM_T *) message);
            break;

        case CL_L2CAP_DISCONNECT_IND:
            /* Send Disconnect Response down the stack only */
            ConnectionL2capDisconnectResponse(
                ((const CL_L2CAP_DISCONNECT_IND_T *) message)->identifier,
                ((const CL_L2CAP_DISCONNECT_IND_T *) message)->sink);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


/****************************************************************************
 *NAME    
 *    avrcpHandleUnhandledMessages    
 *
 *DESCRIPTION
 *  Profile state machine to handle Messages Unhandled messages.
 *
 *PARAMETERS
 *  avrcp       - AVRCP Instance
 *  MessageId   - Message ID.
 *  message     - Message.
 ****************************************************************************/
static void avrcpHandleUnhandledMessages(AVRCP *avrcp, 
                                         MessageId id, 
                                         Message   message)
{
    switch(id)
    {
        case AVRCP_INTERNAL_TASK_DELETE_REQ:
            avrcpHandleDeleteTask(avrcp);
            break;

        case AVRCP_INTERNAL_CONNECT_REQ:
            avrcpSendConnectCfmToApp( avrcp,avrcp_bad_state, 
                  &((const AVRCP_INTERNAL_CONNECT_REQ_T *)message)->bd_addr,
                  0 );
            break;

        case AVRCP_INTERNAL_DISCONNECT_REQ:
            /* Ignore message, if we receive a disconect request from the
               application in this state the connection has already been
               disconnected.  Sending a disconnect confirm back to application
               in this case could be dangerous as the application will have
               already received a disconnect indication */
            break;

    #ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
        case AVRCP_INTERNAL_PASSTHROUGH_REQ:
            avrcpSendCommonCfmMessageToApp(AVRCP_PASSTHROUGH_CFM, 
                                    avrcp_device_not_connected, 0, avrcp);
            break;

        case AVRCP_INTERNAL_UNITINFO_REQ:
            avrcpSendUnitInfoCfmToClient(avrcp, 
                                 avrcp_device_not_connected, 0, 0, (uint32) 0);
            break;

        case AVRCP_INTERNAL_SUBUNITINFO_REQ:
           avrcpSendSubunitInfoCfmToClient(avrcp,
                                          avrcp_device_not_connected, 0, 0); 
           
            break;      

        case AVRCP_INTERNAL_VENDORDEPENDENT_REQ:
           avrcpSendCommonCfmMessageToApp(AVRCP_VENDORDEPENDENT_CFM, 
                                 avrcp_device_not_connected, 0, avrcp);
            break;


        case AVRCP_INTERNAL_WATCHDOG_TIMEOUT:
            avrcpHandleInternalWatchdogTimeout(avrcp);
            break;

    #endif /* !AVRCP_TG_ONLY_LIB */

       case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
             avrcpHandleServiceSearchAttributeCfm(avrcp, 
                        (const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *) message);
             break;

        case CL_L2CAP_CONNECT_IND:
            avrcpHandleL2capConnectInd((const CL_L2CAP_CONNECT_IND_T *) message);
            break;

       case CL_L2CAP_CONNECT_CFM:               /* Fall Through */
       case CL_L2CAP_TIMEOUT_IND:               /* Fall Through */
       case AVRCP_INTERNAL_MESSAGE_MORE_DATA:   /* Fall Through */
       case MESSAGE_MORE_DATA:                  /* Fall Through */
       case MESSAGE_MORE_SPACE:                 /* Fall Through */
       case MESSAGE_SOURCE_EMPTY:               /* Fall Through */
       case MESSAGE_STREAM_DISCONNECT:          /* Fall Through */
                /* ignore */
                break;

        default:
            if(id > AVRCP_MSG_BASE  && id < AVRCP_MSG_BOTTOM)
            {
                /* Ignore these messages. 
                   It may happen due to cross over of messages */
                AVRCP_INFO(("Ignoring the Message(0x%x) in wrong state (%d)\n",
                         id, avrcp->bitfields.state));
            }
            else
            {
                AVRCP_DEBUG((" AVRCP handleUnexpected- State 0x%x MsgId 0x%x\n",
                      avrcp->bitfields.state, id ));

            }
            break;
    }

    return;
}

/****************************************************************************
* NAME
*   avrcpCheckForUnreleasedRegionSource
*
* DESCRIPTION
*   Checks that the library doesn't have a stale request pending. If it finds
*   there is a pending RegionSource left over from a previous connection,
*   empty it, so the Stream manager can dispose the Source.
*****************************************************************************/
void avrcpCheckForUnreleasedRegionSource(AVRCP *avrcp)
{
    if(avrcp->data_app_ind != NULL)
    {
        AVRCP_INFO(("avrcpCheckForUnreleasedRegionSource: emptying Source: %04x\n",
                 avrcp->data_app_ind));
        SourceEmpty(avrcp->data_app_ind);
        avrcp->data_app_ind = NULL;
    }
}


/**************************************************************************
 *NAME    
 *    avrcpInitHandler
 *
 *DESCRIPTION
 *  Profile Task handler function to handle initialization messages
 *
 *PARAMETERS
 *  Task        - Task.
 *  MessageId   - Message ID.
 *  message     - Message.
 **************************************************************************/
void avrcpInitHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    switch(id)
    {
       case AVRCP_INTERNAL_INIT_REQ:
            avrcpHandleInternalInitReq((const AVRCP_INTERNAL_INIT_REQ_T*) message);
            break;

       case CL_L2CAP_REGISTER_CFM:
            avrcpHandleL2capRegisterCfm((const CL_L2CAP_REGISTER_CFM_T *) message); 
            break;

       case CL_SDP_REGISTER_CFM:
            avrcpHandleSdpRegisterCfm((const CL_SDP_REGISTER_CFM_T *) message);
            break;    

       case CL_L2CAP_CONNECT_IND:
            avrcpHandleL2capConnectInd((const CL_L2CAP_CONNECT_IND_T *) message);
            break;

        case CL_L2CAP_UNREGISTER_CFM:
        default:
            /* ignore */
            break;
    }

}


/****************************************************************************
 *NAME    
 *    avrcpProfileHandler    
 *
 *DESCRIPTION
 *  Profile Task handler function ti handle all messages to AVRCP Task.
 *
 *PARAMETERS
 *  Task        - Task.
 *  MessageId   - Message ID.
 *  message     - Message.
 ****************************************************************************/
void avrcpProfileHandler(Task task, MessageId id, Message message)
{
    AVRCP *avrcp = (AVRCP *) task;
    avrcpState profileState = avrcp->bitfields.state;
    bool consumed=TRUE;

    if (profileState == avrcpReady)
    {
        /* Consume any stale request which may have been pending if a disconnect occurred. */
        avrcpCheckForUnreleasedRegionSource(avrcp);

        consumed = avrcpHandleReadyMessages(avrcp, id, message);
    }
    else if (profileState == avrcpConnecting)
    {
        consumed = avrcpHandleConnectingMessages(avrcp, id, message);
    }
    else if (profileState == avrcpDisconnecting)
    {
        avrcpCheckForUnreleasedRegionSource(avrcp);

        consumed = avrcpHandleDisconnectingMessages(avrcp, id, message);
    }
    else
    {
        consumed = avrcpHandleConnectedMessages(avrcp, id, message);
    }

    if (!consumed)
    {
        avrcpHandleUnhandledMessages(avrcp, id, message);
    }

}
/****************************************************************************
*NAME    
*    avrcpDataCleanUp
*
*DESCRIPTION
*    Source handler function to free the allocated data. 
******************************************************************************/
void avrcpDataCleanUp(Task task, MessageId id, Message message)
{
    AvrcpCleanUpTask *cleanTask = (AvrcpCleanUpTask *) task;
    UNUSED(message);

    switch (id)
    {
    case MESSAGE_SOURCE_EMPTY:
        {
            /* Free the previously stored data ptr. */
            if (cleanTask->sent_data)
            {
                free(cleanTask->sent_data);
                AVRCP_INFO(("avrcpDataCleanUp: Cleanup Stored Data"));
            }
            cleanTask->sent_data = 0;
        }
        break;

    default:
        AVRCP_DEBUG((" AVRCP handleUnexpected in "
                   " avrcpDataCleanUp- MsgId 0x%x\n",id));
        break;
    }
}

/****************************************************************************
*NAME    
*    avbpProfileHandler
*
*DESCRIPTION
*    profileHandler function for Browsing channel 
******************************************************************************/
void avbpProfileHandler(Task task, MessageId id, Message message)
{
    AVBP *avbp = (AVBP *) task;

    /* Handle irrespective of state. Trust the connection library will not 
       send these messages  at wrong time.  Check the state only for the
       messages triggered by the peer device. */
    switch(id)
    {
        case AVRCP_INTERNAL_CONNECT_REQ:
            avrcpHandleInternalBrowseConnectReq(avbp, 
                                   (const AVRCP_INTERNAL_CONNECT_REQ_T *) message);
            break;

        case AVRCP_INTERNAL_CONNECT_RES:
             avrcpHandleInternalBrowseConnectRes(avbp, 
                             (const AVRCP_INTERNAL_CONNECT_RES_T *) message);
             break;     

        case AVRCP_INTERNAL_DISCONNECT_REQ:
            avrcpHandleInternalBrowseDisconnectReq(avbp, 1);
            break;

        case CL_L2CAP_CONNECT_IND:
            avrcpHandleL2capConnectIndReject(task,
                                       (const CL_L2CAP_CONNECT_IND_T *) message);
            break;

        case CL_L2CAP_CONNECT_CFM:
            avrcpHandleAvbpConnectCfm(avbp, (const CL_L2CAP_CONNECT_CFM_T*)message);
            break;

       case CL_L2CAP_DISCONNECT_IND:
            avrcpHandleBrowseDisconnectInd(avbp, 
                          (const CL_L2CAP_DISCONNECT_IND_T *) message);
              break;

        case CL_L2CAP_DISCONNECT_CFM:
            avrcpHandleBrowseDisconnectCfm(avbp, 
                          (const CL_L2CAP_DISCONNECT_CFM_T *) message);
              break;

    #ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
       case AVRCP_INTERNAL_WATCHDOG_TIMEOUT:
            avbpHandleInternalWatchdogTimeout(avbp);
            break;

       case AVRCP_INTERNAL_BROWSE_SET_PLAYER_REQ:
            avrcpHandleInternalBrowseSetPlayerReq(avbp, 
                        (const AVRCP_INTERNAL_BROWSE_SET_PLAYER_REQ_T*)message);
            break;

       case AVRCP_INTERNAL_CHANGE_PATH_REQ:
            avrcpHandleInternalChangePathReq(avbp, 
                            (const AVRCP_INTERNAL_CHANGE_PATH_REQ_T*)message);
            break;

       case AVRCP_INTERNAL_GET_ITEM_ATTRIBUTES_REQ:
            avrcpHandleInternalGetItemAttributesReq(avbp,
                    (const AVRCP_INTERNAL_GET_ITEM_ATTRIBUTES_REQ_T*) message);
            break;

       case AVRCP_INTERNAL_GET_FOLDER_ITEMS_REQ:
            avrcpHandleInternalGetFolderItemsReq(avbp,
                   (const AVRCP_INTERNAL_GET_FOLDER_ITEMS_REQ_T*)message);
            break;

       case AVRCP_INTERNAL_GET_NUMBER_OF_ITEMS_REQ:
            avrcpHandleInternalGetNumberOfItemsReq(avbp,
                    (const AVRCP_INTERNAL_GET_NUMBER_OF_ITEMS_REQ_T*)message);
            break;

       case AVRCP_INTERNAL_SEARCH_REQ:
            avrcpHandleInternalSearchReq(avbp,          
                    (const AVRCP_INTERNAL_SEARCH_REQ_T*) message);
            break;

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
       case AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT:
            avbpHandleInternalSendResponseTimeout(avbp);
            break;

       case AVRCP_INTERNAL_BROWSE_SET_PLAYER_RES:
            avrcpHandleInternalBrowseSetPlayerRes(avbp,
                       (const AVRCP_INTERNAL_BROWSE_SET_PLAYER_RES_T*)message);
            break;

       case AVRCP_INTERNAL_CHANGE_PATH_RES:
            avrcpHandleInternalChangePathRes(avbp,
                        (const AVRCP_INTERNAL_CHANGE_PATH_RES_T*)message);
            break;

       case AVRCP_INTERNAL_GET_ITEM_ATTRIBUTES_RES:
            avrcpHandleInternalGetItemAttributesRes(avbp,
                   (const AVRCP_INTERNAL_GET_ITEM_ATTRIBUTES_RES_T*)message);
            break; 

       case AVRCP_INTERNAL_GET_FOLDER_ITEMS_RES:
            avrcpHandleInternalGetFolderItemsRes(avbp,
                   (const AVRCP_INTERNAL_GET_FOLDER_ITEMS_RES_T*) message);
            break;

       case AVRCP_INTERNAL_GET_NUMBER_OF_ITEMS_RES:
            avrcpHandleInternalGetNumberOfItemsRes(avbp,
                   (const AVRCP_INTERNAL_GET_NUMBER_OF_ITEMS_RES_T*) message);
            break;

       case AVRCP_INTERNAL_SEARCH_RES:
            avrcpHandleInternalSearchRes(avbp,
                    (const AVRCP_INTERNAL_SEARCH_RES_T*) message);
            break;

#endif /* !AVRCP_CT_ONLY_LIB*/

       case AVRCP_INTERNAL_MESSAGE_MORE_DATA:
       case MESSAGE_MORE_DATA: /* Fall Through */  
            if(isAvbpConnected(avbp)) 
            {
                avbpHandleReceivedData (avbp);
            }
            break;

       case MESSAGE_MORE_SPACE:
       case MESSAGE_SOURCE_EMPTY:
       case MESSAGE_STREAM_DISCONNECT:
       case CL_L2CAP_TIMEOUT_IND:
                /* ignore */
                break;

        default:
            if(id > AVRCP_MSG_BASE  && id < AVRCP_MSG_BOTTOM)
            {
                /* Ignore these messages. 
                   It may happen due to cross over of messages */
                AVRCP_INFO(("Ignoring the Message(0x%x) \n", id));
            }
            else
            {
                AVRCP_DEBUG(("Unhandled Message = 0x%x\n", id));

            }
            break;
    }
}


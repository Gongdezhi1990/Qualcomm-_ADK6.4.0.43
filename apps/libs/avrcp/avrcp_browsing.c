/****************************************************************************
Copyright (c) 2009 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_browsing.c

DESCRIPTION
This file defines the API Functions for sending all Browsing related 
requests and responses related to Browsing features like
Browsing, Search and NowPlaying.

Support of NowPlaying feature is mandatory if Browsing is supported. Support
of Search mandates the Browsing and NowPlaying feature as well.

Controller API functions are 
    - AvrcpBrowseSetPlayerRequest
    - AvrcpBrowseChangePathRequest
    - AvrcpBrowseGetItemAttributesRequest
    - AvrcpBrowseGetFolderItemsRequest
    - AvrcpBrowseSearchRequest

Target API functions are 
    - AvrcpBrowseSetPlayerResponse
    - AvrcpBrowseChangePathResponse
    - AvrcpBrowseGetItemAttributesResponse
    - AvrcpBrowseGetFolderItemsResponse
    - AvrcpBrowseSearchResponse

NOTES

*/
/****************************************************************************
    Header files
*/
#include <stdlib.h>
#include <source.h>
#include <panic.h>

#include "avrcp_browsing_handler.h"
#include "avrcp_init.h"

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/****************************************************************************
*NAME 
*   AvrcpBrowseSetPlayerRequest
*
*DESCRIPTION
*   This function is used to send SetBrowsedPlayer Command to the TG.
*
*PARAMETRS
*   avrcp              - Task
*   uint16             - player ID
*
**RETURN
*   AVRCP_BROWSE_SET_PLAYER_CFM
*****************************************************************************/
void AvrcpBrowseSetPlayerRequest(AVRCP*    avrcp,   
                          uint16    player_id)
{
    AVBP *avbp = (AVBP*)avrcp->avbp_task;

    /* Send the message if task is valid and it is connected */
    if(isAvbpCheckConnected(avbp))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_BROWSE_SET_PLAYER_REQ);
        message->player_id = player_id;
 
        /* Queue the Command if there is any Outstanding command in progress */
         MessageSendConditionally(&avbp->task,
                                  AVRCP_INTERNAL_BROWSE_SET_PLAYER_REQ,
                                  message,
                                  &avbp->blocking_cmd);

    }
    else
    {
        AVRCP_INFO(("Browsing channel not connected\n"));
        avrcpBrowseSetPlayerCfm(avbp, avrcp_browsing_channel_not_connected,
                                NULL, 0); 
    }
}

/****************************************************************************
*NAME 
*   AvrcpBrowseChangePathRequest
*
*DESCRIPTION
*   This function is used to send ChangePath Command to the TG.
*
*PARAMETRS
*   avrcp                  - Task
*   uint16                 - UID Counter
*   avrcp_browse_direction - Direction of Browsing
*   avrcp_browse_uid       - Folder UID
*
*RETURN
*   AVRCP_BROWSE_CHANGE_PATH_CFM
*****************************************************************************/
void AvrcpBrowseChangePathRequest( AVRCP*                  avrcp,  
                            uint16                  uid_counter,
                            avrcp_browse_direction  direction,
                            avrcp_browse_uid        folder_uid)
{
    AVBP *avbp = (AVBP*)avrcp->avbp_task;

    /* validate avbp and send the message if it is connected */
    if(isAvbpCheckConnected(avbp))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_CHANGE_PATH_REQ);
        message->uid_counter = uid_counter;
        message->direction  = direction;
        message->folder_uid = folder_uid;
 
        /* Queue the Command if there is any Outstanding command in progress */
         MessageSendConditionally(&avbp->task,
                                  AVRCP_INTERNAL_CHANGE_PATH_REQ,
                                  message,
                                  &avbp->blocking_cmd);

    }
    else
    {
        AVRCP_INFO(("Browsing channel not connected\n"));
        avrcpChangePathCfm(avbp, avrcp_browsing_channel_not_connected,
                                 NULL, 0); 
    }
}

/****************************************************************************
*NAME 
*   AvrcpBrowseGetItemAttributesRequest
*
*DESCRIPTION
*   This function is used to send GetItemAttributes Command to the TG.
*
*PARAMETRS
*   avrcp                  - Task
*   avrcp_browse_scope     - Scope in which the UID of the media element 
*                            item or folder item is valid.
*   avrcp_browse_uid       - The UID of the  media element item or folder item
*   uint16                 - UID Counter
*   uint8                  - Number of attributes requested. 0 for all and 
*                            0xFF for No attributes.
*   Source                 - Size of source should be number of attributes * 4.
*
*RETURN
*   AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM
*****************************************************************************/
void AvrcpBrowseGetItemAttributesRequest(  AVRCP*               avrcp,
                                    avrcp_browse_scope   scope,  
                                    avrcp_browse_uid     uid,   
                                    uint16               uid_counter, 
                                    uint8                num_attr, 
                                    Source               attr_list)
{
    AVBP *avbp = (AVBP*)avrcp->avbp_task;
    uint16 size_attr_list = num_attr * 4;

    if(num_attr == 0 || num_attr == 0xFF)
    {
        if(attr_list)
        {
            SourceEmpty(attr_list);
        }
        size_attr_list =  0;
        attr_list = 0;
    }
    else
    {
        /* Validate the length */
        if(SourceBoundary(attr_list) < size_attr_list)
        {
            AVRCP_INFO(("Invalid length of attributes \n"));
            avrcpGetItemAttributesCfm(avbp,avrcp_rejected_invalid_content,
                                     NULL, 0); 
            if(attr_list)
            {
                SourceEmpty(attr_list);
            }
            return;
        }
    }

    /* validate avbp and send the message if it is connected */
    if(isAvbpCheckConnected(avbp))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_ITEM_ATTRIBUTES_REQ);
        message->scope = scope;
        message->uid = uid;
        message->uid_counter = uid_counter;
        message->num_attr = num_attr;
        message->attr_list_size = size_attr_list;
        message->attr_list = attr_list;
        

        /* Queue the Command if there is any Outstanding command in progress */
        MessageSendConditionally(&avbp->task,
                                  AVRCP_INTERNAL_GET_ITEM_ATTRIBUTES_REQ,
                                  message,
                                  &avbp->blocking_cmd);

    }
    else
    {
        AVRCP_INFO(("Browsing channel not connected\n"));
        avrcpGetItemAttributesCfm(avbp, avrcp_browsing_channel_not_connected,
                                 NULL, 0); 
        if(attr_list)
        {
            SourceEmpty(attr_list);
        }
    }
}

/****************************************************************************
*NAME 
*   AvrcpBrowseGetFolderItemsRequest
*
*DESCRIPTION
*   This function is used to send GetFolderItems Command to the TG.
*
*PARAMETRS
*   avrcp                  - Task
*   avrcp_browse_scope     - Scope for media navigation.
*   start                  - Start index of the item in the folder.
*   end                    - End index of the item in the folder
*   uint8                  - Number of attributes requested. 0 for all and 
*                            0xFF for No attributes.
*   Source                 - Size of source should be number of attributes * 4.
*
*RETURN
*   AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM
*****************************************************************************/
void AvrcpBrowseGetFolderItemsRequest( AVRCP*              avrcp,   
                                avrcp_browse_scope  scope,   
                                uint32              start,   
                                uint32              end,      
                                uint8               num_attr, 
                                Source              attr_list)
{
    AVBP *avbp = (AVBP*)avrcp->avbp_task;
    uint16 size_attr_list = num_attr * 4;

    if(num_attr == 0 || num_attr == 0xFF)
    {
        if(attr_list)
        {
            SourceEmpty(attr_list);
        }
        size_attr_list =  0;
        attr_list = 0;
    }
    else
    {
        /* Validate the length */
        if(SourceBoundary(attr_list) < size_attr_list)
        {
            AVRCP_INFO(("Invalid length of attributes \n"));
            avrcpGetFolderItemsCfm(avbp,avrcp_rejected_invalid_content,
                                     NULL, 0); 
            if(attr_list)
            {
                SourceEmpty(attr_list);
            }
            return;
        }
    }

    /* validate the start and end index */
    if(start > end)
    {
        AVRCP_INFO(("End index > Start\n"));
        avrcpGetFolderItemsCfm(avbp, avrcp_rejected_out_of_bound, NULL, 0);
        if(attr_list)
        {
            SourceEmpty(attr_list);
        }
        return; 
    }

    if(isAvbpCheckConnected(avbp))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_FOLDER_ITEMS_REQ);
        message->scope = scope;
        message->start = start;
        message->end = end;
        message->num_attr = num_attr;
        message->attr_list_size = size_attr_list;
        message->attr_list = attr_list;
        

        /* Queue the Command if there is any Outstanding command in progress */
        MessageSendConditionally(&avbp->task,
                                  AVRCP_INTERNAL_GET_FOLDER_ITEMS_REQ,
                                  message,
                                  &avbp->blocking_cmd);

    }
    else
    {
        AVRCP_INFO(("Browsing channel not connected\n"));
        avrcpGetFolderItemsCfm(avbp, avrcp_browsing_channel_not_connected,
                                 NULL, 0); 
        if(attr_list)
        {
            SourceEmpty(attr_list);
        }
    }
}

/****************************************************************************
*NAME 
*   AvrcpBrowseGetNumberOfItemsRequest
*
*DESCRIPTION
*   This function is used to send GetTotalNumberOfItems Command to the TG.
*
*PARAMETRS
*   avrcp                  - Task
*   avrcp_browse_scope     - Scope for media navigation.
*
*RETURN
*   AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM
*****************************************************************************/
void AvrcpBrowseGetNumberOfItemsRequest( AVRCP*              avrcp,   
                                avrcp_browse_scope  scope)
{
    AVBP *avbp = (AVBP*)avrcp->avbp_task;

    /* Validate the scope */
    if(scope > avrcp_now_playing_scope)
    {
         AVRCP_INFO(("Invalid Scope\n"));
        avrcpGetNumberOfItemsCfm(avbp, avrcp_rejected_invalid_scope, NULL, 0);
        return;
    }

    if(isAvbpCheckConnected(avbp))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_NUMBER_OF_ITEMS_REQ);
        message->scope = scope;

        /* Queue the Command if there is any Outstanding command in progress */
        MessageSendConditionally(&avbp->task,
                                  AVRCP_INTERNAL_GET_NUMBER_OF_ITEMS_REQ,
                                  message,
                                  &avbp->blocking_cmd);

    }
    else
    {
        AVRCP_INFO(("Browsing channel not connected\n"));
        avrcpGetNumberOfItemsCfm(avbp, avrcp_browsing_channel_not_connected,
                                 NULL, 0); 
    }
}

/****************************************************************************
*NAME 
*   AvrcpBrowseSearchRequest
*
*DESCRIPTION
*   This function is used to send Search Command to the TG.
*
*PARAMETRS
*   avrcp                  - Task
*   char_set               - Reserved. Always avrcp_char_set_utf_8 
*   str_length             - String length
*   string                 - Search String.    
*
*RETURN
*   AVRCP_BROWSE_SEARCH_CFM
*****************************************************************************/
void AvrcpBrowseSearchRequest( AVRCP*              avrcp,
                        avrcp_char_set      char_set, 
                        uint16              str_length,
                        Source              string)
{
    AVBP *avbp = (AVBP*)avrcp->avbp_task;

    if((!string) || (!str_length))
    {
        avrcpSearchCfm(avbp, avrcp_rejected_invalid_param, NULL, 0); 
        return;
    }
    
    if(SourceBoundary(string) < str_length)
    {
        AVRCP_INFO(("Invalid Content \n"));
        avrcpSearchCfm(avbp,avrcp_rejected_invalid_content, NULL, 0); 
        SourceEmpty(string);
        return;
    }

    if(char_set != avrcp_char_set_utf_8)
    {
        avrcpSearchCfm(avbp, avrcp_rejected_invalid_param, NULL, 0); 
        SourceEmpty(string);
        return;
    }

    if(isAvbpCheckConnected(avbp))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_SEARCH_REQ);
        message->char_set = char_set;
        message->str_length = str_length;
        message->string = string;

       /* Queue the Command if there is any Outstanding command in progress */
       MessageSendConditionally(&avbp->task,
                                  AVRCP_INTERNAL_SEARCH_REQ,
                                  message,
                                  &avbp->blocking_cmd);

    }
    else
    {
        AVRCP_INFO(("Browsing channel not connected\n"));
        SourceEmpty(string);
        avrcpSearchCfm(avbp, avrcp_browsing_channel_not_connected,
                        NULL, 0); 
    }
}

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
/****************************************************************************
*NAME 
*   AvrcpBrowseSetPlayerResponse
*
*DESCRIPTION
*   This function is used to send response for SetBrowsedPlayer command to CT.
*   This will be called in response to a AVRCP_BROWSE_SET_PLAYER_IND message.
*
*PARAMETRS
*   avrcp              - Task
*   avrcp_response_type- response. avrcp_response_browsing_success on Success.
*   uint16             - uid counter 
*   uint32             - number of items
*   avrcp_char_type    - char_type
*   uint8              - folder_depth
*   uint16             - size_folder_path
*   Source             - folder_path
*****************************************************************************/
void AvrcpBrowseSetPlayerResponse(AVRCP*                avrcp, 
                                  avrcp_response_type   response,
                                  uint16                uid_counter,
                                  uint32                num_items, 
                                  avrcp_char_set        char_type, 
                                  uint8                 folder_depth, 
                                  uint16                size_folder_path,
                                  Source                folder_path)
{
    AVBP *avbp = (AVBP*)avrcp->avbp_task;

    /* validate avbp and send the message if it is connected */
    if(isAvbpCheckConnected(avbp) && 
       (avbp->blocking_cmd == AVBP_SET_BROWSED_PLAYER_PDU_ID))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_BROWSE_SET_PLAYER_RES);
        message->response = response;

        if(isAvrcpDatabaseEnabled(avrcpGetDeviceTask()))
        {
            message->uid_counter = uid_counter;
        }
        else
        {
            message->uid_counter = 0;
        }

        message->num_items = num_items;
        message->char_type = char_type;
        message->folder_depth = folder_depth;
        message->size_folder_path = size_folder_path;
        message->folder_path = folder_path;
 
        /* Queue the Response */
        MessageSend(&avbp->task,
                   AVRCP_INTERNAL_BROWSE_SET_PLAYER_RES,
                   message);
    }
    else
    {
        AVRCP_INFO(("Ignoring the response\n"));
        if(folder_path)
        {
            SourceEmpty(folder_path);
        }
    }


}

/****************************************************************************
*NAME 
*   AvrcpBrowseChangePathResponse
*
*DESCRIPTION
*   This function is used to send response for  Changepath command to CT.
*   This will be called in response to a AVRCP_BROWSE_CHANGE_PATH_IND message.
*
*PARAMETRS
*   avrcp              - Task
*   avrcp_response_type- response. avrcp_response_browsing_success on Success.
*   uint32             - number of items
*****************************************************************************/
void AvrcpBrowseChangePathResponse(AVRCP*               avrcp,
                                   avrcp_response_type  response,
                                   uint32               num_items)
{
    AVBP *avbp = (AVBP*)avrcp->avbp_task;

    /* validate avbp and send the message if it is connected */
    if((isAvbpCheckConnected(avbp)) && 
      (avbp->blocking_cmd == AVBP_CHANGE_PATH_PDU_ID))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_CHANGE_PATH_RES);
        message->response = response;
        message->num_items = num_items;

        /* Queue the Response */
        MessageSend(&avbp->task,
                    AVRCP_INTERNAL_CHANGE_PATH_RES,
                    message);
    }
    else
    {
        AVRCP_INFO(("Wrong state.Ignoring the response\n"));
    }
}

/****************************************************************************
*NAME 
*   AvrcpBrowseGetItemAttributesResponse
*
*DESCRIPTION
*   This function is used to send response for GetItemAttributes command to CT.
*   This will be called in response to a AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_IND
*    message.
*
*PARAMETRS
*   avrcp              - Task
*   avrcp_response_type- response. avrcp_response_browsing_success on Success.
*   uint8              - num_attributes
*   uint16             - Size of attributes list in bytes
*   Source             - List of attributes.
*****************************************************************************/

void AvrcpBrowseGetItemAttributesResponse(AVRCP*              avrcp,
                                          avrcp_response_type response,
                                          uint8               num_attributes,
                                          uint16              size_attr_list,
                                          Source              attr_value_list)
{
    AVBP *avbp = (AVBP*)avrcp->avbp_task;

    if((SourceBoundary(attr_value_list) < size_attr_list) ||
       (!num_attributes && size_attr_list) ||
       (size_attr_list > (avbp->avbp_mtu - AVBP_MAX_FIXED_PDU_SIZE)))
    {
        AVRCP_DEBUG(("Invalid parameters\n"));
        if(attr_value_list)
        {
            SourceEmpty(attr_value_list);
        }
        return;
    }

    /* validate avbp and send the message if it is connected */
    if((isAvbpCheckConnected(avbp)) &&
       (avbp->blocking_cmd == AVBP_GET_ITEM_ATTRIBUTES_PDU_ID))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_ITEM_ATTRIBUTES_RES);
        message->response = response;
        message->num_attributes = num_attributes;
        message->size_attr_list = size_attr_list;
        message->attr_value_list = attr_value_list;

        /* Queue the Response */
        MessageSend(&avbp->task,
                    AVRCP_INTERNAL_GET_ITEM_ATTRIBUTES_RES,
                    message);
    }
    else
    {
        AVRCP_INFO(("Wrong State.Ignoring the response\n"));
        if(attr_value_list)
        { 
            SourceEmpty(attr_value_list);
        }
    }
}


/****************************************************************************
*NAME 
*   AvrcpBrowseGetFolderItemsResponse
*
*DESCRIPTION
*   This function is used to send response for  GetFolderItems command to CT.
*   This will be called in response to a AVRCP_BROWSE_GET_FOLDER_ITEMS_IND
*   message.
*
*PARAMETRS
*   avrcp              - Task
*   avrcp_response_type- response. avrcp_response_browsing_success on Success.
*   uint16             - UID Counter. 0 for non database aware players
*   uint16             - number of items returned
*   uint16             - size of the item list in bytes
*   Source             - item list
*****************************************************************************/
void AvrcpBrowseGetFolderItemsResponse( AVRCP*              avrcp,         
                                        avrcp_response_type response,     
                                        uint16              uid_counter, 
                                        uint16              num_items,  
                                        uint16              item_list_size,
                                        Source              item_list)
{
    AVBP *avbp = (AVBP*)avrcp->avbp_task;

    if((SourceBoundary(item_list) < item_list_size) ||
       (!num_items && item_list_size) || 
       (item_list_size > (avbp->avbp_mtu - AVBP_MAX_FIXED_PDU_SIZE)))
    {
        AVRCP_DEBUG(("Wrong parameters\n"));
        if(item_list)
        {
            SourceEmpty(item_list);
        }
        return; 
    }
     
    if((isAvbpCheckConnected(avbp)) && 
      (avbp->blocking_cmd == AVBP_GET_FOLDER_ITEMS_PDU_ID))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_FOLDER_ITEMS_RES);
        message->response = response;
        message->num_items = num_items;
        message->item_list_size = item_list_size;
        message->item_list = item_list;
        if(isAvrcpDatabaseEnabled(avrcpGetDeviceTask()))
        {
           message->uid_counter = uid_counter;
        }
        else
        {
           message->uid_counter = 0;
        }

        /* Queue the Response */
        MessageSend(&avbp->task,
                    AVRCP_INTERNAL_GET_FOLDER_ITEMS_RES,
                    message);
    }
    else
    {
        AVRCP_INFO(("Wrong State. Ignoring the response\n"));
        if(item_list)
        {
            SourceEmpty(item_list);
        }
    }
}

/****************************************************************************
*NAME 
*   AvrcpBrowseGetNumberOfItemsResponse
*
*DESCRIPTION
*   This function is used to send response for  GetNumberOfItems command to CT.
*   This will be called in response to a AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_IND
*   message.
*
*PARAMETRS
*   avrcp              - Task
*   avrcp_response_type- response. avrcp_response_browsing_success on Success.
*   uint16             - UID Counter. 0 for non database aware players
*   uint32             - number of items returned
*****************************************************************************/
void AvrcpBrowseGetNumberOfItemsResponse( AVRCP*              avrcp,         
                                        avrcp_response_type response,     
                                        uint16              uid_counter, 
                                        uint32              num_items)
{
    AVBP *avbp = (AVBP*)avrcp->avbp_task;

    if((isAvbpCheckConnected(avbp)) && 
      (avbp->blocking_cmd == AVBP_GET_NUMBER_OF_ITEM_PDU_ID))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_NUMBER_OF_ITEMS_RES);
        message->response = response;
        message->num_items = num_items;

        if(isAvrcpDatabaseEnabled(avrcpGetDeviceTask()))
        {
           message->uid_counter = uid_counter;
        }
        else
        {
           message->uid_counter = 0;
        }

        /* Queue the Response */
        MessageSend(&avbp->task,
                    AVRCP_INTERNAL_GET_NUMBER_OF_ITEMS_RES,
                    message);
    }
    else
    {
        AVRCP_INFO(("Wrong State. Ignoring the response\n"));
    }
}


/****************************************************************************
*NAME 
*   AvrcpBrowseSearchResponse
*
*DESCRIPTION
*   This function is used to send response for  Search command to CT.
*   This will be called in response to a AVRCP_BROWSE_SEARCH_IND
*   message.
*
*PARAMETRS
*   avrcp              - Task
*   avrcp_response_type- response. avrcp_response_browsing_success on Success.
*   uint16             - UID Counter
*   uint32             - Number of matching items
*****************************************************************************/
void AvrcpBrowseSearchResponse(AVRCP*               avrcp,         
                               avrcp_response_type  response,     
                               uint16               uid_counter, 
                               uint32               num_items)
{
    AVBP *avbp = (AVBP*)avrcp->avbp_task;

    if(isAvbpCheckConnected(avbp))
    {
        if(avbp->blocking_cmd == AVBP_SEARCH_PDU_ID)
        {
            MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_SEARCH_RES);
            message->response = response;
            message->num_items = num_items;

            if(isAvrcpDatabaseEnabled(avrcpGetDeviceTask()))
            {
                message->uid_counter = uid_counter;
            }
            else
            {
               message->uid_counter = 0;
            }

             /* Queue the Response */
            MessageSend(&avbp->task,
                        AVRCP_INTERNAL_SEARCH_RES,
                        message);
        }
        else
        {
            AVRCP_INFO(("GetFolderItems Command is not pending\n"));
        }
    }
    else
    {
        AVRCP_INFO(("Browsing channel not connected. Ignoring the response\n"));
    }
}

#endif /* !AVRCP_CT_ONLY_LIB*/

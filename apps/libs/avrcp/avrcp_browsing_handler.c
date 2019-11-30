/****************************************************************************
Copyright (c) 2009 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_browsing_handler.c

DESCRIPTION
This file defines all internal functions for handling browsing related 
features. Function prototypes of the functions defined here are declared in 
multiple header files associated with its browsing related features. 

NOTES

*/
#include <panic.h>
#include <stream.h>
#include <source.h>
#include <sink.h>
#include <stdlib.h>

#include "avrcp_browsing_handler.h"
#include "avrcp_mediaplayer.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_connect_handler.h"
#include "avrcp_notification_handler.h"
#include "avrcp_init.h"

/* Static functions */
/****************************************************************************
 *NAME    
 *   browseGrabSink    
 *
 *DESCRIPTION
 *  Grab Sink for Browsing PDU
 *
 * PARAMETES
 * Sink        - Browsing sink
 * pdu_size    - Size to be claimed for the PDU
 * packet_size - Sink space required for the whole packet
 * RETURN
 * uint8* pointer to the claimed memory
 ***************************************************************************/
static uint8* browseGrabSink(Sink sink,
                             uint16 pdu_size, 
                             uint16 packet_size)
{
    if(SinkSlack(sink) > packet_size)
    {
        return avrcpGrabSink(sink, pdu_size);   
    }
    else
    {
        return NULL;
    }
}


#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */

/****************************************************************************
 *NAME    
 *   avrcpSearchCfm
 *
 *DESCRIPTION
 * This function returns a confirmation message for 
 *  AvrcpBrowseSearch() request.
 ***************************************************************************/
uint16  avrcpSearchCfm(AVBP*              avbp,
                       avrcp_status_code  status, 
                       const uint8*       ptr,
                       uint16             data_len)
{
    AVRCP* avrcp=(AVRCP*)avbp->avrcp_task;
    uint16 process_len = AVRCP_SEARCH_RES_PARAM_SIZE;

    MAKE_AVRCP_MESSAGE(AVRCP_BROWSE_SEARCH_CFM);
    message->status = status;
    message->avrcp = avrcp;

    /* Decode the PDU 
     * ----------------------------------
     * |    0  |  1-2       |   3 - 6   |
     * ----------------------------------
     * |status |UID Counter | num items |
     * ----------------------------------
     */

    if(data_len >= AVRCP_SEARCH_RES_PARAM_SIZE)
    {
       AVRCP_UINT8_TO_UINT16(ptr, message->uid_counter, 1);
       message->num_items = convertUint8ValuesToUint32(&ptr[3]);
    }
    else
    {
        process_len = data_len;
    }

    MessageSend(avrcp->clientTask, 
                AVRCP_BROWSE_SEARCH_CFM, 
                message);

    return process_len;
}

/****************************************************************************
 *NAME    
 *   avrcpGetFolderItemsCfm
 *
 *DESCRIPTION
 * This function returns a confirmation message for 
 *  AvrcpBrowseGetFolderItems() request.
 ***************************************************************************/
uint16  avrcpGetFolderItemsCfm(AVBP*              avbp,
                               avrcp_status_code  status, 
                               const uint8*       ptr,
                               uint16             data_len)
{
    AVRCP* avrcp=(AVRCP*)avbp->avrcp_task;
    uint16 process_len = AVRCP_GET_FOLDER_ITEMS_RES_PARAM_SIZE;

    MAKE_AVRCP_MESSAGE(AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM);
    message->status = status;
    message->avrcp = avrcp;

     /* Decode the PDU 
       * -------------------------------------------
       * |  0   |   1-2       |   3-4     |  5...n |
       * -------------------------------------------
       * |status| UID  counter| num items |item list|
       * -------------------------------------------
       */


    if(data_len >= AVRCP_GET_FOLDER_ITEMS_RES_PARAM_SIZE)
    {
       AVRCP_UINT8_TO_UINT16(ptr, message->uid_counter, 1);
       AVRCP_UINT8_TO_UINT16(ptr, message->num_items, 3);
       message->item_list_size = data_len - 
                                 AVRCP_GET_FOLDER_ITEMS_RES_PARAM_SIZE;
       message->item_list =StreamSourceFromSink(avbp->avbp_sink);
    }
    else
    {
        process_len = data_len;
    }

    MessageSend(avrcp->clientTask, 
                AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM, 
                message);

    return process_len;
}

/****************************************************************************
 *NAME    
 *   avrcpGetNumberOfItemsCfm
 *
 *DESCRIPTION
 * This function returns a confirmation message for 
 *  AvrcpBrowseGetNumberOfItemsRequest() request.
 ***************************************************************************/
uint16  avrcpGetNumberOfItemsCfm(AVBP*              avbp,
                               avrcp_status_code  status, 
                               const uint8*       ptr,
                               uint16             data_len)
{
    AVRCP* avrcp=(AVRCP*)avbp->avrcp_task;
    uint16 process_len = AVRCP_GET_NUMBER_OF_ITEMS_RES_PARAM_SIZE;

    MAKE_AVRCP_MESSAGE(AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_CFM);
    message->status = status;
    message->avrcp = avrcp;

     /* Decode the PDU 
       * -------------------------------------------
       * |  0   |   1-2       |   3   | 4- 5 | 6-9 |
       * -------------------------------------------
       * |status| UID  counter| num items |
       * -------------------------------------------
       */

    if(data_len == AVRCP_GET_NUMBER_OF_ITEMS_RES_PARAM_SIZE)
    {
       AVRCP_UINT8_TO_UINT16(ptr, message->uid_counter, 1);
       message->num_items = convertUint8ValuesToUint32(&ptr[3]);
    }
    else
    {
        process_len = data_len;
    }

    MessageSend(avrcp->clientTask, 
                AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_CFM, 
                message);

    return process_len;
}


/****************************************************************************
 *NAME    
 *   avrcpGetItemAttributesCfm
 *
 *DESCRIPTION
 * This function returns a confirmation message for 
 *  AvrcpBrowseGetItemAttributes() request.
 *
 * PARAMETERS
 *   AVBP*                   - Task
 *   avrcp_status_code       - Status code
 *   uint8*                  - Parameters
 *   uint16                  - Parameter length 
 * RETURN
 * uint16 - Processed length of Data

 ***************************************************************************/
uint16  avrcpGetItemAttributesCfm(AVBP*              avbp,
                                  avrcp_status_code  status, 
                                  const uint8*       ptr,
                                  uint16             data_len)
{
    AVRCP* avrcp=(AVRCP*)avbp->avrcp_task;
    uint16 process_len = AVRCP_GET_ITEM_ATTRIBUTES_RES_PARAM_SIZE;

    MAKE_AVRCP_MESSAGE(AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM);
    message->status = status;
    message->avrcp = avrcp;
    if(data_len >= AVRCP_GET_ITEM_ATTRIBUTES_RES_PARAM_SIZE)
    {
       message->num_attributes = ptr[1];
       message->size_attr_list = data_len - 
                                 AVRCP_GET_ITEM_ATTRIBUTES_RES_PARAM_SIZE;
       message->attr_value_list =StreamSourceFromSink(avbp->avbp_sink);
    }
    else
    {
        process_len = data_len;
    }

    MessageSend(avrcp->clientTask, 
                AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM, 
                message);

    return process_len;
}

/****************************************************************************
 *NAME    
 *   avrcpChangePathCfm
 *
 *DESCRIPTION
 * This function returns a confirmation message for AvrcpBrowseChangePath
 *      
 *PARAMETERS
 *   AVBP*                   - Task
 *   avrcp_status_code       - Status code
 *   uint8*                  - Parameters
 *   uint16                  - Parameter length 
 * RETURN
 * uint16 - Processed length of Data
 ***************************************************************************/
uint16  avrcpChangePathCfm(AVBP*              avbp,
                           avrcp_status_code  status, 
                           const uint8*       ptr,
                           uint16             data_len)
                             
{
    AVRCP* avrcp=(AVRCP*)avbp->avrcp_task;
    uint16 process_len = AVRCP_CHANGE_PATH_RES_PARAM_SIZE;

    MAKE_AVRCP_MESSAGE(AVRCP_BROWSE_CHANGE_PATH_CFM);
    message->status = status;
    message->avrcp = avrcp;
    if(data_len >= AVRCP_CHANGE_PATH_RES_PARAM_SIZE)
    {
        /* Next 4 bytes are number of items */
        message->num_items = convertUint8ValuesToUint32(&ptr[1]);
    }
    else
    {
        process_len = data_len;
    }
    MessageSend(avrcp->clientTask, AVRCP_BROWSE_CHANGE_PATH_CFM, message);
    return process_len;
}

/****************************************************************************
 *NAME    
 *   avrcpBrowseSetPlayerCfm   
 *
 *DESCRIPTION
 * This function returns a confirmation message for AvrcpBrowseSetPlayer()
 *      
 *PARAMETERS
 *   AVBP*                   - Task
 *   avrcp_status_code       - Status code
 *   uint8*                  - Parameters
 *   uint16                  - Parameter length 
 * RETURN
 * uint16 - Processed length of Data
 ***************************************************************************/
uint16  avrcpBrowseSetPlayerCfm(AVBP*              avbp,
                                avrcp_status_code  status, 
                                const uint8*       ptr,
                                uint16             data_len)
                             
{
    AVRCP* avrcp=(AVRCP*)avbp->avrcp_task;
    uint16 process_len = AVRCP_SET_BROWSED_PLAYER_RES_PARAM_SIZE;

    MAKE_AVRCP_MESSAGE(AVRCP_BROWSE_SET_PLAYER_CFM);
    message->status = status;
    message->avrcp = avrcp;
    if(data_len >= AVRCP_SET_BROWSED_PLAYER_RES_PARAM_SIZE)
    {
        /* Copy 2 byte uid_counter from offset 1 */
        AVRCP_UINT8_TO_UINT16(ptr, message->uid_counter, 1);

        /* Next 4 bytes are number of items */
        message->num_items = convertUint8ValuesToUint32(&ptr[3]);

        /* next 2 bytes character set followed by folder depth*/
        AVRCP_UINT8_TO_UINT16(ptr, message->char_type, 7);
        message->folder_depth = ptr[9];

        message->size_path = data_len - process_len;

        message->folder_path = StreamSourceFromSink(avbp->avbp_sink);
    }
    else
    {
        process_len = data_len;
    }
    MessageSend(avrcp->clientTask, AVRCP_BROWSE_SET_PLAYER_CFM, message);
    return process_len;
}


/****************************************************************************
 *NAME    
 *   avrcpHandleInternalSearchReq
 *
 *DESCRIPTION
 *  This function frame the request packet for Search Command 
 *
 **************************************************************************/
void avrcpHandleInternalSearchReq(AVBP              *avbp,
                    const AVRCP_INTERNAL_SEARCH_REQ_T     *req)
{
    uint16 pdu_size = AVRCP_BROWSE_PDU_HDR_SIZE + 
                      AVRCP_SEARCH_REQ_PARAM_SIZE;

    uint8 *ptr=browseGrabSink(avbp->avbp_sink, pdu_size, 
                              pdu_size + req->str_length);

    if(ptr)
    {
        uint8 offset = AVRCP_BROWSE_PDU_HDR_SIZE;

        /* Frame the PDU 
        * --------------------------------------------
        * |  0-1    |   2-3           | 4......n      |
        * ---------------------------------------------
        * |char set |  string length  |  Search String|
        * ---------------------------------------------
        */
        AVRCP_UINT16_TO_UINT8(req->char_set, ptr, offset);
        AVRCP_UINT16_TO_UINT8(req->str_length, ptr, (offset+2));
        avrcpSendBrowsePdu(avbp, AVBP_SEARCH_PDU_ID, ptr,
                           AVRCP_SEARCH_REQ_PARAM_SIZE,
                           req->string, req->str_length, 0); 
    }
    else
    {
        avrcpSearchCfm(avbp,  avrcp_no_resource, 0 , 0);
    }
}


/****************************************************************************
 *NAME    
 *   avrcpHandleInternalGetFolderItemsReq
 *
 *DESCRIPTION
 *  This function frame the request packet for GetFolderItems Command 
 *
 **************************************************************************/
void avrcpHandleInternalGetFolderItemsReq(AVBP       *avbp,
           const AVRCP_INTERNAL_GET_FOLDER_ITEMS_REQ_T     *req)
{
    uint16 pdu_size = AVRCP_BROWSE_PDU_HDR_SIZE + 
                      AVRCP_GET_FOLDER_ITEMS_REQ_PARAM_SIZE;

    uint8 *ptr=browseGrabSink(avbp->avbp_sink, pdu_size, 
                              pdu_size + req->attr_list_size);

    if(ptr)
    {
        uint8 offset = AVRCP_BROWSE_PDU_HDR_SIZE;

        /* Frame the PDU 
        * -------------------------------------------------------------
        * |  0   |   1-4   | 5-8  |     9          | 10......n     |
        * -------------------------------------------------------------
        * |scope |  Start  |  end | Attribute Count| Attribute list|
        * -------------------------------------------------------------
        */
        ptr[offset] = req->scope;
        convertUint32ToUint8Values(&ptr[offset+1], req->start);
        convertUint32ToUint8Values(&ptr[offset+5], req->end);
        ptr[offset+9] = req->num_attr;

        avrcpSendBrowsePdu(avbp, AVBP_GET_FOLDER_ITEMS_PDU_ID, ptr,
                           AVRCP_GET_FOLDER_ITEMS_REQ_PARAM_SIZE,
                           req->attr_list, req->attr_list_size, 0); 
    }
    else
    {
        avrcpGetFolderItemsCfm(avbp, avrcp_no_resource, 0 , 0);
    }
}

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalGetNumberOfItemsReq
 *
 *DESCRIPTION
 *  This function frame the request packet for GetTotalNumberOfItems Command 
 *
 **************************************************************************/
void avrcpHandleInternalGetNumberOfItemsReq(AVBP       *avbp,
           const AVRCP_INTERNAL_GET_NUMBER_OF_ITEMS_REQ_T     *req)
{
    uint16 pdu_size = AVRCP_BROWSE_PDU_HDR_SIZE + 
                      AVRCP_GET_NUMBER_OF_ITEMS_REQ_PARAM_SIZE;

    uint8 *ptr=browseGrabSink(avbp->avbp_sink, pdu_size, pdu_size);

    if(ptr)
    {
        uint8 offset = AVRCP_BROWSE_PDU_HDR_SIZE;

        /* Frame the PDU 
        * -------------------------------------------------------------
        * |  0   |   1-2   | 3 |
        * -------------------------------------------------------------
        * |scope |
        * -------------------------------------------------------------
        */
        ptr[offset] = req->scope;

        avrcpSendBrowsePdu(avbp, AVBP_GET_NUMBER_OF_ITEM_PDU_ID, ptr,
                           AVRCP_GET_NUMBER_OF_ITEMS_REQ_PARAM_SIZE, (Source)NULL, 0, 0);
    }
    else
    {
        avrcpGetNumberOfItemsCfm(avbp, avrcp_no_resource, 0 , 0);
    }
}

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalGetItemAttributesReq    
 *
 *DESCRIPTION
 *  This function frame the request packet for getItemAttributes Command 
 *
 **************************************************************************/
void avrcpHandleInternalGetItemAttributesReq(AVBP       *avbp,
           const AVRCP_INTERNAL_GET_ITEM_ATTRIBUTES_REQ_T     *req)
{

    uint16 pdu_size = AVRCP_BROWSE_PDU_HDR_SIZE + 
                      AVRCP_GET_ITEM_ATTRIBUTES_REQ_PARAM_SIZE;

    uint8 *ptr=browseGrabSink(avbp->avbp_sink, pdu_size, 
                              pdu_size + req->attr_list_size);

    if(ptr)
    {
        uint8 offset = AVRCP_BROWSE_PDU_HDR_SIZE;

      /* Frame the PDU 
       * -------------------------------------------------------------
       * |  0   |   1-8   |   9-10     |     11       | 12......n     |
       * -------------------------------------------------------------
       * |scope |    UID  | UID counter|Num attributes| Attribute list|
       * -------------------------------------------------------------
       */
        ptr[offset] = req->scope;
        convertUint32ToUint8Values(&ptr[offset+1], req->uid.msb);
        convertUint32ToUint8Values(&ptr[offset+5], req->uid.lsb);
        AVRCP_UINT16_TO_UINT8(req->uid_counter, ptr, (offset+9));
        ptr[offset+11] = req->num_attr;

        
        avrcpSendBrowsePdu(avbp, AVBP_GET_ITEM_ATTRIBUTES_PDU_ID, ptr,
                           AVRCP_GET_ITEM_ATTRIBUTES_REQ_PARAM_SIZE,
                           req->attr_list, req->attr_list_size, 0);  
    }
    else
    {
        avrcpGetItemAttributesCfm(avbp, avrcp_no_resource, 0 , 0);
    }
}

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalChangePathReq    
 *
 *DESCRIPTION
 *  This function frame the request packet for ChangePath Command 
 *
 ***************************************************************************/
void avrcpHandleInternalChangePathReq(AVBP      *avbp,
                const AVRCP_INTERNAL_CHANGE_PATH_REQ_T  *req)
{
    uint16 pdu_size = AVRCP_BROWSE_PDU_HDR_SIZE + 
                      AVRCP_CHANGE_PATH_REQ_PARAM_SIZE;

    uint8 *ptr=browseGrabSink(avbp->avbp_sink, pdu_size, pdu_size);

    if(ptr)
    {
        uint8 offset = AVRCP_BROWSE_PDU_HDR_SIZE;

        /* Frame the PDU 
         * ---------------------------------------
         * |    0 - 1   |      2    |   3 - 10   |
         * ---------------------------------------
         * |UID Counter | Direction | Folder UID |
         * ---------------------------------------
         */
        AVRCP_UINT16_TO_UINT8(req->uid_counter, ptr, offset);
        ptr[offset+2] = req->direction;
        convertUint32ToUint8Values(&ptr[offset+3], req->folder_uid.msb);
        convertUint32ToUint8Values(&ptr[offset+7], req->folder_uid.lsb);

        avrcpSendBrowsePdu(avbp, AVBP_CHANGE_PATH_PDU_ID, ptr,
                           AVRCP_CHANGE_PATH_REQ_PARAM_SIZE,(Source)NULL,0,0);

    }
    else
    {
        avrcpChangePathCfm(avbp, avrcp_no_resource, 0 , 0);
    }
}

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalBrowseSetPlayerReq    
 *
 *DESCRIPTION
 *  This function frame the request packet for SetBrowsedPlayer Command 
 *
 ***************************************************************************/
void avrcpHandleInternalBrowseSetPlayerReq(AVBP       *avbp,
               const AVRCP_INTERNAL_BROWSE_SET_PLAYER_REQ_T *req)
{
    uint16 pdu_size = AVRCP_BROWSE_PDU_HDR_SIZE + 
                      AVRCP_SET_BROWSED_PLAYER_REQ_PARAM_SIZE;

    uint8 *ptr=browseGrabSink(avbp->avbp_sink, pdu_size, pdu_size);

    if(ptr)
    {
        AVRCP_UINT16_TO_UINT8(req->player_id, ptr, AVRCP_BROWSE_PDU_HDR_SIZE);
        avrcpSendBrowsePdu(avbp, AVBP_SET_BROWSED_PLAYER_PDU_ID, ptr,
                          AVRCP_SET_BROWSED_PLAYER_REQ_PARAM_SIZE, (Source)NULL,0,0);
        
    }
    else
    {
        avrcpBrowseSetPlayerCfm(avbp, avrcp_no_resource, 0 , 0);
    }
}

/****************************************************************************
 *NAME    
 *   avbpHandleInternalWatchdogTimeout    
 *
 *DESCRIPTION
 *  This function handles if the Command got timed Out.
 *
 ***************************************************************************/
void avbpHandleInternalWatchdogTimeout(AVBP *avbp)
{

    if(isAvbpConnected(avbp))
    {

        switch(avbp->blocking_cmd)
        {
        case AVBP_SET_BROWSED_PLAYER_PDU_ID:
            avrcpBrowseSetPlayerCfm(avbp, avrcp_browsing_timedout, NULL, 0);
            break;
    
        case AVBP_GET_FOLDER_ITEMS_PDU_ID:
            avrcpGetFolderItemsCfm(avbp,avrcp_browsing_timedout, NULL, 0);
            break;           

        case AVBP_GET_NUMBER_OF_ITEM_PDU_ID:
            avrcpGetNumberOfItemsCfm(avbp,avrcp_browsing_timedout, NULL, 0);
            break;

        case AVBP_CHANGE_PATH_PDU_ID:
            avrcpChangePathCfm(avbp, avrcp_browsing_timedout, NULL, 0);
            break;

        case AVBP_GET_ITEM_ATTRIBUTES_PDU_ID:
            avrcpGetItemAttributesCfm(avbp,avrcp_browsing_timedout, NULL, 0);
            break;

        case AVBP_SEARCH_PDU_ID:
            avrcpSearchCfm(avbp,avrcp_browsing_timedout, NULL, 0);
            break;           

        default:
            break;
        }
    }

    avbp->blocking_cmd = AVRCP_INVALID_PDU_ID;
}

 /****************************************************************************
 *NAME    
 *   avbpHandleResponse    
 *
 *DESCRIPTION
 *  This function handles the incoming Browsing PDU
 *
 *PARAMETERS
 * AVBP          - Task
 * uint16        - PDU ID
 * uint8*        - pointer to response PDU parameters
 * uint16        - length of parameters
 *
 ***************************************************************************/
void avbpHandleResponse(AVBP     *avbp,
                       uint16   pdu_id,
                 const uint8*   data, 
                       uint16   param_len)
{
    avrcp_status_code status = avrcp_success;
    uint16  process_len=param_len; 

    /* First byte in the packet is error status */
    if(data[0] != AVRCP_STATUS_SUCCESS)
    {
        status = AVRCP_ERROR_STATUS_BASE | data[0];
    }

    if((pdu_id == AVBP_GENERAL_REJECT_PDU_ID) && 
       (data[0] == AVRCP_STATUS_SUCCESS))
    {
        /* Unexpected spec violation */
        status = avrcp_rejected;
    }

    switch(avbp->blocking_cmd)
    {
        case AVBP_SET_BROWSED_PLAYER_PDU_ID:
            process_len =avrcpBrowseSetPlayerCfm(avbp,status,data, param_len);
            break;
    
        case AVBP_GET_FOLDER_ITEMS_PDU_ID:
            process_len =avrcpGetFolderItemsCfm(avbp,status,data,param_len);
            break;

        case AVBP_GET_NUMBER_OF_ITEM_PDU_ID:
            process_len = avrcpGetNumberOfItemsCfm(avbp, status, data, param_len);
            break;

        case AVBP_CHANGE_PATH_PDU_ID:
            process_len =avrcpChangePathCfm(avbp,status,data, param_len);
            break;

        case AVBP_GET_ITEM_ATTRIBUTES_PDU_ID:
            process_len =avrcpGetItemAttributesCfm(avbp,status,data,param_len);
            break;

        case AVBP_SEARCH_PDU_ID:
            process_len =avrcpSearchCfm(avbp,status,data, param_len);
            break;

        default:
            break;
    }
    SourceDrop(StreamSourceFromSink(avbp->avbp_sink), process_len);
    param_len -= process_len;
    avbpSetChannelOnIncomingData(avbp,
                                 AVRCP_INVALID_PDU_ID,
                                 param_len);
}

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

/****************************************************************************
 *NAME    
 *   streamlineResponse   
 *
 *DESCRIPTION
 *  streamline the response
 *
 * PARAMETES
 * avrcp_response_type - Response value provided to the Response API.
 *
 * RETURN
 * streamlined avrcp_response_type 
 ***************************************************************************/

static avrcp_response_type streamlineResponse(avrcp_response_type response)
{
    if((response == avctp_response_accepted) ||
       (response == avctp_response_changed) ||
       (response == avctp_response_stable) ||
       ( response == avctp_response_interim))
    {
        response = avrcp_response_browsing_success;
    }
    return response;
}

/****************************************************************************
 *NAME    
 *   avbpGetErrorStatusCode   
 *
 *DESCRIPTION
 *  Extract the error response code from the local application response.
 *
 * PARAMETES
 * avrcp_response_type - Response value provided to the Response API.
 *
 * RETURN
 * uint8 Error status value for Response PDU
 ***************************************************************************/
static uint8 avbpGetErrorStatusCode(avrcp_response_type response)
{

    response = streamlineResponse(response);
    if(response < avrcp_response_rejected_invalid_pdu)
    {
        return AVRCP_STATUS_INTERNAL_ERROR;
    }
    else
    {
        return (response & AVRCP_RESPONSE_ERROR_STATUS_MASK);
    }
}

/****************************************************************************
 *NAME    
 *  browseSendErrorResponse
 *
 *DESCRIPTION
 *  Send a error response including General Reject
 *
 ***************************************************************************/
static void browseSendErrorResponse(AVBP                *avbp,
                                    uint16              pdu_id,
                                    avrcp_response_type response)
{
    uint16 pdu_size = AVRCP_BROWSE_PDU_HDR_SIZE + 
                      AVRCP_GENERAL_REJECT_PARAM_SIZE;

    uint8 *ptr=browseGrabSink(avbp->avbp_sink, pdu_size, pdu_size); 

    if(ptr)
    {
        uint8 offset = AVRCP_BROWSE_PDU_HDR_SIZE;
        ptr[offset] = avbpGetErrorStatusCode(response);
        avrcpSendBrowsePdu(avbp, pdu_id, ptr,
                           AVRCP_GENERAL_REJECT_PARAM_SIZE,
                           (Source)NULL,0,1);
    }
    else
    {
        AVRCP_INFO(("Internal Error\n"));
        avrcpHandleInternalBrowseDisconnectReq(avbp, 0);
    }
}

/****************************************************************************
 *NAME    
 *   avrcpHandleSearchReq
 *
 *DESCRIPTION
 *   Handle the incoming Search Request
 ***************************************************************************/
avrcp_response_type avrcpHandleSearchReq(AVBP        *avbp, 
                                   const uint8*      data,
                                         uint16      param_len)
{
    AVRCP* avrcp=(AVRCP*)avbp->avrcp_task;
    uint16 str_len;

    if(!isAvrcpSearchEnabled(avrcpGetDeviceTask()))
    {
       return avrcp_response_rejected_search_not_supported; 
    }

    if(param_len < AVRCP_SEARCH_REQ_PARAM_SIZE)
    {
        return  avrcp_response_rejected_invalid_param;
    }

    AVRCP_UINT8_TO_UINT16(data, str_len, 2);
    if((AVRCP_SEARCH_REQ_PARAM_SIZE + str_len) > param_len)
    {
        return avrcp_response_rejected_invalid_content;
    } 
    else
    {
        MAKE_AVRCP_MESSAGE(AVRCP_BROWSE_SEARCH_IND);
        AVRCP_UINT8_TO_UINT16(data, message->char_set, 0);
        message->avrcp = avrcp;
        message->str_length = str_len;
        message->string = StreamSourceFromSink(avbp->avbp_sink);

        MessageSend(avrcp->clientTask,
                    AVRCP_BROWSE_SEARCH_IND,
                    message);
    }

    return avrcp_response_browsing_success;
}


/****************************************************************************
 *NAME    
 *   avrcpHandleGetFolderItemsReq
 *
 *DESCRIPTION
 *   Handle the incoming Get Folder Items Request
 ***************************************************************************/
avrcp_response_type avrcpHandleGetFolderItemsReq(AVBP        *avbp, 
                                           const uint8*      data,
                                                 uint16      param_len)
{

    AVRCP* avrcp=(AVRCP*)avbp->avrcp_task;

    if(param_len >=  AVRCP_GET_FOLDER_ITEMS_REQ_PARAM_SIZE)
    {
       /* 10th byte contains number of attributes */
        if((data[9] > 0) && (data[9] < 0xFF))
        {
            if(param_len < ((data[9] * 4) + 
                            AVRCP_GET_FOLDER_ITEMS_REQ_PARAM_SIZE))
            {
                return avrcp_response_rejected_invalid_content;
            }
        }

        if(data[0] > avrcp_now_playing_scope)
        {
            return avrcp_response_rejected_invalid_scope;
        }

    }
    else
    {
        return  avrcp_response_rejected_invalid_param;
    }
   /* On Success validation */
    {   
    
       /* Decode the PDU 
        * -------------------------------------------------------------
        * |  0   |   1-4   | 5-8  |     9          | 10......n     |
        * -------------------------------------------------------------
        * |scope |  Start  |  end | Attribute Count| Attribute list|
        * -------------------------------------------------------------
        */
        MAKE_AVRCP_MESSAGE(AVRCP_BROWSE_GET_FOLDER_ITEMS_IND);
        message->avrcp = avrcp;
        message->scope = data[0];
        message->start = convertUint8ValuesToUint32(&data[1]);
        message->end = convertUint8ValuesToUint32(&data[5]);
        if(message->start > message->end)
        {
            free(message);
            return avrcp_response_rejected_out_of_bound;
        }
        message->num_attr = data[9];
        if(message->num_attr == 0 || message->num_attr == 0xFF)
        {
            message->attr_list = 0;
        }
        else
        {
            message->attr_list = StreamSourceFromSink(avbp->avbp_sink);
        }

        MessageSend(avrcp->clientTask,
                    AVRCP_BROWSE_GET_FOLDER_ITEMS_IND,
                    message);
    }

    return avrcp_response_browsing_success;
}

/****************************************************************************
 *NAME    
 *   avrcpHandleGetNumberOfItemsReq
 *
 *DESCRIPTION
 *   Handle the incoming Get Folder Items Request
 ***************************************************************************/
avrcp_response_type avrcpHandleGetNumberItemsReq(AVBP        *avbp, 
                                           const uint8*      data,
                                                 uint16      param_len)
{

    AVRCP* avrcp=(AVRCP*)avbp->avrcp_task;

    if(param_len >=  AVRCP_GET_NUMBER_OF_ITEMS_REQ_PARAM_SIZE)
    {
        if(data[0] > avrcp_now_playing_scope)
        {
            return avrcp_response_rejected_invalid_scope;
        }

    }
    else
    {
        return  avrcp_response_rejected_invalid_param;
    }
   /* On Success validation */
    {   
    
       /* Decode the PDU 
        * -------------------------------------------------------------
        * |  0   |
        * -------------------------------------------------------------
        * |scope |
        * -------------------------------------------------------------
        */
        MAKE_AVRCP_MESSAGE(AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_IND);
        message->avrcp = avrcp;
        message->scope = data[0];
        MessageSend(avrcp->clientTask,
                    AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_IND,
                    message);
    }

    return avrcp_response_browsing_success;
}

/****************************************************************************
 *NAME    
 *   avrcpHandleGetItemAttributesReq
 *
 *DESCRIPTION
 *   Handle the incoming Get Item attributes Request
 *
 * PARAMETES
 *  AVBP    - *avbp,
 *  uint8*  -  Parameters in the command
 *  uint16  - Parameter length
 * RETURN
 * avrcp_response_type - avrcp_response_browsing_success on success. 
 ***************************************************************************/
avrcp_response_type avrcpHandleGetItemAttributesReq(AVBP        *avbp, 
                                              const uint8*      data,
                                                    uint16      param_len)
{
    AVRCP* avrcp=(AVRCP*)avbp->avrcp_task;
    

    if(param_len >=  AVRCP_GET_ITEM_ATTRIBUTES_REQ_PARAM_SIZE)
    {
        /* 12th byte contains number of attributes */
        if((data[11] > 0) && (data[11] < 0xFF))
        {
            if(param_len < ((data[11] * 4) + 
                            AVRCP_GET_ITEM_ATTRIBUTES_REQ_PARAM_SIZE))
            {
                return avrcp_response_rejected_invalid_content;
            }
        }

        if(data[0] > avrcp_now_playing_scope)
        {
            return avrcp_response_rejected_invalid_scope;
        }

    }
    else
    {
        return  avrcp_response_rejected_invalid_param;
    }
   /* On Success validation */
    {    
         MAKE_AVRCP_MESSAGE(AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_IND);
         message->avrcp = avrcp;
      /* Decode the PDU 
       * -------------------------------------------------------------
       * |  0   |   1-8   |   9-10     |     11       | 12......n     |
       * -------------------------------------------------------------
       * |scope |    UID  | UID counter|Num attributes| Attribute list|
       * -------------------------------------------------------------
       */
        message->scope = data[0];
        message->uid.msb = convertUint8ValuesToUint32(&data[1]);
        message->uid.lsb = convertUint8ValuesToUint32(&data[5]);
        AVRCP_UINT8_TO_UINT16(data, message->uid_counter, 9);
        message->num_attr = data[11];
        if(message->num_attr == 0 || message->num_attr == 0xFF)
        {
            message->attr_list = 0;
        }
        else
        {
            message->attr_list = StreamSourceFromSink(avbp->avbp_sink);
        }

        MessageSend(avrcp->clientTask,
                    AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_IND,
                    message);
    }

    return avrcp_response_browsing_success;
}



/****************************************************************************
 *NAME    
 *   avrcpHandleChangePathReq   
 *
 *DESCRIPTION
 *   Handle the incoming change path request
 *
 * PARAMETES
 *  AVBP    - *avbp,
 *  uint8*  -  Parameters in the command
 *  uint16  - Parameter length
 * RETURN
 * avrcp_response_type - avrcp_response_browsing_success on success. 
 ***************************************************************************/
avrcp_response_type avrcpHandleChangePathReq(AVBP         *avbp, 
                                             const uint8*  data,
                                             uint16        param_len)
{
    AVRCP* avrcp=(AVRCP*)avbp->avrcp_task;
    avrcp_response_type response = avrcp_response_browsing_success;

    if(param_len >=  AVRCP_CHANGE_PATH_REQ_PARAM_SIZE)
    {
        MAKE_AVRCP_MESSAGE(AVRCP_BROWSE_CHANGE_PATH_IND);
        message->avrcp = avrcp;  

        /* decode the PDU 
         * ---------------------------------------
         * |    0 - 1   |      2    |   3 - 10   |
         * ---------------------------------------
         * |UID Counter | Direction | Folder UID |
         * ---------------------------------------
         */
        AVRCP_UINT8_TO_UINT16(data, message->uid_counter, 0);
        message->direction = data[2];
        message->folder_uid.msb = convertUint8ValuesToUint32(&data[3]);
        message->folder_uid.lsb = convertUint8ValuesToUint32(&data[7]);
        
        MessageSend(avrcp->clientTask,AVRCP_BROWSE_CHANGE_PATH_IND,message);
    }
    else
    {
        response = avrcp_response_rejected_invalid_param;
    }

    return response;

}



/****************************************************************************
 *NAME    
 *   avrcpHandleSetBrowsedPlayerReq   
 *
 *DESCRIPTION
 *   Handle the incoming Browsed Set Player Request
 *
 * PARAMETES
 *  AVBP    - *avbp,
 *  uint8*  -  Parameters in the command
 *  uint16  - Parameter length
 * RETURN
 * avrcp_response_type - avrcp_response_browsing_success on success. 
 ***************************************************************************/

avrcp_response_type avrcpHandleSetBrowsedPlayerReq(AVBP         *avbp, 
                                                   const uint8*  data,
                                                   uint16        param_len)
{
    AVRCP* avrcp=(AVRCP*)avbp->avrcp_task;
    avrcp_response_type response = avrcp_response_browsing_success;

    if(param_len >=  AVRCP_SET_BROWSED_PLAYER_REQ_PARAM_SIZE)
    {
        MAKE_AVRCP_MESSAGE(AVRCP_BROWSE_SET_PLAYER_IND);
        message->avrcp = avrcp;  
        AVRCP_UINT8_TO_UINT16(data, message->player_id, 0);
        MessageSend(avrcp->clientTask,AVRCP_BROWSE_SET_PLAYER_IND,message);
    }
    else
    {
        response = avrcp_response_rejected_invalid_param;
    }

    return response;
}

/****************************************************************************
 *NAME    
 *   avrcpHandleNowPlayingCommand    
 *
 *DESCRIPTION
 * This function handles the incoming PlayItem or AddToNowPlaying Command 
 * from CT
 *
 *PARAMETERS
 * avrcp        -       AVRCP Entity
 * ptr          -       pointer to Meta Data command data
 * packet_size  -       size of the request packet
 * pdu_id       -       PDU ID
 *
 ***************************************************************************/
void avrcpHandleNowPlayingCommand(AVRCP*  avrcp,
                            const uint8*  ptr,
                                  uint16  packet_size,
                                  uint16  pdu_id)
{
    if(packet_size < AVRCP_PLAY_ITEM_SIZE)
    {
        /* Send error response */
        avrcpSendRejectMetadataResponse(avrcp,
                                       avrcp_response_rejected_invalid_content,
                                       AVRCP_PLAY_ITEM_PDU_ID);
    }
    else
    {
        
        AvrcpMessageId msg_id;
        MAKE_AVRCP_MESSAGE(AVRCP_PLAY_ITEM_IND);

        /* Send an Indication to the application */
        if(pdu_id == AVRCP_PLAY_ITEM_PDU_ID)
        {
            msg_id = AVRCP_PLAY_ITEM_IND;
        }
        else
        {
            /* This is Add to Now Playing Indication */
            msg_id = AVRCP_ADD_TO_NOW_PLAYING_IND;
        }

        message->avrcp = avrcp;
        message->scope = ptr[0];
        message->uid.msb = convertUint8ValuesToUint32(&ptr[1]);
        message->uid.lsb = convertUint8ValuesToUint32(&ptr[5]);
        AVRCP_UINT8_TO_UINT16(ptr, message->uid_counter, 9);
        MessageSend(avrcp->clientTask,msg_id, message);
    }
}

/****************************************************************************
 *NAME    
 *   avrcpHandleAddressedPlayerCommand   
 *
 *DESCRIPTION
 *   Internal function at TG to handle incoming SetAddressedPlayer
 *   request from CT.
 *      
 *PARAMETERS
 *   avrcp                   - Task
 *   uint8*                  - Parameters
 *   uint16                  - Parameter length 
 *
 *RETURN
 ****************************************************************************/
void avrcpHandleAddressedPlayerCommand(AVRCP*       avrcp,
                                       const uint8* data,
                                       uint16       len)
{
    if(len < AVRCP_PLAYER_ID_SIZE)
    {
        /* Send error response */
        avrcpSendRejectMetadataResponse(avrcp,
                                     avrcp_response_rejected_invalid_content,
                                     AVRCP_SET_ADDRESSED_PLAYER_PDU_ID);
    }
    else
    {
        /* Send an Indication to the application */
        MAKE_AVRCP_MESSAGE(AVRCP_SET_ADDRESSED_PLAYER_IND);
        AVRCP_UINT8_TO_UINT16(data,message->player_id, 0);
        message->avrcp = avrcp; 
        MessageSend(avrcp->clientTask,AVRCP_SET_ADDRESSED_PLAYER_IND,message);
    }
}

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalGetFolderItemsRes
 *
 *DESCRIPTION
 *  This function frame the response packet for GetFolderItems response 
 *
 ***************************************************************************/
void avrcpHandleInternalGetFolderItemsRes(AVBP       *avbp,
            const AVRCP_INTERNAL_GET_FOLDER_ITEMS_RES_T    *res)
{
    if(streamlineResponse(res->response) == avrcp_response_browsing_success)
    {
        uint16 pdu_size = AVRCP_BROWSE_PDU_HDR_SIZE + 
                          AVRCP_GET_FOLDER_ITEMS_RES_PARAM_SIZE;

        uint8 *ptr=browseGrabSink(avbp->avbp_sink, pdu_size, 
                                pdu_size + res->item_list_size);

        if(ptr)
        {
            uint8 offset = AVRCP_BROWSE_PDU_HDR_SIZE;

          /* Frame the PDU 
           * -------------------------------------------
           * |  0   |   1-2       |   3-4     |  5...n |
           * -------------------------------------------
           * |status| UID  counter| num items |item list|
           * -------------------------------------------
           */
           ptr[offset] = avbpGetErrorStatusCode(res->response);
           AVRCP_UINT16_TO_UINT8(res->uid_counter, ptr, (offset+1));
           AVRCP_UINT16_TO_UINT8(res->num_items, ptr, (offset+3));  
           avrcpSendBrowsePdu(avbp, AVBP_GET_FOLDER_ITEMS_PDU_ID, ptr,
                           AVRCP_GET_FOLDER_ITEMS_RES_PARAM_SIZE,
                           res->item_list,res->item_list_size,1);
         }
        else
        {
            AVRCP_INFO(("avrcpHandleinternalGetFolderItemsRes: Error\n"));
            avrcpHandleInternalBrowseDisconnectReq(avbp, 0);
        }
    }
    else
    {
        browseSendErrorResponse(avbp, 
                                AVBP_GET_FOLDER_ITEMS_PDU_ID, 
                                res->response);
    }
}

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalGetNumberOfItemsRes
 *
 *DESCRIPTION
 *  This function frame the response packet for GetNumberOfItems response 
 *
 ***************************************************************************/
void avrcpHandleInternalGetNumberOfItemsRes(AVBP       *avbp,
            const AVRCP_INTERNAL_GET_NUMBER_OF_ITEMS_RES_T    *res)
{
    if(streamlineResponse(res->response) == avrcp_response_browsing_success)
    {
        uint16 pdu_size = AVRCP_BROWSE_PDU_HDR_SIZE + 
                          AVRCP_GET_NUMBER_OF_ITEMS_RES_PARAM_SIZE;

        uint8 *ptr=browseGrabSink(avbp->avbp_sink, pdu_size, pdu_size);

        if(ptr)
        {
            uint8 offset = AVRCP_BROWSE_PDU_HDR_SIZE;

          /* Frame the PDU 
           * -------------------------------------------
           * |  0   |   1-2       |   3-6     |
           * -------------------------------------------
           * |status| UID  counter| num items |
           * -------------------------------------------
           */
           ptr[offset] = avbpGetErrorStatusCode(res->response);
           AVRCP_UINT16_TO_UINT8(res->uid_counter, ptr, (offset+1));
           convertUint32ToUint8Values(&ptr[offset+3], res->num_items);
           avrcpSendBrowsePdu(avbp, AVBP_GET_NUMBER_OF_ITEM_PDU_ID, ptr,
                           AVRCP_GET_NUMBER_OF_ITEMS_RES_PARAM_SIZE, (Source)NULL,0,1);
         }
        else
        {
            AVRCP_INFO(("avrcpHandleinternalGetFolderItemsRes: Error\n"));
            avrcpHandleInternalBrowseDisconnectReq(avbp, 0);
        }
    }
    else
    {
        browseSendErrorResponse(avbp, 
                                AVBP_GET_NUMBER_OF_ITEM_PDU_ID, 
                                res->response);
    }
}

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalGetItemAttributesRes
 *
 *DESCRIPTION
 *  This function frame the response packet for GetItemAttributes response 
 *
 ***************************************************************************/
void avrcpHandleInternalGetItemAttributesRes(AVBP       *avbp,
            const AVRCP_INTERNAL_GET_ITEM_ATTRIBUTES_RES_T    *res)
{
    if(streamlineResponse(res->response) == avrcp_response_browsing_success)
    {

        uint16 pdu_size = AVRCP_BROWSE_PDU_HDR_SIZE + 
                      AVRCP_GET_ITEM_ATTRIBUTES_RES_PARAM_SIZE;

        uint8 *ptr=browseGrabSink(avbp->avbp_sink, pdu_size, 
                                pdu_size + res->size_attr_list);

        if(ptr)
        {
            uint8 offset = AVRCP_BROWSE_PDU_HDR_SIZE;
        
            ptr[offset] = avbpGetErrorStatusCode(res->response);
            ptr[offset+1] = res->num_attributes;
            avrcpSendBrowsePdu(avbp, AVBP_GET_ITEM_ATTRIBUTES_PDU_ID, ptr,
                           AVRCP_GET_ITEM_ATTRIBUTES_RES_PARAM_SIZE,
                           res->attr_value_list,res->size_attr_list,1);
        }
        else
        {
            AVRCP_INFO(("avrcpHandleinternalGetItemAttributesRes: Error\n"));
            avrcpHandleInternalBrowseDisconnectReq(avbp, 0);
        }
    }
    else
    {
        browseSendErrorResponse(avbp, 
                                AVBP_GET_ITEM_ATTRIBUTES_PDU_ID, 
                                res->response);
    }
}


/****************************************************************************
 *NAME    
 *   avrcpHandleInternalSearchRes
 *
 *DESCRIPTION
 *  This function frame the response packet for Search response 
 *
 ***************************************************************************/
void avrcpHandleInternalSearchRes(AVBP              *avbp,
                      const AVRCP_INTERNAL_SEARCH_RES_T   *res)
{
    if(streamlineResponse(res->response) == avrcp_response_browsing_success)
    {

        uint16 pdu_size = AVRCP_BROWSE_PDU_HDR_SIZE + 
                      AVRCP_SEARCH_RES_PARAM_SIZE;

        uint8 *ptr=browseGrabSink(avbp->avbp_sink, pdu_size, pdu_size);

        if(ptr)
        {
            uint8 offset = AVRCP_BROWSE_PDU_HDR_SIZE;

           /* Frame the PDU 
            * ----------------------------------
            * |    0  |  1-2       |   3 - 6   |
            * ----------------------------------
            * |status |UID Counter | num items |
            * ----------------------------------
             */
  
            ptr[offset] = avbpGetErrorStatusCode(res->response);
            AVRCP_UINT16_TO_UINT8(res->uid_counter, ptr, (offset+1));
            convertUint32ToUint8Values(&ptr[offset+3], res->num_items);
            avrcpSendBrowsePdu(avbp, AVBP_SEARCH_PDU_ID, ptr,
                           AVRCP_SEARCH_RES_PARAM_SIZE,(Source)NULL,0,1);
        }
        else
        {
            AVRCP_INFO(("avrcpHandleinternalSearchRes: Error\n"));
            avrcpHandleInternalBrowseDisconnectReq(avbp, 0);
        }
    }
    else
    {
        browseSendErrorResponse(avbp, 
                                AVBP_SEARCH_PDU_ID, 
                                res->response);

    }

}

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalChangePathRes
 *
 *DESCRIPTION
 *  This function frame the response packet for ChangePath response 
 *
 ***************************************************************************/
void avrcpHandleInternalChangePathRes(AVBP       *avbp,
                const AVRCP_INTERNAL_CHANGE_PATH_RES_T  *res)
{
    if(streamlineResponse(res->response) == avrcp_response_browsing_success)
    {
        uint16 pdu_size = AVRCP_BROWSE_PDU_HDR_SIZE + 
                      AVRCP_CHANGE_PATH_RES_PARAM_SIZE;

        uint8 *ptr=browseGrabSink(avbp->avbp_sink, pdu_size, pdu_size);

        if(ptr)
        {
            uint8 offset = AVRCP_BROWSE_PDU_HDR_SIZE;
        
            ptr[offset] = avbpGetErrorStatusCode(res->response);
            convertUint32ToUint8Values(&ptr[offset+1],res->num_items);
            avrcpSendBrowsePdu(avbp, AVBP_CHANGE_PATH_PDU_ID, ptr,
                           AVRCP_CHANGE_PATH_RES_PARAM_SIZE,(Source)NULL,0,1);
        }
        else
        {
            AVRCP_INFO(("avrcpHandleInternalChangePathRes: Error\n"));
            avrcpHandleInternalBrowseDisconnectReq(avbp, 0);
        }
    }
    else
    {
        browseSendErrorResponse(avbp, 
                                AVBP_CHANGE_PATH_PDU_ID, 
                                res->response);
    }
}

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalBrowseSetPlayerRes    
 *
 *DESCRIPTION
 *  This function frame the response packet for SetBrowsedPlayer response. 
 *
 ***************************************************************************/
void avrcpHandleInternalBrowseSetPlayerRes(AVBP       *avbp,
               const AVRCP_INTERNAL_BROWSE_SET_PLAYER_RES_T *res)
{
    if(streamlineResponse(res->response) == avrcp_response_browsing_success)
    {

        uint16 pdu_size = AVRCP_BROWSE_PDU_HDR_SIZE + 
                      AVRCP_SET_BROWSED_PLAYER_RES_PARAM_SIZE;

        uint8 *ptr=browseGrabSink(avbp->avbp_sink, pdu_size, 
                              pdu_size + res->size_folder_path);
  
        if(ptr)
        {
            uint8 offset = AVRCP_BROWSE_PDU_HDR_SIZE;

           /* Frame the PDU 
            * --------------------------------------------------------
            * |  0   | 1-2   |   3-6   |   7-8   | 9   | 10 ...n      |
            * ---------------------------------------------------------
            * |status|counter|num items|char type|depth|length-name...|
            * --------------------------------------------------------
            */
            ptr[offset] = avbpGetErrorStatusCode(res->response);
            AVRCP_UINT16_TO_UINT8(res->uid_counter, ptr, (offset+1));
            convertUint32ToUint8Values(&ptr[offset+3], res->num_items);
            AVRCP_UINT16_TO_UINT8(res->char_type, ptr, (offset+7));
            ptr[offset+9] = res->folder_depth;

            avrcpSendBrowsePdu(avbp, AVBP_SET_BROWSED_PLAYER_PDU_ID, ptr,
                           AVRCP_SET_BROWSED_PLAYER_RES_PARAM_SIZE, 
                           res->folder_path, res->size_folder_path,1);

            /* Reject all Events associated with the Current Player and 
               clear that  Notifications. */
            avrcpRejectRegisterNotifications((AVRCP*)avbp->avrcp_task,
                            AVRCP_BROWSED_PLAYER_ASSOCIATED_EVENTS, 
                            avrcp_response_rejected_internal_error);

        }
        else
        {
            AVRCP_INFO(("avrcpHandleInternalBrowseSetPlayerRes: Error\n"));
            avrcpHandleInternalBrowseDisconnectReq(avbp, 0);
        }
    }
    else
    {
        browseSendErrorResponse(avbp, 
                                AVBP_SET_BROWSED_PLAYER_PDU_ID, 
                                res->response);

    }
}


/****************************************************************************
 *NAME    
 *   avbpHandleInternalSendResponseTimeout
 *
 *DESCRIPTION
 *  This function handles if the Response got timed Out.
 *
 ***************************************************************************/
void avbpHandleInternalSendResponseTimeout(AVBP *avbp)
{
    if(isAvbpConnected(avbp))
    {
        browseSendErrorResponse(avbp, 
                                avbp->blocking_cmd, 
                                avrcp_response_rejected_internal_error);
    }
    avbp->blocking_cmd = AVRCP_INVALID_PDU_ID;
}

/****************************************************************************
 *NAME    
 *   avbpHandleCommand
 *
 *DESCRIPTION
 *  This function handles the incoming Browsing Command Request PDU
 *
 *PARAMETERS
 * AVBP          - Task
 * uint16        - PDU ID
 * uint8*        - pointer to request PDU parameters
 * uint16        - length of parameters
 *
 ***************************************************************************/
void avbpHandleCommand(AVBP     *avbp,
                       uint16   pdu_id,
                 const uint8*   data,
                       uint16   param_len)
{
    avrcp_response_type  response =avrcp_response_rejected_invalid_pdu ;
    uint16 processed_len = param_len;
    
    switch(pdu_id)
    {
        case AVBP_SET_BROWSED_PLAYER_PDU_ID:
            response= avrcpHandleSetBrowsedPlayerReq(avbp, data, param_len);
            processed_len = AVRCP_SET_BROWSED_PLAYER_REQ_PARAM_SIZE;
            break;
    
        case AVBP_GET_FOLDER_ITEMS_PDU_ID:
            response = avrcpHandleGetFolderItemsReq(avbp, data, param_len);
            processed_len = AVRCP_GET_FOLDER_ITEMS_REQ_PARAM_SIZE;
            break;

        case AVBP_GET_NUMBER_OF_ITEM_PDU_ID:
            response = avrcpHandleGetNumberItemsReq(avbp, data, param_len);
            processed_len = AVRCP_GET_NUMBER_OF_ITEMS_REQ_PARAM_SIZE;
            break;

        case AVBP_CHANGE_PATH_PDU_ID:
            response = avrcpHandleChangePathReq(avbp, data, param_len);
            processed_len = AVRCP_CHANGE_PATH_REQ_PARAM_SIZE;
            break;

        case AVBP_GET_ITEM_ATTRIBUTES_PDU_ID:
            response = avrcpHandleGetItemAttributesReq(avbp, data, param_len);
            processed_len = AVRCP_GET_ITEM_ATTRIBUTES_REQ_PARAM_SIZE;
            break;

        case AVBP_SEARCH_PDU_ID:
            response = avrcpHandleSearchReq(avbp, data, param_len);
            processed_len = AVRCP_SEARCH_REQ_PARAM_SIZE;
            break;

        default:
            pdu_id = AVBP_GENERAL_REJECT_PDU_ID;
            break;
    }

    /* Drop the processed packet */
    SourceDrop(StreamSourceFromSink(avbp->avbp_sink), processed_len);

    if(response == avrcp_response_browsing_success)
    {
        avbpSetChannelOnIncomingData( avbp, pdu_id, param_len - processed_len);
    }
    else
    {
        browseSendErrorResponse(avbp, pdu_id, response); 
    }
}

#endif /* !AVRCP_CT_ONLY_LIB*/

/****************************************************************************
 *NAME    
 *   avbpSetChannelOutgoingData    
 *
 *DESCRIPTION
 *  This function blocks or unblock the Browsing channel depends on the 
 *  Outgoing browsing PDU
 *
 * PARAMETERS
 * AVBP*    - Browsing Task 
 * uint16   - PDU ID for Browsing Command
 * bool     - TRUE if it is response
 ***************************************************************************/

void avbpSetChannelOutgoingData(AVBP   *avbp,
                                uint16  pdu_id,
                                bool    response)
{
    Source source = StreamSourceFromSink(avbp->avbp_sink);

    /* Drop the Source used by the application */
    SourceDrop(source, avbp->avbp_sink_data);
    avbp->avbp_sink_data = 0;

    /* If it is response, Unblock the browsing task to receive more data. 
       Otherwise block till receiving the command */
    if(response)
    {

        /* No longer waiting for an application response */
        MessageCancelAll(&avbp->task, AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT);

        /* Unblock for receiving next command*/
        avbp->blocking_cmd = AVRCP_INVALID_PDU_ID;

    }
    else
    {
        avbp->blocking_cmd = pdu_id;

       /* Set a watchdog timeout */
        MessageSendLater( &avbp->task, 
                          AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 
                          0,AVBP_WATCHDOG_TIMEOUT );
    }

    if(SourceBoundary(source))
    {      
      /* There is data in the L2CAP sink to be processed.
       Message Internally to trigger*/
       MessageSend(&avbp->task, AVRCP_INTERNAL_MESSAGE_MORE_DATA, 0);
    }

}

/****************************************************************************
 *NAME    
 *   avbpSetChannelOnIncomingData    
 *
 *DESCRIPTION
 *  This function blocks or unblock the Browsing channel depends on the 
 *  incoming browsing PDU
 *
 * PARAMETERS
 * AVBP*    - Browsing Task 
 * uint16   - PDU ID for Browsing Command. CT always uses this value as
*             AVRCP_INVALID_PDU_ID to clear the pending command.   
 * uint16   - sink data which needs to be preserved for application usage.
 ***************************************************************************/
void avbpSetChannelOnIncomingData(AVBP      *avbp, 
                                  uint16    pdu_id,
                                  uint16    sink_data_len)
{
    avbp->avbp_sink_data = sink_data_len;

    if(pdu_id == AVRCP_INVALID_PDU_ID )
    {
        /* Received the Response. Reset the Pending Command */
        MessageCancelAll(&avbp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);

        /* Unblock for sending next command*/
        avbp->blocking_cmd = AVRCP_INVALID_PDU_ID;
    }
    else
    {
       /* Received a Command */
       avbp->blocking_cmd = pdu_id;
       MessageSendLater( &avbp->task, 
                         AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT, 
                          0,AVBP_SEND_RESPONSE_TIMEOUT );
    }
}

/****************************************************************************
 *NAME    
 *   avrcpSendBrowsePdu    
 *
 *DESCRIPTION
 *  This function frames the PDU for Browsing command and send it to the peer
 *
 * PARAMETERS
 * AVBP     - Browsing Task 
 * uint16   - PDU ID for Browsing Command
 * uint8*   - Pointer to the Start of the claimed sink
 * uint16   - Size of fixed parameters in the PDU
 * Source   - Flexible parameters
 * src_len  - Length of flexible parameters
 * bool     - TRUE if it is response
 ***************************************************************************/

void avrcpSendBrowsePdu(AVBP    *avbp,
                        uint16  pdu_id,
                        uint8*  packet,
                        uint16  param_size,
                        Source  src,
                        uint16  src_len,   
                        bool    response)
{

    /* Frame the AVCTP Header */
    avbpAvctpFrameHeader(avbp, packet, response);

    /* Frame the PDU */
    packet[AVCTP_SINGLE_PKT_HEADER_SIZE] = pdu_id;

    /* calculate the total parameter length */
    param_size += src_len; 

    AVRCP_UINT16_TO_UINT8(param_size, packet, AVRCP_BROWSE_PARAM_LEN_OFFSET);
                          

    if(src_len)
    {
        StreamMove(avbp->avbp_sink, src, src_len);
    }


    (void)SinkFlush(avbp->avbp_sink, param_size + AVRCP_BROWSE_PDU_HDR_SIZE);

    avbpSetChannelOutgoingData(avbp, pdu_id, response);
    if(src)
    {
        SourceEmpty(src);
    }

}

/****************************************************************************
 *NAME    
 *   avbpHandleReceivedData
 *
 *DESCRIPTION
 *  This function handles the incoming browsing channel PDU
 *
 *PARAMETERS
 * AVBP          - Task
 *
 ***************************************************************************/
void avbpHandleReceivedData(AVBP *avbp)
{
    Source source = StreamSourceFromSink(avbp->avbp_sink);
    uint16 packet_size;

    /* Handle the data if there is data in the Browsing Channel and
       application does not block the Sink */
    while((packet_size = SourceBoundary(source)) &&  (!avbp->avbp_sink_data))
    {
        const uint8 *ptr = SourceMap(source);

        if(avbpAvctpProcessHeader(avbp, ptr, packet_size) && 
           packet_size > AVRCP_BROWSE_PDU_HDR_SIZE )
        {
            uint16 param_len;
            uint8 pdu_id = ptr[AVRCP_BROWSE_PDU_OFFSET];
            uint8 cmd = ptr[AVCTP_HEADER_START_OFFSET] & AVCTP0_CR_MASK;

            /* validate the AVRCP Browse PDU length */
            AVRCP_UINT8_TO_UINT16(ptr, param_len,
                                  AVRCP_BROWSE_PARAM_LEN_OFFSET);

            /* Processed the header */
            SourceDrop(source, AVRCP_BROWSE_PDU_HDR_SIZE);
            packet_size -= AVRCP_BROWSE_PDU_HDR_SIZE;

            /* After discarding buffer data, reset the pointer to the start of
               any more buffer data. */
            ptr = SourceMap(source);

            if(packet_size <  param_len)
            {
                /* Integrity check failed. Set Param_len to packet len */
                param_len = packet_size;
            }
            
            if(cmd == AVCTP0_CR_COMMAND)
            {
            #ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
                avbpHandleCommand(avbp, pdu_id, ptr, param_len);
            #else
                SourceDrop(source, param_len);
            #endif 
            }
            else
            {
                if((pdu_id == avbp->blocking_cmd) ||
                   (pdu_id == AVBP_GENERAL_REJECT_PDU_ID))
                {
                #ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
                    avbpHandleResponse( avbp, pdu_id, ptr, param_len);
                #else
                    SourceDrop(source, param_len);
                #endif
                }
                else
                {
                    AVRCP_INFO(("Received PDU ID Mismatch\n")); 
                    SourceDrop(source, param_len);
                }
            }    
        }
        else
        {
            SourceDrop(source, packet_size);
        }
    }
}


/****************************************************************************
Copyright (c) 2009 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_browsing_handler.h
    
DESCRIPTION
    This header file declares the function prototypes for supporting the 
    AVRCP Browsing feature.     
*/

#ifndef AVRCP_BROWSING_HANDLER_H
#define AVRCP_BROWSING_HANDLER_H

#include "avrcp_common.h"

/* Preprocessor definitions */
#define AVRCP_BROWSE_PDU_HDR_SIZE               6
#define AVRCP_BROWSE_PDU_OFFSET                 3
#define AVRCP_BROWSE_PARAM_LEN_OFFSET           4
#define AVRCP_SET_BROWSED_PLAYER_REQ_PARAM_SIZE 2
#define AVRCP_SET_BROWSED_PLAYER_RES_PARAM_SIZE 10
#define AVRCP_CHANGE_PATH_REQ_PARAM_SIZE        11
#define AVRCP_CHANGE_PATH_RES_PARAM_SIZE        5
#define AVRCP_GET_ITEM_ATTRIBUTES_REQ_PARAM_SIZE 12
#define AVRCP_GET_ITEM_ATTRIBUTES_RES_PARAM_SIZE 2
#define AVRCP_SEARCH_REQ_PARAM_SIZE             4
#define AVRCP_GET_FOLDER_ITEMS_REQ_PARAM_SIZE   10
#define AVRCP_GET_NUMBER_OF_ITEMS_REQ_PARAM_SIZE   1
#define AVRCP_SEARCH_RES_PARAM_SIZE             7
#define AVRCP_GET_FOLDER_ITEMS_RES_PARAM_SIZE   5
#define AVRCP_GET_NUMBER_OF_ITEMS_RES_PARAM_SIZE   7
#define AVRCP_GENERAL_REJECT_PARAM_SIZE         1


#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */

/****************************************************************************
 *NAME    
 *   avrcpSearchfm
 *
 *DESCRIPTION
 * This function returns a confirmation message for 
 *  AvrcpBrowseSearch() request.
 ***************************************************************************/
uint16  avrcpSearchCfm(AVBP*              avbp,
                       avrcp_status_code  status, 
                       const uint8*       ptr,
                       uint16             data_len);

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
                               uint16             data_len);

/****************************************************************************
 *NAME    
 *   avrcpGetNumberOfItemsCfm
 *
 *DESCRIPTION
 * This function returns a confirmation message for 
 *  AvrcpBrowseGetNumberOfItems() request.
 ***************************************************************************/
uint16  avrcpGetNumberOfItemsCfm(AVBP*              avbp,
                               avrcp_status_code  status, 
                               const uint8*       ptr,
                               uint16             data_len);
                               
/****************************************************************************
 *NAME    
 *   avrcpGetItemAttributesCfm
 *
 *DESCRIPTION
 * This function returns a confirmation message for 
 *  AvrcpBrowseGetItemAttributes() request.
 ***************************************************************************/
uint16  avrcpGetItemAttributesCfm(AVBP*              avbp,
                                  avrcp_status_code  status, 
                                  const uint8*       ptr,
                                  uint16             data_len);

/****************************************************************************
 *NAME    
 *   avrcpChangePathCfm
 *
 *DESCRIPTION
 * This function returns a confirmation message for AvrcpBrowseChangePath
 ***************************************************************************/
uint16  avrcpChangePathCfm(AVBP*              avbp,
                           avrcp_status_code  status, 
                           const uint8*       ptr,
                           uint16             data_len);

/****************************************************************************
 *NAME    
 *   avrcpBrowseSetPlayerCfm   
 *
 *DESCRIPTION
 * This function returns a confirmation message for AvrcpBrowseSetPlayer()
 *
 ***************************************************************************/
uint16 avrcpBrowseSetPlayerCfm(AVBP*              avbp,
                               avrcp_status_code  status, 
                              const uint8*       ptr,
                              uint16             data_len);

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalSearchReq
 *
 *DESCRIPTION
 *  This function frame the request packet for Search Command 
 *
 **************************************************************************/
void avrcpHandleInternalSearchReq(AVBP              *avbp,
                    const AVRCP_INTERNAL_SEARCH_REQ_T     *req);

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalGetFolderItemsReq
 *
 *DESCRIPTION
 *  This function frame the request packet for GetFolderItems Command 
 *
 **************************************************************************/
void avrcpHandleInternalGetFolderItemsReq(AVBP       *avbp,
           const AVRCP_INTERNAL_GET_FOLDER_ITEMS_REQ_T     *req);

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalGetNumberOfItemsReq
 *
 *DESCRIPTION
 *  This function frame the request packet for GetTotalNumberOfItems Command 
 *
 **************************************************************************/
void avrcpHandleInternalGetNumberOfItemsReq(AVBP       *avbp,
           const AVRCP_INTERNAL_GET_NUMBER_OF_ITEMS_REQ_T     *req);


/****************************************************************************
 *NAME    
 *   avrcpHandleInternalGetItemAttributesReq    
 *
 *DESCRIPTION
 *  This function frame the request packet for ItemAttributes Command 
 *
 **************************************************************************/
void avrcpHandleInternalGetItemAttributesReq(AVBP       *avbp,
           const AVRCP_INTERNAL_GET_ITEM_ATTRIBUTES_REQ_T     *req);


/****************************************************************************
 *NAME    
 *   avrcpHandleInternalChangePathReq    
 *
 *DESCRIPTION
 *  This function frame the request packet for ChangePath  Command 
 *
 ***************************************************************************/
void avrcpHandleInternalChangePathReq(AVBP       *avbp,
               const AVRCP_INTERNAL_CHANGE_PATH_REQ_T *req);

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalBrowseSetPlayerReq    
 *
 *DESCRIPTION
 *  This function frame the request packet for SetBrowsedPlayer Command 
 *
 ***************************************************************************/
void avrcpHandleInternalBrowseSetPlayerReq(AVBP       *avbp,
               const AVRCP_INTERNAL_BROWSE_SET_PLAYER_REQ_T *req);

/****************************************************************************
 *NAME    
 *   avbpHandleInternalWatchdogTimeout    
 *
 *DESCRIPTION
 *  This function handles if the Command got timed Out.
 *
 ***************************************************************************/
void avbpHandleInternalWatchdogTimeout(AVBP *avbp);

/****************************************************************************
 *NAME    
 *   avbpHandleResponse    
 *
 *DESCRIPTION
 *  This function handles the incoming Browsing PDU
 *
 ***************************************************************************/
void avbpHandleResponse(    AVBP            *avbp,
                           uint16           pdu_id,
                           const uint8*     data, 
                           uint16           param_len);

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

/****************************************************************************
 *NAME    
 *   avrcpHandleSearchReq
 *
 *DESCRIPTION
 *   Handle the incoming Search Request
 ***************************************************************************/
avrcp_response_type avrcpHandleSearchReq(AVBP        *avbp, 
                                   const uint8*      data,
                                         uint16      param_len);

/****************************************************************************
 *NAME    
 *   avrcpHandleGetFolderItemsReq
 *
 *DESCRIPTION
 *   Handle the incoming Get Folder Items Request
 ***************************************************************************/
avrcp_response_type avrcpHandleGetFolderItemsReq(AVBP        *avbp, 
                                           const uint8*      data,
                                                 uint16      param_len);


/****************************************************************************
 *NAME    
 *   avrcpHandleGetNumberItemsReq
 *
 *DESCRIPTION
 *   Handle the incoming Get Number Items Request
 ***************************************************************************/
avrcp_response_type avrcpHandleGetNumberItemsReq(AVBP        *avbp, 
                                           const uint8*      data,
                                                 uint16      param_len);
                                                 
/****************************************************************************
 *NAME    
 *   avrcpHandleGetItemAttributesReq
 *
 *DESCRIPTION
 *   Handle the incoming Get Item attributes Request
 ***************************************************************************/
avrcp_response_type avrcpHandleGetItemAttributesReq(AVBP        *avbp, 
                                              const uint8*      data,
                                                    uint16      param_len);


/****************************************************************************
 *NAME    
 *   avrcpHandleChangePathReq   
 *
 *DESCRIPTION
 *   Handle the incoming change path request
 ***************************************************************************/
avrcp_response_type avrcpHandleChangePathReq(AVBP         *avbp, 
                                             const uint8*  data,
                                             uint16        param_len);

/****************************************************************************
 *NAME    
 *   avrcpHandleSetBrowsedPlayerReq   
 *
 *DESCRIPTION
 *   Handle the incoming Browsed Set Player Request
 ***************************************************************************/
avrcp_response_type avrcpHandleSetBrowsedPlayerReq(AVBP         *avbp, 
                                                   const uint8*  data,
                                                   uint16        param_len);


/****************************************************************************
 *NAME    
 *   avrcpHandleNowPlayingCommand    
 *
 *DESCRIPTION
 * This function handles the incoming PlayItem or AddToNowPlaying Command 
 * from CT
 *
 ***************************************************************************/
void avrcpHandleNowPlayingCommand(AVRCP*  avrcp,
                            const uint8*  ptr,
                                  uint16  packet_size,
                                  uint16  pdu_id);

/****************************************************************************
 *NAME    
 *   avrcpHandleAddressedPlayerCommand    
 *
 *DESCRIPTION
 * This function handles the incoming SetAddressedPlayer Command 
 * from CT
 *
 ***************************************************************************/
void avrcpHandleAddressedPlayerCommand(AVRCP*       avrcp,
                                       const uint8* data,
                                       uint16       len);

/****************************************************************************
 *NAME    
 *   avrcpHandleinternalGetFolderItemsRes
 *
 *DESCRIPTION
 *  This function frame the response packet for GetFolderItems response 
 *
 ***************************************************************************/
void avrcpHandleInternalGetFolderItemsRes(AVBP       *avbp,
            const AVRCP_INTERNAL_GET_FOLDER_ITEMS_RES_T    *res);

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalGetNumberOfItemsRes
 *
 *DESCRIPTION
 *  This function frame the response packet for GetNumberOfItems response 
 *
 ***************************************************************************/
void avrcpHandleInternalGetNumberOfItemsRes(AVBP       *avbp,
            const AVRCP_INTERNAL_GET_NUMBER_OF_ITEMS_RES_T    *res);

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalGetItemAttributesRes
 *
 *DESCRIPTION
 *  This function frame the response packet for GetItemAttributes response 
 *
 ***************************************************************************/
void avrcpHandleInternalGetItemAttributesRes(AVBP       *avbp,
            const AVRCP_INTERNAL_GET_ITEM_ATTRIBUTES_RES_T    *res);

/****************************************************************************
 *NAME    
 *   avrcpHandleinternalSearchRes
 *
 *DESCRIPTION
 *  This function frame the response packet for Search response 
 *
 ***************************************************************************/
void avrcpHandleInternalSearchRes(AVBP              *avbp,
                      const AVRCP_INTERNAL_SEARCH_RES_T   *res);

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalChangePathRes
 *
 *DESCRIPTION
 *  This function frame the response packet for ChangePath response 
 *
 ***************************************************************************/
void avrcpHandleInternalChangePathRes(AVBP      *avbp,
                const AVRCP_INTERNAL_CHANGE_PATH_RES_T  *res);

/****************************************************************************
 *NAME    
 *   avrcpHandleInternalBrowseSetPlayerRes    
 *
 *DESCRIPTION
 *  This function frame the response packet for SetBrowsedPlayer Command 
 *
 ***************************************************************************/
void avrcpHandleInternalBrowseSetPlayerRes(AVBP       *avbp,
               const AVRCP_INTERNAL_BROWSE_SET_PLAYER_RES_T *res);


/****************************************************************************
 *NAME    
 *   avbpHandleInternalSendResponseTimeout
 *
 *DESCRIPTION
 *  This function handles if the Response got timed Out.
 *
 ***************************************************************************/
void avbpHandleInternalSendResponseTimeout(AVBP *avbp);

 /****************************************************************************
 *NAME    
 *   avbpHandleCommand
 *
 *DESCRIPTION
 *  This function handles the incoming Browsing Command Request PDU
 *
 ***************************************************************************/
void avbpHandleCommand(AVBP     *avbp,
                       uint16   pdu_id,
                 const uint8*   data,
                       uint16   param_len);

#endif /* !AVRCP_CT_ONLY_LIB*/


/****************************************************************************
 *NAME    
 *   avrcpSendBrowsePdu    
 *
 *DESCRIPTION
 *  This function frames the PDU for Browsing command and send it to the peer
 *
 ***************************************************************************/
void avrcpSendBrowsePdu(AVBP    *avbp,
                        uint16  pdu_id,
                        uint8*  packet,
                        uint16  param_size,
                        Source  src,
                        uint16  src_len,   
                        bool    response);
/****************************************************************************
 *NAME    
 *   avbpFrameAvctpHeader   
 *
 *DESCRIPTION
 *   Frame the AVCTP Header for Browsing packet
 *
 ***************************************************************************/
void avbpFrameAvctpHeader(AVBP *avbp, uint8 *ptr , bool response);


/****************************************************************************
 *NAME    
 *   avbpSetChannelOutgoingData    
 *
 *DESCRIPTION
 *  This function blocks or unblock the Browsing channel depends on the 
 *  Outgoing browsing PDU
 *
 ***************************************************************************/

void avbpSetChannelOutgoingData(AVBP   *avbp,
                                uint16  pdu_id,
                                bool    response);

/****************************************************************************
 *NAME    
 *   avbpSetChannelOnIncomingData    
 *
 *DESCRIPTION
 *  This function blocks or unblock the Browsing channel depends on the 
 *  incoming browsing PDU
 ***************************************************************************/
void avbpSetChannelOnIncomingData(AVBP      *avbp, 
                                  uint16    pdu_id,
                                  uint16    sink_data_len);

 /****************************************************************************
 *NAME    
 *   avbpHandleReceivedData
 *
 *DESCRIPTION
 *  This function handles the incoming browsing channel PDU
 ***************************************************************************/
void avbpHandleReceivedData(AVBP *avbp);

#endif /* AVRCP_BROWSING_HANDLER_H */

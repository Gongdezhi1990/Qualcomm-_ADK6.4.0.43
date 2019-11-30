/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_connect_handler.h
    
DESCRIPTION
    Header file for AVRCP connection procedures. 
    
*/


#ifndef AVRCP_CONNECT_HANDLER_H_
#define AVRCP_CONNECT_HANDLER_H_

#include "avrcp_common.h"
#include "avrcp_init.h"


/****************************************************************************
NAME    
    avrcpSendConnectCfmToApp

DESCRIPTION
    This function sends Connect Confirmation message to the application 
*/
void avrcpSendConnectCfmToApp( AVRCP *avrcp,
                               avrcp_status_code status,
                               const bdaddr* addr,
                               Sink sink );
                               

/****************************************************************************
NAME    
    avrcpHandleInternalConnectReq

DESCRIPTION
    This function handles an internally generated connect request message.
*/
void avrcpHandleInternalConnectReq(AVRCP *avrcp,
                             const AVRCP_INTERNAL_CONNECT_REQ_T *req);


/****************************************************************************
NAME    
    avrcpHandleInternalConnectRes

DESCRIPTION
    This function handles an internally generated connect response message.
*/
void avrcpHandleInternalL2capConnectRes(AVRCP *avrcp, 
                            const AVRCP_INTERNAL_CONNECT_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleInternalDisconnectReq

DESCRIPTION
    This function handles an internally generated disconnect request message.
*/
void avrcpHandleInternalDisconnectReq(AVRCP *avrcp);

/***************************************************************************
NAME    
    avrcpSendBrowseConnectCfmToApp

DESCRIPTION
   Send AVRCP_BROWSE_CONNECT_CFM Message to the application
*/
void avrcpSendBrowseConnectCfmToApp(AVRCP               *avrcp,
                                    avrcp_status_code   status);


/****************************************************************************
NAME    
    avrcpHandleInternalBrowseConnectReq

DESCRIPTION
    This function handles an internally generated connect request message
    for Browsing Channel.
*/
void avrcpHandleInternalBrowseConnectReq(AVBP                   *avbp,
                             const AVRCP_INTERNAL_CONNECT_REQ_T *req);

/****************************************************************************
NAME    
    avrcpHandleInternalBrowseConnectRes

DESCRIPTION
    This function handles an internally generated connect response message for 
    browsing channel.
*/
void avrcpHandleInternalBrowseConnectRes(AVBP                        *avbp, 
                                  const AVRCP_INTERNAL_CONNECT_RES_T *res);

/****************************************************************************
NAME    
    avrcpHandleInternalBrowseDisconnectReq

DESCRIPTION
    This function handles an internally generated Browse disconnect 
    request message.
*/
void avrcpHandleInternalBrowseDisconnectReq(AVBP *avbp, bool app);


#endif /* AVRCP_CONNECT_HANDLER_H_ */

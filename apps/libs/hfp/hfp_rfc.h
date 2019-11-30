/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_rfc.h
    
DESCRIPTION
    
*/

#ifndef HFP_RFC_H_
#define HFP_RFC_H_


/****************************************************************************
NAME    
    hfpHandleRfcommConnectInd

DESCRIPTION
    Notification of an incoming rfcomm connection, pass this up to the app
    to decide whather to accept this or not.

RETURNS
    void
*/
void hfpHandleRfcommConnectInd(const CL_RFCOMM_CONNECT_IND_T *ind);


/****************************************************************************
NAME    
    hfpHandleRfcommServerConnectCfm

DESCRIPTION
    Outcome of the RFCOMM connect response.

RETURNS
    void
*/
void hfpHandleRfcommServerConnectCfm(const CL_RFCOMM_SERVER_CONNECT_CFM_T* cfm);


/****************************************************************************
NAME    
    hfpHandleRfcommClientConnectCfm

DESCRIPTION
    Outcome of the RFCOMM connect request.

RETURNS
    void
*/
void hfpHandleRfcommClientConnectCfm(const CL_RFCOMM_CLIENT_CONNECT_CFM_T* cfm);


/****************************************************************************
NAME    
    hfpHandleRfcommPortnegInd

DESCRIPTION
    Remote device has sent us a portneg request, respond

RETURNS
    void
*/
void hfpHandleRfcommPortnegInd(const CL_RFCOMM_PORTNEG_IND_T* ind);


/****************************************************************************
NAME    
    hfpHandleRfcommDisconnectInd

DESCRIPTION
    Indication that the RFCOMM connection has been disconnected.
*/
void hfpHandleRfcommDisconnectInd(const CL_RFCOMM_DISCONNECT_IND_T *ind);


/****************************************************************************
NAME    
    hfpHandleRfcommDisconnectCfm

DESCRIPTION
    Indication that call to ConnectionRfcommDisconnect has completed

RETURNS
    void
*/
void hfpHandleRfcommDisconnectCfm(const CL_RFCOMM_DISCONNECT_CFM_T* cfm);


/***************************************************************************
NAME    
    hfpHandleRfcommLinkLoss

DESCRIPTION
    Start link loss procedure
*/
void hfpHandleRfcommLinkLoss(hfp_link_data* link);


/***************************************************************************
NAME    
    hfpHandleRfcommLinkLossReconnect

DESCRIPTION
    Attempt to reconnect a dropped link
*/
void hfpHandleRfcommLinkLossReconnect(hfp_link_data* link);


/***************************************************************************
NAME
    hfpHandleRfcommLinkLossComplete

DESCRIPTION
    Attempt to reconnect a dropped link has completed
*/
void hfpHandleRfcommLinkLossComplete(hfp_link_data* link, hfp_connect_status status);


/***************************************************************************
NAME
    hfpHandleRfcommLinkLossAbort

DESCRIPTION
    Attempt to reconnect a dropped link has been aborted by the application
*/
void hfpHandleRfcommLinkLossAbort(hfp_link_data* link, bool force_disconnect);


/***************************************************************************
NAME
    hfpHandleRfcommLinkLossTimeout

DESCRIPTION
    Attempt to reconnect a dropped link has timed out
*/
void hfpHandleRfcommLinkLossTimeout(hfp_link_data* link);

#endif /* HFP_RFC_H_ */

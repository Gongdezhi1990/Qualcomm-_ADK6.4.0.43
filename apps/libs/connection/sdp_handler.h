/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sdp_handler.h
    
DESCRIPTION

*/

#ifndef    CONNECTION_SDP_HANDLER_H_
#define    CONNECTION_SDP_HANDLER_H_

#include <app/bluestack/sdc_prim.h>
#include <app/bluestack/sds_prim.h>

#ifndef CL_EXCLUDE_SDP

/****************************************************************************
NAME    
    connectionHandleSdpRegisterRequest

DESCRIPTION
    Sends a register request to the SDP server

RETURNS
    void    
*/
void connectionHandleSdpRegisterRequest(connectionSdpState *state, const CL_INTERNAL_SDP_REGISTER_RECORD_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleSdpRegisterCfm

DESCRIPTION
    Handle the register response received from BlueStack

RETURNS
    void    
*/
void connectionHandleSdpRegisterCfm(connectionSdpState *state, const SDS_REGISTER_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleSdpUnregisterRequest

DESCRIPTION
    Handle the request to unregister a particular service record

RETURNS
    void    
*/
void connectionHandleSdpUnregisterRequest(connectionSdpState *state, const CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleSdpUnregisterCfm

DESCRIPTION
    Handle the response from BlueStack to an SDP unregister request

RETURNS
    void    
*/
void connectionHandleSdpUnregisterCfm(connectionSdpState *state, const SDS_UNREGISTER_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleSdpServerConfigMtu

DESCRIPTION
    Send a request to BlueStack to configure the SDP server MTU size

RETURNS
    void    
*/
void connectionHandleSdpServerConfigMtu(const connectionSdpState *state, const CL_INTERNAL_SDP_CONFIG_SERVER_MTU_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleSdpClientConfigMtu

DESCRIPTION
    Send a request to BlueStack to configure the SDP client MTU size

RETURNS
    void    
*/
void connectionHandleSdpClientConfigMtu(const connectionSdpState *state, const CL_INTERNAL_SDP_CONFIG_CLIENT_MTU_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleSdpOpenSearchRequest

DESCRIPTION
    Send a request to BlueSTack to open an SDP search session

RETURNS
    void    
*/
void connectionHandleSdpOpenSearchRequest(connectionSdpState *state, const CL_INTERNAL_SDP_OPEN_SEARCH_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleSdpOpenSearchCfm

DESCRIPTION
    Handle the response to the open SDP session request

RETURNS
    void    
*/
void connectionHandleSdpOpenSearchCfm(connectionSdpState *state, const SDC_OPEN_SEARCH_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleSdpCloseSearchRequest

DESCRIPTION
    Issue a close search request to BlueStack

RETURNS
    void    
*/
void connectionHandleSdpCloseSearchRequest(const connectionSdpState *state, const CL_INTERNAL_SDP_CLOSE_SEARCH_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleSdpCloseSearchInd

DESCRIPTION
    Indication from BlueStack informing us that an SDP search session has 
    been closed.

RETURNS
    void    
*/
void connectionHandleSdpCloseSearchInd(connectionSdpState *state, const SDC_CLOSE_SEARCH_IND_T *ind);


/****************************************************************************
NAME    
    connectionHandleSdpServiceSearchRequest

DESCRIPTION
    Handle a request to initiate a service search to a remote device.

RETURNS
    void    
*/
void connectionHandleSdpServiceSearchRequest(connectionSdpState *state, const CL_INTERNAL_SDP_SERVICE_SEARCH_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleSdpServiceSearchCfm

DESCRIPTION
    SDP service search results received so handle them here.

RETURNS
    void    
*/
void connectionHandleSdpServiceSearchCfm(connectionSdpState *state, const SDC_SERVICE_SEARCH_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleSdpAttributeSearchRequest

DESCRIPTION
    Request to issue an SDP attribute search request to BlueStack.

RETURNS
    void    
*/
void connectionHandleSdpAttributeSearchRequest(connectionSdpState *state, const CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleSdpAttributeSearchCfm

DESCRIPTION
    A confirm to the attribute search has been received, handle it here.

RETURNS
    void    
*/
void connectionHandleSdpAttributeSearchCfm(connectionSdpState *state, const SDC_SERVICE_ATTRIBUTE_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleSdpServiceSearchAttrRequest

DESCRIPTION
    Handle a request to perfor a service/ attribute search. If allowed, send
    request to BlueStack otherwise queue it up until it can be sent.

RETURNS
    void    
*/
void connectionHandleSdpServiceSearchAttrRequest(connectionSdpState *state, const CL_INTERNAL_SDP_SERVICE_SEARCH_ATTRIBUTE_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleSdpServiceSearchAttributeCfm

DESCRIPTION
    HAndle the search cfm received from BlueStack.

RETURNS
    void    
*/
void connectionHandleSdpServiceSearchAttributeCfm(connectionSdpState *state, const SDC_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleSdpTerminatePrimitiveRequest

DESCRIPTION
    Send a request to BVlueStack to terminate the current SDP search.

RETURNS
    void    
*/
void connectionHandleSdpTerminatePrimitiveRequest(const connectionSdpState *state, const CL_INTERNAL_SDP_TERMINATE_PRIMITIVE_REQ_T *req);


#endif
#endif    /* CONNECTION_SDP_HANDLER_H_ */

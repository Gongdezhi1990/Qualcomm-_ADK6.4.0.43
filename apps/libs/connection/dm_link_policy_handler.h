/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_link_policy_handler.h
    
DESCRIPTION

*/

#ifndef    CONNECTION_DM_LINK_POLICY_HANDLER_H_
#define    CONNECTION_DM_LINK_POLICY_HANDLER_H_


/****************************************************************************
NAME    connectionHandleLinkPolicySetRoleReq

DESCRIPTION
    This function is called in response to a CL_INTERNAL_DM_SET_ROLE_REQ
    message

RETURNS
    
*/
void connectionHandleLinkPolicySetRoleReq(connectionLinkPolicyState* linkPolicyState, const CL_INTERNAL_DM_SET_ROLE_REQ_T* req);


/****************************************************************************
NAME    connectionHandleLinkPolicyGetRoleReq

DESCRIPTION
    This function is called in response to a CL_INTERNAL_DM_GET_ROLE_REQ
    message

RETURNS
    
*/
void connectionHandleLinkPolicyGetRoleReq(connectionLinkPolicyState* linkPolicyState, const CL_INTERNAL_DM_GET_ROLE_REQ_T* req);


/****************************************************************************
NAME    connectionHandleDmSwitchRoleComplete

DESCRIPTION
    This function is called in response to a HCI_SWITCH_ROLE_COMPLETE
    message

RETURNS
    
*/
void connectionHandleDmSwitchRoleComplete(connectionLinkPolicyState *linkPolicyState, const DM_HCI_SWITCH_ROLE_CFM_T* ind);


/****************************************************************************
NAME    
    connectionHandleRoleDiscoveryComplete

DESCRIPTION
    This function is called in response to a HCI_SWITCH_ROLE_DISCOVERY_COMPLETE
    message

RETURNS
    
*/
void connectionHandleRoleDiscoveryComplete(connectionLinkPolicyState* linkPolicyState, const DM_HCI_ROLE_DISCOVERY_CFM_T * ind);
                            

/****************************************************************************
NAME    
    connectionHandleSetLinkSupervisionTimeoutReq

DESCRIPTION
    This function is called in response to a CL_INTERNAL_DM_SET_LINK_SUPERVISION_TIMEOUT_REQ
    message

RETURNS

*/
void connectionHandleSetLinkSupervisionTimeoutReq(const CL_INTERNAL_DM_SET_LINK_SUPERVISION_TIMEOUT_REQ_T* req);


/****************************************************************************
NAME    
    connectionHandleLinkSupervisionTimeoutInd

DESCRIPTION
    This function is called in response to a DM_HCI_LINK_SUPERV_TIMEOUT_IND 
    message

RETURNS

*/
void connectionHandleLinkSupervisionTimeoutInd(const DM_HCI_LINK_SUPERV_TIMEOUT_IND_T* ind);


/****************************************************************************
NAME    
    connectionHandleSetLinkPolicyReq

DESCRIPTION
    This function is called in response to a CL_INTERNAL_DM_SET_LINK_POLICY_REQ
    message

RETURNS

*/
void connectionHandleSetLinkPolicyReq(const CL_INTERNAL_DM_SET_LINK_POLICY_REQ_T* req);


/****************************************************************************
NAME    
    connectionHandleWriteLinkPolicySettingsReq

DESCRIPTION
    This function is called in response to a
    CL_INTERNAL_DM_WRITE_LINK_POLICY_SETTINGS_REQ message

RETURNS

*/
void connectionHandleWriteLinkPolicySettingsReq(const CL_INTERNAL_DM_WRITE_LINK_POLICY_SETTINGS_REQ_T* req);


/****************************************************************************
NAME    
    connectionLinkPolicyHandleWritePowerStatesCfm

DESCRIPTION
    This function is called when we receive a DM_LP_WRITE_POWERSTATES_CFM
    message which in turn is sent by BLueStack whenever we send it a new
    power table.
*/
void connectionLinkPolicyHandleWritePowerStatesCfm(const DM_LP_WRITE_POWERSTATES_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleSetSniffSubRatePolicyReq

DESCRIPTION
    This function is called in response to a CL_INTERNAL_DM_SET_SNIFF_SUB_RATE_POLICY_REQ
    message
*/
void connectionHandleSetSniffSubRatePolicyReq(connectionReadInfoState* infoState, const CL_INTERNAL_DM_SET_SNIFF_SUB_RATE_POLICY_REQ_T* req);


/****************************************************************************
NAME    
    connectionHandleSniffSubRatingInd

DESCRIPTION
    This function is called when we receive a DM_HCI_SNIFF_SUB_RATING_IND
    from bluestack to indicate we have started sniff sub rating
*/
void connectionHandleSniffSubRatingInd(const DM_HCI_SNIFF_SUB_RATING_IND_T *ind);
        
        
#endif    /* CONNECTION_DM_LINK_POLICY_HANDLER_H_ */

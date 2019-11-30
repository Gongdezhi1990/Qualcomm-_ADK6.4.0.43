/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_info_handler.h
    
DESCRIPTION

*/

#ifndef    CONNECTION_DM_INFO_HANDLER_H_
#define    CONNECTION_DM_INFO_HANDLER_H_


/****************************************************************************
NAME    
    connectionHandleReadAddrRequest

DESCRIPTION
    Handle an internal request to read the local bluetooth address

RETURNS
    void
*/
void connectionHandleReadAddrRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_BD_ADDR_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleReadBdAddrComplete

DESCRIPTION
    Handle a read bd addr complete event

RETURNS
    void
*/
void connectionHandleReadBdAddrComplete(connectionReadInfoState *state, const DM_HCI_READ_BD_ADDR_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleReadLinkQualityRequest

DESCRIPTION
    Issue a request to read the link quality on a particular connection.

RETURNS
    void
*/
void connectionHandleReadLinkQualityRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_LINK_QUALITY_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleReadLinkQualityComplete

DESCRIPTION
    Confirm containing the link quality if the read request succeeded.

RETURNS
    void
*/
void connectionHandleReadLinkQualityComplete(connectionReadInfoState *state, const DM_HCI_GET_LINK_QUALITY_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleReadRssiRequest

DESCRIPTION
    Request to read the RSSI on a particular connection.

RETURNS
    void
*/
void connectionHandleReadRssiRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_RSSI_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleReadRssiComplete

DESCRIPTION
    Confirm containing the RSSI value if the read request succeeded.

RETURNS
    void
*/
void connectionHandleReadRssiComplete(connectionReadInfoState *state, const DM_HCI_READ_RSSI_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleReadclkOffsetRequest

DESCRIPTION
    Request to read the clock offset of a remote device.

RETURNS
    void
*/
void connectionHandleReadclkOffsetRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_CLK_OFFSET_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleReadClkOffsetComplete

DESCRIPTION
    Confirm containing the remote clock offset if the read request succeeded.

RETURNS
    void
*/
void connectionHandleReadClkOffsetComplete(connectionReadInfoState *state, const DM_HCI_READ_CLOCK_OFFSET_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleSetBtVersionReq

DESCRIPTION
    Handle setting BT Version

RETURNS
    void
*/
void connectionHandleSetBtVersionReq(connectionReadInfoState *state, const CL_INTERNAL_DM_SET_BT_VERSION_REQ_T *req);
        
        
/****************************************************************************
NAME    
    connectionHandleSetBtVersionCfm

DESCRIPTION
    Confirm containing the request status and current BT Version

RETURNS
    void
*/
void connectionHandleSetBtVersionCfm(connectionReadInfoState *state, const DM_SET_BT_VERSION_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleReadRemoteSupportedExtFeaturesRequest

DESCRIPTION
    Request to read the supported extended features of a remote device.

RETURNS
    void
*/
void connectionHandleReadRemoteSupportedExtFeaturesRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_REMOTE_SUPP_EXT_FEAT_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleReadRemoteSupportedExtFeaturesCfm

DESCRIPTION
    Confirm containing the remote extended supported features, if the read 
    request succeeded.

RETURNS
    void
*/
void connectionHandleReadRemoteSupportedExtFeaturesCfm(connectionReadInfoState *state, const DM_HCI_READ_REMOTE_EXT_FEATURES_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleReadRemoteSupportedFeaturesRequest

DESCRIPTION
    Request to read the supported features of a remote device.

RETURNS
    void
*/
void connectionHandleReadRemoteSupportedFeaturesRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_REMOTE_SUPP_FEAT_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleReadRemoteSupportedFeaturesCfm

DESCRIPTION
    Confirm containing the remote supported features, if the read request
    succeeded.

RETURNS
    void
*/
void connectionHandleReadRemoteSupportedFeaturesCfm(connectionReadInfoState *state, const DM_HCI_READ_REMOTE_SUPP_FEATURES_CFM_T *cfm);

/****************************************************************************
NAME    
    connectionHandleReadLocalVersionRequest

DESCRIPTION
    Request to read the version information of the local device.
  
RETURNS
    void
*/
void connectionHandleReadLocalVersionRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_LOCAL_VERSION_REQ_T *req);
        
        
/****************************************************************************
NAME    
    connectionHandleReadLocalVersionCfm

DESCRIPTION
    Confirm containing the local version information, if the read request
    succeeded.

RETURNS
    void
*/
void connectionHandleReadLocalVersionCfm(connectionReadInfoState *state, const DM_HCI_READ_LOCAL_VER_INFO_CFM_T *cfm);


/****************************************************************************
NAME
    connectionHandleReadRemoteVersionRequest

DESCRIPTION
    Request to read the version information of a remote device.

RETURNS
    void
*/
void connectionHandleReadRemoteVersionRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_REMOTE_VERSION_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleReadRemoteVersionCfm

DESCRIPTION
    Confirm containing the remote version information, if the read request
    succeeded.

RETURNS
    void
*/
void connectionHandleReadRemoteVersionCfm(connectionReadInfoState *state, const DM_HCI_READ_REMOTE_VER_INFO_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleDmHciModeChangeEvent

DESCRIPTION
    
RETURNS
    void
*/
void connectionHandleDmHciModeChangeEvent(Task task, const DM_HCI_MODE_CHANGE_EVENT_IND_T *ev);

#endif    /* CONNECTION_DM_INFO_HANDLER_H_ */

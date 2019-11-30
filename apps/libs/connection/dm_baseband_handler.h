/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_baseband_handler.h
    
DESCRIPTION

*/

#ifndef    CONNECTION_DM_BASEBAND_HANDLER_H_
#define    CONNECTION_DM_BASEBAND_HANDLER_H_


/****************************************************************************
NAME    
    connectionHandleReadClassOfDeviceRequest

DESCRIPTION
    Handles the internal message that initiates the read class of device

RETURNS
    void
*/
void connectionHandleReadClassOfDeviceRequest(connectionReadInfoState *state, const CL_INTERNAL_DM_READ_CLASS_OF_DEVICE_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleReadClassOfDeviceComplete

DESCRIPTION
    Handles the read class of device complete message from BlueStack

RETURNS
    void
*/
void connectionHandleReadClassOfDeviceComplete(connectionReadInfoState *state, const DM_HCI_READ_CLASS_OF_DEVICE_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleWritePageScanActivityRequest

DESCRIPTION
    Handles the internal message requesting a page scan activity write

RETURNS
    void
*/
void connectionHandleWritePageScanActivityRequest(const CL_INTERNAL_DM_WRITE_PAGESCAN_ACTIVITY_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleWriteInquiryScanActivityRequset

DESCRIPTION
    Handles the internal message requesting a inquiry scan activity write

RETURNS
    void
*/
void connectionHandleWriteInquiryScanActivityRequest(const CL_INTERNAL_DM_WRITE_INQSCAN_ACTIVITY_REQ_T *req);


/****************************************************************************
NAME	
NAME	
    connectionHandleWriteInquiryScanTypeRequset

DESCRIPTION
    Handles the internal message requesting a write inquiry scan type

RETURNS
    void
*/
void connectionHandleWriteInquiryScanTypeRequest(const CL_INTERNAL_DM_WRITE_INQUIRY_SCAN_TYPE_REQ_T *req);

/****************************************************************************
NAME	
    connectionHandleWritePageScanTypeRequset

DESCRIPTION
    Handles the internal message requesting a write page scan type

RETURNS
    void
*/
void connectionHandleWritePageScanTypeRequest(const CL_INTERNAL_DM_WRITE_PAGE_SCAN_TYPE_REQ_T *req);

/****************************************************************************
NAME	
    connectionHandleWriteScanEnableRequest

DESCRIPTION
    Handles the internal message requesting a write scan enable

RETURNS
    void
*/
void connectionHandleWriteScanEnableRequest(const CL_INTERNAL_DM_WRITE_SCAN_ENABLE_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleWriteCodRequest

DESCRIPTION
    Handles the internal message requesting a write class of device

RETURNS
    void
*/
void connectionHandleWriteCodRequest(const CL_INTERNAL_DM_WRITE_CLASS_OF_DEVICE_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleWriteCachedPageModeRequest

DESCRIPTION
    Handles the internal message requesting a to store the page mode for a given device

RETURNS
    void
*/
void connectionHandleWriteCachedPageModeRequest(const CL_INTERNAL_DM_WRITE_CACHED_PAGE_MODE_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleWriteCachedClkOffsetRequest

DESCRIPTION
    Handles the internal message requesting a to store the clock offset for a given device

RETURNS
    void
*/
void connectionHandleWriteCachedClkOffsetRequest(const CL_INTERNAL_DM_WRITE_CACHED_CLK_OFFSET_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleClearParamCacheRequest

DESCRIPTION
    Handles the internal message requesting to clear the parameter cache for 
    a given device

RETURNS
    void
*/
void connectionHandleClearParamCacheRequest(const CL_INTERNAL_DM_CLEAR_PARAM_CACHE_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleWriteFlushTimeoutRequest

DESCRIPTION
    Set the flush timeout for a particular ACL.

RETURNS
    void
*/
void connectionHandleWriteFlushTimeoutRequest(const CL_INTERNAL_DM_WRITE_FLUSH_TIMEOUT_REQ_T *req);


/****************************************************************************
NAME    
    connectionHandleChangeLocalName

DESCRIPTION
    Attempt to change the local name of the device.

RETURNS
    void
*/
void connectionHandleChangeLocalName(const CL_INTERNAL_DM_CHANGE_LOCAL_NAME_REQ_T *req);


#endif    /* CONNECTION_DM_BASEBAND_HANDLER_H_ */

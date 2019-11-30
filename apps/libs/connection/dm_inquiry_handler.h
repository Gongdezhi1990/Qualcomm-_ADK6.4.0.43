/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_inquiry_handler.h
    
DESCRIPTION

*/

#ifndef    CONNECTION_DM_INQUIRY_HANDLER_H_
#define    CONNECTION_DM_INQUIRY_HANDLER_H_



/****************************************************************************
NAME    
    connectionHandleInquiryStart

DESCRIPTION
    This function actually kicks off an inquiry message to BlueStack.

RETURNS
    void
*/
void connectionHandleInquiryStart(connectionInquiryState *state, const CL_INTERNAL_DM_INQUIRY_REQ_T *inquiry_req);


/****************************************************************************
NAME    
    connectionHandleInquiryResult

DESCRIPTION
    This function handles inquiry results as they arrive from BlueStack.

RETURNS
    void
*/
void connectionHandleInquiryResult(const connectionInquiryState *state, const DM_HCI_INQUIRY_RESULT_IND_T *inq_result);


/****************************************************************************
NAME    
    connectionHandleInquiryResultWithRssi

DESCRIPTION
    This function handles inquiry results with RSSI as they arrive from BlueStack.

RETURNS
    void
*/
void connectionHandleInquiryResultWithRssi(const connectionInquiryState *state, const DM_HCI_INQUIRY_RESULT_WITH_RSSI_IND_T *inq_result);


/****************************************************************************
NAME    
    connectionHandleExtendedInquiryResult

DESCRIPTION
    This function handles extended inquiry results as they arrive from BlueStack.

RETURNS
    void
*/
void connectionHandleExtendedInquiryResult(const connectionInquiryState *state, const DM_HCI_EXTENDED_INQUIRY_RESULT_IND_T *ind);


/****************************************************************************
NAME    
    connectionHandleInquiryComplete

DESCRIPTION
    This function handles an inquiry complete indication fromBlueStack.

RETURNS
    void
*/
void connectionHandleInquiryComplete(connectionInquiryState *state);


/****************************************************************************
NAME    
    connectionHandleExitPeriodicInquiryComplete

DESCRIPTION
    This function handles an exit periodic inquiry complete indication from
    BlueStack.

RETURNS
    void
*/
void connectionHandleExitPeriodicInquiryComplete(connectionInquiryState *state);


/****************************************************************************
NAME    
    connectionHandleInquiryCancel

DESCRIPTION
    This function cancels an ongoing inquiry

RETURNS
    void
*/
void connectionHandleInquiryCancel(const connectionInquiryState *state, const CL_INTERNAL_DM_INQUIRY_CANCEL_REQ_T *cancel_req);

/****************************************************************************
NAME    
    connectionHandleReadRemoteName

DESCRIPTION
    This function will initiate a read of the remote name of the specified device

RETURNS
    void
*/
void connectionHandleReadRemoteName(connectionInquiryState *state, const CL_INTERNAL_DM_READ_REMOTE_NAME_REQ_T *req);

/****************************************************************************
NAME    
    connectionHandleReadRemoteNameCancel

DESCRIPTION
    This function will  cancel a Read Remote Name Request

RETURNS
    void
*/
void connectionHandleReadRemoteNameCancel(connectionInquiryState *state, 
                         const CL_INTERNAL_DM_READ_REMOTE_NAME_CANCEL_REQ_T *req);

/****************************************************************************
NAME    
    connectionHandleRemoteNameComplete

DESCRIPTION
    Remote name result

RETURNS
    void
*/
void connectionHandleRemoteNameComplete(connectionInquiryState *state, const DM_HCI_REMOTE_NAME_CFM_T* prim);

/****************************************************************************
NAME    
    connectionHandleRemoteNameCancelCfm

DESCRIPTION
    Request to cancel an outstanding  Read Remote Name Request.

RETURNS
    void
*/
void connectionHandleRemoteNameCancelCfm( connectionInquiryState *state, 
                             const DM_HCI_REMOTE_NAME_REQ_CANCEL_CFM_T* prim);

/****************************************************************************
NAME    
    connectionHandleReadLocalName

DESCRIPTION
    This function will initiate a read of the local name of the device

RETURNS
    void
*/
void connectionHandleReadLocalName(connectionInquiryState *state, const CL_INTERNAL_DM_READ_LOCAL_NAME_REQ_T *req);

/****************************************************************************
NAME    
    connectionHandleLocalNameComplete

DESCRIPTION
    Local name result

RETURNS
    void
*/
void connectionHandleLocalNameComplete(connectionInquiryState *state, const DM_HCI_READ_LOCAL_NAME_CFM_T* prim);

/****************************************************************************
NAME    
    connectionHandleWriteInquiryTx

DESCRIPTION
    This function will initiate a write of the inquiry tx power of the device

RETURNS
    void
*/
void connectionHandleWriteInquiryTx(connectionReadInfoState* infoState, const CL_INTERNAL_DM_WRITE_INQUIRY_TX_REQ_T *req);

/****************************************************************************
NAME    
    connectionHandleReadInquiryTx

DESCRIPTION
    This function will initiate a read of the inquiry tx power of the device

RETURNS
    void
*/
void connectionHandleReadInquiryTx(connectionReadInfoState* infoState, connectionInquiryState *state, const CL_INTERNAL_DM_READ_INQUIRY_TX_REQ_T *req);

/****************************************************************************
NAME    
    connectionHandleReadInquiryTx

DESCRIPTION
    This function handles a read inquiry tx result

RETURNS
    void
*/
void connectionHandleReadInquiryTxComplete(connectionInquiryState *state, const DM_HCI_READ_INQUIRY_RESPONSE_TX_POWER_LEVEL_CFM_T *cfm);

/****************************************************************************
NAME    
    connectionHandleWriteIacLapRequest

DESCRIPTION
    Write IAC

RETURNS
    void
*/
void connectionHandleWriteIacLapRequest(connectionInquiryState *state, const CL_INTERNAL_DM_WRITE_IAC_LAP_REQ_T *req);

/****************************************************************************
NAME    
    connectionHandleWriteIacLapComplete

DESCRIPTION
    Write IAC result

RETURNS
    void
*/
void connectionHandleWriteIacLapComplete(connectionInquiryState *state, const DM_HCI_WRITE_CURRENT_IAC_LAP_CFM_T *prim);

/****************************************************************************
NAME    
    connectionHandleWriteInquiryModeRequest

DESCRIPTION
    Write inquiry mode

RETURNS
    void
*/
void connectionHandleWriteInquiryModeRequest(connectionInquiryState *state, const CL_INTERNAL_DM_WRITE_INQUIRY_MODE_REQ_T *req);

/****************************************************************************
NAME    
    connectionHandleWriteInquiryModeComplete

DESCRIPTION
    Write inquiry mode result

RETURNS
    void
*/
void connectionHandleWriteInquiryModeComplete(connectionInquiryState *state, const DM_HCI_WRITE_INQUIRY_MODE_CFM_T *prim);

/****************************************************************************
NAME    
    connectionHandleReadInquiryMode

DESCRIPTION
    Read inquiry mode

RETURNS
    void
*/
void connectionHandleReadInquiryModeRequest(connectionInquiryState *state, const CL_INTERNAL_DM_READ_INQUIRY_MODE_REQ_T *req);

/****************************************************************************
NAME    
    connectionHandleReadInquiryModeComplete

DESCRIPTION
    Read inquiry mode result

RETURNS
    void
*/
void connectionHandleReadInquiryModeComplete(connectionInquiryState *state, const DM_HCI_READ_INQUIRY_MODE_CFM_T *prim);

/****************************************************************************
NAME
    connectionHandleWriteEirDataRequest

DESCRIPTION
    Write the Extended Inquiry Data

RETURNS
    void
*/
void connectionHandleWriteEirDataRequest(connectionReadInfoState *infoState, const CL_INTERNAL_DM_WRITE_EIR_DATA_REQ_T *req);

/****************************************************************************
NAME
    connectionHandleReadEirDataRequest

DESCRIPTION
    Handles request for Reading the Extended Inquiry Data.

RETURNS
    void
*/
void connectionHandleReadEirDataRequest(connectionReadInfoState *infoState, connectionInquiryState *state, const CL_INTERNAL_DM_READ_EIR_DATA_REQ_T *req);

/****************************************************************************
NAME
    connectionHandleReadEirDataComplete

DESCRIPTION
    Handles result from Reading the Extended Inquiry Data.

RETURNS
    void
*/
void connectionHandleReadEirDataComplete(connectionInquiryState *state, const DM_HCI_READ_EXTENDED_INQUIRY_RESPONSE_DATA_CFM_T *cfm);

#endif    /* CONNECTION_DM_INQUIRY_HANDLER_H_ */

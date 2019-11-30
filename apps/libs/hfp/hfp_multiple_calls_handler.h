/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_multiple_calls_handler.h
    
DESCRIPTION
    
*/

#ifndef HFP_MULTIPLE_CALLS_HANDLER_H_
#define HFP_MULTIPLE_CALLS_HANDLER_H_


/****************************************************************************
NAME    
    hfpHandleCallWaitingNotificationEnable

DESCRIPTION
    Enable call waiting notifications from the AG.

RETURNS
    void
*/
bool hfpHandleCallWaitingNotificationEnable(const HFP_INTERNAL_AT_CCWA_REQ_T *req);


/****************************************************************************
NAME    
    hfpSendChldCmd

DESCRIPTION
    Send AT+CHLD=action to the AG. If both AG and HF support ECC then this
    can also be used to send AT+CHLD=action call_idx.

RETURNS
    void
*/
bool hfpSendChldCmd(const HFP_INTERNAL_AT_CHLD_REQ_T* req);


/****************************************************************************
NAME    
    hfpHandleChldAtAck

DESCRIPTION
    This function handles the receipt of an OK or ERROR response to a
    multiparty call handling request. The status argument is used to inform 
    the application whether the AT command was recognised by the AG or not. 

RETURNS
    void
*/
void hfpHandleChldAtAck(hfp_link_data* link, hfp_chld_action action, hfp_lib_status status);


#endif /* HFP_MULTIPLE_CALLS_HANDLER_H_ */

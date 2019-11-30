/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_response_hold_handler.h
    
DESCRIPTION
    
*/

#ifndef HFP_RESPONSE_HOLD_HANDLER_H_
#define HFP_RESPONSE_HOLD_HANDLER_H_


/****************************************************************************
NAME    
    hfpHandleBtrhStatusReq

DESCRIPTION
    Request response hold status from the AG.

RETURNS
    void
*/
void hfpHandleBtrhStatusReq(hfp_link_data* link);


/****************************************************************************
NAME    
    hfpHandleBtrhStatusAtAck

DESCRIPTION
    Request to AG to get response and hold status has been acknowledged 
    (completed) by the AG.

RETURNS
    void
*/
void hfpHandleBtrhStatusAtAck(hfp_link_data* link);


/****************************************************************************
NAME    
    hfpHandleBtrhHoldReq

DESCRIPTION
    Request to AG to perform a response and hold action.

RETURNS
    void
*/
bool hfpHandleBtrhReq(const HFP_INTERNAL_AT_BTRH_REQ_T* req);


#ifndef HFP_MIN_CFM
/****************************************************************************
NAME    
    hfpHandleBtrhAtAck

DESCRIPTION
    Request to AG to perform a response and hold action has been acknowledged 
    (completed) by the AG.

RETURNS
    void
*/
void hfpHandleBtrhAtAck(hfp_link_data* link, hfp_btrh_action action, hfp_lib_status status);
#endif

#endif /* HFP_RESPONSE_HOLD_HANDLER_H_ */

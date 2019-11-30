/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_indicators_handler.h
    
DESCRIPTION
    
*/

#ifndef HFP_INDICATORS_HANDLER_H_
#define HFP_INDICATORS_HANDLER_H_

/****************************************************************************
NAME    
    hfpHandleSetActiveIndicatorsReq

DESCRIPTION
    Handle request to set active indicators from the app

AT INDICATION
    +BIA=

RETURNS
    void
*/
void hfpHandleSetActiveIndicatorsReq(hfp_link_data* link);


/****************************************************************************
NAME    
    hfpHandleBiaAtAck

DESCRIPTION
    Handle Ack from AG in response to AT+BIA=

RETURNS
    void
*/
void hfpHandleBiaAtAck(hfp_link_data* link);

#endif /* HFP_INDICATORS_HANDLER_H_ */

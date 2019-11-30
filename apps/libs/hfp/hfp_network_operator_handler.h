/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_network_operator_handler.h
    
DESCRIPTION
    
*/

#ifndef HFP_NETWORK_OPERATOR_HANDLER_H_
#define HFP_NETWORK_OPERATOR_HANDLER_H_


/****************************************************************************
NAME    
    hfpHandleNetworkOperatorReq

DESCRIPTION
    Request network operator information from the AG.

RETURNS
    void
*/
void hfpHandleNetworkOperatorReq(hfp_link_data* link);


/****************************************************************************
NAME    
    hfpHandleCopsFormatAtAck

DESCRIPTION
    Network operator reporting format command has been acknowledged (completed)
    by the AG.

RETURNS
    void
*/
MessageId hfpHandleCopsFormatAtAck(hfp_link_data* link, hfp_lib_status status);


#endif /* HFP_NETWORK_OPERATOR_HANDLER_H_ */

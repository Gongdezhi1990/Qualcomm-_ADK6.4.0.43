/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_extended_error_handler.h
    
DESCRIPTION
    
*/

#ifndef HFP_EXTENDED_ERROR_HANDLER_H_
#define HFP_EXTENDED_ERROR_HANDLER_H_


/****************************************************************************
NAME    
    hfpHandleExtendedErrorReq

DESCRIPTION
    Request extended error information from the AG.

RETURNS
    void
*/
void hfpHandleExtendedErrorReq(hfp_link_data* link);


/****************************************************************************
NAME    
    hfpHandleCmeeAtAck

DESCRIPTION
    Extended error information request command has been acknowledged by the AG.

RETURNS
    void
*/
void hfpHandleCmeeAtAck(hfp_link_data* link);


#endif /* HFP_EXTENDED_ERROR_HANDLER_H_ */

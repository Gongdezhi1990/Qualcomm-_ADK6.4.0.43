/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_dial_handler.h
    
DESCRIPTION
    
*/

#ifndef HFP_DIAL_HANDLER_H_
#define HFP_DIAL_HANDLER_H_


/****************************************************************************
NAME    
    hfpHandleLastNumberRedial

DESCRIPTION
    Issue a last number redial request.

RETURNS
    void
*/
void hfpHandleLastNumberRedial(hfp_link_data* link);


/****************************************************************************
NAME    
    hfpHandleDialNumberRequest

DESCRIPTION
    Issue a request to dial the supplied number.

RETURNS
    void
*/
void hfpHandleDialNumberRequest(const HFP_INTERNAL_AT_ATD_NUMBER_REQ_T *req);


/****************************************************************************
NAME    
    hfpHandleDialMemoryRequest

DESCRIPTION
    Send a dial memory location request to the AG.

RETURNS
    void
*/
void hfpHandleDialMemoryRequest(const HFP_INTERNAL_AT_ATD_MEMORY_REQ_T *req);


#endif /* HFP_DIAL_HANDLER_H_ */

/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_dtmf_handler.h
    
DESCRIPTION
    
*/

#ifndef HFP_DTMF_HANDLER_H_
#define HFP_DTMF_HANDLER_H_


/****************************************************************************
NAME    
    hfpHandleDtmfRequest

DESCRIPTION
    HAndle a request to send a DTMF tone to the AG.

RETURNS
    void
*/
void hfpHandleDtmfRequest(const HFP_INTERNAL_AT_VTS_REQ_T *req);


#endif /* HFP_DTMF_HANDLER_H_ */

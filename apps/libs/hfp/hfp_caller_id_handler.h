/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_caller_id_handler.h
    
DESCRIPTION
    
*/

#ifndef HFP_CALLER_ID_HANDLER_H_
#define HFP_CALLER_ID_HANDLER_H_


/****************************************************************************
NAME    
    hfpHandleCallerIdEnableReq

DESCRIPTION
    Attempt to enable the caller id functionality at the AG.

RETURNS
    void
*/
bool hfpHandleCallerIdEnableReq(const HFP_INTERNAL_AT_CLIP_REQ_T *req);


#endif /* HFP_CALLER_ID_HANDLER_H_ */

/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_ok.h
    
DESCRIPTION
    
*/

#ifndef HFP_OK_H_
#define HFP_OK_H_


/****************************************************************************
NAME    
    hfpHandleWaitAtTimeout

DESCRIPTION
    Waiting for OK/ ERROR response to AT cmd timeout has expired. This means 
    that we have not received a response for the last AT cmd. So we don't
    get completely stuck, send out the next cmd anyway.

RETURNS
    void
*/
void hfpHandleWaitAtTimeout(hfp_link_data* link);

#endif /* HFP_OK_H_ */

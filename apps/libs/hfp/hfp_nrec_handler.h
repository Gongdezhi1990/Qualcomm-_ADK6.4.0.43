/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_nrec_handler.h
    
DESCRIPTION
    
*/

#ifndef HFP_NREC_HANDLER_H_
#define HFP_NREC_HANDLER_H_


/****************************************************************************
NAME    
    hfpHandleNrEcDisable

DESCRIPTION
    Send a request to the AG to disable its Noise Reduction (NR) and Echo
    Cancellation (EC) capabilities.

RETURNS
    void
*/
void hfpHandleNrEcDisable(hfp_link_data* link);


/****************************************************************************
NAME    
    hfpHandleNrecAtAck

DESCRIPTION
    Received an ack from the AG for the AT+NREC cmd.

RETURNS
    void
*/
void hfpHandleNrecAtAck(hfp_link_data* link);


#endif /* HFP_NREC_HANDLER_H_ */

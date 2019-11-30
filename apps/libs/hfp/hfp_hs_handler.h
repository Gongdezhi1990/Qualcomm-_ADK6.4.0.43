/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_hs_handler.h
    
DESCRIPTION
    
*/

#ifndef HFP_HS_HANDLER_H_
#define HFP_HS_HANDLER_H_


/****************************************************************************
NAME    
    hfpSendHsButtonPress

DESCRIPTION
    Send a button press AT cmd to the AG.

RETURNS
    void
*/
void hfpSendHsButtonPress(hfp_link_data* link, hfp_at_cmd pending_cmd);


#endif /* HFP_HS_HANDLER_H_ */

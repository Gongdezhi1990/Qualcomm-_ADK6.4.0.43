/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_qce_handler.h

DESCRIPTION

*/

#ifndef HFP_QCE_HANDLER_H
#define HFP_QCE_HANDLER_H


/****************************************************************************
NAME
    hfpHandleQcsAtAck

DESCRIPTION
    Called when we receive OK/ ERROR in response to the AT+QCS cmd.
    If there is an error, then reset the qce_codec_mode_id for this link,
    indicating that no QCE codec is selected.

RETURNS
    void
*/
void hfpHandleQcsAtAck(hfp_link_data* link, hfp_lib_status status);

#endif /* HFP_QCE_HANDLER_H */

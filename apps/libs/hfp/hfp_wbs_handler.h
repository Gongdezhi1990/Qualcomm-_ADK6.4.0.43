/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_wbs_handler.h
    
DESCRIPTION
    
*/

#ifndef HFP_WBS_HANDLER_H_
#define HFP_WBS_HANDLER_H_

#include "hfp_private.h"
#include "hfp_wbs.h"


/****************************************************************************
NAME
    hfpWbsEnable

DESCRIPTION
    Called during initialisation to set up HF supported codecs if the codec
    negotiation feature bit is enabled. If codec data is correctly configured
    this sets up codec data for HFP.

RETURNS
    void
*/
void hfpWbsEnable(const hfp_wbs_codec_mask supported_wbs_codecs);


/****************************************************************************
NAME    
    hfpWbsStartCodecNegotiation

DESCRIPTION
    Start WBS codec negotiation, if supported by the HF and AG, by sending
    AT+BCC to the AG.

RETURNS
    TRUE if negotiation was started, FALSE otherwise
*/
bool hfpWbsStartCodecNegotiation(hfp_link_data* link);


/****************************************************************************
NAME    
    hfpHandleBccAtAck

DESCRIPTION
    Called when we receive OK/ ERROR in response to the AT+BCC cmd. This 
    indicates whether the AG recognised the cmd. If AG responded with OK it
    will send +BCS with the selected codec ID.

RETURNS
    void
*/
void hfpHandleBccAtAck(hfp_link_data* link, hfp_lib_status status);


/****************************************************************************
NAME    
    hfpHandleBcsAtAck

DESCRIPTION
    Called when AG acknowledges our AT+BCS. This should always be an OK as we
    respond with the codec the AG requested. If something has gone wrong then
    we notify the application audio connection has failed.

RETURNS
    void
*/
void hfpHandleBcsAtAck(hfp_link_data* link, hfp_lib_status status);


#endif /* HFP_WBS_HANDLER_H_ */


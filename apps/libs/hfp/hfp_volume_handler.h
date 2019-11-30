/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_volume_handler.h
    
DESCRIPTION
    
*/

#ifndef HFP_SOUND_HANDLER_H_
#define HFP_SOUND_HANDLER_H_


/****************************************************************************
NAME    
    hfpHandleVgsRequest

DESCRIPTION
    Send a volume gain update to the AG.

RETURNS
    void
*/
void hfpHandleVgsRequest(hfp_link_data* link, uint8 gain);


/****************************************************************************
NAME    
    hfpHandleVgmRequest

DESCRIPTION
    Send the VGM request to the AG.

RETURNS
    void
*/
void hfpHandleVgmRequest(hfp_link_data* link, uint8 gain);


#endif /* HFP_SOUND_HANDLER_H_ */

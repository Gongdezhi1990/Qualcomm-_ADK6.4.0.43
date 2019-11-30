/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_voice_tag_handler.h
    
DESCRIPTION
    
*/

#ifndef HFP_VOICE_TAG_HANDLER_H_
#define HFP_VOICE_TAG_HANDLER_H_



/****************************************************************************
NAME    
    hfpHandleGetVoiceTagReq

DESCRIPTION
    Issue an "attach number to a voice tag" request to the AG to retrieve
    a phone number.

RETURNS
    void
*/
bool hfpHandleGetVoiceTagReq(hfp_link_data* link);


#endif /* HFP_VOICE_TAG_HANDLER_H_ */

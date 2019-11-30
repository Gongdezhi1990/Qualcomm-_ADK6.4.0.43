/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_voice_handler.h
    
DESCRIPTION
    
*/

#ifndef HFP_VOICE_HANDLER_H_
#define HFP_VOICE_HANDLER_H_


/****************************************************************************
NAME    
    hfpHandleVoiceRecognitionEnable

DESCRIPTION
    Enable/ disable voice dialling at the AG.

RETURNS
    void
*/
bool hfpHandleVoiceRecognitionEnable(const HFP_INTERNAL_AT_BVRA_REQ_T *req);


/****************************************************************************
NAME    
    hfpHandleVoiceRecognitionEnable

DESCRIPTION
    Enable/ disable voice dialling at the AG.

RETURNS
    void
*/
bool hfpHandleHspVoiceRecognitionEnable(const HFP_INTERNAL_AT_BVRA_REQ_T *req);


#endif /* HFP_VOICE_HANDLER_H_ */

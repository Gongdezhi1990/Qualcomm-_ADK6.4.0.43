/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_voice_handler.c        

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_indicators_handler.h"
#include "hfp_parse.h"
#include "hfp_send_data.h"
#include "hfp_voice_handler.h"
#include "hfp_link_manager.h"
#include "hfp_hs_handler.h"
#include "hfp_call_handler.h"

#include <panic.h>
#include <string.h>


/****************************************************************************
NAME    
    hfpHandleVoiceRecognitionEnable

DESCRIPTION
    Enable/ disable voice dialling at the AG.

RETURNS
    void
*/
bool hfpHandleVoiceRecognitionEnable(const HFP_INTERNAL_AT_BVRA_REQ_T *req)
{
    hfp_link_data* link = req->link;
    
    if(hfFeatureEnabled(HFP_VOICE_RECOGNITION) && agFeatureEnabled(link, AG_VOICE_RECOGNITION))
    {
        /* Only send the cmd if the AG and local device support the voice dial feature */
        char *bvra;

        if (req->enable)
            bvra = "AT+BVRA=1\r";
        else
            bvra = "AT+BVRA=0\r";

        /* Send the AT cmd over the air */
        hfpSendAtCmd(link, (uint16)strlen(bvra), bvra, hfpBvraCmdPending);
        return TRUE;
    }
    
    return FALSE;
}


/****************************************************************************
NAME    
    hfpHandleVoiceRecognitionEnable

DESCRIPTION
    Enable/ disable voice dialling at the AG.

RETURNS
    void
*/
bool hfpHandleHspVoiceRecognitionEnable(const HFP_INTERNAL_AT_BVRA_REQ_T *req)
{
    hfp_link_data* link = req->link;
    
    if(req->enable && link->bitfields.ag_call_state == hfp_call_state_idle)
    {
        hfpSendHsButtonPress(link, hfpBvraCmdPending);
        return TRUE;
    }
    
    return FALSE;
}


/****************************************************************************
NAME    
    hfpHandleVoiceRecognitionStatus

DESCRIPTION
    Voice recognition status indication received from the AG.

AT INDICATION
    +BVRA

RETURNS
    void
*/
void hfpHandleVoiceRecognitionStatus(Task link_ptr, const struct hfpHandleVoiceRecognitionStatus *ind)
{
    /* 
        Send a message to the application telling it the current status of the 
        voice recognition engine at the AG. 
    */
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    
    MAKE_HFP_MESSAGE(HFP_VOICE_RECOGNITION_IND);
    message->priority = hfpGetLinkPriority(link);
    message->enable = ind->enable;
    MessageSend(theHfp->clientTask, HFP_VOICE_RECOGNITION_IND, message);

    /* This is necessary when dealing with 0.96 AGs */
    if(!ind->enable)
        hfpHandleCallVoiceRecDisabled(link);
}

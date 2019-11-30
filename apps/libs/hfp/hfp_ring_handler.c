/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_ring_handler.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_parse.h"
#include "hfp_indicators_handler.h"
#include "hfp_link_manager.h"
#include "hfp_call_handler.h"

#include <panic.h>


/****************************************************************************
NAME    
    hfpHandleRing

DESCRIPTION
    Received a RING indication.

AT INDICATION
    RING

RETURNS
    void
*/
void hfpHandleRing(Task link_ptr)
{
    /* Get the link we got ring indication on */
    hfp_link_data* link = (hfp_link_data*)link_ptr;

    if(link)
    {
        /* Update call state if required */
        hfpHandleCallRingIndication(link);
        
        if(link->bitfields.ag_call_state == hfp_call_state_incoming)
        {
            MAKE_HFP_MESSAGE(HFP_RING_IND);
            message->in_band = FALSE;
            
            /* This is an in band ring if... */
            if((hfpLinkIsHfp(link) && agFeatureEnabled(link, AG_IN_BAND_RING)))
            {
                /* ...AG is HFP and supports in band ring */
                message->in_band = TRUE;
            }
            else if(hfpLinkIsHsp(link) && link->bitfields.audio_state == hfp_audio_connected)
            {
                /* ...AG is HSP and SCO is open */
                message->in_band = TRUE;
            }
            
            /* Send a message to the application */
            message->priority = hfpGetLinkPriority(link);
            MessageSend(theHfp->clientTask, HFP_RING_IND, message);
        }
    }
}


/****************************************************************************
NAME    
    hfpHandleInBandRingTone

DESCRIPTION
    Received a BSIR indication.

AT INDICATION
    +BSIR

RETURNS
    void
*/
void hfpHandleInBandRingTone(Task link_ptr, const struct hfpHandleInBandRingTone *ind)
{
    /* Get the link we got in band ring indication on */
    hfp_link_data* link = (hfp_link_data*)link_ptr;
    
    if(link)
    {
        /* Set/Clear the in band ring feature for this link */
        if(ind->enable)
            link->ag_supported_features |= AG_IN_BAND_RING;
        else
            link->ag_supported_features &= (uint16)~AG_IN_BAND_RING;
    }
}


/****************************************************************************
NAME    
    hfpHandleInBandRingToneDisable

DESCRIPTION
    Received a BSIR disable indication.

AT INDICATION
    +BSIR: 0

RETURNS
    void
*/
void hfpHandleInBandRingToneDisable(Task link_ptr)
{
    struct hfpHandleInBandRingTone ind;
    ind.enable = 0;
    hfpHandleInBandRingTone(link_ptr, &ind);
}


/****************************************************************************
NAME    
    hfpHandleInBandRingToneEnable

DESCRIPTION
    Received a BSIR enable indication.

AT INDICATION
    +BSIR: 1

RETURNS
    void
*/
void hfpHandleInBandRingToneEnable(Task link_ptr)
{
    struct hfpHandleInBandRingTone ind;
    ind.enable = 1;
    hfpHandleInBandRingTone(link_ptr, &ind);
}

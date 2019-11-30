/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_indicators.c
    
DESCRIPTION
    Interface for the HF indicators for the HFP library.

*/

#include "hfp.h"
#include "hfp_private.h"
#include "hfp_indicators.h"
#include "hfp_link_manager.h"
#include "hfp_common.h"
#include "hfp_send_data.h"

#include <print.h>
#include <stdio.h>


void sendHfIndicatorsToApp(hfp_link_data* link, uint16 hf_indicators_mask, uint16 num_hf_indicators)
{
    /* Send a message to the app */
    MAKE_HFP_MESSAGE(HFP_HF_INDICATORS_REPORT_IND);
    message->priority = hfpGetLinkPriority(link);
    message->hf_indicators_mask = hf_indicators_mask;
    message->num_hf_indicators = num_hf_indicators;
    MessageSend(theHfp->clientTask, HFP_HF_INDICATORS_REPORT_IND, message);
}

void sendHfIndicatorSignalToApp(hfp_link_data* link, uint16 assigned_num, bool status)
{
    /* Send a message to the app */
    MAKE_HFP_MESSAGE(HFP_HF_INDICATORS_IND);
    message->priority = hfpGetLinkPriority(link);
    message->hf_indicator_assigned_num = assigned_num;
    message->hf_indicator_status = status;
    MessageSend(theHfp->clientTask, HFP_HF_INDICATORS_IND, message);
}

/****************************************************************************
NAME    
    hfpHfIndicatorMaskToAssignedId

DESCRIPTION
    Convert a hf indicators mask to a Assigned ID (as defined in Assigned Number Hands Free profile)

RETURNS
    The Assigned Number ID if successful, or hf_indicators_invalid if the indicators
    mask passed in was invalid
*/
hfp_indicators_assigned_id hfpHfIndicatorMaskToAssignedId(hfp_hf_indicators_mask indicators_mask)
{
    switch(indicators_mask)
    {
        case hfp_enhanced_safety_mask:
            return hf_enhanced_safety;
        case hfp_battery_level_mask:
            return hf_battery_level;
        default:
            return hf_indicators_invalid;
    }
}


/****************************************************************************
NAME    
    hfpAssignedIdToHfindicatorMask

DESCRIPTION
    Convert a hf indicator Assigned number to  hfp_hf_indicators_mask
    
RETURNS
    hfp_hf_indicators_mask if successful, or hfp_indicator_mask_none if the hf indicator
    Assigned number passed in was invalid
*/
hfp_hf_indicators_mask hfpAssignedIdToHfindicatorMask(hfp_indicators_assigned_id assigned_id)
{
    switch(assigned_id)
    {
        case hf_enhanced_safety :
            return hfp_enhanced_safety_mask;
        case hf_battery_level:
            return hfp_battery_level_mask;
        default:
            return hfp_indicator_mask_none;
    }
}


/****************************************************************************
NAME    
    hfpHandleBievIndStatusRequest

DESCRIPTION
    Send AT+BIEV=anum,value to the AG.

RETURNS
    void
*/
void HfpBievIndStatusRequest(hfp_link_priority priority, uint16 assigned_id, uint16 value)
{
    hfp_link_data*  link = hfpGetLinkFromPriority(priority);

    if(link)
    {    
        /* Only send this if both sides support HF Indicators and indicator is enabled by AG */
        if (hfFeatureEnabled(HFP_HF_INDICATORS) && agFeatureEnabled(link, AG_HF_INDICATORS) &&
		      (link->ag_supported_hf_indicators.hf_indicators_status & (1 << assigned_id)))
        {
           char   biev_req[AT_COMMAND_LENGTH];       
        
           /* Start the AT+BIEV command */
           uint8  char_idx = sprintf(biev_req, "AT+BIEV=");

	       /* Convert this bit to assigned number ID and put into AT+BIND */
           char_idx += sprintf(&biev_req[char_idx], "%d,", assigned_id);
	       char_idx += sprintf(&biev_req[char_idx], "%d,", value);
        
           /* Replace last ',' with '\r' */
           sprintf(&biev_req[char_idx-1], "\r");
        
           /* Send the AT cmd over the air */
           hfpSendAtCmd(link, char_idx, biev_req, hfpBievStatusCmdPending);
        }
    }
}

/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
*/


#include "aghfp.h"
#include "aghfp_common.h"
#include "aghfp_hf_indicators.h"
#include "aghfp_slc_handler.h"
#include "aghfp_ok.h"
#include "aghfp_private.h"
#include "aghfp_send_data.h"

#include <bdaddr.h>
#include <panic.h>
#include <string.h> /* For memcpy */
#include <stdio.h>
#include <sink.h>

#define HF_ENHANCED_SAFETY_INDICATOR_VALUE_MIN       0
#define HF_ENHANCED_SAFETY_INDICATOR_VALUE_MAX       1
#define HF_BATTERY_LEVEL_INDICATOR_VALUE_MIN         0
#define HF_BATTERY_LEVEL_INDICATOR_VALUE_MAX         100

void AghfpBindIndicatorEnable(AGHFP *aghfp, aghfp_hf_indicators_mask   ag_hf_indicators)
{
    /* Store the local supported HF indicators */
    aghfp->ag_hf_indicators_info.active_hf_indicators = ag_hf_indicators;    
}


/****************************************************************************
 The app has asked to send a hf indicator state (enable/disable) to the HF - create an internal
 message that the profile handler will deal with.
*/
void AghfpSendHfIndicatorState(AGHFP *aghfp, aghfp_hf_indicators_assigned_id assigned_num, aghfp_hf_indicator_state state)
{
    aghfp_hf_indicators_mask hf_indicator_mask = aghfpAssignedIdToHfIndicatorMask(assigned_num);
    if((aghfp->ag_hf_indicators_info.active_hf_indicators & hf_indicator_mask) && (aghfp->hf_indicators & hf_indicator_mask))
    {
        MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SEND_BIND_INDICATOR_STATE);
        message->assigned_num = assigned_num;
        message->state = state;

        /* Update the HF indicators state */
        aghfp->ag_hf_indicators_info.hf_indicators_state =  (state << assigned_num);

        MessageSend(&aghfp->task, AGHFP_INTERNAL_SEND_BIND_INDICATOR_STATE, message);
    }
    else
    {
        aghfpSendCommonCfmMessageToApp(AGHFP_SEND_BIND_INDICATOR_STATE_CFM, aghfp, aghfp_fail);
    }
}

/****************************************************************************
	Sends the current status of the HF indicators as given by the client task
*/
void AghfpHfIndicatorsInitStatusResponse(AGHFP *aghfp, uint16 hf_indicators_state_mask)
{
    char buf[16];
    uint16 check_bit;
    aghfp_hf_indicators_assigned_id hf_assigned_num;
    aghfp_hf_indicator_state   hf_indicator_state;

    /* Create a local copy of HF supported indicators */
    aghfp_hf_indicators_mask supported_hf_indicators = aghfp->ag_hf_indicators_info.active_hf_indicators;

    /* Store the initial HF indicators state */
    aghfp->ag_hf_indicators_info.hf_indicators_state = hf_indicators_state_mask;

    /* Run through all bits of supported indicators mask until no more set */
    for(check_bit=1; supported_hf_indicators > 0; check_bit <<= 1)
    {
         /* Check if bit is set in the indicators mask and remote HF supports HF indicator */
         if((supported_hf_indicators & check_bit) & aghfp->hf_indicators)
         {
              aghfpAtCmdBegin(aghfp);
		aghfpAtCmdString(aghfp, "+BIND:");

		/* Convert this bit to assigned number ID and put into +BIND:*/
              hf_assigned_num = aghfpHfIndicatorMaskToAssignedId(supported_hf_indicators & check_bit);		   

              /* Extract the hf indicator state from indicators mask */
		hf_indicator_state= ((hf_indicators_state_mask >> hf_assigned_num) & aghfp_hf_indicator_on);
		
              sprintf(buf, "%d,%d", hf_assigned_num, hf_indicator_state);
              aghfpAtCmdString(aghfp, buf);
              aghfpAtCmdEnd(aghfp);
		
          }
          /* Clear this bit in supported indicators */
          supported_hf_indicators &= ~(supported_hf_indicators & check_bit); 

    }
    
    aghfpSendOk(aghfp);
    /* Send SLC connect cfm to app */
    aghfpSendSlcConnectCfmToApp(aghfp_connect_success, aghfp);
}

static bool aghfpHfIndicatorValidValue(aghfp_hf_indicators_assigned_id hf_assigned_num, uint16 value)
{
    bool status = FALSE;    

    switch(hf_assigned_num)
    {
       case aghfp_enhanced_safety:
         if(value == HF_ENHANCED_SAFETY_INDICATOR_VALUE_MIN || value == HF_ENHANCED_SAFETY_INDICATOR_VALUE_MAX)
         {
            status = TRUE;
         }
         break;
	 case aghfp_battery_level:
         if(value >= HF_BATTERY_LEVEL_INDICATOR_VALUE_MIN && value <= HF_BATTERY_LEVEL_INDICATOR_VALUE_MAX)
         {
            status = TRUE;
         }
         break;     
     default:
         break;
    }
    return status;
}

static bool aghfpHfIndicatorSupported(AGHFP *aghfp, aghfp_hf_indicators_assigned_id hf_assigned_num )
{
    if ((aghfp->ag_hf_indicators_info.hf_indicators_state) & (1<<hf_assigned_num))
    {
        return TRUE;
    } 
    else
    {
        return FALSE;
    }   	
}

static aghfp_hf_indicator_state aghfpGetHfIndicatorState(AGHFP *aghfp, aghfp_hf_indicators_assigned_id hf_assigned_num)
{
    aghfp_hf_indicator_state   hf_indicator_state;
	
    /* Extract the hf indicator state from indicators mask */
    hf_indicator_state= (((aghfp->ag_hf_indicators_info.hf_indicators_state) >> hf_assigned_num) & aghfp_hf_indicator_on);

    return hf_indicator_state;
}
/****************************************************************************
NAME    
    aghfpHfIndicatorMaskToAssignedId

DESCRIPTION
    Convert a hf indicators mask to a Assigned ID (as defined in Assigned Number Hands Free profile)

RETURNS
    The Assigned Number ID if successful, or aghf_indicators_invalid if the indicators
    mask passed in was invalid
*/
aghfp_hf_indicators_assigned_id aghfpHfIndicatorMaskToAssignedId(aghfp_hf_indicators_mask indicators_mask)
{
    switch(indicators_mask)
    {
        case aghfp_enhanced_safety_mask:
            return aghfp_enhanced_safety;
        case aghfp_battery_level_mask:
            return aghfp_battery_level;
        default:
            return aghfp_hf_indicators_invalid;
    }
}

/****************************************************************************
NAME    
    aghfpAssignedIdToHfIndicatorMask

DESCRIPTION
    Convert a hf indicator Assigned number to  aghfp_hf_indicators_mask
    
RETURNS
    aghfp_hf_indicators_mask if successful, or aghfp_indicator_mask_none if the hf indicator
    Assigned number passed in was invalid
*/
aghfp_hf_indicators_mask aghfpAssignedIdToHfIndicatorMask(aghfp_hf_indicators_assigned_id assigned_id)
{
    switch(assigned_id)
    {
        case aghfp_enhanced_safety :
            return aghfp_enhanced_safety_mask;
        case aghfp_battery_level:
            return aghfp_battery_level_mask;
        default:
            return aghfp_indicator_mask_none;
    }
}


/****************************************************************************
	Sends the current status of the HF indicator as given by the client task
*/
void aghfpHfIndicatorsStatusResponse(AGHFP *aghfp, aghfp_hf_indicators_assigned_id hf_assigned_num, aghfp_hf_indicator_state  state )
{
    char buf[16];

    aghfpAtCmdBegin(aghfp);
    aghfpAtCmdString(aghfp, "+BIND:");				
    sprintf(buf, "%d,%d", hf_assigned_num, state);	
    aghfpAtCmdString(aghfp, buf);	
    aghfpAtCmdEnd(aghfp);  
}


/****************************************************************************
NAME	
	aghfpHandleSendBindDetails

DESCRIPTION
	Update the AGHFP's information regarding the HF's hf indicators and send an OK AT command.
	This is a response to the AT+BIND=<assigned num1>,<assigned num2>.. command.

RETURNS
	void
*/
void aghfpHandleBindRequest(AGHFP *aghfp, AGHFP_INTERNAL_AT_BIND_REQ_T *hf_indicators )
{
	uint16 counter;
	MAKE_AGHFP_MESSAGE(AGHFP_HF_INDICATORS_REPORT_IND);
       message->aghfp = aghfp;
   
	aghfp->hf_indicators= 0;

	/* Convert from a list of UUID16s to a hf_indicators mask. */
	for(counter = 0; counter < hf_indicators->num_hf_indicators; counter++)
	{
		aghfp->hf_indicators |= aghfpAssignedIdToHfIndicatorMask(hf_indicators->hf_indicators_uuids[counter]);
	}

       /* Notify to source app about the HF indicators supported */
	 MessageSend(aghfp->client_task, AGHFP_HF_INDICATORS_REPORT_IND, message);
		   
       /* Send an Ok response to AT+BIND */
	aghfpSendOk(aghfp);
}

/****************************************************************************
NAME    
    aghfpHandleSendBindDetails

DESCRIPTION
    Send +BIND:(<a>,<b>..) to the HF.

RETURNS
    void
*/
void aghfpHandleSendBindDetails(AGHFP *aghfp)
{
      char   bind_req[20];
      uint16 check_bit;

      /* Create a local copy of HF supported indicators */
      aghfp_hf_indicators_mask supported_hf_indicators = aghfp->ag_hf_indicators_info.active_hf_indicators;
     
      /* Start the +BIND response */
      uint8  char_idx = sprintf(bind_req, "+BIND:(");

      AGHFP_DEBUG(("aghfpHandleSendBindDetails\n"));
      /* Run through all bits of supported indicators mask until no more set */
      for(check_bit=1; supported_hf_indicators > 0; check_bit <<= 1)
      {
           /* Check if bit is set in the indicators mask */
           if(supported_hf_indicators & check_bit)
            {
                /* Convert this bit to assigned number ID and put into +BIND:*/
                char_idx += sprintf(&bind_req[char_idx], "%d,", aghfpHfIndicatorMaskToAssignedId(supported_hf_indicators & check_bit));
            }
            /* Clear this bit in supported indicators */
            supported_hf_indicators &= ~(supported_hf_indicators & check_bit);
       }
       /* Replace last ',' with ')' */
	sprintf(&bind_req[char_idx-1], ")");
	   
       /* Replace last with '\r' */
       sprintf(&bind_req[char_idx], "\r");

       aghfpSendAtCmd(aghfp, bind_req);
}

/****************************************************************************
NAME    
    aghfpHandleBievRequest

DESCRIPTION
    Handle AT+BIEV=<a>,<value> from the HF

RETURNS
    void
*/
void aghfpHandleBievRequest(AGHFP *aghfp, AGHFP_INTERNAL_AT_BIEV_STATUS_REQ_T* feature)
{
     /* Send OK response if HF indicator is supported and is in enabled state( on AG side) else send an ERROR
       response as per HFP Spec Sec 4.35.1.5 */
    if(!aghfpHfIndicatorSupported(aghfp, feature->hf_indicator_assigned_num))
    {
        /* Send an ERROR response to AT+BIEV  */
        aghfpSendError(aghfp);
    }
    else if(aghfpGetHfIndicatorState(aghfp, feature->hf_indicator_assigned_num))
    {
        if(!aghfpHfIndicatorValidValue(feature->hf_indicator_assigned_num, feature->value))
        {
            /* Send an ERROR response for invalid HF Indicator value as per HFP Spec Sec 4.35.1.5 */        
            aghfpSendError(aghfp);			
        }
	 else
        {
            /* Notify app with the HF Indicator value */
            MAKE_AGHFP_MESSAGE(AGHFP_HF_INDICATORS_VALUE_IND);
            message->aghfp = aghfp;
            message->hf_indicator_assigned_num = feature->hf_indicator_assigned_num;
            message->value = feature->value;  
            MessageSend(aghfp->client_task, AGHFP_HF_INDICATORS_VALUE_IND, message);
		
            /* Send an Ok response to AT+BIEV  */
            aghfpSendOk(aghfp);
        }
    }
    else
    {
        /* Send an ERROR response to AT+BIEV  */
        aghfpSendError(aghfp);
    }
}

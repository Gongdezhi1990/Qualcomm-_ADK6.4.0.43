/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
*/

#ifndef AGHFP_HF_INDICATORS_H_
#define AGHFP_HF_INDICATORS_H_


#include "aghfp.h"

/****************************************************************************
NAME    
    aghfpHfIndicatorMaskToAssignedId

DESCRIPTION
     Convert a hf indicators mask to a Assigned ID (as defined in Assigned Number Hands Free profile)

RETURNS
    The Assigned Number ID if successful, or hf_indicators_invalid if the indicators
    mask passed in was invalid
*/
aghfp_hf_indicators_assigned_id aghfpHfIndicatorMaskToAssignedId(aghfp_hf_indicators_mask indicators_mask);


/****************************************************************************
NAME    
    hfpAssignedIdToHfindicatorMask

DESCRIPTION
    Convert a HF indicator Assigned Number to a HF Indicator mask

RETURNS
    The hf indicator mask if successful, or hfp_indicator_none if the assigned
    ID passed in was invalid
*/
aghfp_hf_indicators_mask aghfpAssignedIdToHfIndicatorMask(aghfp_hf_indicators_assigned_id assigned_id);


/****************************************************************************
NAME	
	aghfpHfIndicatorsStatusResponse

DESCRIPTION
	Sends the current status of the HF indicator as given by the client task
RETURNS
	void
*/
void aghfpHfIndicatorsStatusResponse(AGHFP *aghfp, aghfp_hf_indicators_assigned_id hf_assigned_num, aghfp_hf_indicator_state  state );


/****************************************************************************
NAME	
	aghfpHandleSendBindDetails

DESCRIPTION
	Update the AGHFP's information regarding the HF's hf indicators and send an OK AT command.
	This is a response to the AT+BIND=<assigned num1>,<assigned num2>.. command.

RETURNS
	void
*/
void aghfpHandleBindRequest(AGHFP *aghfp, AGHFP_INTERNAL_AT_BIND_REQ_T *hf_indicators );


/****************************************************************************
NAME    
    aghfpHandleSendBindDetails

DESCRIPTION
    Send +BIND:(<a>,<b>..) to the HF.

RETURNS
    void
*/
void aghfpHandleSendBindDetails(AGHFP *aghfp);


/****************************************************************************
NAME    
    aghfpHandleBievRequest

DESCRIPTION
    Handle AT+BIEV=<a>,<value> from the HF

RETURNS
    void
*/
void aghfpHandleBievRequest(AGHFP *aghfp, AGHFP_INTERNAL_AT_BIEV_STATUS_REQ_T* feature);

#endif /* AGHFP_HF_INDICATORS_H_ */

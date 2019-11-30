/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_indicators.h
    
DESCRIPTION
    Interface for the HF indicators for the HFP library.

*/

#ifndef HFP_INDICATORS_H
#define HFP_INDICATORS_H

#include <bdaddr_.h>
#include <message.h>

void sendHfIndicatorsToApp(hfp_link_data* link, uint16 hf_indicators_mask, uint16 num_hf_indicators);
void sendHfIndicatorSignalToApp(hfp_link_data* link, uint16 assigned_num, bool status);

/****************************************************************************
NAME    
    hfpHfIndicatorMaskToAssignedId

DESCRIPTION
     Convert a hf indicators mask to a Assigned ID (as defined in Assigned Number Hands Free profile)

RETURNS
    The Assigned Number ID if successful, or hf_indicators_invalid if the indicators
    mask passed in was invalid
*/
hfp_indicators_assigned_id hfpHfIndicatorMaskToAssignedId(hfp_hf_indicators_mask indicators_mask);


/****************************************************************************
NAME    
    hfpAssignedIdToHfindicatorMask

DESCRIPTION
    Convert a HF indicator Assigned Number to a HF Indicator mask

RETURNS
    The hf indicator mask if successful, or hfp_indicator_none if the assigned
    ID passed in was invalid
*/
hfp_hf_indicators_mask hfpAssignedIdToHfindicatorMask(hfp_indicators_assigned_id assigned_id);

#endif /* HFP_INDICATORS_H */



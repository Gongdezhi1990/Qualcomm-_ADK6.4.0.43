/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_hf_indicators.h
    
DESCRIPTION
    
*/

#ifndef _SINK_HF_INDICATORS_H_
#define _SINK_HF_INDICATORS_H_

#include <hfp.h>

/****************************************************************************
NAME    
    hfIndicatorTrigger
    
DESCRIPTION
    Send HF Indicator messages to hfp lib to be sent to AG if indicator is enabled

RETURNS
    void
*/
void hfIndicatorTrigger(hfp_link_priority priority, hfp_indicators_assigned_id id, uint16 value);

#ifdef TEST_HF_INDICATORS
/****************************************************************************
NAME    
    hfIndicatorNotify
    
DESCRIPTION
    Interprets the HF Indicator messages to be sent by HF to AG

RETURNS
    void
*/
void hfIndicatorNotify(hfp_indicators_assigned_id id, uint16 value);
#endif /* TEST_HF_INDICATORS */

#endif /* _SINK_HF_INDICATORS_H_ */


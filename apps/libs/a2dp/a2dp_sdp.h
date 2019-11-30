/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_sdp.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_SDP_H_
#define A2DP_SDP_H_

#include "a2dp.h"


/****************************************************************************
NAME
	a2dpHandleSdpRegisterCfm

DESCRIPTION
    Handle a CL_SDP_REGISTER_CFM message from the connection lib indicating
    the outcome of the service register request.
*/
void a2dpHandleSdpRegisterCfm(const CL_SDP_REGISTER_CFM_T *cfm);


#endif /* A2DP_SDP_H_ */

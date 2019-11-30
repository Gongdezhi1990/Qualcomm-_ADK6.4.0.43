/****************************************************************************
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    hid_sdp.h
    
DESCRIPTION
    
*/

#ifndef HID_SDP_H_
#define HID_SDP_H_

#include "hid.h"
#include "hid_private.h"

/****************************************************************************
NAME	
    hidRegisterServiceRecord

DESCRIPTION
    Register the service record corresponding to the specified profile 

RETURNS
    void
*/
void hidRegisterServiceRecord(HID_LIB *hidLib, uint8 chan);

/****************************************************************************
NAME	
    hidHandleSdpRegisterCfm

DESCRIPTION
    Outcome of SDP service register request.

RETURNS
    void
*/
void hidHandleSdpRegisterCfm(HID_LIB *hidLib, const CL_SDP_REGISTER_CFM_T *cfm);

#endif /* HID_SDP_H_ */

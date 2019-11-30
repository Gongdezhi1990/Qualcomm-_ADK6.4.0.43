/****************************************************************************
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    init.h
    
DESCRIPTION
    
*/

#ifndef HID_INIT_H_
#define HID_INIT_H_

#include "hid_private.h"

/****************************************************************************
NAME	
    sppHandleInternalInitReq

DESCRIPTION
    Send internal init req messages until we have completed the profile
    lib initialisation.

RETURNS
    void
*/
void hidHandleInternalInitReq(HID_LIB *hidLib, const HID_INTERNAL_INIT_REQ_T *req);

/****************************************************************************
NAME	
    hidSendInternalInitCfm

DESCRIPTION
    Send an internal init cfm message.

RETURNS
    void
*/
void hidSendInternalInitCfm(HID_LIB *hidLib, hid_init_status status);

/****************************************************************************
NAME	
    hidHandleInternalInitCfm

DESCRIPTION
    This message is sent once various parts of the library initialisation 
    process have completed.

RETURNS
    void
*/
void hidHandleInternalInitCfm(HID_LIB *hidLib, const HID_INTERNAL_INIT_CFM_T *cfm);


#endif /* SPP_INIT_H_ */

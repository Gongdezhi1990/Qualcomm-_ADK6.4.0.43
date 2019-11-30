/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.



FILE NAME
    avrcp_signal_vendor.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_SIGNAL_VENDOR_H_
#define AVRCP_SIGNAL_VENDOR_H_

#include "avrcp_common.h"

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/****************************************************************************
NAME    
    avrcpSendVendordependentCfmToClient

DESCRIPTION
    This function creates a AVRCP_VENDORDEPENDENT_CFM message and sends it to 
    the client task.
*/
void avrcpSendVendordependentCfmToClient(AVRCP *avrcp, 
                                        avrcp_status_code status, 
                                        uint8 response);

/****************************************************************************
NAME    
    avrcpHandleVendorResponse

DESCRIPTION
    This function internally handles a vendor-dependent response 
    received from a remote device.
*/
void avrcpHandleVendorResponse(AVRCP *avrcp, 
                         const uint8 *ptr, 
                               uint16 packet_size);


/****************************************************************************
NAME    
    avrcpHandleInternalVendorDependentReq

DESCRIPTION
    This function internally handles vendor-dependent message request.
*/
void avrcpHandleInternalVendorDependentReq(AVRCP *avrcp, 
                 const AVRCP_INTERNAL_VENDORDEPENDENT_REQ_T *req);


#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
/****************************************************************************
NAME    
    avrcpHandleInternalVendorDependentRes

DESCRIPTION
    This function internally handles vendor-dependent message result.
*/
void avrcpHandleInternalVendorDependentRes(AVRCP *avrcp, 
            const AVRCP_INTERNAL_VENDORDEPENDENT_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleVendorCommand

DESCRIPTION
    This function internally handles a vendor-dependent command received from 
    a remote device.
*/
void avrcpHandleVendorCommand(AVRCP *avrcp, 
                              const uint8 *ptr, 
                              uint16 packet_size);

#endif /* !AVRCP_CT_ONLY_LIB*/

#endif /* AVRCP_SIGNAL_VENDOR_H_ */


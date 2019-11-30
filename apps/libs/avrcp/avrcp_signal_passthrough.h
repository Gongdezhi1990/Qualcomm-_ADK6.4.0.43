/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_signal_passthrough.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_SIGNAL_PASSTHROUGH_H_
#define AVRCP_SIGNAL_PASSTHROUGH_H_

#include "avrcp_common.h"

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/* Macros */
/* Send a AVRCP_PASSTHROUGH_CFM message to the application */
#define avrcpSendPassthroughCfmToClient(avrcp, status)  \
            avrcpSendCommonCfmMessageToApp(AVRCP_PASSTHROUGH_CFM, status, \
                                           avrcp->sink, avrcp)

/****************************************************************************
NAME    
    avrcpHandleInternalPassThroughReq

DESCRIPTION
    This function internally handles a pass through message request.
*/
void avrcpHandleInternalPassThroughReq(AVRCP *avrcp, 
                            const AVRCP_INTERNAL_PASSTHROUGH_REQ_T *req);

/****************************************************************************
NAME    
    avrcpHandlePassthroughResponse

DESCRIPTION
    This function internally handles a packet pass through response
    received from a remote device.
*/
void avrcpHandlePassthroughResponse(AVRCP *avrcp, 
                                    const uint8 *ptr, 
                                    uint16 packet_size);

/****************************************************************************
NAME    
    avrcpHandleGroupResponse

DESCRIPTION
    Handle the response received from the TG.
*/
void avrcpHandleGroupResponse(AVRCP *avrcp, 
                              const uint8 *ptr, 
                              uint16 packet_size);

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

/****************************************************************************
NAME    
    avrcpHandleInternalPassThroughRes

DESCRIPTION
    This function internally handles a pass through message result.
*/
void avrcpHandleInternalPassThroughRes(AVRCP *avrcp, 
                                const AVRCP_INTERNAL_PASSTHROUGH_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandlePassthroughCommand

DESCRIPTION
    This function internally handles a pass through command
    received from a remote device.
*/
void avrcpHandlePassthroughCommand(AVRCP *avrcp, 
                                   const uint8 *ptr, 
                                   uint16 packet_size);



/****************************************************************************
NAME    
    avrcpSendGroupIndToClient

DESCRIPTION
    Send indication of the command up to the app.
*/
void avrcpSendGroupIndToClient(AVRCP *avrcp, 
                               uint16 vendor_id);


/****************************************************************************
NAME    
    avrcpHandleInternalGroupResponse

DESCRIPTION
    Process the response to be sent to CT.
*/
void avrcpHandleInternalGroupResponse(AVRCP *avrcp, 
                        const AVRCP_INTERNAL_GROUP_RES_T *res);


#endif /* !AVRCP_CT_ONLY_LIB*/

#endif /* AVRCP_SIGNAL_PASSTHROUGH_H_ */

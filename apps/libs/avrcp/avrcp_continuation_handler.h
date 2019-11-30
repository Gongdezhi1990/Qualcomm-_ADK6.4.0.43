/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_continuation_handler.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_CONTINUATION_HANDLER_H_
#define AVRCP_CONTINUATION_HANDLER_H_

#include "avrcp_private.h"

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
/****************************************************************************
NAME    
    abortContinuation

DESCRIPTION
    Abort any stored continutation messages.
*/
void abortContinuation(AVRCP *avrcp);


/****************************************************************************
NAME    
    avrcpHandleRequestContinuingCommand

DESCRIPTION
    Handle the continuing command received from the CT.
    
*/
void avrcpHandleRequestContinuingCommand(AVRCP *avrcp, uint16 pdu);


/****************************************************************************
NAME    
    avrcpHandleAbortContinuingCommand

DESCRIPTION
    Handle the abort continuing command received from the CT.
*/
void avrcpHandleAbortContinuingCommand(AVRCP *avrcp, uint16 pdu);


/****************************************************************************
NAME    
    avrcpHandleInternalAbortContinuingResponse

DESCRIPTION
    Prepare to send abort continuing response to the CT.
*/
void avrcpHandleInternalAbortContinuingResponse(AVRCP *avrcp, 
          const AVRCP_INTERNAL_ABORT_CONTINUING_RES_T *res);


/****************************************************************************
NAME    
    avrcpStoreNextContinuationPacket

DESCRIPTION
    Store the fragmented data until the CT requests it to be sent.
*/
void avrcpStoreNextContinuationPacket(  AVRCP   *avrcp, 
                                        Source  data, 
                                        uint16  param_length, 
                                        uint16  pdu_id, 
                                        uint16  response); 


/****************************************************************************
NAME    
    avrcpHandleNextContinuationPacket

DESCRIPTION
    Prepare to send the next packet of fragmented data, that the CT 
    has requested.
*/
void avrcpHandleNextContinuationPacket(AVRCP *avrcp, 
             const AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET_T *message);

#endif /* !AVRCP_CT_ONLY_LIB*/

#endif /* AVRCP_CONTINUATION_HANDLER_H_ */

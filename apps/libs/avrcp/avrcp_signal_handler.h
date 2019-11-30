/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_signal_handler.h
    
DESCRIPTION
    This header file declares prototypes for AVCTP and AVRCP
    packet handling functions. AVCTP functions are defined in 
    avrcp_avctp.c and AVRCP signal handling functions are defined 
    in avrcp_signal_handler.c.
*/

#ifndef AVRCP_SIGNAL_HANDLER_H_
#define AVRCP_SIGNAL_HANDLER_H_

#include "avrcp_common.h"


#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */

/****************************************************************************
NAME    
    avrcpHandleInternalWatchdogTimeout

DESCRIPTION
    Called if the watchdog times out by the CT not receiving a response within
    the alloted time.
*/
void avrcpHandleInternalWatchdogTimeout(AVRCP *avrcp);

/****************************************************************************
NAME    
    avrcpHandleResponse

DESCRIPTION
    This function is called to process a response received over 
    the L2cap connection.
*/
void avrcpHandleResponse(AVRCP *avrcp);

/****************************************************************************
NAME
   avrcpAvctpSetCmdHeader

DESCRIPTION
   Add the AVCTP packet header.
*/
void avrcpAvctpSetCmdHeader(AVRCP   *avrcp, 
                            uint8   *ptr, 
                            uint8   packet_type, 
                            uint8   total_packets);

#else

/* For a TG only library this function will not be called. Define it only 
   for build purpose. */
#define avrcpAvctpSetCmdHeader(a,b,c,d)  return avrcp_no_resource

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
/****************************************************************************
*NAME    
*    avrcpSendAvcResponse    
*
*DESCRIPTION
*    This function is called to Frame the AVRCP AVC Response packet and send
*   to avctp. It sends the response only if the command is outstanding,
*   otherwise ignore the packet.
*
***************************************************************************/
void avrcpSendAvcResponse(AVRCP                *avrcp,
                          uint16                hdr_size,  
                          avrcp_response_type   response,
                          uint16                data_len,
                          Source                data);

/****************************************************************************
NAME    
    avrcpHandleCommand

DESCRIPTION
    This function is called to process command received over 
    the L2cap connection.
*/
void avrcpHandleCommand(AVRCP *avrcp);



#endif /* !AVRCP_CT_ONLY_LIB*/

/****************************************************************************
NAME
    avrcpBlockReceivedData

DESCRIPTION
    Stop handling any more received data until a response has been sent.
*/
void avrcpBlockReceivedData(AVRCP       *avrcp, 
                            avrcpPending pending_command, 
                            uint16       data);


/****************************************************************************
NAME
    avrcpUnblockReceivedData

DESCRIPTION
    Restart handling received data.
*/
void avrcpUnblockReceivedData(AVRCP *avrcp);


/****************************************************************************
NAME    
    avrcpHandleInternalSendResponseTimeout

DESCRIPTION
    Called if the TG has not sent a response in the required time.
*/
void avrcpHandleInternalSendResponseTimeout(AVRCP   *avrcp, 
       const AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT_T *res);


/****************************************************************************
NAME    
    avrcpHandleReceivedData

DESCRIPTION
    This function is called to process data received over the L2cap connection.
*/
void avrcpHandleReceivedData(AVRCP *avrcp);

/*********************** AVCTP Functions ********************/

/****************************************************************************
*NAME    
*    avrcpAvctpSendMessage    
*
*DESCRIPTION
*    This function is called to Frame the AVCTP messages (Command or Response)
*   and send it to the peer. Before calling this function, 
*   fill the AVRCP Message  Header in the sink.
*    
*RETURN
*   avrcp_status_code
*****************************************************************************/
avrcp_status_code avrcpAvctpSendMessage( AVRCP     *avrcp,
                                         uint8      cr_type,
                                         uint8     *ptr,
                                         uint16     hdr_size,
                                         uint16     data_len,
                                         Source     data);   

/****************************************************************************
*NAME    
*    avrcpAvctpReceiveMessage    
*
*DESCRIPTION
*    This function is called to process the AVCTP message received. 
*    
*RETURN
*  bool - Return TRUE if the message is ready to process, otherwise return FALSE
******************************************************************************/
bool avrcpAvctpReceiveMessage(  AVRCP          *avrcp,
                                const uint8*    ptr,
                                uint16          packet_size);

/****************************************************************************
 *NAME    
 *   avbpAvctpProcessHeader  
 *
 *DESCRIPTION
 *  Validate and process the AVCTP header data in the received packet.
 *
 * RETURN
 * bool - FALSE on Failure and TRUE on Success
 ***************************************************************************/

bool avbpAvctpProcessHeader(    AVBP*           avbp, 
                                const uint8*    ptr, 
                                uint16          packet_size);

/****************************************************************************
 *NAME    
 *   avbpAvctpFrameHeader   
 *
 *DESCRIPTION
 *   Frame the AVCTP Header for Browsing packet
 ***************************************************************************/
void avbpAvctpFrameHeader(AVBP *avbp, uint8 *ptr , bool response);



#endif /* AVRCP_SIGNAL_HANDLER_H_ */

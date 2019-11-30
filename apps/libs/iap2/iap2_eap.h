/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    iap2_eap.h

DESCRIPTION
    Interface to the iAP2 EAP functions
*/

#ifndef _IAP2_EAP_H_
#define _IAP2_EAP_H_

/* Function to get the EAP Match Action for an External Accessory protocol */
iap2_app_match_action_t iap2EAGetMatchAction(uint8 protocol_id);


/* Function to get the reverse-DNS name for an External Accessory protocol */
const char *iap2EAGetName(uint8 protocol_id);

/* Function to get the number of External Accessory protocols registered */
uint8 iap2EAGetNumProtocols(void);

/* Function to get the task to receive messages associated with a given
 * External Accessory Protocol
 */
Task iap2EAGetProtocolTask(uint8 protocol_id);

/* Function to set the External Accessory protocol associated with a given
 * EA session
 */
void iap2EASetSessionProtocol(uint16 session_id, uint8 protocol_id);

/* Function to get the External Accessory protocol associated with a given
 * EA session
 */
uint8 iap2EAGetSessionProtocol(uint16 session_id);

/* Function to set the iAP2 link associated with a given EA session */
void iap2EASetSessionLink(uint16 session_id, iap2_link *link);

#define INVALID_SESSION_ID ((uint16)(-1))

/* Function to get an EA session associated with link*/
uint16 iap2EAGetSessionIdFromLink(iap2_link *link);

#endif /* _IAP2_EAP_H_ */

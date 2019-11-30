/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_fps.h

DESCRIPTION
    Routines to handle messages sent from the GATT Running Fast Pair Server Task.
    
NOTES

*/

#ifndef _SINK_GATT_SERVER_FP_H_
#define _SINK_GATT_SERVER_FP_H_

#ifndef GATT_ENABLED
#undef GATT_FP_SERVER
#endif

/* Library headers */

#include <gatt_fast_pair_server.h>
#include <gatt_fast_pair_server_uuids.h>

#include <csrtypes.h>
#include <message.h>

/* Application files */
#include "sink_gatt_server.h"

#define ENABLE_NOTIFICATION_INDICATION (GATT_CCCD_NOTI_BITMASK|GATT_CCCD_INDI_BITMASK)

#define KBP_NOTI_IND_REQD(conex)  (((conex).client_config.key_based_pairing) & ENABLE_NOTIFICATION_INDICATION)
#define PASSKEY_NOTI_IND_REQD(conex)   (((conex).client_config.passkey) & ENABLE_NOTIFICATION_INDICATION)


#ifdef GATT_FP_SERVER
#define sinkGattFastPairServerGetSize() sizeof(GFPS)
#else
#define sinkGattFastPairServerGetSize() 0
#endif


/*******************************************************************************
NAME
    sinkBleSendFastPairNotification
    
DESCRIPTION
    Send Notifications to GATT Fast Pair Service library
    
PARAMETERS
    fastpair_id         GATT characteristic for which Notification has to be sent
    fast_pair_server    Fast pair server instance
    cid                 connection ID
    value               The payload
    
RETURNS
    void
*/
#ifdef GATT_FP_SERVER
void sinkBleSendFastPairNotification(uint16 fastpair_id, 
                            const GFPS  *fast_pair_server, uint16  cid, uint8 *value);
#else
#define sinkBleSendFastPairNotification(fastpair_id, fast_pair_server, cid, value) ((void)0)
#endif

/*******************************************************************************
NAME
    sinkGattFastPairServerInitialise
    
DESCRIPTION
    Initialize Fast Pair server task.
    
PARAMETERS
    ptr - pointer to allocated memory to store server task's Fast Pair data.
    
RETURNS
    TRUE if the Fast Pair server task was initialized, FALSE otherwise.
*/
#ifdef GATT_FP_SERVER
bool sinkGattFastPairServerInitialise(uint16 **ptr);
#else
/* This is set to TRUE if the server is not defined, so as not to block the flow
 * of execution for initialiseOptionalServerTasks in sink_gatt_init.c. If it was
 * false, that function would return prematurely denoting an error in
 * initialization instead of merely skipping an undefined service. */
#define sinkGattFastPairServerInitialise(ptr) (TRUE)
#endif


/*******************************************************************************
NAME
    sinkGattFastPairServerMsgHandler
    
DESCRIPTION
    Handle messages from the GATT Fast Pair Service library
    
PARAMETERS
    task    The task the message is delivered
    id      The ID for the GATT message
    message The message payload
    
RETURNS
    void
*/
#ifdef GATT_FP_SERVER
void sinkGattFastPairServerMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkGattFastPairServerMsgHandler(task, id, message) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattFastPairServiceEnabled
    
DESCRIPTION
    Returns whether Fast Pair service is enabled or not.
*/
bool sinkGattFastPairServiceEnabled(void);

#endif /* _SINK_GATT_SERVER_FP_H_ */


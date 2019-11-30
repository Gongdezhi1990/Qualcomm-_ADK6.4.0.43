/*
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
\file    
\ingroup sink_app
\brief   
    Handles AVRCP Browsing Channel Connection/Disconnection.
*/

#ifndef _SINK_AVRCP_BROWSING_CHANNEL_H_
#define _SINK_AVRCP_BROWSING_CHANNEL_H_

#ifdef ENABLE_AVRCP

#include <avrcp.h>

void sinkAvrcpBrowsingChannelConnectInd(const AVRCP_BROWSE_CONNECT_IND_T *msg);

void sinkAvrcpBrowsingChannelDisconnectInd(const AVRCP_BROWSE_DISCONNECT_IND_T *msg);

void sinkAvrcpBrowsingChannelDisconnectRequest(AVRCP *avrcp);

bool sinkAvrcpBrowsingChannelGetIndexFromInstance(AVRCP *avrcp, uint16 *Index);

/* initialisation */
void sinkAvrcpBrowsingChannelInit(bool all_links, uint16 link_index);

/* connection/disconnection */
void sinkAvrcpBrowsingChannelConnectRequest(AVRCP *avrcp);

void sinkAvrcpBrowsingChannelConnectCfm(const AVRCP_BROWSE_CONNECT_CFM_T *msg);

/* Utility function to check if Browsing channel is connected */
bool sinkAvrcpBrowsingChannelIsConnected(uint16 index);

/* Utility function to check if Browsing channel is disconnected */
bool sinkAvrcpBrowsingChannelIsDisconnected(uint16 index);

/* Utility function to send the message on connection */
void sinkAvrcpBrowsingChannelSendMessageWhenConnected(Task task, MessageId id, void* message, uint16 index);

#endif /* ENABLE_AVRCP */

#endif /* _SINK_AVRCP_BROWSING_CHANNEL_H_ */


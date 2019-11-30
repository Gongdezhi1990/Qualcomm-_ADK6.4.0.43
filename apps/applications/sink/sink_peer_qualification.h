/*
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file    sink_peer_qualification.h
@ingroup sink_app
@brief   Interface to handle Peer State machine to ensure PTS qualification for 
            A2DP Sorce functionality. 
*/

#ifndef _SINK_PEER_QUALIFICATION_H_
#define _SINK_PEER_QUALIFICATION_H_

#include "sink_peer.h"

#include <message.h>
#include <app/message/system_message.h>
#include <a2dp.h>

#ifdef ENABLE_PEER
/******************************************************/
bool peerQualificationAdvanceRelayState (RelayEvent relay_event);
#else
#define peerQualificationAdvanceRelayState (relay_event) (FALSE)
#endif

#ifdef ENABLE_PEER
void handlePeerQualificationEnablePeerOpen(void);
#else
#define handlePeerQualificationEnablePeerOpen() ((void)0)
#endif

#ifdef ENABLE_PEER
bool a2dpIssuePeerReconfigureRequest (void);
#else
#define a2dpIssuePeerReconfigureRequest() (TRUE)
#endif

bool peerQualificationReplaceDelayReportServiceCaps (uint8 *dest_service_caps, uint16 *size_dest_service_caps, const uint8 *src_service_caps, uint16 size_src_service_caps);

void handlePeerQualificationReconfigureCfm(uint8 DeviceId, uint8 StreamId, a2dp_status_code status);

bool a2dpIssueSinkReconfigureRequest(void);

bool a2dpGetConnectedDeviceIndex (uint16* index);

void a2dpIssueSuspendRequest (void);

#endif /* _SINK_PEER_QUALIFICATION_H_ */


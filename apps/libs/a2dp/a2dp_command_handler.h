/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_command_handler.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_COMMAND_HANDLER_H_
#define A2DP_COMMAND_HANDLER_H_

#include "a2dp.h"
#include "a2dp_private.h"

void a2dpSetStreamState(remote_device *device, avdtp_stream_state state);
void a2dpHandleInternalClientRspTimeout(MessageId msg_id);
void a2dpHandleInternalRemoteCmdTimeout(MessageId msg_id);
void a2dpHandleInternalWatchdogTimeout(MessageId msg_id);

void a2dpHandleDiscoverResponse (remote_device *device);
void a2dpHandleGetCapabilitiesResponse (remote_device *device);
void a2dpHandleCodecConfigureResponse(const A2DP_INTERNAL_CODEC_CONFIGURE_RSP_T *rsp);
void a2dpHandleSetConfigurationResponse (remote_device *device);
void a2dpHandleGetConfigurationResponse (remote_device *device);
void a2dpHandleReconfigureResponse (remote_device *device);
void a2dpHandleOpenResponse (remote_device *device);
void a2dpHandleCloseResponse (remote_device *device);
void a2dpHandleStartResponse (remote_device *device);
void a2dpHandleSuspendResponse (remote_device *device);
void a2dpHandleAbortResponse (remote_device *device);
void a2dpHandleGetAllCapabilitiesResponse (remote_device *device);
void a2dpHandleDelayReportResponse (remote_device *device);

void a2dpHandleDiscoverCommand (remote_device *device);
void a2dpHandleGetCapabilitiesCommand (remote_device *device);
void a2dpHandleSetConfigurationCommand (remote_device *device);
void a2dpHandleGetConfigurationCommand (remote_device *device);
void a2dpHandleReconfigureCommand (remote_device *device);
void a2dpHandleOpenCommand (remote_device *device);
bool a2dpHandleCloseCommand (remote_device *device);
bool a2dpHandleStartCommand (remote_device *device);
void a2dpHandleSuspendCommand (remote_device *device);
void a2dpHandleAbortCommand (remote_device *device);
void a2dpHandleGetAllCapabilitiesCommand (remote_device *device);
void a2dpHandleDelayReportCommand (remote_device *device);

bool a2dpStreamClose (remote_device *device);
void a2dpStreamEstablish (const A2DP_INTERNAL_MEDIA_OPEN_REQ_T *req);
void a2dpStreamOpenResponse (const A2DP_INTERNAL_MEDIA_OPEN_RES_T *res);
void a2dpStreamStart (const A2DP_INTERNAL_MEDIA_START_REQ_T *req);
void a2dpStreamStartResponse (const A2DP_INTERNAL_MEDIA_START_RES_T *res);
void a2dpStreamRelease (const A2DP_INTERNAL_MEDIA_CLOSE_REQ_T *req);
void a2dpStreamSuspend (const A2DP_INTERNAL_MEDIA_SUSPEND_REQ_T *req);
void a2dpStreamReconfigure (const A2DP_INTERNAL_MEDIA_RECONFIGURE_REQ_T *req);
void a2dpStreamDelayReport (remote_device *device, uint16 delay);
void a2dpStreamAbort (remote_device *device);
void a2dpStreamReset (remote_device *device);

#endif /* A2DP_COMMAND_HANDLER_H_ */

/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_process_command.h
    
DESCRIPTION
    
	
*/

#ifndef A2DP_PROCESS_COMMAND_H_
#define A2DP_PROCESS_COMMAND_H_

#include "a2dp.h"
#include "a2dp_private.h"

uint8 a2dpGetLocalSeid (remote_device *device);
bool a2dpSetSepAvailable (remote_device *device, uint8 seid, bool available);
a2dp_sep_status a2dpGetSepAvailability (remote_device *device, uint8 seid);
uint16 a2dpProcessDiscoverCommand (remote_device *device, uint16 *payload_size);
uint16 a2dpProcessGetCapabilitiesCommand (remote_device *device, uint16 *payload_size);
uint16 a2dpProcessSetConfigurationCommand (remote_device *device);
uint16 a2dpProcessGetConfigurationCommand (remote_device *device, uint16 *payload_size);
uint16 a2dpProcessReconfigureCommand (remote_device *device);
uint16 a2dpProcessGetAllCapabilitiesCommand (remote_device *device, uint16 *payload_size);
uint16 a2dpProcessDelayReportCommand (remote_device *device);
uint16 a2dpProcessOpenCommand (remote_device *device);
uint16 a2dpProcessStartCommand (remote_device *device);
uint16 a2dpProcessCloseCommand(remote_device *device);
uint16 a2dpProcessSuspendCommand (remote_device *device);
bool a2dpProcessAbortCommand(remote_device *device);
bool a2dpProcessDiscoverResponse(remote_device *device);
bool a2dpProcessGetCapabilitiesResponse(remote_device *device);
uint16 a2dpSelectConfigurationParameters(remote_device *device);
bool a2dpProcessCodecConfigureResponse(remote_device *device, uint8 local_seid, const uint8 *codec_caps, uint16 size_codec_caps);
void a2dpProcessReconfigureResponse(remote_device *device);
bool a2dpProcessGetAllCapabilitiesResponse(remote_device *device);

#endif /* A2DP_PROCESS_COMMAND_H_ */

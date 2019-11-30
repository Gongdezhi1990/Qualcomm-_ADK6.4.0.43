/*
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.
*/
/**
\file
\ingroup sink_app
*/

#ifndef _SINK_AUDIO_ROUTING_H_
#define _SINK_AUDIO_ROUTING_H_

#include <sink.h>
#include <hfp.h>
#include <audio_plugin_if.h>
#include <audio_instance.h>

#include "sink_audio_routing_config_def.h"

/* Macros related to the User PEQ band parameters*/
#define NUM_USER_EQ_BANDS 5
#define USER_EQ_BAND_PARAMS_SIZE 7       /* bytes*/
#define USER_EQ_PARAM_PRE_GAIN_SIZE 2 /* bytes*/

typedef enum
{
    audio_source_none = 0,
    audio_source_FM,
    audio_source_ANALOG,
    audio_source_SPDIF,
    audio_source_I2S,
    audio_source_USB,
    audio_source_a2dp_1,
    audio_source_a2dp_2,
    audio_source_ba,
    audio_source_end_of_list
}audio_sources;

typedef enum
{
    audio_gate_none       = 0x00,
    audio_gate_call       = 0x01,
    audio_gate_ba         = 0x02,
    audio_gate_a2dp       = 0x04,
    audio_gate_usb        = 0x08,
    audio_gate_wired      = 0x10,
    audio_gate_fm         = 0x20,
    audio_gate_multimedia = 0x3E,   /* All but audio_gate_call, audio_gate_sco and audio_gate_relay */
    audio_gate_sco        = 0x40,
    audio_gate_relay      = 0x80,
    audio_gate_noncall    = 0x8E,   /* All but audio_gate_call */
    audio_gate_all        = 0xFF
}audio_gating;

/* intended as a clean mechanism to determine what audio is routed */
typedef enum
{
    audio_route_none                = 0x00,
    audio_route_a2dp_primary        = 0x01,
    audio_route_a2dp_secondary      = 0x02,
    audio_route_hfp_primary         = 0x04,
    audio_route_hfp_secondary       = 0x08,
    audio_route_usb                 = 0x10,
    audio_route_analog              = 0x20,
    audio_route_spdif               = 0x40,
    audio_route_fm                  = 0x80,
    audio_route_i2s                 = 0x100,
    audio_route_ba                  = 0x200
} audio_route_available;

typedef enum
{
    disabled,
    microphone_1,
    microphone_2,
    microphone_3,
    microphone_4,
    microphone_5,
    microphone_6
} mic_selection;

audio_gating audioGateAudio (uint16 audio_gated_mask);
audio_gating audioUngateAudio (uint16 audio_ungated_mask);

typedef struct
{
    Task audio_plugin;
    Sink audio_sink;
    AUDIO_SINK_T sink_type;
    int16 volume;
    uint32 rate;
    AudioPluginFeatures features;
    AUDIO_MODE_T mode;
    AUDIO_ROUTE_T route;
    AUDIO_POWER_T power;
    void * params;
    Task app_task;
} audio_connect_parameters;

/****************************************************************************
NAME    
    audioSwitchToAudioSource
    
DESCRIPTION
	Switch audio routing to the source passed in, it may not be possible
    to actually route the audio at that point until the audio sink becomes
    available. 
    
RETURNS
    None
*/
void audioSwitchToAudioSource(audio_sources source);

/****************************************************************************
NAME
    audioUpdateAudioRouting

DESCRIPTION
	Handle the routing of audio sources based on current status and other
	priority features like Speech Recognition, TWS and others.

RETURNS
    None
*/
void audioUpdateAudioRouting(void);

/****************************************************************************
NAME    
    audioUpdateAudioRoutingAfterDisconnect

DESCRIPTION
    Handle the routing of audio sources based on current status and other
    priority features after certain delay (in ms).  The delay is configurable
    by the user through sink configurator tool

RETURNS
    None
*/
void audioUpdateAudioRoutingAfterDisconnect(void);

/****************************************************************************
NAME    
    audioDisconnectRoutedAudio
DESCRIPTION
    Disconnect the currently routed audio

RETURNS
    
*/
void audioDisconnectRoutedAudio(void);

/****************************************************************************
NAME
    audioDisconnectRoutedVoice

DESCRIPTION
    Disconnect the currently routed voice

RETURNS

*/
void audioDisconnectRoutedVoice(void);

/****************************************************************************
NAME    
    sinkAudioRouteAvailable
    
DESCRIPTION
    returns which audio source is routed. Only the route of highest priority is 
    returned. The priority starting at the top of the enum audio_route_available
    
RETURNS
    audio_route_available
*/
audio_route_available sinkAudioRouteAvailable(void);

/****************************************************************************
NAME    
    audioSuspendDisconnectAllA2dpMedia
    
DESCRIPTION
    called when the SUB link wants to use an ESCO link, there is not enough
    link bandwidth to support a2dp media and esco links so suspend or disconnect
    all a2dp media streams
    
RETURNS
    true if audio disconnected, false if no action taken
*/
bool audioSuspendDisconnectAllA2dpMedia(void);

/****************************************************************************
NAME    
    audioA2dpStartStream
    
DESCRIPTION
    Start the A2DP Audio stream request from the application.
    
RETURNS
    None
*/
void audioA2dpStartStream(void);
/*************************************************************************
NAME
    processEventUsrSelectAudioSource

DESCRIPTION
    Function to handle the user's source selection. Follow up calls and
    configuration settings status will determine the outcome and proceed
    by indicating the event or not.

INPUTS
    EventUsrSelectAudioSource Source selection user events

RETURNS
	TRUE if routing was successful and VM isn't in deviceLimbo state.
	FALSE if routing was not possible or VM is in deviceLimbo state.

*/
bool processEventUsrSelectAudioSource(const MessageId EventUsrSelectAudioSource);

/*************************************************************************
NAME
    processEventUsrWiredAudioConnected

DESCRIPTION
    Function to handle the wired audio connection upon user events

INPUTS
    Source selection user events (i.e. EventUsrAnalogAudioConnected).

RETURNS
      void
*/
void processEventUsrWiredAudioConnected(const MessageId id);

/*************************************************************************
NAME
    processEventWiredAudioDisconnected

DESCRIPTION
    Function to handle the wired audio disconnection upon events

INPUTS
    Event ID (e.g. EventUsrAnalogAudioDisconnected).

RETURNS
    void

*/
void processEventWiredAudioDisconnected(const MessageId id);

/*************************************************************************
NAME
	isAudioGated

DESCRIPTION


INPUTS
	audio_gated_mask

RETURNS
	bool

*/
bool isAudioGated (audio_gating audio_gated_mask);

/****************************************************************************
NAME    
    audioSuspendDisconnectAudioSource
    
DESCRIPTION
    determines source of sink passed in and decides whether to issue a suspend or
    not based on source type, an audio disconnect is performed thereafter regardless
    of wether or not the source requires a suspend
    
RETURNS
    true if audio disconnected, false if no action taken
*/
void audioSuspendDisconnectAudioSource(void);



/****************************************************************************
NAME 
      sinkAudioInit

DESCRIPTION
    Reads the PSKEY containing the audio routing information
 
RETURNS
      void
*/ 
void sinkAudioInit( void );

/****************************************************************************
NAME
    sinkAudioGetPluginFeatures

DESCRIPTION
    Returns the audio plugin features.

RETURNS
    Plugin features
*/
AudioPluginFeatures sinkAudioGetPluginFeatures(void);

/****************************************************************************
NAME
    sinkAudioIsManualSrcSelectionEnabled

DESCRIPTION
    Returns the status of manual source selection feature bit.

RETURNS
    TRUE If manual source selection is enabled, FALSE otherwise
*/
bool sinkAudioIsManualSrcSelectionEnabled(void);

/****************************************************************************
NAME
    sinkAudioGetAudioInputRoute

DESCRIPTION
    Audio input interface to use as source.

RETURNS
    Input audio source
*/
AUDIO_ROUTE_T sinkAudioGetAudioInputRoute(void);

/****************************************************************************
NAME
    sinkAudioGetSilenceThreshold

DESCRIPTION
    Returns the configured silence detection threshold

RETURNS
    threshold
*/
uint16 sinkAudioGetSilenceThreshold(void);

/****************************************************************************
NAME
    sinkAudioGetSilenceTriggerTime

DESCRIPTION
    Returns the silence detection trigger time

RETURNS
    trigger time in seconds (16 bit)
*/
uint16 sinkAudioGetSilenceTriggerTime(void);

/****************************************************************************
NAME
    sinkAudioAmpPowerUpTime

DESCRIPTION
    Return the configured time to power up amp before unmuting (milliseconds)

RETURNS
    Amp power up time before unmute in seconds (16 bit)
*/
uint16 sinkAudioAmpPowerUpTime(void);

/****************************************************************************
NAME
    sinkAudioSetAmpPowerUpTime

DESCRIPTION
    Set the configured time to power up amp before unmuting (milliseconds)

PARAMS
    time  Amp power up time before unmute in seconds (16 bit)
*/
bool sinkAudioSetAmpPowerUpTime(uint16 time);

/****************************************************************************
NAME
    sinkAudioHfpToA2DPSwitchingTime

DESCRIPTION
    Return the switching delay between HFP call disconnection to A2DP streaming (in milli seconds)

RETURNS
    Switching delay between hfp to a2dp (16 bit)
uint16 sinkAudioHfpToA2DPSwitchingTime(void);
*/

/****************************************************************************
NAME
    sinkAudioSourceDisconnectDelay

DESCRIPTION
    Return the audio routing delay after source disconnection

RETURNS
    Audio routing delay after input source disconnection
*/
uint16 sinkAudioSourceDisconnectDelay(void);

/****************************************************************************
NAME
    sinkAudioActiveEnableDelay

DESCRIPTION
    Return the audio active enable delay in milliseconds after audio is routed.

RETURNS
    Audio active enable delay after audio is routed.
*/
uint16 sinkTwsAudioActiveDelay(void);

/****************************************************************************
NAME
    sinkAudioAmpPowerDownTime

DESCRIPTION
    Return the configured time to mute before powering down amp(milliseconds)

RETURNS
    Time to mute amp before power down (16 bit)
*/
uint16 sinkAudioAmpPowerDownTime(void);

/****************************************************************************
NAME
    sinkAudioSetAmpPowerDownTime

DESCRIPTION
    Set the configured time to mute before powering down amp(milliseconds)

PARAMS
    time  Time to mute amp before power down (16 bit)
*/
bool sinkAudioSetAmpPowerDownTime(uint16 time);

/****************************************************************************
NAME
    sinkAudioGetMic1Params

DESCRIPTION
    Return the configured mic 1 settings

RETURNS
    Mic 1 settings
*/
audio_mic_params sinkAudioGetMic1Params(void);

/****************************************************************************
NAME
    sinkAudioGetMic2Params

DESCRIPTION
    Return the configured mic 2 settings

RETURNS
    Mic 2 settings
*/
audio_mic_params sinkAudioGetMic2Params(void);

/****************************************************************************
NAME
    sinkAudioGetMic3Params

DESCRIPTION
    Return the configured mic 3 settings

RETURNS
    Mic 3 settings
*/
audio_mic_params sinkAudioGetMic3Params(void);

/****************************************************************************
NAME
    sinkAudioGetMic4Params

DESCRIPTION
    Return the configured mic 4 settings

RETURNS
    Mic 4 settings
*/
audio_mic_params sinkAudioGetMic4Params(void);

/****************************************************************************
NAME
    sinkAudioGetMic5Params

DESCRIPTION
    Return the configured mic 5 settings

RETURNS
    Mic 5 settings
*/
audio_mic_params sinkAudioGetMic5Params(void);

/****************************************************************************
NAME
    sinkAudioGetMic6Params

DESCRIPTION
    Return the configured mic 6 settings

RETURNS
    Mic 6 settings
*/
audio_mic_params sinkAudioGetMic6Params(void);

/****************************************************************************
NAME
    sinkAudioMicAPreAmpEnabled

DESCRIPTION
    Determines if pre amp is enabled or mic a (analogue only)

RETURNS
    TRUE if mic a pre amp is enabled, FALSE otherwise
*/
bool sinkAudioMicAPreAmpEnabled(void);

/****************************************************************************
NAME
    sinkAudioMicBPreAmpEnabled

DESCRIPTION
    Determines if pre amp is enabled or mic b (analogue only)

RETURNS
    TRUE if mic b pre amp is enabled, FALSE otherwise
*/
bool sinkAudioMicBPreAmpEnabled(void);

/****************************************************************************
NAME
    sinkAudioGetTwsMasterAudioRouting

DESCRIPTION
    Returns the TWS master audio routing mode.

RETURNS
    TWS master audio routing mode
*/
uint8 sinkAudioGetTwsMasterAudioRouting(void);

/****************************************************************************
NAME
    sinkAudioGetTwsSlaveAudioRouting

DESCRIPTION
    Returns the TWS slave audio routing mode.

RETURNS
    TWS slave audio routing mode
*/
uint8 sinkAudioGetTwsSlaveAudioRouting(void);

/****************************************************************************
NAME
    sinkAudioGetAudioActivePio

DESCRIPTION
    Returns the configured PIO for Audio Active.

RETURNS
    Audio active pio
*/
uint8 sinkAudioGetAudioActivePio(void);

/****************************************************************************
NAME
    sinkAudioGetAudioActivePio

DESCRIPTION
    Returns the configured PIO for Aux out detect.

RETURNS
    Aux out detect pio
*/
uint8 sinkAudioGetAuxOutDetectPio(void);

/****************************************************************************
NAME
    sinkAudioGetPowerOnPio

DESCRIPTION
    Returns the configured PIO for Power on.

RETURNS
    Power on pio
*/
uint8 sinkAudioGetPowerOnPio(void);

/****************************************************************************
NAME
    sinkAudioGetAudioMutePio

DESCRIPTION
    Returns the configured PIO for Audio mute.

RETURNS
    Audio mute pio
*/
uint8 sinkAudioGetAudioMutePio(void);

/****************************************************************************
NAME
    sinkAudioGetDefaultAudioSource

DESCRIPTION
    Returns the default audio source set in configuration

RETURNS
    default source
*/
uint8 sinkAudioGetDefaultAudioSource(void);


/****************************************************************************
NAME
    sinkAudioRoutingGetSessionData

DESCRIPTION
    Sets Audio related data using stored session data values.

RETURNS
    None
    
**************************************************************************/
void sinkAudioRoutingGetSessionData(void);

/****************************************************************************
NAME
    sinkAudioRoutingSetSessionData

DESCRIPTION
    Sets Audio related session data using using current data values.
*/
void sinkAudioRoutingSetSessionData(void);

/****************************************************************************
NAME
    sinkAudioGetRequestedAudioSource

DESCRIPTION
    Get the requested Audio source.

RETURNS
    Requested audio source
*/
audio_sources sinkAudioGetRequestedAudioSource(void);

/****************************************************************************
NAME
    sinkAudioGetRoutedAudioSource

DESCRIPTION
    Get the currently routed Audio source.

RETURNS
    routed audio source
*/
audio_sources sinkAudioGetRoutedAudioSource(void);

/****************************************************************************
NAME
    sinkAudioSetRoutedAudioSource

DESCRIPTION
    Updates the currently routed Audio source.

RETURNS
    None
*/
void sinkAudioSetRoutedAudioSource(audio_sources source);

/****************************************************************************
NAME	
	sinkAudioGetPEQ
    
DESCRIPTION
    Returns the user eq bank settings
    
RETURNS 
    user_eq_bank_t
*/
user_eq_bank_t *sinkAudioGetPEQ(void);

/****************************************************************************
NAME	
	sinkAudioPeerGetPEQ
    
DESCRIPTION
    Return the PEQ settings to be sent to currently connected Peer.
    
RETURNS 
    None
*/
bool sinkAudioPeerGetPEQ(uint8 *user_eq_params);

/****************************************************************************
NAME	
	sinkAudioPeerUpdatePEQ
    
DESCRIPTION
    Update the PEQ settings as received from the connected Peer.
    
RETURNS 
    None
*/
void sinkAudioPeerUpdatePEQ(uint8* peer_updated_eq);

/****************************************************************************
NAME	
    sinkAudioIsAudioRouted
    
DESCRIPTION
    Check to determine whether audio is currently routed.
    
RETURNS 
    TRUE if audio is routed, else FALSE
*/
bool sinkAudioIsAudioRouted(void);

/****************************************************************************
NAME	
	sinkAudioGetRoutedAudioSink
    
DESCRIPTION
    Returns the currently routed audio sink.
    
RETURNS 
    Currently routed sink
*/
Sink sinkAudioGetRoutedAudioSink(void);

/****************************************************************************
NAME	
	sinkAudioGetRoutedAudioTask
    
DESCRIPTION
    Returns the currently routed audio task.
    
RETURNS 
    Currently routed task
*/
Task sinkAudioGetRoutedAudioTask(void);

/****************************************************************************
NAME	
    sinkAudioIsVoiceRouted
    
DESCRIPTION
    Check to determine whether voice is currently routed.
    
RETURNS 
    TRUE if voice is routed, else FALSE
*/
bool sinkAudioIsVoiceRouted(void);

/****************************************************************************
NAME	
	sinkAudioGetRoutedVoiceSink
    
DESCRIPTION
    Returns the currently routed voice sink.
    
RETURNS 
    Currently routed sink
*/
Sink sinkAudioGetRoutedVoiceSink(void);

/****************************************************************************
NAME	
	sinkAudioGetRoutedVoiceTask
    
DESCRIPTION
    Returns the currently routed voice task.
    
RETURNS 
    Currently routed task
*/
Task sinkAudioGetRoutedVoiceTask(void);

/****************************************************************************
NAME	
	sinkAudioReSetGatedAudio
    
DESCRIPTION
    Resets the gated audio flag.
    
RETURNS 
    None
*/
void sinkAudioReSetGatedAudio(void);

/****************************************************************************
NAME	
	sinkAudioIsAmpReady
    
DESCRIPTION
    Returns the status of amplifier ready flag.
    
RETURNS 
    TRUE: if audio amplifier is ready, FALSE otherwise
*/
bool sinkAudioIsAmpReady(void);

/****************************************************************************
NAME	
	sinkAudioSetAmpReady
    
DESCRIPTION
    Updates the status of amplifier ready flag.
    
RETURNS 
    None
*/
void sinkAudioSetAmpReady(bool amp_ready);

/****************************************************************************
NAME	
	sinkAudioIsSilenceDetected
    
DESCRIPTION
    Returns the status of Silence detection flag.
    
RETURNS 
    TRUE: If silence detected, FALSE otherwise
*/
bool sinkAudioIsSilenceDetected(void);

/****************************************************************************
NAME	
	sinkAudioSetSilence
    
DESCRIPTION
    Updates the status of silence detection flag.
    
RETURNS 
    None
*/
void sinkAudioSetSilence(bool silence);

/****************************************************************************
NAME	
	sinkAudioGetCommonMicParams
    
DESCRIPTION
    Retrieve the common mic params
    
RETURNS 
    None
*/
void sinkAudioGetCommonMicParams(const voice_mic_params_t **mic_params);

/****************************************************************************
NAME
    sinkAudioGetCommonAnalogueInputParams

DESCRIPTION
    Retrieve the analogue input is params

RETURNS
    None
*/
void sinkAudioGetCommonAnalogueInputParams(const analogue_input_params ** analogue_params);

/****************************************************************************
NAME
    sinkAudioLowPowerCodecEnabled

DESCRIPTION
    Returns the status of low power audio codec use  feature bit

RETURNS
    TRUE if Low power audio codec  use is enabled, FALSE otherwise
*/
bool sinkAudioLowPowerCodecEnabled(void);

void audioRouteSpecificA2dpSource(audio_sources a2dp_source);

/****************************************************************************
NAME
    audioRouteIsMixingOfVoiceAndAudioEnabled

DESCRIPTION
    Returns the status of HFP/Wired mixing

RETURNS
    TRUE if mixing is enabled, FALSE otherwise
*/
bool audioRouteIsMixingOfVoiceAndAudioEnabled(void);

/****************************************************************************
NAME
    sinkAudioSetRoutedAudioInstance

DESCRIPTION
    Sets the current routed audio instance

RETURNS
    None
*/
void sinkAudioSetRoutedAudioInstance(audio_instance_t instance);

/****************************************************************************
NAME
    audioRoutingProcessUpdateMsg

DESCRIPTION
    Single call which looks after the input audio source routing, triggered by
    a queued event.

RETURNS
    None.
*/
void audioRoutingProcessUpdateMsg(void);

/****************************************************************************
NAME
    audioUpdateAudioActivePio

DESCRIPTION
    Single call which looks after updating the audio active Pio considering the 
    current audio routing and whether the audio is currently busy.

RETURNS
    None.
*/
void audioUpdateAudioActivePio(void);

/****************************************************************************
NAME
    enableAudioActivePio

DESCRIPTION
    Enable audio using the audio active Pio.

RETURNS
    None.
*/
void enableAudioActivePio(void);


/****************************************************************************
NAME
    disableAudioActivePio

DESCRIPTION
    Disable audio using the audio active Pio.

RETURNS
    None.
*/
void disableAudioActivePio(void);

/****************************************************************************
NAME
    disableAudioActivePioWhenAudioNotBusy

DESCRIPTION
    Disable audio using the audio amplifier pio when audio is not busy.

RETURNS
    None.
*/
void disableAudioActivePioWhenAudioNotBusy(void);

/****************************************************************************
NAME
    audioRouteIsScoActive

DESCRIPTION
    Checks for any sco being present

RETURNS
    true if sco active, false if no sco available
*/
bool audioRouteIsScoActive(void);



#endif /* _SINK_AUDIO_ROUTING_H_ */


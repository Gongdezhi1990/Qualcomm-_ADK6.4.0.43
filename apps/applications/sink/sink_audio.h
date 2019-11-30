/*
Copyright (c) 2004 - 2018 Qualcomm Technologies International, Ltd.
*/
/**
\file
\ingroup sink_app

*/

#ifndef _SINK_AUDIO_H_
#define _SINK_AUDIO_H_

#include "sink_hfp_data.h"
#include "sink_a2dp.h"
#include "sink_audio_routing.h"

/* Linked-list structure for registering user implemented WBS codecs. */
typedef struct _audio_codec_plugin_info
{
    uint32      					bandwidth;			/* Codec audio paramters - bandwidth */
    uint16      					max_latency;		/* Codec audio paramters - max latency */
    uint16     	 					voice_settings;		/* Codec audio paramters - vioce settings */
    uint16      					retx_effort;		/* Codec audio paramters - re-transmission effort */
    uint16							packet_types;		/* Codec's supported packet types */
} audio_codec_plugin_info;

/* interval at which to check whether the audio amplifier can be turned off after playing a
   tone or a voice prompt, amplifier is turned off instantly when using the dsp to route
   call or a2dp audio */
#define CHECK_AUDIO_AMP_PIO_DRIVE_DELAY 2000

#ifdef DAC_ADC_LINK
/* ADC and DAC linked - force stereo if plugin is 2 mic */
#define AUDIO_PLUGIN_FORCE_STEREO (sinkHfpDataGetAudioPlugin() == 3 || sinkHfpDataGetAudioPlugin() == 4)
#else
/* ADC and DAC independant - no need to force stereo */
#define AUDIO_PLUGIN_FORCE_STEREO (FALSE)
#endif

#define AUDIO_LATENCY_NORMAL_MS (150)
#define AUDIO_LATENCY_SHORT_MS (45)
#define AUDIO_LATENCY_LONG_MS (250)

/* Note: targent latency values in ms are calculated based on following macro as per FR47787 and FR47788
    and average values are considered for now.
    ((audio_out_buf_size + TWS_delay_buf_size) / codec_sample_rate) + ((resampler_buf_size / output_rate) * resampler_enabled) 
    +((rate_adj_buf_size + multi_channel_buf_size) / output_rate) + ((codec_in_buf_size * 16 * .70) / codec_max_bitrate)
*/
#define AUDIO_LATENCY_TWS_NORMAL_MS (550)

#if defined ENABLE_PEER && defined PEER_TWS
#define AUDIO_LATENCY_AAC_MS (550)
#define AUDIO_LATENCY_APTX_MS (620)
#define AUDIO_LATENCY_SBC_MS (635)
#define AUDIO_LATENCY_MP3_MS (580)
#else
#define AUDIO_LATENCY_AAC_MS (210)
#define AUDIO_LATENCY_APTX_MS (300)
#define AUDIO_LATENCY_SBC_MS (280)
#define AUDIO_LATENCY_MP3_MS (240)
#endif

/* TWS Master will send time-to-play values to the TWS Slave and it will play audio according to these time stamps,
     hence target latency on slave is set to zero */
#define AUDIO_LATENCY_TWS_NULL (0)

typedef struct
{
    bdaddr bd_addr;
}UpdateAvrpcMessage_t;

/****************************************************************************
NAME
    audioPriorLatency

DESCRIPTION
    Return initial latency estimate for given endpoint codec

RETURNS
    Predicted latency in milliseconds
*/
uint16 audioPriorLatency(uint8 seid);

/****************************************************************************
NAME    
    getScoPriorityFromHfpPriority
    
DESCRIPTION
    obtain the current sco priority level of the AG priority passed in

RETURNS
    current sco priority level, if any, may not have a sco
*/   
audio_priority getScoPriorityFromHfpPriority(hfp_link_priority priority);

/****************************************************************************
NAME    
    setScoPriorityFromHfpPriority
    
DESCRIPTION
    sets the current sco priority level of the AG priority passed in

RETURNS
    nothing
*/   
void setScoPriorityFromHfpPriority(hfp_link_priority priority, audio_priority level);

/****************************************************************************
NAME    
    audioGetLinkPriority
    
DESCRIPTION
	Common method of getting the link we want to manipulate audio settings on

RETURNS
    
*/
hfp_link_priority audioGetLinkPriority ( bool audio );



/****************************************************************************
NAME    
    audioSwapMediaChannel
    
DESCRIPTION
	attempt to swap between media channels if two channels exist and one of them
    is currentyl being routed to the speaker.
RETURNS
    successful or not status
*/
bool audioSwapMediaChannel(void);

/****************************************************************************
NAME    
    audioHandleSyncConnectInd
    
DESCRIPTION

RETURNS
    
*/
void audioHandleSyncConnectInd ( const HFP_AUDIO_CONNECT_IND_T *pInd );

/****************************************************************************
NAME    
    audioHandleSyncConnectInd
    
DESCRIPTION

RETURNS
    
*/
void audioHandleSyncConnectCfm ( const HFP_AUDIO_CONNECT_CFM_T * pCfm );

/****************************************************************************
NAME    
    audioHandleSyncConnectInd
    
DESCRIPTION

RETURNS
    
*/
void audioHandleSyncDisconnectInd ( const HFP_AUDIO_DISCONNECT_IND_T * pInd ) ;

/****************************************************************************
DESCRIPTION
    toggles a custom audio processing feature
*/
void audioToggleFeature1 ( void ) ;

/****************************************************************************
DESCRIPTION
    toggles a custom audio processing feature
*/
void audioToggleFeature2 ( void ) ;

bool audioHfpPopulateConnectParameters(Sink sink, audio_connect_parameters *connect_parameters);

/****************************************************************************
NAME    
    audioIndicateCodec
    
DESCRIPTION
    Sends an event to indicate which A2DP codec is in use

RETURNS
    void
*/
void audioIndicateCodec(uint8 seid);

void A2dpPostConnectConfiguration(a2dp_index_t Index);

bool A2dpPopulateConnectParameters(a2dp_index_t Index, audio_connect_parameters *connect_parameters);

/****************************************************************************
NAME    
    audioHandleMicSwitch
    
DESCRIPTION
	Handle AT+MICTEST AT command from TestAg. 
    This command swaps between the two microphones to test 2nd mic in production.

RETURNS
    returns true if mic switch command issued to plugin
*/
bool audioHandleMicSwitch( void );

bool audioHandleOutputSwitch(void);

/****************************************************************************
NAME    
    sinkTransferToggle
    
DESCRIPTION
    If the audio is at the sink end transfer it back to the AG and
    vice versa.

RETURNS
    FALSE if an audio transfer to the AG has been initiated.
    TRUE otherwise.
*/
void sinkTransferToggle( void ) ;

/****************************************************************************
NAME    
    sinkCheckForAudioTransfer
    
DESCRIPTION
    checks on connection for an audio connction and performs a transfer if not present

RETURNS
    void
*/
void sinkCheckForAudioTransfer ( hfp_link_priority priority ) ;

/****************************************************************************
NAME    
    audioControlLowPowerCodecs
    
DESCRIPTION
	Enables / disables the low power codecs. 
    will only use low power codecs is the flag UseLowPowerCodecs is set and 
    enable == TRUE
    
    This fn is called whenever an audioConnect occurs in order to select the 
    right codec ouptut stage power 

RETURNS
    none
*/
#define audioControlLowPowerCodecs(enable) ((void)enable)

/****************************************************************************
NAME    
    sinkAudioSetEnhancement
DESCRIPTION
    updates the current audio enhancement settings and updates the dsp if it
    is currently running and streaming a2dp music.

RETURNS
    void
*/
void sinkAudioSetEnhancement(uint16 enhancement, bool enable);

void CreateAudioConnection(void);

/****************************************************************************
NAME    
    disconnectScoIfConnected

DESCRIPTION
    Disconnects the Sco link if its no more required

RETURNS
    void
*/
void disconnectScoIfConnected(bdaddr bd_addr);

/****************************************************************************
NAME
    sinkAudioProcessEvent

DESCRIPTION
    Event handler for usr/sys events related to audio.

RETURNS
    TRUE if the event was handled, otherwise FALSE
*/
bool sinkAudioProcessEvent(MessageId id);

/****************************************************************************
NAME
    sinkAudioEQOperatingStateIndication

DESCRIPTION
    generates an event that can be used to drive an LED to indicate whether the EQ
    funcionality is enabled or not

RETURNS
    void
*/
void sinkAudioEQOperatingStateIndication(void);

/****************************************************************************
NAME
    audioShutDownForUpgrade

DESCRIPTION
    Sends an AVRCP pause command to the AG, disconnects any audio stream and once
    no audio is in use, messages sink_upgrade to proceed

RETURNS
    void
*/
void audioShutDownForUpgrade(Task task, MessageId id);

#endif 



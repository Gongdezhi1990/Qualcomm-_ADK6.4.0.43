/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_hfp_data.h

DESCRIPTION
    Handles the Hfp features.
*/

#ifndef _SINK_HFP_H_
#define _SINK_HFP_H_

#include <csrtypes.h>
#include <stdlib.h>
#include <audio_plugin_if.h>
#include <hfp.h>
#include <sink.h>

/* swap between profiles, when called with primary will return secondary and vice versa */
#define OTHER_PROFILE(x) (x ^ 0x3)

/* hfp profiles have offset of 1, this needs to be removed to be used as index into data array */
#define PROFILE_INDEX(x) ((x == hfp_invalid_link) ? (0) : (x - 1))
/*
This is the status of HFP profile connection status .
*/
typedef enum
{
    hfp_disconnected,
    hfp_connected,
    hfp_disconnecting
}hfp_connection_status;
/* Other header file and Includes */
typedef struct                              /* 1 word of storage for hfp status info */
{
    unsigned        list_id:8;               /* store the PDL list ID for this connection, used for link loss and preventing reconnection */
    unsigned        local_call_action:1;     /* call answered/rejected locally */
    hfp_connection_status        connected:2;             /* enum to signifiy the status of HFP profile from application perspective */
    unsigned        voice_recognition_supported:1;  /* Does this link support voice reconginition. */
}profile_status_info;

/* this is the list of sco audio priorities, it is used to determine which audio is routed by the
   device in multipoint situations, the further up the list will get routed first so be careful
   not to change the order without careful thought of the implications of doing so */
typedef enum
{
    sco_none,
    sco_about_to_disconnect,
    sco_streaming_audio,
    sco_inband_ring,
    sco_held_call,
    sco_active_call
}audio_priority;

typedef struct                              /* storage of audio connection data on a per hfp isntance basis */
{
    audio_priority      sco_priority;        /* the priority level of the sco */
    uint32              tx_bandwidth;
    sync_link_type      link_type:2;          /* link type may be different between AG's and needs to be stored for reorouting audio */
    hfp_wbs_codec_mask  codec_selected:5; /* audio codec being used with this profile */
    unsigned            audio_valid:1;      /* The audio connected message has been received from HFP library */
    unsigned            gSMVolumeLevel:7;   /* volume level for this profile */
    unsigned            gMuted:1;
}profile_audio_info;

typedef struct
{
    profile_status_info status;         /* status for each profile, in hfp index order */
    profile_audio_info  audio;           /* audio connection details used for re-routing audio */
}profile_data_t ;
/*************************************************************************
NAME
    sinkHfpDataHasAtCmdsData

DESCRIPTION
    Function to check if HFP AT Commands data present.

RETURNS
    bool

*/
bool sinkHfpDataHasAtCmdsData(void);
/*************************************************************************
NAME
    sinkHfpDataInit

DESCRIPTION
    Function to Initialise the Hfp configuration data.

RETURNS
    bool

*/
bool sinkHfpDataInit(hfp_init_params *hfp_init_param);
/*************************************************************************
NAME
    sinkHfpDataUpdateSupportedProfile

DESCRIPTION
    Function to update the Hfp supported profile

RETURNS
    void
*/
void sinkHfpDataUpdateSupportedProfile(hfp_profile profile);
/*************************************************************************
NAME
    sinkHfpDataGetSupportedProfile

DESCRIPTION
    To get the Hfp supported profile
RETURNS
    uint8
*/
uint8 sinkHfpDataGetSupportedProfile(void);
/*************************************************************************
NAME
    sinkHfpDataIsLNRCancelsVoiceDialIfActive

DESCRIPTION
    Function to check LNRCancelsVoiceDialIfActive features.

RETURNS
    bool
*/
bool sinkHfpDataIsLNRCancelsVoiceDialIfActive(void);
/*************************************************************************
NAME
    sinkHfpDataUpdateSyncPktTypes

DESCRIPTION
    Function to update HFP sync pkt types.

RETURNS
    void
*/
void sinkHfpDataUpdateSyncPktTypes(void);
/*************************************************************************
NAME
    GetHfpFeaturesSyncType

DESCRIPTION
    Function to get Hfp Supported Features type .

RETURNS
    sync_pkt_type
*/

sync_pkt_type sinkHfpDataGetFeaturesSyncType(void);
/*************************************************************************
NAME
    GetHfpFeaturesAdditionalAudioParams

DESCRIPTION
    Function to get Hfp Features Additional params .

RETURNS
    bool
*/

bool sinkHfpDataGetFeaturesAdditionalAudioParams(void);
/*************************************************************************
NAME
    GetHfpFeaturesTypeAudioParams

DESCRIPTION
    Function to get Hfp Features audio params .

RETURNS
    Pointer to hfp_audio_params
*/

hfp_audio_params* sinkHfpDataGetFeaturesTypeAudioParams(void);
/************************************************************************************
NAME
    sinkHfpDataGetEventCount

DESCRIPTION
    Function to get number of configured AT command events.

RETURNS
    Number of events

*/
uint16 sinkHfpDataGetEventCount(void);
/************************************************************************************
NAME
    sinkHfpDataGetEventAtCommandsEvent

DESCRIPTION
    Function to get At commands event.

RETURNS
    uint8
*/
uint16 sinkHfpDataGetEventAtCommandsEvent(uint16 i);
/************************************************************************************
NAME
    sinkHfpDataGetEventAtCommands

DESCRIPTION
    Function to get At commands .

RETURNS
    uint16
*/
uint16 sinkHfpDataGetEventAtCommands(uint16 i);
/************************************************************************************
NAME
    sinkHfpDataGetAtCommandsSize

DESCRIPTION
    Function to get size of AT Commands data.

RETURNS
    uint16
*/
uint16 sinkHfpDataGetAtCommandsSize(void);
/*************************************************************************
NAME
    sinkHfpDataGetAtCommands

DESCRIPTION
    Pointer to packed command string

RETURNS
    Pointer to Char
*/
uint16 *sinkHfpDataGetAtCommands(void);
/*************************************************************************
NAME
    sinkHfpDataSetAudioValid

DESCRIPTION
    Function to set whether the HFP library has informed the application that audio exists

RETURNS
    void

*/
void sinkHfpDataSetAudioValid(uint8 index, bool status);
/*************************************************************************
NAME
    sinkHfpDataGetAudioValid

DESCRIPTION
    Function to get whether the HFP library has informed the application that audio exists

RETURNS
    boolean indicating audio validity

*/
bool sinkHfpDataGetAudioValid(uint8 index);
/******************************************************************************
NAME
    sinkHfpDataGetDefaultVolume

DESCRIPTION
    Retrieve default HFP volume.

RETURNS
    uint8 value of default HFP volume.

*/
uint8 sinkHfpDataGetDefaultVolume(void);

/******************************************************************************
NAME
    sinkHfpDataSetDefaultVolume

DESCRIPTION
    Set the default HFP volume.

PARAMS
    volume  Value of default HFP volume.

RETURNS
    TRUE if default HFP volume was changed, FALSE otherwise.
*/
bool sinkHfpDataSetDefaultVolume(uint8 volume);
/*************************************************************************
NAME
    sinkHfpdataSetAudioSMVolumeLevel

DESCRIPTION
    Function to set audio data Volume level.

RETURNS
    void
*/
void sinkHfpdataSetAudioSMVolumeLevel( uint16 volume_level, uint8 index);
/*************************************************************************
NAME
    sinkHfpDataGetAudioSMVolumeLevel

DESCRIPTION
    Function to get audio data Volume level.

RETURNS
    uint8
**************************************************************************/
uint8 sinkHfpDataGetAudioSMVolumeLevel(uint8 index);
/*************************************************************************
NAME
    sinkHfpDataSetAudioSynclinkType

DESCRIPTION
    Function to set audio data Sync Link Type

RETURNS
    void

*/
void sinkHfpDataSetAudioSynclinkType( sync_link_type link_type, uint8 index);
/*************************************************************************
NAME
    sinkHfpDataGetAudioSynclinkType

DESCRIPTION
    Function to get audio data Sync Link Type

RETURNS
    sync_link_type

**************************************************************************/
sync_link_type sinkHfpDataGetAudioSynclinkType(uint8 index);
/*************************************************************************
NAME
    sinkHfpDataSetAudioCodec

DESCRIPTION
    Function to set audio data codec selected

RETURNS
    void

*/
void sinkHfpDataSetAudioCodec(hfp_wbs_codec_mask codec_selected, uint8 index);
/*************************************************************************
NAME
    sinkHfpDataGetAudioCodec

DESCRIPTION
    Function to get audio data codec selected

RETURNS
    hfp_wbs_codec_mask

*/
hfp_wbs_codec_mask sinkHfpDataGetAudioCodec(uint8 index);
/*************************************************************************
NAME
    sinkHfpDataGetAudioSink

DESCRIPTION
    Function to get the HFP audio sink

RETURNS
    HFP audio sink or NULL if the sink could not be retrieved

*/
Sink sinkHfpDataGetAudioSink(uint8 index);
/*************************************************************************
NAME
    sinkHfpDataSetScoPriority

DESCRIPTION
    Function to set audio data sco priority

RETURNS
    void

*/
void sinkHfpDataSetScoPriority(audio_priority sco_priority, uint16 index);
/*************************************************************************
NAME
    sinkHfpDataGetScoPriority

DESCRIPTION
    Function to get audio data sco priority

RETURNS
    audio_priority

*/
audio_priority sinkHfpDataGetScoPriority(uint16 index);
/*************************************************************************
NAME
    sinkHfpDataSetAudioTxBandwidth

DESCRIPTION
    Function to set audio data Tx bandwidth

RETURNS
    void

*/
void sinkHfpDataSetAudioTxBandwidth(uint32 tx_bandwidth, uint8 index);
/*************************************************************************
NAME
    sinkHfpDataGetAudioTxBandwidth

DESCRIPTION
    Function to get audio data Tx bandwidth

RETURNS
    uint32

*/
uint32 sinkHfpDataGetAudioTxBandwidth(uint8 index);
/*************************************************************************
NAME
	sinkHfpDataSetAudioGMuted

DESCRIPTION
    Function to set audio data Mute state.

RETURNS
    void

*/
void sinkHfpDataSetAudioGMuted(bool g_muted, uint8 index);
/*************************************************************************
NAME
    sinkHfpDataGetAudioGMuted

DESCRIPTION
    Function to get audio data Mute state.

RETURNS
    bool

*/
bool sinkHfpDataGetAudioGMuted(uint8 index);
/*************************************************************************
NAME
    sinkHfpDataGetStoredNumber

DESCRIPTION
    Function to get stored phone number.

RETURNS
    uint16

*/
uint16 sinkHfpDataGetStoredNumber(uint16 *phone_number);
/*************************************************************************
NAME
    sinkHfpDataUpdateWriteableConfig

DESCRIPTION
    Function to update the stored number.

*/
void sinkHfpDataUpdateStoredNumber(uint8 * phone_number_key, uint16 size_phone_number);
/*************************************************************************
NAME
    sinkHfpDataGetActionOnCallTransfer

DESCRIPTION
    Functions to return HFP Features config- ActionOnCallTransfer.

RETURNS
    uint8

*/
uint8 sinkHfpDataGetActionOnCallTransfer(void);
/*************************************************************************
NAME
    sinkHfpDataGetAudioPlugin

DESCRIPTION
    Get the audio plugin associated with hfp.

RETURNS
    uint8

*/
uint8 sinkHfpDataGetAudioPlugin(void);
/*************************************************************************
NAME
    sinkHfpDataIsForceEV3S1ForSco2

DESCRIPTION
    Functions to check ForceEV3S1ForSco2.

RETURNS
    bool

*/
bool sinkHfpDataIsForceEV3S1ForSco2(void);
/*************************************************************************
NAME
    sinkHfpDatacopyLinkData

DESCRIPTION
    Function to copy Hfp profile data from secondary into primary link.
RETURNS
    void

*/
void sinkHfpDatacopyLinkData(uint8 primary_index, uint8 secondary_index );
/*************************************************************************
NAME
    sinkHfpDataGetVoiceRecognitionIsActive

DESCRIPTION
    Function to get VoiceRecognitionIsActive.

*/
uint8 sinkHfpDataGetVoiceRecognitionIsActive(void);
/*************************************************************************
NAME
    sinkHfpDataSetVoiceRecognitionIsActive

DESCRIPTION
    Function to set VoiceRecognitionIsActive.

*/
void sinkHfpDataSetVoiceRecognitionIsActive(uint16 status);

/*!
    @brief Check if the given HFP link supports voice recognition.
    
    @param index The index of the HFP link to check.
    
    @return TRUE if voice recognition is supported; FALSE otherwise.
*/
bool sinkHfpDataGetVoiceRecognitionSupported(uint8 index);

/*!
    @brief Set if the given HFP link supports voice recognition.
    
    @param index The index of the HFP link to set.
    @param supported TRUE if this HFP link supports voice recognition; FALSE otherwise.
*/
void sinkHfpDataSetVoiceRecognitionSupported(uint8 index, bool supported);

/*************************************************************************
NAME
    sinkHfpDataSetNetworkPresent

DESCRIPTION
    Interface function to set Network status
*************************************************************************/
void sinkHfpDataSetNetworkPresent(bool status);
/*************************************************************************
NAME
    sinkHfpDataSetNetworkPresent

DESCRIPTION
    Interface function to get Network status
*************************************************************************/
bool sinkHfpDataIsNetworkPresent(void);
/*************************************************************************
NAME
    sinkHfpDataSetLastOutgoingAg

DESCRIPTION
    Interface function to set last_outgoing_ag
*************************************************************************/
void sinkHfpDataSetLastOutgoingAg(hfp_link_priority link_priority);
/*************************************************************************
NAME
    sinkHfpDataGetLastOutgoingAg

DESCRIPTION
    Interface function to get last_outgoing_ag
*************************************************************************/
uint8 sinkHfpDataGetLastOutgoingAg(void);
/*************************************************************************
NAME
    sinkHfpDataSetRepeatCallerIDFlag

DESCRIPTION
    Interface function to set RepeatCallerIDFlag
*************************************************************************/
void sinkHfpDataSetRepeatCallerIDFlag(bool status);
/*************************************************************************
NAME
    sinkHfpDataGetRepeatCallerIDFlag

DESCRIPTION
    Interface function to get RepeatCallerIDFlag
*************************************************************************/
bool sinkHfpDataGetRepeatCallerIDFlag(void);
/*************************************************************************
NAME
    sinkHfpDataSetHeldCallIndex

DESCRIPTION
    Interface function to set HeldCallIndex
*************************************************************************/
void sinkHfpDataSetHeldCallIndex(uint8 index);
/*************************************************************************
NAME
    sinkHfpDataGetHeldCallIndex

DESCRIPTION
    Interface function to get HeldCallIndex
*************************************************************************/
uint8 sinkHfpDataGetHeldCallIndex(void);
/*************************************************************************
NAME
    sinkHfpDataSetMissedCallIndicated

DESCRIPTION
    Interface function to set MissedCallIndicated
*************************************************************************/
void sinkHfpDataSetMissedCallIndicated(uint16 value);
/*************************************************************************
NAME
    sinkHfpDataGetMissedCallIndicated

DESCRIPTION
    Interface function to get MissedCallIndicated
*************************************************************************/
uint8 sinkHfpDataGetMissedCallIndicated(void);
/*************************************************************************
NAME
    sinkHfpDataSetLinkLossReminderTime

DESCRIPTION
    Interface function to set linkLossReminderTime
*************************************************************************/
void sinkHfpDataSetLinkLossReminderTime(uint32 link_loss_interval_time);
/*************************************************************************
NAME
    sinkHfpDataGetLinkLossReminderTime

DESCRIPTION
    Interface function to get linkLossReminderTime
*************************************************************************/
uint8 sinkHfpDataGetLinkLossReminderTime(void);
#ifdef WBS_TEST
/*************************************************************************
NAME
    sinkHfpDataSetFailAudioNegotiation

DESCRIPTION
    Interface function to set FailAudioNegotiation
*************************************************************************/
void sinkHfpDataSetFailAudioNegotiation(bool status);
/*************************************************************************
NAME
    sinkHfpDataSetRenegotiateSco

DESCRIPTION
    Interface function to set RenegotiateSco
*************************************************************************/
void sinkHfpDataSetRenegotiateSco(bool status);
/*************************************************************************
NAME
    sinkHfpDataIsFailAudioNegotiation

DESCRIPTION
    Interface function to get FailAudioNegotiation
*************************************************************************/
bool sinkHfpDataIsFailAudioNegotiation(void);
/*************************************************************************
NAME
    sinkHfpDataIsRenegotiateSco

DESCRIPTION
    Interface function to get RenegotiateSco
*************************************************************************/
bool sinkHfpDataIsRenegotiateSco(void);
#endif

#if defined(TEST_HF_INDICATORS)
/*************************************************************************
NAME
    sinkHfpDataSetHfEnhancedSafety

DESCRIPTION
    Interface function to set hf_enhancedSafety
*************************************************************************/
void sinkHfpDataSetHfEnhancedSafety(bool status);
/*************************************************************************
NAME
    sinkHfpDataGetHfEnhancedSafety

DESCRIPTION
    Interface function to get hf_enhancedSafety
*************************************************************************/
bool sinkHfpDataGetHfEnhancedSafety(void);
#endif
/*************************************************************************
NAME
    sinkHfpDataGetNWSIndicatorRepeatTime

DESCRIPTION
    Functions to return HFP Features timeouts - NetworkServiceIndicatorRepeatTime.

RETURNS
    uint16

*/
uint16 sinkHfpDataGetNWSIndicatorRepeatTime(void);
/*************************************************************************
NAME
    sinkHfpDataSetNWSIndicatorRepeatTime

DESCRIPTION
    Functions to set HFP Features timeouts - NetworkServiceIndicatorRepeatTime.

RETURNS
    bool

*/
bool sinkHfpDataSetNWSIndicatorRepeatTime(uint8 repeat_timeout);
/*************************************************************************
NAME
    sinkHfpDataGetMuteRemindTime

DESCRIPTION
    Functions to return HFP Features timeouts - MuteRemindTime.

RETURNS
    uint16

*/
uint16 sinkHfpDataGetMuteRemindTime(void);
/*************************************************************************
NAME
    sinkHfpDataSetMuteRemindTime

DESCRIPTION
    Functions to set HFP Features timeouts - MuteRemindTime.

RETURNS
    bool

*/
bool sinkHfpDataSetMuteRemindTime(uint16 mute_timeout);
/*************************************************************************
NAME
    sinkHfpDataGetMissedCallIndicateTime

DESCRIPTION
    Functions to return HFP Features timeouts - MissedCallIndicateTime.

RETURNS
    uint16

*/
uint16 sinkHfpDataGetMissedCallIndicateTime(void);
/*************************************************************************
NAME
    sinkHfpDataSetMissedCallIndicateTime

DESCRIPTION
    Functions to set HFP Features timeouts - MissedCallIndicateTime.

RETURNS
    bool

*/
bool sinkHfpDataSetMissedCallIndicateTime(uint16 timeout);
/*************************************************************************
NAME
    sinkHfpDataGetMissedCallIndicateAttempsTime

DESCRIPTION
    Functions to return HFP Features timeouts - MissedCallIndicateAttemps.

RETURNS
    uint16

*/
uint16 sinkHfpDataGetMissedCallIndicateAttempsTime(void);
/*************************************************************************
NAME
    sinkHfpDataSetMissedCallIndicateAttempsTime

DESCRIPTION
    Functions to set HFP Features timeouts - MissedCallIndicateAttemps.

RETURNS
    bool

*/
bool sinkHfpDataSetMissedCallIndicateAttempsTime(uint16 timeout);
/*************************************************************************
NAME
    sinkHfpDataIsEnableSyncMuteMicrophones

DESCRIPTION
    Function to check EnableSyncMuteMicrophones features.

RETURNS
    bool

*/
bool sinkHfpDataIsEnableSyncMuteMicrophones(void);
/*************************************************************************
NAME
    sinkHfpDataIsAutoAnswerOnConnect

DESCRIPTION
    Function to check AutoAnswerOnConnect features.

RETURNS
    bool

*/
bool sinkHfpDataIsAutoAnswerOnConnect(void);
/*************************************************************************
NAME
    sinkHfpDataGetCallActivePIO

DESCRIPTION
    Function to get CallActivePIO.

RETURNS
    uint8

*/
uint8 sinkHfpDataGetCallActivePIO(void);
/*************************************************************************
NAME
    sinkHfpDataGetIncomingRingPIO

DESCRIPTION
    Function to get IncomingRingPIO.

RETURNS
    uint8

*/
uint8 sinkHfpDataGetIncomingRingPIO(void);
/*************************************************************************
NAME
    sinkHfpDataGetOutgoingRingPIO

DESCRIPTION
    Function to get OutgoingRingPIO.

RETURNS
    uint8

*/
uint8 sinkHfpDataGetOutgoingRingPIO(void);
/*************************************************************************
NAME
    sinkHfpDataSetProfileConnectedStatus

DESCRIPTION
    Function to set Hfp profiles connected status.

RETURNS
    void
*/
void sinkHfpDataSetProfileConnectedStatus(hfp_connection_status value, uint16 index);
/*************************************************************************
NAME
    sinkHfpDataSetProfileListIdStatus

DESCRIPTION
    Function to set Hfp profiles List Id status.

RETURNS
    void
*/
void sinkHfpDataSetProfileListIdStatus(uint8 value, uint16 index);
/*************************************************************************
NAME
    sinkHfpDataSetProfileLocalCallActionStatus

DESCRIPTION
    Function to set Hfp profiles local call action status.

RETURNS
    void

*/
void sinkHfpDataSetProfileLocalCallActionStatus(bool value, uint16 index);
/*************************************************************************
NAME
    sinkHfpDataGetProfileConnectedStatus

DESCRIPTION
    Function to get Hfp profiles connected status.

RETURNS
    hfp_connection_status

*/
hfp_connection_status sinkHfpDataGetProfileStatusConnected(uint16 index);
/*************************************************************************
NAME
    sinkHfpDataGetProfileStatusListId

DESCRIPTION
    Function to Get Hfp profiles List Id status.

RETURNS
    uint8

*/
uint8 sinkHfpDataGetProfileStatusListId(uint16 index);
/*************************************************************************
NAME
    sinkHfpDataGetProfileLocalCallActionStatus

DESCRIPTION
    Function to Get Hfp profiles local call action status.

RETURNS
    bool

*/
bool sinkHfpDataGetProfileStatusLocalCallAction(uint16 index);
/*************************************************************************
NAME
    sinkHfpDataSetAudioPlugin

DESCRIPTION
    Interface function to set HFP Features config audio plugin.
*************************************************************************/
void sinkHfpDataSetAudioPlugin(uint16 audio_plugin);
/*************************************************************************
NAME
    sinkHfpDataGetHfpPluginParams

DESCRIPTION
    Functions to retrieve HFP plugin params.

RETURNS
    Pointer to hfp_common_plugin_params_t.

*/
hfp_common_plugin_params_t * sinkHfpDataGetHfpPluginParams(void);

#endif

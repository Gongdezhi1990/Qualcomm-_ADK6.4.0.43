/*
Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    These are the functions for controlling the Party Mode feature of the soundbar, 
    bluetooth connections and audio routing is handled differently in this mode
    of operation
*/
    
#include "sink_partymode.h"
#include "sink_statemanager.h"
#include "sink_audio.h"
#include "sink_audio_routing.h"
#include "sink_configmanager.h"
#include "sink_slc.h"
#include "sink_main_task.h"
#ifdef ENABLE_PBAP        
#include "sink_pbap.h"
#endif

#ifdef ENABLE_MAPC
#include "sink_mapc.h"
#endif

#include <stdlib.h>
#include <audio.h>
#include <bdaddr.h>

#ifdef DEBUG_PARTY_MODE
    #define PTY_DEBUG(x) DEBUG(x)
#else
    #define PTY_DEBUG(x) 
#endif

#ifdef ENABLE_PARTYMODE
#include "config_definition.h"
#include "sink_partymode_config_def.h"
#include <config_store.h>

#define OTHER_DEVICE(x) (x ^ 0x1)

/* Sink PartyMode Global Data */
typedef struct __sink_partymode_global_data_t
{
    unsigned:15;
    unsigned PartyModeEnabled:1;
    pty_pause_state_t partymode_pause;  
}sink_partymode_global_data_t;

static sink_partymode_global_data_t gPartyModeData;
#define PARTYMODE_GDATA gPartyModeData


/****************************************************************************
NAME    
    sinkCheckAvrcpStateMatch
    
DESCRIPTION
    helper function to check an avrcp play status for a given a2dp link priority
    
RETURNS
    true is the avrcp play state matches that passed in
    false if state does not match or no matching avrcp link is found for a2dp profile index
*/
static bool sinkCheckAvrcpStateMatch(a2dp_index_t index, avrcp_play_status play_status)
{
    if((BdaddrIsSame(getA2dpLinkBdAddr(index), sinkAvrcpGetLinkBdAddr(a2dp_primary))&&(sinkAvrcpGetPlayStatus(a2dp_primary) == play_status))||
       (BdaddrIsSame(getA2dpLinkBdAddr(index), sinkAvrcpGetLinkBdAddr(a2dp_secondary))&&(sinkAvrcpGetPlayStatus(a2dp_secondary) == play_status)))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
         

/*************************************************************************
NAME
    sinkPartymodeGetOperatingMode
    
DESCRIPTION
    Function to get operating mode
    
RETURNS
    Configured operating Partymode : Either none /barge-in /avrcp control state
*/
PartyModeOperatingMode sinkPartymodeGetOperatingMode(void)
{
    sink_partymode_readonly_config_def_t *read_config = NULL;
    PartyModeOperatingMode mode = partymode_none;

    if (configManagerGetReadOnlyConfig(SINK_PARTYMODE_READONLY_CONFIG_BLK_ID, (const void **)&read_config))
    {
       mode = read_config->PartyMode;
       configManagerReleaseConfig(SINK_PARTYMODE_READONLY_CONFIG_BLK_ID);
    }
    return mode;
}
/*************************************************************************
NAME
    sinkPartymodeTurnOn
    
DESCRIPTION
    Function to turn on Partymode
*/
void sinkPartymodeTurnOn(void)
{
    PARTYMODE_GDATA.PartyModeEnabled = TRUE;   
}
/*************************************************************************
NAME
    sinkPartymodeTurnOff
    
DESCRIPTION
    Function to turn off Partymode
*/
void sinkPartymodeTurnOff(void)
{
    PARTYMODE_GDATA.PartyModeEnabled = FALSE;   
}
/*************************************************************************
NAME
    sinkPartymodeIsEnabled
    
DESCRIPTION
    Function to get global Partymode Enabled flag if set or not
    
RETURNS
    Partymode Enabled status
*/
bool sinkPartymodeIsEnabled (void)
{
    return PARTYMODE_GDATA.PartyModeEnabled;
}


/*************************************************************************
NAME
    sinkPartymodeGetMusicTimeOut
    
DESCRIPTION
    Function to get configured music timeout 
RETURNS
    Configured Party Mode music timeout value
*/
uint16 sinkPartymodeGetMusicTimeOut(void)
{
    sink_partymode_readonly_config_def_t *read_config = NULL;
    uint16 music_timeout_val = 0;

    if (configManagerGetReadOnlyConfig(SINK_PARTYMODE_READONLY_CONFIG_BLK_ID, (const void **)&read_config))
    {
       music_timeout_val = read_config->PartyModeMusicTimeOut_s;
       configManagerReleaseConfig(SINK_PARTYMODE_READONLY_CONFIG_BLK_ID);
    }
    return music_timeout_val;
}

bool sinkPartymodeSetMusicTimeOut(uint16 timeout)
{
    sink_partymode_readonly_config_def_t *read_config = NULL;

    if (configManagerGetWriteableConfig(SINK_PARTYMODE_READONLY_CONFIG_BLK_ID, (void **)&read_config, 0))
    {
        read_config->PartyModeMusicTimeOut_s = timeout;
        configManagerUpdateWriteableConfig(SINK_PARTYMODE_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/*************************************************************************
NAME
    sinkPartymodeGetStreamResumeTimeOut
    
DESCRIPTION
    Function to get configured stream resume timeout 
    
RETURNS
    Configured Party Mode stream resume timeout value
*/
uint16 sinkPartymodeGetStreamResumeTimeOut(void)
{
    sink_partymode_readonly_config_def_t *read_config = NULL;
    uint16 timeout = 0;

    if (configManagerGetReadOnlyConfig(SINK_PARTYMODE_READONLY_CONFIG_BLK_ID, (const void **)&read_config))
    {
        timeout = read_config->PartyModeStreamResumeTimeOut_s;
        configManagerReleaseConfig(SINK_PARTYMODE_READONLY_CONFIG_BLK_ID);
    }
    return timeout;
}

bool sinkPartymodeSetStreamResumeTimeOut(uint16 timeout)
{
    sink_partymode_readonly_config_def_t *read_config = NULL;

    if (configManagerGetWriteableConfig(SINK_PARTYMODE_READONLY_CONFIG_BLK_ID, (void **)&read_config, 0))
    {
        read_config->PartyModeStreamResumeTimeOut_s = timeout;
        configManagerUpdateWriteableConfig(SINK_PARTYMODE_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/*************************************************************************
NAME
    sinkPartymodePauseAudio
    
DESCRIPTION
    Function to set global Partymode flag to indicate whether audio source primary or 
    secondary has been paused and is queued for playing
*/
void sinkPartymodePauseAudio(PartyModeAudioSource audio_type, bool value)
{
    if(audio_type == partymode_primary)
    {
       PARTYMODE_GDATA.partymode_pause.audio_source_primary_paused = value;
    }
    else
    {
       PARTYMODE_GDATA.partymode_pause.audio_source_secondary_paused = value;
    }
}


/****************************************************************************
NAME    
    sinkCheckPartyModeAudio
    
DESCRIPTION
    Called when checking for PartyMode being enabled, if not enabled no action is
    taken, when enabled decisions are made as to what audio should be playing or
    paused/resumed etc
    
RETURNS
    bool false if party mode not enabled, true is party mode enabled and action has
    been taken with regards to the routing of the audio source
*/
bool sinkCheckPartyModeAudio(void)
{
    uint8 index;
    
    /* check whether party mode is enabled and configured and if the currently routed audio is one of the a2dp streams */
    if( sinkPartymodeIsEnabled() && sinkAudioIsAudioRouted() && sinkA2dpAudioPrimaryOrSecondarySinkMatch(sinkAudioGetRoutedAudioSink()))
    {
        sink_partymode_readonly_config_def_t *read_config = NULL;
        uint16 PartyModeStreamResumeTimeOut_s = 0;
        PartyModeOperatingMode PartyMode = partymode_none;

        if (configManagerGetReadOnlyConfig(SINK_PARTYMODE_READONLY_CONFIG_BLK_ID, (const void **)&read_config))
    {
            PartyModeStreamResumeTimeOut_s = read_config->PartyModeStreamResumeTimeOut_s;
            PartyMode = read_config->PartyMode;
            configManagerReleaseConfig(SINK_PARTYMODE_READONLY_CONFIG_BLK_ID);
        }
        
        /* determine the PartyMode operating mode required */     
        switch(PartyMode)
        {
            /* simple barge-in mode of operation, a new audio source streaming music
               disconnects any currently streaming device */
            case partymode_barge_in:
                PTY_DEBUG(("PTY: bargein\n"));

                /* if the current streaming audio A2DP pri and a stream for A2DP sec is available, switch to that */            
                if(a2dpAudioSinkMatch(a2dp_primary, sinkAudioGetRoutedAudioSink()) && sinkA2dpGetStreamState(a2dp_secondary) == a2dp_stream_streaming)
                {
#ifdef ENABLE_AVRCP
                    /*AG supports AVRCP, now route its audio only if it's AVRCP PLAY STATUS is PLAYING, in a way to block touch tones
                    Note: This work around is not useful in following circumstances and AG can still Barge-in :
                    1. If AG does not support AVRCP Profile 2. If AG Supports AVRCP 1.0 Version in which current Play Status reporting feature is not available
                    3. If AG Supports Later versions AVRCP 1.0 but AVRCP profile is not yet connected during Barge-in */
                    if ((getAgAvrcpSupportStatus(a2dp_secondary) != avrcp_support_supported) || sinkCheckAvrcpStateMatch(a2dp_secondary, avrcp_play_status_playing))
#endif
                    {
                        PTY_DEBUG(("PTY: drop current pri route new sec\n"));

                        /* drop bluetooth connection to device currently streaming */
                        for(index = a2dp_primary; index < (a2dp_secondary+1); index++)
                        {
                            /* is a2dp connected? */
                            if(getA2dpStatusFlag(CONNECTED, index))
                            {
                                /* check whether the a2dp connection is present and streaming data and that the audio is routed */
                                if(sinkAudioGetRoutedAudioSink() == A2dpMediaGetSink(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index)))
                                {
                                    if(A2dpMediaGetState(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index)) == a2dp_stream_streaming)
                                {                                   
                                    /* disconnect a2dp audio device */
                                    sinkPartyModeDisconnectDevice(index);
                                }
                                    else
                                    {
                                        /* Dont disconnect the a2dp device. Just close the Active audio sink */
                                        audioDisconnectRoutedAudio();
                                    }
                            }
                        }
                        }
                        /* route the audio from the new device */
                        audioRouteSpecificA2dpSource(audio_source_a2dp_2);
                    }
                }
                /* if the current streaming audio A2DP sec and a stream for A2DP pri is available, switch to that */            
                else if(a2dpAudioSinkMatch(a2dp_secondary, sinkAudioGetRoutedAudioSink()) && (sinkA2dpGetStreamState(a2dp_primary) == a2dp_stream_streaming))
                {
#ifdef ENABLE_AVRCP
                    /*AG supports AVRCP, now route its audio only if it's AVRCP PLAY STATUS is PLAYING, in a way to block touch tones
                    Note: This work around is not useful in following circumstances and AG can still Barge-in :
                    1. If AG does not support AVRCP Profile 2. If AG Supports AVRCP 1.0 Version in which current Play Status reporting feature is not available
                    3. If AG Supports Later versions AVRCP 1.0 but AVRCP profile is not yet connected during Barge-in */
                    if ((getAgAvrcpSupportStatus(a2dp_primary) != avrcp_support_supported) || sinkCheckAvrcpStateMatch(a2dp_primary, avrcp_play_status_playing))
#endif
                    {
                        PTY_DEBUG(("PTY: drop current sec route new pri\n"));
                        /* drop bluetooth connection to device currently streaming */
                        for(index = a2dp_primary; index < (a2dp_secondary+1); index++)
                        {
                            /* is a2dp connected? */
                            if(getA2dpStatusFlag(CONNECTED, index))
                            {
                                /* check whether the a2dp connection is present and streaming data and that the audio is routed */
                                if(sinkAudioGetRoutedAudioSink() == A2dpMediaGetSink(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index)))
                                {
                                    if(A2dpMediaGetState(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index)) == a2dp_stream_streaming)
                                {                                   
                                   /* disconnect a2dp audio device */
                                    sinkPartyModeDisconnectDevice(index);
                                }
                                    else
                                    {
                                        /* Dont disconnect the a2dp device. Just close the Active audio sink */
                                        audioDisconnectRoutedAudio();
                                    }
                            }
                        }
                        }
                        /* route the audio from the new device */
                        audioRouteSpecificA2dpSource(audio_source_a2dp_1);
                    }
                }
                /* no action has been taken */
                else
                    return TRUE;
            break;
        
            /* more complex use case, a new streaming audio source is paused using avrcp
               until the current playing track completes */
            case partymode_avrcp_control:
                PTY_DEBUG(("PTY: avrcp ctrl AG1[%x] AG2[%x] AVRCP1[%x] AVRCP[%x] \n",sinkA2dpGetStreamState(a2dp_primary),sinkA2dpGetStreamState(a2dp_secondary), sinkAvrcpGetPlayStatus(a2dp_primary),sinkAvrcpGetPlayStatus(a2dp_secondary)));

                /* if the current streaming audio is A2DP pri and a stream for A2DP sec is available, pause that and wait for the 
                   current track to finish playing */            
                if(a2dpAudioSinkMatch(a2dp_primary, sinkAudioGetRoutedAudioSink())
                        && sinkA2dpGetStreamState(a2dp_primary) == a2dp_stream_streaming && sinkA2dpGetStreamState(a2dp_secondary) == a2dp_stream_streaming
                        && sinkCheckAvrcpStateMatch(a2dp_secondary, avrcp_play_status_playing))
                {
                    PTY_DEBUG(("PTY: suspend a2dp sec audio until pri track finished \n"));
                    SuspendA2dpStream(a2dp_secondary);
                    /* set paused flag */
                   sinkPartymodePauseAudio(partymode_secondary, TRUE);
                }
                /* if the current streaming audio A2DP sec and a stream for A2DP pri is available, switch to that */            
                else if(a2dpAudioSinkMatch(a2dp_secondary, sinkAudioGetRoutedAudioSink())
                        && sinkA2dpGetStreamState(a2dp_primary) == a2dp_stream_streaming && sinkA2dpGetStreamState(a2dp_secondary) == a2dp_stream_streaming
                        && sinkCheckAvrcpStateMatch(a2dp_primary, avrcp_play_status_playing))
                {
                    PTY_DEBUG(("PTY: suspend a2dp pri audio until sec track finished \n"));
                    SuspendA2dpStream(a2dp_primary);
                    /* set paused flag */
                    sinkPartymodePauseAudio(partymode_primary, TRUE);
                }
                /* check if currently routed primary source is still valid, disconnect if not */
                else if(a2dpAudioSinkMatch(a2dp_primary, sinkAudioGetRoutedAudioSink()) && sinkA2dpGetStreamState(a2dp_primary) != a2dp_stream_streaming)
                {
                    /* check if other source is present and paused */
                    a2dp_index_t link = sinkPartyModeCheckForOtherPausedSource(a2dp_primary);
                    PTY_DEBUG(("PTY: pri source not valid, disconnect\n"));
                    /* disconnect a2dp primary audio */
                    audioDisconnectRoutedAudio();
                    /* disconnect primary audio device */
                    if(deviceManagerNumConnectedDevs() > 1)
                        sinkPartyModeDisconnectDevice(a2dp_primary);
                    /* check for other paused audio source to route */                    
                    if((link != a2dp_invalid)&&(PARTYMODE_GDATA.partymode_pause.audio_source_secondary_paused))
                    {
                        PTY_DEBUG(("PTY: resume paused sec\n"));
                        /* resume paused device */
                        if(ResumeA2dpStream(a2dp_secondary))
                        {
                            audioRouteSpecificA2dpSource(audio_source_a2dp_2);
                        }
                        /* Cancel any existing timer running for secondary device */
                        MessageCancelAll(&theSink.task,(EventSysPartyModeTimeoutDevice2));
                        /* start partymode stream resume timer. On lapse of this timer, secondary device gets disconnected if it does not start streaming */
                        MessageSendLater(&theSink.task,(EventSysPartyModeTimeoutDevice2),0,D_SEC(PartyModeStreamResumeTimeOut_s));
                    }
                    return TRUE;                
                }
                /* check if currently routed secondary source is still valid, disconnect if not */
                else if(a2dpAudioSinkMatch(a2dp_secondary, sinkAudioGetRoutedAudioSink()) && sinkA2dpGetStreamState(a2dp_secondary) != a2dp_stream_streaming)
                {
                    /* check if other source is present and paused */
                    a2dp_index_t link = sinkPartyModeCheckForOtherPausedSource(a2dp_secondary);
                    PTY_DEBUG(("PTY: sec source not valid, disconnect\n"));
                    /* disconnect a2dp secondary audio */
                    audioDisconnectRoutedAudio();
                    /* disconnect secondary audio device */
                    if(deviceManagerNumConnectedDevs() > 1)
                        sinkPartyModeDisconnectDevice(a2dp_secondary);
                    /* check for other paused audio source to route */                    
                    if((link != a2dp_invalid)&&(PARTYMODE_GDATA.partymode_pause.audio_source_primary_paused))
                    {
                        PTY_DEBUG(("PTY: resume paused pri\n"));
                        /* resume paused device */
                        if(ResumeA2dpStream(a2dp_primary))
                        {
                            audioRouteSpecificA2dpSource(audio_source_a2dp_1);
                        }
                        /* Cancel any existing timer running for primary device */
                        MessageCancelAll(&theSink.task,(EventSysPartyModeTimeoutDevice1));
                        /* start partymode stream resume timer. On lapse of this timer, primary device gets disconnected if it does not start streaming */
                        MessageSendLater(&theSink.task,(EventSysPartyModeTimeoutDevice1),0,D_SEC(PartyModeStreamResumeTimeOut_s));
                    }
                    return TRUE;                
                }
                /* take no action at this time */
                else
                {
                    PTY_DEBUG(("PTY: avrcp - no action - AR[%p] Pri[%p] Sec[%p]\n",
                               (void*) sinkAudioGetRoutedAudioSink(),
                               (void*) sinkA2dpGetAudioSink(a2dp_primary),
                               (void*) sinkA2dpGetAudioSink(a2dp_secondary)));
                    
                    return TRUE;
                }
            break;
            
            /* not a valid configuration of operating mode */
            default:
                PTY_DEBUG(("PTY: invalid mode - no action\n"));
                /* default to standard audio routing mode of operation */
                return FALSE;
                       
        }
        return TRUE;   
    }        
    /* no audio routed or party mode not enabled */
    else
    {
        /* party mode not active or no audio currently routed, take no action and let standard audio routing
           functions take control */
        PTY_DEBUG(("PTY: NOT ACTIVE [%d] or NO AUDIO ROUTED AR[%p] Pri[%p] Sec[%p]\n",
                   sinkPartymodeIsEnabled(),                                                                                     
                   (void*) sinkAudioGetRoutedAudioSink(),
                   (void*) sinkA2dpGetAudioSink(a2dp_primary),
                   (void*) sinkA2dpGetAudioSink(a2dp_secondary)));
        
        return FALSE;
    }
}    
   
/****************************************************************************
NAME    
    sinkPartymodeDisconnectPrimaryResumeSecondary
    
DESCRIPTION
    To disconnect Primary AG and start streaming from Secondary AG
    This gets called when there is either skip track event or change in play status on primary a2dp device
    
RETURNS
    none
*/
static void sinkPartymodeDisconnectPrimaryResumeSecondary(uint16 index)
{	
    /* Check if primary AG is still streaming and audio is routed */
    if(( a2dpAudioSinkMatch(a2dp_primary, sinkAudioGetRoutedAudioSink())
			&& (sinkA2dpGetStreamState(a2dp_primary) == a2dp_stream_streaming)))
    {
        /* track change on a2dp primary is valid, check if a2dp secondary is paused */
        if((sinkA2dpGetStreamState(a2dp_secondary) == a2dp_stream_streaming)||(sinkA2dpGetStreamState(a2dp_secondary) == a2dp_stream_open))
        {
            /* check avrcp play status is paused */
            if((PARTYMODE_GDATA.partymode_pause.audio_source_secondary_paused)&&(sinkAvrcpGetPlayStatus((index^1)) == avrcp_play_status_paused))
            {
                PTY_DEBUG(("PTY: track change, route audio from secondary AG and disconnect primary AG\n"));
				
                /* disconnect primary device and resume streaming from secondary device*/
                sinkPartyModeDisconnectAndResume(a2dp_primary);
                /* Cancel any existing timer running for secondary device */
                MessageCancelAll(&theSink.task,(EventSysPartyModeTimeoutDevice2));
                /* start timer of 5 seconds after which secondary device gets disconnected if it does not start streaming */
                MessageSendLater(&theSink.task,(EventSysPartyModeTimeoutDevice2),0,D_SEC(sinkPartymodeGetStreamResumeTimeOut()));
                /* reset paused flag */
                sinkPartymodePauseAudio(partymode_secondary, FALSE);
             }
         }
     }
}

/****************************************************************************
NAME    
    sinkPartymodeDisconnectSecondaryResumePrimary
    
DESCRIPTION
    To disconnect Secondary AG and start streaming from Primary AG
    This gets called when there is either skip track event or change in play status on Secondary a2dp device
    
RETURNS
    none
*/
static void sinkPartymodeDisconnectSecondaryResumePrimary(uint16 index)
{
    /* Check if secondary AG is still streaming and audio is routed */
    if(( a2dpAudioSinkMatch(a2dp_secondary, sinkAudioGetRoutedAudioSink())
			&& (sinkA2dpGetStreamState(a2dp_secondary) == a2dp_stream_streaming)))
    {
        /* track change on a2dp secondary is valid, check if a2dp primary is paused */
        if((sinkA2dpGetStreamState(a2dp_primary) == a2dp_stream_streaming)||(sinkA2dpGetStreamState(a2dp_primary) == a2dp_stream_open))
        {
            /* check avrcp play status is paused */
            if((PARTYMODE_GDATA.partymode_pause.audio_source_primary_paused)&&(sinkAvrcpGetPlayStatus((index^1)) == avrcp_play_status_paused))
            {
                PTY_DEBUG(("PTY: track change, route audio from primary AG and disconnect secondary AG\n"));
				
                /* disconnect secondary device and resume primary device */
                sinkPartyModeDisconnectAndResume(a2dp_secondary);
                /* Cancel any existing timer running for primary device */
                MessageCancelAll(&theSink.task,(EventSysPartyModeTimeoutDevice1));
                /* start timer of 5 seconds after which primary device gets disconnected if it does not start streaming */
                MessageSendLater(&theSink.task,(EventSysPartyModeTimeoutDevice1),0,D_SEC(sinkPartymodeGetStreamResumeTimeOut()));
                /* reset paused flag */
                sinkPartymodePauseAudio(partymode_primary, FALSE);
             }
         }
     }
}

/****************************************************************************
NAME    
    sinkPartyModeTrackChangeIndication
    
DESCRIPTION
    Called when AVRCP has detected a change in play status or end of current playing track
    
RETURNS
    none
*/
void sinkPartyModeTrackChangeIndication(uint16 index)
{
    /* check to see if this is an indication from the currently active device */
    if((sinkPartymodeIsEnabled())&&(index == sinkAvrcpGetActiveConnection()))
    {
        if(!avrcpAvrcpIsEnabled() || !sinkAvrcpIsAvrcpAudioSwitchingEnabled())
        {
            PTY_DEBUG(("PTY: ignore AVRCP in party mode\n"));
            return;
        }
        /*  check whether to change audio routing */
	    else
        {
            /* ensure device is still streaming/playing */
			
            /* Check for change in play status on an a2dp device */
            if((sinkAvrcpGetPlayStatus(index) == avrcp_play_status_playing) ||
                  (sinkAvrcpGetPlayStatus(index) == avrcp_play_status_stopped) ||
                  (sinkAvrcpGetPlayStatus(index) == avrcp_play_status_paused))
            {
                if(BdaddrIsSame(getA2dpLinkBdAddr(a2dp_primary), sinkAvrcpGetLinkBdAddr(index)))
                {
                    /* track change on primary a2dp device, Disconnect it and resume Secondary a2dp device */
                    sinkPartymodeDisconnectPrimaryResumeSecondary(index);
                }
		        else if(BdaddrIsSame(getA2dpLinkBdAddr(a2dp_secondary), sinkAvrcpGetLinkBdAddr(index)))
		        {
                    /* track change on secondary a2dp device, Disconnect it and resume Primary a2dp device */
		            sinkPartymodeDisconnectSecondaryResumePrimary(index);
		        }
            }
        }
    }
    /* indication from device that isn't currently streaming audio, ignore */
    else
    {
        PTY_DEBUG(("PTY: track change ignored PTY[%d] index[%d] ActiveIdx[%d]\n",sinkPartymodeIsEnabled(),index,sinkAvrcpGetActiveConnection()));
    }
}

/****************************************************************************
NAME    
    sinkPartyModeSkipTrackIndication
    
DESCRIPTION
    Called when AVRCP has detected a skip track
    
RETURNS
    none
*/
void sinkPartyModeSkipTrackIndication(uint16 index)
{
    /* check to see if this is an indication from the currently active device */
    if((sinkPartymodeIsEnabled())&&(index == sinkAvrcpGetActiveConnection()))
    {
        if(!avrcpAvrcpIsEnabled() || !sinkAvrcpIsAvrcpAudioSwitchingEnabled())
        {
            PTY_DEBUG(("PTY: ignore AVRCP in party mode\n"));
            return;
        }	
        /*  check whether to change audio routing */		
        else
        {
            /* ensure device is still streaming/playing */
            if(BdaddrIsSame(getA2dpLinkBdAddr(a2dp_primary), sinkAvrcpGetLinkBdAddr(index)))
            {
                /* Skip track event on primary a2dp device, Disconnect it and resume Secondary a2dp device */
                sinkPartymodeDisconnectPrimaryResumeSecondary(index);
            }
            else if(BdaddrIsSame(getA2dpLinkBdAddr(a2dp_secondary), sinkAvrcpGetLinkBdAddr(index)))
            {
                /* Skip track event on secondary a2dp device, Disconnect it and resume Primary a2dp device */
                sinkPartymodeDisconnectSecondaryResumePrimary(index);
            }
        }
    }
    /* indication from device that isn't currently streaming audio, ignore */
    else
    {
        PTY_DEBUG(("PTY:skip track ignored PTY[%d] index[%d] ActiveIdx[%d]\n",sinkPartymodeIsEnabled(),index,sinkAvrcpGetActiveConnection()));
    }
}
				
/****************************************************************************
NAME    
    sinkPartyModeDisconnectDevice
    
DESCRIPTION
    Disconnects the AG specified by the passed in a2dp link priority 
    
RETURNS
    successful or unable to disconnect state as true or false
*/
bool sinkPartyModeDisconnectDevice(a2dp_index_t link)
{
    PTY_DEBUG(("PTY: Disc Dev[%d] Connected[%d]\n",link ,getA2dpStatusFlag(CONNECTED, link)));
    /* is a2dp connected? */
    if(getA2dpStatusFlag(CONNECTED, link))
    {
        /* disconnect a2dp primary audio */
        audioDisconnectRoutedAudio();
        /* disconnect device bluetooth link */
        A2dpSignallingDisconnectRequest(getA2dpLinkDataDeviceId(link));
        /* if hfp connected to this device then disconnect it */
        if(deviceManagerIsSameDevice(link, hfp_primary_link))
            HfpSlcDisconnectRequest(hfp_primary_link);
        else if(deviceManagerIsSameDevice(link, hfp_secondary_link))
            HfpSlcDisconnectRequest(hfp_secondary_link);

#ifdef ENABLE_MAPC
        /* Disconnect all MAPC links */
        mapcDisconnectMns();
#endif
#ifdef ENABLE_PBAP        
        /* Disconnect all PBAP links */
        pbapDisconnect();
#endif	  
        return TRUE;                                
    }
    /* didn't disconnect anything */
    else
        return FALSE;
}

/****************************************************************************
NAME    
    sinkPartyModeDisconnectAndResume
    
DESCRIPTION
    Disconnects the AG specified by the passed in a2dp link priority and
    resumes the a2dp playing
    
RETURNS
    none
*/
void sinkPartyModeDisconnectAndResume(a2dp_index_t link)
{
    /* attempt to disconnect passed in device */
    if(sinkPartyModeDisconnectDevice(link))
    {                           
        /* send avrcp play or a2dp start to resume audio and connect it */        
        if(link == a2dp_secondary)    
        {
            if(ResumeA2dpStream(a2dp_primary))
            {
                audioRouteSpecificA2dpSource(audio_source_a2dp_1);
            }
        }
        else
        {
            if(ResumeA2dpStream(a2dp_secondary))
            {
                audioRouteSpecificA2dpSource(audio_source_a2dp_2);
            }
        }
    }
}

/****************************************************************************
NAME    
    sinkPartyModeCheckForOtherPausedSource
    
DESCRIPTION
    function to look for a device other than the one passed in that is in an paused
    state
    
RETURNS
    link priority if an avrcp paused device is found
*/
a2dp_index_t sinkPartyModeCheckForOtherPausedSource(a2dp_index_t link)
{
    
    /* determine if the other device is connected and has avrcp connected*/
    if(BdaddrIsSame(getA2dpLinkBdAddr(OTHER_DEVICE(link)), sinkAvrcpGetLinkBdAddr(a2dp_primary)))
    {
        /* device is connected, check its current avrcp state, if paused then return index of device */
        if(sinkAvrcpGetPlayStatus(a2dp_primary) == avrcp_play_status_paused)
            return OTHER_DEVICE(link);
    }
    /* determine if the other device is connected and has avrcp connected*/
    else if(BdaddrIsSame(getA2dpLinkBdAddr(OTHER_DEVICE(link)), sinkAvrcpGetLinkBdAddr(a2dp_secondary)))
    {
        /* device is connected, check its current avrcp state, if paused then return index of device */        
        if(sinkAvrcpGetPlayStatus(a2dp_secondary) == avrcp_play_status_paused)
            return OTHER_DEVICE(link);
    }
    /* no paused other devices found */
    return a2dp_invalid;
}

/****************************************************************************
NAME    
    sinkUpdatePartyModeStateChangeEvent
    
DESCRIPTION
    Send party mode state change event to the sink application.
*/
void sinkUpdatePartyModeStateChangeEvent( sinkEvents_t appEvent)
{
    MessageSend(sinkGetMainTask(),appEvent, 0);
}

void sinkPartyModeUpdateOnSuspend(uint16 Id)
{
    /* check whether party mode is enabled */
    if(sinkPartymodeIsEnabled() && sinkPartymodeGetOperatingMode())
    {
        /* Cancel any existing partymode timer running for AG */
        MessageCancelAll(&theSink.task,(EventSysPartyModeTimeoutDevice1 + Id));
        /* If the suspend is triggered as part of Party Mode functionality (AVRCP Control type), i.e.
           if the suspend is triggered locally, we should not start the timer. Because we cant guarantee
           the current track which is playing would get over within the configured "sinkPartymodeGetMusicTimeOut".
           This timer should be used only when the stream is intentionally suspended by the AG which is
           waiting in the Queue or that just connect and does nothing.Hence adding the check for the suspend
           state here
        */
        if(a2dp_local_suspended != SinkA2dpGetSuspendState(Id))
        {
            /* start a timer when music is suspended in party mode, if no music is played before the timeout
               occurs the device will get disconnected to allow other devices to connect, this timer is a configurable
               item in Sink configuration tool
            */
            MessageSendLater(&theSink.task,(EventSysPartyModeTimeoutDevice1 + Id),0,D_SEC(sinkPartymodeGetMusicTimeOut()));
        }
    }
}
#endif

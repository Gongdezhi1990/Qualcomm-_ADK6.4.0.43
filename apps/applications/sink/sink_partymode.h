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

#ifndef _HS_PARTYMODE_H_
#define _HS_PARTYMODE_H_

#include "sink_audio_routing.h"
#include "sink_a2dp.h"

/* flag to indicate that audio source has been paused and is queued for playing
   when current flag is finished */
typedef struct    
{
    unsigned audio_source_primary_paused:1;
    unsigned audio_source_secondary_paused:1;                                
}pty_pause_state_t;


#ifdef ENABLE_PARTYMODE

typedef enum
{
    partymode_none,
    partymode_barge_in,
    partymode_avrcp_control    
}PartyModeOperatingMode;

typedef enum
{
    partymode_primary = 0x00,
    partymode_secondary = 0x01
} PartyModeAudioSource;


/*************************************************************************
NAME    
    sinkPartymodeGetOperatingMode
    
DESCRIPTION
    Function to get operating mode
    
RETURNS
    Configured operating Partymode : Either none /barge-in /avrcp control state
*/
PartyModeOperatingMode sinkPartymodeGetOperatingMode(void);

/*************************************************************************
NAME
    sinkPartymodeTurnOn
    
DESCRIPTION
    Function to turn on Partymode
*/
void sinkPartymodeTurnOn(void);

/*************************************************************************
NAME
    sinkPartymodeTurnOff
    
DESCRIPTION
    Function to turn off Partymode
*/
void sinkPartymodeTurnOff(void);

/*************************************************************************
NAME
    sinkPartymodePauseAudio
    
DESCRIPTION
    Function to set global Partymode flag to indicate whether audio source primary or 
    secondary has been paused and is queued for playing
*/
void sinkPartymodePauseAudio(PartyModeAudioSource audio_type, bool value);

/****************************************************************************
NAME    
    sinkPartyModeTrackChangeIndication
    
DESCRIPTION
    Called when AVRCP has detected a change in play status or end of current playing track
    
RETURNS
    none
*/
void sinkPartyModeTrackChangeIndication(uint16 index);

/****************************************************************************
NAME    
    sinkPartyModeSkipTrackIndication
    
DESCRIPTION
    Called when AVRCP has detected a skip track
    
RETURNS
    none
*/
void sinkPartyModeSkipTrackIndication(uint16 index);

/****************************************************************************
NAME    
    sinkPartyModeDisconnectDevice
    
DESCRIPTION
    Disconnects the AG specified by the passed in a2dp link priority 
    
RETURNS
    successful or unable to disconnect state as true or false
*/
bool sinkPartyModeDisconnectDevice(a2dp_index_t link);

/****************************************************************************
NAME    
    sinkPartyModeDisconnectAndResume
    
DESCRIPTION
    Disconnects the AG specified by the passed in a2dp link priority and
    resumes the a2dp playing
    
RETURNS
    none
*/
void sinkPartyModeDisconnectAndResume(a2dp_index_t link);

/****************************************************************************
NAME    
    sinkPartyModeCheckForOtherPausedSource
    
DESCRIPTION
    function to look for a device other than the one passed in that is in an paused
    state
    
RETURNS
    link priority if an avrcp paused device is found
*/
a2dp_index_t sinkPartyModeCheckForOtherPausedSource(a2dp_index_t link);


/* ENABLE_PARTYMODE */
#endif

/*************************************************************************
NAME
    sinkPartymodeIsEnabled
    
DESCRIPTION
    Function to get global Partymode Enabled flag if set or not
    
RETURNS
    Partymode Enabled status
*/
#ifdef ENABLE_PARTYMODE
bool sinkPartymodeIsEnabled (void);
#else
#define sinkPartymodeIsEnabled() (FALSE)
#endif /* ENABLE_PARTYMODE */

/****************************************************************************
NAME
    sinkUpdatePartyModeStateChangeEvent
    
DESCRIPTION
    Update partymode  state change event to the sink application.
*/
#ifdef ENABLE_PARTYMODE
void sinkUpdatePartyModeStateChangeEvent( sinkEvents_t appEvent);
#else
#define sinkUpdatePartyModeStateChangeEvent(appEvent) ((void) (0))
#endif /* ENABLE_PARTYMODE */

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
#ifdef ENABLE_PARTYMODE
bool sinkCheckPartyModeAudio(void);
#else
#define sinkCheckPartyModeAudio() (FALSE)
#endif /* ENABLE_PARTYMODE */


/*************************************************************************
NAME    
    sinkPartymodeGetMusicTimeOut
    
DESCRIPTION
    Function to get configured music timeout

RETURNS
    Configured Party Mode music timeout value
*/
#ifdef ENABLE_PARTYMODE
uint16 sinkPartymodeGetMusicTimeOut(void);
#else
#define sinkPartymodeGetMusicTimeOut() ((uint16)0)
#endif /* ENABLE_PARTYMODE */

/*************************************************************************
NAME    
    sinkPartymodeSetMusicTimeOut
    
DESCRIPTION
    Function to Set configured music timeout

PARAMS
    timeout  Party Mode music timeout value
*/
#ifdef ENABLE_PARTYMODE
bool sinkPartymodeSetMusicTimeOut(uint16 timeout);
#else
#define sinkPartymodeSetMusicTimeOut(timeout) (FALSE)
#endif /* ENABLE_PARTYMODE */

/*************************************************************************
NAME
    sinkPartymodeGetStreamResumeTimeOut
    
DESCRIPTION
    Function to get configured stream resume timeout 
    
RETURNS
    Configured Party Mode stream resume timeout value
*/
#ifdef ENABLE_PARTYMODE
uint16 sinkPartymodeGetStreamResumeTimeOut(void);
#else
#define sinkPartymodeGetStreamResumeTimeOut() ((uint16)0)
#endif /* ENABLE_PARTYMODE */

/*************************************************************************
NAME
    sinkPartymodeSetStreamResumeTimeOut
    
DESCRIPTION
    Function to set configured stream resume timeout 
    
PARAMS
    timeout  Party Mode stream resume timeout value
*/
#ifdef ENABLE_PARTYMODE
bool sinkPartymodeSetStreamResumeTimeOut(uint16 timeout);
#else
#define sinkPartymodeSetStreamResumeTimeOut(timeout) (FALSE)
#endif /* ENABLE_PARTYMODE */

/*************************************************************************
NAME
    sinkPartyModeUpdateOnSuspend

DESCRIPTION
    Update party mode on an A2DP suspend

PARAMS
    Id of connection

*/
#ifdef ENABLE_PARTYMODE
void sinkPartyModeUpdateOnSuspend(uint16 Id);
#else
#define sinkPartyModeUpdateOnSuspend(Id) (UNUSED(Id))
#endif /* ENABLE_PARTYMODE */

/* _HS_PARTYMODE_H_ */
#endif


/*
Copyright (c) 2015 - 2016 Qualcomm Technologies International, Ltd.

*/
/** 
\file 
\ingroup sink_app
\brief Support for Active Noise Cancellation (ANC). 

The ANC feature is included in
    an add-on installer and is only supported on CSR8675.
*/

#ifndef _SINK_ANC_H_
#define _SINK_ANC_H_


#include <csrtypes.h>


#define ANC_SINK ((Sink)0xFFFD)


#ifdef ENABLE_ANC

#include <anc.h>

#include "sink_audio_routing.h"
        
/* Sink ANC States */
typedef enum
{
    sink_anc_state_uninitialised,
    sink_anc_state_power_off,
    sink_anc_state_enabled,
    sink_anc_state_disabled,
    sink_anc_state_disconnecting_audio,
	sink_anc_state_tuning_mode_active
} sink_anc_state;

#else

#define SINK_ANC_DATA(anc)

#endif


/****************************************************************************
NAME
    sinkAncInit

DESCRIPTION
    Initialisation of ANC feature.
*/    
#ifdef ENABLE_ANC
void sinkAncInit(void);
#else
#define sinkAncInit() ((void)(0))
#endif


/****************************************************************************
NAME
    sinkAncHandlePowerOn

DESCRIPTION
    ANC specific handling due to the device Powering On.
*/    
#ifdef ENABLE_ANC
void sinkAncHandlePowerOn(void);
#else
#define sinkAncHandlePowerOn() ((void)(0))
#endif


/****************************************************************************
NAME
    sinkAncHandlePowerOff

DESCRIPTION
    ANC specific handling due to the device Powering Off.
*/    
#ifdef ENABLE_ANC
void sinkAncHandlePowerOff(void);
#else
#define sinkAncHandlePowerOff() ((void)(0))
#endif


/****************************************************************************
NAME
    sinkAncEnable

DESCRIPTION
    Enable ANC functionality.
*/    
#ifdef ENABLE_ANC
void sinkAncEnable(void);
#else
#define sinkAncEnable() ((void)(0))
#endif


/****************************************************************************
NAME
    sinkAncDisable

DESCRIPTION
    Disable ANC functionality.
*/    
#ifdef ENABLE_ANC
void sinkAncDisable(void);
#else
#define sinkAncDisable() ((void)(0))
#endif

/****************************************************************************
NAME
    sinkAncSetMode

DESCRIPTION
    Set the operating mode of ANC to configured mode_n.
*/    
#ifdef ENABLE_ANC
void sinkAncSetMode(anc_mode_t mode);
#else
#define sinkAncSetMode(x) ((void)(0))
#endif


/****************************************************************************
NAME
    sinkAncGetMode

DESCRIPTION
    Get the Anc mode configured.
*/    
#ifdef ENABLE_ANC
anc_mode_t sinkAncGetMode(void);
#else
#define sinkAncGetMode() (0)
#endif

/****************************************************************************
NAME
    sinkAncVolumeDown

DESCRIPTION
    Decrease the ANC Volume.
*/    
#ifdef ENABLE_ANC
void sinkAncVolumeDown(void);
#else
#define sinkAncVolumeDown() ((void)(0))
#endif


/****************************************************************************
NAME
    sinkAncVolumeUp

DESCRIPTION
    Increase the ANC Volume.
*/    
#ifdef ENABLE_ANC
void sinkAncVolumeUp(void);
#else
#define sinkAncVolumeUp() ((void)(0))
#endif

/****************************************************************************
NAME
    sinkAncCycleAdcDigitalGain

DESCRIPTION
    Cycle through the ADC digital gain for fine tuning.
*/    
#ifdef ENABLE_ANC
void sinkAncCycleAdcDigitalGain(void);
#else
#define sinkAncCycleAdcDigitalGain() ((void)(0))
#endif

/****************************************************************************
NAME
    sinkAncProcessEvent

DESCRIPTION
    Processes user and system events specific to anc

*/
#ifdef ENABLE_ANC
bool sinkAncProcessEvent(const MessageId anc_event);
#else
#define sinkAncProcessEvent(x) (FALSE)
#endif

/****************************************************************************
NAME
    sinkAncIsEnabled

DESCRIPTION
    Checks if ANC is due to be enabled

*/
#ifdef ENABLE_ANC
bool sinkAncIsEnabled (void);
#else
#define sinkAncIsEnabled() (FALSE)
#endif

/****************************************************************************
NAME
    sinkAncSychroniseStateWithPeer

DESCRIPTION
    Configure ANC settings when a Peer connects

*/
#ifdef ENABLE_ANC
void sinkAncSychroniseStateWithPeer(void);
#else
#define sinkAncSychroniseStateWithPeer() ((void)(0))
#endif

/******************************************************************************
NAME
    sinkAncGetNextState

DESCRIPTION
    Checks the next enabled/disabled state of ANC
*/
#ifdef ENABLE_ANC
MessageId sinkAncGetNextState(void);
#else
#define sinkAncGetNextState() (0)
#endif

/******************************************************************************
NAME
    sinkAncGetNextMode

DESCRIPTION
    Checks the next mode of ANC
*/
#ifdef ENABLE_ANC
anc_mode_t sinkAncGetNextMode(anc_mode_t mode);
#else
#define sinkAncGetNextMode(x) (0)
#endif

/******************************************************************************
NAME
    sinkAncGetUsrEventFromAncMode

DESCRIPTION
     Get user event for the corresponding Anc mode.
*/
#ifdef ENABLE_ANC
MessageId sinkAncGetUsrEventFromAncMode(anc_mode_t mode);
#else
#define sinkAncGetUsrEventFromAncMode(x) (0)
#endif

/******************************************************************************
NAME
    sinkAncIsTuningModeActive

DESCRIPTION
    Checks whether tuning mode is currently active
*/
#ifdef ENABLE_ANC
bool sinkAncIsTuningModeActive(void);
#else
#define sinkAncIsTuningModeActive() (FALSE)
#endif

/****************************************************************************
NAME
    sinkResetAncStateMachine

DESCRIPTION
    Test hook for unit tests to reset the Sink ANC state

*/
#ifdef ANC_TEST_BUILD

#ifdef ENABLE_ANC
void sinkAncResetStateMachine(sink_anc_state anc_state);
#else
#define sinkAncResetStateMachine(x) ((void)(0))
#endif

#endif /* ANC_TEST_BUILD*/

#endif /* _SINK_ANC_H_ */

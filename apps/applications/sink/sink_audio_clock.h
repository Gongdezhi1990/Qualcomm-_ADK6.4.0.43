/*
Copyright (c) 2017 Qualcomm Technologies International, Ltd.
*/

/*!
\file    
\ingroup sink_app
\brief   Interface to the audio clock functionality. 
*/

#ifndef _SINK_AUDIO_CLOCK_H_
#define _SINK_AUDIO_CLOCK_H_

/*************************************************************************
DESCRIPTION
    Set up audio clock configuration
**************************************************************************/
#ifdef ENABLE_AUDIO_CLOCK
void sinkAudioClockInit(void);
#else
#define sinkAudioClockInit() ((void)0)
#endif
#endif /* _SINK_AUDIO_CLOCK_H_ */


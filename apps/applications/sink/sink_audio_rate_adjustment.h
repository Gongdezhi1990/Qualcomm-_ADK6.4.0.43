/*
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*/

/*!
\file
\ingroup sink_app
\brief   Set the time-to-play parameters for A2DP and TWS.
*/

#ifndef _SINK_AUDIO_RATE_ADJUSTMENT_H_
#define _SINK_AUDIO_RATE_ADJUSTMENT_H_

/*************************************************************************
DESCRIPTION
    Function to initialize standalone rate adjustment from readonly config, and updates the
audio_config.
**************************************************************************/
#ifdef ENABLE_STANDALONE_RATE_ADJUSTMENT
void sinkAudioRateAdjustmentInit(void);
#else
#define sinkAudioRateAdjustmentInit() ((void)0)
#endif
#endif /* _SINK_AUDIO_RATE_ADJUSTMENT_H_ */


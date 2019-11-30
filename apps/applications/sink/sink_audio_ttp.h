/*
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.
*/

/*!
\file
\ingroup sink_app
\brief   Set the time-to-play parameters for A2DP and TWS.
*/

#ifndef _SINK_AUDIO_TTP_H_
#define _SINK_AUDIO_TTP_H_

/*************************************************************************
DESCRIPTION
    Set up audio ttp configuration
**************************************************************************/
#ifdef ENABLE_AUDIO_TTP
void sinkAudioTtpLatencyParamsInit(void);
#else
#define sinkAudioTtpLatencyParamsInit() ((void)0)
#endif

#endif /* _SINK_AUDIO_TTP_H_ */

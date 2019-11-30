/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_output_tws_ctx.h
 
DESCRIPTION
    Set and get the input task.
*/

#ifndef AUDIO_OUTPUT_TWS_CTX_H_
#define AUDIO_OUTPUT_TWS_CTX_H_

#include <message.h>

void AudioOutputTwsSetInputTask(Task task);

Task AudioOutputTwsGetInputTask(void);

#endif /* AUDIO_OUTPUT_TWS_CTX_H_ */

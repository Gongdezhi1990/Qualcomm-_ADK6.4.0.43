/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_spdif_driver.h

DESCRIPTION
    Implementation of spdif driver which is responsible for
    handling drives specific messages.
*/

#ifndef AUDIO_INPUT_SPDIF_DRIVER_H_
#define AUDIO_INPUT_SPDIF_DRIVER_H_

#include <message.h>


Task AudioInputSpdifGetDriverHandlerTask(void);
void AudioInputSpdifSetTargetLatency(void);


#endif /* AUDIO_INPUT_SPDIF_DRIVER_H_ */

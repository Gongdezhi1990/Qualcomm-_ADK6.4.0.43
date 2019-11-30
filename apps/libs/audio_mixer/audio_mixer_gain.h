/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_mixer_gain.h

DESCRIPTION
    Gain values used by the audio mixer
*/

#ifndef _AUDIO_MIXER_GAIN_H_
#define _AUDIO_MIXER_GAIN_H_

#include <gain_utils.h>

#define GAIN_MAX                GainIn60thdB(90)
#define GAIN_UNITY              GainIn60thdB(0)
#define GAIN_MIN                GainIn60thdB(-90)

#define GAIN_BACKGROUND_MIXING  GainIn60thdB(-30)
#define GAIN_BACKGROUND         GainIn60thdB(-20)
#define GAIN_FOREGROUND         GainIn60thdB(-1)
#define GAIN_DOWNMIX            GainIn60thdB(-6)

#endif /* _AUDIO_MIXER_GAIN_H_ */

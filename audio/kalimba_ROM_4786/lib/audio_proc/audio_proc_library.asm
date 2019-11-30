// *****************************************************************************
// Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Audio Process Library
//
// DESCRIPTION:
//    This library contains the following modules:
//       - delay
//       - peq: Parametric Equalizer
//       - peak signal monitor
//       - stream gain
//       - stream mixer
//
// *****************************************************************************

#ifndef AUDIO_PROC_LIBRARY_INCLUDED
#define AUDIO_PROC_LIBRARY_INCLUDED

#ifndef KYMERA
#include "flash.h"
#endif
#include "cmpd100.asm"
#include "delay.asm"
#include "mute_control.asm"
#include "peak_monitor.asm"
#include "peq.asm"
#include "stereo_3d_enhancement.asm"
#include "stereo_copy.asm"
#include "stream_gain.asm"
#include "stream_mixer.asm"
#include "mic_grouping.asm"
#include "dbe.asm"
#include "vse.asm"
#include "xover.asm"
#include "compander.asm"

#endif // AUDIO_PROC_LIBRARY_INCLUDED

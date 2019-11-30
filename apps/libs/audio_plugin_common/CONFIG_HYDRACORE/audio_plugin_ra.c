/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_plugin_common.c
    
DESCRIPTION
    Implementation of standalone rate adjustment related functions for audio plugin common library (specific to hydracore platform).
*/

#include <stream.h>
#include <source.h>

#include "audio_plugin_if.h"
#include "audio_plugin_common.h"

/****************************************************************************
DESCRIPTION
    Configure to use standalone rate adjustment capability
*/
void AudioPluginUseStandaloneRateAdjustment(Source source, uint32 op)
{
    PanicFalse(SourceConfigure(source, STREAM_RM_USE_RATE_ADJUST_OPERATOR, op));
}

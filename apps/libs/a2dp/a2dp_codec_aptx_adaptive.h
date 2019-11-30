/****************************************************************************
Copyright (c) 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_codec_aptx_adaptive.h

DESCRIPTION
    This file contains aptX Adaptive specific code.

*/

#ifndef A2DP_CODEC_APTX_AD_H_
#define A2DP_CODEC_APTX_AD_H_

#ifndef A2DP_SBC_ONLY

/*************************************************************************
NAME
     areAptxAdCodecsCompatible

DESCRIPTION
    Checks whether the sink and source aptX Adaptive service capabilities are compatible.
*/
bool areAptxAdCodecsCompatible(const uint8 *local_caps, const uint8 *remote_caps, uint8 local_losc, uint8 remote_losc);

/*************************************************************************
NAME
     selectOptimalAptxAdCapsSink

DESCRIPTION
    Selects the optimal configuration for aptX Adaptive playback by setting a single
    bit in each field of the passed caps structure.

    Note that the priority of selecting features is a
    design decision and not specified by the A2DP profiles.

*/
void selectOptimalAptxAdCapsSink(const uint8 *local_caps, uint8 *remote_caps);

/*************************************************************************
NAME
     selectOptimalAptxAdCapsSource

DESCRIPTION
    Selects the optimal configuration for aptX Adaptive playback by setting a single
    bit in each field of the passed caps structure.

    Note that the priority of selecting features is a
    design decision and not specified by the A2DP profiles.

*/
void selectOptimalAptxAdCapsSource(const uint8 *local_caps, uint8 *remote_caps);

/*************************************************************************
NAME
     getAptxAdConfigSettings

DESCRIPTION
    Return the codec configuration settings (rate and channel mode) for the physical codec based
    on the A2DP codec negotiated settings.
*/
void getAptxAdConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings);

#endif  /* A2DP_SBC_ONLY */

#endif  /* A2DP_CODEC_APTX_AD_H_ */

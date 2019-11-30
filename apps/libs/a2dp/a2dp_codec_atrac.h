/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_codec_atrac.h

DESCRIPTION

*/

#ifndef A2DP_CODEC_ATRAC_H_
#define A2DP_COCEC_ATRAC_H_


#ifdef INCLUDE_ATRAC


/*************************************************************************
NAME
    getAtracConfigSettings

DESCRIPTION
    Return the codec configuration settings (rate and channel mode) for the physical codec based
    on the A2DP codec negotiated settings.

*/
void getAtracConfigSettings(const uint8 *service_caps, a2dp_codec_settings *codec_settings);


/*************************************************************************
NAME
     selectOptimalAtracCapsSink

DESCRIPTION
    Selects the optimal configuration for ATRAC playback by setting a single
    bit in each field of the passed caps structure.

    Note that the priority of selecting features is a
    design decision and not specified by the A2DP profiles.

*/
void selectOptimalAtracCapsSink(const uint8 *local_codec_caps, uint8 *remote_codec_caps);


#endif /* INCLUDE_ATRAC */


#endif /* A2DP_CODEC_ATRAC_H_ */

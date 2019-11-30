/*
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file    sink_a2dp_capabilities.h
@brief   The data structures used for the capability exchange with a source device.
*/

#ifndef _SINK_A2DP_CAPABILITIES_H_
#define _SINK_A2DP_CAPABILITIES_H_

#include <csrtypes.h>

#define SPLIT_IN_2_OCTETS(x) ((x) >> 8)  & 0xFF, (x) & 0xFF
#define SPLIT_IN_4_OCTETS(x) ((x) >> 24) & 0xFF, ((x) >> 16) & 0xFF, ((x) >> 8) & 0xFF, (x) & 0xFF

#define RESERVED                      (0x00)
#define LENGTH_OF_CP_TYPE_SCMS_VALUE  (2)
#define OFFSET_FOR_LENGTH_OF_TWS_CAPS (10)

extern const uint8 sbc_caps_sink[16];
extern const uint8 sbc_med_caps_sink[16];
extern const uint8 mp3_caps_sink[16];
extern const uint8 aac_caps_sink[18];
extern const uint8 faststream_caps_sink[14];
extern const uint8 aptx_caps_sink[19];
extern const uint8 aptx_acl_sprint_caps_sink[27];
extern const uint8 aptx_ll_caps_sink[27];
extern const uint8 aptxhd_caps_sink[23];

extern const uint8 tws_sbc_caps[26];
extern const uint8 tws_mp3_caps[26];
extern const uint8 tws_aac_caps[28];
extern const uint8 tws_aptx_caps[29];

#endif /* _SINK_A2DP_CAPABILITIES_H_ */

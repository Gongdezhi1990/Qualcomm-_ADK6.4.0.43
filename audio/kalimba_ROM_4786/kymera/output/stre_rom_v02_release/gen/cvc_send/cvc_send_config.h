// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __CVC_SEND_CONFIG_H__
#define __CVC_SEND_CONFIG_H__

#define uses_DCBLOCK 	1
#define uses_SND_PEQ 	1
#define uses_NSVOLUME	1
#define uses_SND_AGC 	1
#define uses_SND_NS  	1
#define uses_ADF     	0
#define uses_NC      	1
#define uses_AEC     	1
#define uses_VCAEC   	1
#define uses_MGDC    	1
#define uses_ASF     	1


#define flag_uses_DCBLOCK 	0x00004000
#define flag_uses_SND_PEQ 	0x00001000
#define flag_uses_NSVOLUME	0x00000800
#define flag_uses_SND_AGC 	0x00000400
#define flag_uses_SND_NS  	0x00000200
#define flag_uses_ADF     	0x00000100
#define flag_uses_NC      	0x00000080
#define flag_uses_AEC     	0x00000008
#define flag_uses_VCAEC   	0x00000004
#define flag_uses_MGDC    	0x00000002
#define flag_uses_ASF     	0x00000001



// Bit-mask flags to return in SPI status, which tells the Parameter Manager
// (Windows Realtime Tuning GUI) which modules are included in the build.  The
// mask is also written to the kap file so that the build configuration can be
// identified using a text editor.

#define CVC_SEND_CONFIG_FLAG	uses_DCBLOCK*flag_uses_DCBLOCK +  uses_SND_PEQ*flag_uses_SND_PEQ +  uses_NSVOLUME*flag_uses_NSVOLUME +  uses_SND_AGC*flag_uses_SND_AGC +  uses_SND_NS*flag_uses_SND_NS +  uses_ADF*flag_uses_ADF +  uses_NC*flag_uses_NC +  uses_AEC*flag_uses_AEC +  uses_VCAEC*flag_uses_VCAEC +  uses_MGDC*flag_uses_MGDC +  uses_ASF*flag_uses_ASF


#endif // __CVC_SEND_CONFIG_H__

// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __AEC_REFERENCE_CONFIG_H__
#define __AEC_REFERENCE_CONFIG_H__

#define uses_SIDETONE     	1
#define uses_AEC_REFERENCE	1


#define flag_uses_SIDETONE     	0x00008000
#define flag_uses_AEC_REFERENCE	0x00010000



// Bit-mask flags to return in SPI status, which tells the Parameter Manager
// (Windows Realtime Tuning GUI) which modules are included in the build.  The
// mask is also written to the kap file so that the build configuration can be
// identified using a text editor.

#define AEC_REFERENCE_CONFIG_FLAG	uses_SIDETONE*flag_uses_SIDETONE +  uses_AEC_REFERENCE*flag_uses_AEC_REFERENCE


#endif // __AEC_REFERENCE_CONFIG_H__

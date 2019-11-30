// -----------------------------------------------------------------------------
// Copyright (c) 2017                  Qualcomm Technologies International, Ltd.
//
#ifndef __CVC_RECV_CONFIG_H__
#define __CVC_RECV_CONFIG_H__

#define uses_RCV_AGC	1
#define uses_DCBLOCK	1
#define uses_RCV_PEQ	1
#define uses_RCV_NS 	1
#define uses_AEQ    	1


#define flag_uses_RCV_AGC	0x00008000
#define flag_uses_DCBLOCK	0x00004000
#define flag_uses_RCV_PEQ	0x00002000
#define flag_uses_RCV_NS 	0x00000040
#define flag_uses_AEQ    	0x00000020



// Bit-mask flags to return in SPI status, which tells the Parameter Manager
// (Windows Realtime Tuning GUI) which modules are included in the build.  The
// mask is also written to the kap file so that the build configuration can be
// identified using a text editor.

#define CVC_RECV_CONFIG_FLAG	uses_RCV_AGC*flag_uses_RCV_AGC +  uses_DCBLOCK*flag_uses_DCBLOCK +  uses_RCV_PEQ*flag_uses_RCV_PEQ +  uses_RCV_NS*flag_uses_RCV_NS +  uses_AEQ*flag_uses_AEQ


#endif // __CVC_RECV_CONFIG_H__

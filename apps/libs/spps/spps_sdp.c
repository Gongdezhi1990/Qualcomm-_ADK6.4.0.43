/* Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd. */
/*  

FILE NAME
	spp_sdp.c        

DESCRIPTION
	Functions used storing the SPP sdp_handler state.

*/

#include "spps_private.h"

/* Default Spp service record */

const uint8 spp_service_record [SSP_SERVICE_RECORD_SIZE] =
{
    0x09, 0x00, 0x01,           /* ServiceClassIDList(0x0001) */
    0x35, 0x03,                 /* DataElSeq 3 bytes */
    0x19, 0x11, 0x01,           /* UUID SerialPort(0x1101) */
    0x09, 0x00, 0x04,           /* ProtocolDescriptorList(0x0004) */
    0x35, 0x0c,                 /* DataElSeq 12 bytes */
    0x35, 0x03,                 /* DataElSeq 3 bytes */
    0x19, 0x01, 0x00,           /* UUID L2CAP(0x0100) */
    0x35, 0x05,                 /* DataElSeq 5 bytes */
    0x19, 0x00, 0x03,           /* UUID RFCOMM(0x0003) */
    0x08, SPP_DEFAULT_CHANNEL,  /* uint8 Suggested RFCOMM channel for SPP */
    0x09, 0x00, 0x06,           /* LanguageBaseAttributeIDList(0x0006) */
    0x35, 0x09,                 /* DataElSeq 9 bytes */
    0x09, 0x65, 0x6e,           /* uint16 0x656e */
    0x09, 0x00, 0x6a,           /* uint16 0x006a */
    0x09, 0x01, 0x00,           /* uint16 0x0100 */
    0x09, 0x00, 0x09,           /* BluetoothProfileDescriptorList(0x0009) */
    0x35, 0x06,                 /* DataElSeq 3 bytes */
    0x19, 0x11, 0x01,           /* UUID SerialPort(0x1101) */   
    0x09, 0x01, 0x02,           /* SerialPort Version (0x0102) */
    0x09, 0x01, 0x00,           /* ServiceName(0x0100) = "SPP Dev" */
    0x25, 0x07,                 /* String length 7 */
    'S','P','P',' ','D', 'e', 'v'
};

static uint32 spp_sdp_service_handle = 0;

void sppStoreServiceHandle(uint32 service_handle)
{
    spp_sdp_service_handle = service_handle;
}

uint32 sppGetServiceHandle(void)
{
    return spp_sdp_service_handle;
}



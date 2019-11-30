/*****************************************************************
Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_iap2_rtt.h

DESCRIPTION
   Header file for iAP2 Round Trip Test mini application
*/

#ifndef _SINK_IAP2_RTT_H_
#define _SINK_IAP2_RTT_H_

typedef enum {
    VERSION,
    ECHO_PKT,
    READ_IO,
    WRITE_IO,
    DELAY,
    REPEAT,
    MAX_PAYLOAD
} IAPTESTCMDS;


void sinkIap2RTTHandleIncomingData(uint16 transport_id, const uint8 *in_payload, uint16 length);


/*! \brief Init data on iOS app.
*/
void sinkIap2RTTInternalDataInit(uint16 protocol_id);

#endif

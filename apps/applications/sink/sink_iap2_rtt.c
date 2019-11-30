/*****************************************************************
Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_iap2_rtt.c

DESCRIPTION
....Basic Round Trip Test application
*/

#include "sink_accessory.h"

#ifdef ENABLE_IAP2

#include <iap2.h>
#include <transport_manager.h>
#include "sink_iap2_rtt.h"
#include "sink_iap2_transport.h"

#define IAP_INCOMING_DATA_LEN 104
#define IAP_MAX_PAYLOAD_LEN 10

static const uint8 iap_version[2] = { VERSION, 0x55 };

/*! \brief Init data on iOS app.
*/
void sinkIap2RTTInternalDataInit(uint16 protocol_id)
{
    TransportMgrWriteData(transport_mgr_type_accessory, protocol_id, (uint8 *) iap_version, sizeof iap_version);
}


void sinkIap2RTTHandleIncomingData(uint16 protocol_id, const uint8 *in_payload, uint16 length)
{
    uint8 *payload;
   
    if (length > IAP_INCOMING_DATA_LEN)
    {
        IAP2_DEBUG(("INCOMING DATA TOO LONG\n"));
        return;
    }

    if (in_payload[0] != ECHO_PKT)
    {
        IAP2_DEBUG(("BAD MESSAGE HEADER\n"));
        return;
    }

    payload = malloc(length);
    if (payload == NULL)
    {
        IAP2_DEBUG(("sinkIap2RTTHandleIncomingData: Malloc failed\n"));
        /* sinkIap2DisconnectRequest(info.link); */
        return;
    }

    memcpy(payload, in_payload, length);
    payload[length-1] = 0;

    TransportMgrWriteData(transport_mgr_type_accessory, protocol_id, payload, length);
    free(payload);
}

#endif

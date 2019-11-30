/*****************************************************************
Copyright (c) 2013 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_iap2_transport.c

DESCRIPTION
....Implements transport layer for connecting to a Bluetooth transport
*/


#include "sink_accessory.h"
#include "sink_devicemanager.h"

#ifdef ENABLE_IAP2

#include "sink_iap2_transport.h"
#include <library.h>
#include <vmal.h>

static const iap2_link_param preferred_link_param =
{
    7,                      /* MaximumNumberofPackets */
    MAX_EAP_PKT_SIZE + 12,  /* MaximumPacketLengthRX */
    1000,                   /* RetransmissionTimeout */
    73,                     /* CumulativeAcknowledgementTimeout */
    10,                     /* MaxNumberofRetransmissions */
    4,                      /* MaximumCumulativeAcknowledgements */
    FALSE,                  /* hasFileTransferSession */
    TRUE,                   /* hasExternalAccessorySession */
    TRUE,                   /* supportZeroAckLink */
};

static void sinkIap2UsbConnectRequest(void)
{
}

static void sinkIap2UsbDisconnectRequest(void)
{
}

void sinkIap2ConnectRequest(bdaddr * pAddr)
{
    if(sinkAccessoryGetTransport() == iap2_transport_bluetooth)
    {
         Iap2ConnectRequest(pAddr , D_MIN(1), &preferred_link_param);
    }
    else
    {
         /*  USB transport , when USB only one link should work with the connected device ?*/
         sinkIap2UsbConnectRequest();
    }
}

void sinkIap2ConnectResponse(iap2_link *link, bool accept)
{
     if(accept)
     {
         Iap2ConnectResponse(link, accept, &preferred_link_param);
     }
     else
     {
        Iap2ConnectResponse(link, accept, NULL);
     }

}
void sinkIap2DisconnectRequest(iap2_link *link)
{
    if(sinkAccessoryGetTransport() == iap2_transport_bluetooth)
    {
         Iap2DisconnectRequest(link);
    }
    else
    {
         /*  USB transport , when USB only one link should work with the connected device ?*/
         sinkIap2UsbDisconnectRequest();
    }
}

#endif

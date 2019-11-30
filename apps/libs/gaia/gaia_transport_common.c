/*****************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.
*/

#include <panic.h>
#include <source.h>
#include "gaia_private.h"
#include "gaia_transport.h"
#include "gaia_transport_common.h"

/*! @brief Utility function to construct a GAIA SDP record
 *
 *  @param record The constant record to use as a base
 *  @param size_record The size of the base record
 *  @param channel_offset The channel offset in the base record
 *  @param channel The channel to advertise in the SDP record
 */
const uint8* gaiaTransportCommonServiceRecord_c(const uint8* record, uint16 size_record, uint8 channel_offset, uint8 channel)
{
    uint8* sr;

    /* If channel in record matches, nothing needs to be done  */
    if(channel == record[channel_offset])
        return record;
    
    /* Allocate a dynamic record */
    sr = malloc(size_record);
    
    if (sr == NULL)
    {
        GAIA_DEBUG(("Out of memory (sr)\n"));
        return NULL;
    }
    
    /* Copy in the record and set the channel */
    memcpy(sr, record, size_record);
    sr[channel_offset] = channel;
    return (const uint8*) sr;
}

/*! @brief Utility function to send a GAIA_CONNECT_CFM message to client task.
 *
 *  @param transport The gaia transport on which the event occurred.
 *  @param success Boolean indicating success (TRUE) or failure (FALSE) of connection attempt.
 */
void gaiaTransportCommonSendGaiaConnectCfm(gaia_transport *transport, bool success)
{
    MESSAGE_PMAKE(gcc, GAIA_CONNECT_CFM_T);
    gcc->transport = (GAIA_TRANSPORT*)transport;
    gcc->success = success;
    MessageSend(gaia->app_task, GAIA_CONNECT_CFM, gcc);
}

/*! @brief Utility function to send a GAIA_CONNECT_IND message to client task.
 *
 *  @param transport The gaia transport on which the event occurred.
 *  @param success Boolean indicating success (TRUE) or failure (FALSE) of connection attempt.
 */
void gaiaTransportCommonSendGaiaConnectInd(gaia_transport *transport, bool success)
{
    MESSAGE_PMAKE(gci, GAIA_CONNECT_IND_T);
    gci->transport = (GAIA_TRANSPORT*)transport;
    gci->success = success;
    MessageSend(gaia->app_task, GAIA_CONNECT_IND, gci);
}

/*! @brief Utility function to send a GAIA_DISCONNECT_IND message to client task.
 *
 *  @param transport The gaia transport on which the event occurred.
 */
void gaiaTransportCommonSendGaiaDisconnectInd(gaia_transport *transport)
{
    MESSAGE_PMAKE(gdi, GAIA_DISCONNECT_IND_T);
    gdi->transport = (GAIA_TRANSPORT*)transport;
    MessageSend(gaia->app_task, GAIA_DISCONNECT_IND, gdi);
}

/*! @brief Utility function to send a GAIA_DISCONNECT_CFM message to client task.
 *
 *  @param transport The gaia transport on which the event occurred.
 */
void gaiaTransportCommonSendGaiaDisconnectCfm(gaia_transport *transport)
{
    MESSAGE_PMAKE(gdc, GAIA_DISCONNECT_CFM_T);
    gdc->transport = (GAIA_TRANSPORT*)transport;
    MessageSend(gaia->app_task, GAIA_DISCONNECT_CFM, gdc);
}

/*! @brief Utility function to send a GAIA_SEND_PACKET_CFM message to client task.
 *
 *  @param transport The gaia transport on which the event occurred.
 *  @param packet Pointer to the packet data that was transmitted.
 *  @param success Boolean indicating success or failure of Gaia command transmission.
 */
void gaiaTransportCommonSendGaiaSendPacketCfm(gaia_transport *transport, uint8 *packet, bool success)
{
    MESSAGE_PMAKE(gspc, GAIA_SEND_PACKET_CFM_T);
    gspc->transport = (GAIA_TRANSPORT*)transport;
    gspc->packet = packet;
    gspc->success = success;
    MessageSend(gaia->app_task, GAIA_SEND_PACKET_CFM, gspc);
}


/*! @brief Utility function to send a GAIA_START_SERVICE_CFM message to client task.
 *
 *  @param transport_type The GAIA transport type
 *  @param transport The gaia transport on which the service runs
 *  @param success Boolean indicating success or failure of the GaiaStartService request
 */
void gaiaTransportCommonSendGaiaStartServiceCfm(gaia_transport_type transport_type, gaia_transport* transport, bool success)
{
    MESSAGE_PMAKE(gssc, GAIA_START_SERVICE_CFM_T);
    gssc->transport_type = transport_type;
    gssc->transport = (GAIA_TRANSPORT*)transport;
    gssc->success = success;
    MessageSend(gaia->app_task, GAIA_START_SERVICE_CFM, gssc);
}

/*! @brief Cleanup battery and RSSI threshold state for a given transport.
 
    Used once a Gaia transport has closed, as we no longer need to perform the
    checking or resend the timer messages.
*/
void gaiaTransportCommonCleanupThresholdState(gaia_transport *transport)
{
    /* reset battery threshold state */
    transport->threshold_count_lo_battery = 0;
    transport->sent_notification_lo_battery_0 = FALSE;
    transport->sent_notification_lo_battery_1 = FALSE;
    transport->threshold_count_hi_battery = 0;
    transport->sent_notification_hi_battery_0 = FALSE;
    transport->sent_notification_hi_battery_1 = FALSE;

    /* reset RSSI threshold state */
    transport->threshold_count_lo_rssi = 0;
    transport->sent_notification_lo_rssi_0 = FALSE;
    transport->sent_notification_lo_rssi_1 = FALSE;
    transport->threshold_count_hi_rssi = 0;
    transport->sent_notification_hi_rssi_0 = FALSE;
    transport->sent_notification_hi_rssi_1 = FALSE;
}


/*! @brief Discard unread input data
 
    Must be performed before deallocating a channel to allow the firmware
    to free it.
*/
void gaiaTransportFlushInput(gaia_transport *transport)
{
    Source source;
    uint16 unread_count;

    source = gaiaTransportGetSource(transport);
    unread_count = SourceSize(source);

    if (unread_count)
    {
	SourceDrop(source,unread_count);
    }
}

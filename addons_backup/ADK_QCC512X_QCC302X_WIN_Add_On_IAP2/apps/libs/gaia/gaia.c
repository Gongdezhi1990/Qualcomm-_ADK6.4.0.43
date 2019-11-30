/*************************************************************************
Copyright (c) 2010 - 2018 Qualcomm Technologies International, Ltd.


FILE
    gaia.c

DESCRIPTION
    Generic Application Interface Architecture
    This library implements the GAIA protocol for a server- or client-side
    SPP connection.

    The library exposes a functional downstream API and a message-based upstream API.
*/
#include <stream.h>
#include <sink.h>
#include <source.h>
#include <vm.h>
#include <boot.h>
#include <ps.h>
#include <stdlib.h>
#include <stdio.h>
#include <panic.h>
#include <print.h>
#include <string.h>
#include <power.h>
#include <partition.h>
#include <file.h>
#include <spp_common.h>
#include <audio.h>
#include <loader.h>
#include <upgrade.h>
#include <vmal.h>
#include <gatt_manager.h>
#include <message.h>

#include "gaia_private.h"
#include "gaia_transport.h"
#include "gaia_voice_assistant.h"
#include "gaia_register_custom_sdp.h"

#include "../upgrade/upgrade_partition_data.h"
#ifdef GAIA_TRANSPORT_GATT
#include "gaia_transport_gatt.h"
#endif


#include "rwcp_server.h"
#include "gaia_db.h"

#include <byte_utils.h>


#define GAIA_UPGRADE_HEADER_SIZE (0x04)  /*This includes the length of VM opcode, length and more data*/

#define GAIA_UPGRADE_DATA_BYTES_REQ (0x03)
#define GAIA_UPGRADE_DATA_BYTES_REQ_OFFSET (0x06)
#define GAIA_UPGRADE_DATA_SIZE_OFFSET (0x03)

#define GAIA_UPGRADE_HOST_DATA_REQ (0x04) /*UPGRADE_HOST_DATA*/
#define GAIA_UPGRADE_HEADER_MORE_DATA_OFFSET (0x03)
#define GAIA_UPGRADE_HEADER_LENGTH_OFFSET (0x01)
#define GAIA_UPGRADE_COMMAND_ID_OFFSET (0x00)

/* 
 * A macro for obtaining the number of elements in an array.
 * This macros is not present in BlueLab so provide it if not present so that
 * it is available for host testing.
 */
#ifndef ARRAY_DIM
#define ARRAY_DIM(x) (sizeof ((x)) / sizeof ((x)[0]))
#endif

#ifndef MIN
#define MIN(a,b)    (((a)<(b))?(a):(b))
#endif

/*  Commands which can be handled by the library but can be supplanted
 *  by an application implementation.
 */
static const uint16 lib_commands[] =
{
    GAIA_COMMAND_DEVICE_RESET,
    GAIA_COMMAND_GET_BOOTMODE,
    GAIA_COMMAND_GET_API_VERSION,

    GAIA_COMMAND_GET_CURRENT_RSSI,
    GAIA_COMMAND_GET_CURRENT_BATTERY_LEVEL,
    GAIA_COMMAND_RETRIEVE_PS_KEY,
    GAIA_COMMAND_RETRIEVE_FULL_PS_KEY,

    GAIA_COMMAND_STORE_PS_KEY,
    GAIA_COMMAND_SEND_APPLICATION_MESSAGE,
    GAIA_COMMAND_SEND_KALIMBA_MESSAGE,
    GAIA_COMMAND_DELETE_PDL,

    GAIA_COMMAND_REGISTER_NOTIFICATION,
    GAIA_COMMAND_GET_NOTIFICATION,
    GAIA_COMMAND_CANCEL_NOTIFICATION
};

GAIA_T *gaia = NULL;

/*************************************************************************
NAME
    dwunpack

DESCRIPTION
    Unpack a uint32 into an array of uint8s
*/
static void dwunpack(uint8 *dest, uint32 src)
{
    *dest++ = src >> 24;
    *dest++ = (src >> 16) & 0xFF;
    *dest++ = (src >> 8) & 0xFF;
    *dest = src & 0xFF;
}


/*************************************************************************
NAME
    wpack

DESCRIPTION
    Pack array of uint8s into array of uint16
*/
static void wpack(uint16 *dest, uint8 *src, uint16 length)
{
    while (length--)
    {
        *dest = *src++ << 8;
        *dest++ |= *src++;
    }
}


/*************************************************************************
NAME
    find_locus_bit

DESCRIPTION
    Determine if command can be handled by the library
    Return index into lib_commands array or GAIA_INVALID_ID
*/
static uint16 find_locus_bit(uint16 command_id)
{
    uint16 idx;

    for (idx = 0;
        idx < ARRAY_DIM(lib_commands);
        ++idx)
    {
        if (lib_commands[idx] == command_id)
        {
            return idx;
        }
    }

    return GAIA_INVALID_ID;
}

/*************************************************************************
NAME
    read_battery_voltage

DESCRIPTION
    Read the current battery voltage using the power library
*/
static uint16 read_battery_voltage(void)
{
    voltage_reading reading;
    return PowerBatteryGetVoltage(&reading) ? reading.voltage : 0;
}

/*************************************************************************
NAME
    app_will_handle_command

DESCRIPTION
    Determine if the specified command is to be passed to the onchip app
*/
static bool app_will_handle_command(uint16 command_id)
{
/*  Not a command but an acknowledgement of an event notification  */
    if (command_id == GAIA_ACK_NOTIFICATION)
        return FALSE;

/*  Look it up  */
    return GaiaGetAppWillHandleCommand(command_id);
}


/*************************************************************************
NAME
    build_packet

DESCRIPTION
    Build a Gaia protocol packet in the given buffer
    uint8 payload variant
    If <status> is other than GAIA_STATUS_NONE it is inserted
    before the first byte of the passed-in payload to simplify
    the building of acknowledgement and notification packets

    Returns the length of the packet or 0 on error
*/
static uint16 build_packet(uint8 *buffer, uint8 flags,
                              uint16 vendor_id, uint16 command_id, uint16 status,
                              uint8 payload_length, uint8 *payload)
{
    uint8 *data = buffer;
    uint16 packet_length = GAIA_OFFS_PAYLOAD + payload_length;

    if (status != GAIA_STATUS_NONE)
        ++packet_length;

    if (flags & GAIA_PROTOCOL_FLAG_CHECK)
        ++packet_length;

    if (packet_length > GAIA_MAX_PACKET)
        return 0;

    /*  Build the header.  It's that diagram again ...
     *  0 bytes  1        2        3        4        5        6        7        8          9    len+8      len+9
     *  +--------+--------+--------+--------+--------+--------+--------+--------+ +--------+--/ /---+ +--------+
     *  |  SOF   |VERSION | FLAGS  | LENGTH |    VENDOR ID    |   COMMAND ID    | | PAYLOAD   ...   | | CHECK  |
     *  +--------+--------+--------+--------+--------+--------+--------+--------+ +--------+--/ /---+ +--------+
     */
    *data++ = GAIA_SOF;
    *data++ = GAIA_VERSION;
    *data++ = flags;
    *data++ = payload_length;
    *data++ = HIGH(vendor_id);
    *data++ = LOW(vendor_id);
    *data++ = HIGH(command_id);
    *data++ = LOW(command_id);

    if (status != GAIA_STATUS_NONE)
    {
        /*  Insert status-cum-event byte and increment payload length  */
        *data++ = status;
        ++buffer[GAIA_OFFS_PAYLOAD_LENGTH];
    }

    /*  Copy in the payload  */
    while (payload_length--)
        *data++ = *payload++;

    if (flags & GAIA_PROTOCOL_FLAG_CHECK)
    {
    /*  Compute the checksum  */
        uint8 check = 0;
        data = buffer;

        while (--packet_length)
            check ^= *data++;

        *data++ = check;
    }

    return data - buffer;
}


/*************************************************************************
NAME
    build_packet_16

DESCRIPTION
    Build a Gaia protocol packet in the given buffer
    uint16 payload variant
    If <status> is other than GAIA_STATUS_NONE it is inserted
    before the first byte of the passed-in payload to simplify
    the building of acknowledgement and notification packets

    payload_length is the number of uint16s in the payload
    Returns the length of the packet or 0 on error
*/
static uint16 build_packet_16(uint8 *buffer, uint8 flags,
                              uint16 vendor_id, uint16 command_id, uint16 status,
                              uint8 payload_length, uint16 *payload)
{
    uint8 *data = buffer;
    uint16 packet_length = GAIA_OFFS_PAYLOAD + 2 * payload_length;

    if (status != GAIA_STATUS_NONE)
        ++packet_length;

    if (flags & GAIA_PROTOCOL_FLAG_CHECK)
        ++packet_length;

    if (packet_length > GAIA_MAX_PACKET)
        return 0;

    /*  Build the header.  It's that diagram again ...
     *  0 bytes  1        2        3        4        5        6        7        8          9    len+8      len+9
     *  +--------+--------+--------+--------+--------+--------+--------+--------+ +--------+--/ /---+ +--------+
     *  |  SOF   |VERSION | FLAGS  | LENGTH |    VENDOR ID    |   COMMAND ID    | | PAYLOAD   ...   | | CHECK  |
     *  +--------+--------+--------+--------+--------+--------+--------+--------+ +--------+--/ /---+ +--------+
     */
    *data++ = GAIA_SOF;
    *data++ = GAIA_VERSION;
    *data++ = flags;
    *data++ = 2 * payload_length;
    *data++ = HIGH(vendor_id);
    *data++ = LOW(vendor_id);
    *data++ = HIGH(command_id);
    *data++ = LOW(command_id);

    if (status != GAIA_STATUS_NONE)
    {
        /*  Insert status-cum-event byte and increment payload length  */
        *data++ = status;
        ++buffer[GAIA_OFFS_PAYLOAD_LENGTH];
    }

    /*  Copy in the payload  */
    while (payload_length--)
    {
        *data++ = HIGH(*payload);
        *data++ = LOW(*payload++);
    }

    if (flags & GAIA_PROTOCOL_FLAG_CHECK)
    {
        /*  Compute the checksum  */
        uint8 check = 0;
        data = buffer;

        while (--packet_length)
            check ^= *data++;

        *data++ = check;
    }

    return data - buffer;
}


/*************************************************************************
NAME
    send_packet

DESCRIPTION
    Send a Gaia packet
*/
static void send_packet(Task task, gaia_transport *transport, uint16 packet_length, uint8 *packet)
{
    MESSAGE_PMAKE(m, GAIA_INTERNAL_SEND_REQ_T);

    GAIA_DEBUG(("gaia: send_packet: t=%p s=%d\n", (void *) task, VmGetAvailableAllocations()));

#ifdef DEBUG_GAIA_VERBOSE
    {
        uint16 i = 0;

        for (i = 0; i < packet_length; ++i)
            GAIA_DEBUG(("%02X ", packet[i]));
        
        GAIA_DEBUG(("\n"));
    }
#endif /* DEBUG_GAIA_VERBOSE */

    m->task = task;
    m->transport = transport;
    m->length = packet_length;
    m->data = packet;

    MessageSend(&gaia->task_data, GAIA_INTERNAL_SEND_REQ, m);
}


/*************************************************************************
NAME
    send_response

DESCRIPTION
    Build and send a Gaia packet
*/
static void send_response(gaia_transport *transport, uint16 vendor_id, uint16 command_id,
                          uint8 status, uint16 payload_length, uint8 *payload)
{
#ifdef GAIA_TRANSPORT_GATT
    if (transport->type == gaia_transport_gatt)
    {
        /* Upgrade related notifications are sent via streams if it were configured */
        if(transport->state.gatt.snk && status == GAIA_EVENT_VMUP_PACKET)
        {
            gaiaTransportAttStreamsSend(transport, vendor_id, command_id, status, payload_length, payload, FALSE);
        }
        else
        {
            GAIA_DEBUG(("Sent via PRIMS: EVENT TYPE: %2X\n", status));
            gaiaTransportGattSend(transport, vendor_id, command_id, status, payload_length, payload, FALSE);
        }
    }
    else
#endif
    {
        uint16 packet_length;
        uint8 *packet;
        uint8 flags = transport->flags;

        packet_length = GAIA_OFFS_PAYLOAD + payload_length + 1;
        if (flags & GAIA_PROTOCOL_FLAG_CHECK)
            ++packet_length;

        packet = malloc(packet_length);

        if (packet)
        {
            packet_length = build_packet(packet, flags, vendor_id, command_id,
                                         status, payload_length, payload);

            send_packet(NULL, transport, packet_length, packet);
        }
    }
}


/*************************************************************************
NAME
    send_ack_16

DESCRIPTION
    Build and send a Gaia acknowledgement packet from a uint16[] payload
*/
static void send_ack_16(gaia_transport *transport, uint16 vendor_id, uint16 command_id,
                          uint8 status, uint16 payload_length, uint16 *payload)
{
#ifdef GAIA_TRANSPORT_GATT
    if (transport->type == gaia_transport_gatt)
    {
        gaiaTransportGattSend(transport, vendor_id, command_id | GAIA_ACK_MASK, status, payload_length, payload, TRUE);
    }
    else
#endif
    {
        uint16 packet_length;
        uint8 *packet;
        uint8 flags = transport->flags;


        packet_length = GAIA_OFFS_PAYLOAD + 2 * payload_length + 1;
        if (flags & GAIA_PROTOCOL_FLAG_CHECK)
            ++packet_length;

        packet = malloc(packet_length);

        if (packet)
        {
            packet_length = build_packet_16(packet, flags, vendor_id, command_id | GAIA_ACK_MASK,
                                         status, payload_length, payload);

            send_packet(NULL, transport, packet_length, packet);
        }
    }
}


/*************************************************************************
NAME
    send_api_version

DESCRIPTION
    Send a completed GAIA_COMMAND_GET_API_VERSION response
*/
static void send_api_version(gaia_transport *transport)
{
    uint8 payload[3];

    payload[0] = GAIA_VERSION;
    payload[1] = GAIA_API_VERSION_MAJOR;
    payload[2] = gaia->api_minor;

    send_success_payload(transport, GAIA_COMMAND_GET_API_VERSION, 3, payload);
}


/*************************************************************************
NAME
    send_rssi

DESCRIPTION
    Send a completed GAIA_COMMAND_GET_CURRENT_RSSI response, or start an
    RSSI enquiry on a BLE connection
*/
static void send_rssi(gaia_transport *transport)
{
    int16 rssi = 0;
#ifdef GAIA_TRANSPORT_GATT
    if(transport->type==gaia_transport_gatt)
    {
        tp_bdaddr tpaddr;
        if(VmGetBdAddrtFromCid(transport->state.gatt.cid, &tpaddr))
        {
            ConnectionGetRssiBdaddr(&gaia->task_data,&tpaddr);
        } 
        else
        {
            send_simple_response(transport, GAIA_COMMAND_GET_CURRENT_RSSI, GAIA_STATUS_NOT_SUPPORTED);
        }
    }
    else
#endif  /* GAIA_TRANSPORT_GATT */
    {   
        Sink sink = gaiaTransportGetSink(transport);
        if (sink == BAD_SINK)
        {
            send_simple_response(transport, GAIA_COMMAND_GET_CURRENT_RSSI, GAIA_STATUS_NOT_SUPPORTED);
        }
        else if (SinkGetRssi(sink, &rssi))
        {
            uint8 payload = LOW(rssi);
            send_success_payload(transport, GAIA_COMMAND_GET_CURRENT_RSSI, 1, &payload);
        }
        else
        {
            send_insufficient_resources(transport, GAIA_COMMAND_GET_CURRENT_RSSI);
        }
    }
}


#ifdef GAIA_TRANSPORT_GATT
/*****************************************************************************
NAME
    report_rssi

DESCRIPTION
    Process CL_DM_RSSI_BDADDR_CFM_T from the firmware and distribute
*/
static void report_rssi( CL_DM_RSSI_BDADDR_CFM_T *cfm )
{
    uint16 idx;
    gaia_transport *transport;

    FOREACH_TRANSPORT(idx, transport)
    {
        tp_bdaddr tpaddr;
        if((transport->type==gaia_transport_gatt)&&VmGetBdAddrtFromCid(transport->state.gatt.cid,&tpaddr)&&BdaddrTpIsSame(&tpaddr,&cfm->tpaddr))
        {
            if(cfm->status==0)
            {
                send_success_payload(transport, GAIA_COMMAND_GET_CURRENT_RSSI, 1, &cfm->rssi);
            }
            else
            {
                send_insufficient_resources(transport, GAIA_COMMAND_GET_CURRENT_RSSI);
            }
        break;
        }
    }
}
#endif  /* GAIA_TRANSPORT_GATT */


/*************************************************************************
NAME
    send_battery_level

DESCRIPTION
    Send a completed GAIA_COMMAND_GET_CURRENT_BATTERY_LEVEL response
*/
static void send_battery_level(gaia_transport *transport)
{
    uint16 voltage = read_battery_voltage();

    if (voltage == 0)
        send_insufficient_resources(transport, GAIA_COMMAND_GET_CURRENT_BATTERY_LEVEL);

    else
    {
        uint8 payload[2];

        payload[0] = HIGH(voltage);
        payload[1] = LOW(voltage);

        send_success_payload(transport, GAIA_COMMAND_GET_CURRENT_BATTERY_LEVEL, 2, payload);
    }
}


/*************************************************************************
NAME
    queue_device_reset

DESCRIPTION
    Queue a device reset in the near future.  TATA_TIME should be time
    enough for the host to get our acknowledgement before we go.  Setting
    the 'rebooting' flag inhibits any further commands from the host.
 */
static void queue_device_reset(gaia_transport *transport, uint16 bootmode)
{
    uint16 *reset_message;

    GAIA_CMD_DEBUG(("gaia: queue_device_reset %u\n", bootmode));

    reset_message = malloc(sizeof (uint16));
    if (!reset_message)
        send_insufficient_resources(transport, GAIA_COMMAND_DEVICE_RESET);

    else
    {
        gaia->rebooting = TRUE;
        *reset_message = bootmode;
        MessageSendLater(&gaia->task_data, GAIA_INTERNAL_REBOOT_REQ, reset_message, TATA_TIME);
        send_success(transport, GAIA_COMMAND_DEVICE_RESET);
    }
}


/*************************************************************************
NAME
    send_bootmode

DESCRIPTION
    Handle a GAIA_COMMAND_GET_BOOTMODE by returning the current bootmode
*/
static void send_bootmode(gaia_transport *transport)
{
    uint8 payload = BootGetMode();
    send_success_payload(transport, GAIA_COMMAND_GET_BOOTMODE, 1, &payload);
}


/*! @brief Utility function to start timer for checking battery level thresholds.
 */
static void start_check_battery_threshold_timer(gaia_transport *transport)
{
    MESSAGE_PMAKE(batt_timer, GAIA_INTERNAL_CHECK_BATTERY_THRESHOLD_REQ_T);
    batt_timer->transport = transport;
    MessageSendLater(&gaia->task_data, GAIA_INTERNAL_CHECK_BATTERY_THRESHOLD_REQ, batt_timer, GAIA_CHECK_THRESHOLDS_PERIOD);
}

/*! @brief Utility function to start timer for checking RSSI level thresholds.
 */
static void start_check_rssi_threshold_timer(gaia_transport *transport)
{
    MESSAGE_PMAKE(rssi_timer, GAIA_INTERNAL_CHECK_RSSI_THRESHOLD_REQ_T);
    rssi_timer->transport = transport;
    MessageSendLater(&gaia->task_data, GAIA_INTERNAL_CHECK_RSSI_THRESHOLD_REQ, rssi_timer, GAIA_CHECK_THRESHOLDS_PERIOD);
}

/*! @brief Utility function to send GAIA_INTERNAL_ATT_STREAMS_BUFFER_UNAVAILABLE
 */
void send_internal_att_streams_buffer_unavailable(gaia_transport *transport, 
                                 uint16 vendor_id,
                                 uint16 command_id,
                                 uint8 status, 
                                 uint8 size_payload,
                                 void *payload,
                                 bool unpack)
{
    GAIA_INTERNAL_ATT_STREAMS_BUFFER_UNAVAILABLE_REQ_T *att_buffer = (GAIA_INTERNAL_ATT_STREAMS_BUFFER_UNAVAILABLE_REQ_T *)PanicUnlessMalloc(sizeof(*att_buffer) + size_payload - 1);
  
    att_buffer->transport = transport;
    att_buffer->vendor_id = vendor_id;
    att_buffer->command_id = command_id;
    att_buffer->status = (uint16) status;
    att_buffer->unpack = (uint16) unpack;
    att_buffer->size = size_payload;
    ByteUtilsMemCpyToStream(att_buffer->data, (uint8*)payload, size_payload);

    MessageSendLater(&gaia->task_data, GAIA_INTERNAL_ATT_STREAMS_BUFFER_UNAVAILABLE, att_buffer, GAIA_ATT_STREAMS_BUFF);
}


/*************************************************************************
NAME
    register_notification

DESCRIPTION
    Registers the host's desire to receive notification of an event
    GAIA_EVENT_DEBUG_MESSAGE requires GAIA_FEATURE_DEBUG feature bit
*/
static void register_notification(gaia_transport *transport, uint16 payload_length, uint8 *payload)
{
    uint16 status = GAIA_STATUS_INVALID_PARAMETER;

    GAIA_CMD_DEBUG(("gaia: register_notification %u: 0x%02X\n", payload_length, payload[0]));

    switch (payload[0])
    {
    case GAIA_EVENT_BATTERY_LOW_THRESHOLD:

        /* start the battery threshold check timer if necessary */
        if (!transport->threshold_count_lo_battery && !transport->threshold_count_hi_battery)
        {
            start_check_battery_threshold_timer(transport);
        }

        transport->threshold_count_lo_battery = 0;

    /*  Check for 1 or 2 two-byte arguments  */
        switch (payload_length)
        {
        case 5:
            transport->battery_lo_threshold[1] = W16(payload + 3);
            transport->threshold_count_lo_battery = 1;
        /*  drop through  */
        case 3:
            transport->battery_lo_threshold[0] = W16(payload + 1);
            transport->threshold_count_lo_battery++;
            status = GAIA_STATUS_SUCCESS;
            break;
        }

        GAIA_DEBUG(("gaia: batt lo %d: %d %d\n",
               transport->threshold_count_lo_battery,
               transport->battery_lo_threshold[0],
               transport->battery_lo_threshold[1]));

        break;

    case GAIA_EVENT_BATTERY_HIGH_THRESHOLD:

        /* start the battery threshold check timer if necessary */
        if (!transport->threshold_count_lo_battery && !transport->threshold_count_hi_battery)
        {
            start_check_battery_threshold_timer(transport);
        }

        transport->threshold_count_hi_battery = 0;

    /*  Check for 1 or 2 two-byte arguments  */
        switch (payload_length)
        {
        case 5:
            transport->battery_hi_threshold[1] = W16(payload + 3);
            transport->threshold_count_hi_battery = 1;
        /*  drop through  */
        case 3:
            transport->battery_hi_threshold[0] = W16(payload + 1);
            transport->threshold_count_hi_battery++;
            status = GAIA_STATUS_SUCCESS;
            break;
        }

        GAIA_DEBUG(("gaia: batt hi %d: %d %d\n",
               transport->threshold_count_hi_battery,
               transport->battery_hi_threshold[0],
               transport->battery_hi_threshold[1]));

        break;

    case GAIA_EVENT_RSSI_LOW_THRESHOLD:

        /* start the RSSI check timer if necessary */
        if (!transport->threshold_count_lo_rssi && !transport->threshold_count_hi_rssi)
        {
            start_check_rssi_threshold_timer(transport);
        }

        transport->threshold_count_lo_rssi = 0;

    /*  Check for 1 or 2 one-byte arguments  */
        switch (payload_length)
        {
        case 3:
            transport->rssi_lo_threshold[1] = SEXT(payload[2]);
            transport->threshold_count_lo_rssi = 1;
        /*  drop through  */
        case 2:
            transport->rssi_lo_threshold[0] = SEXT(payload[1]);
            transport->threshold_count_lo_rssi++;
            status = GAIA_STATUS_SUCCESS;
            break;
        }

        GAIA_DEBUG(("gaia: RSSI lo %d: %d %d\n",
               transport->threshold_count_lo_rssi,
               transport->rssi_lo_threshold[0],
               transport->rssi_lo_threshold[1]));

        break;

    case GAIA_EVENT_RSSI_HIGH_THRESHOLD:

        /* start the RSSI check timer if necessary */
        if (!transport->threshold_count_lo_rssi && !transport->threshold_count_hi_rssi)
        {
            start_check_rssi_threshold_timer(transport);
        }

        transport->threshold_count_hi_rssi = 0;

    /*  Check for 1 or 2 one-byte arguments  */
        switch (payload_length)
        {
        case 3:
            transport->rssi_hi_threshold[1] = SEXT(payload[2]);
            transport->threshold_count_hi_rssi = 1;
        /*  drop through  */
        case 2:
            transport->rssi_hi_threshold[0] = SEXT(payload[1]);
            transport->threshold_count_hi_rssi++;
            status = GAIA_STATUS_SUCCESS;
            break;
        }

        GAIA_DEBUG(("gaia: RSSI hi %d: %d %d\n",
               transport->threshold_count_hi_rssi,
               transport->rssi_hi_threshold[0],
               transport->rssi_hi_threshold[1]));

        break;

    case GAIA_EVENT_DFU_STATE:
        transport->notify_dfu_state = TRUE;
        status = GAIA_STATUS_SUCCESS;
        break;

    case GAIA_EVENT_VMUP_PACKET:
        transport->notify_vmup = TRUE;
        status = GAIA_STATUS_SUCCESS;
        break;
    }

    send_ack(transport, GAIA_VENDOR_QTIL, GAIA_COMMAND_REGISTER_NOTIFICATION, status, 1, payload);
}


/*************************************************************************
NAME
    send_notification_setting

DESCRIPTION
    Respond to a GAIA_COMMAND_GET_NOTIFICATION command
    The response holds the notification setting for the given event,
    if any, e.g. if two Battery Low Thresholds were configured:

    +--------+--------+--------+--------+--------+--------+--------+
    | STATUS |  TYPE  | COUNT  |   THRESHOLD 1   |   THRESHOLD 2   |
    +--------+--------+--------+--------+--------+--------+--------+

    Status: 0x00, success
    Type:   0x02, Batery Low Threshold
    Count:  2

    For events which are simply disabled or enabled the 'count' is 0 or 1

NOTE
    payload_length does not include the STATUS byte; GaiaSendResponse()
    adds that later
*/
static void send_notification_setting(gaia_transport *transport, uint8 event)
{
    uint8 payload[6];
    uint8 *p = payload;

    *p++ = event;

    switch (event)
    {
    case GAIA_EVENT_RSSI_LOW_THRESHOLD:
        *p++ = transport->threshold_count_lo_rssi;

        if (transport->threshold_count_lo_rssi > 0)
            *p++ = transport->rssi_lo_threshold[0];

        if (transport->threshold_count_lo_rssi > 1)
            *p++ = transport->rssi_lo_threshold[1];

        break;

    case GAIA_EVENT_RSSI_HIGH_THRESHOLD:
        *p++ = transport->threshold_count_hi_rssi;

        if (transport->threshold_count_hi_rssi > 0)
            *p++ = transport->rssi_hi_threshold[0];

        if (transport->threshold_count_hi_rssi > 1)
            *p++ = transport->rssi_hi_threshold[1];

        break;

    case GAIA_EVENT_BATTERY_LOW_THRESHOLD:
        *p++= transport->threshold_count_lo_battery;

        if (transport->threshold_count_lo_battery > 0)
        {
            *p++ = HIGH(transport->battery_lo_threshold[0]);
            *p++ = LOW(transport->battery_lo_threshold[0]);
        }

        if (transport->threshold_count_lo_battery > 1)
        {
            *p++ = HIGH(transport->battery_lo_threshold[1]);
            *p++ = LOW(transport->battery_lo_threshold[1]);
        }

        break;

    case GAIA_EVENT_BATTERY_HIGH_THRESHOLD:
        *p++ = transport->threshold_count_hi_battery;

        if (transport->threshold_count_hi_battery > 0)
        {
            *p++ = HIGH(transport->battery_hi_threshold[0]);
            *p++ = LOW(transport->battery_hi_threshold[0]);
        }

        if (transport->threshold_count_hi_battery > 1)
        {
            *p++ = HIGH(transport->battery_hi_threshold[1]);
            *p++ = LOW(transport->battery_hi_threshold[1]);
        }

        break;

    case GAIA_EVENT_DFU_STATE:
        *p++ = transport->notify_dfu_state;
        break;

    case GAIA_EVENT_VMUP_PACKET:
        *p++ = transport->notify_vmup;
        break;

/*  Dummies for legal but unimplemented events  */
    case GAIA_EVENT_BATTERY_CHARGED:
    case GAIA_EVENT_DEVICE_STATE_CHANGED:
    case GAIA_EVENT_PIO_CHANGED:
        *p++ = GAIA_FEATURE_DISABLED;
        break;
    }


    if (p > payload)
        send_success_payload(transport, GAIA_COMMAND_GET_NOTIFICATION, p - payload, payload);

    else
        send_invalid_parameter(transport, GAIA_COMMAND_GET_NOTIFICATION);
}

/*************************************************************************
NAME
    cancel_notification

DESCRIPTION
    The host desires to receive no further notification of an event
*/
static void cancel_notification(gaia_transport *transport, uint8 event_id)
{
/*  uint16 old_flags = gaia->threshold_poll_flags;  */
    uint16 status = GAIA_STATUS_INVALID_PARAMETER;

    switch(event_id)
    {
    case GAIA_EVENT_BATTERY_LOW_THRESHOLD:
        transport->threshold_count_lo_battery = 0;
        transport->sent_notification_lo_battery_0 = FALSE;
        transport->sent_notification_lo_battery_1 = FALSE;
        status = GAIA_STATUS_SUCCESS;
        break;

    case GAIA_EVENT_BATTERY_HIGH_THRESHOLD:
        transport->threshold_count_hi_battery = 0;
        transport->sent_notification_hi_battery_0 = FALSE;
        transport->sent_notification_hi_battery_1 = FALSE;
        status = GAIA_STATUS_SUCCESS;
        break;

    case GAIA_EVENT_RSSI_LOW_THRESHOLD:
        transport->threshold_count_lo_rssi = 0;
        transport->sent_notification_lo_rssi_0 = FALSE;
        transport->sent_notification_lo_rssi_1 = FALSE;
        status = GAIA_STATUS_SUCCESS;
        break;

    case GAIA_EVENT_RSSI_HIGH_THRESHOLD:
        transport->threshold_count_hi_rssi = 0;
        transport->sent_notification_hi_rssi_0 = FALSE;
        transport->sent_notification_hi_rssi_1 = FALSE;
        status = GAIA_STATUS_SUCCESS;
        break;

    case GAIA_EVENT_DFU_STATE:
        transport->notify_dfu_state = FALSE;
        status = GAIA_STATUS_SUCCESS;
        break;

    case GAIA_EVENT_VMUP_PACKET:
        transport->notify_vmup = FALSE;
        status = GAIA_STATUS_SUCCESS;
        break;
    }


    send_ack(transport, GAIA_VENDOR_QTIL, GAIA_COMMAND_CANCEL_NOTIFICATION, status, 1, &event_id);
}


/*************************************************************************
NAME
    send_modulemodule_id

DESCRIPTION
    Respond to a GAIA_COMMAND_GET_MODULE_ID command

    The response holds the chip id, design id and module id proper.
    For Flash devices, chip id will be zero and module id information
    is read from persistent store if present.
*/
static void send_module_id(gaia_transport *transport)
{
    uint16 module_id[4];
    memset(module_id, 0, sizeof module_id);

    module_id[0] = (uint16) (VmalVmReadProductId() >> 16);

    PsFullRetrieve(PSKEY_MODULE_DESIGN, module_id + 1, 1);
    PsFullRetrieve(PSKEY_MODULE_ID, module_id + 2, 2);

    GAIA_DEBUG(("gaia: mod %04X %04X %04X %04X\n", module_id[0], module_id[1], module_id[2], module_id[3]));

    send_ack_16(transport, GAIA_VENDOR_QTIL, GAIA_COMMAND_GET_MODULE_ID,
        GAIA_STATUS_SUCCESS, ARRAY_DIM(module_id), module_id);
}


/*************************************************************************
NAME
    set_data_endpoint_mode

DESCRIPTION
    Responds to a GAIA_COMMAND_SET_DATA_ENDPOINT_MODE command. 
    Sets the data endpoint mode
*/
static bool set_data_endpoint_mode(gaia_transport *transport, uint8 mode)
{
    switch(mode)
    {
        case GAIA_DATA_ENDPOINT_MODE_NONE:
            gaia->data_endpoint_mode = GAIA_DATA_ENDPOINT_MODE_NONE;
            send_ack(transport, GAIA_VENDOR_QTIL, GAIA_COMMAND_SET_DATA_ENDPOINT_MODE, GAIA_STATUS_SUCCESS, 0, NULL);
            return TRUE;

        case GAIA_DATA_ENDPOINT_MODE_RWCP:
            gaia->data_endpoint_mode = GAIA_DATA_ENDPOINT_MODE_RWCP;
            send_ack(transport, GAIA_VENDOR_QTIL, GAIA_COMMAND_SET_DATA_ENDPOINT_MODE, GAIA_STATUS_SUCCESS, 0, NULL);
            RwcpSetClientTask(&gaia->task_data);
            return TRUE;

        default:
            send_ack(transport, GAIA_VENDOR_QTIL, GAIA_COMMAND_SET_DATA_ENDPOINT_MODE, GAIA_STATUS_INVALID_PARAMETER, 0, NULL);
            return FALSE;
    }
}

/*************************************************************************
NAME
    get_data_endpoint_mode

DESCRIPTION
    Reponds to a GAIA_COMMAND_GET_DATA_ENDPOINT_MODE command. Returns the 
    data endpoint mode which is set.
*/
static bool get_data_endpoint_mode(gaia_transport *transport)
{
    uint8 mode = gaia->data_endpoint_mode;
    send_ack(transport, GAIA_VENDOR_QTIL, GAIA_COMMAND_GET_DATA_ENDPOINT_MODE, GAIA_STATUS_SUCCESS, sizeof(mode), &mode);    

    return TRUE;
}

/*************************************************************************
NAME
    gaia_handle_control_command

DESCRIPTION
    Handle a control command or return FALSE if we can't
*/
static bool gaia_handle_control_command(gaia_transport *transport, uint16 command_id,
                                        uint8 payload_length, uint8 *payload)
{
    switch (command_id)
    {
    case GAIA_COMMAND_DEVICE_RESET:
        if (payload_length == 0)
            queue_device_reset(transport, BootGetMode());

        else if ((payload_length == 1) && (payload[0] <= PS_MAX_BOOTMODE))
            queue_device_reset(transport, payload[0]);

        else
            send_invalid_parameter(transport, GAIA_COMMAND_DEVICE_RESET);

        return TRUE;

    case GAIA_COMMAND_GET_BOOTMODE:
        send_bootmode(transport);
        return TRUE;

    case GAIA_COMMAND_SET_DATA_ENDPOINT_MODE:
        return set_data_endpoint_mode(transport, payload[0]) ;

    case GAIA_COMMAND_GET_DATA_ENDPOINT_MODE:
        return get_data_endpoint_mode(transport);
    }

    return FALSE;
}


/*************************************************************************
NAME
    gaia_handle_status_command

DESCRIPTION
    Handle a Polled Status command or return FALSE if we can't
*/
static bool gaia_handle_status_command(gaia_transport *transport, uint16 command_id,
                                       uint8 payload_length, uint8 *payload)
{
    UNUSED(payload_length);
    UNUSED(payload);
    switch (command_id)
    {
    case GAIA_COMMAND_GET_API_VERSION:
        send_api_version(transport);
        return TRUE;

    case GAIA_COMMAND_GET_CURRENT_RSSI:
        send_rssi(transport);
        return TRUE;

    case GAIA_COMMAND_GET_CURRENT_BATTERY_LEVEL:
            send_battery_level(transport);
        return TRUE;

    case GAIA_COMMAND_GET_MODULE_ID:
        send_module_id(transport);
        return TRUE;
    }

    return FALSE;
}


/*************************************************************************
NAME
    gaia_handle_feature_command

DESCRIPTION
    Handle a Feature Control command or return FALSE if we can't
*/
static bool gaia_handle_feature_command(gaia_transport *transport, uint16 command_id,
                                        uint8 payload_length, uint8 *payload)
{
    switch (command_id)
    {
    case GAIA_COMMAND_SET_SESSION_ENABLE:
        if ((payload_length > 0) && (payload[0] < 2))
        {
            transport->enabled = payload[0];
            send_success(transport, command_id);
        }

        else
            send_invalid_parameter(transport, command_id);

        return TRUE;

    case GAIA_COMMAND_GET_SESSION_ENABLE:
        {
           uint8 response;
           response = transport->enabled;
           send_success_payload(transport, command_id, 1, &response);
           return TRUE;
        }
    }
    return FALSE;
}

/*************************************************************************
NAME
    dfu_reset_timeout

DESCRIPTION
    Reset the DFU start / transfer timeout

*/
static void dfu_reset_timeout(uint16 period)
{
    GAIA_DFU_DEBUG(("gaia: dfu tmo=%u\n", period));

    MessageCancelAll(&gaia->task_data, GAIA_INTERNAL_DFU_TIMEOUT);

    if (period)
    {
        MessageSendLater(&gaia->task_data, GAIA_INTERNAL_DFU_TIMEOUT, NULL, D_SEC(period));
    }
}


/*************************************************************************
NAME
    dfu_confirm

DESCRIPTION
    Send a GAIA_DFU_CFM response to the application task

*/
static void dfu_confirm(gaia_transport *transport, bool success)
{
    MESSAGE_PMAKE(cfm, GAIA_DFU_CFM_T);
    cfm->transport = (GAIA_TRANSPORT *) transport;
    cfm->success = success;
    MessageSend(gaia->app_task, GAIA_DFU_CFM, cfm);
}

/*************************************************************************
NAME
    dfu_indicate

DESCRIPTION
    Send a GAIA_DFU_IND to the application task

*/
static void dfu_indicate(uint8 state)
{
    MESSAGE_PMAKE(ind, GAIA_DFU_IND_T);
    ind->transport = (GAIA_TRANSPORT *) gaia->outstanding_request;
    ind->state = state;
    MessageSend(gaia->app_task, GAIA_DFU_IND, ind);
}


/*************************************************************************
NAME
    dfu_send_state

DESCRIPTION
    Send a DFU State Change notification


*/
static void dfu_send_state(uint8 state)
{
    if (gaia->outstanding_request && gaia->outstanding_request->notify_dfu_state)
    {
        send_notification(gaia->outstanding_request, GAIA_EVENT_DFU_STATE, 1, &state);
    }

    dfu_indicate(state);
}

#ifdef HAVE_PARTITION_FILESYSTEM

/*************************************************************************
NAME
    pfs_status

DESCRIPTION
    Return status of given storage partition

*/
static gaia_partition_state pfs_status(uint8 device, uint8 partition)
{
    uint32 value = 0;
    bool ok;

    ok = PartitionGetInfo(device, partition, PARTITION_INFO_TYPE, &value) &&
            (value == PARTITION_TYPE_FILESYSTEM);

    GAIA_DEBUG(("gaia: pfs %u %u sts %u %lu\n", device, partition, ok, value));

    if (!ok)
        return PARTITION_INVALID;

    ok = PartitionGetInfo(device, partition, PARTITION_INFO_IS_MOUNTED, &value);

    GAIA_DEBUG(("gaia: pfs %u %u mnt %u %lu\n", device, partition, ok, value));

    if (!ok)
        return PARTITION_INVALID;

    if (value)
        return PARTITION_MOUNTED;

    return PARTITION_UNMOUNTED;
}


/*************************************************************************
NAME
    send_pfs_status

DESCRIPTION
    Respond to a GAIA_COMMAND_GET_STORAGE_PARTITION_STATUS command with
    details for given partition

    +--------+--------+--------+-----------------------------------+--------+
    | DEVICE |PARTIT'N|  TYPE  |                SIZE               |MOUNTED |
    +--------+--------+--------+--------+--------+--------+--------+--------+
        0        1        2        3        4        5        6        7
*/
static void send_pfs_status(gaia_transport *transport, uint8 device, uint8 partition)
{
    uint32 value;
    bool ok;
    uint8 response[8];

    ok = PartitionGetInfo(device, partition, PARTITION_INFO_TYPE, &value);

    if (ok)
    {
        response[0] = device;
        response[1] = partition;
        response[2] = value;

        ok = PartitionGetInfo(device, partition, PARTITION_INFO_SIZE, &value);
    }

    if (ok)
    {
        dwunpack(response + 3, value);
        ok = PartitionGetInfo(device, partition, PARTITION_INFO_IS_MOUNTED, &value);
    }

    if (ok)
    {
        response[7] = value;

        send_success_payload(transport, GAIA_COMMAND_GET_STORAGE_PARTITION_STATUS,
            sizeof response, response);
    }

    else
        send_invalid_parameter(transport, GAIA_COMMAND_GET_STORAGE_PARTITION_STATUS);
}


/*************************************************************************
NAME
    open_storage_partition

DESCRIPTION
    Prepare a storage partition for writing

*/
static void open_storage_partition(gaia_transport *transport, uint8 *payload, uint8 payload_length)
{
    uint8 status;

    if (IsAudioBusy())
    {
        GAIA_DEBUG(("gaia: open_storage_partition - audio busy error 0x%p (0x%p)\n",(void *)AudioBusyTask(), (void *)&gaia->task_data ));
        status = GAIA_STATUS_INCORRECT_STATE;
    }
    else if ((payload_length != 3) && (payload_length != 7))
    /*  may or may not have 32-bit CRC  */
        status = GAIA_STATUS_INVALID_PARAMETER;

    else if ((payload[0] != PARTITION_SERIAL_FLASH) || (payload[2] != GAIA_PFS_MODE_OVERWRITE))
    /*  Currently we can only overwrite, and it has to be ext flash  */
        status = GAIA_STATUS_INVALID_PARAMETER;

    else if ((gaia->pfs_state != PFS_NONE) || (gaia->outstanding_request != NULL))
    /*  We support only one stream  */
        status = GAIA_STATUS_INSUFFICIENT_RESOURCES;

    else if (pfs_status(payload[0], payload[1]) != PARTITION_UNMOUNTED)
    /*  The partition must be a filesystem and must be unmounted  */
        status = GAIA_STATUS_INCORRECT_STATE;

    else
    {
        bool ok;

        GAIA_DEBUG(("gaia: open_storage_partition - set audio busy\n"));
        SetAudioBusy(&gaia->task_data);

        GAIA_DEBUG(("gaia: start overwrite sink %lu\n", VmGetClock()));
        gaia->pfs.sink = StreamPartitionOverwriteSink(payload[0], payload[1]);
        GAIA_DEBUG(("gaia: finish overwrite sink %lu\n", VmGetClock()));

        if (payload_length == 3)
            ok = PartitionSetMessageDigest(gaia->pfs.sink, PARTITION_MESSAGE_DIGEST_SKIP, NULL, 0);

        else
        {
            uint32 crc;
            crc = payload[3];
            crc = (crc << 8) | payload[4];
            crc = (crc << 8) | payload[5];
            crc = (crc << 8) | payload[6];

            GAIA_DEBUG(("gaia: write pfs crc %lu\n", crc));

            ok = PartitionSetMessageDigest(gaia->pfs.sink, PARTITION_MESSAGE_DIGEST_CRC, (uint16 *) &crc, sizeof crc);
        }

        if (ok)
        {
            uint8 payload_param;

            gaia->outstanding_request = transport;
            MessageStreamTaskFromSink(gaia->pfs.sink, &gaia->task_data);

            gaia->pfs_sequence = 0;
            gaia->pfs_state = PFS_PARTITION;
            payload_param = ++gaia->pfs_open_stream;

            GAIA_DEBUG(("gaia: pfs slack=%u\n", SinkSlack(gaia->pfs.sink)));
            status = GAIA_STATUS_SUCCESS;
            send_success_payload(transport, GAIA_COMMAND_OPEN_STORAGE_PARTITION, 1, &payload_param);
        }

        else
        {
            SetAudioBusy(NULL);
            status = GAIA_STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (status != GAIA_STATUS_SUCCESS)
    {
        send_simple_response(transport, GAIA_COMMAND_OPEN_STORAGE_PARTITION, status);
    }
}


/*************************************************************************
NAME
    mount_storage_partition

DESCRIPTION
    Mount a storage partition

    The payload holds three octets corresponding to the three word arguments
    of PartitionMountFilesystem()

    +--------+--------+--------+
    | DEVICE |PARTIT'N|PRIORITY|
    +--------+--------+--------+
        0        1        2
*/
static void mount_storage_partition(gaia_transport *transport, uint8 *payload, uint8 payload_length)
{
    uint8 status = GAIA_STATUS_INVALID_PARAMETER;

    if (payload_length == 3)
    {
        switch (pfs_status(payload[0], payload[1]))
        {
        case PARTITION_UNMOUNTED:
            if (PartitionMountFilesystem(payload[0], payload[1], payload[2]))
            {
                send_success_payload(transport, GAIA_COMMAND_MOUNT_STORAGE_PARTITION, 3, payload);
                status = GAIA_STATUS_SUCCESS;
            }

            else
                status = GAIA_STATUS_INSUFFICIENT_RESOURCES;

            break;

        case PARTITION_MOUNTED:
            status = GAIA_STATUS_INCORRECT_STATE;
            break;

        case PARTITION_INVALID:
            break;
       }
    }

    if (status != GAIA_STATUS_SUCCESS)
        send_simple_response(transport, GAIA_COMMAND_MOUNT_STORAGE_PARTITION, status);
    }


/*************************************************************************
NAME
    write_partition_data

DESCRIPTION
    Write data to partition

*/
static void write_partition_data(gaia_transport *transport, uint8 *payload, uint8 length)
{
    uint8 response[5];
    uint8 status = GAIA_STATUS_SUCCESS;

    if ((length < 5) || (payload[0] != gaia->pfs_open_stream))
        status = GAIA_STATUS_INVALID_PARAMETER;

    else if (gaia->pfs_state != PFS_PARTITION)
        status = GAIA_STATUS_INCORRECT_STATE;

    else
    {
        uint32 sequence;

        sequence = payload[1];
        sequence = (sequence << 8) | payload[2];
        sequence = (sequence << 8) | payload[3];
        sequence = (sequence << 8) | payload[4];

        GAIA_DEBUG(("gaia: write pfs %lu\n", sequence));

        if (sequence != gaia->pfs_sequence)
            status = GAIA_STATUS_INVALID_PARAMETER;

        else
        {
            uint16 data_length = length - 5;

            response[0] = gaia->pfs_open_stream;
            dwunpack(response + 1, sequence);

            if (data_length)
            {
                if (SinkClaim(gaia->pfs.sink, data_length) == BAD_SINK_CLAIM)
                    status = GAIA_STATUS_INSUFFICIENT_RESOURCES;

                else
                {
                    memmove(SinkMap(gaia->pfs.sink), payload + 5, data_length);
                    SinkFlush(gaia->pfs.sink, data_length);
                }
            }
        }
    }

    if (status == GAIA_STATUS_SUCCESS)
    {
        send_success_payload(transport, GAIA_COMMAND_WRITE_STORAGE_PARTITION, 5, response);
        ++gaia->pfs_sequence;
    }

    else
    {
        if (AudioBusyTask() == &gaia->task_data)
        {
            GAIA_DEBUG(("gaia: write_partition_data - clear audio busy\n"));
            SetAudioBusy(NULL);
        }
        send_simple_response(transport, GAIA_COMMAND_WRITE_STORAGE_PARTITION, status);
    }
}


/*************************************************************************
NAME
    close_storage_partition

DESCRIPTION
    Close the open storage partition

*/
static void close_storage_partition(gaia_transport *transport)
{
    uint8 status;

    GAIA_DEBUG(("gaia: close pfs\n"));

    if (gaia->pfs_state == PFS_PARTITION)
    {
        if (SinkClose(gaia->pfs.sink))
            status = GAIA_STATUS_SUCCESS;

        else
            status = GAIA_STATUS_INSUFFICIENT_RESOURCES;
    }
    else
        status = GAIA_STATUS_INCORRECT_STATE;

    if (AudioBusyTask() == &gaia->task_data)
    {
        GAIA_DEBUG(("gaia: close_storage_partition - clear audio busy\n"));
        SetAudioBusy(NULL);
    }

    if (status != GAIA_STATUS_SUCCESS)
    {
        uint8 response = gaia->pfs_open_stream;
        send_ack(transport, GAIA_VENDOR_QTIL, GAIA_COMMAND_CLOSE_STORAGE_PARTITION,
             status, 1, &response);
    }


}

#ifdef HAVE_DFU_FROM_SQIF
/*************************************************************************
NAME
    dfu_begin

DESCRIPTION
    Prepare a storage partition for DFU

*/
static void dfu_begin(gaia_transport *transport, uint8 *payload, uint8 payload_length)
{
    uint8 status;
    bool ok = FALSE;


    if (gaia->dfu_state != DFU_READY)
    {
        GAIA_DEBUG(("gaia: dfu_begin: not ready\n"));
        status = GAIA_STATUS_INCORRECT_STATE;
    }

    else
    {
        dfu_reset_timeout(0);

        if (IsAudioBusy())
        {
            GAIA_DEBUG(("gaia: dfu_begin: audio busy\n"));
            status = GAIA_STATUS_INCORRECT_STATE;
        }

        else if (payload_length != 8)
            status = GAIA_STATUS_INVALID_PARAMETER;

        else if ((gaia->pfs_state != PFS_NONE) || (gaia->outstanding_request != NULL))
        /*  We support only one stream  */
            status = GAIA_STATUS_INSUFFICIENT_RESOURCES;

        else
        {
            uint32 crc;
            uint32 size;
            uint32 space;

            size = payload[0];
            size = (size << 8) | payload[1];
            size = (size << 8) | payload[2];
            size = (size << 8) | payload[3];

            if (PartitionGetInfo(PARTITION_SERIAL_FLASH, gaia->pfs_open_stream, PARTITION_INFO_SIZE, &space))
            {
            /*  Partition size is in words, image size is in bytes */
                space = 2 * (space - DFU_PARTITION_OVERHEAD);
            }

            else
            {
                space = 0;
            }


            if (size > space)
            {
                GAIA_DEBUG(("gaia: dfu_begin: size %lu > space %lu\n", size, space));
                ok = FALSE;
            }

            else
            {
                SetAudioBusy(&gaia->task_data);

                crc = VmGetClock(); /* stack is precious */
                gaia->pfs.sink = StreamPartitionOverwriteSink(PARTITION_SERIAL_FLASH, gaia->pfs_open_stream);
                GAIA_DEBUG(("gaia: dfu_begin %lu\n", VmGetClock() - crc));

                ok = gaia->pfs.sink != NULL;
            }

            if (ok)
            {
                GAIA_DEBUG(("gaia: dfu_begin overwrite %u OK\n", gaia->pfs_open_stream));

                crc = payload[4];
                crc = (crc << 8) | payload[5];
                crc = (crc << 8) | payload[6];
                crc = (crc << 8) | payload[7];

                gaia->pfs_raw_size = size;

                GAIA_DEBUG(("gaia: dfu_begin: size=%lu space=%lu CRC=0x%08lX\n", size, space, crc));

                ok = PartitionSetMessageDigest(gaia->pfs.sink, PARTITION_MESSAGE_DIGEST_CRC, (uint16 *) &crc, sizeof crc);
            }

            if (ok)
            {
                gaia->outstanding_request = transport;

                MessageStreamTaskFromSink(gaia->pfs.sink, &gaia->task_data);

                gaia->pfs_state = PFS_DFU;
                dfu_reset_timeout(DFU_PACKET_TIMEOUT);

                GAIA_DEBUG(("gaia: pfs slack=%u\n", SinkSlack(gaia->pfs.sink)));
                status = GAIA_STATUS_SUCCESS;
            }

            else
            {
                GAIA_DEBUG(("gaia: dfu_begin FAIL\n"));
                SetAudioBusy(NULL);
                status = GAIA_STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    send_simple_response(transport, GAIA_COMMAND_DFU_BEGIN, status);

    if (ok)
    {
        gaia->dfu_state = DFU_DOWNLOAD;
        dfu_send_state(GAIA_DFU_STATE_DOWNLOAD);
    }

    else
    {
        gaia->dfu_state = DFU_IDLE;
        dfu_indicate(GAIA_DFU_STATE_DOWNLOAD_FAILURE);
    }
}
#endif /* def HAVE_DFU_FROM_SQIF */
#endif /* def HAVE_PARTITION_FILESYSTEM */

/*************************************************************************
NAME
    open_file

DESCRIPTION
    Prepare a file for reading

*/
static void open_file(gaia_transport *transport, uint8 *payload, uint8 payload_length)
{
    uint8 status;

    if ((payload_length < 2) || (payload[0] != GAIA_PFS_MODE_READ))
        status = GAIA_STATUS_INVALID_PARAMETER;

    else if (gaia->pfs_state != PFS_NONE)
    /*  We support only one stream  */
        status = GAIA_STATUS_INSUFFICIENT_RESOURCES;

    else
    {
        FILE_INDEX file = FileFind(FILE_ROOT, (char *) (payload + 1), payload_length - 1);

        if (file == FILE_NONE)
            status = GAIA_STATUS_INVALID_PARAMETER;

        else
        {
            gaia->pfs.source = StreamFileSource(file);
            if (gaia->pfs.source == NULL)
                status = GAIA_STATUS_INSUFFICIENT_RESOURCES;

            else
            {
                uint8 payload_param;

                gaia->pfs_sequence = 0;
                gaia->pfs_state = PFS_FILE;
                payload_param = ++gaia->pfs_open_stream;

                send_success_payload(transport, GAIA_COMMAND_OPEN_FILE, 1, &payload_param);

                status = GAIA_STATUS_SUCCESS;
            }
        }
    }

    if (status != GAIA_STATUS_SUCCESS)
        send_simple_response(transport, GAIA_COMMAND_OPEN_FILE, status);
}


/*************************************************************************
NAME
    read_file

DESCRIPTION
    Read and send a packetful of data from the open file

    |<----------- File packet header ----------->| <- File data -->|
    +--------+--------+--------+--------+--------+--------+--/ /---+
    | STREAM |              SEQUENCE             |  DATA     ...   |
    +--------+--------+--------+--------+--------+--------+--/ /---+
         0        1        2        3        4        5      ...
*/
static void read_file(gaia_transport *transport, uint8 *payload, uint8 payload_length)
{
    uint8 status;

    if ((payload_length != 5) || (payload[0] != gaia->pfs_open_stream))
        status = GAIA_STATUS_INVALID_PARAMETER;

    else if (gaia->pfs_state != PFS_FILE)
        status = GAIA_STATUS_INCORRECT_STATE;

    else
    {
        uint32 sequence;
        uint8 *response;

        sequence = payload[1];
        sequence = (sequence << 8) | payload[2];
        sequence = (sequence << 8) | payload[3];
        sequence = (sequence << 8) | payload[4];

        GAIA_DEBUG(("gaia: read file %lu\n", sequence));

        if (sequence != gaia->pfs_sequence)
            status = GAIA_STATUS_INVALID_PARAMETER;

        else
        {
            response = malloc(FILE_PACKET_HEADER_SIZE + FILE_BUFFER_SIZE);

            if (response == NULL)
                status = GAIA_STATUS_INSUFFICIENT_RESOURCES;

            else
            {
                uint16 length = SourceSize(gaia->pfs.source);

                if (length)
                {
                    const uint8 *data = SourceMap(gaia->pfs.source);

                    if (length > FILE_BUFFER_SIZE)
                        length = FILE_BUFFER_SIZE;

                    memmove(response + FILE_PACKET_HEADER_SIZE, data, length);
                    SourceDrop(gaia->pfs.source, length);
                }

                response[0] = gaia->pfs_open_stream;
                dwunpack(response + 1, sequence);
                send_success_payload(transport, GAIA_COMMAND_READ_FILE, FILE_PACKET_HEADER_SIZE + length, response);

                free(response);
                ++gaia->pfs_sequence;

                status = GAIA_STATUS_SUCCESS;
            }
        }
    }

    if (status != GAIA_STATUS_SUCCESS)
        send_simple_response(transport, GAIA_COMMAND_READ_FILE, status);
    }


/*************************************************************************
NAME
    close_file

DESCRIPTION
    Close the open data file

*/
static void close_file(gaia_transport *transport)
{
    uint8 response = gaia->pfs_open_stream;
    uint8 status = GAIA_STATUS_SUCCESS;

    GAIA_DEBUG(("gaia: close file\n"));

    if (gaia->pfs_state == PFS_FILE)
    {
        if (SourceClose(gaia->pfs.source))
            gaia->pfs_state = PFS_NONE;

        else
            status = GAIA_STATUS_INSUFFICIENT_RESOURCES;
    }

    else
        status = GAIA_STATUS_INCORRECT_STATE;

    send_ack(transport, GAIA_VENDOR_QTIL, GAIA_COMMAND_CLOSE_FILE,
             status, 1, &response);
}

/*************************************************************************
NAME
    isRwcpEnabled

DESCRIPTION
    Checks if RWCP is enabled

*/

static bool isRwcpEnabled(void)
{
    return (gaia->data_endpoint_mode == GAIA_DATA_ENDPOINT_MODE_RWCP);
}

/*************************************************************************
NAME
    isHostUpgradeDataBytesReq

DESCRIPTION
    Checks if the received data is UPGRADE_HOST_DATA_BYTES_REQ

*/
bool isHostUpgradeDataBytesReq(uint8* data, uint16 size_data)
{
    return (data != NULL && (size_data > GAIA_UPGRADE_DATA_BYTES_REQ_OFFSET) && data[0] == GAIA_UPGRADE_DATA_BYTES_REQ);
}

/*************************************************************************
NAME
    upgrade_connect

DESCRIPTION
    Connect a GAIA transport to the connection library

*/
static void upgrade_connect(gaia_transport *transport)
{
    if (gaia->upgrade_transport == NULL)
    {
        gaia->upgrade_transport = transport;
        /* Connect transfer task and request UPGRADE_TRANSPORT_DATA_CFM
         * messages, but don't request several blocks */
        UpgradeTransportConnectRequest(&gaia->task_data, TRUE, FALSE);

        if(gaia->upgrade_transport->type==gaia_transport_gatt)
        {
            if (isRwcpEnabled())
            {
                RwcpServerInit(GAIA_GATT_HEADER_SIZE + GAIA_UPGRADE_HEADER_SIZE);
                RwcpSetClientTask(&gaia->task_data);
            }
            /* Obtain source from Enhanced ATT streams */
            transport->state.gatt.src = StreamAttServerSource(transport->state.gatt.cid);
            GAIA_DEBUG(("Enhanced ATT Streams getting registered with cid-->%x\n",transport->state.gatt.cid));
            if(transport->state.gatt.src)
            {
                GAIA_DEBUG(("ATT Stream Source valid\n"));
                /* Get the adjusted handle from GattServer Database */
                transport->state.gatt.handle_data_endpoint = GattManagerGetServerDatabaseHandle(&gaia->task_data, HANDLE_GAIA_DATA_ENDPOINT);
                PanicFalse(StreamAttAddHandle(transport->state.gatt.src, transport->state.gatt.handle_data_endpoint));
                GAIA_DEBUG(("GAIA Task registered for recieving the messages\n"));
                /* Configure the source for getting all the messages */
                SourceConfigure(transport->state.gatt.src, VM_SOURCE_MESSAGES, VM_MESSAGES_SOME);
                /* Associate the task with the stream source */
                MessageStreamTaskFromSink(StreamSinkFromSource(transport->state.gatt.src), &gaia->task_data);
                /* Obtaining the sink from the ATT streams and storing the same */
                transport->state.gatt.snk = StreamAttServerSink(transport->state.gatt.cid);
                /* Configure the sink such that messages are not received */
                SinkConfigure(transport->state.gatt.snk, VM_SINK_MESSAGES, VM_MESSAGES_NONE);
                transport->state.gatt.handle_response_endpoint = GattManagerGetServerDatabaseHandle(&gaia->task_data, HANDLE_GAIA_RESPONSE_ENDPOINT);
            }
            else
            {
                transport->state.gatt.snk = 0;
                GAIA_DEBUG(("ATT Streams were not configured.\n")); 
            }
        }
    }
    else
    {
        send_incorrect_state(transport, GAIA_COMMAND_VM_UPGRADE_CONNECT);
    }
}


/*************************************************************************
NAME
    upgrade_disconnect

DESCRIPTION
    Connect a GAIA transport to the connection library

*/
static void upgrade_disconnect(gaia_transport *transport)
{
    if (gaia->upgrade_transport == transport)
    {
        UpgradeTransportDisconnectRequest();
        gaia->upgrade_transport = NULL;
        send_success(transport, GAIA_COMMAND_VM_UPGRADE_DISCONNECT);
        /* remove the handles attached previously.*/
        StreamAttSourceRemoveAllHandles(transport->state.gatt.cid);
        transport->state.gatt.snk = 0;
    }
    else
    {
        send_incorrect_state(transport, GAIA_COMMAND_VM_UPGRADE_DISCONNECT);
    }
}

/*************************************************************************
NAME
    createUpgradeDataReq

DESCRIPTION
    Allocates and builds an upgrade data request buffer with the provided uint8 data

*/
static uint8* createUpgradeDataReq(uint16 upgrade_len, uint8 *data, uint8 more_data)
{
    uint8* buffer  = PanicUnlessMalloc(UPGRADE_MAX_PARTITION_DATA_BLOCK_SIZE + GAIA_UPGRADE_HEADER_SIZE);
    uint8 count = 0;

    buffer[count++] = GAIA_UPGRADE_HOST_DATA_REQ;
    buffer[count++] = HIGH((upgrade_len + 1));
    buffer[count++] = LOW((upgrade_len + 1));
    buffer[count++] = more_data;

    memmove(&buffer[GAIA_UPGRADE_HEADER_SIZE], data, upgrade_len);
    return buffer;
}

/*************************************************************************
NAME
    getNextUpgradeData

DESCRIPTION
    Fetch the next chunk of upgrade data from source, build an upgrade data 
    request packet and send it to the upgrade library.

*/
static void getNextUpgradeData(gaia_transport *transport)
{
    uint16 available_bytes;
    uint16 upgrade_len;
    uint8 *data, *upgrade_data_buffer;
    uint8 more_data = 0;
    Source source = gaiaTransportGetSource(transport);

    if(transport->type == gaia_transport_gatt)
    {
        available_bytes = SourceBoundary(source);
    }
    else
    {
        available_bytes = SourceSize(source);
    }

    data =  (uint8 *) SourceMap(source);

    GAIA_DEBUG(("getNextUpgradeData available_bytes %d\n", available_bytes));
    upgrade_len = MIN(available_bytes, UPGRADE_MAX_PARTITION_DATA_BLOCK_SIZE);
    GAIA_DEBUG(("getNextUpgradeData upgrade_len %d\n", upgrade_len));

    if(upgrade_len == available_bytes)
    {
         more_data = gaia->upgrade_large_data.more_data;
         gaia->upgrade_large_data.in_progress = FALSE;
    }

    upgrade_data_buffer = createUpgradeDataReq(upgrade_len, data, more_data);

    UpgradeProcessDataRequest(upgrade_len + GAIA_UPGRADE_HEADER_SIZE , upgrade_data_buffer);

    free(upgrade_data_buffer);
    SourceDrop(source, upgrade_len);
}

/*************************************************************************
NAME
    isLargeDataUpgrade

DESCRIPTION
    Finds out if the upgrade data is larger than the upgrade library's buffer size

*/
static bool isLargeDataUpgrade(gaia_transport *transport, uint8 *payload, uint8 payload_length)
{
    if(transport->type != gaia_transport_spp && transport->type != gaia_transport_gatt)
        return FALSE;

    if(payload_length <= (UPGRADE_MAX_PARTITION_DATA_BLOCK_SIZE + GAIA_UPGRADE_HEADER_SIZE))
        return FALSE;
    
    if(payload[GAIA_UPGRADE_COMMAND_ID_OFFSET] != GAIA_UPGRADE_HOST_DATA_REQ)
        return FALSE;

    return TRUE;
}

/*************************************************************************
NAME
    upgrade_control

DESCRIPTION
    Tunnel a VM Upgrade Protocol command to the upgrade library

*/
static void upgrade_control(gaia_transport *transport, uint8 *payload, uint8 payload_length)
{
    uint16 gaia_header_bytes;

    if (transport == gaia->upgrade_transport)
    {
        if(isLargeDataUpgrade(transport, payload, payload_length))
        {
            gaia->upgrade_large_data.in_progress = TRUE;
            gaia->upgrade_large_data.more_data = payload[GAIA_UPGRADE_HEADER_MORE_DATA_OFFSET];
            gaia_header_bytes = (transport->type == gaia_transport_gatt)?
                                                    (GAIA_GATT_OFFS_PAYLOAD + GAIA_HANDLE_SIZE + RWCP_HEADER_SIZE):
                                                    GAIA_OFFS_PAYLOAD;
            SourceDrop(gaiaTransportGetSource(transport), gaia_header_bytes + GAIA_UPGRADE_HEADER_SIZE);
            getNextUpgradeData(transport);
        }
        else
        {
            UpgradeProcessDataRequest(payload_length, payload);
        }
    }
    else
    {
        send_incorrect_state(transport, GAIA_COMMAND_VM_UPGRADE_CONTROL);
    }
}


/*************************************************************************
NAME
    gaia_handle_data_transfer_command

DESCRIPTION
    Handle a Data Transfer command or return FALSE if we can't

*/
static bool gaia_handle_data_transfer_command(gaia_transport *transport,
    uint16 command_id, uint8 payload_length, uint8 *payload)
{
#ifdef HAVE_PARTITION_FILESYSTEM

    GAIA_DEBUG(("gaia: PFS %u\n", gaia->have_pfs));

    /* gaia->have_pfs is a check for filesystem existence. It doesn't work.
       TODO: fix it
       (transport->type != gaia_transport_gatt) was introduced as a safeguard
       to not use GAIA over GATT until upgrade library will be ready.
       We want GAIA over GATT so check is disabled.
    */
#if 0
    if(gaia->have_pfs && (transport->type != gaia_transport_gatt))
#endif
    {
        switch (command_id)
        {
        case GAIA_COMMAND_GET_STORAGE_PARTITION_STATUS:
            if (payload_length == 2)
                send_pfs_status(transport, payload[0], payload[1]);

            else
                send_invalid_parameter(transport, command_id);

            return TRUE;


        case GAIA_COMMAND_OPEN_STORAGE_PARTITION:
            open_storage_partition(transport, payload, payload_length);
            return TRUE;


        case GAIA_COMMAND_MOUNT_STORAGE_PARTITION:
            mount_storage_partition(transport, payload, payload_length);
            return TRUE;


        case GAIA_COMMAND_WRITE_STORAGE_PARTITION:
            write_partition_data(transport, payload, payload_length);
            return TRUE;


        case GAIA_COMMAND_CLOSE_STORAGE_PARTITION:
            if ((payload_length == 1) && (payload[0] == gaia->pfs_open_stream))
                close_storage_partition(transport);

            else
                send_invalid_parameter(transport, command_id);

            return TRUE;


        case GAIA_COMMAND_DFU_REQUEST | GAIA_ACK_MASK:
            if (gaia->dfu_state == DFU_WAITING)
            {
                bool ok = (payload_length == 1) && (payload[0] == GAIA_STATUS_SUCCESS);

                if (ok)
                    gaia->dfu_state = DFU_READY;

                else
                {
                    dfu_reset_timeout(0);
                    gaia->dfu_state = DFU_IDLE;
                }


                dfu_confirm(transport, ok);
            }
            return TRUE;

#ifdef HAVE_DFU_FROM_SQIF
        case GAIA_COMMAND_DFU_BEGIN:
            dfu_begin(transport, payload, payload_length);
            return TRUE;
#endif
        }
    }
#endif /* def HAVE_PARTITION_FILESYSTEM */

    switch (command_id)
    {
    case GAIA_COMMAND_OPEN_FILE:
        open_file(transport, payload, payload_length);
        return TRUE;


    case GAIA_COMMAND_READ_FILE:
        read_file(transport, payload, payload_length);
        return TRUE;


    case GAIA_COMMAND_CLOSE_FILE:
        if ((payload_length == 1) && (payload[0] == gaia->pfs_open_stream))
            close_file(transport);

        else
            send_invalid_parameter(transport, command_id);

        return TRUE;

    case GAIA_COMMAND_VM_UPGRADE_CONNECT:
        GAIA_DEBUG(("GAIA_COMMAND_VM_UPGRADE_CONNECT\n"));
        upgrade_connect(transport);
        return TRUE;

    case GAIA_COMMAND_VM_UPGRADE_DISCONNECT:
        GAIA_DEBUG(("GAIA_COMMAND_VM_UPGRADE_DISCONNECT\n"));
        upgrade_disconnect(transport);
        return TRUE;

    case GAIA_COMMAND_VM_UPGRADE_CONTROL:
        GAIA_DEBUG(("GAIA_COMMAND_VM_UPGRADE_CONTROL\n"));
        upgrade_control(transport, payload, payload_length);
        return TRUE;
    }

    return FALSE;
}

/*************************************************************************
NAME
    validate_payload_length

DESCRIPTION
    Check that the Gaia command payload is within the expected range.
    Returns TRUE if so, otherwise responds immediately with
    GAIA_STATUS_INVALID_PARAMETER and returns FALSE
*/
static bool validate_payload_length(gaia_transport *transport, uint16 command_id, uint8 length, uint8 min, uint8 max)
{
    if ((length < min) || (length > max))
    {
        send_invalid_parameter(transport, command_id);
        return FALSE;
    }

    return TRUE;
}


/*************************************************************************
NAME
    ps_retrieve

DESCRIPTION
    Process Gaia Debugging Commands GAIA_COMMAND_RETRIEVE_PS_KEY and
    GAIA_COMMAND_RETRIEVE_FULL_PS_KEY
*/
static void ps_retrieve(gaia_transport *transport, uint16 command_id, uint16 key)
{
    uint16 key_length;

    if (command_id == GAIA_COMMAND_RETRIEVE_FULL_PS_KEY)
        key_length = PsFullRetrieve(key, NULL, 0);

    else
        key_length = PsRetrieve(key, NULL, 0);

    if (key_length == 0)
        send_invalid_parameter(transport, command_id);

    else
    {
        uint16 *key_data = malloc((key_length + 1) * sizeof(uint16));

        if (key_data)
        {
            if (command_id == GAIA_COMMAND_RETRIEVE_FULL_PS_KEY)
                PsFullRetrieve(key, key_data + 1, key_length);

            else
                PsRetrieve(key, key_data + 1, key_length);

            *key_data = key;

            send_ack_16(transport, GAIA_VENDOR_QTIL, command_id, GAIA_STATUS_SUCCESS,
                                 key_length + 1, key_data);
            free(key_data);
        }

        else
            send_insufficient_resources(transport, command_id);
    }
}


/*************************************************************************
NAME
    store_ps_key

DESCRIPTION
    Process Gaia Debugging Command GAIA_COMMAND_STORE_PS_KEY
*/
static void store_ps_key(gaia_transport *transport, uint16 payload_length, uint8 *payload)
{
    uint16 status;

    if (((payload_length & 1) != 0) || (payload_length < 2))
        status = GAIA_STATUS_INVALID_PARAMETER;

    else
    {
        uint16 key_index = W16(payload);
        uint16 *packed_key = malloc((payload_length / 2) * sizeof(uint16));

        if (!packed_key)
            status = GAIA_STATUS_INSUFFICIENT_RESOURCES;

        else
        {
            uint16 key_length = PS_SIZE_ADJ((payload_length - 2) / 2);
            uint16 ps_length;

            wpack(packed_key, payload + 2, key_length);

            ps_length = PsStore(key_index, packed_key, key_length);
            free(packed_key);

            if (key_length && !ps_length)
                status = GAIA_STATUS_INSUFFICIENT_RESOURCES;

            else
                status = GAIA_STATUS_SUCCESS;
        }
    }

    send_simple_response(transport, GAIA_COMMAND_STORE_PS_KEY, status);
}


/*************************************************************************
NAME
    get_memory_slots

DESCRIPTION
    Process Gaia Debugging Command GAIA_COMMAND_GET_MEMORY_SLOTS
    Responds with the number of malloc() slots and the amount of
    writable storage available for PS keys
*/
static void get_memory_slots(gaia_transport *transport)
{
    uint16 malloc_slots = VmGetAvailableAllocations();
    uint16 ps_space = PsFreeCount(0);
    uint8 payload[4];

    payload[0] = HIGH(malloc_slots);
    payload[1] = LOW(malloc_slots);

    payload[2] = HIGH(ps_space);
    payload[3] = LOW(ps_space);

    send_success_payload(transport, GAIA_COMMAND_GET_MEMORY_SLOTS, 4, payload);
}


/*************************************************************************
NAME
    send_kalimba_message

DESCRIPTION
    Send an arbitrary (short) message to the Kalimba DSP
*/
static void send_kalimba_message(gaia_transport *transport, uint8 *payload)
{
    UNUSED(payload);
    send_insufficient_resources(transport, GAIA_COMMAND_SEND_KALIMBA_MESSAGE);
}


/*************************************************************************
NAME
    gaia_handle_debug_command

DESCRIPTION
    Handle a debugging command or return FALSE if we can't
*/
static bool gaia_handle_debug_command(gaia_transport *transport, uint16 command_id,
                                      uint8 payload_length, uint8 *payload)
{
    switch (command_id)
    {
    case GAIA_COMMAND_NO_OPERATION:
        send_success(transport, GAIA_COMMAND_NO_OPERATION);
        return TRUE;

    case GAIA_COMMAND_RETRIEVE_PS_KEY:
    case GAIA_COMMAND_RETRIEVE_FULL_PS_KEY:
        if (validate_payload_length(transport, command_id, payload_length, 2, 2))
            ps_retrieve(transport, command_id, W16(payload));

        return TRUE;

    case GAIA_COMMAND_STORE_PS_KEY:
        store_ps_key(transport, payload_length, payload);
        return TRUE;

    case GAIA_COMMAND_FLOOD_PS:
        PsFlood();
        send_success(transport, GAIA_COMMAND_FLOOD_PS);
        return TRUE;

    case GAIA_COMMAND_GET_MEMORY_SLOTS:
        get_memory_slots(transport);
        return TRUE;

    case GAIA_COMMAND_DELETE_PDL:
        ConnectionSmDeleteAllAuthDevices(0);
        send_success(transport, GAIA_COMMAND_DELETE_PDL);
        return TRUE;

    case GAIA_COMMAND_SEND_APPLICATION_MESSAGE:
        if (validate_payload_length(transport, command_id, payload_length, 2, 2))
            MessageSend(gaia->app_task, W16(payload), NULL);

        return TRUE;

    case GAIA_COMMAND_SEND_KALIMBA_MESSAGE:
        if (validate_payload_length(transport, command_id, payload_length, 10, 10))
            send_kalimba_message(transport, payload);

        return TRUE;
    }

    return FALSE;
}


/*************************************************************************
NAME
    gaia_handle_notification_command

DESCRIPTION
    Handle a debugging command or return FALSE if we can't
*/
static bool gaia_handle_notification_command(gaia_transport *transport, uint16 command_id,
                                             uint8 payload_length, uint8 *payload)
{
    switch (command_id)
    {
    case GAIA_COMMAND_REGISTER_NOTIFICATION:
        {
            GAIA_DEBUG(("GAIA_COMMAND_REGISTER_NOTIFICATION payload len %d\n", payload_length));
            if (payload_length == 0)
                send_invalid_parameter(transport, GAIA_COMMAND_REGISTER_NOTIFICATION);

            else if (GaiaGetAppWillHandleNotification(payload[0]))
            {
                GAIA_DEBUG(("GaiaGetAppWillHandleNotification\n"));
                return FALSE;
            }
            else
                register_notification(transport, payload_length, payload);

            return TRUE;
        }
    case GAIA_COMMAND_GET_NOTIFICATION:
        if (payload_length != 1)
            send_invalid_parameter(transport, GAIA_COMMAND_GET_NOTIFICATION);

        else if (GaiaGetAppWillHandleNotification(payload[0]))
            return FALSE;

        else
            send_notification_setting(transport, payload[0]);

        return TRUE;

    case GAIA_COMMAND_CANCEL_NOTIFICATION:
        if (payload_length != 1)
            send_invalid_parameter(transport, GAIA_COMMAND_CANCEL_NOTIFICATION);

        else if (GaiaGetAppWillHandleNotification(payload[0]))
            return FALSE;

        else
            cancel_notification(transport, payload[0]);

        return TRUE;

    case GAIA_ACK_NOTIFICATION:
        if ((payload_length > 0) && GaiaGetAppWillHandleNotification(payload[0]))
            return FALSE;

        return TRUE;
    }

    return FALSE;
}


/*************************************************************************
NAME
    check_enable

DESCRIPTION
    Validate SESSION_ENABLE credentials
*/
static bool check_enable(gaia_transport *transport, uint16 vendor_id, uint16 command_id, uint8 payload_length, uint8 *payload)
{
    uint16 usb_vid;
    bool ok;

    UNUSED(transport);
    if (command_id & GAIA_ACK_MASK)
        ok = TRUE;

    else if (vendor_id != GAIA_VENDOR_QTIL)
        ok = FALSE;

    else if (command_id == GAIA_COMMAND_GET_SESSION_ENABLE)
        ok = TRUE;

    else if ((command_id != GAIA_COMMAND_SET_SESSION_ENABLE) ||
        (payload_length != 3))
        ok = FALSE;

    else if (PsFullRetrieve(PSKEY_USB_VENDOR_ID, &usb_vid, 1) == 0)
        ok = FALSE;

    else if (W16(payload + 1) != usb_vid)
        ok = FALSE;

    else
        ok = TRUE;

    GAIA_DEBUG(("gaia: %04X:%04X en %u\n", vendor_id, command_id, ok));
    return ok;
}

/*************************************************************************
NAME
    check_battery_thresholds

DESCRIPTION
    Called periodically to see if the battery voltage has reached one of
    the notification thresholds
*/
static void check_battery_thresholds(gaia_transport *transport)
{
    int16 voltage = read_battery_voltage();

    if (voltage != 0)
    {
        bool notify = FALSE;

    /*  Check low thresholds  */
        if ((transport->threshold_count_lo_battery > 0)
            && (voltage <= transport->battery_lo_threshold[0]) 
            && !transport->sent_notification_lo_battery_0)
        {
            notify = TRUE;
            transport->sent_notification_lo_battery_0 = TRUE;
        }

        else if (transport->sent_notification_lo_battery_0
            && (voltage > transport->battery_lo_threshold[0] + GAIA_BATT_HYSTERESIS))
            transport->sent_notification_lo_battery_0 = FALSE;

        if ((transport->threshold_count_lo_battery > 1)
            && (voltage <= transport->battery_lo_threshold[1]) 
            && !transport->sent_notification_lo_battery_1)
        {
            notify = TRUE;
            transport->sent_notification_lo_battery_1 = TRUE;
        }

        else if (transport->sent_notification_lo_battery_1
            && (voltage > transport->battery_lo_threshold[1] + GAIA_BATT_HYSTERESIS))
            transport->sent_notification_lo_battery_1 = FALSE;

        GAIA_THRES_DEBUG(("gaia: chk bat lo %d (%d %d) %d\n",
               voltage, 
               transport->battery_lo_threshold[0],
               transport->battery_lo_threshold[1],
               notify));

        if (notify)
        {
            uint8 payload[2];

            payload[0] = HIGH(voltage);
            payload[1] = LOW(voltage);

            send_notification(transport, GAIA_EVENT_BATTERY_LOW_THRESHOLD, 2, payload);
        }

        /*  Check high thresholds  */
        notify = FALSE;

        if ((transport->threshold_count_hi_battery > 0)
            && (voltage >= transport->battery_hi_threshold[0]) 
            && !transport->sent_notification_hi_battery_0)
        {
            notify = TRUE;
            transport->sent_notification_hi_battery_0 = TRUE;
        }

        else if (transport->sent_notification_hi_battery_0
            && (voltage < transport->battery_hi_threshold[0] - GAIA_BATT_HYSTERESIS))
            transport->sent_notification_hi_battery_0 = FALSE;

        if ((transport->threshold_count_hi_battery > 1)
            && (voltage >= transport->battery_hi_threshold[1]) 
            && !transport->sent_notification_hi_battery_1)
        {
            notify = TRUE;
            transport->sent_notification_hi_battery_1 = TRUE;
        }

        else if (transport->sent_notification_hi_battery_1
            && (voltage < transport->battery_hi_threshold[1] - GAIA_BATT_HYSTERESIS))
            transport->sent_notification_hi_battery_1 = FALSE;

        GAIA_THRES_DEBUG(("gaia: chk batt hi %d (%d %d) %d\n",
               voltage, 
               transport->battery_hi_threshold[0],
               transport->battery_hi_threshold[1],
               notify));

        if (notify)
        {
            uint8 payload[2];

            payload[0] = HIGH(voltage);
            payload[1] = LOW(voltage);

            send_notification(transport, GAIA_EVENT_BATTERY_HIGH_THRESHOLD, 2, payload);
        }
    }
}

/*************************************************************************
NAME
    check_rssi_thresholds

DESCRIPTION
    Called periodically to see if the RSSI has reached one of the
    notification thresholds
*/
static void check_rssi_thresholds(gaia_transport *transport)
{
    bool notify = FALSE;
    int16 rssi = 0;

    /* get the current RSSI */
    SinkGetRssi(gaiaTransportGetSink(transport), &rssi);

    /*  Check low thresholds  */
    if ((transport->threshold_count_lo_rssi > 0)
        && (rssi <= transport->rssi_lo_threshold[0])
        && !transport->sent_notification_lo_rssi_0)
    {
        notify = TRUE;
        transport->sent_notification_lo_rssi_0 = TRUE;
    }

    else if (transport->sent_notification_lo_rssi_0 && (rssi > (transport->rssi_lo_threshold[0] + GAIA_RSSI_HYSTERESIS)))
        transport->sent_notification_lo_rssi_0 = FALSE;

    if ((transport->threshold_count_lo_rssi > 1)
        && (rssi <= transport->rssi_lo_threshold[1])
        && !transport->sent_notification_lo_rssi_1)
    {
        notify = TRUE;
        transport->sent_notification_lo_rssi_1 = TRUE;
    }

    else if (transport->sent_notification_lo_rssi_1 && (rssi > (transport->rssi_lo_threshold[1] + GAIA_RSSI_HYSTERESIS)))
        transport->sent_notification_lo_rssi_1 = FALSE;

    GAIA_THRES_DEBUG(("gaia: chk RSSI lo %d (%d %d) %d\n",
           rssi,
           transport->rssi_lo_threshold[0],
           transport->rssi_lo_threshold[1],
           notify));

    if (notify)
    {
        uint8 payload = LOW(rssi);
        send_notification(transport, GAIA_EVENT_RSSI_LOW_THRESHOLD, 1, &payload);
    }

    /*  Check high thresholds  */
    notify = FALSE;

    if ((transport->threshold_count_hi_rssi > 0)
        && (rssi >= transport->rssi_hi_threshold[0])
        && !transport->sent_notification_hi_rssi_0)
    {
        notify = TRUE;
        transport->sent_notification_hi_rssi_0 = TRUE;
    }

    else if (transport->sent_notification_hi_rssi_0 && ((rssi < transport->rssi_hi_threshold[0] - GAIA_RSSI_HYSTERESIS)))
        transport->sent_notification_hi_rssi_0 = FALSE;

    if ((transport->threshold_count_hi_rssi > 1)
        && (rssi >= transport->rssi_hi_threshold[1])
        && !transport->sent_notification_hi_rssi_1)
    {
        notify = TRUE;
        transport->sent_notification_hi_rssi_1 = TRUE;
    }

    else if (transport->sent_notification_hi_rssi_1 && (rssi < (transport->rssi_hi_threshold[1] - GAIA_RSSI_HYSTERESIS)))
        transport->sent_notification_hi_rssi_1 = FALSE;

    GAIA_THRES_DEBUG(("gaia: chk RSSI hi %d (%d %d) %d\n",
           rssi,
           transport->rssi_hi_threshold[0],
           transport->rssi_hi_threshold[1],
           notify));

    if (notify)
    {
        uint8 payload = LOW(rssi);
        send_notification(transport, GAIA_EVENT_RSSI_HIGH_THRESHOLD, 1, &payload);
    }
}





/*************************************************************************
NAME
    gaia_init

DESCRIPTION
    Initialise the library
*/
static void gaia_init(void)
{
    MESSAGE_PMAKE(status, GAIA_INIT_CFM_T);
    
    gaia->command_locus_bits = 0;

    /*  Default API minor version (may be overridden by GaiaSetApiMinorVersion())  */
    gaia->api_minor = GAIA_API_VERSION_MINOR;

    /*  Library handles RSSI, Battery, DFU State and VM Upgrade Protocol events  */
    gaia->event_locus_bits = ~(ULBIT(GAIA_EVENT_RSSI_LOW_THRESHOLD)
            | ULBIT(GAIA_EVENT_RSSI_HIGH_THRESHOLD)
            | ULBIT(GAIA_EVENT_BATTERY_LOW_THRESHOLD)
            | ULBIT(GAIA_EVENT_BATTERY_HIGH_THRESHOLD)
            | ULBIT(GAIA_EVENT_DFU_STATE)
            | ULBIT(GAIA_EVENT_VMUP_PACKET));

    /*  So we can use AUDIO_BUSY interlocking  */
    AudioLibraryInit();

    /*  Perform custom SDP registration */
    register_custom_sdp(PSKEY_MOD_MANUF0);

#ifdef HAVE_PARTITION_FILESYSTEM
    {
        uint32 pfs_type;

    /*  Probe partition 0.  We don't really care about the type, just its existence  */
        gaia->have_pfs = PartitionGetInfo(PARTITION_SERIAL_FLASH, 0, PARTITION_INFO_TYPE, &pfs_type);
    }
#endif /* def HAVE_PARTITION_FILESYSTEM */

    status->success = TRUE;
    MessageSend(gaia->app_task, GAIA_INIT_CFM, status);
}

/*************************************************************************
NAME
    dfu_finish

DESCRIPTION
    Clean up after a DFU transfer
*/
static void dfu_finish(bool success)
{
    dfu_reset_timeout(0);

    gaia->pfs_raw_size = 0;
    SinkClose(gaia->pfs.sink);
    SetAudioBusy(NULL);

    if (success)
    {
        dfu_send_state(GAIA_DFU_STATE_VERIFICATION);
    }

    else
    {
        gaia->pfs_state = PFS_NONE;
        dfu_send_state(GAIA_DFU_STATE_DOWNLOAD_FAILURE);
    }
}

/*************************************************************************
 *  NAME
 *      GaiaRwcpSendNotification
 *
 *  DESCRIPTION
 *      This function handles payloads sent from RWCP server
 */
void GaiaRwcpSendNotification(uint8 *payload, uint16 payload_length)
{
    gaia_transport *trans = gaia->upgrade_transport;
    GAIA_DEBUG(("send_rwcp_notification\n"));

    /* Send notifications via ATT streams if streams were configured and space was available */
    if(trans->state.gatt.snk && (SinkSlack(trans->state.gatt.snk)) >= payload_length + GAIA_HANDLE_SIZE)
    {
        uint8 *snkData = SinkMap(trans->state.gatt.snk);
        uint16 offset = SinkClaim(trans->state.gatt.snk, (payload_length + GAIA_HANDLE_SIZE));
        GAIA_TRANS_DEBUG(("send_rwcp_notification snkData-->%d\t size_response-->%d\n", offset, payload_length));

        snkData[offset++] = LOW(trans->state.gatt.handle_data_endpoint);
        snkData[offset++] = HIGH(trans->state.gatt.handle_data_endpoint);
        memcpy(snkData+offset, payload, payload_length);
        GAIA_TRANS_DEBUG(("GaiaRwcpSendNotification:Notification Response flushed\n"));
        SinkFlush(trans->state.gatt.snk, payload_length + GAIA_HANDLE_SIZE);
    }

    else
    {
        if(!trans->state.gatt.snk)
        {
            GAIA_DEBUG(("GaiaRwcpSendNotification:Stream not configured\n"));
        }
        GAIA_DEBUG(("GaiaRwcpSendNotification:Notification was not sent via streams\n"));
    }

    free(payload);
}

/*************************************************************************
 *  NAME
 *      GaiaRwcpProcessCommand
 *
 *  DESCRIPTION
 *      This function processes the GAIA packet and extracts vendor_id and command_id
 */
void GaiaRwcpProcessCommand(uint8 *command, uint16 size_command)
{
    gaia_transport *transport = gaia->upgrade_transport;

#ifdef DEBUG_GAIA_TRANSPORT
    uint16 i;
    GAIA_TRANS_DEBUG(("gaia: command:"));
    for (i = 0; i < size_command; ++i)
    {
        GAIA_TRANS_DEBUG((" %02X",  command[i]));
    }
    GAIA_TRANS_DEBUG(("\n"));
#endif

    if (size_command >= GAIA_GATT_OFFS_PAYLOAD)
    {
        uint8 *payload = NULL;
        uint16 size_payload = size_command - GAIA_GATT_OFFS_PAYLOAD;
        uint16 vendor_id = (command[GAIA_GATT_OFFS_VENDOR_ID_H] << 8) | command[GAIA_GATT_OFFS_VENDOR_ID_L];
        uint16 command_id = (command[GAIA_GATT_OFFS_COMMAND_ID_H] << 8) | command[GAIA_GATT_OFFS_COMMAND_ID_L];

        if(size_payload)
        {
            payload = command + GAIA_GATT_OFFS_PAYLOAD;
        }

        transport->state.gatt.size_response = GAIA_GATT_HEADER_SIZE + GAIA_GATT_RESPONSE_STATUS_SIZE;
        memmove(transport->state.gatt.response, command, GAIA_GATT_HEADER_SIZE);
        transport->state.gatt.response[GAIA_GATT_OFFS_COMMAND_ID_H] |= GAIA_ACK_MASK_H;
        transport->state.gatt.response[GAIA_GATT_OFFS_PAYLOAD] = GAIA_STATUS_IN_PROGRESS;

        gaiaProcessCommand(transport, vendor_id, command_id, size_payload, payload);
    }
}
/*************************************************************************
 *  NAME
 *      handle_upgrade_transport_data_cfm
 *
 *  DESCRIPTION
 *      This function handles the UPGRADE_TRANSPORT_DATA_CFM
 */
void handle_upgrade_transport_data_cfm(uint8 packet_type, upgrade_status_t status)
{
    GAIA_DEBUG(("GAIA: UPGRADE_PROCESS_DATA_CFM\n"));

    if(gaia->upgrade_large_data.in_progress)
    {
        getNextUpgradeData(gaia->upgrade_transport);
        return;
    }
   
    if(gaia->upgrade_transport->type == gaia_transport_gatt)
    {
        gaiaTransportGattProcessSource();
    }
    
    if (isRwcpEnabled() && (packet_type == GAIA_UPGRADE_HOST_DATA_REQ))
        return;

    send_ack(gaia->upgrade_transport, GAIA_VENDOR_QTIL, GAIA_COMMAND_VM_UPGRADE_CONTROL, status, 0, NULL);
}

/*************************************************************************
 *  NAME
 *      handle_upgrade_transport_data_ind
 *
 *  DESCRIPTION
 *      This function handles the UPGRADE_TRANSPORT_DATA_IND
 */
static void handle_upgrade_transport_data_ind(bool is_data_state, uint8* data, uint16 size_data)
{
    static bool first = FALSE;

    GAIA_DEBUG(("GAIA: UPGRADE_TRANSPORT_DATA_IND\n"));

    if(is_data_state && isHostUpgradeDataBytesReq(data, size_data))
    {
        if(first == FALSE)
        {
            if(ByteUtilsGet4BytesFromStream(&data[GAIA_UPGRADE_DATA_SIZE_OFFSET]) == UPGRADE_MAX_PARTITION_DATA_BLOCK_SIZE)
            {
                uint32 size = 0;

                size = UpgradeGetPartitionSizeInPartitionDataState();
                data[GAIA_UPGRADE_DATA_BYTES_REQ_OFFSET] = size & 0xFF;
                data[GAIA_UPGRADE_DATA_BYTES_REQ_OFFSET - 1] = ( size >> 8 ) & 0xFF;
                data[GAIA_UPGRADE_DATA_BYTES_REQ_OFFSET - 2] = ( size >> 16 ) & 0xFF;
                data[GAIA_UPGRADE_DATA_BYTES_REQ_OFFSET - 3] = ( size >> 24 ) & 0xFF;
                first = TRUE;
            }
            send_notification(gaia->upgrade_transport, GAIA_EVENT_VMUP_PACKET, size_data, data);
        }
    }
    else
    {
        first = FALSE;
        send_notification(gaia->upgrade_transport, GAIA_EVENT_VMUP_PACKET, size_data, data);
    }
}

/*************************************************************************
NAME
    message_handler

DESCRIPTION
    Handles internal messages and anything from the underlying SPP
*/
static void message_handler(Task task, MessageId id, Message message)
{
/*  If SDP registration confirmation is caused by a custom record,
    don't bother the transport handlers with it
*/
    if ((id == CL_SDP_REGISTER_CFM) && gaia->custom_sdp)
    {
        GAIA_DEBUG(("gaia: CL_SDP_REGISTER_CFM (C): %d\n",
                    ((CL_SDP_REGISTER_CFM_T *) message)->status));

        gaia->custom_sdp = FALSE;
        return;
    }

    if (id == MESSAGE_MORE_DATA && gaia->pfs_raw_size)
    {
    /*  Data is to be copied directly to the PFS sink  */
        MessageMoreData *m = (MessageMoreData *) message;
        uint16 size = SourceSize(m->source);

        GAIA_DFU_DEBUG(("gaia: raw %u of %lu\n", size, gaia->pfs_raw_size));

        if (size)
        {
            dfu_reset_timeout(DFU_PACKET_TIMEOUT);

            if (size > gaia->pfs_raw_size)
            {
                GAIA_DEBUG(("gaia: DFU: too much data\n"));

                SourceDrop(m->source, size);
                dfu_finish(FALSE);
                return;
            }

            else
            {
                while (size)
                {
                    uint16 chunk = (size > GAIA_RAW_CHUNK_SIZE) ? GAIA_RAW_CHUNK_SIZE : size;

                    if (SinkClaim(gaia->pfs.sink, chunk) == BAD_SINK_CLAIM)
                    {
                        GAIA_DEBUG(("gaia: DFU: bad sink %u\n", chunk));
                        SourceDrop(m->source, size);
                        dfu_finish(FALSE);
                        return;
                    }

                    else
                    {
                        memmove(SinkMap(gaia->pfs.sink), SourceMap(m->source), chunk);
                        SinkFlush(gaia->pfs.sink, chunk);
                        SourceDrop(m->source, chunk);
                    }

                    size -= chunk;
                    gaia->pfs_raw_size -= chunk;
                }

                if (gaia->pfs_raw_size == 0)
                {
                    GAIA_DEBUG(("gaia: DFU: transfer complete\n"));
                    dfu_finish(TRUE);
                }
            }
        }

        return;
    }

    /* see if a transport can handle this message */
    /* TODO handle multipoint case, loop through all transports? */
    if (gaiaTransportHandleMessage(task, id, message))
        return;

    switch (id)
    {
    case MESSAGE_STREAM_PARTITION_VERIFY:
        {
        /*  We have finished processing a GAIA_COMMAND_CLOSE_STORAGE_PARTITION or DFU  */
            MessageStreamPartitionVerify *m = (MessageStreamPartitionVerify *) message;
            GAIA_DEBUG(("gaia: pfs %u verify %u\n", m->partition, m->verify_result));

            if (gaia->outstanding_request)
            {
                if ((gaia->pfs_state == PFS_PARTITION) || (gaia->pfs_state == PFS_FILE))
                {
                    uint8 status;
                    uint8 response = gaia->pfs_open_stream;

                    if (m->verify_result == PARTITION_VERIFY_PASS)
                        status = GAIA_STATUS_SUCCESS;

                    else
                        status = GAIA_STATUS_INVALID_PARAMETER;

                    send_ack(gaia->outstanding_request, GAIA_VENDOR_QTIL, GAIA_COMMAND_CLOSE_STORAGE_PARTITION,
                         status, 1, &response);
                }

                else if (gaia->pfs_state == PFS_DFU)
                {
                    if (m->verify_result == PARTITION_VERIFY_PASS)
                    {
                        gaia->rebooting = TRUE;
                        dfu_send_state(GAIA_DFU_STATE_VERIFICATION_SUCCESS);
                        MessageSendLater(&gaia->task_data, GAIA_INTERNAL_DFU_REQ, NULL, TATA_TIME);
                    }

                    else
                    {
                        gaia->dfu_state = DFU_IDLE;
                        dfu_send_state(GAIA_DFU_STATE_VERIFICATION_FAILURE);
                    }
                }

                gaia->outstanding_request = NULL;
            }

            gaia->pfs_state = PFS_NONE;
        }
        break;

#ifdef GAIA_TRANSPORT_SPP
    case SPP_CLIENT_CONNECT_CFM:
    case SPP_DISCONNECT_IND:
    case SPP_MESSAGE_MORE_DATA:
        gaiaTransportHandleMessage(task, id, message);
        break;

    case SPP_MESSAGE_MORE_SPACE:
        break;
#endif

    case UPGRADE_TRANSPORT_CONNECT_CFM:
        if (gaia->upgrade_transport)
        {
            UPGRADE_TRANSPORT_CONNECT_CFM_T *cfm = (UPGRADE_TRANSPORT_CONNECT_CFM_T *) message;
            uint8 status = cfm->status == upgrade_status_success ?
                GAIA_STATUS_SUCCESS :
                GAIA_STATUS_INCORRECT_STATE;

            GAIA_DEBUG(("GAIA: UPGRADE_TRANSPORT_CONNECT_CFM\n"));

            send_ack(gaia->upgrade_transport, GAIA_VENDOR_QTIL, GAIA_COMMAND_VM_UPGRADE_CONNECT, status, 0, NULL);

            if (cfm->status != upgrade_status_success)
            {
                gaia->upgrade_transport = NULL;
            }
            else
            {
                /* Tell the sink type of upgrade transport connected */
                MESSAGE_PMAKE(ind, GAIA_UPGRADE_CONNECT_IND_T);
                ind->transport_type = gaia->upgrade_transport->type;
                MessageSend(gaia->app_task, GAIA_UPGRADE_CONNECT_IND, ind);
            }
        }
        break;

    case UPGRADE_TRANSPORT_DISCONNECT_CFM:
        {
            /* Tell the sink type of upgrade transport disconnected */
            MESSAGE_PMAKE(ind, GAIA_UPGRADE_DISCONNECT_IND_T);

            /* If disconnected via a gaia command, the transport is already 
               removed. Set the transport type to none in this case. */
            ind->transport_type = gaia->upgrade_transport ? gaia->upgrade_transport->type : gaia_transport_none;
            MessageSend(gaia->app_task, GAIA_UPGRADE_DISCONNECT_IND, ind);
        }
        break;

    case UPGRADE_TRANSPORT_DATA_IND:
        GAIA_DEBUG(("GAIA: UPGRADE_TRANSPORT_DATA_IND transport 0x%p\n", (void *)gaia->upgrade_transport));
        if(gaia->upgrade_transport)
        {
            GAIA_DEBUG(("GAIA: UPGRADE_TRANSPORT_DATA_IND notify 0x%x\n", gaia->upgrade_transport->notify_vmup));
        }
        if (gaia->upgrade_transport && gaia->upgrade_transport->notify_vmup)
        {
            UPGRADE_TRANSPORT_DATA_IND_T *ind = (UPGRADE_TRANSPORT_DATA_IND_T *) message;
            handle_upgrade_transport_data_ind(ind->is_data_state, ind->data, ind->size_data);
        }
        break;

    case UPGRADE_TRANSPORT_DATA_CFM:
        if (gaia->upgrade_transport)
        {
            UPGRADE_TRANSPORT_DATA_CFM_T *cfm = (UPGRADE_TRANSPORT_DATA_CFM_T *) message;
            uint8 status = cfm->status == upgrade_status_success ?
                GAIA_STATUS_SUCCESS :
                GAIA_STATUS_INVALID_PARAMETER;

            handle_upgrade_transport_data_cfm(cfm->packet_type, status);
        }
        break;

    case RWCP_UPGRADE_DATA:
        {
            RWCP_DATA_T *ind = (RWCP_DATA_T *) message;
            GAIA_DEBUG(("GAIA: RWCP_UPGRADE_DATA\n"));
            GaiaRwcpProcessCommand(ind->payload, ind->size_payload);
            free(ind->payload);
        }
        break;

    case RWCP_UPGRADE_DATA_ACK:
        {
            RWCP_DATA_T *ind = (RWCP_DATA_T *) message;
            GAIA_DEBUG(("GAIA: RWCP_UPGRADE_DATA_ACK\n"));
            GaiaRwcpSendNotification(ind->payload, ind->size_payload);
        }        
        break;

    case GAIA_SEND_PACKET_CFM:
        {
            GAIA_SEND_PACKET_CFM_T *m = (GAIA_SEND_PACKET_CFM_T *) message;
            PRINT(("gaia: GAIA_SEND_PACKET_CFM: s=%d\n", VmGetAvailableAllocations()));
            free(m->packet);
        }
        break;


    case GAIA_INTERNAL_INIT:
        gaia_init();
        break;

#ifdef GAIA_TRANSPORT_GATT
    case GAIA_INTERNAL_ATT_STREAMS_BUFFER_UNAVAILABLE:
        {
            GAIA_INTERNAL_ATT_STREAMS_BUFFER_UNAVAILABLE_REQ_T *m = (GAIA_INTERNAL_ATT_STREAMS_BUFFER_UNAVAILABLE_REQ_T *) message;
            GAIA_DEBUG(("GAIA_INTERNAL_ATT_STREAMS_BUFFER_UNAVAILABLE\n"));
            gaiaTransportAttStreamsSend(m->transport, m->vendor_id, m->command_id, m->status, m->size, m->data, m->unpack);
        }
        break;
#endif


    case GAIA_INTERNAL_SEND_REQ:
        {
            GAIA_INTERNAL_SEND_REQ_T *m = (GAIA_INTERNAL_SEND_REQ_T *) message;
            gaiaTransportSendPacket(m->task, m->transport, m->length, m->data);
        }
        break;

    case GAIA_INTERNAL_CHECK_BATTERY_THRESHOLD_REQ:
        {
            GAIA_INTERNAL_CHECK_BATTERY_THRESHOLD_REQ_T *req = (GAIA_INTERNAL_CHECK_BATTERY_THRESHOLD_REQ_T *)message;

            if (req->transport->connected)
            {
                /* if we have any battery thresholds set for this transport */
                if (req->transport->threshold_count_lo_battery || req->transport->threshold_count_hi_battery)
                {
                    /* check if a threshold has been reached, inform host, and restart the timer */
                    check_battery_thresholds(req->transport);
                    start_check_battery_threshold_timer(req->transport);
                }
            }
        }
        break;

    case GAIA_INTERNAL_CHECK_RSSI_THRESHOLD_REQ:
        {
            GAIA_INTERNAL_CHECK_RSSI_THRESHOLD_REQ_T *req = (GAIA_INTERNAL_CHECK_RSSI_THRESHOLD_REQ_T *)message;
            if (req->transport->connected)
            {
                /* if we have any RSSI thresholds set for this transport */
                if (req->transport->threshold_count_lo_rssi || req->transport->threshold_count_hi_rssi)
                {
                    /* check if a threshold has been reached, inform host, and restart the timer */
                    check_rssi_thresholds(req->transport);
                    start_check_rssi_threshold_timer(req->transport);
                }
            }
        }
        break;


    case GAIA_INTERNAL_REBOOT_REQ:
        BootSetMode(* (uint16 *) message);
        break;


    case GAIA_INTERNAL_DFU_REQ:
        GAIA_DEBUG(("gaia: LoaderPerformDfuFromSqif(%u)\n", gaia->pfs_open_stream));
#ifdef HAVE_DFU_FROM_SQIF
        LoaderPerformDfuFromSqif(gaia->pfs_open_stream);
#endif
        break;


    case GAIA_INTERNAL_DISCONNECT_REQ:
        {
            GAIA_INTERNAL_DISCONNECT_REQ_T *m = (GAIA_INTERNAL_DISCONNECT_REQ_T *) message;
            gaiaTransportDisconnectReq(m->transport);
        }
        break;

    case GAIA_INTERNAL_DFU_TIMEOUT:
        GAIA_DFU_DEBUG(("gaia: dfu tmo in state %d\n", gaia->dfu_state));

        if (gaia->dfu_state == DFU_DOWNLOAD)
            dfu_finish(FALSE);

        if (gaia->dfu_state == DFU_WAITING)
            dfu_confirm(gaia->outstanding_request, FALSE);

        else
            dfu_indicate(GAIA_DFU_STATE_DOWNLOAD_FAILURE);

        gaia->dfu_state = DFU_IDLE;

        break;

#ifdef GAIA_TRANSPORT_GATT
    case GATT_MANAGER_SERVER_ACCESS_IND:
        gaiaHandleGattManagerAccessInd((GATT_MANAGER_SERVER_ACCESS_IND_T *) message);
        break;
#endif

    case CL_SM_ENCRYPTION_CHANGE_IND:
        break;

#ifdef GAIA_TRANSPORT_GATT
    case CL_DM_RSSI_BDADDR_CFM:
        report_rssi( (CL_DM_RSSI_BDADDR_CFM_T *) message );
        break;
#endif

    default:
        GAIA_DEBUG(("gaia: unh 0x%04X\n", id));
        break;
    }
}

/*************************************************************************
NAME
    gaiaProcessData

DESCRIPTION
    Process the GAIA data received on GAIA data endpoint
    Return TRUE if the data was processed and sent to Upgrade library.
           FALSE if the data was control/out of sequence packet.
*/
bool gaiaProcessData(gaia_transport *transport, uint16 size_data, uint8 *data)
{
    UNUSED(transport);
    return (RwcpServerHandleMessage(data, size_data));
}

/*************************************************************************
NAME
    gaiaProcessCommand

DESCRIPTION
    Process a GAIA command or pass it up as unhandled
*/
void gaiaProcessCommand(gaia_transport *transport, uint16 vendor_id, uint16 command_id, uint16 size_payload, uint8 *payload)
{
    if (gaia->rebooting)
    {
        send_ack(transport, vendor_id, command_id, GAIA_STATUS_INCORRECT_STATE, 0, NULL);
    }
    else
    {
        bool handled;

        if (!transport->enabled)
        {
            if (!check_enable(transport, vendor_id, command_id, size_payload, payload))
            {
                send_incorrect_state(transport, command_id);
                return;
            }
        }

        if (vendor_id == GAIA_VENDOR_QTIL)
        {
            handled = !app_will_handle_command(command_id);
        }
        else
        {
            handled = FALSE;
        }

        GAIA_CMD_DEBUG(("gaiax: <- %04x:%04x %c\n", vendor_id, command_id, handled ? 'L' : 'A'));

        if (handled)
        {
            switch (command_id & GAIA_COMMAND_TYPE_MASK)
            {
            case GAIA_COMMAND_TYPE_CONTROL:
                handled = gaia_handle_control_command(transport, command_id, size_payload, payload);
                break;

            case GAIA_COMMAND_TYPE_STATUS:
                handled = gaia_handle_status_command(transport, command_id, size_payload, payload);
                break;

            case GAIA_COMMAND_TYPE_FEATURE:
                handled = gaia_handle_feature_command(transport, command_id, size_payload, payload);
                break;

            case GAIA_COMMAND_TYPE_DATA_TRANSFER:
                GAIA_DEBUG(("GAIA_COMMAND_TYPE_DATA_TRANSFER\n"));
                handled = gaia_handle_data_transfer_command(transport, command_id, size_payload, payload);
                break;

            case GAIA_COMMAND_TYPE_DEBUG:
                handled = gaia_handle_debug_command(transport, command_id, size_payload, payload);
                break;

            case GAIA_COMMAND_TYPE_VOICE_ASSISTANT:
                handled = gaiaHandleVoiceAssistantCommand(transport, command_id, size_payload, payload);
                break;
                
            case GAIA_COMMAND_TYPE_NOTIFICATION:
                handled = gaia_handle_notification_command(transport, command_id, size_payload, payload);
                break;

            default:
                handled = FALSE;
                break;
            }
        }

        if (!handled)
        {
        /*  Pass it on sans the start-of-frame and checksum bytes  */
            GAIA_UNHANDLED_COMMAND_IND_T *gunk = malloc(sizeof (GAIA_UNHANDLED_COMMAND_IND_T) + size_payload - 1);

            GAIA_CMD_DEBUG(("gaia: unhandled command\n"));

            if (gunk)
            {
                gunk->transport = (GAIA_TRANSPORT *) transport;
                gunk->protocol_version = GAIA_VERSION;
                gunk->size_payload = size_payload;
                gunk->vendor_id = vendor_id;
                gunk->command_id = command_id;

                if (size_payload > 0)
                {
                    memmove(gunk->payload, payload, size_payload);
                }

                MessageSend(gaia->app_task, GAIA_UNHANDLED_COMMAND_IND, gunk);
            }
            else
            {
                send_ack(transport, vendor_id, command_id, GAIA_STATUS_INSUFFICIENT_RESOURCES, 0, NULL);
            }
        }
    }
}


/*************************************************************************
 *                                                                       *
 *  Public interface functions                                           *
 *                                                                       *
 *************************************************************************/

/*************************************************************************
NAME
    GaiaInit

DESCRIPTION
    Initialise the Gaia protocol handler library
*/
void GaiaInit(Task task, uint16 max_connections)
{
    /* size of buffer required for GAIA_T + transport data */
    uint16 buf_size = sizeof (GAIA_T) + (max_connections - 1) * sizeof (gaia_transport);

    GAIA_DEBUG(("gaia: GaiaInit n=%d s=%d\n", max_connections, buf_size));

    if ((gaia == NULL) &&
    (max_connections > 0) &&
    (max_connections <= MAX_SUPPORTED_GAIA_TRANSPORTS))
    {
        gaia = PanicUnlessMalloc(buf_size);
    }

    if (gaia != NULL)
    {
        /* initialise message handler */
        memset(gaia, 0, buf_size);
        gaia->task_data.handler = message_handler;
        gaia->app_task = task;

        gaia->transport_count = max_connections;

        MessageSend(&gaia->task_data, GAIA_INTERNAL_INIT, NULL);
    }

    else
    {
        MESSAGE_PMAKE(status, GAIA_INIT_CFM_T);
        status->success = FALSE;
        MessageSend(task, GAIA_INIT_CFM, status);
    }
}



/*************************************************************************
NAME
    GaiaBtConnectRequest

DESCRIPTION
    Request a connection to the given address
*/
void GaiaBtConnectRequest(GAIA_TRANSPORT *transport, tp_bdaddr *address)
{
    GAIA_DEBUG(("GaiaBtConnectRequest\n"));

    if (gaia != NULL)
        gaiaTransportConnectReq((gaia_transport *) transport, address);
}


/*************************************************************************
NAME
    GaiaDisconnectRequest

DESCRIPTION
    Disconnect from host
*/
void GaiaDisconnectRequest(GAIA_TRANSPORT *transport)
{
    if (gaia != NULL)
        gaiaTransportDisconnectReq((gaia_transport *) transport);
    }


/*************************************************************************
NAME
    GaiaDisconnectResponse

DESCRIPTION
    Indicates that the client has processed a GAIA_DISCONNECT_IND message
*/
void GaiaDisconnectResponse(GAIA_TRANSPORT *transport)
{
    gaiaTransportTidyUpOnDisconnection((gaia_transport *) transport);
}


/*************************************************************************
NAME
    GaiaStartService

DESCRIPTION
    Start a service of the given type
*/
void GaiaStartService(gaia_transport_type transport_type)
{
    if (gaia != NULL)
        gaiaTransportStartService(transport_type);
}


/*************************************************************************
NAME
    GaiaStartGattServer

DESCRIPTION
    Starts the GAIA GATT Server
*/
void GaiaStartGattServer(uint16 start_handle, uint16 end_handle)
{
    if (gaia != NULL)
    {
        gaiaTransportStartGattServer(start_handle, end_handle);
        RwcpServerInit(GAIA_GATT_HEADER_SIZE + GAIA_UPGRADE_HEADER_SIZE);
        gaia->data_endpoint_mode = GAIA_DATA_ENDPOINT_MODE_NONE;
        gaia->upgrade_large_data.in_progress = FALSE;
    }
}


/*************************************************************************
NAME
    GaiaSendPacket

DESCRIPTION
    Send a Gaia packet over the indicated connection
*/
void GaiaSendPacket(GAIA_TRANSPORT *transport, uint16 packet_length, uint8 *packet)
{
    if (gaia != NULL)
        send_packet(gaia->app_task, (gaia_transport *) transport, packet_length, packet);
}


/*************************************************************************
NAME
    GaiaGetAppWillHandleCommand

DESCRIPTION
    Return TRUE if the given command is to be passed to application code
    rather than being handled by the library
*/
bool GaiaGetAppWillHandleCommand(uint16 command_id)
{
    uint16 idx = find_locus_bit(command_id);

    if ((idx == GAIA_INVALID_ID) || (gaia == NULL))
        return FALSE;

    return (gaia->command_locus_bits & ULBIT(idx)) != 0;
}


/*************************************************************************
NAME
    GaiaSetApiMinorVersion

DESCRIPTION
    Changes the API Minor Version reported by GAIA_COMMAND_GET_API_VERSION
    Returns TRUE on success, FALSE if the value is out of range (0..15) or
    the GAIA storage is not allocated
*/
bool GaiaSetApiMinorVersion(uint8 version)
{
    if (gaia == NULL || version > GAIA_API_VERSION_MINOR_MAX)
        return FALSE;

    gaia->api_minor = version;
    return TRUE;
}


/*************************************************************************
NAME
    GaiaSetAppWillHandleCommand

DESCRIPTION
    Request that the given command be passed to application code
    rather than being handled by the library.  Returns TRUE on success.
*/
bool GaiaSetAppWillHandleCommand(uint16 command_id, bool value)
{
    uint16 idx;

    if (gaia == NULL)
        return TRUE;

    idx = find_locus_bit(command_id);

    if (idx != GAIA_INVALID_ID)
    {
        if (value)
            gaia->command_locus_bits |= ULBIT(idx);

        else
            gaia->command_locus_bits &= ~ULBIT(idx);

        return TRUE;
    }

    return FALSE;
}


/*************************************************************************
NAME
    GaiaGetAppWillHandleNotification

DESCRIPTION
    Return TRUE if the given event is to be notified by application code
    rather than being handled by the library.
*/
bool GaiaGetAppWillHandleNotification(uint8 event_id)
{
    if ((event_id > GAIA_IMP_MAX_EVENT_BIT) || (gaia == NULL))
        return FALSE;

    return (gaia->event_locus_bits & ULBIT(event_id)) != 0;
}


/*************************************************************************
NAME
    GaiaSetAppWillHandleNotification

DESCRIPTION
    Request that the given event be raised by application code
    rather than by the library.  Returns TRUE on success.
*/
bool GaiaSetAppWillHandleNotification(uint8 event_id, bool value)
{
    if ((event_id > GAIA_IMP_MAX_EVENT_BIT) || (gaia == NULL))
        return FALSE;


    if (value)
        gaia->event_locus_bits |= ULBIT(event_id);

    else
        gaia->event_locus_bits &= ~ULBIT(event_id);

    return TRUE;
}


/*************************************************************************
NAME
    GaiaBuildPacket

DESCRIPTION
    Build an arbitrary Gaia packet into the supplied buffer
*/
uint16 GaiaBuildPacket(uint8 *buffer, uint8 flags,
                            uint16 vendor_id, uint16 command_id,
                            uint8 size_payload, uint8 *payload)
{
    return build_packet(buffer, flags, vendor_id, command_id,
                        GAIA_STATUS_NONE, size_payload, payload);
}


/*************************************************************************
NAME
    GaiaBuildPacket16

DESCRIPTION
    Build an arbitrary Gaia packet into the supplied buffer from a
    uint16[] payload
*/
uint16 GaiaBuildPacket16(uint8 *buffer, uint8 flags, uint16 vendor_id, uint16 command_id,
                         uint8 size_payload, uint16 *payload)
{
    return build_packet_16(buffer, flags, vendor_id, command_id,
                        GAIA_STATUS_NONE, size_payload, payload);
}


/*************************************************************************
NAME
    GaiaBuildResponse

DESCRIPTION
    Build a Gaia acknowledgement packet into the supplied buffer
*/
uint16 GaiaBuildResponse(uint8 *buffer, uint8 flags,
                         uint16 vendor_id, uint16 command_id,
                         uint8 status, uint8 size_payload, uint8 *payload)
{
    return build_packet(buffer, flags, vendor_id, command_id,
                        status, size_payload, payload);
}


/*************************************************************************
NAME
    GaiaBuildResponse16

DESCRIPTION
    Build a Gaia acknowledgement packet into the supplied buffer from a
    uint16 payload
*/
uint16 GaiaBuildResponse16(uint8 *buffer, uint8 flags,
                            uint16 vendor_id, uint16 command_id,
                            uint8 status, uint8 size_payload, uint16 *payload)
{
    return build_packet_16(buffer, flags, vendor_id, command_id,
                        status, size_payload, payload);
}


/*************************************************************************
NAME
    GaiaBuildAndSendSynch

DESCRIPTION
    Build a Gaia packet in the transport sink and flush it
    The payload is an array of uint8s; contrast GaiaBuildAndSendSynch16()
    The function completes synchronously and no confirmation message is
    sent to the calling task
*/
void GaiaBuildAndSendSynch(GAIA_TRANSPORT *transport,
                           uint16 vendor_id, uint16 command_id, uint8 status,
                           uint8 size_payload, uint8 *payload)
{
    switch (GaiaTransportGetType(transport))
    {
#ifdef GAIA_TRANSPORT_GATT
    case gaia_transport_gatt:
        gaiaTransportGattSend((gaia_transport *) transport, vendor_id, command_id, status, size_payload, payload, FALSE);
        break;
#endif
    case gaia_transport_rfcomm:
    case gaia_transport_spp:
        {
            Sink sink = gaiaTransportGetSink((gaia_transport *) transport);
            uint16 packet_length = GAIA_OFFS_PAYLOAD + size_payload;
            uint8 flags = ((gaia_transport *)transport)->flags;

            if (gaia == NULL || sink == NULL)
                return;

            if (status != GAIA_STATUS_NONE)
                ++packet_length;

            if (flags & GAIA_PROTOCOL_FLAG_CHECK)
                ++packet_length;

            if (packet_length > GAIA_MAX_PACKET)
                return;

            GAIA_TRANS_DEBUG(("gaia: bss %d\n", packet_length));

            if (SinkClaim(sink, packet_length) == BAD_SINK_CLAIM)
            {
                GAIA_TRANS_DEBUG(("gaia: no sink\n"));
                return;
            }

            build_packet(SinkMap(sink), flags, vendor_id, command_id,
                                status, size_payload, payload);

#ifdef DEBUG_GAIA_TRANSPORT
            {
                uint16 idx;
                uint8 *data = SinkMap(sink);
                GAIA_DEBUG(("gaia: put"));
                for (idx = 0; idx < packet_length; ++idx)
                    GAIA_DEBUG((" %02x", data[idx]));
                GAIA_DEBUG(("\n"));
            }
#endif

            SinkFlush(sink, packet_length);
        }
        break;
        
    default:
        GAIA_TRANS_DEBUG(("gaia: bad transport %u\n", GaiaTransportGetType(transport)));
    }
}


/*************************************************************************
NAME
    GaiaBuildAndSendSynch16

DESCRIPTION
    Build a Gaia packet in the transport sink and flush it
    The payload is an array of uint16s; contrast GaiaBuildAndSendSynch()
    The function completes synchronously and no confirmation message is
    sent to the calling task
*/
void GaiaBuildAndSendSynch16(GAIA_TRANSPORT *transport,
                             uint16 vendor_id, uint16 command_id, uint8 status,
                             uint16 size_payload, uint16 *payload)
{
#ifdef GAIA_TRANSPORT_GATT
    if (GaiaTransportGetType(transport) == gaia_transport_gatt)
    {
        gaiaTransportGattSend((gaia_transport *) transport, vendor_id, command_id, status, size_payload, payload, TRUE);
    }
    else
#endif
    {
        Sink sink = gaiaTransportGetSink((gaia_transport *) transport);
        uint16 packet_length = GAIA_OFFS_PAYLOAD + 2 * size_payload;
        uint8 flags = ((gaia_transport *)transport)->flags;

        if (gaia == NULL || sink == NULL)
            return;

        if (status != GAIA_STATUS_NONE)
            ++packet_length;

        if (flags & GAIA_PROTOCOL_FLAG_CHECK)
            ++packet_length;

        GAIA_TRANS_DEBUG(("gaia: bss16 %d\n", packet_length));

        if (packet_length > GAIA_MAX_PACKET)
            return;

        if (SinkClaim(sink, packet_length) == BAD_SINK_CLAIM)
        {
            GAIA_TRANS_DEBUG(("gaia: no sink\n"));
            return;
        }

        build_packet_16(SinkMap(sink), flags, vendor_id, command_id,
                            status, size_payload, payload);

        SinkFlush(sink, packet_length);
    }
}


/*************************************************************************
NAME
    GaiaTransportGetFlags

DESCRIPTION
    Returns the control flags for the given transport instance
*/
uint8 GaiaTransportGetFlags(GAIA_TRANSPORT *transport)
{
    if (transport)
        return ((gaia_transport *) transport)->flags;

    return 0;
}


/*************************************************************************
NAME
    GaiaTransportGetType

DESCRIPTION
    Returns the transport type for the given transport instance
*/
gaia_transport_type GaiaTransportGetType(GAIA_TRANSPORT *transport)
{
    if (transport)
        return ((gaia_transport *) transport)->type;

   return gaia_transport_none;
}


/*************************************************************************
NAME
    GaiaTransportGetBdAddr
    
DESCRIPTION
    Returns the typed Bluetooth device address for the transport instance,
    if any
*/
bool GaiaTransportGetBdAddr(GAIA_TRANSPORT *transport, typed_bdaddr *taddr)
{
    bool result = FALSE;
    
    if (transport)
    {
        tp_bdaddr current_addr;
        gaia_transport_type transport_type = GaiaTransportGetType(transport);
        memset(&current_addr, 0, sizeof current_addr);
        
#ifdef GAIA_TRANSPORT_GATT
        if (transport_type == gaia_transport_gatt)
        {
            result = VmGetBdAddrtFromCid(((gaia_transport *) transport)->state.gatt.cid, &current_addr);
        }
        else
#endif
        {
            Sink sink = gaiaTransportGetSink((gaia_transport *) transport);
            
            if (sink)
            {
                result = SinkGetBdAddr(sink, &current_addr);
            }
        }
		
        if(result)
        {
            *taddr = current_addr.taddr;
        }
    }

    return result;
}


/*************************************************************************
NAME
    GaiaTransportSetFlags

DESCRIPTION
    Sets the control flags for the given transport instance
*/
void GaiaTransportSetFlags(GAIA_TRANSPORT *transport, uint8 flags)
{
    ((gaia_transport *) transport)->flags = flags;
}


/*************************************************************************
NAME
    GaiaGetSessionEnable

DESCRIPTION
    Returns TRUE if GAIA session is enabled for the given transport instance
*/
bool GaiaGetSessionEnable(GAIA_TRANSPORT *transport)
{
    return ((gaia_transport *) transport)->enabled;
}


/*************************************************************************
NAME
    GaiaSetSessionEnable

DESCRIPTION
    Enables or disables GAIA session for the given transport instance
*/
void GaiaSetSessionEnable(GAIA_TRANSPORT *transport, bool enable)
{
    ((gaia_transport *) transport)->enabled = enable;
}


/*************************************************************************
NAME
    GaiaTransportGetSink
    
DESCRIPTION
    Returns the underlying sink for the given transport instance or 0 if
    there is none
*/
Sink GaiaTransportGetSink(GAIA_TRANSPORT *transport)
{
    if (transport)
    {
        return gaiaTransportGetSink((gaia_transport *) transport);
    }

    return 0;
}

/*************************************************************************
NAME
    GaiaOnTransportConnect

DESCRIPTION
    Do what is needed on transport connection.
*/
void GaiaOnTransportConnect(GAIA_TRANSPORT *transport)
{
    /* Read and process data from source. This is needed because
       packet could have been received before sending of
       MESSAGE_MORE_DATA was enabled.
     */
    gaiaTransportProcessSource((gaia_transport *) transport);
}


/*************************************************************************
NAME
    GaiaDfuRequest

DESCRIPTION
    Enables and Requests Device Firmware Upgrade

    Does some sanity checks:
    o The <partition> is of the correct type
    o The <period> is reasonable
    o Gaia is initialised
    o A DFU is not already pending or in progress
    o No other Partition Filesystem operation is in progress
    o Exactly one transport is connected

*/
void GaiaDfuRequest(GAIA_TRANSPORT *transport, uint16 partition, uint16 period)
{
#ifndef HAVE_PARTITION_FILESYSTEM
    UNUSED(partition);
    UNUSED(period);
#else   /* HAVE_PARTITION_FILESYSTEM */
    uint32 pfs_type = 0xFF;
    uint16 idx;
    bool ok = TRUE;

    ok = PartitionGetInfo(PARTITION_SERIAL_FLASH, partition, PARTITION_INFO_TYPE, &pfs_type);

    GAIA_DEBUG(("gaia: dfu request p=%u s=%u t=%lu\n", partition, ok, pfs_type));

    if ((gaia == NULL) ||
        (transport == NULL) ||
        (gaia->dfu_state != DFU_IDLE) ||
        (gaia->pfs_state != PFS_NONE) ||
        (partition > PFS_MAX_PARTITION) ||
        (period < 1) ||
        (period > DFU_MAX_ENABLE_PERIOD) ||
        (pfs_type != PARTITION_TYPE_RAW_SERIAL))
        ok = FALSE;

    if (ok)
    {
    /*  Make sure there is only one transport  */
        for (idx = 0; ok && idx < gaia->transport_count; ++idx)
        {
            if ((gaia->transport[idx].type != gaia_transport_none) &&
                ((GAIA_TRANSPORT *) &gaia->transport[idx] != transport))
                ok = FALSE;
        }
    }

    if (ok)
    {
        gaia->dfu_state = DFU_WAITING;
        gaia->pfs_open_stream = partition;
        send_simple_command(transport, GAIA_COMMAND_DFU_REQUEST, 0, NULL);
        dfu_reset_timeout(period);
    }

    else
#endif  /* HAVE_PARTITION_FILESYSTEM */
       dfu_confirm((gaia_transport *) transport, FALSE);
    }


/*************************************************************************
NAME
    GaiaGetCidOverGattTransport

DESCRIPTION
    Get CID over GATT transport
*/
uint16 GaiaGetCidOverGattTransport(void)
{
    if ((gaia->upgrade_transport) && (gaia->upgrade_transport->type==gaia_transport_gatt))
        return gaia->upgrade_transport->state.gatt.cid;
    else
        return INVALID_CID;
}


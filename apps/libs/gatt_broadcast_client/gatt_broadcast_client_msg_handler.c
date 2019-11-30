/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gatt.h>
#include <gatt_manager.h>
#include <broadcast.h>
#include <message.h>
#include <panic.h>
#include <gatt_broadcast_server_uuids.h>

#include "gatt_broadcast_client_msg_handler.h"
#include "gatt_broadcast_client_debug.h"

/* Macro to pack two uint8 data into uint16 */
#define PACK_16_BITS(msb,lsb)       ((uint16)(((uint16)msb << 8) | ((uint16)lsb)))

/* Macro to pack the uint32 type LAP part of BD Address */
#define PACK_32_BIT_LAP(a,b,c,d)    ((uint32)(((uint32)a << 24) | \
                                                     ((uint32)b << 16) | \
                                                     ((uint32)c << 8)  | d))

/* Macro to validate the broadcast_version_characteristic size */
#define BROADCAST_VERSION_SIZE      (2)

/* Macro to validate the broadcast_address_characteristic size */
#define BROADCAST_ADDRESS_SIZE      (7)

/* Macro to validate the broadcast_identifier_characteristic size */
#define BROADCAST_IDENTIFIER_SIZE      (4)

 /*!
 * Array to hold only unique 32 bit part of Broadcast characteristics UUID 
 */

static const gatt_uuid_t ba_version[1] =    {UUID_BROADCAST_VERSION_U32};
static const gatt_uuid_t ba_address[1] =   {UUID_BROADCAST_ADDRESS_U32};
static const gatt_uuid_t ba_status[1] =     {UUID_BROADCAST_STATUS_U32};
static const gatt_uuid_t ba_seckey[1] =    {UUID_BROADCAST_SECKEY_U32};
static const gatt_uuid_t ba_ssr[1] =         {UUID_BROADCAST_STREAM_SERVICE_RECORDS_U32};
static const gatt_uuid_t ba_identifier[1] = {UUID_BROADCAST_IDENTIFIER_U32};

/***************************************************************************
NAME
    send_broadcast_client_init_cfm

DESCRIPTION
   Utility function to send Broadcast client init cfm with info read from the Broadcast Server
   to application
*/
static void send_broadcast_client_init_cfm(GBSC* broadcast_client,
                               gatt_broadcast_client_status status,
                               broadcast_assoc_data* assoc_data)
{
    MESSAGE_MAKE(message, GATT_BROADCAST_CLIENT_INIT_CFM_T);
    message->ba_client = broadcast_client;
    message->status = status;
    message->assoc_data = assoc_data;
    MessageSend(broadcast_client->app_task, GATT_BROADCAST_CLIENT_INIT_CFM, message);
}

/***************************************************************************
NAME
    broadcast_read_char_value_by_uuid

DESCRIPTION
   Utility function to read the Broadcast server characteristc value based on 128 bit UUID
*/
static void broadcast_read_char_value_by_uuid(const Task task, gatt_uuid_t uuid_top, 
                                                                gatt_broadcast_characteristics ba_char)
{
    GBSC* broadcast_client = (GBSC*)task;
    gatt_manager_client_service_data_t service_data;
    gatt_uuid_t complete_uuid[4] = {0, UUID_BROADCAST_BASE_C, UUID_BROADCAST_BASE_B, 
                                                UUID_BROADCAST_BASE_A};
    complete_uuid[0] = uuid_top;

    if(GattManagerGetClientData(task, &service_data))
    {
        GattManagerReadUsingCharacteristicUuid(task, service_data.start_handle,
                        service_data.end_handle, gatt_uuid128, (const gatt_uuid_t *) complete_uuid);
        broadcast_client->current_characteristic_read = ba_char;
    }
}

/***************************************************************************
NAME
    broadcast_client_handle_gatt_manager_msg

DESCRIPTION
   Utility function to handle GATT Manager messages of Broadcast Client
*/
static void broadcast_client_handle_gatt_manager_msg(Task task, MessageId id, Message message)
{
    GBSC* broadcast_client = (GBSC*)task;

    switch(id)
    {
        case GATT_MANAGER_READ_LONG_CHARACTERISTIC_VALUE_CFM:
        {
            GATT_MANAGER_READ_LONG_CHARACTERISTIC_VALUE_CFM_T* cfm = (GATT_MANAGER_READ_LONG_CHARACTERISTIC_VALUE_CFM_T*)message;
            BROADCAST_CLIENT_DEBUG(("BL:GATTMAN:CLIENT: LREAD Handle:0x%x Size:0x%x Offset:0x%x Status:0x%x More:%s\n", 
                                     cfm->handle, cfm->size_value, cfm->offset,
                                     cfm->status, cfm->more_to_come ? "Y":"N"));

            if (cfm->status == gatt_status_success && cfm->size_value)
            {
                /* we only currently use long reads for HANDLE_BROADCAST_STREAM_SERVICE_RECORDS */

                /* (re)allocate enough space for service record data already received and latest */
                uint8* new_record = realloc(broadcast_client->assoc_data.stream_service_records,
                                            broadcast_client->assoc_data.stream_service_records_len + cfm->size_value);
                if (new_record)
                {
                    /* append latest piece of record data to the new memory */
                    memmove(new_record + (uint8)broadcast_client->assoc_data.stream_service_records_len,
                            cfm->value, cfm->size_value);
                    /* update our pointer to the new_record and remember new length */
                    broadcast_client->assoc_data.stream_service_records = new_record;
                    broadcast_client->assoc_data.stream_service_records_len += cfm->size_value;

                    if(!cfm->more_to_come)
                    {
                        broadcast_read_char_value_by_uuid(&broadcast_client->lib_task, ba_identifier[0], 
                                                            broadcast_identifier_characteristic);
                    }
                }
                else
                {
                /* Send BA Gatt client Init failed message to application */
                send_broadcast_client_init_cfm(broadcast_client,
                                          gatt_broadcast_client_status_read_failed,
                                          NULL);
                }

            }
            else
            {
                /* Send BA Gatt client Init failed message to application */
                send_broadcast_client_init_cfm(broadcast_client,
                                          gatt_broadcast_client_status_read_failed,
                                          NULL);
            }
        }
        break;

        case GATT_MANAGER_READ_USING_CHARACTERISTIC_UUID_CFM:
        {
            GATT_MANAGER_READ_USING_CHARACTERISTIC_UUID_CFM_T* cfm = 
                        (GATT_MANAGER_READ_USING_CHARACTERISTIC_UUID_CFM_T*)message;

            BROADCAST_CLIENT_DEBUG(("BL:GATTMAN:CLIENT: DISC Handle:0x%x More:%s\n",
                                    cfm->handle, cfm->more_to_come ? "Y":"N"));

            if (cfm->status == gatt_status_success && cfm->size_value)
            {
                switch(broadcast_client->current_characteristic_read)
                {
                    /* Broadcast Version */
                    case broadcast_version_characteristic:
                    {
                        if(BROADCAST_VERSION_SIZE == cfm->size_value)
                        {
                            broadcast_client->assoc_data.broadcast_service_version = 
                                                PACK_16_BITS(cfm->value[0], cfm->value[1]);
                            /* Read next characteristic value */
                            broadcast_read_char_value_by_uuid(&broadcast_client->lib_task, ba_address[0], 
                                                                broadcast_address_characteristic);
                        }
                        else
                        {
                            /* Send BA Gatt client Init failed message to application */
                            send_broadcast_client_init_cfm(broadcast_client,
                                          gatt_broadcast_client_status_read_failed,
                                          NULL);
                        }
                    }
                    break;

                    /* Broadcaster Address */
                    case broadcast_address_characteristic:
                    {
                        if(BROADCAST_ADDRESS_SIZE == cfm->size_value)
                        {
                            broadcast_client->assoc_data.broadcast_addr.lap = PACK_32_BIT_LAP(cfm->value[0], 
                                                cfm->value[1], cfm->value[2], cfm->value[3]);
                            broadcast_client->assoc_data.broadcast_addr.uap = cfm->value[4];
                            broadcast_client->assoc_data.broadcast_addr.nap = PACK_16_BITS(cfm->value[5], cfm->value[6]);
                            /* Read next characteristic value */
                            broadcast_read_char_value_by_uuid(&broadcast_client->lib_task, ba_status[0], 
                                                                broadcast_status_characteristic);
                        }
                        else
                        {
                            /* Send BA Gatt client Init failed message to application */
                            send_broadcast_client_init_cfm(broadcast_client,
                                          gatt_broadcast_client_status_read_failed,
                                          NULL);
                        }
                    }
                    break;

                    /* Broadcast Status */
                    case broadcast_status_characteristic:
                    {
                        /* limit length to the size of the buffer we're copying to */
                        broadcast_client->assoc_data.device_status_len = 
                            cfm->size_value <= sizeof(broadcast_client->assoc_data.device_status) ?
                            cfm->size_value : sizeof(broadcast_client->assoc_data.device_status);
                        /* copy amount of data calculated above */
                        memmove(broadcast_client->assoc_data.device_status,
                                cfm->value, 
                                broadcast_client->assoc_data.device_status_len);
                        /* Read next characteristic value */
                        broadcast_read_char_value_by_uuid(&broadcast_client->lib_task, ba_seckey[0], 
                                                            broadcast_sec_key_characteristic);
                    }
                    break;

                    /* Broadcast Security Key */
                    case broadcast_sec_key_characteristic:
                    {
                        /* limit length to the size of the buffer we're copying to */
                        broadcast_client->assoc_data.seckey_len =
                            cfm->size_value <= sizeof(broadcast_client->assoc_data.seckey) ?
                            cfm->size_value : sizeof(broadcast_client->assoc_data.seckey);
                        /* copy amount of data calculated above */
                        memmove(broadcast_client->assoc_data.seckey, cfm->value,
                                broadcast_client->assoc_data.seckey_len);
                        /* Read next characteristic value */
                        broadcast_read_char_value_by_uuid(&broadcast_client->lib_task, ba_ssr[0], 
                                                            broadcast_ssr_characteristic);
                    }
                    break;

                    /* Broadcast Stream Service Records */
                    case broadcast_ssr_characteristic:
                    {
                        GattManagerReadLongCharacteristicValue(&broadcast_client->lib_task, 
                                                               cfm->handle);
                    }
                    break;

                    /* Broadcast Identifier */
                    case broadcast_identifier_characteristic:
                    {
                        if(BROADCAST_IDENTIFIER_SIZE == cfm->size_value)
                        {
                            broadcast_client->assoc_data.broadcast_identifier[BROADCAST_IDENTIFIER_PRODUCT_OFFSET] = 
                                                PACK_16_BITS(cfm->value[0], cfm->value[1]);
                            broadcast_client->assoc_data.broadcast_identifier[BROADCAST_IDENTIFIER_VERSION_OFFSET] = 
                                                PACK_16_BITS(cfm->value[2], cfm->value[3]);

                            /* Send BA Gatt client Init success message to application as all the characteristics values
                            are read successfully */
                            send_broadcast_client_init_cfm(broadcast_client, 
                                                       gatt_broadcast_client_status_success,
                                                       &broadcast_client->assoc_data);
                        }
                        else
                        {
                            /* Send BA Gatt client Init failed message to application */
                            send_broadcast_client_init_cfm(broadcast_client,
                                          gatt_broadcast_client_status_read_failed,
                                          NULL);
                        }
                    }
                    break;

                    default:
                    {
                       /* Unknown Handle, Should not reach here */
                       BROADCAST_CLIENT_DEBUG(("Func:broadcast_client_msg_handler(), UnKnownCharacteristic Value\n"));
                    }
                    break;
                }
            }
            else
            {
                /* Send BA Gatt client Init failed message to application */
                send_broadcast_client_init_cfm(broadcast_client,
                                          gatt_broadcast_client_status_read_failed,
                                          NULL);
            }
        }
        break;

        default:
        {
            BROADCAST_CLIENT_DEBUG(("BL:GATTMAN:CLIENT: Unhandled Message %x\n", id));
        }
        break;
    }
}

/***************************************************************************
NAME
    broadcast_client_handle_internal_msg

DESCRIPTION
   Utility function to handle Internal message of Broadcast Client
*/
static void broadcast_client_handle_internal_msg(Task task)
{
    GBSC* broadcast_client = (GBSC*)task;
    broadcast_read_char_value_by_uuid(&broadcast_client->lib_task, ba_version[0], 
                                    broadcast_version_characteristic);
}

/*! @brief Broadcast service client message handler. */
void broadcast_client_msg_handler(Task task, MessageId id, Message message)
{
    if ((id >= GATT_MANAGER_MESSAGE_BASE) && (id < GATT_MANAGER_MESSAGE_TOP))
    {
        broadcast_client_handle_gatt_manager_msg(task, id, message);
    }
    else if(BROADCAST_CLIENT_INTERNAL_START_FINDING_CHAR_VALUES == id)
    {
        broadcast_client_handle_internal_msg(task);
    }
}


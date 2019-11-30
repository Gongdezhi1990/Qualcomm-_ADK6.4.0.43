/****************************************************************************
Copyright (c) 2014 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_ble_advertising.c

DESCRIPTION
    BLE Advertising functionality
*/
#if defined(ENABLE_AMA) && defined(ENABLE_BISTO) 
#error "Advertising BISTO and AMA support together is not supported" 
#endif

#if defined GATT_BISTO_COMM_SERVER && !defined ENABLE_BISTO
#error GATT_BISTO_COMM_SERVER defined without ENABLE_BISTO
#endif

#include "sink_ble_advertising.h"


#ifdef ENABLE_BISTO
#include "bisto/sink2bisto.h"
#endif

#ifdef GATT_BISTO_COMM_SERVER
#define BISTO_SERVICE_UUID (0xFE26)
#define BISTO_SERVICE_DATA_LENGTH (6)
#endif

#ifdef ENABLE_FAST_PAIR
#include "sink_main_task.h"
#include "sink_fast_pair_config_def.h"
#include<byte_utils.h>
#include<util.h>
#include<cryptoalgo.h>
#define FAST_PAIR_GOOGLE_IDENTIFIER 0xFE2C
#define NEW_PAIR_AD_DATA_LENGTH 10
#define NEW_PAIR_AD_SERVICE_DATA_LENGTH 6
#define EMPTY_FILTER_AD_DATA_LENGTH 8
#define EMPTY_FILTER_AD_SERVICE_DATA_LENGTH 4
#define TX_POWER_AD_DATA_LENGTH 2
#define FAST_PAIR_ACCOUNT_DATA_FLAG_VALUE 0
/* global Fast Pair Account Key Data instance */
key_filter_t *g_key_filter = NULL ;
#endif

#ifdef GATT_AMA_SERVER
#define AMA_SERVICE_ADV (0xFE03)
#define AMA_SERVICE_DATA_LENGTH (13)
#define AMA_VENDOR_ID (0x000A)
#define AMA_PRODUCT_ID (0x0001)
#define AMA_ACCESSORY_COLOR (0x00)
#define AMA_DEVICE_STATE (0x02)
#define AMA_RFCOMM_PREFERRED (0x01)
#define AMA_RESERVED (0x00)
#endif

#include "sink_ble.h"
#include "sink_gatt_device.h"
#include "sink_gatt_manager.h"
#include "sink_gatt_server_battery.h"
#include "sink_gatt_server_lls.h"
#include "sink_gatt_server_tps.h"
#include "sink_gatt_server_ias.h"
#include "sink_gatt_server_hrs.h"
#ifdef ACTIVITY_MONITORING
#include "sink_gatt_server_rscs.h"
#include "sink_gatt_server_logging.h"
#include "gatt_logging_server_uuids.h"
#endif
#include "sink_gatt_server_dis.h"
#include "sink_debug.h"
#include "sink_development.h"
#include "sink_utils.h"
#include "sink_configmanager.h"
#include "sink_va_if.h"
#include "sink_ba.h"
#include "sink_ba_ble_gap.h"
#include "sink_statemanager.h"
#include "sink_audio_routing.h"

#include <connection.h>
#include <gatt.h>
#include <local_device.h>

#include <csrtypes.h>
#include <stdlib.h>
#include <string.h>


#ifdef GATT_ENABLED


#ifdef ENABLE_AMA
#include "sink_ama.h"
#define LONG_AMA_ADV
#define SHORT_AMA_ADV
#endif

/* Macro for BLE AD Data Debug */
#ifdef DEBUG_BLE
#include <stdio.h>
#define BLE_AD_INFO(x) DEBUG(x)
#define BLE_AD_INFO_STRING(name, len) {unsigned i; for(i=0;i<len;i++) BLE_AD_INFO(("%c", name[i]));}
#define BLE_AD_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else
#define BLE_AD_INFO(x)
#define BLE_AD_INFO_STRING(name, len)
#define BLE_AD_ERROR(x)
#endif

#ifndef MIN
#define MIN(a, b)   ((a < b) ? a : b)
#endif

#define MODE_TO_MASK(mode)  (1 << mode)

#define SIZE_UUID16                         (2)
#define AD_FIELD_LENGTH(data_length)        (data_length + 1)
#define USABLE_SPACE(space)                 ((*space) > AD_DATA_HEADER_SIZE ? (*space) - AD_DATA_HEADER_SIZE : 0)

#define SERVICE_DATA_LENGTH(num_services)   (num_services * OCTETS_PER_SERVICE)
#define NUM_SERVICES_THAT_FIT(space)        (USABLE_SPACE(space) / OCTETS_PER_SERVICE)

#define WRITE_AD_DATA(ad_data, space, value) \
{ \
    *ad_data = value; \
    BLE_AD_INFO(("0x%02x ", *ad_data)); \
    ad_data++; \
    (*space)--; \
}

#ifdef ENABLE_FAST_PAIR
/* Global Fast Pair Data instance */
extern  sink_fast_pair_data_t  g_fast_pair_data;
#endif

/******************************************************************************/
static uint8* setupFlagsAdData(uint8* ad_data, uint8* space, adv_discoverable_mode_t mode, ble_gap_read_name_t reason)
{
    uint16 flags = 0;

    if(reason == ble_gap_read_name_broadcasting || reason == ble_gap_read_name_associating)
        flags = BLE_FLAGS_DUAL_HOST;

    if (mode == adv_discoverable_mode_general)
        flags |= BLE_FLAGS_GENERAL_DISCOVERABLE_MODE;
    else if (mode == adv_discoverable_mode_limited)
        flags |= BLE_FLAGS_LIMITED_DISCOVERABLE_MODE;

#ifdef ENABLE_FAST_PAIR
    flags |= BLE_FLAGS_DUAL_HOST;
    flags |= BLE_FLAGS_DUAL_CONTROLLER;
#endif    

    /* According to CSSv6 Part A, section 1.3 "FLAGS" states:
        "The Flags data type shall be included when any of the Flag bits are non-zero and the advertising packet
        is connectable, otherwise the Flags data type may be omitted"
     */
    if(flags)
    {
        BLE_AD_INFO(("AD Data: flags = ["));

        ad_data = bleAddHeaderToAdData(ad_data, space, FLAGS_DATA_LENGTH, ble_ad_type_flags);
        WRITE_AD_DATA(ad_data, space, flags);

        BLE_AD_INFO(("]\n"));
    }
    return ad_data;
}

/******************************************************************************/
static uint8* updateServicesAdData(uint8* ad_data, uint8* space)
{
#ifdef GATT_AMA_SERVER
#ifdef SHORT_AMA_ADV
    if(SinkAmaIsAdvertiseAmaEnabled())
    {
        ad_data = bleAddServiceUuidToAdData(ad_data, space, AMA_SERVICE_ADV);
    }
#endif
#endif

    if (sinkGattBatteryServiceEnabled())
    {
        BLE_AD_INFO(("BAS "));
        ad_data = bleAddServiceUuidToAdData(ad_data, space, GATT_SERVICE_UUID_BATTERY_SERVICE);
    }

    if (sinkGattLinkLossServiceEnabled())
    {
        BLE_AD_INFO(("LLS "));
        ad_data = bleAddServiceUuidToAdData(ad_data, space, GATT_SERVICE_UUID_LINK_LOSS);
    }

    if (sinkGattTxPowerServiceEnabled())
    {
        BLE_AD_INFO(("TPS "));
        ad_data = bleAddServiceUuidToAdData(ad_data, space, GATT_SERVICE_UUID_TX_POWER);
    }

    if (sinkGattImmAlertServiceEnabled())
    {
        BLE_AD_INFO(("IAS "));
        ad_data = bleAddServiceUuidToAdData(ad_data, space, GATT_SERVICE_UUID_IMMEDIATE_ALERT);
    }

    if (sinkGattHeartRateServiceEnabled())
    {
        BLE_AD_INFO(("HRS "));
        ad_data = bleAddServiceUuidToAdData(ad_data, space, GATT_SERVICE_UUID_HEART_RATE);
    }

#ifdef ACTIVITY_MONITORING
    if (sinkGattRSCServiceEnabled())
    {
        BLE_AD_INFO(("RSCS"));
        ad_data = bleAddServiceUuidToAdData(ad_data, space, GATT_SERVICE_UUID_RUNNING_SPEED_AND_CADENCE);
    }
    /* Advertising of 128 bit UUIDS is not currently supported by the Sink App */
/*
    if (sinkGattLoggingServiceEnabled() && (*space))
    {
        BLE_AD_INFO(("LOGGING"));
        ad_data = bleAddServiceUuidToAdData(ad_data, space, GATT_SERVICE_UUID_LOGGING);
    }
*/
#endif

    if (sinkGattDeviceInfoServiceEnabled())
    {
        BLE_AD_INFO(("DIS "));
        ad_data = bleAddServiceUuidToAdData(ad_data, space, GATT_SERVICE_UUID_DEVICE_INFORMATION);
    }

    return ad_data;
}

/******************************************************************************/
static uint16 getNumberOfServersEnabled(void)
{
    uint16 num_services = 0;

#ifdef GATT_AMA_SERVER
#ifdef SHORT_AMA_ADV

    if(SinkAmaIsAdvertiseAmaEnabled())
    {
        /* AMA TODO : Revisit for future improvement */
        num_services++;
    }
#endif
#endif

    if (sinkGattBatteryServiceEnabled())
        num_services++;

    if (sinkGattLinkLossServiceEnabled())
        num_services++;

    if (sinkGattTxPowerServiceEnabled())
        num_services++;

    if (sinkGattImmAlertServiceEnabled())
        num_services++;

    if (sinkGattHeartRateServiceEnabled())
        num_services++;

#ifdef ACTIVITY_MONITORING
    if (sinkGattRSCServiceEnabled())
        num_services++;
/* We must not add this until 128 bit UUIDs are supported in Sink App! */
/*    if (sinkGattLoggingServiceEnabled()) */
/*        num_services++; */
#endif

    if (sinkGattDeviceInfoServiceEnabled())
        num_services++;

    return num_services;
}

/******************************************************************************/
static uint8* setupServicesAdData(uint8* ad_data, uint8* space, ble_gap_read_name_t reason)
{
    if(reason == ble_gap_read_name_broadcasting || reason == ble_gap_read_name_advertising_broadcasting)
    {
        /* Add the broadcaster variant IV service data */
        ad_data = setupBroadcasterIvServiceData(ad_data, space);
        if(reason == ble_gap_read_name_broadcasting)
            return ad_data;
    }

    if(reason == ble_gap_read_name_associating)
    {
        /* Set up receiver association service data */
        ad_data = setupReceiverAssociationServiceData(ad_data, space);
    }
    else
    {
        uint16 num_services = getNumberOfServersEnabled();
        uint8 num_services_that_fit = NUM_SERVICES_THAT_FIT(space);

        if (num_services && num_services_that_fit)
        {
            uint8 service_data_length;
            uint8 service_field_length;
            uint8 ad_tag = ble_ad_type_complete_uuid16;

            /* Is there enough room to store the complete list of services defined for the device? */
            if(num_services > num_services_that_fit)
            {
                /* Advertise incomplete list */
                ad_tag = ble_ad_type_more_uuid16;
                num_services = num_services_that_fit;
            }

            /* Setup AD data for the services */
            BLE_AD_INFO(("AD Data: services = ["));
            service_data_length = SERVICE_DATA_LENGTH(num_services);
            service_field_length = AD_FIELD_LENGTH(service_data_length);
            ad_data = bleAddHeaderToAdData(ad_data, space, service_field_length, ad_tag);

            /* Add UUID of enabled services to advertising list */
            ad_data = updateServicesAdData(ad_data, space);

            BLE_AD_INFO(("]\n"));
        }
    }
    /* return the advertising data counter as next available index based on configured number of services */
    return ad_data;
}

/******************************************************************************/
static uint8* setupLocalNameAdvertisingData(uint8 *ad_data, uint8* space, uint16 size_local_name, const uint8 * local_name)
{
    uint8 name_field_length;
    uint8 name_data_length = size_local_name;
    uint8 ad_tag = ble_ad_type_complete_local_name;
    uint8 usable_space = USABLE_SPACE(space);

    if((name_data_length == 0) || usable_space <= 1)
        return ad_data;

    if(name_data_length > usable_space)
    {
        ad_tag = ble_ad_type_shortened_local_name;
        name_data_length = usable_space;
    }

    BLE_AD_INFO(("Setup local name = ["));

    name_field_length = AD_FIELD_LENGTH(name_data_length);
    ad_data = bleAddHeaderToAdData(ad_data, space, name_field_length, ad_tag);

    /* Setup the local name advertising data */
    memmove(ad_data, local_name, name_data_length);
    BLE_AD_INFO_STRING(ad_data, name_data_length);
    ad_data += name_data_length;
    *space -= name_data_length;

    BLE_AD_INFO(("]\n"));
    return ad_data;
}


#ifdef ENABLE_FAST_PAIR
/******************************************************************************/
static bool getFastPairModelInfoConfig(fast_pair_model_info_config_def_t* model_info_config)
{
    uint16 size;
    fast_pair_model_info_config_def_t* fast_pair_ad_config;

    size = configManagerGetReadOnlyConfig(FAST_PAIR_MODEL_INFO_CONFIG_BLK_ID, (const void **)&fast_pair_ad_config);

    *model_info_config = *fast_pair_ad_config;

    configManagerReleaseConfig(FAST_PAIR_MODEL_INFO_CONFIG_BLK_ID);

    return(size !=0);    
}

/******************************************************************************/

void bleAccountKeySha256Req(uint8 input_array[SHA256_INPUT_ARRAY_LENGTH])
{
    ConnectionEncryptBlockSha256(&theSink.task, (uint16 *)input_array, (SHA256_INPUT_ARRAY_LENGTH));
}

void bleSetupFastPairAdvertisingDataReconnection(adv_discoverable_mode_t mode, ble_gap_read_name_t reason)
{
    uint8 space = MAX_AD_DATA_SIZE_IN_OCTETS * sizeof(uint8);
    uint8 *ad_start = malloc(space);
    uint8 i;
    if(ad_start)
    {
        uint8* ad_head = ad_start;

        ad_head = setupFlagsAdData(ad_head, &space, mode, reason);

        /* Add account data in bloom filter format in advertisement */
        BLE_AD_INFO(("Fast Pair AD Data for Reconnection: ["));
        for(i = 0; i < g_key_filter->len + 8; i++)
            WRITE_AD_DATA(ad_head, &space, g_key_filter->ad_data[i]);
        BLE_AD_INFO(("]\n"));

        if(mode == adv_discoverable_mode_limited)
        {
            /* In BLE bondable mode(BR/EDR is non-discoverable), add Service UUIDs if
               free space is available in Advertisement data after adding account data
             */
            ad_head = setupServicesAdData(ad_head, &space, reason);
        }

        ConnectionDmBleSetAdvertisingDataReq(ad_head - ad_start, ad_start);
        BLE_AD_INFO(("bleSetupFastPairAdvertisingDataReconnection remaining space in AD data [%x]\n", space));

        free (ad_start);
        free(g_key_filter->ad_data);
        if(g_key_filter->account_keys.keys)
        {
            free(g_key_filter->account_keys.keys);
        }
        free(g_key_filter);
        g_key_filter = NULL;
    }
}

void bleFastPairHandleAccountKeys(CL_CRYPTO_HASH_CFM_T *cfm)
{
    uint32 hash_value, hash_account_key;
    uint8 j;
    for (j = 0; j < 16; j += 2)
    {
        hash_value  = ((uint32) cfm->hash[j] & 0x00FF) << 24;
        hash_value |= ((uint32) cfm->hash[j] & 0xFF00) << 8;
        hash_value |= ((uint32) cfm->hash[j + 1] & 0x00FF) << 8;
        hash_value |= ((uint32) cfm->hash[j + 1] & 0xFF00) >> 8;

        hash_account_key = hash_value % (g_key_filter->s * 8);
        g_key_filter->key_filter[hash_account_key/ 8] = g_key_filter->key_filter[hash_account_key / 8] | (1 << (hash_account_key % 8));
    }
    
     if(g_key_filter->number_of_keys_processed == g_key_filter->number_of_keys)
     {
         bleSetupFastPairAdvertisingDataReconnection(g_key_filter->mode, g_key_filter->reason);
         gapStartFastPairAdvTimer();
         return;
     }  
      
     if(g_key_filter->number_of_keys_processed < g_key_filter->number_of_keys)
     {          
         memmove(g_key_filter->temp, &g_key_filter->account_keys.keys[g_key_filter->number_of_keys_processed][0], 16);
         bleAccountKeySha256Req(g_key_filter->temp);
         g_key_filter->number_of_keys_processed++;
     }

}

uint8 bleFastPairCalcAccountKeyData(fast_pair_account_keys_t *keys, uint8 *buf)
{
    uint8 size;

    if (keys->num_keys == 0)
    {
        /* Nothing to do when we don't have account keys */
        size = 0;
    }
    else
    {
        /* size of account key filter */
        g_key_filter->s = keys->num_keys * 6 / 5 + 3;
        size = g_key_filter->s + 3;

        if (buf)
        {         
            /* 1 Byte Salt */
            uint8 salt = UtilRandom() & 0xFF;
            g_key_filter->key_filter = &buf[1];
            memset(buf, 0, size);

            buf[0] = (g_key_filter->s << 4) & 0xF0;
            buf[size - 2] = 0x11;
            buf[size - 1] = salt;

            g_key_filter->temp[16] = salt;

            g_key_filter->number_of_keys_processed = 0;
            g_key_filter->number_of_keys = keys->num_keys;
            memmove(g_key_filter->temp, &keys->keys[g_key_filter->number_of_keys_processed][0], 16);
            bleAccountKeySha256Req(g_key_filter->temp);
            g_key_filter->number_of_keys_processed++;              
        }
    }

    return size;
}

/* Set up fast pair advertising data for new pair*/
static uint8* setupFastPairAdvertisingDataNewPair(uint8 *fast_pair_ad_data, uint8 *space)
{
    fast_pair_model_info_config_def_t fast_pair_ad_config;
    uint32 model_id = 0;
    int8 tx_power = 0;

    if(getFastPairModelInfoConfig(&fast_pair_ad_config))
    {
        model_id = (((uint32)fast_pair_ad_config.fast_pair_model_id_hi << 16) | (uint32)fast_pair_ad_config.fast_pair_model_id_lo) & 0xFFFFFFUL;
        tx_power = (int8)fast_pair_ad_config.fast_pair_tx_power;
    }
    else
    {
        BLE_AD_INFO(("Fast Pair readonly config invalid!\n"));
    }

    if(NEW_PAIR_AD_DATA_LENGTH <= (*space))
    {
        BLE_AD_INFO(("Fast Pair AD Data for new Pair: ["));
        
        /*Service Data*/
        WRITE_AD_DATA(fast_pair_ad_data, space, (uint8)NEW_PAIR_AD_SERVICE_DATA_LENGTH);
        WRITE_AD_DATA(fast_pair_ad_data, space, (uint8)ble_ad_type_service_data);
        WRITE_AD_DATA(fast_pair_ad_data, space, (uint8)(FAST_PAIR_GOOGLE_IDENTIFIER & 0xFF));
        WRITE_AD_DATA(fast_pair_ad_data, space, (uint8)((FAST_PAIR_GOOGLE_IDENTIFIER >> 8) & 0xFF));
        WRITE_AD_DATA(fast_pair_ad_data, space, (uint8)((model_id >> 16) & 0xFF));
        WRITE_AD_DATA(fast_pair_ad_data, space, (uint8)((model_id >> 8) & 0xFF));
        WRITE_AD_DATA(fast_pair_ad_data, space, (uint8)(model_id & 0xFF));
        
        /* TX power */
        WRITE_AD_DATA(fast_pair_ad_data, space, (uint8)TX_POWER_AD_DATA_LENGTH);
        WRITE_AD_DATA(fast_pair_ad_data, space, (uint8)ble_ad_type_tx_power_level);
        WRITE_AD_DATA(fast_pair_ad_data, space,(uint8)tx_power);
        BLE_AD_INFO(("]\n"));
    }
    return fast_pair_ad_data;    
}

/* Set up fast pair advertising data for empty account key filter -This part expected to change on later GFPS spec*/
static uint8* setupFastPairAdvertisingDataEmptyFilter(uint8 *fast_pair_ad_data, uint8 *space)
{
    fast_pair_model_info_config_def_t fast_pair_ad_config;
    int8 tx_power = 0;

    /*There should not be any account key present*/
    if(!gapFastPairAccountKeysPresent())
    {
        if(getFastPairModelInfoConfig(&fast_pair_ad_config))
        {
            tx_power = (int8)fast_pair_ad_config.fast_pair_tx_power;
        }
        else
        {
            BLE_AD_INFO(("Fast Pair readonly config invalid!\n"));
        }

        if(EMPTY_FILTER_AD_DATA_LENGTH <= (*space))
        {
            BLE_AD_INFO(("Fast Pair AD Data Empty Account Key Filter: ["));
            
            /*Service Data*/
            WRITE_AD_DATA(fast_pair_ad_data, space, (uint8)EMPTY_FILTER_AD_SERVICE_DATA_LENGTH);
            WRITE_AD_DATA(fast_pair_ad_data, space, (uint8)ble_ad_type_service_data);
            WRITE_AD_DATA(fast_pair_ad_data, space, (uint8)(FAST_PAIR_GOOGLE_IDENTIFIER & 0xFF));
            WRITE_AD_DATA(fast_pair_ad_data, space, (uint8)((FAST_PAIR_GOOGLE_IDENTIFIER >> 8) & 0xFF));
            WRITE_AD_DATA(fast_pair_ad_data, space, 0x00);/*Flags*/
            
            /* TX power */
            WRITE_AD_DATA(fast_pair_ad_data, space, (uint8)TX_POWER_AD_DATA_LENGTH);
            WRITE_AD_DATA(fast_pair_ad_data, space, (uint8)ble_ad_type_tx_power_level);
            WRITE_AD_DATA(fast_pair_ad_data, space,(uint8)tx_power);
            BLE_AD_INFO(("]\n"));
        }
    }
    return fast_pair_ad_data;    
}

/* Set up fast pair advertising data for reconnection */
void bleFastPairAdvertisingDataReconnection(void)
{
    int8 tx_power = 0;
    uint8 i;
    fast_pair_model_info_config_def_t fast_pair_ad_config;

    if(getFastPairModelInfoConfig(&fast_pair_ad_config))
    {
        tx_power = (int8)fast_pair_ad_config.fast_pair_tx_power;
    }
    /* First update the number of account keys present in PS store */
    g_key_filter->account_keys.num_keys = g_fast_pair_data.account_key.num_keys;
    if(g_key_filter->account_keys.num_keys)
    {
        BLE_AD_INFO(("Fast Pair account keys num: %d\n", g_key_filter->account_keys.num_keys));
              
        g_key_filter->account_keys.keys = PanicUnlessMalloc(g_key_filter->account_keys.num_keys * sizeof(g_key_filter->account_keys.keys[0]));
        for (i = 0; i < g_key_filter->account_keys.num_keys; i++)
        {
          memcpy(&g_key_filter->account_keys.keys[i][0], &g_fast_pair_data.account_key.keys[i][0], FAST_PAIR_ACCOUNT_KEY_LEN);
        }


            
        g_key_filter->len = bleFastPairCalcAccountKeyData(&g_key_filter->account_keys, NULL);
        g_key_filter->ad_data = PanicUnlessMalloc(g_key_filter->len + 8);
              
        /* TX power */
        g_key_filter->ad_data[0] = TX_POWER_AD_DATA_LENGTH;
        g_key_filter->ad_data[1] = ble_ad_type_tx_power_level;
        g_key_filter->ad_data[2] = (uint8)tx_power;
              
        /* Service Data */      
        g_key_filter->ad_data[3] = g_key_filter->len + 4;
        g_key_filter->ad_data[4] = ble_ad_type_service_data;
        g_key_filter->ad_data[5] = FAST_PAIR_GOOGLE_IDENTIFIER & 0xFF;
        g_key_filter->ad_data[6] = (FAST_PAIR_GOOGLE_IDENTIFIER >> 8) & 0xFF;
        g_key_filter->ad_data[7] = FAST_PAIR_ACCOUNT_DATA_FLAG_VALUE;

        if (g_key_filter->len)
        {
           bleFastPairCalcAccountKeyData(&g_key_filter->account_keys, &g_key_filter->ad_data[8]);
        }
    }
}

#endif /*ENABLE_FAST_PAIR*/

/******************************************************************************/
uint8* bleAddHeaderToAdData(uint8* ad_data, uint8* space, uint8 size, uint8 type)
{
    WRITE_AD_DATA(ad_data, space, size);
    WRITE_AD_DATA(ad_data, space, type);

    return ad_data;
}

/******************************************************************************/
uint8* bleAddServiceUuidToAdData(uint8* ad_data, uint8* space, uint16 uuid)
{
    if(*space >= SIZE_UUID16)
    {
        *ad_data = (uuid & 0xFF);
        *(ad_data + 1) = (uuid >> 8);

        BLE_AD_INFO(("0x%02x%02x ", ad_data[1], ad_data[0]));
        ad_data += SIZE_UUID16;
        *space -= SIZE_UUID16;
    }
    else
    {
        BLE_AD_INFO(("Not Added"));
    }
    return ad_data;
}


#ifdef GATT_AMA_SERVER
/*******************************************************************************
NAME
    bleSetAmaServiceData

DESCRIPTION
    Setup AMA Service data

RETURNS
    Pointer beyond AMA service data
*/
static uint8* bleSetAmaServiceData(uint8 *ama_service_data, uint8 *space)
{
    BLE_AD_INFO(("AMA AD Data in Scan Response: ["));

    if((AMA_SERVICE_DATA_LENGTH+1) <= (*space))
    {     
        WRITE_AD_DATA(ama_service_data, space, (uint8)AMA_SERVICE_DATA_LENGTH);     /*Length for Service Data AD Type (23 bytes)*/
        WRITE_AD_DATA(ama_service_data, space, (uint8)ble_ad_type_service_data);    /*Service Data AD Type Identifier*/

        WRITE_AD_DATA(ama_service_data, space, (uint8)(AMA_SERVICE_ADV & 0xFF));    /*AMA Service ID*/
        WRITE_AD_DATA(ama_service_data, space, (uint8)((AMA_SERVICE_ADV >> 8) & 0xFF));

        WRITE_AD_DATA(ama_service_data, space, (uint8)(AMA_VENDOR_ID & 0xFF));      /*Vendor Id assigned by BT*/
        WRITE_AD_DATA(ama_service_data, space, (uint8)((AMA_VENDOR_ID >> 8) & 0xFF));

        WRITE_AD_DATA(ama_service_data, space, (uint8)(AMA_PRODUCT_ID & 0xFF));     /*Product Id for Alexa-enabled Headphones*/
        WRITE_AD_DATA(ama_service_data, space, (uint8)((AMA_PRODUCT_ID >> 8) & 0xFF));

        WRITE_AD_DATA(ama_service_data, space, (uint8)AMA_ACCESSORY_COLOR);         /*Color of the Accessory*/
        WRITE_AD_DATA(ama_service_data, space, (uint8)AMA_DEVICE_STATE);            /*Device State bit mask.  Bit 1: 1, if classic bluetooth is discoverable*/
        WRITE_AD_DATA(ama_service_data, space, (uint8)AMA_RFCOMM_PREFERRED);   
        WRITE_AD_DATA(ama_service_data, space, (uint8)AMA_RESERVED);   
        WRITE_AD_DATA(ama_service_data, space, (uint8)AMA_RESERVED);   
        WRITE_AD_DATA(ama_service_data, space, (uint8)AMA_RESERVED);
    }
    BLE_AD_INFO(("]\n"));
    return ama_service_data;
}
#endif /* GATT_AMA_SERVER */

#ifdef GATT_BISTO_COMM_SERVER
/*******************************************************************************
NAME
    bleSetBistoServiceData

DESCRIPTION
    Setup Bisto Service data

RETURNS
    Pointer beyond Bisto service data
*/
static uint8 *bleSetBistoServiceData(uint8 *service_data, uint8 *space)
{
    if (*space >= BISTO_SERVICE_DATA_LENGTH + 1)
    {
        uint32 model_id;

        PanicFalse(BistoGetModelId(&model_id));

        BLE_AD_INFO(("Bisto Scan Response ["));

        WRITE_AD_DATA(service_data, space, BISTO_SERVICE_DATA_LENGTH);
        WRITE_AD_DATA(service_data, space, ble_ad_type_service_data);

        WRITE_AD_DATA(service_data, space, BISTO_SERVICE_UUID & 0xFF);
        WRITE_AD_DATA(service_data, space, BISTO_SERVICE_UUID >> 8);

        WRITE_AD_DATA(service_data, space, model_id & 0xFF);
        WRITE_AD_DATA(service_data, space, (model_id >> 8) & 0xFF);
        WRITE_AD_DATA(service_data, space, (model_id >> 16) & 0xFF);

        BLE_AD_INFO(("]\n"));
    }
    
    return service_data;
}
#endif /* GATT_BISTO_COMM_SERVER */

/*******************************************************************************
NAME
    bleSetupScanResponseData

DESCRIPTION
    Setup Scan Response Data with Device name and service data

PARAMETERS
    Local name and size

RETURNS
    None
*/
static void bleSetupScanResponseData(uint16 size_local_name, const uint8 *local_name)
{
    uint8 space = MAX_AD_DATA_SIZE_IN_OCTETS * sizeof(uint8);
    uint8 *srd_start = PanicUnlessMalloc(space);
    uint8 *srd_head = srd_start;

#ifdef GATT_AMA_SERVER
#ifdef LONG_AMA_ADV
        if(SinkAmaIsAdvertiseAmaEnabled() && (stateManagerGetState() != deviceLimbo))
        {
            srd_head = bleSetAmaServiceData(srd_head, &space);
        }
#endif
#endif

#ifdef GATT_BISTO_COMM_SERVER
    srd_head = bleSetBistoServiceData(srd_head, &space);
#endif

    /* Use the remaining space for the device name */
    BLE_AD_INFO(("BLE ad name \"%.*s\" = %d\n", size_local_name, local_name, size_local_name));
    srd_head = setupLocalNameAdvertisingData(srd_head, &space, size_local_name, local_name);

    ConnectionDmBleSetScanResponseDataReq(srd_head - srd_start, srd_start);

    BLE_AD_INFO(("bleSetupScanResponseData remaining space %d\n", space));

    free (srd_start);
}

uint8 *bleBistoPrefixedName(const uint8 *name, uint16 *size_name)
{
    char *prefix = "LE-";
    uint16 size_prefix = strlen(prefix);
    uint8 *prefixed_name = PanicUnlessMalloc(size_prefix + *size_name);

    memmove(prefixed_name, prefix, size_prefix);
    memmove(prefixed_name + size_prefix, name, *size_name);

    *size_name += size_prefix;
    return prefixed_name;
}

/******************************************************************************/
void bleSetupAdvertisingData(uint16 size_local_name, const uint8 *local_name, adv_discoverable_mode_t mode, ble_gap_read_name_t reason)
{

#ifdef ENABLE_FAST_PAIR
    advertising_data_type_t ad_type = le_normal_adverts;

    if(stateManagerGetState() != deviceLimbo)
    {
        /* For BA during association and BA Broadcaster during broadcasting BA adverts will be used */
        if((gapBaGetAssociationInProgress()) ||
            (BA_BROADCASTER_MODE_ACTIVE && (reason & ble_gap_read_name_broadcasting)))
        {
            /* For BA broadcaster during when BLE is bondable the advertising reason will be advertising_broadcasting */
            ad_type = le_ba_association_broadcasting_adverts;
        }
        else
        {
             /* If PDL is empty/reset 
               - Then account keys will not be present 
               - BR/EDR can be discoverable/non-discoverable
               - BA association may be TRUE/FALSE
               - BLE bonding mode may be TRUE/FALSE
               - SCO will not be active as we have not paired yet
             */
             if(ConnectionTrustedDeviceListSize()==0)
             {
                 if(stateManagerGetState() == deviceConnDiscoverable)
                 {
                    /* If BR/EDR is discoverable then we should be advertising Google Fast Pair model ID
                        The only exception to this will be 
                        -In BA association mode for BA receiver. This was handled at the start of the function.
                     */
                    ad_type = le_fp_model_id_adverts;
                 }
                 else
                 {
                     /* If we have timed out of BR/EDR discoverable mode and BLE bondable mode,
                        advertise empty account data filter
                      */
                     ad_type = le_fp_empty_account_data_adverts;
                 }
                 
             }
             else
             {
                 /* PDL is not Empty 
                    - BR/EDR can discoverable/non-discoverable
                    - BA association may be TRUE/FALSE
                    - BLE bonding mode may be TRUE/FALSE
                    - Account keys may be present/not-present
                    - SCo/eSCO can be active/inactive
                 */
                if(!audioRouteIsScoActive())
                {
                    /* Enable FP adverts only when no SCO/eSCO is present. 
                       Fast Pairing will not be allowed during the time when 
                       SCO/eSCO is active.
                     */
                    if(stateManagerGetState() == deviceConnDiscoverable)
                    {
                        /*  If BR/EDR is discoverable then we should be advertising Google Fast Pair model ID
                        The exceptions to this are during 
                        - BA association mode for BA receiver. This was handled at the start of the function. 
                        - SCO/eSCO being active. Checked above
                        */
                        ad_type = le_fp_model_id_adverts;
                    }
                    else if(gapFastPairAccountKeysPresent())
                    {
                        /* Use Fast Pair account data based bloom filter advertisement only if 
                        - Device is in non-discoverable mode
                        - Device is not in BLE bondable mode
                        - BA association is not in progress for BA receiver.
                        - No active SCO/eSCO is present
                        */
                        ad_type = le_fp_account_data_adverts;
                    }
                    else
                    {
                        /* Use empty account data bloom filter when no account keys are present */
                        ad_type = le_fp_empty_account_data_adverts;
                    }
                }
                else
                {
                    /* Default advertsing data */
                    ad_type = le_normal_adverts;
                }
             }
        }
    }
    else
    {
        /* In Limbo State only regular BLE adertsing data will be allowed 
           - No Fast Pair adverts(Model ID or Account Data)
           - No BA Broadcasting specific data
           - No VA specific data
         */
         ad_type = le_normal_adverts;
    }

    if(ad_type != le_fp_account_data_adverts)
#endif
    {
        uint8 space = MAX_AD_DATA_SIZE_IN_OCTETS * sizeof(uint8);
        uint8 *ad_start = malloc(space);

        if(ad_start)
        {
            uint8* ad_head = ad_start;

            ad_head = setupFlagsAdData(ad_head, &space, mode, reason);


#ifdef ENABLE_FAST_PAIR
                if(ad_type == le_fp_model_id_adverts)
                {
                    ad_head = setupFastPairAdvertisingDataNewPair(ad_head, &space);
                }
                else if(ad_type == le_fp_empty_account_data_adverts)
                {
                    ad_head = setupFastPairAdvertisingDataEmptyFilter(ad_head, &space);
                }
#endif
            ad_head = setupServicesAdData(ad_head, &space, reason);

            /*Setup advertisement data in adverts*/
            ConnectionDmBleSetAdvertisingDataReq((MAX_AD_DATA_SIZE_IN_OCTETS * sizeof(uint8)) - space, ad_start);
            BLE_AD_INFO(("bleSetupAdvertisingData remaining space in AD data [%x]\n", space));

            free (ad_start);
        }
    }
#ifdef ENABLE_FAST_PAIR      
    else
    {
        if(g_key_filter == NULL)
            g_key_filter = PanicUnlessMalloc(sizeof(key_filter_t));

        g_key_filter->mode = mode;
        g_key_filter->reason = reason; 

        bleFastPairAdvertisingDataReconnection();
    }

#endif

#ifdef GATT_BISTO_COMM_SERVER
    local_name = bleBistoPrefixedName(local_name, &size_local_name);
#endif

    /*Setup advertisement data in Scan Response Data with Device name and AMA service data, if AMA enabled. 
      This is for both bondable and non-bondable mode*/      
    bleSetupScanResponseData(size_local_name, local_name);

#ifdef GATT_BISTO_COMM_SERVER
    free((uint8 *) local_name);
#endif
}

/******************************************************************************/
void bleHandleSetAdvertisingData(const CL_DM_BLE_SET_ADVERTISING_DATA_CFM_T * cfm)
{
    ble_gap_event_t event;

    BLE_AD_INFO(("CL_DM_BLE_SET_ADVERTISING_DATA_CFM [%x]\n", cfm->status));

    if (cfm->status != success)
    {
        BLE_AD_ERROR(("  Failed!\n"));
    }

    /* Send GAP event after set of advertising data */
    event.id = ble_gap_event_set_advertising_complete;
    event.args = NULL;
    sinkBleGapEvent(event);
}

#endif /* GATT_ENABLED */

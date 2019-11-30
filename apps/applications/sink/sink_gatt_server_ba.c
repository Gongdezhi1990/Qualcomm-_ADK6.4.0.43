/****************************************************************************
Copyright (c) 20015 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_gatt_server_ba.c

DESCRIPTION
    Routines to handle messages sent from the GATT Broadcast Audio Server Task.
*/
/* Firmware headers */
#include <csrtypes.h>
#include <message.h>
#include <util.h>

/* Library headers */
#include <gatt_broadcast_server.h>
#include <broadcast_stream_service_record.h>

/* Application headers */
#include "sink_main_task.h"
#include "sink_gatt_common.h"
#include "sink_gatt_db.h"
#include "sink_ble.h"
#include "sink_gatt_server.h"
#include "sink_gatt_server_ba.h"
#include "sink_development.h"
#include "sink_ba_broadcaster.h"
#include "sink_utils.h"
#include "sink_ba.h"
#include "sink_private_data.h"

#if defined GATT_ENABLED && defined GATT_BROADCAST_SERVER
#include "sink_broadcast_audio_config_def.h"

#ifdef DEBUG_GATT_BA_SERVER
#define GATT_BA_SERVER_INFO(x) DEBUG(x)
#define GATT_BA_SERVER_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else
#define GATT_BA_SERVER_INFO(x)
#define GATT_BA_SERVER_ERROR(x)
#endif

/*-----------------Broadcaster identifiers-------------------*/
#define BROADCAST_IDENTIFIER_PRODUCT    0x0A02
#define BROADCAST_IDENTIFIER_VERSION    0x0304

/*---------------------CELT related Defines ----------------*/
/* Security flags for the CELT stream. */
#define CELT_SECURITY_FLAGS (BSSR_SECURITY_ENCRYPT_TYPE_AESCCM | BSSR_SECURITY_KEY_TYPE_PRIVATE)

/* Broadcast Stream Service Record for a private encrypted audio stream. */
static const uint8 default_bssr_private[] = {
    BSSR_ENTRY1(STREAM_ID, CELT_STREAM_ID_EC_2_5),
    BSSR_ENTRY1(ERASURE_CODE, BSSR_ERASURE_CODE_2_5),
    BSSR_ENTRY1(SECURITY, BSSR_UINT16_TO_UINT8S(CELT_SECURITY_FLAGS)),
    BSSR_ENTRY1(CHANNELS, BSSR_UINT16_TO_UINT8S(BSSR_CHANNELS_STEREO)),
    BSSR_ENTRY1(SAMPLE_SIZE, BSSR_SAMPLE_SIZE_16_BIT),
    BSSR_ENTRY1(CODEC_TYPE, BSSR_CODEC_TYPE_CELT),
    BSSR_ENTRY1(AFH_CHANNEL_MAP_UPDATE_METHOD, BSSR_AFH_CHANNEL_MAP_UPDATE_METHOD_TRIGGERED_SYNC_TRAIN),

    /* CELT config at 44100 */
    BSSR_ENTRY3(CODEC_CONFIG_CELT,
                BSSR_UINT16_TO_UINT8S(BSSR_CODEC_FREQ_44100HZ),
                BSSR_UINT16_TO_UINT8S(CELT_CODEC_FRAME_SIZE_44100HZ),
                BSSR_UINT16_TO_UINT8S(CELT_CODEC_FRAME_SAMPLES_44100HZ)),
    /* CELT config at 48000 */
    BSSR_ENTRY3(CODEC_CONFIG_CELT,
                BSSR_UINT16_TO_UINT8S(BSSR_CODEC_FREQ_48KHZ),
                BSSR_UINT16_TO_UINT8S(CELT_CODEC_FRAME_SIZE_48KHZ),
                BSSR_UINT16_TO_UINT8S(CELT_CODEC_FRAME_SAMPLES_48KHZ)),

    BSSR_ENTRY1(STREAM_ID, CELT_STREAM_ID_EC_3_9),
    BSSR_ENTRY1(ERASURE_CODE, BSSR_ERASURE_CODE_3_9),
    BSSR_ENTRY1(SECURITY, BSSR_UINT16_TO_UINT8S(CELT_SECURITY_FLAGS)),
    BSSR_ENTRY1(CHANNELS, BSSR_UINT16_TO_UINT8S(BSSR_CHANNELS_STEREO)),
    BSSR_ENTRY1(SAMPLE_SIZE, BSSR_SAMPLE_SIZE_16_BIT),
    BSSR_ENTRY1(CODEC_TYPE, BSSR_CODEC_TYPE_CELT),
    BSSR_ENTRY1(AFH_CHANNEL_MAP_UPDATE_METHOD, BSSR_AFH_CHANNEL_MAP_UPDATE_METHOD_TRIGGERED_SYNC_TRAIN),

    /* CELT config at 44100 */
    BSSR_ENTRY3(CODEC_CONFIG_CELT,
                BSSR_UINT16_TO_UINT8S(BSSR_CODEC_FREQ_44100HZ),
                BSSR_UINT16_TO_UINT8S(CELT_CODEC_FRAME_SIZE_44100HZ_EC3_9),
                BSSR_UINT16_TO_UINT8S(CELT_CODEC_FRAME_SAMPLES_44100HZ)),
    /* CELT config at 48000 */
    BSSR_ENTRY3(CODEC_CONFIG_CELT,
                BSSR_UINT16_TO_UINT8S(BSSR_CODEC_FREQ_48KHZ),
                BSSR_UINT16_TO_UINT8S(CELT_CODEC_FRAME_SIZE_48KHZ_EC3_9),
                BSSR_UINT16_TO_UINT8S(CELT_CODEC_FRAME_SAMPLES_48KHZ))
};

/***************************************************************************
NAME
    baServerUpdateLocalAddress
 
DESCRIPTION
    Utility function to update the local address into the association data
 
PARAMS
    Void

RETURNS
    broadcast_assoc_data Association Data structure
*/
static void baServerUpdateLocalAddress(broadcast_assoc_data* assoc_data)
{
    sinkDataGetLocalBdAddress(&assoc_data->broadcast_addr);
}

/***************************************************************************
NAME
    baServerUpdateBroadcastIdentifier
 
DESCRIPTION
    Utility function to update the broadcast identifier data into the association data
 
PARAMS
    Void

RETURNS
    broadcast_assoc_data Association Data structure
*/
static void baServerUpdateBroadcastIdentifier(broadcast_assoc_data* assoc_data)
{
    assoc_data->broadcast_identifier[0] = BROADCAST_IDENTIFIER_PRODUCT;
    assoc_data->broadcast_identifier[1] = BROADCAST_IDENTIFIER_VERSION;
}

/***************************************************************************
NAME
    baServerCreateAssocConfig
 
DESCRIPTION
    Utility function to create the data structure for configuring association
 
PARAMS
    Void

RETURNS
    broadcast_assoc_data Association Data structure
*/
static broadcast_assoc_data* baServerCreateAssocConfig(void)
{
    broadcast_assoc_data* assoc_data = NULL;

    GATT_BA_SERVER_INFO(("Broadcaster: broadcasterCreateAssocConfig()\n"));

    /* get memory for new association configuration */
    assoc_data = PanicUnlessMalloc(sizeof(broadcast_assoc_data));
    memset(assoc_data, 0, sizeof(broadcast_assoc_data));

     baServerUpdateLocalAddress(assoc_data);
     baServerUpdateBroadcastIdentifier(assoc_data);
  
    return assoc_data;
}


/***************************************************************************
NAME
    baServerUpdateAssocConfigSeckey
 
DESCRIPTION
    Utility function to fill in Security Key field of a 
    broadcast_assoc_data structure
 
PARAMS
    assoc_data Association configuration data
    seckey Security Key
    key_len Security Key Length
RETURNS
    void
*/
static void baServerUpdateAssocConfigSeckey(broadcast_assoc_data* assoc_data,
                                       uint16* seckey, 
                                       uint16 key_len)
{
    GATT_BA_SERVER_INFO(("Broadcaster: broadcasterUpdateAssocConfigSeckey()\n"));

    /* setup the new key, len 16-bit words to bytes */
    assoc_data->seckey_len = (key_len & 0x00ff) * 2;
    memcpy_unpack(assoc_data->seckey, seckey, key_len);
}

/***************************************************************************
NAME
    baServerUpdateAssocConfigDeviceStatus
 
DESCRIPTION
    Utility function to fill in Device Status field of a broadcast_assoc_data structure
 
PARAMS
    assoc_data Association configuration data
    status Device status need to be filled in configuration data
    stream_id Stream ID
RETURNS
    void
*/
static void baServerUpdateAssocConfigStreamID(broadcast_assoc_data* assoc_data,
                                             uint8 stream_id)
{
    GATT_BA_SERVER_INFO(("Broadcaster: baServerUpdateAssocConfigStreamID()\n"));

    assoc_data->device_status_len = BROADCAST_DEVICE_STATUS_MAX_LEN;
    assoc_data->device_status[0] = 0; /* This _must_ be set to zero */
    assoc_data->device_status[1] = stream_id;
}

/***************************************************************************
NAME
    baServerUpdateAssocConfigBSSR
 
DESCRIPTION
    Utility function to fill in BSSR field of a broadcast_assoc_data structure

PARAMS
    assoc_data Association configuration data

RETURNS
    void
*/
static void baServerUpdateAssocConfigBSSR(broadcast_assoc_data* assoc_data)
{
    GATT_BA_SERVER_INFO(("Broadcaster: broadcasterUpdateAssocConfigBSSR()\n"));

    /* Setup broadcast stream service records. */
    assoc_data->stream_service_records_len = sizeof(default_bssr_private);

    /* cast away const, the broadcast_assoc_data structure is is shared by
     * client and server and on the client the memory usage will be dynamic,
     * so the field isn't defined as const, even though we want to use const
     * memory on the server side to store the BSSRs */
    assoc_data->stream_service_records = (uint8*)default_bssr_private;
}


/*******************************************************************************/
bool sinkGattBAServerInitialise(uint16 **ptr)
{
    broadcast_assoc_data* assoc_data = NULL;
    
    /* Initialize the broadcaster server library */
    if (GattBroadcastServerInit( sinkGetBleTask(),  (GBSS*)*ptr, HANDLE_BROADCAST_SERVICE,
                                     HANDLE_BROADCAST_SERVICE_END) == gatt_broadcast_server_status_success)
    {
        GATT_BA_SERVER_INFO(("GATT BA Server initialised\n"));
        /* Update the pointer */
        gattServerSetServicePtr(ptr, gatt_server_service_ba);

        assoc_data = baServerCreateAssocConfig();
        if(assoc_data)
        {
            /* Start updating the data filed*/
            baServerUpdateAssocConfigBSSR(assoc_data);
            /* Update the BA server database with the BSSR value */
            GattBroadcastServerSetupCharacteristics((GBSS*)*ptr, assoc_data);
            
            /* done with updating the server, just free the temporary memory */
            free(assoc_data);
            assoc_data = NULL;
        }
        /* The size of BA Server is also calculated and memory is allsocated.
        * So advance the ptr so that the next Server while initializing.
        * shall not over write the same memory */
        *ptr += ADJ_GATT_STRUCT_OFFSET(GBSS);
        return TRUE;
    }
    
    /* Broadcast Service library initialization failed */
    GATT_BA_SERVER_INFO(("GATT BA Server init failed \n"));
    return FALSE;
}

/*******************************************************************************/
void sinkGattBAServerEnableAssociation(bool enable_assoc)
{
    uint16 index = 0; 
    /* As there is only 1 instance of BA, its fine to access the 0th index of the ble server array*/
    /* Enable/Disable the access for assocaition data */
    GattBroadcastServerEnable(GATT_SERVER.ba_server, enable_assoc);
}

/*******************************************************************************/
void sinkGattBAServerUpdateSecKey(uint16* key, uint16 key_len)
{
    broadcast_assoc_data* assoc_data = NULL;
    uint16 index = 0;
    /* As there is only 1 instance of BA, its fine to access the 0th index of the ble server array*/
    assoc_data = baServerCreateAssocConfig();
    if((key) && (assoc_data))
    {
        /* configure security key */
        baServerUpdateAssocConfigSeckey(assoc_data, key, key_len);
        /* Update the BA server database with the new key */
        GattBroadcastServerSetupCharacteristics(GATT_SERVER.ba_server, assoc_data);
        /* done with updating the server, just free the temporary memory */
        free(assoc_data);
        assoc_data = NULL;
    }
}

/*******************************************************************************/
void sinkGattBAServerUpdateStreamID(uint8 stream_id)
{
    broadcast_assoc_data* assoc_data = NULL;
    uint16 index = 0;
    /* As there is only 1 instance of BA, its fine to access the 0th index of the ble server array*/
    assoc_data = baServerCreateAssocConfig();
    if(assoc_data)
    {
        baServerUpdateAssocConfigStreamID(assoc_data, stream_id);
        /* Update the server assocaition structure */
        GattBroadcastServerSetupCharacteristics(GATT_SERVER.ba_server, assoc_data); 
        /* done with updating the server, just free the temporary memory */
        free(assoc_data);
        assoc_data = NULL;
    }
}

#endif /* GATT_BROADCAST_SERVER */

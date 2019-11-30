/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sink_ba_broadcaster_encryption.c
 
DESCRIPTION
    Handles encryption keys.
*/

#include "sink_ba_broadcaster_encryption.h"
#include "sink_debug.h"

#ifdef ENABLE_BROADCAST_AUDIO

#include "sink_gatt_server_ba.h"
#include "broadcast_stream_service_record.h"

#include "sink_broadcast_audio_config_def.h"
#include "sink_configmanager.h"

#include <broadcast_context.h>

#include <util.h>
#include <panic.h>

#include <string.h>

#ifdef DEBUG_BA_BROADCASTER
#define DEBUG_BROADCASTER(x) DEBUG(x)
#else
#define DEBUG_BROADCASTER(x)
#endif

static bool broadcasterConfigLoadKey(uint16* key_dest)
{
    broadcaster_writeable_keys_config_def_t* rw_config_data = NULL;
    bool ret_val = FALSE;

    if (configManagerGetReadOnlyConfig(BROADCASTER_WRITEABLE_KEYS_CONFIG_BLK_ID, (const void **)&rw_config_data))
    {
        /* if we loaded something, but the first words is 0x0000, return FALSE
        this is just initialised data not a real key */
        if (rw_config_data->broadcaster_keys.sec[0] != 0x0000)
        {
            /* get data from PS and copy section we need to destination */
            memmove(key_dest, &rw_config_data->broadcaster_keys.sec, sizeof(rw_config_data->broadcaster_keys.sec));
            ret_val = TRUE;
        }
        configManagerReleaseConfig(BROADCASTER_WRITEABLE_KEYS_CONFIG_BLK_ID);
    }
    return ret_val;
}

static bool broadcasterConfigLoadFixedIv(uint16* fixed_iv_dest)
{
    broadcaster_writeable_keys_config_def_t* rw_config_data = NULL;
    bool ret_val = FALSE;

    if (configManagerGetReadOnlyConfig(BROADCASTER_WRITEABLE_KEYS_CONFIG_BLK_ID, (const void **)&rw_config_data))
    {
        /* if we loaded something, but the first words is 0x0000, return FALSE
        this is just initialised data not a real fixed IV */
        if (rw_config_data->broadcaster_keys.fiv[0] != 0x0000)
        {
            /* get data from PS and copy section we need to destination */
            memmove(fixed_iv_dest, &rw_config_data->broadcaster_keys.fiv, sizeof(rw_config_data->broadcaster_keys.fiv));
            ret_val = TRUE;
        }
        configManagerReleaseConfig(BROADCASTER_WRITEABLE_KEYS_CONFIG_BLK_ID);
    }
    return ret_val;
}

static void broadcasterConfigStoreKey(uint16* key)
{
    broadcaster_writeable_keys_config_def_t* rw_config_data = NULL;

    DEBUG_BROADCASTER(("Broadcaster: broadcasterConfigStoreKey(): Security Key\n"));

    if (configManagerGetWriteableConfig(BROADCASTER_WRITEABLE_KEYS_CONFIG_BLK_ID, (void **)&rw_config_data, 0))
    {
        memmove(&rw_config_data->broadcaster_keys.sec, key, sizeof(rw_config_data->broadcaster_keys.sec));
        configManagerUpdateWriteableConfig(BROADCASTER_WRITEABLE_KEYS_CONFIG_BLK_ID);
    }
}

/***************************************************************************
PARAMS
    num_bits number of bit random number to be generated (16bit random number, 128bit random number etc)
    dest pointer to where number needs to be stored

*/
static void broadcasterRandomBitsGenerator(uint16 num_bits, uint16* dest)
{
    uint16 iterations = num_bits / 16;
    uint16 i;

    for (i=0; i<iterations; i++)
    {
        *dest = UtilRandom();
        dest++;
    }
}

static void broadcasterSecKeyGenerator(broadcast_encr_config_t* encr_config)
{
    broadcasterRandomBitsGenerator(128, &encr_config->seckey[1]);

    /* only using private/AES keys at the moment, just set that type */
   encr_config->seckey[0] = (BSSR_SECURITY_ENCRYPT_TYPE_AESCCM |
            BSSR_SECURITY_KEY_TYPE_PRIVATE);

    /* store the key in PS */
    broadcasterConfigStoreKey(encr_config->seckey);
}

static broadcast_encr_config_t* broadcasterReadEncryptionConfig(void)
{
    broadcast_encr_config_t* encr_config = NULL;

    /* get memory for encryption configuration */
    encr_config = PanicUnlessMalloc(sizeof(broadcast_encr_config_t));
    memset( encr_config, 0, sizeof(broadcast_encr_config_t) );

    /* Load security key (if already associated), else generate one */
    if (!broadcasterConfigLoadKey(encr_config->seckey))
    {
        /* No security key, not already associated hence generate one */
        broadcasterSecKeyGenerator(encr_config);
    }

    /* Load Fixed IV */
    if (!broadcasterConfigLoadFixedIv(encr_config->fixed_iv))
    {
        /* must have a FixedIV in PS */
        Panic();
    }

    return encr_config;
}

void setupEncryption(void)
{
    broadcast_encr_config_t* encr_config = broadcasterReadEncryptionConfig();

    /* update the security key in our GATT Broadcast Service server */
    sinkGattBAServerUpdateSecKey(encr_config->seckey,
                                        sizeof(encr_config->seckey)/sizeof(uint16));

    /* Generate the new Variant IV */
    broadcasterRandomBitsGenerator(16, &encr_config->variant_iv);

    /* Update broadcast context library with encryption information, to be used by the plugin */
    BroadcastContextSetEncryptionConfig(encr_config);

    free(encr_config);
}

#endif

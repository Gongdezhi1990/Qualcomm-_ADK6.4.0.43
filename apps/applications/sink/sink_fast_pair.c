/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_fast_pair.c

DESCRIPTION
    Interface with the fast pair library 
*/

#include <stdlib.h>
#ifdef ENABLE_FAST_PAIR
#include <connection.h>
#include <util.h>
#include <byte_utils.h>
#include <config_store.h>
#ifdef HYDRACORE
#include <rsa_pss_constants.h>
#include <panic.h>
#endif

/* Application headers */
#include "sink_main_task.h"
#include "sink_debug.h"
#include "sink_config.h"
#include "sink_configmanager.h"
#include "sink_statemanager.h"
#include "sink_development.h"
#include "sink_utils.h"
#include "sink_events.h"
#include "sink_ble_gap.h"
#include "sink_ble.h"
#include "sink_slc.h"
#include "sink_scan.h"

#include "sink_fast_pair.h"
#include "sink_fast_pair_config_def.h"
#include "sink_ble_advertising.h"
#include "sink_gatt_server_fps.h"

#ifdef DEBUG_FAST_PAIR
#define FAST_PAIR_INFO(x) DEBUG(x)
#define FAST_PAIR_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else
#define FAST_PAIR_INFO(x)
#define FAST_PAIR_ERROR(x)
#endif

/* FP state timeout in seconds*/
#define FAST_PAIR_STATE_TIMEOUT         (10)
/* FP Seeker triggered discoverability timeout in seconds */
#define FAST_PAIR_DISCOVERABILITY_TIMEOUT    (10)
/* Quarantine timeout in seconds*/
#define FAST_PAIR_QUARANTINE_TIMEOUT         (300)
/* Failure attempts in seconds*/
#define FAST_PAIR_MAX_FAIL_ATTEMPTS            (10)
#define ACCOUNT_KEY_START_OCTET                    (0x04)

/* Global Fast Pair Data instance */
sink_fast_pair_data_t    g_fast_pair_data;

/****************************************************************************
NAME
    sinkFastPairResetFailCount

DESCRIPTION
    Reset the failure counter
*/
void sinkFastPairResetFailCount(void)
{
    g_fast_pair_data.failure_count = 0;

    /* Whenever this function is getting called on the expiry of the timer or 
        during Power Off*/
    if(sinkFastPairGetFastPairState() == fast_pair_state_fail_quarantine)
    {
        /* Its time to come out of quarantine state as the 
        failure count is reset.*/
        sinkFastPairSetFastPairState(fast_pair_state_idle);
    }
}

#ifdef HYDRACORE
static void sinkFastPairUnscramblePrivateKey(const uint16* scrambled_aspk, const uint16* seed_data, uint8 * dest)
{  
    if(scrambled_aspk && seed_data && dest)
    {
        uint16 i,j;
        uint16* unscrambled_aspk = PanicUnlessMalloc(FAST_PAIR_PRIVATE_KEY_LEN);

        for(i=0, j=FAST_PAIR_PRIVATE_KEY_LEN/2; i <(FAST_PAIR_PRIVATE_KEY_LEN/2) && j > 0; i++,j--)
        {
            /* Use last 16 words of M array in rsa_decrypt_constant_mod structure */
            unscrambled_aspk[i] = scrambled_aspk[i] ^ rsa_decrypt_constant_mod.M[RSA_SIGNATURE_SIZE - j] ^ seed_data[i];
        }
        ByteUtilsMemCpyUnpackString(&dest[0], (const uint16 *)&unscrambled_aspk[0], FAST_PAIR_PRIVATE_KEY_LEN);
        free(unscrambled_aspk);
    }
    else
    {
        FAST_PAIR_ERROR(("Invalid scrambled ASPK, seed or destination buffer\n"));
        Panic();
    }
}
#endif

/*******************************************************************************
NAME
    sinkFastPairGetAntiSpoofingPrivateKey

DESCRIPTION
    Get anti spoofing private key when kbp request is recieved.

PARAMETERS
    None

RETURNS
    True if private key is obtained
*/
static bool sinkFastPairGetAntiSpoofingPrivateKey(void)
{
    uint16 size = 0;

#ifdef HYDRACORE
    uint16 seed_size = 0;
    fast_pair_readonly_seed_config_def_t *seed_data = NULL;
#endif

    bool status = FALSE;
    fast_pair_readonly_private_key_config_def_t *data = NULL;

    uint8* private_key = PanicUnlessMalloc(FAST_PAIR_PRIVATE_KEY_LEN);

    size = configManagerGetReadOnlyConfig(FAST_PAIR_READONLY_PRIVATE_KEY_CONFIG_BLK_ID, (const void **)&data);

#ifdef HYDRACORE
    seed_size = configManagerGetReadOnlyConfig(FAST_PAIR_READONLY_SEED_CONFIG_BLK_ID, (const void **)&seed_data);
#endif

    if ((size * sizeof(uint16) == sizeof(fast_pair_readonly_private_key_config_def_t)) 
#ifdef HYDRACORE
    &&
          (seed_size * sizeof(uint16) == sizeof(fast_pair_readonly_seed_config_def_t))
#endif
)
    {
#ifdef HYDRACORE
        sinkFastPairUnscramblePrivateKey((const uint16 *)&data->private_key[0], (const uint16 *)&seed_data->seed[0], &private_key[0]);
#else
        ByteUtilsMemCpyUnpackString(&private_key[0], (const uint16 *)&data->private_key[0], FAST_PAIR_PRIVATE_KEY_LEN);
#endif
        status = TRUE;
    }
    else
    {
        FAST_PAIR_ERROR(("Fast Pair private key size invalid!\n"));
    }

    configManagerReleaseConfig(FAST_PAIR_READONLY_SEED_CONFIG_BLK_ID);
    configManagerReleaseConfig(FAST_PAIR_READONLY_PRIVATE_KEY_CONFIG_BLK_ID);

    g_fast_pair_data.private_key = (uint16 *)private_key;
    return status;
}

/*******************************************************************************
NAME
    sinkFastPairRetrieveAccountKeys

DESCRIPTION
    Retrieve Account key from Persistant Store.

PARAMETERS
    None
    
RETURNS
    None
*/
static void sinkFastPairRetrieveAccountKeys(void)
{
    uint16 buffer_size, count;
    buffer_size = ConfigRetrieve(CONFIG_FAST_PAIR_ACCOUNT_KEY_INDEX, g_fast_pair_data.account_key.key_index, MAX_FAST_PAIR_ACCOUNT_KEYS*sizeof(uint16));

    if(buffer_size != MAX_FAST_PAIR_ACCOUNT_KEYS)
    {
        FAST_PAIR_ERROR(("Fast Pair Account key index could not be retrived from PS as only %d words stored!\n", buffer_size));
    }

    /* Validate account keys stored in Account key Index. 
       Check if account key Index values are less than  MAX_FAST_PAIR_ACCOUNT_KEYS
       Account keys when read from PS store will be validated by checking first octect is 0x04 
     */
    for(count=0;count<MAX_FAST_PAIR_ACCOUNT_KEYS;count++)
    {
        if(g_fast_pair_data.account_key.key_index[count] < MAX_FAST_PAIR_ACCOUNT_KEYS)
        {
            g_fast_pair_data.account_key.num_keys++;
        }
        else
        {
            /* No more valid account keys stored in PS */ 
            break;
        }
    }

    buffer_size = ConfigRetrieve(CONFIG_FAST_PAIR_ACCOUNT_KEYS, g_fast_pair_data.account_key.keys, (MAX_FAST_PAIR_ACCOUNT_KEYS * FAST_PAIR_ACCOUNT_KEY_LEN));

    if(buffer_size != (MAX_FAST_PAIR_ACCOUNT_KEYS * FAST_PAIR_ACCOUNT_KEY_LEN/2))
    {
        FAST_PAIR_ERROR(("Fast Pair Account keys could not be stored in PS as only %d words stored!\n", buffer_size));
    }
}
/*******************************************************************************
NAME
    sinkFastPairStoreAccountKeys

DESCRIPTION
    Store Account key provided by Fast Pair Seeker to PS.

PARAMETERS
    pointer to decrypted account key data

RETURNS
    None
*/
static void sinkFastPairStoreAccountKeys(uint8* decrypted_data)
{
    uint16 buffer_size;
    
    uint16 count; 
    uint16 temp;
    uint16 duplicate_account_key_index = 0xFF; /* Invalid Index as AKI varies from 0 to (MAX_FAST_PAIR_ACCOUNT_KEYS-1) */

    FAST_PAIR_INFO(("FAST_PAIR_ACCOUNT_KEY_IND curr_key_cnt=%d\n", g_fast_pair_data.account_key.num_keys));

    /* Newly added account key will always have highest priority. 
       The account key list will also point from highest to lowest priority. 
     */
    for(count = 0; count < g_fast_pair_data.account_key.num_keys; count++)
    {
        if(g_fast_pair_data.account_key.key_index[count] < MAX_FAST_PAIR_ACCOUNT_KEYS)
        {
            if(memcmp(decrypted_data, &g_fast_pair_data.account_key.keys[g_fast_pair_data.account_key.key_index[count]][0], FAST_PAIR_ACCOUNT_KEY_LEN) == 0)
            {
                /* Found duplicate account key. Remove duplicate key by removing earlier written value */
                duplicate_account_key_index = count;
                break;
            }
        }
    }

    if(duplicate_account_key_index == 0xFF)
    {
        /* No duplicate account key found. Add to existing list.*/

        /* If list is not already full, add to the account key list */
        if(g_fast_pair_data.account_key.num_keys < MAX_FAST_PAIR_ACCOUNT_KEYS)
        {
            /* If the account key list is not full then only account key positions from 0 to g_fast_pair_data.account_key.num_keys -1 
               are utlized. The g_fast_pair_data.account_key.num_keys should be free to use. Store the new account key */
            memcpy(&g_fast_pair_data.account_key.keys[g_fast_pair_data.account_key.num_keys][0], decrypted_data, FAST_PAIR_ACCOUNT_KEY_LEN);

            /* Update account key Index */
            for(count=g_fast_pair_data.account_key.num_keys;count>0;count--)
            {
                g_fast_pair_data.account_key.key_index[count] = g_fast_pair_data.account_key.key_index[count-1];
            }
            g_fast_pair_data.account_key.key_index[0] = g_fast_pair_data.account_key.num_keys;

            /* Update number of account keys */
            g_fast_pair_data.account_key.num_keys++;
        }
        else if(g_fast_pair_data.account_key.num_keys == MAX_FAST_PAIR_ACCOUNT_KEYS)
        {
            /* Account key index will point to most recently used account key to least recently account key.
               Account key Index 0 will always point to the most recently used account key and account key Index
               and MAX_FAST_PAIR_ACCOUNT_KEYS-1 index will have least recently used location of account key
             */
            temp = g_fast_pair_data.account_key.key_index[MAX_FAST_PAIR_ACCOUNT_KEYS-1];

            /* Cache locally the account key */
            memcpy(&g_fast_pair_data.account_key.keys[temp][0], decrypted_data, FAST_PAIR_ACCOUNT_KEY_LEN);

            /* Update account key Index */
            for(count=g_fast_pair_data.account_key.num_keys;count>1;count--)
            {
                g_fast_pair_data.account_key.key_index[count-1] = g_fast_pair_data.account_key.key_index[count-2];
            }
            g_fast_pair_data.account_key.key_index[0] = temp;

        }
        else
        {
            /* Should not reach here */
            FAST_PAIR_ERROR(("Fast Pair Account key number mismatch!\n"));
        }
    }
    else
    {
        /* Duplicate account key found. Remove that and update index 0 to duplicate key */

        if(duplicate_account_key_index != 0)
        {
            /* Account key index will point to most recently used account key to least recently account key.
               Account key Index 0 will always point to the most recently used account key and account key Index
               and MAX_FAST_PAIR_ACCOUNT_KEYS-1 index will have least recently used location of account key
             */
            temp = g_fast_pair_data.account_key.key_index[duplicate_account_key_index];

            /* Update account key Index */
            for(count=duplicate_account_key_index;count>0;count--)
            {
                g_fast_pair_data.account_key.key_index[count] = g_fast_pair_data.account_key.key_index[count-1];
            }
            g_fast_pair_data.account_key.key_index[0] = temp;
        }

    }
    
    /* Store Account key Index and Account keys to PS Store */
    buffer_size = ConfigStore(CONFIG_FAST_PAIR_ACCOUNT_KEY_INDEX, g_fast_pair_data.account_key.key_index, MAX_FAST_PAIR_ACCOUNT_KEYS*sizeof(uint16));

    if(buffer_size != (MAX_FAST_PAIR_ACCOUNT_KEYS))
    {
        FAST_PAIR_ERROR(("Fast Pair Account key index could not be stored in PS as only %d words stored!\n", buffer_size));
    }

    if(duplicate_account_key_index == 0xFF)
    {
        /* Update Account keys only if new key has been added */
        buffer_size = ConfigStore(CONFIG_FAST_PAIR_ACCOUNT_KEYS, g_fast_pair_data.account_key.keys, (MAX_FAST_PAIR_ACCOUNT_KEYS * FAST_PAIR_ACCOUNT_KEY_LEN));

        if(buffer_size != (MAX_FAST_PAIR_ACCOUNT_KEYS * FAST_PAIR_ACCOUNT_KEY_LEN/2))
        {
            FAST_PAIR_ERROR(("Fast Pair Account keys could not be stored in PS as only %d words stored!\n", buffer_size));
        }
    }
    
}

/*******************************************************************************
NAME
    sinkFastPairStoreEncryptedBlock

DESCRIPTION
    Store encrypted block provided by Fast Pair Seeker until AES key is derived.

PARAMETERS
    pointer to encrypted block

RETURNS
    None
*/
static void sinkFastPairStoreEncryptedBlock(uint8 *ptr)
{
    uint8* encrypted_data = PanicUnlessMalloc(FAST_PAIR_ENCRYPTED_REQUEST_LEN);

    g_fast_pair_data.encrypted_data = encrypted_data;
    g_fast_pair_data.encrypted_data_size = FAST_PAIR_ENCRYPTED_REQUEST_LEN;

    memmove(g_fast_pair_data.encrypted_data, ptr, g_fast_pair_data.encrypted_data_size);
}

/*******************************************************************************
NAME
    sinkFastPairStorePublicKey

DESCRIPTION
    Store public key provided by Fast Pair Seeker.

PARAMETERS
    pointer to public keys

RETURNS
    None
*/
static void sinkFastPairStorePublicKey(uint8 *ptr)
{
    uint8* public_key = PanicUnlessMalloc(FAST_PAIR_PUBLIC_KEY_LEN);

    g_fast_pair_data.public_key = (uint16 *)public_key;

    memmove(g_fast_pair_data.public_key, ptr, FAST_PAIR_PUBLIC_KEY_LEN);
}

/*******************************************************************************
NAME
    sinkFastPairCalculateSharedSecret

DESCRIPTION
    Calculate shared secret using public and private key on ECDH sec256p curve.

PARAMETERS
    pointer to public and private keys

RETURNS
    None
*/
static void sinkFastPairCalculateSharedSecret(uint16 *private_key, uint16 *public_key)
{
    ConnectionGenerateSharedSecretKey(sinkGetMainTask(), cl_crypto_ecc_p256, private_key, public_key);
}

/*******************************************************************************
NAME
    sinkFastPairConvertBigEndianBDAddress

DESCRIPTION
    Convertion function used to decode BD address from big endian coded data.

PARAMETERS
    pointer to bdaddr which needs to be updated
    pointer to decrypted data
    Offset from which decryptd data to be used in bytes

RETURNS
    None
*/
static void sinkFastPairConvertBigEndianBDAddress(bdaddr *device_addr, uint16 *decrypted_data, uint16 data_offset)
{
    uint8 *decrypted_packet_be = (uint8 *)decrypted_data;

    if(device_addr != NULL)
    {
         device_addr->nap = (uint16)(decrypted_packet_be[data_offset]<<8)| decrypted_packet_be[data_offset+1];
         device_addr->uap = (uint8)(decrypted_packet_be[data_offset+2]);
         device_addr->lap = (uint32)(decrypted_packet_be[data_offset+3] & 0xFF) << 16 | (uint32)(decrypted_packet_be[data_offset+4]) << 8 | (uint32)decrypted_packet_be[data_offset+5];
    }
}

/*******************************************************************************
NAME
    sinkFastPairMatchProviderAddress

DESCRIPTION
    Match provider address from decrypted key based pairing request.

PARAMETERS
    pointer to decrypted_data

RETURNS
    True if Provider address matches else False
*/
static bool sinkFastPairMatchProviderAddress(uint16 *decrypted_data)
{
    bool status = FALSE;  
    bdaddr provider_addr;

#ifdef ENABLE_RANDOM_ADDR
    typed_bdaddr *plocal_typed_addr = PanicUnlessMalloc(sizeof(typed_bdaddr));

    if(plocal_typed_addr != NULL)
    {
        sinkBleGapGetLocalAddr(sinkBleGapFindGapConnFlagIndexByCid(g_fast_pair_data.cid), plocal_typed_addr);
    }
#else
     bdaddr local_addr;

     /* Use Public address */
     sinkDataGetLocalBdAddress(&local_addr);
#endif

    sinkFastPairConvertBigEndianBDAddress(&provider_addr, decrypted_data, FAST_PAIR_PROVIDER_ADDRESS_OFFSET);

     FAST_PAIR_INFO(("Provider addr provided by FP Seeker %04x%02x%06lx\n", provider_addr.nap, provider_addr.uap, provider_addr.lap));
#ifdef ENABLE_RANDOM_ADDR
     FAST_PAIR_INFO(("Local BLE Address %04x%02x%06lx\n", plocal_typed_addr->addr.nap, plocal_typed_addr->addr.uap, plocal_typed_addr->addr.lap ));

     if (!BdaddrIsSame(&(plocal_typed_addr->addr), &provider_addr))
#else     
     if (!BdaddrIsSame(&local_addr, &provider_addr))
#endif
     {
         FAST_PAIR_ERROR(("Fast Pair provider addr mismatch!\n"));
     }
     else
     {  
        status = TRUE;
    }

#ifdef ENABLE_RANDOM_ADDR
    free(plocal_typed_addr);
#endif

     return status;
}

/*******************************************************************************
NAME
    sinkFastPairGenerateResponse

DESCRIPTION
    Generate key based pairing response.

PARAMETERS
    None

RETURNS
    pointer to raw response block
*/
static uint8* sinkFastPairGenerateResponse(void)
{
    uint8 *response = PanicUnlessMalloc(FAST_PAIR_ENCRYPTED_REQUEST_LEN);
    uint16 i;
    bdaddr local_addr;

    /* Check local addrss */
    sinkDataGetLocalBdAddress(&local_addr);
    
    response[0] = 0x01;
    response[1] = (local_addr.nap >> 8) & 0xFF;
    response[2] = local_addr.nap & 0xFF;
    response[3] = local_addr.uap;
    response[4] = (local_addr.lap >> 16) & 0xFF;
    response[5] = (local_addr.lap >> 8) & 0xFF;
    response[6] = local_addr.lap & 0xFF;
    for (i = 7; i < 16; i++)
    {
        response[i] = UtilRandom() & 0xFF;
    }

    return response;

}

/*******************************************************************************
NAME
    sinkFastPairPasskeyResponse

DESCRIPTION
    Generate provider passkey response.

PARAMETERS
    None

RETURNS
    pointer to raw passkey block
*/
static uint8* sinkFastPairPasskeyResponse(void)
{
    uint8 *response = PanicUnlessMalloc(FAST_PAIR_ENCRYPTED_PASSKEY_BLOCK_LEN);
    uint16 i;


    response[0] = fast_pair_passkey_provider;
    response[1] = (g_fast_pair_data.passkey.provider_passkey >> 16) & 0xFF;
    response[2] = (g_fast_pair_data.passkey.provider_passkey >> 8) & 0xFF;
    response[3] = g_fast_pair_data.passkey.provider_passkey & 0xFF;
    for (i = 4; i < 16; i++)
    {
        response[i] = UtilRandom() & 0xFF;
    }

    return response;
}

/*******************************************************************************
NAME
    checkSeekerPasskey

DESCRIPTION
    Matches seeker passkey with provider passkey and sends Accept or Reject Ind.

PARAMETERS
    passkey - Seeker encrypted passkey in decrypted format

RETURNS
    True or False
*/
static bool checkSeekerPasskey(uint32 passkey)
{
    if (g_fast_pair_data.passkey.provider_passkey != PASSKEY_INVALID)
    {
        MessageCancelAll(&theSink.task, EventUsrConfirmationReject);

        FAST_PAIR_INFO(("Fast Pair passkey received seeker=%06ld provider=%06ld\n", passkey, g_fast_pair_data.passkey.provider_passkey));

        MessageSend(&theSink.task, (g_fast_pair_data.passkey.provider_passkey == passkey) ? EventUsrConfirmationAccept : EventUsrConfirmationReject, NULL);
        return (g_fast_pair_data.passkey.provider_passkey == passkey) ? TRUE: FALSE;
    }

    return FALSE;
}

/*******************************************************************************
NAME
    sinkFastPairStartTimer

DESCRIPTION
    Starts a timer when the Fast Pair state apaprt from Idle is entered.

PARAMETERS
    None

RETURNS
    None
*/
static void sinkFastPairStartTimer(void)
{
    uint16 timeout_s = 0;
    timeout_s = (sinkFastPairGetFastPairState() == fast_pair_state_fail_quarantine)?FAST_PAIR_QUARANTINE_TIMEOUT:FAST_PAIR_STATE_TIMEOUT;

    FAST_PAIR_INFO(("sinkFastPairStartTimer timeout=[%u s]\n", timeout_s));

    /* Make sure any pending messages are cancelled */
    MessageCancelAll(sinkGetMainTask(), EventSysFastPairTimeout);

    /* Start Fast Pair timer */
    MessageSendLater(sinkGetMainTask(), EventSysFastPairTimeout, 0, D_SEC(timeout_s));
}

/*******************************************************************************
NAME
    sinkFastPairStartDiscoverabilityTimer

DESCRIPTION
    Starts a FP Seeker triggered discoverability timer on Kbp write request with discoverability bit set to 1

PARAMETERS
    None

RETURNS
    None
*/
static void sinkFastPairStartDiscoverabilityTimer(void)
{
    FAST_PAIR_INFO(("sinkFastPairStartDiscoverabilityTimer timeout=[%u s]\n", FAST_PAIR_DISCOVERABILITY_TIMEOUT));

    /* Make sure any pending messages are cancelled */
    MessageCancelAll(sinkGetMainTask(), EventSysFastPairDiscoverabilityTimeout);

    /* Start FP Seeker triggered discoverability timer */
    MessageSendLater(sinkGetMainTask(), EventSysFastPairDiscoverabilityTimeout, 0, D_SEC(FAST_PAIR_DISCOVERABILITY_TIMEOUT));
}

/*******************************************************************************
NAME
    sinkFastPairStopDiscoverabilityTimer

DESCRIPTION
    Stops a FP Seeker triggered discoverability timer 

PARAMETERS
    None

RETURNS
    None
*/
void sinkFastPairStopDiscoverabilityTimer(void)
{
    FAST_PAIR_INFO(("sinkFastPairStopDiscoverabilityTimer"));

    /* Make sure any pending messages are cancelled */
    MessageCancelAll(sinkGetMainTask(), EventSysFastPairDiscoverabilityTimeout);
}

/*******************************************************************************
NAME
    sinkFastPairStopTimer

DESCRIPTION
    Stops a timer when the fast pair state changes.

PARAMETERS
    None

RETURNS
    None
*/
void sinkFastPairStopTimer(void)
{
    FAST_PAIR_INFO(("sinkFastPairStopTimer"));

    /* End Fast Pairing timer */
    MessageCancelAll(sinkGetMainTask(), EventSysFastPairTimeout);

    if(sinkFastPairGetFastPairState() == fast_pair_state_key_based_pairing_response)
    {
        FAST_PAIR_INFO(("Pairing Procedure Triggered\n"));
        FAST_PAIR_INFO(("sinkFastPairStopTimer: fast_pair_state_key_based_pairing_response ==>fast_pair_state_wait_passkey\n"));
        sinkFastPairSetFastPairState(fast_pair_state_wait_passkey);
        /*change the scan params back to regular when fp state is changed from fast_pair_state_key_based_pairing_response*/
        sinkDisableConnectable();
        sinkEnableConnectable();
    }
}


/*******************************************************************************
NAME
    sinkFastPairClearProcessedAccountKeysNum

DESCRIPTION
    Clear the account keys proccessed list.

PARAMETERS
    None

RETURNS
    None
*/
static void sinkFastPairClearProcessedAccountKeysNum(void)
{
    /* Free account keys list as no longer useful */
    g_fast_pair_data.account_key.num_keys_processed = 0;
}
/*******************************************************************************
NAME
    sinkFastPairIsInProgress

DESCRIPTION
    Checks if fast pairing is in progress. There should a valid AES key and 
    Fast pair SM should not be in quarantine or idle state

*/
bool sinkFastPairIsInProgress(void)
{

    if ((g_fast_pair_data.fp_state != fast_pair_state_idle) && (g_fast_pair_data.fp_state != fast_pair_state_fail_quarantine)
        && (g_fast_pair_data.aes_key != NULL))
        return TRUE;
    else
        return FALSE;
}

/*******************************************************************************
NAME
    sinkFastPairIsInQuarantine

DESCRIPTION
    Checks if fast pair state machine is in Quarantine.

*/
bool sinkFastPairIsInQuarantineState(void)
{

    if (g_fast_pair_data.fp_state == fast_pair_state_fail_quarantine)
        return TRUE;
    else
        return FALSE;
}

/*******************************************************************************
NAME
    sinkFastPairGetStatus

DESCRIPTION
    Checks if fast pair is in successful or not.

*/
bool sinkFastPairGetStatus(void)
{

    if (g_fast_pair_data.fp_status == fast_pair_success)
        return TRUE;
    else
        return FALSE;
}

/*******************************************************************************
NAME
    sinkFastPairSetStatus

DESCRIPTION
    Sets the status of fast pair.

*/
void sinkFastPairSetStatus(fast_pair_status_t fp_status)
{
    g_fast_pair_data.fp_status= fp_status;
}


/*******************************************************************************
NAME
    sinkFastPairHandleProviderPasskey

DESCRIPTION
    Save provider passkey when User confirmation Indication is invoked.

*/
void sinkFastPairHandleProviderPasskey(uint32 passkey)
{
    FAST_PAIR_INFO(("FPS: Provider Passkey Provided\n"));
    if (sinkFastPairIsInProgress())
    {
        /* Wait for passkey */
        g_fast_pair_data.passkey.provider_passkey = passkey;

        FAST_PAIR_INFO(("Provider passkey=%06ld\n", g_fast_pair_data.passkey.provider_passkey));
    }
    else
        g_fast_pair_data.passkey.provider_passkey = PASSKEY_INVALID;
}

/*******************************************************************************
NAME
    sinkFastPairDisconnectLELink

DESCRIPTION
    Disconnects LE link and resets fastpair state back to idle.

*/
void sinkFastPairDisconnectLELink(void)
{
    FAST_PAIR_INFO(("sinkFastPairDisconnectLELink: Fastpair State=%d\n", g_fast_pair_data.fp_state));
    FAST_PAIR_INFO(("sinkFastPairDisconnectLELink: Fastpair cid=%d\n", g_fast_pair_data.cid));

    if(g_fast_pair_data.cid != INVALID_CID)
    {
        FAST_PAIR_INFO(("sinkFastPairDisconnectLELink: Entered\n"));
        GattManagerDisconnectRequest(g_fast_pair_data.cid);
        sinkFastPairSetFastPairState(fast_pair_state_idle);
    }
}

/*******************************************************************************
NAME
    sinkFastPairGetFastPairState

DESCRIPTION
    Gets new fast pair state.

*/
fast_pair_state_t sinkFastPairGetFastPairState(void)
{
    return g_fast_pair_data.fp_state;
}

/*************************************************************************
NAME
    SinkFastPairDeInit

DESCRIPTION
    De-Initialize memory for fast pair global structure
*/
void SinkFastPairDeInit(void)
{
    uint16 fail_count = 0;

    FAST_PAIR_INFO(("SinkFastPairDeInit: Clearing of the cached info from g_fast_pair_data\n"));

    /*ensure to revert back to standard scan*/
    if(sinkIsConnectable())
    {
        /*change the scan params back to normal*/
        sinkDisableConnectable();
        sinkEnableConnectable();
    }

    /* Free allocated Memory */
    if(g_fast_pair_data.private_key)
    {
        free(g_fast_pair_data.private_key);
        g_fast_pair_data.private_key = NULL;
    }

    if(g_fast_pair_data.public_key)
    {
        free(g_fast_pair_data.public_key);
         g_fast_pair_data.public_key = NULL;
    }

    if(g_fast_pair_data.aes_key)
    {
        free(g_fast_pair_data.aes_key);
        g_fast_pair_data.aes_key = NULL;
    }

    if(g_fast_pair_data.encrypted_data)
    {
        free(g_fast_pair_data.encrypted_data);
        g_fast_pair_data.encrypted_data = NULL;
    }
     
    /* Retain the failure count */
    fail_count = g_fast_pair_data.failure_count;
    memset(&g_fast_pair_data.kbp_req, 0, sizeof(g_fast_pair_data.kbp_req));

    g_fast_pair_data.fast_pair_server = NULL;
    g_fast_pair_data.cid = INVALID_CID;
    g_fast_pair_data.encrypted_data_size = 0;
    g_fast_pair_data.fp_status = fast_pair_success;
    g_fast_pair_data.passkey.passkey_match = FALSE;
    g_fast_pair_data.passkey.provider_passkey = 0;
    g_fast_pair_data.passkey.seeker_passkey = 0;

    /* Retain the failure count */
    g_fast_pair_data.failure_count = fail_count;

    /*If the counter reaches the max no failure attempts move to quarntine for 5 min*/
    if(g_fast_pair_data.failure_count >= FAST_PAIR_MAX_FAIL_ATTEMPTS)
    {
        /* Reset the counter as the procedure seems to be successful*/
        sinkFastPairResetFailCount();
        sinkFastPairSetFastPairState(fast_pair_state_fail_quarantine);
    }

}


/*******************************************************************************
NAME
    sinkFastPairSetFastPairState

DESCRIPTION
    Sets new fast pair state. Reset fast pair data if state moves back to Idle

*/
void sinkFastPairSetFastPairState(fast_pair_state_t new_state)
{
    bool start_timer = FALSE;
    
    FAST_PAIR_INFO(("sinkFastPairSetFastPairState: %d\n",new_state));

    g_fast_pair_data.fp_state = new_state;

    switch(new_state)
    {
        case fast_pair_state_idle:
            sinkFastPairStopTimer();
            SinkFastPairDeInit();
            break;
        case fast_pair_state_wait_aes_key:
            /* No Action on timers required in this state*/
            break;
        case fast_pair_state_key_based_pairing_response:
            /*The timer here is started to meet the expectations
            from the Fast Pair specification as mentioned below
            1. STEP 4: If pairing has not been started after marking K as usable for decrypting Passkey writes.
            2. STEP 5 : If the decrypted request’s Flags bit requesting discoverability is set to 1.
            3. STEP 8:If the Request’s Flags byte has bit 1 set to 1,the Seeker is requesting the Provider to 
                initiate bonding to the Seeker’s BR/EDR address
            4. STEP 9: f the Request’s Flags byte has bit 1 set to 0 Wait up to 10 seconds for a pairing request.*/
            start_timer = TRUE;
            break;
        case fast_pair_state_wait_passkey:
            /*The timer here is started to meet the expectations
            from the Fast Pair specification as mentioned below
            1. STEP 11:When confirmation of the passkey is needed, wait up to 10 seconds for a write to the 
                Passkey characteristic.*/
            start_timer = TRUE;
            break;
        case fast_pair_state_passkey_response:
            /* No Action on timers required in this state*/
            break;
        case fast_pair_state_wait_account_key:
            /*The timer here is started to meet the expectations
            from the Fast Pair specification as mentioned below
            1. STEP 18:If the pairing succeeds, then mark K as usable for decrypting Account Key writes on 
                this LE link, but not for any subsequent Passkey writes nor any writes on any other link*/
            start_timer = TRUE;
            break;
         case fast_pair_state_fail_quarantine:
            /* Error handling as per Fastpair Specfication
            step 3:If no key could successfully decrypt the value, ignore the write and exit.
            Keep a count of these failures. When the failure count hits 10, fail all new requests
            immediately. Reset the failure count after 5 minutes, after power on, or after a success*/
            start_timer = TRUE;
            break;
        default:
            break;
    }

    /* Start the timer if needed*/
    if(start_timer == TRUE)
    {
        sinkFastPairStartTimer();
    }
}

/*******************************************************************************
NAME
    sinkFastPairKeybasedPairingWrite

DESCRIPTION
    Invoked when new key based pairing write request is recieved. Fetches private key
    stores encrypted data block, public key until shared secret is calculated.

*/
void sinkFastPairKeybasedPairingWrite(const GFPS* fast_pair_server, uint16 cid, uint8* enc_data, uint16 size)
{
    /* Store Encrytped Write request */
    g_fast_pair_data.fast_pair_server = fast_pair_server;
    g_fast_pair_data.cid = cid;

    sinkFastPairSetFastPairState(fast_pair_state_wait_aes_key);

    /* If Fast pair seeker has written public key along with encrypted request,
       use the public private key ECDH algorithm to arrive at the AES key
     */
    if(size == (FAST_PAIR_ENCRYPTED_REQUEST_LEN+FAST_PAIR_PUBLIC_KEY_LEN))
    {      
        /* Proceed with Public/Private key based fast pair procedure only if we are in discoverable mode */
        if(stateManagerGetState() == deviceConnDiscoverable)
        {
            /* Proceed only if valid private key is available else exit the procedure */
            if(sinkFastPairGetAntiSpoofingPrivateKey())
            {
                sinkFastPairStorePublicKey(&(enc_data[FAST_PAIR_ENCRYPTED_REQUEST_LEN]));
                
                /* Initiate calculation of shared secret Key */
                sinkFastPairCalculateSharedSecret(g_fast_pair_data.private_key, g_fast_pair_data.public_key);

                /* Store encrypted request data for later use */
                sinkFastPairStoreEncryptedBlock(enc_data);
            }
            else
            {
                sinkFastPairSetFastPairState(fast_pair_state_idle);
            }
        }
        else
        {
            sinkFastPairSetFastPairState(fast_pair_state_idle);
        }
    }
    else if(size == FAST_PAIR_ENCRYPTED_REQUEST_LEN)
    {
       /* If only encrypted request is written to KbP, then try to find an account key
          from account key list which can function as AES key to decode encrypted packets
        */
        
        /* Store encrypted kbp data for later use if decoding with first account key fails */
        sinkFastPairStoreEncryptedBlock(enc_data);

        if (g_fast_pair_data.account_key.num_keys)
        {
            g_fast_pair_data.account_key.num_keys_processed = 0;

            /* Use account keys to decrypt Kbp packet */
            ConnectionDecryptBlockAes(sinkGetMainTask(), (uint16 *)g_fast_pair_data.encrypted_data, (uint16 *)&g_fast_pair_data.account_key.keys[g_fast_pair_data.account_key.num_keys_processed][0]);
            
            g_fast_pair_data.account_key.num_keys_processed++;

        }
        else
        {
            /* Sufficient information not available to decode encrypted request. 
               Ignore the write and exit 
             */
            sinkFastPairSetFastPairState(fast_pair_state_idle);
        }
        
    }
    else
    {
        /* The counter is incremented here to adhere to failure 
            handling mechanism as per FastPair specification*/
        g_fast_pair_data.failure_count++;

        /* Invalid KbP Write. Ignore and exit */
        sinkFastPairSetFastPairState(fast_pair_state_idle);
        
    }     
    
}

/*******************************************************************************
NAME
    sinkFastPairPasskeyWrite

DESCRIPTION
    Invoked when new passkey write request is recieved. .

*/
void sinkFastPairPasskeyWrite(const GFPS* fast_pair_server, uint16 cid, uint8* enc_data)
{
    UNUSED(fast_pair_server);

    if(g_fast_pair_data.fp_state == fast_pair_state_wait_passkey)
    {

        /* Check the passkey write came from same LE link which initiated KbP write) */
        
        if(g_fast_pair_data.cid == cid)
        {
        /* Kill the fast pair timer if its already running which would have started
            to  adhere to STEP 11 from Fastpair specification*/
            sinkFastPairStopTimer();
            /* Decrypt passkey Block */
            ConnectionDecryptBlockAes(sinkGetMainTask(), (uint16 *)enc_data, (uint16 *)g_fast_pair_data.aes_key);
        }
    }
}

/*******************************************************************************
NAME
    sinkFastPairAccountKeyWrite

DESCRIPTION
    Invoked when new account key write request is recieved. .

*/
void sinkFastPairAccountKeyWrite(const GFPS* fast_pair_server, uint16 cid, uint8* enc_data)
{    
    UNUSED(fast_pair_server);

    if(g_fast_pair_data.fp_state == fast_pair_state_wait_account_key)
    {
        /* Check the account key write came from same LE link which initiated KbP write) */
        if(g_fast_pair_data.cid == cid)
        {
        /* Kill the fast pair timer if its already running which would have started
            to  adhere to STEP 11 from Fastpair specification*/
            sinkFastPairStopTimer();
            /* Decrypt account key block */
            ConnectionDecryptBlockAes(sinkGetMainTask(), (uint16 *)enc_data, (uint16 *)g_fast_pair_data.aes_key);
        }
    }
}

/*******************************************************************************
NAME
    sinkFastPairHandleMessages

DESCRIPTION
    Message handler for sink fast pair

*/
void sinkFastPairHandleMessages(Task task, MessageId id, Message message)
{
    UNUSED(task);

    switch(id)
    {
        case CL_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM:
        {
            CL_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM_T *cfm = (CL_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM_T *)message;

            if(cfm->status == success)
            {
                ConnectionEncryptBlockSha256(sinkGetMainTask(), cfm->shared_secret_key, (CL_CRYPTO_SHA_DATA_LEN*2));
            }
            else
            {
                FAST_PAIR_ERROR(("Shared Secret Key calcuation failed!\n", id));
            }
        }
        break;

        case CL_CRYPTO_HASH_CFM:
        {
            CL_CRYPTO_HASH_CFM_T *cfm = (CL_CRYPTO_HASH_CFM_T *)message;
            
            if(cfm->status == success)
            {
                if(g_fast_pair_data.fp_state == fast_pair_state_wait_aes_key)
                {
                    /* Store AES Key */
                    g_fast_pair_data.aes_key = PanicUnlessMalloc(AES_BLOCK_SIZE);
                    memcpy(g_fast_pair_data.aes_key, cfm->hash, AES_BLOCK_SIZE);

                    /*Fetch Encrypted Block */
                    ConnectionDecryptBlockAes(sinkGetMainTask(), (uint16 *)g_fast_pair_data.encrypted_data, (uint16 *)cfm->hash);

                }
                else
                {
                    /* Handle Account Key filter generation */
                    bleFastPairHandleAccountKeys(cfm);
                }
            }
            else
            {
                FAST_PAIR_ERROR(("Hash calcuation failed!\n", id));
            }
        }
        break;

        case CL_CRYPTO_ENCRYPT_CFM:
        {
             CL_CRYPTO_ENCRYPT_CFM_T *cfm = (CL_CRYPTO_ENCRYPT_CFM_T *)message;

            if(cfm->status == success)
            {
                if(g_fast_pair_data.fp_state == fast_pair_state_key_based_pairing_response)
                {
                    /* Send Notification */
                    sinkBleSendFastPairNotification(UUID_KEYBASED_PAIRING, 
                            g_fast_pair_data.fast_pair_server, g_fast_pair_data.cid,
                            (uint8 *)cfm->encrypted_data);  

                    /*change the scan params*/
                    sinkDisableConnectable();
                    sinkEnableConnectable();
                  
                }
                else if(g_fast_pair_data.fp_state == fast_pair_state_passkey_response)
                {

                    /* Send Encrypted Passkey */
                    sinkBleSendFastPairNotification(UUID_PASSKEY, 
                            g_fast_pair_data.fast_pair_server, g_fast_pair_data.cid,
                            (uint8 *)cfm->encrypted_data); 
                    if(g_fast_pair_data.passkey.passkey_match == TRUE)
                    {
                        /* Go to account key write state */
                        sinkFastPairSetFastPairState(fast_pair_state_wait_account_key);

                        g_fast_pair_data.passkey.passkey_match = FALSE;
                    }
                    else
                    {
                        /* Go to idle state if failed to match passkey */
                        sinkFastPairSetFastPairState(fast_pair_state_idle);
                    }
                }

            }
            else
            {
                FAST_PAIR_ERROR(("AES Encrypt failed!\n", id));
            }  
        }
        break;

        case CL_CRYPTO_DECRYPT_CFM:
        {
            CL_CRYPTO_DECRYPT_CFM_T *cfm = (CL_CRYPTO_DECRYPT_CFM_T *)message;

            if(cfm->status == success)
            {
                if(g_fast_pair_data.fp_state == fast_pair_state_wait_aes_key)
                {
                    if(sinkFastPairMatchProviderAddress(cfm->decrypted_data))
                    {
                        uint8* raw_response = sinkFastPairGenerateResponse();
                        uint8* decrypted_data_be = (uint8 *)cfm->decrypted_data;

                        /* Reset the counter as the decrypt procedure seems to be successful*/
                        sinkFastPairResetFailCount();

                        /* Declare AES key valid when match is found using account key as AES key */
                        if(g_fast_pair_data.public_key == NULL)
                        {
                            /* Store AES Key */
                            g_fast_pair_data.aes_key = PanicUnlessMalloc(AES_BLOCK_SIZE);
                            memcpy(g_fast_pair_data.aes_key, g_fast_pair_data.account_key.keys[g_fast_pair_data.account_key.num_keys_processed-1], AES_BLOCK_SIZE);

                            sinkFastPairClearProcessedAccountKeysNum();
                        }

                        sinkFastPairSetFastPairState(fast_pair_state_key_based_pairing_response);

                        g_fast_pair_data.kbp_req.request_discoverability = (decrypted_data_be[1] & 0x1) ? TRUE : FALSE;
                         
                        g_fast_pair_data.kbp_req.request_bonding = (decrypted_data_be[1] & 0x2) ? TRUE : FALSE;

                        sinkFastPairConvertBigEndianBDAddress(&g_fast_pair_data.kbp_req.provider_addr, cfm->decrypted_data, FAST_PAIR_PROVIDER_ADDRESS_OFFSET);

                        /* If seeker request provider to be discoverable, become dicoverable for 10 seconds
                           Discoverablity timeout handled with fast pair state key based pairing response timer
                         */
                        if(g_fast_pair_data.kbp_req.request_discoverability)
                        {
                            sinkInquirySetInquirySession(inquiry_session_normal);
                            if(stateManagerGetState() != deviceConnDiscoverable)
                            {
                                stateManagerEnterConnDiscoverableState(TRUE);
                                sinkFastPairSetDiscoverabilityStatus(fp_seeker_triggered_discoverability);
                                sinkFastPairStartDiscoverabilityTimer();
                            }
                        }

                        if (g_fast_pair_data.kbp_req.request_bonding)
                        {
                            sinkFastPairConvertBigEndianBDAddress(&g_fast_pair_data.kbp_req.seeker_addr, cfm->decrypted_data, FAST_PAIR_SEEKER_ADDRESS_OFFSET);
                            
                            FAST_PAIR_INFO(("Send SecurityRequest to peer_addr %04x%02x%06lx\n", g_fast_pair_data.kbp_req.seeker_addr.nap, g_fast_pair_data.kbp_req.seeker_addr.uap, g_fast_pair_data.kbp_req.seeker_addr.lap));
                            
                            /* Send security request to seeker as seeker asked provider to send pairing request */
                            gapStartEncryption(FALSE, g_fast_pair_data.cid);

                        }
                        else
                        {
                            memset(&g_fast_pair_data.kbp_req.seeker_addr, 0, sizeof(bdaddr));
                        }

                        /* Encrypt Raw Response with AES Key */
                        ConnectionEncryptBlockAes(sinkGetMainTask(), (uint16 *)raw_response, (uint16 *)g_fast_pair_data.aes_key);

                        free(raw_response);                      

                    }
                    else
                    {
                        /* If account key pairing is in progress check with another account to decrypt KbP packet */
                        if(g_fast_pair_data.public_key == NULL)
                        {
                            if(g_fast_pair_data.account_key.num_keys_processed < g_fast_pair_data.account_key.num_keys)
                            {
                                /* Use next account key to decrypt KbP packet */
                                ConnectionDecryptBlockAes(sinkGetMainTask(), (uint16 *)g_fast_pair_data.encrypted_data, (uint16 *)&g_fast_pair_data.account_key.keys[g_fast_pair_data.account_key.num_keys_processed][0]);
                                g_fast_pair_data.account_key.num_keys_processed++;
                            }
                            else
                            {
                                sinkFastPairClearProcessedAccountKeysNum();
                                /* The counter is incremented here to adhere to failure 
                                handling mechanism as per FastPair specification*/
                                g_fast_pair_data.failure_count++;
                                /* No Valid AES Key!. Free it and set fast Pair state to state to Idle */
                                sinkFastPairSetFastPairState(fast_pair_state_idle);
                            }
                        }
                        else
                        {
                            /* The counter is incremented here to adhere to failure 
                            handling mechanism as per FastPair specification*/
                            g_fast_pair_data.failure_count++;
                            /* AES Key Not Valid!. Free it and set fast Pair state to state to Idle */
                            sinkFastPairSetFastPairState(fast_pair_state_idle);
                        }
                    }
                }
                else if(g_fast_pair_data.fp_state == fast_pair_state_wait_passkey)
                {
                    uint8* decrypted_data_be = (uint8 *)cfm->decrypted_data;

                    uint8* passkey_response =  sinkFastPairPasskeyResponse();
                    
                    if (decrypted_data_be[0] != fast_pair_passkey_seeker)
                    {
                        /* We failed to decrypt passkey */
                        FAST_PAIR_ERROR(("Failed to decrypt passkey!\n"));

                        /* AES Key Not Valid!. Free it and set fast Pair state to state to Idle */
                        sinkFastPairSetFastPairState(fast_pair_state_idle);

                    }
                    else
                    {
                        g_fast_pair_data.passkey.seeker_passkey = ((uint32)decrypted_data_be[1] << 16 ) |
                                                  ((uint32)decrypted_data_be[2] << 8 ) |
                                                  (uint32)decrypted_data_be[3];
                    }

                    if(checkSeekerPasskey(g_fast_pair_data.passkey.seeker_passkey))
                    {
                        /* Pairing was successful. If Pairing was done using account key pairing, update the account key table */
                        if(g_fast_pair_data.public_key == NULL)
                        {
                            /* The account key will be present in account key list already, De-duplication logic 
                               will take care of updating list to track most recently used account keys 
                             */
                            sinkFastPairStoreAccountKeys((uint8 *)g_fast_pair_data.aes_key);
                        }
                        /* Encrypt Raw Passkey Response with AES Key */
                        ConnectionEncryptBlockAes(sinkGetMainTask(), (uint16 *)passkey_response, (uint16 *)g_fast_pair_data.aes_key);

                        free(passkey_response);

                        g_fast_pair_data.passkey.passkey_match = TRUE;
                        
                        sinkFastPairSetFastPairState(fast_pair_state_passkey_response);
                         
                    }
                    else
                    {
                        /* We failed to match passkey */
                        FAST_PAIR_ERROR(("Failed to match passkey!\n"));
                        
                        /* Encrypt Raw Passkey Response with AES Key */
                        ConnectionEncryptBlockAes(sinkGetMainTask(), (uint16 *)passkey_response, (uint16 *)g_fast_pair_data.aes_key);  

                        free(passkey_response);

                        g_fast_pair_data.passkey.passkey_match = FALSE;

                        sinkFastPairSetFastPairState(fast_pair_state_passkey_response);
                    }

                }
                else if(g_fast_pair_data.fp_state == fast_pair_state_wait_account_key)
                {
                    uint8 decrypted_data_be = cfm->decrypted_data[0];
                    if((decrypted_data_be&0xFF) == ACCOUNT_KEY_START_OCTET)
                    {
                        sinkFastPairStoreAccountKeys((uint8 *)cfm->decrypted_data);
                    }

                    /* since we are with fast pairing set BLE bonding state to non-bondable */
                    sinkBleSetGapBondState(ble_gap_bond_state_non_bondable);
                    /* Go to Fast pair Idle state for next conenction */
                    sinkFastPairSetFastPairState(fast_pair_state_idle);
                }
            }
            else
            {
                FAST_PAIR_ERROR(("AES Decrypt failed!\n", id));
            }  
        }
        break;
        
        default:
        {
            FAST_PAIR_ERROR(("handleFastPairCryptoMessage: unhandled (0x%04x)!\n", id));
        }
        break;
    }

}

/*************************************************************************
NAME
    SinkFastPairInit

DESCRIPTION
    Initialize Fast Pair by allocating memory for fast pair global structure
*/
void sinkFastPairInit(void)
{
    /* Initialise Fast Pair global structure */
    memset(&g_fast_pair_data, 0, sizeof(g_fast_pair_data));

    g_fast_pair_data.cid = INVALID_CID;
    g_fast_pair_data.passkey.passkey_match = FALSE;

    /* Set the handler */
    g_fast_pair_data.task.handler = sinkFastPairHandleMessages;

    g_fast_pair_data.account_key.keys = PanicUnlessMalloc(MAX_FAST_PAIR_ACCOUNT_KEYS * sizeof(g_fast_pair_data.account_key.keys[0]));

    memset(g_fast_pair_data.account_key.key_index, 0xFF, MAX_FAST_PAIR_ACCOUNT_KEYS*sizeof(uint16)); 
    sinkFastPairRetrieveAccountKeys(); 
}

/*************************************************************************
NAME
    sinkFastPairDeleteAccountKeys

DESCRIPTION
    Delete all the fast pair account keys from PS store
*/
void sinkFastPairDeleteAccountKeys(void)
{
    uint16 buffer_size;
    memset(g_fast_pair_data.account_key.key_index, 0xFF, MAX_FAST_PAIR_ACCOUNT_KEYS*sizeof(uint16)); 
    memset(g_fast_pair_data.account_key.keys, 0xFF, MAX_FAST_PAIR_ACCOUNT_KEYS*FAST_PAIR_ACCOUNT_KEY_LEN); 
    g_fast_pair_data.account_key.num_keys = 0;

    FAST_PAIR_INFO(("Try to delete Fast Pair Account Keys\n"));

    /* Store Account key Index and Account keys to PS Store */
    buffer_size = ConfigStore(CONFIG_FAST_PAIR_ACCOUNT_KEY_INDEX, g_fast_pair_data.account_key.key_index, (MAX_FAST_PAIR_ACCOUNT_KEYS * sizeof(uint16)));

    if(buffer_size != (MAX_FAST_PAIR_ACCOUNT_KEYS))
    {
        FAST_PAIR_ERROR(("Fast Pair Account key index could not be stored in PS as only %d words stored!\n", buffer_size));
    }

    buffer_size = ConfigStore(CONFIG_FAST_PAIR_ACCOUNT_KEYS, g_fast_pair_data.account_key.keys, (MAX_FAST_PAIR_ACCOUNT_KEYS * FAST_PAIR_ACCOUNT_KEY_LEN));

    if(buffer_size != (MAX_FAST_PAIR_ACCOUNT_KEYS * FAST_PAIR_ACCOUNT_KEY_LEN/2))
    {
        FAST_PAIR_ERROR(("Fast Pair Account keys could not be stored in PS as only %d words stored!\n", buffer_size));
    }

}

/****************************************************************************
NAME    
    sinkFastPairSetDiscoverabilityStatus

DESCRIPTION
    Function to set FP discoverability status
*/
void sinkFastPairSetDiscoverabilityStatus(fp_discoverability_status_t status)
{
    g_fast_pair_data.discoverability_status = status;
}

/****************************************************************************
NAME
    sinkFastPairHandleDiscoverability

DESCRIPTION
    Function to decide whether sink should go non-discoverable
    If sink discoverability is due to Kbp write request with discoverability set to 1 
    then on FP timer lapse, sink shall go non-discoverable
*/
void sinkFastPairHandleDiscoverability(void)
{
    if((stateManagerGetState() == deviceConnDiscoverable) && 
        (g_fast_pair_data.discoverability_status == fp_seeker_triggered_discoverability))
    {
        /* FP Seeker triggered discoverability timer is lapsed. now we should go non discoverable */
        stateManagerEnterConnectableState(FALSE);
        /* Update the discoverability status */
        g_fast_pair_data.discoverability_status = no_discoverability;
    }
}

#endif

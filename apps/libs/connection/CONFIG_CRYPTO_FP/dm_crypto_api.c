/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_crypto_api.c        

DESCRIPTION
    This file contains functions for sending message requests regarding cryptography functions on P1 for fastpair.
    
NOTES

*/
#include <vm.h>

#include "connection.h"
#include "connection_private.h"
#include "common.h"

#include "cryptoalgo.h"


#define SHA_DATA_OCTET_LEN ((sizeof(uint16) == 1) ? CL_CRYPTO_SHA_DATA_LEN * 2: CL_CRYPTO_SHA_DATA_LEN)

#ifndef UNUSED
#define UNUSED(x)       ((void)x)
#endif


/*******************************************************************************
 *                                  FUNCTIONS                                  *
 *******************************************************************************/
void ConnectionGeneratePublicPrivateKey(Task theAppTask, cl_ecc_type key_type)
{
    UNUSED(theAppTask);
    UNUSED(key_type);
}

void ConnectionGenerateSharedSecretKey(Task theAppTask, cl_ecc_type key_type, uint16 private_key[CL_CRYPTO_LOCAL_PVT_KEY_LEN], uint16 public_key[CL_CRYPTO_REMOTE_PUB_KEY_LEN])
{
    uint8 *secret = PanicUnlessMalloc(CL_CRYPTO_SECRET_KEY_LEN * sizeof(uint16));
    uint8 *key_public = PanicUnlessMalloc(CL_CRYPTO_REMOTE_PUB_KEY_LEN * sizeof(uint16));
    uint8 *key_private = PanicUnlessMalloc(CL_CRYPTO_LOCAL_PVT_KEY_LEN * sizeof(uint16));
    bool  valid_params =  TRUE;


    MAKE_CL_MESSAGE(CL_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM);

    message->key_type = key_type;    

    if(public_key)
    {
        memmove(key_public,  public_key, CL_CRYPTO_REMOTE_PUB_KEY_LEN * sizeof(uint16));
    }
    else
    {
        message->status = cl_ecdh_empty_public_key;    
        message->error = cl_crypto_error;  
        valid_params = FALSE;
    }
    
    if(private_key)
    {
        memmove(key_private, private_key, CL_CRYPTO_LOCAL_PVT_KEY_LEN * sizeof(uint16));
    }
    else
    {
        message->status = cl_ecdh_empty_private_key;   
        message->error = cl_crypto_error;          
        valid_params = FALSE;
    }

    if(valid_params)
    {
        if(!secp256r1_shared_secret(key_public, key_private, secret))
        {
            message->status = cl_crypto_error;    
        }
        else
        {
            message->status = cl_crypto_success;
            message->error = cl_no_error;  
            memmove(message->shared_secret_key, secret, CL_CRYPTO_SECRET_KEY_LEN * sizeof(uint16));
        }
    }

    free(secret);
    free(key_public);
    free(key_private);
   
    MessageSend(theAppTask, CL_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM, message);
}

void ConnectionEncryptBlockAes(Task theAppTask, uint16 data_array[CL_CRYPTO_AES_DATA_LEN], uint16 key_array[CL_CRYPTO_AES_KEY_LEN])
{
    uint8 *array_data = PanicUnlessMalloc(CL_CRYPTO_AES_DATA_LEN * sizeof(uint16));
    uint8 *array_key = PanicUnlessMalloc(CL_CRYPTO_AES_KEY_LEN * sizeof(uint16));
    uint8 *data_encrypted = PanicUnlessMalloc(CL_CRYPTO_AES_DATA_LEN * sizeof(uint16)); 
    bool  valid_params =  TRUE;

    MAKE_CL_MESSAGE(CL_CRYPTO_ENCRYPT_CFM);

    if(data_array)
    {
        memmove(array_data, data_array, CL_CRYPTO_AES_DATA_LEN * sizeof(uint16));
    }
    else
    {
        message->status = cl_crypto_error;
        message->error = cl_aes_empty_data_array;
        valid_params =  FALSE;
    }    
    if(key_array)
    {
        memmove(array_key, key_array, CL_CRYPTO_AES_KEY_LEN * sizeof(uint16));
    }
    else
    {
        message->status = cl_crypto_error;
        message->error = cl_aes_empty_key_array;
        valid_params =  FALSE;
    }
    
    if(valid_params)
    {
        aes128_encrypt(array_data, data_encrypted, array_key);
        memmove(message->encrypted_data, data_encrypted, CL_CRYPTO_AES_DATA_LEN * sizeof(uint16));
        message->status = cl_crypto_success; /* current encrypt API doesn't have a status check */
        message->error = cl_no_error;
        message->flags = 0;
    }
    
    free(array_data);
    free(array_key);
    free(data_encrypted);

    MessageSend(theAppTask, CL_CRYPTO_ENCRYPT_CFM, message);
}


void ConnectionEncryptBlockSha256(Task theAppTask, uint16 *data_array, uint16 array_size)
{
    uint8 *array_data = PanicUnlessMalloc(array_size);
    uint8 *hash = PanicUnlessMalloc(CL_CRYPTO_SHA_HASH_LEN * sizeof(uint16));
    bool  valid_params =  TRUE;

    MAKE_CL_MESSAGE(CL_CRYPTO_HASH_CFM);

    if(data_array)
    {
        memmove(array_data, data_array, array_size);
    }
    else
    {
        message->status = cl_crypto_error;
        message->error = cl_sha_empty_data_array;
        valid_params =  FALSE;
    }

    if(valid_params)
    {
        sha256(array_data, (uint8)array_size, hash);
        memmove(message->hash, hash, CL_CRYPTO_SHA_HASH_LEN * sizeof(uint16));
        message->status = cl_crypto_success; /* current  API doesnt have a status check */
        message->error = cl_no_error;
    }

    free(array_data);
    free(hash);

    MessageSend(theAppTask, CL_CRYPTO_HASH_CFM, message);
}

void ConnectionEncryptBlockSha256Continue(Task theAppTask, uint16 *data_array, uint16 array_size, uint16 current_index)
{
    UNUSED(theAppTask);
    UNUSED(data_array);
    UNUSED(array_size);
    UNUSED(current_index);
}
void ConnectionDecryptBlockAes(Task theAppTask, uint16 data_array[CL_CRYPTO_AES_DATA_LEN], uint16 key_array[CL_CRYPTO_AES_KEY_LEN])
{
    uint8 *array_data = PanicUnlessMalloc(CL_CRYPTO_AES_DATA_LEN * sizeof(uint16));
    uint8 *array_key = PanicUnlessMalloc(CL_CRYPTO_AES_KEY_LEN * sizeof(uint16));
    uint8 *data_decrypted = PanicUnlessMalloc(CL_CRYPTO_AES_DATA_LEN * sizeof(uint16)); 
    bool  valid_params =  TRUE;

    MAKE_CL_MESSAGE(CL_CRYPTO_DECRYPT_CFM);
    
    if(data_array)
    {
        memmove(array_data, data_array, CL_CRYPTO_AES_DATA_LEN * sizeof(uint16));
    }
    else
    {
        message->status = cl_crypto_error;
        message->error = cl_aes_empty_data_array;
        valid_params =  FALSE;   
    }
    if(key_array)
    {
        memmove(array_key, key_array, CL_CRYPTO_AES_KEY_LEN * sizeof(uint16));
    }
    else
    {
        message->status = cl_crypto_error;
        message->error = cl_aes_empty_key_array;
        valid_params =  FALSE;        
    }
    
    if(valid_params)
    {
        aes128_decrypt(array_data, data_decrypted, array_key);
        memmove(message->decrypted_data, data_decrypted, CL_CRYPTO_AES_DATA_LEN * sizeof(uint16));

        message->status = cl_crypto_success;/* current decrypt API doesnt have a status check */
        message->error = cl_no_error;
    }

    free(array_data);
    free(array_key);
    free(data_decrypted);
    
    MessageSend(theAppTask, CL_CRYPTO_DECRYPT_CFM, message);
}

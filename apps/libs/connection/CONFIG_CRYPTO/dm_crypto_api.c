/****************************************************************************
Copyright (c) 2017 - 2019 Qualcomm Technologies International, Ltd 


FILE NAME
    dm_crypto_api.c        

DESCRIPTION
    This file contains functions for sending message requests regarding
    cryptography to the Bluestack.

NOTES

*/
#include <vm.h>

#include "connection.h"
#include "connection_private.h"
#include "common.h"


#define SHA_DATA_OCTET_LEN ((sizeof(uint16) == 1) ? CL_CRYPTO_SHA_DATA_LEN * 2: CL_CRYPTO_SHA_DATA_LEN)

#ifndef UNUSED
#define UNUSED(x)       ((void)x)
#endif

/*******************************************************************************
 *                                  FUNCTIONS                                  *
 *******************************************************************************/
void ConnectionGeneratePublicPrivateKey(Task theAppTask, cl_ecc_type key_type)
{
    uint8_t set_key_type;
        
    /* Ensure that an appropriate key type was provided. If not, "short-circuit" the message
       process by sending the originating task an error struct with the appropriate error type */
    switch (key_type)
    {
        case cl_crypto_ecc_p192:
            set_key_type = DM_CRYPTO_ECC_P192;
            break;
            
        case cl_crypto_ecc_p256:
            set_key_type = DM_CRYPTO_ECC_P256;
            break;
        
        default:
            {
                /* Allocate error struct and free the previously allocated prim to eliminate memory leaks. */
                MAKE_CL_MESSAGE(CL_CRYPTO_GENERATE_PUBLIC_PRIVATE_KEY_CFM);
                message->status = cl_crypto_error;
                message->error = cl_ecc_invalid_key_type;
                MessageSend(theAppTask, CL_CRYPTO_GENERATE_PUBLIC_PRIVATE_KEY_CFM, message);
            }
            
            return;
    }

    {
        /* Create message prim. */
        MAKE_PRIM_T(DM_CRYPTO_GENERATE_PUBLIC_PRIVATE_KEY_REQ);
        /* Set the prim's fields to the appropriate values and send it. */
        prim->key_type = set_key_type;
        prim->phandle   = 0;
        prim->context   = (context_t) theAppTask;
        VmSendDmPrim(prim);
    }
}

void ConnectionGenerateSharedSecretKey(Task theAppTask, cl_ecc_type key_type, uint16 private_key[CL_CRYPTO_LOCAL_PVT_KEY_LEN], uint16 public_key[CL_CRYPTO_REMOTE_PUB_KEY_LEN])
{
    /* Create message prim. */
    /* The prim macro uses PanicUnlessMalloc, so prim will always be valid. */
    MAKE_PRIM_T(DM_CRYPTO_GENERATE_SHARED_SECRET_KEY_REQ);
    
    /* Ensure that an appropriate key type was provided. If not, "short-circuit" the message
       process by sending the originating task an error struct with the appropriate error type */
    switch (key_type)
    {
        case cl_crypto_ecc_p192:
            prim->key_type = DM_CRYPTO_ECC_P192;
            break;
            
        case cl_crypto_ecc_p256:
            prim->key_type = DM_CRYPTO_ECC_P256;
            break;
        
        default:
            {
                /* Allocate error struct and free the previously allocated prim to eliminate memory leaks. */
                MAKE_CL_MESSAGE(CL_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM);
                free(prim);
                message->status = cl_crypto_error;
                message->error = cl_ecdh_invalid_key_type;
                MessageSend(theAppTask, CL_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM, message);
            }
            
            return;
    }
    
    /* Ensure that the provided arrays are actually non-null. If not, "short-circuit" the message
       process by sending the originating task an error struct with the appropriate error type */
    if (public_key)
    {
        memmove(&prim->remote_public_key, public_key, CL_CRYPTO_REMOTE_PUB_KEY_LEN * sizeof(uint16));
    }
    else
    {
        /* Allocate error struct and free the previously allocated prim to eliminate memory leaks. */
        MAKE_CL_MESSAGE(CL_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM);
        free(prim);
        message->status = cl_crypto_error;
        message->error = cl_ecdh_empty_public_key;
        MessageSend(theAppTask, CL_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM, message);
        return;
    }
    
    if (private_key)
    {
        memmove(&prim->local_private_key, private_key, CL_CRYPTO_LOCAL_PVT_KEY_LEN * sizeof(uint16));
    }
    else
    {
        /* Allocate error struct and free the previously allocated prim to eliminate memory leaks. */
        MAKE_CL_MESSAGE(CL_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM);
        free(prim);
        message->status = cl_crypto_error;
        message->error = cl_ecdh_empty_private_key;
        MessageSend(theAppTask, CL_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM, message);
        return;
    }
    
    /* Set the prim's fields to the appropriate values and send it. */
    prim->phandle   = 0;
    prim->context   = (context_t) theAppTask;

    VmSendDmPrim(prim);
}

void ConnectionEncryptBlockAes(Task theAppTask, uint16 data_array[CL_CRYPTO_AES_DATA_LEN], uint16 key_array[CL_CRYPTO_AES_KEY_LEN])
{
    /* Create message prim. */
    /* The prim macro uses PanicUnlessMalloc, so prim will always be valid. */
    MAKE_PRIM_T(DM_CRYPTO_ENCRYPT_REQ);
    
    /* Ensure that the provided arrays are actually non-null. If not, "short-circuit" the message
       process by sending the originating task an error struct with the appropriate error type */
    if (data_array)
    {
        memmove(&prim->data, data_array, CL_CRYPTO_AES_DATA_LEN * sizeof(uint16));
    }
    else
    {
        /* Allocate error struct and free the previously allocated prim to eliminate memory leaks. */
        MAKE_CL_MESSAGE(CL_CRYPTO_ENCRYPT_CFM);
        free(prim);
        message->status = cl_crypto_error;
        message->error = cl_aes_empty_data_array;
        MessageSend(theAppTask, CL_CRYPTO_ENCRYPT_CFM, message);
        return;
    }
    
    if (key_array)
    {
        memmove(&prim->encryption_key, key_array, CL_CRYPTO_AES_KEY_LEN * sizeof(uint16));
    }
    else
    {
        /* Allocate error struct and free the previously allocated prim to eliminate memory leaks. */
        MAKE_CL_MESSAGE(CL_CRYPTO_ENCRYPT_CFM);
        free(prim);
        message->status = cl_crypto_error;
        message->error = cl_aes_empty_key_array;
        MessageSend(theAppTask, CL_CRYPTO_ENCRYPT_CFM, message);
        return;
    }
    
    /* Set the prim's fields to the appropriate values and send it. */
    prim->flags     = 0;
    prim->phandle   = 0;
    prim->context   = (context_t) theAppTask;
    
    VmSendDmPrim(prim);
}


void ConnectionEncryptBlockSha256(Task theAppTask, uint16 *data_array, uint16 array_size)
{
    uint16 move_array_size;
    
    /* Create message prim. */
    /* The prim macro uses PanicUnlessMalloc, so prim will always be valid. */
    MAKE_PRIM_T(DM_CRYPTO_HASH_REQ);
    
    /* Ensure that the provided array is actually non-null. If not, "short-circuit" the message
       process by sending the originating task an error struct with the appropriate error type */
    if (!data_array)
    {
        /* Allocate error struct and free the previously allocated prim to eliminate memory leaks. */
        MAKE_CL_MESSAGE(CL_CRYPTO_HASH_CFM);
        free(prim);
        message->status = cl_crypto_error;
        message->error = cl_sha_empty_data_array;
        MessageSend(theAppTask, CL_CRYPTO_HASH_CFM, message);
        return;
    }
    
    prim->flags     = 0;
    prim->phandle   = 0;
    prim->context   = (context_t) theAppTask;
    memset(&prim->data, 0, CL_CRYPTO_SHA_DATA_LEN * sizeof(uint16));
    
    /* Handling packing octets into words if the chip used is Bluecore. 
       array_size is in octets. */
    if (sizeof(uint16) == 1)
    {
        move_array_size = (array_size + 1) / 2;
    }
    else
    {
        move_array_size = array_size;
    }
    
    /*  If the valid characters are less than 32 bytes, the hash operation can 
        be completed in one pass. */
    if (array_size <= SHA_DATA_OCTET_LEN)
    {
        prim->data_size = array_size;
        memmove(&prim->data, data_array, move_array_size);
        prim->operation = DM_CRYPTO_SINGLE_BLOCK;
    }
    else
    {
        /* Since the data is larger than a single block of 32 bytes, the first 32 bytes are sent for hashing.
           The end of non-zero characters within the array is stored in sha_buff_offset, to be used for calculating when
           hashing is complete. */
        prim->data_size = SHA_DATA_OCTET_LEN;
        memmove(&prim->data, data_array, CL_CRYPTO_SHA_DATA_LEN * sizeof(uint16));
        prim->operation = DM_CRYPTO_DATA_START;
    }
    
    VmSendDmPrim(prim);
}

void ConnectionEncryptBlockSha256Continue(Task theAppTask, uint16 *data_array, uint16 array_size, uint16 current_index)
{
    uint16 size;
    uint16 move_size;
    
    /* Create message prim. */
    /* The prim macro uses PanicUnlessMalloc, so prim will always be valid. */
    MAKE_PRIM_T(DM_CRYPTO_HASH_REQ);
    
    /* Ensure that the provided array is actually non-null. If not, "short-circuit" the message
       process by sending the originating task an error struct with the appropriate error type */
    if (!data_array)
    {
        /* Allocate error struct and free the previously allocated prim to eliminate memory leaks. */
        MAKE_CL_MESSAGE(CL_CRYPTO_HASH_CFM);
        free(prim);
        message->status = cl_crypto_error;
        message->error = cl_sha_cont_empty_data_array;
        MessageSend(theAppTask, CL_CRYPTO_HASH_CFM, message);
        return;
    }
    
    prim->flags     = 0;
    prim->phandle   = 0;
    prim->context   = 0;
    memset(&prim->data, 0, CL_CRYPTO_SHA_DATA_LEN * sizeof(uint16));
    
    /* Calculate the amount of remaining data to be hashed. */
    size = array_size - current_index;
    
    /* Handling packing octets into words if the chip used is Bluecore. */
    if (sizeof(uint16) == 1)
    {
        move_size = (size + 1) / 2;
    }
    else
    {
        move_size = size;
    }
    
    /* Remaining data can be hashed as one block, and therefore this will be the last block sent. */
    if (size <= SHA_DATA_OCTET_LEN)
    {
        prim->operation = DM_CRYPTO_DATA_END;
        prim->data_size = size;
        memmove(&prim->data, &data_array[current_index / 2], move_size);
    }
    else
    {
        /* Remaining data is still more than 32 bytes, so send a block of 32 bytes. */
        prim->operation = DM_CRYPTO_DATA_CONTI;
        prim->data_size = SHA_DATA_OCTET_LEN;
        memmove(&prim->data, &data_array[current_index / 2], CL_CRYPTO_SHA_DATA_LEN * sizeof(uint16));
    }
    
    VmSendDmPrim(prim);
}

void ConnectionDecryptBlockAes(Task theAppTask, uint16 data_array[CL_CRYPTO_AES_DATA_LEN], uint16 key_array[CL_CRYPTO_AES_KEY_LEN])
{
    UNUSED(theAppTask);
    UNUSED(data_array);
    UNUSED(key_array);
}

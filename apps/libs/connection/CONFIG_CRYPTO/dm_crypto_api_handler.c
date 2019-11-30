/****************************************************************************
Copyright (c) 2017 - 2019 Qualcomm Technologies International, Ltd 


FILE NAME
    dm_crypto_api_handler.c        

DESCRIPTION
    This file contains functions for handling the confirmation messages from the Bluestack 
    regarding cryptography.

NOTES

*/

#include <vm.h>

#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "dm_crypto_api_handler.h"

#define CASE(from,to)  \
        case (from): \
            if ((from)==(to)) \
                goto coerce; \
            else \
                return (to);


/*******************************************************************************
 *                          Helper function prototypes                         *
 *******************************************************************************/
 cl_hash_operation connectionConvertHashOperation(uint8_t op);
 cl_crypto_status connectionConvertCryptoStatus(uint8_t status);
 cl_ecc_type connectionConvertEccType(uint8_t ecc_key);
 
 
/*******************************************************************************
 *                      Message handling function prototypes                   *
 *******************************************************************************/
 void connectionHandleGeneratePublicPrivateKeyCfm(DM_CRYPTO_GENERATE_PUBLIC_PRIVATE_KEY_CFM_T *cfm);
 void connectionHandleGenerateSharedSecretKey(DM_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM_T * cfm);
 void connectionHandleAesEncrypt(DM_CRYPTO_ENCRYPT_CFM_T * cfm);
 void connectionHandleHash(DM_CRYPTO_HASH_CFM_T * cfm);
 
 /*******************************************************************************
 *                          Message handling functions                          *
 *******************************************************************************/
 
 /****************************************************************************
NAME    
    connectionBluestackHandlerDmCrypto

DESCRIPTION
    Handler for Crypto messages

RETURNS
    TRUE if message handled, otherwise false
*/
bool connectionBluestackHandlerDmCrypto(const DM_UPRIM_T *message)
{
    switch (message->type)
    {
        case DM_CRYPTO_GENERATE_PUBLIC_PRIVATE_KEY_CFM:
        {
            CL_DEBUG_INFO(("DM_CRYPTO_GENERATE_PUBLIC_PRIVATE_KEY_CFM\n"));
            connectionHandleGeneratePublicPrivateKeyCfm((DM_CRYPTO_GENERATE_PUBLIC_PRIVATE_KEY_CFM_T *)message);
            return TRUE;
        }

        case DM_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM:
        {
            CL_DEBUG_INFO(("DM_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM\n"));
            connectionHandleGenerateSharedSecretKey((DM_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM_T *)message);
            return TRUE;
        }

        case DM_CRYPTO_ENCRYPT_CFM:
        {
            CL_DEBUG_INFO(("DM_CRYPTO_ENCRYPT_CFM\n"));	
            connectionHandleAesEncrypt((DM_CRYPTO_ENCRYPT_CFM_T *)message);
            return TRUE;
        }

        case DM_CRYPTO_HASH_CFM:
        {
            CL_DEBUG_INFO(("DM_CRYPTO_HASH_CFM\n"));	
            connectionHandleHash((DM_CRYPTO_HASH_CFM_T *)message);
            return TRUE;
        }
        
        default:
            return FALSE;
    }   

}
 
 /****************************************************************************
NAME    
    connectionHandleGeneratePublicPrivateKeyCfm

DESCRIPTION
    Handle the Bluestack response to a request for the generation of a 
    public/private encryption key pair using ECC.
    
PARAMS
    cfm      The confirmation message from Bluestack.

RETURNS
    void
*/
void connectionHandleGeneratePublicPrivateKeyCfm(DM_CRYPTO_GENERATE_PUBLIC_PRIVATE_KEY_CFM_T *cfm)
{
    Task theAppTask = (Task) cfm->context;
    
    if(theAppTask)
    {
        MAKE_CL_MESSAGE(CL_CRYPTO_GENERATE_PUBLIC_PRIVATE_KEY_CFM);
        
        message->status = connectionConvertCryptoStatus(cfm->status);
        
        if (message->status == cl_crypto_success)
        {
            /* Key pair generation successful. */
            memmove(message->public_key, cfm->public_key, CL_CRYPTO_PUBLIC_KEY_LEN * sizeof(uint16));
            memmove(message->private_key, cfm->private_key, CL_CRYPTO_PRIVATE_KEY_LEN * sizeof(uint16));
            message->key_type = connectionConvertEccType(cfm->key_type);
        }
        
        MessageSend(theAppTask, CL_CRYPTO_GENERATE_PUBLIC_PRIVATE_KEY_CFM, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("Failed to get task context from DM_CRYPTO_GENERATE_PUBLIC_PRIVATE_KEY_CFM message. \n"));
    }
#endif
}


/****************************************************************************
NAME    
    connectionHandleGenerateSharedSecretKey

DESCRIPTION
    Handle the Bluestack response to a request for the generation of a shared
    secret encryption key from a public/private key pair using ECHD.
    
PARAMS
    cfm      The confirmation message from Bluestack.

RETURNS
    void
*/
void connectionHandleGenerateSharedSecretKey(DM_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM_T * cfm)
{
    Task theAppTask = (Task) cfm->context;
    
    if(theAppTask)
    {
        MAKE_CL_MESSAGE(CL_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM);
        
        message->status = connectionConvertCryptoStatus(cfm->status);
        
        if (message->status == cl_crypto_success)
        {
            /* Shared secret generation successful. */
            memmove(message->shared_secret_key, cfm->shared_secret_key, CL_CRYPTO_SECRET_KEY_LEN * sizeof(uint16));
            message->key_type = connectionConvertEccType(cfm->key_type);
        }
        
        MessageSend(theAppTask, CL_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("Failed to get task context from DM_CRYPTO_GENERATE_SHARED_SECRET_KEY_CFM message. \n"));
    }
#endif
}


/****************************************************************************
NAME    
    connectionHandleAesEncrypt

DESCRIPTION
    Handle the Bluestack response to a request for the encryption of a 
    128-bit block of data using a 128-bit encryption key using AES.
    
PARAMS
    cfm      The confirmation message from Bluestack.

RETURNS
    void
*/
void connectionHandleAesEncrypt(DM_CRYPTO_ENCRYPT_CFM_T * cfm)
{
    Task theAppTask = (Task) cfm->context;
    
    if(theAppTask)
    {
        MAKE_CL_MESSAGE(CL_CRYPTO_ENCRYPT_CFM);
        
        message->status = connectionConvertCryptoStatus(cfm->status);
        
        if (message->status == cl_crypto_success)
        {
            /* AES Encryption successful. */
            memmove(message->encrypted_data, cfm->encrypted_data, CL_CRYPTO_AES_DATA_LEN * sizeof(uint16));
        }
        
        message->flags = 0;
        
        MessageSend(theAppTask, CL_CRYPTO_ENCRYPT_CFM, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("Failed to get task context from DM_CRYPTO_ENCRYPT_CFM message. \n"));
    }
#endif
}


/****************************************************************************
NAME    
    connectionHandleHash

DESCRIPTION
    Handle the Bluestack response to a request for the encryption of a data
    block of arbitrary length using SHA256.
    
PARAMS
    cfm      The confirmation message from Bluestack.

RETURNS
    void
*/
void connectionHandleHash(DM_CRYPTO_HASH_CFM_T * cfm)
{
    Task theAppTask = (Task) cfm->context;
    
    if(theAppTask)
    {
        MAKE_CL_MESSAGE(CL_CRYPTO_HASH_CFM);
        
        message->status = connectionConvertCryptoStatus(cfm->status);
        message->operation = connectionConvertHashOperation(cfm->operation);
        
        if (message->status == cl_crypto_success)
        {
            if((message->operation == cl_crypto_single_block) || (message->operation == cl_crypto_data_end))
            {
                message->status = cl_crypto_success;
                memmove(message->hash, cfm->hash, CL_CRYPTO_SHA_HASH_LEN * sizeof(uint16));
            }
        }
        
        message->flags = 0;
        
        MessageSend(theAppTask, CL_CRYPTO_HASH_CFM, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("Failed to get task context from DM_CRYPTO_HASH_CFM message. \n"));
    }
#endif
}

/****************************************************************************
NAME    
    connectionConvertHashOperation

DESCRIPTION
    Convert the Bluestack defined Hash Operation into the Connection Lib
    defined Hash Operation

RETURNS
    Connection Lib Hash Operation
*/
cl_hash_operation connectionConvertHashOperation(uint8_t op)
{
    switch(op)
    {
        CASE(DM_CRYPTO_SINGLE_BLOCK, cl_crypto_single_block);
        CASE(DM_CRYPTO_DATA_START, cl_crypto_data_start);
        CASE(DM_CRYPTO_DATA_CONTI, cl_crypto_data_conti);
        CASE(DM_CRYPTO_DATA_END, cl_crypto_data_end);
        coerce: return (cl_hash_operation)op;
        default:
            CL_DEBUG(("Unrecognised hash operation %d\n", op));
            return cl_crypto_unknown_operation;
    }
}

/****************************************************************************
NAME    
    connectionConvertCryptoStatus

DESCRIPTION
    Convert the Bluestack defined Crypto status into the Connection Lib
    defined Crypto status

RETURNS
    Connection Lib Crypto status
*/
cl_crypto_status connectionConvertCryptoStatus(uint8_t status)
{
    switch(status)
    {
        CASE(DM_CRYPTO_SUCCESS, cl_crypto_success);
        CASE(DM_CRYPTO_INT_SUCCESS, cl_crypto_int_success);
        CASE(DM_CRYPTO_BUSY, cl_crypto_busy);
        CASE(DM_CRYPTO_INVALID_PARAM, cl_crypto_invalid_param);
        CASE(DM_CRYPTO_GENERIC_FAIL, cl_crypto_generic_fail);
        coerce: return (cl_crypto_status)status;
        default:
            CL_DEBUG(("Unrecognised crypto status %d\n", status));
            return cl_crypto_unknown_status;
    }
}

/****************************************************************************
NAME    
    connectionConvertEccType

DESCRIPTION
    Convert the Bluestack defined ECC Key type into the Connection Lib
    defined ECC Key type

RETURNS
    Connection Lib ECC Key type
*/
cl_ecc_type connectionConvertEccType(uint8_t ecc_key)
{
    switch(ecc_key)
    {
        CASE(DM_CRYPTO_ECC_P192, cl_crypto_ecc_p192);
        CASE(DM_CRYPTO_ECC_P256, cl_crypto_ecc_p256);
        coerce: return (cl_ecc_type)ecc_key;
        default:
            CL_DEBUG(("Unrecognised ECC key type %d\n", ecc_key));
            return cl_crypto_ecc_unknown_type;
    }
}

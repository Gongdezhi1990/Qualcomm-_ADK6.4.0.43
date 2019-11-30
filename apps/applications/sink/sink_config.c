/*
Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/

#include "sink_config.h"
#include "sink_debug.h"
#include "sink_led_err.h"
#include "sink_leds.h"
#include "sink_configmanager.h"

#include <ps.h>
#include <string.h>
#include <panic.h>
#include <stdlib.h>
#include <file.h>
#include <stream.h>
#include <print.h>

#ifdef DEBUG_CONFIG
#define CONFIG_DEBUG(x) DEBUG(x)
#else
#define CONFIG_DEBUG(x) 
#endif


/****************************************************************************
NAME 
    ConfigRetrieve

DESCRIPTION
    This function is called to read a configuration key.  If the key exists
    in persistent store it is read from there.  If it does not exist then
    the default is read from constant space

    The len parameter should match the value given by sizeof(). On the XAP 
    processor this will be in the unit of words.
 
RETURNS
    0 if no data was read otherwise the length of data in words. If comparing
    against a sizeof() then the sizeof() value should be converted to words
    using the PS_SIZE_ADJ() macro
*/
uint16 ConfigRetrieve(uint16 config_id, void* data, uint16 len)
{
    uint16 ret_len = 0;
    uint16 requested_words;
    uint16 key_len;
    CONFIG_DEBUG(("CONF: kevcvy_id[%u]\n",config_id));

#ifdef HYDRACORE
    if ((unsigned)data & 0x1)
    {
        Panic();    /* Address is not aligned */
    }
#endif
    requested_words = PS_SIZE_ADJ(len);

    key_len = PsRetrieve(config_id, NULL, 0);

    /* read from PS */
    CONFIG_DEBUG(("CONF:PS config_id[%u]\n", config_id)) ;	            
    /* Read requested key from PS if it exists */

    if (key_len != requested_words)
    {
        CONFIG_DEBUG(("Key Len Mismatch for USR%d. Want %d, have %d",config_id,requested_words,key_len));
    }

    ret_len = PsRetrieve(config_id, (uint16 *)data, requested_words);
        
    /* Check if the pskey was read */
    if(!ret_len)
    {
        /* No PSKEY exists */
        CONFIG_DEBUG(("CONF:No PSKEY[%u]\n",config_id)) ;
    }

    /* check data length */
     switch(config_id)
     {
            /* Config where it's ok for (ret_len != len) 
                or where it doesn't matter if it exists */
            case CONFIG_UPGRADE_CONTEXT:
            case CONFIG_FAST_PAIR_ACCOUNT_KEY_INDEX:
            case CONFIG_FAST_PAIR_ACCOUNT_KEYS:                    
                break;

            default:
                if (ret_len != requested_words)
                {
                    PRINT(("CONF:BADLEN![%u][0x%x][0x%x]\n",config_id, ret_len, requested_words)) ;
                    LedsIndicateError(led_err_id_enum_config) ;
                }
                break;
     }

    return ret_len;
}

/****************************************************************************
NAME 
    ConfigStore

DESCRIPTION
    This function is called to store a configuration key.  This will always
    in PS Store

    The len parameter should match the value given by sizeof(). On the XAP 
    processor this will be in the unit of words. If comparing against a sizeof() 
    then the sizeof() value should be converted to words using the PS_SIZE_ADJ() 
    macro
 
RETURNS
    0 if no data was stored otherwise the length of data in words
*/
uint16 ConfigStore(uint16 config_id, const void *data, uint16 len)
{
    uint16 storeLen = 0;

#ifdef HYDRACORE
    if ((unsigned)data & 0x1)
    {
        Panic();    /* Address is not aligned */
    }
#endif

    /* stored keys */
    storeLen = PsStore(config_id, (const uint16 *)data, PS_SIZE_ADJ(len));
    CONFIG_DEBUG(("CONF:Stored[%u]len[0x%x]words\n",config_id, storeLen)) ;
    return storeLen;
}

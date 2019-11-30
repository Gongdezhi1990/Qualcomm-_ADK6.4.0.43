/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    ps_dynamic_config_store.c
 
DESCRIPTION
    See the ps_dynamic_config_store.h file for more details.
*/


#include <vmtypes.h>
#include <ps_dynamic_config_store.h>
#include <config_data.h>
#include <ps.h>
#include <stdlib.h>
#include <panic.h>
#include <csrtypes.h>
#include <string.h>


/* Macro used for all critical code sections */
#define CONFIG_STORE_ASSERT(x) {if (!(x)) Panic();}


#define SINGLE_PS_KEY_ITEM_BITS         (16)
#define USED_PS_KEYS_SIZE               (13)
#define USED_PS_KEYS_OFFSET(ps_key)     (ps_key / SINGLE_PS_KEY_ITEM_BITS)
#define USED_PS_KEYS_BIT(ps_key)        ((uint16)(1 << (ps_key % SINGLE_PS_KEY_ITEM_BITS)))


static uint16 resolvePSKey(const void *const_config_data, config_blk_id_t id);
static bool initUsedPsKeysPsKey(void);
static bool getPsKeyUsedState(uint16 ps_key);
static dynamic_config_status_t setPsKeyUsedState(uint16 ps_key, bool used);


/* Private PS Dynamic Config Store Data */
typedef struct __ps_dynamic_config_store_data
{
    uint16 used_ps_keys_key;                /* Number of PS key which keeps the informations about used ps keys */
    uint16 used_ps_keys[USED_PS_KEYS_SIZE]; /* Information about used ps keys */
} ps_dynamic_config_store_data_t;


static ps_dynamic_config_store_data_t *psDynamicConfigStore;



/******************************************************************************/
dynamic_config_status_t DynamicConfigInit(const void *const_config_data)
{
    const config_blk_data_t *config_blk_data;


    psDynamicConfigStore = PanicUnlessMalloc(sizeof(ps_dynamic_config_store_data_t));
    memset(psDynamicConfigStore->used_ps_keys, 0, (USED_PS_KEYS_SIZE * sizeof(uint16)));


    config_blk_data = GET_PS_CONFIG_SET_DATA_PTR(const_config_data);
    psDynamicConfigStore->used_ps_keys_key = config_blk_data->value[0];

    if(initUsedPsKeysPsKey())
    {
        return dynamic_config_success;
    }
    else
    {
        return dynamic_config_error_init;
    }
}


/******************************************************************************/
dynamic_config_status_t DynamicConfigGetConfig(const void *const_config_data, config_blk_id_t id, void *config_data, uint16 requested_config_data_size)
{
    uint16 ps_key;
    uint16 retrieved_data_size;

    ps_key = resolvePSKey(const_config_data, id);

    retrieved_data_size = PsRetrieve(ps_key, config_data, requested_config_data_size);

    /* Check if amount of words retrieved matches the expected value */
    if(retrieved_data_size != requested_config_data_size)
    {
        return dynamic_config_error_retrieve;
    }

    return dynamic_config_success;
}


/******************************************************************************/
dynamic_config_status_t DynamicConfigWriteConfig(const void *const_config_data, config_blk_id_t id, void *config_data, uint16 config_data_size)
{
    uint16 ps_key;
    uint16 written_data_size;

    ps_key = resolvePSKey(const_config_data, id);

    written_data_size = PsStore(ps_key, config_data, config_data_size);

    /* Check if amount of words written matches the expected value */
    if(written_data_size != config_data_size)
    {
        return dynamic_config_error_write;
    }

    return setPsKeyUsedState(ps_key, TRUE);
}


/******************************************************************************/
dynamic_config_status_t DynamicConfigRemoveConfig(const void *const_config_data, config_blk_id_t id)
{
    uint16 ps_key;
    uint16  written_data_size;

    ps_key = resolvePSKey(const_config_data, id);

    written_data_size = PsStore(ps_key, NULL, 0);

    if(written_data_size == 0)
    {
        return setPsKeyUsedState(ps_key, FALSE);
    }
    else
    {
        return dynamic_config_error_removing;
    }
}


/******************************************************************************/
int16 DynamicConfigGetConfigSize(const void *const_config_data, config_blk_id_t id)
{
    uint16 ps_key;

    ps_key = resolvePSKey(const_config_data, id);

    if(getPsKeyUsedState(ps_key))
    {
        /* Block present in ps store. */
        return PsRetrieve(ps_key, NULL, 0);
    }
    else
    {
        /* Block not present in ps store. */
        return -1;
    }
}


/***************************************************************************
NAME
    ResolvePSKey
 
DESCRIPTION
    Function that resolves the PS key in which dynamic config data is placed.
 
PARAMS
    const_config_data Pointer to the const config data.
    id                Config block id.
RETURNS
    PS key id.
*/
static uint16 resolvePSKey(const void *const_config_data, config_blk_id_t id)
{
    const config_blk_data_t *config_blk_data = GET_CONFIG_BLOCK_DATA_PTR(const_config_data, id);
    const config_ps_blk_data_t *ps_config_blk_data = GET_PS_CONFIG_BLOCK_DATA_PTR(const_config_data, id, config_blk_data);

    /* Sanity check for config block data entry */
    CONFIG_STORE_ASSERT(GET_TYPE_FROM_TLV(config_blk_data) == CONFIG_DATA_CONFIG_BLOCK_DATA_TYPE);
    /* Sanity check for ps config block data entry */
    CONFIG_STORE_ASSERT(GET_TYPE_FROM_TLV(ps_config_blk_data) == CONFIG_DATA_PS_CONFIG_BLOCK_DATA_TYPE);

    return ps_config_blk_data->value[0];
}


/***************************************************************************
NAME
    initUsedPsKeysPsKey
 
DESCRIPTION
    Function that checks if Used PS keys ps key is present.
    If it's not, this functions creates initial content of this key.

RETURNS
    TRUE if the key is present, FALSE otherwise.
*/
static bool initUsedPsKeysPsKey(void)
{
    uint16 ps_key_size = PsRetrieve(psDynamicConfigStore->used_ps_keys_key, psDynamicConfigStore->used_ps_keys, USED_PS_KEYS_SIZE);

    if(!ps_key_size) /* PS key not present, it has to be created */
    {
        uint16 written_data_size;

        written_data_size = PsStore(psDynamicConfigStore->used_ps_keys_key, psDynamicConfigStore->used_ps_keys, USED_PS_KEYS_SIZE);
        if(written_data_size != USED_PS_KEYS_SIZE)
        {
            return FALSE;
        }
    }
    else if(ps_key_size != USED_PS_KEYS_SIZE)
    {
        /* There is already some data in Used PS keys key, but it's size is incorrect */
        return FALSE;
    }

    return TRUE;
}


/***************************************************************************
NAME
    setPsKeyUsedState
 
DESCRIPTION
    Function that sets given ps key as used or unused.
 
PARAMS
    ps_key PS key number.
    used   If this param is set to TRUE this function will set given PS key as used.
           If it is FALSE, the key will be set as unused.
RETURNS
    dynamic_config_success if everything was OK, error code otherwise.
*/
static dynamic_config_status_t setPsKeyUsedState(uint16 ps_key, bool used)
{
    uint16 written_data_size;

    if(used)
    {
        psDynamicConfigStore->used_ps_keys[USED_PS_KEYS_OFFSET(ps_key)] |= USED_PS_KEYS_BIT(ps_key);
    }
    else
    {
        psDynamicConfigStore->used_ps_keys[USED_PS_KEYS_OFFSET(ps_key)] &= ~USED_PS_KEYS_BIT(ps_key);
    }

    written_data_size = PsStore(psDynamicConfigStore->used_ps_keys_key, psDynamicConfigStore->used_ps_keys, USED_PS_KEYS_SIZE);
    if(written_data_size != USED_PS_KEYS_SIZE)
    {
        return dynamic_config_error_write;
    }

    return dynamic_config_success;
}


/***************************************************************************
NAME
    getPsKeyUsedState
 
DESCRIPTION
    Function that checks if given PS key is used.
 
PARAMS
    ps_key PS key number.

RETURNS
    TRUE if the given key is used, FALSE otherwise.
*/
static bool getPsKeyUsedState(uint16 ps_key)
{
    return (psDynamicConfigStore->used_ps_keys[USED_PS_KEYS_OFFSET(ps_key)] & USED_PS_KEYS_BIT(ps_key));
}

/***************************************************************************/
dynamic_config_status_t DynamicConfigResetToDefaults(void)
{
    uint16 ps_key;
    uint16 data_size;

    for (ps_key = 0; USED_PS_KEYS_OFFSET(ps_key) < USED_PS_KEYS_SIZE; ps_key++)
    {
        if (psDynamicConfigStore->used_ps_keys[USED_PS_KEYS_OFFSET(ps_key)] & USED_PS_KEYS_BIT(ps_key))
        {
            PsStore(ps_key, NULL, 0);
            psDynamicConfigStore->used_ps_keys[USED_PS_KEYS_OFFSET(ps_key)] &= ~USED_PS_KEYS_BIT(ps_key);
        }
    }

    data_size = PsStore(psDynamicConfigStore->used_ps_keys_key, psDynamicConfigStore->used_ps_keys, USED_PS_KEYS_SIZE);
    if (data_size != USED_PS_KEYS_SIZE)
    {
        /* At this point the used ps keys in the ps store does not match the
           ps keys actually in use and could lead to a config block using 
           invalid data. This should never happen, so panic. */
        Panic();
    }

    return dynamic_config_success;
}

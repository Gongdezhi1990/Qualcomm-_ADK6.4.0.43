/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    config_store.c
 
DESCRIPTION
    See the config_store.h file for more details.
*/


#include <config_store.h>
#include <config_data.h>
#include <dynamic_config_if.h>
#include <panic.h>
#include <print.h>
#include <string.h>
#include <stdlib.h>


#ifdef DEBUG_PRINT_ENABLED
#define CONFIG_STORE_DEBUG_ASSERT(x) {if (!(x)) Panic();}
#else
#define CONFIG_STORE_DEBUG_ASSERT(x)
#endif


/* Macro used for all critical code sections */
#define CONFIG_STORE_ASSERT(x) {if (!(x)) Panic();}

/* Macro that returns the smaller of two input values */
#define SMALLER_VALUE(x,y)       ((x) > (y) ? (y) : (x))
/* Macro that returns the biger of two input values */
#define BIGER_VALUE(x,y)       ((x) > (y) ? (x) : (y))

/* Each time we run out of memory used for keeping track of requested config blocks,
   we have to reallocate more memory. This macro defines the number of new request
   we can track after the reallocation.*/
#define CONFIG_BLOCK_MEMORY_ALLOC_STEP          (5)

/* Value used for marking config block as not in use */
#define CONFIG_BLOCK_ID_UNUSED                  (0)

/* Struct used for storing config data requested by the module.
   If requested config data is placed only in const memory
   'dynamic_config_data' will be set to NULL.
   If config block is writeable, 'writeable_config_data_size'
   is set to the size of 'dynamic_config_data', if not it's set to zero.
   
*/
typedef struct __config_block
{
    config_blk_id_t id;                         /* Id of module requesting config data. */
    uint16          writeable_config_data_size; /* Size of the writeable config. */
    void            *dynamic_config_data;       /* Pointer to dynamic config data. */
} config_block_t;


/* Private Config Store data */
typedef struct __config_store_data
{
    const void     *const_config_set;       /* Configuration Set */
    uint16         num_config_blocks;       /* Number of config blocks that can be stored in 'config_blocks' array */
    bool           const_mode;              /* Do not read/write config data from/to dynamic config store. */
    config_block_t config_blocks[1];        /* Config blocks */
} config_store_data_t;
/* Macro that returns the size of config store private data struct */
#define CONFIG_STORE_PRIVATE_DATA_SIZE(config_blocks_used)     (sizeof(config_store_data_t) - \
                                                                sizeof(config_block_t) + \
                                                                (sizeof(config_block_t) * (config_blocks_used)))

static config_store_data_t *configStore;


/***************************************************************************
NAME
    resetConfigDataPtrAndSize
 
DESCRIPTION
    Function that resets the pointer to config data and its size.
 
PARAMS
    data_ptr  pointer to the config data.
    data_size pointer to the size of data.
 
RETURNS
    void
*/
static void resetConfigDataPtrAndSize(const void **data_ptr, uint16 *data_size)
{
    *data_ptr = NULL;
    *data_size = 0;
}


/***************************************************************************
NAME
    getConfigBlock
 
DESCRIPTION
    This function returns pointer to config block with given id,
    if it finds it.
 
RETURNS
    config_block_t* Pointer to the given config block if it's found,
    NULL otherwise.
*/
static config_block_t* getConfigBlock(config_blk_id_t id)
{
    uint16 i;

    for(i = 0; i < configStore->num_config_blocks; i++)
    {
        if(configStore->config_blocks[i].id == id)
        {
            return &configStore->config_blocks[i];
        }
    }

    return NULL;
}


/***************************************************************************
NAME
    isConfigBlockInUse
 
DESCRIPTION
    Function that checks if the given config block is in use.
 
PARAMS
    id config block id.
 
RETURNS
    bool TRUE if it is in use, FALSE otherwise.
*/
static bool isConfigBlockInUse(config_blk_id_t id)
{
    if(getConfigBlock(id))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/***************************************************************************
NAME
    isConstConfigDataValid

DESCRIPTION
    Verify the const data for the given config block id is valid.

PARAMS
    id config block id.

RETURNS
    bool TRUE if it is valid, FALSE otherwise.
*/
static bool isConstConfigDataValid(config_blk_id_t id)
{
    const config_blk_t* config_blk;
    const config_blk_data_t* config_blk_data;

    config_blk = GET_CONFIG_BLOCK_PTR(configStore->const_config_set, id);
    config_blk_data = GET_CONFIG_BLOCK_DATA_PTR(configStore->const_config_set, id);

    if (id >= GET_LENGTH_FROM_TLV(configStore->const_config_set)
        || CONFIG_DATA_CONFIG_BLOCK_TYPE != GET_TYPE_FROM_TLV(config_blk)
        || CONFIG_DATA_CONFIG_BLOCK_DATA_TYPE != GET_TYPE_FROM_TLV(config_blk_data)
        || GET_LENGTH_FROM_TLV(config_blk_data) > GET_LENGTH_FROM_TLV(config_blk))
        return FALSE;

    return TRUE;
}

/***************************************************************************
NAME
    addConfigBlock
 
DESCRIPTION
    Function that adds config block into the config store.
 
PARAMS
    id                          config block id.
    dynamic_config_data         pointer to module's dynamically allocated data (allocated outside of this function).
    writeable_config_data_size  Size of the writeable config.
                                If the config block is read only this is set to zero.
RETURNS
    bool TRUE if config block was added to the config store correctly, FALSE otherwise.
*/
static bool addConfigBlock(config_blk_id_t id, void * dynamic_config_data, uint16 writeable_config_data_size)
{
    config_block_t *unused_config_block = getConfigBlock(CONFIG_BLOCK_ID_UNUSED);

    if(!unused_config_block)
    {
        /* Unused config block not found, which means that we have to increase
           amount of memory used for tracking config block requests */
        config_store_data_t *reallocated_config_store = NULL;
        uint16 config_block_count = configStore->num_config_blocks;
        uint16 new_config_store_size = CONFIG_STORE_PRIVATE_DATA_SIZE(config_block_count + CONFIG_BLOCK_MEMORY_ALLOC_STEP);

        reallocated_config_store = realloc(configStore, new_config_store_size);

        if(reallocated_config_store)
        {
            /* Clear the newly allocated config blocks */
            void* new_config_blocks = (void*)&reallocated_config_store->config_blocks[config_block_count];

            memset(new_config_blocks, 0x00, (sizeof(config_block_t) * CONFIG_BLOCK_MEMORY_ALLOC_STEP));

            configStore = reallocated_config_store;
            configStore->num_config_blocks += CONFIG_BLOCK_MEMORY_ALLOC_STEP;

            unused_config_block = getConfigBlock(CONFIG_BLOCK_ID_UNUSED);
        }

        if(!unused_config_block)
        {
            return FALSE;
        }
    }

    unused_config_block->id = id;
    unused_config_block->dynamic_config_data = dynamic_config_data;
    unused_config_block->writeable_config_data_size = writeable_config_data_size;

    return TRUE;
}


/***************************************************************************
NAME
    removeConfigBlock
 
DESCRIPTION
    Function that removes config block from config store, and releases
    all dynamically allocated data, used by the given config block.
 
PARAMS
    id config block id.
*/
static void removeConfigBlock(config_blk_id_t id)
{
    config_block_t *config_block_to_remove = getConfigBlock(id);

    CONFIG_STORE_DEBUG_ASSERT(config_block_to_remove);

    if(config_block_to_remove)
    {
        config_block_to_remove->id = CONFIG_BLOCK_ID_UNUSED;
        /* If there was any dynamic memory allocated for this config block - free it */
        if(config_block_to_remove->dynamic_config_data)
        {
            free(config_block_to_remove->dynamic_config_data);
            resetConfigDataPtrAndSize((void*)&config_block_to_remove->dynamic_config_data,
                                      &config_block_to_remove->writeable_config_data_size);
        }
    }
}


/***************************************************************************
NAME
    GetConstConfigData
 
DESCRIPTION
    Function that extracts pointer to config block's const data
    placed and its size.
 
PARAMS
    id               config block id.
    config_data_size config block's size passed as a reference.
RETURNS
    const void * Pointer to the config block's const data.
*/
static const void * GetConstConfigData(config_blk_id_t id, uint16* config_data_size)
{
    const config_blk_data_t* config_blk_data = GET_CONFIG_BLOCK_DATA_PTR(configStore->const_config_set, id);

    /* Sanity check for config block data entry */
    CONFIG_STORE_ASSERT(GET_TYPE_FROM_TLV(config_blk_data) == CONFIG_DATA_CONFIG_BLOCK_DATA_TYPE);

    *config_data_size = GET_LENGTH_FROM_TLV(config_blk_data);
    return (const void *)&config_blk_data->value[0];
}


/******************************************************************************/
config_store_status_t ConfigStoreInit(const void *const_config_data, uint16 const_config_data_size, bool const_mode)
{
    const config_set_t* config_set = (const config_set_t*)const_config_data;

    /* Sanity check for config set */
    CONFIG_STORE_ASSERT(GET_TYPE_FROM_TLV(config_set) == CONFIG_DATA_CONFIG_SET_TYPE);
    CONFIG_STORE_ASSERT(GET_LENGTH_FROM_TLV(config_set) == (const_config_data_size - (sizeof(config_set->type_and_length) / sizeof(uint16))));

    /* Initialise the dynamic config store */
    if(DynamicConfigInit(const_config_data) == dynamic_config_success)
    {
        uint16 total_config_store_size = CONFIG_STORE_PRIVATE_DATA_SIZE(CONFIG_BLOCK_MEMORY_ALLOC_STEP);

        configStore = PanicUnlessMalloc(total_config_store_size);
        memset(configStore, 0, total_config_store_size);
        configStore->const_config_set = const_config_data;
        configStore->num_config_blocks = CONFIG_BLOCK_MEMORY_ALLOC_STEP;
        configStore->const_mode = const_mode;

        return config_store_success;
    }
    else
    {
        return config_store_error_dynamic_config_init;
    }
}


/******************************************************************************/
config_store_status_t ConfigStoreGetReadOnlyConfig(config_blk_id_t id, uint16 *config_data_size, const void **data)
{
    dynamic_config_status_t dynamic_config_status;
    int16 dynamic_store_data_size;

    if(!config_data_size || !data)
    {
        return config_store_error_wrong_param;
    }

    if(isConfigBlockInUse(id))
    {
        resetConfigDataPtrAndSize(data, config_data_size);

        return config_store_error_config_block_in_use;
    }

    if (!isConstConfigDataValid(id))
    {
        return config_store_error_config_block_not_found;
    }

    /* Check if there is any config data for the module in dynamic store */
    if (configStore->const_mode)
        dynamic_store_data_size = -1;
    else
        dynamic_store_data_size = DynamicConfigGetConfigSize(configStore->const_config_set , id);

    if(dynamic_store_data_size > 0) /* There is config block data in dynamic config store */
    {
        uint16* dynamic_config_data = (uint16*)malloc(sizeof(uint16) * dynamic_store_data_size);

        if(!dynamic_config_data || !addConfigBlock(id, dynamic_config_data, 0))
        {
            resetConfigDataPtrAndSize(data, config_data_size);
            if(dynamic_config_data)
            {
                free(dynamic_config_data);
            }
            return config_store_error_memory;
        }
        /* Retrieve data from dynamic config store */
        dynamic_config_status = DynamicConfigGetConfig(configStore->const_config_set, id, (void*)dynamic_config_data, dynamic_store_data_size);
        if(dynamic_config_status != dynamic_config_success)
        {
            removeConfigBlock(id);
            resetConfigDataPtrAndSize(data, config_data_size);
            return config_store_error_retrieving_dynamic_data;
        }
        *data = dynamic_config_data;
        *config_data_size = dynamic_store_data_size;
        return config_store_success;
    }
    else if(dynamic_store_data_size == 0) /* There is zero-length data block in dynamic config store for selected config block */
    {
        if(!addConfigBlock(id, NULL, 0))
        {
            return config_store_error_memory;
        }
        resetConfigDataPtrAndSize(data, config_data_size);
        return config_store_success;
    }
    else /* There is only const data for selected config block */
    {
        /* Second argument is NULL, because there's no dynamically allocated data for this config block */
        if(addConfigBlock(id, NULL, 0))
        {
            *data = GetConstConfigData(id, config_data_size);
            return config_store_success;
        }
        else
        {
            resetConfigDataPtrAndSize(data, config_data_size);
            return config_store_error_memory;
        }
    }
}


/******************************************************************************/
config_store_status_t ConfigStoreGetWriteableConfig(config_blk_id_t id, uint16 *config_data_size, void **data)
{
    int16 ro_data_size;
    const void * const_data = NULL;
    bool data_in_dynamic_store = FALSE;

    if(!config_data_size || !data)
    {
        if(data)
        {
            *data = NULL;
        }
        return config_store_error_wrong_param;
    }

    if (!isConstConfigDataValid(id))
    {
        return config_store_error_config_block_not_found;
    }

    if(isConfigBlockInUse(id))
    {
        *data = NULL;
        return config_store_error_config_block_in_use;
    }
    else
    {
        uint16 size_of_data_to_copy;

        /* Check if there is a config data for the module in dynamic store */
        if (configStore->const_mode)
            ro_data_size = -1;
        else            
            ro_data_size = DynamicConfigGetConfigSize(configStore->const_config_set , id);

        if(ro_data_size == -1) /* There is only const data for the given config block *//* There is only const data for the given config block */
        {
            const_data = GetConstConfigData(id, (uint16*)(&ro_data_size));
        }
        else if(ro_data_size == 0) /* There is zero-size config block in dynamic config store */
        {
            if(*config_data_size == 0)
            {
                *data = NULL;
                /* Set the config block as being in use */
                if(!addConfigBlock(id, *data, *config_data_size))
                {
                    return config_store_error_memory;
                }
                else
                {
                    return config_store_success;
                }
            }
        }
        else
        {
            data_in_dynamic_store = TRUE;
        }

        if(*config_data_size == 0)
        {
            /* Config data size has not been provided, so we should set the size of data to copy to the size of RO data */
            size_of_data_to_copy = ro_data_size;
            *config_data_size = ro_data_size;
        }
        else
        {
            /* Config data size has been provided.
            We have to make sure that copied RO data size isn't bigger than the buffer size.
            If the RO data size is smaller or equal to the buffer size,
            then the size of data to copy into the buffer will be equal to the RO data size.*/
            size_of_data_to_copy = SMALLER_VALUE((uint16)ro_data_size, *config_data_size);
        }

        *data = malloc((BIGER_VALUE(*config_data_size, (uint16)ro_data_size)) * sizeof(uint16));
        if(!(*data))
        {
            return config_store_error_memory;
        }
        /* Copy RO data to the buffer */
        if(data_in_dynamic_store)
        {
            dynamic_config_status_t status;
            status = DynamicConfigGetConfig(configStore->const_config_set, id, *data, (uint16)ro_data_size);

            if(status != dynamic_config_success)
            {
                free(*data);
                *data = NULL;

                return config_store_error_retrieving_dynamic_data;
            }
        }
        else
        {
            memcpy(*data, const_data, (size_of_data_to_copy * sizeof(uint16)));
        }

        /* Set the config block as being in use */
        if(!addConfigBlock(id, *data, *config_data_size))
        {
            free(*data);
            *data = NULL;
            return config_store_error_memory;
        }
        else
        {
            return config_store_success;
        }
    }
}


/******************************************************************************/
config_store_status_t ConfigStoreWriteConfig(config_blk_id_t id)
{
    config_block_t* config_block;

    config_block = getConfigBlock(id);

    if(!config_block)
    {
        return config_store_error_config_block_not_found;
    }

    if(!config_block->writeable_config_data_size)
    {
        return config_store_error_config_not_writeable;
    }
    else if (configStore->const_mode)
    {
        return config_store_success;
    }
    else
    {
        dynamic_config_status_t status;
        const void * const_config_data;
        uint16       const_config_data_size;

        const_config_data = GetConstConfigData(id, &const_config_data_size);

        /*Compare writeable data with const data*/
        if((config_block->writeable_config_data_size == const_config_data_size) &&
           (!memcmp (config_block->dynamic_config_data, const_config_data, const_config_data_size * sizeof(uint16))))
        {
            status = DynamicConfigRemoveConfig(configStore->const_config_set, id);
        }
        else
        {
            status = DynamicConfigWriteConfig(configStore->const_config_set, id,
                                              config_block->dynamic_config_data, config_block->writeable_config_data_size);
        }

        if(status != dynamic_config_success)
        {
            return config_store_error_writing_dynamic_data;
        }
        else
        {
            return config_store_success;
        }
    }
}


/******************************************************************************/
void ConfigStoreReleaseConfig(config_blk_id_t id)
{
    removeConfigBlock(id);
}


/******************************************************************************/
config_store_status_t ConfigStoreRemoveConfig(config_blk_id_t id)
{
    if(isConfigBlockInUse(id))
    {
        return config_store_error_config_block_in_use;
    }

    if (!isConstConfigDataValid(id))
    {
        return config_store_error_config_block_not_found;
    }

    if (!configStore->const_mode
        && dynamic_config_success != DynamicConfigWriteConfig(configStore->const_config_set, id, NULL, 0))
    {
        return config_store_error_removing_data;
    }

    return config_store_success;
}

/******************************************************************************/
config_store_status_t ConfigStoreGetConfigMetadata(const config_store_meta_data_t **metadata)
{
    const config_blk_data_t *config_blk_data;

    if(!metadata)
    {
        return config_store_error_wrong_param;
    }
    
    if (!configStore || !configStore->const_config_set)
    {
        return config_store_error_memory;
    }
    
    config_blk_data = GET_CONFIG_SET_METADATA_PTR(configStore->const_config_set);

    /* Sanity check for config block data entry */
    CONFIG_STORE_ASSERT(GET_TYPE_FROM_TLV(config_blk_data) == CONFIG_DATA_CONFIG_SET_METADATA_TYPE);

    *metadata = (config_store_meta_data_t *)&config_blk_data->value[0];
    return config_store_success;
}

/******************************************************************************/
config_store_status_t ConfigStoreResetToDefaults(void)
{
    if (configStore->const_mode
        && dynamic_config_success == DynamicConfigResetToDefaults())
    {
        return config_store_success;
    }

    return config_store_error_removing_data;
}

/******************************************************************************/
config_store_status_t ConfigStoreDisableConstMode(void)
{
    configStore->const_mode = FALSE;
    
    return config_store_success;
}

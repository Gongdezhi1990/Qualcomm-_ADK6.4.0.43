/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    config_store.h
 
DESCRIPTION
    The main purpose of the ConfigStore library is to extract
    where the configuration data is stored so that any applications using it
    just get or set configuration data without having explicit knowledge
    of how or where the data is stored.
*/
 
 
/*!
@file    <module_name>.h
@brief   The main purpose of the ConfigStore library is to extract
         where the configuration data is stored so that any applications using it
         just get or set configuration data without having explicit knowledge
         of how or where the data is stored. To enable generic operation of the
         ConfigStore library, the concept of config blocks are used
         The ConfigStore library has no knowledge of what the configuration data is
         and that is only known by the owner of a specific config block.
*/


#ifndef _CONFIG_STORE_H_
#define _CONFIG_STORE_H_

#include <csrtypes.h>


/*!
    @brief Type used as config block identifier.

    Every config block has its own unique identifier,
    assigned during the build time.
*/
typedef uint16 config_blk_id_t;


/*!
    @brief Config Store operation status codes

    Defines all possible status codes that may be returned
    by Config Store public API functions.
 */
typedef enum
{
    config_store_success = 0,                   /*! Operation Succeeded */
    config_store_error_dynamic_config_init,     /*! Initialization of Dynamic Config failed */
    config_store_error_config_block_in_use,     /*! The requested config block is currently being used */
    config_store_error_memory,                  /*! Not enough memory */
    config_store_error_retrieving_dynamic_data, /*! Config data couldn't be retrieved from dynamic store */
    config_store_error_writing_dynamic_data,    /*! Config data couldn't be written to dynamic store */
    config_store_error_wrong_param,             /*! Parameter(s) passed to the library are wrong */
    config_store_error_config_block_not_found,  /*! Config block is not available */
    config_store_error_config_not_writeable,    /*! Config block is not writeable */
    config_store_error_removing_data            /*! Config block couldn't be removed */

} config_store_status_t;

/*!
    @brief Configuration metadata.

    Useful information about the configuration store and definition file that
    is required by the wider system, or sent to the Host.
*/
typedef struct
{
    uint16 signature[16];       /**< 128bit MD5 hash of the config definition file as an ASCII hex string.
                                     Packed as 2 bytes per uint16. */
    uint32 file_size;           /**< Size of the compressed file on-device. */
    uint16 filename_length;     /**< Equivalent to strlen() of the filename. */
    uint16 filename[1];         /**< On-device path to the compressed config definition.
                                     Packed as 2 bytes per uint16. */
} config_store_meta_data_t;

/*!
    @brief Initialization of the Config Store library

    During the initialization, appropriate dynamic store module is initialized as well.
    This function should be called before using any other Config Store functions.

    If const_mode is TRUE the ConfigStore will be started in "const mode". In this mode
    it will not read or write data from or to the dynamic config store. Only the values
    in the const_config_data will be returned to the application. Const mode must only 
    be used when trying out an upgraded application before the upgrade is committed.
    
    If started in const mode, ConfigStore will stay in that mode until 
    ConfigStoreDisableConstMode is called.

    @param const_config_data       Pointer to the config definition data.
    @param const_config_data_size  Config definition data size.
    @param const_mode              If TRUE, start the ConfigStore in const mode.
    @return                        Status of the initialization, error code if operation failed.
*/
config_store_status_t ConfigStoreInit(const void *const_config_data, uint16 const_config_data_size, bool const_mode);


/*!
    @brief Receiving read only config block's data.

    @param id                Config block identifier.
    @param config_data_size  Size of received data block (in words), filled inside the function.
    @param data              Pointer to received config data.
    @return                  Status of the operation, error code if operation failed.
*/
config_store_status_t ConfigStoreGetReadOnlyConfig(config_blk_id_t id, uint16 *config_data_size, const void **data);


/*!
    @brief Obtaining pointer to the writable config block.

    @param id                Config block identifier.
    @param config_data_size  Pointer to the size of the requested writeable config block (in words).
                             If it points to zero then this function will set it to the size of the RO config data.
    @param data              Pointer to writeable config data.
    @return                  Status of the operation, error code if operation failed.
*/
config_store_status_t ConfigStoreGetWriteableConfig(config_blk_id_t id, uint16 *config_data_size, void **data);


/*!
    @brief Writing configuration block to the dynamic store.

    First we have to call ConfigStore_GetWriteableConfig,
    to obtain pointer to writeable config. After making changes to it,
    calling this function actually writes all changes to the dynamic store.

    If the ConfigStore is in const mode this function will not write any
    changes to the dynamic store, but will still return config_store_success

    @param id  Config block identifier.
    @return    Status of the operation, appropriate error code if operation fails.
*/
config_store_status_t ConfigStoreWriteConfig(config_blk_id_t id);


/*!
    @brief This function releases the previously requested config block.

    Config block can be requested either by ConfigStore_GetReadOnlyConfig
    function or ConfigStore_GetWriteableConfig function.
    Each block has to be released before it can be requested again.

    @param id  Config block identifier.
*/
void ConfigStoreReleaseConfig(config_blk_id_t id);


/*!
    @brief This function removes config data for the selected config block.
           Removing a config block makes it a zero-length config block,
           and overrides any default data in the const config.

    If the ConfigStore is in const mode this function will not remove the
    config block, but will still return config_store_success.

    @param id  Config block identifier.

    @return Status of the operation, appropriate error code if operation fails.
*/
config_store_status_t ConfigStoreRemoveConfig(config_blk_id_t id);


/*!
    @brief Retrieve the metadata for the entire config set.
    
    @param metadata Pointer to the config metadata.

    @return Status of the operation, appropriate error code if operation fails.
*/
config_store_status_t ConfigStoreGetConfigMetadata(const config_store_meta_data_t **metadata);

/*!
    @brief Reset all config blocks to their default values.

    This function should only be called if ConfigStoreInit was called with
    const_mode == TRUE.

    All config blocks are reset to their default values in the const config
    data by removing any data in the dynamic store.

    If this function is called when the config store is not in const mode
    an error will be returned.

    @return Status of the operation, appropriate error code if operation fails.
*/
config_store_status_t ConfigStoreResetToDefaults(void);

/*!
    @brief Disable const mode in the ConfigStore.
    
    If the ConfigStore is not in const mode this function has no effect.
    
    If the ConfigStore is in const mode then after this function has been 
    called, all reads or writes from or to a config block will revert back
    to using the values in the dynamic store in preference to the const values.
*/
config_store_status_t ConfigStoreDisableConstMode(void);

#endif /* _CONFIG_STORE_H_*/

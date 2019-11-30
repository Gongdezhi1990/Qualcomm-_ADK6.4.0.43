/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    dynamic_config_if.h
 
DESCRIPTION
    This file provides documentation for the Dynamic Config Interface.
    The Config Store will call directly into a dynamic configuration library,
    and therefore each dynamic configuration library must implement
    the following interface.
*/
 
 
/*!
@file    dynamic_config_if.h

@brief   This file provides documentation for the Dynamic Config Interface.
         The Config Store will call directly into a dynamic configuration library,
         and therefore each dynamic configuration library must implement
         the following interface.
*/



#ifndef _DYNAMIC_CONFIG_IF_H_
#define _DYNAMIC_CONFIG_IF_H_

#include <csrtypes.h>
#include <config_store.h>



/*!
    @brief Dynamic Config operation status codes. It defines all possible status codes 
           that may be returned by Dynamic Config public interface.
 */
typedef enum
{
    dynamic_config_success = 0,     /*! Operation succeeded */
    dynamic_config_error_init,      /*! Initialization failed */
    dynamic_config_error_retrieve,  /*! Error retrieving data from the dynamic storage */
    dynamic_config_error_write,     /*! Error writing data into the dynamic storage */
    dynamic_config_error_removing   /*! Error removing data from the dynamic storage */
 
} dynamic_config_status_t;


/*!
    @brief Initialization of the Dynamic Config interface. This function is responsible for initializing 
           appropriate Dynamic Config Store library.
    
    @param const_config_data       Pointer to the config definition data.
    
    @return                        Status of the initialization.
*/
dynamic_config_status_t DynamicConfigInit(const void *const_config_data);


/*!
    @brief Retrieving config data from dynamic store
    
    @param const_config_data          Pointer to the config definition data.
    
    @param id                         Config block identifier.
    
    @param config_data                Buffer into which the config data will be written.
    
    @param requested_config_data_size Size of the config data buffer
    
    @return                           Status of the operation.
*/
dynamic_config_status_t DynamicConfigGetConfig(const void *const_config_data, config_blk_id_t id, 
                                               void *config_data, uint16 requested_config_data_size);


/*!
    @brief Writing config data into dynamic store

    @param const_config_data        Pointer to the config definition data.
    
    @param id                       Config block identifier.
    
    @param config_data              Buffer with config data to be written.
    
    @param config_data_size         Size of the config data that will be written into Dynamic Config Store(in bytes).
    
    @return                         Status of the operation.
*/
dynamic_config_status_t DynamicConfigWriteConfig(const void *const_config_data, config_blk_id_t id, 
                                                 void *config_data, uint16 config_data_size);


/*!
    @brief  Removing config block data kept in dynamic store.

    @param const_config_data        Pointer to the config definition data.
    
    @param id                       Config block identifier.
    
    @return                         Status of the operation.
*/
dynamic_config_status_t DynamicConfigRemoveConfig(const void *const_config_data, config_blk_id_t id);


/*!
    @brief  Retrieving the size of config block data kept in dynamic store.

    @param const_config_data        Pointer to the config definition data.
    
    @param id                       Config block identifier.
    
    @return                         '-1' if config block is not present,
                                    size of the config block (in bytes) otherwise.
*/
int16 DynamicConfigGetConfigSize(const void *const_config_data, config_blk_id_t id);

/*!
    @brief  Remove all the config block data kept in the dynamic store.
    
    @return                         Status of the operation.
*/
dynamic_config_status_t DynamicConfigResetToDefaults(void);

#endif /* _DYNAMIC_CONFIG_IF_H_ */

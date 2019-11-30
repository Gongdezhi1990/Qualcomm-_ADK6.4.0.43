/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    config_data.h
 
DESCRIPTION
    This file provides definitions and data types common to the Config Store 
    and Dynamic Config Store libraries.
*/
 
 
/*!
@file    config_data.h

@brief   This file provides definitions and data types common to the Config Store
         and Dynamic Config Store libraries. The configuration data will always be
         stored in the same format regardless of whether it is allocated in const 
         or dynamic memory. Since this data is accessed from multiple places,
         all the information relating to the data itself is defined within the 
         Config Data library. This library is the single place where all the required
         types are defined, as well as acting as a utility library for common functions
         relating to the data.
*/


#ifndef _CONFIG_DATA_H_
#define _CONFIG_DATA_H_


#include <csrtypes.h>


#define CONFIG_STORE_TYPE_BASE                  (0x1)
#define PS_DYNAMIC_CONFIG_STORE_TYPE_BASE       (0x8)


/* Config Store Generic Types */
#define CONFIG_DATA_CONFIG_SET_TYPE             (CONFIG_STORE_TYPE_BASE)
#define CONFIG_DATA_CONFIG_BLOCK_TYPE           (CONFIG_STORE_TYPE_BASE + 1)
#define CONFIG_DATA_CONFIG_BLOCK_DATA_TYPE      (CONFIG_STORE_TYPE_BASE + 2)

/* Config Store Metadata Types */
#define CONFIG_DATA_CONFIG_SET_METADATA_TYPE    (CONFIG_STORE_TYPE_BASE + 3)

/* PS Key Dynamic Store Types */
#define CONFIG_DATA_PS_CONFIG_BLOCK_DATA_TYPE   (PS_DYNAMIC_CONFIG_STORE_TYPE_BASE)
#define CONFIG_DATA_PS_CONFIG_SET_DATA_TYPE     (PS_DYNAMIC_CONFIG_STORE_TYPE_BASE + 1)


/* Generic structure for Config Data TLV(Type Length Value) */
typedef struct{
    uint16 type_and_length;
    uint16 value[1];
} config_data_tlv_t;


/* TLVs used by Config Store and Dynamic Config Store libraries */
typedef config_data_tlv_t config_set_t;             /* Meaning of TLV fields: Config Set Type, Config Set size, Config Blocks */
typedef config_data_tlv_t config_blk_t;             /* Meaning of TLV fields: Config Block Type, Config Block size, Config Block data */
typedef config_data_tlv_t config_blk_data_t;        /* Meaning of TLV fields: Config Block Data Type, Config Block Data size, Const data */
typedef config_data_tlv_t config_set_metadata_t;    /* Meaning of TLV fields: Config Set Metadata Type, Config Set Metadata size, Const data */
typedef config_data_tlv_t config_ps_blk_data_t;     /* Meaning of TLV fields: PS Config Block Data Type, PS Config Block Data size, PS key in which data is kept */

/* Macros used for extracting Type and length values which are kept in one word */
#define GET_TYPE_FROM_TLV(x)    (uint8)(((*((const uint16*)x)) >> 12) & 0x000F)
#define GET_LENGTH_FROM_TLV(x)  (uint16)((*((const uint16*)x)) & 0x0FFF)
#define GET_TLV_SIZE(x)         (uint16)(GET_LENGTH_FROM_TLV(x) + 1)

/* Offsets for different TLV structures */
#define CONFIG_SET_METADATA_OFFSET                                  (1)
#define CONFIG_BLOCK_OFFSET                                         (0)
#define CONFIG_BLOCK_DATA_OFFSET                                    (1)
#define PS_CONFIG_BLOCK_DATA_OFFSET(config_block_data_length)       (2 + (config_block_data_length))

/* Macros for obtaining pointers to different TLVs in Config Definition */
#define GET_CONFIG_SET_METADATA_PTR(const_config_data)                               ((const config_blk_data_t*)&((uint16*)const_config_data)[CONFIG_SET_METADATA_OFFSET])
#define GET_CONFIG_BLOCK_PTR(const_config_data, id)                                  ((const config_blk_data_t*)&((uint16*)const_config_data)[id+CONFIG_BLOCK_OFFSET])
#define GET_CONFIG_BLOCK_DATA_PTR(const_config_data, id)                             ((const config_blk_data_t*)&((uint16*)const_config_data)[id+CONFIG_BLOCK_DATA_OFFSET])
#define GET_PS_CONFIG_BLOCK_DATA_PTR(const_config_data, id, config_blk_data)         ((const config_ps_blk_data_t*)&((uint16*)const_config_data) \
                                                                                      [id+PS_CONFIG_BLOCK_DATA_OFFSET(GET_LENGTH_FROM_TLV(config_blk_data))])
#define GET_PS_CONFIG_SET_DATA_PTR(const_config_data)                                ((const config_blk_data_t*)&((uint16*)const_config_data)[ CONFIG_SET_METADATA_OFFSET +\
                                                                                       GET_TLV_SIZE(GET_CONFIG_SET_METADATA_PTR(const_config_data))])



#endif /* _CONFIG_DATA_H_ */

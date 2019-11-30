/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    image_upgrade_if.h

CONTAINS
    Definitions for the upgrading the images during DFU.

DESCRIPTION
    This file is seen by the stack, and VM applications, and
    contains things that are common between them.

*/

#ifndef __IMAGE_UPGRADE_IF_H__
#define __IMAGE_UPGRADE_IF_H__

/*! @brief Image section in the Image Upgrade traps.
    The set of possible image sections in QSPI device.
*/
typedef enum
{
    /** Nonce image section. */
    IMAGE_SECTION_NONCE                        = 0,

    /** Apps P0 image header image section. */
    IMAGE_SECTION_APPS_P0_HEADER               = 1,

    /** Apps P1 image header image section. */
    IMAGE_SECTION_APPS_P1_HEADER               = 2,

    /** Audio image header image section. */
    IMAGE_SECTION_AUDIO_HEADER                 = 3,

    /** Curator file system image section. */
    IMAGE_SECTION_CURATOR_FILESYSTEM           = 4,

    /** Apps P0 executable image section. */
    IMAGE_SECTION_APPS_P0_IMAGE                = 5,

    /** Apps read-only config file system image section. */
    IMAGE_SECTION_APPS_RO_CONFIG_FILESYSTEM    = 6,

    /** Apps read-only file system image section. */
    IMAGE_SECTION_APPS_RO_FILESYSTEM           = 7,

    /** Apps read-write file system image section. */
    /* NOTE: this image section cannot be upgraded using image upgrade
     * traps. 
     * Apps P0 will copy this image section from the boot image bank
     * to the other image bank whilst handling the ImageUpgradeSwapTry() trap.
     */
    IMAGE_SECTION_APPS_RW_FILESYSTEM           = 8,

    /** Apps P1 executable image section. */
    IMAGE_SECTION_APPS_P1_IMAGE                = 9,

    /** Apps device read-only file system table image section. */
    IMAGE_SECTION_APPS_DEVICE_RO_FILESYSTEM    = 0xa,

    /** Apps device read-only config table image section. */
    /* NOTE: this image section cannot be upgraded using image upgrade
     * traps. 
     * Apps P0 will copy this image section from the boot image bank
     * to the other image bank whilst handling the ImageUpgradeSwapTry() trap.
     */
    IMAGE_SECTION_APPS_RW_CONFIG               = 0xb,
    
    /** Audio executable image section. */
    IMAGE_SECTION_AUDIO_IMAGE                  = 0xc,

    /** Maximum value of the image section. */
    IMAGE_SECTION_ID_MAX                       = 0xd
}image_section_id;

/*! @brief Keys used with ImageUpgradeGetInfo() */
typedef enum
{
    IMAGE_SIZE       = 0  /*!< size of image section in bytes(8bit!) */
} image_info_key;

/*! @brief Algorithm in the image upgrade hash traps.
    The set of possible algorithms for image upgrade hash traps.
*/
typedef enum
{
    /*!< SHA256 algorithm. */
    SHA256_ALGORITHM                        = 0
}hash_algo_t;

/* Hash context pointer in the image upgrade hash traps.
 */
typedef void * hash_context_t;


#endif /* __IMAGE_UPGRADE_IF_H__ */

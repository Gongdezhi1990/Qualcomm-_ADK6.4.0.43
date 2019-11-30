/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
        file_if.h  -  FILE interface

CONTAINS
        The GLOBAL definitions for the FILE subsystem from the VM

DESCRIPTION
    This file is seen by the stack, and VM applications, and
    contains things that are common between them.
*/

#ifndef __APP_FILE_IF_H__
#define __APP_FILE_IF_H__

typedef uint16 FILE_INDEX;  /*!< File index type. */

#define FILE_NONE 0 /*!< No such file. */
#define FILE_ROOT 1 /*!< Root directory. */

/*! Indicates the entire file should be mapped.
  For passing to the FileMap size argument. */
#define FILE_MAP_SIZE_ALL ((uint32)0xfffffffful)

/*!
    @brief The type of file.
*/
typedef enum
{
    FILE_TYPE_ERROR,           /*!< An error has occured.*/
    FILE_TYPE_DIRECTORY,       /*!< This is a directory within the filesystem.*/
    FILE_TYPE_NARROW_FILE,     /*!< A narrow (8 bit wide) file.*/
    FILE_TYPE_WIDE_FILE        /*!< A wide (16 bit wide) file.*/
} FILE_TYPE;

/*!
  @brief Identifier for a filesystem.
 */
typedef enum
{
    FILESYSTEM_ID_CFG_RO_FS,    /*!< Read only config. */
    FILESYSTEM_ID_DEVICE_RO_FS, /*!< Device read only config. */
    FILESYSTEM_ID_RO_FS,        /*!< Read only filesystem. */
    FILESYSTEM_ID_RW_CONFIG,    /*!< Read write config. */
    FILESYSTEM_ID_RW_FS,        /*!< Read write filesystem. */
    FILESYSTEM_ID_APPS_P1       /*!< Apps P1. */
} FILESYSTEM_ID;

#endif

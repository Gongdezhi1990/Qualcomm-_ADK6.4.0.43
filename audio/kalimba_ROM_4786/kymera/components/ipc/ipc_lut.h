/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/** \file
 *
 *  This is the main public project header for the \c ipc LUT library.
 *
 */
/****************************************************************************
Include Files
*/

#ifndef IPC_LUT_H
#define IPC_LUT_H

#include "ipc_status.h"

/****************************************************************************
Public Type Declarations
*/

/*
 * This lookup table ptr structure maintained by all processors
 * used to populate and access the shared lookup table.
 * SLT in P0 points to ipc_lookup_table_ptr of P0 which points to
 * the shared lookup table. P1 access the shared lookup table
 * address through SLT and populates its ipc_lookup_table_ptr
 * during initialisation.
 * Once the shared memory is initialised, it also sets the valid flag.
 * P0 access like -> ipc_lookup_table_ptr ->  shared lookup table
 * P1 Initialised its ipc_loopkup_table_ptr like
 * P1 -> P0's SLT -> P0's ipc_lookup_table_ptr -> shared lookup table
 * and copy it and populate the addess on its ipc_lookup_table_ptr.
 *
 * Note: This 2-level dereferencing is to avoid direct stomping of monolithic
 * images, which should know what exactly it is doing. Also it will keep
 * a local ptr to the shared table at initialisation.
 */
typedef struct
{
    bool valid;
    void *lookup_table;

} ipc_lookup_table_ptr_struct;

extern ipc_lookup_table_ptr_struct ipc_lookup_table_ptr;

/*
 * Enumeration of (external) keys for the IPC LUT. To add
 * new keys, make sure that 'IPC_LUT_ID_EXT_LOOKUP_TABLE_END'
 * remains at the end of the table and has the highest numeric
 * value. NOTE: the enum values consist of two bytes: the
 * bottom byte is (effectively) an array index, the top byte
 * is an identifier; together, the two bytes must be unique.
 * Enum 'IPC_LUT_ID_UNUSED_SHARED_ADDR' is not used (at the
 * time of writing this) and can be used as a free entry.
 */
typedef enum
{
    IPC_LUT_ID_MMU_SHARED_ADDR          = 0xAA0D,
    IPC_LUT_ID_BAC_MONITORS_SHARED_ADDR = 0xAA0E,
    IPC_LUT_ID_HEAP_CONFIG_SHARED_ADDR  = 0xAA0F,
    IPC_LUT_ID_AUDIO_STATUS_SHARED_ADDR = 0xAA10,
    IPC_LUT_ID_AUDIO_CONFIG_SHARED_ADDR = 0xAA11,
    IPC_LUT_ID_DATABASE_DOWNLOAD_LIST   = 0xAA12,
    IPC_LUT_ID_CAP_DOWNLOAD_KCS_DB      = 0xAA13,
    IPC_LUT_ID_EXPORTED_CONSTANTS       = 0xAA14,
    IPC_LUT_ID_UNUSED_SHARED_ADDR_2     = 0xAA15,
    IPC_LUT_ID_EXT_LOOKUP_TABLE_END     = 0xAA16
} IPC_LUT_EXT_ID;

/*
 * LUT address enumeration entries (IPC_LUT_EXT_ID above) 
 * are also used as diatribe in 'panic_diatribe' if a LUT 
 * access error occurs. There are also a few occations 
 * where a LUT access error occurs that is not related to
 * a specific location in the LUT. These are specified 
 * below. Useful to have a unique identifier w.r.t. the
 * IPC_LUT_EXT_ID (so it can be indentied quickly), but 
 * not a problem if not.
 */
typedef enum
{
    IPC_LUT_ERROR_LUT_NOT_PRESENT       = 0xDEAD,
    IPC_LUT_ERROR_CONFIG_NULL           = 0x0666
} IPC_LUT_ERROR_ID;

/****************************************************************************
Public Constant and macros
*/

/****************************************************************************
Public Variable Declarations
*/

/****************************************************************************
Public Function Declarations
*/

/**
 * \brief  get the shared static address from the lookup table
 *
 * \param[in] key               - The key defined in IPC_LUT_EXT_ID
 * \param[out] addr             - The address associated with key in the (key, address) pair in the IPC LUT
 *
 * \return  IPC_SUCCESS if lookup address was found, any other return value indicates an error.
 */
extern IPC_STATUS ipc_get_lookup_addr(IPC_LUT_EXT_ID key, uintptr_t *addr);

/**
 * \brief  set the shared static address into the lookup table
 *
 * \param[in] key               - The key defined in IPC_LUT_EXT_ID
 * \param[in] addr              - The address associated with key, to set in the (key, address) pair in the IPC LUT
 *
 * \return  IPC_SUCCESS if the address was successfully entered in the IPC LUT for the specified key,
 *          any other return value indicates an error.
 */
extern IPC_STATUS ipc_set_lookup_addr(IPC_LUT_EXT_ID key, uintptr_t addr);

#endif /* IPC_LUT_H */

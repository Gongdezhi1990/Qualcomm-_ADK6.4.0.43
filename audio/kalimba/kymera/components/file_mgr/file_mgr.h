/****************************************************************************
 * Copyright (c) 2017 - 2019 Qualcomm Technologies International, Ltd 
****************************************************************************/
/**
 * \defgroup file_mgr file manager
 *
 * \file file_mgr.h
 * \ingroup file_mgr
 *
 * Public definitions for file_mgr
 */
#ifdef INSTALL_FILE_MGR
#ifndef _FILE_MGR_H_
#define _FILE_MGR_H_

/****************************************************************************
Include Files
*/
#include "file_mgr_for_ops.h"
#include "accmd_prim.h"

/**
 * Definition of callback type. Accmd callback prototype
 *
 * \param con_id The undecoded PDU
 * \param status Audio status code
 */
typedef bool (*FILE_MGR_ALLOC_CBACK)(unsigned con_id, unsigned status, uint16 file_id);
typedef bool (*FILE_MGR_DEALLOC_CBACK)(unsigned con_id, unsigned status);

/**
 *
 * \brief Allocates a buffer of a requested size for a data file and calls
 *        accmd response callback
 *
 * \param con_id     Connection id
 * \param type       Type of file
 * \param auto_free  Free the buffer internally after first use.
 * \param file_size  Size of buffer to be reserved for the file (in octets)
 * \param cback      callback
 */
extern void file_mgr_accmd_alloc_file(unsigned con_id, 
                                      ACCMD_TYPE_OF_FILE type,
                                      bool auto_free, 
                                      uint32 file_size, 
                                      FILE_MGR_ALLOC_CBACK cback);

/**
 *
 * \brief Deallocates a buffer of a requested size for a data file and calls
 *        accmd response callback
 *
 * \param con_id     Connection id
 * \param file_id  ID of the file to be removed)
 * \param cback      callback
 */
extern void file_mgr_accmd_dealloc_file(unsigned con_id, uint16 file_id,
        FILE_MGR_DEALLOC_CBACK cback);


/**
 * \brief Initialises and array of files. Size of the array is a configuration
 * option
 */
void file_mgr_init(void);

/*Macro Definitions*/
#define FILE_MGR_INVALID_FILE_ID      0

#endif /*_FILE_MGR_H_*/
#endif /* INSTALL_FILE_MGR */

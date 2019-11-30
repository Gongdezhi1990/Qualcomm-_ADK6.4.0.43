/****************************************************************************
 * Copyright (c) 2017 - 2019 Qualcomm Technologies International, Ltd 
****************************************************************************/
/**
 * \defgroup file_mgr file manager
 *
 * \file file_mgr_for_ops.h
 * \ingroup file_mgr
 *
 * Public definitions for file_mgr
 */
#ifdef INSTALL_FILE_MGR
#ifndef _FILE_MGR_FOR_OPS_H_
#define _FILE_MGR_FOR_OPS_H_

/****************************************************************************
Include Files
*/
#include "hydra_types.h"
#include "buffer/cbuffer_c.h"

/* Struct with information about the data file */
typedef struct
{
    uint16      type;
    bool        valid;
    tCbuffer    *file_data;
}DATA_FILE;

/* The callback function definition for transfer complete */
typedef void (*FILE_MGR_EOF_CBACK)(bool);


void file_mgr_task_init(void **data);
/**
 *
 * \brief Finds and returns a file of a given ID
 *
 * \param file_id  ID of the file
 *
 * \return pointer to a file
 */
extern DATA_FILE * file_mgr_get_file(uint16 file_id);


/**
 *
 * \brief release the file back for auto free files 
 *
 * \param file_id  ID of the file
 *
 * \return
 */
extern void file_mgr_release_file(uint16 file_id);

/**     
 *                               
 * \brief The audio data service calls this API once transfer is 
 *        done
 *  
 * \param file     The file transfered
 * \param file_id  ID of the file to be removed)
 * \param cback      callback
 **/
extern void file_mgr_transfer_done ( DATA_FILE *file, bool status,
                                   FILE_MGR_EOF_CBACK cback);


#endif /*_FILE_MGR_FOR_OPS_H_*/
#endif /* INSTALL_FILE_MGR */

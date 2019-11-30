
/****************************************************************************
 * Copyright (c) 2017 - 2019 Qualcomm Technologies International, Ltd 
****************************************************************************/
/**
 * \defgroup file_mgr file manager
 *
 * \file file_mgr_private.h
 * \ingroup file_mgr
 *
 * Private definitions for file_mgr
 */
#ifdef INSTALL_FILE_MGR
#ifndef _FILE_MGR_PRIVATE_H_
#define _FILE_MGR_PRIVATE_H_

/****************************************************************************
Include Files
*/
#include "status_prim.h"
#include "hydra_log/hydra_log.h"
#include "fault/fault.h"
#include "hal/hal_dm_sections.h"
#include "pmalloc/pl_malloc.h"
#include "sched_oxygen/sched_oxygen.h"
#include "patch/patch.h"
#include "pl_assert.h"
#include "buffer.h"
#include "file_mgr.h"

/*Macro Definitions*/

/* base file ID */
#define FILE_MGR_AUTO_MASK          0x0F00
#define FILE_MGR_BASE_MASK          0xF000
#define FILE_MGR_BASE_ID            (FILE_MGR_BASE_MASK | FILE_MGR_AUTO_MASK)
#define FILE_MGR_INDEX_MASK         (~FILE_MGR_BASE_ID)

/* Max allowed file id */
#define FILE_MGR_MAX_ID             FILE_MGR_BASE_ID + FILE_MGR_MAX_NUMBER_OF_FILES
/* Convert an external file id to an file index in the file array */
#define FILE_ID_TO_INDEX(id)        (FILE_MGR_INDEX_MASK & id )
/* Convert a file index to an external file id */
#define INDEX_TO_NON_AUTO_FILE_ID(index)      (FILE_MGR_BASE_MASK   | index)
#define INDEX_TO_AUTO_FILE_ID(index) (FILE_MGR_AUTO_MASK | INDEX_TO_NON_AUTO_FILE_ID(index))

#define FILE_MGR_IS_AUTO_FREE(id) (((id) & FILE_MGR_AUTO_MASK) != 0)

/* Will round up and convert a number of octets to words */
#define OCTETS_TO_WORDS(x)          (x + (OCTETS_PER_SAMPLE - 1)) / OCTETS_PER_SAMPLE
/* Due to read/write pointers, Cbuffer needs to be 1 word larger than the
 * size of the file we need to store */
#define OCTETS_TO_CBUFFER_SIZE(x)   OCTETS_TO_WORDS(x) + 1

/* message_ids for put_message() */
typedef enum {
             FILE_MGR_EOF,
             FILE_MGR_ERROR
} file_mgr_message_id;


#endif /*_FILE_MGR_PRIVATE_H_*/
#endif /* INSTALL_FILE_MGR */

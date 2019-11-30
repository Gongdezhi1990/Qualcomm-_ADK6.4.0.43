/*****************************************************************************
*
* Copyright (c) 2017 - 2019 Qualcomm Technologies International, Ltd 
*
*****************************************************************************/
/**
 * \file file_mgr.c
 * \ingroup file_mgr
 */
#ifdef INSTALL_FILE_MGR
/****************************************************************************
Include Files
*/

#include "file_mgr_private.h"

/****************************************************************************
Private Variable Definitions
*/

/* hash table where array index is the file id */
static DM_P0_RW_ZI DATA_FILE *stored_files[FILE_MGR_MAX_NUMBER_OF_FILES];

/****************************************************************************
Private Function Definitions
*/
/**
 * \brief Checks if the provided file ID is an acceptable file ID.
 *
 *
 * \param file_id ID a file ID
 *
 * \return TRUE if the provided id is valid
 */
static bool file_mgr_is_valid_id(uint16 file_id)
{
    patch_fn_shared(file_mgr);
    /* Check if the id is in acceptable range */
    if (file_id > FILE_MGR_BASE_ID || file_id < FILE_MGR_MAX_ID)
    {
        /* check if the file has been allocated */
        if(stored_files[FILE_ID_TO_INDEX(file_id)] != NULL)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * \brief Frees allocated file memory.
 *
 *
 * \param file_idx file index
 *
 */
static void file_mgr_clean_up(uint16 file_idx)
{

    DATA_FILE *file = stored_files[file_idx];
    patch_fn_shared(file_mgr);

    /* pfree checks for null pointers so we do not have to worry*/
    if( file != NULL)
    {
        cbuffer_destroy(file->file_data);
        pdelete(file);
    }

    /* reset the pointers to NULL */
    stored_files[file_idx] = NULL;

}

/**
 *
 * \brief Allocates a buffer of a requested size for a data file
 *
 * \param file_size  Size of buffer to be reserved for the file (in octets)
 *
 * \return Audio Status code
 */
static uint16 file_mgr_alloc_file( uint32 file_size,
                            ACCMD_TYPE_OF_FILE type,
                            bool auto_free )
{
    uint16 file_idx, file_size_in_words;
    patch_fn_shared(file_mgr);

    for (file_idx = 0; file_idx < FILE_MGR_MAX_NUMBER_OF_FILES; file_idx++)
    {
        if (stored_files[file_idx] == NULL)
        {
            tCbuffer *buffer;
            DATA_FILE *file = NULL;

            /* convert size to words needed by the cbuffer */
            file_size_in_words = OCTETS_TO_CBUFFER_SIZE(file_size);

            /* allocate the file struct */
            stored_files[file_idx] = pnew(DATA_FILE);

            file = xpnew(DATA_FILE);
            if (file == NULL)
            {
                /* fail */
                fault_diatribe(FAULT_AUDIO_INSUFFICIENT_MEMORY,
                        sizeof(DATA_FILE));
                break;
            }

            /*  get the cbuffer */
            buffer = cbuffer_create_with_malloc( file_size_in_words,
                                                 BUF_DESC_SW_BUFFER);

            if (buffer == NULL)
            {
                fault_diatribe(FAULT_AUDIO_INSUFFICIENT_MEMORY, file_size);
                /* clean up and fail */
                pfree(file);
                break;
            }

            /* set the file type */
            file->file_data = buffer;
            file->type = type;
            file->valid = FALSE;

            stored_files[file_idx] = file;

            /* Success return file ID */
            return (auto_free)? INDEX_TO_AUTO_FILE_ID(file_idx):
                                INDEX_TO_NON_AUTO_FILE_ID(file_idx);
        }
    }
    /* file allocation failed */
    return FILE_MGR_INVALID_FILE_ID;
}

/**
 *
 * \brief Deallocates a buffer of a requested size for a data file
 *
 * \param file_id  ID of the file to be removed
 *
 * \return Audio Status code
 */
static bool file_mgr_dealloc_file(uint16 file_id)
{
    /* convert file id to file index */
    uint16 file_idx;
    patch_fn_shared(file_mgr);

    if (file_mgr_is_valid_id(file_id) == TRUE)
    {
        /* get file index*/
        file_idx = FILE_ID_TO_INDEX(file_id);

        /* free allocated buffers */
        file_mgr_clean_up(file_idx);
        return TRUE;
    }
    return FALSE;
}

/****************************************************************************
Public Function Definitions
*/

/* Reserve file for the file */
void file_mgr_accmd_alloc_file( unsigned con_id, 
                                ACCMD_TYPE_OF_FILE type,
                                bool auto_free,
                                uint32 file_size,
                                FILE_MGR_ALLOC_CBACK cback)
{
    uint16 file_id;
    unsigned status = STATUS_OK;
    patch_fn_shared(file_mgr);

    /* ACCMD style callback. It is used to return status and file_id*/
    PL_ASSERT(cback != NULL);

    /* allocate space for file*/
    file_id = file_mgr_alloc_file(file_size, type, auto_free);
    if (file_id == FILE_MGR_INVALID_FILE_ID)
    {
        status = STATUS_CMD_FAILED;
    }

    cback(con_id, status, file_id);
}

/* De-allocate the file */
void file_mgr_accmd_dealloc_file(unsigned con_id, uint16 file_id,
        FILE_MGR_DEALLOC_CBACK cback)
{
    unsigned status = STATUS_OK;
    patch_fn_shared(file_mgr);

    /* ACCMD style callback. It is used to return status*/
    PL_ASSERT(cback != NULL);

    /* allocate space for file*/
    if (file_mgr_dealloc_file(file_id) == FALSE)
    {
        /* There is no file with the requested ID */
        status = STATUS_INVALID_CMD_PARAMS;
    }

    cback(con_id, status);
}

/* Get the file using the file id */
DATA_FILE * file_mgr_get_file(uint16 file_id)
{
    patch_fn_shared(file_mgr);

    if (file_mgr_is_valid_id(file_id))
    {
        /*if the id has not been allocated this will be NULL*/
        return stored_files[FILE_ID_TO_INDEX(file_id)];
    }
    return NULL;
}

/* Operator calls this function to release file */
void file_mgr_release_file( uint16 file_id)
{
    patch_fn_shared(file_mgr);

    /* Release the file only if it isfile_id) an auto free one */
    if(FILE_MGR_IS_AUTO_FREE(file_id))
    {
        file_mgr_dealloc_file(file_id);
    }
}

/* The Audio data service calls this funnction once 
 * data has been downloaded
 */
void file_mgr_transfer_done(DATA_FILE *file, bool status, 
                           FILE_MGR_EOF_CBACK cback)
{
    uint16 msg_id;
    patch_fn_shared(file_mgr);

    PL_ASSERT(file != NULL );

    msg_id =  (status)? FILE_MGR_EOF:FILE_MGR_ERROR;
    /* update the file status here */
    file->valid = (status)? TRUE: FALSE;

    put_message(FILE_MGR_TASK_QUEUE_ID, msg_id,
                (void*)cback);
}
                           
/* Init function.  Doing nothing now. keeping it only for patching purpose */
void file_mgr_init(void)
{
    patch_fn_shared(file_mgr);
}

/**
 * \brief Dummy init function, needed by all static tasks
 **/
void file_mgr_task_init(void **data)
{
    patch_fn_shared(file_mgr);
}

/**
 *   \brief  handler for the messages in the task queue.
 *   \param  data pointer to the pointer to task data
 **/
void file_mgr_task_handler(void **data)
{

    file_mgr_message_id msg_id;
    void* msg_data;
    patch_fn_shared(file_mgr);

    while (get_message( FILE_MGR_TASK_QUEUE_ID, (uint16*)&msg_id, &msg_data))
    {
        switch (msg_id)
        {
            /* A file has been downloaded successfully */
             case FILE_MGR_EOF:
                {
                    FILE_MGR_EOF_CBACK callback = (FILE_MGR_EOF_CBACK) msg_data;
                    callback(TRUE);
                }
                break;

             case FILE_MGR_ERROR:
                {
                    FILE_MGR_EOF_CBACK callback = (FILE_MGR_EOF_CBACK) msg_data;
                    callback(FALSE);
                }
                break;
             default:
                break;
        }
    }
}


#endif /* INSTALL_FILE_MGR */

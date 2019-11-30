/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file 
 * \ingroup exp_const
 *
 * File manager for transfer of exported_constants.
 *
 * 
 */

#include "exported_constants_private.h"

#ifdef INSTALL_CAPABILITY_CONSTANT_EXPORT
#include "id/id.h"
#include "hydra/hydra.h"

/** 
 * Structure used to collate status information for file transfer.
 *
 * Transfer of a single file at a time is the only option.
 */
static struct
{
    bool                    in_use;
    ISP_ADDR                clientDataAddress;
    ISP_PORT_MAPPING_ENUM   clientDataPort;
    uint32                  session_id;
    uint32                  file_handle;
    uint32                  file_size_bytes;
    uint32                  bytes_left;
    uint8                  *data;
    uint8                  *data_next; 
    exp_const_mapping      *current_constant;
} transfer_state;

/****************************************************************************
Private Function Definitions
*/

/** Prepare the FTP file specification based on the file identifier.
 * 
 *  The other fields of the file specification are fixed
 *
 * \param spec      Pointer to the filespec to be completed
 * \param file_id   16 bit identifier to place in the filespec
 */
static void exp_const_name_spec(FTP_FILESPEC *spec,uint16 file_id)
{
    system_bus my_ssid = hydra_get_local_ssid();
    my_ssid = HYDRA_TRANSLATE_SSID_FOR_PEER(my_ssid, CURATOR_SSID);

    uint16 vsn = id_get_build_id();

    patch_fn(exp_const_file_id);

    /* Pack this into a filespec 
     * The auto-generated macros to _PACK or _MARSHAL don't actually compile */
    FILENAME_DATA_FW_VERSION_SET(spec, vsn);
    FILENAME_DATA_NUMBER_SET(spec, file_id);
    FILENAME_DATA_SUBSYSTEM_SET(spec, my_ssid);
    FILENAME_DATA_FORMAT_SET(spec, FILENAME_FORMAT_DATA);

}

/** 
 * Tidy up of a completed transfer (success or error).
 *
 * This handler is called when a transfer is ending, either after an 
 * error has been declared - or in the case of a normal completion.
 *
 * If neccesary an open file is closed and the session terminated, before
 * the completion message is sent to the exported_constants task.
 * File close and session close will end up calling this function again.
 */
static void handle_ended_transfer(void)
{
    if (transfer_state.file_handle)
    {
        ftp_ctrl_file_close(transfer_state.file_handle);
        return;
    }
    else if (transfer_state.session_id)
    {
        ftp_ctrl_session_destroy(transfer_state.session_id);
        return;
    }

    transfer_state.in_use = FALSE;
    transfer_state.current_constant = NULL;

    put_message(MEM_UTILS_QUEUE, EXP_CONST_MSG_TRANSFER_OVER_KICK_ID, NULL);
}

/** Tidy-up file transfer in the event of an error.
 *
 * The transfer has failed in some way so set an error code and free 
 * allocated RAM (if any) before calling handle_ended_transfer() which
 * will manage closure of the the file and session, if needed.
 *
 * \param error_state   The error status to set
 */
static void handle_aborting_transfer(exp_const_status error_state)
{
    transfer_state.current_constant->status = error_state;
    if (transfer_state.data)
    {
        pfree(transfer_state.data);
        transfer_state.data = NULL;
    }

    handle_ended_transfer();
}

/** 
 * Request more data from the file.
 *
 *  Request more data from the file server. Request is capped at 65535 (16-bit)
 *  so can theoretically can handle files larger than 64K.
 */
static void request_file_data(void)
{
    uint16 bytes_requested, bytes_to_request;

    bytes_to_request = MIN(transfer_state.bytes_left,0xfffful);
    bytes_requested = ftp_ctrl_file_read(transfer_state.file_handle, bytes_to_request);
    L3_DBG_MSG2("mem_util/exp_const: request read of %d bytes out of %d",
                                            bytes_requested, bytes_to_request);
    UNUSED(bytes_requested); /* Only accessed in debug */
}

/** 
 * Copy words from received file buffer.
 *
 * Helper function to copy from the fileserver buffer. The MMU buffer does not
 * support wrap so we have to copy word by word, there being a guarantee that the
 * buffer is on a word boundary 
 *
 * Note that the function doesn't explicitly handle the wrap. The fileserv adjusts
 * the offset correctly, but does not handle the case where offset + words wraps
 * to the start of the buffer.
 *
 * \param pdu       Pointer to the PDU received by the data callback function 
 * \param words     Number of words to copy
 * \param target    Pointer to the destination
 */
static void copy_words_with_wrap(const uint8 *pdu,uint16 words, uint16 *target)
{
    uint16 offset = 0;

    while (words--)
    {
        *target = fileserv_get_16_bit_word(pdu, offset);
        target++;
        offset += 2;
    }
}

/**
 * Handle signal in response to ftp_ctrl_session_create
 *
 * Check the staus of the session create and if successful open the required 
 * file.
 *
 * \param header    Pointer to message received by the control callback
 */
static void session_create_cfm(const uint16 *header)
{
    FTP_CTRL_SESSION_CREATE_CFM cfm;
    FTP_CTRL_SESSION_CREATE_CFM_UNMARSHALL(header, &cfm);

    L4_DBG_MSG1("mem_util/exp_const: session_create result %x",cfm.result);

    if (CSR_FTP_RESULT_SUCCESS != cfm.result)
    {
        handle_aborting_transfer(EXP_CONST_STATUS_ERROR_SESSION);
    }
    else
    {
        FTP_FILESPEC spec;
        uint16       file;

        (void)DynLoaderGetExternalRedirectFile(transfer_state.current_constant->const_in_ROM,&file);
        exp_const_name_spec(&spec,file);

        /* Keep the session id so we can close down later */
        transfer_state.session_id = cfm.session_id;

        L3_DBG_MSG3("mem_util/exp_const:send fopen sid:%d client:%d:%d",transfer_state.session_id,
                                                        transfer_state.clientDataAddress,
                                                        transfer_state.clientDataPort);
        ftp_ctrl_file_open( transfer_state.session_id, 
                            CSR_FTP_OPEN_MODE_OPEN_EXISTING,
                            spec,
                            transfer_state.clientDataAddress, 
                            transfer_state.clientDataPort);
    }
}

/**
 * handle signal in response to ftp_ctrl_file_open
 *
 * Check if the file opened successfully, record information about the file
 * and request file data.
 *
 * In case of error the transfer session is aborted.
 *
 * \param header    Pointer to message received by the control callback
 */
static void file_open_cfm(const uint16 *header)
{
    FTP_CTRL_FILE_OPEN_CFM  cfm;

    FTP_CTRL_FILE_OPEN_CFM_UNMARSHALL(header, &cfm);

    assert(transfer_state.in_use);

    L4_DBG_MSG2("mem_util/exp_const: file_open_cfm result %d (handle %x)",cfm.result,cfm.handle);

    if (CSR_FTP_RESULT_SUCCESS == cfm.result)
    {
        transfer_state.file_handle = cfm.handle;
        transfer_state.file_size_bytes = cfm.size;

        L3_DBG_MSG1("mem_util/exp_const: File size reported %u",transfer_state.file_size_bytes);

        if (transfer_state.file_size_bytes)
        {
            /* pmalloc appears to be the correct answer to get addresses that
             * are accesible on all processors. i.e. there is no need to state
             * a preference using ppmalloc. This means that operators on p1 
             * will use ram from p0.
             *
             * The memory allocated here is
             * either
             *     error: freed in handle_aborting_transfer
             * or
             *     success: stored in the constants table in file_read_cfm()
             *          and subsequently released in 
             *          exported_constants.c:free_unused_memory()
             */
            uint8 *p = (uint8 *)xpmalloc(transfer_state.file_size_bytes);
            if (!p)
            {
                handle_aborting_transfer(EXP_CONST_STATUS_ERROR_MEMORY);
            }
            else
            {
                /* Initialise the data transfer */
                transfer_state.data_next = transfer_state.data = p;
                transfer_state.bytes_left = transfer_state.file_size_bytes;
                transfer_state.current_constant->status = EXP_CONST_STATUS_LOADING;
                
                request_file_data();
            }
            return;
        }
    }
    else
    {
        uint16       failed_id;

        (void)DynLoaderGetExternalRedirectFile(transfer_state.current_constant->const_in_ROM,&failed_id);

        L1_DBG_MSG2("Failed to open file for constant %p (ID:%d)",transfer_state.current_constant->const_in_ROM,failed_id);
    }

    /* File not found, or a size of 0 (!). Permanent error for this file */
    handle_aborting_transfer(EXP_CONST_STATUS_ERROR);

}

/**
 * handle signal in response to ftp_ctrl_file_read
 *
 * If there was no error, check if the data transfer has completed and
 * if so start process of ending the file transfer session.
 *
 * In case of error the transfer session is aborted.
 *
 * \note Actual data is not transferred by this function. There is a separate
 *       data callback for this.
 *
 * \param header    Pointer to message received by the control callback
 */
static void file_read_cfm(const uint16* header)
{
    FTP_CTRL_FILE_READ_CFM cfm;
    FTP_CTRL_FILE_READ_CFM_UNMARSHALL(header, &cfm);
    
    assert(transfer_state.in_use);

    if (CSR_FTP_RESULT_SUCCESS != cfm.result)
    {
        handle_aborting_transfer(EXP_CONST_STATUS_ERROR_SESSION);
    }
    else
    {
        /* Data processing handled during the signal */
        if (transfer_state.bytes_left)
        {
            /* Request as much as we can */
            request_file_data();
        }
        else
        {
            L2_DBG_MSG("mem_util/exp_const: file read complete");
            transfer_state.current_constant->status = EXP_CONST_STATUS_IN_RAM;
            transfer_state.current_constant->ram_copy = transfer_state.data;
            transfer_state.data = NULL;
            handle_ended_transfer();
        }
    }
}

/**
 * handle signal in response to ftp_ctrl_file_close
 *
 * The only action we take is to continue terminating the file transfer session.
 * We don't want to treat failure to close as fatal.
 *
 * \param header    Pointer to message received by the control callback
 */
static void file_close_cfm(const uint16 *header)
{
    FTP_CTRL_FILE_CLOSE_CFM cfm;

    FTP_CTRL_FILE_CLOSE_CFM_UNMARSHALL(header, &cfm);

    assert(transfer_state.in_use);

    /* If we haven't closed the file, we don't want to try again */
    if (CSR_FTP_RESULT_SUCCESS != cfm.result)
    {
        L2_DBG_MSG2("mem_util/exp_const: File close error. Handle: 0x%x Sts: 0x%x",
                        transfer_state.file_handle,cfm.result);
    }
    transfer_state.file_handle=0;
    handle_ended_transfer();
}

/**
 * handle signal in response to ftp_ctrl_session_destroy
 *
 * We only report an error, if diagnostics are active
 *
 * \param header    Pointer to message received by the control callback
 */
static void session_destroy_cfm(const uint16 *header)
{
    FTP_CTRL_SESSION_DESTROY_CFM cfm;

    FTP_CTRL_SESSION_DESTROY_CFM_UNMARSHALL(header, &cfm);
    if (CSR_FTP_RESULT_SUCCESS != cfm.result)
    {
        L2_DBG_MSG2("mem_util/exp_const: Session destroy error. Session: 0x%x Sts: 0x%x",
                        cfm.session_id,cfm.result);
    }
    transfer_state.session_id = 0;
    handle_ended_transfer();
}

/**
 * handle signals from the fileserv module
 *
 * This is the control callback registered when creating a session with
 * fileserv. Fileserv routes the FTP signals to us when received.
 *
 * \param header        Pointer to signal
 * \param msg_len_words Length of the signal, used only for error checking
 */
static void control_point_callback(const uint16 *header, uint16 msg_len_words)
{
    FTP_SIGNAL_ID msg_id;

    patch_fn_shared(exp_const);

    if (0 == msg_len_words || !header)
    {
        L2_DBG_MSG2("mem_util/exp_const: control_callback bad parameters %p, %d", 
                            header, msg_len_words);
        return;
    }

    msg_id = (FTP_SIGNAL_ID)(header[0]);
    L3_DBG_MSG1("mem_util/exp_const: control_callback ID: 0x%x",msg_id);

    switch (msg_id)
    {
    case FTP_SIGNAL_ID_SESSION_CREATE_CFM:
        session_create_cfm(header);
        break;

    case FTP_SIGNAL_ID_FILE_OPEN_CFM:
        file_open_cfm(header);
        break;

    case FTP_SIGNAL_ID_FILE_READ_CFM:
        file_read_cfm(header);
        break;

    case FTP_SIGNAL_ID_FILE_CLOSE_CFM:
        file_close_cfm(header);
        break;

    case FTP_SIGNAL_ID_SESSION_DESTROY_CFM:
        session_destroy_cfm(header);
        break;

    default:
        L2_DBG_MSG1("mem_util/exp_const: Unhandled FTP message id %d",msg_id);
        break;
    }
}

/**
 * handle data callback from the fileserv module
 *
 * This is the data callback registered when creating a session with
 * fileserv. The callback is called when data is available.
 *
 * A helper function, copy_words_with_wrap(), is used as the raw fileserv
 * functions do not allow copying when the buffer wraps.
 *
 * \param pdu            PDU information passed by fileserv
 * \param data_len_words Number of bytes available
 */
static void process_received_data(const uint8 *pdu, uint16 data_len_bytes)
{
    uint16 len_words = data_len_bytes / 2 ;

    L3_DBG_MSG1("mem_util/exp_const: read cfm for %d bytes", data_len_bytes);

    copy_words_with_wrap(pdu,len_words,(uint16*)transfer_state.data_next);

    transfer_state.data_next += data_len_bytes;
    transfer_state.bytes_left -= data_len_bytes;
}

/****************************************************************************
Public Function Definitions
*/

/** Initialise the file transfer_state
 *
 * One time initialisation of our status
 */
void exp_const_file_transferer_init(void)
{
    system_bus my_ssid = hydra_get_local_ssid();
    my_ssid = HYDRA_TRANSLATE_SSID_FOR_PEER(my_ssid, CURATOR_SSID);
    memset(&transfer_state,0,sizeof(transfer_state));
    transfer_state.clientDataAddress = (ISP_ADDR)my_ssid;
    transfer_state.clientDataPort = ISP_PORT_MAPPING_ENUM_FTP_DATA;
}



/** Start transfer of a file
 *
 * This function creates a file transfer session and manages transfer
 * of the exported constants referenced in the map parameter.
 *
 * The function is private to the exported_constants code which 
 * guarantees a single transfer.
 *
 * When the transfer is completed (success or failure) the status in
 * the map will be updated, and a message sent to the exported 
 * constants message queue ( MEM_UTILS_QUEUE )
 *
 * \param map   Pointer to the constant mapping table.
 */
void exp_const_load_external_table(exp_const_mapping *map)
{
    assert(!transfer_state.in_use);

    L4_DBG_MSG("mem_util/exp_const: starting load of file");
    transfer_state.in_use = TRUE;
    transfer_state.current_constant = map;
    ftp_ctrl_session_create(control_point_callback, process_received_data, 
                            CSR_FTP_SERVER_HOST | CSR_FTP_SERVER_LOCAL);
}

#endif /* INSTALL_CAPABILITY_CONSTANT_EXPORT */


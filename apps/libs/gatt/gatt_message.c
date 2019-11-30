/*************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_message.c

DESCRIPTION
    Functions for handling message queueing.

NOTES
*/

#include "gatt_private.h"
#include <string.h>

#define CASE(from,to)  \
        case (from): \
            if ((from)==(to)) \
                goto coerce; \
            else \
                return (to);

/*************************************************************************
NAME    
    gatt_message_prepare
    
DESCRIPTION
    This function is used to prepare for creating a new message, possibly
    sending pending messages to the application.

NOTES
    This function sets the following boolean flags:
    - more_to_come - a flag in the pending message indicating if more
                     responses _will_ follow.
    - more - more responses can follow
    - send - a new message can be sent without waiting for more information

RETURNS
    True if another message can be sent, false otherwise
*/
bool gatt_message_prepare(cid_map_t *conn,
                          bool *more, bool *send,
                          att_result_t result,
                          bool final,
                          bool *more_to_come,
                          uint16 id)
{
    bool last = FALSE;

    *more = FALSE;
    *send = TRUE;
    
    switch (result)
    {
        case ATT_RESULT_SUCCESS:
            if (final) break; /* procedure handle range already done */
            
            /* Last result for current request. We'll have to start a new
             * request to find out if there are still more results to come.
             * Store current results for later delivery. */
            *send = FALSE;
            *more = TRUE;
            break;

        case ATT_RESULT_SUCCESS_MORE:
            *more = TRUE;
            break;

        default:
            last = TRUE;
    }

    /* deliver pending message */
    if (more_to_come)
    {
        /* update fields */
        *more_to_come = (*more || *send) && !last;

        gatt_message_send(conn, id);
        
        if (last)
        {
            gattSetConnState(conn, NULL, gatt_ms_none);
            return FALSE;
        }
    }

    return TRUE;
}

/*************************************************************************
NAME    
    gatt_message_send
    
DESCRIPTION
    This function is used to send messages to the application.

RETURNS
    
*/
void gatt_message_send(cid_map_t *conn, uint16 id)
{
    if (conn->data.stash)
    {
        MessageSend(conn->data.app, id, conn->data.stash);
        conn->data.stash = NULL;
    }
}

/*************************************************************************
NAME    
    gatt_message_status
    
DESCRIPTION
    This function is used to convert Bluestack ATT result codes to GATT
    status codes.

RETURNS
    Corresponding GATT result code
*/
gatt_status_t gatt_message_status(att_result_t result)
{
    if (result == ATT_RESULT_SUCCESS_MORE)
    {
        result = ATT_RESULT_SUCCESS;
    }
    
    switch(result)
	{
        CASE(ATT_RESULT_SUCCESS, gatt_status_success);
        CASE(ATT_RESULT_INVALID_HANDLE, gatt_status_invalid_handle);
        CASE(ATT_RESULT_READ_NOT_PERMITTED, gatt_status_read_not_permitted);
        CASE(ATT_RESULT_WRITE_NOT_PERMITTED, gatt_status_write_not_permitted);
        CASE(ATT_RESULT_INVALID_PDU, gatt_status_invalid_pdu);
        CASE(ATT_RESULT_INSUFFICIENT_AUTHENTICATION, gatt_status_insufficient_authentication);
        CASE(ATT_RESULT_REQUEST_NOT_SUPPORTED, gatt_status_request_not_supported);
        CASE(ATT_RESULT_INVALID_OFFSET, gatt_status_invalid_offset);
        CASE(ATT_RESULT_INSUFFICIENT_AUTHORIZATION, gatt_status_insufficient_authorization);   
        CASE(ATT_RESULT_PREPARE_QUEUE_FULL, gatt_status_prepare_queue_full);
        CASE(ATT_RESULT_ATTR_NOT_FOUND, gatt_status_attr_not_found);
        CASE(ATT_RESULT_NOT_LONG, gatt_status_not_long);
        CASE(ATT_RESULT_INSUFFICIENT_ENCR_KEY_SIZE, gatt_status_insufficient_encr_key_size);
        CASE(ATT_RESULT_INVALID_LENGTH, gatt_status_invalid_length);
        CASE(ATT_RESULT_UNLIKELY_ERROR, gatt_status_unlikely_error);
        CASE(ATT_RESULT_INSUFFICIENT_ENCRYPTION, gatt_status_insufficient_encryption);
        CASE(ATT_RESULT_UNSUPPORTED_GROUP_TYPE, gatt_status_unsupported_group_type);
        CASE(ATT_RESULT_INSUFFICIENT_RESOURCES, gatt_status_insufficient_resources);
        CASE(ATT_RESULT_APP_MASK, gatt_status_application_error);       
        CASE(ATT_RESULT_DEVICE_NOT_FOUND, gatt_status_device_not_found);
        CASE(ATT_RESULT_SIGN_FAILED, gatt_status_sign_failed);
        CASE(ATT_RESULT_BUSY, gatt_status_busy);
        CASE(ATT_RESULT_TIMEOUT, gatt_status_timeout);
        CASE(ATT_RESULT_INVALID_MTU, gatt_status_invalid_mtu);
        CASE(ATT_RESULT_INVALID_UUID, gatt_status_invalid_uuid);
        CASE(ATT_RESULT_SUCCESS_MORE, gatt_status_success_more);
        CASE(ATT_RESULT_SUCCESS_SENT, gatt_status_success_sent);
        CASE(ATT_RESULT_INVALID_CID, gatt_status_invalid_cid);
        CASE(ATT_RESULT_INVALID_DB, gatt_status_invalid_db);
        CASE(ATT_RESULT_DB_FULL, gatt_status_db_full);
        CASE(ATT_RESULT_INVALID_PHANDLE, gatt_status_invalid_phandle);
        CASE(ATT_RESULT_INVALID_PERMISSIONS, gatt_status_invalid_permissions);
        CASE(ATT_RESULT_SIGNED_DISALLOWED, gatt_result_signed_disallowed);
#ifdef BUILD_FOR_HOST_FOR_ENCRYPTION_ATT_RACE
        CASE(ATT_RESULT_ENCRYPTION_PENDING, gatt_result_encryption_pending);
#endif /* BUILD_FOR_HOST_FOR_ENCRYPTION_ATT_RACE */

		coerce: return (gatt_status_t)result;
		default:
            return result & ~0x8000; /* mask out sign bit of integer */
	}
}

/*************************************************************************
NAME    
    gattSetConnState
    
DESCRIPTION
    This function is used to set the state machine and related locks.

RETURNS
    
*/
void gattSetConnState(cid_map_t *conn, Task task, gatt_msg_scen_t sm)
{
    conn->data.app = task;
    conn->data.scenario = sm;
}

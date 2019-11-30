/****************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_access.c   

DESCRIPTION
    GATT Access IND/RES message scenario functions.

NOTES

*/

/***************************************************************************
    Header Files
*/

#include "gatt.h"
#include "gatt_private.h"

#include <string.h>
#include <stdlib.h>
#include <vm.h>

/* Shorthand macro for GattAccessResponse with No Data, only a result code.*/
#define GATT_ACCESS_RSP(ind, result) \
    GattAccessResponse(ind->cid, ind->handle, result, 0, NULL)

/****************************************************************************
NAME
    gatt_access_ind_start_queued_write

DESCRIPTION
    Handles the ATT_ACCESS_IND message indicating the start of a queued write
    from Bluestack (flags == ATT_ACCESS_WRITE_PERMISSION).

RETURNS

*/
static void gatt_access_ind_start_queued_write(const ATT_ACCESS_IND_T *ind)
{
    cid_map_t *conn = gattFindConnOrPanic(ind->cid); /* never NULL */
    GATT_ACCESS_IND_T *stash = (GATT_ACCESS_IND_T *)conn->data.stash;

    /* If there is already a stash, we have no resources so reject.
     * This will also reject a queued write for another handle, if
     * they occur concurrently.
     */
    if (stash != NULL)
    {
        GATT_ACCESS_RSP(ind, gatt_status_insufficient_resources);
        GATT_DEBUG_INFO(("Stash is NOT NULL and should be\n"));
    }
    else
    {
        /* We are in a queued write, don't let anything else start. */
        gattSetConnState(
                    conn,
                    gattGetCidMappedTask(ind->cid),
                    gatt_ms_access_ind_queued_write
                    );

        /* Send the ATT_ACCESS_PERMISSION for the Service/Profile to respond
         * to. No data with this message.
         */

        conn->data.stash = stash = (GATT_ACCESS_IND_T *)PanicNull(
                    calloc(1, sizeof(GATT_ACCESS_IND_T))
                    );

        stash->cid = ind->cid;
        stash->handle = ind->handle;
        stash->flags = ATT_ACCESS_PERMISSION;

        /* Send the first GATT_ACCESS_IND seeking access permission,
         * this will set the Stash back to NULL.
         */
        gatt_message_send(conn, GATT_ACCESS_IND);

        /* Prepare the next GATT_ACCESS_IND message that will contain,
         * all the data from multiple queued writes.
         */
        conn->data.stash = stash = (GATT_ACCESS_IND_T *)PanicNull(
                    calloc(1, sizeof(GATT_ACCESS_IND_T) + MAX_ACCESS_IND_LEN)
                    );

        stash->cid = ind->cid;
        stash->handle = ind->handle;
    }
}

/****************************************************************************
NAME
    gatt_access_ind_queued_write

DESCRIPTION
    Handles the ATT_ACCESS_IND message containing a queued write (one of many)
    from Bluestack (flags == ATT_ACCESS_WRITE). Queued writes are
    not responded to (ATT_ACCESS_RSP).

    NOTE: 'value' has already been converted from a FW handle to a VM slot.

RETURNS

*/
static void gatt_access_ind_queued_write(const ATT_ACCESS_IND_T *ind, const uint8 *value)
{
    cid_map_t *conn = gattFindConnOrPanic(ind->cid); /* never NULL */
    GATT_ACCESS_IND_T *stash = (GATT_ACCESS_IND_T *)conn->data.stash;

    /* If no stash is prepared then something is wrong. */
    if (stash == NULL)
    {
        GATT_ACCESS_RSP(ind, gatt_status_unlikely_error);
        GATT_DEBUG(("Stash is NULL and should not be\n"));
    }
    /* Only handle one queued write at a time. If there is a queued write
     * for another handle, then reject it.
     */
    else if (ind->handle != stash->handle)
    {
        GATT_ACCESS_RSP(ind, gatt_status_insufficient_resources);
        GATT_DEBUG_INFO(("Simultaneous IRQ Queued Writes!\n"));
    }
    else
    {
        uint16 new_size = stash->size_value + ind->size_value;

        if (new_size > MAX_ACCESS_IND_LEN)
        {
            PanicNull( realloc(stash, new_size) );
        }
        memmove(
                &stash->value[stash->size_value],
                value,
                ind->size_value
                );
        stash->size_value = new_size;
    }
}

/****************************************************************************
NAME
    gatt_access_ind_queued_write_complete

DESCRIPTION
    Handles the ATT_ACCESS_IND message containing a queued write (one of many)
    from Bluestack (flags == ATT_ACCESS_WRITE_COMPLETE).

    The stashed GATT_ACCESS_IND, including the whole of the value data, is
    sent on to the upper layer.

    The upper layer is responsible for sending the response.

RETURNS

*/
static void gatt_access_ind_queued_write_complete(const ATT_ACCESS_IND_T *ind)
{
    cid_map_t *conn = gattFindConnOrPanic(ind->cid); /* never NULL */
    GATT_ACCESS_IND_T *stash = (GATT_ACCESS_IND_T *)conn->data.stash;

    /* If no stash is prepared then something is wrong. */
    if (stash == NULL)
    {
        GATT_ACCESS_RSP(ind, gatt_status_unlikely_error);
        GATT_DEBUG(("Stash is NULL and should not be\n"));
    }
    else
    {
        /* The queued write is complete, send on the stashed message.*/
        stash->flags = ATT_ACCESS_WRITE | ATT_ACCESS_WRITE_COMPLETE;

        gatt_message_send(conn, GATT_ACCESS_IND);
     }
     gattSetConnState(conn, NULL, gatt_ms_none);
}

/****************************************************************************
NAME
    gattHandleAttAccessInd

DESCRIPTION
    Handles the ATT_ACCESS_IND message (non-blocking).

RETURNS

*/
void gattHandleAttAccessInd(const ATT_ACCESS_IND_T *ind)
{
    uint8 *value = NULL;

    if (ind->size_value)
    {
        value = (uint8 *) VmGetPointerFromHandle(ind->value);
    }

    if ( gattCidIsValid(ind->cid) )
    {

        GATT_DEBUG_INFO((
            "ATT_ACCESS_IND: handle 0x%04x, flags 0x%04x, offset 0x%04x\n",
            ind->handle,
            ind->flags,
            ind->offset
            ));

        switch(ind->flags)
        {
            /* If only ATT_ACCESS_PERMISSION flag is set, this is the
             * start of a series of queued writes (write prepare/write
             * execute).
             */
            case ATT_ACCESS_PERMISSION:
                gatt_access_ind_start_queued_write(ind);
                break;

            /* If the only flag is ATT_ACCESS_WRITE, then this is a queued
             * write.
             */
            case ATT_ACCESS_WRITE:
                gatt_access_ind_queued_write(ind, value);
                break;

            /* If the only flag is ATT_ACCESS_WRITE_COMPLETE, then the queued
             * write is complete.
             */
            case ATT_ACCESS_WRITE_COMPLETE:
                gatt_access_ind_queued_write_complete(ind);
                break;

            /* Default handles single MTU sized ATT_ACCESS_IND reads and
             * writes.
             */
            default:
            {

                MAKE_GATT_MESSAGE_WITH_VALUE(
                        GATT_ACCESS_IND,
                        ind->size_value,
                        value
                        );

                 message->cid = ind->cid;
                 message->handle = ind->handle;
                 message->flags = ind->flags;
                 message->offset = ind->offset;

                 MessageSend(
                        gattGetCidMappedTask(ind->cid),
                        GATT_ACCESS_IND,
                        message
                        );
                 break;
            }
        }
    }
    else
    {
        GATT_DEBUG_INFO((
                "gattHandleAttAccessInd: cid 0x%04x not found\n", ind->cid
                ));
    }

    free(value);
}

/****************************************************************************
NAME
    GattAccessResponse

DESCRIPTION
    API function

RETURNS

*/
void GattAccessResponse(uint16 cid, uint16 handle, uint16 result, uint16 size_value, const uint8 *value)
{
    MAKE_GATT_MESSAGE_WITH_VALUE(
        GATT_INTERNAL_ACCESS_RES, 
        size_value,
        value
        );

    message->cid = cid;
    message->handle = handle;
    message->result = result;

    MessageSend(gattGetTask(), GATT_INTERNAL_ACCESS_RES, message);
}

/****************************************************************************
NAME
    gattHandleInternalAccessRes
           
DESCRIPTION
   Handle the internal Access Reponse. 

RETURN

*/
void gattHandleInternalAccessRes(const GATT_INTERNAL_ACCESS_RES_T *res)
{
    if (gattCidIsValid(res->cid))
    {
        cid_map_t *conn = gattFindConnOrPanic(res->cid);
        GATT_ACCESS_IND_T *stash = (GATT_ACCESS_IND_T *)conn->data.stash;
        MAKE_ATT_PRIM(ATT_ACCESS_RSP);

        /* If the Access Response is set to an error, set the connection state to none.
         * Also, if there is a stash during a Queued_Write and something went wrong,
         * clear the stash.
         */

        if (res->result != gatt_status_success)
        {
            if (stash != NULL && stash->flags == ATT_ACCESS_PERMISSION)
            {
                free(stash);
                stash = NULL;
            }
        }
           
        if (res->size_value)
        {
            uint8 *value = (uint8 *)PanicUnlessMalloc(res->size_value);
            memmove(value, res->value, res->size_value);
            prim->value = (uint8 *)VmGetHandleFromPointer(value);
        }
        else
        {
            prim->value = NULL;
        }

        prim->cid = res->cid;
        prim->handle = res->handle;
        prim->result = res->result;
        prim->size_value = res->size_value;

        VmSendAttPrim(prim);
    }
    else
    {
        GATT_DEBUG(("gattHandleInternalAccessRes: Invalid CID 0x%x\n", res->cid));
    }
}


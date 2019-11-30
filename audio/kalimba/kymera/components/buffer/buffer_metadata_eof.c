/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup buffer Buffer Subsystem
 * \file  buffer_metadata_eof.c
 *
 * \ingroup buffer
 *
 * Metadata end-of-file handling
 *
 */

/****************************************************************************
Include Files
*/
#include "buffer_private.h"


#ifdef INSTALL_METADATA

/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Macro Declarations
*/

/****************************************************************************
Public Variable Definitions
*/

/****************************************************************************
Private Function Declarations
*/

/****************************************************************************
Private Function Definitions
*/

/*
 * metadata_eof_delete_final
 *
 * Called when all local and remote references have gone away. This is only
 * called on the EOF-originating core.
 */
static void metadata_eof_delete_final(metadata_eof_callback_ref *cb_ref)
{
    PL_ASSERT(cb_ref->ref_count_local == 0);
    PL_ASSERT(cb_ref->ref_count_remote == 0);

    PL_ASSERT(cb_ref->callback != NULL);

    cb_ref->callback(cb_ref->data);

    pdelete(cb_ref);
}

/****************************************************************************
Public Function Definitions
*/

#ifdef INSTALL_METADATA_DUALCORE

/*
 * metadata_eof_remote_cb
 *
 * This is used as an intermediate callback when a final deletion happens
 * on a remote (not the EOF originator) core.
 *
 * The data parameter is the original local reference pointer.
 *
 */
void buff_metadata_eof_remote_cb(metadata_eof_callback_ref *cb_ref)
{
    PL_ASSERT(cb_ref != NULL);

    if ((--cb_ref->ref_count_remote == 0) && (cb_ref->ref_count_local == 0))
    {
        metadata_eof_delete_final(cb_ref);
    }
}

void buff_metadata_kip_prepare_eof_before_append(metadata_tag* tag)
{
    PL_ASSERT(METADATA_STREAM_END(tag));

    metadata_eof_callback *cb;
    unsigned length;

    if (buff_metadata_find_private_data(tag, META_PRIV_KEY_EOF_CALLBACK, &length, (void**)&cb))
    {
        PL_ASSERT(length == sizeof(metadata_eof_callback));
        PL_ASSERT(cb != NULL);
        PL_ASSERT(cb->ref != NULL);
    }
    else {
        PL_ASSERT(0); //TODO: Do we expect this to ever happen?
    }

    cb->ref->ref_count_local--;

    /* Appending to the originating core (assuming no more than 2 cores) */
    if (hal_get_reg_processor_id() != cb->proc_id)
    {
        if (cb->ref->ref_count_local == 0)
        {
            pdelete(cb->ref);
            cb->last_remote_copy =  TRUE;
        }
        else
        {
            cb->last_remote_copy =  FALSE;
        }
    }

    /**
     * NB There's nothing to be done if we're appending from the originating core.
     * cb->parent and cb->proc_id should already be set.
     */

    /**
     * This is not exactly necessary as the other core uses the proc_id to decide
     * if it needs to allocate its own local cb->ref but let's set it to NULL to
     * be safe.
     */
    cb->ref = NULL;
}

void buff_metadata_kip_prepare_eof_after_remove(metadata_tag* tag)
{
    PL_ASSERT(METADATA_STREAM_END(tag));

    metadata_eof_callback *cb;
    unsigned length;

    if (buff_metadata_find_private_data(tag, META_PRIV_KEY_EOF_CALLBACK, &length, (void**)&cb))
    {
        PL_ASSERT(length == sizeof(metadata_eof_callback));
        PL_ASSERT(cb->ref == NULL);
    }
    else {
        PL_ASSERT(0); //TODO: Do we expect this to ever happen?
    }

    if (hal_get_reg_processor_id() == cb->proc_id) /* Tag coming back home */
    {
        cb->ref = cb->parent;

        //TODO: Is it important to increment first? Do we have to worry about things being atomic?
        cb->ref->ref_count_local++;

        /**
         * We can only decrement our remote counter when we know there are no
         * copies of it left on the remote processor. This is not relevant when
         * we get an explicit KIP message.
         */
        if (cb->last_remote_copy)
        {
            cb->ref->ref_count_remote--;
        }
    }
    else
    {
        if ((cb->ref = xzpnew(metadata_eof_callback_ref)) != NULL)
        {
            cb->ref->ref_count_local = 1;

            /** These are not going to get used on this core anyway */
            cb->ref->callback = NULL;
            cb->ref->data = 0;
        }
    }
}
#endif /* INSTALL_METADATA_DUALCORE */

/*
 * metadata_handle_eof_tag_deletion
 *
 * A tag is being deleted - check for EOF callback data
 * If this is the last reference then trigger the callback
 */
void metadata_handle_eof_tag_deletion(metadata_tag *tag)
{
    metadata_eof_callback *cb;
    unsigned length;

    patch_fn_shared(buff_metadata);

    if (buff_metadata_find_private_data(tag, META_PRIV_KEY_EOF_CALLBACK, &length, (void **)&cb))
    {
        PL_ASSERT(length == sizeof(metadata_eof_callback));
        PL_ASSERT(cb != NULL);

        /**
         * ref pointer can be NULL when the tag is crossing cores but
         * buff_metadata_delete_tag() shouldn't call this function in that
         * scenario.
         */
        PL_ASSERT(cb->ref != NULL);

        if ((--cb->ref->ref_count_local == 0) && (cb->ref->ref_count_remote == 0))
        {
#if defined(INSTALL_METADATA_DUALCORE) && !defined(UNIT_TEST_BUILD)
            if (hal_get_reg_processor_id() != cb->proc_id)
            {
                pdelete(cb->ref);

                uint16 conn_id;
                KIP_MSG_HANDLE_EOF_REQ kip_msg;
                conn_id = PACK_CONID_PROCID(hal_get_reg_processor_id(), cb->proc_id);
                KIP_MSG_HANDLE_EOF_REQ_CON_ID_SET(&kip_msg, conn_id);

                KIP_MSG_HANDLE_EOF_REQ_CB_REF_SET(&kip_msg, (uintptr_t)cb->parent);

                kip_adaptor_send_message(conn_id, KIP_MSG_ID_HANDLE_EOF_REQ, KIP_MSG_HANDLE_EOF_REQ_WORD_SIZE, (unsigned *) &kip_msg, NULL);
            }
            else
#endif // INSTALL_METADATA_DUALCORE
            {
                metadata_eof_delete_final(cb->ref);
            }
        }
    }
}

/*
 * metadata_handle_eof_tag_copy
 *
 * A tag is being copied so increment the reference count
 */
void metadata_handle_eof_tag_copy(metadata_tag *tag, bool remote_copy)
{
    metadata_eof_callback *cb;
    unsigned length;

    patch_fn_shared(buff_metadata);

    if (buff_metadata_find_private_data(tag, META_PRIV_KEY_EOF_CALLBACK, &length, (void **)&cb))
    {
        PL_ASSERT(length == sizeof(metadata_eof_callback));
        PL_ASSERT(cb != NULL);
        PL_ASSERT(cb->ref != NULL);
#ifdef INSTALL_METADATA_DUALCORE
        if (remote_copy)
        {
            if (cb->proc_id == hal_get_reg_processor_id())
            {
                cb->ref->ref_count_remote++;
            }
        }
        else
#endif // INSTALL_METADATA_DUALCORE
        {
            cb->ref->ref_count_local++;
        }
    }
}

/*
 * \brief Add EOF callback struct to a metadata tag
 */
bool buff_metadata_add_eof_callback(metadata_tag *tag, eof_callback callback, unsigned eof_data)
{
    metadata_eof_callback cb;
    patch_fn_shared(buff_metadata);
    METADATA_STREAM_END_SET(tag);

    if ((cb.ref = xzpnew(metadata_eof_callback_ref)) == NULL)
    {
        return FALSE;
    }

    cb.ref->ref_count_local = 1;
    cb.ref->callback = callback;
    cb.ref->data = eof_data;

#ifdef INSTALL_METADATA_DUALCORE
    cb.proc_id = hal_get_reg_processor_id();
    cb.parent = cb.ref;
#endif // INSTALL_METADATA_DUALCORE

    return (buff_metadata_add_private_data(tag, META_PRIV_KEY_EOF_CALLBACK, sizeof(metadata_eof_callback), &cb) != NULL);
}

/*
 * \brief Retrieve EOF callback struct from a metadata tag
 */
bool buff_metadata_get_eof_callback(metadata_tag *tag, metadata_eof_callback *cb_struct)
{
    unsigned length;
    metadata_eof_callback *cb;

    patch_fn_shared(buff_metadata);

    if (buff_metadata_find_private_data(tag, META_PRIV_KEY_EOF_CALLBACK, &length, (void **)&cb))
    {
        PL_ASSERT(length == sizeof(metadata_eof_callback));
        *cb_struct = *cb;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

#endif /* INSTALL_METADATA */

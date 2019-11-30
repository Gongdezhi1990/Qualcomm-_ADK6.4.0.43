/****************************************************************************
 * Copyright (c) 2015 - 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  splitter_metadata.c
 * \ingroup  capabilities
 *
 *  Splitter's helper functions.
 *
 */
/****************************************************************************
Include Files
*/
#include "splitter_private.h"

/**
 * \brief timer handler to allow internal self-kick
 * This prevents a stall when buffers fill up
 *
 * \param timer_data Pointer to the operator instance data.
 */
void splitter_timer_task(void *timer_data)
{
    OPERATOR_DATA *op_data = (OPERATOR_DATA*) timer_data;
    SPLITTER_OP_DATA *splitter = get_instance_data(op_data);

    splitter->self_kick_timer = TIMER_ID_INVALID;
    /* Raise a bg int to process */
    opmgr_kick_operator(op_data);
}

/**
 * \brief Helper function used to populate the terminal information.
 *
 * \param terminal_id information from connect/disconnect/buffer_details calls.
 * \param buffer given only for connect, NULL otherwise.
 * \param terminal_info_t address where the terminal information (terminal id,
                       index etc) will be stored. [output]
 * \param save_buffer If true, the input buffer will be saved into the terminal info.
 *
 * \return TRUE if the call is correct, FALSE otherwise.
 */
bool get_terminal_info(unsigned terminal_id, tCbuffer *buffer, terminal_info_t *terminal_info, bool save_buffer)
{
    terminal_info->terminal_num = terminal_id & TERMINAL_NUM_MASK;
    terminal_info->is_input = terminal_id & TERMINAL_SINK_MASK;

    if(save_buffer)
    {
        terminal_info->buffer = buffer;
    }

    if (terminal_info->is_input)
    {
        if (terminal_info->terminal_num >= SPLITTER_MAX_INPUTS)
        {
            SPLITTER_ERRORMSG1("Splitter: Incorrect terminal number %4d!", terminal_info->terminal_num);
            return FALSE;
        }
        /* For input terminals the channel id is equal to the terminal ID. */
        terminal_info->channel_id = terminal_info->terminal_num;
        /* Input index is always 0 . */
        terminal_info->index = 0;
    }
    else
    {
        if (terminal_info->terminal_num >= SPLITTER_MAX_OUTPUTS)
        {
            SPLITTER_ERRORMSG1("Splitter: Incorrect terminal number %4d!", terminal_info->terminal_num);
            return FALSE;
        }
        /* For outp terminals the channel id is the terminal ID shifted
         * with the number of outputs. */
        terminal_info->channel_id = terminal_info->terminal_num >> 1;

        terminal_info->index = terminal_info->terminal_num & 1;
    }

    return TRUE;
}

/**
 * Helper function to set the cbuffer functions.
 */
void set_cbuffer_functions(SPLITTER_OP_DATA *splitter)
{
#ifdef INSTALL_CBUFFER_EX

    if ((splitter->data_format == AUDIO_DATA_FORMAT_FIXP) || (splitter->data_format == AUDIO_DATA_FORMAT_FIXP_WITH_METADATA))
    {

        if (splitter->packing == UNPACKED)
        {
            /* Just copy the audio*/
            splitter->cbuffer.unpacked_space = cbuffer_calc_amount_space_in_words;
            splitter->cbuffer.data = cbuffer_calc_amount_data_in_words;

            splitter->cbuffer.packed_space = cbuffer_calc_amount_space_in_words;
            splitter->cbuffer.pack = cbuffer_copy;
            splitter->cbuffer.unpack = cbuffer_copy;
            splitter->cbuffer.discard = cbuffer_advance_read_ptr;
#ifdef SPLITTER_EXT_BUFFER
            splitter->cbuffer.sram_packed_space = audio_ext_buffer_amount_space;
            splitter->cbuffer.sram_pack = audio_ext_buffer_circ_write;
            splitter->cbuffer.sram_unpack = audio_ext_buffer_circ_read;
            splitter->cbuffer.sram_discard = audio_ext_buffer_discard_data;
#endif
            splitter->cbuffer.data_size = 4;
            return;
        }
        else
        {
            /* Pack the audio. */
            splitter->cbuffer.unpacked_space = cbuffer_calc_amount_space_in_words;
            splitter->cbuffer.data = cbuffer_calc_amount_data_in_words;

            splitter->cbuffer.packed_space = packed_space;
            splitter->cbuffer.pack = pack_audio; /* cbuffer_copy_ex will pack the data when needed because it know about the usable octets.*/
            splitter->cbuffer.unpack = un_pack_audio; /* same applies for unpacking. */
            splitter->cbuffer.discard = advance_packed;
#ifdef SPLITTER_EXT_BUFFER
            splitter->cbuffer.sram_packed_space = pack_ext_buffer_amount_space;
            splitter->cbuffer.sram_pack = ext_buffer_pack;
            splitter->cbuffer.sram_unpack = ext_buffer_unpack;
            splitter->cbuffer.sram_discard = pack_ext_buffer_discard_data;
#endif
            splitter->cbuffer.data_size = 4;
            return;
        }
    }
    /* Data will be packed and unpacked if the
     * usable octets are different. */
    splitter->cbuffer.unpacked_space = cbuffer_calc_amount_space_ex;
    splitter->cbuffer.data = cbuffer_calc_amount_data_ex;

    splitter->cbuffer.packed_space = cbuffer_calc_amount_space_ex;
    splitter->cbuffer.pack = cbuffer_copy_ex; /* cbuffer_copy_ex will pack the data when needed because it know about the usable octets.*/
    splitter->cbuffer.unpack = cbuffer_copy_ex; /* same applies for unpacking. */
    splitter->cbuffer.discard = cbuffer_advance_read_ptr_ex;
#ifdef SPLITTER_EXT_BUFFER
    splitter->cbuffer.sram_packed_space = ext_buffer_amount_space;
    splitter->cbuffer.sram_pack = ext_buffer_circ_write;
    splitter->cbuffer.sram_unpack = ext_buffer_circ_read;
    splitter->cbuffer.sram_discard = ext_buffer_discard_data;
#endif
    splitter->cbuffer.data_size = 1;
    return;

#endif
}


/**
 * Allocates the buffering buffer. The buffer can be internal or external (SRAM).
 */
static bool create_internal_data_buffer(SPLITTER_OP_DATA *splitter, unsigned buffer_size, data_buffer_t* internal_buff)
{
    if ((buffer_size>>14) != 0)
    {
        SPLITTER_ERRORMSG1("splitter: Cannot create a buffer bigger than 16383 words or 65535 octets."
                "\n  Current buffer size is %d. Note when packing the buffer size is halved!", buffer_size);
        return FALSE;
    }
    if (splitter->location == INTERNAL)
    {
        tCbuffer *buffer;
        buffer = cbuffer_create_with_malloc(buffer_size, BUF_DESC_SW_BUFFER);
        SPLITTER_MSG1("splitter: Internal cbuffer 0x%08x", buffer);
        internal_buff->dsp = buffer;
        if (buffer != NULL)
        {
            return TRUE;
        }
    }
    else
    {
#ifdef SPLITTER_EXT_BUFFER
        EXT_BUFFER *buffer;
        buffer = ext_buffer_create(buffer_size, 0);
        /* Allocate the memory on the external SRAM */
        SPLITTER_MSG1("splitter: Internal SRAM buffer 0x%08x", internal_buff);
        internal_buff->sram = buffer;
        if (buffer != NULL)
        {
            return TRUE;
        }
#endif
    }
    SPLITTER_ERRORMSG("splitter: Could not create the internal buffer.");
    return FALSE;
}

#ifdef INSTALL_METADATA
static void destroy_internal_data_buffer(SPLITTER_OP_DATA *splitter, data_buffer_t* internal_buff)
{
    /* delete the internal metadata buffer allocated for the channel. */
    if (splitter->location == INTERNAL)
    {
        /* delete the internal buffer allocated for the channel. */
        cbuffer_destroy(internal_buff->dsp);
    }
    else
    {
#ifdef SPLITTER_EXT_BUFFER
        ext_buffer_destroy(internal_buff->sram);
#else
        PL_ASSERT(splitter->location == INTERNAL);
#endif
    }
}
#endif

/**
 * Helper function which creates a channel.
 */
static SPLITTER_CHANNEL_STRUC *create_channel(SPLITTER_OP_DATA *splitter, tCbuffer* internal_metadata_buff)
{
    SPLITTER_CHANNEL_STRUC *channel;
    data_buffer_t internal_buff;
    unsigned buffer_size;


    SPLITTER_MSG("Splitter: Creating internal buffer.");

    /* Allocate the channel buffer structure. */
    channel = xzpnew(SPLITTER_CHANNEL_STRUC);
    if (channel == NULL)
    {
        SPLITTER_ERRORMSG("Splitter: Unable to allocate CHANNEL_STRUC!" );
        return NULL;
    }
    /* No need to allocate internal buffer when in cloning mode. */
    if (splitter->working_mode == CLONE_BUFFER)
    {
        return channel;
    }

    /* If the data is packed we only need half of the buffer. */
    if (splitter->packing == PACKED)
    {
        buffer_size = splitter->buffer_size/2;
    }
    else
    {
        buffer_size = splitter->buffer_size;
    }

    /* Allocate the buffer. Metadata flag is set later. */
    if (create_internal_data_buffer(splitter, buffer_size, &internal_buff) == FALSE)
    {
        SPLITTER_ERRORMSG("splitter: Unable to allocate internal buffer for the channel!" );
        /* Free up the channel structure.*/
        pdelete(channel);
        return NULL;
    }

#ifdef INSTALL_METADATA
    if (internal_metadata_buff == NULL)
    {
        if (create_internal_metadata(splitter, &internal_buff, buffer_size) == FALSE)
        {
            destroy_internal_data_buffer(splitter, &internal_buff);
            /* Free up the channel structure.*/
            pdelete(channel);
            return FALSE;
        }
    }
#endif
    /* Finally, set the channel's internal buffer. */
    channel->internal.buffer = internal_buff;

    /* Transition to hold is instant. No need to have an output buffer. */
    if (splitter->hold_streams != OUT_STREAM__0_OFF__1_OFF)
    {
        unsigned i;
        for(i=0; i<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; i++)
        {
            if (get_next_output_state(splitter, i)== HOLD)
            {
                set_current_output_state(splitter, i, HOLD);
            }
        }
    }

    return channel;
}

/**
 * Returns the channel with the given ID. If the channel is not present it will create it.
 */
SPLITTER_CHANNEL_STRUC *get_channel_struct(SPLITTER_OP_DATA *splitter, unsigned channel_id)
{
    SPLITTER_CHANNEL_STRUC *channel = splitter->channel_list;
    while (channel != NULL)
    {
        if (channel->id == channel_id)
        {
            return channel;
        }
        channel = channel->next;
    }

    if(splitter->buffer_size == 0)
    {
        SPLITTER_ERRORMSG("Splitter: Internal buffer size not set cannot connect!" );
        return NULL;
    }

    /* Create a new channel with the id. */
#ifdef INSTALL_METADATA
    channel = create_channel(splitter, get_internal_metadata_buffer(splitter));
#else
    channel = create_channel(splitter, NULL);
#endif

    if (channel != NULL)
    {
        /* Set the channel ID*/
        channel->id = channel_id;
        /* Add the channel to the channel list. */
        if (splitter->channel_list == NULL)
        {
            /* First Channel. */
            splitter->channel_list = channel;
        }
        else
        {
            /* Add the channel to the end of the list. */
            SPLITTER_CHANNEL_STRUC *temp_channel = splitter->channel_list;
            while (temp_channel->next != NULL)
            {
                temp_channel = temp_channel->next;
            }
            temp_channel->next = channel;
        }
    }
    return channel;
}


/**
 * Deletes the channel.
 */
static void delete_channel(SPLITTER_OP_DATA *splitter, SPLITTER_CHANNEL_STRUC *channel)
{
    SPLITTER_MSG1("splitter: Deleting channel %d !", channel->id);
    /* Select the correct interface */
    if (splitter->location == INTERNAL)
    {
        if (channel->internal.buffer.dsp != NULL)
        {
            SPLITTER_MSG1("Deleting internal buffer 0x%08x",channel->internal.buffer.dsp);
            cbuffer_destroy(channel->internal.buffer.dsp);
        }
    }
    else
    {
#ifdef SPLITTER_EXT_BUFFER
        if (channel->internal.buffer.sram != NULL)
        {
            SPLITTER_MSG1("Deleting internal sram buffer 0x%08x",channel->internal.buffer.sram);
            ext_buffer_destroy(channel->internal.buffer.sram);
        }
#else
        PL_ASSERT(splitter->location == INTERNAL);
#endif
    }

    /* delete the channel structure */
    pdelete(channel);
}


/**
 * Removes and deletes the disconnected channels.
 */
void delete_disconnected_channel(SPLITTER_OP_DATA *splitter)
{
    SPLITTER_CHANNEL_STRUC **channel_ptr = &splitter->channel_list;
    while (*channel_ptr != NULL)
    {
        bool channel_disconnected = TRUE;
        int i;

        /* Check if the channel has connection. */
        if ((*channel_ptr)->input_buffer != NULL)
        {
            channel_disconnected = FALSE;
        }
        else
        {
            for(i=0; i<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; i++)
            {
                if ((*channel_ptr)->output_buffer[i] != NULL)
                {
                    channel_disconnected = FALSE;
                    break;
                }
            }
        }

        if (channel_disconnected)
        {
            SPLITTER_CHANNEL_STRUC *temp = *channel_ptr;

            /* Remove it from the list, */
            *channel_ptr = temp->next;
            /* Delete the metadata if this is the last channel. */
            if (splitter->channel_list == NULL)
            {
#ifdef INSTALL_METADATA
                delete_internal_metadata(splitter);
#endif
            }
            /* Delete the channel. */
            delete_channel(splitter, temp);
        }

        channel_ptr = &((*channel_ptr)->next);
    }
}

#if defined(SPLITTER_DEBUG) && defined(INSTALL_METADATA)
/**
 * Function which returns the read offset of the internal buffer.
 */
static unsigned get_buffer_read_offset(SPLITTER_OP_DATA *splitter, SPLITTER_CHANNEL_STRUC *channel)
{

    if (splitter->location == INTERNAL)
    {
        tCbuffer *buffer = channel->internal.buffer.dsp;
        unsigned int offset, rd_offset;
        unsigned int base_addr = (unsigned int)(uintptr_t)buffer->base_addr;

        rd_offset = (unsigned int)(uintptr_t)cbuffer_get_read_address_ex(buffer, &offset) - base_addr;
        rd_offset += offset;
        return rd_offset;
    }
    else
    {
#ifdef SPLITTER_EXT_BUFFER
        return ext_buffer_get_read_offset(channel->internal.buffer.sram);
#else
        PL_ASSERT(splitter->location == INTERNAL);
        /* Avoid compiler warning. */
        return 0;
#endif
    }
}

/**
 * Function which returns the write offset of the internal buffer.
 */
static unsigned get_buffer_write_offset(SPLITTER_OP_DATA *splitter, SPLITTER_CHANNEL_STRUC *channel)
{
    unsigned wr_offset;
    if (splitter->location == INTERNAL)
    {
        tCbuffer *buffer = channel->internal.buffer.dsp;
        unsigned int offset;
        unsigned int base_addr = (unsigned int)(uintptr_t)buffer->base_addr;

        wr_offset = (unsigned int)(uintptr_t)cbuffer_get_write_address_ex(buffer, &offset) - base_addr;
        wr_offset += offset;
    }
    else
    {
#ifdef SPLITTER_EXT_BUFFER
        wr_offset = ext_buffer_get_write_offset(channel->internal.buffer.sram);
#else
        PL_ASSERT(splitter->location == INTERNAL);
        /* Avoid compiler warning. */
        wr_offset = 0;
#endif
    }

    return wr_offset;
}

/**
 * Helper function which check if all the buffers in the operator are aligned.
 * This buffer should only be used for debugging as it takes a lot of MIPS to
 * run.
 */
void check_buffers_validity(SPLITTER_OP_DATA *splitter)
{
    SPLITTER_CHANNEL_STRUC *channel;
    int read_index;
    int before_prev_rd_indexes;
    unsigned i;
    tCbuffer *metadata_buffer;
    data_buffer_t internal;
    unsigned basic_space, chan0_space,  chan1_space;
    unsigned basic_read_idx, chan0_read_idx, chan1_read_idx;

    channel = splitter->channel_list;

    /* Note that the metadata will be checked for all channels.*/
    while (channel != NULL)
    {
        /* initialise the default values. */
        basic_space = UINT_MAX;
        chan0_space = UINT_MAX;
        chan1_space = UINT_MAX;
        basic_read_idx = 0;
        chan0_read_idx = 0;
        chan1_read_idx = 0;

        /* It is important to make sure that there is no leaking
         * tags as they can block the system.*/
        metadata_buffer = get_internal_metadata_buffer(splitter);
        check_metadata_tags_length(splitter, metadata_buffer);

        /* Get the internal buffer read pointer and read index.
         * by default this pointer should be the minimum of one
         * of the two channels.*/
        if ((get_current_output_state(splitter, 0) != INACTIVE)||
            (get_current_output_state(splitter, 1) != INACTIVE))
        {
            basic_space = internal_buffer_space(splitter, &channel->internal.buffer);
            basic_read_idx = metadata_buffer->metadata->prev_rd_index;
        }
        else /* Both outputs are inactive. */
        {
            if(get_buffer_read_offset(splitter, channel) != get_buffer_write_offset(splitter, channel))
            {
                SPLITTER_MSG2("Splitter: Not all data discarded while both outputs INACTIVE!\n "
                        "read 0x%08x write 0x%08x",
                        get_buffer_read_offset(splitter, channel),
                        get_buffer_write_offset(splitter, channel));
                PL_ASSERT(FALSE);
            }
            if(metadata_buffer->metadata->prev_rd_index !=  metadata_buffer->metadata->prev_wr_index)
            {
                SPLITTER_MSG("Splitter: Not all metadata discarded while both outputs INACTIVE! ");
                PL_ASSERT(FALSE);
            }
        }

        /* Check the read pointer and buffers for each channel. */
        internal = channel->internal.buffer;

        /* Get the read offset before the check operation*/
        read_index = get_buffer_rd_offset(splitter, &internal);
        before_prev_rd_indexes = metadata_buffer->metadata->prev_rd_index;

        for (i=0; i<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; i++)
        {
            SPLITTER_OUTPUT_STATE state;

            state = get_current_output_state(splitter, i);
            if ((state == HOLD) || (state == ACTIVE))
            {
                unsigned space;

                /* Set up the internal buffer metadata read indexes!*/
                metadata_buffer->metadata->prev_rd_index = splitter->internal_metadata.prev_rd_indexes[i];
                /* Set up the internal buffer read pointers!*/
                set_buffer_rd_offset(splitter, &internal, channel->internal.read_offset[i]);

                /* Save the channel read pointer/index. */
                if (i == 0)
                {
                    chan0_space = internal_buffer_space(splitter, &channel->internal.buffer);
                    chan0_read_idx = metadata_buffer->metadata->prev_rd_index;
                }
                else
                {
                    chan1_space = internal_buffer_space(splitter, &channel->internal.buffer);
                    chan1_read_idx = metadata_buffer->metadata->prev_rd_index;
                }

                /* Check if the metadata space is equal to the data buffer space.*/
                space = get_interal_metadata_space(splitter);
                check_data_space(splitter, channel, space, i);
            }


        }/* END of output for loop. */

        /* Restore the read pointer of the internal buffer.*/
        set_buffer_rd_offset(splitter, &internal, read_index);
        metadata_buffer->metadata->prev_rd_index = before_prev_rd_indexes;

        /* Check if the main buffer is tracking the minimum. The best we can
         * do is to check that at least one read pointer is equalt with the main
         * read pointer/index. */
        if((chan0_space < basic_space) || (chan1_space < basic_space))
        {
            SPLITTER_MSG3("Splitter: The default internal read pointer should have the smallest space!"
                "\n sram basic space 0x%08x"
                "\n sram chan0 space 0x%08x"
                "\n sram chan1 space 0x%08x",
                basic_space,
                chan0_space,
                chan1_space);
            PL_ASSERT(FALSE);
        }
        if((chan0_space != basic_space) && (chan1_space != basic_space))
        {
            SPLITTER_MSG3("Splitter: At least one read pointer should have the same space as the basic read pointer! "
                "\n sram basic space 0x%08x"
                "\n sram chan0 space 0x%08x"
                "\n sram chan1 space 0x%08x",
                basic_space,
                chan0_space,
                chan1_space);
            PL_ASSERT(FALSE);
        }
        if((chan0_read_idx != basic_read_idx) && (chan1_read_idx != basic_read_idx))
        {
            SPLITTER_MSG3("Splitter: wrong read indexes! "
                "\n sram basic read_idx 0x%08x"
                "\n sram chan0 read_idx 0x%08x"
                "\n sram chan1 read_idx 0x%08x",
                basic_read_idx,
                chan0_read_idx,
                chan1_read_idx);
            PL_ASSERT(FALSE);
        }

        /* Move to next channel. */
        channel = channel->next;
    }/* END of channel loop. */
}
#endif



/****************************************************************************
 * Copyright (c) 2015 - 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  splitter.c
 * \ingroup  capabilities
 *
 *  Splitter capability.
 *
 */
/****************************************************************************
Include Files
*/
#include "splitter.h"
#include "splitter_private.h"

/****************************************************************************
Private Constant Definitions
*/

/****************************************************************************
Public Constant Declarations
*/
/** The splitter capability function handler table */
const handler_lookup_struct splitter_handler_table =
{
    splitter_create,           /* OPCMD_CREATE */
    splitter_destroy,          /* OPCMD_DESTROY */
    splitter_start,            /* OPCMD_START */
    base_op_stop,              /* OPCMD_STOP */
    base_op_reset,             /* OPCMD_RESET */
    splitter_connect,          /* OPCMD_CONNECT */
    splitter_disconnect,       /* OPCMD_DISCONNECT */
    splitter_buffer_details,   /* OPCMD_BUFFER_DETAILS */
    splitter_get_data_format,  /* OPCMD_DATA_FORMAT */
    splitter_get_sched_info    /* OPCMD_GET_SCHED_INFO */
};

/** The splitter capability operator message handler table */
const opmsg_handler_lookup_table_entry splitter_opmsg_handler_table[] =
{
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,              base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_SET_BUFFER_SIZE,                     splitter_set_buffer_size},
    {OPMSG_SPLITTER_ID_SET_RUNNING_STREAMS,               splitter_set_running_streams},
    {OPMSG_SPLITTER_ID_ACTIVATE_STREAMS,                  splitter_activate_streams},
    {OPMSG_SPLITTER_ID_ACTIVATE_STREAMS_AFTER_TIMESTAMP,  splitter_activate_streams_after_timestamp},
    {OPMSG_SPLITTER_ID_DEACTIVATE_STREAMS,                splitter_deactivate_streams},
    {OPMSG_SPLITTER_ID_BUFFER_STREAMS,                    splitter_buffer_streams},
    {OPMSG_SPLITTER_ID_SET_DATA_FORMAT,                   splitter_set_data_format},
    {OPMSG_SPLITTER_ID_SET_WORKING_MODE,                  splitter_set_working_mode},
    {OPMSG_SPLITTER_ID_SET_BUFFERING_LOCATION,            splitter_set_location},
    {OPMSG_SPLITTER_ID_SET_PACKING,                       splitter_set_packing},
    {OPMSG_SPLITTER_ID_SET_REFRAMING,                     splitter_set_reframing},
    {OPMSG_COMMON_SET_SAMPLE_RATE,                        splitter_set_sample_rate},
    {0, NULL}
};

/** splitter capability data */
const CAPABILITY_DATA splitter_cap_data =
{
    SPLITTER_CAP_ID,                                       /* Capability ID */
    0, 3,                                                  /* Version information - hi and lo parts */
    SPLITTER_MAX_INPUTS, SPLITTER_MAX_OUTPUTS,             /* Max number of sinks/inputs and sources/outputs */
    &splitter_handler_table,                               /* Pointer to message handler function table */
    splitter_opmsg_handler_table,                          /* Pointer to operator message handler function table */
    splitter_process_data,                                 /* Pointer to data processing function */
    0,                                                     /* Reserved */
    sizeof(SPLITTER_OP_DATA)
};
#if !defined(CAPABILITY_DOWNLOAD_BUILD)
MAP_INSTANCE_DATA(CAP_ID_SPLITTER, SPLITTER_OP_DATA)
#else
MAP_INSTANCE_DATA(CAP_ID_DOWNLOAD_SPLITTER, SPLITTER_OP_DATA)
#endif /* CAPABILITY_DOWNLOAD_BUILD */

/****************************************************************************
Private Function Declaration
*/
static inline void splitter_process_data_clone(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched);
#ifdef INSTALL_METADATA
static inline void splitter_process_data_buffering(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched);

static unsigned amount_to_pack(SPLITTER_OP_DATA *splitter);
static void pack_to_internal(SPLITTER_OP_DATA *splitter, unsigned data_to_pack, bool offload_enabled);
static bool move_data_from_internal_to_output(SPLITTER_OP_DATA *splitter);
static void pack_to_internal_data_buffer(SPLITTER_OP_DATA *splitter, unsigned data_to_pack);
static inline void remove_from_data_buffer(SPLITTER_OP_DATA *splitter, SPLITTER_CHANNEL_STRUC *channel, unsigned data_to_remove);
static void unpack_data_from_internal_to_output(SPLITTER_OP_DATA *splitter, unsigned data_0, unsigned data_1, unsigned data_to_remove);
#endif

/****************************************************************************
Private Function Definitions
*/

/**
 * \brief Updates all buffer pointers so that outputs can read any new data and
 * inputs can see any new space created from down stream reads.
 *
 * \param op_data Pointer to the operator instance data.
 */
static inline void splitter_process_data_clone(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    SPLITTER_OP_DATA *splitter = get_instance_data(op_data);
    unsigned i, min_new_data, min_new_space;
    int *new_output_write_addr;
    int *new_input_read_addr;

    SPLITTER_CHANNEL_STRUC *channel = splitter->channel_list;
    patch_fn(splitter_process_data);

    timer_cancel_event_atomic(&splitter->self_kick_timer);

    /* This code is very naughty and reaches into the cbuffer structures. It can
     * be done safely because they have to be local, and it's lightning fast as
     * a result. Cbuffer API is subverted because it isn't designed for this.
     */
    min_new_data = min_new_space = UINT_MAX;
    /* Iterate through the list of all active channels. */
    while (NULL != channel)
    {
        int new_data, new_space;
        tCbuffer *out = NULL; /* Initialise to keep the compiler happy. */
        tCbuffer *in = channel->input_buffer;

        for (i=0; i<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; i++)
        {
            if (get_current_output_state(splitter, i) == ACTIVE)
            {
                out = channel->output_buffer[i];

                /* Find out minimum available space. */
                new_space = (char *)out->read_ptr - (char *)in->read_ptr;
                if (new_space < 0)
                {
                    new_space += in->size;
                }

                if (new_space < min_new_space)
                {
                    min_new_space = new_space;
                }
            }
        }

        /* Find out minimum available data. */
        new_data =  (char *)in->write_ptr -  (char *)out->write_ptr;
        if (new_data < 0)
        {
            new_data += out->size;
        }

        if (new_data < min_new_data)
        {
            min_new_data = new_data;
        }

        channel = channel->next;
    }

    /* Typically only one of  min_new_space OR min_new_data are non zero on a
     * given kick so we separate the looping out to reduce the amount of work done.
     */
    if (min_new_space > 0)
    {
        channel = splitter->channel_list;
        while (NULL != channel)
        {
            tCbuffer *in = channel->input_buffer;
            new_input_read_addr = (int *)((char *)in->read_ptr + min_new_space);
            if (new_input_read_addr >= (int *)((char *)in->base_addr + in->size))
            {
                new_input_read_addr = (int *)((char *)new_input_read_addr - in->size);
            }
            in->read_ptr = new_input_read_addr;
            channel = channel->next;
        }
        /* N.B. Because the splitter runs in place and is designed to be
         * cascaded, it is necessary to kick back whenever data is consumed so
         * that the input buffer pointers of a splitter that proceeds this one
         * are updated. */
        touched->sinks = splitter->touched_sinks;
    }

    if (min_new_data > 0)
    {
#ifdef INSTALL_METADATA
        unsigned octets_moved;
        tCbuffer *metadata_ip_buffer;
        tCbuffer *metadata_op_buffer;

        metadata_ip_buffer = get_metadata_buffer(splitter, TRUE, 0);
        metadata_op_buffer = get_metadata_buffer(splitter, FALSE, 0);


        if (buff_has_metadata(metadata_ip_buffer))
        {
            /* Transport all the available metadata. The available tags will be limited by
             * the cbuffer available data. This will eliminate difference between
             * octet and _ex buffers. */
            octets_moved =  buff_metadata_available_octets(metadata_ip_buffer);
        }
        else
        {
            /* There is no metadata for the input buffer, but still update the output
             * buffer metadata write index. Due to the lack of the metadata (so no info on
             * the usable octets) min_new_data is a fair approximation.  */
            octets_moved = min_new_data;
        }

        /* Transport any metadata to the output. */
        metadata_strict_transport(metadata_ip_buffer,
                                    metadata_op_buffer,
                                    octets_moved);
#endif /* INSTALL_METADATA */
        channel = splitter->channel_list;
        while (NULL != channel)
        {
            for (i=0; i<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; i++)
            {
                if (get_current_output_state(splitter, i) == ACTIVE)
                {
                    tCbuffer *out = channel->output_buffer[i];
                    new_output_write_addr = (int *)((char *)out->write_ptr + min_new_data);
                    if (new_output_write_addr >= (int *)((char *)out->base_addr + out->size))
                    {
                        new_output_write_addr = (int *)((char *)new_output_write_addr - out->size);
                    }
                    out->write_ptr = new_output_write_addr;
                }
#ifdef INSTALL_METADATA
                else
                {
                    /* If the output is disabled and connected metadata is created
                     * for the output and not consumed by anyone. Delete those metadata.
                     * NOTE: It would be better not to create it in the first place, but
                     * that involves the splitter to take care of the metadata transport. */
                    tCbuffer *out = channel->output_buffer[i];
                    if (out)
                    {
                        metadata_tag *ret_mtag;
                        unsigned b4idx, afteridx;
                        ret_mtag = buff_metadata_remove(out, octets_moved, &b4idx, &afteridx);
                        buff_metadata_tag_list_delete(ret_mtag);
                    }
                }
#endif /* INSTALL_METADATA */
            }
            channel = channel->next;
        }
        touched->sources = splitter->touched_sources;
    }

    timer_schedule_event_in_atomic(SPLITTER_SELF_KICK_RATIO * stream_if_get_system_kick_period(),
        splitter_timer_task, (void*)op_data, &splitter->self_kick_timer);
}

#ifdef INSTALL_METADATA

/**
 * Helper function to calculate the amount to pack/copy to the internal buffer.
 */
static unsigned amount_to_pack(SPLITTER_OP_DATA *splitter)
{
    SPLITTER_CHANNEL_STRUC *channel;
    unsigned data_to_pack;
    unsigned new_data;
    unsigned min_data;
    tCbuffer *in;

    /* When reframing make sure that the we are not consuming less than
     * the minimum frame size. If in transition do not limit to
     * frame size to make the transition faster.  */
    if (splitter->reframe_enabled && !splitter->in_transition)
    {
        min_data = splitter->frame_size;
        /* covert the minimum data to octets if needed.*/
        if (splitter->cbuffer.data_size == 1)
        {
            min_data = words_to_octets(min_data);
        }
        data_to_pack = min_data;
    }
    else
    {
        /* If we have less than 1 word or octets just exit. */
        min_data = 1;
        data_to_pack = UINT_MAX;
    }

    /* Get the minimum available data for all inputs. */
    channel = splitter->channel_list;
    /* We must have at least one channel. */
    SPLITTER_DEBUG_INSTR(PL_ASSERT(channel != NULL));

    while ((channel != NULL) && (data_to_pack != 0))
    {
        in = channel->input_buffer;

        /* Calculate the minimum available data.*/
        new_data = splitter->cbuffer.data(in);
        if (new_data < data_to_pack)
        {
            if (new_data < min_data)
            {
                /* Exit early if there is not enough data. */
                return 0;
            }
            /* Limit data. */
            data_to_pack = new_data;
        }
        /* Calculate the minimum available metadata.*/
        if (buff_has_metadata(in))
        {
            new_data = buff_metadata_available_octets(in);
            if (splitter->cbuffer.data_size == 4)
            {
                new_data = octets_to_samples(new_data);
            }
            if (new_data < data_to_pack)
            {
                SPLITTER_MSG2("Splitter: Input limited by the available metadata!"
                        "\n limiting %d to %d", data_to_pack, new_data);
                if (new_data < min_data)
                {
                    /* Exit early if there is not enough data. */
                    return 0;
                }
                data_to_pack = new_data;
            }
        }
        channel = channel->next;
    }

    /* Calculate the internal space. It is enough to do it for one channel
     * because the internal buffer is synced. Also there is no need to check
     * for the data as we keep this buffer synced with the metadata.*/
    new_data = get_interal_metadata_space(splitter);
    if (new_data < data_to_pack)
    {
        if (new_data < min_data)
        {
            /* Exit early if there is not enough data. */
            return 0;
        }
        /* Limit data. */
        data_to_pack = new_data;
#if defined(SPLITTER_DEBUG)
        /* Amount of data limited by available space in the internal buffer. */
        if (splitter->internal_buffer_full == FALSE)
        {
            splitter->internal_buffer_full = TRUE;
            SPLITTER_MSG("Splitter Internal buffer full! ");
        }
    }
    else
    {
        splitter->internal_buffer_full = FALSE;
#endif
    }

    return data_to_pack;
}
/**
 * Helper function which packs to the internal data buffer.
 */
static inline void pack_to_data_buffer(SPLITTER_OP_DATA *splitter, SPLITTER_CHANNEL_STRUC *channel, unsigned data_to_pack)
{
    unsigned packed;
    tCbuffer *in = channel->input_buffer;

    /* Select the correct interface */
    if (splitter->location == INTERNAL)
    {
        tCbuffer *internal = channel->internal.buffer.dsp;
        packed = splitter->cbuffer.pack(internal, in, data_to_pack);
    }
    else
    {
#ifdef SPLITTER_EXT_BUFFER
        EXT_BUFFER * internal = channel->internal.buffer.sram;
        packed = splitter->cbuffer.sram_pack(internal, in, data_to_pack);
#else
        PL_ASSERT(splitter->location == INTERNAL);
        /* Stop the compiler complaining */
        packed = 0;
#endif
    }
    /* If the amount of data pack is different to then
     * amount_to_pack is incorrect or another error occurred.
     * There is no reason handling this case because it will
     * cause metadata problems further down the chain. */
    if (packed != data_to_pack)
    {
        SPLITTER_ERRORMSG1("Splitter Only packed = %d!",
                packed);
    }
    PL_ASSERT(packed == data_to_pack);
}

/**
 * Helper function which packs for all channels to the internal buffer.
 */
static void pack_to_internal_data_buffer(SPLITTER_OP_DATA *splitter, unsigned data_to_pack)
{
    SPLITTER_CHANNEL_STRUC *channel;
#ifdef SPLITTER_OFFLOAD
    /* First, make sure that there is no other buffer access running. */
    SPLITTER_DEBUG_INSTR(PL_ASSERT(!splitter->buffer_access));
    /* Set buffer_access to true even if the operator only runs in one core. */
    splitter->buffer_access = TRUE;
    SPLITTER_MSG1("Splitter Pack/copy to internal buffer:"
            "data_pack = %d ",
            data_to_pack);
#endif

#ifdef SPLITTER_EXT_BUFFER
    if (splitter->location == SRAM)
    {
        /* Get access to the SRAM.*/
        extmem_lock(EXTMEM_SPI_RAM, FALSE);
    }
#endif

    channel = splitter->channel_list;
    while (channel != NULL)
    {
        /* Pack to the internal data buffer which can be located
         * in the sram or internally in the dsp. */
        pack_to_data_buffer(splitter, channel, data_to_pack);

        channel = channel->next;
    }

#ifdef SPLITTER_EXT_BUFFER
    if (splitter->location == SRAM)
    {
        /* Release the SRAM */
        extmem_unlock(EXTMEM_SPI_RAM);
    }
#endif

    /* It is necessary to kick back whenever data is consumed so
     * so we can make full use of the internal buffer. */
    splitter->kick_backward = TRUE;
#ifdef SPLITTER_OFFLOAD
    /* Processing finished! */
    splitter->buffer_access = FALSE;
    SPLITTER_MSG("Splitter: pack_to_internal_cbuffer finished");
#endif
}

/**
 * Helper function which packs or copies data to the internal buffer.
 */
static void pack_to_internal(SPLITTER_OP_DATA *splitter, unsigned data_to_pack, bool offload_enabled)
{
    /* Transport metadata to the internal buffer. */
    splitter_metadata_transport_to_internal(
            splitter,
            data_to_pack
    );

#ifdef SPLITTER_OFFLOAD
    if (offload_enabled && audio_thread_offload_is_active())
    {
        splitter->offload_activated = TRUE;
        if(wait_for_buffer_access_end(splitter))
        {
            return;
        }

        SPLITTER_MSG1("Splitter: Calling to offload! data_to_pack = %d",data_to_pack);
        /* Call the offload function */
        audio_thread_rpc_call(
                (uintptr_t)pack_to_internal_data_buffer,
                (uintptr_t)splitter,
                (uintptr_t)data_to_pack,
                (uintptr_t)0,
                (uintptr_t)0);
    }
    else
#endif
    {
        /* Just call the function directly. */
        pack_to_internal_data_buffer(splitter, data_to_pack);
    }

    /*
     * Remove all the data from the internal buffer if both outputs
     * are disabled.
     * TODO make this more efficient by not copying at all, just
     * updating the appropriate pointer.
     */
    if (!splitter->in_transition)
    {
        if ((get_current_output_state(splitter, 0) == INACTIVE)&&
            (get_current_output_state(splitter, 1) == INACTIVE))
        {
            /* remove the metadata */
            remove_metadata_from_internal(splitter, data_to_pack);
            /* remove the data by copying 0 0 but  removing data */
            unpack_data_from_internal_to_output(splitter, 0, 0, data_to_pack);
        }
    }


    return;
}

/**
 * Helper function to get the amount of data per output streams.
 */
static void internal_buffer_data(SPLITTER_OP_DATA *splitter, unsigned *data)
{
    int before_prev_rd_indexes;
    tCbuffer *metadata_bufffer;
    unsigned i;
    SPLITTER_OUTPUT_STATE output_state;

    metadata_bufffer = get_internal_metadata_buffer(splitter);

    /* Save the read pointer */
    before_prev_rd_indexes = metadata_bufffer->metadata->prev_rd_index;
    for (i=0; i<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; i++)
    {
        output_state = get_current_output_state(splitter, i);
        if (output_state == INACTIVE)
        {
            /* Inactive channels discards data instantaneously.*/
            data[i] = 0;
        }
        else if ((output_state == ACTIVE) || (output_state == HOLD))
        {
            /* Set up the internal buffer metadata read indexes!*/
            metadata_bufffer->metadata->prev_rd_index = splitter->internal_metadata.prev_rd_indexes[i];

            SPLITTER_MSG2("Splitter internal_buffer_data output index %d  available meta data 0x%08x  ",
                    i, buff_metadata_available_octets(metadata_bufffer));
            /* The internal buffer is always synchronised with the metadata
             * so there is no need to check both data and metadata. */
            data[i] = buff_metadata_available_octets(metadata_bufffer);
        }
    }/* END of output for loop. */
    metadata_bufffer->metadata->prev_rd_index = before_prev_rd_indexes;
}

/**
 * Helper function to limit the amount of data per output streams which will
 * be unpacked/copied to the output buffers.
 */
static void limit_data_to_move(SPLITTER_OP_DATA *splitter, unsigned *data)
{
    SPLITTER_CHANNEL_STRUC *channel;
    int before_prev_rd_indexes;
    unsigned i;
    tCbuffer *metadata_buffer;
    SPLITTER_OUTPUT_STATE output_state;

    metadata_buffer = get_internal_metadata_buffer(splitter);
    /* Limit the data based on the output space. */
    channel = splitter->channel_list;
    while (channel != NULL)
    {
        /* Save the read pointer */
        before_prev_rd_indexes = metadata_buffer->metadata->prev_rd_index;

        for (i=0; i<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; i++)
        {
            output_state = get_current_output_state(splitter, i);
            /* Inactive and hold output streams need no limiting. */
            if (output_state == ACTIVE)
            {
                unsigned space;
                tCbuffer *output = channel->output_buffer[i];

                space = splitter->cbuffer.unpacked_space(output);
                data[i] = MIN(data[i], space);
                /* Now limit to the available metadata. */
                space = buff_metadata_available_space(output);
                if (splitter->cbuffer.data_size == 4)
                {
                    space = octets_to_samples(space);
                }
                data[i] = MIN(data[i], space);
            }
            else if (output_state == HOLD)
            {
                unsigned space;
                /* Set up the internal buffer metadata read indexes!*/
                metadata_buffer->metadata->prev_rd_index = splitter->internal_metadata.prev_rd_indexes[i];

                space = get_interal_metadata_space(splitter);
                SPLITTER_DEBUG_INSTR(check_data_space(splitter, channel, space, i));
                /* discard enough data to hold a frame*/
                if (space < splitter->frame_size + 3)
                {
                    /* Leave three more words in case we are connected to a SBC decode.*/
                    data[i] = splitter->frame_size - space + 3;
                }
                else
                {
                    data[i] = 0;
                }
            }

        }/* END of output for loop. */

        metadata_buffer->metadata->prev_rd_index = before_prev_rd_indexes;

        /* Move to next channel. */
        channel = channel->next;
    }/* END of channel loop. */

    SPLITTER_MSG2("Splitter limit data data[0],data[1]\n"
            " 0x%08x 0x%08x  ",
            data[0],data[1]);
}


/**
 * Function which unpacks to all active channels from the internal data buffer.
 */
static inline void unpack_from_data_buffer(SPLITTER_OP_DATA *splitter, SPLITTER_CHANNEL_STRUC *channel, unsigned data_0, unsigned data_1)
{
    data_buffer_t internal;
    unsigned read_offset;
    unsigned i;

    internal = channel->internal.buffer;
    read_offset = get_buffer_rd_offset(splitter, &internal);

    for (i=0; i<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; i++)
    {
        unsigned data;
        /* Select the channel. */
        if (i==0)
        {
            data = data_0;
        }
        else
        {
            data = data_1;
        }

        /* Check if anythings needs to be done for the output stream*/
        if ((data != 0) && (get_current_output_state(splitter, i) == ACTIVE))
        {
            tCbuffer *out = channel->output_buffer[i];
            PL_ASSERT(out != NULL);

            /* Set up the internal buffer read pointers!*/
            set_buffer_rd_offset(splitter, &internal, channel->internal.read_offset[i]);

            /* Select the correct interface */
            if (splitter->location == INTERNAL)
            {
                unsigned unpacked;
                /* Unpack/copy the data to the output. */
                unpacked = splitter->cbuffer.unpack(out, internal.dsp, data);
                /* If not all data could be unpacked something went wrong. */
                PL_ASSERT(unpacked == data);
            }
            else
            {
#ifdef SPLITTER_EXT_BUFFER
                /* Unpack the data to the output. */
                splitter->cbuffer.sram_unpack(out, internal.sram, data);
#else
                PL_ASSERT(splitter->location == INTERNAL);
#endif
            }
            /* Unpack updates the read pointer for the internal buffer.
             * Update the read pointer for the output stream index.*/
            channel->internal.read_offset[i] = get_buffer_rd_offset(splitter, &internal);
        }
#if defined(SPLITTER_DEBUG)
        /**
         * Update the read pointer for channels in hold
         */
        if ((data != 0) && (get_current_output_state(splitter, i) == HOLD))
        {
            /* Set up the internal buffer read pointers!*/
            set_buffer_rd_offset(splitter, &internal, channel->internal.read_offset[i]);

            remove_from_data_buffer(splitter, channel, data);

            /* Discard updates the read pointer for the internal buffer.
             * Update the read pointer for the output stream index.*/
            channel->internal.read_offset[i] = get_buffer_rd_offset(splitter, &internal);
        }

#endif

    }/* END of output for loop. */

    /* Restore the read pointer of the internal buffer.*/
    set_buffer_rd_offset(splitter, &internal, read_offset);
}

/**
 * Function discards data from the internal data buffer.
 */
static inline void remove_from_data_buffer(SPLITTER_OP_DATA *splitter, SPLITTER_CHANNEL_STRUC *channel, unsigned data_to_remove)
{
    /* Select the correct interface */
    if (splitter->location == INTERNAL)
    {
        splitter->cbuffer.discard(channel->internal.buffer.dsp, data_to_remove);
    }
    else
    {
#ifdef SPLITTER_EXT_BUFFER
        splitter->cbuffer.sram_discard(channel->internal.buffer.sram, data_to_remove);
#else
        PL_ASSERT(splitter->location == INTERNAL);
#endif
    }
}

/**
 * Helper function to copy/unpack from the internal to the output.
 */
static void unpack_data_from_internal_to_output(SPLITTER_OP_DATA *splitter, unsigned data_0, unsigned data_1, unsigned data_to_remove)
{
    SPLITTER_CHANNEL_STRUC *channel;

    SPLITTER_MSG3("Splitter Upack/copy to output buffer:"
            " data_0 = %d, data_1 = %d, data_to_remove = %d ",
            data_0, data_1, data_to_remove);
#ifdef SPLITTER_OFFLOAD
    /* First, make sure that there is no other buffer access running. */
    PL_ASSERT(!splitter->buffer_access);
    /* Set buffer_access to true even if the operator only runs in one core. */
    splitter->buffer_access = TRUE;
#endif


#ifdef SPLITTER_EXT_BUFFER
    if (splitter->location == SRAM)
    {
        /* Get access to the SRAM.*/
        extmem_lock(EXTMEM_SPI_RAM, FALSE);
    }
#endif
    /* Time to unpack/copy the data from the internal to the output */
    channel = splitter->channel_list;
    while (channel != NULL)
    {
        unpack_from_data_buffer(splitter, channel, data_0, data_1);

        if (data_to_remove)
        {
            /* Discard the unused data. */
            remove_from_data_buffer(splitter, channel, data_to_remove);
        }

        /* Move to next channel. */
        channel = channel->next;
    }/* END of channel loop. */

#ifdef SPLITTER_EXT_BUFFER
    if (splitter->location == SRAM)
    {
        /* Release the SRAM */
        extmem_unlock(EXTMEM_SPI_RAM);
    }
#endif

    /* It is necessary to kick forward whenever data is produced. */
    splitter->kick_forward = TRUE;
#ifdef SPLITTER_OFFLOAD
    /* Processing finished! */
    splitter->buffer_access = FALSE;
    SPLITTER_MSG("Splitter Upack/copy finished");
#endif
}


/**
 * Helper function which moves data from the internal buffer to the output buffers
 */
static bool move_data_from_internal_to_output(SPLITTER_OP_DATA *splitter)
{
    unsigned data_to_move[SPLITTER_MAX_OUTPUTS_PER_CHANNEL];
    unsigned internal_data_before[SPLITTER_MAX_OUTPUTS_PER_CHANNEL];
    unsigned data_to_remove;


    /* bail out early if the internal buffer is empty. */
    if (buff_metadata_available_octets(get_internal_metadata_buffer(splitter)) == 0)
    {
#if defined(SPLITTER_DEBUG)
        /* Only display the error message once*/
        if (splitter->internal_buffer_empty == FALSE)
        {
            splitter->internal_buffer_empty = TRUE;
            SPLITTER_MSG("Splitter: Internal buffer is empty, cannot copy to output!");
        }
#endif
        return FALSE;
    }
    SPLITTER_DEBUG_INSTR(splitter->internal_buffer_empty = FALSE);

    /* get the available data in the internal buffer */
    internal_buffer_data(splitter, internal_data_before);

    if (MAX(internal_data_before[0], internal_data_before[1]) == 0 )
    {
        SPLITTER_DEBUG_INSTR(splitter->internal_buffer_empty = TRUE);
        return FALSE;
    }
    SPLITTER_DEBUG_INSTR(splitter->internal_buffer_empty = FALSE);

    if((splitter->output_state[0] == INACTIVE)&&
       (splitter->output_state[1] == INACTIVE))
    {
        /* Both of the output streams are inactive. No need to copy to output */
        data_to_move[0] = 0;
        data_to_move[1] = 0;
        /* Remove the maximum amount possible. */
        data_to_remove = MAX(internal_data_before[0], internal_data_before[1]);
        SPLITTER_MSG3("Splitter: move_data_from_internal_to_output"
                "\n out 0 internal data before = %d"
                "\n out 1 internal data before = %d"
                "\n data_to_remove = %d",
                internal_data_before[0],
                internal_data_before[1],
                data_to_remove);
    }
    else
    {
        /* Limit the amount to copy to the available data */
        data_to_move[0] = internal_data_before[0];
        data_to_move[1] = internal_data_before[1];

        /* Limit the data to move based on the output space. */
        limit_data_to_move(splitter, data_to_move);

        /* Exit early if there is no place to copy. */
        if (MAX(data_to_move[0], data_to_move[1]) == 0 )
        {
            SPLITTER_DEBUG_INSTR(splitter->output_buffer_full = TRUE);
            return FALSE;
        }
        SPLITTER_DEBUG_INSTR(splitter->output_buffer_full = FALSE);

        /*
         * data_to_remove =
         *    MAX(internal_data_before[0], internal_data_before[1]) -
         *    MAX(internal_data_after[0], internal_data_after[1]);
         */
        data_to_remove = MAX(internal_data_before[0], internal_data_before[1]);
        data_to_remove = data_to_remove -  (
                MAX(internal_data_before[0] - data_to_move[0], internal_data_before[1] - data_to_move[1])
            );
        SPLITTER_MSG5("Splitter: move_data_from_internal_to_output"
                "\n out 0 internal data before = %d"
                "\n out 0 internal data after = %d"
                "\n out 1 internal data before = %d"
                "\n out 1 internal data after = %d"
                "\n data_to_remove = %d",
                internal_data_before[0],
                internal_data_before[0] - data_to_move[0],
                internal_data_before[1],
                internal_data_before[1] - data_to_move[1],
                data_to_remove);
    }


    /* copy the metadata to the active output streams */
    splitter_metadata_copy(splitter, data_to_move, data_to_remove);

#ifdef SPLITTER_OFFLOAD
    if (splitter->offload_enabled && audio_thread_offload_is_active())
    {
        splitter->offload_activated = TRUE;
        if(wait_for_buffer_access_end(splitter))
        {
            return FALSE;
        }

        /* Call offload */
        audio_thread_rpc_call(
                (uintptr_t)unpack_data_from_internal_to_output,
                (uintptr_t)splitter,
                (uintptr_t)data_to_move[0],
                (uintptr_t)data_to_move[1],
                (uintptr_t)data_to_remove);
    }
    else
#endif
    {
        /* Time to unpack/copy the data from the internal to the output */
        unpack_data_from_internal_to_output(splitter, data_to_move[0], data_to_move[1], data_to_remove);
    }

    return TRUE;

}

/**
 * Function which sets the read offset of one of the output streams.
 */
static inline void set_data_buffer_read_offset(SPLITTER_OP_DATA *splitter, SPLITTER_CHANNEL_STRUC *channel, unsigned output_index, unsigned read_offset)
{
    channel->internal.read_offset[output_index] = read_offset;
}

/**
 * Transition from INACTIVE to ACTIVE.
 *
 * Start copying to the output when the next metadata tag arrives.
 */
static bool splitter_transition_to_active(SPLITTER_OP_DATA *splitter, unsigned output_index)
{
    SPLITTER_CHANNEL_STRUC *channel;
    tCbuffer *metadata_buffer;
    metadata_tag *tag_list;
    unsigned b4idx;
    SPLITTER_OUTPUT_STATE output_state;
    unsigned read_index;
    unsigned read_offset;

    metadata_buffer = get_internal_metadata_buffer(splitter);

    tag_list = buff_metadata_peek_ex(metadata_buffer, &b4idx);

    output_state = get_current_output_state(splitter, output_index);
    if (output_state == HOLD)
    {
        /* Go trough the metadata and discard unwanted tags.*/
        unsigned channel_data_to_remove, input_data;
        unsigned data_to_remove;
        unsigned internal_data[SPLITTER_MAX_OUTPUTS_PER_CHANNEL];

        if (tag_list == NULL)
        {
            SPLITTER_ERRORMSG("splitter_transition_to_active: No tags to align to! Not enough data buffered. ");
            splitter->kick_backward = TRUE;
            return FALSE;
        }
        else
        {
            SPLITTER_ERRORMSG3("splitter_transition_to_active"
                    "\n first buffered tag timestamp 0x%08x index 0x%08x b4idx %d " ,
                    tag_list->timestamp, tag_list->index, b4idx);
        }
        /* Check if timestamp too far in past. */
        if (IS_TIMESTAMPED_TAG(tag_list) && (tag_list->timestamp >= splitter->transition_point))
        {
            SPLITTER_ERRORMSG("Splitter: Timestamp too far in the past. Maybe you need a bigger buffer. ");
        }

        /* Discard the data from the previous tag */
        channel_data_to_remove = b4idx;
        input_data = buff_metadata_available_octets(metadata_buffer);

        /* Go through the metadata until the last tag. */
        while((tag_list->next != NULL) &&
              (input_data != 0) &&
              (IS_TIMESTAMPED_TAG(tag_list) || IS_TIME_OF_ARRIVAL_TAG(tag_list)) &&
              (tag_list->timestamp < splitter->transition_point))
        {
            unsigned tag_data;
            /* tag_legth = tag_list->length / data_size */
            tag_data = MIN(input_data, tag_list->length);

            if (tag_data != 0)
            {
                SPLITTER_ERRORMSG2("Splitter: Metadata tag with timestamp 0x%08x and "
                        "index 0x%08x will be discarded.",
                        tag_list->timestamp, tag_list->index);
                /* Increase the discarded amount by the tag length. */
                channel_data_to_remove += tag_data;
                /* ... decrease the amount of data. */
                input_data -= tag_data;
            }
            tag_list = tag_list->next;

        }

        /* Check if all data was discarded.*/
        if (tag_list->next == NULL)
        {
            SPLITTER_ERRORMSG("Splitter: All buffered data discarded. Timestamp in future? ");
        }
        /* Sanity check the result of the discard. */
        if (tag_list == NULL)
        {
            SPLITTER_ERRORMSG("Splitter: No more metadata tag at the input buffer. Probably the buffering is too small. ");
            return FALSE;
        }
        else if (input_data == 0)
        {
            SPLITTER_ERRORMSG("Splitter: No data left to search for timestamp (probably the buffering is too small). "
                    "Activating output anyways.");
        }
        else if (!IS_TIMESTAMPED_TAG(tag_list) && !IS_TIME_OF_ARRIVAL_TAG(tag_list))
        {
            while((tag_list->next != NULL) &&
                  (input_data != 0))
            {
                unsigned tag_data;
                /* tag_legth = tag_list->length / data_size */
                tag_data = MIN(input_data, tag_list->length);

                if (tag_data != 0)
                {
                    SPLITTER_ERRORMSG2("Splitter: Metadata tag with timestamp 0x%08x and "
                            "index 0x%08x will be discarded.",
                            tag_list->timestamp, tag_list->index);
                    /* Increase the discarded amount by the tag length. */
                    channel_data_to_remove += tag_data;
                    /* ... decrease the amount of data. */
                    input_data -= tag_data;
                }
                tag_list = tag_list->next;
            }
            SPLITTER_ERRORMSG2("Splitter: Non timestamped tag found.\n"
                    " All data will be discarded and output stream will be activated!\n"
                    " Tag with index 0x%08x is the last tag in the buffer"
                    "\n Total discarded octets %d",
                    tag_list->index, channel_data_to_remove);
        }
        else
        {
            SPLITTER_ERRORMSG4("Splitter: Tag with ttp 0x%08x and index 0x%08x "
                    "is the first valid tag after transition 0x%08x"
                    "\n Total discarded octets %d",
                    tag_list->timestamp, tag_list->index, splitter->transition_point, channel_data_to_remove);
        }


        /* get the available data in the internal buffer */
        internal_buffer_data(splitter, internal_data);
        SPLITTER_MSG2("Splitter splitter_transition_to_active"
                "\n out 0 internal data before = %d"
                "\n out 1 internal data before = %d",
                internal_data[0],
                internal_data[1]);
        /*
         * data_to_remove =
         *    MAX(internal_data_before[0], internal_data_before[1]) -
         *    MAX(internal_data_after[0], internal_data_after[1]);
         */
        data_to_remove = MAX(internal_data[0], internal_data[1]);
        /* remove the data from the current channel */
        internal_data[output_index] = internal_data[output_index] - channel_data_to_remove;
        data_to_remove = data_to_remove - MAX(internal_data[0], internal_data[1]);

        SPLITTER_MSG3("Splitter splitter_transition_to_active"
                "\n out 0 internal data after = %d"
                "\n out 1 internal data after = %d"
                "\n data_to_remove = %d",
                internal_data[0],
                internal_data[1],
                data_to_remove);

        /* This will remove data from all channels. */
        if (data_to_remove !=0)
        {
            /* The channel is still in hold. so by moving data
             * the data will be discarded from the channel.
             */
            unsigned data_0 = 0, data_1 = 0;
            if (output_index == 0)
            {
                data_0 = data_to_remove;
            }
            else
            {
                data_1 = data_to_remove;
            }
            /* remove the metadata */
            remove_metadata_from_internal(splitter, data_to_remove);
            /* remove the data by copying 0 0 but  removing data */
            unpack_data_from_internal_to_output(splitter, data_0, data_1, data_to_remove);
        }

        read_index = tag_list->index;
    }
    else
    {
        /* make sure that the operator is in INACTIVE state. */
        SPLITTER_DEBUG_INSTR(PL_ASSERT(output_state == INACTIVE));
        if (tag_list == NULL)
        {
            if (splitter->reframe_enabled)
            {
                /* Because re-frame is enabled the next tag will
                 * have no b4idx. in this manner we can use the
                 * write index. */
                read_index = metadata_buffer->metadata->prev_wr_index;
            }
            else
            {
                SPLITTER_ERRORMSG("splitter_transition_to_active: "
                        "No tags to align to and re-frame is disabled. ");
                return FALSE;
            }
        }
        else
        {
            SPLITTER_MSG3("splitter_transition_to_active"
                    "\n tag_list 0x%08x  tag_list->index 0x%08x b4idx %d " ,
                    tag_list, tag_list->index, b4idx);
            read_index = tag_list->index;
            /* Remove data until the tag start.*/
            if((get_current_output_state(splitter, opposite_output_index(output_index)) == INACTIVE)&&(b4idx != 0))
            {
                /* remove the metadata */
                remove_metadata_from_internal(splitter, b4idx);
                /* remove the data by copying 0 0 but  removing data */
                unpack_data_from_internal_to_output(splitter, 0, 0, b4idx);
            }
        }
    }

    /* set the metadata read index and head. */
    splitter->internal_metadata.prev_rd_indexes[output_index] = read_index;
    splitter->internal_metadata.head_tag[output_index] = tag_list;

    /* Convert the read index to buffer read offset */
    read_offset = metadata_read_idx_to_buffer_offset(splitter, read_index);

    /* set the read pointer for all channels */
    channel = splitter->channel_list;
    while (channel != NULL)
    {
        /* Change state.*/
        set_current_output_state(splitter, output_index, ACTIVE);

        /* set the read offset in the buffer.*/
        set_data_buffer_read_offset(splitter, channel, output_index,  read_offset);

        channel = channel->next;
    }

    return TRUE;
}

static bool splitter_finish_copying_last_tag(SPLITTER_OP_DATA *splitter, unsigned output_index)
{
    tCbuffer *metadata_buffer;
    metadata_tag *tag_list, *before_head_tag=NULL;
    unsigned b4idx, before_prev_rd_indexes = 0;
    SPLITTER_OUTPUT_STATE output_state;

    metadata_buffer = get_internal_metadata_buffer(splitter);

    /* Save the index value. */
    before_prev_rd_indexes = metadata_buffer->metadata->prev_rd_index;
    before_head_tag = metadata_buffer->metadata->tags.head;

    /* Set up the internal buffer read pointers!*/
    metadata_buffer->metadata->prev_rd_index = splitter->internal_metadata.prev_rd_indexes[output_index];
    metadata_buffer->metadata->tags.head = splitter->internal_metadata.head_tag[output_index];

    /* b4idx will indicate the octets remaining from the last tag. */
    tag_list = buff_metadata_peek_ex(metadata_buffer, &b4idx);

    /* restore the index value. */
    metadata_buffer->metadata->prev_rd_index = before_prev_rd_indexes;
    metadata_buffer->metadata->tags.head = before_head_tag;


    SPLITTER_MSG3("splitter_finish_copying_last_tag"
            "\n tag_list 0x%08x  tag_list->index 0x%08x b4idx %d octets" ,
            tag_list, tag_list->index, b4idx);

    if (tag_list == NULL)
    {
        /* Would be nice to code against this. */
        return FALSE;
    }

    output_state = get_current_output_state(splitter, output_index);
    if (output_state == ACTIVE)
    {
        unsigned data_before[SPLITTER_MAX_OUTPUTS_PER_CHANNEL] = {0,0};
        unsigned data_to_move[SPLITTER_MAX_OUTPUTS_PER_CHANNEL] = {0,0};
        unsigned data_to_remove = 0;

        /* Get the amount of date buffered internally. */
        internal_buffer_data(splitter, data_before);

        /* finish copying the current tag. */
        data_to_move[output_index] = b4idx;

        SPLITTER_MSG3("splitter_finish_copying_last_tag:"
                   "\n output_index %d  data_to_move[0] %d data_to_move[1] %d" ,
                   output_index, data_to_move[0], data_to_move[1]);

        /* Limit the data to move based on the output space. */
        limit_data_to_move(splitter, data_to_move);

        SPLITTER_MSG3("splitter_finish_copying_last_tag:"
                   "\n after limit: data_to_move[0] %d data_to_move[1] %d b4idx %d " ,
                   data_to_move[0], data_to_move[1], b4idx);
        if (data_to_move[output_index] != b4idx)
        {
            SPLITTER_ERRORMSG("splitter_finish_copying_last_tag:"
                       " Not enough space to finish copying tag!" );
            return FALSE;
        }

        /*
         * data_to_remove =
         *    MAX(internal_data_before[0], internal_data_before[1]) -
         *    MAX(internal_data_after[0], internal_data_after[1]);
         */
        data_to_remove = MAX(data_before[0], data_before[1]);
        data_to_remove = data_to_remove -  (
                MAX(data_before[0] - data_to_move[0], data_before[1] - data_to_move[1])
            );
        SPLITTER_MSG3("splitter_finish_copying_last_tag:"
                   "\n BATTER: data_before[0] %d data_before[1] %d data_to_remove %d " ,
                   data_before[0], data_before[1], data_to_remove);

        /* copy the metadata to the active output streams */
        splitter_metadata_copy(splitter, data_to_move, data_to_remove);

        /* Finish copying last tag. */
        unpack_data_from_internal_to_output(splitter, data_to_move[0], data_to_move[1], data_to_remove);
    }

    SPLITTER_MSG("splitter_finish_copying_last_tag: finished");
    return TRUE;

}


/**
 * Function which invalidates the read position.
 */
static inline void invalidate_read_offset(SPLITTER_CHANNEL_STRUC *channel, unsigned output_index)
{
    channel->internal.read_offset[output_index] = -1;
}

/**
 * Transition from any state to INACTIVE.
 *
 */
static bool splitter_transition_to_inactive(SPLITTER_OP_DATA *splitter, unsigned output_index)
{
    SPLITTER_CHANNEL_STRUC *channel;
    unsigned data_to_remove;
    unsigned internal_data_before[SPLITTER_MAX_OUTPUTS_PER_CHANNEL];
    unsigned internal_data_after[SPLITTER_MAX_OUTPUTS_PER_CHANNEL];

    if (splitter->finish_last_tag)
    {
        /* If copying the last tag failed, return failure */
        if (!splitter_finish_copying_last_tag(splitter, output_index))
        {
            return FALSE;
        }
    }

    /* get the available data in the internal buffer */
    internal_buffer_data(splitter, internal_data_before);

    /* set the metadata read index and head. */
    splitter->internal_metadata.prev_rd_indexes[output_index] = -1;
    splitter->internal_metadata.head_tag[output_index] = (metadata_tag *)-1;

    /* Deactivate all output streams with index "output_index". */
    channel = splitter->channel_list;
    while (channel != NULL)
    {
        /* Change state.*/
        set_current_output_state(splitter, output_index, INACTIVE);

        /* For security invalidate the read position.*/
        invalidate_read_offset(channel, output_index);

        channel = channel->next;
    }

    /* get the available data in the internal buffer */
    internal_buffer_data(splitter, internal_data_after);
    /*
     * data_to_remove =
     *    MAX(internal_data_before[0], internal_data_before[1]) -
     *    MAX(internal_data_after[0], internal_data_after[1]);
     */
    data_to_remove = MAX(internal_data_before[0], internal_data_before[1]);
    data_to_remove = data_to_remove - MAX(internal_data_after[0], internal_data_after[1]);
    SPLITTER_MSG5("Splitter splitter_transition_to_inactive"
            "\n out 0 internal data before = %d"
            "\n out 0 internal data after = %d"
            "\n out 1 internal data before = %d"
            "\n out 1 internal data after = %d"
            "\n data_to_remove = %d",
            internal_data_before[0],
            internal_data_after[0],
            internal_data_before[1],
            internal_data_after[1],
            data_to_remove);
    /* Now remove the data */
    if (data_to_remove !=0)
    {
        /* remove the metadata */
        remove_metadata_from_internal(splitter, data_to_remove);
        /* remove the data by copying 0 0 but  removing data */
        unpack_data_from_internal_to_output(splitter, 0, 0, data_to_remove);
    }

    return TRUE;
}



/**
 * Transition to HOLD.
 */
static bool splitter_transition_to_hold(SPLITTER_OP_DATA *splitter, unsigned output_index)
{
    SPLITTER_CHANNEL_STRUC *channel;
    tCbuffer *metadata_buffer;
    unsigned read_index;
    unsigned read_offset;

    metadata_buffer = get_internal_metadata_buffer(splitter);
    /* set the metadata read index and head. */
    read_index = metadata_buffer->metadata->prev_rd_index;
    splitter->internal_metadata.prev_rd_indexes[output_index] = read_index;
    splitter->internal_metadata.head_tag[output_index] = metadata_buffer->metadata->tags.head;

    /* Convert the read index to buffer read offset */
    read_offset = metadata_read_idx_to_buffer_offset(splitter, read_index);

    /* Put output streams with index "output_index" on hold. */
    channel = splitter->channel_list;
    while (channel != NULL)
    {
        /* Change state.*/
        set_current_output_state(splitter, output_index, HOLD);

        /* set the read offset in the buffer.*/
        set_data_buffer_read_offset(splitter, channel, output_index, read_offset);
        /* If the other output is disconnected set the new read offset.*/
        if (get_current_output_state(splitter, opposite_output_index(output_index)) == INACTIVE)
        {
            set_buffer_rd_offset(splitter, &channel->internal.buffer, read_offset);
        }

        channel = channel->next;
    }

    return TRUE;
}


/**
 * Function which handles the transitions for the splitter capability.
 */
bool splitter_transition(SPLITTER_OP_DATA *splitter)
{
    SPLITTER_CHANNEL_STRUC *channel;
    unsigned i;
    SPLITTER_OUTPUT_STATE next_state;
    SPLITTER_OUTPUT_STATE current_state;
    bool retval = TRUE;

    channel = splitter->channel_list;
    if (channel == NULL)
    {
        /* There are no connections, the transition will
         * be handled by connect or channel creation.
         */
        return TRUE;
    }

    SPLITTER_MSG("splitter_transition method called!");

    for (i=0; i<SPLITTER_MAX_OUTPUTS_PER_CHANNEL; i++)
    {
        /* Get the new state for the output stream. */
        next_state = get_next_output_state(splitter, i);
        current_state = get_current_output_state(splitter, i);

        SPLITTER_MSG3("splitter_transition: Output stream %d transition from %d to %d "
                "(where 0 INACTIVE, 1 ACTIVE, 2 HOLD)",
                i, current_state, next_state);

        /* First metadata should be solved.*/
        if (current_state != next_state)
        {
            bool transition;

            switch (next_state)
            {
                case INACTIVE:
                {
                    transition = splitter_transition_to_inactive(splitter, i);
                    break;
                }
                case ACTIVE:
                {
                    transition = splitter_transition_to_active(splitter, i);
                    break;
                }
                case HOLD:
                {
                    transition = splitter_transition_to_hold(splitter, i);
                    break;
                }
                default:
                {
                    transition = FALSE;
                    PL_ASSERT(next_state < NR_OF_STATE);
                }
            }
            /* All channel transition must be terminated to
             * finish the overall transition. */
            retval = transition && retval;
        }/* end of state transition. */
    }
    SPLITTER_MSG1("splitter_transition: returns %d", retval);
    /* If this is a debug build check if the buffers are still aligned. */
    SPLITTER_DEBUG_INSTR(check_buffers_validity(splitter));
    return retval;
}



/**
 * Helper function which is the core method for internal/external buffering.
 */
static inline void splitter_process_data_buffering(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    SPLITTER_OP_DATA *splitter = get_instance_data(op_data);
    /* Data to pack/copy. and the actual copies/packed data.*/
    unsigned data_to_pack;
    bool in_transition = splitter->in_transition;
    INTERVAL kick_period;
    patch_fn(splitter_process_data);

    timer_cancel_event_atomic(&splitter->self_kick_timer);


    /*Calculate the min available data at the input */
    data_to_pack = amount_to_pack(splitter);

    SPLITTER_MSG1("Splitter data_to_pack 0x%08x!", data_to_pack);

    /* Try to finish the transition.*/
    if (in_transition)
    {

        SPLITTER_MSG("Splitter in transition!");
        /* Transition based on timestamp can wait for a specific timestamp.
         * Pack the input without offload to avoid dead lock. */
        if (data_to_pack != 0)
        {
            SPLITTER_MSG1("Splitter in transition! First pack data %d", data_to_pack);
            pack_to_internal(splitter, data_to_pack, FALSE);
            /* No more data to pack. */
            data_to_pack = 0;
        }
        if (splitter_transition(splitter))
        {
            /* This time we managed to finish transitioning .*/
            in_transition = FALSE;
            /* Save the new state. */
            splitter->in_transition = in_transition;
        }
    }

    /* Don't bother packing/unpacking until transition is unfinished. */
    if (!in_transition)
    {
#ifdef SPLITTER_OFFLOAD
        if (splitter->offload_enabled)
        {
            if (!splitter->next_run_unpack)
            {
                /* Pack data to the internal buffer only if packing is not in progress. */
                if (data_to_pack != 0)
                {
                    /* Pack/Copy to internal buffer. */
                    pack_to_internal(splitter, data_to_pack, TRUE);
                }
                splitter->next_run_unpack = TRUE;
            }
            else
            {
                /* Unpack/Copy from internal to the output as much as possible*/
                move_data_from_internal_to_output(splitter);
                splitter->next_run_unpack = FALSE;
            }
        }
        else
#endif
        {
            /* Pack data to the internal buffer only if packing is not in progress. */
            if (data_to_pack != 0)
            {
                /* Pack/Copy to internal buffer. */
                pack_to_internal(splitter, data_to_pack, FALSE);
            }
            /* Unpack/Copy from internal to the output as much as possible*/
            move_data_from_internal_to_output(splitter);
        }
        /* If this is a debug build check if the buffers are still aligned. */
        SPLITTER_DEBUG_INSTR(check_buffers_validity(splitter));
    }

    /* Kick backwards and forwards */
    if (splitter->kick_backward)
    {
        splitter->kick_backward = FALSE;
        touched->sinks = splitter->touched_sinks;
    }
    if (splitter->kick_forward)
    {
        splitter->kick_forward = FALSE;
        touched->sources = splitter->touched_sources;
    }
    kick_period = stream_if_get_system_kick_period();
#ifdef SPLITTER_OFFLOAD
    if (!splitter->offload_activated)
    {
        /* Kicks are only used for stall detection when
         * offloading is not enabled. */
        kick_period = kick_period * 2;
    }
#endif
    timer_schedule_event_in_atomic(kick_period,
        splitter_timer_task, (void*)op_data, &splitter->self_kick_timer);
}
#endif


/****************************************************************************
Public Function Definitions
*/
/********************    process data   ****************************/
/**
 * \brief The main data processing function only selects the
 *  right data processing based on the working mode.
 *
 */
void splitter_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    /* The comment after the function call helps the ACAT to find the extra operator data
     * for old implementations where the data type is not save in the elf. */
    SPLITTER_OP_DATA *splitter = get_instance_data(op_data); /* old way = (SPLITTER_OP_DATA *)(op_data->extra_op_data);*/

    SPLITTER_MSG1("#### SPLITTER 0x%08x ####",base_op_get_ext_op_id(op_data));

    switch (splitter->working_mode)
    {
        case CLONE_BUFFER:
            splitter_process_data_clone(op_data, touched);
            break;
#ifdef INSTALL_METADATA
        case BUFFER_DATA:
            splitter_process_data_buffering(op_data, touched);
            break;
#endif
        default:
            SPLITTER_ERRORMSG1("SPLITTER Error, unsupported mode: %d.",splitter->working_mode);
            /* Check if the mode is valid one or is just unsupported.*/
            PL_ASSERT(splitter->working_mode < NR_OF_MODES);
            break;
    }

}

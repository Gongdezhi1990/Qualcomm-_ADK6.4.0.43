/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 *
 * \defgroup sco_fw Sco
 * \file  sco_fw_c.c
 *
 * SCO fw source
 *
 */

/****************************************************************************
Include Files
*/

#include "sco_fw_c.h"
#include "buffer/cbuffer_c.h"
#include "pmalloc/pl_malloc.h"
#include "fault/fault.h"
#include "types.h"
#include "opmgr/opmgr_for_ops.h"
#ifdef SCO_RX_OP_GENERATE_METADATA
#include "ttp/ttp.h"
#endif
#include "patch/patch.h"
/****************************************************************************
Private Constant Definitions
*/

/**
 * Constant which defines after how many out of time packets should we reset
 * sco receive timestamp variables.
 */
#define OUT_OF_TIME_PKT_LIMIT (20)

/**
 * Sync word.
 */
#define SYNC_WORD (0x5c5c)


/****************************************************************************
Private Macro Declarations
*/

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private Function Definitions
*/

/**
 * TODO
 */
static inline int transform_metadata(int data, CONNECTION_TYPE type)
{
    if (type == SCO_NB)
    {
        return SCO_NB_METADATA_TRANSFORM(data);
    }
    else
    {
        return WBS_METADATA_TRANSFORM(data);
    }
}
/**
 * TODO
 */
static inline void transform_metadata_header(int *data, CONNECTION_TYPE type, stream_sco_metadata *sco_metadata)
{
    int status_val;

    /* Read the length of the metadata. (in words). */
    sco_metadata->metadata_length = transform_metadata(data[0], type);

    /* The length of the associated data packet, in octets. */
    sco_metadata->packet_length = transform_metadata(data[1], type);

    /* Read the status of the sco packet. */
    status_val = transform_metadata(data[2], type);
    if (status_val & (~MASK_16_BIT_VAL))
    {
        SCO_DBG_MSG("Error! Metadata reserved not zero. This probably means that metadata "
                "transform function is incorrect or the status and the reserve bits are swapped. ");
        fault_diatribe(FAULT_AUDIO_INVALID_SCO_METADATA_RESERVE_FIELD_NOT_ZERO, status_val);
    }
    sco_metadata->status = MASK_16_BIT(status_val);

    /* And finally  read the sco time stamp. */
    sco_metadata->time_stamp = transform_metadata(data[3], type);
}

/****************************************************************************
Public Function Definitions
*/

/**
 *  Checks if there is enough data in the input buffer.
 *
 * \param sco_data - Pointer to the common SCO rcv operator data
 * \param data_size - Size of the data which needs reading from the input.
 * \returns If there is enough data, FALSE otherwise
 */
bool enough_data_to_run(SCO_COMMON_RCV_OP_DATA* sco_data, unsigned data_size)
{
    unsigned input_data;

    /* calc input data */
    input_data = cbuffer_calc_amount_data_in_words(sco_data->buffers.ip_buffer);

#ifdef SCO_RX_OP_GENERATE_METADATA
    if(buff_has_metadata(sco_data->buffers.ip_buffer))
    {
        /* make sure we don't consume more than what metadata indicates */
        unsigned input_meta = buff_metadata_available_octets(sco_data->buffers.ip_buffer)/OCTETS_PER_SAMPLE;
        if(input_meta < input_data)
        {
            SCO_DBG_MSG2("SCO_RCV_DBG, Less metadata than data: data=%d, meta=%d, capped", input_data, input_meta);
            input_data = input_meta;
        }
    }
#endif

    SCO_DBG_MSG1("enough_data_to_run: input buffer data |%4d|", input_data);

    /* Check for run condition.
     * Input packet can have no length.
     */
    if (input_data < data_size)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/**
 *  Checks if there is enough space in the output buffer.
 *
 * \param sco_data - Pointer to the common SCO rcv operator data
 * \param output_packet_size - Size of the packet which need writing to the output.
 * \returns If there is enough space, FALSE otherwise
 */
bool enough_space_to_run(SCO_COMMON_RCV_OP_DATA* sco_data, unsigned output_packet_size)
{
    unsigned output_space;

    /* calc output space */
    output_space = cbuffer_calc_amount_space_in_words(sco_data->buffers.op_buffer);

    SCO_DBG_MSG1("enough_space_to_run: output buffer space |%4d|.", output_space);

    /* Check for run condition.
     * Only one packet is decoded per run.
     */
    if (output_packet_size > output_space)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/**
 *  Reads the metadata of the incoming packet and populates it to the stream sco packet
 *  structure.
 *
 * \param ip_buffer - Pointer to the input buffer of the SCO rcv operator.
 * \param in_packet - Pointer to the packet structure to read.
 * \param type - Type of the packet. 0 SCO, 1 WBS.
 * \returns the status of the read.
 */
stream_sco_metadata_status read_packet_metadata(SCO_COMMON_RCV_OP_DATA* sco_data, stream_sco_metadata *sco_metadata, CONNECTION_TYPE type)
{
    int data[METADATA_HEADER_SIZE - 1];
    tCbuffer *ip_buffer = sco_data->buffers.ip_buffer;
    unsigned ip_buffer_data = cbuffer_calc_amount_data_in_words(ip_buffer);

    patch_fn(read_packet_metadata);

#ifdef SCO_RX_OP_GENERATE_METADATA
    if(buff_has_metadata(ip_buffer))
    {
        /* make sure we don't consume more than what metadata indicates */
        unsigned input_meta = buff_metadata_available_octets(ip_buffer) / OCTETS_PER_SAMPLE;
        if(input_meta < ip_buffer_data)
        {
            SCO_DBG_MSG2("SCO_RCV_DBG, Less metadata than data: data=%d, meta=%d, capped", ip_buffer_data, input_meta);
            ip_buffer_data = input_meta;
        }
    }
#endif

    /* Search for the sync word until it is possible to read a sco metadata header.*/
    while(ip_buffer_data >= METADATA_HEADER_SIZE)
    {
        /* There is data on the ip buffer so the read must always pass. */
#ifdef SCO_RX_OP_GENERATE_METADATA
        /* keep metadata aligned with the buffer */
        sco_rcv_transport_metadata(sco_data, 1, 0, type);
#endif
        cbuffer_read(ip_buffer, data, 1);
        /* Decrement the available data in the input buffer. */
        ip_buffer_data--;

        if (transform_metadata(data[0], type) == SYNC_WORD)
        {

            /* Syncword found, read the rest of the metadata.
             * There are enough data because we check at the while statement. */
#ifdef SCO_RX_OP_GENERATE_METADATA
            /* at this time the last read tag must be valid, otherwise
             * it's most probably an input buffer wrap around has
             * happened
             */
            if(buff_has_metadata(ip_buffer) &&
               !sco_data->last_tag.valid)
            {
                SCO_DBG_MSG("Error! sync was seen when metadata tag invalid, perhaps metadata and buffer are out of sync");
                return SCO_METADATA_INVALID;
            }
            /* keep metadata aligned with the buffer */
            sco_rcv_transport_metadata(sco_data, METADATA_HEADER_SIZE - 1, 0, type);
#endif /* SCO_RX_OP_GENERATE_METADATA */
            cbuffer_read(ip_buffer, data, METADATA_HEADER_SIZE - 1);
            /* Decrement the available data in the input buffer. */
            ip_buffer_data = ip_buffer_data - (METADATA_HEADER_SIZE - 1);

            /* Transform the rest of the metadata and check if it is valid. */
            transform_metadata_header(data, type, sco_metadata);

            /* check if the SCO metadata length is valid.*/
            if ( sco_metadata->metadata_length != METADATA_HEADER_SIZE )
            {
                SCO_DBG_MSG1("Error! Invalid metadata length  %d. ",
                        sco_metadata->metadata_length);
                return SCO_METADATA_INVALID;
            }

            /* check if the SCO packet length is plausible. */

            /*
             * If the BT LC has been unable to service SCO for an extended
             * period, when it returns, it can write a single metadata
             * record with a length big enough to cover all the frames it
             * missed.
             * This can wrap around the buffer one or more times. Ideally
             * the LC wouldn't do that (B-111114) but it does; we can cope
             * by just moving our read offset modulo the buffer size, so
             * that it points to where the next record should turn up.
             *
             * (It isn't guaranteed that we get as far as this graceful
             * handling. Hopefully we should turn up in time before
             * another record is written; if we're late then the BTSS
             * could have overwritten the record we're trying to read with
             * a new one. If the wrap is unlucky we might turn up on time
             * but think there's no header because the buffer offset
             * doesn't let us see it, in which case we'd give up and the
             * first header would be overwritten and we'd get confused.
             * If any of this happens we'll likely lose sync, which will
             * be dealt with elsewhere.)
             */
            {
                unsigned wrapped_packet_len
                    = sco_metadata->packet_length
                    % CONVERT_SAMPLES_TO_OCTETS(cbuffer_get_size_in_words(ip_buffer));

                if (wrapped_packet_len != sco_metadata->packet_length)
                {
                    /* Record bigger than buffer; probably the above-mentioned
                     * wrap occurred. */
                    SCO_DBG_MSG3("SCO metadata record length (%d octets) "
                                 "bigger than buffer! Assuming buffer wrap "
                                 "and advancing pointer by only %d instead. "
                                 "(status = %d)",
                                 sco_metadata->packet_length,
                                 wrapped_packet_len, sco_metadata->status);
                    sco_metadata->packet_length = wrapped_packet_len;
                    /* We'd expect such a huge record to have NEVER_SCHEDULED
                     * status. */
                    if (sco_metadata->status == OK
                        || sco_metadata->status == CRC_ERROR)
                    {
                        /* The situation is blatantly worse than the record's
                         * status claims, so ignore the record's status. */
                        sco_metadata->status = NOTHING_RECEIVED;
                        SCO_DBG_MSG1("...that status was clearly bogus, "
                                     "using %d instead", sco_metadata->status);
                    }
                }

                /*
                 * We found less data in the buffer than the metadata record
                 * promised (even having accounted for the possibility of a
                 * huge buffer-wrapping record above).
                 * http://wiki/Kalimba_stream_metadata_format says "The LC
                 * will guarantee that the index always points to the end
                 * of a packet, not the middle of the metadata or the start
                 * or middle of the packet (i.e., the LC updates the public
                 * index atomically)."
                 * So this shouldn't happen.
                 */
                if (CONVERT_OCTETS_TO_SAMPLES(sco_metadata->packet_length) > ip_buffer_data)
                {
                    SCO_DBG_MSG2("Error! SCO metadata record implies "
                                 "%d octets in buffer but there are only "
                                 "%d words. Giving up.",
                                 sco_metadata->packet_length, ip_buffer_data);
                    return SCO_METADATA_INVALID;
                }
            }

            /* All checks passed, metadata is valid. */
            return SCO_METADATA_READY;
        }
    }

    SCO_DBG_MSG("read_packet_metadata: Failed to read SCO metadata.");
    return SCO_METADATA_INVALID;
}

/**
 *  Check the timestamp and decide if given timestamp is for a late/expected or early packet.
 *
 * \param sco_data - Pointer to the common SCO rcv operator data
 * \param in_packet - Packet to analyse.
 * \returns Status of the analyse.
 */
stream_sco_metadata_status analyse_sco_metadata(SCO_COMMON_RCV_OP_DATA* sco_data, stream_sco_metadata *sco_metadata)
{
    unsigned time_stamp;

    patch_fn(analyse_sco_metadata);

    time_stamp = sco_metadata->time_stamp;

    /*
     * Read in the expected time stamp
     */
    if (sco_data->sco_rcv_parameters.expected_time_stamp == -1)
    {
        SCO_DBG_MSG1("Kick nr %d. expected_time_stamp == -1. Packet on time. ",
                                                sco_data->sco_rcv_parameters.md_num_kicks);

        /* packet_on_time */
        sco_data->sco_rcv_parameters.expected_time_stamp = time_stamp;

        return SCO_METADATA_ON_TIME;
    }

    if (sco_data->sco_rcv_parameters.t_esco == 0)
    {
        /* tesco not initialised yet. Assume expected TS is TS from last packet. */

        if (time_stamp < sco_data->sco_rcv_parameters.expected_time_stamp)
        {
            SCO_DBG_MSG2("Warning! t_esco == 0. Packet time %d < Expected time %d. Packet late. ",
                                                time_stamp,
                                                sco_data->sco_rcv_parameters.expected_time_stamp);

            /* This is definitely not possible. But cover for this. */
            return SCO_METADATA_LATE;
        }

        /*
         * Initialise T(esco) to difference between prev packet and the current packet
         * This is risky as we could have missed a packet. But can't do much more if
         * these fields are not initialised.
         * halve the TS difference as it is in BT ticks and T(esco) is in slots (= 2 BT Ticks)
         */
        sco_data->sco_rcv_parameters.t_esco = (time_stamp - sco_data->sco_rcv_parameters.expected_time_stamp)/ 2;
        /* update expected packet to actual TS */
        sco_data->sco_rcv_parameters.expected_time_stamp = time_stamp;
        return SCO_METADATA_ON_TIME;
    }

    /*
     * ETn could have wrapped when updated last time. Or it could wrap in the next
     * half T(esco) ticks. These cases need to be accounted for. Luckily, comparison
     * of 2 16-bit Timestamps are done in 24-bit (or higher) processor.
     * Most often ETn and ATn matches.
     */
    if (time_stamp == sco_data->sco_rcv_parameters.expected_time_stamp )
    {
        return SCO_METADATA_ON_TIME;
    }

    /*
     * All other case are complicated as ATn and ETn could be
     * at the edge of 16-bit number wrap.
     * Look for next 8 packets boundaries, as in hydra there could be multiple
     * packets per kick. (r0 is T(esco_in_ticks)/2, shift it by 4)
     */
    if (sco_data->sco_rcv_parameters.expected_time_stamp & 0x10000)
    {
        /* Overflow occurred in the expected_time_stamp. */
        if (sco_data->sco_rcv_parameters.expected_time_stamp < (sco_data->sco_rcv_parameters.t_esco << 4))
        {

            SCO_DBG_MSG1("Warning! Packet time overflow %d, Changed to packet time %d. ", time_stamp);
            time_stamp |= 0x10000;
        }
        else
        {
            sco_data->sco_rcv_parameters.expected_time_stamp &= 0xFFFF;
        }

    }

    /* All adjustments to TS have been made to account for any wrapping.
     * Late if ATn < ETn - T(esco_in_ticks) /2
     */
    if (time_stamp < sco_data->sco_rcv_parameters.expected_time_stamp - sco_data->sco_rcv_parameters.t_esco)
    {
        SCO_DBG_MSG3("Warning! Packet time %d < Expected time %d - t_esco %d /2. Packet late. ",
                                            time_stamp,
                                            sco_data->sco_rcv_parameters.expected_time_stamp,
                                            sco_data->sco_rcv_parameters.t_esco);
        return SCO_METADATA_LATE;
    }


    /* Early if ETn + T(esco_in_ticks) /2 < ATn */
    if (time_stamp > sco_data->sco_rcv_parameters.expected_time_stamp + sco_data->sco_rcv_parameters.t_esco)
    {
        SCO_DBG_MSG3("Warning! Packet time %d > Expected time %d + t_esco %d /2. Packet early. ",
                                            time_stamp,
                                            sco_data->sco_rcv_parameters.expected_time_stamp,
                                            sco_data->sco_rcv_parameters.t_esco);
        return SCO_METADATA_EARLY;
    }

    return SCO_METADATA_ON_TIME;
}

/**
 * \brief Discards an invalid packet form the input buffer.
 *
 * \param sco_data - Pointer to the common SCO rcv operator data
 * \param discard_packet - Packet to discard.
 */
void discard_packet(SCO_COMMON_RCV_OP_DATA* sco_data, stream_sco_metadata* sco_metadata)
{
    unsigned amount_to_discard = get_packet_payload_length(sco_metadata);
    unsigned amount_available = cbuffer_calc_amount_data_in_words(sco_data->buffers.ip_buffer);
    if(amount_to_discard > amount_available)
    {
        /* limit the amount to discard, so metadata tags moves the same amount as the cbuffer does,
         * perhaps not necessary in practice, since if we don't have a full packet of data
         * in the buffer, a wrap around has happened and that should be addressed later by
         * clearing the entire buffer.
         */
        amount_to_discard = amount_available;
    }

    SCO_DBG_MSG1("Error! Discard packet. Packet length %d.", sco_metadata->packet_length);
#ifdef SCO_RX_OP_GENERATE_METADATA
    /* remove metadata associated with the amount to discard from main buffer */
    sco_rcv_transport_metadata(sco_data,
                               amount_to_discard, /* amount_processed */
                               0,                 /* amount generated */
                               0);                /* type, no effect when amount generated=0*/
#endif
    cbuffer_advance_read_ptr(sco_data->buffers.ip_buffer, amount_to_discard);
}

#ifdef SCO_RX_OP_GENERATE_METADATA
/**
 * \brief transport metadata to output PCM buffer
 * \param sco_data - Pointer to the common SCO rcv operator data
 * \param input_processed - amount of input processed (in words)
 * \param output_generated - amount of output generated (in samples)
 * \param type - connection type (SCO_NB or WBS) only used to determine sample rate
 */
void sco_rcv_transport_metadata(SCO_COMMON_RCV_OP_DATA* sco_data,
                                unsigned input_processed,
                                unsigned output_generated,
                                CONNECTION_TYPE type)
{
    metadata_tag *in_mtag = NULL;
    metadata_tag *out_mtag = NULL;
    unsigned b4idx=0, afteridx=0;
    SCO_COMMON_RCV_METADATA_HIST *last_tag = &sco_data->last_tag;

    /* see if anything has been consummed from input buffer */
    if(buff_has_metadata(sco_data->buffers.ip_buffer) &&
       input_processed != 0)
    {
        /* something has been consumed from input, remove the relevant tag */
        in_mtag = buff_metadata_remove(sco_data->buffers.ip_buffer, input_processed * OCTETS_PER_SAMPLE, &b4idx, &afteridx);
        if(in_mtag != NULL)
        {
            if(IS_TIME_OF_ARRIVAL_TAG(in_mtag) &&
               in_mtag->next == NULL &&
               b4idx == 0)
            {
                /* Just passed the start of a new metadata tag,
                 * tag info (time stamp and sp_adjust) is suitable
                 * for output tag.
                 */
                last_tag->timestamp = in_mtag->timestamp;
                last_tag->sp_adjust = in_mtag->sp_adjust;
                last_tag->nrof_samples = 0;
                last_tag->valid = TRUE;
                SCO_DBG_MSG5("SCO_RCV_TRANSPORT_METADATA, New tag read, valid=1, input=%d, toa=0x%08x, sp_adjust=%d, before=%d, after=%d",
                             input_processed,
                             last_tag->timestamp,
                             last_tag->sp_adjust,
                             b4idx,
                             afteridx);
            }
            else
            {
                /* this could be that a large amount of input was processed
                 * perhaps in search of sync word, or emptying the buffer.  Anyway,
                 * tag info isn't suitable to be passed to the output.
                 */
                last_tag->valid = FALSE;
                SCO_DBG_MSG4("SCO_RCV_TRANSPORT_METADATA, New tag read, valid=0, input=%d, next_tag=%d, before=%d, after=%d",
                             input_processed,
                             (unsigned)(uintptr_t)in_mtag->next,
                             b4idx,
                             afteridx);
            }
        }
        else
        {
            SCO_DBG_MSG4("SCO_RCV_TRANSPORT_METADATA, Null tag read, input=%d, last_valid=%d, before=%d, after=%d",
                         input_processed,
                         last_tag->valid,
                         b4idx,
                         afteridx);
        }
    }

    if((output_generated == 0) ||
       !buff_has_metadata(sco_data->buffers.op_buffer))
    {
        /* No output is generated, or even better, output
         * buffer doesn't support metadata at all. We've
         * done with input tag, delete it now.
         */
        if(in_mtag != NULL)
        {
            SCO_DBG_MSG3("SCO_RCV_TRANSPORT_METADATA, Input tag deleted, toa=0x%08x, before=%d, after=%d",
                         in_mtag->timestamp,
                         b4idx,
                         afteridx);
            buff_metadata_tag_list_delete(in_mtag);
        }
        /* Nothing more do we need to do here */
        return;
    }

    /* one tag for each chunk of produced output,
     * assumes that the input is processed at packet rate
     * which is the case for SCO rcv operators.
     */
    if(NULL != in_mtag)
    {
        /* delete any additional tags that got returned */
        buff_metadata_tag_list_delete(in_mtag->next);
        in_mtag->next = NULL;

        /* transport input tag */
        out_mtag = in_mtag;
    }
    else
    {
        /* create new tag for output */
        out_mtag = buff_metadata_new_tag();
    }


    if(out_mtag != NULL)
    {
        TIME toa; /* time of arrival for this tag */
        TIME current_time = hal_get_time();

        out_mtag->length = output_generated * OCTETS_PER_SAMPLE;
        /* we have valid info to be used for the output tag */
        out_mtag->sp_adjust = last_tag->sp_adjust;

        if(last_tag->valid)
        {
            if(last_tag->nrof_samples == 0)
            {
                /* transport toa from input to output
                 * In good days this should always be the case
                 */
                toa = last_tag->timestamp;
                SCO_DBG_MSG4("SCO_RCV_TRANSPORT_METADATA, Appending output tag, uses input tag info, toa=0x%08x(%dus in the past), sp_adjust=%d, len=%d",
                             toa, time_sub(current_time, toa), out_mtag->sp_adjust, out_mtag->length);
                /* store the the toa and current_time difference */
                last_tag->time_offset = time_sub(current_time, toa);
            }
            else
            {
                /* In two cases this could happen:
                 *  - PLC is generating output without any input received
                 *  - or less likely, input tag is larger than the amount processed by the operator
                 *  Anyway, in this case we calculate the new time stamp from the previous stamp.
                 */
                toa = ttp_get_next_timestamp(last_tag->timestamp,
                                             last_tag->nrof_samples,
                                             type == SCO_NB? 8000:16000,
                                             last_tag->sp_adjust);
                SCO_DBG_MSG4("SCO_RCV_TRANSPORT_METADATA, Appending output tag, uses previous tag info, toa =0x%08x(%dus in the past), sp_adjust=%d, len=%d",
                             toa, time_sub(current_time, toa), out_mtag->sp_adjust, out_mtag->length);

            }
        }
        else /* if(last_tag->valid) */
        {
            /* We don't have a good reference for time stamping
             * of the output samples. This should happen very rare.
             * the reasons that this could happen is that a big chuck
             * of input is trashed, or any chunk right at the beginning,
             * or input isn't ToA tagged at all.
             *
             * ToA is built using info from last valid tag. This
             * will might not be accurate ToA.
             */
            toa = time_sub(current_time, last_tag->time_offset);
            SCO_DBG_MSG4("SCO_RCV_TRANSPORT_METADATA, Appending output tag, no time reference, uses info from last valid tag, toa =0x%08x(%dus in the past), sp_adjust=%d, len=%d",
                         toa, time_sub(current_time, toa), out_mtag->sp_adjust, out_mtag->length);

        }

        if(sco_data->sco_rcv_parameters.generate_timestamp && out_mtag != NULL)
        {
            /* conver ToA to TTP by adding constant offset */
            TIME ts = time_add(toa, sco_data->sco_rcv_parameters.timestamp_latency);
            METADATA_TIMESTAMP_SET(out_mtag, ts, METADATA_TIMESTAMP_LOCAL);
        }
        else
        {
            /* generate ToA Tag */
            METADATA_TIME_OF_ARRIVAL_SET(out_mtag, toa);
        }

        /* next tag can use current tag info, this is useful
         * when running PLC multiple time, consecutively.
         */
        last_tag->nrof_samples = output_generated;
        last_tag->timestamp = toa;
    }
    else /* if(out_mtag != NULL) */
    {
        SCO_DBG_MSG2("SCO_RCV_TRANSPORT_METADATA, Appending Null tag, len=%d, time=0x%08x", output_generated * OCTETS_PER_SAMPLE, hal_get_time());
        last_tag->valid = FALSE;
    }

    /* append generated metadata to the output buffer */
    b4idx = 0;
    afteridx = output_generated * OCTETS_PER_SAMPLE;
    buff_metadata_append(sco_data->buffers.op_buffer, out_mtag, b4idx, afteridx);
    return;
}
#endif

/**
 * \brief Fakes a packet with PLC100.
 *
 * TODO Consider to inline this function.
 * \param sco_data - Pointer to the common SCO rcv operator data
 */
unsigned fake_packet(SCO_COMMON_RCV_OP_DATA* sco_data, unsigned packet_size, CONNECTION_TYPE type)
{
    patch_fn(fake_packet);

#ifdef INSTALL_PLC100
    if(!enough_space_to_run(sco_data, packet_size))
    {
        return TOUCHED_NOTHING;
    }

    SCO_DBG_MSG("Packet faked!");

    sco_data->sco_rcv_parameters.md_pkt_faked++;
    if (sco_data->sco_rcv_parameters.force_plc_off)
    {
        sco_data->sco_rcv_parameters.plc100_struc->bfi = OK;
    }
    else
    {
        sco_data->sco_rcv_parameters.plc100_struc->bfi = NOTHING_RECEIVED;
    }
    sco_data->sco_rcv_parameters.plc100_struc->packet_len = packet_size;

#ifdef SCO_RX_OP_GENERATE_METADATA
    /* keep metadata aligned with the buffer */
    sco_rcv_transport_metadata(sco_data,
                               0,           /* input_processed */
                               packet_size, /* output_generated */
                               type         /* type */);
#else
    UNUSED(type);
#endif
    /*
    * Do packet loss concealment (PLC).
    */
    plc100_process(sco_data->sco_rcv_parameters.plc100_struc);

    return TOUCHED_SOURCE_0;

#else //INSTALL_PLC100
    return TOUCHED_NOTHING;

#endif
}

/**
 * Updates the expected timestamp in SCO for better metadata handling.
 *
 * \param sco_data - Pointer to the common SCO rcv operator data.
 */
void sco_fw_update_expected_timestamp(SCO_COMMON_RCV_OP_DATA* sco_data)
{
    patch_fn(sco_fw_update_expected_timestamp);
    if (sco_data->sco_rcv_parameters.expected_time_stamp == -1)
    {
        return;
    }
    /* TODO remove kick number */
    SCO_DBG_MSG3("Kick nr %d. update_expected_timestamp. expected_time_stamp %d + t_esco %d * 2 = new expected_time_stamp. ",
                                        sco_data->sco_rcv_parameters.md_num_kicks,
                                        sco_data->sco_rcv_parameters.expected_time_stamp,
                                        sco_data->sco_rcv_parameters.t_esco);

   /* T(esco) is in slots (= 2*BT ticks) and timestamp is in BT ticks */
   sco_data->sco_rcv_parameters.expected_time_stamp += sco_data->sco_rcv_parameters.t_esco * 2;
   /* Metadata timestamp field is 16-bits. */
   sco_data->sco_rcv_parameters.expected_time_stamp &= 0xFFFF;
}


/**
 *  This is used by sco (NBS/WBS) after each kick has finished the
 *  processing. If the number of bad kicks goes higher than the limit,
 *  reset the timestamp.
 *
 * @param sco_data - Pointer to the SCO rcv operator data
 * @param exp_pkts_left - How many good packets were missing.
 */
void sco_fw_check_bad_kick_threshold(SCO_COMMON_RCV_OP_DATA* sco_data)
{
    /* Increase the counter of OUT_OF_TIME_PKT_CNT.
     */
    patch_fn(sco_fw_check_bad_kick_threshold);

    sco_data->sco_rcv_parameters.out_of_time_pkt_cnt ++;

    SCO_DBG_MSG1("Warning! Out of time counter incremented to %d.",
            sco_data->sco_rcv_parameters.out_of_time_pkt_cnt);


    if (sco_data->sco_rcv_parameters.out_of_time_pkt_cnt > OUT_OF_TIME_PKT_LIMIT)
    {
        /* If the counter reaches the limit (OUT_OF_TIME_PKT_LIMIT) then something
         * has gone wrong with the timestamp checkings. Reset it and move along to
         * "packets_ok".
         */
        SCO_DBG_MSG("Error! Out of time reset.");

        sco_data->sco_rcv_parameters.expected_time_stamp = -1;
        sco_data->sco_rcv_parameters.md_num_kicks = 0;
        sco_data->sco_rcv_parameters.out_of_time_pkt_cnt = 0;
        sco_data->sco_rcv_parameters.md_out_of_time_reset++;
    }

}

/**
 *  Returns the SCO rcv output words.
 *
 * @param sco_data - Pointer to the SCO rcv operator data
 */
unsigned sco_rcv_get_output_size_words(SCO_COMMON_RCV_OP_DATA* sco_data)
{
    unsigned packet_size;

#ifdef CVSD_CODEC_SOFTWARE
    packet_size = sco_data->sco_rcv_parameters.sco_pkt_size; /*convert to bytes */
    if (packet_size == 0 )
    {
        /*
         * SCO_PKT_SIZE field is not initialised, use block size for the moment.
         */
        packet_size = SCO_RCV_MIN_BLOCK_SIZE*2;
    }
#else
	packet_size = sco_data->sco_rcv_parameters.sco_pkt_size / 2; /*convert to words */
	if (packet_size == 0)
	{
		/*
		* SCO_PKT_SIZE field is not initialised, use block size for the moment.
		*/
		packet_size = SCO_RCV_MIN_BLOCK_SIZE;
	}
#endif
    return packet_size;
}


/**
 *  Returns the SCO rcv packet size in words.
 *
 * @param sco_data - Pointer to the SCO rcv operator data
 */
unsigned sco_rcv_get_packet_size(SCO_COMMON_RCV_OP_DATA* sco_data)
{
    unsigned packet_size;
    packet_size = sco_data->sco_rcv_parameters.sco_pkt_size / 2; /*convert to words */
    if (packet_size == 0 )
    {
        /*
         * SCO_PKT_SIZE field is not initialised, use block size for the moment.
         */
        packet_size = SCO_RCV_DEFAULT_BLOCK_SIZE;
    }
    return packet_size;
}

/**
 * sco_rcv_flush_input_buffer
 * \brief clearing sco input buffer with metadata
 * \param ip_buffer input buffer
 */
void sco_rcv_flush_input_buffer(SCO_COMMON_RCV_OP_DATA* sco_data)
{
    tCbuffer *ip_buffer = sco_data->buffers.ip_buffer;

    SCO_DBG_MSG1("sco rx, Flushing buffer with metadata, buf=0x%x", (unsigned)(uintptr_t)ip_buffer);
#ifdef SCO_RX_OP_GENERATE_METADATA
    /* interrupts are blocked so SCO endpoint won't try
     * to write a new tag during buffer flush. */
    LOCK_INTERRUPTS;
    sco_data->last_tag.valid = FALSE;
    cbuffer_empty_buffer_and_metadata(ip_buffer);
    /* Make sure next time metadata is generated for the right offset */
    stream_sco_reset_sco_metadata_buffer_offset(sco_data->sco_source_ep);
    UNLOCK_INTERRUPTS;
#else
    cbuffer_empty_buffer(ip_buffer);
#endif
}

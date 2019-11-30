/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  sco_rcv_data_processing_c.c
 * \ingroup  operators
 *
 *  SCO receive operator
 *
 */
/****************************************************************************
Include Files
*/
#include "sco_nb_private.h"

#include "patch/patch.h"
#ifdef CVSD_CODEC_SOFTWARE
#include "cvsd.h"
#endif

/****************************************************************************
Private Constant Definitions
*/
#define MAX_SCO_NB_PACKETS_TO_PROCESS 2

/****************************************************************************
Private Type Definitions
*/

/****************************************************************************
Private Function Declarations
*/
static unsigned copy_packet(SCO_COMMON_RCV_OP_DATA* sco_data, stream_sco_metadata* sco_metadata);
/*****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Public Function Definitions
*/

/**
 * Processing function for the sco_nb_rcv operators.
 * \param op_data
 * \param touched
 */
unsigned sco_rcv_processing(OPERATOR_DATA *op_data)
{
    SCO_COMMON_RCV_OP_DATA* sco_data = (SCO_COMMON_RCV_OP_DATA*)(op_data->extra_op_data);
    stream_sco_metadata sco_metadata;
    unsigned packets_to_process = MAX_SCO_NB_PACKETS_TO_PROCESS;
    unsigned ret_val = TOUCHED_NOTHING;
    stream_sco_metadata_status status = SCO_METADATA_NOT_READ;

    /* increment the kick count */
    sco_data->sco_rcv_parameters.md_num_kicks++;

    /* Log the SCO state */
    print_SCO_state(sco_data);


    /*
     * If expected time stamp is not initialised, decoder is yet to produce data
     * since it was started. Similarly, packet size may not yet be set. So try to make
     * sense of whatever data we see in input - for each kick, we should have some data
     * to read and hopefully find at least a metadata header to update the needed packet
     * size and timestamp information. If somehow not even a header worth of data arrived,
     * then we have no choice other than flag this as a bad kick and return later with
     * renewed hope...
     */
    if (!enough_data_to_run(sco_data, METADATA_HEADER_SIZE))
    {
        /* This should never happen. In case it does, just hope that for the next kick
         * we will have enough data on the output.
         */
        SCO_DBG_MSG("Warning! Not enough data to run for the first time");

        /* There is no valid packet in the input buffer. */
        sco_data->sco_rcv_parameters.num_bad_kicks ++;

        /* Before exiting increment the bad kick counter. */
        sco_fw_check_bad_kick_threshold(sco_data);

        /* No  valid packet on the input buffer which mean there is no reason to
         * try and check it again. Increment the expected timestamp.*/
        sco_fw_update_expected_timestamp(sco_data);

        ret_val |= fake_packet(sco_data, sco_rcv_get_output_size_words(sco_data), SCO_NB);

        return ret_val;
    }
    if (!enough_space_to_run(sco_data, sco_rcv_get_output_size_words(sco_data)))
    {
        /* This should never happen. In case it does, just hope that for the next kick
         * we will have enough data on the output.
         */
        SCO_DBG_MSG("Warning! Not enough space to run for the first time");

        /* There is no valid packet in the input buffer. */
        sco_data->sco_rcv_parameters.num_bad_kicks ++;

        /* Before exiting increment the bad kick counter. */
        sco_fw_check_bad_kick_threshold(sco_data);

        return ret_val;
    }

    /* Read the metadata using SCO NB specific transformation for the header. */
    status = read_packet_metadata(sco_data, &sco_metadata, SCO_NB);
    print_sco_metadata(&sco_metadata);

    while(packets_to_process > 0)
    {
        /* Check if there is enough data in the input buffer to read the packet payload.*/
        if ((status == SCO_METADATA_INVALID) ||
            !enough_data_to_run(sco_data, CONVERT_OCTETS_TO_SAMPLES(sco_metadata.packet_length)) )
        {
            SCO_DBG_MSG("Warning! invalid or not enough data ");
            
            /*
             * sync lost or something went wrong, flush the input buffer
             */
            sco_rcv_flush_input_buffer(sco_data);

            /* There is no valid packet in the input buffer. */
            sco_data->sco_rcv_parameters.num_bad_kicks ++;

            /* Fake a packet */
            ret_val |= fake_packet(sco_data, sco_rcv_get_output_size_words(sco_data), SCO_NB);

            /* No  valid packet on the input buffer which mean there is no reason to
             * try and check it again. Increment the expected timestamp.*/
            sco_fw_update_expected_timestamp(sco_data);

            /* Before exiting update the bad kick counter. */
            sco_fw_check_bad_kick_threshold(sco_data);

            return ret_val;
        }

        /* Check if the packet size changed. */
        if (sco_metadata.packet_length != sco_data->sco_rcv_parameters.sco_pkt_size)
        {
            /* update expected packet */
            sco_data->sco_rcv_parameters.sco_pkt_size = sco_metadata.packet_length;
            sco_data->sco_rcv_parameters.md_pkt_size_changed++;
            sco_data->sco_rcv_parameters.exp_pkts = 0;
        }

        status = analyse_sco_metadata(sco_data, &sco_metadata);
        switch(status)
        {
            case SCO_METADATA_LATE:
            {
                /* Progress the input buffer read address past this invalid packet */
                sco_data->sco_rcv_parameters.md_late_pkts++;

                /* discard packet*/
                discard_packet(sco_data, &sco_metadata);

                if (!enough_data_to_run(sco_data, METADATA_HEADER_SIZE) ||
                    !enough_space_to_run(sco_data, sco_rcv_get_output_size_words(sco_data)))
                {
                    /* This should never happen. In case it does, just hope that for the next kick
                     * we will have enough data on the output.
                     */
                    SCO_DBG_MSG("Warning! Not enough data to run");

                    /* Before exiting increment the bad kick counter. */
                    sco_fw_check_bad_kick_threshold(sco_data);
                    return ret_val;
                }

                status = read_packet_metadata(sco_data, &sco_metadata, SCO_NB);
                break;
            }
            case SCO_METADATA_ON_TIME:
            {
                /* Decrement the packets to process. */
                packets_to_process--;

                /* Reset the counter if we get a good packet. This means that everything
                 * is in sync and we shouldn't take any actions if we only get
                 * late/early packets once in a while.
                 */
                sco_data->sco_rcv_parameters.out_of_time_pkt_cnt = 0;

                /* Copy the valid packet to the output buffer. Run plc (if enabled) on the output. */
                ret_val |= copy_packet(sco_data, &sco_metadata);

                sco_fw_update_expected_timestamp(sco_data);


                if(packets_to_process > 0)
                {
                    if (!enough_data_to_run(sco_data, METADATA_HEADER_SIZE) ||
                        !enough_space_to_run(sco_data, sco_rcv_get_output_size_words(sco_data)))
                    {
                        /* No space or data to run again, this is normal */
                        return ret_val;
                    }

                    status = read_packet_metadata(sco_data, &sco_metadata, SCO_NB);
                }
                else
                {
                    return ret_val;
                }
                break;
            }
            case SCO_METADATA_EARLY:
            {
                /* Increment early packet count */
                sco_data->sco_rcv_parameters.md_early_pkts++;

                /* Fake a packet. */
                ret_val |= fake_packet(sco_data, sco_rcv_get_output_size_words(sco_data), SCO_NB);

                /* Increment the timestamp with one. */
                sco_fw_update_expected_timestamp(sco_data);

                /* Increment the bad kick threshold and check if wee need a SCO reset. */
                sco_fw_check_bad_kick_threshold(sco_data);

                if (!enough_space_to_run(sco_data, sco_rcv_get_packet_size(sco_data)))
                {
                    /* discard packet, this isn't usable any more */
                    discard_packet(sco_data, &sco_metadata);

                    /* There is no valid packet in the input buffer. */
                    sco_data->sco_rcv_parameters.num_bad_kicks ++;

                    /* update expected time so next kick we are looking for newer packet */
                    sco_fw_update_expected_timestamp(sco_data);

                    /* There is not enough data to run again. Exit. */
                    SCO_DBG_MSG("Warning! Not enough space to run again - packet discarded");
                    return ret_val;
                }
                break;
            }
            default:
            {
                /* This shouldn't happen. This means that analyse_packet doesn't cover
                 * all the possible cases. */
                fault_diatribe(FAULT_AUDIO_INVALID_SCO_METADATA_STATE, status);
                break;
            }

        }/*switch */

    }/* while exp_packets */

    return ret_val;

}


/****************************************************************************
Private Function Definitions
*/
/**
 * \brief Copies a valid packet to the output buffer. Runs plc100 on the output.
 *
 * \param sco_data - Pointer to the common SCO rcv operator data
 * \param current_packet - current packet containing the metadata information.
 */
static unsigned copy_packet(SCO_COMMON_RCV_OP_DATA* sco_data, stream_sco_metadata* sco_metadata)
{
    unsigned amount_copied, output_words;
#ifdef INSTALL_PLC100
    int *op_buffer_write_address;
    unsigned out_space;
    bool space_limited = FALSE;
#endif /* INSTALL_PLC100 */

    /* What the metadata record says the payload length is (words).
     * (After read_packet_metadata() has sanitised it for buffer
     * wraps.) */
#ifdef CVSD_CODEC_SOFTWARE 
	output_words = sco_rcv_get_output_size_words(sco_data); 
#else
	output_words = get_packet_payload_length(sco_metadata);
#endif
		

    sco_data->sco_rcv_parameters.num_good_pkts_per_kick++;
    sco_data->sco_rcv_parameters.frame_count++;


#ifdef INSTALL_PLC100
    if (sco_data->sco_rcv_parameters.force_plc_off)
    {
        sco_data->sco_rcv_parameters.plc100_struc->bfi = OK;
    }
    else
    {
        sco_data->sco_rcv_parameters.plc100_struc->bfi = sco_metadata->status;
    }
	sco_data->sco_rcv_parameters.plc100_struc->packet_len = output_words;

    /*
     * Because plc100 updates the write pointer we need to reset the original write
     * pointer after the cbops copy.
     */
    op_buffer_write_address = sco_data->buffers.op_buffer->write_ptr;

    /* Check whether all the data will actually fit in the output
     * If it won't, don't run PLC because that would wrap the buffer 
     */
    out_space = cbuffer_calc_amount_space_in_words(sco_data->buffers.op_buffer);
    if (output_words > out_space)
    {
        L2_DBG_MSG2("copy_packet output space = %u payload = %u", out_space, output_words);
        space_limited = TRUE;
    }

#endif /* INSTALL_PLC100 */

#ifdef CVSD_CODEC_SOFTWARE 
	cvsd_receive_asm(&sco_data->sco_rcv_parameters.cvsd_struct, sco_data->buffers.ip_buffer, sco_data->buffers.op_buffer, sco_data->sco_rcv_parameters.ptScratch, output_words);
	amount_copied = output_words;
#else
	/* copy the packet to the output */
	amount_copied = cbuffer_copy(sco_data->buffers.op_buffer, sco_data->buffers.ip_buffer, output_words);
#endif //CVSD_CODEC_SOFTWARE


#ifdef INSTALL_PLC100
    /*
     * Reset the write pointer for the PLC library.
     */
    if (!space_limited)
    {
        cbuffer_set_write_address(sco_data->buffers.op_buffer, (unsigned int*)op_buffer_write_address);
    }
#endif

#ifdef SCO_RX_OP_GENERATE_METADATA
    /* both input and output buffers meta data needs updating,
     * this is done before actual copying. */
    sco_rcv_transport_metadata(sco_data,
                               amount_copied, /* input_processed */
#ifdef INSTALL_PLC100
                               (space_limited?amount_copied:output_words), /* output_generated (PLC) */
#else
                               amount_copied, /* output_generated (no PLC) */
#endif
                               SCO_NB);
#endif



#ifdef INSTALL_PLC100
    /*
    * Do packet loss concealment (PLC).
    */
    if (!space_limited)
    {
        plc100_process(sco_data->sco_rcv_parameters.plc100_struc);
    }

#endif /* INSTALL_PLC100 */

    SCO_DBG_MSG2("After copy!  input buffer data |%4d|, output buffer space |%4d|.",
                                cbuffer_calc_amount_data_in_words(sco_data->buffers.ip_buffer),
                                cbuffer_calc_amount_space_in_words(sco_data->buffers.op_buffer));

	if (amount_copied < output_words)
    {
        /* This really shouldn't happen. read_packet_metadata() checked
         * and dealt with inconsistencies between the metadata record
         * length and the buffer write offset, so those shouldn't show
         * up here. */
        SCO_DBG_MSG3("Kick nr %d. OOPS! amount_copied %d < get_packet_payload_length(current_packet) %d.",
                                            sco_data->sco_rcv_parameters.md_num_kicks,
                                            amount_copied,
											output_words);
#if 0
        /*
         * FIXME: this condition can still occur if we get a
         * NEVER_SCHEDULED giant metadata record. There's some confusion
         * which leads to us trying to fake many packets' worth of
         * output in one go; of course the output buffer runs out of
         * space, and so we hit this.
         * We'll avoid emitting the fault until we've got to the bottom
         * of that.
         */
        fault_diatribe(FAULT_AUDIO_INVALID_SCO_NB_COPY_PACKET_FAILED, amount_copied);
#endif
    }

    /* touch the output */
    return TOUCHED_SOURCE_0;
}

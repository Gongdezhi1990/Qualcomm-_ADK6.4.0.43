/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  wbs_dec_data_processing_c.c
 * \ingroup  operators
 *
 *  WBS decode operator
 *
 */
/****************************************************************************
Include Files
*/
#include "wbs_private.h"

#include "patch/patch.h"
#include "pl_assert.h"

/****************************************************************************
Private Constant Definitions
*/
#define INPUT_BLOCK_SIZE                           (30)
#define OUTPUT_BLOCK_SIZE                         (120)
#define OUTPUT_TWO_BLOCK_SIZE                     (240)

/****************************************************************************
Private Type Definitions
*/

/****************************************************************************
Private Function Declarations
*/
static unsigned decode_packet(OPERATOR_DATA *op_data, stream_sco_metadata* sco_metadata);
static void wbs_dec_update_expected_pkts(SCO_COMMON_RCV_OP_DATA* sco_data);
/*****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Public Function Definitions
*/

/**
 * Processing function for the wbs_dec operators.
 * \param op_data
 * \param touched
 */
unsigned wbs_dec_processing(OPERATOR_DATA *op_data)
{
    WBS_DEC_OP_DATA* wbs_data = (WBS_DEC_OP_DATA*)(op_data->extra_op_data);
    SCO_COMMON_RCV_OP_DATA* sco_data = &(wbs_data->sco_rcv_op_data);

    stream_sco_metadata sco_metadata;
    unsigned ret_val = TOUCHED_NOTHING;
    stream_sco_metadata_status status = SCO_METADATA_NOT_READ;
#ifdef SCO_DEBUG
    /* increment the kick count for wbs. This might be a duplicated in sco_rcv_parameters.md_num_kicks */
    wbs_data->wbs_dec_num_kicks++;
#endif
    /* Init run specific variables for WBS */
    wbs_data->good_pkts_per_kick = 0;
    wbs_data->wbs_dec_output_samples = 0;

    /* increment the kick count */
    sco_data->sco_rcv_parameters.md_num_kicks++;

    /*
     * calculate the expected packets - if packet size was not yet initialised, this will
     * always return 1
     */
    wbs_dec_update_expected_pkts(sco_data);

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
        /* Fake a packet */
        if (enough_space_to_run(sco_data, OUTPUT_BLOCK_SIZE))
        {
            sco_data->sco_rcv_parameters.exp_pkts--;
            if (sco_data->sco_rcv_parameters.exp_pkts==0)
            {
                /* No packet on the input buffer and we want to
                 * generate a fake packet, so we need to make sure
                 * the late packet will not be used later.
                 */
                sco_fw_update_expected_timestamp(sco_data);

                ret_val |= fake_packet(sco_data, OUTPUT_BLOCK_SIZE, WBS);                
            }
        }


        return ret_val;
    }


    if (!enough_space_to_run(sco_data, OUTPUT_BLOCK_SIZE))
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

    /* Read the metadata using WBS specific tranformations for the header. */
    status = read_packet_metadata(sco_data, &sco_metadata, WBS);

    while(sco_data->sco_rcv_parameters.exp_pkts > 0)
    {
        patch_fn_shared(wbs_decode_data_processing);

#ifdef SCO_DEBUG
        /* Increment number of packet searched for WBS. */
        wbs_data->wbs_dec_num_pkts_searched ++;
#endif
        /* Check if there is enough data in the input buffer to read the packet payload.*/
        if ((status == SCO_METADATA_INVALID) ||
            !enough_data_to_run(sco_data, CONVERT_OCTETS_TO_SAMPLES(sco_metadata.packet_length)) )
        {
            /*
             * sync lost flush the input buffer
             */
            sco_rcv_flush_input_buffer(sco_data);

            /* There is no valid packet in the input buffer. */
            sco_data->sco_rcv_parameters.num_bad_kicks ++;

            /* Fake a packet */
            sco_data->sco_rcv_parameters.exp_pkts--;
            if (sco_data->sco_rcv_parameters.exp_pkts==0)
            {
                ret_val |= fake_packet(sco_data, OUTPUT_BLOCK_SIZE, WBS);
            }

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
            /* sco_data->sco_rcv_parameters.exp_pkts only gets update if is  set to 0. */
            sco_data->sco_rcv_parameters.exp_pkts = 0;
            wbs_dec_update_expected_pkts(sco_data);

        }
#ifdef SCO_DEBUG
        wbs_data->wbs_dec_metadata_found ++;
#endif
        print_sco_metadata(&sco_metadata);

        status = analyse_sco_metadata(sco_data, &sco_metadata);

        patch_fn_shared(wbs_decode_data_processing);

        switch(status)
        {
            case SCO_METADATA_LATE:
            {
                /* Progress the input buffer read address past this invalid packet */
                sco_data->sco_rcv_parameters.md_late_pkts++;

                /* discard packet*/
                discard_packet(sco_data, &sco_metadata);

                if (!enough_data_to_run(sco_data, METADATA_HEADER_SIZE) ||
                    !enough_space_to_run(sco_data, OUTPUT_BLOCK_SIZE))
                {
                    /* This should never happen. In case it does, just hope that for the next kick
                     * we will have enough data on the output.
                     */
                    SCO_DBG_MSG("Warning! Not enough data to run");

                    /* Before exiting increment the bad kick counter. */
                    sco_fw_check_bad_kick_threshold(sco_data);
                    return ret_val;
                }

                status = read_packet_metadata(sco_data, &sco_metadata, WBS);
                break;
            }
            case SCO_METADATA_ON_TIME:
            {
                /* Decrement the expected packets. */
                sco_data->sco_rcv_parameters.exp_pkts--;

                /* Reset the counter if we get a good packet. This means that everything
                 * is in sync and we shouldn't take any actions if we only get
                 * late/early packets once in a while.
                 */
                sco_data->sco_rcv_parameters.out_of_time_pkt_cnt = 0;

                /* Decode the valid packet to the output buffer. Run plc (if enabled) on the output. */
                ret_val = decode_packet(op_data, &sco_metadata);

                /* Update the expected timestamp */
                sco_fw_update_expected_timestamp(sco_data);


                if(sco_data->sco_rcv_parameters.exp_pkts > 0)
                {
                    if (!enough_data_to_run(sco_data, METADATA_HEADER_SIZE) ||
                        !enough_space_to_run(sco_data, OUTPUT_BLOCK_SIZE))
                    {
                        /* There is not enough data to run again. Exit. */
                        SCO_DBG_MSG("Warning! Not enough data to run again");
                        return ret_val;
                    }

                    status = read_packet_metadata(sco_data, &sco_metadata, WBS);
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
                ret_val |= fake_packet(sco_data, OUTPUT_BLOCK_SIZE, WBS);

                /* Increment the timestamp with one. */
                sco_fw_update_expected_timestamp(sco_data);

                /* Increment the bad kick threshold and check if wee need a SCO reset. */
                sco_fw_check_bad_kick_threshold(sco_data);

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
 * \brief Decode a valid packet to the output buffer. Runs plc100 on the output.
 *
 * \param op_data - Pointer to the operator data
 * \param current_packet - current packet containing the metadata information.
 */
static unsigned decode_packet(OPERATOR_DATA *op_data, stream_sco_metadata* sco_metadata)
{
    WBS_DEC_OP_DATA* wbs_data = (WBS_DEC_OP_DATA*)(op_data->extra_op_data);
    SCO_COMMON_RCV_OP_DATA* sco_data = &(wbs_data->sco_rcv_op_data);
    unsigned wbs_packet_length = 0;
    unsigned amount_advanced = 0;

    /* validate the current packet. */
    int validate = sco_decoder_wbs_validate(op_data, sco_metadata->packet_length, &wbs_packet_length, &amount_advanced);

    /* before calling this function we only know that we have one full packet
     * It will be a serious bug if the validate function tries to go beyond one full packet
     */
    PL_ASSERT(amount_advanced <= CONVERT_OCTETS_TO_SAMPLES(sco_metadata->packet_length));

    SCO_DBG_MSG1("validate retval = %4d", validate);

    if(validate == 120)
    {
#ifdef SCO_DEBUG
        wbs_data->wbs_dec_output_samples += validate;
        wbs_data->wbs_dec_validate_ret_good ++;
#endif
    }
    else if(validate == 240)
    {
#ifdef SCO_DEBUG
        wbs_data->wbs_dec_output_samples += validate;
        wbs_data->wbs_dec_validate_ret_good ++;
#endif
    }
    else
    {
        if (!((validate == 0) || (validate == 1) || (validate == 2)))
        {
            fault_diatribe(FAULT_AUDIO_INVALID_WBS_VALIDATE_RESULT, validate);
        }
    }

    int  result = sco_decoder_wbs_process(op_data, sco_metadata, validate, wbs_packet_length);

    SCO_DBG_MSG1("sco_decoder_wbs_process result = %4d",
                                result);
    /* sco_decoder_wbs_process does not increment the read pointer of the input buffer.
     * The sco packet length was checked in read_packet_metadata. */
    {
        /* advance the read pointer by one packet length, the input buffer has already
           been checked that it has one full packet, however part of that might have
           been consumed by the validate function
         */
        unsigned amount_to_advance = CONVERT_OCTETS_TO_SAMPLES(sco_metadata->packet_length);
#ifdef SCO_RX_OP_GENERATE_METADATA
        if(buff_has_metadata(sco_data->buffers.ip_buffer))
        {
            /* remove associated metadata before updating main buffer */
            sco_rcv_transport_metadata(sco_data, amount_to_advance, 0, WBS);
        }
#endif
        /* part of packet might already be advanced by validate function */
        amount_to_advance -= amount_advanced;
        cbuffer_advance_read_ptr(sco_data->buffers.ip_buffer, amount_to_advance);
    }

    if (result == WBS_DEC_PROCESS_NO_OUTPUT)
    {
        return TOUCHED_NOTHING;
    }
    else
    {
#ifdef INSTALL_PLC100
        unsigned bfi;
        if (sco_data->sco_rcv_parameters.force_plc_off)
        {
            bfi = OK;
        }
        else if (result == WBS_DEC_PROCESS_FAKE_FRAME)
        {
            bfi = NOTHING_RECEIVED;
        }
        else /* ok, crc error, or nothing received */
        {
            bfi = result;
        }
        sco_data->sco_rcv_parameters.plc100_struc->bfi = bfi;

        /* Check if the validate returned a valid block size, if not use the default one
         * block. */
        if ((validate == OUTPUT_BLOCK_SIZE) || (validate == OUTPUT_TWO_BLOCK_SIZE))
        {
            sco_data->sco_rcv_parameters.plc100_struc->packet_len = validate;
        }
        else
        {
            sco_data->sco_rcv_parameters.plc100_struc->packet_len = OUTPUT_BLOCK_SIZE;
            /* This should never happen if the result is zero. */
            if (result == WBS_DEC_PROCESS_OK)
            {
                fault_diatribe(FAULT_AUDIO_INVALID_WBS_VALIDATE_DECODE_RESULT, validate);
            }
        }

        /* Do packet loss concealment (PLC). */
#ifdef SCO_RX_OP_GENERATE_METADATA
        /* update metadata first */
        sco_rcv_transport_metadata(sco_data, 0, sco_data->sco_rcv_parameters.plc100_struc->packet_len, WBS);
#endif
        plc100_process(sco_data->sco_rcv_parameters.plc100_struc);

#else /* No plc */
        /* if result == OK: */
        /* sco_decoder_wbs_process does not increment the write pointer of the output buffer.
         * That is the plc100 job but now is missing so we should do it now. */
        /* if result != OK: */
        /* No output from the decode and no PLC. Just increment the write pointer of the
         * output buffer.
         * TODO maybe inserting silences is better. */
#ifdef SCO_RX_OP_GENERATE_METADATA
        /* update metadata first */
        sco_rcv_transport_metadata(sco_data, 0, validate, WBS);
#endif
        cbuffer_advance_write_ptr(sco_data->buffers.op_buffer, validate);
#endif /* INSTALL_PLC100 */

        /* We produced an output so we don't expect more packets. */
        sco_data->sco_rcv_parameters.exp_pkts = 0;
        return TOUCHED_SOURCE_0;
    }
}

/**
 *  Calculates the expected packets needed to decode one WBS frame.
 *
 * \param sco_data - Pointer to the SCO rcv operator data
 * \return Expected number of packets needed in worst case to decode one WBS frame.
 */
static void wbs_dec_update_expected_pkts(SCO_COMMON_RCV_OP_DATA* sco_data)
{
    patch_fn_shared(wbs_decode_data_processing);

    /* Only update exp_pkts if it is 0. */
    if (sco_data->sco_rcv_parameters.exp_pkts == 0)
    {
        unsigned sco_packet_size_words = sco_rcv_get_packet_size(sco_data);

        sco_data->sco_rcv_parameters.exp_pkts =
                (INPUT_BLOCK_SIZE + (sco_packet_size_words - 1)) / sco_packet_size_words;
    }
}


/**
 *  Reset the internals of the decoder
 * \param op_data - operator data pointer
 */
void wbs_dec_reset_sbc_data(OPERATOR_DATA* op_data)
{
    WBS_DEC_OP_DATA* wbs_data = (WBS_DEC_OP_DATA*) op_data->extra_op_data;

    /* initialise the wbs decoder */
    sco_decoder_wbs_initialize(wbs_data->codec_data);

    /* reset the sbc decoder */
    sbcdec_reset_decoder(wbs_data->codec_data);

    /* now init the tables */
    sbcdec_init_tables(wbs_data->codec_data);

}





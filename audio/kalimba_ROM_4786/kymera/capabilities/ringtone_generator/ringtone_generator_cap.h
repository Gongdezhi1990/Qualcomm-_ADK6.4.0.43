/**
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
 * \defgroup ringtone_generator
 * \file  ringtone_generator_cap.h
 * \ingroup capabilities
 *
 * Ringtone generator operator capability header file. <br>
 *
 */


 #ifndef RINGTONE_GENERATOR_CAP_H_
 #define RINGTONE_GENERATOR_CAP_H_

 #include "capabilities.h"
 #include "buffer.h"
 #include "stream/stream_audio_data_format.h"
 #include "platform/pl_intrinsics.h"
 #include "platform/pl_fractional.h"
 #include "platform/pl_assert.h"
 #include "ringtone_if.h"
 #include "common_conversions.h"
 #include "tone_synth.h"
 
 #include "patch/patch.h"


/** This ringtone generator doesn't generate data periodically or based on any frame size */
 #define RINGTONE_GENERATOR_BLOCK_SIZE                  1

 #define CHANNEL_MASK(ID) (1<<(ID))

 /** Allow to connect up to 8 output channels */
 #define MAX_OUTPUT_CHANS 8
 /** Maximum length of tone parameters in words in one operator message */
 #define MAX_TONE_LEN 60

 #define RINGTONE_OUTPUT_LEN  32   // in words


 enum {BASE_SAMPLE_RATE = 8000};


typedef struct RINGTONE_BUFFER
{
    struct RINGTONE_BUFFER *next_tone;    /**< point to next tone in the list */
    unsigned *tone_param_end;           /** Indicate the tone message content end,
                                         * just in case no RINGTONE_END is sent from the application */
    unsigned *index;
    unsigned *cur_tone_buf[];
}RINGTONE_BUFFER;

typedef struct RINGTONE_LIST
{
    RINGTONE_BUFFER *head;
    RINGTONE_BUFFER *tail;
}RINGTONE_LIST;


typedef struct RINGTONE_GENERATOR_OP_DATA
{
    unsigned tone_data_block_size;
    bool tone_end;
    bool unsolicited_sent;
    unsigned active_chans;             /**< Bitfield of connected channels in the output channel array*/
    tCbuffer *connecting_buf;          /**< Buffer that was supplied by buffer_details and should be provided by the
                                        * connect call. This is saved in case connect fails before
                                        * ringtone_generator_connect()is called and we need to release this memory at a later time. */

    tCbuffer *op_chnl_cbuffer[MAX_OUTPUT_CHANS];
    RINGTONE_LIST tone_list;          /**< a list of tone parameters in sequences received from VM */
    TONE_SYNTH_DATA info;
    tCbuffer *out_cbuffer;
    unsigned start_seq_stage;

#ifdef INSTALL_METADATA
    /** The output buffer with metadata to transport to */
    tCbuffer *metadata_op_buffer;
#endif /* INSTALL_METADATA */

} RINGTONE_GENERATOR_OP_DATA;



/**
* \brief    Main API function of generating the ringtone audio
*
* \param    op_data  The ringtone generator specific operator data
* \param    len      The length of how many samples to generate each time
*/
extern void ringtone_play(RINGTONE_GENERATOR_OP_DATA *op_data, unsigned len);


/**
 * \brief   Callback to output one sample
 * \param   cb_data Caller context
 * \param   value   Sample value
 */
extern void ringtone_write(void* cb_data, int sample);


/**
* \brief    Initialise a ringtone sequence
*
* \param    op_data  The ringtone generator specific operator data
*/
extern void ringtone_init(RINGTONE_GENERATOR_OP_DATA *op_data);


/**
* \brief    Advance a sequence
*           All tone generation is based on 8000Hz, so for higher
*           frequencies, we scale relative to this. For example, to
*           generate samples at 16000Hz we increase the duration of
*           each tone by two, and half the size of the steps we take
*           through the tone.
*
* \param    op_data  The ringtone generator specific operator data
*
* \return   FALSE indicates reach the end of ringtone sequence, otherwise, return TRUE
*/
extern bool ringtone_sequence_advance(RINGTONE_GENERATOR_OP_DATA *op_data);


/****************************************************************************
Private Function Definitions
*/
extern bool ringtone_generator_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool ringtone_generator_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool ringtone_generator_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool ringtone_generator_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool ringtone_generator_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool ringtone_generator_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool ringtone_generator_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool ringtone_generator_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool ringtone_generator_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern void ringtone_generator_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched);

extern void ringtone_update_read_ptr(RINGTONE_GENERATOR_OP_DATA *op_data);
extern void ringtone_update_write_ptr(RINGTONE_GENERATOR_OP_DATA *rgop_data);

/******************************************************************************
 Operator message handlers definitions
 */
extern bool ringtone_generator_opmsg_config(OPERATOR_DATA *op_data, void *message_data, unsigned int *response_length, OP_OPMSG_RSP_PAYLOAD **response_data);
extern bool ringtone_generator_opmsg_tones(OPERATOR_DATA *op_data, void *message_data, unsigned int *response_length, OP_OPMSG_RSP_PAYLOAD **response_data);

#endif /* RINGTONE_GENERATOR_CAP_H_ */

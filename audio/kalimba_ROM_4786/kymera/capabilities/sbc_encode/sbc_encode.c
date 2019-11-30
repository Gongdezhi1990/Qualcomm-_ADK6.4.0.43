/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  sbc_encode.c
 * \ingroup  capabilities
 *
 *  SBC decode operator
 *
 */

#include "encoder/common_encode.h"
#include "capabilities.h"
#include "common_conversions.h"
#include "mem_utils/scratch_memory.h"
#include "mem_utils/memory_table.h"
#include "codec_c.h"
#include "sbc_c.h"

#include "op_msg_helpers.h"

// add autogen header
#include "sbc_encode_gen_c.h"
#include "sbc_encode.h"

/****************************************************************************
Private Constant Definitions
*/
/** The scratch buffer size required if performing ratematching inside the capability
 * This has to be maximum SBC frame size (128) + 1 sample as this buffer is viewed as a cbuffer which
 * always leaves one free word when the buffer is full.*/
#define SBC_SCRATCH_BUFFER_SIZE ((A2DP_DECODE_OUTPUT_BUFFER_SIZE/2 * sizeof(unsigned)) + sizeof(unsigned))

/* Encoder delay figures (samples)
 * Values are derived from a combination of measurements and reading the A2DP spec
 */
#define SBC_DELAY_8_SUBBANDS 73
#define SBC_DELAY_4_SUBBANDS 37

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define SBC_ENCODER_CAP_ID CAP_ID_DOWNLOAD_SBC_ENCODER
#else
#define SBC_ENCODER_CAP_ID CAP_ID_SBC_ENCODER
#endif

/****************************************************************************
Private Type Definitions
*/
typedef struct
{
    /** A2DP_DECODER_PARAMS must be the first parameters always */
    ENCODER_PARAMS    encoder_data;

    /** The sbc_codec specific data */
    sbc_codec codec_data;
} SBC_ENC_OP_DATA;

/****************************************************************************
Private Constant Declarations
*/
/** The maximum number of samples in a single SBC encoded frame */
#define MAX_SBC_BLOCK_SIZE              128

/** The length of the SBC Decoder capability malloc table */
#define SBC_ENC_MALLOC_TABLE_LENGTH 3

/** The sbc_encoder capability function handler table */
const handler_lookup_struct sbc_encode_handler_table =
{
    sbc_encode_create,          /* OPCMD_CREATE */
    encoder_destroy,         /* OPCMD_DESTROY */
    encoder_start,              /* OPCMD_START */
    base_op_stop,               /* OPCMD_STOP */
    encoder_reset,           /* OPCMD_RESET */
    encoder_connect,            /* OPCMD_CONNECT */
    encoder_disconnect,         /* OPCMD_DISCONNECT */
    encoder_buffer_details,     /* OPCMD_BUFFER_DETAILS */
    encoder_get_data_format,    /* OPCMD_DATA_FORMAT */
    encoder_get_sched_info      /* OPCMD_GET_SCHED_INFO */
};

const opmsg_handler_lookup_table_entry sbc_encode_opmsg_handler_table[] =
{
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION, base_op_opmsg_get_capability_version},
    {OPMSG_SBC_ENC_ID_SET_ENCODING_PARAMS, sbc_enc_opmsg_set_encoding_params},
    /* {OPMSG_AD2P_DEC_ID_CONTENT_PROTECTION_ENABLE, sbc_dec_opmsg_content_protection_enable}, */
    {OPMSG_COMMON_ID_SET_CONTROL,                  sbc_enc_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                   sbc_enc_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,                 sbc_enc_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                   sbc_enc_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                   sbc_enc_opmsg_obpm_get_status},
    {0, NULL}
};

/** sbc encode capability data */
const CAPABILITY_DATA sbc_encode_cap_data =
{
    SBC_ENCODER_CAP_ID,
    SBC_ENCODE_SBCENC_VERSION_MAJOR, 1, /* Version information - hi and lo parts */
    2, 1,                           /* Max 1 sink and 2 sources */
    &sbc_encode_handler_table,
    sbc_encode_opmsg_handler_table,
    encode_process_data,        /* Data processing function */
    0,                              /* TODO: this would hold processing time information */
    sizeof(SBC_ENC_OP_DATA)
};


/** Memory owned by an sbc encoder instance */
const malloc_t_entry sbc_enc_malloc_table[SBC_ENC_MALLOC_TABLE_LENGTH] =
{
    {6, MALLOC_PREFERENCE_NONE, offsetof(sbc_codec, pre_post_proc_struc)},
    {SBC_ANALYSIS_BUFF_LENGTH, MALLOC_PREFERENCE_DM2, offsetof(sbc_codec, analysis_xch1)},
    {SBC_ANALYSIS_BUFF_LENGTH, MALLOC_PREFERENCE_DM2, offsetof(sbc_codec, analysis_xch2)}
};

const scratch_table encoder_scratch_table =
{
    SBC_ENC_DM1_SCRATCH_TABLE_LENGTH,
    SBC_ENC_DM2_SCRATCH_TABLE_LENGTH,
    0,
    sbc_scratch_table_dm1,
    sbc_scratch_table_dm2,
    NULL
};

const ENCODER_CAP_VIRTUAL_TABLE sbc_enc_vt =
{
        sbc_frame_encode,
        sbc_encode_free_data,
        sbc_encode_reset,
        sbc_encode_frame_sizes,
        &encoder_scratch_table,
};

/***************************************************************************
Public Function Definitions
*/




/***************************************************************************
Private Function Declarations
*/

/**
 * \brief Frees up any data that has been allocated by the instance of the
 * sbc_encode capability.
 *
 * \param op_data Pointer to the operator instance data.
 */
void sbc_encode_free_data(OPERATOR_DATA *op_data)
{
    SBC_ENC_OP_DATA *sbc_data = (SBC_ENC_OP_DATA *)op_data->extra_op_data;
    /* This can be called when create fails or on destroy so it checks that
     * every allocation has happened before calling free. */
    /* The variables aren't reset to NULL as this is either a failed create or
     * a destroy which means the operator data is about to be freed.
     */

    /* free the shared codec data */
    mem_table_free_shared((void *)(&(sbc_data->codec_data)),
                            sbc_shared_malloc_table, SBC_SHARED_TABLE_LENGTH);

    /* free shared decoder data */
    mem_table_free_shared((void *)(&(sbc_data->codec_data)),
                    sbc_enc_shared_malloc_table, SBC_ENC_SHARED_TABLE_LENGTH);

    /* free non-shared memory */
    mem_table_free((void *)(&(sbc_data->codec_data)), sbc_enc_malloc_table,
                                                SBC_ENC_MALLOC_TABLE_LENGTH);

}

/**
 * \brief Frees up any data that has been allocated by the instnace of the
 * sbc_encode capability and sets the response field to failed.
 *
 * \param op_data Pointer to the operator instance data.
 * \param response Pointer to the response message to give a failed status
 */
static void free_data_and_fail(OPERATOR_DATA *op_data, void **response)
{
    /* Free the data and then override the response message status to fail */
    sbc_encode_free_data(op_data);
    base_op_change_response_status(response, STATUS_CMD_FAILED);
}

/**
 * \brief reports input and output frame sizes
 *
 * \param in_size_samples number of samples consumed per frame
 * \param out_size_octets number of octets produced per frame
 */
void sbc_encode_frame_sizes(OPERATOR_DATA *op_data, unsigned *in_size_samples, unsigned *out_size_octets)
{
    SBC_ENC_OP_DATA *sbc_data = (SBC_ENC_OP_DATA *)op_data->extra_op_data;
    sbc_codec *codec_data = &sbc_data->codec_data;
    *in_size_samples = codec_data->enc_setting_nrof_subbands * codec_data->enc_setting_nrof_blocks;
    
    /* Calculate the encoded frame size from the encoder parameters
     * It might be better to cache this rather than recalculating for each frame 
     */
    switch (codec_data->enc_setting_channel_mode)
    {
    case 0 : /* MONO */
        /* nrof_channels = 1;
            * frame_length = 4 + (4 * nrof_subbands * nrof_channels) / 8 +  ceil((nrof_blocks * nrof_channels * bitpool) / 8); */
        *out_size_octets = 4 + (codec_data->enc_setting_nrof_subbands) / 2 + 
            ((codec_data->enc_setting_nrof_blocks * codec_data->enc_setting_bitpool) + 7) / 8;
        break;
    case 1: /* DUAL */
        /* nrof_channels = 2;
            * frame_length = 4 + (4 * nrof_subbands * nrof_channels) / 8 +  ceil((nrof_blocks * nrof_channels * bitpool) / 8); */
        *out_size_octets = 4 + codec_data->enc_setting_nrof_subbands + 
            ((codec_data->enc_setting_nrof_blocks * 2 * codec_data->enc_setting_bitpool) + 7) / 8;
        break;
    case 2: /* STEREO */
        /* nrof_channels = 2;
            * frame_length = 4 + (4 * nrof_subbands * nrof_channels) / 8 +  ceil((nrof_blocks * bitpool) / 8); */
        *out_size_octets = 4 + codec_data->enc_setting_nrof_subbands + 
            ((codec_data->enc_setting_nrof_blocks * codec_data->enc_setting_bitpool) + 7) / 8;
        break;
    case 3: /* JOINT */
        /* nrof_channels = 2;
            * frame_length = 4 + (4 * nrof_subbands * nrof_channels) / 8 +  ceil((nrof_subbands + nrof_blocks *  bitpool)) / 8); */
        *out_size_octets = 4 + codec_data->enc_setting_nrof_subbands + 
            ((codec_data->enc_setting_nrof_subbands + codec_data->enc_setting_nrof_blocks * codec_data->enc_setting_bitpool) + 7) / 8;
        break;
    default:
        L2_DBG_MSG("sbc_encode_frame_sizes unknown mode!");
        *out_size_octets = 0;
    }
}

/**
 * \brief Allocates the sbc_encode specific capability memory and initialises
 * the decoder.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the create request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool sbc_encode_create(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data)
{
    bool new_allocation;
    SBC_ENC_OP_DATA *sbc_data = (SBC_ENC_OP_DATA *)op_data->extra_op_data;

    /* call base_op create, which also allocates and fills response message */
    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* Create the link to the base class object */
    if (!encoder_base_class_init(op_data, &sbc_data->encoder_data, &(sbc_data->codec_data), &sbc_enc_vt))
    {
        free_data_and_fail(op_data, response_data);
        return TRUE;
    }

    /* Stage 1 - Malloc the sbc decoder fields. A lot of this can be shared
     * between any other sbc decoders/encoders in the system. Including WBS
     * capabilities. */

    /* Share memory with decoders. */
    if (!mem_table_zalloc_shared((void *)(&(sbc_data->codec_data)),
                    sbc_enc_shared_malloc_table, SBC_ENC_SHARED_TABLE_LENGTH,
                    &new_allocation))
    {
        free_data_and_fail(op_data, response_data);
        return TRUE;
    }

    /* share memory with other sbc instances */
    if (!mem_table_zalloc_shared((void *)(&(sbc_data->codec_data)),
                            sbc_shared_malloc_table, SBC_SHARED_TABLE_LENGTH,
                            &new_allocation) )
    {
        free_data_and_fail(op_data, response_data);
        return TRUE;
    }

    /* now allocate the non-shareable memory */
    if (!mem_table_zalloc((uintptr_t *)(&(sbc_data->codec_data)), sbc_enc_malloc_table,
                                                SBC_ENC_MALLOC_TABLE_LENGTH))
    {
        free_data_and_fail(op_data, response_data);
        return TRUE;
    }

    /* Successfully allocated everything! */
    /* Stage 2 populate the ENCODER structure */


    /* Call the sbc decoder init_decode and init_tables functions. */
    sbc_encode_lib_init(&(sbc_data->codec_data));

    /* Init default encoding values */
    sbc_data->codec_data.enc_setting_nrof_subbands = 8;
    sbc_data->codec_data.enc_setting_nrof_blocks = 16;
    sbc_data->codec_data.enc_setting_bitpool = 55;
    sbc_data->codec_data.enc_setting_sampling_freq = 3;
    sbc_data->codec_data.enc_setting_channel_mode = 2;
    sbc_data->codec_data.enc_setting_allocation_method = 0;

    return TRUE;
}


/**
 * \brief Resets the sbc_encode capability.
 *
 * \param op_data Pointer to the operator instance data.
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool sbc_encode_reset(OPERATOR_DATA *op_data)
{
    sbc_encode_lib_reset(((SBC_ENC_OP_DATA *)(op_data->extra_op_data))->encoder_data.codec.encoder_data_object);

    return TRUE;
}


bool sbc_enc_opmsg_set_encoding_params(OPERATOR_DATA *op_data, void *message_data, unsigned int *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    unsigned sample_freq, enc_sample_freq;
    SBC_ENC_OP_DATA *sbc_data = (SBC_ENC_OP_DATA *)(op_data->extra_op_data);

    /* Check that the message recieved is the correct length. No checking for
     * stupid configurations though this might be nice to have. */
    if (((unsigned*)message_data)[1] != 7)
    {
        return FALSE;
    }

    /* SBC wants sampling frequency stored as logical mapping 48k = 3, 44.1k = 2
     * 32k = 1, 16k = 0.
     * The message format is fs/25 so map valid values, otherwise reject the
     * encoding parameters received.
     */
    sample_freq = ((unsigned*)message_data)[6];
    switch (sample_freq)
    {
        case 48000/CONVERSION_SAMPLE_RATE_TO_HZ:
        {
            enc_sample_freq = 3;
            break;
        }
        case 44100/CONVERSION_SAMPLE_RATE_TO_HZ:
        {
            enc_sample_freq = 2;
            break;
        }
        case 32000/CONVERSION_SAMPLE_RATE_TO_HZ:
        {
            enc_sample_freq = 1;
            break;
        }
        case 16000/CONVERSION_SAMPLE_RATE_TO_HZ:
        {
            enc_sample_freq = 0;
            break;
        }
        default:
        {
            return FALSE;
        }
    }

    /* Populate the sbc structure with the new fields */
    sbc_data->codec_data.enc_setting_nrof_subbands = ((unsigned*)message_data)[3];
    sbc_data->codec_data.enc_setting_nrof_blocks = ((unsigned*)message_data)[4];
    sbc_data->codec_data.enc_setting_bitpool = ((unsigned*)message_data)[5];
    sbc_data->codec_data.enc_setting_sampling_freq = enc_sample_freq;
    sbc_data->codec_data.enc_setting_channel_mode = ((unsigned*)message_data)[7];
    sbc_data->codec_data.enc_setting_allocation_method = ((unsigned*)message_data)[8];

#ifdef INSTALL_METADATA
    sbc_data->encoder_data.sample_rate = sample_freq * CONVERSION_SAMPLE_RATE_TO_HZ;
    if (sbc_data->codec_data.enc_setting_nrof_subbands == 8)
    {
        sbc_data->encoder_data.delay_samples = SBC_DELAY_8_SUBBANDS;
    }
    else
    {
        sbc_data->encoder_data.delay_samples = SBC_DELAY_4_SUBBANDS;
    }
#endif

    return TRUE;
}

/* obpm support for encoder */
bool sbc_enc_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    /* In the case of this capability, nothing is done for control message. Just follow protocol and ignore any content. */
    return cps_control_setup(message_data, resp_length, resp_data,NULL);
}


bool sbc_enc_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    return FALSE;
}

bool sbc_enc_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
  
     return FALSE;
}

bool sbc_enc_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    /* Set the parameter(s). For future proofing, it is using the whole mechanism, although currently there is only one field
     * in opdata structure that is a setable parameter. If later there will be more (ever), must follow contiquously the first field,
     * as commented and instructed in the op data definition. Otherwise consider moving them into a dedicated structure.
     */

    return FALSE;
}

bool sbc_enc_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SBC_ENC_OP_DATA * op_extra_data =  (SBC_ENC_OP_DATA*)op_data->extra_op_data;

    unsigned* resp = NULL;
    
    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(SBC_ENCODE_STATISTICS) ,&resp))
    {
         return FALSE;
    }

    if(resp)  
    {    
            resp = cpsPackWords(&op_extra_data->codec_data.sampling_freq, &op_extra_data->codec_data.channel_mode, resp);
            resp = cpsPackWords(&op_extra_data->codec_data.bitpool, &op_extra_data->codec_data.nrof_blocks, resp);   
            resp = cpsPackWords(&op_extra_data->codec_data.nrof_channels, &op_extra_data->codec_data.nrof_subbands, resp);
            cpsPackWords(&op_extra_data->codec_data.allocation_method,NULL, resp);
    }

	return TRUE;
}




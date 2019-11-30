/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  aac_decode.c
 * \ingroup  capabilities
 *
 *  AAC decode operator
 *
 */

#include "capabilities.h"
#include "mem_utils/scratch_memory.h"
#include "mem_utils/memory_table.h"
#include "codec_c.h"
#include "aac_c.h"
#include "a2dp_decode/a2dp_common_decode.h"
#include "platform/pl_assert.h"
#include "op_msg_helpers.h"

#ifdef INSTALL_METADATA
    #include "hydra_cbuff.h"
#endif /* INSTALL_METADATA */

// add autogen header
#include "patch/patch.h"
#include "aac_decode_gen_c.h"
#include "aac_decode.h"
/****************************************************************************
Private Constant Declarations
*/
/** The maximum number of samples in a single AAC encoded frame */

#define AAC_DECODE_INPUT_BUFFER_SIZE    MAX_AAC_FRAME_SIZE_IN_WORDS
#define SRA_HEADROOM                    64
#if defined (AACDEC_SBR_ADDITIONS) || defined(AACDEC_PS_ADDITIONS) || defined(AACDEC_ELD_ADDITIONS)
    #define AAC_DECODE_OUTPUT_BUFFER_SIZE   2*AAC_FRAME_BUF_LENGTH + SRA_HEADROOM
#else
    #define AAC_DECODE_OUTPUT_BUFFER_SIZE   AAC_FRAME_BUF_LENGTH + SRA_HEADROOM
#endif
#define AAC_MP4                         0
#define AAC_ADTS                        1
#define AAC_LATM                        2
/****************************************************************************
Private Constant Definitions
*/
/** The scratch buffer size required if performing ratematching inside the capability
 * This has to be AAC frame size + some SRA headroom (already included above)*/
#define AAC_SCRATCH_BUFFER_SIZE  AAC_DECODE_OUTPUT_BUFFER_SIZE * sizeof(unsigned)

/****************************************************************************
Private Type Definitions
*/

/* THIS IS THIEVED FROM an old capabilities.h */
typedef struct
{
    /** A2DP_DECODER_PARAMS must be the first parameters always */
    A2DP_DECODER_PARAMS decoder_data;

    /** The aac_codec specific data */
    aac_codec codec_data;
} AAC_DEC_OP_DATA;

#define A2DP_STRIP_AAC_HDR           A2DP_STRIP_BFRAME
#define A2DP_AAC_HDR_SIZE            A2DP_BFRAME_HDR_SIZE
#define A2DP_STRIP_AAC_CP_HDR        ( A2DP_STRIP_BFRAME | \
                                        A2DP_STRIP_RTP | \
                                        A2DP_STRIP_CP )
#define A2DP_AAC_CP_HDR_SIZE        ( A2DP_BFRAME_HDR_SIZE + \
                                        A2DP_RTP_HDR_SIZE + \
                                        A2DP_CP_HDR_SIZE )

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define AAC_DECODER_CAP_ID CAP_ID_DOWNLOAD_AAC_DECODER
#define AAC_SHUNT_DECODER_CAP_ID CAP_ID_DOWNLOAD_AAC_SHUNT_DECODER
#else
#define AAC_DECODER_CAP_ID CAP_ID_AAC_DECODER
#define AAC_SHUNT_DECODER_CAP_ID CAP_ID_AAC_SHUNT_DECODER
#endif

/** The aac_decoder capability function handler table */
const handler_lookup_struct aac_decode_handler_table =
{
    aac_decode_create,           /* OPCMD_CREATE */
    aac_decode_destroy,          /* OPCMD_DESTROY */
    a2dp_decode_start,           /* OPCMD_START */
    base_op_stop,                /* OPCMD_STOP */
    aac_decode_reset,            /* OPCMD_RESET */
    aac_decode_connect,          /* OPCMD_CONNECT */
    a2dp_decode_disconnect,      /* OPCMD_DISCONNECT */
    aac_decode_buffer_details,   /* OPCMD_BUFFER_DETAILS */
    a2dp_decode_get_data_format, /* OPCMD_DATA_FORMAT */
    a2dp_decode_get_sched_info   /* OPCMD_GET_SCHED_INFO */
};
const opmsg_handler_lookup_table_entry aac_decode_opmsg_handler_table[] =
{
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION, base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_FADEOUT_ENABLE, a2dp_dec_opmsg_enable_fadeout},
    {OPMSG_COMMON_ID_FADEOUT_DISABLE, a2dp_dec_opmsg_disable_fadeout},
    {OPMSG_SET_CTRL, a2dp_dec_assign_buffering},
#ifndef TIMED_PLAYBACK_MODE
    {OPMSG_COMMON_SET_RM_ENACTING, aac_dec_opmsg_ratematch_enacting},
#endif
    {OPMSG_AAC_ID_SET_FRAME_TYPE, aac_set_frame_type},

    {OPMSG_COMMON_ID_SET_CONTROL,                  aac_dec_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                   aac_dec_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,                 aac_dec_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                   aac_dec_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                   aac_dec_opmsg_obpm_get_status},
    {0, NULL}
};
/** aac decode capability data */
#ifdef INSTALL_OPERATOR_AAC_DECODE
const CAPABILITY_DATA aac_decode_cap_data =
{
    AAC_DECODER_CAP_ID,
    AAC_DECODE_AACD_VERSION_MAJOR, 1, /* Version information - hi and lo parts */
    1, 2,                           /* Max 1 sink and 2 sources */
    &aac_decode_handler_table,
    aac_decode_opmsg_handler_table,
    aac_decode_process_data,        /* Data processing function */
    0,                              /* TODO: this would hold processing time information */
    sizeof(AAC_DEC_OP_DATA)
};
#endif

#ifdef INSTALL_OPERATOR_AAC_SHUNT_DECODER

const opmsg_handler_lookup_table_entry aac_strip_decode_opmsg_handler_table[] =
{
    {OPMSG_COMMON_ID_GET_CAPABILITY_VERSION, base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_FADEOUT_ENABLE, a2dp_dec_opmsg_enable_fadeout},
    {OPMSG_COMMON_ID_FADEOUT_DISABLE, a2dp_dec_opmsg_disable_fadeout},
    {OPMSG_SET_CTRL, a2dp_dec_assign_buffering},
    {OPMSG_AD2P_DEC_ID_CONTENT_PROTECTION_ENABLE, aac_dec_opmsg_content_protection_enable},
#ifndef TIMED_PLAYBACK_MODE
    {OPMSG_COMMON_SET_RM_ENACTING, aac_dec_opmsg_ratematch_enacting},
#endif
    {OPMSG_COMMON_ID_SET_CONTROL,                  aac_dec_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                   aac_dec_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,                 aac_dec_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                   aac_dec_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                   aac_dec_opmsg_obpm_get_status},
    {0, NULL}
};

const CAPABILITY_DATA aac_a2dp_decoder_cap_data =
{
    AAC_SHUNT_DECODER_CAP_ID,
    0, 1,                           /* Version information - hi and lo parts */
    1, 2,                           /* Max 1 sink and 2 sources */
    &aac_decode_handler_table,
    a2dp_decode_opmsg_handler_table,
    aac_decode_process_data,        /* Data processing function */
    0,                              /* TODO: this would hold processing time information */
    sizeof(AAC_DEC_OP_DATA) + sizeof(A2DP_HEADER_PARAMS)
};
#endif

/** Memory owned by an aac decoder instance */
    const malloc_t_entry aac_dec_malloc_table[] =
    {
#ifdef AACDEC_ELD_ADDITIONS
        {AAC_OVERLAP_ADD_LENGTH,                  MALLOC_PREFERENCE_DM2,  offsetof(aac_codec, overlap_add_left_ptr)},
        {AAC_OVERLAP_ADD_LENGTH,                  MALLOC_PREFERENCE_DM2,  offsetof(aac_codec, overlap_add_right_ptr)},
        {AAC_TNS_INPUT_HISTORY_LENGTH,            MALLOC_PREFERENCE_NONE, offsetof(aac_codec, tns_fir_input_history_ptr)},
        {AAC_DEC_SBR_X_IMAG_LENGTH,               MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_x_imag_ptr)},
        {AAC_DEC_SBR_X_REAL_LENGTH,               MALLOC_PREFERENCE_DM2,  offsetof(aac_codec, SBR_x_real_ptr)},
        {AAC_DEC_SBR_V_BUF_LENGTH,                MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_v_buffer_left_ptr)},
        {AAC_DEC_SBR_V_BUF_LENGTH,                MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_v_buffer_right_ptr)},
        {AAC_DEC_SBR_X_INPUT_BUF_LENGTH,          MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_x_input_buffer_left_ptr)},
        {AAC_DEC_SBR_X_INPUT_BUF_LENGTH,          MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_x_input_buffer_right_ptr)},
        {AAC_DEC_SBR_OTHER_IMAG_LENGTH,           MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_X_sbr_other_imag_ptr)},
        {AAC_DEC_SBR_INFO_LENGTH,                 MALLOC_PREFERENCE_NONE, offsetof(aac_codec, SBR_info_ptr)},
        {AAC_DEC_SBR_SYNTH_TMP_LENGTH,            MALLOC_PREFERENCE_NONE, offsetof(aac_codec, SBR_synth_temp_ptr)},
        {AAC_DEC_PS_INFO_LENGTH,                  MALLOC_PREFERENCE_NONE, offsetof(aac_codec, PS_info_ptr)},
        {AAC_DEC_PS_X_HYBRID_RIGHT_IMAG_LENGTH,   MALLOC_PREFERENCE_NONE, offsetof(aac_codec, PS_X_hybrid_imag_address[1])},
        {AAC_DEC_PS_HYBRID_ALLPASS_BUFFER_LENGTH, MALLOC_PREFERENCE_NONE, offsetof(aac_codec, PS_hybrid_allpass_feedback_buffer_ptr)},
        {AAC_DEC_PS_QMF_ALLPASS_BUFFER_LENGTH,    MALLOC_PREFERENCE_NONE, offsetof(aac_codec, PS_qmf_allpass_feedback_buffer_ptr)},
        {AAC_DEC_PS_HYBRID_TYPE_A_BUFFER_LENGTH,  MALLOC_PREFERENCE_NONE, offsetof(aac_codec, PS_hybrid_type_a_fir_filter_input_buffer_ptr)},
        {AAC_DEC_PS_LONG_DELAY_BUFFER_LENGTH,     MALLOC_PREFERENCE_NONE, offsetof(aac_codec, PS_long_delay_band_buffer_real_ptr)},
        {AAC_DEC_PS_LONG_DELAY_BUFFER_LENGTH,     MALLOC_PREFERENCE_NONE, offsetof(aac_codec, PS_long_delay_band_buffer_imag_ptr)},
        {AAC_DEC_ELD_TEMP_U_LENGTH,               MALLOC_PREFERENCE_NONE, offsetof(aac_codec, ELD_temp_u_ptr)},
        {AAC_DEC_ELD_SYNTH_TEMP1_LENGTH,          MALLOC_PREFERENCE_NONE, offsetof(aac_codec, ELD_synthesis_temp1_ptr)},
        {AAC_DEC_ELD_SYNTH_TEMP2_LENGTH,          MALLOC_PREFERENCE_NONE, offsetof(aac_codec, ELD_synthesis_temp2_ptr)},
        {AAC_DEC_ELD_SYNTH_TEMP3_LENGTH,          MALLOC_PREFERENCE_NONE, offsetof(aac_codec, ELD_synthesis_temp3_ptr)},
        {AAC_DEC_ELD_GW_BUF_LENGTH,               MALLOC_PREFERENCE_NONE, offsetof(aac_codec, ELD_synthesis_g_w_buffer_ptr)},
        {AAC_DEC_ELD_SBR_TEMP_LENGTH,             MALLOC_PREFERENCE_DM2,  offsetof(aac_codec, ELD_sbr_temp_5_ptr)},
        {AAC_DEC_ELD_SBR_TEMP_LENGTH,             MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, ELD_sbr_temp_6_ptr)},
        {AAC_DEC_ELD_SBR_TEMP_LENGTH,             MALLOC_PREFERENCE_DM2,  offsetof(aac_codec, ELD_sbr_temp_7_ptr)},
        {AAC_DEC_ELD_SBR_TEMP_LENGTH,             MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, ELD_sbr_temp_8_ptr)},
        {AAC_DEC_ELD_IFFT_RE_LENGTH,              MALLOC_PREFERENCE_NONE, offsetof(aac_codec, ELD_ifft_re_ptr)},
    };
#else
#ifdef AACDEC_PS_ADDITIONS
        {AAC_OVERLAP_ADD_LENGTH,                  MALLOC_PREFERENCE_DM2,  offsetof(aac_codec, overlap_add_left_ptr)},
        {AAC_OVERLAP_ADD_LENGTH,                  MALLOC_PREFERENCE_DM2,  offsetof(aac_codec, overlap_add_right_ptr)},
        {AAC_TNS_INPUT_HISTORY_LENGTH,            MALLOC_PREFERENCE_NONE, offsetof(aac_codec, tns_fir_input_history_ptr)},
        {AAC_DEC_SBR_X_IMAG_LENGTH,               MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_x_imag_ptr)},
        {AAC_DEC_SBR_X_REAL_LENGTH,               MALLOC_PREFERENCE_DM2,  offsetof(aac_codec, SBR_x_real_ptr)},
        {AAC_DEC_SBR_V_BUF_LENGTH,                MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_v_buffer_left_ptr)},
        {AAC_DEC_SBR_V_BUF_LENGTH,                MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_v_buffer_right_ptr)},
        {AAC_DEC_SBR_X_INPUT_BUF_LENGTH,          MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_x_input_buffer_left_ptr)},
        {AAC_DEC_SBR_X_INPUT_BUF_LENGTH,          MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_x_input_buffer_right_ptr)},
        {AAC_DEC_SBR_OTHER_IMAG_LENGTH,           MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_X_sbr_other_imag_ptr)},
        {AAC_DEC_SBR_INFO_LENGTH,                 MALLOC_PREFERENCE_NONE, offsetof(aac_codec, SBR_info_ptr)},
        {AAC_DEC_SBR_SYNTH_TMP_LENGTH,            MALLOC_PREFERENCE_NONE, offsetof(aac_codec, SBR_synth_temp_ptr)},
        {AAC_DEC_PS_INFO_LENGTH,                  MALLOC_PREFERENCE_NONE, offsetof(aac_codec, PS_info_ptr)},
        {AAC_DEC_PS_X_HYBRID_RIGHT_IMAG_LENGTH,   MALLOC_PREFERENCE_NONE, offsetof(aac_codec, PS_X_hybrid_imag_address[1])},
        {AAC_DEC_PS_HYBRID_ALLPASS_BUFFER_LENGTH, MALLOC_PREFERENCE_NONE, offsetof(aac_codec, PS_hybrid_allpass_feedback_buffer_ptr)},
        {AAC_DEC_PS_QMF_ALLPASS_BUFFER_LENGTH,    MALLOC_PREFERENCE_NONE, offsetof(aac_codec, PS_qmf_allpass_feedback_buffer_ptr)},
        {AAC_DEC_PS_HYBRID_TYPE_A_BUFFER_LENGTH,  MALLOC_PREFERENCE_NONE, offsetof(aac_codec, PS_hybrid_type_a_fir_filter_input_buffer_ptr)},
        {AAC_DEC_PS_LONG_DELAY_BUFFER_LENGTH,     MALLOC_PREFERENCE_NONE, offsetof(aac_codec, PS_long_delay_band_buffer_real_ptr)},
        {AAC_DEC_PS_LONG_DELAY_BUFFER_LENGTH,     MALLOC_PREFERENCE_NONE, offsetof(aac_codec, PS_long_delay_band_buffer_imag_ptr)},
    };
#else
#ifdef AACDEC_SBR_ADDITIONS
        {AAC_OVERLAP_ADD_LENGTH,                  MALLOC_PREFERENCE_DM2,  offsetof(aac_codec, overlap_add_left_ptr)},
        {AAC_OVERLAP_ADD_LENGTH,                  MALLOC_PREFERENCE_DM2,  offsetof(aac_codec, overlap_add_right_ptr)},
        {AAC_TNS_INPUT_HISTORY_LENGTH,            MALLOC_PREFERENCE_NONE, offsetof(aac_codec, tns_fir_input_history_ptr)},
        {AAC_DEC_SBR_X_IMAG_LENGTH,               MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_x_imag_ptr)},
        {AAC_DEC_SBR_X_REAL_LENGTH,               MALLOC_PREFERENCE_DM2,  offsetof(aac_codec, SBR_x_real_ptr)},
        {AAC_DEC_SBR_V_BUF_LENGTH,                MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_v_buffer_left_ptr)},
        {AAC_DEC_SBR_V_BUF_LENGTH,                MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_v_buffer_right_ptr)},
        {AAC_DEC_SBR_X_INPUT_BUF_LENGTH,          MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_x_input_buffer_left_ptr)},
        {AAC_DEC_SBR_X_INPUT_BUF_LENGTH,          MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_x_input_buffer_right_ptr)},
        {AAC_DEC_SBR_OTHER_IMAG_LENGTH,           MALLOC_PREFERENCE_DM1,  offsetof(aac_codec, SBR_X_sbr_other_imag_ptr)},
        {AAC_DEC_SBR_INFO_LENGTH,                 MALLOC_PREFERENCE_NONE, offsetof(aac_codec, SBR_info_ptr)},
        {AAC_DEC_SBR_SYNTH_TMP_LENGTH,            MALLOC_PREFERENCE_NONE, offsetof(aac_codec, SBR_synth_temp_ptr)},
    };
#else
        {AAC_FRAME_BUF_LENGTH,                    MALLOC_PREFERENCE_DM2,  offsetof(aac_codec, buf_left_ptr)},
        {AAC_FRAME_BUF_LENGTH,                    MALLOC_PREFERENCE_DM2,  offsetof(aac_codec, buf_right_ptr)},
        {AAC_OVERLAP_ADD_LENGTH,                  MALLOC_PREFERENCE_DM2,  offsetof(aac_codec, overlap_add_left_ptr)},
        {AAC_OVERLAP_ADD_LENGTH,                  MALLOC_PREFERENCE_DM2,  offsetof(aac_codec, overlap_add_right_ptr)},
        {AAC_TNS_INPUT_HISTORY_LENGTH,            MALLOC_PREFERENCE_NONE, offsetof(aac_codec, tns_fir_input_history_ptr)},
    };
#endif
#endif
#endif



/***************************************************************************
Public Function Definitions
*/
/** Initialise the aac decoder (ASM) */
extern void aac_decode_lib_init(DECODER *decoder);
/** Reset the aac decoder (ASM) */
extern void aac_decode_lib_reset(DECODER *decoder);
/** Populate AAC_DEC_OP_DATA with functions aac decoder functions */
extern void populate_aac_asm_funcs(void (**decode_frame)(void), void (**silence)(void));
/** Populate AAC_DEC_OP_DATA with functions aac decoder functions */
extern void populate_strip_aac_asm_funcs(void (**decode_frame)(void),
                                         void (**silence)(void),
                                         void (**get_bits)(void));

extern void aac_decode_free_decoder_twiddle(void);


/***************************************************************************
Private Function Declarations
*/

/**
 * \brief Frees up any data that has been allocated by the instance of the
 * aac_decode capability.
 *
 * \param op_data Pointer to the operator instance data.
 */
static void free_data(OPERATOR_DATA *op_data)
{
    AAC_DEC_OP_DATA *aac_data = (AAC_DEC_OP_DATA *)op_data->extra_op_data;
    /* This can be called when create fails or on destroy so it checks that
     * every allocation has happened before calling free. */
    /* The variables aren't reset to NULL as this is either a failed create or
     * a destroy which means the operator data is about to be freed.
     */

    /* free shared decoder data */
    mem_table_free_shared((void *)(&(aac_data->codec_data)),
                    aac_dec_shared_malloc_table, sizeof(aac_dec_shared_malloc_table) / sizeof(share_malloc_t_entry));

    /* free non-shared memory */
    mem_table_free((void *)(&(aac_data->codec_data)),
                    aac_dec_malloc_table, sizeof(aac_dec_malloc_table) / sizeof(malloc_t_entry));

    aac_decode_free_decoder_twiddle();
}

/**
 * \brief Frees up any data that has been allocated by the instnace of the
 * aac_decode capability and sets the response field to failed.
 *
 * \param op_data Pointer to the operator instance data.
 * \param response Pointer to the response message to give a failed status
 */
static void free_data_and_fail(OPERATOR_DATA *op_data, void **response)
{
    /* Free the data and then override the response message status to fail */
    free_data(op_data);
    base_op_change_response_status(response, STATUS_CMD_FAILED);
}

/**
 * \brief Allocates the aac_decode specific capability memory and initialises
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
bool aac_decode_create(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data)
{
    bool new_allocation;
    AAC_DEC_OP_DATA *aac_data = (AAC_DEC_OP_DATA *)op_data->extra_op_data;

    /* call base_op create, which also allocates and fills response message */
    if (!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }


         /* Share memory with decoders. */
    if( !mem_table_zalloc_shared((void *)(&(aac_data->codec_data)),
                    aac_dec_shared_malloc_table, sizeof(aac_dec_shared_malloc_table) / sizeof(share_malloc_t_entry),
                    &new_allocation))
    {
        free_data_and_fail(op_data, response_data);
        return TRUE;
    }

    /* now allocate the non-shareable memory */
    if(!mem_table_zalloc((uintptr_t *)(&(aac_data->codec_data)),
                    aac_dec_malloc_table, sizeof(aac_dec_malloc_table) / sizeof(malloc_t_entry)))
    {
        free_data_and_fail(op_data, response_data);
        return TRUE;
    }

 if (scratch_register())
    {
        if (scratch_reserve(AAC_FRAME_MEM_POOL_LENGTH*sizeof(int), MALLOC_PREFERENCE_DM2))
        {
            if (scratch_reserve(AAC_TMP_MEM_POOL_LENGTH*sizeof(int), MALLOC_PREFERENCE_DM1))
            {

                /* Successfully allocated everything! */
                /* Stage 2 populate the DECODER structure */

                /* Tell the codec structure where to find the aac codec data */
                aac_data->decoder_data.codec.decoder_data_object = &(aac_data->codec_data);

#ifdef AACDEC_SBR_ADDITIONS
                aac_data->codec_data.buf_left_ptr                        = aac_data->codec_data.SBR_x_real_ptr     + 1024;
                aac_data->codec_data.buf_right_ptr                       = aac_data->codec_data.SBR_x_real_ptr     + 2048;
                aac_data->codec_data.tmp_mem_pool_ptr                    = aac_data->codec_data.SBR_x_imag_ptr     + 512;
                aac_data->codec_data.tmp_mem_pool_end_ptr                = aac_data->codec_data.tmp_mem_pool_ptr;
                aac_data->codec_data.SBR_synthesis_post_process_imag_ptr = aac_data->codec_data.SBR_x_real_ptr;
                aac_data->codec_data.SBR_X_2env_imag_ptr                 = aac_data->codec_data.SBR_x_imag_ptr;
                aac_data->codec_data.SBR_X_curr_imag_ptr                 = aac_data->codec_data.SBR_x_imag_ptr     + 128;
                aac_data->codec_data.SBR_X_2env_real_ptr                 = aac_data->codec_data.SBR_x_real_ptr     + 512;
                aac_data->codec_data.SBR_X_curr_real_ptr                 = aac_data->codec_data.SBR_x_real_ptr     + 640;
                aac_data->codec_data.SBR_X_sbr_other_real_ptr            = aac_data->codec_data.SBR_x_real_ptr     + 128;
                aac_data->codec_data.SBR_temp_2_ptr                      = aac_data->codec_data.SBR_x_imag_ptr     + 3072;
                aac_data->codec_data.SBR_temp_4_ptr                      = aac_data->codec_data.SBR_x_imag_ptr     + 3200;
#endif

#ifdef AACDEC_PS_ADDITIONS
                aac_data->codec_data.PS_X_hybrid_real_address[0]         = aac_data->codec_data.SBR_synth_temp_ptr;
                aac_data->codec_data.PS_X_hybrid_real_address[1]         = aac_data->codec_data.SBR_synth_temp_ptr + 640;
                aac_data->codec_data.PS_X_hybrid_imag_address[0]         = aac_data->codec_data.SBR_synth_temp_ptr + 320;
#endif

                /* Call the aac decoder init_decode and init_tables functions. */
                aac_data->codec_data.read_frame_function = AAC_LATM;
                aac_decode_lib_init(&(aac_data->decoder_data.codec));


#ifdef INSTALL_OPERATOR_AAC_SHUNT_DECODER
                if( op_data->cap_data->id == AAC_SHUNT_DECODER_CAP_ID)
                {
                    aac_data->decoder_data.a2dp_header =(A2DP_HEADER_PARAMS*)
                              ((unsigned*)aac_data + sizeof(AAC_DEC_OP_DATA));
                    aac_data->decoder_data.a2dp_header->type = A2DP_STRIP_AAC_HDR;
                    aac_data->decoder_data.a2dp_header->hdr_size = A2DP_AAC_HDR_SIZE;
                    /* a2dp_header will be valid here, hence no need to check */
                    populate_strip_aac_asm_funcs(&(aac_data->decoder_data.decode_frame),
                               &(aac_data->decoder_data.silence),
                               &(aac_data->decoder_data.a2dp_header->get_bits));
                }
                else  /* CAP_ID_AAC_DECODE */
#endif /* INSTALL_OPERATOR_AAC_SHUNT_DECODER */
                {
                    populate_aac_asm_funcs(&(aac_data->decoder_data.decode_frame),
                               &(aac_data->decoder_data.silence));
                }
                 return TRUE;
        }
    }

       /* Fail free all the scratch memory we reserved */
        scratch_deregister();
    }
            /* Clear up all the allocated memory. */
    free_data_and_fail(op_data, response_data);
    return TRUE;
}
/**
 * \brief Reports the buffer requirements of the requested capability terminal
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */

bool aac_decode_buffer_details(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data)
{
    return a2dp_decode_buffer_details_core(op_data, message_data,response_id, response_data,AAC_DECODE_INPUT_BUFFER_SIZE,AAC_DECODE_OUTPUT_BUFFER_SIZE);
}
 /**
 * \brief Connects a capability terminal to a buffer.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the connect request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool aac_decode_connect(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data)
{
    return a2dp_decode_connect_core(op_data, message_data,response_id, response_data,AAC_DECODE_OUTPUT_BUFFER_SIZE);
}

#ifndef TIMED_PLAYBACK_MODE
/**
 * \brief Enable or disable ratematching.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the request message payload
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 *
 * \Note The recieved message format is:
 *    word 0 - client id
 *    word 1 - length
 *    word 2 - op message id
 *    word 3 - boolean TRUE = enable ratematching, FALSE = disable
 *    word 4 - pointer to where the rateadjustment value will be written
 */
 bool aac_dec_opmsg_ratematch_enacting(OPERATOR_DATA *op_data,
                    void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
   return a2dp_dec_opmsg_ratematch_enacting_core(op_data,message_data, AAC_DECODE_OUTPUT_BUFFER_SIZE);
}

#endif /* TIMED_PLAYBACK_MODE */
/**
 * \brief Deallocates the aac_decode specific capability memory.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the destroy request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool aac_decode_destroy(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data)
{
    if(base_op_destroy(op_data, message_data, response_id, response_data))
    {
        /* Free all the scratch memory we reserved */
        scratch_deregister();
        /* Clear up the aac_decode specific work and then let base_op do
         * the grunt work. */
        free_data(op_data);
        return TRUE;
    }

    return FALSE;
}

/**
 * \brief set the aac frame type needed by AAC decoder
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the destroy request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * Mp4 -->0 , adts --->1 , latm --->2
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool aac_set_frame_type(OPERATOR_DATA *op_data, void *message_data,
                                        unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{

    AAC_DEC_OP_DATA *aac_data = (AAC_DEC_OP_DATA *)op_data->extra_op_data;

    if(op_data->state == OP_RUNNING)
    {
     /* Can't change Frame type  while running */
       return (FALSE);
    }

     /* Call the aac decoder init_decode and init_tables functions. */
    aac_data->codec_data.read_frame_function = OPMSG_FIELD_GET(message_data,
                                OPMSG_AAC_SET_FRAME_TYPE, FRAME_TYPE);
    aac_decode_lib_init(&(aac_data->decoder_data.codec));
    return TRUE;
}

/**
 * \brief Resets the aac_decode capability.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the reset request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool aac_decode_reset(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data)
{
    AAC_DEC_OP_DATA *aac_data = (AAC_DEC_OP_DATA *)(op_data->extra_op_data);

    if (!base_op_reset(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    aac_decode_lib_reset(&(aac_data->decoder_data.codec));

    return TRUE;
}

/**
 * \brief process function to decode available input data
 *
 * \param op_data Pointer to the operator instance data.
 * \param touched Structure to return the terminals which this operator wants kicked
 */
void aac_decode_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    AAC_DEC_OP_DATA *aac_data;
    unsigned output_samples;

    patch_fn(aac_decode);

    aac_data = (AAC_DEC_OP_DATA *)(op_data->extra_op_data);


    /* Check the input hasn't gone away, if it has then nothing we can do. It's
     * a radio link so this can happen */
    if (NULL == aac_data->decoder_data.codec.in_buffer)
    {
        /* One option is to produce silence in this case.*/
        return;
    }

    if((aac_data->decoder_data.play != NULL) &&
       (!(*(aac_data->decoder_data.play))))
    {
        return;
    }

    /* Commit any scratch memory ideally this should be done later after the
     * decision to decode is made. The call is cheap so it doesn't hurt to
     * do it here currently. */
    aac_data->codec_data.frame_mem_pool_ptr =
            scratch_commit(AAC_FRAME_MEM_POOL_LENGTH*sizeof(int), MALLOC_PREFERENCE_DM2);
    aac_data->codec_data.tmp_mem_pool_ptr =
            scratch_commit(AAC_TMP_MEM_POOL_LENGTH*sizeof(int), MALLOC_PREFERENCE_DM1);

    aac_data->codec_data.frame_mem_pool_end_ptr =  aac_data->codec_data.frame_mem_pool_ptr;
    aac_data->codec_data.tmp_mem_pool_end_ptr =  aac_data->codec_data.tmp_mem_pool_ptr;


#ifdef AACDEC_SBR_ADDITIONS
    aac_data->codec_data.SBR_temp_1_ptr        = aac_data->codec_data.frame_mem_pool_ptr + 256;
    aac_data->codec_data.SBR_temp_3_ptr        = aac_data->codec_data.frame_mem_pool_ptr + 384;
#endif

#ifdef AACDEC_PS_ADDITIONS
    aac_data->codec_data.PS_fmp_remains_ptr    = aac_data->codec_data.frame_mem_pool_ptr + 512;
#endif

    do
    {
#ifndef TIMED_PLAYBACK_MODE
    if (NULL != aac_data->decoder_data.sra)
    {
        if (aac_data->decoder_data.codec.out_left_buffer)
        {
            unsigned op_space;
            void *scratch_buf = scratch_commit(AAC_SCRATCH_BUFFER_SIZE, MALLOC_PREFERENCE_NONE);
            cbuffer_scratch_commit_update(aac_data->decoder_data.codec.out_left_buffer, scratch_buf);
            /* Make sure that we don't create too many samples that the sra will run out
             * of space if it's going to insert samples (It won't insert more than 2).
             * Calc amount of space subtracts 1 so that full doesn't look like empty, we
             * add that back in here as we are setting the buffer size */
            op_space = cbuffer_calc_amount_space_in_addrs(aac_data->decoder_data.op_out_left) + (1 - 2) * sizeof(unsigned);
            if (op_space < AAC_SCRATCH_BUFFER_SIZE)
            {
                aac_data->decoder_data.codec.out_left_buffer->size = op_space;
            }
            else
            {
                aac_data->decoder_data.codec.out_left_buffer->size = AAC_SCRATCH_BUFFER_SIZE;
            }
        }

            if (aac_data->decoder_data.codec.out_right_buffer)
            {
                unsigned op_space;
                void *scratch_buf = scratch_commit(AAC_SCRATCH_BUFFER_SIZE, MALLOC_PREFERENCE_NONE);
                cbuffer_scratch_commit_update(aac_data->decoder_data.codec.out_right_buffer, scratch_buf);
                /* Make sure that we don't create too many samples that the sra will run out
                 * of space if it's going to insert samples. (It won't insert more than 2) */
                op_space = cbuffer_calc_amount_space_in_addrs(aac_data->decoder_data.op_out_right) + (1 - 2) * sizeof(unsigned);
                if (op_space < AAC_SCRATCH_BUFFER_SIZE)
                {
                    aac_data->decoder_data.codec.out_right_buffer->size = op_space;
                }
                else
                {
                    aac_data->decoder_data.codec.out_right_buffer->size = AAC_SCRATCH_BUFFER_SIZE;
                }
            }
        }
#endif /* TIMED_PLAYBACK_MODE */

#ifdef INSTALL_METADATA
        unsigned pre_run_bit_position = aac_data->codec_data.get_bitpos;
        unsigned pre_run_bits_consumed = 16 - pre_run_bit_position;
        /* cbuffer_get_read_offset() returns the offset in words (data in 16-bit unpacked mode),
         * (* 2) to convert to octets */
        unsigned pre_run_read_offset = cbuffer_get_read_offset(aac_data->decoder_data.codec.in_buffer) << 1;
#endif /* INSTALL_METADATA */

        /* Checks for enough data and enough output space are done at the top of
         * this function so it's not done in this C code as well. */
        a2dp_decoder_decode(&(aac_data->decoder_data.codec),
                             aac_data->decoder_data.decode_frame,
                             CODEC_NORMAL_DECODE,
                             aac_data->decoder_data.a2dp_header);

#ifdef INSTALL_METADATA
        unsigned post_run_bits_consumed = 16 - aac_data->codec_data.get_bitpos;
        /* cbuffer_get_read_offset() returns the offset in words (data in 16-bit unpacked mode),
         * (* 2) to convert to octets */
        unsigned post_run_read_offset = cbuffer_get_read_offset(aac_data->decoder_data.codec.in_buffer) << 1;
        /* cbuffer_get_size_in_words() returns the size in words (data in 16-bit unpacked mode),
         * (* 2) to convert the buffer size to the number of octets actually used */
        unsigned real_buff_size = cbuffer_get_size_in_words(aac_data->decoder_data.codec.in_buffer) << 1;
        int amount_consumed;
        tCbuffer *src, *dst;
        unsigned b4idx, afteridx;
        metadata_tag *mtag;
        bool corrupted_frame = FALSE;
        /* first estimate based on offsets (in octets but with a 16-bit unpacked word resolution),
         * check and correct if read offset wrapped */
        amount_consumed = post_run_read_offset - pre_run_read_offset;
        if (amount_consumed < 0)
        {
             amount_consumed = amount_consumed + real_buff_size;
        }
        /* Adjust calculated amount by the number of bits within a 16-bit word consumed
         * pre- and post-processing, byte-align in both cases (the decoder does the same):
         * - for pre-process, the decoder does byte alignment first and starts decoding a new frame
         * - for post-process, we don't consider any partial octets for the amount calculation -
         * the sub-octet amount will be discarded at the start of next frame decode. */
        amount_consumed = amount_consumed - (pre_run_bits_consumed >> 3);
        amount_consumed = amount_consumed + (post_run_bits_consumed >> 3);
#endif /* INSTALL_METADATA */

        output_samples = aac_data->decoder_data.codec.num_output_samples;

#ifdef INSTALL_METADATA
        src = aac_data->decoder_data.codec.in_buffer;
        dst = aac_data->decoder_data.metadata_op_buffer;

        /* Metadata handling logic:
         * - remove the input buffer tags if we've consumed encoded data
         * - add new output buffer tags if we've produced output
         * These changes have been prompted by the decoder's behaviour:
         * - consume input stream and produce output audio (normal decode)
         * - consume less than a frame and produce no output audio (corrupted frame)
         * - consume more than a frame and produce no output audio (corrupted frame)
         * - consume and produce nothing (condition known as 'buffer underflow' - there
         *   is less than a frame worth of data in the input buffer)
         */
        if (aac_data->decoder_data.codec.mode == CODEC_FRAME_CORRUPT)
        {
            corrupted_frame = TRUE;
        }
        else
        {
            mtag = buff_metadata_peek(src);
            if (mtag != NULL)
            {
                if((amount_consumed > 0) && (amount_consumed != mtag->length))
                {
                    corrupted_frame = TRUE;
                }
            }
        }

        if ((amount_consumed > 0) &&(corrupted_frame == FALSE))
        {
            unsigned output_octets;

            mtag = buff_metadata_remove(src, amount_consumed, &b4idx, &afteridx);

            if (BUFF_METADATA(dst))
            {
                /* TODO In the current implementation of timestamp insertion (RTP decode) each packet
                 * will have a tag (with timestamp) so it makes sense to check if this condition
                 * is not true. In the future the decoder should support timestamp insertion
                 * in case if not all packet has tags.*/
                if (mtag != NULL)
                {
                    if ( IS_TIMESTAMPED_TAG(mtag))
                    {
                        PL_ASSERT(b4idx == 0);
                        PL_ASSERT(mtag->next == NULL);
                    }
                }
                else
                {
                    /* produce one tag per frame
                       NB. this is an invented tag, no timestamps, nor other private data,
                       it only serves the purpose of removing at audio EP TODO TTP
                       this is still use for testin. */
                    mtag = buff_metadata_new_tag();
                }

                /* consumed but not produced? that should be frame corrupt. */
                if (output_samples == 0)
                {
                    L2_DBG_MSG("AAC DECODER didn't produce anything! Frame should be corrupt! ");
                }

                /* decoded output will have different frame lengths */
                output_octets = output_samples * OCTETS_PER_SAMPLE;
                if (mtag != NULL)
                {
                    mtag->length = output_octets;
                    METADATA_PACKET_START_SET(mtag);
                    METADATA_PACKET_END_SET(mtag);
                }
                buff_metadata_append(dst, mtag, 0, output_octets);
            }
            else
            {
                buff_metadata_delete_tag(mtag, TRUE);
            }
        }
        else if(corrupted_frame)
        {
            unsigned discard_amt, new_bit_pos;

            discard_amt = buff_metadata_available_octets(src);

            /* This should have been dealt with further up the audio chain (RTP DECODE).
             * If for any reason we've been passed a corrupt frame we discard the whole
             * input buffer and correct the bit position.*/
            L2_DBG_MSG1("AAC DECODER discarding all input buffer = %d octets ",
                    discard_amt);

            /* Get the pre-run bit position.*/
            new_bit_pos = pre_run_bit_position;
            switch(pre_run_bit_position)
            {
            case 16:
                if(discard_amt & 1)
                {
                    new_bit_pos = 8;
                }
                break;
            case 8:
                if(discard_amt & 1)
                {
                    new_bit_pos = 16;
                }
                break;
            case 0:
                if(discard_amt & 1)
                {
                    new_bit_pos = 8;
                }
            }
            /* Set new bit position, */
            aac_data->codec_data.get_bitpos = new_bit_pos;

            /* discard all data in the buffer,*/
            src->read_ptr = src->write_ptr;

            /* and remove all the available tags*/
            mtag = buff_metadata_remove(src, discard_amt, &b4idx, &afteridx);
            buff_metadata_tag_list_delete(mtag);
        }

#endif      /* INSTALL_METADATA */

        if (output_samples > 0)
        {
            unsigned num_chans;
            /* Source 0 is always touched */
            unsigned touched_sources = TOUCHED_SOURCE_0;

            /* Is fadeout enabled? if yes, do it on the new output data */
            if ( (aac_data->decoder_data.left_fadeout.fadeout_state != NOT_RUNNING_STATE))
            {
                /* If it is enabled it is enabled on both channels. Call it on left and
                 * right. They should both fadeout at the same rate so if one says
                 * finished then they both should be and we tell the host. */

                /* A small optimisation here, if there is only a mono output connected
                 * then we only need to call fade out on the left channel, which is
                 * why we tested the left state above. If left is fading out it is
                 * definitely connected so we always fade it out. By fading out the
                 * right channel first (iff it is connected) we don't need to think
                 * about it's response or when to send the fade out done message.
                 */
                if (aac_data->decoder_data.codec.out_right_buffer != NULL)
                {
                    mono_cbuffer_fadeout(aac_data->decoder_data.codec.out_right_buffer,
                                        output_samples, &(aac_data->decoder_data.right_fadeout));
                }
                if (mono_cbuffer_fadeout(aac_data->decoder_data.codec.out_left_buffer,
                                        output_samples, &(aac_data->decoder_data.left_fadeout)))
                {
                    common_send_simple_unsolicited_message(op_data, OPMSG_REPLY_ID_FADEOUT_DONE);
                }
            }

           /* If stereo outputs are connected then source 1 was touched as well */
           if (aac_data->decoder_data.codec.out_right_buffer != NULL)
           {
               touched_sources |= TOUCHED_SOURCE_1;
           }

         /* If stereo outputs are connected then source 1 was touched as well */
           if (aac_data->decoder_data.codec.out_right_buffer != NULL)
           {
               touched_sources |= TOUCHED_SOURCE_1;
               num_chans = 2;
           }
           else
           {
               num_chans = 1;
           }

           /* Source(s) touched */
           touched->sources = touched_sources;

#ifndef TIMED_PLAYBACK_MODE
            /* If SRA is enabled then perform that step. */
            if (NULL != aac_data->decoder_data.sra)
            {
                sra_resample(&aac_data->decoder_data.codec.out_left_buffer, &aac_data->decoder_data.op_out_left, num_chans, output_samples, aac_data->decoder_data.sra);
            }
#else
            NOT_USED(num_chans);
#endif /* TIMED_PLAYBACK_MODE */

        }

    }while(aac_data->decoder_data.codec.mode == CODEC_SUCCESS);

    /* Free the scratch memory used */
    scratch_free();


#ifdef INSTALL_AAC_DATA_TEST
    /* Kick the input to get it going */
    touched->sinks |= TOUCHED_SINK_0;
#else
    if(aac_data->decoder_data.play == NULL)
    {

        /* If the play pointer is not populated it means that we are connected to RTP
         * decode. If there was not enoguh input kick backwards. In case of an error
         * (CODEC_ERROR or CODEC_FRAME_CORRUPT) kick backwards if there is less than
         * the maximum AAC frame size (worst case scenario).
         */
        if ((aac_data->decoder_data.codec.mode == CODEC_NOT_ENOUGH_INPUT_DATA) ||
                ( ((aac_data->decoder_data.codec.mode == CODEC_ERROR) || (aac_data->decoder_data.codec.mode == CODEC_FRAME_CORRUPT)) &&
                  (cbuffer_calc_amount_data_in_words(aac_data->decoder_data.codec.in_buffer) < MAX_AAC_FRAME_SIZE_IN_WORDS)             ) )
        {
            touched->sinks |= TOUCHED_SOURCE_0;
        }
    }
#endif
}
#ifdef INSTALL_OPERATOR_AAC_SHUNT_DECODER

/**
 * \brief Enable or disable the A2DP content protection.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the request message payload
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool aac_dec_opmsg_content_protection_enable(OPERATOR_DATA *op_data,
                    void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    A2DP_HEADER_PARAMS* a2dp_header;

    a2dp_header = (A2DP_HEADER_PARAMS*)(((AAC_DEC_OP_DATA *) \
                  op_data->extra_op_data)->decoder_data.a2dp_header);


    if (a2dp_header == NULL || OP_RUNNING == op_data->state)
    {
        return FALSE;
    }

    /* Set the Content protection bit */
    if(((unsigned*)message_data)[3] != 0 )
    {
        a2dp_header->type = (unsigned) A2DP_STRIP_AAC_CP_HDR;
        a2dp_header->hdr_size =  (unsigned)A2DP_AAC_CP_HDR_SIZE;
    }
    else
    {
        a2dp_header->type = (unsigned)A2DP_STRIP_AAC_HDR;
        a2dp_header->hdr_size = (unsigned) A2DP_AAC_HDR_SIZE;
    }

    return TRUE;
}

#endif /* INSTALL_OPERATOR_AAC_SHUNT_DECODER */
bool aac_dec_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    /* In the case of this capability, nothing is done for control message. Just follow protocol and ignore any content. */
    return cps_control_setup(message_data, resp_length, resp_data,NULL);

}


bool aac_dec_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    return FALSE;
}

bool aac_dec_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{

     return FALSE;
}

bool aac_dec_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    /* Set the parameter(s). For future proofing, it is using the whole mechanism, although currently there is only one field
     * in opdata structure that is a setable parameter. If later there will be more (ever), must follow contiquously the first field,
     * as commented and instructed in the op data definition. Otherwise consider moving them into a dedicated structure.
     */

    return FALSE;
}

bool aac_dec_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    AAC_DEC_OP_DATA * op_extra_data =  (AAC_DEC_OP_DATA*)op_data->extra_op_data;

    unsigned* resp = NULL;



    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(AAC_DECODE_STATISTICS) ,&resp))
    {
         return FALSE;
    }
    // TBD: check for NULL pointers
    if(resp)
    {
            resp = cpsPackWords(&op_extra_data->codec_data.sf_index_field, &op_extra_data->codec_data.channel_configuration_field, resp);
            resp = cpsPackWords(&op_extra_data->codec_data.audio_object_type_field, &op_extra_data->codec_data.extension_audio_object_type_field, resp);
            cpsPackWords(&op_extra_data->codec_data.sbr_present_field, &op_extra_data->codec_data.mp4_frame_count, resp);
    }

    return TRUE;
}

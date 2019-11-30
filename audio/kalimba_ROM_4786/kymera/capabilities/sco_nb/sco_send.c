/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  sco_send.c
 * \ingroup  operators
 *
 *  SCO send operator
 *
 */
/****************************************************************************
Include Files
*/
#include "sco_nb_private.h"
#include "sco_common_funcs.h"

#include "patch/patch.h"
#include "mem_utils/scratch_memory.h"

#ifdef CVSD_CODEC_SOFTWARE
#include "cvsd.h"
//#define GENERATE_SCO_INPUT
//#define LOGOUTPUT
#include "math.h"
#endif


/****************************************************************************
Private Constant Definitions
*/
/** Default buffer sizes for sco send */
#define SCO_SEND_INPUT_BUFFER_SIZE (128)
#define SCO_SEND_OUTPUT_BUFFER_SIZE                  (SCO_DEFAULT_SCO_BUFFER_SIZE)

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define SCO_SEND_CAP_ID CAP_ID_DOWNLOAD_SCO_SEND
#else
#define SCO_SEND_CAP_ID CAP_ID_SCO_SEND
#endif

/****************************************************************************
Private Type Definitions
*/
/** capability-specific extra operator data for SCO SEND */
typedef struct
{
    /** Terminal buffers */
    SCO_TERMINAL_BUFFERS buffers;

    /** Fade-out parameters */
    FADEOUT_PARAMS fadeout_parameters;

    /** Amount of data to process each time */
    unsigned frame_size;		// in uint16 words

#ifdef CVSD_CODEC_SOFTWARE 
	sCvsdState_t cvsd_struct;
	int* ptScratch;				// pointer to scratch memory
#endif

} SCO_SEND_OP_DATA;

/****************************************************************************
Private Constant Declarations
*/
/** The SCO send capability function handler table */
const handler_lookup_struct sco_send_handler_table =
{
    sco_send_create,          /* OPCMD_CREATE */
	sco_send_destroy,          /* OPCMD_DESTROY */
    sco_send_start,           /* OPCMD_START */
    base_op_stop,             /* OPCMD_STOP */
    sco_send_reset,           /* OPCMD_RESET */
    sco_send_connect,         /* OPCMD_CONNECT */
    sco_send_disconnect,      /* OPCMD_DISCONNECT */
    sco_send_buffer_details,  /* OPCMD_BUFFER_DETAILS */
    sco_send_get_data_format, /* OPCMD_DATA_FORMAT */
    sco_send_get_sched_info   /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table */
const opmsg_handler_lookup_table_entry sco_send_opmsg_handler_table[] =
    {{OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,    base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_FADEOUT_ENABLE,             sco_send_opmsg_enable_fadeout},
    {OPMSG_COMMON_ID_FADEOUT_DISABLE,            sco_send_opmsg_disable_fadeout},
    {OPMSG_SCO_SEND_ID_SET_TO_AIR_INFO,          sco_send_opmsg_set_to_air_info},
    {0, NULL}};


const CAPABILITY_DATA sco_send_cap_data =
{
    SCO_SEND_CAP_ID,            /* Capability ID */
    0, 1,                           /* Version information - hi and lo parts */
    1, 1,                           /* Max number of sinks/inputs and sources/outputs */
    &sco_send_handler_table,        /* Pointer to message handler function table */
    sco_send_opmsg_handler_table,   /* Pointer to operator message handler function table */
    sco_send_process_data,          /* Pointer to data processing function */
    0,                              /* TODO - Processing time information */
    sizeof(SCO_SEND_OP_DATA)        /* Size of capability-specific per-instance data */
};

/****************************************************************************
Public Function Definitions
*/

/****************************************************************************
Private Function Declarations
*/
/* ******************************* Helper functions ************************************ */


/* initialise various working data params of the specific operator */
static void sco_send_reset_working_data(OPERATOR_DATA *op_data)
{
    SCO_SEND_OP_DATA* x_data = (SCO_SEND_OP_DATA*)(op_data->extra_op_data);

    if(x_data != NULL)
    {
        /* Initialise fadeout-related parameters */
        x_data->fadeout_parameters.fadeout_state = NOT_RUNNING_STATE;
        x_data->fadeout_parameters.fadeout_counter = 0;
        x_data->fadeout_parameters.fadeout_flush_count = 0;
    }
}

/* ********************************** API functions ************************************* */

bool sco_send_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    /* call base_op create, which also allocates and fills response message */
    if(!base_op_create(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }
    
    /* initialise specific data (was allocated and pointer to it filled by OpMgr  */
    sco_send_reset_working_data(op_data);

#ifdef CVSD_CODEC_SOFTWARE
	if (!scratch_register())
	{
		return TRUE;
	}
	if (!scratch_reserve(SCRATCH_SIZE_WORDS, MALLOC_PREFERENCE_NONE))
	{
		return(FALSE);
	}
#endif /* CVSD_CODEC_SOFTWARE */

    return TRUE;
}

bool sco_send_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
	/* call base_op destroy that creates and fills response message, too */
	if (!base_op_destroy(op_data, message_data, response_id, response_data))
	{
		return(FALSE);
	}

#ifdef CVSD_CODEC_SOFTWARE
	scratch_deregister();
#endif
	return TRUE;
}


bool sco_send_reset(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    if(!base_op_reset(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    /* now initialise specific working data */
    sco_send_reset_working_data(op_data);

    return TRUE;
}


bool sco_send_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    SCO_SEND_OP_DATA* opx_data;
    *response_id = OPCMD_START;
    /* checks whether all terminals are connected, sets operator state to running, builds response message */
    if(!base_op_build_std_response(STATUS_OK, op_data->id, response_data))
    {
        return FALSE;
    }
	
    if(op_data->state==OP_RUNNING)
    {
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

	opx_data = (SCO_SEND_OP_DATA*)(op_data->extra_op_data);
    if (!opx_data->buffers.op_buffer || !opx_data->buffers.ip_buffer)
    {
        /* The operator is not connected yet. Change the already allocated response to
         * command failed. No extra error info.*/
        base_op_change_response_status(response_data, STATUS_CMD_FAILED);
        return TRUE;
    }

    /* TODO flush input */
    /* ... */
    op_data->state = OP_RUNNING;

    return TRUE;
}



bool sco_send_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    if(!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }

    if (((unsigned *)message_data)[0] & TERMINAL_SINK_MASK)
    {
        ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size = SCO_SEND_INPUT_BUFFER_SIZE;
    }
    else
    {
        ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size = SCO_SEND_OUTPUT_BUFFER_SIZE;
    }

    L4_DBG_MSG1("sco_send_buffer_details  %d \n", ((OP_STD_RSP*)*response_data)->resp_data.data );

    return TRUE;
}

bool sco_send_connect(OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data)
{
    return sco_common_connect(op_data, message_data, response_id, response_data,
            &(((SCO_SEND_OP_DATA*)(op_data->extra_op_data))->buffers), NULL);
}

bool sco_send_disconnect(OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data)
{
    return sco_common_disconnect(op_data, message_data, response_id, response_data,
            &(((SCO_SEND_OP_DATA*)(op_data->extra_op_data))->buffers), NULL);
}

bool sco_send_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id,
        void **response_data)
{
#ifdef CVSD_CODEC_SOFTWARE
	return sco_common_get_data_format(op_data, message_data, response_id, response_data,
		AUDIO_DATA_FORMAT_FIXP, AUDIO_DATA_FORMAT_16_BIT);
#else
    return sco_common_get_data_format(op_data, message_data, response_id, response_data,
            AUDIO_DATA_FORMAT_FIXP, AUDIO_DATA_FORMAT_FIXP);
#endif
}

bool sco_send_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id,
        void **response_data)
{
    return sco_common_get_sched_info(op_data, message_data, response_id, response_data, 0, 0);
}

/* ************************************* Data processing-related functions and wrappers **********************************/

void sco_send_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    int available_output, available_input, samples_in, samples_out;
    SCO_SEND_OP_DATA* x_data = (SCO_SEND_OP_DATA*)(op_data->extra_op_data);

    patch_fn(sco_send_process_data);
  
    samples_out = x_data->frame_size;
    #ifdef CVSD_CODEC_SOFTWARE 
    samples_in=(x_data->frame_size)<<1;    // one audio sample for each compressed byte
    #else
        samples_in=x_data->frame_size;
    #endif
        
    #ifdef GENERATE_SCO_INPUT
	//cbuffer_advance_read_ptr(x_data->buffers.ip_buffer, -samples_in);
    //cbuffer_advance_write_ptr(x_data->buffers.ip_buffer, -samples_in);

	static int samplecount = 0;
	for (int i = 0; i<samples_in; i++) {
		int data = 0x40000000 * sin(6.28f*100 / 8000 * samplecount);
		samplecount = samplecount + 1;
		cbuffer_write(x_data->buffers.ip_buffer, &data, 1);
	}    
    #endif

    /* work out amount of input data to process, based on output space and input data amount */
    available_output = cbuffer_calc_amount_space_in_words(x_data->buffers.op_buffer);
	available_input = cbuffer_calc_amount_data_in_words(x_data->buffers.ip_buffer);

	L3_DBG_MSG3("sco_send_process_data: Samples to process: %d, available space %d, frame size: %d\n", available_input, available_output, samples_out);

    if (available_output < samples_out)
    {
        return;
    }
	if (available_input < samples_in)
	{
		return;
	}
	else if (available_input - samples_in < samples_in)
	{
		/* If there isn't enough data to process a frame next time kick backwards */
		touched->sinks = TOUCHED_SINK_0;
	}
    
    /* Is fadeout enabled? if yes, do it on the current input data */
    if(x_data->fadeout_parameters.fadeout_state != NOT_RUNNING_STATE)
    {
        /* the wrapper below takes operator data - this might be lifted out to a common function */
		if (mono_cbuffer_fadeout(x_data->buffers.ip_buffer, samples_in,
                                 &x_data->fadeout_parameters))
        {
            common_send_simple_unsolicited_message(op_data, OPMSG_REPLY_ID_FADEOUT_DONE);
        }
    }

#ifdef CVSD_CODEC_SOFTWARE
	x_data->ptScratch = (int*)scratch_commit(SCRATCH_SIZE_WORDS, MALLOC_PREFERENCE_NONE);
	cvsd_send_asm(&x_data->cvsd_struct, x_data->buffers.ip_buffer, x_data->buffers.op_buffer, x_data->ptScratch, samples_in);
	
#ifdef LOGOUTPUT
	for (int i = 0; i < samples_in * 8; i++) {
		L3_DBG_MSG2("%d %x\n", i, x_data->ptScratch[i]);
	}
#endif	
	
	
	scratch_free();
#else
	/* Copy a frame's worth of data to the output buffer */
	cbuffer_copy(x_data->buffers.op_buffer, x_data->buffers.ip_buffer, x_data->frame_size);
#endif

    available_input = cbuffer_calc_amount_data_in_words(x_data->buffers.ip_buffer);

	L3_DBG_MSG1("sco_send_process_data POST: Samples left to process: %d\n", available_input);

    touched->sources =  TOUCHED_SOURCE_0;
}



/* **************************** Operator message handlers ******************************** */


bool sco_send_opmsg_enable_fadeout(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    common_set_fadeout_state(&(((SCO_SEND_OP_DATA*)(op_data->extra_op_data))->fadeout_parameters), RUNNING_STATE);

    return TRUE;
}


bool sco_send_opmsg_disable_fadeout(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    common_set_fadeout_state(&(((SCO_SEND_OP_DATA*)(op_data->extra_op_data))->fadeout_parameters), NOT_RUNNING_STATE);

    return TRUE;
}

bool sco_send_opmsg_set_to_air_info(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    SCO_SEND_OP_DATA* x_data = (SCO_SEND_OP_DATA*)(op_data->extra_op_data);

    x_data->frame_size = ((unsigned*)message_data)[3];

    return TRUE;
}



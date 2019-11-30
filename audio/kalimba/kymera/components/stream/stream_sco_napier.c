/****************************************************************************
 * Copyright (c) 2016 - 2019 Qualcomm Technologies International, Ltd 
****************************************************************************/
/**
 * \file  stream_sco_napier.c
 * \ingroup stream
 *
 * stream sco type file. <br>
 * This file contains stream functions for napier sco endpoints. <br>
 *
 * \section sec1 Contains:
 * stream_sco_create <br>
 * stream_sco_get_endpoint <br>
 * stream_sco_destroy <br>
 */

/****************************************************************************/
/* Include Files                                                            */
/****************************************************************************/
#include "stream_private.h"
#include "sco_mgr_napier.h"

/* #define DEBUG_STREAM_SCO_NAPIER_SOURCE */
/* #define DEBUG_STREAM_SCO_NAPIER_SINK */
#if defined(DEBUG_STREAM_SCO_NAPIER_SOURCE) || defined(DEBUG_STREAM_SCO_NAPIER_SINK)
#include "stdio.h"
#include "stdlib.h"
#endif

/* metadata Sync word */
#define SYNC_WORD (0x5c5c)
/* Byte swap */
#define WBS_METADATA_TRANSFORM(x)    ((x<<8) & 0xFF00) | ((x>>8) & 0x00FF)

/****************************************************************************/
/* Private Type Declarations                                                */
/****************************************************************************/

/****************************************************************************/
/* Private Constant Declarations                                            */
/****************************************************************************/
#define MAX_NUM_RX_BUFFERS  3
#define MAX_NUM_TX_BUFFERS  2
#define SCO_BUFFER_SIZE_NAPIER 256 /* buffer comprises 256 words */
/****************************************************************************/
/* Private Macro Declarations                                               */
/****************************************************************************/
static inline int min(int x,int y){ return ((x<y) ? x : y);}

/****************************************************************************/
/* Private Variable Definitions                                             */
/****************************************************************************/

/****************************************************************************/
/* Private Function Declarations                                            */
/****************************************************************************/
static bool sco_close(ENDPOINT *endpoint);
static bool sco_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick);
static bool sco_disconnect(ENDPOINT *endpoint);
static void sco_kick(ENDPOINT *endpoint, ENDPOINT_KICK_DIRECTION kick_dir);
static void sco_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info);
static bool sco_buffer_details(ENDPOINT *endpoint, BUFFER_DETAILS *details);
static bool sco_configure_napier(ENDPOINT *endpoint, unsigned int key, uint32 value);
static bool sco_get_config_napier(ENDPOINT *endpoint, unsigned int key, uint32 *value);
static sco_metadata_t get_metadata(int* buffer_ptr);
static sco_majority_vote_t sco_ErrDataProcess_MajorityVoting(sco_metadata_t* sco_metadata, int32 buf_num, tCbuffer** rx_buf_ptr);
static unsigned calc_HammingWeight(int num_Bytes, int* rx_buf_ptr0, int* rx_buf_ptr1);
static unsigned calc_QualityMetric_3RxBuffers(int num_Bytes, int* rx_buf_ptr0, int* rx_buf_ptr1,  int* rx_buf_ptr2);

DEFINE_ENDPOINT_FUNCTIONS(sco_functions, sco_close, sco_connect,
                          sco_disconnect, sco_buffer_details,
                          sco_kick, sco_sched_kick, sco_start,
                          sco_stop, sco_configure_napier, sco_get_config_napier,
                          sco_get_timing, stream_sync_sids_dummy);



/****************************************************************************/
/* PROCESSING                                                               */
/****************************************************************************/

/****************************************************************************/
/* Public Function Definitions                                              */
/****************************************************************************/
/*--------------------------------------------------------------------------*
 * stream_sco_create                                                        *
 *--------------------------------------------------------------------------*/
bool sco_conn_create(unsigned sco_handle, sco_conn_params_t* sco_params)

{
    /* copy those parameters to end format */
    unsigned    tx_buf_size = sco_params->p_stat.tx_buf_size;
    unsigned    tx_buf_num  = sco_params->p_stat.tx_buf_num;
    unsigned    rx_buf_size = sco_params->p_stat.rx_buf_size;
    unsigned    rx_buf_num  = sco_params->p_stat.rx_buf_num;

    patch_fn_shared(stream_sco);

    uintptr_t   addr_u32;

/*  unsigned    key = sco_handle;       // The hci handle is the key (unique for the type and direction) */
    ENDPOINT    *source_ep = NULL;
    ENDPOINT    *sink_ep = NULL;

    int         k;
    
    PL_PRINT_P1(TR_PL_TEST_TRACE, "\t FUNCTION: bool stream_sco_create(sco_handle=0x%04x) \n", sco_handle);

    /*-------------------*
     * SOURCE_ENDPOINT CHECK: Check that we don't already have an existing source endpoint for the specified hci handle
     *-------------------*/
    if (stream_get_endpoint_from_key_and_functions(sco_handle, SOURCE, &endpoint_sco_functions) != NULL)
    {   /* There is alread an exisitng endpoint: caller should not have called us for a second time without deleting the existing endpoint first */
        goto handle_error;
    }
    
    /*-------------------*
     * SOURCE_ENDPOINT CREATE: Create and initialise a source endpoint
     *-------------------*/
    source_ep = STREAM_NEW_ENDPOINT(sco, sco_handle, SOURCE, INVALID_CON_ID);
    if (source_ep == NULL)
    {
        goto handle_error;
    }
    /* source ep created */
    source_ep->can_be_closed = FALSE;
    source_ep->can_be_destroyed = FALSE;
    source_ep->is_real = TRUE; /* SCO endpoints are always at the end of a chain */
    
    /*-------------------*
     * RX BUFFERS linked to SOURCE_EP: setup tCbuffer struct 
     *-------------------*/
    source_ep->state.sco.source_buf_num = min(rx_buf_num, MAX_NUM_RX_BUFFERS);  /* up to 3 RX buffers (from-air) */
    source_ep->state.sco.sink_buf_num = 1;                                      /* 1 buffer towards DSP operators */
    addr_u32 = (sco_params->p_stat.rx_buf_ptr);    /* rx_buf_ptr */
    if (rx_buf_size%(1<<LOG2_ADDR_PER_WORD) != 0)  /* request buffer size in Bytes to be integer multiple of buffer size in words */
    {
        goto handle_error; /* buffer size in Bytes not integer multiple of buffer size in words */
    }
    for (k=0; k<source_ep->state.sco.source_buf_num; k++)
    {
        /* given buffer size is in [Bytes] */
        /* allocate memory for tCbuffer struct and initialize pointer and size */
        /*
         * TODO: Is this buffer really a BUF_DESC_IS_REMOTE_MMU_MASK? It is not in a hydra sense, but this descriptor matches
         * the corresponding cbuffer_destroy functionality
         *
         * Note:
         * Given rx_buf_size is in [BYTES]
         * Buffer size function parameter of 'cbuffer_create' function is in words
         * Buffer size stored in tCbuffer struct is in [BYTES]
         * Actually, the size parameter in the tCbuffer struct depends on descriptor (SW_BUFFER: size in 32bit words, MMU_BUFFER size in 16bit words)
         */ 
        source_ep->state.sco.source_buf[k] = cbuffer_create((void*) addr_u32, (rx_buf_size>>LOG2_ADDR_PER_WORD), BUF_DESC_IS_REMOTE_MMU_MASK); /* considers size in words */
        addr_u32 += rx_buf_size;
        if (source_ep->state.sco.source_buf[k] == NULL)
        {
            goto handle_error; /* something went wrong */
        }
        /* source_ep->state.sco.source_buf[k]->write_ptr = NULL; *//* DSP does not use write pointer of the RX buffer */
    }

    /*-------------------*
     * SINK_ENDPOINT CHECK: Check that we don't already have an existing source endpoint for the specified hci handle
     *-------------------*/
    if (stream_get_endpoint_from_key_and_functions(sco_handle, SINK, &endpoint_sco_functions) != NULL)
    {   /* There is alread an exisitng endpoint: caller should not have called us for a second time without deleting the existing endpoint first */
        goto handle_error;
    }

    /*-------------------*
     * SINK_ENDPOINT CREATE: Create and initialise a source endpoint
     *-------------------*/
    sink_ep = STREAM_NEW_ENDPOINT(sco, sco_handle, SINK, INVALID_CON_ID);
    if (sink_ep == NULL)
    {
        goto handle_error;
    }
    /* sink ep created */
    sink_ep->can_be_closed = FALSE;
    sink_ep->can_be_destroyed = FALSE;
    sink_ep->is_real = TRUE;        /* SCO endpoints are always at the end of a chain */

    /*-------------------*
     * TX BUFFERS linked to SINK_EP: setup tCbuffer struct 
     *-------------------*/
    sink_ep->state.sco.sink_buf_num = min(tx_buf_num, MAX_NUM_TX_BUFFERS);  /* up to 2 TX buffers (to-air) */
    sink_ep->state.sco.source_buf_num = 1;                                  /* 1 buffer from DSP operators */
    addr_u32 = (sco_params->p_stat.tx_buf_ptr);    /* tx_buf_ptr */
    if (tx_buf_size%(1<<LOG2_ADDR_PER_WORD) != 0)  /* request buffer size in Bytes to be integer multiple of buffer size in words */
    {
        goto handle_error; /* buffer size in Bytes not integer multiple of buffer size in words */
    }
    for (k=0; k<sink_ep->state.sco.sink_buf_num; k++)
    {   /* given buffer size is in [Bytes] */
        /*
         * TODO: Is this buffer really a BUF_DESC_IS_REMOTE_MMU_MASK? It is not in a hydra sense, but this descriptor matches
         * the corresponding cbuffer_destroy functionality
         *
         *
         * Note:
         * Given tx_buf_size is in [BYTES]
         * Buffer size function parameter of 'cbuffer_create' function is in words
         * Buffer size stored in tCbuffer struct is in [BYTES]
         * Actually, the size parameter in the tCbuffer struct depends on descriptor (SW_BUFFER: size in 32bit words, MMU_BUFFER size in 16bit words)
         */ 
        sink_ep->state.sco.sink_buf[k] = cbuffer_create((void*) addr_u32, (tx_buf_size>>LOG2_ADDR_PER_WORD), BUF_DESC_IS_REMOTE_MMU_MASK); /* considers size in words */
        addr_u32 += tx_buf_size;
        if (sink_ep->state.sco.sink_buf[k] == NULL)
        {
            goto handle_error; /* something went wrong */
        }
        /* sink_ep->state.sco.sink_buf[k]->read_ptr = NULL; *//* DSP does not use read pointer of the TX buffer */
    }

    /*-------------------*
     * CRREATE SCO LIST ENTRY with linked timing record
     *-------------------*/
    if (!sco_conn_list_add_entry(sco_handle, sco_params)) /* sco_parameters that will be included in the sco list entry for the given sco_handle */
    {   /* an error occured */
        goto handle_error; /* something went wrong */
    }

    /* Initialize measured rate to 1.0*/
    sink_ep->state.sco.rate_measurement = 1 << STREAM_RATEMATCHING_FIX_POINT_SHIFT;
    source_ep->state.sco.rate_measurement = 1 << STREAM_RATEMATCHING_FIX_POINT_SHIFT;

    /* Initialize majority vote */
    source_ep->state.sco.majority_vote_bypass = FALSE;
    source_ep->state.sco.majority_vote_questionable_bits_max = 0;

    /* Result: stream_sco_create OK */
    return TRUE;



/*-------------------*/
handle_error:
/*-------------------*/
    /* Cleanup source endpoint and cbuffer if they exist */
    if (source_ep != NULL)
    {
        for (k=0; k<rx_buf_num; k++)
        {
            if (source_ep->state.sco.source_buf[k] != NULL)
            {
                cbuffer_destroy(source_ep->state.sco.source_buf[k]); /* Free up the buffer and associated data space */
            }
        }
        source_ep->can_be_destroyed = TRUE;
        stream_destroy_endpoint(source_ep);
    }

    /* Cleanup sink endpoint and cbuffer if they exist */
    if (sink_ep != NULL)
    {
        for (k=0; k<tx_buf_num; k++)
        {
            if (source_ep->state.sco.sink_buf[k] != NULL)
            {
                cbuffer_destroy(source_ep->state.sco.sink_buf[k]); /* Free up the buffer and associated data space */
            }
        }
        sink_ep->can_be_destroyed = TRUE;
        stream_destroy_endpoint(sink_ep);
    }

    /* Result: stream_sco_create failed */
    PL_PRINT_P0(TR_PL_TEST_TRACE, "\t stream_sco_create failed!!! \n");
    return FALSE;
}


/*--------------------------------------------------------------------------*
 * stream_sco_remove                                                    *
 *--------------------------------------------------------------------------*/
/*
 * sco_params[0] = sco_handle
 */
void sco_conn_remove(unsigned sco_handle)
{

/*  unsigned    key = sco_handle;       // The hci handle is the key (unique for the type and direction) */
    ENDPOINT *ep;

    patch_fn_shared(stream_sco);

    PL_PRINT_P1(TR_PL_TEST_TRACE, "\t FUNCTION: bool stream_sco_destroy(sco_handle=0x%04x) \n", sco_handle);

    /*-------------------*
     * SOURCE_ENDPOINT: Get and close the source endpoint associated with the hci handle
     *-------------------*/
    if ((ep = stream_get_endpoint_from_key_and_functions(sco_handle, SOURCE, &endpoint_sco_functions)) != NULL)
    {   /* exisiting SOURCE endpoint to be destroyed */
        /* clear tCbuffer structs and endpoint for RX/SOURCE */
        ep->can_be_closed = TRUE;
        ep->can_be_destroyed = TRUE;
        stream_close_endpoint(ep);
    }

    /*-------------------*
     * SINK_ENDPOINT: Get and close the sink endpoint associated with the hci handle
     *-------------------*/
    if ((ep = stream_get_endpoint_from_key_and_functions(sco_handle, SINK, &endpoint_sco_functions)) != NULL)
    {   /* exisiting SINK endpoint to be destroyed */
        /* clear tCbuffer structs for TX/SINK */
        ep->can_be_closed = TRUE;
        ep->can_be_destroyed = TRUE;
        stream_close_endpoint(ep);
    }

    /*-------------------*
     * DESTROY SCO LIST ENTRY with linked timing record
     *-------------------*/
    /* Let the sco_mgr_napier know that this endpoint is going away to remove entry in sco connection list */
    PL_PRINT_P0(TR_PL_TEST_TRACE, "\t delete dco list entry... \n");
    if (!sco_conn_list_remove_entry(sco_handle))
    {
        PL_PRINT_P0(TR_PL_TEST_TRACE, "\t ... delete sco list entry failed \n");
    }
    else
    {
        PL_PRINT_P0(TR_PL_TEST_TRACE, "\t ... delete sco list entry done \n");
    }
    PL_PRINT_P0(TR_PL_TEST_TRACE, "\t DONE: stream_sco_destroy(..) \n");
    return;
}


/*--------------------------------------------------------------------------*
 * stream_sco_get_endpoint                                                  *
 *--------------------------------------------------------------------------*/
/*
 *   con_id:     connection ID of the originator of this request
 *   dir:        whether a source or sink is requested
 *   num_params: number of parameters provided
 *   params[0]:  sco_handle
 */
ENDPOINT* stream_sco_get_endpoint(unsigned con_id, ENDPOINT_DIRECTION dir, unsigned num_params, unsigned* params)
{
    ENDPOINT *ep;
    unsigned sco_handle;

    patch_fn_shared(stream_sco);

    PL_PRINT_P0(TR_PL_TEST_TRACE, "\t FUNCTION: ENDPOINT* stream_sco_get_endpoint(..)\n");
    
    /* Expect reception of 2 parameters:
     * params[0]: INSTANCE; INSTANCE = sco_handle as only parameter in Napier on initial ACCMD_GET_SOURCE/SINK_REQ
     * params[1]: CHANNEL;  Note: CHANNEL parameter need not to be considered in case of SCO 
     */
    if (num_params != 2)
    {
        PL_PRINT_P0(TR_PL_TEST_TRACE, "\t ERROR in FUNCTION: ENDPOINT* stream_sco_get_endpoint(..): (num_params != 1) --> return NULL \n");
        return NULL;
    }
    sco_handle = params[0];

    /*-------------------*
     * if ep already exists, get pointer to requested endpoint ID, otherwise NULL
     *-------------------*/
    ep = stream_get_endpoint_from_key_and_functions(sco_handle, dir, &endpoint_sco_functions);
    if (ep)
    {
        /*-------------------*
         * Endpoint is available -> check ID
         *-------------------*/
        if (ep->con_id == INVALID_CON_ID)
        {
            ep->con_id = con_id;
        }
        /* If the client does not own the endpoint they can't access it */
        else if (ep->con_id != con_id)
        {
            PL_PRINT_P0(TR_PL_TEST_TRACE, "\t ERROR in FUNCTION: ENDPOINT* stream_sco_get_endpoint(..): (ep->con_id != con_id) --> return NULL \n");
            return NULL;
        }
    }
    PL_PRINT_P0(TR_PL_TEST_TRACE, "\t DONE: ENDPOINT* stream_sco_get_endpoint(..) \n");
    return ep;
}




/****************************************************************************/
/* Private Function Definitions                                             */
/****************************************************************************/

/*--------------------------------------------------------------------------*
 * sco_close                                                                *
 *--------------------------------------------------------------------------*/
static bool sco_close(ENDPOINT *endpoint)
{
    patch_fn_shared(stream_sco);

    int k;

    /* When reaching this function we will have stopped everything from running, so all we need to do is tidy up the buffer */
    if (endpoint->direction == SOURCE)
    {   /* SOURCE */
        for (k=0; k<endpoint->state.sco.source_buf_num; k++)
        {
            cbuffer_destroy(endpoint->state.sco.source_buf[k]);
        }
    }
    else
    {   /* SINK */
        for (k=0; k<endpoint->state.sco.sink_buf_num; k++)
        {
            cbuffer_destroy(endpoint->state.sco.sink_buf[k]);
        }
    }
    return TRUE;
}


/*--------------------------------------------------------------------------*
 * sco_connect                                                              *
 *--------------------------------------------------------------------------*/
static bool sco_connect(ENDPOINT *endpoint, tCbuffer *Cbuffer_ptr, ENDPOINT *ep_to_kick)
{   /* this function concerns the buffers btw. EP and operators */

    patch_fn_shared(stream_sco);

    endpoint->ep_to_kick = ep_to_kick;
    if (endpoint->direction == SOURCE)
    {
        /* Remember this buffer so that the endpoint can write into it
         * when it gets kicked */
        endpoint->state.sco.sink_buf[0] = Cbuffer_ptr; /* there is only a single buffer btw. SCO_EP and operators to connect */
        /* Make a record of the SCO source endpoint in the SCO connection list structure for first kick scheduling calculations */
        sco_from_air_endpoint_set(stream_sco_get_hci_handle(endpoint), endpoint);
    }
    else
    {
        /* Remember this buffer so that the endpoint can read from it
         * when it gets kicked */
        endpoint->state.sco.source_buf[0] = Cbuffer_ptr; /* there is only a single buffer btw. SCO_EP and operators to connect */
        /* Make a record of the SCO sink endpoint in the SCO connection list structure for first kick scheduling calculations */
        sco_to_air_endpoint_set(stream_sco_get_hci_handle(endpoint), endpoint);
    }
    return TRUE;
}


/*--------------------------------------------------------------------------*
 * sco_buffer_details                                                       *
 *--------------------------------------------------------------------------*/
static bool sco_buffer_details(ENDPOINT *endpoint, BUFFER_DETAILS *details)
{   /* this function is related to SCO_ep_BUFFER towards OPERATOR */

    patch_fn_shared(stream_sco);

    /* this buffer is SW buffers */
    if (endpoint == NULL || details == NULL)
    {
        return FALSE;
    }
 
    /* when using different in and output buffers in endpoint endpoint set the buffer size of the buffer towards operators to fix size. */
    details->b.buff_params.size = SCO_BUFFER_SIZE_NAPIER;
    details->b.buff_params.flags = BUF_DESC_SW_BUFFER; /* for SW buffers the size is in words */

    /* when buffer is already allocated prior connect and already available (supplies_buffer=TRUE); when buffer is allocated while connect (supplies_buffer=FALSE) */
    details->supplies_buffer = FALSE;
    details->runs_in_place = FALSE;

    return TRUE;
}


/*--------------------------------------------------------------------------*
 * sco_set_data_format                                                      *
 *--------------------------------------------------------------------------*/
bool sco_set_data_format (ENDPOINT *endpoint, AUDIO_DATA_FORMAT format)
{   /* data format of buffers bzw. EP and Operators */

    patch_fn_shared(stream_sco);

    /* NOTE: below formats do not really match with our format type -> rather define own format types? */

    /* The data format can only be set before connect */
    if (NULL != endpoint->connected_to)
    {
        return FALSE;
    }

    /* Sources and sinks have different data formats due to metadata */
    if(stream_direction_from_endpoint(endpoint) == SOURCE)
    {
        /*-------------------*
         * SOURCE
         *-------------------*/
        /* variable 'format' carries the DATA_FORMAT of the input buffer terminal of the connected operator sink --> check if this format is supported by endpoint */
        switch(format)
        {   /* this source ep could be connected to operators, which have an input buffer terminals of the following DATA_FORMAT_TYPES: */
            case AUDIO_DATA_FORMAT_16_BIT_WITH_METADATA: 
                 /* SCO_NB: CVSD_DEC operator input terminal */
                 cbuffer_set_write_shift(endpoint->state.sco.sink_buf[0], 0);
                 cbuffer_set_write_byte_swap(endpoint->state.sco.sink_buf[0], FALSE);
                 endpoint->state.sco.audio_data_format = AUDIO_DATA_FORMAT_16_BIT_WITH_METADATA;
                 return TRUE;

            case AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP_WITH_METADATA:
                 /* SCO_WBS: WBS_DEC operator input terminal */
                 cbuffer_set_write_shift(endpoint->state.sco.sink_buf[0], 0);
                 cbuffer_set_write_byte_swap(endpoint->state.sco.sink_buf[0], TRUE);
                 endpoint->state.sco.audio_data_format = AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP_WITH_METADATA;
                 return TRUE;
 
            default: /* DATA FORMAT is not supported by endpoint */
                 return FALSE;
        }
    }
    else
    {
        /*-------------------*
         * SINK
         *-------------------*/
        switch(format)
        {   /* this sink ep could be connected to operators, which have an output buffer terminals of the following DATA_FORMAT_TYPES: */
            /* this the input format, feed by CVSD_ENCODER. 
             * Despite of that term, CVSD delivers pure data and not metadata counter.
             * Metadata counter is added in SCO_SINK_EP
             */
            case AUDIO_DATA_FORMAT_16_BIT:
                 /* SCO_NB: CVSD_ENC operator output terminal */
                 cbuffer_set_read_shift(endpoint->state.sco.source_buf[0], 0);
                 cbuffer_set_write_byte_swap(endpoint->state.sco.source_buf[0], FALSE);
                 endpoint->state.sco.audio_data_format = AUDIO_DATA_FORMAT_16_BIT;
                 return TRUE;

            case AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP:
                 /* SCO_WBS: WBS_ENC operator output terminal */
                 cbuffer_set_read_shift(endpoint->state.sco.source_buf[0], 0);
                 cbuffer_set_write_byte_swap(endpoint->state.sco.source_buf[0], TRUE);
                 endpoint->state.sco.audio_data_format = AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP;
                 return TRUE;

        default:
                 return FALSE;
        }
    }
}


/*--------------------------------------------------------------------------*
 * sco_get_data_format                                                      *
 *--------------------------------------------------------------------------*/
AUDIO_DATA_FORMAT sco_get_data_format (ENDPOINT *endpoint)
{

     patch_fn_shared(stream_sco);

#if 0
    /* 'cbuffer_get_write_byte_swap(..)', 'cbuffer_get_read_byte_swap(..)' not available
     *
     * Note:
     * In BC, Hydra, audio format is distinguished using the byte swap or shift indication
     * In Napier: AUDIO_DATA_FORMAT_16_BIT_WITH_METADATA and AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP_WITH_METADATA
     * could only be distinguished using the byte swap indication, because the shift indication is the same.
     * However, the cbuffer functions do currently not provide a read function for the byte swap indication.
     * That's why in napier, the audio data format is directly written to the endpoint sco struct
     */
    if(stream_direction_from_endpoint(endpoint) == SOURCE)
    {
         if (cbuffer_get_write_byte_swap(endpoint->state.sco.sink_buf[0]) == FALSE)
         {   /* SCO_NB: CVSD_DEC operator input terminal */
             return AUDIO_DATA_FORMAT_16_BIT_WITH_METADATA;
         }
         /* otherwise: */
         /* SCO_WBS: WBS_DEC operator input terminal */
         return AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP_WITH_METADATA;
    }
    else
    {
         if (cbuffer_get_read_byte_swap(endpoint->state.sco.source_buf[0]) == FALSE)
         {   /* SCO_NB: CVSD_ENC operator output terminal */
             return AUDIO_DATA_FORMAT_16_BIT;
         }
         /* otherwise: */
         /* SCO_WBS: WBS_ENC operator output terminal */
         return AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP;
    }
#endif

    return (endpoint->state.sco.audio_data_format);


}


/*--------------------------------------------------------------------------*
 * sco_get_timing                                                           *
 *--------------------------------------------------------------------------*/
void sco_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info)
{
    patch_fn_shared(stream_sco);

    sco_common_get_timing(endpoint, time_info);
    
    if (endpoint->direction == SOURCE)
    {
        time_info->wants_kicks = FALSE;
    }
    else
    {
        time_info->wants_kicks = TRUE;
    }
}


/*--------------------------------------------------------------------------*
 * sco_disconnect                                                           *
 *--------------------------------------------------------------------------*/
static bool sco_disconnect(ENDPOINT *endpoint)
{   /* this function concerns the buffers btw. EP and operators */

    patch_fn_shared(stream_sco);

    int k;
    /* TODO: do we need to stop any scheduling at this time? */
    if (endpoint->direction == SOURCE)
    {   /* SOURCE */
        for (k=0; k<endpoint->state.sco.sink_buf_num; k++)
        {
            endpoint->state.sco.sink_buf[k] = NULL;
        }
    }
    else
    {   /* SINK */
        for (k=0; k<endpoint->state.sco.source_buf_num; k++)
        {
            endpoint->state.sco.source_buf[k] = NULL;
        }
    }
    return TRUE;
}


/*--------------------------------------------------------------------------*
 * sco_kick                                                                 *
 *--------------------------------------------------------------------------*/
static void sco_kick(ENDPOINT *endpoint, ENDPOINT_KICK_DIRECTION kick_dir)
{
    int             k;
    unsigned        ScoPacketSize_Bytes;

    patch_fn_shared(stream_sco);

    /* if it is a source then we need to kick the next endpoint in the chain */
    /* if it is a sink we are the end of the chain and there is nothing more to do */
    if (endpoint->direction == SOURCE)
    {
        unsigned                      words16bit_deliver2operator=0;
        sco_metadata_t                metadata_rx[MAX_NUM_RX_BUFFERS]; /* RX BUF_1..3 */
        unsigned                      tesco_bt_ticks = (sco_tesco_get(stream_sco_get_hci_handle(endpoint))<<1)/US_PER_SLOT;
        int                           word16=0;
        int                           shift=0;
        sco_majority_vote_t           bufVote;

        /**********************************/
        /*    SCO EP SOURCE PROCESSING    */
        /**********************************/
        /*
         * NOTE:
         *       We find the metadata always on the same address of the RX buffer
         *       Except from the CRC indication, we expect same metadata in all Rx buffers 
         */
        for (k=0; k<endpoint->state.sco.source_buf_num; k++)
        {
             metadata_rx[k] = get_metadata(endpoint->state.sco.source_buf[k]->read_ptr); /* RX BUF_1..3 */
        }

        /*
         *************************************************************
         * Select buffer Idx or enable majority voting if applicable...
         * 
         * STATUS as delivered by M0 via in-band metadata:
         *           RX_BUFFER_STATUS_NONE    = 0: No AC detect or Packet Header Checksum Failure
         *           RX_BUFFER_STATUS_VALID   = 1: Valid SCO packet or eSCO with good CRC
         *           RX_BUFFER_STATUS_BAD_CRC = 2: eSCO packet with CRC failure
         *************************************************************
         * If status of selected buffer is RX_BUFFER_STATUS_NONE, we don't have new SCO PACKET -> apply PLC: handled in subsequent SCO_RCV resp. WBS_DEC
         * metadata.counter and metadata.status are used as they are...
         */
        if (endpoint->state.sco.majority_vote_bypass == TRUE)
        {   /* MAJORITY VOTING BYPASSED */
            bufVote.isVoting = FALSE;         /* voting will not be applied */
            bufVote.bufIdx = 0;               /* RX_buf[0] selected */
            bufVote.quality_metric = 0;       /* number of questionable bits per sco packet */
        }
        else
        {   /* MAJORITY VOTING ENABLED */
            bufVote = sco_ErrDataProcess_MajorityVoting(metadata_rx, endpoint->state.sco.source_buf_num, endpoint->state.sco.source_buf);
        }
        /*-------------------------- backup metadata_len and payload_size --------------------*/
        /* metadata counter is updated in each case (all buffers have same counter value) */
        /* The subsequent 2 backups would be obsoete in case if (except of counter value and status), data in shared memory are not (over-)written */
        /* despite of that, we assume that metadata_len and payload_size could have changed in RX buffers despite of status = NONE */
        if (metadata_rx[bufVote.bufIdx].status == RX_BUFFER_STATUS_NONE)
        {   /* use last useable metadata.metadata_len and payload instead */
            metadata_rx[bufVote.bufIdx].metadata_len = endpoint->state.sco.metadata_backup_len;
            metadata_rx[bufVote.bufIdx].payload_size = endpoint->state.sco.metadata_backup_payload_size;
        }
        else
        {   /* RX_BUFFER_STATUS_VALID or RX_BUFFER_STATUS_BAD_CRC */
            endpoint->state.sco.metadata_backup_len = metadata_rx[bufVote.bufIdx].metadata_len; /* store last useable metadata_len */
            endpoint->state.sco.metadata_backup_payload_size = metadata_rx[bufVote.bufIdx].payload_size; /* store last useable payload_size */
        }
        /* in absence of any previous valid metadata, there's nothing to do... */
        if (metadata_rx[bufVote.bufIdx].metadata_len == 0)
        {
            return;
        }
        /*-------------------------- backup metadata_len and payload_size --------------------*/

        /*
         ***********************************************************************
         * check whether the delivery fits in the associated buffers in [Bytes]
         * all buffers have same metadat_len, payload-size and buffer size value, we select RxBuf[0])
         ***********************************************************************
         */
        ScoPacketSize_Bytes = (metadata_rx[bufVote.bufIdx].metadata_len + metadata_rx[bufVote.bufIdx].payload_size); /* SCO_SOURCE: packet_size = metadata_size + payload_size   */
        if (  ((ScoPacketSize_Bytes + 2) > endpoint->state.sco.sink_buf[0]->size)                                    /* remind: We have to add 2 Bytes due to SyncWord to output */
           || ( ScoPacketSize_Bytes      > endpoint->state.sco.source_buf[bufVote.bufIdx]->size))
        {  /* ERROR: SCO packet delivery does not fit in associated buffers */
           return;
        }


        /*****************************
         * NOTE:
         * The data buffer interface towards the capabilities considers 32bit words (SCO_SOURCE_EP_SINK_BUFFER)
         * Keep in mind, that only 16 bits are transferred per 32-bit word.
         * In contrast to that, RX buffer as read by SCO_SOURCE_EP provides 32 bit per 32-bit word.
         *
         * To sink buffer we write 16 encoded bits per addressed 32-bit word (NB: AUDIO_DATA_FORMAT_16_BIT_WITH_METADATA,  WBS: AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP_WITH_METADATA)
         *****************************/ 

        /* 
         * read packet size and metadatalength in [BYTES] from in-band metadata
         * and process integer amount of 16bit words
         */
        words16bit_deliver2operator = (ScoPacketSize_Bytes + 1) >> 1;

#ifdef DEBUG_STREAM_SCO_NAPIER_SOURCE
        static unsigned counter_sco_src;
        printf ("\n");
        printf ("********************************************************\n");
        printf ("*** file:     components/stream/stream_sco_napier.c  ***\n");
        printf ("*** function: 'sco_kick'                             ***\n");
        printf ("*** SCO_SOURCE_EP is KICKING.....                    ***\n");
        printf ("********************************************************\n");
        printf ("\n");
        for (k=0; k<endpoint->state.sco.source_buf_num; k++)
        {
            printf(" METADATA RX_BUF[%02d]: BufSize=0x%02X [BYTES] \t metadata_len=0x%02X [BYTES] \t status=0x%02X \t payload_size=0x%02X [BYTES] \t counter=%04d [BYTES]\n", 
                     k, endpoint->state.sco.source_buf[k]->size, metadata_rx[k].metadata_len, metadata_rx[k].status, metadata_rx[k].payload_size, metadata_rx[k].counter);
        }
        printf(" AUDIO_DATA_FORMAT (of connected OP)=%02d   \t MAJORITY_VOTE_BYPASS=%02d  \n", sco_get_data_format(endpoint), endpoint->state.sco.majority_vote_bypass);
        printf(" MAJORITY VOTING isVoting=%02d   or   SELECTION of RX_BUF[%02d]     BUFFER STATUS=%02d (0:STATUS_NONE  1:STATUS_VALID  2:STATUS_BAD_CRC) \n", bufVote.isVoting, bufVote.bufIdx, metadata_rx[bufVote.bufIdx].status);
        printf(" \n");
        printf(" Current WALLCLOCK value = 0x%4X \n", sco_wallclock_get(stream_sco_get_hci_handle(endpoint)));
        printf(" \n");
        printf(" words16bit_deliver2operator (w/o SYNC_WORD) = %02d [WORD16] \n", words16bit_deliver2operator);
        printf(" SCO_SOURCE_COUNTER = %02d \n", counter_sco_src);
        printf(" time_now = %04d [us] \n", hal_get_time());
        printf(" INIT_TIMESTAMP = 0x%04X \t\t tesco_bt_ticks = %d [BT-TICKS] \n", endpoint->state.sco.time_stamp_init, tesco_bt_ticks);
        printf("\n");
        for (k=0; k<endpoint->state.sco.source_buf[0]->size>>LOG2_ADDR_PER_WORD; k++)
        {
             printf(" RX_BUF_0[%2d]: ADDR=0x%08X / VALUE=0x%08X    ", k, endpoint->state.sco.source_buf[0]->read_ptr + k, endpoint->state.sco.source_buf[0]->read_ptr[k]);
             if (endpoint->state.sco.source_buf_num > 1)
             {
                 printf(" RX_BUF_1[%2d]: ADDR=0x%08X / VALUE=0x%08X    ", k, endpoint->state.sco.source_buf[1]->read_ptr + k, endpoint->state.sco.source_buf[1]->read_ptr[k]);
             }
             if (endpoint->state.sco.source_buf_num > 2)
             {
                 printf(" RX_BUF_2[%2d]: ADDR=0x%08X / VALUE=0x%08X    ", k, endpoint->state.sco.source_buf[2]->read_ptr + k, endpoint->state.sco.source_buf[2]->read_ptr[k]);
             }
             printf("\n");
        }

        printf("QUALITY METRIC: UNRELIABLE-TO-TOTAL BITS = %3d / %3d per SCO PACKET \n", bufVote.quality_metric, 8*metadata_rx[bufVote.bufIdx].payload_size);
        printf("Majority Voting enabled: %2d  \n", bufVote.isVoting);
        printf("\n");
        printf(" ep->state.sco.sink_buf[0]->size=0x%04X [BYTES] \t base_addr=0x%08X \t write_ptr=0x%08X \t read_ptr=0x%08X \n", 
                 endpoint->state.sco.sink_buf[0]->size, endpoint->state.sco.sink_buf[0]->base_addr, endpoint->state.sco.sink_buf[0]->write_ptr, endpoint->state.sco.sink_buf[0]->read_ptr);
        counter_sco_src++;
#endif

        /*
         ********************************
         * SCO_SOURCE: COPY SCO_PACKET  FROM IN- to OUT buffer
         *
         * Transform metadata (received from M0 and included in RX buffer)
         * to metadata format as used by BC/Hydra to reuse subsequent capabilities,
         * as e.g. WBS_DEC, SCO_RCV
         *
         * SCO_SOURCE_EP DATA_FORMAT@SINK_BUF:
         * SCO_NB:  AUDIO_DATA_FORMAT_16_BIT_WITH_METADATA
         * SCO_WBS: AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP_WITH_METADATA
         ********************************
         */
        for (k=0; k<words16bit_deliver2operator + 1; k++)
        {
             shift = (16*((k-1)%2));
             if (k==METADATA_SYNC_WORD_IDX)         /* DELIVERY: SYNC_WORD16 */
             {
                 word16 = SYNC_WORD;
             } 
             else if (k==METADATA_METADATA_LENGTH_IDX)    /* DELIVERY: METADATA_LENGTH (incr. delivered metatdata length by 2 (Bytes) for added SYNC_WORD) */
             {
                 word16 = metadata_rx[bufVote.bufIdx].metadata_len;
                 word16 += 2;
                 /*
                  * NOTE:
                  * WBS: SCO_WB and SCO_NB expects metadatalength in unit [WORD16]
                 */
                 word16 >>= 1; /* METADATA_LENGTH in [WORD16] instead of [BYTES] because WBS_DEC expects unit in [WORD16] */   
             }
             /*--- SWAP WORD16 of staus and payload size ---*/
             else if (k==METADATA_PAYLOAD_SIZE_IDX)    /* DELIVERY: PAYLOAD_SIZE */
             {  /* read payload size */
                word16 = metadata_rx[bufVote.bufIdx].payload_size; /* deliver 16 bits per WORD32 */
             }
             else if (k==METADATA_STATUS_IDX)  /* DELIVERY: STATUS */
             {  /* read status */
                /*
                 * Note: Hydra and BC consider metadata.status in sco_nb and sco_wb as follows (--> see sco_struct.h in capabilities/sco_fw/),
                 *       which is different from RX_BUFFER_STATUS we got from M0:
                         typedef enum{
                                                  OK = 0,
                                           CRC_ERROR = 1,
                                    NOTHING_RECEIVED = 2,
                                     NEVER_SCHEDULED = 3   
                         }metadata_status;
                 * --> we have to map the status we got from M0 to the expected status of subsequent capabilities
                 */
                 word16 = (metadata_rx[bufVote.bufIdx].status == RX_BUFFER_STATUS_VALID)   ? 0 : /* OK */
                          (metadata_rx[bufVote.bufIdx].status == RX_BUFFER_STATUS_BAD_CRC) ? 1 : /* CRC_ERROR */
                                                                                             2;  /* NOTHING_RECEIVED */

                 /* check quality meassure in case of MajorityVoting && RX_BUFFER_STATUS_BAD_CRC */
                 if (endpoint->state.sco.majority_vote_bypass == FALSE)
                 {   /* in case of 'RX_BUFFER_STATUS_BAD_CRC', we overwrite the status: CRC_ERROR -> OK, if less than given thrshld of Bits are unreliable after majority voting */
                     if ((bufVote.quality_metric <= endpoint->state.sco.majority_vote_questionable_bits_max) && (metadata_rx[bufVote.bufIdx].status == RX_BUFFER_STATUS_BAD_CRC))
                     {   
                          word16 = 0; /* delivered status = OK (0) instead of CRC error */
                     }
                 }
             }
             else if (k==METADATA_TIMESTAMP_IDX)  /* DELIVERY: COUNTER resp. TIMESTAMP */
             {
                /* COUNTER */
                // word16 = metadata_rx[bufVote.bufIdx].counter; /* deliver 16 bits per WORD32 */

                /* TIMESTAMP */
                word16 = (endpoint->state.sco.time_stamp_init + (tesco_bt_ticks * metadata_rx[bufVote.bufIdx].counter)); 
                word16 &= 0xFFFF;

                /*
                 * Note: What we got along with the M0 metadata is just a counter....
                 *
                 * actually, the SCO_RCV and SCO_WBS expect a timestamp in BT ticks.
                 * The time btw. 2 master slots in [BT_TICKS] would be 2*Tesco if Tesco is given in [SLOTS]
                 *
                 * However, the available timing parameters provide Tesco only in [us] -> this would mean: TIME_SPAN [BT_TICKS] = Tesco [us] / 312.5 [us]
                 * The Tesco in [us] we could get from
                 * Tesco [us] = endpoint->state.sco.kick_period>>6;
                 * Tesco [us] = sco_tesco_get(stream_sco_get_hci_handle(endpoint));
                 *
                 * we apply a "counter-to-timestamp" conversion
                 */
             }
             else
             {   /* DELIVERY: PAYLOAD  */
                 if (bufVote.isVoting)
                 {  /* apply majority voting (2 out of 3 voting) */
                    word16   = ((endpoint->state.sco.source_buf[0]->read_ptr[(k-1)>>1]) >> shift) & ((endpoint->state.sco.source_buf[1]->read_ptr[(k-1)>>1]) >> shift);
                    word16  |= ((endpoint->state.sco.source_buf[1]->read_ptr[(k-1)>>1]) >> shift) & ((endpoint->state.sco.source_buf[2]->read_ptr[(k-1)>>1]) >> shift);
                    word16  |= ((endpoint->state.sco.source_buf[0]->read_ptr[(k-1)>>1]) >> shift) & ((endpoint->state.sco.source_buf[2]->read_ptr[(k-1)>>1]) >> shift);
                    word16 &= 0xFFFF;
                 }
                 else
                 {  /* use bufVote.bufIdx */
                    word16 = 0xFFFF & (endpoint->state.sco.source_buf[bufVote.bufIdx]->read_ptr[(k-1)>>1]) >> shift; /* deliver 16 bits per WORD32 */
                 }

                 /* for odd number of Bytes, mask unused Byte of last WORD16 */
                 if ((k == words16bit_deliver2operator) && (ScoPacketSize_Bytes%2 != 0))
                 {
                       word16 &= 0x00FF; /* upper Byte unused -> mask */
                 }
             }
 
             /* write to output */
             if (sco_get_data_format(endpoint) == AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP_WITH_METADATA)
             {    /* SCO_WBS: AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP_WITH_METADATA */
                  // Note: Observations on a given WBS unit test appeared, that the BYTE swap was obviously done differently in GORDON and AMBER
                  //                                              WORD16[0..4]      WORD16[ELSE]         RESULT
                  // rep_sp_wb_meta.dat (applied for gordon):     SWAP              NO                   WBS_DEC: INVALID
                  //                                              SWAP              SWAP                 (OK) @ 8kHz
                  //
                  // comp_dist_wb_meta.dat (applied for amber)    SWAP              NO                   OK @ 16kHz
                  //                                              SWAP              SWAP                 INVALID
                  //
                  // FOR NAPIER, we apply BYTE swap as considered in WBS_DEC operator on data as well as on payload
                  word16 = WBS_METADATA_TRANSFORM(word16);
             }
             /* SCO_NB: AUDIO_DATA_FORMAT_16_BIT_WITH_METADATA */
             cbuffer_write(endpoint->state.sco.sink_buf[0], &word16, 1);  /* write word16 to buffer */


#ifdef DEBUG_STREAM_SCO_NAPIER_SOURCE
             cbuffer_advance_write_ptr(endpoint->state.sco.sink_buf[0], -1); //incr. out_buffer's write_ptr by 1 word
             printf(" endpoint->state.sco.sink_buf[0]->write_ptr[%2d]:  ADDR=0x%08X / VALUE=0x%04X \n", k, endpoint->state.sco.sink_buf[0]->write_ptr, *(endpoint->state.sco.sink_buf[0]->write_ptr));
             cbuffer_advance_write_ptr(endpoint->state.sco.sink_buf[0],  1); //incr. out_buffer's write_ptr by 1 word
#endif

        }

        /* kick the conected Op */
        propagate_kick(endpoint, STREAM_KICK_INTERNAL);

#ifdef DEBUG_STREAM_SCO_NAPIER_SOURCE
        printf("\n");
#endif
    }
    else
    {
        /**********************************/
        /*    SCO EP SINK PROCESSING      */
        /**********************************/
        /*
         * read the to_air_packet length in [BYTES] from the timing info of the SCO connection
         *
         * Note on 'to_air_length' (taken from sco_start in stream_sco_common.c):
         *
         * The to-air frame length actually has two meanings according to HYDRA resp. BC. It is:
         * 1. The data frame so BT rate-matching doesn't throw away a portion
         * of an encoded frame.
         * 2. The maximum length that audio will deliver data into the sco
         * buffer. If the operator will run more than once per kick then it
         * may produce more than a packet worth. BT needs to know otherwise
         * it could discard data and then later will have insufficient to
         * service a later packet
         *
         * Note: to_air_length and from_air_length are both considered as timing_info->block_size in 'sco_common_get_timing()'
         *
         * NOTE: Actually, unit of "to_air_length" is [16bit words] in Hydra and BC. However we need it in [BYTES] here.
         *       As long as compression factor parameter is set to 2, the "to_air_length" is equal to packet length in [BYTES],
         *       otherwise, a factor of 2 might be considered.
         */

         unsigned        byte2write;
         unsigned        word32=0;
         unsigned        InData_AmountOfBytes = cbuffer_calc_amount_data_in_words(endpoint->state.sco.source_buf[0])<<1;

         /* TX double buffering in case of 2 TX buffers */
         int sel_BufIdx = (endpoint->state.sco.sink_buf_num == 2) ? (endpoint->state.sco.metadata_TxCnt%2) : 0; 

#ifdef DEBUG_STREAM_SCO_NAPIER_SINK
         int  debug_buf_idx = sel_BufIdx; /* only for debug */
#endif

         /**************************/
         /* read words from source buffer and write bytewise SCO_PACKET to TX buffer */
         /**************************/
        /* 
         * NOTE: in case that PCM_EP initiates kicking, 
         * we might get 32bit words (which conveys 16bit data each)
         * on a different rate and size (e,g, size smaller than full sco packets) 
         *  
         * read input bytes to write complete SCO packet  
         */
         /* ScoPacketSize_Bytes = sco_to_air_length_get(stream_sco_get_hci_handle(endpoint)); */     /* when stored in timing struct as BYTES */
         ScoPacketSize_Bytes = sco_to_air_length_get(stream_sco_get_hci_handle(endpoint))<<1;        /* when stored in timing struct as uint16 words */

        /* In DEBUG_SCO_SINK_ODD_PACKET_SIZE_DISABLED case, we always start with 1st Byte and discard the last Bytes(s) if packet length is not multiple of 2.
         * In pratice, we assume not to have odd number of BYTES per SCO packet in 64kbps on air rate, due to packet_size = 5*tesco for 64kbps, tesco even. 
         * Anyhow, in case of packet lengths that are not multiuple of 2 Bytes, 
         * some stored legacy Bytes from last word in previous kick might be processed first (endpoint->state.sco.TxNumBytes2Write!=0)
         */  
         endpoint->state.sco.TxNumBytes2Write=0; /* see note above */

         /************************
         printf(" DEBUG: stream_sco_napier->sco_kick: time_now = %04d [us] \n", hal_get_time());
         printf(" DEBUG: ScoPacketSize_Bytes=%04d [BYTES]   \t     InData_AmountOfBytes=%04d [BYTES] \n", ScoPacketSize_Bytes, InData_AmountOfBytes);       
         ************************/
         
         /* write [BYTES] to TX buffer: we can write max. 1 sco packets at once */
         for (k=0; k<min(InData_AmountOfBytes, ScoPacketSize_Bytes); k++)
         {
              /* read input */
              if (endpoint->state.sco.TxNumBytes2Write == 0)
              {    /* we don't have some previous bytes in memory -> read new word */
                   cbuffer_read(endpoint->state.sco.source_buf[0], &endpoint->state.sco.TxBytes2Write, 1);  /* read word from buffer */
                   endpoint->state.sco.TxNumBytes2Write = 2; /* next 2 new bytes are read */

                   if (sco_get_data_format(endpoint) == AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP)
                   {    /* SCO_WB: AUDIO_DATA_FORMAT_16_BIT_BYTE_SWAP */
                        endpoint->state.sco.TxBytes2Write = WBS_METADATA_TRANSFORM(endpoint->state.sco.TxBytes2Write); /* we got byte-swapped WORD16 from WBS encoder -> undo byte-swap */
                   }
              }
              
              /* LSByte to MSByte */
              byte2write = (endpoint->state.sco.TxBytes2Write>>(16-(8*endpoint->state.sco.TxNumBytes2Write))) & 0xFF;
              endpoint->state.sco.TxNumBytes2Write--;
          
              /* write to output */
              if ((k%4)==0)
              {
                 word32 = 0; /* initialize next 32-bit word to be written */
              }            
              word32 |= ((byte2write&0xFF)<<(8*(k%4)));  /* write Byte to output (LSB to MSB) */ 
              
              /*******
              printf(" DEBUG: BUFF[%d]: k / TxNumBytesWritten = %2d / %2d \n", sel_BufIdx, k, endpoint->state.sco.TxNumBytesWritten); 
               *******/      
                     
              endpoint->state.sco.sink_buf[sel_BufIdx]->write_ptr[(endpoint->state.sco.TxNumBytesWritten>>LOG2_ADDR_PER_WORD)+1] = word32;
              endpoint->state.sco.TxNumBytesWritten++;   /* increment counter that counts written Bytes of current TX SCO packet */

              /* check whether SCO TX packet is complete */
              if (endpoint->state.sco.TxNumBytesWritten >= ScoPacketSize_Bytes)
              {   /* 
                   * SCO_TX-packet complete:
                   * write packet related metadata counter
                   * TX buffer toggling 
                   */
                   endpoint->state.sco.metadata_TxCnt++;             /* increment counter and write it as in-band metadata */
                   endpoint->state.sco.metadata_TxCnt &= 0x0000FFFF; /* we have 16bit counter; upper 16 bit of 1st word are dummy(=0) */
                   endpoint->state.sco.sink_buf[sel_BufIdx]->write_ptr[0] = endpoint->state.sco.metadata_TxCnt; /* write metadata counter at last */
                   sel_BufIdx = (endpoint->state.sco.metadata_TxCnt%2); /* toggle TX buffer to proceed writing to other TX buffer */
                   endpoint->state.sco.TxNumBytesWritten = 0;  /* reset Byte counter */
              }
         } /* end of for */

#ifdef DEBUG_STREAM_SCO_NAPIER_SINK
        /**************************/
        /* DEBUG (START) */
        /**************************/
        static unsigned counter_sco_sink;
        printf ("\n********************************************************\n");
        printf ("*** file:     components/stream/stream_sco_napier.c  ***\n");
        printf ("*** function: 'sco_kick'                             ***\n");
        printf ("*** SCO_SINK_EP gets KICKED.....                     ***\n");
        printf ("********************************************************\n");

        printf(" TX BUFFER SIZE (=TO_AIR_BUFFER) (payload + counter) = 0x%04d [32bit WORDS] = 0x%04d [BYTES] \n", cbuffer_get_size_in_words(endpoint->state.sco.sink_buf[0]), cbuffer_get_size_in_addrs(endpoint->state.sco.sink_buf[0]));
        printf(" SOURCE buffer size (=CVSD DELIVERY buffer) = %02d [WORD32] = %04d [BYTES] \n", cbuffer_get_size_in_words(endpoint->state.sco.source_buf[0]), cbuffer_get_size_in_addrs(endpoint->state.sco.source_buf[0]));
        printf(" \n"); 

        /* NOTE: Actually, unit of "to_air_length" is [16bit words], However we neeed the packet length in [BYTES] */
        /*       If compression factor holds value 2, the related timing value as written, is equal to packet length in [BYTES] */
        printf(" SCO packet length to be written to TX buffer:  ScoPacketSize_Bytes = 0x%04X [BYTES] \n", ScoPacketSize_Bytes);
        printf(" endpoint->state.sco.TxNumBytesWritten=%02d \t endpoint->state.sco.TxNumBytes2Write=%02d \t endpoint->state.sco.TxBytes2Write=0x%08X \n", endpoint->state.sco.TxNumBytesWritten, endpoint->state.sco.TxNumBytes2Write, endpoint->state.sco.TxBytes2Write);
        printf(" SCO_SINK_COUNTER = %02d \n", counter_sco_sink);
        counter_sco_sink++;
        printf("\n");
        
        for (k=0; k<cbuffer_get_size_in_words(endpoint->state.sco.sink_buf[0]); k++)
        {
             printf(" endpoint->state.sco.sink_buf[%d]->write_ptr[%02d]: ADDR=0x%08X / VALUE=0x%08X \n", debug_buf_idx, k, endpoint->state.sco.sink_buf[debug_buf_idx]->write_ptr+k, endpoint->state.sco.sink_buf[debug_buf_idx]->write_ptr[k]);
        }
        printf(" AUDIO_DATA_FORMAT of connected OP: sco_get_data_format(endpoint)=%02d \n", sco_get_data_format(endpoint));
        printf("\n");
        /**************************/
        /* DEBUG (STOP) */
        /**************************/
#endif

        /**********************************/
        /* SCO EP SINK PROCESSING 8END)   */
        /**********************************/
    }
}


/*--------------------------------------------------------------------------*
 * get_metadata                                                             *
 *--------------------------------------------------------------------------*/
static sco_metadata_t get_metadata(int* rx_buffer_ptr)
{

    patch_fn_shared(stream_sco);

    /* The following metadata order is considered:
     *
     *  -------------------------- -------------------------- -------------------------- --------------------------
     * |    BYTE_3: MSB_STATUS    |    BYTE_2: LSB_STATUS    | BYTE_1: MSB_METADATA_LEN | BYTE_0: LSB_METADATA_LEN |
     *  --------------------------^--------------------------^--------------------------^--------------------------^
     *                                                                                                             |
     *                                                                                                       &rx_buffer_ptr[0]
     *
     *
     *  -------------------------- -------------------------- -------------------------- --------------------------
     * |    BYTE_3: MSB_COUNTER   |    BYTE_2: LSB_COUNTER   | BYTE_1: MSB_PAYLOAD_SIZE | BYTE_0: LSB_PAYLOAD_SIZE |
     *  --------------------------^--------------------------^--------------------------^--------------------------^
     *                                                                                                             |
     *                                                                                                       &rx_buffer_ptr[1]
     */
    sco_metadata_t result;
    /* metadata are 8 Bytes in SCO NAPIER*/
    result.metadata_len   =  rx_buffer_ptr[0] & 0x000000FF;   /* BYTE[0]: LSB of Lenght of in band metadata in BYTES */
    result.metadata_len  |=  rx_buffer_ptr[0] & 0x0000FF00;   /* BYTE[1]: MSB of Lenght of in band metadata in BYTES */

    result.status   = (rx_buffer_ptr[0]>>16)  & 0x000000FF;   /* BYTE[2]: LSB of Status of SCO packet (OK, CRC failed, Not received)  */
    result.status  |= (rx_buffer_ptr[0]>>16)  & 0x0000FF00;   /* BYTE[3]: MSB of Status of SCO packet (OK, CRC failed, Not received)  */

    result.payload_size   =  rx_buffer_ptr[1] & 0x000000FF;   /* BYTE[4]: LSB of payload size in BYTES */
    result.payload_size  |=  rx_buffer_ptr[1] & 0x0000FF00;   /* BYTE[5]: MSB of payload size in BYTES */

    result.counter   = (rx_buffer_ptr[1]>>16) & 0x000000FF;   /* BYTE[6]: MSB of counter */
    result.counter  |= (rx_buffer_ptr[1]>>16) & 0x0000FF00;   /* BYTE[7]: LSB of counter */

    return result;
}


/*--------------------------------------------------------------------------*
 * sco_ErrDataProcess_getRxBufIdx                                                 *
 *--------------------------------------------------------------------------*/
static sco_majority_vote_t sco_ErrDataProcess_MajorityVoting(sco_metadata_t* metadata_rx, int32 buf_num, tCbuffer** rx_buf_ptr)
{
    unsigned              hamingWeight_RxBuf_1_0;
    unsigned              hamingWeight_RxBuf_2_0;
    unsigned              hamingWeight_RxBuf_2_1;
    sco_majority_vote_t   result;
    int k;

    /* check crc result of all rx buffers: if CRC of one out of these data is OK select it -> return buf idx (0/1/2) */
    for (k=0; k<buf_num; k++)
    {
         if (metadata_rx[k].status == RX_BUFFER_STATUS_VALID)
         {
             result.bufIdx = k;          /* selected Buffer Index: return idx=k (just selecting buffer[k])   */
             result.isVoting = 0;        /* indicates whetehr majority voting should be applied(1) or not(0) */
             result.quality_metric = 0;  /* there is one valiud packet, so we assume best quality */
             return (result);            /* return buffer idx that passes CRC */
         }
    }

    /* all received sco packets have either RX_BUFFER_STATUS_BAD_CRC or RX_BUFFER_STATUS_NONE */
    if (buf_num == 1)
    {
               result.bufIdx = 0;  
               result.isVoting = 0;
               result.quality_metric = metadata_rx[0].payload_size; /* we tread all payload bits of SCO packets as uncertain */
               return (result);
    }
    else if (buf_num == 2)
    {   /* 2 RX buffers, both with CRC fails*/
        if (  (metadata_rx[0].status == RX_BUFFER_STATUS_BAD_CRC)
           && (metadata_rx[1].status == RX_BUFFER_STATUS_BAD_CRC))
        {
               result.bufIdx = 0;  
               result.isVoting = 0;
               hamingWeight_RxBuf_1_0 = calc_HammingWeight(metadata_rx[0].payload_size, &rx_buf_ptr[0]->read_ptr[2], &rx_buf_ptr[1]->read_ptr[2]); /* corelate payload: ignore first 2 word32 -> metadata */
               result.quality_metric = hamingWeight_RxBuf_1_0;
               return (result);
        }
        else
        {
               /* 2 RX buffers, one with RX_BUFFER_STATUS_BAD_CRC, the other with RX_BUFFER_STATUS_NONE -> select the one out of the two with RX_BUFFER_STATUS_BAD_CRC */
               for (k=0; k<2; k++)
               {
                  if (metadata_rx[k].status == RX_BUFFER_STATUS_BAD_CRC)
                  {
                      result.bufIdx = k;    /* selected Buffer Index: return idx=k (just selecting buffer[k])   */
                      result.isVoting = 0;  /* indicates whether majority voting should be applied(1) or not(0) */
                      result.quality_metric = metadata_rx[0].payload_size; /* we tread all payload bits of SCO packets as uncertain */
                      return (result);
                  }
               }

               /* 2 RX buffers, both RX_BUFFER_STATUS_NONE */
               result.bufIdx = 0;  
               result.isVoting = 0;
               result.quality_metric = metadata_rx[0].payload_size; /* we tread all payload bits of SCO packets as uncertain */
               return (result);
        }
    }
    else
    {   /* 3 RX buffers */
        if (  (metadata_rx[0].status == RX_BUFFER_STATUS_BAD_CRC)
           && (metadata_rx[1].status == RX_BUFFER_STATUS_BAD_CRC)
           && (metadata_rx[2].status == RX_BUFFER_STATUS_BAD_CRC))
        {
            /*
             **************************************
             * 3 RX buffers -> All have CRC error *
             **************************************
             */
             // check wheter 2 out of the 3 have improved correlation on the poayload -> select one out of the two
             // otherwise do xor pairwise on all three buffers (use metadata from first buffer)
             hamingWeight_RxBuf_1_0 = calc_HammingWeight(metadata_rx[0].payload_size, &rx_buf_ptr[0]->read_ptr[2], &rx_buf_ptr[1]->read_ptr[2]); /* corelate payload: ignore first 2 word32 -> metadata */
             hamingWeight_RxBuf_2_0 = calc_HammingWeight(metadata_rx[0].payload_size, &rx_buf_ptr[0]->read_ptr[2], &rx_buf_ptr[2]->read_ptr[2]); /* corelate payload: ignore first 2 word32 -> metadata */
             hamingWeight_RxBuf_2_1 = calc_HammingWeight(metadata_rx[1].payload_size, &rx_buf_ptr[1]->read_ptr[2], &rx_buf_ptr[2]->read_ptr[2]); /* corelate payload: ignore first 2 word32 -> metadata */

             if (     (hamingWeight_RxBuf_1_0 < (hamingWeight_RxBuf_2_0>>1)) && (hamingWeight_RxBuf_1_0 < (hamingWeight_RxBuf_2_1>>1)))
             {   /* RX_buf_2 poorly correlated -> discard */
                 result.bufIdx = 0;    /* selected Buffer Index: select RX_buf_0 or RX_buf_1 */
                 result.isVoting = 0;  /* indicates whetehr majority voting should be applied(1) or not(0) */
                 /* quality metric counts uncertain bits per SCO packet; the lower the value the better the quality */
                 result.quality_metric = hamingWeight_RxBuf_1_0;
             }
             else if ((hamingWeight_RxBuf_2_0 < (hamingWeight_RxBuf_1_0>>1)) && (hamingWeight_RxBuf_2_0 < (hamingWeight_RxBuf_2_1>>1)))
             {   /* RX_buf_1 poorly correlated -> discard */
                 result.bufIdx = 0;    /* selected Buffer Index: select RX_buf_0 or RX_buf_2 */
                 result.isVoting = 0;  /* indicates whetehr majority voting should be applied(1) or not(0) */
                 /* quality metric counts uncertain bits per SCO packet; the lower the value the better the quality */
                 result.quality_metric = hamingWeight_RxBuf_2_0;
             }
             else if ((hamingWeight_RxBuf_2_1 < (hamingWeight_RxBuf_1_0>>1)) && (hamingWeight_RxBuf_2_1 < (hamingWeight_RxBuf_2_0>>1)))
             {   /* RX_buf_0 poorly correlated -> discard */
                 result.bufIdx = 1;    /* selected Buffer Index: select RX_buf_1 or RX_buf_2 */
                 result.isVoting = 0;  /* indicates whetehr majority voting should be applied(1) or not(0) */
                /* quality metric counts uncertain bits per SCO packet; the lower the value the better the quality */
                result.quality_metric = hamingWeight_RxBuf_2_1;
             }
             else
             { /* all RX buffers have about the same correlation -> 2 out of 3 voting */
                 result.bufIdx = 0;     /* selected Buffer Index: select RX_buf_1 or RX_buf_2 */
                 result.isVoting = 1;   /* indicates whetehr majority voting should be applied(1) or not(0): process majority voting (best 2 out of 3) */
                 /* quality metric counts uncertain bits per SCO packet; the lower the value the better the quality */
                 result.quality_metric = calc_QualityMetric_3RxBuffers(metadata_rx[0].payload_size, &rx_buf_ptr[0]->read_ptr[2], &rx_buf_ptr[1]->read_ptr[2], &rx_buf_ptr[2]->read_ptr[2]);
             }
             return (result);
        }
        else
        {
             /*
              *****************************************
              * 3 RX buffers -> Max. 2 have CRC error *
              * the others have RX_BUFFER_STATUS_NONE *
              *****************************************
              */
              if (  (metadata_rx[0].status == RX_BUFFER_STATUS_BAD_CRC)
                 && (metadata_rx[1].status == RX_BUFFER_STATUS_BAD_CRC))
              {
                  result.bufIdx = 0;  
                  result.isVoting = 0;
                  result.quality_metric = calc_HammingWeight(metadata_rx[0].payload_size, &rx_buf_ptr[0]->read_ptr[2], &rx_buf_ptr[1]->read_ptr[2]); /* corelate payload: ignore first 2 word32 -> metadata */
                  return (result);
              }
              else if (  (metadata_rx[0].status == RX_BUFFER_STATUS_BAD_CRC)
                      && (metadata_rx[2].status == RX_BUFFER_STATUS_BAD_CRC))
              {
                  result.bufIdx = 0;  
                  result.isVoting = 0;
                  result.quality_metric = calc_HammingWeight(metadata_rx[0].payload_size, &rx_buf_ptr[0]->read_ptr[2], &rx_buf_ptr[2]->read_ptr[2]); /* corelate payload: ignore first 2 word32 -> metadata */
                  return (result);
              }
              else if (  (metadata_rx[1].status == RX_BUFFER_STATUS_BAD_CRC)
                      && (metadata_rx[2].status == RX_BUFFER_STATUS_BAD_CRC))
              {
                  result.bufIdx = 1;  
                  result.isVoting = 0;
                  result.quality_metric = calc_HammingWeight(metadata_rx[1].payload_size, &rx_buf_ptr[1]->read_ptr[2], &rx_buf_ptr[2]->read_ptr[2]); /* corelate payload: ignore first 2 word32 -> metadata */
                  return (result);
              }
              else
              {
                     /* 3 RX buffers, one with RX_BUFFER_STATUS_BAD_CRC, the others with RX_BUFFER_STATUS_NONE -> select the one out of the two with RX_BUFFER_STATUS_BAD_CRC */
                     for (k=0; k<3; k++)
                     {
                        if (metadata_rx[k].status == RX_BUFFER_STATUS_BAD_CRC)
                        {
                            result.bufIdx = k;                                   /* selected Buffer Index: return idx=k (just selecting buffer[k])   */
                            result.isVoting = 0;                                 /* indicates whether majority voting should be applied(1) or not(0) */
                            result.quality_metric = metadata_rx[0].payload_size; /* we tread all payload bits of SCO packets as uncertain */
                            return (result);
                        }
                     }
                     /* 3 RX buffers, all 3 RX_BUFFER_STATUS_NONE */
                     result.bufIdx = 0;  
                     result.isVoting = 0;
                     result.quality_metric = metadata_rx[0].payload_size; /* we tread all payload bits of SCO packets as uncertain */
                     return (result);
              }
        }
    }
}


/*--------------------------------------------------------------------------*
 * calc_HammingWeight                                                       *
 *--------------------------------------------------------------------------*/
static unsigned calc_HammingWeight(int num_Bytes, int* rx_buf_ptr0, int* rx_buf_ptr1)
{
    int k;
    /* int j; */
    unsigned right_shift;
    unsigned byte_compare;
    unsigned HammingWeight = 0;

    for (k=0; k<num_Bytes; k++)
    {
          /* bitwise xor of Bytes */
          right_shift = 8*(k%4); /* 0, 8, 16, 24 */
          byte_compare = (((rx_buf_ptr0[k>>2])>>right_shift) ^ ((rx_buf_ptr1[k>>2])>>right_shift)) & 0xFF;     /* Note: rx_buf_ptr0, rx_buf_ptr0 point to WORD32 */

          /*-------------------------*/
          /* count '1' bits in current BYTE */
          /* Algorithm_1: probably more efficient than Algorithm_2 */
          while(byte_compare)
          {
              HammingWeight++;
              byte_compare &= (byte_compare- 1);
          }

          /*-------------------------*/
          /* Algorithm_2 */
          /*
          for (j=0; j<8; j++)
          {  
              if ((byte_compare>>j) & 0x1)
                   HammingWeight++;
          }
          */
    }
    return (HammingWeight);
}


/*--------------------------------------------------------------------------*
 * calc_QualityMetric_3RxBuffers                                                       *
 *--------------------------------------------------------------------------*/
static unsigned calc_QualityMetric_3RxBuffers(int num_Bytes, int* rx_buf_ptr0, int* rx_buf_ptr1, int* rx_buf_ptr2)
{
    int k;
    unsigned right_shift;
    unsigned byte_compare;
    unsigned QualityWeight = 0;

    patch_fn_shared(stream_sco);

    /* if all 3 bits are equal --> 0, otherwise -> 1 */
    /* the higher the metric value, the worse the quality */
    for (k=0; k<num_Bytes; k++)
    {
          right_shift = 8*(k%4); /* 0, 8, 16, 24 */
          byte_compare = (((rx_buf_ptr0[k>>2])>>right_shift) | ((rx_buf_ptr1[k>>2])>>right_shift) | ((rx_buf_ptr2[k>>2])>>right_shift)) & 0xFF;     /* Note: rx_buf_ptr points to WORD32 */
          byte_compare &= (~(((rx_buf_ptr0[k>>2])>>right_shift) & ((rx_buf_ptr1[k>>2])>>right_shift) & ((rx_buf_ptr2[k>>2])>>right_shift))) & 0xFF;

          /* count '1' bits in current BYTE */
          while(byte_compare)
          {
              QualityWeight++;
              byte_compare &= (byte_compare- 1);
          }
    }
    return (QualityWeight);
}


/*--------------------------------------------------------------------------*
 * sco_configure_napier                                                     *
 *--------------------------------------------------------------------------*/
static bool sco_configure_napier(ENDPOINT *endpoint, unsigned int key, uint32 value)
{
    switch(key)
    {
        case ACCMD_CONFIG_KEY_STREAM_SCO_SRC_MAJORITY_VOTE_BYPASS:
             if(endpoint->direction == SOURCE)
             {   /* majority voting is only applied on RX buffers as read by SCO_SOURCE */
                 endpoint->state.sco.majority_vote_bypass = (value==1) ? TRUE : FALSE;
                 return TRUE;
             }
             else
             {   /* indicates, that majority voting is not relevant for SCO_SINK */
                 return FALSE;
             }
             break;

        case ACCMD_CONFIG_KEY_STREAM_SCO_SRC_MAJORITY_VOTE_QUESTIONABLE_BITS_MAX:
             if(endpoint->direction == SOURCE)
             {   /* majority voting: max. number of uncorrelated bits to status overwrite in case of CRC_FAIL */
                 endpoint->state.sco.majority_vote_questionable_bits_max = (value<256) ? value : 255; /* uint8 */
                 return TRUE;
             }
             else
             {   /* indicates, that majority voting is not relevant for SCO_SINK */
                 return FALSE;
             }
             break;

        default:
             /* call common function */
             return (sco_configure(endpoint, key, value));
    }
}


/*--------------------------------------------------------------------------*
 * sco_get_config_napier                                                    *
 *--------------------------------------------------------------------------*/
static bool sco_get_config_napier(ENDPOINT *endpoint, unsigned int key, uint32 *value)
{
    switch(key)
    {
        case ACCMD_CONFIG_KEY_STREAM_SCO_SRC_MAJORITY_VOTE_BYPASS:
             if(endpoint->direction == SOURCE)
             {   /* majority voting is only applied on RX buffers as read by SCO_SOURCE */
                 *value = (uint32) endpoint->state.sco.majority_vote_bypass;
                 return TRUE;
             }
             else
             {   /* indicates, that majority voting is not relevant for SCO_SINK */
                 return FALSE;
             }
             break;

        case ACCMD_CONFIG_KEY_STREAM_SCO_SRC_MAJORITY_VOTE_QUESTIONABLE_BITS_MAX:
             if(endpoint->direction == SOURCE)
             {   /* majority voting: max. number of uncorrelated bits to status overwrite in case of CRC_FAIL */
                 *value = (uint32) endpoint->state.sco.majority_vote_questionable_bits_max;
                 return TRUE;
             }
             else
             {   /* indicates, that majority voting is not relevant for SCO_SINK */
                 return FALSE;
             }
             break; 

        default:
             /* call common function */
             return (sco_get_config(endpoint, key, value));
    }
}


/*--------------------------------------------------------------------------*
 * flush_endpoint_buffers                                                   *
 *--------------------------------------------------------------------------*/
void flush_endpoint_buffers(ENDPOINT *ep)
{
    tCbuffer* cbuffer = stream_transform_from_endpoint(ep)->buffer;

    patch_fn_shared(stream_sco);

    if(ep->direction == SOURCE)
    {
        cbuffer_empty_buffer(cbuffer);
    }
    else
    {
        cbuffer_flush_and_fill(cbuffer, 0);
    }
}






/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
*    \file a2dp_common_decode.h
*
*    A2DP common header file. Contains common A2DP function headers.
*/

/****************************************************************************
Include Files
*/
#include "capabilities.h"
#include "codec_c.h"
#include "mem_utils/memory_table.h"

/****************************************************************************
Public Type Declarations
*/

/** Function prototype of function that frees all the resources allocated by the
 * inheriting class. */
typedef void (*FREE_DATA_FN)(OPERATOR_DATA *);

typedef bool (*RESET_FN)(OPERATOR_DATA *);

/** Function prototype of function that performs an encode */
typedef void (*ENCODE_FN)(void);

/** Function prototype of function that gets frame sizes */
typedef void (*FRAME_SIZE_FN)(OPERATOR_DATA *op_data, unsigned *in_size_samples, unsigned *out_size_octets);

/** Constant attributes that an inheriting class implements */
typedef struct
{
    /** The function that performs the frame encode */
    ENCODE_FN encode_fn;

    /** The function that will free resources allocated at create */
    FREE_DATA_FN free_fn;

    /** The function that will reset the inernal encoder state */
    RESET_FN reset_fn;

    /** The function that will report the input and output frame sizes */
    FRAME_SIZE_FN frame_size_fn;

    /** Table of any scratch memory needed by the encoder */
    const scratch_table *scratch_allocs;
} ENCODER_CAP_VIRTUAL_TABLE;

/** Common attributes from sbc and aptX decoder operator structures */
typedef struct
{
    /** A DECODER structure used by the codec library */
    ENCODER codec;

    /** Virtual function/data table provided by the inheriting class */
    const ENCODER_CAP_VIRTUAL_TABLE *vt;

#ifdef INSTALL_METADATA
   /** The input buffer with metadata to transport from */
   tCbuffer *metadata_ip_buffer;

   /** The input sample rate */
   unsigned sample_rate;

   /** Timestamp from the last metadata tag processed */
   TIME last_tag_timestamp;

   /** Sample period adjustment value from the last metadata tag */
   unsigned last_tag_spa;

   /** Samples read since the last metadata tag */
   unsigned last_tag_samples;

   /* Error offset ID from last tag */
   unsigned last_tag_err_offset_id;

   /** Encode / Decode algorithmic delay, in samples */
   unsigned delay_samples;

#endif /* INSTALL_METADATA */
    
} ENCODER_PARAMS;

/****************************************************************************
Public Constant Declarations
*/
/** Minimum input buffer size */
#define A2DP_DECODE_OUTPUT_BUFFER_SIZE    768
/** Minimum output buffer size */
#define A2DP_DECODE_INPUT_BUFFER_SIZE   256

/** Terminal numbers */
#define LEFT_IN_TERMINAL_ID            (0 | TERMINAL_SINK_MASK)
#define RIGHT_IN_TERMINAL_ID           (1 | TERMINAL_SINK_MASK)
#define OUTPUT_TERMINAL_ID               (0)

/****************************************************************************
Public Constant Definitions
*/


/****************************************************************************
Public Function Declarations
*/

/**
 * \brief Initialises the encoder base class. The memory for the base class is
 * expected to have been allocated by the inheriting class. Any scratch memory
 * needed by the inheriting class is registered for and reserved.
 *
 * \param op_data Pointer to the operator instance data.
 * \param enc_base The location of the base class structure (ENCODER_PARAMS)
 * \param codec_data The codec specifc data structure
 * \param vt Table containing all the attributes that the inheriting class has
 * implemented that this base class may need to invoke/use
 *
 * \return Whether the initialisation succeeded.
 */
extern bool encoder_base_class_init(OPERATOR_DATA *op_data, ENCODER_PARAMS *enc_base,
                        void *codec_data, const ENCODER_CAP_VIRTUAL_TABLE * const vt);

/**
 * \brief Deallocates the encoders allocated memory after after calling the
 * inheriting capability's release function.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the destroy request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
extern bool encoder_destroy(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data);

/* Command handlers */
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
extern bool encoder_connect(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data);

/**
 * \brief Disconnects a capability terminal from a buffer.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the disconnect request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */extern bool encoder_disconnect(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data);

 /**
  * \brief Starts the encoder capability so encoding will be attempted on a
  * kick.
  *
  * \param op_data Pointer to the operator instance data.
  * \param message_data Pointer to the start request message
  * \param response_id Location to write the response message id
  * \param response_data Location to write a pointer to the response message
  *
  * \return Whether the response_data field has been populated with a valid
  * response
  */
extern bool encoder_start(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data);

/**
 * \brief Resets an encoder capability.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the reset request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
bool encoder_reset(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data);

/**
 * \brief Reports the buffer requirements of the requested capability terminal.
 * Request input buffers of 256 words and an output buffer of 768 words.
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
extern bool encoder_buffer_details(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data);

/**
 * \brief Reports the buffer requirements of the requested capability terminal
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the buffer size request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 * \param inp_bufsize Size of input buffer to request for the connection.
 * \param out_bufsize Size of output buffer to request for the connection.
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
extern bool encoder_buffer_details_core(OPERATOR_DATA *op_data, void *message_data,
                                    unsigned *response_id, void **response_data,
                                    unsigned inp_bufsize, unsigned out_bufsize);

/**
 * \brief Reports the data format of the requested capability terminal
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the data format request message
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
extern bool encoder_get_data_format(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data);

/**
 * \brief Returns the block size
 *
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the get_block_size request message payload
 * \param response_id Location to write the response message id
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
extern bool encoder_get_sched_info(OPERATOR_DATA *op_data, void *message_data,
                                unsigned *response_id, void **response_data);
                                

/** \brief Generic encoder capability process_data function template. Calls the
 * capabilities underlying frame_encode function and provides it an opportunity
 * to commit any scratch memory needed by the algorithm.
 *
 * \param op_data Pointer to the operator instance data.
 * \param touched Return value for indicating connections to kick
 */
extern void encode_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched);


/**
 * \brief prototype for calling the ASM wrapper for the codec library's frame_encode function
 *
 * \param encoder The generic encoder structure for the capability
 * \param encode_fn The address of the frame_encode function that will perform the encode
 *
 * \return True if something was encoded. False if nothing encoded.
 */
extern bool encoder_encode(ENCODER *encoder, ENCODE_FN encode_fn);

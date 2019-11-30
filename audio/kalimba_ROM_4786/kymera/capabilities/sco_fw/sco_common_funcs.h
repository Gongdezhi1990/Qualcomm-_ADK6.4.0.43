/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  sco_common_funcs.h
 * \ingroup  capabilities
 *
 *  Common functions, used by NB and WB SCO capabilities.
 *  Functions "sco_common_rcv_..." are for receive capabilities (SCO_RCV & WBS_DEC).
 *  "sco_common_send_..." are for send capabilities (SCO_SEND, WBS_ENC).
 *  "sco_common_..." are for any SCO capability.
 *
 */

#include "capabilities.h"
#include "sco_struct.h"


/* initialise some common parts of SCO operator data - it trusts that everything referenced here was allocated before call */

/**
 * \brief Initialise NB or WB SCO receive operators' certain parameters during creation. It differs from
 *        sco_common_rcv_reset_working_data() as latter can be called during operator reset.
 *        The function can only be called once the capability-specific and (if present in the build) the PLC data structures
 *        are allocated!
 *
 * \param  sco_rcv_op_data  Pointer to the common SCO receive operator data structure, with already allocated PLC structure.
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern void sco_common_rcv_initialise(SCO_COMMON_RCV_OP_DATA* sco_rcv_op_data);

/**
 * \brief Delete all PLC data and NULLify the PLC struct pointer in operator data.
 *
 * \param  sco_rcv_op_data  Pointer to the common SCO receive operator data structure.
 */
#ifdef INSTALL_PLC100
void sco_common_rcv_destroy_plc_data(SCO_COMMON_RCV_OP_DATA* sco_rcv_op_data);
#endif /* INSTALL_PLC100 */

/**
 * \brief Initialise various working data params of the NB or WB SCO receive operators.
 *
 * \param  sco_rcv_op_data  Pointer to the common SCO receive operator data structure.
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
/*
 * The part of the extra_op_data that is referenced to
 * by this is identical for both capabilities, hence pointer casts are safe
 * unless someone re-arranges the operator data structs and ignores comments :)
 */
extern bool sco_common_rcv_reset_working_data(SCO_COMMON_RCV_OP_DATA* sco_rcv_op_data);

/**
 * \brief Helper function for opmsg handlers that set from air information.
 *
 * \param  op_data  Pointer to the operator structure.
 * \param  sco_rcv_op_data  Pointer to the common SCO receive operator data structure.
 * \param  message_data  Pointer to message structure.
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern bool sco_common_rcv_set_from_air_info_helper(OPERATOR_DATA *op_data, SCO_COMMON_RCV_OP_DATA* sco_rcv_op_data, void *message_data);

#ifdef INSTALL_PLC100
/**
 * \brief Helper function for opmsg handlers that set the flag that forces PLC off (if non-zero value is sent).
 *
 * \param  sco_rcv_op_data  Pointer to the common SCO receive operator data structure.
 * \param  message_data  Pointer to message structure.
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern bool sco_common_rcv_force_plc_off_helper(SCO_COMMON_RCV_OP_DATA* sco_rcv_op_data, void *message_data);
#endif /* INSTALL_PLC100 */

/**
 * \brief Helper function for opmsg handlers that  get the frame count statistics
 *
 * \param  sco_rcv_op_data  Pointer to the common SCO receive operator data structure.
 * \param  message_data  Pointer to message structure.
 * \param  resp_length  Pointer to response message length in words
 * \param  resp_data  Pointer to a pointer to response message structure, allocated in this function.
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern bool sco_common_rcv_frame_counts_helper(SCO_COMMON_RCV_OP_DATA* sco_rcv_op_data, void *message_data,
                                               unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

/**
 * \brief Operator message handler for getting the terminal data formats
 *
 * \param  op_data  Pointer to the operator structure.
 * \param  message_data  Pointer to message structure.
 * \param  resp_length  Pointer to response message length in words
 * \param  resp_data  Pointer to a pointer to response message structure, allocated in this function.
 * \param  input_format  The data format of the capability input terminal.
 * \param  output_format  The data format of the capability output terminal.
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern bool sco_common_get_data_format(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id,
        void **response_data, AUDIO_DATA_FORMAT input_format, AUDIO_DATA_FORMAT output_format);

/**
 * \brief Operator message handler for getting the block size of the operator
 *
 * \param  op_data  Pointer to the operator structure.
 * \param  message_data  Pointer to message structure.
 * \param  resp_length  Pointer to response message length in words
 * \param  resp_data  Pointer to a pointer to response message structure, allocated in this function.
 * \param  input_block_size  the input terminal block size of the capability.
 * \param  output_block_size  the output terminal block size of the capability.
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern bool sco_common_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id,
        void **response_data, unsigned input_block_size, unsigned output_block_size);

/**
 * \brief Operator message handler for connecting operator terminal
 *
 * \param  op_data  Pointer to the operator structure.
 * \param  message_data  Pointer to message structure.
 * \param  resp_length  Pointer to response message length in words
 * \param  resp_data  Pointer to a pointer to response message structure, allocated in this function.
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern bool sco_common_connect(OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data, SCO_TERMINAL_BUFFERS *bufs,
        unsigned *terminal_id);

/**
 * \brief Operator message handler for disconnecting operator terminal
 *
 * \param  op_data  Pointer to the operator structure.
 * \param  message_data  Pointer to message structure.
 * \param  resp_length  Pointer to response message length in words
 * \param  resp_data  Pointer to a pointer to response message structure, allocated in this function.
 *
 * \return TRUE if completed successfully, FALSE on fail.
 */
extern bool sco_common_disconnect(OPERATOR_DATA *op_data, void *message_data,
        unsigned *response_id, void **response_data, SCO_TERMINAL_BUFFERS *bufs,
        unsigned *terminal);

#ifdef SCO_RX_OP_GENERATE_METADATA
/**
 * sco_common_rcv_opmsg_set_ttp_latency
 * \brief sets the rcv op to generate timestamp tags instead of default toa
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the start request message
 * \param resp_length pointer to location to write the response message length
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
extern bool sco_common_rcv_opmsg_set_ttp_latency(OPERATOR_DATA *op_data, void *message_data,
                                     unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
/**
 * sco_common_rcv_opmsg_set_buffer_size
 * \brief message handler to set required sco rcv output buffer size
 * \param op_data Pointer to the operator instance data.
 * \param message_data Pointer to the start request message
 * \param resp_length pointer to location to write the response message length
 * \param response_data Location to write a pointer to the response message
 *
 * \return Whether the response_data field has been populated with a valid
 * response
 */
extern bool sco_common_rcv_opmsg_set_buffer_size(OPERATOR_DATA *op_data, void *message_data,
                                          unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#endif /* SCO_RX_OP_GENERATE_METADATA */

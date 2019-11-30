/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup opmgr Operator Manager
 *
 * \file  opmgr.h
 * \ingroup opmgr
 *
 * \brief
 * Operator Manager public header file. <br>
 * This file contains public opmgr functions and types. <br>
 *
 */

#ifndef OPMGR_H
#define OPMGR_H

/****************************************************************************
Include Files
*/

#include "types.h"
#include "buffer.h"
#include "stream/stream.h"
#include "cap_id_prim.h"
#include "status_prim.h"
#include "sched_oxygen/sched_oxygen.h"


#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
#include "ipc/ipc_kip.h" /* for IPC_PROCESSOR_ID_NUM */
#endif

/****************************************************************************
Public Type Declarations
*/

/** Forward declared type for OPMSG_COMMON_GET_CONFIG result, to refer to pointers */
typedef union OPMSG_GET_CONFIG_RESULT OPMSG_GET_CONFIG_RESULT;

/**
 * Opmgr struct to define key/value style configuration
 * for create operator (extended) command used across platforms.
 */
typedef struct
{
    /** Key to specify the type of the configure. */
    unsigned int key;
    /** The configure value for the key.*/
    int32  value;
} OPERATOR_CREATE_EX_INFO;

/**
 * Op Message struct to simplify their handling.
 * Public as must be seen by accmd_audio_interface and opmgr.
 */
typedef struct {
    unsigned int opid;
    unsigned int* op_msg;
    unsigned int op_msg_len;
} op_msg_struct;


/** Standard unsolicited message from operator */
typedef struct
{
    unsigned op_id;
    unsigned client_id;
    unsigned msg_id;
    unsigned length; /* zero */
} OP_STD_UNSOLICITED_MSG;

/** Unsolicited message from operator */
typedef struct
{
    unsigned op_id;
    unsigned client_id;
    unsigned msg_id;
    unsigned length; 
    unsigned payload[];
} OP_UNSOLICITED_MSG;

/**
 * Enum which is used to signal which side of the operator doesn't want kick.
 * SOURCE_SIDE and SINK_SIDE can be used as mask to check if the kicks are disabled at
 * one side.
 */
typedef enum{
    NEITHER_SIDE = 0,
    SOURCE_SIDE = 1,
    SINK_SIDE = 2,
    BOTH_SIDES = 3
}STOP_KICK;

/** Call back prototype for operator messages with lists of operators */
typedef bool (*OP_STD_LIST_CBACK)(unsigned con_id, unsigned status,
        unsigned count, unsigned err_code);

/** Call back prototype for generic operator message */
typedef bool (*OP_MSG_CBACK)(unsigned con_id, unsigned status, unsigned op_id,
                             unsigned num_resp_params, unsigned *resp_params);

/** Call back prototype for create operator message */
typedef bool (*OP_CREATE_CBACK)(unsigned con_id, unsigned status, unsigned op_id);

/** Callback prototype for messages that return a status, an item count and a list of items */
typedef bool (*OP_INFO_LIST_CBACK)(unsigned con_id, unsigned status, unsigned length, const unsigned *info_list);

/** Callback prototype for messages that return a status, an item count and a list of items */
typedef bool (*OPID_LIST_CBACK)(unsigned con_id, unsigned status, unsigned total, unsigned length, const unsigned *info_list);

/** Callback prototype for GET_CAPABILITIES */
typedef bool (*CAP_INFO_LIST_CBACK)(unsigned con_id, unsigned status, unsigned total, unsigned length, const unsigned *info_list);

/** Call back prototype for operator messages with lists of operators */
typedef void (*OP_CON_ID_CBACK)(void *data);

/****************************************************************************
Public Constant Declarations
*/


/****************************************************************************
Public Macro Declarations
*/

/****************************************************************************
Public Variable Declarations
*/

/****************************************************************************
Public Function Declarations
*/
/**
 * \brief  Instantiate the specified capability as an Operator
 *
 * \param  con_id - The connection that created the
 *         operator.
 * \param  cap_id - The ID of the capability to instantiate,
 * \param  op_id - The operator ID provided by the client
 * \param  callback - Function pointer to the function to call with the response.
 */
extern void opmgr_create_operator(unsigned int con_id, CAP_ID cap_id,
        unsigned int op_id, OP_CREATE_CBACK callback);

/**
 * \brief  Instantiate the specified capability as an Operator
 *
 * \param  con_id - The connection that created the
 *         operator.
 * \param  cap_id - The ID of the capability to instantiate,
 * \param  op_id - The operator ID provided by the client
 * \param  num_keys - number of keys in the info array
 * \param  info - pointer to an array of key/value objects
 * \param  callback - Function pointer to the function to call with the response.
 */
extern void opmgr_create_operator_ex(unsigned int con_id, CAP_ID cap_id,
        unsigned int op_id, unsigned int num_keys,
        OPERATOR_CREATE_EX_INFO *info, OP_CREATE_CBACK callback);

/**
 * \brief  Test all the operators in the supplied list.
 *         The purpose of this is to test on Px (x=1,2,3) that processing
 *         proceeds fine up until the preproc function. At this point the
 *         function returns (via KIP) to P0, which then runs the preproc
 *         function, and then the destroy operator via KIP.
 *
 * \param  con_id - The connection that created the
 *         operator.
 * \param  num_ops - The number of operators in oplist.
 * \param  op_list - Pointer to an array of opids to destroy.
 * \param  callback - Function pointer to the function to call with the response.
 */
extern void opmgr_p1_run_until_preproc_operator(unsigned int con_id, unsigned int num_ops,
        unsigned int *op_list, OP_STD_LIST_CBACK callback);

/**
 * \brief  Destroy all the operators in the supplied list.
 *
 * \param  con_id - The connection that created the
 *         operator.
 * \param  num_ops - The number of operators in oplist.
 * \param  op_list - Pointer to an array of opids to destroy.
 * \param  callback - Function pointer to the function to call with the response.
 */
extern void opmgr_destroy_operator(unsigned int con_id, unsigned int num_ops,
        unsigned int *op_list, OP_STD_LIST_CBACK callback);

/**
 * \brief  Send message to specified operator.
 *
 * \param  con_id - The connection that created the
 *         operator.
 * \param  op_id - The id of the operator to send a message to.
 * \param  num_params - The length of the message pointed to by params
 * \param  params - A pointer to the memory containing the message
 * \param  callback - Function pointer to the function to call with the response.
 */
extern void opmgr_operator_message(unsigned int con_id, unsigned op_id,
        unsigned num_params, unsigned *params, OP_MSG_CBACK callback);

/**
 * \brief  Start all the operators in the supplied list.
 *
 * \param  con_id - The connection that created the
 *         operator.
 * \param  num_ops - The number of operators in oplist.
 * \param  op_list - Pointer to an array of opids to destroy.
 * \param  callback - Function pointer to the function to call with the response.
 */

extern void opmgr_start_operator(unsigned int con_id, unsigned int num_ops,
        unsigned int *op_list, OP_STD_LIST_CBACK callback);

/**
 * \brief  Stop all the operators in the supplied list.
 *
 * \param  con_id - The connection that created the
 *         operator.
 * \param  num_ops - The number of operators in oplist.
 * \param  op_list - Pointer to an array of opids to destroy.
 * \param  callback - Function pointer to the function to call with the response.
 */
extern void opmgr_stop_operator(unsigned int con_id, unsigned int num_ops,
        unsigned int *op_list, OP_STD_LIST_CBACK callback);

/**
 * \brief  Reset all the operators in the supplied list.
 *
 * \param  con_id - The connection that created the
 *         operator.
 * \param  num_ops - The number of operators in oplist.
 * \param  op_list - Pointer to an array of opids to destroy.
 * \param  callback - Function pointer to the function to call with the response.
 */
extern void opmgr_reset_operator(unsigned int con_id, unsigned int num_ops,
        unsigned int *op_list, OP_STD_LIST_CBACK callback);

/**
 * \brief  Connect the supplied cbuffer to the operator id
 *         endpoint provided
 *
 * \param  opidep - The operator endpoint to connect the buffer
 *         to.
 *
 * \param  Cbuffer_ptr - The cbuffer to connect to the endpoint.
 *
 * \param connected_wants_kicks - Boolean indicating whether the
 *        thing being connected wants to be kicked when this operator
 *        touches this terminal buffer.
 *
 * \return  TRUE if the buffer was connected to the endpoint.
 */
extern bool opmgr_connect_buffer_to_endpoint (unsigned int opidep, tCbuffer *Cbuffer_ptr, bool connected_wants_kicks);

/**
 * \brief  Disconnect the buffer from the supplied operator
 *         endpoint
 *
 * \param  opidep - The operator endpoint to disconnect the
 *         buffer from.
 *
 * \return  TRUE if the buffer was disconnected from the
 *          endpoint.
 */
extern bool opmgr_disconnect_buffer_from_endpoint (unsigned int opidep);

/**
 * \brief  Get the block size for the specified
 *         operator endpoints.
 *
 * \param  ep_id - The operator endpoint to get the block size for.
 *
 *
 * \param  ep_block_size - Pointer to block size to update.
 *
 */
extern void opmgr_get_block_size(unsigned int ep_id, unsigned int *ep_block_size);

/**
 * \brief  Get the scheduling information for an operator endpoint.
 *
 * NOTE: ep_locally_clocked is only valid if the endpoint supplied is real, otherwise
 * it should be ignored by the caller.
 *
 * \param endpoint_id - The operator endpoint to get the block size for.
 *
 * \param ep_block_size - Pointer to return the endpoint's block size in.
 *
 * \param ep_period - Pointer to return the endpoint's period in.
 *
 * \param ep_locally_clocked - Pointer to return whether the endpoint is locally
 *  clocked or not. (Only valid when endpoint is real.)
 *
 *  \param ep_wants_kicks - Pointer to return whether the operator wants
 *  to be kicked on this terminal or not.
 */
extern void opmgr_get_sched_info(unsigned int endpoint_id, unsigned *ep_block_size,
        unsigned *ep_period, bool *ep_locally_clocked, bool *ep_wants_kicks);

/**
 * \brief Sends a get clock id message to the operator for a given terminal.
 *
 * \param endpoint_id - The operator endpoint ID for whom the message is for.
 *
 * \return The clock id for that terminal, 0 represents the endpoint is not real.
 */
extern unsigned opmgr_get_operator_ep_clock_id(unsigned int endpoint_id);

/**
 * \brief  Sends a get configuration message to the operator.
 *
 * \param  endpoint_id - The operator endpoint ID for whom the message is for.
 *
 * \param  key   - Configuration key which specify the type of the get configuration.
 *
 * \param  value - Pointer which will be populated with the asked configuration value.
 *
 * \return TRUE if the message was successfully sent, FALSE otherwise
 *
 */
extern bool opmgr_get_config_msg_to_operator(unsigned int endpoint_id, unsigned int key, OPMSG_GET_CONFIG_RESULT *result);

/**
 * \brief  Sends a configure message to the operator.
 *
 * \param  endpoint_id - The operator endpoint ID for whom the message is for.
 *
 * \param  key   - Configuration key which specify the type of the configure.
 *
 * \param  value - The configure value which will be applied to the operator endpoint.
 *
 * \return TRUE if the message was successfully sent, FALSE otherwise
 *
 */
extern bool opmgr_config_msg_to_operator(unsigned int endpoint_id, unsigned int key, uint32 value);

/**
 * \brief  Get the operator capability ID.
 *
 * \param  ep_id - The operator endpoint to get the capability ID for.
 *
 * \return operator's capability ID.
 *
 */
extern unsigned int opmgr_get_op_capid(unsigned int ep_id);


/**
 * \brief  Get the processing time for an operator based on
 *         samples available at its input. This includes any
 *         overhead of switching tasks. I.e. this is the time
 *         from being asked to run until the data is available
 *         at the given output.

 *
 * \param  sink_id - The sink operator endpoint to get the
 *         processing time for.
 *
 * \param  words_to_process - The number of words to process.
 *
 * \return processing time in us.
 *
 */
extern unsigned int opmgr_get_processing_time(unsigned int sink_id,
                                               unsigned int words_to_process);

/**
 * \brief  Get the external operator ID from an operator's endpoint ID.
 *
 * \param  opidep - endpoint id containing information on opid, terminal and
 *         direction.
 *
 * \return The external ID of the operator.
 */
extern unsigned int get_ext_opid_from_opidep(unsigned int opidep);

/**
 * \brief This function is used for determining the operator endpoint ID for a given
 * terminal of the operator.
 *
 * \param  op_id operator id.
 * \param terminal_id the terminal id
 * \return  the terminal number on the operator the endpoint refers to.
 */
extern unsigned int get_opidep_from_opid_and_terminalid(unsigned int op_id, unsigned int terminal_id);

/**
 * \brief  Indicates whether an operator endpoint ID represents a valid operator
 *         terminal in the system.
 *
 * \param  opidep - endpoint id containing information on opid, terminal and
 *         direction.
 *
 * \return TRUE if there is a valid operator terminal in existence, FALSE if not.
 */
extern bool opmgr_is_opidep_valid(unsigned int opidep);

/**
 * \brief Gets the operator id given a terminal endpoint of that operator.
 *
 * \param  opidep the endpoint id of an operator terminal endpoint.
 *
 * \return The id of the operator.
 */
extern unsigned int get_opid_from_opidep(unsigned int opidep);

/**
 * \brief  Get the operator endpoint buffer details for a particular system stream rate.
 *
 * \param  endpoint_id - endpoint id containing information
 *         on opid, port and direction.
 *
 * \param  buff_details - pointer to a BUFFER_DETAILS structure to populate
 *
 * \return void
 */
extern bool opmgr_get_buffer_details( unsigned int endpoint_id,
                                      BUFFER_DETAILS *buff_details );

/**
 * \brief  Get endpoint data format.
 *
 * \param  opidep - endpoint id containing information on opid,
 *         port and direction.
 *
 * \return The audio data format required by the operator
 *         endpoint. Opmgr will panic if the operator requests
 *         an invalid data format for the specified endpoint.
 */
extern AUDIO_DATA_FORMAT opmgr_get_data_format(unsigned int opidep);

/**
 * \brief  Destroy all operators created by a specific
 *         connection ID.
 *
 * \param  con_id - The connection for which to destroy
 *         the operators.
 * \param  callback - Callback function
 * \param  data - Data pointer for the calback function.
 *
 * \return void
 */
extern void opmgr_destroy_ops_by_con_id(unsigned con_id, OP_CON_ID_CBACK callback, void *data);

/* TODO!!! - below, check what is now cap data and what is really op data! */
/**
 * \brief  Count the number of operators with matching capability. If capability ID is zero, count all operators.
 *
 * \param  capid - The capability ID to be searched for. If zero, all operators get counted.
 *
 * \return Number of instances of operator with matching capability or count of all operators.
 */
extern unsigned int opmgr_get_ops_count(unsigned int capid);


#ifdef INSTALL_DUAL_CORE_SUPPORT
/**
 * \brief  Count the number of operators with matching capability, on remote cores. If capability ID is zero, count all operators.
 *         It is only for P0 primary processor.
 *
 * \param  capid - The capability ID to be searched for. If zero, all operators get counted.
 *
 * \return Number of instances of operator with matching capability or count of all operators, on all cores.
 */
extern unsigned int opmgr_get_remote_ops_count(unsigned int capid);
extern unsigned int opmgr_get_list_remote_ops_count(unsigned int num_ops, unsigned int *op_list, uint16 proc_id);

/**
 * \brief  Get the processor id from the operator id
 *
 * \param ext_opid     - The (external) operator id
 * \param processor_id - pointer to the processor id to update
 *
 * \return TRUE if the operator endpoint exits
 */
extern bool opmgr_get_processor_id_from_opid(unsigned ext_opid, IPC_PROCESSOR_ID_NUM *processor_id);

/**
 * \brief  Get the processor id from the operator endpoint id
 *
 * \param ep_id - The operator endpoint id
 * \param processor_id - pointer to the processor id to update
 *
 * \return TRUE if the operator endpoint exits
 */
extern bool opmgr_get_processor_id( unsigned int ep_id, 
                                    IPC_PROCESSOR_ID_NUM *processor_id);

#endif /* INSTALL_DUAL_CORE_SUPPORT */


/**
 * \brief  Find out whether operator with specified op data pointer exists in the chain(s).
 *
 * \param  op_data - The OPERATOR_DATA pointer for the operator to be found.
 *
 * \return TRUE/FALSE depending on operator found or not.
 */
extern bool opmgr_does_op_exist(void* op_data);

/**
 * \brief Checks if the operator is running.
 *      If the operator doesnt't exist the function returns FALSE.
 *
* \param op_id The ID of the operator
 *
 * \return True if the operator is running false otherwise.
 */
extern bool is_op_running(unsigned int op_id);

/**
 * \brief Make an external oprator endpoint id
 *
 * \param opid - external operator id
 * \param idx - endpoint connection index
 * \param dir - endpoint direction, SOURCE or SINK
 *
 * \return The external operator endpoint id
 */
extern unsigned int opmgr_create_endpoint_id(unsigned int opid, unsigned int idx,
                                         ENDPOINT_DIRECTION dir);

/**
 * \brief Get a list of supported capability APIs
 *
 * \param con_id - ID of the connection with sender and recipient IDs
 * \param callback - callback function for response
 * \param start_index - Number of capabilities to skip from the beginning of the system's list of capabilities
 * \param max_count - Maximum number of capabilties to pass to callback function
 *
 */
extern void opmgr_get_capid_list(unsigned con_id, CAP_INFO_LIST_CBACK callback, unsigned start_index, unsigned max_count);


/**
 * \brief Get a list of ID pairs: created operator IDs and their respective capability IDs, for a certain cap ID or all of them
 *
 * \param con_id - ID of the connection with sender and recipient IDs
 * \param capid - capability ID. If zero, it will produce list of all operators for all capability IDs, otherwise list of operators for this capability ID
 * \param callback - callback function for response
 * \param start_index - number of operators to skip from the result of the search
 * \param max_count - maximum number of operators to pass to callback function
 *
 */
extern void opmgr_get_opid_list(unsigned con_id, unsigned capid, OPID_LIST_CBACK callback, unsigned start_index, unsigned max_count);

/**
 * \brief Get the capability information for a certain capability ID
 *
 * \param con_id - ID of the connection with sender and recipient IDs
 * \param capid - The id of the capability to get info for.
 * \param callback - callback function for response
 *
 */
extern void opmgr_get_capability_info(unsigned con_id, unsigned capid, OP_INFO_LIST_CBACK callback);

/**
 * \brief Access function to set operator's kickability flags
 *
 * \param  op_id - operator id
 * \param  side - The side where the kicks will be disabled.
 *
 */
extern void opmgr_stop_kicks(unsigned int op_id, STOP_KICK side);

/**
 * \brief Get the number of sink terminals an operator has
 *
 * \param opid The external id of the operator being queried
 *
 * \return the number of sink terminals
 */
extern unsigned opmgr_get_num_sink_terminals(unsigned opid);

/**
 * \brief Get the number of source terminals an operator has
 *
 * \param opid The external id of the operator being queried
 *
 * \return the number of source terminals
 */
extern unsigned opmgr_get_num_source_terminals(unsigned opid);

/**
 * \brief    Handle the unsolicited message from the operator
 *
 * \param    con_id          Connection id
 * \param    processor_id    Processor id where the operator is created
 * \param    *msg_from_op    Unsolicited messsage from the operator 
 * \param    *rinfo          Information of rounting source/destination
 */
extern bool opmgr_unsolicited_message(unsigned con_id,
                      unsigned processor_id,
                      OP_UNSOLICITED_MSG *msg_from_op,
                      tRoutingInfo *rinfo);

/*************************************************************************
 * \brief   Sets the creator id for all operators in a list.
 *          The operators in the list MUST be running on the primary core.
 *          Panics if an operator in the list is not found.
 * \param   op_list operator id list 
 * \param   num_ops number of operators
 * \param   con_id  connection id to set
 */
void opmgr_set_creator_id(const unsigned * op_list, unsigned num_ops, unsigned con_id);

/*
 *  \brief construct a simple unsolicited message without a payload
 *  \param op_id operator id
 *  \param client_id the external owner's client id, 0xFF if invalid.
 *  \param msg_id message id
 *  \param pointer to the length of the message constructed
 */
const OP_UNSOLICITED_MSG * opmgr_make_simple_unsolicited_message(
            unsigned op_id,
            unsigned client_id,
            unsigned msg_id,
            unsigned *length);

#endif /* OPMGR_H */


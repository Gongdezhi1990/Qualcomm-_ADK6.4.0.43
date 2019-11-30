/****************************************************************************
 * Copyright (c) 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  opmgr_op_client_interface.h
 * \ingroup  opmgr
 *
 * Operator Manager for Operator Clients. <br>
 * This file contains the operator manager functionalities that can be used by
 * an operator client. <br>
 */

#ifndef OPMGR_OPERATOR_CLIENT_INTERFACE_H
#define OPMGR_OPERATOR_CLIENT_INTERFACE_H

/****************************************************************************
Include Files
*/

#include "types.h"
#include "opmgr_for_ops.h"
/****************************************************************************
Public Type Declarations
*/
/* This is the type that should be used for operator ids. The operator client
 * will receive any reference to other operators through operator ids sent by
 * the application client. These ids should not be confused with the id used
 * internally by the framework, and thus marked as "internal". The ids to use
 * are here marked external (EXT_OP_ID) as they are used by the clients (both
 * application and operator client).
 */
#define OPERATOR_ID EXT_OP_ID

/****************************************************************************
Public Constant Declarations
*/
#define CLIENT_UNSOLICITED_MESSAGE_SIZE_EXTRA  (OPMSG_OP_CLIENT_UNSOLICITED_MESSAGE_PAYLOAD_WORD_OFFSET)
#define CLIENT_UNSOLICITED_MESSAGE_SIZE  (CLIENT_UNSOLICITED_MESSAGE_SIZE_EXTRA + OPCMD_MSG_HEADER_SIZE)
#define CLIENT_MSG_RESPONSE_SIZE_EXTRA (OPMSG_OP_CLIENT_MESSAGE_RESPONSE_PAYLOAD_WORD_OFFSET)
#define CLIENT_MSG_RESPONSE_SIZE (CLIENT_MSG_RESPONSE_SIZE_EXTRA + OPCMD_MSG_HEADER_SIZE)
#define CLIENT_CMD_RESPONSE_SIZE (OPMSG_OP_CLIENT_COMMAND_RESPONSE_WORD_SIZE + OPCMD_MSG_HEADER_SIZE)

/****************************************************************************
Public Macro Declarations
*/
/* Helper macros for creation of opmsgs to send to owned operators
 * These are necessary because the structure definitions and associated macros
 * in opmsg_prim are wrong for at least some platforms
 *
 * Example of declaring and filling in a message with fixed length:
 *
 * unsigned msg[MSGTYPE_WORD_SIZE];
 * OP_CLIENT_MSG_FIELD_SET(msg, MSGTYPE, FIELD, value);
 * opmgr_op_client_send_message(op_data, target_op_id, MSGTYPE_WORD_SIZE, msg);
 *
 * Example of declaring and filling in a message with payload at its end:
 *
 * unsigned len = payload_size + MSGTYPE_PAYLOAD_WORD_OFFSET;
 * unsigned *msg = xpnew(len, unsigned);
 * OP_CLIENT_MSG_FIELD_POINTER_GET(msg, MSGTYPE, PAYLOAD);
 * opmgr_op_client_send_message(op_data, target_op_id, len, msg);
 * pfree(msg);
 */
/* macro for reading a word from opmsg with an offset from a named field */
#define OPMSG_CREATION_FIELD_GET_FROM_OFFSET(msg, msgtype, field, offset) (((unsigned int *)msg)[msgtype##_##field##_WORD_OFFSET + offset])

/* macro for writing a word from opmsg with an offset from a named field */
#define OPMSG_CREATION_FIELD_SET_FROM_OFFSET(msg, msgtype, field, offset, value) ((((unsigned int *)msg)[msgtype##_##field##_WORD_OFFSET + offset]) = (value))

/* macro for retrieving a pointer to a word from opmsg with an offset from a named field */
#define OPMSG_CREATION_FIELD_POINTER_GET_FROM_OFFSET(msg, msgtype, field, offset) (&((unsigned int *)msg)[msgtype##_##field##_WORD_OFFSET + offset])

/* get a uint16 opmsg field */
#define OPMSG_CREATION_FIELD_GET(msg, msgtype, field) OPMSG_CREATION_FIELD_GET_FROM_OFFSET(msg, msgtype, field, 0)

/* set a uint16 opmsg field */
#define OPMSG_CREATION_FIELD_SET(msg, msgtype, field, value) OPMSG_CREATION_FIELD_SET_FROM_OFFSET(msg, msgtype, field, 0, value)

/* get a pointer to an opmsg field */
#define OPMSG_CREATION_FIELD_POINTER_GET(msg, msgtype, field) OPMSG_CREATION_FIELD_POINTER_GET_FROM_OFFSET(msg, msgtype, field, 0)

/* set a uint16 opmsg field */
#define OP_CLIENT_MSG_FIELD_SET(msg, msgtype, field, value) OPMSG_CREATION_FIELD_SET_FROM_OFFSET(msg, msgtype, field, 0, value)

/* get a pointer to an opmsg field */
#define OP_CLIENT_MSG_FIELD_POINTER_GET(msg, msgtype, field) OPMSG_CREATION_FIELD_POINTER_GET_FROM_OFFSET(msg, msgtype, field, 0)

/* macro for retrieving a pointer to a word from opmsg with an offset from a named field */
#define OPMSG_FIELD_POINTER_GET_FROM_OFFSET(msg, msgtype, field, offset) (&((unsigned int *)msg)[msgtype##_##field##_WORD_OFFSET + OPCMD_MSG_HEADER_SIZE + offset])

/* get a pointer to an opmsg field */
#define OPMSG_FIELD_POINTER_GET(msg, msgtype, field) OPMSG_FIELD_POINTER_GET_FROM_OFFSET(msg, msgtype, field, 0)


/****************************************************************************
Public Variable Declarations
*/

/****************************************************************************
Public Function Definitions
To be used for the development of an operator client.
*/

/**
 * \brief Send a message to one of the owned operators.
 *        This will fail if the target operator is not owned by this client.
 *
 * \param op_data OPERATOR_DATA of the requesting operator client
 * \param target_op_id External operator id of the target operator
 * \param msg_length Length of the message to send
 * \param msg Contents of the message to send. Note: this should be an array of
 *            16-bit values, stored in "unsigned" machine words.
 *
 * \return TRUE if the message has been successfully sent
 */
bool opmgr_op_client_send_message(OPERATOR_DATA *op_data, OPERATOR_ID target_op_id,
                                  unsigned msg_length, unsigned * msg);
/**
 * \brief Send a "start operator" request to a list of the owned operators.
 *        This will fail if the target operators are not owned by this client.
 *
 * \param op_data OPERATOR_DATA of the requesting operator client
 * \param num_ops Number of operators to start
 * \param op_list List of operators to start
 *
 * \return TRUE if the request has been successfully forwarded to opmgr
 */
bool opmgr_op_client_start_operator(OPERATOR_DATA *op_data,
                                    unsigned num_ops, OPERATOR_ID *op_list);

/**
 * \brief Send a "stop operator" request to a list of the owned operators.
 *        This will fail if the target operators are not owned by this client.
 *
 * \param op_data OPERATOR_DATA of the requesting operator client
 * \param num_ops Number of operators to stop
 * \param op_list List of operators to stop
 *
 * \return TRUE if the request has been successfully forwarded to opmgr
 */
bool opmgr_op_client_stop_operator(OPERATOR_DATA *op_data,
                                   unsigned num_ops, OPERATOR_ID *op_list);


/**
 * \brief Send a "reset operator" request to a list of the owned operators.
 *        This will fail if the target operators are not owned by this client.
 *
 * \param op_data OPERATOR_DATA of the requesting operator client
 * \param num_ops Number of operators to reset
 * \param op_list List of operators to reset
 *
 * \return TRUE if the request has been successfully forwarded to opmgr
 */
bool opmgr_op_client_reset_operator(OPERATOR_DATA *op_data,
                                    unsigned num_ops, OPERATOR_ID *op_list);

#endif /* OPMGR_OPERATOR_CLIENT_FRAMEWORK_H */


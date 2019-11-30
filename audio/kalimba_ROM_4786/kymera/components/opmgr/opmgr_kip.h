/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  opmgr_kip.h
 * \ingroup  opmgr
 *
 * Operator Manager KIP-related parts. <br>
 */

#ifndef OPMGR_KIP_H
#define OPMGR_KIP_H

/****************************************************************************
Include Files
*/
#include "opmgr.h"
#include "kip/kip_msg_adaptor.h"
#include "kip_mgr.h"
#include "opmgr/opmgr_for_ops.h"
/****************************************************************************
Type definitions
*/

/* Structure to keep extra create operator keys from create command */
typedef struct
{
    unsigned num_keys;
    OPERATOR_CREATE_EX_INFO* ex_info;
} OPMGR_CREATE_REQ_KEYS;

/****************************************************************************
Variable definitions
*/

/* Create-time extra keys kept during async create stages */
extern OPMGR_CREATE_REQ_KEYS create_req_keys;


/****************************************************************************
Function Definitions
*/

/* Free the memory allocated in create_req_keys. */
void opmgr_kip_free_req_keys(void);

/* Send 'list' type command via KIP: stop, reset, start, destroy */
bool opmgr_kip_build_send_list_cmd(unsigned con_id, unsigned num_ops,
                                   unsigned* op_list, uint16 kip_msg_id, void* callback);

/**
 * \brief    Send create operator REQ to adaptor
 *
 * \param    con_id  Connection id
 * \param    cap_id  Capability id
 * \param    op_id  The created operator id
 * \param    create_req_keys  the required key information (such as 
 *                            priority, processor id) for creating this operator
 * \param    callback  The callback function of create operator
 *
 */
bool opmgr_kip_build_send_create_op_req(unsigned con_id, unsigned cap_id, unsigned op_id,
                                        OPMGR_CREATE_REQ_KEYS *create_req_keys, void* callback);

/**
 * \brief    send operator message to KIP
 *
 * \param    con_id      Connection id
 * \param    op_id       The created operator id
 * \param    num_params  Length of parameters in message
 * \param    *params     Pointer to the Parameters
 * \param    callback    The callback function of operator message
 */
bool opmgr_kip_build_send_opmsg(unsigned con_id, unsigned op_id, unsigned num_params,unsigned* params, void* callback);
 

/**
 * \brief    Handle the create_operator_ex request from kip adaptor
 *
 * \param    con_id      Connection id
 * \param    op_id       The created operator id
 * \param    num_params  Length of parameters in operator message
 * \param    *params     Parameters in the operator message
 * \param    callback    The callback function of create operator for KIP
 */
void opmgr_kip_create_operator_ex_req_handler(unsigned int con_id,
                                     CAP_ID cap_id, unsigned int op_id,
                                     unsigned int num_keys,
                                     OPERATOR_CREATE_EX_INFO *info,
                                     OP_CREATE_CBACK callback);

/**
 * \brief    Handle the operator message request from kip adaptor
 *
 * \param    con_id      Connection id
 * \param    op_id       The created operator id
 * \param    num_params  Length of parameters in operator message
 * \param    *params     Parameters in the operator message
 * \param    callback    The callback function of operator message
 */
void opmgr_kip_operator_message_req_handler(unsigned con_id, unsigned status,
                                            unsigned op_id,unsigned num_param,
                                            unsigned *params, OP_MSG_CBACK callback);
                  
#ifdef INSTALL_DUAL_CORE_SUPPORT
/**
 * \brief    Handle the create operator response from kip
 *
 * \param    con_id  Connection id
 * \param    status  Status of the request
 * \param    op_id  The created operator id
 * \param    callback  The callback function of create operator
 *
 * \return   Endpoint if successfull else NULL.
 */
extern void opmgr_kip_create_resp_handler(unsigned con_id, unsigned status,
                                          unsigned op_id, OP_CREATE_CBACK callback);

/**
 * \brief    Handle the std list operator (start/stop/reset/destroy) response from kip
 *
 * \param    con_id    Connection id
 * \param    status    Status of the request
 * \param    count     Number of operators handled successfully
 * \param    err_code  Error code upon operator error
 * \param    callback  The callback function of create operator
 */
void opmgr_kip_stdlist_resp_handler(unsigned con_id, unsigned status,
                                    unsigned count,  unsigned err_code,
                                    OP_STD_LIST_CBACK callback);
#endif

/**
 * \brief    Handle the operator message response from kip
 *
 * \param    con_id           Connection id
 * \param    status           Status of the request
 * \param    op_id            The created operator id
 * \param    num_resp_params  Length of response message
 * \param    *resp_params     Response message payload
 * \param    callback         The callback function of operator message from kip
 */
void opmgr_kip_operator_msg_resp_handler(unsigned con_id, unsigned status,
                                         unsigned op_id, unsigned num_resp_params,
                                         unsigned *resp_params, void *context);

/**
 * \brief    Send the unsolicited message to KIP
 *
 * \param    con_id          Connection id
 * \param    *msg_from_op    Unsolicited messsage from the operator 
 */
extern bool opmgr_kip_unsolicited_message(unsigned con_id, OP_UNSOLICITED_MSG *msg_from_op);
#endif /* OPMGR_KIP_H */


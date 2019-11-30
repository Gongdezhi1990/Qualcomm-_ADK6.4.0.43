/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  opmgr_kip.c
 * \ingroup  opmgr
 *
 * Operator Manager KIP-related parts. <br>
 */

/****************************************************************************
Include Files
*/

#include "opmgr_private.h"
#include "kip_msg_prim.h"
#include "patch.h"

/****************************************************************************
Variable definitions
*/

OPMGR_CREATE_REQ_KEYS create_req_keys;

/****************************************************************************
Function Definitions
*/

/* Free the memory allocated in create_req_keys. */
void opmgr_kip_free_req_keys(void)
{
    if (create_req_keys.num_keys > 0)
    {
        pdelete(create_req_keys.ex_info);
        create_req_keys.ex_info = NULL;
        create_req_keys.num_keys = 0;
    }
}

/* Send 'list' type command via KIP: stop, reset, start, destroy */
bool opmgr_kip_build_send_list_cmd(unsigned con_id, unsigned num_ops, unsigned* op_list, uint16 kip_msg_id, void* callback)
{
    bool status = FALSE;
    unsigned length = KIP_MSG_OPLIST_CMD_REQ_OP_LIST_WORD_OFFSET + num_ops;

    uint16* msg_data = (uint16*)xpmalloc(length*sizeof(uint16));

    if(msg_data != NULL)
    {
        msg_data[KIP_MSG_OPLIST_CMD_REQ_CON_ID_WORD_OFFSET] = con_id;
        msg_data[KIP_MSG_OPLIST_CMD_REQ_COUNT_WORD_OFFSET] = num_ops;

        unsigned opcount;
        for(opcount=0; opcount < num_ops; opcount++)
        {
            msg_data[KIP_MSG_OPLIST_CMD_REQ_OP_LIST_WORD_OFFSET + opcount] = (uint16)op_list[opcount];
        }

        status = kip_adaptor_send_message(con_id, kip_msg_id, length,
                                       (unsigned*)msg_data, callback);

        pfree(msg_data);
    }

    return status;
}

/**
 * \brief    Send create operator REQ to adaptor
 *
 * \param    con_id  Connection id
 * \param    cap_id  Capability id
 * \param    op_id  The created operator id
 * \param    create_req_keys  the required key information (such as
 *                            priority, processor id) for creating this operator
 * \param    callback  The callback function of create operator request
 *
 */
bool opmgr_kip_build_send_create_op_req(unsigned con_id, unsigned cap_id, unsigned op_id,
                                         OPMGR_CREATE_REQ_KEYS *create_req_keys, void* callback)
{
    patch_fn_shared(kip);

    /* Send KIP message to create it on remote processor. The KIP response will lead to
     * the API callback being called, so use some housekeeping there to call the callback when response comes back.
     */
    bool status = FALSE;
    unsigned length = KIP_MSG_CREATE_OPERATOR_REQ_INFO_WORD_OFFSET +
                      create_req_keys->num_keys*sizeof(OPERATOR_CREATE_EX_INFO)/sizeof(uint16);

    uint16* msg_data = (uint16*)xzpmalloc(length*sizeof(uint16));

    if(msg_data != 0)
    {
        msg_data[KIP_MSG_CREATE_OPERATOR_REQ_CON_ID_WORD_OFFSET]        = con_id;
        msg_data[KIP_MSG_CREATE_OPERATOR_REQ_CAPABILITY_ID_WORD_OFFSET] = cap_id;
        msg_data[KIP_MSG_CREATE_OPERATOR_REQ_OP_ID_WORD_OFFSET]         = op_id;
        msg_data[KIP_MSG_CREATE_OPERATOR_REQ_NUM_KEYS_WORD_OFFSET] = create_req_keys->num_keys;

        /* Copy the keys */
        OPERATOR_CREATE_EX_INFO* msg_ex_info = (OPERATOR_CREATE_EX_INFO*)&msg_data[KIP_MSG_CREATE_OPERATOR_REQ_INFO_WORD_OFFSET];
        unsigned count;

        for(count=0; count < create_req_keys->num_keys; count++)
        {
            msg_ex_info[count].key = create_req_keys->ex_info[count].key;
            msg_ex_info[count].value = create_req_keys->ex_info[count].value;
        }

        status = kip_adaptor_send_message(con_id, KIP_MSG_ID_CREATE_OPERATOR_REQ,
                 length, (unsigned*)msg_data, callback);

        pfree(msg_data);
    }

    return status;
}

/**
 * \brief    send operator message to KIP
 *
 * \param    con_id      Connection id
 * \param    op_id       The created operator id
 * \param    num_params  Length of parameters in message
 * \param    *params     Pointer to the Parameters
 * \param    callback    The callback function of operator message
 *
 */
bool opmgr_kip_build_send_opmsg(unsigned con_id, unsigned op_id, unsigned num_params,
                                       unsigned* params, void* callback)
{
    patch_fn_shared(kip);

    bool status = FALSE;

    uint16 length = KIP_MSG_OPERATOR_MESSAGE_REQ_OP_MESSAGE_WORD_OFFSET + num_params;
    uint16* kip_msg = xpnewn(length, uint16);

    if(kip_msg != NULL)
    {
        /* Conid already has remote processor ID added by ACCMD adaptor */
        kip_msg[KIP_MSG_OPERATOR_MESSAGE_REQ_CON_ID_WORD_OFFSET] = con_id;
        kip_msg[KIP_MSG_OPERATOR_MESSAGE_REQ_OPID_WORD_OFFSET]   = op_id;

        adaptor_pack_list_to_uint16(&kip_msg[KIP_MSG_OPERATOR_MESSAGE_REQ_OP_MESSAGE_WORD_OFFSET], params, num_params);

        /* Callback provided for return/response path, so a response from Pn will lead to calling this
         * external API callback to respond to client.
         */
        status = kip_adaptor_send_message(con_id,KIP_MSG_ID_OPERATOR_MSG_REQ,
                                       length,(unsigned*)kip_msg, callback);

        pfree(kip_msg);
    }

    return status;
}

/***************** Request handler functions *********************/

/**
 * \brief    Handle the create_operator_ex request from KIP adaptor
 *
 * \param    con_id      Connection id
 * \param    cap_id      Capability id
 * \param    op_id       The created operator id
 * \param    num_keys    The number of keys
 * \param    *info       Key information
 * \param    callback    The callback function of create operator for KIP
 */
void opmgr_kip_create_operator_ex_req_handler(unsigned int con_id,
                                     CAP_ID cap_id, unsigned int op_id,
                                     unsigned int num_keys,
                                     OPERATOR_CREATE_EX_INFO *info,
                                     OP_CREATE_CBACK callback)
{
    opmgr_create_operator_ex(con_id, cap_id, op_id, num_keys, info, callback);
}
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
                                            unsigned *params, OP_MSG_CBACK callback)
{
    patch_fn_shared(kip);

    if(status == STATUS_OK)
    {
        opmgr_operator_message(con_id, op_id, num_param, params, callback);
    }
    else
    {
        callback(con_id, status, 0, 0, NULL);
    }
}

/***************** Response handler functions *********************/

#ifdef INSTALL_DUAL_CORE_SUPPORT
/**
 * \brief    Handle the create operator response from kip
 *
 * \param    con_id  Connection id
 * \param    status  Status of the request
 * \param    op_id  The created operator id
 * \param    callback  The callback function of create operator
 */
void opmgr_kip_create_resp_handler(unsigned con_id, unsigned status,
                                   unsigned op_id, OP_CREATE_CBACK callback)
{
    patch_fn_shared(kip);

    PL_ASSERT(NULL != callback);

    if(status != STATUS_OK)
    {
        /* Remove the given operator's data structure from the remote operator list */
        remove_op_data_from_list(EXT_TO_INT_OPID(op_id), &remote_oplist_head);
    }

    callback(con_id, status, op_id);
}

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
                                    OP_STD_LIST_CBACK callback)
{
    patch_fn_shared(kip);

    if (callback != NULL)
    {
        callback(con_id, status, count, err_code);
    }
    else
    {
        PL_PRINT_P0(TR_DUALCORE, "No context for KIP_MSG_ID_X_OPERATOR_RES. (X = DESTROY or START or STOP or RESET)\n");
    }
}
#endif /* INSTALL_DUAL_CORE_SUPPORT */


/**
 * \brief    Handle the operator message response from kip
 *
 * \param    con_id           Connection id
 * \param    status           Status of the request
 * \param    op_id            The created operator id
 * \param    num_resp_params  Length of parameters in response message
 * \param    *resp_params     Parameters in response message
 * \param    callback         The callback function of operator message from kip
 */
void opmgr_kip_operator_msg_resp_handler(unsigned con_id, unsigned status,
                                         unsigned op_id, unsigned num_resp_params,
                                         unsigned *resp_params, void *context)
{
    patch_fn_shared(kip);

    OP_MSG_CBACK callback = (OP_MSG_CBACK)context;

    if(status == STATUS_OK)
    {
        /* convert the response to the shape that the adaptor wants - if there is a response message */
        callback(con_id, status, op_id, num_resp_params, resp_params);
    }
    else
    {
        /* the infamous case of no response - one convention here is that we can still call back with NULLs and let it take whatever action */
        callback(con_id, status, 0, 0, NULL);
    }
}

/**
 * \brief    Send the unsolicited message to KIP
 *
 * \param    con_id          Connection id
 * \param    *msg_from_op    Unsolicited messsage from the operator
 */
bool opmgr_kip_unsolicited_message(unsigned con_id, OP_UNSOLICITED_MSG *msg_from_op)
{
    patch_fn_shared(kip);

    bool status = FALSE;
    OP_UNSOLICITED_MSG *msg = msg_from_op;
    /* The unsolicitied message total length is the actual payload length
     * plus the four fields in the message header*/
    unsigned msg_length = msg->length + UNSOLICITED_MSG_HEADER_SIZE;
    unsigned length = KIP_MSG_MESSAGE_FROM_OPERATOR_REQ_MESSAGE_WORD_OFFSET + msg_length;

    uint16* kip_msg = (uint16*)xzpmalloc(length*sizeof(uint16));

    if(kip_msg != NULL)
    {
        kip_msg[KIP_MSG_MESSAGE_FROM_OPERATOR_REQ_CON_ID_WORD_OFFSET] = con_id;
        kip_msg[KIP_MSG_MESSAGE_FROM_OPERATOR_REQ_OPID_WORD_OFFSET]   = msg->op_id;

        /* Convert the internal op_id into external for the unsolicited message body */
        msg->op_id = INT_TO_EXT_OPID(msg->op_id);

        adaptor_pack_list_to_uint16(&kip_msg[KIP_MSG_MESSAGE_FROM_OPERATOR_REQ_MESSAGE_WORD_OFFSET], (unsigned*)msg, msg_length);//msg->length);

        /* Reverse the connection id here because we are sending message from P1 to P0 now */
        status = kip_adaptor_send_message(REVERSE_CONNECTION_ID(con_id),KIP_MSG_ID_UNSOLICITED_FROM_OP_REQ,
                                         length,(unsigned*)kip_msg, NULL);

        pfree(kip_msg);
    }

    return status;

}

